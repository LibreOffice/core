/*************************************************************************
*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: OOoSpotlightImporter.m,v $
 * $Revision: 1.4 $
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

#import "OOoSpotlightImporter.h"
#import "OOoMetaDataParser.h"
#import "OOoContentDataParser.h"

#define CASESENSITIVITY (0)
#define BUFFER_SIZE (4096)

/* a dictionary to hold the UTIs */
static NSDictionary *uti2kind;

@implementation OOoSpotlightImporter

/* initialize is only called once the first time this class is loaded */
+ (void)initialize
{
    static BOOL isInitialized = NO;
    if (isInitialized == NO) {
        NSMutableDictionary *temp = [NSMutableDictionary new];
        [temp setObject:@"OpenOffice.org 1.0 Text" forKey:@"org.openoffice.text"];
        [temp setObject:@"OpenDocument Text" forKey:@"org.oasis.opendocument.text"];
        [temp setObject:@"OpenOffice.org 1.0 Spreadsheet" forKey:@"org.openoffice.spreadsheet"];
        [temp setObject:@"OpenDocument Spreadsheet" forKey:@"org.oasis.opendocument.spreadsheet"];
        [temp setObject:@"OpenOffice.org 1.0 Presentation" forKey:@"org.openoffice.presentation"];
        [temp setObject:@"OpenDocument Presentation" forKey:@"org.oasis.opendocument.presentation"];
        [temp setObject:@"OpenOffice.org 1.0 Drawing" forKey:@"org.openoffice.graphics"];
        [temp setObject:@"OpenDocument Drawing" forKey:@"org.oasis.opendocument.graphics"];
        [temp setObject:@"OpenOffice.org 1.0 Master" forKey:@"org.openoffice.text-master"];
        [temp setObject:@"OpenDocument Master" forKey:@"org.oasis.opendocument.text-master"];
        [temp setObject:@"OpenOffice.org 1.0 Formula" forKey:@"org.openoffice.formula"];
        [temp setObject:@"OpenDocument Formula" forKey:@"org.oasis.opendocument.formula"];
        [temp setObject:@"OpenOffice.org 1.0 Text Template" forKey:@"org.openoffice.text-template"];
        [temp setObject:@"OpenDocument Text Template" forKey:@"org.oasis.opendocument.text-template"];
        [temp setObject:@"OpenOffice.org 1.0 Spreadsheet Template" forKey:@"org.openoffice.spreadsheet-template"];
        [temp setObject:@"OpenDocument Spreadsheet Template" forKey:@"org.oasis.opendocument.spreadsheet-template"];
        [temp setObject:@"OpenOffice.org 1.0 Presentation Template" forKey:@"org.openoffice.presentation-template"];
        [temp setObject:@"OpenDocument Presentation Template" forKey:@"org.oasis.opendocument.presentation-template"];
        [temp setObject:@"OpenOffice.org 1.0 Drawing Template" forKey:@"org.openoffice.graphics-template"];
        [temp setObject:@"OpenDocument Drawing Template" forKey:@"org.oasis.opendocument.graphics-template"];
        [temp setObject:@"OpenOffice.org 1.0 Database" forKey:@"org.openoffice.database"];
        [temp setObject:@"OpenDocument Chart" forKey:@"org.oasis.opendocument.chart"];
        
        uti2kind = [[NSDictionary dictionaryWithDictionary:temp] retain];
        [temp release];
        
        isInitialized = YES;
    }
}

