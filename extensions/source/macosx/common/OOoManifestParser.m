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

#import "OOoManifestParser.h"

@implementation OOoManifestParser

- (id)init
{
    if ((self = [super init]) != nil) {
        metaValues = nil;

        return self;
    }

    return nil;
}

- (void)parseXML:(NSData*)data intoDictionary:(NSMutableDictionary*)dict
{
    metaValues = dict;

    NSXMLParser *parser = [[NSXMLParser alloc] initWithData:data];

    [parser setDelegate:self];

    [parser setShouldResolveExternalEntities:NO];
    [parser parse];

    [parser release];

    metaValues = nil;
}

- (void)parser:(NSXMLParser *)parser didStartElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qualifiedName attributes:(NSDictionary *)attributeDict
{
    (void) parser; // unused
    (void) namespaceURI; // unused
    (void) qualifiedName; // unused

    if ([@"manifest:file-entry" isEqualToString:elementName]) {
        NSString *mediaType = [attributeDict objectForKey:@"manifest:media-type"];
        NSString *path = [attributeDict objectForKey:@"manifest:full-path"];
        if (mediaType && path && [path length]) {
            NSString *key = nil;
            NSArray *mediaComponents = [mediaType componentsSeparatedByString:@"/"];
            if (mediaComponents && [mediaComponents count] && [[mediaComponents objectAtIndex:0] isEqualToString:@"image"]) {
                key = mediaType;
            }
            else if ([mediaType isEqualToString:MEDIA_TYPE_PDF]) {
                key = MEDIA_TYPE_PDF;
            }
            else if (![mediaType length]) {
                // NeoOffice sometimes adds PDF thumbnails with an empty mime type
                if ([path hasSuffix:@".pdf"])
                    key = MEDIA_TYPE_PDF;
                // Just to be safe, add any PNG thumbnails
                else if ([path hasSuffix:@".png"])
                    key = MEDIA_TYPE_PNG;
            }

            // OpenOffice, NeoOffice, and LibreOffice appear to only insert a
            // single thumbnail per media type so ignore duplicate thumbnails
            if (key && ![metaValues objectForKey:key])
                [metaValues setObject:path forKey:key];
        }
    }
}

- (void)parser:(NSXMLParser *)parser didEndElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName
{
    (void) parser; // unused
    (void) namespaceURI; // unused
    (void) qName; // unused
}

- (void)parser:(NSXMLParser *)parser foundCharacters:(NSString *)string
{
    (void) parser; // unused
    (void) string; // unused
}

- (void)parser:(NSXMLParser *)parser parseErrorOccurred:(NSError *)parseError
{
    //NSLog(@"parsing finished with error");
    NSLog(@"Error %li, Description: %@, Line: %li, Column: %li", (long) [parseError code],
          [[parser parserError] localizedDescription], (long) [parser lineNumber],
          (long) [parser columnNumber]);
}

@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
