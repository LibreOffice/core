/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#import <UIKit/UIKit.h>
#import "CommunicationManager.h"

// Once connected, users will be directed to this page. If the slideshow is not running yet, users may change the preferences and start the presentation. Otherwise, users will be directly taken to the presentation view controller.
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
