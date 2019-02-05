/* -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*****************************************************************************
 * HIDRemoteControlDevice.h
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

#import <apple_remote/RemoteControl.h>

#import <IOKit/hid/IOHIDLib.h>

/*
    Base class for HID based remote control devices
 */
@interface HIDRemoteControlDevice : RemoteControl {
    IOHIDDeviceInterface** hidDeviceInterface; // see IOKit/hid/IOHIDLib.h
    IOHIDQueueInterface**  queue;  // IOKit/hid/IOHIDLib.h
    NSMutableArray*        allCookies;
    NSMutableDictionary*   cookieToButtonMapping;
    CFRunLoopSourceRef     eventSource;

    BOOL fixSecureEventInputBug;
    BOOL openInExclusiveMode;
    BOOL processesBacklog;

    int supportedButtonEvents;
}

// When your application needs too much time on the main thread when processing an event other events
// may already be received which are put on a backlog. As soon as your main thread
// has some spare time this backlog is processed and may flood your delegate with calls.
// Backlog processing is turned off by default.
- (BOOL) processesBacklog;
- (void) setProcessesBacklog: (BOOL) value;

// methods that should be overwritten by subclasses
- (void) setCookieMappingInDictionary: (NSMutableDictionary*) cookieToButtonMapping;

- (void) sendRemoteButtonEvent: (RemoteControlEventIdentifier) event pressedDown: (BOOL) pressedDown;

+ (BOOL) isRemoteAvailable;

@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
