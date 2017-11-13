// -----------------------------------
// Halloween 2017
// -----------------------------------
/*
This is the particle softare for controlling props and a scarecrow with pan, tilt, jaw, and eyes. Special thanks 
to Simon Monk for the Event and NewTimer classes.

Written By: Guy Bieber
Date: 2017-10 

Required Changes
STEP 1: Tune your servos. It is easiest to open the paremters up:
  initServo(jawServo, jawServoPin, 0, 180, "50"); 
  initServo(neckTiltServo, neckTiltServoPin, 0, 180, "50");
  initServo(neckPanServo, neckPanServoPin, 0, 180, "50"); 
Then use these commands in the particles console to figure out min, max, and default ranges.
  "servoJaw"
  "servoPan"
  "servoTilt"
The system sets a min and max (0 to 180 degrees) and then maps the remaining movement to a scale of 0 to 100 percent.

FUTURE:
1) Refactor head control functions into a class.

NOTE MAX OF 63 CHARACTERS ON A FUNCTION PARAMETER and MAX of 12 on a function name.

LICENSE
Copyright (c) 2017 Guy Bieber

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include "Event.h"
#include "NewTimer.h"

//PIR Motion sensor parameters. Note by using pin seven the onboard led shows when motion is sensed.
int led0 = D7; // on board led mainly for testing
int pirMotion = D7;
bool pirMotionReady = false;
int pirMotionTriggered = 0;

//leds
int led1 = D6; // can control external LED
int led2 = D3; // can control external LED 

// Transistors 
int transistor1 = D5;
int transistor2 = D4;

// servos 
int jawServoPin = D0; // has to be pwm
int neckPanServoPin = D1; // has to be pwm
int neckTiltServoPin = D2; // has to be pwn
typedef struct {
  int pin; 
  int minSafeDeg;
  int maxSafeDeg; 
  int currentPos; 
  Servo servo; 
} ServoType; 
ServoType jawServo; 
ServoType neckPanServo;
ServoType neckTiltServo;

// parameter parsing. For the sequences we take in a comma separated list of <delay_ms>,<action>.
int MAX_PARAMS = 2;
char DELIMITER =',';

//eyes
typedef struct {
  int leftEye;
  int rightEye;
  int leftLevel; // 0 to 1
  int rightLevel; // 0 to 1
} EyesType;
EyesType eyes; 

//NewTimer is used to schedule callbacks so we don't have to do a blocking delay
NewTimer t; 
int pirMotionInitEvent;
int checkPirEvent;

//variables for debugging 
int servoPin = -1;
int servoTarget = -1;

//sequence variables
int busy = 0;
String sequenceString = "";

/* 
This controls the led and transistor pins.

Command:
"on" turn the pin HIGH
"off" turn the pin LOW
"pulse" turn the pin HIGH, wait 1s, turn the pin LOW

Return Values:
0 = pin is left low (true for off and pulse)
1 = pin is left high 
-1 = invalid command
*/
int pinControl(String command, int pin) {


    if (command=="on") {
        digitalWrite(pin,HIGH);
        return 1;
    }
    else if (command=="off") {
        digitalWrite(pin,LOW);
        return 0;
    }
    else if (command=="pulse") {
        t.pulseImmediate(pin, 500, HIGH);
        return 0;
    }
    else {
        return -1;
    }
}

/*
This callback is used to enable the PIR sensor after its startup stabilization period.
*/
int pirMotionInitialize(String param) {
    pirMotionReady = true;
    return 1;
}

/*
This function reads the PIR sensor and publish motion if it was triggered.
*/
int checkPir(String param) {
    // if the pir has been initialized
    if (pirMotionReady) {
        //read the sensor 
       int reading = digitalRead(pirMotion);
        //if the reading has changed
        if (reading != pirMotionTriggered) {
            pirMotionTriggered = reading; 
            // if it just went high then we triggered
            if (reading) {
                // send a message
                Particle.publish("motion");
            }
                
        }
    }
    return 1;
}

// published function for led1
int led1Control(String command) {
    return pinControl(command, led1);
}

// published function for led2
int led2Control(String command) {
    return pinControl(command, led2);
}

