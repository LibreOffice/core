/* -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#import <apple_remote/RemoteMainController.h>
#import "AppleRemote.h"
#import "KeyspanFrontRowControl.h"
#import "GlobalKeyboardDevice.h"
#import "RemoteControlContainer.h"
#import "MultiClickRemoteBehavior.h"


// Sample Code 3: Multi Click Behavior and Hold Event Simulation


@implementation AppleRemoteMainController

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

        if ( [container instantiateAndAddRemoteControlDeviceWithClass: [AppleRemote class]] != 0 ) {
#if OSL_DEBUG_LEVEL >= 2
            NSLog(@"[container instantiateAndAddRemoteControlDeviceWithClass: [AppleRemote class]] successful");
        }
        else {
            NSLog(@"[container instantiateAndAddRemoteControlDeviceWithClass: [AppleRemote class]] failed");
#endif
        }

        if ( [container instantiateAndAddRemoteControlDeviceWithClass: [GlobalKeyboardDevice class]] != 0 ) {
#if OSL_DEBUG_LEVEL >= 2
            NSLog(@"[container instantiateAndAddRemoteControlDeviceWithClass: [GlobalKeyboardDevice class]] successful");
        }
        else {
            NSLog(@"[container instantiateAndAddRemoteControlDeviceWithClass: [GlobalKeyboardDevice class]] failed");
#endif
        }
        // to give the binding mechanism a chance to see the change of the attribute
        [self setValue: container forKey: @"remoteControl"];
#if OSL_DEBUG_LEVEL >= 2
            NSLog(@"AppleRemoteMainController init done");
#endif
    }
    else
        NSLog(@"AppleRemoteMainController init failed");
    return self;
}

- (void) postTheEvent: (short int)buttonIdentifier modifierFlags:(int)modifierFlags
{
SAL_WNODEPRECATED_DECLARATIONS_PUSH
        // 'NSApplicationDefined' is deprecated: first deprecated in macOS 10.12
    [NSApp postEvent:
    [NSEvent    otherEventWithType:NSApplicationDefined
                location:NSZeroPoint
                modifierFlags:modifierFlags
                timestamp: 0
                windowNumber:[[NSApp keyWindow] windowNumber]
                context:nil
                subtype:AppleRemoteControlEvent
                data1: buttonIdentifier
                data2: 0]
    atStart: NO];
SAL_WNODEPRECATED_DECLARATIONS_POP
}


- (void) remoteButton: (RemoteControlEventIdentifier)buttonIdentifier pressedDown: (BOOL) pressedDown clickCount: (unsigned int)clickCount
{
    (void)clickCount;
#if OSL_DEBUG_LEVEL >= 2
    NSString* pressed = @"";
    NSString* buttonName = nil;
#endif
    if (pressedDown)
    {
#if OSL_DEBUG_LEVEL >= 2
        pressed = @"(AppleRemoteMainController: button pressed)";

        switch(buttonIdentifier)
        {
            case kRemoteButtonPlus:         buttonName = @"Volume up";              break;  // MEDIA_COMMAND_VOLUME_UP  ( see include/vcl/commandevent.hxx )
            case kRemoteButtonMinus:        buttonName = @"Volume down";            break;  // MEDIA_COMMAND_VOLUME_DOWN
            case kRemoteButtonMenu:         buttonName = @"Menu";                   break;  // MEDIA_COMMAND_MENU
            case kRemoteButtonPlay:         buttonName = @"Play";                   break;  // MEDIA_COMMAND_PLAY
            case kRemoteButtonRight:        buttonName = @"Next slide";             break;  // MEDIA_COMMAND_NEXTTRACK
            case kRemoteButtonLeft:         buttonName = @"Left";                   break;  // MEDIA_COMMAND_PREVIOUSTRACK
            case kRemoteButtonRight_Hold:   buttonName = @"Last slide";             break;  // MEDIA_COMMAND_NEXTTRACK_HOLD
            case kRemoteButtonLeft_Hold:    buttonName = @"First slide";            break;  // MEDIA_COMMAND_PREVIOUSTRACK_HOLD
            case kRemoteButtonPlus_Hold:    buttonName = @"Volume up holding";      break;
            case kRemoteButtonMinus_Hold:   buttonName = @"Volume down holding";    break;
            case kRemoteButtonPlay_Hold:    buttonName = @"Play (sleep mode)";      break;  // MEDIA_COMMAND_PLAY_HOLD
            case kRemoteButtonMenu_Hold:    buttonName = @"Menu (long)";            break;  // MEDIA_COMMAND_MENU_HOLD
            case kRemoteControl_Switched:   buttonName = @"Remote Control Switched";break;

            default:    NSLog( @"AppleRemoteMainController: Unmapped event for button %d", buttonIdentifier);   break;
        }
#endif
        [ self postTheEvent:buttonIdentifier modifierFlags: 0 ];
    }
    else // not pressed
    {
#if OSL_DEBUG_LEVEL >= 2
        pressed = @"(AppleRemoteMainController: button released)";
#endif
    }

#if OSL_DEBUG_LEVEL >= 2
	//NSLog(@"Button %@ pressed %@", buttonName, pressed);
	NSString* clickCountString = @"";
	if (clickCount > 1) clickCountString = [NSString stringWithFormat: @"%d clicks", clickCount];
	NSString* feedbackString = [NSString stringWithFormat:@"(Value:%4d) %@  %@ %@", buttonIdentifier, buttonName, pressed, clickCountString];

	// print out events
	NSLog(@"%@", feedbackString);

    if (pressedDown == NO) printf("\n");
	// simulate slow processing of events
	// [NSThread sleepUntilDate: [NSDate dateWithTimeIntervalSinceNow: 0.5]];
#endif
}

- (void) dealloc {
    [ remoteControl release ]; remoteControl = nil;
    [ remoteControlBehavior release ]; remoteControlBehavior = nil;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
