/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