// published function for transistor1
int transistor1Control(String command) {
    return pinControl(command, transistor1);
}

// published function for transistor2
int transistor2Control(String command) {
    return pinControl(command, transistor2);
}

/* 
This function is used to control the servo by turning the command (0 to 100) into a safe movement.
This returns the percentage in the safe range this is set to.
*/
int servoControlDoit(String command, ServoType &servo) {
  // convert command into percentage
  int pos = command.toInt();
  if ((pos >= 0) && (pos <= 100)) {
    // calculate the target in degrees
    servo.currentPos = pos;
    int range = servo.maxSafeDeg - servo.minSafeDeg;
    float targetfloat = (float) range * ( (float) servo.currentPos / 100.0) + (float) servo.minSafeDeg;
    int target = (int) targetfloat;

    //Particle.publish(String(target));
    //write the servo
    servoPin = servo.pin;
    servoTarget = target; 
    
    servo.servo.write(target); 
    //servo.servo.write(90); 
    //return the current position
    return servo.currentPos;
  } else {
    return -1;
  }
}

/* 
This controls the jaw servo.
Command is percentage between min and max 
*/
int servo_jaw_control(String command) {
  return servoControlDoit(command, jawServo);
}

/* 
This controls the neck pan servo.
Command is percentage between min and max 
*/
int servo_neck_pan_control(String command) {
  return servoControlDoit(command, neckPanServo);
}

/* 
This controls the neck tilt servo.
Command is percentage between min and max 
*/
int servo_neck_tilt_control(String command) {
  return servoControlDoit(command, neckTiltServo);
}

/* 
This controls one of both of the eyes.
*/
int eyeDoit (String eye, int level) {
  /*
  if ((level < 0) && (level > 10)) {
    return -1;
  }
  */
  // check the eye is valid and take action
  if (eye == "left") {
    eyes.leftLevel = level;
    digitalWrite(eyes.leftEye,eyes.leftLevel);
  } else if (eye == "right") {
    eyes.rightLevel = level;
    digitalWrite(eyes.rightEye,eyes.rightLevel);
  } else if (eye == "both") {
    eyes.rightLevel = level;
    eyes.leftLevel = level;
    digitalWrite(eyes.rightEye,eyes.rightLevel);
    digitalWrite(eyes.leftEye,eyes.leftLevel);
  } else {
    return -1;
  }

  // got here everything worked
  return 1;
}

/* 
This prepends a sequence to add to the existing scarecrow sequence. 
This has the effect of expanding complex commands.
*/
int scarecrow_sequenceControlAdd(String command) {
    String newString = command + "," + sequenceString; 
    sequenceString = newString;
    String str = "SequenceAdd=" + sequenceString;
    Particle.publish(str);
    return 1; 
}

/*
Make the head level
*/
int headLevel () {
    return servoControlDoit("25", neckTiltServo);
}

/*
Make the scarecrow smile. 
*/
int smile() {
    servoControlDoit("100", jawServo);
}

