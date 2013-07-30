//
//  slideShowPreview_vc~ipad.h
//  iosremote
//
//  Created by Siqi Liu on 7/26/13.
//  Copyright (c) 2013 libreoffice. All rights reserved.
//

#import "slideShowPreviewTable_vc.h"

@protocol MainSplitViewControllerProtocol <NSObject>

- (void) didReceivePresentationStarted;

@end

@interface slideShowPreviewTable_vc_ipad : slideShowPreviewTable_vc

@property (strong, nonatomic) id <MainSplitViewControllerProtocol> delegate;

@end
