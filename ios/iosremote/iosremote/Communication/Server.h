// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import <Foundation/Foundation.h>

typedef enum protocol {NETWORK} Protocol_t;

@interface Server : NSObject <NSCoding>

@property (nonatomic) Protocol_t protocol;
@property (nonatomic, strong) NSString* serverName;
@property (nonatomic, strong) NSString* serverAddress;
@property (nonatomic, strong) NSString* serverVersion;

- (id)initWithProtocol:(Protocol_t)protocal
             atAddress:(NSString*) address
                ofName:(NSString*) name
             ofVersion:(NSString*) version;
@end

@interface NSString (IPValidation)

- (BOOL)isValidIPAddress;

@end