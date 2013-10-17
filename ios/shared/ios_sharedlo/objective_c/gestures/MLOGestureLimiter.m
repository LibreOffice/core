// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOGestureLimiter.h"
#import "MLOGestureEngine_Impl.h"
#import "MLOMainViewController_Friend.h"
#import "MLOScrollerData.h"
#import "MLOScrollerViewController.h"
#import "MLORenderManager_Impl.h"
#import "mlo_utils.h"
#import "mlo_uno.h"

@interface MLOGestureLimiter ()
@property MLOGestureEngine * engine;
@property MLOMainViewController * mainViewController;
@property CGFloat actualZoom;
@property CGFloat actualPixelToLogicScale;
@property CGSize halfViewSizeInLogic;
@property CGPoint viewCenterInLogic;
@property BOOL didCreateViewCenterInLogic;
@property CGFloat initialPinchZoom;
@property CGFloat previousPinchScale;
@property CGFloat previousScaleSentToLibreOffice;
@end

static const CGFloat DELTA_SCALE_ZOOM_IN_THRESHOLD=1.5f,
                    DELTA_SCALE_ZOOM_OUT_THRESHOLD=0.7f,
                    MIN_ZOOM_FOR_HORIZONAL_PAN = 133.0f;


@implementation MLOGestureLimiter

-(CGFloat) zoom{
    return self.actualZoom;
}
-(id) initWithGestureEngine:(MLOGestureEngine *) engine{
 
    self =  [self init];
    if(self){
        self.engine= engine;
        self.mainViewController = nil;
    }
    return self;
}

-(void) showLibreOffice{
    self.mainViewController = _engine.mainViewController;
    [self resetLocationMetrics];
}

-(NSString *)azimuthToString:(CGFloat) delta direction:(MLOPixelDeltaDirection) direction{
    if(delta==0){
        return @"N/A";
    }
    switch (direction) {
        case DELTA_X:
            if(delta < 0.0f) {
                return @"swipe LEFT (scroll right)";
            }
            return @"swipe RIGHT (scroll left)";
            
        case DELTA_Y:
            if(delta < 0.0f){
                
                return @"swipe UP (scroll down)";
            }
            return @"swipe DOWN (scroll up)";
    }
}

-(void)logPanRawDeltaX:(NSInteger) rawDeltaX rawDeltaY:(NSInteger) rawDeltaY limitedDeltaX:(NSInteger) limitedDeltaX limitedDeltaY:(NSInteger) limitedDeltaY{
    NSLog(@"PAN %@ %@: limited:(%d,%d) raw:(%d,%d) center:(%d,%d) viewSize:(%d,%d) logicSize:(%d,%d)",
          [self azimuthToString:rawDeltaX direction:DELTA_X],
          [self azimuthToString:rawDeltaY direction:DELTA_Y],
          limitedDeltaX,
          limitedDeltaY,
          rawDeltaX,
          rawDeltaY,
          (NSInteger)self.viewCenterInLogic.x,
          (NSInteger)self.viewCenterInLogic.y,
          (NSInteger)self.halfViewSizeInLogic.width *2,
          (NSInteger)self.halfViewSizeInLogic.height *2,
          (NSInteger)self.documentSizeInLogic.width,
          (NSInteger)self.documentSizeInLogic.height);
}


-(CGFloat)limitDelta:(CGFloat) delta direction:(MLOPixelDeltaDirection) direction{
    
    if(delta==0){
        return 0;
    }
    CGFloat deltaInLogic = [self pixelsToLogic:delta];
    
    CGFloat limit = [self limitForRawDelta:deltaInLogic direction:direction];
    
    if(deltaInLogic > 0){
        deltaInLogic = min(limit,deltaInLogic);
    }else{
        deltaInLogic = max(limit,deltaInLogic);
    }
    
    [self updateCenterInLogic:deltaInLogic direction:direction];
    
    return [self logicToPixels:deltaInLogic];
}

-(CGFloat) limitForRawDelta:(CGFloat) raw direction:(MLOPixelDeltaDirection) direction{
      
    CGFloat halfscreen = [self halfViewSizeInLogic:direction];
    CGFloat center = [self viewCenterCord:direction];
    
    CGFloat limit;
    
    if(raw < 0.0f){
    
        CGFloat document = [self documentSizeInLogic:direction];
        limit = halfscreen + center - document;
        
        if(LOG_GESTURE_LIMITING){
            NSLog(@"%@ LIMIT: raw=%f limit(%f) = halfscreen(%f) + center(%f) - document(%f)",MLOPixelDeltaDirectionString(direction),raw,limit,halfscreen,center,document);
        }
        
    }else{
        limit = center - halfscreen;
        
        if(LOG_GESTURE_LIMITING){
            NSLog(@"%@ LIMIT: raw=%f limit(%f) = center(%f) - halfscreen(%f)",MLOPixelDeltaDirectionString(direction),raw,limit,center,halfscreen);
        }
    }
    if(limit*raw <0){
        // negative means limit flipped sign, so limit is zero
        return 0;
    }
    
    return limit;
}

