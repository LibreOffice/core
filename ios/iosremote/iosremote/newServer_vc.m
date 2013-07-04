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
#import "serverList_vc.h"
#import "Server.h"

@implementation newServerViewController

@synthesize server = _server;

@synthesize nameCell = _nameCell;
@synthesize addrCell = _addrCell;

- (IBAction)save:(id)sender {
    NSString *serverName = [self.nameCell.textField text];
    NSString *serverAddr = [self.addrCell.textField text];
    if ([serverAddr isValidIPAddress]) {
        if (!serverName)
            serverName = @"Computer";
        NSLog(@"New server name:%@ ip:%@", serverName, serverAddr);
        [self.comManager addServersWithName:serverName AtAddress:serverAddr];
        [self.navigationController popViewControllerAnimated:YES];
    } else {
        UIAlertView *message = [[UIAlertView alloc] initWithTitle:@"Invalid IP Address"
                                                          message:@"A valid IP address should be like this: \"192.168.1.1\""
                                                         delegate:nil
                                                cancelButtonTitle:@"OK"
                                                otherButtonTitles:nil];
        [message show];
    }
    NSLog(@"New server name:%@ ip:%@", serverName, serverAddr);
    [self.comManager addServersWithName:serverName AtAddress:serverAddr];
    [self.navigationController popViewControllerAnimated:YES];
}

- (BOOL)isModal
{
    NSArray *viewControllers = [[self navigationController] viewControllers];
    UIViewController *rootViewController = [viewControllers objectAtIndex:0];
    
    return rootViewController == self;
}

- (EditableTableViewCell *)newDetailCellWithTag:(NSInteger)tag
{
    EditableTableViewCell *cell = [[EditableTableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:nil];
    
    [[cell textField] setDelegate:self];
    [[cell textField] setTag:tag];
    
    return cell;
}

#pragma mark -
#pragma mark Action Methods

- (void)save
{
    [self dismissModalViewControllerAnimated:YES];
}

- (void)cancel
{
    [self dismissModalViewControllerAnimated:YES];
}

#pragma mark -
#pragma mark UIViewController Methods

- (void)viewDidLoad
{
    //  If the user clicked the '+' button in the list view, we're
    //  creating a new entry rather than modifying an existing one, so
    //  we're in a modal nav controller. Modal nav controllers don't add
    //  a back button to the nav bar; instead we'll add Save and
    //  Cancel buttons.
    //
    if ([self isModal])
    {
        UIBarButtonItem *saveButton = [[UIBarButtonItem alloc]
                                       initWithBarButtonSystemItem:UIBarButtonSystemItemSave
                                       target:self
                                       action:@selector(save)];
        
        [[self navigationItem] setRightBarButtonItem:saveButton];
        
        UIBarButtonItem *cancelButton = [[UIBarButtonItem alloc]
                                         initWithBarButtonSystemItem:UIBarButtonSystemItemCancel
                                         target:self
                                         action:@selector(cancel)];
        
        [[self navigationItem] setLeftBarButtonItem:cancelButton];
    }
    self.comManager = [CommunicationManager sharedComManager];
    [self setNameCell: [self newDetailCellWithTag:ServerName]];
    [self setAddrCell: [self newDetailCellWithTag:ServerAddr]];
}

//  Override this method to automatically place the insertion point in the
//  first field.
//
- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
    
    NSUInteger indexes[] = { 0, 0 };
    NSIndexPath *indexPath = [NSIndexPath indexPathWithIndexes:indexes
                                                        length:2];
    
    EditableTableViewCell *cell = (EditableTableViewCell *)[[self tableView]
                                                      cellForRowAtIndexPath:indexPath];
    
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

#pragma mark -
#pragma mark UITextFieldDelegate Protocol

//  Sets the label of the keyboard's return key to 'Done' when the insertion
//  point moves to the table view's last field.
//
- (BOOL)textFieldShouldBeginEditing:(UITextField *)textField
{
    if ([textField tag] == ServerAddr)
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
    static NSNumberFormatter *_formatter;
    
    if (_formatter == nil)
    {
        _formatter = [[NSNumberFormatter alloc] init];
    }
    
    NSString *text = [textField text];
    
    switch ([textField tag])
    {
        case ServerName:     [self.server setServerName:text];          break;
        case ServerAddr:  [self.server setServerAddress:text];       break;
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
    else if ([self isModal])
    {
        //  We're in a modal navigation controller, which means the user is
        //  adding a new book rather than editing an existing one.
        //
        [self save];
    }
    else
    {
        [[self navigationController] popViewControllerAnimated:YES];
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
    return section == 0 ? 2 : 1;
}

- (NSString *)tableView:(UITableView *)tableView
titleForHeaderInSection:(NSInteger)section
{
    switch (section)
    {
        case InformationSection:  return nil;
    }    
    return nil;
}

- (UITableViewCell *)tableView:(UITableView *)tableView
         cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    EditableTableViewCell *cell = nil;
    NSInteger tag = INT_MIN;
    NSString *text = nil;
    NSString *placeholder = nil;
    UIKeyboardType keyboardType;
    
    //  Pick the editable cell and the values for its textField
    //
    NSUInteger section = [indexPath section];
    switch (section)
    {
        case InformationSection:
        {
            if ([indexPath row] == 0)
            {
                cell = [self nameCell];
                text = [self.server serverName];
                tag = ServerName;
                placeholder = @"Server Name (optional)";
                keyboardType = UIKeyboardTypeDefault;
            }
            else
            {
                cell = [self addrCell];
                text = [self.server serverAddress];
                tag = ServerAddr;
                placeholder = @"IP Address";
                keyboardType = UIKeyboardTypeNumbersAndPunctuation;
            }
            break;
        }
    }
    [cell.textField setPlaceholder:placeholder];
    [cell setTag:tag];
    [cell.textField setText:text];
<<<<<<< HEAD
    [cell.textField setKeyboardType:keyboardType];
=======
>>>>>>> connect to server list implemented
    return cell;
}

@end

