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

@end

@implementation server_list_vc

@synthesize center = _center;
@synthesize comManager = _comManager;
@synthesize slideShowPreviewStartObserver = _slideShowPreviewStartObserver;

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
    
    self.center = [NSNotificationCenter defaultCenter];
    self.comManager = [CommunicationManager sharedComManager];
    self.serverTable.dataSource = self;
    self.serverTable.delegate = self;
    
    NSOperationQueue *mainQueue = [NSOperationQueue mainQueue];
    self.slideShowPreviewStartObserver = [[NSNotificationCenter defaultCenter] addObserverForName:STATUS_CONNECTED_SLIDESHOW_RUNNING
                                                                                           object:nil
                                                                                            queue:mainQueue
                                                                                       usingBlock:^(NSNotification *note) {
                                                                                           [self performSegueWithIdentifier:@"SlideShowPreview" sender:self ];
                                                                                    }];
}

-(void)viewWillAppear:(BOOL)animated
{
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
    [self.tableView cellForRowAtIndexPath:self.lastSpinningCellIndex].accessoryView = nil;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
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
