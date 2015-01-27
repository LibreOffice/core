// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOCommon.h"

#import <UIKit/UIKit.h>

#import "MLOMainViewController.h"
#import "MLORenderManager_Impl.h"
#import "MLORenderBuffer.h"
#import "MLOScalingBuffer.h"
#import "MLOGestureLimiter.h"
#import "MLOGestureEngine_Impl.h"
#import "MLOSelectionViewController.h"
#import "NSObject+MLOUtils.h"
#import "MLOPostRenderManager.h"
#import "mlo_utils.h"
#import <QuartzCore/QuartzCore.h>

#include <touch/touch.h>

static MLORenderManager * instance = nil;

static const CGFloat
    HORIZONAL_BUFFER_SCALE=1.0f,
    VERTICAL_BUFFER_SCALE=1.0f,
    BUFFER_SCALE_BIAS =1.0f,

    RENDERING_BIAS_RATIO = 1.0f/(BUFFER_SCALE_BIAS*HORIZONAL_BUFFER_SCALE *VERTICAL_BUFFER_SCALE),

    MAX_RENDER_PER_SECOND_ON_DEFAULT_ZOOM =4 * RENDERING_BIAS_RATIO,
    MAX_RENDER_PER_SECOND_ON_MAX_ZOOM_IN=3 * RENDERING_BIAS_RATIO;

static const NSTimeInterval    
    LO_PAN_RENDER_MAX_DURATION = 0.3f,
    LO_PINCH_RENDER_MAX_DURATION = 0.5f,
    RESET_TRANSFORM_ANIMATION_DURATION=0.1f,
    LO_RENDER_BACKOFF_MIN = 1.0f / MAX_RENDER_PER_SECOND_ON_DEFAULT_ZOOM,
    LO_RENDER_BACKOFF_MAX = 1.0f / MAX_RENDER_PER_SECOND_ON_MAX_ZOOM_IN,
    LO_RENDER_BACK_OFF_MAX_DELTA = LO_RENDER_BACKOFF_MAX - LO_RENDER_BACKOFF_MIN;



static const NSInteger BUFFER_COUNT=2;

#define MLOGestureDirectionString(enum) [@[@"X",@"Y",@"Z"] objectAtIndex:enum]

@interface MLORenderManager ()
@property MLOGestureEngine * gestureEngine;
@property MLOPostRenderManager * post;
@property NSArray * buffers;
@property NSInteger activeBufferIndex,nextBufferIndex,frameIdCounter;
@property NSTimeInterval bufferTransfromResetDeadline,renderBlockReleaseTime;
@property CGFloat inRenderTiltX,inRenderTiltY, inRenderTiltScale;
@end

@implementation MLORenderManager 

+(MLORenderManager *) getInstance{
    if(instance == nil){
        instance = [MLORenderManager new];
    }
    return instance;
}

-(MLORenderBuffer *)getBufferAtIndex:(NSInteger) index{
    return [_buffers objectAtIndex:index];
}

-(id)init{
    self = [super init];
    if(self){
        self.gestureEngine = nil;
        [self createBufffers];
        self.renderBlockReleaseTime = 0;
        self.activeBufferIndex = 0;
        self.nextBufferIndex = _activeBufferIndex + 1;
        [self loRenderWillBegin];
        self.view.backgroundColor = [UIColor whiteColor];
        self.currentGesture = NO_GESTURE;
    }
    return self;
}

-(void) createBufffers{
    NSMutableArray * array = [NSMutableArray new];
    
    for (NSInteger i = 0 ; i < BUFFER_COUNT; i++) {
        
        [array addObject:[[MLORenderBuffer alloc] initWithArrayIndex: i renderManager:self]];
    }
    
    self.buffers = [NSArray arrayWithArray:array];
    
    for (NSInteger i = 0 ; i < BUFFER_COUNT; i++) {
        
        MLORenderBuffer * buffer = [_buffers objectAtIndex:i];
        
        NSInteger previousIndex = BUFFER_COUNT -1;
        if(i !=0){
            previousIndex = i-1;
        }
        buffer.previous = [_buffers objectAtIndex:previousIndex];
        [self.view addSubview:buffer];
    }
}

-(void)showLibreOffice:(MLOGestureEngine *) gestureEngine{
    self.gestureEngine = gestureEngine;
}

-(void)hideLibreOffice{
    self.currentGesture = NO_GESTURE;
    [self.scaler hide];
    self.gestureEngine = nil;
}

-(void) panDeltaX:(CGFloat) deltaX deltaY:(CGFloat) deltaY{
    
    self.currentGesture = PAN;
        
    if(deltaX || deltaY){
        [[self getActiveBuffer] moveDeltaX:deltaX deltaY:deltaY];
       
        self.inRenderTiltX+=deltaX;
        self.inRenderTiltY+=deltaY;
    }
}

-(void) pinchDeltaX:(CGFloat)deltaX deltaY:(CGFloat)deltaY scale:(CGFloat)scale{
    
    if(ENABLE_PINCH_RENDERING_VIA_IOS){
               
        self.currentGesture = PINCH;
        
        if(self.scaler ==nil){
            self.scaler = [[MLOScalingBuffer alloc] initWithRenderManager:self];
        }
        
        [self.scaler scale:scale deltaX:deltaX deltaY:deltaY];
    
        self.inRenderTiltScale*=scale;
        self.inRenderTiltX = self.inRenderTiltX*scale +deltaX;
        self.inRenderTiltY = self.inRenderTiltY*scale +deltaY;
        
    }

}
-(void)endGestures{
    self.currentGesture = NO_GESTURE;
    NSLog(@"RenderManager: self.currentGesture = NO_GESTURE");
}

