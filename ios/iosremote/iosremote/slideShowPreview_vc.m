//
//  slideShowPreview_vc.m
//  iosremote
//
//  Created by Liu Siqi on 7/4/13.
//  Copyright (c) 2013 libreoffice. All rights reserved.
//

#import "slideShowPreview_vc.h"
#import "SlideShow.h"
#import "CommandInterpreter.h"
#import "CommunicationManager.h"

@interface slideShowPreview_vc ()

@property (nonatomic, strong) CommunicationManager* comManager;
@property (nonatomic, strong) SlideShow* slideshow;

@end

@implementation slideShowPreview_vc

@synthesize slideshow = _slideshow;
@synthesize comManager = _comManager;

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view.
    self.comManager = [CommunicationManager sharedComManager];
    self.slideshow = [[self.comManager interpreter] slideShow];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end
