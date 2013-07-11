// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "serverList_vc.h"
#import "CommunicationManager.h"
#import "newServer_vc.h"
#import "Server.h"

@interface server_list_vc ()

@property (nonatomic, strong) CommunicationManager *comManager;
@property (nonatomic, weak) NSNotificationCenter* center;
@property (nonatomic, strong) id slideShowPreviewStartObserver;
@property (nonatomic, strong) id pinValidationObserver;
@property (atomic, strong) NSIndexPath *lastSpinningCellIndex;

@end

@implementation server_list_vc

@synthesize center = _center;
@synthesize comManager = _comManager;
@synthesize lastSpinningCellIndex = _lastSpinningCellIndex;
@synthesize slideShowPreviewStartObserver = _slideShowPreviewStartObserver;
@synthesize pinValidationObserver = _pinValidationObserver;

- (id)initWithStyle:(UITableViewStyle)style
{
    self = [super initWithStyle:style];
    if (self) {
        // Custom initialization
    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    // Uncomment the following line to preserve selection between presentations.
    // self.clearsSelectionOnViewWillAppear = NO;
 
    // Uncomment the following line to display an Edit button in the navigation bar for this view controller.
    // self.navigationItem.rightBarButtonItem = self.editButtonItem;
    
//    self.lastSpinningCellIndex = [[NSIndexPath alloc] init];
    self.center = [NSNotificationCenter defaultCenter];
    self.comManager = [CommunicationManager sharedComManager];
    self.serverTable.dataSource = self;
    self.serverTable.delegate = self;
}

- (void) viewDidAppear:(BOOL)animated
{
    NSOperationQueue *mainQueue = [NSOperationQueue mainQueue];
    self.pinValidationObserver = [[NSNotificationCenter defaultCenter] addObserverForName:STATUS_PAIRING_PINVALIDATION
                                                                                   object:nil
                                                                                    queue:mainQueue
                                                                               usingBlock:^(NSNotification *note) {
                                                                                   self.comManager.state = CONNECTED;
                                                                                   [self disableSpinner];
                                                                                   [self performSegueWithIdentifier:@"pinValidation" sender:self ];
                                                                               }];
    self.slideShowPreviewStartObserver = [[NSNotificationCenter defaultCenter] addObserverForName:STATUS_PAIRING_PAIRED
                                                                                           object:nil
                                                                                            queue:mainQueue
                                                                                       usingBlock:^(NSNotification *note) {
                                                                                           self.comManager.state = CONNECTED;
                                                                                           [self disableSpinner];
                                                                                           [self performSegueWithIdentifier:@"SlideShowPreview" sender:self ];
                                                                                       }];
    [super viewDidAppear:animated];
}

- (void) viewWillDisappear:(BOOL)animated
{
    [self disableSpinner];
    [super viewWillDisappear:animated];
}

- (void) viewDidDisappear:(BOOL)animated
{
    [[NSNotificationCenter defaultCenter] removeObserver:self.slideShowPreviewStartObserver];
    [[NSNotificationCenter defaultCenter] removeObserver:self.pinValidationObserver];
    [super viewDidDisappear:animated];
}


-(void)viewWillAppear:(BOOL)animated
{
    [self.comManager setDelegate:self];
    [self.serverTable reloadData];
    [super viewWillAppear:animated];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark - Table view delegate

- (void)disableSpinner
{
    @synchronized(self.lastSpinningCellIndex){
        if ([[self.tableView cellForRowAtIndexPath:self.lastSpinningCellIndex] respondsToSelector:@selector(accessoryView)]) {
            if (self.tableView && [self.tableView cellForRowAtIndexPath:self.lastSpinningCellIndex]) {
                [[self.tableView cellForRowAtIndexPath:self.lastSpinningCellIndex] setAccessoryView:nil];
            }
        }
    }
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    UITableViewCell *cell = [tableView cellForRowAtIndexPath:indexPath];
    
    if(self.comManager.state!=CONNECTING){
        self.lastSpinningCellIndex = indexPath;
        UIActivityIndicatorView *activityView = [[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleGray];
        [activityView startAnimating];
        [cell setAccessoryView:activityView];
    }

    NSLog(@"Connecting to %@:%@", [[self.comManager.servers objectAtIndex:indexPath.row] serverName], [[self.comManager.servers objectAtIndex:indexPath.row] serverAddress]);
    self.comManager.delegate = self;
    [self.comManager connectToServer:[self.comManager.servers objectAtIndex:indexPath.row]];
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
}

- (void)viewDidUnload {
    [self setServerTable:nil];
    [super viewDidUnload];
}

-(NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return [self.comManager.servers count];
}

-(UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    static NSString *cellIdentifier = @"server_item_cell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:cellIdentifier];
    
    Server *s = [self.comManager.servers objectAtIndex:indexPath.row];
    
    [cell.textLabel setText:[s serverName]];
    [cell.detailTextLabel setText:[s serverAddress]];
    return cell;
}

- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath
{
    return YES;
}

- (UITableViewCellEditingStyle)tableView:(UITableView *)tableView editingStyleForRowAtIndexPath:(NSIndexPath *)indexPath{
    return UITableViewCellEditingStyleDelete;
}

- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath{
    
    if(editingStyle == UITableViewCellEditingStyleDelete){
        [self.comManager removeServerAtIndex:indexPath.row];
        [tableView deleteRowsAtIndexPaths:[NSArray arrayWithObject:indexPath] withRowAnimation:UITableViewRowAnimationFade];
    }
}

@end
