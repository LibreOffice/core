/* -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*****************************************************************************
 * RemoteControlWrapper.m
 * RemoteControlWrapper
 *
 * Created by Martin Kahr on 11.03.06 under a MIT-style license.
 * Copyright (c) 2006 martinkahr.com. All rights reserved.
 *
 * Code modified and adapted to OpenOffice.org
 * by Eric Bachard on 11.08.2008 under the same license
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

#import "AppleRemote.h"

#import <mach/mach.h>
#import <mach/mach_error.h>
#import <IOKit/IOKitLib.h>
#import <IOKit/IOCFPlugIn.h>
#import <IOKit/hid/IOHIDKeys.h>

static const char* AppleRemoteDeviceName = "AppleIRController";

@implementation AppleRemote

+ (const char*) remoteControlDeviceName {
	return AppleRemoteDeviceName;
}

- (void) setCookieMappingInDictionary: (NSMutableDictionary*) _cookieToButtonMapping	{

    // TODO : avoid such magics
#if OSL_DEBUG_LEVEL >= 2
    NSLog( @"Apple Remote: setting 10.6 cookies" );
#endif
    // 10.6.x Snow Leopard
    [_cookieToButtonMapping setObject:[NSNumber numberWithInt:kRemoteButtonPlus]		forKey:@"33_31_30_21_20_2_"];
    [_cookieToButtonMapping setObject:[NSNumber numberWithInt:kRemoteButtonMinus]		forKey:@"33_32_30_21_20_2_"];
    [_cookieToButtonMapping setObject:[NSNumber numberWithInt:kRemoteButtonMenu]		forKey:@"33_22_21_20_2_33_22_21_20_2_"];
    [_cookieToButtonMapping setObject:[NSNumber numberWithInt:kRemoteButtonPlay]		forKey:@"33_23_21_20_2_33_23_21_20_2_"];
    [_cookieToButtonMapping setObject:[NSNumber numberWithInt:kRemoteButtonRight]		forKey:@"33_24_21_20_2_33_24_21_20_2_"];
    [_cookieToButtonMapping setObject:[NSNumber numberWithInt:kRemoteButtonLeft]		forKey:@"33_25_21_20_2_33_25_21_20_2_"];
    [_cookieToButtonMapping setObject:[NSNumber numberWithInt:kRemoteButtonRight_Hold]	forKey:@"33_21_20_14_12_2_"];
    [_cookieToButtonMapping setObject:[NSNumber numberWithInt:kRemoteButtonLeft_Hold]	forKey:@"33_21_20_13_12_2_"];
    [_cookieToButtonMapping setObject:[NSNumber numberWithInt:kRemoteButtonMenu_Hold]	forKey:@"33_21_20_2_33_21_20_2_"];
    [_cookieToButtonMapping setObject:[NSNumber numberWithInt:kRemoteButtonPlay_Hold]	forKey:@"37_33_21_20_2_37_33_21_20_2_"];
    [_cookieToButtonMapping setObject:[NSNumber numberWithInt:kRemoteControl_Switched]	forKey:@"19_"];
    [_cookieToButtonMapping setObject:[NSNumber numberWithInt:kMetallicRemote2009ButtonPlay]       forKey:@"33_21_20_8_2_33_21_20_8_2_"];
    [_cookieToButtonMapping setObject:[NSNumber numberWithInt:kMetallicRemote2009ButtonMiddlePlay] forKey:@"33_21_20_3_2_33_21_20_3_2_"];
    [_cookieToButtonMapping setObject:[NSNumber numberWithInt:kRemoteControl_Switched] forKey:@"19_"];
}

- (void) sendRemoteButtonEvent: (RemoteControlEventIdentifier) event pressedDown: (BOOL) pressedDown {
	if (pressedDown == NO && event == kRemoteButtonMenu_Hold) {
		// There is no separate event for pressed down on menu hold. We are simulating that event here
		[super sendRemoteButtonEvent:event pressedDown:YES];
	}

	[super sendRemoteButtonEvent:event pressedDown:pressedDown];

	if (pressedDown && (event == kRemoteButtonRight || event == kRemoteButtonLeft || event == kRemoteButtonPlay || event == kRemoteButtonMenu || event == kRemoteButtonPlay_Hold)) {
		// There is no separate event when the button is being released. We are simulating that event here
		[super sendRemoteButtonEvent:event pressedDown:NO];
	}
}

@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
