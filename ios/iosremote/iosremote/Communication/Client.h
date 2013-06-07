/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#import <Foundation/Foundation.h>
#import "Server.h"
#import "CommunicationManager.h"
#import "CommandInterpreter.h"

@interface Client : NSObject

@property BOOL mReady;
@property (nonatomic, strong) NSNumber* mPin;
@property (nonatomic, strong) NSString* mName;

-(void) connect;

- (id) initWithServer:(Server*)server
            managedBy:(CommunicationManager*)manager
        interpretedBy:(CommandInterpreter*)receiver;

-(void)stream:(NSStream *)stream handleEvent:(NSStreamEvent)eventCode;

@end