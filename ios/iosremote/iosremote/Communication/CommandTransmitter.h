//
//  CommandTransmitter.h
//  iosremote
//
//  Created by Liu Siqi on 6/9/13.
//  Copyright (c) 2013 libreoffice. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "Client.h"

@interface CommandTransmitter : NSObject

- (CommandTransmitter*) initWithClient:(Client *)client;

- (void) nextTransition;
- (void) previousTransition;

- (void) gotoSlide:(uint) slide;

- (void) blankScreen;
- (void) blankScreenWithColor:(UIColor*)color;
- (void) resume;

- (void) startPresentation;
- (void) stopPresentation;

@end
