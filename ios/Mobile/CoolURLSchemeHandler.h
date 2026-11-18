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
#import "MobileSocket.h"

#import <wsd/DocumentBroker.hpp>

/**
 * Used to capture HTTP requests from mobile so we can use web requests without hosting a server
 *
 * Analogous to Android's COWebViewClient
 */
@interface CoolURLSchemeHandler : NSObject<WKURLSchemeHandler>
{
    NSMutableSet<id<WKURLSchemeTask>> *ongoingTasks;
    NSMutableSet<id<WKURLSchemeTask>> *ongoingMobileSocketTasks;
    CODocument *document;
    MobileSocket *mobileSocket;
}

- (id)initWithDocument:(CODocument *)document;
- (std::shared_ptr<DocumentBroker>)getDocumentBroker;
- (std::optional<std::tuple<NSUInteger, NSUInteger, NSUInteger>>)getPositionsAndSizeForRange:(NSString *)range withTotalSize:(NSInteger)size;

- (void)handleMediaTask:(id<WKURLSchemeTask>)urlSchemeTask;
- (void)handleMobileSocketTask:(id<WKURLSchemeTask>)urlSchemeTask;
- (void)queueSend:(std::string)message then:(void (^)())callback;
@end

// vim:set shiftwidth=4 softtabstop=4 expandtab:
