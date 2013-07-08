//
//  slideShowSwipeInList.h
//  iosremote
//
//  Created by Liu Siqi on 7/8/13.
//  Copyright (c) 2013 libreoffice. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface slideShowSwipeInList : UITableViewController <UITableViewDataSource, UITableViewDelegate>

@property (strong, nonatomic) IBOutlet UITableView *slidesTable;

@end
