// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "newServer_vc.h"
#import "CommunicationManager.h"
#import "EditableTableViewCell.h"
#import "UIViewController+LibOStyling.h"
#import "serverList_vc.h"
#import "Server.h"
#import <SystemConfiguration/CaptiveNetwork.h>

@implementation newServerViewController

@synthesize server = _server;

@synthesize nameCell = _nameCell;
@synthesize addrCell = _addrCell;

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation
{
    if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad)
        return YES;
    else {
        return toInterfaceOrientation == UIInterfaceOrientationMaskPortrait;
    }
}

- (IBAction)save:(id)sender {
    NSString *serverName = [self.nameCell.textField text];
    NSString *serverAddr = [self.addrCell.textField text];
    if ([serverAddr isValidIPAddress]) {
        if (!serverName || [serverName isEqualToString:@""]) {
            serverName = [[self fetchSSIDInfo] valueForKey:@"SSID"];
        }
        NSLog(@"New server name:%@ ip:%@", serverName, serverAddr);
        [self.comManager addServersWithName:serverName AtAddress:serverAddr];
        [self.navigationController popViewControllerAnimated:YES];
    } else {
        UIAlertView *message = [[UIAlertView alloc] initWithTitle:NSLocalizedString(@"Invalid IP Address", @"Invalid IP Address Alert title")
                                                          message:NSLocalizedString(@"Invalid IP Address Msg", @"Invalid IP Address Alert message")
                                                         delegate:nil
                                                cancelButtonTitle:NSLocalizedString(@"OK", @"OK Button on Alert view")
                                                otherButtonTitles:nil];
        [message show];
    }
}

- (EditableTableViewCell *)newDetailCellWithTag:(NSInteger)tag
{
    EditableTableViewCell *cell = [[EditableTableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:nil];
    
    [[cell textField] setDelegate:self];
    [[cell textField] setTag:tag];
    
    return cell;
}

#pragma mark -
#pragma mark UIViewController Methods

- (id)fetchSSIDInfo {
    NSArray *ifs = (__bridge_transfer id)CNCopySupportedInterfaces();
    id info = nil;
    for (NSString *ifnam in ifs) {
        info = (__bridge_transfer id)CNCopyCurrentNetworkInfo((__bridge CFStringRef)ifnam);
        NSLog(@"%@ => %@", ifnam, info);
        if (info && [info count]) { break; }
    }
    return info;
}

- (void) handleBack
{
    [self.navigationController popViewControllerAnimated:YES];
}

- (void)viewDidLoad
{
    self.comManager = [CommunicationManager sharedComManager];
    [self setNameCell:[self newDetailCellWithTag:ServerName]];
    [self setAddrCell:[self newDetailCellWithTag:ServerAddr]];
    
    if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad)
        [self setTitle:NSLocalizedString(@"New Server", nil)];
    
    UIBarButtonItem *backButton = [[UIBarButtonItem alloc] initWithTitle:NSLocalizedString(@"Cancel", nil) style:UIBarButtonItemStyleBordered target:self action:@selector(handleBack)];
    [backButton setBackgroundImage:[UIImage imageNamed:@"backButton"] forState:UIControlStateNormal barMetrics:UIBarMetricsDefault];
    self.navigationItem.leftBarButtonItem = backButton;
    
}

//  Override this method to automatically place the insertion point in the
//  first field.
//
- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];

    [self.comManager setDelegate:self];
}

- (void)viewDidAppear:(BOOL)animated
{
    EditableTableViewCell *cell = (EditableTableViewCell *)[[self tableView]
                                                            cellForRowAtIndexPath:[NSIndexPath indexPathForRow:0 inSection:0]];
    
    [[cell textField] becomeFirstResponder];
}

