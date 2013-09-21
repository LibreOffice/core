/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#import <UIKit/UIKit.h>

@class Timer;

typedef enum TitleLabelOwner : NSInteger TitleLabelOwner;
enum TitleLabelOwner : NSInteger {
    STOPWATCH,
    TIMER
};

TitleLabelOwner owner;

@interface UIViewController (LibOStyling)

- (void)setTitle:(NSString *)title sender:(id)sender;
- (void)setTitle:(NSString *)title;

- (void)setOwner:(TitleLabelOwner) aOwner;

@end
