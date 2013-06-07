// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


#import "CommandInterpreter.h"
#import "SlideShow.h"
#import "Base64.h"
#import "CommunicationManager.h"

@interface CommandInterpreter()

@end

@implementation CommandInterpreter

@synthesize mSlideShow = _mSlideShow;

- (BOOL) isSlideRunning {
    return [self.mSlideShow size] > 0;
}

// Received a set of instructions from server.
- (void) parse:(NSArray*)command{
    if ([command count] == 0) {
        return;
    }
    NSString *instruction = [command objectAtIndex:0];
    
    if([instruction isEqualToString:@"slideshow_started"]){
        unsigned int slideLength = [[command objectAtIndex:1] unsignedIntValue];
        unsigned int currentSlide = [[command objectAtIndex:2] unsignedIntValue];
        
        [self.mSlideShow setLength:slideLength];
        [self.mSlideShow setCurrentSlide:currentSlide];
        
        [[NSNotificationCenter defaultCenter] postNotificationName:STATUS_CONNECTED_NOSLIDESHOW
                          object:nil];
        [[NSNotificationCenter defaultCenter] postNotificationName:MSG_SLIDE_CHANGED object:self.mSlideShow.mCurrentSlide];
        
    } else if ([instruction isEqualToString:@"slideshow_finished"]){
        self.mSlideShow = [SlideShow init];
        [[NSNotificationCenter defaultCenter] postNotificationName:STATUS_CONNECTED_NOSLIDESHOW object:nil];
    } else {
        if (self.mSlideShow == nil)
            return;
        if ([instruction isEqualToString:@"slide_updated"]) {
            unsigned int newSlideNumber = [[command objectAtIndex:1] unsignedIntValue];
            [self.mSlideShow setCurrentSlide:newSlideNumber];
            
            [[NSNotificationCenter defaultCenter] postNotificationName:MSG_SLIDE_CHANGED object:nil];
            
        } else if ([instruction isEqualToString:@"slide_preview"]){
            unsigned int slideNumber = [[command objectAtIndex:1] unsignedIntValue];
            NSString * imageData = [command objectAtIndex:2];
            [Base64 initialize];
            NSData* data = [Base64 decode:imageData];
            UIImage* img = [UIImage imageWithData:data];
            [self.mSlideShow putImage:img
                              AtIndex:slideNumber];
            [[NSNotificationCenter defaultCenter] postNotificationName:MSG_SLIDE_PREVIEW object:[NSNumber numberWithUnsignedInt:slideNumber]];
        } else if ([instruction isEqualToString:@"slide_notes"]){
            unsigned int slideNumber = [[command objectAtIndex:1] unsignedIntValue];
            NSString *notes;
            for (int i = 2; i<command.count; ++i) {
                [notes stringByAppendingString:[command objectAtIndex:i]];
            }
            
            [self.mSlideShow putNotes:notes
                              AtIndex:slideNumber];
            [[NSNotificationCenter defaultCenter] postNotificationName:MSG_SLIDE_NOTES object: [NSNumber numberWithUnsignedInt:slideNumber]];
        }
        
    }
    
}

@end
