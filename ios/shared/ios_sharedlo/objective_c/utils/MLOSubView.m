// -*- Mode: Objective-C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOCommon.h"
#import "MLOSubView.h"

#import <QuartzCore/QuartzCore.h>

#define SIZE_CASE(CASE,RECT) case CASE: self.frame = self.bounds = RECT;break;

static const CGFloat DEFAULT_FADE_DURAION= 1.0f;

@interface MLOSubView ()
@property CGRect defaultRect;
@end

@implementation MLOSubView

- (id)initWithFrame:(CGRect)frame color:(UIColor *) color cornerRadius:(CGFloat) cornerRadius alpha:(CGFloat) alpha
{
    self = [super initWithFrame:frame];
    if (self) {
        self.defaultRect = frame;
        self.backgroundColor = color;
        self.fadeDuration = DEFAULT_FADE_DURAION;
        
        if([self isLegalNewAlpha:alpha]){
            self.alpha = alpha;
        }
        
        if(cornerRadius >= 0){
            [self.layer setCornerRadius:cornerRadius];
            [self.layer setMasksToBounds:YES];
        }
    }
    return self;
}

- (id)initHiddedWithColor:(UIColor *) color cornerRadius:(CGFloat) cornerRadius{
    
    return [self initWithFrame:CGRECT_ZERO color:color cornerRadius:cornerRadius alpha: 0.0f];
}

- (id)initHiddedWithColor:(UIColor *) color{
    return [self initHiddedWithColor:color cornerRadius:-1.0f];
}


-(BOOL)isLegalNewAlpha:(CGFloat) alpha{
    return (alpha>=0) && (alpha<=1) && (alpha!= self.alpha);
}


-(void)fade:(MLOFadeType) type{
    CGFloat alpha;
    switch (type) {
        case IN: alpha = 1.0f; break;
        case OUT: alpha = 0.0f; break;
    }
    [self fadeToPercent:alpha];
}


-(void)setSize:(MLORectSize) size{

    switch(size){
        SIZE_CASE(ZERO, CGRECT_ZERO)
        SIZE_CASE(ONE, CGRECT_ONE)
        SIZE_CASE(INITIAL, _defaultRect)
        SIZE_CASE(FULL_SCREEN, [[UIApplication sharedApplication] keyWindow].frame)
    }
}

-(void) hide{
    self.alpha=0;
    [self setSize:ZERO];
}

-(void)fadeToPercent:(CGFloat) toPercent{
    if([self isLegalNewAlpha:toPercent]){
    
        [UIView animateWithDuration:1.0 animations:^{
            
            self.alpha = toPercent;
            
        }];
    }
}



@end
