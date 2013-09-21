// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "ServerListViewController.h"
#import "CommunicationManager.h"
#import "NewServerViewController.h"
#import "Server.h"
#import "Client.h"
#import "PopoverView.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

@interface ServerListViewController () <NSNetServiceBrowserDelegate, NSNetServiceDelegate, PopoverViewDelegate>

@property (nonatomic, strong) CommunicationManager *comManager;
@property (nonatomic, weak) NSNotificationCenter* center;
@property (nonatomic, strong) id slideShowPreviewStartObserver;
@property (nonatomic, strong) id pinValidationObserver;
@property (atomic, strong) NSIndexPath *lastSpinningCellIndex;
@property (nonatomic, strong) NSNetServiceBrowser *serviceBrowser;

@property (nonatomic, strong) NSTimer *searchLabelTimer;
@property (nonatomic, strong) NSTimer *searchTimeoutTimer;
@property (nonatomic, strong) NSString *searchStateText;
@property (nonatomic)  UITableViewCellSelectionStyle style;

@end

@implementation ServerListViewController

@synthesize style = _style;
@synthesize searchStateText = _searchStateText;
@synthesize searchLabelTimer = _searchLabelTimer;
@synthesize center = _center;
@synthesize comManager = _comManager;
@synthesize lastSpinningCellIndex = _lastSpinningCellIndex;
@synthesize slideShowPreviewStartObserver = _slideShowPreviewStartObserver;
@synthesize pinValidationObserver = _pinValidationObserver;
@synthesize serviceBrowser = _serviceBrowser;

- (IBAction)onClickMenuButton:(id)sender {
    [PopoverView showPopoverAtPoint:CGPointMake([[sender view] center].x, [[sender view] center].y + [[sender view] frame].size.height * 0.5) inView:[sender view].superview withStringArray:
     [NSArray arrayWithObjects:NSLocalizedString(@"Connection Help", @"Popover option"), NSLocalizedString(@"About Impress Remote", @"Popover option"), nil]
                           delegate:self];
}

- (void)popoverView:(PopoverView *)popoverView didSelectItemAtIndex:(NSInteger)index
{
    [popoverView dismiss];
    switch (index) {
        case 0:
            [self performSegueWithIdentifier:@"howtoSegue" sender:self];
            break;
        case 1:
            [self performSegueWithIdentifier:@"aboutSegue" sender:self];
            break;
        default:
            break;
    }
}