/* 
This is the main scarecrow control function. 
*/
int scarecrowControlInt (String command) {
    
  String str;
  int ret_val = 1;
  
  str = "doing=" + command;
  Particle.publish(str);

  if (command == "yes") {
    //head level, center, down, level
    scarecrow_sequenceControlAdd ("5,smile,5,level,50,center,50,down,400,level");
  } else if (command == "no") {
    //head level, center, right, left, center
    scarecrow_sequenceControlAdd ("5,level,50,center,50,right,400,left,400,center");
  } else if (command == "center") {
    //head level
    headLevel();
    //center
    ret_val = servoControlDoit("50", neckPanServo);
  } else if (command =="left") {
    //head level
    headLevel();
    //turn left
    ret_val = servoControlDoit("80", neckPanServo);
  } else if (command =="right") {
    //head level
    headLevel();
    //turn right
    ret_val = servoControlDoit("0", neckPanServo);
  } else if (command == "up") {
    //head up
    ret_val = servoControlDoit("100", neckTiltServo);
  } else if (command == "down") {
    //close mouth
    smile();
    //head down
    ret_val = servoControlDoit("0", neckTiltServo);
  } else if (command == "level") {
    //head level
    headLevel();
  } else if (command == "grimace") {
    //head level
    headLevel();
    //half open mouth
    ret_val = servoControlDoit("50", jawServo);
  } else if (command == "teeth") {
    //head level
    headLevel();
    //open mouth
    ret_val = servoControlDoit("0", jawServo);
  } else if (command == "smile") {
    //close mouth
    smile();
  } else if (command == "chomp") {
    //head level, teeth, smile, teeth, smile
    scarecrow_sequenceControlAdd ("5,level,50,teeth,500,smile,500,teeth,500,smile");
  } else if (command == "wake") {
    // head level, center, eyes on 
    scarecrow_sequenceControlAdd ("5,level,50,center,50,eyes_on");
  } else if (command == "sleep") {
    // head level, center, down, eyes off
    scarecrow_sequenceControlAdd ("5,eyes_crazy,700,level,50,center,50,down");
  } else if (command == "eyes_crazy") {
    //scarecrow_sequenceControlAdd("50,eyes_on,50,eyes_off,100,eyes_on,100,eyes_off,200,eyes_on,200,eyes_off");
    // both eyes off
    eyeDoit("both", LOW);
    // flicker 5 times 
    t.oscillate(eyes.leftEye, 100, LOW, 5);
    t.oscillate(eyes.rightEye, 100, LOW, 5);
  } else if (command == "eyes_blink") {
    // both eyes on 
    eyeDoit("both", HIGH);
    // blink right eye for 1 second.
    t.pulseImmediate(eyes.leftEye, 500, LOW);
    t.pulseImmediate(eyes.rightEye, 500, LOW);
  } else if (command == "eyes_wink_left") {
    // both eyes on 
    eyeDoit("both", HIGH);
    // blink right eye for 1 second.
    t.pulseImmediate(eyes.leftEye, 1000, LOW);
  } else if (command == "eyes_wink_right") {
    // both eyes on 
    eyeDoit("both", HIGH);
    // blink right eye for 1 second.
    t.pulseImmediate(eyes.rightEye, 1000, LOW);
  } else if (command == "eyes_on") {  
    eyeDoit("both", HIGH);
  } else if (command == "eyes_off") {  
    eyeDoit("both", LOW);
  } else {
    String str = "Bad Command:" + command;
    Particle.publish(str);
    ret_val = -1;
  }
  
  // see if there is anything else to do
  scarecrow_sequenceControlInt(""); 
  return ret_val; 
}

/*
sInput = input line
cDelim = delimiter character
sParmas[] = output parameters
iMaxParams = max parameters
index = index to the next unread part of the string; -1 if nothing left 
return = number of parsed parameters
*/
int StringSplit(String sInput, char cDelim, String sParams[], int iMaxParams, int &index)
{
    int iParamCount = 0;
    int iPosDelim, iPosStart = 0;
    do {
        // Searching the delimiter using indexOf()
        iPosDelim = sInput.indexOf(cDelim,iPosStart);
        if (iPosDelim >= (iPosStart+1)) {
            // Adding a new parameter using substring() 
            sParams[iParamCount] = sInput.substring(iPosStart,iPosDelim);
            //String str = "delay=" + sParams[iParamCount] + "from,to=" + iPosStart + "," + iPosDelim;
            //Particle.publish(str);
            iParamCount++;
    
            iPosStart = iPosDelim + 1;
            // Checking the number of parameters
            if (iParamCount >= iMaxParams) {
                index = iPosDelim + 1; 
                return (iParamCount);
            }

        }
    } while (iPosDelim >= 0);
    if (iParamCount < iMaxParams) {
        // Adding the last parameter as the end of the line
        sParams[iParamCount] = sInput.substring(iPosStart);
        iParamCount++;
    }
    // denote we reached the end 
    index = -1; 
    return (iParamCount);
}


