/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#import <UIKit/UIKit.h>

@interface slideShow_vc : UIViewController

- (IBAction)nextSlideAction:(id)sender;
- (IBAction)previousSlideAction:(id)sender;
- (IBAction)pointerAction:(id)sender;

@property (weak, nonatomic) IBOutlet UIView *notesView;
@property (weak, nonatomic) IBOutlet UIWebView *lecturer_notes;
@property (weak, nonatomic) IBOutlet UIImageView *slideView;
@property (weak, nonatomic) IBOutlet UIImageView *secondarySlideView;
@property (weak, nonatomic) IBOutlet UILabel *slideNumber;

@end