-(CGPoint) getShiftFromCanvasCenter{
    
    CGPoint bufferCenter= [self currentBufferCenter];
    CGPoint canvasCenter = _gestureEngine.mainViewController.canvas.center;
    
    return CGPointMake(bufferCenter.x - canvasCenter.x,
                       bufferCenter.y - canvasCenter.y);
}

-(CGPoint) currentBufferCenter{
    if(self.currentGesture ==PINCH){
        return self.scaler.center;
    }
    return [self getActiveBuffer].center;
}

-(void)loRenderWillBegin{
    self.inRenderTiltX = NO_MOVE_DELTA;
    self.inRenderTiltY = NO_MOVE_DELTA;
    self.inRenderTiltScale = NO_SCALE;
}

-(void)setWidth:(NSInteger) width height:(NSInteger) height{
        
    self.view.frame = CGRectMake(0,0, width,height);
    
    CGFloat bufferWidth = width*HORIZONAL_BUFFER_SCALE;
    CGFloat bufferHeight = height*VERTICAL_BUFFER_SCALE;
    self.bufferFrame = CGRectMake(0,0, bufferWidth,bufferHeight);
    
    for (MLORenderBuffer * buffer in _buffers) {
        buffer.frame = self.bufferFrame;
    }
    
    touch_lo_set_view_size(bufferWidth,bufferHeight);
}
-(MLORenderBuffer *) getActiveBuffer{
    return[_buffers objectAtIndex:self.activeBufferIndex];
}

-(MLORenderBuffer *) getNextBuffer{
    return[_buffers objectAtIndex:self.nextBufferIndex];
}

-(void) renderInContext:(CGContextRef) context{
    // used for magnification
     [[self getActiveBuffer].layer renderInContext:context];
}

-(void)swapIndexes{
    self.nextBufferIndex = (self.nextBufferIndex +1)% BUFFER_COUNT;
    self.activeBufferIndex = (self.activeBufferIndex +1)% BUFFER_COUNT;
}

-(void)swapPreviousBuffer:(MLORenderingUIView*) previous withNextBuffer:(MLORenderBuffer *) next{
    
    NSTimeInterval bufferTransformResetDelay = [self getBufferTransformResetDelay];
    NSTimeInterval bufferTransformResetDeadline =CACurrentMediaTime() + bufferTransformResetDelay;
    
    self.bufferTransfromResetDeadline = bufferTransformResetDeadline;
    
    if(self.scaler && self.scaler.didRender){
        previous = self.scaler;
    }
     
    [self showBuffer:next];
    
    [self swapIndexes];
   
    [previous hide];
    
}

-(NSTimeInterval) getBufferTransformResetDelay{
   
    switch(self.currentGesture){
        case PAN: return LO_PAN_RENDER_MAX_DURATION;
        case PINCH: return LO_PINCH_RENDER_MAX_DURATION;
        case NO_GESTURE: return 0;
    }
}

-(void)showBuffer:(MLORenderBuffer *) buffer{

    buffer.alpha = 1.0f;
}

-(CGFloat) currentZoomRatio{
    return [_gestureEngine.limiter zoom] / MAX_ZOOM;
}

-(void) renderWithRect:(CGRect) rect{

    if(ENABLE_LO_DESKTOP){
    
        switch(self.currentGesture){
            case PAN:
                {
                    NSTimeInterval now = CACurrentMediaTime();
                    
                    NSTimeInterval delta =  LO_RENDER_BACKOFF_MIN +
                                            LO_RENDER_BACK_OFF_MAX_DELTA * [self currentZoomRatio];
                    
                    NSTimeInterval releaseTime = now + delta;
                    
                    NSTimeInterval currentReleaseTime = self.renderBlockReleaseTime;
                    
                    NSInteger currentFrameId = self.frameIdCounter++;
                    
                    if(now > currentReleaseTime){
                   
                        [self pereodicRender:rect releaseTime:releaseTime];
                   
                    }else{
                        
                        [self performBlock:^{
                            
                            if((self.renderBlockReleaseTime == currentReleaseTime)
                               && (currentFrameId==0)){
                               
                                [self pereodicRender:rect releaseTime:releaseTime];
                            }
                        }afterDelay:delta];
                    }
                }
                break;
            case PINCH:
            case NO_GESTURE:
                [[self getNextBuffer] setNeedsDisplayInRect:rect];
            break;
        }
    }
 }

-(void)renderNow{
    [[self getNextBuffer] setNeedsDisplayInRect:self.bufferFrame];
}

-(void)pereodicRender:(CGRect) rect releaseTime:(NSTimeInterval) releaseTime{
    
    static NSTimeInterval lastRender = 0;
    
    [[self getNextBuffer] setNeedsDisplayInRect:rect];
    
    self.frameIdCounter = 0;
        
    self.renderBlockReleaseTime =releaseTime;
    
    NSTimeInterval now = CACurrentMediaTime();
    NSLog(@"Render interval %f",now - lastRender);
    
    lastRender = now;
}


// C functions
// ===========
//
// Functions called in the LO thread, which thus need to dispatch any
// CocoaTouch activity to happen on the GUI thread. Use
// dispatch_async() consistently.


void touch_ui_damaged(int minX, int minY, int width, int height)
{
    CGRect rect = CGRectMake(minX, minY, width, height);

    dispatch_async(dispatch_get_main_queue(), ^{
        
        [[MLORenderManager getInstance] renderWithRect:rect];
        
    });
    // NSLog(@"lo_damaged: %dx%d@(%d,%d)", (int)rect.size.width, (int)rect.size.height, (int)rect.origin.x, (int)rect.origin.y);
}

@end

// vim:set shiftwidth=4 softtabstop=4 expandtab:
