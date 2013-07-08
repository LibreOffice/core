//
//  Animal.m
//  SlideoutNavigation
//
//  Created by Tammy Coron on 1/10/13.
//  Copyright (c) 2013 Tammy L Coron. All rights reserved.
//

#import "Animal.h"

@implementation Animal

+ (id)itemWithTitle:(NSString *)title withImage:(UIImage *)image withCreator:(NSString *)creator
{
    return [[self alloc] initWithTitle:(NSString *)title withImage:(UIImage *)image withCreator:(NSString *)creator];
}

- (id)initWithTitle:(NSString *)title withImage:(UIImage *)image withCreator:(NSString *)creator
{
    if ((self = [super init]))
    {
        _title = title;
        _image = image;
        _creator = creator;
    }
    
    return self;
}

@end
