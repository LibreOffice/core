/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#import "slideShowPreviewTable_vc.h"
#import "CommunicationManager.h"
#import "CommandTransmitter.h"
#import "CommandInterpreter.h"
#import "SlideShow.h"
#import "AppDelegate.h"
#import "IASKAppSettingsViewController.h"

@interface slideShowPreviewTable_vc () <IASKSettingsDelegate>

@property (nonatomic, strong) IASKAppSettingsViewController *appSettingsViewController;

@end

@implementation slideShowPreviewTable_vc

@synthesize startButton = _startButton;
@synthesize titleObserver = _titleObserver;
@synthesize appSettingsViewController = _appSettingsViewController;

#pragma mark IASKSettingsDelegate
- (void)settingsViewControllerDidEnd:(IASKAppSettingsViewController*)sender
{
    [(AppDelegate *)[[UIApplication sharedApplication] delegate] reconfigure];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation
{
    if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad)
        return YES;
    else {
        return toInterfaceOrientation == UIInterfaceOrientationMaskPortrait;
    }
}

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
    
    // Setting up UIButton stretchable background
    UIImage *backgroundImage = [UIImage imageNamed:@"buttonBackground"];
    UIEdgeInsets insets = UIEdgeInsetsMake(20, 7, 20, 7);
    UIImage *stretchableBackgroundImage = [backgroundImage resizableImageWithCapInsets:insets];
    [self.startButton setBackgroundImage:stretchableBackgroundImage forState:UIControlStateNormal];
    [self.prefButton setBackgroundImage:stretchableBackgroundImage forState:UIControlStateNormal];

}

- (void) viewDidDisappear:(BOOL)animated
{
    [[NSNotificationCenter defaultCenter] removeObserver:self.slideShowStartObserver];
    self.slideShowStartObserver = nil;
    [super viewDidDisappear:animated];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark - Table view data source

-(IBAction)startPresentationAction:(id)sender {
    [[self.comManager transmitter] startPresentation];
}

- (IBAction)startPrefSettings:(id)sender {
    self.appSettingsViewController.showDoneButton = NO;
	[self.navigationController pushViewController:self.appSettingsViewController animated:YES];
}

#pragma mark - IN app setting
- (IASKAppSettingsViewController*)appSettingsViewController {
	if (!_appSettingsViewController) {
		_appSettingsViewController = [[IASKAppSettingsViewController alloc] initWithNibName:@"IASKAppSettingsView" bundle:nil];
		_appSettingsViewController.delegate = self;
	}
	return _appSettingsViewController;
}

- (void)viewDidUnload {
    [[NSNotificationCenter defaultCenter] removeObserver:self.slideShowStartObserver];
    self.slideShowStartObserver = nil;
    [self setStartButton:nil];
    [self setPrefButton:nil];
    [self setTitleLabel:nil];
    [super viewDidUnload];
}
@end
