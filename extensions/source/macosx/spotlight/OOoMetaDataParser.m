/*************************************************************************
*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: OOoMetaDataParser.m,v $
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

#import "OOoMetaDataParser.h"

static NSSet *singleValueXMLElements;
static NSSet *multiValueXMLElements;
static NSDictionary *metaXML2MDIKeys;

@implementation OOoMetaDataParser

+ (void)initialize
{
    static BOOL isInitialized = NO;
    
    if (isInitialized == NO) {
        //set up the meta elements with only one value
        NSMutableSet *temp = [NSMutableSet new];
        [temp addObject:@"dc:title"];
        [temp addObject:@"dc:description"];
        [temp addObject:@"meta:user-defined"];
        singleValueXMLElements = [[NSSet setWithSet:temp] retain];
        
        //set up the meta elements that can have more than one value
        [temp removeAllObjects];
        [temp addObject:@"dc:subject"];
        [temp addObject:@"meta:keyword"];
        [temp addObject:@"meta:initial-creator"];
        [temp addObject:@"dc:creator"];
        multiValueXMLElements = [[NSSet setWithSet:temp] retain];
        [temp release];
        
        //set up the map to store the values with the correct MDI keys
        NSMutableDictionary *tempDict = [NSMutableDictionary new];
        [tempDict setObject:(NSString*)kMDItemTitle forKey:@"dc:title"];
        [tempDict setObject:(NSString*)kMDItemDescription forKey:@"dc:description"];
        [tempDict setObject:(NSString*)kMDItemKeywords forKey:@"dc:subject"];
        [tempDict setObject:(NSString*)kMDItemAuthors forKey:@"meta:initial-creator"];
        [tempDict setObject:(NSString*)kMDItemAuthors forKey:@"dc:creator"];
        [tempDict setObject:(NSString*)kMDItemKeywords forKey:@"meta:keyword"];
        [tempDict setObject:@"org_openoffice_opendocument_custominfo1" forKey:@"Info 1"];
        [tempDict setObject:@"org_openoffice_opendocument_custominfo2" forKey:@"Info 2"];
        [tempDict setObject:@"org_openoffice_opendocument_custominfo3" forKey:@"Info 3"];
        [tempDict setObject:@"org_openoffice_opendocument_custominfo4" forKey:@"Info 4"];
        metaXML2MDIKeys = [[NSDictionary dictionaryWithDictionary:tempDict] retain];
        [tempDict release];
        
        isInitialized = YES;
    }
}

- (id)init
{
    if ((self = [super init]) != nil) {
        shouldReadCharacters = NO;
//        currentElement = nil;
        textCurrentElement = nil;
        
        return self;
    }
    
    return nil;
}

- (void)parseXML:(NSData*)data intoDictionary:(NSMutableDictionary*)dict
{
    metaValues = dict;
    
    //NSLog(@"data: %@ %d", data, [data length]);
        
    //init parser settings
    shouldReadCharacters = NO;
    
    NSXMLParser *parser = [[NSXMLParser alloc] initWithData:data];
    
    [parser setDelegate:self];
    [parser setShouldResolveExternalEntities:NO];
    [parser parse];
    
    [parser release];
    
    //NSLog(@"finished parsing meta");
}

- (void)parser:(NSXMLParser *)parser didStartElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qualifiedName attributes:(NSDictionary *)attributeDict
{
//    NSLog(@"<%@>", elementName);
    if ([singleValueXMLElements containsObject:elementName] == YES) {
        shouldReadCharacters = YES;
    } else if ([multiValueXMLElements containsObject:elementName] == YES) {
        shouldReadCharacters = YES;
    } else {
        //we are not interested in this element
        shouldReadCharacters = NO;
        return;
    }
    
    if (shouldReadCharacters == YES) {
        textCurrentElement = [NSMutableString new];
        isCustom = [elementName isEqualToString:@"meta:user-defined"];
        if (isCustom == YES) {
            customAttribute = [[attributeDict objectForKey:@"meta:name"] retain];
            //NSLog(customAttribute);
        }
    }

    //NSLog(@"start element %@", elementName);
}

- (void)parser:(NSXMLParser *)parser didEndElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName 
{
//    NSLog(@"</%@>", elementName);
    if (shouldReadCharacters == YES) {
        NSString *mdiName = nil;
        if (isCustom == YES) {
            mdiName = (NSString*)[metaXML2MDIKeys objectForKey:customAttribute];
        } else {
            mdiName = (NSString*)[metaXML2MDIKeys objectForKey:elementName];
        }
        //NSLog(@"mdiName: %@", mdiName);
        
        if (mdiName == nil) {
            return;
        }
        
        if ([singleValueXMLElements containsObject:elementName] == YES) {
            [metaValues setObject:textCurrentElement forKey:mdiName];
        } else {
            // must be multi-value
            NSMutableArray *arr = [metaValues objectForKey:mdiName];
            if (arr == nil) {
                // we have no array yet, create it
                arr = [[NSMutableArray new] autorelease];
                // and store it
                [metaValues setObject:arr forKey:mdiName];
            }
            // only store an element once, no need for duplicates
            if ([arr containsObject:textCurrentElement] == NO) {
                [arr addObject:textCurrentElement];
            }
        }
        // cleanup part 1
        [textCurrentElement release];
        if (customAttribute != nil) {
            [customAttribute release];
        }
    }
    
    //cleanup part 2
    shouldReadCharacters = NO;
    isCustom = NO;
}

- (void)parser:(NSXMLParser *)parser foundCharacters:(NSString *)string
{
//    NSLog(@"%@", string);
    if (shouldReadCharacters == NO) {
        return;
    }
    
    // this delegate method might be called several times for a single element, 
    // so we have to collect the received data
    [textCurrentElement appendString:string];
    
    //NSLog(@"chars read: %@", string);
}

- (void)parser:(NSXMLParser *)parser parseErrorOccurred:(NSError *)parseError
{
    //NSLog(@"parsing finished with error");
    NSLog([NSString stringWithFormat:@"Error %i, Description: %@, Line: %i, Column: %i", [parseError code], 
        [[parser parserError] localizedDescription], [parser lineNumber],
        [parser columnNumber]]);
}

@end
