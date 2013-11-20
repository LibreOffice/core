// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "WalkThroughPageViewController.h"

@interface WalkThroughPageViewController ()

@end

@implementation WalkThroughPageViewController

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
    // Do any additional setup after loading the view from its nib.
    
    switch (self.index) {
        case 0:
            [self.image setImage:[UIImage imageNamed:@"WTconnection"]];
            self.instructionText.text = NSLocalizedString(@"Connect your devices to the same WiFi network.", @"connect instructions");
            self.instructionHint.text = NSLocalizedString(@"Create a personal hotspot on your iPhone if needed.", @"connect instructions");
            self.instructionTitle.text = NSLocalizedString(@"Connection", @"instruction page title");
            [self.hintImage setImage:[UIImage imageNamed:@"WTConnectionHotspot"]];
            break;
        case 1:
            [self.image setImage:[UIImage imageNamed:@"WTconnecting"]];
            self.instructionText.text = NSLocalizedString(@"Launch Impress & select your computer.", @"Pairing instruction text");
            self.instructionHint.text = NSLocalizedString(@"Enter the PIN code in SlideShow - Impress Remote", @"Pairing instruction text");
            self.instructionTitle.text = NSLocalizedString(@"Pairing", @"Pairing instruction title");
            [self.hintImage setImage:[UIImage imageNamed:@"WTPairing"]];
            break;
        case 2:{
            if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPhone)
                [self.image setImage:[UIImage imageNamed:@"WTcontrol~iphone"]];
            else
                [self.image setImage:[UIImage imageNamed:@"WTcontrol~ipad"]];
//            self.instructionText.text = @"Control your presentation at your fingertips!";
            self.instructionTitle.text = NSLocalizedString(@"Control", @"Control instruction title");
            UIImage *backgroundImage = [UIImage imageNamed:@"buttonBackground"];
            UIEdgeInsets insets = UIEdgeInsetsMake(20, 7, 20, 7);
            UIImage *stretchableBackgroundImage = [backgroundImage resizableImageWithCapInsets:insets];
            
            [self.okButton setBackgroundImage:stretchableBackgroundImage forState:UIControlStateNormal];
            
            self.okButton.hidden = NO;
            break;
        }
        default:
            break;
    }
    self.indexLabl.text = [NSString stringWithFormat:@"%d", self.index+1];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)viewDidUnload {
    [self setImage:nil];
    [self setInstructionText:nil];
    [self setInstructionTitle:nil];
    [self setOkButton:nil];
    [self setInstructionHint:nil];
    [self setIndexLabl:nil];
    [self setHintImage:nil];
    [super viewDidUnload];
}
- (IBAction)okButtonHandleBack:(id)sender {
    [self.navigationController popToRootViewControllerAnimated:YES];
}
@end
