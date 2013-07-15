/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#import "pinValidation_vc.h"
#import "SlideShow.h"
#import "CommandInterpreter.h"
#import "CommunicationManager.h"

@interface pinValidation_vc ()

@property (nonatomic, strong) CommunicationManager *comManager;
@property (nonatomic, strong) id slideShowPreviewStartObserver;

@end

@implementation pinValidation_vc

@synthesize comManager = _comManager;
@synthesize slideShowPreviewStartObserver = _slideShowPreviewStartObserver;

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
    [self.pinLabel setText:[NSString stringWithFormat:@"%@", [self.comManager getPairingPin]]];
}

- (void) viewDidAppear:(BOOL)animated
{
    NSOperationQueue *mainQueue = [NSOperationQueue mainQueue];
    self.slideShowPreviewStartObserver = [[NSNotificationCenter defaultCenter] addObserverForName:STATUS_PAIRING_PAIRED
                                                                                           object:nil
                                                                                            queue:mainQueue
                                                                                       usingBlock:^(NSNotification *note) {
                                                                                           [self performSegueWithIdentifier:@"pinValidated" sender:self ];
                                                                                       }];
    [super viewDidAppear:animated];
}

- (void) viewDidDisappear:(BOOL)animated
{
    [[NSNotificationCenter defaultCenter] removeObserver:self.slideShowPreviewStartObserver];
    [super viewDidDisappear:animated];
}


- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)viewDidUnload {
    [self setPinLabel:nil];
    [self setStatusLabel:nil];
    [super viewDidUnload];
}
@end
