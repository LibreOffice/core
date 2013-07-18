/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#import "CommandTransmitter.h"
#import "Client.h"

@interface CommandTransmitter()

@property (nonatomic, weak) Client* client;

@end

@implementation CommandTransmitter

@synthesize client = _client;

- (CommandTransmitter*) initWithClient:(Client *)client
{
    self = [self init];
    self.client = client;

    return self;
}

- (void) nextTransition
{
    [self.client sendCommand:@"transition_next\n\n"];
}

- (void) previousTransition
{
    [self.client sendCommand:@"transition_previous\n\n"];
}

- (void) gotoSlide:(uint) slide
{
    [self.client sendCommand:[NSString stringWithFormat:@"goto_slide\n%u\n\n", slide]];
}

- (void) pointerCoordination:(CGPoint) aPoint
{
    [self.client sendCommand:[NSString stringWithFormat:@"pointer_coordination\n%f\n%f\n\n", aPoint.x, aPoint.y]];
}


/**
 * Blank the screen to the default colour (set server-side), which is
 * generally black. This is slightly faster than using
 * <code> blankScreen( colour ) </code>.
 */
- (void) blankScreen
{
    [self.client sendCommand:@"presentation_blank_screen\n\n"];
}


/**
 * Set the screen to a specific colour. Only use if a non default colour is
 * needed.
 *
 * @param aColor
 */
- (void) blankScreenWithColor:(UIColor*)color
{
//    CGColorRef colorRef = color.CGColor;
//    NSString *colorString = [CIColor colorWithCGColor:colorRef].stringRepresentation;
    
    // Need new server-end interface, since this is a platform dependent representation
//    [self.client sendCommand:[NSString stringWithFormat:@"presentation_blank_screen\n%@\n\n", colorString]];
}

- (void) resume
{
    [self.client sendCommand:@"presentation_resume\n\n"];
}

- (void) startPresentation
{
    [self.client sendCommand:@"presentation_start\n\n"];
}

- (void) stopPresentation
{
    [self.client sendCommand:@"presentation_stop\n\n"];
}

@end
