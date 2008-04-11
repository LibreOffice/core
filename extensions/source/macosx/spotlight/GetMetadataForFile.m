/*************************************************************************
*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: GetMetadataForFile.m,v $
 * $Revision: 1.3 $
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
*************************************************************************/

#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h> 
#include <Foundation/Foundation.h>


#import "OOoSpotlightImporter.h"


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
    /* Pull any available metadata from the file at the specified path */
    /* Return the attribute keys and attribute values in the dict */
    /* Return TRUE if successful, FALSE if there was no data provided */
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    OOoSpotlightImporter *importer = [OOoSpotlightImporter new];
    
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
