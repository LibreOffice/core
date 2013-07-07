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
- (IBAction)pointerAction:(id)sender;

@property (weak, nonatomic) IBOutlet UIView *notesView;
@property (weak, nonatomic) IBOutlet UIWebView *lecturer_notes;
@property (weak, nonatomic) IBOutlet UIImageView *slideView;
@property (weak, nonatomic) IBOutlet UIImageView *secondarySlideView;
@property (weak, nonatomic) IBOutlet UILabel *slideNumber;

@end
