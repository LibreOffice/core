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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include "vcl/print.hxx"

#include "aqua/aquaprintview.h"
#include "aqua/salprn.h"

@implementation AquaPrintView
-(id)initWithController: (vcl::PrinterController*)pController withInfoPrinter: (AquaSalInfoPrinter*)pInfoPrinter
{
    NSRect aRect = { NSZeroPoint, [pInfoPrinter->getPrintInfo() paperSize] };
    if( (self = [super initWithFrame: aRect]) != nil )
    {
        mpController = pController;
        mpInfoPrinter = pInfoPrinter;
    }
    return self;
}

-(BOOL)knowsPageRange: (NSRangePointer)range
{
    range->location = 1;
    range->length = mpInfoPrinter->getCurPageRangeCount();
    return YES;
}

-(NSRect)rectForPage: (int)page
{
    NSSize aPaperSize =  [mpInfoPrinter->getPrintInfo() paperSize];
    int nWidth = (int)aPaperSize.width;
    // #i101108# sanity check
    if( nWidth < 1 )
        nWidth = 1;
    NSRect aRect = { NSMakePoint( page % nWidth, page / nWidth), aPaperSize };
    return aRect;
}

-(NSPoint)locationOfPrintRect: (NSRect)aRect
{
    (void)aRect;
    return NSZeroPoint;
}

-(void)drawRect: (NSRect)rect
{
    NSPoint aPoint = [self locationOfPrintRect: rect];
    mpInfoPrinter->setStartPageOffset( static_cast<int>(rect.origin.x), static_cast<int>(rect.origin.y) );
    NSSize aPaperSize =  [mpInfoPrinter->getPrintInfo() paperSize];
    int nPage = (int)(aPaperSize.width * rect.origin.y + rect.origin.x);
    
    // page count is 1 based
    if( nPage - 1 < (mpInfoPrinter->getCurPageRangeStart() + mpInfoPrinter->getCurPageRangeCount() ) )
        mpController->printFilteredPage( nPage-1 );
}
@end
