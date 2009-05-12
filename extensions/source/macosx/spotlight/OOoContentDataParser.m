/*************************************************************************
*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: OOoContentDataParser.m,v $
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

#import "OOoContentDataParser.h"

@implementation OOoContentDataParser

- (id)init
{
    if ((self = [super init]) != nil) {
        shouldReadCharacters = NO;
        textContent = nil;
        runningTextContent = nil;
        
        return self;
    }
    
    return nil;
}

- (void)parseXML:(NSData*)data intoDictionary:(NSMutableDictionary*)dict
{
    mdiValues = dict;
    
    //NSLog(@"data: %@ %d", data, [data length]);
    
    //init parser settings
    shouldReadCharacters = NO;
    
    NSXMLParser *parser = [[NSXMLParser alloc] initWithData:data];
    
    [parser setDelegate:self];
    [parser setShouldResolveExternalEntities:NO];
    [parser parse];
    
    [parser release];
    
    //NSLog(@"finished");
}

- (void)parser:(NSXMLParser *)parser didStartElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qualifiedName attributes:(NSDictionary *)attributeDict
{
    // all text content is stored inside <text:p> elements
    if ([elementName isEqualToString:@"text:p"] == YES) {
        runningTextContent = [NSMutableString new];
        shouldReadCharacters = YES;
        //NSLog(@"start");
    } else {
        return;
    }
    
    //NSLog(@"start element %@", elementName);
}

- (void)parser:(NSXMLParser *)parser didEndElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName 
{
    if (shouldReadCharacters == TRUE) {
        if (textContent == nil) {
            textContent = [NSMutableString new];
        } else if ([runningTextContent isEqualToString:@""] == NO) {
            // separate by whitespace
            [textContent appendString:@" "];
        }
        //NSLog(@"end");
        
        [textContent appendString:[NSString stringWithString:runningTextContent]];
        [runningTextContent release];
    }
    shouldReadCharacters = NO;
}

- (void)parser:(NSXMLParser *)parser foundCharacters:(NSString *)string
{
    if (shouldReadCharacters == NO) {
        return;
    }
    //NSLog(string);
    
    [runningTextContent appendString:string];
    
    //NSLog(@"currentElement: %@", currentElement);
    //NSLog(@"read: %@", string);
    
}

- (void)parser:(NSXMLParser *)parser parseErrorOccurred:(NSError *)parseError
{
    //NSLog(@"parsing finished with error");
    NSLog([NSString stringWithFormat:@"An error occured parsing the document. (Error %i, Description: %@, Line: %i, Column: %i)", [parseError code], 
        [[parser parserError] localizedDescription], [parser lineNumber],
        [parser columnNumber]]);
    
    if (runningTextContent != nil) {
        [runningTextContent release];
    }
    if (textContent != nil) {
        [textContent release];
    }
}

- (void)parserDidEndDocument:(NSXMLParser *)parser
{
    if (textContent != nil && [textContent length] > 0) {
        [mdiValues setObject:[NSString stringWithString:textContent] forKey:(NSString*)kMDItemTextContent];
        [textContent release];
    }
}

@end
