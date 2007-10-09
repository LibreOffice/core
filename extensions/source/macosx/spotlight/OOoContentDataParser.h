/*************************************************************************
*
*   OpenOffice.org - a multi-platform office productivity suite
*
*   $RCSfile: OOoContentDataParser.h,v $
*
*   $Revision: 1.2 $
*
*   last change: $Author: kz $ $Date: 2007-10-09 15:04:29 $
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

#import <Cocoa/Cocoa.h>


@interface OOoContentDataParser : NSObject {
    // indicates if we are interested in an element's content
    BOOL shouldReadCharacters;

    // the MD importer's values
    NSMutableDictionary *mdiValues;

    // all of the text inside a document
    NSMutableString *textContent;

    // the current element's content
    NSMutableString *runningTextContent;
}

- (void)parseXML:(NSData*)data intoDictionary:(NSMutableDictionary*)dict;

// delegates
- (void)parser:(NSXMLParser *)parser didStartElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qualifiedName attributes:(NSDictionary *)attributeDict;

- (void)parser:(NSXMLParser *)parser didEndElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName;

- (void)parser:(NSXMLParser *)parser foundCharacters:(NSString *)string;

- (void)parser:(NSXMLParser *)parser parseErrorOccurred:(NSError *)parseError;

- (void)parserDidEndDocument:(NSXMLParser *)parser;

@end
