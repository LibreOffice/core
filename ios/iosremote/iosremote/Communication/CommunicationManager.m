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
#import <dispatch/dispatch.h>

@interface CommunicationManager()

@property (nonatomic, strong) Client* client;
@property (nonatomic, strong) CommandInterpreter* interpreter;
@property (nonatomic, strong) CommandTransmitter* transmitter;
@property (atomic, strong) NSMutableArray* servers;

@end

// Singlton Pattern
@implementation CommunicationManager

@synthesize client = _client;
@synthesize state = _state;
@synthesize interpreter = _interpreter;
@synthesize transmitter = _transmitter;
@synthesize servers = _servers;

NSLock *connectionLock;

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
    connectionLock = [NSLock new];
    backgroundQueue = dispatch_queue_create("org.libreoffice.iosremote", NULL);
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

- (void) connectToServer:(Server*)server
{
    dispatch_async(backgroundQueue, ^(void) {
        if ([connectionLock tryLock]) {
            self.state = CONNECTING;
            [self.client disconnect];
            // initialise it with a given server
            self.client = [[Client alloc]initWithServer:server managedBy:self interpretedBy:self.interpreter];
            if([self.client connect]){
                self.state = CONNECTED;
                self.transmitter = [[CommandTransmitter alloc] initWithClient:self.client];
            }
            else{
                // streams closing is handled by client itself in case of connection failure
                self.state = DISCONNECTED;
            }
            [connectionLock unlock];
        }
        else
            // Already a threading working on that ... and that thread will unlock in 5 seconds anyway, so just return for now. 
            return;
    });
}

- (NSNumber *) getPairingPin{
    return [self.client pin];
}

- (NSString *) getPairingDeviceName
{
    return [self.client name];
}

+ (id)allocWithZone:(NSZone *)zone
{
    return [self sharedComManager];
}





@end
