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

#include <objc/objc-runtime.h>

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
    
    // Once again...
    // class 'OOoContentDataParser' does not implement the 'NSXMLParserDelegate' protocol
    // So instead of this:
    // [parser setDelegate:self];
    // do this:
    objc_msgSend(parser, @selector(setDelegate:), self);

    [parser setShouldResolveExternalEntities:NO];
    [parser parse];
    
    [parser release];
    
    //NSLog(@"finished");
}

- (void)parser:(NSXMLParser *)parser didStartElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qualifiedName attributes:(NSDictionary *)attributeDict
{
    (void) parser; // unused
    (void) namespaceURI; // FIXME this should not be ignored but should be used
                         // instead of text: prefix in the comparison below!
    (void) qualifiedName; // unused
    (void) attributeDict; // unused
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
    (void) parser; // unused
    (void) elementName; // unused
    (void) namespaceURI; // unused
    (void) qName; // unused
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
    (void) parser; // unused
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
    NSLog(@"An error occurred parsing the document. (Error %li, Description: %@, Line: %li, Column: %li)", (long) [parseError code], 
          [[parser parserError] localizedDescription], (long) [parser lineNumber],
          (long) [parser columnNumber]);
    
    if (runningTextContent != nil) {
        [runningTextContent release];
    }
    if (textContent != nil) {
        [textContent release];
    }
}

- (void)parserDidEndDocument:(NSXMLParser *)parser
{
    (void) parser; // unused
    if (textContent != nil && [textContent length] > 0) {
        [mdiValues setObject:[NSString stringWithString:textContent] forKey:(NSString*)kMDItemTextContent];
        [textContent release];
    }
}

@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
