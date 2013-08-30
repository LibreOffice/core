//
//  WalkThroughPageViewController.m
//  iosremote
//
//  Created by Siqi Liu on 8/27/13.
//  Copyright (c) 2013 libreoffice. All rights reserved.
//

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
            self.instructionText.text = @"Connect your devices to the same WiFi network.";
            self.instructionHint.text = @"Create a personal hotspot on your iPhone if needed.";
            self.instructionTitle.text = @"Connection";
            [self.hintImage setImage:[UIImage imageNamed:@"WTConnectionHotspot"]];
            break;
        case 1:
            [self.image setImage:[UIImage imageNamed:@"WTconnecting"]];
            self.instructionText.text = @"Launch Impress & select your computer.";
            self.instructionHint.text = @"Enter the PIN code in SlideShow - Impress Remote";
            self.instructionTitle.text = @"Pairing";
            [self.hintImage setImage:[UIImage imageNamed:@"WTPairing"]];
            break;
        case 2:{
            [self.image setImage:[UIImage imageNamed:@"WTcontrol"]];
//            self.instructionText.text = @"Control your presentation at your fingertips!";
            self.instructionTitle.text = @"Control";
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