#pragma mark - helper
- (void) startSearching
{
    [self.serviceBrowser stop];
    [self.comManager.autoDiscoveryServers removeAllObjects];
    [self.serviceBrowser searchForServicesOfType:@"_impressremote._tcp" inDomain:@"local"];
    [self.serviceBrowser scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
}

- (void) setSearchStateText:(NSString *)searchStateText
{
    _searchStateText = searchStateText;
    // This doesn't work well on iOS7, might be a bug. The text will get duplicated, it seems that the older section header view was not removed
    [self.serverTable reloadSections:[NSIndexSet indexSetWithIndex:0] withRowAnimation:UITableViewRowAnimationNone];
    //    [self.serverTable reloadData];
}

- (void) setStyle:(UITableViewCellSelectionStyle)style
{
    _style = style;
    [self.serverTable reloadSections:[NSIndexSet indexSetWithIndex:0] withRowAnimation:UITableViewRowAnimationNone];
}

#pragma mark - netservice resolve delegate
-(void) netServiceDidResolveAddress:(NSNetService *)sender
{
    if ([sender.addresses count]){
        NSData * address = [[sender addresses] objectAtIndex: 0];
        struct sockaddr_in *socketAddress = (struct sockaddr_in *) [address bytes];
        NSString * ipString = [NSString stringWithFormat: @"%s",inet_ntoa(socketAddress->sin_addr)];
        int port = socketAddress->sin_port;
        NSLog(@"Resolved at %@:%u", ipString, port);
        
        [self.comManager connectToServer:[[Server alloc] initWithProtocol:NETWORK atAddress:ipString ofName:sender.name]];
    }
}

-(void) netService:(NSNetService *)sender didNotResolve:(NSDictionary *)errorDict
{
    NSLog(@"Failed to resolve");
    [self.comManager.autoDiscoveryServers removeAllObjects];
    [self startSearching];
    
    UIAlertView *message = [[UIAlertView alloc] initWithTitle:NSLocalizedString(@"Failed to reach your computer", nil)
                                                      message:NSLocalizedString(@"Please consider restart LibreOffice Impress on your computer.", nil)
                                                     delegate:self
                                            cancelButtonTitle:NSLocalizedString(@"OK", nil)
                                            otherButtonTitles:nil];
    [self disableSpinner];
    [message show];
}

#pragma mark - bonjour service discovery

-(void) handleSearchTimeout
{
    [self.serviceBrowser stop];
}

-(void) updateSearchLabel
{
    static short count = 1;
    NSString * searchText = NSLocalizedString(@"Searching", nil);
    for (uint i = 0; i<=count; ++i) {
        searchText = [searchText stringByAppendingString:@"."];
    }
    NSLog(@"Updating count = %u SearchText = %@", count, searchText);
    self.searchStateText = searchText;
    count++;
    count = count % 3;
}

-(void) netServiceBrowserWillSearch:(NSNetServiceBrowser *)aNetServiceBrowser
{
    NSLog(@"Will search");
    self.comManager.searchState = SEARCHING;
    self.searchStateText = NSLocalizedString(@"Searching", nil);
    [self.searchLabelTimer invalidate];
    [self.searchTimeoutTimer invalidate];
    self.searchLabelTimer = [NSTimer scheduledTimerWithTimeInterval:1.0
                                                             target:self
                                                           selector:@selector(updateSearchLabel)
                                                           userInfo:nil
                                                            repeats:YES];
    
    self.searchTimeoutTimer = [NSTimer scheduledTimerWithTimeInterval:5.0
                                                               target:self
                                                             selector:@selector(handleSearchTimeout)
                                                             userInfo:nil
                                                              repeats:NO];
    
    [[NSRunLoop currentRunLoop] addTimer:self.searchLabelTimer forMode:NSRunLoopCommonModes];
    self.style = UITableViewCellSelectionStyleNone;
}

-(void) netServiceBrowserDidStopSearch:(NSNetServiceBrowser *)aNetServiceBrowser
{
    NSLog(@"End search");
    self.comManager.searchState = WAITING;
    [self.searchLabelTimer invalidate];
    [self.searchTimeoutTimer invalidate];
    self.searchStateText = NSLocalizedString(@"Click to refresh", nil);
    self.style = UITableViewCellSelectionStyleBlue;
}

- (void)netServiceBrowser:(NSNetServiceBrowser *)aNetServiceBrowser didNotSearch:(NSDictionary *)errorDict
{
    NSLog(@"search error");
}

- (void)netServiceBrowser:(NSNetServiceBrowser *)aNetServiceBrowser
           didFindService:(NSNetService *)aNetService
               moreComing:(BOOL)moreComing
{
    //    [self.comManager.autoDiscoveryServers removeObject:aNetService];
    [self.comManager.autoDiscoveryServers addObject:aNetService];
    
    NSLog(@"Got service %p with hostname %@\n", aNetService,
          [aNetService name]);
    
    [aNetService setDelegate:self];
    
    if(!moreComing)
    {
        [self.searchTimeoutTimer invalidate];
        [self.searchLabelTimer invalidate];
        self.comManager.searchState = WAITING;
        [self.tableView reloadSections:[NSIndexSet indexSetWithIndex:0] withRowAnimation:UITableViewRowAnimationAutomatic];
    }
}

// Sent when a service disappears
- (void)netServiceBrowser:(NSNetServiceBrowser *)browser
         didRemoveService:(NSNetService *)aNetService
               moreComing:(BOOL)moreComing
{
    [self.comManager.autoDiscoveryServers removeObject:aNetService];
    
    if(!moreComing)
    {
        //        [self.tableView reloadData];
        [self.tableView reloadSections:[NSIndexSet indexSetWithIndex:0] withRowAnimation:UITableViewRowAnimationAutomatic];
        [self startSearching];
    }
}

# pragma mark - System defaults

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation
{
    if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad)
        return YES;
    else {
        return toInterfaceOrientation == UIInterfaceOrientationMaskPortrait;
    }
}


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
    
    self.center = [NSNotificationCenter defaultCenter];
    self.comManager = [CommunicationManager sharedComManager];
    self.serverTable.dataSource = self;
    self.serverTable.delegate = self;
    _style = UITableViewCellSelectionStyleNone;
    
    [self setTitle:NSLocalizedString(@"Impress Remote", @"App name displayed on navbar")];
}

