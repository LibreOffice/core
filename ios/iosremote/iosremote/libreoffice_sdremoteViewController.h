//
//  libreoffice_sdremoteViewController.h
//  iosremote
//
//  Created by Liu Siqi on 6/4/13.
//  Copyright (c) 2013 libreoffice. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface libreoffice_sdremoteViewController : UIViewController

- (IBAction)connectToServer:(id)sender;

@property (weak, nonatomic) IBOutlet UITextField *ipAddressTextEdit;

@end
