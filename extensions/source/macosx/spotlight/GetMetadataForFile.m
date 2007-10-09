/*************************************************************************
*
*   OpenOffice.org - a multi-platform office productivity suite
*
*   $RCSfile: GetMetadataForFile.m,v $
*
*   $Revision: 1.2 $
*
*   last change: $Author: kz $ $Date: 2007-10-09 15:04:18 $
*
*   The Contents of this file are made available subject to
*   the terms of GNU Lesser General Public License Version 2.1.
*
*
*     GNU Lesser General Public License Version 2.1
*     =============================================
*     Copyright 2005 by Sun Microsystems, Inc.
*     901 San Antonio Road, Palo Alto, CA 94303, USA
*
*     This library is free software; you can redistribute it and/or
*     modify it under the terms of the GNU Lesser General Public
*     License version 2.1, as published by the Free Software Foundation.
*
*     This library is distributed in the hope that it will be useful,
*     but WITHOUT ANY WARRANTY; without even the implied warranty of
*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*     Lesser General Public License for more details.
*
*     You should have received a copy of the GNU Lesser General Public
*     License along with this library; if not, write to the Free Software
*     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
*     MA  02111-1307  USA
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
