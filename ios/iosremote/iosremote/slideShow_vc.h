//
//  slideShow_vc.h
//  iosremote
//
//  Created by Liu Siqi on 7/5/13.
//  Copyright (c) 2013 libreoffice. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface slideShow_vc : UIViewController

- (IBAction)nextSlideAction:(id)sender;
- (IBAction)previousSlideAction:(id)sender;

@property (weak, nonatomic) IBOutlet UIWebView *lecturer_notes;
@property (weak, nonatomic) IBOutlet UIImageView *slideView;

@end
