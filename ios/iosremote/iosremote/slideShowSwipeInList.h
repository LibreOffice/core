/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#import <UIKit/UIKit.h>

@interface slideShowSwipeInList : UITableViewController <UITableViewDataSource, UITableViewDelegate>
// StopWatch
- (IBAction)stopWatchStart:(id)sender;
- (IBAction)stopWatchClear:(id)sender;
@property (strong, nonatomic) NSTimer *stopWatchTimer;
@property (strong, nonatomic) NSDate *startDate;

@end
