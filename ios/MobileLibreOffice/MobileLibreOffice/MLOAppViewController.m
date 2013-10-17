// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOAppViewController.h"
#import "MLOAppDelegate.h"
#import "MLOManager.h"
#import "MLOFileManagerViewController.h"
#import "MLOResourceImage.h"

static const CGFloat LOGO_SCALE_RATIO = 0.5f;
static const CGFloat BACKGROUND_ALPHA = 0.4f;
static const NSTimeInterval FADE_IN_DURATION = 1.0f;

@interface MLOAppViewController ()
@property UIView * backgroundImage;
@property CGRect currentFullscreenFrame;
@end

@implementation MLOAppViewController

-(id)initWithAppDelegate:(MLOAppDelegate *)appDelegate{
    self = [self init];
    if(self){
        
        self.appDelegate = appDelegate;
        
        [self addLoBackground];
        
        [self rotateTo:[[UIApplication sharedApplication] statusBarOrientation]];
        
        self.fileManager = [[MLOFileManagerViewController alloc] initWithAppViewController:self];
        

    }
    return self;
    
}

-(void)addLoBackground{
    
    self.view.backgroundColor = [UIColor whiteColor];
    
    self.backgroundImage = [[UIView alloc] initWithFrame:CGRECT_ZERO];
    
    [self.view addSubview:self.backgroundImage];
    [self.view sendSubviewToBack:self.backgroundImage];
    
    
}

-(CGRect)backgroundRectForOrientation:(UIInterfaceOrientation) orientation{
    
    CGRect appBounds = [self.appDelegate bounds];
    CGFloat bigger = appBounds.size.height;
    CGFloat smaller = appBounds.size.width;
    
    if(smaller>bigger){
        bigger= smaller;
        smaller = appBounds.size.height;
    }
    
    CGFloat frameWidth,frameHeight;
    
    if((orientation == UIInterfaceOrientationLandscapeLeft) ||
       (orientation == UIInterfaceOrientationLandscapeRight)){
        frameHeight = smaller;
        frameWidth = bigger;
    }else{
        frameHeight= bigger;
        frameWidth = smaller;
    }
    
    self.currentFullscreenFrame = CGRectMake(0,0,frameWidth,frameHeight);
    
    CGFloat width = frameWidth * LOGO_SCALE_RATIO;
    
    UIImage * logo = [MLOResourceImage loLogo];
    
    CGFloat height = (width / logo.size.width) * logo.size.height;
    CGFloat originX = (frameWidth- width) /2.0f;
    CGFloat originY = (frameHeight - (height+((width-height)/2.0f))) /2.0f;
    
    return CGRectMake(originX,originY,width,height);
}

-(void)adjustLoBackground:(UIInterfaceOrientation) orientation{
    
    self.backgroundImage.alpha = 0.0f;
    
    self.backgroundImage.frame = [self backgroundRectForOrientation:orientation];
    
    UIGraphicsBeginImageContext( self.backgroundImage.frame.size);
    [[MLOResourceImage loLogo] drawInRect: self.backgroundImage.bounds];
    UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
   
    self.backgroundImage.backgroundColor = [UIColor colorWithPatternImage:image];
    
    [UIView animateWithDuration:FADE_IN_DURATION animations:^{
        self.backgroundImage.alpha = BACKGROUND_ALPHA;
    }];
    
}

-(void)rotateTo:(UIInterfaceOrientation) orientation{
    
    [self adjustLoBackground:orientation];
    
    [self.fileManager onRotate];
}
-(void)willRotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation duration:(NSTimeInterval)duration{
    
    [UIView animateWithDuration:duration*0.7f animations:^{
    
        self.backgroundImage.alpha = 0.0f;
    }];
    
    [self rotateTo:toInterfaceOrientation];
}

-(void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)fromInterfaceOrientation{
    
    [[MLOManager getInstance] application:nil didChangeStatusBarFrame:self.view.frame];
}

@end
