// -*- Mode: Objective-C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOGestureEngine.h"

@class MLOGestureFlick,
    MLOMainViewController,
    MLOSelectionViewController,
    MLOScrollerViewController,
    MLOAnimation,
    MLORenderManager,
    MLOGestureLimiter;

@interface MLOGestureEngine ()

@property MLOMainViewController * mainViewController;
@property MLOSelectionViewController * selection;
@property MLOScrollerViewController * scroller;
@property MLOAnimation * currentAnimation;
@property MLOGestureFlick * flick;
@property MLORenderManager * renderer;
@property MLOGestureLimiter * limiter;
@property BOOL focused;
-(NSInteger) handledGesturesCount;
-(BOOL) loPanDeltaX:(NSInteger) rawDeltaX deltaY:(NSInteger) rawDeltaY state:(UIGestureRecognizerState) state;
-(void)loZoomDeltaX:(CGFloat)deltaX deltaY:(CGFloat)deltaY scale:(CGFloat) scale;
-(void) onKeyboardShow;
-(void) onKeyboardHide;
-(void) endFlick;
@end
