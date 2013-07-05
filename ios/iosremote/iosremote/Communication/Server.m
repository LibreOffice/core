// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "Server.h"
#import <arpa/inet.h>

@interface Server()

@end

@implementation Server


@synthesize protocol = _protocol;
@synthesize serverName = _serverName;
@synthesize serverAddress = _serverAddress;


- (void)encodeWithCoder:(NSCoder *)coder;
{
    [coder encodeObject:self.serverName forKey:@"name"];
    [coder encodeObject:self.serverAddress forKey:@"address"];
    [coder encodeInteger:self.protocol forKey:@"protocol"];
}

- (id)initWithCoder:(NSCoder *)coder;
{
    self = [self initWithProtocol:[coder decodeIntegerForKey:@"protocol"]
                        atAddress:[coder decodeObjectForKey:@"address"]
                           ofName:[coder decodeObjectForKey:@"name"]];
    return self;
}


- (id)initWithProtocol:(Protocol_t)protocal
           atAddress:(NSString*) address
              ofName:(NSString*) name
{
    self = [self init];
    self.protocol = protocal;
    self.serverAddress = address;
    self.serverName = name;
    return self;
}

- (NSString *)description{
    return [NSString stringWithFormat:@"Server: Name:%@ Addr:%@", self.serverName, self.serverAddress];
}

@end


@implementation NSString (IPValidation)

- (BOOL)isValidIPAddress
{
    const char *utf8 = [self UTF8String];
    int success;
    
    struct in_addr dst;
    success = inet_pton(AF_INET, utf8, &dst);
    if (success != 1) {
        struct in6_addr dst6;
        success = inet_pton(AF_INET6, utf8, &dst6);
    }
    
    return success;
}

@end
