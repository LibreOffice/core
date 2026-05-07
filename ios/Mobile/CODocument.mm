// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*-
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#import "config.h"

#import <algorithm>

// This is not "external" code in the UNO-based extensions sense. To be able to include
// <comphelper/lok.hxx>, we must #define LIBO_INTERNAL_ONLY.

#define LIBO_INTERNAL_ONLY
#include <sal/config.h>
#include <sal/log.hxx>
#include <rtl/ustring.hxx>
#include <comphelper/kit.hxx>
#include <i18nlangtag/languagetag.hxx>

#import "ios.h"
#import "AppDelegate.h"
#import "CODocument.h"
#import "DocumentViewController.h"

#import "ClientSession.hpp"
#import "DocumentBroker.hpp"
#import "FakeSocket.hpp"
#import "Kit.hpp"
#import "KitHelper.hpp"
#import "Log.hpp"
#import "COOLWSD.hpp"
#import "MobileApp.hpp"
#import "Protocol.hpp"

#import "CoolURLSchemeHandler.h"

static inline bool isMessageOfType(const char *message, const char *type, size_t lengthOfMessage) {
    // Note: message is not zero terminated but type is
    size_t typeLen = strlen(type);
    return (typeLen <= lengthOfMessage && !strncmp(message, type, typeLen));
}

@implementation CODocument

- (id)contentsForType:(NSString*)typeName error:(NSError **)errorPtr {
    return [NSData dataWithContentsOfFile:[copyFileURL path] options:0 error:errorPtr];
}

// We keep a running count of opening documents here. This is not necessarily in sync with the
// DocBrokerId in DocumentBroker due to potential parallelism when opening multiple documents in
// quick succession.

static std::atomic<unsigned> appDocIdCounter(1);

- (BOOL)loadFromContents:(id)contents ofType:(NSString *)typeName error:(NSError **)errorPtr {

    // If this method is called a second time on the same CODocument object, just ignore it. This
    // seems to happen occasionally when the device is awakened after sleep. See tdf#122543.
    if (fakeClientFd >= 0)
        return YES;

    fakeClientFd = fakeSocketSocket();

    appDocId = appDocIdCounter++;
    NSURL *copyFileDirectory = [[NSFileManager.defaultManager temporaryDirectory] URLByAppendingPathComponent:[NSString stringWithFormat:@"%d", appDocId]];
    if (![NSFileManager.defaultManager createDirectoryAtURL:copyFileDirectory withIntermediateDirectories:YES attributes:nil error:nil]) {
        LOG_ERR("Could not create directory " << [[copyFileDirectory path] UTF8String]);
        return NO;
    }

    copyFileURL = [copyFileDirectory URLByAppendingPathComponent:[[[self fileURL] path] lastPathComponent]];

    NSError *error;
    [[NSFileManager defaultManager] removeItemAtURL:copyFileURL error:nil];
    [[NSFileManager defaultManager] copyItemAtURL:[self fileURL] toURL:copyFileURL error:&error];
    if (error != nil)
        return NO;

    NSURL *url = [[NSBundle mainBundle] URLForResource:@"cool" withExtension:@"html"];
    NSURLComponents *components = [NSURLComponents componentsWithURL:url resolvingAgainstBaseURL:NO];
    DocumentData::allocate(appDocId).coDocument = self;
    components.queryItems = @[ [NSURLQueryItem queryItemWithName:@"file_path" value:[copyFileURL absoluteString]],
                               [NSURLQueryItem queryItemWithName:@"closebutton" value:@"1"],
                               [NSURLQueryItem queryItemWithName:@"permission" value:(readOnly ? @"readonly" : @"edit")],
                               [NSURLQueryItem queryItemWithName:@"lang" value:app_locale],
                               [NSURLQueryItem queryItemWithName:@"appdocid" value:[NSString stringWithFormat:@"%u", appDocId]],
                               [NSURLQueryItem queryItemWithName:@"userinterfacemode" value:([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPad ? @"notebookbar" : @"classic")],
                               // Related to issue #5841: the iOS app sets the
                               // base text direction via the "dir" parameter
                               [NSURLQueryItem queryItemWithName:@"dir" value:app_text_direction],
                             ];

    NSURLRequest *request = [[NSURLRequest alloc]initWithURL:components.URL];
    [self.viewController.webView loadRequest:request];

    return YES;
}

- (void)send2JS:(const char *)buffer length:(int)length {
    LOG_DBG("To JS: " << COOLProtocol::getAbbreviatedMessage(buffer, length).c_str());

    std::string message = std::string(buffer, length);    
    [(__bridge CoolURLSchemeHandler*)self.viewController.schemeHandler queueSend:message then:^() {
        dispatch_async(dispatch_get_main_queue(), ^{
            [self.viewController.webView evaluateJavaScript:@"window.socket.doSend()"
                                         completionHandler:^(id _Nullable obj, NSError * _Nullable error)
                 {
                     if (error) {
                         LOG_ERR("Error while notifying MobileSocket of waiting message");
                         NSString *jsException = error.userInfo[@"WKJavaScriptExceptionMessage"];
                         if (jsException != nil)
                             LOG_ERR("JavaScript exception: " << [jsException UTF8String]);
                     }
                 }
             ];
        });
    }];
}

@end

// vim:set shiftwidth=4 softtabstop=4 expandtab:
