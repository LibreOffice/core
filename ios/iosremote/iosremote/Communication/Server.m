//
//  Server.m
//  sdremote
//
//  Created by Liu Siqi on 6/3/13.
//  Copyright (c) 2013 libreoffice. All rights reserved.
//

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
