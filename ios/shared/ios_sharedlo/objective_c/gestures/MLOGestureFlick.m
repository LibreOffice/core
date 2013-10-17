// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOGestureFlick.h"
#import "MLOGestureEngine_Impl.h"
#import "MLOMainViewController.h"
#import "MLOScrollerViewController.h"
#import "MLORenderManager.h"
#import "MLOAnimation.h"
#import "mlo_utils.h"

@interface  MLOGestureFlick  ()
@property MLOGestureEngine * engine;
@property CFTimeInterval startTime;
@property CGPoint startPoint;
@property CGFloat xFlickSpeed,yFlickSpeed;
@property BOOL isEvaluatingFlic;
@end

static const CGFloat
    MAX_SPEED = MLO_IPAD_HEIGHT_IN_PIXELS/0.5f, // max speed is the height per half a second
    MAX_DURATION = 5.0f, // 5 seconds flic top
    DECCELERATION = 800.0f,
    SPEED_IGNORE_LIMIT=2.0f,
    FLICK_FPS=50.0f;

static const NSTimeInterval
    MAX_INTERVAL_BETWEEN_EVENTS_THRESHOLD=0.7f; // 70% a second

@implementation MLOGestureFlick

-(CGFloat) normalizeSpeed:(CGFloat) gestureSpeed currentSpeed:(CGFloat) currentSpeed{
    NSLog(@"GestureSpeed=%f currentSpeed=%f",gestureSpeed,currentSpeed);
    if(currentSpeed* gestureSpeed > 0){
        gestureSpeed+=currentSpeed;
    }
    if(gestureSpeed > MAX_SPEED){
        return  MAX_SPEED;
    }
    if (gestureSpeed < -MAX_SPEED){
        return -MAX_SPEED;
    }
    if (abs(gestureSpeed) < SPEED_IGNORE_LIMIT) {
        return 0;
    }
    return gestureSpeed;
    
}
-(id)initWithEngine:(MLOGestureEngine *) engine{
    
    self = [self init];
    
    if(self){
        self.engine = engine;
        _startTime = -1;
        _yFlickSpeed= _xFlickSpeed=0.0f;
        _isEvaluatingFlic =NO;
    }
    return self;
}

-(BOOL)eval:(UIGestureRecognizer *) gesture{
    
    BOOL returned = NO;
    
    UIGestureRecognizerState state = gesture.state;
    
    if(state == UIGestureRecognizerStateBegan){
        self.isEvaluatingFlic = YES;
        
        self.startTime = CACurrentMediaTime();
        self.startPoint = [gesture locationInView:_engine.renderer.view];
        
    }else if(state == UIGestureRecognizerStateFailed
             || state == UIGestureRecognizerStateCancelled){
        self.startTime = -1;
        self.isEvaluatingFlic =NO;
        
    }else{
        
        if(state == UIGestureRecognizerStateEnded){
            
            CFTimeInterval startTime = self.startTime;
            
            CGFloat xSpeed = self.xFlickSpeed;
            CGFloat ySpeed = self.yFlickSpeed;
            
            [_engine.currentAnimation cancel];
    
            if(startTime > 0){
        
                CGPoint t0 =self.startPoint;
                
                CFTimeInterval deltaT = CACurrentMediaTime() - startTime;
                
                self.startTime =-1;
                
                if( (deltaT < MAX_INTERVAL_BETWEEN_EVENTS_THRESHOLD) &&
                    (deltaT > 0) &&
                    (_engine.handledGesturesCount == 0)){
                    
                    CGPoint t1 = [gesture locationInView:_engine.renderer.view];
              
                    
                    returned = [self flicXSpeed:   [self normalizeSpeed:   (t1.x - t0.x) / deltaT
                                                currentSpeed:   xSpeed]
                              ySpeed:   [self normalizeSpeed:   (t1.y - t0.y) / deltaT
                                                currentSpeed:   ySpeed]];
                }
            }
            
            self.isEvaluatingFlic =NO;
        }
    }
    
    return returned;
    
}
                 
