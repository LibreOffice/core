//
//  slideShowViewController.h
//  iosremote
//
//  Created by Liu Siqi on 6/9/13.
//  Copyright (c) 2013 libreoffice. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "SlideShow.h"

@interface slideShowViewController : UIViewController

@property (weak, nonatomic) IBOutlet UIImageView *image;
@property (weak, nonatomic) IBOutlet UIWebView *lecturer_notes;

@property (nonatomic, strong) SlideShow *slideshow;
@property (nonatomic, strong) id slideShowImageReadyObserver;
@property (nonatomic, strong) id slideShowNoteReadyObserver;

@end