- (void) viewDidAppear:(BOOL)animated
{
    NSOperationQueue *mainQueue = [NSOperationQueue mainQueue];
    self.pinValidationObserver = [[NSNotificationCenter defaultCenter] addObserverForName:STATUS_PAIRING_PINVALIDATION
                                                                                   object:nil
                                                                                    queue:mainQueue
                                                                               usingBlock:^(NSNotification *note) {
                                                                                   [self.comManager.client stopConnectionTimeoutTimer];
                                                                                   self.comManager.state = CONNECTED;
                                                                                   [self disableSpinner];
                                                                                   [self performSegueWithIdentifier:@"pinValidation" sender:self ];
                                                                               }];
    self.slideShowPreviewStartObserver = [[NSNotificationCenter defaultCenter] addObserverForName:STATUS_PAIRING_PAIRED
                                                                                           object:nil
                                                                                            queue:mainQueue
                                                                                       usingBlock:^(NSNotification *note) {
                                                                                           [self.comManager.client stopConnectionTimeoutTimer];
                                                                                           self.comManager.state = CONNECTED;
                                                                                           [self disableSpinner];
                                                                                           [self performSegueWithIdentifier:@"SlideShowPreview" sender:self ];
                                                                                       }];
    NSLog(@"Clear auto discovered servers");
    [self.comManager.autoDiscoveryServers removeAllObjects];
    //    [self.serverTable reloadData];
    [self.tableView reloadSections:[NSIndexSet indexSetWithIndex:0] withRowAnimation:UITableViewRowAnimationAutomatic];
    self.serviceBrowser = [[NSNetServiceBrowser alloc] init];
    [self.serviceBrowser setDelegate:self];
    [self startSearching];
    
    if (![[NSUserDefaults standardUserDefaults] boolForKey:@"HOWTO_GUIDE_SEEN"])
    {
        UIAlertView * av = [[UIAlertView alloc] initWithTitle:@"How-To guide" message:NSLocalizedString(@"Take a tour of iOS Impress Remote?", nil) delegate:self cancelButtonTitle:NSLocalizedString(@"No, thanks", nil) otherButtonTitles:NSLocalizedString(@"Yes", nil), nil];
        
        [av show];
        [[NSUserDefaults standardUserDefaults] setBool:YES forKey:@"HOWTO_GUIDE_SEEN"];
    }
    
    [super viewDidAppear:animated];
}

- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex{
    if (buttonIndex == 0){
        [alertView dismissWithClickedButtonIndex:0 animated:YES];
    }else if (buttonIndex == 1){
        [alertView dismissWithClickedButtonIndex:0 animated:YES];
        [self performSegueWithIdentifier:@"howtoSegue" sender:self];
    }
}

