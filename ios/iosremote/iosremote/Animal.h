//
//  Animal.h
//  SlideoutNavigation
//
//  Created by Tammy Coron on 1/10/13.
//  Copyright (c) 2013 Tammy L Coron. All rights reserved.
//

@interface Animal : NSObject

+ (id)itemWithTitle:(NSString *)title withImage:(UIImage *)image withCreator:(NSString *)creator;
- (id)initWithTitle:(NSString *)title withImage:(UIImage *)image withCreator:(NSString *)creator;

@property (nonatomic, copy) NSString *title;
@property (nonatomic, copy) UIImage  *image;
@property (nonatomic, copy) NSString *creator;

@end