-(BOOL)flicXSpeed:(CGFloat) xSpeedInitial ySpeed:(CGFloat) ySpeedInitial{
     
     if((xSpeedInitial !=0) || (ySpeedInitial!=0)) {
         
         NSLog(@"FLIC: xSpeed=%f ySpeed=%f",xSpeedInitial, ySpeedInitial);
         
         __block CGFloat xDistanceTotal = 0,yDistanceTotal=0;
         
         const CGFloat maxSpeedVector = max( abs(xSpeedInitial) , abs(ySpeedInitial) );
         const CGFloat duration =min(   maxSpeedVector  /  DECCELERATION   ,   MAX_DURATION   );
         const CGFloat deccelerationRate = maxSpeedVector / duration;
         const CGFloat xSign= xSpeedInitial<0? -1: 1;
         const CGFloat ySign= ySpeedInitial<0? -1: 1;

         MLOScrollerViewController * scroller = _engine.scroller;
         
         __block MLOAnimation * copy = nil;
         
         MLOAnimation * animation =
         [[MLOAnimation alloc]
          initWithBehavior: CANCELABLE
          fractionType: FULL_FRACTION
          animation: ^(CGFloat fraction){
             
              CGFloat time = fraction * duration;
              CGFloat reducedSpeed = deccelerationRate* time;
              CGFloat decelerationDistance = reducedSpeed *time / 2.0f;
              
              CGFloat xDistanceCurrent = xSpeedInitial *time;
              CGFloat yDistanceCurrent = ySpeedInitial *time;
              CGFloat xSpeedCurrent = 0;
              CGFloat ySpeedCurrent = 0;
             
              if(xSign*xDistanceCurrent > decelerationDistance){
                  xDistanceCurrent -= (xSign * decelerationDistance);
                  xSpeedCurrent = xSpeedInitial - xSign * reducedSpeed;
              }else{
                  xDistanceCurrent = xDistanceTotal;
              }
             
              if(ySign*yDistanceCurrent > decelerationDistance){
                  yDistanceCurrent -= (ySign * decelerationDistance);
                  ySpeedCurrent = ySpeedInitial - ySign * reducedSpeed;
                  
              }else{
                  yDistanceCurrent = yDistanceTotal;
              }
              
              [self setFlicSpeedX:xSpeedCurrent y:ySpeedCurrent context:@"FLIC iteration"];

              CGFloat deltaX = xDistanceCurrent - xDistanceTotal;
              CGFloat deltaY = yDistanceCurrent - yDistanceTotal;
             
              if([_engine loPanDeltaX:deltaX deltaY:deltaY state:UIGestureRecognizerStateChanged]){
                              
                  [scroller updateByPixelDeltaY:deltaY];
                 
                  xDistanceTotal = xDistanceCurrent;
                  yDistanceTotal = yDistanceCurrent;
              }else{
                  [copy cancel];
              }
         } ];
         
         animation.duration = duration;
         animation.fps = FLICK_FPS;
         [animation linearCurve];
         
         copy = animation;
         
         animation.endBlock = ^{
             if(![copy isCancelled] || !self.isEvaluatingFlic){
         
                 [self setFlicSpeedX:0.0f y:0.0f context:@"FLIC end"];
                 [_engine endFlick];
             }
         };
         [_engine.currentAnimation cancel];
         _engine.currentAnimation = animation;
         
         [animation animate];
    
         return YES;
     }
    
    NSLog(@"FLIC ignored: xSpeed = ySpeed = 0");
    
    return NO;
 }

-(void) setFlicSpeedX: (CGFloat) xSpeed y:(CGFloat) ySpeed context:(NSString *)context{
    self.xFlickSpeed = xSpeed;
    self.yFlickSpeed = ySpeed;
    if(LOG_FLICK_FRAMES){
        NSLog(@"FLIC (%@) : set xSpeed = %f, ySpeed = %f",context, xSpeed,ySpeed);
    }
}


@end
