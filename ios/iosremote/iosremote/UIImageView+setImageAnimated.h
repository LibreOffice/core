//
//  NSObject+UIImageView_setImageAnimated.h
//  iosremote
//
//  Created by Siqi Liu on 7/31/13.
//  Copyright (c) 2013 libreoffice. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface UIImageView (setImageAnimated)

- (void) setImage:(UIImage *)image animated:(BOOL) animated Duration:(double)duration;
- (void) setImage:(UIImage *)image animated:(BOOL) animated;

@end
