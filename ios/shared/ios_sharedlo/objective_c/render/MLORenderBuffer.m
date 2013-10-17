// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLORenderBuffer.h"
#import "MLORenderManager_Impl.h"
#import "mlo_utils.h"
#import <QuartzCore/QuartzCore.h>
#include <touch/touch.h>

@interface  MLORenderBuffer ()
@property MLORenderManager * manager;
@end
static const CGFloat
    MIN_AVERAGE_RENDER_TIME_THRESHOLD=1.0f/100.0f,
    MIN_FPS=10.0f;

static const NSTimeInterval SCALING_ANIMATION_DURATION = 0.02f;

static CGFloat averageFps,maxFps;
@implementation MLORenderBuffer

-(id)initWithArrayIndex:(NSInteger) index renderManager:(MLORenderManager *) manager{
    self= [self init];
    if(self){
        self.index = index;
        self.manager = manager;
        self.backgroundColor = [UIColor whiteColor];
        self.previous = nil;
    }
    return self;
}

-(void)hide{
    self.alpha = 0;
    [self resetTransform];
}

-(void)setNeedsDisplayInRect:(CGRect)rect{
    [self resetTransform];
    [super setNeedsDisplayInRect:rect];
}

-(void) resetTransform{
    if(self.frame.origin.x ||self.frame.origin.y ){
        self.frame = CGRectMake(0,0,_manager.bufferFrame.size.width,_manager.bufferFrame.size.height);
    }
}

- (void)drawRect:(CGRect)rect
{
    if(ENABLE_LO_DESKTOP){
        CGContextRef context = UIGraphicsGetCurrentContext();
       
        //rect = self.frame;
        LOG_RECT(rect, @"drawRect");
        
        CGContextSaveGState(context);
        CGContextSetFillColorWithColor(context,[UIColor whiteColor].CGColor);
        CGContextTranslateCTM(context, 0, _manager.bufferFrame.size.height);
        CGContextScaleCTM(context, 1, -1);
        CGContextScaleCTM(context, 1, 1);
        NSDate *startDate = [NSDate date];
        
        [_manager loRenderWillBegin];
        
        touch_lo_render_windows(context, rect.origin.y, rect.origin.y, rect.size.width, rect.size.height);

        CGContextRestoreGState(context);
        
        CGFloat duration =  [[NSDate date] timeIntervalSinceDate: startDate];
        
        maxFps = max(maxFps,1.0f/duration);
        
        static float totalTime = 0,counter = 0;
        
        totalTime +=duration;
        counter++;
        
        CGFloat averageTime = totalTime / counter;
        if(averageTime >MIN_AVERAGE_RENDER_TIME_THRESHOLD){
            averageFps = 1.0f/ averageTime;
        }
        
        if(LOG_DRAW_RECT){
            NSLog(@"drawRect: lo_render_windows: time=%f sec, average=%f sec, fps=%f",
                  duration, averageTime, averageFps);
        }
        if(_manager.currentGesture != PINCH){
            [_manager swapPreviousBuffer:_previous withNextBuffer:self];
        }
    }
}

+(CGFloat) getAverageFps{
    return max(averageFps, MIN_FPS);
}

+(CGFloat) getMaxFps{
    return max(maxFps, MIN_FPS);
}

@end
