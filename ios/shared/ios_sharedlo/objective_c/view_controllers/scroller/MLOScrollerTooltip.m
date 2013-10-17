// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOCommon.h"
#import "MLOScrollerTooltip.h"
#import "MLOManager.h"
#import <QuartzCore/QuartzCore.h>

@interface MLOScrollerTooltip ()
@property NSInteger lastPage;
@end

static const CGFloat BORDER_WIDTH = 1.0f,VISIBLE_ALPHA=0.85f;

@implementation MLOScrollerTooltip

- (id)init{
    
    self = [super init];
    if(self){
    
        self.alpha= 0.0f;
        self.frame =CGRECT_ONE;
        self.backgroundColor = [UIColor whiteColor];
        
        self.layer.borderWidth = BORDER_WIDTH;
        self.layer.borderColor = [[UIColor grayColor] CGColor];
        
        self.extension=nil;
        _lastPage = -1;
        
        self.textAlignment = NSTextAlignmentCenter;
        self.textColor = [UIColor blackColor];
    }
    return self;
}

-(void) updateWithFrame:(CGRect) newFrame inPage:(NSInteger) page{
    
    BOOL isForceUpdateFrame =NO;
    
    if(_extension==nil){
        self.extension = [[[MLOManager getInstance] extension] uppercaseString];
        
        isForceUpdateFrame = YES;
    }
     
    if((self.frame.origin.y != newFrame.origin.y) || isForceUpdateFrame){
        
        self.alpha=VISIBLE_ALPHA;
        
        self.frame = newFrame;
        
        self.text = [NSString stringWithFormat:@"Page %d This %@ is editable",page,_extension];
        
        [self setNeedsDisplay];
        
        _lastPage = page;
    }
    
}

-(void) hide{
    self.frame = self.bounds=CGRECT_ONE;
    self.alpha = 0.0f;
}

-(void) hideLibreOffice{
    self.extension = nil;
}


@end
