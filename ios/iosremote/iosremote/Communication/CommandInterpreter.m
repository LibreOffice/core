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

dispatch_queue_t backgroundQueue;

- (CommandInterpreter *) init
{
    self = [super init];
    self.slideShow = [[SlideShow alloc] init];
    return self;
}

- (BOOL) isSlideRunning {
    return [self.slideShow size] > 0;
}

// Received a set of instructions from server.
- (void) parse:(NSArray*)command{
    uint marker = 0;
    if ([command count] == 0) {
        return;
    }

    NSString *instruction = [command objectAtIndex:0];
    if ([instruction isEqualToString:STATUS_PAIRING_PINVALIDATION]) {
        [[NSNotificationCenter defaultCenter] postNotificationName:STATUS_PAIRING_PINVALIDATION
                                                            object:nil];
        marker = 2;
    }
    else if ([instruction isEqualToString:STATUS_PAIRING_PAIRED]){
//        NSLog(@"Paired command: %@", command);
        [[NSNotificationCenter defaultCenter] postNotificationName:STATUS_PAIRING_PAIRED
                                                            object:nil];
        marker = 2;
    }
    else if([instruction isEqualToString:@"slideshow_started"]){
        uint slideLength = [[command objectAtIndex:1] integerValue];
        uint currentSlide = [[command objectAtIndex:2] integerValue];
        NSLog(@"Interpreter: slideshow_started with currentSlide: %u slideLength: %u", currentSlide, slideLength);

        [self.slideShow setSize:slideLength];
        [self.slideShow setCurrentSlide:currentSlide];

        [[NSNotificationCenter defaultCenter] postNotificationName:STATUS_CONNECTED_SLIDESHOW_RUNNING
                                                            object:nil];
        [[NSNotificationCenter defaultCenter] postNotificationName:MSG_SLIDE_CHANGED object:nil];
        marker = 4;
    } else if ([instruction isEqualToString:@"slideshow_finished"]){
        NSLog(@"Interpreter: slideshow_finished");
        self.slideShow = [[SlideShow alloc] init];
        [[NSNotificationCenter defaultCenter] postNotificationName:STATUS_CONNECTED_NOSLIDESHOW object:nil];
        marker = 2;
    }  else if([instruction isEqualToString:@"slideshow_info"]){
        NSLog(@"Interpreter: slideshow_info");
        NSString * title = [command objectAtIndex:1];
        NSLog(@"title: %@", title);
        [self.slideShow setTitle:title];
        [[NSNotificationCenter defaultCenter] postNotificationName:SLIDESHOW_INFO_RECEIVED
                                                            object:nil];
        marker = 3;
    } else {
        if ([instruction isEqualToString:@"slide_updated"]) {
            NSLog(@"Interpret   er: slide_updated");
            uint newSlideNumber = [[command objectAtIndex:1] integerValue];
            [self.slideShow setCurrentSlide:newSlideNumber];

            [[NSNotificationCenter defaultCenter] postNotificationName:MSG_SLIDE_CHANGED object:nil];
            marker = 3;
        } else if ([instruction isEqualToString:@"slide_preview"]){
//            NSLog(@"Interpreter: slide_preview");
            backgroundQueue = dispatch_queue_create("com.libreoffice.iosremote", NULL);
            dispatch_async(backgroundQueue, ^(void) {
                uint slideNumber = [[command objectAtIndex:1] integerValue];
                NSString * imageData = [command objectAtIndex:2];
                [self.slideShow putImage:imageData
                                 AtIndex:slideNumber];
            });
            marker = 4;
        } else if ([instruction isEqualToString:@"slide_notes"]){
//            NSLog(@"Interpreter: slide_notes");
            backgroundQueue = dispatch_queue_create("com.libreoffice.iosremote", NULL);
            uint slideNumber = [[command objectAtIndex:1] integerValue];
            NSMutableString *notes = [[NSMutableString alloc] init];
            for (int i = 2; i<command.count; ++i) {
                [notes appendString:[command objectAtIndex:i]];
                if ([notes hasSuffix:@"</body>"]) {
                    marker = i+2;
                    break;
                }
            }
            [self.slideShow putNotes:notes
                             AtIndex:slideNumber];
            [[NSNotificationCenter defaultCenter] postNotificationName:MSG_SLIDE_NOTES object: [NSNumber numberWithUnsignedInt:slideNumber]];
        }

    }
    if ([command objectAtIndex:marker] && ![[command objectAtIndex:marker] isEqualToString:@""])
    {
        NSRange range;
        range.location = marker;
        range.length = [command count] - marker;
        [self parse:[command subarrayWithRange:range]];
    }
}

@end
