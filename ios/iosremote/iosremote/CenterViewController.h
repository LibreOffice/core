//
//  CenterViewController.h
//  Navigation
//
//  Created by Tammy Coron on 1/19/13.
//  Copyright (c) 2013 Tammy L Coron. All rights reserved.
//

#import "LeftPanelViewController.h"
#import "RightPanelViewController.h"

@protocol CenterViewControllerDelegate <NSObject>

@optional
- (void)movePanelLeft;
- (void)movePanelRight;

@required
- (void)movePanelToOriginalPosition;

@end

@interface CenterViewController : UIViewController <LeftPanelViewControllerDelegate, RightPanelViewControllerDelegate>

@property (nonatomic, assign) id<CenterViewControllerDelegate> delegate;

@property (nonatomic, weak) IBOutlet UIButton *leftButton;
@property (nonatomic, weak) IBOutlet UIButton *rightButton;

@end
