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
#import "MLOMainViewController_Impl.h"
#import "MLOKeyboardManager.h"
#import "mlo_uno.h"

@interface MLOMainViewController ()

@property BOOL focused;
@property CGFloat topBarHeight;
@property UIView * flasher;
@property MLOTopbarViewController * topbar;
@property MLOToolbarViewController * toolbar;

@end

static const CGFloat
    FLASH_DURATION= 0.2f,
    EXPAND_DURATION=0.5f,
    ROTATE_FLASH_DURATION=0.5f;

@implementation MLOMainViewController

-(void)onTextEdit{
    [_scroller contentHasChanged];
}

-(BOOL)isTappable{
    return [_toolbar isTappable];
}

-(void)showLibreOffice:(UIWindow *) window{

    _topBarHeight = TOP_BAR_HEIGHT;
    [[UIApplication sharedApplication]setStatusBarHidden:YES];
    
    self.view.bounds = self.view.frame = [self getFullFrameForRect:self.view.frame];
    
    [_renderManager setWidth:self.view.frame.size.width height:self.view.frame.size.height];
    
    [_gestureEngine showLibreOffice:window];

    [self onStart];
    
    [_topbar showLibreOffice];
    [_toolbar showLibreOffice];

}

-(void)onStart{
    self.focused = YES;
    [self rotate];
}

-(void) hideLibreOffice{
    if(self.focused){
        
        self.focused = NO;
        [self.topbar hideLibreOffice];
        [self.toolbar hideLibreOffice];
    
        [self resetSubviews];
    
        [[UIApplication sharedApplication]setStatusBarHidden:NO];
            
        [self.gestureEngine hideLibreOffice];
    
        [self.scroller hideLibreOffice];
    
        [self.selection reset];
        
        [self.keyboard hide];
        
        [self.view removeFromSuperview];
        
        [[MLOManager getInstance] hideLibreOffice];
    }
}

-(void)initCanvas{
    
    self.view.frame = self.view.bounds =[self getFullFrameForRect:[[MLOManager getInstance] bounds]];
    
    self.canvas = [[UIView alloc]
                   initWithFrame:CGRectMake(0,
                                            TOP_BAR_HEIGHT,
                                            self.view.frame.size.width,
                                            self.view.frame.size.height - TOP_BAR_HEIGHT)];
    self.canvas.clipsToBounds = YES;
    self.canvas.backgroundColor = [UIColor whiteColor];
}

- (id) init{
    self = [super init];
    if(self){
    
        [self initCanvas];
        
        self.scroller = [[MLOScrollerViewController alloc] initWithMainViewController:self];
        
        self.selection = [[MLOSelectionViewController alloc] initWithMainViewController: self];
        
        self.toolbar = [[MLOToolbarViewController alloc] initWithMainViewController:self];

        self.topbar = [[MLOTopbarViewController alloc] initWithMainViewController:self];
        
        self.flasher =[[UIView alloc] initWithFrame:CGRECT_ZERO];
        
        self.keyboard = [[MLOKeyboardManager alloc]initWithMainViewController:self];
        
        _flasher.alpha = 0.0f;
        _flasher.backgroundColor = [UIColor whiteColor];

        self.gestureEngine = nil;
        [self addSubviews];
        
        [self onStart];
    
        _focused =NO;
        _topBarHeight = TOP_BAR_HEIGHT;
    }
    return self;
}

-(void) flash{
    _flasher.frame =self.view.frame;
    _flasher.alpha = 1.0f;
    [self.view addSubview:_flasher];
    [UIView animateWithDuration:FLASH_DURATION animations:^{
        _flasher.alpha=0.0f;
    } completion:^(BOOL finished) {
        [_flasher removeFromSuperview];
    }];
}

-(void) toggleExpand{
    CGFloat targetHeight = (_topBarHeight==0.0f)?TOP_BAR_HEIGHT:0.0f;
    CGRect mainFrame = self.view.frame;
    
    [UIView animateWithDuration:EXPAND_DURATION animations:^(void){
    
        _canvas.frame = CGRectMake(0, targetHeight, mainFrame.size.width, mainFrame.size.height - targetHeight);
        _renderManager.view.alpha= 0.0f;

        
    } completion:^(BOOL completed){
    
        _topBarHeight = targetHeight;
        [self rotate];
        [_toolbar expandDidToggle];
    }];
}

-(void)resize{
    CGRect mainViewRect = [self getFullFrameForRect:self.view.bounds];
    
    LOG_RECT(mainViewRect, @"MLO Resize: main view");
    
    self.view.bounds = self.view.frame = mainViewRect;
    
    CGFloat width = self.view.frame.size.width;
    CGFloat height = self.view.frame.size.height - _topBarHeight;
    
    CGRect canvasRect =CGRectMake(0, _topBarHeight, width, height);
   
    _canvas.frame =  canvasRect;
    
    [_renderManager setWidth:width  height:height];
    
    LOG_RECT(canvasRect, @"MLO Resize: canvas");

    
}

-(void) addSubviews{
    
    [_topbar addToMainViewController];
    
    [self.view addSubview:_canvas];
    
    self.view.backgroundColor = [UIColor whiteColor];
    self.renderManager = [MLORenderManager getInstance];
    
    [self resize];
    
    [self.canvas addSubview: _renderManager.view];
    
    self.gestureEngine = [[MLOGestureEngine alloc] initWithMainViewController:self];
    [_toolbar addToMainViewController];
    [_scroller addToMainViewController];
    [_selection addToMainViewController];
    [_keyboard addToMainViewController];
}

-(void)rotate{
    if(_focused){
        
        NSLog(@"MLO rotate");
        
        _renderManager.view.alpha= 0.0f;
        
        [self resize];
        
        [_gestureEngine onRotate];
        
        [_topbar onRotate];
        
        [_scroller reset];
        
        [_scroller showLibreOffice];
        
        [_selection onRotate];
        
        [_toolbar onRotate];
        
        
        [UIView animateWithDuration:ROTATE_FLASH_DURATION animations:^(void){
            _renderManager.view.alpha=1.0f;
        }];
        
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
