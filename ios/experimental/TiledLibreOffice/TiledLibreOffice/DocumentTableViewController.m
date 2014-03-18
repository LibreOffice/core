// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import <UIKit/UIKit.h>

#import "AppDelegate.h"
#import "DocumentTableViewController.h"

@interface DocumentTableViewController ()
{
    NSArray *documents;
    AppDelegate *appDelegate;
}
@end

@implementation DocumentTableViewController

+ (id)createForDocuments:(NSArray*)documents forAppDelegate:(AppDelegate*)appDelegate;
{
    DocumentTableViewController *result = [[DocumentTableViewController alloc] initWithStyle:UITableViewStylePlain];

    result->documents = documents;
    result->appDelegate = appDelegate;

    return result;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return [documents count];
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    static NSString *simpleTableIdentifier = @"SimpleTableCell";

    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:simpleTableIdentifier];

    if (cell == nil) {
        cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:simpleTableIdentifier];
    }

    cell.textLabel.text = [[documents objectAtIndex:indexPath.row] lastPathComponent];
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    [appDelegate startDisplaying:[documents objectAtIndex:indexPath.row]];
}

@end

// vim:set shiftwidth=4 softtabstop=4 expandtab:
