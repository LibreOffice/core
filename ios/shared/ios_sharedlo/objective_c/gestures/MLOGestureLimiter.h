// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOObject.h"

static const CGFloat
    MAX_ZOOM = 600.0f,
    MIN_ZOOM = 80.0f;

typedef enum {DELTA_X,DELTA_Y} MLOPixelDeltaDirection;
#define MLOPixelDeltaDirectionString(enum) [@[@"DELTA_X",@"DELTA_Y"] objectAtIndex:enum]

@class MLOGestureEngine;
@interface MLOGestureLimiter : MLOObject
@property CGSize documentSizeInLogic;

-(id) initWithGestureEngine:(MLOGestureEngine *) engine;
-(void) showLibreOffice;
-(CGFloat)limitDelta:(CGFloat) delta direction:(MLOPixelDeltaDirection) direction;
-(void)logPanRawDeltaX:(NSInteger) preDeltaX rawDeltaY:(NSInteger) preDeltaY limitedDeltaX:(NSInteger) actualDeltaX limitedDeltaY:(NSInteger) actualDeltaY;
-(void)beginPinch;
-(void)onRotate;
-(CGFloat)inPinchGetRatioToLastScale:(CGFloat) newScale;
-(void)fireLoZoomEventsDuringPinch;
-(CGFloat)currentPinchScale;
-(CGFloat)endPinchAndGetScaleForLo:(CGFloat)scale;
-(CGFloat)zoom;
@end
