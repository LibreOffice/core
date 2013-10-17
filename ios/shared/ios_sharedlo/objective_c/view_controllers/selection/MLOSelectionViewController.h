// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOObject.h"

@class MLOMainViewController;
@interface MLOSelectionViewController : MLOObject

-(id)initWithMainViewController:(MLOMainViewController *) mainViewController;
-(void)addToMainViewController;
-(void)reset;
-(void) onRotate;
-(void) showBetweenFinger:(CGPoint) finger andPreviousPoint:(CGPoint) previousPoint showMenu:(BOOL) showMenu;
-(void) showPostSelectAll;
-(void) hide;
-(BOOL)handleMoveAtPoint:(CGPoint) center gestureState:(UIGestureRecognizerState) state;
@end
