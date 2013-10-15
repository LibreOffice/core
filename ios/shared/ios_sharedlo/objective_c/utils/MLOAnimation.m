// -*- Mode: Objective-C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOAnimation.h"


typedef CGFloat (^MLOAnimationCurve)(CGFloat time);

@interface MLOAnimation ()
@property BOOL active,didPost,cancelled;
@property CGFloat frameCount;
@property NSDate * startDate;
@property MLOAnimationBehavior behavior;
@property MLOAnimationFractionType fractionType;
@property (nonatomic,strong) MLOAnimationBlock animation;
@property (nonatomic,strong) MLOAnimationCurve curve;
@end

@implementation MLOAnimation

-(void) linearCurve{
    static const MLOAnimationCurve LINEAR = ^(CGFloat completedFraction){
        return completedFraction;
    };
    [self setCurve:LINEAR name:@"LINEAR"];
}
-(void) easeOutCurve{
    static const MLOAnimationCurve EASE_OUT  = ^(CGFloat completedFraction){
        return 1 - completedFraction*completedFraction;
    };
    [self setCurve:EASE_OUT name:@"EASE_OUT"];
}

-(void) easeInCurve{
    static const MLOAnimationCurve EASE_IN  = ^(CGFloat completedFraction){
        return completedFraction*completedFraction;
    };
    
    [self setCurve:EASE_IN name:@"EASE_IN"];
}

-(void)setCurve:(MLOAnimationCurve) curve name:(NSString *) name{
    self.curve = curve;
    NSLog(@"MLOAnimation curve set to: %@",name);
}

-(void) cancel{
    if(_behavior == CANCELABLE){
        self.cancelled= YES;
        self.active = NO;
        [self doPost:nil];
        
        if(_startDate){
        
            NSLog(@"MLOAnimation cancelled after %f millis",[_startDate timeIntervalSinceNow]);
        }else{
            NSLog(@"MLOAnimation aborted");
        }
    }else{
        NSLog(@"CLOAnimation cannot be cancelled");
    }
}
-(BOOL) isCancelled{
    return self.cancelled;
}

-(id)initWithBehavior:(BOOL) behavior fractionType:(MLOAnimationFractionType) fractionType animation:(MLOAnimationBlock) animation{
    self = [super init];
    if(self){
        _active=YES;
        _duration = DEFAULT_MLO_ANIMATION_DURAION;
        _fps = DEFAULT_ANIMATION_FPS;
        _behavior=CANCELABLE;
        _fractionType = fractionType;
        _frameCount = -1.0f;
        _startDate = nil;
        _didPost =NO;
        _cancelled=NO;
        [self linearCurve];
        self.animation = animation;
    }
    return self;
}

-(void)animate{

    if(!_startDate){
        self.startDate = [NSDate date];
        _frameCount = _duration *_fps;
        
        if(_frameCount>0){
            CGFloat frameDuration = 1.0f/_fps;
            
            NSLog(@"MLOAnimation: duration=%f frameCount=%f fps=%f frameDuration=%f fractionType=%@",_duration,_frameCount,_fps,frameDuration,[self fractionTypeAsString]);
            
            for (CGFloat i = 1; i <= _frameCount; i++) {
                [self performSelector:@selector(doFrame:) withObject:[NSNumber numberWithFloat:i] afterDelay: i*frameDuration];
            }
        [self performSelector:@selector(doPost:) withObject:nil afterDelay:_duration + frameDuration];
        }else{
            NSLog(@"MLOAnimation cannot run (zero frames)");
        }
    }
}

-(void) doPost:(NSObject *) niller{
    if(!_didPost){
        _didPost = YES;
        if(_endBlock){
            _endBlock();
        }
    }
}

-(NSString *)fractionTypeAsString{
    switch (_fractionType) {
        case DELTA_ONLY:
            return @"DELTA_ONLY";
        case FULL_FRACTION:
            return @"FULL_FRACTION";
    }
}

-(void)doFrame:(NSNumber *) frame{
    if(_active){
        
        CGFloat fFrame = [frame floatValue];
        
        CGFloat currentFraction = _curve(fFrame/_frameCount);
        
        if(_fractionType == DELTA_ONLY){
            currentFraction -= _curve( (fFrame-1.0f) /_frameCount);
        }
        
        _animation(_curve(currentFraction));
        
        if(fFrame ==_frameCount){
        
            _behavior =MANDATORY;
        }
    }
}

@end
