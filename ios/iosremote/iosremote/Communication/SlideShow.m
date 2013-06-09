// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


#import "SlideShow.h"
#import "Base64.h"

@interface SlideShow()

@property (nonatomic, strong) NSMutableArray* imagesArray;
@property (nonatomic, strong) NSMutableArray* notesArray;

@end

@implementation SlideShow

@synthesize size = _size;
@synthesize currentSlide = _currentSlide;

- (SlideShow *) init{
    self = [super init];
    _size = 0;
    _currentSlide = 0;
    return self;
}

- (void) putImage: (NSString *)img AtIndex: (uint) index{
    [Base64 initialize];
    NSData* data = [Base64 decode:img];
    UIImage* image = [UIImage imageWithData:data];
    [self.imagesArray insertObject:image atIndex:index];
}

- (void) putNotes: (NSString *)notes AtIndex: (uint) index{
    [self.notesArray insertObject:notes atIndex:index];
}


@end
