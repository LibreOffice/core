// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOObject.h"

typedef void (^MLOAnimationBlock)(CGFloat fraction);
typedef void (^MLOAnimationBlockEnd)();

typedef enum {CANCELABLE, MANDATORY} MLOAnimationBehavior;
#define MLOAnimationBehaviorString(enum) [@[@"CANCELABLE",@"MANDATORY"] objectAtIndex:enum]
typedef enum {DELTA_ONLY,FULL_FRACTION} MLOAnimationFractionType;
#define MLOAnimationFractionTypeString(enum) [@[@"DELTA_ONLY",@"FULL_FRACTION"] objectAtIndex:enum]

static const NSTimeInterval DEFAULT_MLO_ANIMATION_DURAION=1.0F;

static const CGFloat DEFAULT_ANIMATION_FPS=25;

@interface MLOAnimation : MLOObject
@property NSTimeInterval duration;
@property CGFloat fps;
@property (nonatomic,strong) MLOAnimationBlockEnd endBlock;

-(id)initWithBehavior:(BOOL) behavior fractionType:(MLOAnimationFractionType) fractionType animation:(MLOAnimationBlock) animation;
-(void) linearCurve;
-(void) easeOutCurve;
-(void) easeInCurve;
-(void) cancel;
-(void) animate;
-(BOOL) isCancelled;

@end