-(CGFloat) halfViewSizeInLogic:(MLOPixelDeltaDirection) direction{
    switch (direction) {
        case DELTA_X: return self.halfViewSizeInLogic.width;
        case DELTA_Y: return self.halfViewSizeInLogic.height;
    }
}

-(CGFloat) documentSizeInLogic:(MLOPixelDeltaDirection) direction{
    switch (direction) {
        case DELTA_X: return self.documentSizeInLogic.width;
        case DELTA_Y: return self.documentSizeInLogic.height;
    }
}

-(CGFloat) viewCenterCord:(MLOPixelDeltaDirection) direction{
    switch (direction) {
        case DELTA_X: return [self centerInLogic].x;
        case DELTA_Y: return [self centerInLogic].y;
    }
}

-(void) updateCenterInLogic:(CGFloat) delta direction:(MLOPixelDeltaDirection) direction{
    switch (direction) {
        case DELTA_X: _viewCenterInLogic.x-=delta; break;
        case DELTA_Y: _viewCenterInLogic.y-=delta; break;
    }
}

-(void)createHalfViewSizeInLogic{
    self.halfViewSizeInLogic = CGSizeMake(_mainViewController.canvas.frame.size.width
                                          * self.actualPixelToLogicScale
                                          / 2.0f,
                                          _mainViewController.canvas.frame.size.height
                                          * self.actualPixelToLogicScale
                                          / 2.0f);
}

-(CGFloat)pixelsToLogic:(CGFloat) distanceInPixels{
    return roundf(self.actualPixelToLogicScale * distanceInPixels);
}

-(CGFloat)logicToPixels:(CGFloat) distanceInLogic{
    return roundf(distanceInLogic / self.actualPixelToLogicScale);
}

-(CGPoint)centerInLogic{
    if(!self.didCreateViewCenterInLogic){
        [self createHalfViewSizeInLogic];
        self.viewCenterInLogic = CGPointMake(self.halfViewSizeInLogic.width,
                                             self.halfViewSizeInLogic.height);
        self.didCreateViewCenterInLogic = YES;
    }
    
    return self.viewCenterInLogic;
}

-(void)resetLocationMetrics{
    self.actualZoom = 100.0f;
    [self updateActualZoom:100.f];
    self.didCreateViewCenterInLogic = NO;
}

-(void)updateActualZoom:(CGFloat) newActualZoom{
    
    CGFloat viewResizeScale = self.actualZoom / newActualZoom;
    
    self.halfViewSizeInLogic = CGSizeMake(self.halfViewSizeInLogic.width * viewResizeScale,
                                          self.halfViewSizeInLogic.height * viewResizeScale);
    self.actualZoom = newActualZoom;
    
    static const CGFloat PIXEL_TO_LOGIC_RATION_TIMES_HUNDRED = PIXEL_TO_LOGIC_RATIO * 100.0f;
    
    self.actualPixelToLogicScale  = PIXEL_TO_LOGIC_RATION_TIMES_HUNDRED / newActualZoom;
 }

-(void)beginPinch{
    [_mainViewController.scroller updateByLogic];
    self.previousPinchScale = NO_SCALE;
    self.previousScaleSentToLibreOffice = NO_SCALE;
    self.initialPinchZoom = self.actualZoom;
}


-(void)onRotate{
    if(self.didCreateViewCenterInLogic){
        [self createHalfViewSizeInLogic];
    }
}
-(CGFloat) inPinchGetRatioToLastScale:(CGFloat)newScale{
    
    CGFloat newZoom = self.initialPinchZoom *newScale;
    
    newZoom = max(min(newZoom, MAX_ZOOM),MIN_ZOOM);
    
    NSLog(@"new zoom is %f",newZoom);
    
    [self updateActualZoom:newZoom];
    
    newScale = newZoom / self.initialPinchZoom;
    
    CGFloat scaleRatioToLastScale  = newScale / self.previousPinchScale;
    
    self.previousPinchScale =newScale;
    
    return scaleRatioToLastScale;
}

-(void)fireLoZoomEventsDuringPinch{
    CGFloat deltaScale = self.previousPinchScale / self.previousScaleSentToLibreOffice;
    
    if((deltaScale > DELTA_SCALE_ZOOM_IN_THRESHOLD) ||
       (deltaScale < DELTA_SCALE_ZOOM_OUT_THRESHOLD)){
        self.previousScaleSentToLibreOffice = self.previousPinchScale;
     [_engine loZoomDeltaX:0.0f deltaY:0.0f scale:deltaScale];
    }
}
-(CGFloat)endPinchAndGetScaleForLo:(CGFloat)scale{
   [self updateActualZoom: floorf(self.actualZoom)];
    return scale/self.previousScaleSentToLibreOffice;
}

-(CGFloat)currentPinchScale{
    return self.previousPinchScale;
}

@end
