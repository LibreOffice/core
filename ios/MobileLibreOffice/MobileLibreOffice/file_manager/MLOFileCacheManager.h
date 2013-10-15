// -*- Mode: Objective-C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOObject.h"
#import <MessageUI/MFMailComposeViewController.h>

@class MLOFileManagerViewController,MLOCachedFile;

@interface MLOFileCacheManager : MLOObject<MFMailComposeViewControllerDelegate>

@property MLOFileManagerViewController * fileManager;

-(id)initWithFileManager:(MLOFileManagerViewController *) fileManager;
-(NSUInteger)count;
-(void)deleteIndexPath:(NSIndexPath *) indexPath;
-(UITableViewCell*)cellForTableView:(UITableView *)tableView atIndexPath:(NSIndexPath *)indexPath;
-(void)openFilePath:(NSString *) filePath;
-(void)openFileAtIndex:(NSUInteger) index;
-(void)sendFileAtIndexPath:(NSIndexPath *) indexPath;

@end
