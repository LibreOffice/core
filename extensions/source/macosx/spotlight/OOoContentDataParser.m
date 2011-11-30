/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



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
