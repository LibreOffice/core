/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#import "touchPointer_vc.h"
#import "CommunicationManager.h"
#import "CommandTransmitter.h"
#import "CommandInterpreter.h"
#import "SlideShow.h"
#import <QuartzCore/QuartzCore.h>

@interface touchPointer_vc ()

@property (nonatomic, strong) CommunicationManager *comManager;
@end
@implementation touchPointer_vc

@synthesize comManager = _comManager;

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view.
    self.comManager = [CommunicationManager sharedComManager];
    [self.comManager.interpreter.slideShow getContentAtIndex:self.comManager.interpreter.slideShow.currentSlide forView:self.imageView];

    self.imageView.layer.shadowColor = [[UIColor blackColor] CGColor];
    self.imageView.layer.shadowOpacity = 0.5;
    self.imageView.layer.shadowRadius = 4.0;
    self.imageView.layer.shadowOffset = CGSizeMake(3.0f, 3.0f);
    self.imageView.layer.shadowPath = [UIBezierPath bezierPathWithRect:self.imageView.bounds].CGPath;
    self.imageView.clipsToBounds = NO;
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (IBAction)dismissModal:(id)sender {
    [self dismissViewControllerAnimated:YES completion:nil];
}
- (void)viewDidUnload {
    [self setImageView:nil];
    [super viewDidUnload];
}
@end
