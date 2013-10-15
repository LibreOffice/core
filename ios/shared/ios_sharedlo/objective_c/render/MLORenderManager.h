// -*- Mode: Objective-C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOViewController.h"
#import <UIKit/UIKit.h>

static const BOOL ENABLE_PINCH_RENDERING_VIA_IOS = YES;

@class MLOGestureEngine;

@interface MLORenderManager : MLOViewController

+(MLORenderManager *) getInstance;

-(void) showLibreOffice:(MLOGestureEngine *) gestureEngine;
-(void) hideLibreOffice;
-(void) panDeltaX:(CGFloat) deltaX deltaY:(CGFloat) deltaY;
-(void) pinchDeltaX:(CGFloat)deltaX deltaY:(CGFloat)deltaY scale:(CGFloat)scale;
-(void) endGestures;
-(void)setWidth:(NSInteger) width height:(NSInteger) height;
-(void) renderInContext:(CGContextRef) context;
-(CGPoint) getShiftFromCanvasCenter;
-(void)renderNow;
@end

