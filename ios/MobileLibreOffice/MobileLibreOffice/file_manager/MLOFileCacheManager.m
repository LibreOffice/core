// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOFileCacheManager.h"
#import "MLOFileManagerViewController_Impl.h"
#import "MLOCachedFile.h"
#import "MLOAppViewController.h"
#import "MLOAppDelegate.h"
#import "MLOManager.h"
#import "NSObject+MLOUtils.h"
#import "NSObject+MLOFileUtils.h"

#define CACHED_DATA_FILE_NAME @"mlo_cache_data"

static const NSTimeInterval DELAY_BEFORE_OPENEING_FIRST_DOCUMENT = 2.0f;

@interface MLOFileCacheManager ()
@property NSMutableArray * files;
@property NSString *dbFilePath;
@end

static NSString * OpenedFilesCountKey = @"openedFiles";
static NSString * CachedFilesKey = @"cachedFiles";

@implementation MLOFileCacheManager

-(id)initWithFileManager:(MLOFileManagerViewController *) fileManager{
    self = [self init];
    if(self){
        self.fileManager =fileManager;
        self.dbFilePath =  [self cachedFilePath:CACHED_DATA_FILE_NAME];
        [self load];
    }
    return self;
}

-(void)loadFile{
    
    NSDictionary * dictionary = [NSDictionary dictionaryWithContentsOfFile:self.dbFilePath];
    
    [MLOCachedFile setCachedFilesCount:[((NSNumber *)dictionary[OpenedFilesCountKey]) unsignedIntValue]];
    
    NSArray * array = dictionary[CachedFilesKey];
    
    for (id loadable in array) {
        [self.files addObject:[[MLOCachedFile alloc] initByLoading:loadable]];
    }
}

-(void)load{
    
    self.files = [NSMutableArray new];
    
    if([[NSFileManager defaultManager] fileExistsAtPath:self.dbFilePath]){
        
        [self loadFile];
        
    }else{
        self.files = [NSMutableArray new];
        
        NSString * exampleFileNameWithExtension = @"test1.odt";
      
        [self addFileAndGetNewIndexForPath:
            [[[NSBundle mainBundle] bundlePath] stringByAppendingPathComponent:exampleFileNameWithExtension]];
        
    }
    
    [self.fileManager reloadData];
    
}
-(void)save{
    NSMutableArray * array = [NSMutableArray new];
    
    for (MLOCachedFile * file in self.files) {
        
        [array addObject:[file toSavable]];
        
    }
    
    [@{OpenedFilesCountKey:[NSNumber numberWithUnsignedInt:[MLOCachedFile cachedFilesCount]],
            CachedFilesKey:array}
     
                writeToFile:self.dbFilePath
                 atomically:YES];
    
}

-(NSUInteger)addFileAndGetNewIndexForPath:(NSString *) newFilePath{
    
    NSUInteger nextFileIndex = self.files.count;
    
    MLOCachedFile * file = [[MLOCachedFile alloc] initWithOriginFilePath:newFilePath];
    
    if([file exists]){
    
        [self.files addObject:file];
        
        [self save];
        
        [self.fileManager reloadData];
    }else{
        nextFileIndex = self.files.count;
    }
    return nextFileIndex;
}

-(NSUInteger)count{
    return self.files.count;
}

-(MLOCachedFile *)getFileAtIndex:(NSUInteger) index{
    return self.files[index];
}

-(void)deleteFileAtIndex:(NSUInteger)index{
    
    MLOCachedFile * toDelete = [self getFileAtIndex:index];
    
    if([toDelete deleteFile]){
        [self.files removeObject:toDelete];
        [self save];
    }
}

-(void)deleteIndexPath:(NSIndexPath*) indexPath{
    [self deleteFileAtIndex:indexPath.row];
}

-(UITableViewCell*)cellForTableView:(UITableView *)tableView atIndexPath:(NSIndexPath *)indexPath{
    static NSString *CellIdentifier = @"Cell";

    UITableViewCell * cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleSubtitle reuseIdentifier:CellIdentifier];
    
    MLOCachedFile * file = [self getFileAtIndex:indexPath.row];
    
    cell.textLabel.text =file.cachedFileNameWithExtension;
    cell.detailTextLabel.text = [NSDateFormatter localizedStringFromDate:[file lastModified]
                                                                  dateStyle:NSDateFormatterShortStyle
                                                                  timeStyle:NSDateFormatterFullStyle];
    cell.accessoryType =UITableViewCellAccessoryDetailDisclosureButton;
    
    return cell;
}

-(void)openFilePath:(NSString *) filePath{
    [self openFileAtIndex:[self addFileAndGetNewIndexForPath:filePath]];
}
-(void)openFileAtIndex:(NSUInteger) index{
    if(index != self.files.count){
        MLOCachedFile * file = [self getFileAtIndex:index];
        
        [self performBlock:^{
        
            [[MLOManager getInstance] openInLibreOfficeFilePath:[file cachedFilePath]
                                          fileNameWithExtension:[file cachedFileNameWithExtension]
                                                      superView:self.fileManager.appViewController.view
                                                         window:self.fileManager.appViewController.appDelegate.window
                                                        invoker:self.fileManager.appViewController.appDelegate];
            
        } afterDelay:DELAY_BEFORE_OPENEING_FIRST_DOCUMENT];
        
    }
}

-(void)sendFileAtIndexPath:(NSIndexPath *) indexPath{
    
    MLOCachedFile * file = [self getFileAtIndex:indexPath.row];
    
    MFMailComposeViewController *mailer = [MFMailComposeViewController new];
    
    mailer.mailComposeDelegate = self;
    [mailer setMessageBody:@"Best" isHTML:NO];
    [mailer setSubject:file.cachedFileNameWithExtension];
    
    NSData *myData =[[NSData alloc] initWithContentsOfFile:file.cachedFilePath];
    [mailer addAttachmentData:myData mimeType:@"iapplication/msword" fileName:file.cachedFileNameWithExtension];
    
    [self.fileManager presentViewController:mailer animated:YES completion:nil];
    
}
- (void)mailComposeController:(MFMailComposeViewController*)controller didFinishWithResult:(MFMailComposeResult)result error:(NSError*)error
{
    switch (result)
    {
        case MFMailComposeResultCancelled:
            NSLog(@"Sending file: canceled");
            break;
        case MFMailComposeResultSaved:
            NSLog(@"Sending file: saved");
            break;
        case MFMailComposeResultSent:
            NSLog(@"Sending file: sent");
            break;
        case MFMailComposeResultFailed:
            NSLog(@"Sending file: failed");
            break;
        default:
            NSLog(@"Sending file: not sent");
            break;
    }
    [self.fileManager dismissViewControllerAnimated:YES completion:nil];
}


@end
