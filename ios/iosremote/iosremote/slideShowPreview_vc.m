/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#import "slideShowPreview_vc.h"
#import "SlideShow.h"
#import "CommandInterpreter.h"
#import "CommunicationManager.h"

@interface slideShowPreview_vc () <UINavigationControllerDelegate>

@property (nonatomic, strong) CommunicationManager* comManager;
@property (nonatomic, strong) SlideShow* slideshow;

@end

@implementation slideShowPreview_vc

@synthesize slideshow = _slideshow;
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
    self.slideshow = [[self.comManager interpreter] slideShow];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end
