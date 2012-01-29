/*****************************************************************************
 * RemoteControlWrapper.h
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
 * THE SOFTWARE IS PROVIDED ‚ÄúAS IS‚Äù, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *****************************************************************************/

#ifndef DID_DEFINE_SOME_FUTURE_TYPES
#import <Foundation/NSObjCRuntime.h>
#if defined (NSFoundationVersionNumber10_5) &&  MAC_OS_X_VERSION_MAX_ALLOWED < 1050
@class CALayer;
@class NSViewController;
typedef int NSColorRenderingIntent;
#define DID_DEFINE_SOME_FUTURE_TYPES
#endif
#endif

#import <Cocoa/Cocoa.h>
#import "HIDRemoteControlDevice.h"

/*  Interacts with the Apple Remote Control HID device
    The class is not thread safe
*/
@interface AppleRemote : HIDRemoteControlDevice {
}

@end
