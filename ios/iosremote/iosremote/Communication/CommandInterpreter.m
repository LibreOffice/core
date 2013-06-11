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

@synthesize slideShow = _slideShow;

- (SlideShow*) slideshow{
    if (!self.slideShow)
        self.slideShow = [[SlideShow alloc] init];
    return self.slideShow;
}

- (BOOL) isSlideRunning {
    return [self.slideShow size] > 0;
}

// Received a set of instructions from server.
- (void) parse:(NSArray*)command{
    if ([command count] == 0) {
        return;
    }
    NSString *instruction = [command objectAtIndex:0];
    
    if([instruction isEqualToString:@"slideshow_started"]){
        NSLog(@"Interpreter: slideshow_started");
        uint slideLength = [[command objectAtIndex:1] integerValue];
        uint currentSlide = [[command objectAtIndex:2] integerValue];
        self.slideShow = [[SlideShow alloc] init];
        
        [self.slideShow setSize:slideLength];
        [self.slideShow setCurrentSlide:currentSlide];
        
        [[NSNotificationCenter defaultCenter] postNotificationName:STATUS_CONNECTED_SLIDESHOW_RUNNING
                          object:nil];
        [[NSNotificationCenter defaultCenter] postNotificationName:MSG_SLIDE_CHANGED object:[NSNumber numberWithUnsignedInt:currentSlide]];
        
    } else if ([instruction isEqualToString:@"slideshow_finished"]){
        NSLog(@"Interpreter: slideshow_started");
        self.slideShow = [[SlideShow alloc] init];
        [[NSNotificationCenter defaultCenter] postNotificationName:STATUS_CONNECTED_NOSLIDESHOW object:nil];
    } else {
        if (self.slideShow == nil)
            return;
        if ([instruction isEqualToString:@"slide_updated"]) {
            NSLog(@"Interpreter: slide_updated");
            uint newSlideNumber = [[command objectAtIndex:1] integerValue];
            [self.slideShow setCurrentSlide:newSlideNumber];
            
            [[NSNotificationCenter defaultCenter] postNotificationName:MSG_SLIDE_CHANGED object:nil];
            
        } else if ([instruction isEqualToString:@"slide_preview"]){
            NSLog(@"Interpreter: slide_preview");
            uint slideNumber = [[command objectAtIndex:1] integerValue];
            NSString * imageData = [command objectAtIndex:2];
            
            [self.slideShow putImage:imageData
                              AtIndex:slideNumber];
            [[NSNotificationCenter defaultCenter] postNotificationName:MSG_SLIDE_PREVIEW object:[NSNumber numberWithUnsignedInt:slideNumber]];
        } else if ([instruction isEqualToString:@"slide_notes"]){
            uint slideNumber = [[command objectAtIndex:1] integerValue];
            NSMutableString *notes = [[NSMutableString alloc] init];
            for (int i = 2; i<command.count; ++i) {
                [notes appendString:[command objectAtIndex:i]];
            }
            [self.slideShow putNotes:notes
                              AtIndex:slideNumber];
            [[NSNotificationCenter defaultCenter] postNotificationName:MSG_SLIDE_NOTES object: [NSNumber numberWithUnsignedInt:slideNumber]];
        }
        
    }
    
}

@end
