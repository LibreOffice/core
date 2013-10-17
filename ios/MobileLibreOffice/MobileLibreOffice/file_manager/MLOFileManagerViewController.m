// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOFileManagerViewController_Impl.h"
#import "MLOFileListViewController.h"
#import "MLOResourceImage.h"
#import "MLOAppViewController.h"
#import "MLOAppDelegate.h"
#import "MLOFileCacheManager.h"
#import "NSObject+MLOUtils.h"

static const NSTimeInterval FADE_TIME=1.0f;

static const CGFloat EMPTY_LABEL_WIDTH = 500.0f,
            EMPTY_LABEL_HEIGHT =120.0f,
            EMPTY_FONT_SIZE = 30.0f;

@interface MLOFileManagerViewController ()
@property MLOFileListViewController * list;
@property UILabel * emptyLabel;
@property BOOL isInit,isFirstRotation;
@end

@implementation MLOFileManagerViewController

-(id) initWithAppViewController:(MLOAppViewController *) appViewController{
    self = [super init];
    if(self){
        
        self.isInit = NO;
        self.isFirstRotation = YES;
        self.appViewController = appViewController;
        self.cache = [[MLOFileCacheManager alloc] initWithFileManager:self];
        self.list = [[MLOFileListViewController alloc] initWithFileManager:self];
        [self initEmptyLabel];
    
    }
    return self;
}

-(void)initEmptyLabel{
    self.emptyLabel = [[UILabel alloc]initWithFrame:CGRECT_ZERO];
    self.emptyLabel.text = @"No files to open.\r\nTry opening files from other apps.";
    self.emptyLabel.font = [UIFont systemFontOfSize:EMPTY_FONT_SIZE];
    self.emptyLabel.textAlignment =NSTextAlignmentCenter;
    self.emptyLabel.backgroundColor = [UIColor clearColor];
    self.emptyLabel.numberOfLines = 2;
    self.emptyLabel.alpha=  0.0f;
}

-(void)reloadData{
    if([self updateSubviewsAndIsHasItems]){
        [self.list reloadData];
    }
}
-(void)show{
    
    [[UIApplication sharedApplication]setStatusBarHidden:YES];
    
    if(!self.isInit){
        self.isInit = YES;
        
        [self.appViewController.view addSubview:self.view];
        [self.view addSubview:self.list.view];
        [self.view addSubview:self.emptyLabel];
        
        [self fade:0.0f];
    }
    
    [self reloadData];
    
    [self onRotate];
        
    [self animateFade:1.0f];
}

-(void)didHideLibreOffice{
    self.isFirstRotation = YES;
    [self show];
}

-(void)hide{
    [self animateFade:0.0f];
}

-(void)onRotate{
    
    CGRect superFrame = [self currentFullscreenFrame];
    
    if(self.isFirstRotation){
        self.isFirstRotation = NO;
        self.view.frame = superFrame;
    }else{
        self.view.frame = CGRectMake(0,0, superFrame.size.height, superFrame.size.width);
    }
    
    LOG_RECT(self.view.frame, @"MLOFileManagerViewController frame");
    
    if([self updateSubviewsAndIsHasItems]){
        
        [self.list onRotate];
    }
}

-(BOOL)updateSubviewsAndIsHasItems{
    if([self.cache count]==0){
        
        CGSize size =[self currentFullscreenFrame].size;
        
        self.emptyLabel.frame = CGRectMake((size.width - EMPTY_LABEL_WIDTH)/2.0f,
                                           size.height -(EMPTY_LABEL_HEIGHT+ 50),
                                           EMPTY_LABEL_WIDTH,
                                           EMPTY_LABEL_HEIGHT);
        
        self.emptyLabel.adjustsFontSizeToFitWidth=YES;
        
        self.emptyLabel.alpha = 1.0f;
        self.list.view.alpha = 0.0f;
        
        return NO;
    
    }
    
    self.emptyLabel.alpha = 0.0f;
    self.list.view.alpha = 1.0f;
    
    return YES;

}

-(void)animateFade:(CGFloat) alphaTarget{
    
    if(self.view.alpha!=alphaTarget){
    
        [UIView animateWithDuration:FADE_TIME animations:^{
            
            [self fade:alphaTarget];
        }];
    }
}

-(void)openFilePath:(NSString *)filePath{
    [self.cache openFilePath:filePath];
}

-(void)fade:(CGFloat) alpha{
    self.view.alpha = alpha;
}


-(CGRect)currentFullscreenFrame{
    return [self.appViewController currentFullscreenFrame];
}
@end
