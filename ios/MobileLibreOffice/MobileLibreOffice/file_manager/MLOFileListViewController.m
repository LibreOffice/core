// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOFileListViewController.h"
#import "MLOFileManagerViewController_Impl.h"
#import "MLOFileCacheManager.h"
#import "MLOCachedFile.h"

@interface MLOFileListViewController ()
@property MLOFileManagerViewController * fileManager;
@property UITableView * list;
@end

@implementation MLOFileListViewController

-(id)initWithFileManager:(MLOFileManagerViewController *)fileManager{
    self = [self initWithStyle:UITableViewStylePlain];

    if(self){
        self.fileManager = fileManager;
        
        self.list = [UITableView new];
        
        self.view.backgroundColor = [UIColor clearColor];
        
        [self reloadData];
    }
    return self;
}

-(void)onRotate{
    
    self.list.frame = [self.fileManager currentFullscreenFrame];
}

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 1;
}

-(void)reloadData{
    [self.list reloadData];
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return [self.fileManager.cache count];
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    return [self.fileManager.cache cellForTableView:tableView atIndexPath:indexPath];
}

- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath
{
    return YES;
}

-(BOOL)hasFiles{
    return [self.fileManager.cache count] > 0;
}

- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath
{
    if (editingStyle == UITableViewCellEditingStyleDelete) {
        [self.fileManager.cache deleteIndexPath:indexPath];
        [tableView deleteRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationFade];
        
        if(![self hasFiles]){
            [self.fileManager reloadData];
        }
    }   
}


- (void)tableView:(UITableView *)tableView accessoryButtonTappedForRowWithIndexPath:(NSIndexPath *)indexPath{
    [self.fileManager.cache sendFileAtIndexPath:indexPath];
}

#pragma mark - Table view delegate

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    [self.fileManager.cache openFileAtIndex:indexPath.row];
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
}

@end
