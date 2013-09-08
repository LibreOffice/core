// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

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
-(void)startConnectionModal:(id)sender;
- (void) setWelcomePageVisible:(BOOL)visible;
@property (strong, nonatomic) UIView *welcome_blocking_page;

@property (weak, nonatomic) IBOutlet UITableView *horizontalTableView;

@property (weak, nonatomic) IBOutlet UIButton *gearButton;
- (IBAction)popOverUp:(id)sender;

// Stopwatch/Timer

@property (weak, nonatomic) IBOutlet UIButton *startButton;
@property (weak, nonatomic) IBOutlet UIButton *clearButton;
@property (weak, nonatomic) IBOutlet UILabel *timeLabel;

@property (weak, nonatomic) IBOutlet UIButton *timerStartButton;
@property (weak, nonatomic) IBOutlet UIButton *timerClearButton;
@property (weak, nonatomic) IBOutlet UIButton *timerSetTimeButton;
@property (weak, nonatomic) IBOutlet UILabel *timerTimeLabel;

@property (weak, nonatomic) IBOutlet UIView *stopWatchView;
@property (weak, nonatomic) IBOutlet UIView *timerView;
@property (weak, nonatomic) IBOutlet UIScrollView *stopWatchTimerScrollView;

//Reconnection
- (void) didReceiveDisconnection;

@end
