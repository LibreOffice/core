// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "Server.h"

@interface Server()

@end



@implementation Server

@synthesize protocol = _protocol;
@synthesize serverName = _serverName;
@synthesize serverAddress = _serverAddress;


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
