// -*- Mode: objc; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*-
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#import "ios.h"
#import <Foundation/NSObject.h>

#import <WebKit/WKURLSchemeTask.h>
#import <WebKit/WKURLSchemeHandler.h>

#import "CODocument.h"

#import <wsd/DocumentBroker.hpp>

/**
 * Used to send messages to the webview from the native parts of the app
 *
 * This was previously done with evaluateJavaScript on @"window.TheFakeWebSocket.onmessage([...someData...])" but that was very slow
 *
 * This code connects to MobileSocket (a class based on ProxySocket which is used to workaround restrictive firewalls that do not
 * allow websocket use). MobileSocket is also used on Android by the similarly-named MobileSocket Java class
 *
 * Return messages are still sent via calling native methods from JS rather than by sending through the MobileSocket. That's because in
 * Android it is not possible to read the body of an intercepted request, which prevents you from implementing the other side of the
 * ProxySocket protocol. Though there is no such technical restriction on iOS, it was deemed to be more helpful to keep the sending
 * mechanisms the same between the mobile apps. Similarly, long-running HTTP requests which continually receive partial data were ruled
 * out for similar reasons.
 */
@interface MobileSocket : NSObject
- (void)open:(id<WKURLSchemeTask>)urlSchemeTask;
- (void)write:(id<WKURLSchemeTask>)urlSchemeTask onFinish:(void (^)(void))callback;

- (void)queueSend:(std::string)message then:(void (^)(void))callback;

- (void)stopURLSchemeTask:(id<WKURLSchemeTask>)urlSchemeTask;
@end

// vim:set shiftwidth=4 softtabstop=4 expandtab:
