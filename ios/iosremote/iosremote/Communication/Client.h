//
//  Client.h
//  
//
//  Created by Liu Siqi on 6/3/13.
//  Copyright (c) 2013 libreoffice. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "Server.h"
#import "CommunicationManager.h"
#import "Receiver.h"

@interface Client : NSObject

-(void) connect;

- (id) initWithServer:(Server*)server
            managedBy:(CommunicationManager*)manager
        interpretedBy:(Receiver*)receiver;

-(void)stream:(NSStream *)stream handleEvent:(NSStreamEvent)eventCode;

@end