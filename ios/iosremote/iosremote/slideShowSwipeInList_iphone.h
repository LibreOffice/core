/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#import <UIKit/UIKit.h>

@class stopWatch;
@class Timer;

@interface slideShowSwipeInList : UITableViewController <UITableViewDataSource, UITableViewDelegate>

@property (nonatomic, strong) stopWatch *stopWatch;
@property (nonatomic, strong) Timer *timer;

@end
