
#import <UIKit/UIKit.h>
#import "MyListController.h"

@class Book;
@class EditableTableViewCell;

//  Constants representing the book's fields.
//
enum {
    BookTitle,
    BookLastName,
    BookFirstName,
    BookYear,
    BookImagePath
};

//  Constants representing the various sections of our grouped table view.
//
enum {
    TitleSection,
    AuthorSection,
    YearSection,
    ImageSection
};

typedef NSUInteger BookAttribute;

@interface MyDetailController : UITableViewController <UITextFieldDelegate>
{
    Book *_book;
    MyListController *_listController;
    EditableDetailCell *_titleCell;
    EditableDetailCell *_firstNameCell;
    EditableDetailCell *_lastNameCell;
    EditableDetailCell *_yearCell;
    EditableDetailCell *_imagePathCell;
}

@property (nonatomic, retain) Book *book;

//  Watch out for retain cycles (objects that retain each other can never
//  be deallocated). If the list controller were to retain the detail controller,
//  we should change 'retain' to 'assign' below to avoid a cycle. (Note that in
//  that case, dealloc shouldn't release the list controller.)
//
@property (nonatomic, retain) MyListController *listController;

@property (nonatomic, retain) EditableDetailCell *titleCell;
@property (nonatomic, retain) EditableDetailCell *firstNameCell;
@property (nonatomic, retain) EditableDetailCell *lastNameCell;
@property (nonatomic, retain) EditableDetailCell *yearCell;
@property (nonatomic, retain) EditableDetailCell *imagePathCell;

- (BOOL)isModal;

- (EditableDetailCell *)newDetailCellWithTag:(NSInteger)tag;

//  Action Methods
//
- (void)save;
- (void)cancel;

@end
