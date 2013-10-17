// -*- Mode: Objective-C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOToolbarViewController.h"
#import "MLOSelectionViewController.h"
#import "MLOGestureEngine.h"
#import "MLOScrollerViewController.h"
#import "MLOManager.h"
#import "MLOGestureLimiter.h"
#import "MLOGestureEngine_Impl.h"
#import "MLORenderManager.h"
#import "NSObject+MLOUtils.h"
#import "MLOTopbarViewController.h"
#import "MLOMainViewController_Private.h"
#import "MLOKeyboardManager.h"
#import "MLOAppRoleFactory.h"
#import "MLOAppRoleBase.h"
#import "mlo_uno.h"

static const CGFloat
    FLASH_DURATION= 0.2f,
    EXPAND_DURATION=0.5f;

@implementation MLOMainViewController

-(void)onTextEdit{
    [_scroller contentHasChanged];
}

-(BOOL)isTappable{
    return [_toolbar isTappable];
}

-(void)showLibreOffice:(UIWindow *) window{

    self.topBarHeight = TOP_BAR_HEIGHT;
    
    [[UIApplication sharedApplication]setStatusBarHidden:YES];
    
    self.view.bounds = self.view.frame = [self getFullFrameForRect:self.view.frame];

    [self.role initWindow:window];
    
    [self onStart];
    
    [self.topbar showLibreOffice];

    [self.role showLibreOffice];

}

-(void)onStart{
    self.focused = YES;
    [self rotate];
}

-(void) hideLibreOffice{
    if(self.focused){
        
        self.focused = NO;

        [self.topbar hideLibreOffice];

        [[UIApplication sharedApplication]setStatusBarHidden:NO];

        [self.role hideLibreOffice];
        [self.view removeFromSuperview];
        
        [[MLOManager getInstance] hideLibreOffice];
    }
}

-(void)initCanvas{
    self.view.frame = self.view.bounds =[self getFullFrameForRect:[[MLOManager getInstance] bounds]];
    CGRect canvasRect = CGRectMake(0,
                                   TOP_BAR_HEIGHT,
                                   self.view.frame.size.width,
                                   self.view.frame.size.height - TOP_BAR_HEIGHT);
    self.canvas = [[UIView alloc] initWithFrame:canvasRect];
    self.canvas.clipsToBounds = YES;
    self.canvas.backgroundColor = [UIColor whiteColor];
}

- (id) init{
    self = [super init];
    if(self){
        
        self.role = [MLOAppRoleFactory getInstanceWithMainViewController:self];
        
        [self initCanvas];

        [self.role initSubviews];

        self.topbar = [[MLOTopbarViewController alloc] initWithMainViewController:self];
        
        [self addSubviews];
        
        [self onStart];
    
        self.focused = NO;
        self.topBarHeight = TOP_BAR_HEIGHT;
    }
    return self;
}

-(void) flash{
    self.flasher.frame =self.view.frame;
    self.flasher.alpha = 1.0f;
    [self.view addSubview:_flasher];
    [UIView animateWithDuration:FLASH_DURATION animations:^{
        self.flasher.alpha=0.0f;
    } completion:^(BOOL finished) {
        [self.flasher removeFromSuperview];
    }];
}

-(void) toggleExpand{
    CGFloat targetHeight = (_topBarHeight==0.0f)?TOP_BAR_HEIGHT:0.0f;
    CGRect mainFrame = self.view.frame;
    
    [UIView animateWithDuration:EXPAND_DURATION animations:^(void){
    
        self.canvas.frame = CGRectMake(0, targetHeight, mainFrame.size.width, mainFrame.size.height - targetHeight);
        self.renderManager.view.alpha= 0.0f;

        
    } completion:^(BOOL completed){
    
        self.topBarHeight = targetHeight;
        [self rotate];
        [self.toolbar expandDidToggle];
    }];
}

-(void)resize{
    CGRect mainViewRect = [self getFullFrameForRect:self.view.bounds];
    LOG_RECT(mainViewRect, @"MLO Resize: main view");
    
    self.view.bounds = self.view.frame = mainViewRect;
    
    CGFloat width = self.view.frame.size.width;
    CGFloat height = self.view.frame.size.height - self.topBarHeight;
    
    CGRect canvasRect =CGRectMake(0, _topBarHeight, width, height);
    self.canvas.frame =  canvasRect;
    [self.role setWidth:width height:height];
    
    LOG_RECT(canvasRect, @"MLO Resize: canvas");
}

-(void) addSubviews{
    
    [self.topbar addToMainViewController];
    [self.view addSubview:self.canvas];
    self.view.backgroundColor = [UIColor whiteColor];
    [self.role addSubviews];
}

-(void)rotate{
    if(self.focused){
        [self.role rotate];
    }
}

-(void)resetSubviews{

    [_gestureEngine reset];
    [_scroller reset];
    [_selection reset];
}

-(CGFloat) getZoom{
    return [_gestureEngine.limiter zoom];
}


@end


// vim:set shiftwidth=4 softtabstop=4 expandtab:
