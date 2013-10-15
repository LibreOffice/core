// -*- Mode: Objective-C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOScrollerViewController.h"
#import "MLOMainViewController.h"
#import "MLOSubView.h"
#import "MLOScrollerData.h"
#import "MLOScrollerTooltip.h"
#import "MLOScrollerGridViewController.h"
#import "mlo_uno.h"

@interface MLOScrollerViewController ()
@property BOOL isContentChanged,hasUpdated;
@property MLOMainViewController * mainViewController;
@property MLOSubView * scroller;
@property MLOScrollerTooltip * tooltip;
@property MLOScrollerGridViewController * grid;
@property NSDate * fadeOutTime;
@end

static const CGFloat
SCROLLER_CORNER_RADIUS =3.0f,
RESHAPE_ANIMATION_DURATION= 0.05f,
SCROLLER_FADE_OUT_DELAY=0.45f,
SCROLLER_FADE_OUT_DURATION=1.0f,
SCROLLER_FADE_OUT_INVOCATION =SCROLLER_FADE_OUT_DELAY+0.05f;

// Tooltip is removed at request of PM
// Maybe they'll want it back, or similar, in the future

static const BOOL IS_SHOW_TOOLTIP =NO;

@implementation MLOScrollerViewController

-(id) initWithMainViewController:(MLOMainViewController *) mainViewController{
    self = [super init];
    if(self){
        self.mainViewController = mainViewController;
        self.scroller = [[MLOSubView alloc] initHiddedWithColor:[UIColor grayColor] cornerRadius:SCROLLER_CORNER_RADIUS];
        self.data = [[MLOScrollerData alloc] initWithMainViewController:mainViewController];
        if(IS_SHOW_TOOLTIP){
            self.tooltip = [MLOScrollerTooltip new];
        }else{
            self.tooltip = nil;
        }
        self.fadeOutTime = nil;
        self.grid = [[MLOScrollerGridViewController alloc] initWithMainViewController:mainViewController];
        _isContentChanged =YES;
        _hasUpdated =NO;

        [_data onRotateWithGrid:_grid];
    }
    return self;
}

-(void) addToMainViewController{
    
    [_mainViewController.canvas addSubview:_scroller];
    [_mainViewController.canvas addSubview:_tooltip];
}

-(void) showLibreOffice{
    [_data showLibreOffice];
    [self contentHasChanged];
    
    if(mlo_is_document_open()){
        [self updateByLogic:NO];
    }
}

-(void)contentHasChanged{
    self.isContentChanged = YES;
}

-(void)onRotate{
    [self contentHasChanged];
    [_data onRotateWithGrid:_grid];

}

-(void)updateByLogic{
    [self updateByLogic:YES];
}

-(void)updateByPixelDeltaY:(CGFloat) pixelDeltaY{

    [self updateAndShowScroller:YES newScrollerFrame:[_data getShiftedScrollerFrame:pixelDeltaY]];
}

-(void)updateByLogic:(BOOL) isShow {
        
    if(_isContentChanged){
        
       [_grid onPageCountChanged:[_data getTotalPages]];
        
        _isContentChanged=NO;
    }

    [self updateAndShowScroller:isShow newScrollerFrame:[_data getNewScrollerFrame]];
}

-(void)updateAndShowScroller:(BOOL) isShow newScrollerFrame:(CGRect) newScrollerFrame{

    if(isShow){

        _scroller.alpha = 0.5;
    }
    [UIView animateWithDuration:_hasUpdated ? RESHAPE_ANIMATION_DURATION :0.0f
                     animations:^{ _scroller.frame = newScrollerFrame; }
                     completion:^(BOOL isCompleted){

                         self.fadeOutTime  = [NSDate dateWithTimeIntervalSinceNow:SCROLLER_FADE_OUT_DELAY];
                         [self performSelector:@selector(timedFadeOut) withObject:nil afterDelay:SCROLLER_FADE_OUT_INVOCATION];

                     }];

    [_data updateTooltip:_tooltip withGrid:_grid];

    _hasUpdated=YES;
}


-(void) reset{
    
    [_scroller hide];
    [_tooltip hide];
    
    _hasUpdated=NO;
    [self onRotate];
}

-(void) timedFadeOut {
    NSDate * date = self.fadeOutTime;

    if(![[date laterDate:[NSDate date]] isEqualToDate:date]){
        [self fadeOut];
    }
}

-(void)fadeOut{

    [_scroller fade:OUT];
}

-(void) hideLibreOffice{
    [_tooltip hideLibreOffice];
    [_grid hide];
    [self contentHasChanged];
    [_data hideLibreOffice];
}

@end
