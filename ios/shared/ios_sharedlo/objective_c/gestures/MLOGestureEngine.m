// -*- Mode: Objective-C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOAnimation.h"
#import "MLOMainViewController_Impl.h"
#import "MLORenderManager_Impl.h"
#import "MLOSelectionViewController.h"
#import "MLOScrollerViewController.h"
#import "MLOGestureFlick.h"
#import "MLOScrollerData.h"
#import "MLOGestureLimiter.h"
#import "MLOGestureEngine_Impl.h"
#import "MLOKeyboardManager.h"
#import "mlo.h"
#import "mlo_utils.h"
#include <touch/touch.h>

static const CGFloat
    DOUBLE_TAP_ZOOM_IN=300.0f,
    DOUBLE_TAP_ZOOM_OUT=100.0f,
    DOUBLE_TAP_ZOOM_DURATION=0.7f,
    DOUBLE_TAP_FPS_RATIO=0.5f,
    DOUBLE_TAP_FPS=50.0f;

static const BOOL ENABLE_LO_EVENTS_DURING_PINCH = NO;

@interface  MLOGestureEngine  ()
@property BOOL listening;
@property BOOL doubleTapped;
@property NSInteger handledGesturesCount;
@property CGPoint pinchCenter;
@property CGPoint panPrevious;
@property CGPoint dragStart;
@property NSArray * gestureRecognizers;
@property UIWindow * window;
@end

@implementation MLOGestureEngine

-(id)initWithMainViewController:(MLOMainViewController *) mainViewController{
    self = [self init];
    if(self){
        self.mainViewController = mainViewController;
        self.renderer = [MLORenderManager getInstance];
        self.selection = mainViewController.selection;
        self.scroller = mainViewController.scroller;
        self.flick = [[MLOGestureFlick alloc] initWithEngine:self];
        self.dragStart= self.panPrevious = self.pinchCenter = CGPointMake(0,0);
        self.limiter = [[MLOGestureLimiter alloc] initWithGestureEngine:self];
        self.window = nil;
        self.focused =NO;
        self.listening = NO;
        [self reset];
    }
    
    return self;
}

-(void) createListeners:(UIWindow *) window{
    
    UITapGestureRecognizer * tap = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(tap:)];
    UITapGestureRecognizer * doubleTap = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(doubleTap:)];
    doubleTap.numberOfTapsRequired =2;
    doubleTap.numberOfTouchesRequired=1;
    [tap requireGestureRecognizerToFail:doubleTap];
    
    self.gestureRecognizers = @[tap,
                                doubleTap,
                                [[UIPanGestureRecognizer alloc] initWithTarget:self action:@selector(pan:)],
                                [[UILongPressGestureRecognizer alloc] initWithTarget:self action:@selector(drag:)],
                                [[UIPinchGestureRecognizer alloc] initWithTarget:self action:@selector(pinch:)]];
    
 
}

-(void)gestureRecognizersEnable:(BOOL) enable{
    for(UIGestureRecognizer * recognizer in self.gestureRecognizers){
        if(enable){
            [self.window addGestureRecognizer:recognizer];
        }else{
            [self.window removeGestureRecognizer:recognizer];
        }
    }
}

-(void)showLibreOffice:(UIWindow *)window{
    
    self.window = window;
    
    if(!_listening) {
    
        [self createListeners:window];
        _listening = YES;
    }
    
    [self gestureRecognizersEnable:YES];
    
    [_renderer showLibreOffice:self];
    _focused = YES;
    [self reset];
    [self.limiter showLibreOffice];
}

-(void)hideLibreOffice{
    
    [self onGestureInterrupt];
    
    [_renderer hideLibreOffice];
    
    [_selection reset];
    
    [self gestureRecognizersEnable:NO];
    
    self.window = nil;

     _focused = NO;
}



-(void)drag:(UILongPressGestureRecognizer *) gesture{
    
    if(_focused){
        
        CGPoint point = [gesture locationInView:_renderer.view];
        
        if([_mainViewController.canvas pointInside:point withEvent:nil]){
            
            [_selection hide];
            
            UIGestureRecognizerState state = gesture.state;
            
            NSLog(@"MLO gesture engine long press state %d in point: (%f,%f)",state ,point.x,point.y);
            
            if(state == UIGestureRecognizerStateBegan) {

                self.dragStart = point;

                touch_lo_mouse_drag(point.x, point.y, DOWN);
                
            } else if(state == UIGestureRecognizerStateChanged) {
            
                touch_lo_mouse_drag(point.x, point.y, MOVE);

                [_selection showBetweenFinger:point andPreviousPoint:self.dragStart showMenu:NO];
                
            } else if(state == UIGestureRecognizerStateEnded) {
                
                touch_lo_mouse_drag(point.x, point.y, UP);

                [_selection showBetweenFinger:point andPreviousPoint:self.dragStart showMenu:YES];
            }
            [self evalGestureCount: state];
        }
    }
}

