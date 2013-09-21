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
@property (nonatomic, strong) Server* server;

// Connect to the server stored in [server]
- (void) connect;
// Disconnect from server
- (void) disconnect;
- (id) initWithServer:(Server*)server
            managedBy:(CommunicationManager*)manager
        interpretedBy:(CommandInterpreter*)receiver;
// Send a command (command by transmitter) to the server, base64 encoded
- (void) sendCommand:(NSString *)aCommand;
// Attempt to connect with a time out.
- (void)startConnectionTimeoutTimerwithInterval:(double) interval;
// Stop connection timeout timer
- (void)stopConnectionTimeoutTimer;
// Delegate method, handle stream events and transfer events to CommandInterpreter
-(void)stream:(NSStream *)stream handleEvent:(NSStreamEvent)eventCode;

@end