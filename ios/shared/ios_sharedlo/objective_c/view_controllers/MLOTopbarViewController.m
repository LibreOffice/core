// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOTopbarViewController.h"
#import "MLOMainViewController_Friend.h"
#import "MLOMainViewController.h"
#import "MLOButton.h"
#import "MLOResourceImage.h"
#import "MLOManager.h"

@interface MLOTopbarViewController ()
@property MLOMainViewController * mainViewController;
@property UIView * blackbox;
@property MLOButton * button;
@property UILabel * label;
@property MLOResourceImage * buttonImage;
@end

static const CGFloat
    BUTTON_LEFT_SPACING = 18.0f,
    FONT_SIZE = 15.0f;


@implementation MLOTopbarViewController
-(id)initWithMainViewController:(MLOMainViewController *) mainViewController{
    self = [self init];
    if(self){
        self.mainViewController = mainViewController;
        
        self.blackbox = [[UIView alloc] initWithFrame:CGRECT_ZERO];
        _blackbox.backgroundColor = [UIColor blackColor];
        
        self.label = [[UILabel alloc] initWithFrame:CGRECT_ZERO];
        _label.textColor = [UIColor whiteColor];
        _label.backgroundColor = [UIColor clearColor];
        _label.textAlignment = NSTextAlignmentCenter;
        _label.font =[UIFont systemFontOfSize:FONT_SIZE];
        
        self.buttonImage = [MLOResourceImage backWithSize:NORMAL];
        self.button = [MLOButton buttonWithImage:_buttonImage];
        [_button addTarget:_mainViewController action:@selector(hideLibreOffice)];
        
        [self hideLibreOffice];
    }
    return self;
}
-(void)addToMainViewController{
    [_mainViewController.view addSubview:_blackbox];
    [_mainViewController.view addSubview:_button];
    [_mainViewController.view addSubview:_label];
}

-(void)hideLibreOffice{
    _button.alpha =0.0f;
    _button.alpha =0.0f;
    _label.alpha =0.0f;
    _blackbox.frame = CGRECT_ZERO;
    _button.frame =CGRECT_ZERO;
    _label.frame = CGRECT_ZERO;
}

-(void)showLibreOffice{
    _blackbox.alpha= 1.0f;
    _button.alpha = 1.0f;
    _label.alpha=1.0f;
    _label.text = [[MLOManager getInstance] filenameWithExtension];
}

-(void)onRotate{
    
    CGFloat screenWidth = _mainViewController.view.frame.size.width;
    
    _blackbox.frame = CGRectMake(0,0,screenWidth,TOP_BAR_HEIGHT);
    
    _button.frame = CGRectMake(0,//BUTTON_LEFT_SPACING,
                               0,//(TOP_BAR_HEIGHT - _buttonImage.image.size.height)/2.0f,
                               TOP_BAR_HEIGHT,// _buttonImage.image.size.width,
                               TOP_BAR_HEIGHT);//     _buttonImage.image.size.height);
    
    static const CGFloat
        LABEL_Y = (TOP_BAR_HEIGHT - FONT_SIZE)/4.0f,
        LABEL_HEIGHT = TOP_BAR_HEIGHT - LABEL_Y;
    
    _label.frame = CGRectMake(BUTTON_LEFT_SPACING, LABEL_Y, screenWidth-BUTTON_LEFT_SPACING, LABEL_HEIGHT);
}


@end
