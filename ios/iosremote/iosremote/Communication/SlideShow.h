//
//  SlideShow.h
//  iosremote
//
//  Created by Liu Siqi on 6/7/13.
//  Copyright (c) 2013 libreoffice. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface SlideShow : NSObject

@property (nonatomic, strong) NSArray* mSlides;
@property (nonatomic, strong) NSNumber* mCurrentSlide;

- (NSUInteger) size;
- (void) setCurrentSlide:(NSUInteger)number;
- (void) setLength:(NSUInteger)number;

@end
