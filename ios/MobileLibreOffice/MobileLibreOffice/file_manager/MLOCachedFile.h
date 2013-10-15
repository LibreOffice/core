// -*- Mode: Objective-C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOObject.h"

@interface MLOCachedFile : MLOObject

-(id)initWithOriginFilePath:(NSString *) originFilePath;
-(id)initByLoading:(id) source;
-(id)toSavable;
-(BOOL)deleteFile;
-(BOOL)exists;
-(NSString *) cachedFilePath;
-(NSString *) cachedFileNameWithExtension;
-(NSDate *) lastModified;
+(NSUInteger)cachedFilesCount;
+(void)setCachedFilesCount:(NSUInteger) count;
@end
