//
//  SlideShow.m
//  iosremote
//
//  Created by Liu Siqi on 6/7/13.
//  Copyright (c) 2013 libreoffice. All rights reserved.
//

#import "SlideShow.h"

@implementation SlideShow

@synthesize mSlides = _mSlides;
@synthesize mCurrentSlide = _mCurrentSlide;

- (NSUInteger) size{
    return self.mSlides.count;
}

@end
