/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#import <UIKit/UIKit.h>
#import "CommunicationManager.h"

#define OPTION_TIMER NSLocalizedString(@"Timer auto-start", nil)
#define OPTION_POINTER NSLocalizedString(@"Touch pointer", nil)
#define KEY_TIMER @"TIMER_AUTOSTART_ENABLED"
#define KEY_POINTER @"TOUCH_POINTER_ENABLED"

@interface slideShowPreviewTable_vc : UITableViewController <UITableViewDataSource, UITableViewDelegate>

@property (strong, nonatomic) IBOutlet UITableView *optionsTable;
@property (nonatomic, strong) CommunicationManager * comManager;
@property (nonatomic, strong) id slideShowStartObserver;
@property (nonatomic, strong) NSArray * optionsArray;

@property (nonatomic, strong) id titleObserver;

-(IBAction)startPresentationAction:(id)sender;

@end
