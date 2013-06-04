//
//  Server.h
//  sdremote
//
//  Created by Liu Siqi on 6/3/13.
//  Copyright (c) 2013 libreoffice. All rights reserved.
//

#import <Foundation/Foundation.h>

typedef enum protocol {NETWORK} Protocol_t;

@interface Server : NSObject

@property (nonatomic) Protocol_t protocol;
@property (nonatomic, strong) NSString* serverName;
@property (nonatomic, strong) NSString* serverAddress;

- (id)initWithProtocol:(Protocol_t)protocal
             atAddress:(NSString*) address
                ofName:(NSString*) name;

@end
