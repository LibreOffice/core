// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import <Foundation/Foundation.h>

@class Client;


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

- (void) pointerCoordination:(CGPoint) aPoint;

@end
