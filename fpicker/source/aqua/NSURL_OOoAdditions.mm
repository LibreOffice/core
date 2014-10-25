/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "NSString_OOoAdditions.hxx"
#include "NSURL_OOoAdditions.hxx"
#include "sal/log.hxx"

@implementation NSURL (OOoAdditions)
- (rtl::OUString) OUStringForInfo:(InfoType)info
{
    NSAutoreleasePool *pool = [NSAutoreleasePool new];

    NSString *sURLString = nil;

    switch(info) {
        case FULLPATH:
            SAL_INFO("fpicker.aqua","Extracting the full path of an item");
            sURLString = [self absoluteString];
            [sURLString retain];
            break;
        case FILENAME:
            {
                SAL_INFO("fpicker.aqua","Extracting the file name of an item");
                NSString *path = [self path];
                if (path == nil) {
                    sURLString = @"";
                }
                else {
                    sURLString = [path lastPathComponent];
                }
                [sURLString retain];
            }
            break;
        case PATHWITHOUTLASTCOMPONENT:
            {
                SAL_INFO("fpicker.aqua","Extracting the last but one component of an item's path");
                NSString *path = [self absoluteString];
                if (path == nil) {
                    sURLString = @"";
                }
                else {
                    NSString* lastComponent = [path lastPathComponent];
                    unsigned int lastLength = [lastComponent length];
                    sURLString = [path substringToIndex:([path length] - lastLength)];
                }
                [sURLString retain];
            }
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
                                                   reinterpret_cast<CFStringRef>(i_pSystemPath),
                                                   kCFURLPOSIXPathStyle, false);
    if( rUrl != nullptr )
    {
#if MACOSX_SDK_VERSION < 1060
        FSRef rFS;
        if( CFURLGetFSRef( rUrl, &rFS ) )
        {
            Boolean bIsFolder = false;
            Boolean bAlias = false;
            OSErr err = FSResolveAliasFile( &rFS, true, &bIsFolder, &bAlias );
            if( (err == noErr) && bAlias )
            {
                CFURLRef rResolvedUrl = CFURLCreateFromFSRef( kCFAllocatorDefault, &rFS );
                if( rResolvedUrl != nullptr )
                {
                    pResolvedPath = const_cast<NSString*>(reinterpret_cast<NSString const *>(CFURLCopyFileSystemPath( rResolvedUrl, kCFURLPOSIXPathStyle )));
                    CFRelease( rResolvedUrl );
                }
            }
        }
        CFRelease( rUrl );
#else
        CFErrorRef rError;
        CFDataRef rBookmark = CFURLCreateBookmarkDataFromFile( nullptr, rUrl, &rError );
        CFRelease( rUrl );
        if( rBookmark != nullptr )
        {
            Boolean bIsStale;
            CFURLRef rResolvedUrl = CFURLCreateByResolvingBookmarkData( kCFAllocatorDefault, rBookmark, kCFBookmarkResolutionWithoutUIMask,
                                                                        nullptr, nullptr, &bIsStale, &rError );
            CFRelease( rBookmark );
            if( rResolvedUrl == nullptr )
            {
                CFRelease( rError );
            }
            else
            {
                pResolvedPath = const_cast<NSString*>(reinterpret_cast<NSString const *>(CFURLCopyFileSystemPath( rResolvedUrl, kCFURLPOSIXPathStyle )));
                CFRelease( rResolvedUrl );
            }
        }
#endif
    }
    
    return pResolvedPath;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
