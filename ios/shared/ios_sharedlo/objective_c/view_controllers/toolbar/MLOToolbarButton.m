// -*- Mode: Objective-C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOToolbarButton.h"
#import "MLOToolbarViewController.h"
#import "MLOToolbarViewController_Impl.h"
#import "MLOMainViewController.h"
#import "MLOResourceImage.h"

static const CGFloat
    TAPPED_ALPHA=1.0f,
    UNTAPPED_ALPHA=0.3f,
    BUTTON_IMAGE_X_EDGE_OF_CANVAS_PADDING = 40.0f,
    BUTTON_IMAGE_Y_EDGE_OF_CANVAS_PADDING = 30.0f,
    BUTTON_IMAGE_Y_SPACING = 40.0f,
    BUTTON_THICKENING = 10.f,
    FADE_TO_UNTAPPED_TIME=0.5f;

static NSInteger toolbarButtonsTotalHeight=BUTTON_IMAGE_Y_EDGE_OF_CANVAS_PADDING;

static const BOOL IS_FLASH_ON_TAP = NO;

@interface MLOToolbarButton ()
@property MLOToolbarViewController * toolbarController;
@property (nonatomic,strong) MLOToolbarButtonCallback onTap, onTapRelease;
@property BOOL isTapped,currentImageIsMain;
@property MLOToolbarButtonTapReleaseType tapReleaseType;
@property MLOResourceImage * mainImage;
@end


@implementation MLOToolbarButton

-(void)addToToolbarControler:(MLOToolbarViewController *) toolbar{
    self.toolbarController = toolbar;
    
}
+(MLOToolbarButton *)buttonWithImage:(MLOResourceImage *) image onTap:(MLOToolbarButtonCallback) onTap tapRelease:(MLOToolbarButtonTapReleaseType)type{
    return [MLOToolbarButton buttonWithImage:image onTap:onTap tapRelease:type onTapRelease:MLO_TOOLBAR_BUTTON_STUB_CALLBACK];
}
+(MLOToolbarButton *)buttonWithImage:(MLOResourceImage *) image onTap:(MLOToolbarButtonCallback) onTap tapRelease:(MLOToolbarButtonTapReleaseType)type onTapRelease:(MLOToolbarButtonCallback)onTapRelease
{
    
    MLOToolbarButton * button = [MLOToolbarButton buttonWithType:UIButtonTypeCustom];
  
    if(button){

        button.mainImage = image;
        button.onTap = onTap;
        button.tapReleaseType = type;
        button.onTapRelease = onTapRelease;
        button.alternateImage=nil;
        
        static const CGFloat BUTTON_X=BUTTON_IMAGE_X_EDGE_OF_CANVAS_PADDING -BUTTON_THICKENING,
                BUTTON_TWICE_THICKENING=2.0f*BUTTON_THICKENING;
        
        button.frame = CGRectMake(BUTTON_X,
                                  toolbarButtonsTotalHeight-BUTTON_THICKENING,
                                  image.image.size.width + BUTTON_TWICE_THICKENING,
                                  image.image.size.height + BUTTON_TWICE_THICKENING);
        
        toolbarButtonsTotalHeight += image.image.size.height + BUTTON_IMAGE_Y_SPACING;

        [button reset:0];
        [button addAction:@selector(onTapAction)];
        
    }
    return button;
}

-(BOOL)isHold{
    return _tapReleaseType != AUTOMATIC;
}

-(void)releaseIfNeeded{
    if(_isTapped && [self isHold]){
        [self invokeOnTapRelease];
    }
    _isTapped = NO;
}

-(void)reset:(CGFloat) alpha{
    [self releaseIfNeeded];
    self.alpha = alpha;
}

-(void)onOtherButtonTapped{
    if(_tapReleaseType == RETAP_OR_OTHER_TAPPED){
        [self releaseIfNeeded];
        [self fadeToUntapped];
    }
}

-(void)invokeOnTapRelease{
    [self invoke:_onTapRelease named:@"onTapRelease"];
}

-(void)switchImage{
    if(_alternateImage!=nil){
        
        [self setDefaultImage:_currentImageIsMain ? _alternateImage.image : _mainImage.image ];
        _currentImageIsMain^=YES;
    }
}
- (void) onTapAction{
    if(_isTapped){
        if([self isHold]){
            [self fadeToUntapped];
            [self invokeOnTapRelease];
        }
    }else{
        _isTapped=YES;
        
        if(IS_FLASH_ON_TAP){
            [_toolbarController.mainViewController flash];
        }
        self.alpha = TAPPED_ALPHA;
        if(![self isHold]){
            [self fadeToUntapped];
        }
        [_toolbarController hideAllButtonsBut:self];
        [self invoke:_onTap named:@"onTap"];
    }
}

-(void) fadeToUntapped{
    if(self.alpha>UNTAPPED_ALPHA){
        
        [UIView animateWithDuration:FADE_TO_UNTAPPED_TIME animations:^{
            self.alpha = UNTAPPED_ALPHA;
        }];
    }
    
    _isTapped = NO;
}

-(void)showLibreOffice{
    [self setDefaultImage:_mainImage.image];
    _currentImageIsMain = YES;
    [self reset:UNTAPPED_ALPHA];
}

-(void)hideLibreOffice{
    [self reset: 0];
}

-(void) invoke:(MLOToolbarButtonCallback) callback named:(NSString *) name{
    callback();
    NSLog(@"%@ button perfromed: %@",_mainImage.name, name);
}

@end
