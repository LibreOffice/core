// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOScalingBuffer.h"
#import "MLORenderBuffer.h"
#import "MLORenderingUIView.h"

#import "MLORenderManager_Impl.h"
#import <QuartzCore/QuartzCore.h>

@interface MLOScalingBuffer ()
@property MLORenderManager * manager;
@property MLORenderBuffer * active;
@property BOOL didHideActive;
@end

@implementation MLOScalingBuffer

-(id)initWithRenderManager:(MLORenderManager *) manager{
    self= [self initWithFrame:manager.view.frame];
    if(self){
        self.didRender = NO;
        self.didHideActive =NO;
        self.manager = manager;
        self.active  =[manager getActiveBuffer];
        self.alpha = 1.0f;
        self.backgroundColor = [UIColor clearColor];
        [self.manager.view addSubview:self];
        self.frame = manager.bufferFrame;
        [self setNeedsDisplay];
        
    }
    return self;
}

-(void)hideActive{
    if(self.didRender && !self.didHideActive){
        self.didHideActive = YES;
        [self.active hide];
    }
}

-(void)scale:(CGFloat) scale deltaX:(CGFloat) deltaX deltaY:(CGFloat) deltaY{

    [self hideActive];
    if(self.didHideActive){
        self.layer.transform = CATransform3DMakeScale(scale,scale,1);
    }
    
    [self moveDeltaX:deltaX deltaY:deltaY];
}

-(void)hide{
    self.alpha = 0.0f;
    [self removeFromSuperview];
    self.manager.scaler = nil;
}

-(void)drawRect:(CGRect)rect{

    if(!self.didRender){
        
        CGContextRef context = UIGraphicsGetCurrentContext();
//        CGContextSaveGState(context);
//        CGContextFillRect(context, self.bounds);
        [self.active.layer renderInContext:context];
//        CGContextRestoreGState(context);
        self.didRender = YES;
    }else{
        [self hideActive];
        [super drawRect:rect];
    }
}
@end
