// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

typedef enum {TOP_LEFT,BOTTOM_RIGHT} MLOSelectionHandleType;
#define MLOSelectionHandleTypeString(enum) [@[@"TOP_LEFT",@"BOTTOM_RIGHT"] objectAtIndex:enum]

@class MLOSelectionViewController;
@interface MLOSelectionHandle : UIView

-(id)initWithType:(MLOSelectionHandleType) type selection:(MLOSelectionViewController *) selectionViewController;
-(void)addToMainViewController;
-(void)showAt:(CGPoint) edge;
-(void)hide;
-(void)onRotate;
-(CGPoint)getPivot;
-(BOOL)handleMoveAtPoint:(CGPoint) center gestureState:(UIGestureRecognizerState) state;
@end
