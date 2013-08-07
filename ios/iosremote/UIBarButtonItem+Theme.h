//
//  UIBarButtonItem+Theme.h
//  VLC for iOS
//
//  Created by Romain Goyet on 14/06/13.
//  Copyright (c) 2013 Applidium. All rights reserved.
//
//  Refer to the COPYING file of the official project for license.
//

#import <UIKit/UIKit.h>

@interface UIBarButtonItem (Theme)
+ (UIBarButtonItem *)themedDoneButtonWithTarget:(id)target andSelector:(SEL)selector;
+ (UIBarButtonItem *)themedBackButtonWithTarget:(id)target andSelector:(SEL)selector;
@end
