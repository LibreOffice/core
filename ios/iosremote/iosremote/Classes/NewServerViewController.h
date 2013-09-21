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

enum {
    ServerAddr,
    ServerName
};

enum {
    InformationSection
};

typedef NSUInteger ServerAttribute;

// View controller used to add new server
@interface NewServerViewController : UITableViewController <UITextFieldDelegate>
@property (nonatomic, strong) Server *server;
@property (nonatomic, strong) EditableTableViewCell *nameCell;
@property (nonatomic, strong) EditableTableViewCell *addrCell;
@property (nonatomic, strong) CommunicationManager *comManager;

@property (weak, nonatomic) IBOutlet UIBarButtonItem *saveButton;

- (IBAction)save:(id)sender;

- (EditableTableViewCell *)newDetailCellWithTag:(NSInteger)tag;

@end
