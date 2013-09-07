//
//  UIView+Shadowing.m
//  iosremote
//
//  Created by Siqi Liu on 9/6/13.
//  Copyright (c) 2013 libreoffice. All rights reserved.
//

#import "UIView+Shadowing.h"
#import <QuartzCore/CALayer.h>

@implementation UIView (Shadowing)

- (void) setShadow
{
    self.layer.shadowColor = [[UIColor blackColor] CGColor];
    self.layer.shadowOpacity = 0.5;
    self.layer.shadowRadius = 4.0;
    self.layer.shadowOffset = CGSizeMake(3.0f, 3.0f);
    self.layer.shadowPath = [UIBezierPath bezierPathWithRect:self.bounds].CGPath;
    self.clipsToBounds = NO;
}

@end
