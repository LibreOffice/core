//
//  PresentationViewController.h
//  iosremote
//
//  Created by Siqi Liu on 7/26/13.
//  Copyright (c) 2013 libreoffice. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "slideShowPreviewTable_vc_ipad.h"

@interface MainSplitViewController : UISplitViewController <MainSplitViewControllerProtocol>

- (void) didReceivePresentationStarted;

@property (strong, nonatomic) UIViewController * splitController;
@end