//the string is a comma separated list of scarecrow commands
int scarecrow_sequenceControlInt (String param) {

  //Particle.publish("log_ControlInt Start");
  // pull the next item off the string 
  String commands[MAX_PARAMS];
  int commandCount;
  int index; 
  String str;
  
  str = "Sequencing=" + sequenceString;
  Particle.publish(str);
  
  // if the command has something left in it
  if (sequenceString.length() > 0) {
    //parse off the command
    commandCount = StringSplit(sequenceString, DELIMITER, commands, MAX_PARAMS, index);

    //adjust the string 
    //if we have reached the end then set it to the empty string 
    if (index == -1) {
      sequenceString = "";
    // else we need to save the rest of the string for next time.
    } else {
      String newString = sequenceString.substring(index); 
      sequenceString = newString;
    }
    
    // if we got the right amount do the command
    if (commandCount == MAX_PARAMS) {
      //schedule the action
      int delay = commands[0].toInt();
      str = "Sceduling=" + commands[0] + "," + commands[1];
      Particle.publish(str);
      t.after(delay, scarecrowControlInt, commands[1]);
        
    // if we didnt get properly formated commands
    } else {
      //String str = "bad command=" + 
      //Particle.publish(str);
      // malformed... dump the rest of the command 
      sequenceString = "";
      busy = 0;
      return -1;
    }
    
  // empty string the sequence is done. we are ready for a new command  
  } else {
    busy = 0;
  }
  
  return 1; 
}

/*
This is the publish control function that allows prior commands to finish before taking on new ones.
*/
int scarecrow_sequenceControl (String command) {
    // if we are not busy 
    if (busy == 0) {
        // this is the only place we set busy
        busy = 1;
        scarecrow_sequenceControlAdd(command);
        return scarecrow_sequenceControlInt("");
    } else {
        return -1;
    }
}

// helper function to control scarecrow without a delay
int scarecrowControl (String command) {
    String newCommand = "5," + command; 
    return scarecrow_sequenceControl(newCommand);
}

// this function intitializes a servo
int initServo(ServoType &servo, int pin, int minSafeDeg, int maxSafeDeg, String command) {
  servo.pin = pin;
  servo.minSafeDeg = minSafeDeg;
  servo.maxSafeDeg = maxSafeDeg;
  pinMode(servo.pin, OUTPUT);
  servo.servo.attach(servo.pin); 
  return servoControlDoit(command, servo);
}

// this function initializes the scarecrow
void initScarecrow () {
  // add raw led control
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  Particle.function("led1",led1Control);
  Particle.function("led2",led2Control);
  
  //add eye control
  eyes.leftEye = led2;
  eyes.rightEye = led1;
  eyeDoit("both", LOW);

  //add raw servo control
  // STEP 1: Tune your servos
  initServo(jawServo, jawServoPin, 0, 120, "100"); 
  initServo(neckTiltServo, neckTiltServoPin, 10, 90, "40");
  initServo(neckPanServo, neckPanServoPin, 0, 85, "50"); 

  //raw servo control
  Particle.function("servoJaw", servo_jaw_control);
  Particle.function("servoPan", servo_neck_pan_control);
  Particle.function("servoTilt", servo_neck_tilt_control);

  // add main scarecrow control
  Particle.function("scarecrow", scarecrowControl);
  Particle.function("scarecrowSeq", scarecrow_sequenceControl);
  
  // variables for debugging 
  Particle.variable("sequenceStr", sequenceString);
  Particle.variable("busy", busy);
  Particle.variable("ServoPin", servoPin);
  Particle.variable("ServoPos", servoTarget);
}

// setup 
void setup()
{
   // Here's the pin configuration, same as last time
   pinMode(pirMotion, INPUT);  
   pinMode(transistor1, OUTPUT);
   pinMode(transistor2, OUTPUT); 

   // For good measure, let's also make sure both LEDs are off when we start:
   digitalWrite(transistor1, LOW);  
   digitalWrite(transistor2, LOW); 

   // enable cloud based control of transitors 
   Particle.function("transistor1", transistor1Control);
   Particle.function("transistor2", transistor2Control);
   
   //setup function to wait for PIR to initialize 1m
   pirMotionInitEvent = t.after(1000*60, pirMotionInitialize, "");
   
   //check the PIR sensor every 500ms
   checkPirEvent = t.every(500, checkPir, "");
   initScarecrow();
}

// loop just calls the timer function which schedules everything
void loop()
{
    // let the timer figure out when it is time to run things.
    t.update();
}
    


