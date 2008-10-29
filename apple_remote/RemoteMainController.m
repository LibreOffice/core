/*****************************************************************************
 * RemoteMainController.m
 *
 * Created by Martin Kahr on 11.03.06 under a MIT-style license. 
 * Copyright (c) 2006 martinkahr.com. All rights reserved.
 *
 * Code modified and adapted to OpenOffice.org 
 * by Eric Bachard on 11.08.2008 under the same License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *****************************************************************************/

#import "RemoteMainController.h"
#import "AppleRemote.h"
#import "KeyspanFrontRowControl.h"
#import "GlobalKeyboardDevice.h"
#import "RemoteControlContainer.h"
#import "MultiClickRemoteBehavior.h"



// -------------------------------------------------------------------------------------------
// Sample Code 3: Multi Click Behavior and Hold Event Simulation
// -------------------------------------------------------------------------------------------

@implementation MainController

- (id) init {
    self = [super init];  // because we redefined our own init instead of use the fu..nny awakeFromNib 
    if (self != nil) {

        // 1. instantiate the desired behavior for the remote control device
        remoteControlBehavior = [[MultiClickRemoteBehavior alloc] init];	

        // 2. configure the behavior
        [remoteControlBehavior setDelegate: self];
		
        // 3. a Remote Control Container manages a number of devices and conforms to the RemoteControl interface
        //    Therefore you can enable or disable all the devices of the container with a single "startListening:" call.
        RemoteControlContainer* container = [[RemoteControlContainer alloc] initWithDelegate: remoteControlBehavior];

        if ( [container instantiateAndAddRemoteControlDeviceWithClass: [AppleRemote class]] != nil ) {
#ifdef DEBUG
            NSLog(@"[container instantiateAndAddRemoteControlDeviceWithClass: [AppleRemote class]] successfull");
        }
        else {
            NSLog(@"[container instantiateAndAddRemoteControlDeviceWithClass: [AppleRemote class]] failed");
#endif
        }

        if ( [container instantiateAndAddRemoteControlDeviceWithClass: [KeyspanFrontRowControl class]] != nil ) {
#ifdef DEBUG
            NSLog(@"[container instantiateAndAddRemoteControlDeviceWithClass: [KeyspanFrontRowControl class]] successfull");
        }
        else {
            NSLog(@"[container instantiateAndAddRemoteControlDeviceWithClass: [KeyspanFrontRowControl class]] failed");
#endif
        }

        if ( [container instantiateAndAddRemoteControlDeviceWithClass: [GlobalKeyboardDevice class]] != nil ) {
#ifdef DEBUG
            NSLog(@"[container instantiateAndAddRemoteControlDeviceWithClass: [GlobalKeyboardDevice class]] successfull");
        }
        else {
            NSLog(@"[container instantiateAndAddRemoteControlDeviceWithClass: [GlobalKeyboardDevice class]] failed");
#endif
        }	
        // to give the binding mechanism a chance to see the change of the attribute
        [self setValue: container forKey: @"remoteControl"];	
#ifdef DEBUG
            NSLog(@"MainController init done");
#endif
    }
    else 
        NSLog(@"MainController init failed");
    return self;
}

// delegate method for the MultiClickRemoteBehavior

// Hack: we define here our events, and post them directly to the NSApp, who is there to receive the notifications.
// NSEvent class gives several possibilities, and the one above works out of the box :
//+ keyEventWithType:location:modifierFlags:timestamp:windowNumber:context:characters:charactersIgnoringModifiers:isARepeat:keyCode:  
- (void) postTheEvent: (unichar)theUnicharCode theKeyCode:(long unsigned int)theKeyCode modifierFlags:(int)modifierFlags isARepeat:(BOOL)toBeRepeated
{

    unichar theKey=theUnicharCode;
    NSString *characters=[NSString stringWithCharacters: &theKey length: 1];

    [NSApp postEvent: 

    [NSEvent keyEventWithType:NSKeyDown
    location: NSZeroPoint
    modifierFlags : modifierFlags
    timestamp: 0
    windowNumber: [[NSApp keyWindow] windowNumber]
    context: nil
    characters: characters
    charactersIgnoringModifiers: characters
    isARepeat: toBeRepeated
    keyCode: theKeyCode]
    atStart: NO];
}

- (void) remoteButton: (RemoteControlEventIdentifier)buttonIdentifier pressedDown: (BOOL) pressedDown clickCount: (unsigned int)clickCount
{
    NSString* buttonName = nil;
    NSString* pressed = @"";

    SystemUIMode currentMode; // Fullscreen is kUIModeAllHidden, other are "normal mode"
    SystemUIOptions currentModeOptions;
    GetSystemUIMode (&currentMode,&currentModeOptions);

    if (pressedDown)
    {
        pressed = @"(pressed)";
        // kUIModeAllHidden means fullscreen 
        // and pressed only allows F5 in normal mode (and fixes the bounce) 
        if ( kUIModeAllHidden == currentMode )
        {
            switch(buttonIdentifier) 
            {
                case kRemoteButtonPlus:
                {
                    // MEDIA_COMMAND_VOLUME_UP  ( see vcl/inc/vcl/cmdevt.hxx )
                    buttonName = @"Volume up";			
                    unichar volumeUpKey=NSUpArrowFunctionKey;//NSPageUpFunctionKey;//1025; //
                    [ self postTheEvent: volumeUpKey theKeyCode: 0xF72C modifierFlags: 0 isARepeat:NO ]; 
                }
                    break;

                case kRemoteButtonMinus:
                {
                    // MEDIA_COMMAND_VOLUME_DOWN
                    buttonName = @"Volume down";			
                    unichar volumeDownKey=NSDownArrowFunctionKey;
                    [ self postTheEvent: volumeDownKey theKeyCode: 1024 modifierFlags: 0 isARepeat:NO ];
                }
                    break;

                case kRemoteButtonMenu:
                {
                    // MEDIA_COMMAND_MENU
                    buttonName = @"Menu";
                    unichar menuKey=NSMenuFunctionKey; //
                    [ self postTheEvent: menuKey theKeyCode: 0xF735 modifierFlags: 0 isARepeat:NO ]; 

                }
                    break;

                case kRemoteButtonPlay:
                {
                    // MEDIA_COMMAND_PLAY
                    buttonName = @"Play";
                    unichar playFunction=NSF5FunctionKey; //
                    [ self postTheEvent: playFunction theKeyCode: 96 modifierFlags: 0 isARepeat:NO ]; 
                }
                    break;			

                case kRemoteButtonRight:
                {
                    // MEDIA_COMMAND_NEXTTRACK
                    buttonName = @"Next slide";
                    unichar rightArrow=NSRightArrowFunctionKey; //NSRightArrowKey == 124
                    [ self postTheEvent: rightArrow theKeyCode: 124 modifierFlags: 0 isARepeat:NO ]; 
                }
                    break;

                case kRemoteButtonLeft:
                {
                    // MEDIA_COMMAND_PREVIOUSTRACK
                    buttonName = @"Left";
                    unichar leftArrow=NSLeftArrowFunctionKey;
                    [ self postTheEvent: leftArrow theKeyCode: 123 modifierFlags: 0 isARepeat:NO ];
                }
                    break;

                case kRemoteButtonRight_Hold:
                {
                    // MEDIA_COMMAND_NEXTTRACK_HOLD
                    buttonName = @"Last slide";	
                    unichar lastSlide=NSEndFunctionKey; // Goes to last slide(value from in offuh/com/sun/star/awt/Key.hdl)
                    [ self postTheEvent: lastSlide theKeyCode: 1029 modifierFlags: 0 isARepeat:NO ]; 
                }
                    break;

                case kRemoteButtonLeft_Hold:
                {
                    // MEDIA_COMMAND_PREVIOUSTRACK_HOLD
                    buttonName = @"First slide";	
                    unichar firstSlide=NSHomeFunctionKey; // Goes to first slide (value from in offuh/com/sun/star/awt/Key.hdl)
                    [ self postTheEvent: firstSlide theKeyCode: 1028 modifierFlags: 0 isARepeat:NO ]; 
                }
            	    break;			

                case kRemoteButtonPlus_Hold:
                    buttonName = @"Volume up holding";	
                    break;				

                case kRemoteButtonMinus_Hold:			
                    buttonName = @"Volume down holding";	
                    break;				

                case kRemoteButtonPlay_Hold:
                    // MEDIA_COMMAND_PLAY_HOLD
                    buttonName = @"Play (sleep mode)";
                    break;			

                case kRemoteButtonMenu_Hold:
                {
                    // MEDIA_COMMAND_MENU_HOLD
                    buttonName = @"Menu (long)";
                    unichar escapeKey=27;
                    [ self postTheEvent: escapeKey theKeyCode: 27 modifierFlags: 0 isARepeat:NO ]; 
                }
                    break;

                case kRemoteControl_Switched:
                    buttonName = @"Remote Control Switched";
                    break;

                default:
#ifdef DEBUG
                    NSLog(@"Unmapped event for button %d", buttonIdentifier); 
#endif
                    break;
            }
        }
        else // normal mode
        {
        
            switch(buttonIdentifier) 
            {
                case kRemoteButtonPlay:
                {
                    // MEDIA_COMMAND_PLAY
                    buttonName = @"Play";
                    unichar playFunction=NSF5FunctionKey; //
                    [ self postTheEvent: playFunction theKeyCode: 772 modifierFlags: 0 isARepeat:NO ]; 
                }
                    break;
		
                case kRemoteButtonMenu_Hold:
                {
                    // MEDIA_COMMAND_MENU_HOLD
                    buttonName = @"Menu (long)";
                    unichar escapeKey=27;
                    [ self postTheEvent: escapeKey theKeyCode: 27 modifierFlags: 0 isARepeat:NO ]; 
                }
                    break;		

                case kRemoteControl_Switched:
                    buttonName = @"Remote Control Switched";
                    break;

                default:
#ifdef DEBUG
                    NSLog(@"Unmapped event for button %d", buttonIdentifier); 
#endif
                    break;       

#ifdef DEBUG
                NSLog(@"Normal mode ");
#endif
            }
        }
    }
    else // not pressed
    {
        pressed = @"(released)";
    }

#ifdef DEBUG
	//NSLog(@"Button %@ pressed %@", buttonName, pressed);
	NSString* clickCountString = @"";
	if (clickCount > 1) clickCountString = [NSString stringWithFormat: @"%d clicks", clickCount];
	NSString* feedbackString = [NSString stringWithFormat:@"(Value:%4d) %@  %@ %@",buttonIdentifier, buttonName, pressed, clickCountString];

	// print out events
	NSLog(@"%@", feedbackString);

    if (pressedDown == NO) printf("\n");
	// simulate slow processing of events
	// [NSThread sleepUntilDate: [NSDate dateWithTimeIntervalSinceNow: 0.5]];
#endif
}

- (void) dealloc {
    [remoteControl autorelease];
	[remoteControlBehavior autorelease];
	[super dealloc];
}

// for bindings access
- (RemoteControl*) remoteControl {
	return remoteControl;
}

- (MultiClickRemoteBehavior*) remoteBehavior {
	return remoteControlBehavior;
}

@end