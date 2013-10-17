// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOMainViewController_Friend.h"
#import "MLOMagnifier.h"
#import "MLOResourceImage.h"
#import "MLORenderManager.h"
#import <QuartzCore/QuartzCore.h>


@interface MLOMagnifier ()
@property CGPoint hotspot;
@property MLOMainViewController * mainViewController;
@property NSString * glassImageName,* maskImageName;
@property BOOL shown,added;
@end

static const CGFloat MAGNIFIER_DIM = 160.0f,
        HOTSPOT_TO_MAGNIFIER_DISTANCE= 66.0f;

@implementation MLOMagnifier

-(id) initWithMainViewController:(MLOMainViewController *) mainViewController{
    self = [self initWithFrame:CGRectMake(MAGNIFIER_DIM,MAGNIFIER_DIM,MAGNIFIER_DIM,MAGNIFIER_DIM)];
    if(self){
        self.mainViewController = mainViewController;
        self.layer.cornerRadius = MAGNIFIER_DIM / 2.0f;
        self.layer.masksToBounds = YES;
        self.glassImageName = [MLOResourceImage magnifierName:IMAGE];
        self.maskImageName = [MLOResourceImage magnifierName:MASK];
        self.shown = NO;
        self.added = NO;
    }
    return self;
}

-(void) addToMainViewController{

}


-(void) showAt:(CGPoint) point{
    if(!self.shown){
        self.shown = YES;
        [_mainViewController.canvas addSubview:self];
    }

    self.hotspot = point;

    self.center = CGPointMake(point.x, point.y-HOTSPOT_TO_MAGNIFIER_DISTANCE);

    [self setNeedsDisplay];

}
-(void) hide{

    if(self.shown){
        self.shown =NO;
        [self removeFromSuperview];
    }

    self.center = CGPointMake(2*MAGNIFIER_DIM, 2*MAGNIFIER_DIM);

}


- (void)drawRect:(CGRect)rect
{
    CGContextRef context = UIGraphicsGetCurrentContext();
    UIImage * glass = [UIImage imageNamed:_glassImageName];
 //   CGImageRef mask = [UIImage imageNamed:_maskImageName].CGImage;
    CGRect bounds = self.bounds;
    
    CGContextSaveGState(context);
    //CGContextClipToMask(context, bounds, mask);
    CGContextFillRect(context, bounds);
    CGContextScaleCTM(context, 1.2, 1.2);

    //draw your subject view here
    CGContextTranslateCTM(context,1*(self.frame.size.width*0.5),1*(self.frame.size.height*0.5));
    //CGContextScaleCTM(context, 1.5, 1.5);
    CGContextTranslateCTM(context,-1*(self.hotspot.x),-1*(self.hotspot.y));
    [_mainViewController.renderManager renderInContext:context];

    CGContextRestoreGState(context);
    [glass drawInRect: bounds];
}

@end
