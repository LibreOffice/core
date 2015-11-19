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

#include <com/sun/star/uno/Sequence.hxx>

#include <hintids.hxx>
#include <tools/fract.hxx>
#include <vcl/window.hxx>
#include <vcl/oldprintadaptor.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/progress.hxx>
#include <pvprtdat.hxx>
#include <viewsh.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <viewimp.hxx>
#include <viewopt.hxx>
#include <printdata.hxx>
#include <fldbas.hxx>
#include <ptqueue.hxx>
#include <swregion.hxx>
#include <hints.hxx>
#include <fntcache.hxx>

#include <statstr.hrc>
#include <comcore.hrc>

#include <IDocumentFieldsAccess.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <vprint.hxx>

using namespace ::com::sun::star;

// OD 12.12.2002 #103492#
SwPagePreviewLayout* SwViewShell::PagePreviewLayout()
{
    return Imp()->PagePreviewLayout();
}

void SwViewShell::ShowPreviewSelection( sal_uInt16 nSelPage )
{
    Imp()->InvalidateAccessiblePreviewSelection( nSelPage );
}

/** adjust view options for page preview

    OD 09.01.2003 #i6467#
*/
void SwViewShell::AdjustOptionsForPagePreview(SwPrintData const& rPrintOptions)
{
    if ( !IsPreview() )
    {
        OSL_FAIL( "view shell doesn't belongs to a page preview - no adjustment of its view options");
        return;
    }

    PrepareForPrint( rPrintOptions );

    return;
}

/// print brochure
// OD 05.05.2003 #i14016# - consider empty pages on calculation of the scaling
// for a page to be printed.
void SwViewShell::PrintProspect(
    OutputDevice *pOutDev,
    const SwPrintData &rPrintData,
    sal_Int32 nRenderer // the index in the vector of prospect pages to be printed
    )
{
    const sal_Int32 nMaxRenderer = rPrintData.GetRenderData().GetPagePairsForProspectPrinting().size() - 1;
    OSL_ENSURE( 0 <= nRenderer && nRenderer <= nMaxRenderer, "nRenderer out of bounds");
    Printer *pPrinter = dynamic_cast< Printer * >(pOutDev);
    if (!pPrinter || nMaxRenderer < 0 || nRenderer < 0 || nRenderer > nMaxRenderer)
        return;

    // save settings of OutputDevice (should be done always since the
    // output device is now provided by a call from outside the Writer)
    pPrinter->Push();

    std::pair< sal_Int32, sal_Int32 > rPagesToPrint =
            rPrintData.GetRenderData().GetPagePairsForProspectPrinting()[ nRenderer ];
    OSL_ENSURE( rPagesToPrint.first  == -1 || rPrintData.GetRenderData().GetValidPagesSet().count( rPagesToPrint.first ) == 1, "first Page not valid" );
    OSL_ENSURE( rPagesToPrint.second == -1 || rPrintData.GetRenderData().GetValidPagesSet().count( rPagesToPrint.second ) == 1, "second Page not valid" );

    // create a new shell for the printer
    SwViewShell aShell( *this, nullptr, pPrinter );

    SET_CURR_SHELL( &aShell );

    aShell.PrepareForPrint( rPrintData );

    //!! applying view options and formatting the document should now only be done in getRendererCount!

    MapMode aMapMode( MAP_TWIP );
    Size aPrtSize( pPrinter->PixelToLogic( pPrinter->GetPaperSizePixel(), aMapMode ) );

    SwTwips nMaxRowSz, nMaxColSz;

    const SwPageFrame *pStPage    = nullptr;
    const SwPageFrame *pNxtPage   = nullptr;
    if (rPagesToPrint.first > 0)
    {
        pStPage = sw_getPage(*aShell.GetLayout(), rPagesToPrint.first);
    }
    if (rPagesToPrint.second > 0)
    {
        pNxtPage = sw_getPage(*aShell.GetLayout(), rPagesToPrint.second);
    }

    // OD 05.05.2003 #i14016# - consider empty pages on calculation
    // of page size, used for calculation of scaling.
    Size aSttPageSize;
    if ( pStPage )
    {
        if ( pStPage->IsEmptyPage() )
        {
            if ( pStPage->GetPhyPageNum() % 2 == 0 )
                aSttPageSize = pStPage->GetPrev()->Frame().SSize();
            else
                aSttPageSize = pStPage->GetNext()->Frame().SSize();
        }
        else
        {
            aSttPageSize = pStPage->Frame().SSize();
        }
    }
    Size aNxtPageSize;
    if ( pNxtPage )
    {
        if ( pNxtPage->IsEmptyPage() )
        {
            if ( pNxtPage->GetPhyPageNum() % 2 == 0 )
                aNxtPageSize = pNxtPage->GetPrev()->Frame().SSize();
            else
                aNxtPageSize = pNxtPage->GetNext()->Frame().SSize();
        }
        else
        {
            aNxtPageSize = pNxtPage->Frame().SSize();
        }
    }

    if( !pStPage )
    {
        nMaxColSz = 2 * aNxtPageSize.Width();
        nMaxRowSz = aNxtPageSize.Height();
    }
    else if( !pNxtPage )
    {
        nMaxColSz = 2 * aSttPageSize.Width();
        nMaxRowSz = aSttPageSize.Height();
    }
    else
    {
        nMaxColSz = aNxtPageSize.Width() + aSttPageSize.Width();
        nMaxRowSz = std::max( aNxtPageSize.Height(), aSttPageSize.Height() );
    }

    // set the MapMode
    aMapMode.SetOrigin( Point() );
    {
        Fraction aScX( aPrtSize.Width(), nMaxColSz );
        Fraction aScY( aPrtSize.Height(), nMaxRowSz );
        if( aScX < aScY )
            aScY = aScX;

        {
            // Round percentages for Drawings so that these can paint their objects properly
            aScY *= Fraction( 1000, 1 );
            long nTmp = (long)aScY;
            if( 1 < nTmp )
                --nTmp;
            else
                nTmp = 1;
            aScY = Fraction( nTmp, 1000 );
        }

        aMapMode.SetScaleY( aScY );
        aMapMode.SetScaleX( aScY );
    }

    Size aTmpPrtSize( pPrinter->PixelToLogic( pPrinter->GetPaperSizePixel(), aMapMode ) );

    // calculate start point for equal border on all sides
    Point aSttPt( (aTmpPrtSize.Width() - nMaxColSz) / 2,
                  (aTmpPrtSize.Height() - nMaxRowSz) / 2 );
    for( int nC = 0; nC < 2; ++nC )
    {
        if( pStPage )
        {
            aShell.Imp()->SetFirstVisPageInvalid();
            aShell.maVisArea = pStPage->Frame();

            Point aPos( aSttPt );
            aPos -= aShell.maVisArea.Pos();
            aMapMode.SetOrigin( aPos );
            pPrinter->SetMapMode( aMapMode );
            pStPage->GetUpper()->Paint( *pOutDev, pStPage->Frame() );
        }

        pStPage = pNxtPage;
        aSttPt.X() += aTmpPrtSize.Width() / 2;
    }

    SwPaintQueue::Repaint();

    //!! applying/modifying view options and formatting the document should now only be done in getRendererCount!

    pFntCache->Flush();

    // restore settings of OutputDevice (should be done always now since the
    // output device is now provided by a call from outside the Writer)
    pPrinter->Pop();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
