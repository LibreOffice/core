// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "NSObject+MLOFileUtils.h"

@implementation NSObject (MLOFileUtils)
-(NSString *)cachedFilePath:(NSString *) lastPathComponent{
    
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    
    NSString *documentsDirectory = [paths objectAtIndex:0];
    
    BOOL isDir;
    
    NSFileManager * fileManager = [NSFileManager defaultManager];
    
    if (! [fileManager fileExistsAtPath:documentsDirectory isDirectory:&isDir]) {
    
        NSError * error;
        
        BOOL success = [fileManager createDirectoryAtPath:documentsDirectory withIntermediateDirectories:NO attributes:nil error:&error];
        
        if (!success || error) {
            NSLog(@"Error: %@", [error localizedDescription]);
        }
    }
    
    return [documentsDirectory stringByAppendingPathComponent:lastPathComponent];
}
@end
