//
//  slideShowPreview_vc.h
//  iosremote
//
//  Created by Liu Siqi on 7/4/13.
//  Copyright (c) 2013 libreoffice. All rights reserved.
//

#import <UIKit/UIKit.h>

@class SlideShow;

@interface slideShowPreview_vc : UITableViewController

@property (nonatomic, strong) SlideShow* slideshow;
@property (nonatomic, strong) id slideShowImageReadyObserver;
@property (nonatomic, strong) id slideShowNoteReadyObserver;

@end
