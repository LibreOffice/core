// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


#import "CommunicationManager.h"
#import "Client.h"
#import "Server.h"
#import "CommandTransmitter.h"
#import "CommandInterpreter.h"

@interface CommunicationManager()

@property (nonatomic, strong) Client* client;
@property (nonatomic, strong) CommandInterpreter* interpreter;
@property (atomic, strong) NSMutableArray* servers;

@end

// Singlton Pattern
@implementation CommunicationManager

@synthesize client = _client;
@synthesize state = _state;
@synthesize interpreter = _interpreter;
@synthesize servers = _servers;

+ (CommunicationManager *)sharedComManager
{
    static CommunicationManager *sharedComManager = nil;
    
    static dispatch_once_t _singletonPredicate;
    
    dispatch_once(&_singletonPredicate, ^{
        sharedComManager = [[super allocWithZone:nil] init];
    });
    
    return sharedComManager;
}

- (id) init
{
    self = [super init];
    self.state = DISCONNECTED;
    return self;
}

- (id) initWithExistingServers
{
    self = [self init];

    NSUserDefaults * userDefaluts = [NSUserDefaults standardUserDefaults];
    
    if(!userDefaluts)
        NSLog(@"userDefaults nil");
    
    NSData *dataRepresentingExistingServers = [userDefaluts objectForKey:@"ExistingServers"];
    if (dataRepresentingExistingServers != nil)
    {
        NSArray *oldSavedArray = [NSKeyedUnarchiver unarchiveObjectWithData:dataRepresentingExistingServers];
        if (oldSavedArray != nil)
            self.servers = [[NSMutableArray alloc] initWithArray:oldSavedArray];
        else
            self.servers = [[NSMutableArray alloc] init];
    }
}

+ (id)allocWithZone:(NSZone *)zone
{
    return [self sharedComManager];
}





@end
