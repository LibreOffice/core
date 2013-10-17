// -*- Mode: Objective-C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOAppRoleLoApp.h"
#import "MLOMainViewController_Private.h"
#import "MLORenderManager.h"
#import "MLOGestureEngine.h"
#import "MLOToolbarViewController.h"
#import "MLOSelectionViewController.h"
#import "MLOScrollerViewController.h"
#import "MLOTopbarViewController.h"
#import "MLOKeyboardManager.h"

static const NSTimeInterval ROTATE_FLASH_DURATION=0.5f;

@implementation MLOAppRoleLoApp

-(void)initSubviews{
    MLOMainViewController * main = [self mainViewController];
    main.scroller = [[MLOScrollerViewController alloc] initWithMainViewController:main];
    main.selection = [[MLOSelectionViewController alloc] initWithMainViewController: main];
    main.toolbar = [[MLOToolbarViewController alloc] initWithMainViewController:main];
    main.keyboard = [[MLOKeyboardManager alloc]initWithMainViewController:main];
    main.flasher =[[UIView alloc] initWithFrame:CGRECT_ZERO];
    main.flasher.alpha = 0.0f;
    main.flasher.backgroundColor = [UIColor whiteColor];
    main.gestureEngine = nil;
}

-(void)setWidth:(CGFloat) width height:(CGFloat) height{

    [[self mainViewController].renderManager setWidth:width  height:height];
}

-(void)addSubviews{

    MLOMainViewController * main = [self mainViewController];
    main.renderManager = [MLORenderManager getInstance];

    [main resize];

    [main.canvas addSubview: main.renderManager.view];

    main.gestureEngine = [[MLOGestureEngine alloc] initWithMainViewController:main];
    [main.toolbar addToMainViewController];
    [main.scroller addToMainViewController];
    [main.selection addToMainViewController];
    [main.keyboard addToMainViewController];
}

-(void)rotate{

    MLOMainViewController * main = self.mainViewController;
    
    NSLog(@"MLO rotate");

    main.renderManager.view.alpha= 0.0f;

    [main resize];

    [main.gestureEngine onRotate];

    [main.topbar onRotate];

    [main.scroller reset];

    [main.scroller showLibreOffice];

    [main.selection onRotate];

    [main.toolbar onRotate];

    [UIView animateWithDuration:ROTATE_FLASH_DURATION animations:^(void){
        main.renderManager.view.alpha=1.0f;
    }];
}


-(void)showLibreOffice{

    [self.mainViewController.toolbar showLibreOffice];
}

-(void)hideLibreOffice{
    MLOMainViewController * main = self.mainViewController;
    
    [main.toolbar hideLibreOffice];
    [main resetSubviews];
    [main.gestureEngine hideLibreOffice];
    [main.scroller hideLibreOffice];
    [main.selection reset];
    [main.keyboard hide];
}

-(void)initWindow:(UIWindow *) window{
    MLOMainViewController * main = self.mainViewController;
    [main.renderManager setWidth:main.view.frame.size.width height:main.view.frame.size.height];

    [main.gestureEngine showLibreOffice:window];
}

@end
