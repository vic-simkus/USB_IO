/*
 
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <https://unlicense.org>

Original author: Vidas Simkus (vic.simkus@simkus.com) 2022-04-17
*/

// Brute force way to reboot
void(* resetFunc) (void) = 0;

// Bit masks of the individual outputs in the output state
const unsigned char M_P1 = 0b00000001;
const unsigned char M_P2 = 0b00000010;
const unsigned char M_P3 = 0b00000100;
const unsigned char M_P4 = 0b00001000;
const unsigned char M_P5 = 0b00010000;
const unsigned char M_P6 = 0b00100000;

// When all of the outputs are on, what is the maximum output value.
// All masks above must be ORed together
const unsigned int OUTPUT_MAX = (M_P1 | M_P2 | M_P3 | M_P4 | M_P5 | M_P6);

// Pins of the individual outputs
const unsigned char RP1 = 2;
const unsigned char RP2 = 3;
const unsigned char RP3 = 4;
const unsigned char RP4 = 5;
const unsigned char RP5 = 6;
const unsigned char RP6 = 7;

// The "event loop" interval.
// The main event loops is freewheeling.  We use the interval predominantly for demo mode
// One/8 of a second ..
#define CLIKC_INTERVAL_US 125000

// Global output state
unsigned char outputState = 0;

// Shorthand typedef
typedef unsigned long timer_t;

// We use timerA and timerB for detecting timer wraparounds.
// Do we need to? I dunno.
timer_t timerA = 0;
timer_t timerB = 0;

// We only do stuff every so often
// This is the timestamp of the last click
timer_t lastClickTimerUs = 0;

// Main state enum.  The application can be in the following states
typedef enum
{
	// Application state at startup
	FIRST_START = 0,

	// The system has started receiving input, but has not yet received an \n or \r
	IN_COMMAND,

	// The system had been receiving input and has now received \n or \r.
	END_COMMAND,

	// No state.  The application has executed at least one conmmand after startup and is just here
	IM_HERE,

	// Demo mode.  Makes the ligths do pretty things :) :)
	DEMO
	
} EnMainState;

// Main application state
EnMainState stateMain = EnMainState::FIRST_START;

// Data being seent to us via serial connection
String commandBuffer;

// This is invoked by loop() every CLICK_INTERVAL_US
void doDemoMode()
{
	outputState += 1;
	
	updateOutput();
	
	if(outputState == OUTPUT_MAX)
	{
		outputState = 0;
	}
	
	return;
}

// Parses the incomming command
void doEndCommand()
{
	commandBuffer.trim();
	
	// Process command here...
	if(commandBuffer.length() < 1)
	{
		doError("Zero length command");
		switchState(IM_HERE);
	}
	else
	{
		printInfo("Command as we understand: [" + commandBuffer + "]");				

		// XXX this is an overflow waiting to happen.  Will lead to a remote code excution vulnerability on the board 
		int partVect[8];
		int spaces=0, i=-1;
		
		while( (i = commandBuffer.indexOf(' ',i+1)) > -1)
		{
			partVect[spaces]=i;
			spaces += 1;
		}				

		partVect[spaces] = commandBuffer.length();

		// Number of parts is always? more than spaces
		int parts = spaces + 1;

		/*
		printInfo("Spaces found: " + String(spaces));
		printInfo("Partsfound: " + String(parts));
		
		for(i=0;i<parts;i++)
		{
			String s = commandBuffer.substring(j,partVect[i]);	
			//printInfo(String(i) + " @" + String(partVect[i]) + " [" + s + "]");

			// Skip the space at the actual mark
			j = partVect[i] + 1;
		}*/


		// Command buffer has somehting in it			
		if(commandBuffer[0] == 'h' || commandBuffer[0] == 'v' || commandBuffer[0] == '?')
		{
			dumpHelp();
			switchState(IM_HERE);			
		}		
		else if(commandBuffer[0] == 'r')
		{
			printInfo("We are rebooting");
			resetFunc();
		}
		else if(commandBuffer[0] == 's')
		{
			if(parts < 2)
			{
				doError("Insuficient parameters for command 's'");
			}
			else
			{
				int x = commandBuffer.substring(partVect[0] + 1,partVect[1]).toInt();
				printInfo("Parameter as we understand it: [" + String(x,DEC) + " (0x" + String(x,HEX) + ")]");
				outputState = (unsigned char)x;
				updateOutput();
			}			

			switchState(IM_HERE);
		}
		else if(commandBuffer[0] == 'd')
		{
			printError("Demo mode is not yet implemented");
			switchState(DEMO);
		}
		else
		{
			doError("Unrecognized command: [" + commandBuffer + "].  Will print help.");						
			switchState(IM_HERE);
		}		
	} 		
}

// Invoked by the Arduino API  repeatedly.
void loop() 
{
	timerB = timerA;
	timerA = micros();

	if( timerB == 0)
	{
		// First invocation.  Need to counters populated to hava delta		
		return;
	}

	// We always expect timerA to be higher than timerB because micros() is monotonic ... until it wraps
	// If that is not the case that means the timer wrapped

	if( timerB > timerA)
	{
		// Lets not overthink this ...
		timerA = timerA + (timerB - timerA);
	}

	// At this point timerA contains the most recentish timestamp

	if(stateMain == FIRST_START)
	{
		// It's the first start ... 
		lastClickTimerUs = timerA;
		switchState(IM_HERE);
	}

	bool shouldClick = false;

	if( (timerA - lastClickTimerUs) >= CLIKC_INTERVAL_US)
	{
		lastClickTimerUs = timerA;
		digitalWrite(LED_BUILTIN,!digitalRead(LED_BUILTIN));			
		shouldClick = true;
	}
	
	
	// Command processing first ...
	if(stateMain == EnMainState::END_COMMAND)
	{			
		doEndCommand();
	}// stateMain -== END_COMMAND


	if(shouldClick)
	{
		// The only time we care about this is in demo mode ...
		if(stateMain == EnMainState::DEMO)
		{
			doDemoMode();
		}
	}	
}

// Switches to new application state
// We do this in a separate method so that we don't have
// to do repeat mode switching housekeeping throughout the code.
void switchState(const EnMainState _newState)
{
	// check old state
	if(stateMain == EnMainState::END_COMMAND)
	{
		// reset command buffer
		commandBuffer = "";	
	}
	
	stateMain = _newState;
	
	return;
}

// This gets invoked between loop() invocations
void serialEvent()
{
	while( Serial.available() > 0)
	{
		int inByte = Serial.read();

		if(inByte < 0)
		{
			// The read failed.  Do we care? If there's no more data the loop should terminate above
			printError("Failed to read serial?");
			continue;
		}

		if(stateMain == EnMainState::END_COMMAND)
		{
			// We have a command in the buffer that hasn't been processed yet.
			// We continue to sink characters until the previous command is processed
			printError("Invalid program state - previous command has not been processed.");
			continue;
		}

		if(inByte == '\n' || inByte == '\r')
		{
			switchState(EnMainState::END_COMMAND);
			continue;
		}

		// At this point all that's left is a character in state of either FIRST_STATE,IN_COMMAND, or IM_HERE
		commandBuffer += (char)inByte;		
	} // Serial.available	
}

void setup() 
{
	Serial.begin(115200);
	
	Serial.print("\r\n");	

	printInfo("Serial init complete.");

	pinMode(RP1, OUTPUT);
	pinMode(RP2, OUTPUT);
	pinMode(RP3, OUTPUT);
	pinMode(RP4, OUTPUT);
	pinMode(RP5, OUTPUT);
	pinMode(RP6, OUTPUT);	
	pinMode(LED_BUILTIN,OUTPUT);

	digitalWrite(RP1, LOW); 
	digitalWrite(RP2, LOW); 
	digitalWrite(RP3, LOW); 
	digitalWrite(RP4, LOW); 
	digitalWrite(RP5, LOW); 
	digitalWrite(RP6, LOW); 
	
	digitalWrite(LED_BUILTIN,HIGH);

	printInfo("Maximum X: " + String(OUTPUT_MAX,DEC) + "(0x" + String(OUTPUT_MAX,HEX) + ")");
	printError("Feed me a kitten!");
	
	return;
}

void updateOutput()
{
	if(outputState > OUTPUT_MAX)
	{
		printError("Output state clipped to 0x" + String(OUTPUT_MAX,HEX) );
		outputState = OUTPUT_MAX;
	}
	
	digitalWrite(RP1, outputState & M_P1); 
	digitalWrite(RP2, outputState& M_P2); 
	digitalWrite(RP3, outputState& M_P3); 
	digitalWrite(RP4, outputState& M_P4); 
	digitalWrite(RP5, outputState& M_P5); 
	digitalWrite(RP6, outputState& M_P6); 

	printInfo("State: " + String(outputState,DEC) + " (0x" + String(outputState,HEX) + ")");
	
	return;
}

void printInfo(const String& _msg)
{
	Serial.print("ii ");
	Serial.println(_msg);
	Serial.flush();					
	
	return;
}

void doError(const String& _msg)
{
	printError(_msg);
	dumpHelp();
	switchState(IM_HERE);
	
	return;
}

void printError(const String& _msg)
{
	Serial.print("!! ");
	Serial.println(_msg);
	Serial.flush();					

	return;
}

void dumpHelp()
{
	printInfo("--- Start of Help ---");
	printInfo("\tThis is YAUBOB v1.0");
	printInfo("\tAll command must be terminated by either an \\n or \\r.");
	printInfo("\t v|?|h - Help; this");
	printInfo("\t r - Reboot/reset");
	printInfo("\t d - Demo mode");
	printInfo("\t s X - Set outputs where X is the bitmap in decimal form. Ex. s 1 will turn on first output.  s 2 will turn on second.  s 3 will turn on first and second and so forth in that manner.");
	printInfo("--- End of Help ---");
	Serial.flush();					
	
	return;
}