- (void)tap:(UITapGestureRecognizer *)gesture
{
    [self tapGeneric:gesture name:@"tap" requiresTappable:YES action:^(CGPoint location){
   
        touch_lo_tap(location.x, location.y);
        
        [self.mainViewController.keyboard show];
        
    }];
   
}

-(void) doubleTap:(UITapGestureRecognizer *)gesture{
    [self tapGeneric:gesture name:@"double tap" requiresTappable:NO action:^(CGPoint location){
        
        __block CGFloat initialZoom = [_limiter zoom];
        
        CGFloat targetZoom = _doubleTapped ? DOUBLE_TAP_ZOOM_OUT : DOUBLE_TAP_ZOOM_IN;
        CGFloat deltaZoom = targetZoom - initialZoom;
        
        if(LOG_DOUBLE_TAP){
            NSLog(@"DoubleTap currentZoom=%f targetZoom=%f",initialZoom,targetZoom);
        }
        
        [self pinch:location scale:1.0f state:UIGestureRecognizerStateBegan];
        
        MLOAnimation * animation=
        [[MLOAnimation alloc]
         initWithBehavior:  MANDATORY
            fractionType:   FULL_FRACTION
                animation:  ^(CGFloat fraction){
                    [self pinch:  location
                          scale:  (initialZoom + deltaZoom *fraction)/ initialZoom
                          state:  UIGestureRecognizerStateChanged];
        }];
        
        animation.fps = DOUBLE_TAP_FPS;
        animation.duration = DOUBLE_TAP_ZOOM_DURATION;
        [animation linearCurve];
        animation.endBlock = ^(void){
            
            [self pinch:location
                  scale:targetZoom / initialZoom
                  state:UIGestureRecognizerStateEnded];

        };
        [_currentAnimation cancel];
        self.currentAnimation = animation;
        
        [animation animate];
        
        _doubleTapped^=YES;
        
    }];
}


-(void)tapGeneric:(UITapGestureRecognizer *)    gesture
             name:(NSString *)                  name
 requiresTappable:(BOOL)                        isRequiresTappable
           action:(void (^)(CGPoint location))  doTap {
    
    if(_focused){
        
        UIGestureRecognizerState state = gesture.state;
        
        if (state == UIGestureRecognizerStateEnded) {
            
            [self onGestureInterrupt];
            
            CGPoint point = [gesture locationInView: _renderer.view];
            
            if([_mainViewController.canvas pointInside:point withEvent:nil]){
                
                if(!isRequiresTappable || [_mainViewController isTappable]){
                    
                    NSLog(@"%@: at: (%d,%d)",name, (int)point.x, (int)point.y);
                    doTap(point);
                    
                }else{
                    NSLog(@"%@ gesture aborted - main view controller is not tappable",name);
                }
            }
        } else{
            NSLog(@"%@ Gesture: %@", name, gesture);
        }
        
        [self evalGestureCount:state];
    }
}


-(void)onScroll:(UIGestureRecognizerState) state deltaY:(CGFloat) deltaY{
    if((state ==UIGestureRecognizerStateEnded)||
       (state == UIGestureRecognizerStateBegan)){
        [_scroller updateByLogic];
    }else{
        [_scroller updateByPixelDeltaY:deltaY];
    }
    [self evalGestureCount: state];

}


- (void)pan:(UIPanGestureRecognizer *)gesture{
    if(_focused){

        CGPoint location = [gesture locationInView:_renderer.view];

        UIGestureRecognizerState state = gesture.state;

        if(![_selection handleMoveAtPoint:location gestureState:state] &&
           [_mainViewController.canvas pointInside:location withEvent:nil]){

            if(state != UIGestureRecognizerStateBegan){
                
                NSInteger deltaX = location.x - self.panPrevious.x;
                NSInteger deltaY = location.y - self.panPrevious.y;
                
                [self loPanDeltaX:deltaX deltaY:deltaY state:state];
            }
            
            [self evalGestureCount:state];
            
            self.panPrevious = location;

            if(![_flick eval:gesture] &&
               (state == UIGestureRecognizerStateEnded)){
                
                [self endPan];
            }
            
        }
    }
}

-(void)endFlick{
    [self endPan];
    [self onScroll:UIGestureRecognizerStateEnded deltaY:0.0f];
}

