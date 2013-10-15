// -*- Mode: Objective-C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLORenderManager.h"

static const CGFloat
    NO_SCALE = 1.0f,
    NO_MOVE_DELTA = 0.0f;

typedef enum {PAN,PINCH,NO_GESTURE} MLOGestureType;
#define MLOGestureTypeString(enum) [@[@"PAN",@"PINCH",@"NO_GESTURE"] objectAtIndex:enum]

@class MLORenderBuffer,MLOScalingBuffer;
@interface MLORenderManager ()
@property MLOScalingBuffer * scaler;
@property CGRect bufferFrame;
@property MLOGestureType currentGesture;
-(void)swapPreviousBuffer:(MLORenderBuffer*) previous withNextBuffer:(MLORenderBuffer *) next;
-(void)loRenderWillBegin;
-(MLORenderBuffer *) getActiveBuffer;
@end
