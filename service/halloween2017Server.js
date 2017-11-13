/*
This software Node software communicates with the particle cloud to control halloween props.

Written By: Guy Bieber
Date: 2017-10 

Required Changes
STEP 1) You need to put in your particle token.
STEP 2) You need to put in your devices and device ids
STEP 3) Enumerate your props
STEP 4) Write your motion callbacks
STEP 5) Enumerate your motion sensors


FUTURE:
1) Refactor into a class that takes in configuation from files and allows motion callback registration.

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

// required to access the particle cloud
var Particle = require('particle-api-js');
var particle = new Particle();

// Particle Service Access Token
// STEP 1) You need to put in your particle token.
var token = ""; 
var particleAddress = "https://api.particle.io/v1/devices/";

// Particle Devices enummerated as <device_name> : <deviceId>
// STEP 2) You need to put in your devices and device ids
var deviceIDs = {
	guy_spark1:'', 
	guy_spark2:'', 
	guy_spark3:'', 
	guy_spark4:'', 
	guy_spark5:'', 
	guy_spark6:'',
	guy_spark7:''
};

// This is a mapping of the props to devices, io, and actions. This is in the format:
// <prop> : {device:<device name>, method:<method>, action:<action> }	
// The actions are in the particle code. Here is a summary. All props have the following:
// 		pulse - pulses the trigger for the prop to set it off
//      on - turns on the signal
//      off - turns off the signal
// For the scarecrow the following commands are possible:
//		yes - Nods head yes.
//		no - shakes head no
// 		center - pans head to center
//		left - turns head to left
//		right - turns head to right
//		up - puts head all the way up
//		down - puts head all the way down
// 		level - makes head level
// 		grimace - half opens mouth
//		teeth - fully opens mouth showing teeth
// 		smile - closes mouth hiding teeth
//		chomp - bites twice
// 		wake - makes head level, center, and turns on eyes
// 		sleep - eyes go crazy, head goes center and down
//		eyes_crazy - eyes flicker and go off
//		eyes_blink - eyes blink off and on once
//		eyes_wink_left - left eye winks on and off with right eye on
//		eyes_wink_right - right eye winks on and off with left eye on
//		eyes_on - turns both eyes on
// 		eyes_off - turns both eyes off
//		scarecrowSeq - does a sequence of scarecrow commands based on a comma separated list 
//         in the format <delay>,<action>. <delay> is the time to wait in ms before doing the <action>.
// STEP 3) Enumerate your props
var props = {
  	electric_wire_r:{ 
		device:'guy_spark1',
		method:'transistor1',
		action:'pulse'},
	electric_wire_l:{
		device:'guy_spark4',
		method:'transistor2', //1',
		action:'pulse' },
	electric_box:{
		device:'guy_spark4',
		method:'transistor1', //2',
		action:'pulse' },	
	grave_grabber:{
		device:'guy_spark7',
		method:'transistor1',
		action:'pulse'},
	tree:{
		device:'guy_spark6', 
		method:'transistor1',
		action:'pulse'},
	urn:{ 
		device:'guy_spark5',
		method:'transistor1',
		action:'pulse' },
	lamp:{ 
		device:'guy_spark2',
		method:'led1',
		action:'pulse'},
	ball:{ 
		device:'guy_spark2',
		method:'transistor1',
		action:'pulse'},
	scarecrow_yes:{
		device:'guy_spark3',
		method:'scarecrow',
		action:'yes'},
	scarecrow_no:{
		device:'guy_spark3',
		method:'scarecrow',
		action:'no'},
	scarecrow_center:{
		device:'guy_spark3',
		method:'scarecrow',
		action:'center'},
	scarecrow_left:{
		device:'guy_spark3',
		method:'scarecrow',
		action:'left'},
	scarecrow_right:{
		device:'guy_spark3',
		method:'scarecrow',
		action:'right'},
	scarecrow_up:{
		device:'guy_spark3',
		method:'scarecrow',
		action:'up'},
	scarecrow_down:{
		device:'guy_spark3',
		method:'scarecrow',
		action:'down'},
	scarecrow_level:{
		device:'guy_spark3',
		method:'scarecrow',
		action:'level'},
	scarecrow_grimace:{
		device:'guy_spark3',
		method:'scarecrow',
		action:'grimace'},
	scarecrow_teeth:{
		device:'guy_spark3',
		method:'scarecrow',
		action:'teeth'},
	scarecrow_smile:{
		device:'guy_spark3',
		method:'scarecrow',
		action:'smile'},
	scarecrow_chomp:{
		device:'guy_spark3',
		method:'scarecrow',
		action:'chomp'},
	scarecrow_wake:{
		device:'guy_spark3',
		method:'scarecrow',
		action:'wake'},
	scarecrow_sleep:{
		device:'guy_spark3',
		method:'scarecrow',
		action:'sleep'},
	scarecrow_eyes_crazy: {
		device:'guy_spark3',
		method:'scarecrow',
		action:'eyes_crazy'}, 
	scarecrow_eyes_blink: {
		device:'guy_spark3',
		method:'scarecrow',
		action:'eyes_blink'}, 	
	scarecrow_eyes_wink_left: {
		device:'guy_spark3',
		method:'scarecrow',
		action:'eyes_wink_left'}, 
	scarecrow_eyes_wink_right: {
		device:'guy_spark3',
		method:'scarecrow',
		action:'eyes_wink_right'}, 
	scarecrow_eyes_on: {
		device:'guy_spark3',
		method:'scarecrow',
		action:'eyes_on'}, 	
	scarecrow_eyes_off: {
		device:'guy_spark3',
		method:'scarecrow',
		action:'eyes_off'}, 
	scarecrow_sequence:{
		device:'guy_spark3',
		method:'scarecrowSeq',
		action:''} // this contains a comma seperated list of commands to complete in form "delay,cmd"
};

// The scarecrow sleep timeout variables. This puts the scarecrow to sleep after the timeout.
var scarecrowSleepTimeout=null;
var timeoutLength = 3 * 60 * 1000; // after 3 minutes sleep

// The motion wake timeout variables. This basically turns off motion activation for the timeout after a motion.
var motionWakeTimeout=null;
var motionWakeTimeoutLength = 20 * 1000; // 20 seconds 
var motionActive = true; //false
var motionActiveFilterDisable = true; // true if you don't want the motion to timeout after a motion happens.

// This is used to switch between setting off two props.
var toggleProp = true;

// This function will execute an action on a prop
function trigger (prop, action) {

	// lookup the device in the prop
    var device = props[prop].device; 
   	// lookup the deviceID
  	var deviceID = deviceIDs[device];
  	// lookup the method to call
  	var method = props[prop].method;
  	// lookup the action to take
  	// var action = props[prop].action;
  	
	//call the function and log results
    var fnPr = particle.callFunction({ deviceId: deviceID, name: method, argument: action, auth: token });
	fnPr.then(
		function(data) {
    		console.log('  Called {prop, method, action} = {', prop, ', ', method, ', ', action, '}'); 
    	}, 
    	function(err) {
		  	console.log('  Called Failed {prop, method, action} = {', prop, ', ', method, ', ', action, '}');
		}
	);
}

// this is the main function to dispatch an action
function doit(prop, action) {
	var myaction = action;
	if (myaction == null) {
		myaction = props[prop].action;
	}
	trigger(prop,myaction); 
}

// if nothting has happened for a while we want to call this
function scarecrowSleepCallback () {
	console.log('>Putting Scarecrow to sleep');
	doit('scarecrow_sleep');
	//doit('scarecrow_sleep');
	scarecrowSleepTimeout = null;
}

// This function turns motion activation back on
function motionWakeCallback () {
	console.log('>Motion Active');
	motionActive = true;
}

// This will put the scarecrow back to sleep after timeoutLength
function scarecrowSleepReset () {
	// clear the old timeout 
	if (scarecrowSleepTimeout != null)
		clearTimeout(scarecrowSleepTimeout);
	// set the time for the scarecrow to go to sleep
	scarecrowSleepTimeout = setTimeout(scarecrowSleepCallback, timeoutLength); 
}

function motionWakeReset () {
	//if we disabled the filter just turn motion active to on 
	if (motionActiveFilterDisable) {
		motionWakeCallback();
	} else {
		// clear the old timeout 
		console.log('>Motion Inactive');
		if (motionWakeTimeout != null) 
			clearTimeout(motionWakeTimeout);
		// set the time for the motion wakeup
		motionWakeTimeout = setTimeout(motionWakeCallback, motionWakeTimeoutLength);
		motionActive = false; 		
	}
}

// STEP 4) Write your motion callbacks

// This is the scarecrow motion callback. This gets invoked when the motion sensor on the scarecrow goes off.
function scarecrowCallback (data) {
	console.log('>Motion at Scarecrow');
	console.log('  event: ', data);
	if (motionActive) {
		// do a scarecrow sequence
		doit('scarecrow_sequence','5,eyes_on,5,no,5,left,500,teeth,3000,sleep'); 
		setTimeout(doit,10000,'scarecrow_sequence', '5,wake,5,chomp,500,sleep');

		//set off a random prop
		if (toggleProp) {
			setTimeout(doit,10000,'lamp');
		} else {
			setTimeout(doit,10000,'ball');
		}
		toggleProp = !toggleProp;

		// reset the callback for the scarecrow to go back to sleep
		scarecrowSleepReset();

		// reset the motion callback timeout
		motionWakeReset();
	}
}

// This is the grave grabber callback. When the motion sensor in the left of the yard goes off this is called.
function graveCallback (data) {
	console.log('>Motion at Gravegrabber');
	console.log('  event: ', data);

	if (motionActive) {

		doit('grave_grabber');
		doit('scarecrow_sequence','5,eyes_on,5,left');

		// reset the callback for the scarecrow to go back to sleep
		scarecrowSleepReset();

		// reset the motion callback timeout
		motionWakeReset();
	}
}

// This is the electical motion callback. 
// This goes off when the user sets off the motion sensor coming up the driveway.
// This then sets off several electrical props
function electricCallback (data) {
	console.log('>Motion at Electric');
	console.log('  event: ', data);
	if (motionActive) {
		// shock
		setImmediate(doit,'electric_wire_r');
		setImmediate(doit,'electric_wire_l');
		setImmediate(doit,'electric_box');
		// scarecrow react 
		doit('scarecrow_sequence','5,eyes_on,5,left,500,grimace');
		
		// reset the callback for the scarecrow to go back to sleep
		scarecrowSleepReset();

		// reset the motion callback timeout
		motionWakeReset();
	}
}

// This callback goes off when there is motion by the tree in the sideyard.
function treeCallback (data) {
	console.log('>Motion at Tree');
	console.log('  event: ', data);

	//I wanted this to happen regardless so I commented out the motionActive 
	//if (motionActive) {
		doit('tree');

		if (toggleProp) {
			setTimeout(doit,10000,'lamp');
		} else {
			setTimeout(doit,10000,'ball');
		}
		toggleProp = !toggleProp;
		//console.log("Event: " + data);
		//motionWakeReset();
	//}
}

// This goes off when there is motion by the urn prop in the right side of the yard.
function urnCallback (data) {
	console.log('>Motion at Urn');
	console.log('  event: ', data);

	if (motionActive) {
		// set off the urn and have the scarecrow look right 
		doit('urn'); 
		//scarecrow react
		doit('scarecrow_sequence','5,eyes_on,5,right');

		// reset the scarecrow timeout
		scarecrowSleepReset();

		// reset the motion timeout
		motionWakeReset();
	}
}

// This enumerates all the motion sensors to listen for and their callbacks.
// STEP 5) Enumerate your motion sensors
var motion = {
	scarecrow_motion:{
		device: props.scarecrow_yes.device,
		mesg: 'motion',
		callback: scarecrowCallback
	},
	grave_grabber_motion:{
		device: props.grave_grabber.device,
		mesg: 'motion',
		callback: graveCallback
	},
	electric_motion:{
		device: props.electric_box.device,
		mesg: 'motion',
		callback: electricCallback
	},
	tree_motion:{
		device: props.tree.device,
		mesg: 'motion',
		callback: treeCallback
	},
	urn_motion:{
		device: props.urn.device,
		mesg: 'motion',
		callback: urnCallback
	}
};

// This function configures the initial state and installs motion callbacks.
function setup () {
	/*
	// get the devices just to see if things are working
	var devicesPr = particle.listDevices({ auth: token });
	devicesPr.then(
	  function(devices){
	    console.log('Devices: ', devices);
	  },
	  function(err) {
	    console.log('List devices call failed: ', err);
	  }
	);
	*/

	// set the time for the scarecrow to go to sleep
	motionWakeReset();
	scarecrowSleepReset();

	// setup for capturing motion events
	for (let x in motion) {
		//setup event subscription
		//The let is very important here as it assures the function parameters are set correctly.
		let dev = motion[x].device;
		let devID = deviceIDs[dev];
		let name =  motion[x].mesg;
		let callback = motion[x].callback;

		// note the let on variables was important to make this work right.
		particle.getEventStream({ deviceId: dev, name: name, auth: token }).then(
			(stream) => {
  				stream.on('event', callback);
  				console.log('Adding Callback: device=', dev, ' name=', name, 'devID=', devID, ' callback=', callback);
			}
		);
	}

	console.log("READY");
}

//Configure and run
setup();







	


