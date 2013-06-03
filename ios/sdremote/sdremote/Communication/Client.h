//
//  Client.h
//  sdremote
//
//  Created by Liu Siqi on 6/3/13.
//  Copyright (c) 2013 libreoffice. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface Client : NSObject <NSStreamDelegate>


+(void) connect;


@end