//  Force textfields to resign firstResponder so that our implementation of
//  -textFieldDidEndEditing: gets called. That will ensure that the current
//  UI values are flushed to our model object before we return to the list view.
//
- (void)viewWillDisappear:(BOOL)animated
{
    [super viewWillDisappear:animated];
    
    for (NSUInteger section = 0; section < [[self tableView] numberOfSections]; section++)
    {
        for (NSUInteger row = 0; row < [[self tableView] numberOfRowsInSection:section]; row++)
        {
            NSUInteger indexes[] = { section, row };
            NSIndexPath *indexPath = [NSIndexPath indexPathWithIndexes:indexes
                                                                length:2];
            
            EditableTableViewCell *cell = (EditableTableViewCell *)[[self tableView]
                                                              cellForRowAtIndexPath:indexPath];
            if ([[cell textField] isFirstResponder])
            {
                [[cell textField] resignFirstResponder];
            }
        }
    }
}

- (BOOL)disablesAutomaticKeyboardDismissal {
    return NO;
}

#pragma mark -
#pragma mark UITextFieldDelegate Protocol

//  Sets the label of the keyboard's return key to 'Done' when the insertion
//  point moves to the table view's last field.
//
- (BOOL)textFieldShouldBeginEditing:(UITextField *)textField
{
    if ([textField tag] == ServerName)
    {
        [textField setReturnKeyType:UIReturnKeyDone];
    }
    
    return YES;
}

//  UITextField sends this message to its delegate after resigning
//  firstResponder status. Use this as a hook to save the text field's
//  value to the corresponding property of the model object.
//
- (void)textFieldDidEndEditing:(UITextField *)textField
{    
    NSString *text = [textField text];
    
    switch ([textField tag])
    {
        case ServerName:     [self.server setServerName:text];          break;
        case ServerAddr:     [self.server setServerAddress:text];       break;
    }
}

//  UITextField sends this message to its delegate when the return key
//  is pressed. Use this as a hook to navigate back to the list view
//  (by 'popping' the current view controller, or dismissing a modal nav
//  controller, as the case may be).
//
//  If the user is adding a new item rather than editing an existing one,
//  respond to the return key by moving the insertion point to the next cell's
//  textField, unless we're already at the last cell.
//
- (BOOL)textFieldShouldReturn:(UITextField *)textField
{
    if ([textField returnKeyType] != UIReturnKeyDone)
    {
        //  If this is not the last field (in which case the keyboard's
        //  return key label will currently be 'Next' rather than 'Done'),
        //  just move the insertion point to the next field.
        //
        //  (See the implementation of -textFieldShouldBeginEditing: above.)
        //
        NSInteger nextTag = [textField tag] + 1;
        UIView *nextTextField = [[self tableView] viewWithTag:nextTag];
        
        [nextTextField becomeFirstResponder];
    }
    else
    {
        [self save:nil];
    }
    
    return YES;
}

#pragma mark -
#pragma mark UITableViewDataSource Protocol

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView
 numberOfRowsInSection:(NSInteger)section
{
    return 2;
}

- (UITableViewCell *)tableView:(UITableView *)tableView
         cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    EditableTableViewCell *cell = nil;
    NSString *text = nil;
    NSString *placeholder = nil;
    UIKeyboardType keyboardType = UIKeyboardTypeDefault;
    
    //  Pick the editable cell and the values for its textField
    //
    NSUInteger section = [indexPath section];
    switch (section)
    {
        case InformationSection:
        {
            if ([indexPath row] == 1)
            {
                cell = [self nameCell];
                text = [self.server serverName];
                placeholder = NSLocalizedString(@"Name (optional)", @"EditableTableViewCell placeholder");
                keyboardType = UIKeyboardTypeDefault;
            }
            else
            {
                cell = [self addrCell];
                text = [self.server serverAddress];
                placeholder = NSLocalizedString(@"IP Address", @"EditableTableViewCell placeholder");
                keyboardType = UIKeyboardTypeNumbersAndPunctuation;
            }
            break;
        }
    }
    [cell.textField setPlaceholder:placeholder];
    [cell.textField setText:text];
    [cell.textField setKeyboardType:keyboardType];
    return cell;
}

- (void)viewDidUnload {
    [self setSaveButton:nil];
    [super viewDidUnload];
}
@end

