/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#import <Foundation/Foundation.h>

@class Server;
@class CommunicationManager;
@class CommandInterpreter;

@interface Client : NSObject

@property BOOL connected;
@property (nonatomic, strong) NSNumber* pin;
@property (nonatomic, strong) NSString* name;
@property (nonatomic, weak) Server* server;

- (void) connect;
- (void) disconnect;

- (id) initWithServer:(Server*)server
            managedBy:(CommunicationManager*)manager
        interpretedBy:(CommandInterpreter*)receiver;

- (void) sendCommand:(NSString *)aCommand;

- (void)startConnectionTimeoutTimerwithInterval:(double) interval;
- (void)stopConnectionTimeoutTimer;

-(void)stream:(NSStream *)stream handleEvent:(NSStreamEvent)eventCode;

@end