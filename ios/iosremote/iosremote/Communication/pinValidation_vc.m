//
//  pinValidation_vcViewController.m
//  iosremote
//
//  Created by Liu Siqi on 7/4/13.
//  Copyright (c) 2013 libreoffice. All rights reserved.
//

#import "pinValidation_vc.h"
#import "slideShowPreview_vc.h"
#import "SlideShow.h"
#import "CommandInterpreter.h"
#import "CommunicationManager.h"

@interface pinValidation_vc ()

@property (nonatomic, strong) CommunicationManager *comManager;
@property (nonatomic, strong) id slideShowPreviewStartObserver;

@end

@implementation pinValidation_vc

@synthesize comManager = _comManager;
@synthesize slideShowPreviewStartObserver = _slideShowPreviewStartObserver;

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
    [self.pinLabel setText:[NSString stringWithFormat:@"%@", [self.comManager getPairingPin]]];
    NSOperationQueue *mainQueue = [NSOperationQueue mainQueue];
    self.slideShowPreviewStartObserver = [[NSNotificationCenter defaultCenter] addObserverForName:STATUS_CONNECTED_SLIDESHOW_RUNNING
                                                                                           object:nil
                                                                                            queue:mainQueue
                                                                                       usingBlock:^(NSNotification *note) {
                                                                       [self performSegueWithIdentifier:@"pinValidated" sender:self ];
                                                                   }];
}

- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender{
    if ([segue.identifier isEqualToString:@"pinValidated"]) {
        slideShowPreview_vc *destViewController = segue.destinationViewController;
        destViewController.slideshow = [self.comManager.interpreter slideShow];
        [destViewController.slideshow setDelegate:destViewController];
    }
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)viewDidUnload {
    [self setPinLabel:nil];
    [self setStatusLabel:nil];
    [super viewDidUnload];
}
@end