/* importDocument is the real starting point for our plugin */
- (BOOL)importDocument:(NSString*)pathToFile contentType:(NSString*)contentTypeUTI attributes:(NSMutableDictionary*)attributes
{
    //NSLog(contentTypeUTI);
    //NSLog(pathToFile);
    
    NSString *itemKind = [uti2kind objectForKey:contentTypeUTI];
    if (itemKind != nil) {
        [attributes setObject:itemKind forKey:(NSString*)kMDItemKind];
    }
    
    //first check to see if this is a valid zipped file that contains a file "meta.xml"
    unzFile unzipFile = [self openZipFileAtPath:pathToFile];

    //
    if (unzipFile == nil) {
        //NSLog(@"zip file not open");
        return YES;
    }
    
    //first get the metadata
    NSData *metaData = [self metaDataFileFromZip:unzipFile];
    if (metaData == nil) {
        unzClose(unzipFile);
        return YES;
    }

    [metaData retain];
    
    OOoMetaDataParser *parser = [OOoMetaDataParser new];
    if (parser != nil) {
	//parse and extract the data
	[parser parseXML:metaData intoDictionary:attributes];
    }

    [metaData release];
    [parser release];
    
    //and now get the content
    NSData *contentData = [self contentDataFileFromZip:unzipFile];
    if (contentData == nil) {
        unzClose(unzipFile);
        return YES;
    }
    
    [contentData retain];
    
    OOoContentDataParser *parser2 = [OOoContentDataParser new];
    if (parser2 != nil) {
	//parse and extract the data
	[parser2 parseXML:contentData intoDictionary:attributes];
    }

    [contentData release];
    [parser2 release];

    unzClose(unzipFile);
    
    return YES;
}

/* openZipFileAtPath returns the file as a valid data structure or nil otherwise*/
- (unzFile)openZipFileAtPath:(NSString*)pathToFile
{
    unzFile unzipFile = nil;
    
    const char *zipfilename = [pathToFile UTF8String];
    
    if (zipfilename != nil)
    {
        unzipFile = unzOpen(zipfilename);
    }
    
    if (unzipFile == nil)
    {
        //NSLog(@"Cannot open %s",zipfilename);
        return nil;
    }
    
    //NSLog(@"%s opened",zipfilename);
    
    return unzipFile;
}

/* metaDataFileFromZip extracts the file meta.xml from the zip file and returns it as an NSData* structure 
   or nil if the metadata is not present */
- (NSData*) metaDataFileFromZip:(unzFile)unzipFile
{
    //search and set the cursor to meta.xml
    if (unzLocateFile(unzipFile, "meta.xml", CASESENSITIVITY) != UNZ_OK) {
        //we hit an error, do cleanup
        unzCloseCurrentFile(unzipFile);
        return nil;
    }
    
    //open the current file
    if (unzOpenCurrentFile(unzipFile) != UNZ_OK) {
        //we hit an error, do cleanup
        unzCloseCurrentFile(unzipFile); 
        unzClose(unzipFile);
        return nil;
    }

    NSMutableData *data = [NSMutableData new];

    unsigned buffer[BUFFER_SIZE];
    int bytesRead = 0;
    while ((bytesRead = unzReadCurrentFile(unzipFile, buffer, sizeof(buffer))) > 0) {
        //append the data until we are finished
        [data appendData:[NSData dataWithBytes:(const void *)buffer length:bytesRead]];
    }
    
    //we no longer need the file, so close it
    unzCloseCurrentFile(unzipFile);
    
    NSData *returnValue = [NSData dataWithData:data];
    [data release];
    
    return returnValue;
}

/* contentDataFileFromZip extracts the file content.xml from the zip file and returns it as an NSData* structure 
   or nil if the metadata is not present */
- (NSData*) contentDataFileFromZip:(unzFile)unzipFile
{
    //search and set the cursor to content.xml
    if (unzLocateFile(unzipFile, "content.xml", CASESENSITIVITY) != UNZ_OK) {
        //we hit an error, do cleanup
        unzCloseCurrentFile(unzipFile);
        return nil;
    }
    
    //open the current file
    if (unzOpenCurrentFile(unzipFile) != UNZ_OK) {
        //we hit an error, do cleanup
        unzCloseCurrentFile(unzipFile); 
        unzClose(unzipFile);
        return nil;
    }
    
    NSMutableData *data = [NSMutableData new];
    
    unsigned buffer[BUFFER_SIZE];
    int bytesRead = 0;
    while ((bytesRead = unzReadCurrentFile(unzipFile, buffer, sizeof(buffer))) > 0) {
        //append the data
        [data appendData:[NSData dataWithBytes:(const void *)buffer length:bytesRead]];
    }
    
    //we no longer need the file, so close it
    unzCloseCurrentFile(unzipFile);
    
    NSData *returnValue = [NSData dataWithData:data];
    [data release];
    
    return returnValue;
}


@end
