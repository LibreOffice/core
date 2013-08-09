/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#import <UIKit/UIKit.h>

@class Server;
@class EditableTableViewCell;
@class CommunicationManager;

//  Constants representing the book's fields.
//
enum {
    ServerAddr,
    ServerName
};

//  Constants representing the various sections of our grouped table view.
//
enum {
    InformationSection
};

typedef NSUInteger ServerAttribute;

@interface newServerViewController : UITableViewController <UITextFieldDelegate>
@property (nonatomic, strong) Server *server;
@property (nonatomic, strong) EditableTableViewCell *nameCell;
@property (nonatomic, strong) EditableTableViewCell *addrCell;
@property (nonatomic, strong) CommunicationManager *comManager;

@property (weak, nonatomic) IBOutlet UIBarButtonItem *saveButton;

- (IBAction)save:(id)sender;

- (EditableTableViewCell *)newDetailCellWithTag:(NSInteger)tag;

@end
