//
//  BasePresentationViewController.h
//  iosremote
//
//  Created by Siqi Liu on 7/27/13.
//  Copyright (c) 2013 libreoffice. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "SlideShow.h"
#import "slideShowPreviewTable_vc_ipad.h"

@interface BasePresentationViewController : UIViewController <UISplitViewControllerDelegate, UITableViewDelegate, UITableViewDataSource, AsyncLoadHorizontalTableDelegate>

@property (weak, nonatomic) IBOutlet UIView *NotesView;
@property (weak, nonatomic) IBOutlet UIWebView *NoteWebView;
@property (weak, nonatomic) IBOutlet UIView *movingPointer;
@property (weak, nonatomic) IBOutlet UIImageView *currentSlideImageView;
@property (weak, nonatomic) IBOutlet UILabel *slideNumber;

@property (weak, nonatomic) IBOutlet UIButton *previousButton;
@property (weak, nonatomic) IBOutlet UIButton *nextButton;

// Split view controller
- (void) hideMaster:(BOOL)hideState;
@property BOOL masterIsHiddenWhenLandscape;

// Welcome page
-(IBAction)startConnectionModal:(id)sender;
- (void) setWelcomePageVisible:(BOOL)visible;
@property (weak, nonatomic) IBOutlet UIView *welcome_blocking_page;
@property (weak, nonatomic) IBOutlet UIImageView *welcome_libO_icon;
@property (weak, nonatomic) IBOutlet UIButton *welcome_connect_button;
@property (weak, nonatomic) IBOutlet UILabel *welcome_label;

@property (weak, nonatomic) IBOutlet UITableView *horizontalTableView;
@property (weak, nonatomic) IBOutlet UIButton *startButton;
@property (weak, nonatomic) IBOutlet UIButton *clearButton;
@property (weak, nonatomic) IBOutlet UILabel *timeLabel;

//Reconnection
- (void) didReceiveDisconnection;

@end