- (void) viewWillDisappear:(BOOL)animated
{
    [self disableSpinner];
    [self.serviceBrowser stop];
    [self.serviceBrowser removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
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
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
    
    if ([self.comManager.autoDiscoveryServers count] == 0 && indexPath.section == 0){
        // No discovered server and not searching => in a "click to refresh" state, so we restart searching process
        if (self.comManager.searchState == WAITING){
            [self startSearching];
        }
        // Return when browser is still searching...
        return;
    }
    
    // Return when nothing should be done
    if (self.comManager.state == CONNECTING)
        return;
    
    UITableViewCell *cell = [tableView cellForRowAtIndexPath:indexPath];
    
    // Setting up the spinner to the right cell
    self.lastSpinningCellIndex = indexPath;
    UIActivityIndicatorView *activityView = [[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleGray];
    [activityView startAnimating];
    [cell setAccessoryView:activityView];
    
    if (indexPath.section == 1){
        NSLog(@"Connecting to %@:%@", [[self.comManager.servers objectAtIndex:indexPath.row] serverName], [[self.comManager.servers objectAtIndex:indexPath.row] serverAddress]);
        [self.comManager connectToServer:[self.comManager.servers objectAtIndex:indexPath.row]];
    } else if (indexPath.section == 0){
        NSLog(@"Connecting to %@", [[self.comManager.autoDiscoveryServers objectAtIndex:indexPath.row] name]);
        [[self.comManager.autoDiscoveryServers objectAtIndex:indexPath.row] resolveWithTimeout:0.0];
    }
}

- (void)viewDidUnload {
    [self setServerTable:nil];
    [self setMenuButton:nil];
    [super viewDidUnload];
}

- (UIView *)tableView:(UITableView *)tableView viewForHeaderInSection:(NSInteger)section {
    
    NSString *sectionName = nil;
    
    switch (section) {
        case 0:
            sectionName = [NSString stringWithFormat:NSLocalizedString(@"Detected Connections", @"Server list section title")];
            break;
        case 1:
            sectionName = [NSString stringWithFormat:NSLocalizedString(@"Custom Connections", @"Server list section title")];
            break;
    }
    UILabel *sectionHeader;
    if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPhone) {
        sectionHeader = [[UILabel alloc] initWithFrame:CGRectMake(10, 10, 310, 40)];
    } else {
        sectionHeader = [[UILabel alloc] initWithFrame:CGRectMake(25, 10, 295, 40)];
    }
    sectionHeader.backgroundColor = [UIColor clearColor];
    sectionHeader.font = kAppTextFont;
    sectionHeader.textColor = [UIColor darkTextColor];
    sectionHeader.text = sectionName;
    UIView *view = [[UIView alloc] initWithFrame:CGRectMake(0, 0, tableView.bounds.size.width, [self tableView:tableView heightForHeaderInSection:section])];
    [view addSubview:sectionHeader];
    
    if (self.comManager.searchState == SEARCHING && section == 0) {
        UIActivityIndicatorView * aiv = [[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleGray];
        aiv.center = CGPointMake([sectionHeader.text sizeWithFont: sectionHeader.font].width + 2 * aiv.frame.size.width, sectionHeader.center.y);
        [aiv startAnimating];
        [aiv setTag:4];
        [view addSubview:aiv];
    }
    return view;
}

- (UIView *)tableView:(UITableView *)tableView viewForFooterInSection:(NSInteger)section {
    if ([self.comManager.servers count] == 0 && section == 1) {
        UILabel *sectionFooter = [[UILabel alloc] initWithFrame:CGRectMake(20, 10, tableView.frame.size.width - 50, 100)];
        [sectionFooter setLineBreakMode:NSLineBreakByWordWrapping];
        [sectionFooter setNumberOfLines:5];
        sectionFooter.backgroundColor = [UIColor clearColor];
        sectionFooter.font = kAppSmallTextFont;
        sectionFooter.textColor = kTextTintColor;
        sectionFooter.text = NSLocalizedString(@"Customize server config instruction", @"Displayed when no customized server is available");
        
        UIView *view = [[UIView alloc] initWithFrame:CGRectMake(0, 0, tableView.bounds.size.width, [self tableView:tableView heightForFooterInSection:section])];
        [view addSubview:sectionFooter];
        return view;
    }
    return nil;
}

- (CGFloat)tableView:(UITableView *)tableView heightForHeaderInSection:(NSInteger)section
{
    return 50.0;
}

//- (CGFloat)tableView:(UITableView *)tableView heightForFooterInSection:(NSInteger)section
//{
////    return 50.0;
//}

-(NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 2;
}

-(NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    switch (section) {
        case 0:
            return [self.comManager.autoDiscoveryServers count] == 0 ? 1 : [self.comManager.autoDiscoveryServers count];
            break;
        case 1:
            return [self.comManager.servers count];
        default:
            return -1;
            break;
    }
}

-(void)revealHelpInfo:(UIView *)sender
{
    if (sender) {
        if ([[CommunicationManager fetchSSIDInfo] valueForKey:@"SSID"])
            [PopoverView showPopoverAtPoint:CGPointMake(sender.center.x,
                                                        sender.frame.origin.y + sender.frame.size.height/2)
                                     inView:[sender superview]
                                   withText:[NSString
                                             stringWithFormat:NSLocalizedString(@"* Launch Impress on your computer\n* Connect both devices to the same WiFi network\n* P.S. Your iOS device is connected to \"%@\" now", nil), [[CommunicationManager fetchSSIDInfo] valueForKey:@"SSID"]]
                                   delegate:nil];
        else {
            [PopoverView showPopoverAtPoint:CGPointMake(sender.center.x,
                                                        sender.frame.origin.y + sender.frame.size.height/2)
                                     inView:[sender superview]
                                   withText:[NSString
                                             stringWithFormat:NSLocalizedString(@"* You don't have a WiFi connection now.\n* Connect your iOS device and your computer to the same network\n* Refresh\n* hint: you may create a personal hotspot on your computer/iPhone", nil), [[CommunicationManager fetchSSIDInfo] valueForKey:@"SSID"]]
                                   delegate:nil];
        }
    }
}

-(UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    static NSString *cellIdentifier = @"server_item_cell";
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:cellIdentifier];
    
    if (indexPath.section == 0){
        if ([self.comManager.autoDiscoveryServers count] == 0){
            cell.textLabel.text = NSLocalizedString(self.searchStateText, nil);
            cell.textLabel.lineBreakMode = UILineBreakModeClip;
            cell.selectionStyle = self.style;
            
            UIButton *infoBtn = [UIButton buttonWithType:UIButtonTypeInfoDark];
            infoBtn.frame = CGRectMake(3, 8, 30, 30);
            [infoBtn addTarget:self action:@selector(revealHelpInfo:) forControlEvents:UIControlEventTouchUpInside];
            
            cell.accessoryView = infoBtn;
        } else {
            id s = [self.comManager.autoDiscoveryServers objectAtIndex:indexPath.row];
            [cell.textLabel setText:[s name]];
            [cell.detailTextLabel setText:@""];
            cell.accessoryView = nil;
        }
    }
    else {
        Server *s = [self.comManager.servers objectAtIndex:indexPath.row];
        
        [cell.textLabel setText:[s serverName]];
        [cell.detailTextLabel setText:[s serverAddress]];
    }
    return cell;
}

- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath
{
    if (indexPath.section == 1)
        return YES;
    else
        return NO;
}

- (UITableViewCellEditingStyle)tableView:(UITableView *)tableView editingStyleForRowAtIndexPath:(NSIndexPath *)indexPath{
    if (indexPath.section == 1) {
        return UITableViewCellEditingStyleDelete;
    } else {
        return UITableViewCellEditingStyleNone;
    }
}

- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath{
    
    if(editingStyle == UITableViewCellEditingStyleDelete){
        [self.comManager removeServerAtIndex:indexPath.row];
        [tableView deleteRowsAtIndexPaths:[NSArray arrayWithObject:indexPath] withRowAnimation:UITableViewRowAnimationFade];
    }
}

@end
