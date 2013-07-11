/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#import <UIKit/UIKit.h>

#define OPTION_TIMER @"Timer auto-start"
#define OPTION_POINTER @"Touch pointer"
#define KEY_TIMER @"TIMER_AUTOSTART_ENABLED"
#define KEY_POINTER @"TOUCH_POINTER_ENABLED"

@interface slideShowPreviewTable_vc : UITableViewController <UITableViewDataSource, UITableViewDelegate>

@property (strong, nonatomic) IBOutlet UITableView *optionsTable;

@end
