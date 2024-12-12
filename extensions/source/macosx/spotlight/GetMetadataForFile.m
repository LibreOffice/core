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

#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <Foundation/Foundation.h>

#include "GetMetadataForFile.h"
#import "OOoSpotlightAndQuickLookImporter.h"

/* -----------------------------------------------------------------------------
    Get metadata attributes from file

   This function's job is to extract useful information your file format supports
   and return it as a dictionary
   ----------------------------------------------------------------------------- */

Boolean GetMetadataForFile(void* thisInterface,
			   CFMutableDictionaryRef attributes,
			   CFStringRef contentTypeUTI,
			   CFStringRef pathToFile)
{
    (void) thisInterface; /* unused */
    /* Pull any available metadata from the file at the specified path */
    /* Return the attribute keys and attribute values in the dict */
    /* Return TRUE if successful, FALSE if there was no data provided */
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    OOoSpotlightAndQuickLookImporter *importer = [OOoSpotlightAndQuickLookImporter new];

    Boolean importOK = NO;
    @try {
        importOK = [importer importDocument:(NSString*)pathToFile
                                contentType:(NSString*)contentTypeUTI
                                 attributes:(NSMutableDictionary*)attributes];
    }
    @catch (NSException *exception) {
        NSLog(@"main: Caught %@: %@", [exception name], [exception  reason]);
    }

    [importer release];

    [pool release];

    return importOK;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
