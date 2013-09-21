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
#define STOPWATCH_AUTO_START @"STOPWATCH_AUTO_START"

@interface SlideShowPreviewViewController : UIViewController

@property (nonatomic, strong) CommunicationManager * comManager;

@property (nonatomic, strong) id slideShowStartObserver;
@property (nonatomic, strong) id titleObserver;

@property (weak, nonatomic) IBOutlet UILabel *titleLabel;
@property (weak, nonatomic) IBOutlet UIButton *startButton;
@property (weak, nonatomic) IBOutlet UIButton *prefButton;

- (IBAction)startPresentationAction:(id)sender;
- (IBAction)startPrefSettings:(id)sender;

@end
