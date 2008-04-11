/************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: aquaprintview.mm,v $
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
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include "aquaprintview.h"
#include "salprn.h"
#include "vcl/impprn.hxx"

@implementation AquaPrintView
-(id)initWithQPrinter: (ImplQPrinter*)pPrinter withInfoPrinter: (AquaSalInfoPrinter*)pInfoPrinter
{
    NSRect aRect = { { 0, 0 }, [pInfoPrinter->getPrintInfo() paperSize] };
    if( (self = [super initWithFrame: aRect]) != nil )
    {
        mpQPrinter = pPrinter;
        mpInfoPrinter = pInfoPrinter;
    }
    return self;
}

-(MacOSBOOL)knowsPageRange: (NSRangePointer)range
{
    range->location = 1;
    range->length = mpQPrinter->GetPrintPageCount();
    return YES;
}

-(NSRect)rectForPage: (int)page
{
    NSSize aPaperSize =  [mpInfoPrinter->getPrintInfo() paperSize];
    int nWidth = (int)aPaperSize.width;
    NSRect aRect = { { page % nWidth, page / nWidth }, aPaperSize };
    return aRect;
}

-(NSPoint)locationOfPrintRect: (NSRect)aRect
{
    NSPoint aPoint = { 0, 0 };
    return aPoint;
}

-(void)drawRect: (NSRect)rect
{
    NSPoint aPoint = [self locationOfPrintRect: rect];
    mpInfoPrinter->setStartPageOffset( rect.origin.x, rect.origin.y );
    NSSize aPaperSize =  [mpInfoPrinter->getPrintInfo() paperSize];
    int nPage = (int)(aPaperSize.width * rect.origin.y + rect.origin.x);
    
    // page count is 1 based
    mpQPrinter->PrintPage( nPage-1 );
}
@end
