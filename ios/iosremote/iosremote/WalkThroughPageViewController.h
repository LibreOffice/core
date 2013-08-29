//
//  WalkThroughPageViewController.h
//  iosremote
//
//  Created by Siqi Liu on 8/27/13.
//  Copyright (c) 2013 libreoffice. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface WalkThroughPageViewController : UIViewController
@property (weak, nonatomic) IBOutlet UIImageView *image;
@property (weak, nonatomic) IBOutlet UILabel *instructionTitle;
@property (weak, nonatomic) IBOutlet UILabel *instructionText;
@property (weak, nonatomic) IBOutlet UIImageView *hintImage;
@property (weak, nonatomic) IBOutlet UILabel *instructionHint;
@property (weak, nonatomic) IBOutlet UILabel *indexLabl;
@property (weak, nonatomic) IBOutlet UIButton *okButton;

- (IBAction)okButtonHandleBack:(id)sender;
@property (assign, nonatomic) NSInteger index;
@end
