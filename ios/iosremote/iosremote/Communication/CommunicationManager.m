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

@interface CommunicationManager() <UIAlertViewDelegate>
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
            // A 5 seconds timer waiting for pairing response.
            [self.client startConnectionTimeoutTimerwithInterval:5.0];
            self.transmitter = [[CommandTransmitter alloc] initWithClient:self.client];
        }
    } else if ([[note name] isEqualToString:@"connection.status.disconnected"]){
        if ([self.delegate respondsToSelector:@selector(disableSpinner)])
            [self.delegate disableSpinner];
        if (self.state != DISCONNECTED) {
            NSLog(@"Connection Failed");
            self.state = DISCONNECTED;
            if ([self.delegate isKindOfClass:[server_list_vc class]]){
                dispatch_async(dispatch_get_main_queue(), ^{
                    UIAlertView *message = [[UIAlertView alloc] initWithTitle:@"Failed to reach server"
                                                                      message:@"Please verify your IP address and make sure that LibreOffice Impress is running with impress remote feature enabled. "
                                                                     delegate:self
                                                            cancelButtonTitle:@"OK"
                                                            otherButtonTitles:@"Help", nil];
                    [message show];
                });
            } else {
                [[self.delegate navigationController] popToRootViewControllerAnimated:YES];
                dispatch_async(dispatch_get_main_queue(), ^{
                    UIAlertView *message = [[UIAlertView alloc] initWithTitle:@"Connection Lost"
                                                                      message:@"Oups, connection lost...Please try to reconnect to your computer. "
                                                                     delegate:nil
                                                            cancelButtonTitle:@"OK"
                                                            otherButtonTitles:nil];
                    [message show];
                });
            }
        }
    }
}

- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex{
    if (buttonIndex == 0){
        [alertView dismissWithClickedButtonIndex:0 animated:YES];
    }else if (buttonIndex == 1){
        [alertView dismissWithClickedButtonIndex:0 animated:YES];
        UIAlertView *message = [[UIAlertView alloc] initWithTitle:@"Pairing instructions"
                                                          message:@"1. Launch your LibreOffice Impress on your Computer\n\n"
                                @"2. Enable Preferences - LibreOffice Impress - General - Enable remote control\n\n"
                                @"3. Enable Preferences - LibreOffice Impress - Advanced - Enable Experimental Features\n\n"
                                @"4. Make sure your Computer and your device are connected to the same WiFi network and Enter your Computer's IP address\n\n"
                                @"5. Connect and Enjoy!\n\n"
                                                         delegate:nil
                                                cancelButtonTitle:@"OK"
                                                otherButtonTitles:nil];
        [message show];
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
