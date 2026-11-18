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

#import "config.h"

#import "MobileSocket.h"

#import "MobileApp.hpp"

#import <Foundation/NSSet.h>
#import <Foundation/NSURL.h>
#import <Foundation/NSURLResponse.h>

#import <Foundation/NSString.h>

#import <Poco/URI.h>

#import <wsd/DocumentBroker.hpp>

@interface MobileSocket()
{
    dispatch_queue_t queue;
    int serial;
    std::vector<std::string> sendingMessages;
    NSMutableSet<id<WKURLSchemeTask>> *ongoingTasks;
}

- (id)init;

- (void)send:(std::string)message to:(id<WKURLSchemeTask>)urlSchemeTask;
@end

@implementation MobileSocket
- (id)init {
    self->ongoingTasks = [[NSMutableSet alloc] init];
    self->queue = dispatch_queue_create("com.collabora.iOS.CoolURLSchemeHandler.proxySocketQueue", DISPATCH_QUEUE_SERIAL);
    return self;
}

- (void)send:(const std::string)message to:(id<WKURLSchemeTask>)urlSchemeTask {
    serial++;
    
    std::ostringstream header;
    
    bool binary = false;
    
    for (const char &c : message) {
        if (c == '\n') {
            binary = true;
            break;
        }
    }
    
    if (binary) {
        header << "B";
    } else {
        header << "T";
    }
    
    header << "0x";
    header << std::hex << serial;
    header << "\n";
    
    header << "0x";
    header << std::hex << message.size();
    header << "\n";

    [urlSchemeTask didReceiveData:[NSData dataWithBytes:header.str().data() length:header.str().length()]];
    [urlSchemeTask didReceiveData:[NSData dataWithBytes:message.data() length:message.length()]];
    [urlSchemeTask didReceiveData:[@"\n" dataUsingEncoding:NSUTF8StringEncoding]];
}

- (void)open:(id<WKURLSchemeTask>)urlSchemeTask {
    dispatch_async(queue, ^{
        [self->ongoingTasks addObject:urlSchemeTask];
        NSMutableDictionary<NSString*, NSString*> * responseHeaders = [[NSMutableDictionary alloc] init];
        [responseHeaders setObject:@"null" forKey:@"Access-Control-Allow-Origin"]; // Yes, the origin really is 'null' for 'file:' origins
        
        self->serial = 0;
        NSData * identifier = [@"mobile" dataUsingEncoding:NSUTF8StringEncoding]; // This is entirely arbitrary - "mobile" is nice but for functionality it may as well be "xnopyt", "pomni" or any other random string without colons
        [responseHeaders setObject:[NSString stringWithFormat:@"%lu", identifier.length] forKey:@"Content-Length"];
        
        NSHTTPURLResponse * response = [[NSHTTPURLResponse alloc]
                                        initWithURL:urlSchemeTask.request.URL
                                        statusCode:200
                                        HTTPVersion:nil
                                        headerFields:responseHeaders
        ];
        [urlSchemeTask didReceiveResponse:response];
        [urlSchemeTask didReceiveData:identifier];
        
        [self->ongoingTasks removeObject:urlSchemeTask];
        [urlSchemeTask didFinish];
    });
}

- (void)write:(id<WKURLSchemeTask>)urlSchemeTask onFinish:(void (^)(void))callback {
    dispatch_async(queue, ^{
        [self->ongoingTasks addObject:urlSchemeTask];
        NSMutableDictionary<NSString*, NSString*> * responseHeaders = [[NSMutableDictionary alloc] init];
        [responseHeaders setObject:@"null" forKey:@"Access-Control-Allow-Origin"]; // Yes, the origin really is 'null' for 'file:' origins
        
        NSHTTPURLResponse * response = [[NSHTTPURLResponse alloc]
                                        initWithURL:urlSchemeTask.request.URL
                                        statusCode:200
                                        HTTPVersion:nil
                                        headerFields:responseHeaders
        ];
        [urlSchemeTask didReceiveResponse:response];

        for (const std::string &message : self->sendingMessages) {
            [self send:message to:urlSchemeTask];
            
            if (![self->ongoingTasks containsObject:urlSchemeTask]) {
                return; // Call no further methods - we're not going to worry too much about re-sending stuff here ... this abort is a fairly odd situation anyway
            }
        }
        
        self->sendingMessages.clear();
        
        // Tell the other side that we finished, and remove the ongoing-ness of the task
        [self->ongoingTasks removeObject:urlSchemeTask];
        [urlSchemeTask didFinish];
        
        dispatch_async(dispatch_get_main_queue(), callback);
    });
}

- (void)queueSend:(std::string)message then:(void (^)(void))callback {
    dispatch_async(queue, ^{
        self->sendingMessages.push_back(message);
        dispatch_async(dispatch_get_main_queue(), callback);
    });
}

- (void)stopURLSchemeTask:(id<WKURLSchemeTask>)urlSchemeTask {
    dispatch_async(queue, ^{
        // Yeet the task from the ongoingTasks
        [self->ongoingTasks removeObject:urlSchemeTask];
    });
}
@end
