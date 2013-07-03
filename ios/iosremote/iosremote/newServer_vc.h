
#import <UIKit/UIKit.h>

@class Server;
@class EditableTableViewCell;
@class CommunicationManager;

//  Constants representing the book's fields.
//
enum {
    ServerName,
    ServerAddr,
    ServerStore
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
- (IBAction)save:(id)sender;
- (BOOL)isModal;

- (EditableTableViewCell *)newDetailCellWithTag:(NSInteger)tag;

//  Action Methods
- (void)save;
- (void)cancel;

@end
