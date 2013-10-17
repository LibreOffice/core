// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOCachedFile.h"
#import "MLOFileCacheManager.h"
#import "MLOManager.h"
#import "NSObject+MLOFileUtils.h"
#import "NSObject+MLOUtils.h"

static const NSString * CACHED_FILE_PATH_KEY                   =   @"cahcedPath";
static const NSString * CACHED_FILE_NAME_WITH_EXTENSION_KEY    =   @"cachedName";

@interface MLOCachedFile ()
@property NSString * cachedFilePath;
@property NSString * cachedFileNameWithExtension;
@end

static NSUInteger cachedFilesCounter = 0;
@implementation MLOCachedFile


-(BOOL)createByCachingFilePath:(NSString *) originFilePath{
    self.cachedFileNameWithExtension = [originFilePath lastPathComponent];
    
    NSString * newFileName = [NSString stringWithFormat:@"%d%@",cachedFilesCounter++,self.cachedFileNameWithExtension];
    self.cachedFilePath = [self cachedFilePath:newFileName];
    
    NSError * error;
    
    return [self returnSuccees:[[NSFileManager defaultManager] copyItemAtPath:originFilePath
                                                                       toPath:self.cachedFilePath
                                                                        error:&error]
                     forAction:[NSString stringWithFormat:@"create cached file by copying %@ to %@",
                                originFilePath,
                                self.cachedFilePath]
                         andAlertForError:error];
}

+(NSUInteger)cachedFilesCount{
    return  cachedFilesCounter;
}

+(void)setCachedFilesCount:(NSUInteger) count{
    cachedFilesCounter = count;    
}

-(id)initWithOriginFilePath:(NSString *) originFilePath{
    
    self = [self init];
    
    if(self){
        
        if(![self createByCachingFilePath:originFilePath]){
            self = nil;
        }
    }
    return self;
}

-(id)initByLoading:(id) source{
    self = [self init];
    if(self){
        NSDictionary * dictionary = source;
        self.cachedFilePath = dictionary[CACHED_FILE_PATH_KEY];
        self.cachedFileNameWithExtension = dictionary[CACHED_FILE_NAME_WITH_EXTENSION_KEY];
    }
    return self;
}

-(id) toSavable{
    return @{                CACHED_FILE_PATH_KEY : self.cachedFilePath,
              CACHED_FILE_NAME_WITH_EXTENSION_KEY : self.cachedFileNameWithExtension};
}

-(BOOL)deleteFile{
    NSError * error;
    
    return [self returnSuccees:[[NSFileManager defaultManager] removeItemAtPath:self.cachedFilePath
                                                                          error:&error]
                     forAction:[NSString stringWithFormat:@"delete the cached file %@",self.cachedFilePath]
                         andAlertForError:error];

}
-(NSDictionary *) fileAttrributes{
    return [[NSFileManager defaultManager] attributesOfItemAtPath:self.cachedFilePath error:nil];
}
-(NSDate *) lastModified{
    return[[self fileAttrributes] fileModificationDate];
}

-(BOOL)exists{
    return [[NSFileManager defaultManager] fileExistsAtPath:self.cachedFilePath];
}
@end