-(void) endPan{
    NSLog(@"PAN ENDED");
    [_renderer endGestures];
}

-(BOOL) loPanDeltaX:(NSInteger) rawDeltaX deltaY:(NSInteger) rawDeltaY state:(UIGestureRecognizerState) state{
    
    CGFloat limitedDeltaX = [_limiter limitDelta:rawDeltaX direction:DELTA_X];
    CGFloat limitedDeltaY = [_limiter limitDelta:rawDeltaY direction:DELTA_Y];
    
    if((limitedDeltaX!=0.0f)
       || (limitedDeltaY!=0.0f)){
       
        if(LOG_PAN){
            [_limiter logPanRawDeltaX:rawDeltaX
                            rawDeltaY:rawDeltaY
                        limitedDeltaX:limitedDeltaX
                        limitedDeltaY:limitedDeltaY];
        }
        
        [_mainViewController.renderManager panDeltaX:(CGFloat)limitedDeltaX
                                              deltaY:(CGFloat)limitedDeltaY];
        
        
        [self onScroll:state deltaY: limitedDeltaY];
        
        touch_lo_pan(limitedDeltaX, limitedDeltaY);
        
        return YES;
   }
   return NO;
}

- (void)pinch:(UIPinchGestureRecognizer *)gesture
{
    if(_focused){
        
        if(gesture.numberOfTouches>1){
            self.pinchCenter = [gesture locationInView:_renderer.view];
        }
                
        [self pinch:self.pinchCenter scale:[gesture scale] state:gesture.state];
        
    }
}

-(void)pinch:(CGPoint) location scale:(CGFloat) scale state:(UIGestureRecognizerState) state{
    
    [_selection hide];
    
    if(state ==UIGestureRecognizerStateBegan){
        self.panPrevious = location;
        [_limiter beginPinch];
    }else{
        
        CGFloat scaleRatioToLastScale  = [_limiter inPinchGetRatioToLastScale:scale];
        CGFloat deltaX = (location.x - self.panPrevious.x)*scaleRatioToLastScale;
        CGFloat deltaY = (location.y - self.panPrevious.y)*scaleRatioToLastScale;
        
        deltaX = [_limiter limitDelta:deltaX direction:DELTA_X];
        deltaY = [_limiter limitDelta:deltaY direction:DELTA_Y];
        
        self.panPrevious = location;
        
        if(ENABLE_PINCH_RENDERING_VIA_IOS){
            
            [_renderer pinchDeltaX:deltaX
                            deltaY:deltaY
                             scale:[_limiter currentPinchScale]];
        
            if(ENABLE_LO_EVENTS_DURING_PINCH){
                [_limiter fireLoZoomEventsDuringPinch];
            }
            
            if(state==UIGestureRecognizerStateEnded){
                
                CGPoint shift = [_renderer getShiftFromCanvasCenter];
                
                [_renderer endGestures];
                
                [self loZoomDeltaX:shift.x
                            deltaY:shift.y
                             scale:[_limiter endPinchAndGetScaleForLo:scale]];
                
            }

        }else{
            [self loZoomDeltaX:deltaX deltaY:deltaY scale:scaleRatioToLastScale];
        }
    }
    
    [self onScroll:state deltaY:0];
}
-(void)loZoomDeltaX:(CGFloat)deltaX deltaY:(CGFloat)deltaY scale:(CGFloat) scale{
    NSInteger xInt = deltaX;
    NSInteger yInt = deltaY;
    
    if(LOG_PINCH){
        NSLog(@"touch_lo_zoom: (%d,%d) scale=%f",xInt,yInt, scale);
    }
    
    touch_lo_zoom(xInt, yInt, (float)scale);
}



-(void)onGestureInterrupt{
    [_selection hide];
    [_currentAnimation cancel];
 
}

-(void)evalGestureCount:(UIGestureRecognizerState) state{
    if(state == UIGestureRecognizerStateBegan){
        
        _handledGesturesCount++;
        
    }else if(state == UIGestureRecognizerStateCancelled ||
             state == UIGestureRecognizerStateEnded ||
             state == UIGestureRecognizerStateFailed){
        
        _handledGesturesCount--;
        
        if(_handledGesturesCount==0){
            [_scroller fadeOut];
        } else if(_handledGesturesCount < 0){
            self.handledGesturesCount = 0;
        }
    }
}

- (void)onKeyboardShow{
    if(_focused){
        
        [_selection hide];
    }
}

- (void)onKeyboardHide{
    
}

-(void)reset{
    self.doubleTapped = NO;
    self.handledGesturesCount = 0;
}

-(void)onRotate{
    self.handledGesturesCount=0;
    [_limiter onRotate];
}

@end


