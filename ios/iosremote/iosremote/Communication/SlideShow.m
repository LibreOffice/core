// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


#import "SlideShow.h"
#import "Base64.h"
#import <dispatch/dispatch.h>
#import "slideShowViewController.h"

@interface SlideShow()

@property (nonatomic, strong) NSMutableArray* imagesArray;
@property (nonatomic, strong) NSMutableArray* notesArray;

@end

@implementation SlideShow

@synthesize size = _size;
@synthesize currentSlide = _currentSlide;
@synthesize delegate = _delegate;

dispatch_queue_t backgroundQueue;

- (SlideShow *) init{
    self = [super init];
    self.imagesArray = [[NSMutableArray alloc] init];
    self.notesArray = [[NSMutableArray alloc] init];
    _size = 0;
    _currentSlide = 0;
    
    backgroundQueue = dispatch_queue_create("org.libreoffice.iosremote.bgqueue", NULL);
    
    return self;
}

- (void) putImage: (NSString *)img AtIndex: (uint) index{
    
    NSData* data = [NSData dataWithBase64String:img];
    UIImage* image = [UIImage imageWithData:data];
    [self.imagesArray insertObject:image atIndex:index];
    [[NSNotificationCenter defaultCenter] postNotificationName:@"IMAGE_READY" object:nil];
}

- (void) putNotes: (NSString *)notes AtIndex: (uint) index{
    [self.notesArray insertObject:notes atIndex:index];
    [[NSNotificationCenter defaultCenter] postNotificationName:@"NOTE_READY" object:nil];
}

- (UIImage *) getImageAtIndex: (uint) index
{
    return [self.imagesArray objectAtIndex:index];
}

- (NSString *) getNotesAtIndex: (uint) index
{
    return [self.notesArray objectAtIndex:index];
}



@end
