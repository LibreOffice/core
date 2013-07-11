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
#import "SlideShow.h"
#import "CommandTransmitter.h"
#import "CommandInterpreter.h"
#import "serverList_vc.h"
#import <dispatch/dispatch.h>

#define ExistingServersKey @"CommunicationManager.ExistingServers"

@interface CommunicationManager()

@property (nonatomic, strong) Client* client;

@end

// Singlton Pattern
@implementation CommunicationManager

@synthesize client = _client;
@synthesize state = _state;
@synthesize interpreter = _interpreter;
@synthesize transmitter = _transmitter;
@synthesize servers = _servers;
@synthesize delegate = _delegate;

+ (CommunicationManager *)sharedComManager
{
    static CommunicationManager *sharedComManager = nil;
    static dispatch_once_t _singletonPredicate;
    
    dispatch_once(&_singletonPredicate, ^{
        sharedComManager = [[super allocWithZone:nil] initWithExistingServers];
    });
    
    return sharedComManager;
}


- (void) connectionStatusHandler:(NSNotification *)note
{
    if([[note name] isEqualToString:@"connection.status.connected"]){
        if (self.state!=CONNECTED){
            NSLog(@"Connected, waiting for pairing response");
            self.transmitter = [[CommandTransmitter alloc] initWithClient:self.client];
        }
    } else if ([[note name] isEqualToString:@"connection.status.disconnected"]){
        if ([self.delegate respondsToSelector:@selector(disableSpinner)])
            [self.delegate disableSpinner];
        if (self.state != DISCONNECTED) {
            NSLog(@"Connection Failed");
            self.state = DISCONNECTED;
            [self.client disconnect];
            UIAlertView *message = [[UIAlertView alloc] initWithTitle:@"Failed to reach server"
                                                              message:@"Please verify the IP address and try again later"
                                                             delegate:nil
                                                    cancelButtonTitle:@"OK"
                                                    otherButtonTitles:@"Help", nil];
            [message show];
        }
    }
}

- (id) init
{
    self = [super init];
    self.state = DISCONNECTED;
    self.interpreter = [[CommandInterpreter alloc] init];
    self.servers = [[NSMutableArray alloc] init];
    
    [[NSNotificationCenter defaultCenter]addObserver: self
                                            selector: @selector(connectionStatusHandler:)
                                                name: @"connection.status.connected"
                                              object: nil];
    [[NSNotificationCenter defaultCenter]addObserver: self
                                            selector: @selector(connectionStatusHandler:)
                                                name: @"connection.status.disconnected"
                                              object: nil];
    
    return self;
}



- (id) initWithExistingServers
{
    self = [self init];
    NSUserDefaults * userDefaluts = [NSUserDefaults standardUserDefaults];
    
    if(!userDefaluts)
        NSLog(@"userDefaults nil");
    
    NSData *dataRepresentingExistingServers = [userDefaluts objectForKey:ExistingServersKey];
    if (dataRepresentingExistingServers != nil)
    {
        NSArray *oldSavedArray = [NSKeyedUnarchiver unarchiveObjectWithData:dataRepresentingExistingServers];
        if (oldSavedArray != nil)
            self.servers = [[NSMutableArray alloc] initWithArray:oldSavedArray];
        else
            self.servers = [[NSMutableArray alloc] init];
    } 
    return self;
}

- (void) connectToServer:(Server*)server
{
    if (self.state == CONNECTING) {
        return;
    } else {
            [self.client disconnect];
            self.state = CONNECTING;
            // initialise it with a given server
            self.client = [[Client alloc]initWithServer:server managedBy:self interpretedBy:self.interpreter];
            self.transmitter = [[CommandTransmitter alloc] initWithClient:self.client];
            [self.client connect];
    }
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

- (void) addServersWithName:(NSString*)name
                  AtAddress:(NSString*)addr
{
    Server * s = [[Server alloc] initWithProtocol:NETWORK atAddress:addr ofName:name];
    [self.servers addObject:s];
    [[NSUserDefaults standardUserDefaults] setObject:[NSKeyedArchiver archivedDataWithRootObject:self.servers] forKey:ExistingServersKey];
    [[NSUserDefaults standardUserDefaults] synchronize];
}

- (void) removeServerAtIndex:(NSUInteger)index
{
    [self.servers removeObjectAtIndex:index];
    [[NSUserDefaults standardUserDefaults] setObject:[NSKeyedArchiver archivedDataWithRootObject:self.servers] forKey:ExistingServersKey];
    [[NSUserDefaults standardUserDefaults] synchronize];
}

@end
