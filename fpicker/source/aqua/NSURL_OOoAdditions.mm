/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



#ifndef _NSSTRING_OOOADDITIONS_HXX_
#include "NSString_OOoAdditions.hxx"
#endif

#include "NSURL_OOoAdditions.hxx"

@implementation NSURL (OOoAdditions)
- (rtl::OUString) OUStringForInfo:(InfoType)info
{
    NSAutoreleasePool *pool = [NSAutoreleasePool new];

    NSString *sURLString = nil;

    switch(info) {
        case FULLPATH:
            OSL_TRACE("Extracting the full path of an item");
            sURLString = [self absoluteString];
            [sURLString retain];
            break;
        case FILENAME:
            OSL_TRACE("Extracting the file name of an item");
            NSString *path = [self path];
            if (path == nil) {
                sURLString = @"";
            }
            else {
                sURLString = [path lastPathComponent];
            }
            [sURLString retain];
            break;
        case PATHWITHOUTLASTCOMPONENT:
            OSL_TRACE("Extracting the last but one component of an item's path");
            path = [self absoluteString];
            if (path == nil) {
                sURLString = @"";
            }
            else {
                NSString* lastComponent = [path lastPathComponent];
                unsigned int lastLength = [lastComponent length];
                sURLString = [path substringToIndex:([path length] - lastLength)];
            }
            [sURLString retain];
            break;
        default:
            break;
    }

    rtl::OUString sResult = [sURLString OUString];
    [sURLString release];

    [pool release];

    return sResult;
}
@end

NSString* resolveAlias( NSString* i_pSystemPath )
{
    NSString* pResolvedPath = nil;
    CFURLRef rUrl = CFURLCreateWithFileSystemPath( kCFAllocatorDefault,
                                                   (CFStringRef)i_pSystemPath,
                                                   kCFURLPOSIXPathStyle, false);
    if( rUrl != NULL )
    {
        FSRef rFS;
        if( CFURLGetFSRef( rUrl, &rFS ) )
        {
            Boolean bIsFolder = false;
            Boolean bAlias = false;
            OSErr err = FSResolveAliasFile( &rFS, true, &bIsFolder, &bAlias);
            if( (err == noErr) && bAlias )
            {
                CFURLRef rResolvedUrl = CFURLCreateFromFSRef( kCFAllocatorDefault, &rFS );
                if( rResolvedUrl != NULL )
                {
                    pResolvedPath = (NSString*)CFURLCopyFileSystemPath( rResolvedUrl, kCFURLPOSIXPathStyle );
                    CFRelease( rResolvedUrl );
                }
            }
        }
        CFRelease( rUrl );
    }
    
    return pResolvedPath;
}
