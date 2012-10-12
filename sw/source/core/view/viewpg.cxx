/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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


#include <com/sun/star/uno/Sequence.hxx>

#include <hintids.hxx>
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

#include <statstr.hrc>  // Text fuer SfxProgress
#include <comcore.hrc>

#include <IDocumentFieldsAccess.hxx>
#include <IDocumentDeviceAccess.hxx>


using namespace ::com::sun::star;


SwPageFrm const*
sw_getPage(SwRootFrm const& rLayout, sal_Int32 const nPage); // vprint.cxx

// OD 12.12.2002 #103492#
SwPagePreviewLayout* ViewShell::PagePreviewLayout()
{
    return Imp()->PagePreviewLayout();
}

void ViewShell::ShowPreViewSelection( sal_uInt16 nSelPage )
{
    Imp()->InvalidateAccessiblePreViewSelection( nSelPage );
}

/** adjust view options for page preview

    OD 09.01.2003 #i6467#
*/
void ViewShell::AdjustOptionsForPagePreview(SwPrintData const& rPrintOptions)
{
    if ( !IsPreView() )
    {
        OSL_FAIL( "view shell doesn't belongs to a page preview - no adjustment of its view options");
        return;
    }

    PrepareForPrint( rPrintOptions );

    return;
}

// print brochure
// OD 05.05.2003 #i14016# - consider empty pages on calculation of the scaling
// for a page to be printed.
void ViewShell::PrintProspect(
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
// const USHORT nPageMax = static_cast< USHORT >(rPagesToPrint.first > rPagesToPrint.second ?
//            rPagesToPrint.first : rPagesToPrint.second);
    OSL_ENSURE( rPagesToPrint.first  == -1 || rPrintData.GetRenderData().GetValidPagesSet().count( rPagesToPrint.first ) == 1, "first Page not valid" );
    OSL_ENSURE( rPagesToPrint.second == -1 || rPrintData.GetRenderData().GetValidPagesSet().count( rPagesToPrint.second ) == 1, "second Page not valid" );

    // eine neue Shell fuer den Printer erzeugen
    ViewShell aShell( *this, 0, pPrinter );

    SET_CURR_SHELL( &aShell );

    aShell.PrepareForPrint( rPrintData );

    //!! applying view options and formatting the document should now only be done in getRendererCount!

    MapMode aMapMode( MAP_TWIP );
    Size aPrtSize( pPrinter->PixelToLogic( pPrinter->GetPaperSizePixel(), aMapMode ) );

    SwTwips nMaxRowSz, nMaxColSz;

    const SwPageFrm *pStPage    = 0;
    const SwPageFrm *pNxtPage   = 0;
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
                aSttPageSize = pStPage->GetPrev()->Frm().SSize();
            else
                aSttPageSize = pStPage->GetNext()->Frm().SSize();
        }
        else
        {
            aSttPageSize = pStPage->Frm().SSize();
        }
    }
    Size aNxtPageSize;
    if ( pNxtPage )
    {
        if ( pNxtPage->IsEmptyPage() )
        {
            if ( pNxtPage->GetPhyPageNum() % 2 == 0 )
                aNxtPageSize = pNxtPage->GetPrev()->Frm().SSize();
            else
                aNxtPageSize = pNxtPage->GetNext()->Frm().SSize();
        }
        else
        {
            aNxtPageSize = pNxtPage->Frm().SSize();
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
        nMaxRowSz = Max( aNxtPageSize.Height(), aSttPageSize.Height() );
    }

    // den MapMode einstellen
    aMapMode.SetOrigin( Point() );
    {
        Fraction aScX( aPrtSize.Width(), nMaxColSz );
        Fraction aScY( aPrtSize.Height(), nMaxRowSz );
        if( aScX < aScY )
            aScY = aScX;

        {
            // fuer Drawing, damit diese ihre Objecte vernuenftig Painten
            // koennen, auf "glatte" Prozentwerte setzen
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
            aShell.aVisArea = pStPage->Frm();

            Point aPos( aSttPt );
            aPos -= aShell.aVisArea.Pos();
//            aPos -= aPrtOff;
            aMapMode.SetOrigin( aPos );
            pPrinter->SetMapMode( aMapMode );
            pStPage->GetUpper()->Paint( pStPage->Frm() );
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
