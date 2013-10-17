// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import <UIKit/UIKit.h>
#import "MLOCommon.h"

typedef enum {ZERO, ONE, INITIAL,FULL_SCREEN} MLORectSize;
#define MLORectSizeString(enum) [@[@"ZERO",@"ONE",@"INITIAL",@"FULL_SCREEN"] objectAtIndex:enum]

@interface MLOSubView : UIView

@property CGFloat fadeDuration;

- (id)initWithFrame:(CGRect)frame color:(UIColor *) color cornerRadius:(CGFloat) cornerRadius alpha:(CGFloat) alpha;
- (id)initHiddedWithColor:(UIColor *) color cornerRadius:(CGFloat) cornerRadius;
- (id)initHiddedWithColor:(UIColor *) color;
-(void)fade:(MLOFadeType) type;
-(void)fadeToPercent:(CGFloat) toPercent;
-(void)setSize:(MLORectSize) size;
-(void) hide;
@end
