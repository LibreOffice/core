/************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: aquaprintview.mm,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:58:59 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
