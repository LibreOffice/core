/*************************************************************************
 *
 *  $RCSfile: viewpg.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: od $ $Date: 2002-12-06 16:26:55 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SV_WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif
#ifndef _SFX_PROGRESS_HXX //autogen
#include <sfx2/progress.hxx>
#endif

#ifndef _PVPRTDAT_HXX
#include <pvprtdat.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>
#endif
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif
#ifndef _VIEWIMP_HXX
#include <viewimp.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _SWPRTOPT_HXX
#include <swprtopt.hxx> // SwPrtOptions
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _PTQUEUE_HXX
#include <ptqueue.hxx>
#endif
#ifndef _SWREGION_HXX
#include <swregion.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
#ifndef _FNTCACHE_HXX
#include <fntcache.hxx>
#endif

#ifndef _STATSTR_HRC
#include <statstr.hrc>  // Text fuer SfxProgress
#endif
#ifndef _COMCORE_HRC
#include <comcore.hrc>
#endif
// OD 05.12.2002 #103492#
#include <algorithm>

const SwTwips nXFree = 4*142;        // == 0.25 cm * 4
const SwTwips nYFree = 4*142;
static USHORT nPrevViewXFreePix = 0;
static USHORT nPrevViewYFreePix = 0;

SwPageFrm* lcl_GetSttPage( BYTE& rRow, BYTE& rCol, USHORT& rSttPage,
                                const SwRootFrm* pRoot )
{
    USHORT nCalcSttPg = rSttPage;
    if( nCalcSttPg )
        --nCalcSttPg;
    if( !rCol )
        ++rCol;
    if( !rRow )
        ++rRow;

    // suche die Start-Seite
    SwPageFrm *pSttPage = (SwPageFrm*)pRoot->Lower(), *pPrevPage = pSttPage;
    while( pSttPage && nCalcSttPg )
    {
        pPrevPage = pSttPage;
        pSttPage = (SwPageFrm*)pSttPage->GetNext();
        --nCalcSttPg;
    }

    // keine Seiten mehr ??
    if( !pSttPage )
    {
        pSttPage = pPrevPage;
        rSttPage -= nCalcSttPg;     // das sind die tatsaechlichen
        if( 1 == (rSttPage & 1 ))   // eine rechte Seite?
            ++rSttPage;             // dann sorge dafuer das sie rechts steht

        // dann dem Wunsch entsprechend Rueckwaerts
        for( USHORT n = rRow * rCol; n && pSttPage; --n )
        {
            pPrevPage = pSttPage;
            pSttPage = (SwPageFrm*)pSttPage->GetPrev();
            --rSttPage;
        }
        if( !pSttPage )
        {
            pSttPage = pPrevPage;
            rSttPage = 1;
        }
    }
    return pSttPage;
}

USHORT ViewShell::CalcPreViewPage(
        const Size& rWinSize,       // auf diese Size die Scalierung errechnen
        USHORT& rRowCol,            // Zeilen/Spalten (Row im Hi, Col im LowByte!)
        USHORT nSttPage,            // Start ab dieser Seite, eine gueltige ??
        Size& rMaxSize,             // MaxSize einer Seite
        USHORT& rVirtPageNo,        // virtuelle SeitenNummer
        USHORT nAccSelPage )        // selected page for accessibility
{
    const SwRootFrm* pRoot = GetLayout();
    ASSERT( pRoot, "Wo ist mein Layout?" );

    // ohne Win-Size ist nichts los
    if( !rWinSize.Width() || !rWinSize.Height() )
        return 0;

    // von allen Seiten die Size holen. Das Max entscheidet
    // ueber die Scalierung.  (So macht es WW)
    register SwTwips nMaxRowSz = 0, nMaxColSz = 0;
    register USHORT nCalcSttPg = nSttPage;
    if( nCalcSttPg )
        --nCalcSttPg;

    BYTE nRow = BYTE(rRowCol >> 8), nCol = BYTE(rRowCol & 0xff);
    if( !nCol )
        ++nCol;
    if( !nRow )
        ++nRow;

    // suche bestimme die Max-Breite
    register SwPageFrm *pSttPage = (SwPageFrm*)pRoot->Lower(),
                        *pPrevPage = pSttPage;
    while( pSttPage && nCalcSttPg )
    {
        pSttPage->Calc();
        register Size& rSize = pSttPage->Frm().SSize();
        if( nMaxColSz < rSize.Width() )
            nMaxColSz = rSize.Width();
        if( nMaxRowSz < rSize.Height() )
            nMaxRowSz = rSize.Height();
        pPrevPage = pSttPage;
        pSttPage = (SwPageFrm*)pSttPage->GetNext();
        --nCalcSttPg;
    }

    // keine Seiten mehr ??
    if( !pSttPage )
    {
        pSttPage = pPrevPage;
        nSttPage -= nCalcSttPg;     // das sind die tatsaechlichen
#if OLD
// hiermit stimmt die Errechnung der vir. PageNo nicht !!
        if( 1 == (nSttPage & 1 ))   // eine rechte Seite?
            ++nSttPage;             // dann sorge dafuer das sie rechts steht

        // dann dem Wunsch entsprechend Rueckwaerts
        for( USHORT n = nRow * nCol; n && pSttPage; --n )
        {
            pPrevPage = pSttPage;
            pSttPage = (SwPageFrm*)pSttPage->GetPrev();
            --nSttPage;
        }
#else
        USHORT n = nRow * nCol;
        if( 1 == (nSttPage & 1 ))   // eine rechte Seite?
            --n;                    // dann sorge dafuer das sie rechts steht

        // dann dem Wunsch entsprechend Rueckwaerts
        for( ; n && pSttPage; --n )
        {
            pPrevPage = pSttPage;
            pSttPage = (SwPageFrm*)pSttPage->GetPrev();
            --nSttPage;
        }
#endif
        if( !pSttPage )
        {
            pSttPage = pPrevPage;
            nSttPage = 0;
        }
        rVirtPageNo = pPrevPage->GetVirtPageNum();
    }
    else    // dann haben wir unsere StartSeite,
            // also weiter und das Max bestimmen
    {
        rVirtPageNo = pSttPage->GetVirtPageNum();
        while( pSttPage )
        {
            pSttPage->Calc();
            register Size& rSize = pSttPage->Frm().SSize();
            if( nMaxColSz < rSize.Width() )
                nMaxColSz = rSize.Width();
            if( nMaxRowSz < rSize.Height() )
                nMaxRowSz = rSize.Height();
            pSttPage = (SwPageFrm*)pSttPage->GetNext();
        }
    }

    rMaxSize.Height() = nMaxRowSz;
    rMaxSize.Width() = nMaxColSz;
    rRowCol = (nRow << 8) + nCol;       // und wieder returnen

    // Jetzt die Scalierung incl. der gewuenschten Zwischenraueme errechnen
    nMaxColSz = nCol * nMaxColSz;
    nMaxRowSz = nRow * nMaxRowSz;

    MapMode aMapMode( MAP_TWIP );
    // falls die Abstaende zwischen den Seiten noch nicht berechnet wurden
    if( !nPrevViewXFreePix || !nPrevViewYFreePix )
    {
        Size aTmpSz( nXFree, nYFree );
        aTmpSz = GetOut()->LogicToPixel( aTmpSz, aMapMode );
        nPrevViewXFreePix = USHORT( aTmpSz.Width() );
        nPrevViewYFreePix = USHORT( aTmpSz.Height() );
    }
    Size aSz( rWinSize.Width() - ((nCol+1) * nPrevViewXFreePix),
              rWinSize.Height() - ( (nRow+1) * nPrevViewYFreePix ) );
    aSz = GetOut()->PixelToLogic( aSz, aMapMode );
    Fraction aScX( aSz.Width(), nMaxColSz );
    Fraction aScY( aSz.Height(), nMaxRowSz );
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
        if( pOpt )                              // fuer den Font-Cache die
            pOpt->SetZoom( USHORT(nTmp / 10));  // Options mitpflegen!
    }
    aMapMode.SetScaleY( aScY );
    aMapMode.SetScaleX( aScY );

#ifdef ACCESSIBLE_LAYOUT
    if( USHRT_MAX == nAccSelPage )
        nAccSelPage = nSttPage;
    else if(nAccSelPage < nSttPage || nAccSelPage > nSttPage + (nRow * nCol) )
        nAccSelPage = nSttPage ? nSttPage : 1;

    Imp()->UpdateAccessiblePreview( nRow, nCol, nSttPage, rMaxSize,
                                    GetPreviewFreePix(), aScY, nAccSelPage );
#endif

    // was machen wir, wenn der MapMode unter einer
    // gewissen Schwelle (z.B. 10% (WW6)) ist ???

    GetOut()->SetMapMode( aMapMode );
    return nSttPage;
}
#ifdef ACCESSIBLE_LAYOUT
void ViewShell::ShowPreViewSelection( sal_uInt16 nSelPage )
{
    Imp()->InvalidateAccessiblePreViewSelection( nSelPage );
}
#endif

void ViewShell::PreViewPage(
        const Rectangle& rRect,     // Paint-Rect von Windows
        USHORT nRowCol,             // Anzahl Zeilen/Spalten
        USHORT nSttPage,            // Start ab dieser Seite, eine gueltige ??
        const Size& rPageSize,      // MaxSize einer Seite
        sal_uInt16 nSelectedPage )  // Page to be highlighted
{
    if( !GetWin() && !GetOut()->GetConnectMetaFile() )
        return;                     //Fuer den Drucker tun wir hier nix

    SET_CURR_SHELL( this );

    const SwRootFrm* pRoot = GetLayout();
    ASSERT( pRoot, "Wo ist mein Layout?" );

    BYTE nRow = BYTE(nRowCol >> 8), nCol = BYTE(nRowCol & 0xff);
    SwPageFrm* pSttPage = ::lcl_GetSttPage( nRow, nCol, nSttPage, pRoot );

    // damit auch die Seiten im Paint angezeigt werden, gleich die richtigen
    // Werte setzen! (siehe auch Bug 24575)
    Imp()->bFirstPageInvalid = FALSE;
    Imp()->pFirstVisPage = pSttPage;

    const Rectangle aPixRect( GetOut()->LogicToPixel( rRect ) );

    MapMode aMapMode( GetOut()->GetMapMode() );
    MapMode aCalcMapMode( aMapMode );
    aCalcMapMode.SetOrigin( Point() );

    SwRegionRects aRegion( rRect );//fuer die Wiese

    Point aFreePt( nPrevViewXFreePix, nPrevViewYFreePix );
    aFreePt = GetOut()->PixelToLogic( aFreePt, aCalcMapMode );
    Point aCalcPt( aFreePt );
    long nPageHeight = aFreePt.Y() + rPageSize.Height()+1;
    SwPageFrm* pPage = pSttPage;

    // erstmal die Wiese
    // Sonderbehandlung fuer die erste Seite
    // JP 19.08.98: aber nur wenn sie mehrspaltig angezeigt wird
    int bFirstPg = !nSttPage && 1 != nCol;

    for( BYTE nCntRow = 0; pPage && nCntRow < nRow; ++nCntRow )
    {
        aCalcPt.X() = aFreePt.X();
        for( BYTE nCntCol = 0; pPage && nCntCol < nCol; ++nCntCol )
        {
            pPage->Calc();

            // Sonderbehandlung fuer 1. Seite, ist immer eine rechte
            if( bFirstPg )
            {
                bFirstPg = FALSE;
                aCalcPt.X() += pPage->Frm().Width()+1 + aFreePt.X();
                continue;
            }

            if( pPage->IsEmptyPage() )
            {
                aRegion -= SwRect( aCalcPt, rPageSize );
                aCalcPt.X() += rPageSize.Width()+1 + aFreePt.X();
            }
            else
            {
                aRegion -= SwRect( aCalcPt, pPage->Frm().SSize() );
                aCalcPt.X() += pPage->Frm().Width()+1 + aFreePt.X();
            }
            pPage = (SwPageFrm*)pPage->GetNext();
        }
        aCalcPt.Y() += nPageHeight;
    }
    GetOut()->SetMapMode( aCalcMapMode );
    _PaintDesktop( aRegion );

    // und dann das Dokument
    bFirstPg = !nSttPage && 1 != nCol;
    aCalcPt = aFreePt;
    pPage = pSttPage;
    Font* pEmptyPgFont = 0;
    nSelectedPage -= nSttPage;
    Color aLineColor(COL_LIGHTBLUE);
    DBG_ASSERT(pWin, "no window available")
    const StyleSettings& rSettings = GetWin()->GetSettings().GetStyleSettings();
    if(rSettings.GetHighContrastMode())
        aLineColor = rSettings.GetHighlightTextColor();
    const Color& rColor = rSettings.GetFieldTextColor();
    for( nCntRow = 0; pPage && nCntRow < nRow; ++nCntRow )
    {
        aCalcPt.X() = aFreePt.X();
        for( BYTE nCntCol = 0; pPage && nCntCol < nCol; ++nCntCol )
        {
            // Sonderbehandlung fuer 1. Seite, ist immer eine rechte
            if( bFirstPg )
            {
                bFirstPg = FALSE;
                aCalcPt.X() += pPage->Frm().Width()+1 + aFreePt.X();
                nSelectedPage--;
                continue;
            }

            Rectangle aPageRect;

            if( pPage->IsEmptyPage() )
            {
                Rectangle aRect( aCalcPt, rPageSize );

                aMapMode.SetOrigin( Point(0,0) );
                GetOut()->SetMapMode( aMapMode );

                const Color aRetouche( Imp()->GetRetoucheColor() );

                if( GetOut()->GetFillColor() != aRetouche )
                    GetOut()->SetFillColor( aRetouche );

                GetOut()->DrawRect( aRect );

                if( !pEmptyPgFont )
                {
                    pEmptyPgFont = new Font;
                    pEmptyPgFont->SetSize( Size( 0, 80 * 20 )); // == 80 pt
                    pEmptyPgFont->SetWeight( WEIGHT_BOLD );
                    pEmptyPgFont->SetStyleName( aEmptyStr );
                    pEmptyPgFont->SetName( String::CreateFromAscii(
                            RTL_CONSTASCII_STRINGPARAM( "Helvetica" )) );
                    pEmptyPgFont->SetFamily( FAMILY_SWISS );
                    pEmptyPgFont->SetTransparent( TRUE );
                    pEmptyPgFont->SetColor( COL_GRAY );
                }
                Font aOldFont( GetOut()->GetFont() );
                GetOut()->SetFont( *pEmptyPgFont );
                GetOut()->DrawText( aRect, SW_RESSTR( STR_EMPTYPAGE ),
                                    TEXT_DRAW_VCENTER |
                                    TEXT_DRAW_CENTER |
                                    TEXT_DRAW_CLIP );
                GetOut()->SetFont( aOldFont );
                aPageRect = GetOut()->LogicToPixel( aRect );

                aCalcPt.X() += rPageSize.Width()+1 + aFreePt.X();
            }
            else
            {
                aVisArea = pPage->Frm();

                Point aPos( aCalcPt );
                aPos -= aVisArea.Pos();

                aMapMode.SetOrigin( aPos );
                GetOut()->SetMapMode( aMapMode );

                Rectangle aSVRect( GetOut()->LogicToPixel( aVisArea.SVRect() ) );
                aPageRect = aSVRect;
                if( aPixRect.IsOver( aSVRect ) )
                {
                    aSVRect.Intersection( aPixRect );
                    aSVRect = GetOut()->PixelToLogic( aSVRect );

                    Paint( aSVRect );
                }
                aCalcPt.X() += pPage->Frm().Width()+1 + aFreePt.X();
            }

            Rectangle aShadow( aPageRect.Left()+2, aPageRect.Bottom()+1,
                                aPageRect.Right()+2, aPageRect.Bottom()+2 );
            Color aFill( GetOut()->GetFillColor() );
            Color aLine( GetOut()->GetLineColor() );
            GetOut()->SetFillColor( rColor );
            GetOut()->SetLineColor( rColor );
            aShadow = GetOut()->PixelToLogic( aShadow );
            GetOut()->DrawRect( aShadow );
            aShadow.Left() = aPageRect.Right() + 1;
            aShadow.Right() = aShadow.Left() + 1;
            aShadow.Top() = aPageRect.Top() + 2;
            aShadow.Bottom() = aPageRect.Bottom();
            aShadow = GetOut()->PixelToLogic( aShadow );
            GetOut()->DrawRect( aShadow );
            aShadow = GetOut()->PixelToLogic( aPageRect );
            GetOut()->SetFillColor( Color( COL_TRANSPARENT ) );
            GetOut()->DrawRect( aShadow );
            if( !nSelectedPage )
            {
                aShadow.Right() = aPageRect.Right() - 1;
                aShadow.Left() = aPageRect.Left() + 1;
                aShadow.Top() = aPageRect.Top() + 1;
                aShadow.Bottom() = aPageRect.Bottom() - 1;
                aShadow = GetOut()->PixelToLogic( aShadow );
                GetOut()->SetFillColor( Color( COL_TRANSPARENT ) );
                GetOut()->SetLineColor( aLineColor );
                GetOut()->DrawRect( aShadow );
                aShadow.Right() = aPageRect.Right() - 2;
                aShadow.Left() = aPageRect.Left() + 2;
                aShadow.Top() = aPageRect.Top() + 2;
                aShadow.Bottom() = aPageRect.Bottom() - 2;
                aShadow = GetOut()->PixelToLogic( aShadow );
                GetOut()->DrawRect( aShadow );
            }
            GetOut()->SetFillColor( aFill );
            GetOut()->SetLineColor( aLine );

            pPage = (SwPageFrm*)pPage->GetNext();
            nSelectedPage--;
        }
        aCalcPt.Y() += nPageHeight;
    }

    delete pEmptyPgFont;
    GetOut()->SetMapMode( aCalcMapMode );
    aVisArea.Clear();       // verhinder internes Painten !!
}

/** init page preview layout - new method, replacing <CalcPreViewPage>

    OD 27.11.2002 #103492#
    initialize the page preview settings for a given layout.
    side effects:
    (1) data struture for current preview settings are initialized and set.
    (2) if parameter <_bCalcScale> is true, mapping mode with calculated
    scaling is set at the output device and the zoom at the view options is
    set with the calculated scaling.

    @author OD

    @param _nCols
    input parameter - initial number of page columns in the preview.

    @param _nRows
    input parameter - initial number of page rows in the preview.

    @param _orMaxPageSize
    output parameter - maximal size in width and height of all pages

    @param _orPreviewDocSize
    output parameter - size of the document in the proposed preview layout
    included the spacing between the pages.

    @param _bCalcScale
    input parameter - control, if method should calculate the needed
    scaling for the proposed preview layout for the given window size
    and sets the scaling at the output device and the view options.

    @param _pPxWinSize
    input parameter - window size in which the preview will be displayed and
    for which the scaling will be calculated.

    @return boolean, indicating, if preview layout is successful initialized.
*/
bool ViewShell::InitPreviewLayout( const sal_uInt16 _nCols,
                                   const sal_uInt16 _nRows,
                                   Size&            _orMaxPageSize,
                                   Size&            _orPreviewDocSize,
                                   const bool       _bCalcScale,
                                   const Size*      _pPxWinSize
                                 )
{
    const SwRootFrm* pRootFrm = GetLayout();
    // check environment and parameters
    {
        ASSERT( pRootFrm, "No layout - preview layout can *not* be initialized" );
        if ( !pRootFrm )
            return false;

        bool bColsRowsValid = (_nCols != 0) && (_nRows != 0);
        ASSERT( bColsRowsValid, "preview layout parameters not correct - preview layout can *not* be initialized" );
        if ( !bColsRowsValid )
            return false;

        bool bPxWinSizeValid = _pPxWinSize &&
                (_pPxWinSize->Width() != 0) && (_pPxWinSize->Height() != 0);
        ASSERT( bPxWinSizeValid, "no window size - preview layout can *not* be initialized" );
        if ( !bPxWinSizeValid )
            return false;
    }

    // environment and parameters ok

    // clear existing preview settings
    Imp()->CurrPrevwData()->Clear();

    // set layout information at preview settings
    Imp()->CurrPrevwData()->nCols = _nCols;
    Imp()->CurrPrevwData()->nRows = _nRows;

    // calculate maximal page size; calculate also number of pages
    const SwPageFrm* pPage = static_cast<const SwPageFrm*>(pRootFrm->Lower());
    register Size aMaxPageSize( 0,0 );
    register sal_uInt16 nPages = 0;
    while ( pPage )
    {
        ++nPages;
        pPage->Calc();
        register const Size& rPageSize = pPage->Frm().SSize();
        if ( rPageSize.Width() > aMaxPageSize.Width() )
            aMaxPageSize.Width() = rPageSize.Width();
        if ( rPageSize.Height() > aMaxPageSize.Height() )
            aMaxPageSize.Height() = rPageSize.Height();
        pPage = static_cast<const SwPageFrm*>(pPage->GetNext());
    }
    // set maximal page size, column width and row height at preview settings
    Imp()->CurrPrevwData()->aMaxPageSize = aMaxPageSize;
    Imp()->CurrPrevwData()->nColWidth = aMaxPageSize.Width() + nXFree;
    Imp()->CurrPrevwData()->nRowHeight = aMaxPageSize.Height() + nYFree;

    // set number of pages at preview settings
    Imp()->CurrPrevwData()->nPages = nPages;
    // validate layout information
    Imp()->CurrPrevwData()->bLayoutInfoValid = true;

    // return calculated maximal size in width and height of all pages
    _orMaxPageSize = Imp()->CurrPrevwData()->aMaxPageSize;

    // calculate document size in preview layout and set it at preview settings
    {
        // document width
        Imp()->CurrPrevwData()->aPreviewDocSize.Width() =
                _nCols * aMaxPageSize.Width() + (_nCols+1) * nXFree;

        // document height
        // determine number of rows needed for <nPages> in preview layout
        // <nPages + 1>, because top-left-corner of preview layout is left free.
        sal_uInt16 nDocRows = (nPages + 1) / _nCols;
        if ( ( (nPages + 1) % _nCols ) > 0 )
            ++nDocRows;
        Imp()->CurrPrevwData()->aPreviewDocSize.Height() =
                nDocRows * aMaxPageSize.Height() + (nDocRows+1) * nYFree;
    }
    // return calculated document size in preview layout
    _orPreviewDocSize = Imp()->CurrPrevwData()->aPreviewDocSize;

    if ( _bCalcScale )
    {
        // calculate scaling
        SwTwips nInitLayoutWidth = _nCols * aMaxPageSize.Width() +
                                   (_nCols+1) * nXFree;
        SwTwips nInitLayoutHeight = _nRows * aMaxPageSize.Height() +
                                    (_nRows+1) * nYFree;
        MapMode aMapMode( MAP_TWIP );
        Size aWinSize = GetOut()->PixelToLogic( *(_pPxWinSize), aMapMode );
        Fraction aXScale( aWinSize.Width(), nInitLayoutWidth );
        Fraction aYScale( aWinSize.Height(), nInitLayoutHeight );
        if( aXScale < aYScale )
            aYScale = aXScale;
        {
            // adjust scaling for Drawing layer.
            aYScale *= Fraction( 1000, 1 );
            long nNewNuminator = aYScale.operator long();
            if( nNewNuminator < 1 )
                nNewNuminator = 1;
            aYScale = Fraction( nNewNuminator, 1000 );
            // propagate scaling as zoom percentation to view option for font cache
            if( pOpt )
                pOpt->SetZoom( USHORT(nNewNuminator / 10));
        }
        aMapMode.SetScaleY( aYScale );
        aMapMode.SetScaleX( aYScale );
        // set created mapping mode with calculated scaling at output device.
        GetOut()->SetMapMode( aMapMode );
    }

    // set window size in twips at preview settings
    Imp()->CurrPrevwData()->aWinSize =
                    GetOut()->PixelToLogic( *(_pPxWinSize) );
    // validate layout sizes
    Imp()->CurrPrevwData()->bLayoutSizesValid = true;

    return true;
}

/** calculate visible pages and further needed data for current preview settings

    OD 04.12.2002 #103492#

    @author OD
*/
void ViewShell::_CalcVisiblePagesForPreview()
{
    // determine start page frame
    const SwRootFrm* pRootFrm = GetLayout();
    ASSERT( pRootFrm, "No layout - no prepare of preview paint");
    sal_uInt16 nStartPageNum = Imp()->CurrPrevwData()->nPaintPhyStartPageNum;
    const SwPageFrm* pStartPage = static_cast<const SwPageFrm*>( pRootFrm->Lower() );
    while ( pStartPage && pStartPage->GetPhyPageNum() < nStartPageNum )
    {
        pStartPage = static_cast<const SwPageFrm*>( pStartPage->GetNext() );
    }

    // get necessary preview layout data
    sal_uInt16 nCols = Imp()->CurrPrevwData()->nCols;
    sal_uInt16 nRows = Imp()->CurrPrevwData()->nRows;
    bool bDoesLayoutFits = Imp()->CurrPrevwData()->bDoesLayoutRowsFitIntoWindow;
    SwTwips nColWidth = Imp()->CurrPrevwData()->nColWidth;
    SwTwips nRowHeight = Imp()->CurrPrevwData()->nRowHeight;
    Size aMaxPageSize = Imp()->CurrPrevwData()->aMaxPageSize;
    Size aPrevwWinSize = Imp()->CurrPrevwData()->aWinSize;
    sal_uInt16 nStartCol = Imp()->CurrPrevwData()->nPaintStartCol;

    // calculate initial paint offset
    Point aInitialPaintOffset;
    if ( Imp()->CurrPrevwData()->aPaintStartPageOffset != Point( -1, -1 ) )
    {
        aInitialPaintOffset = Point(0,0) - Imp()->CurrPrevwData()->aPaintStartPageOffset;
    }
    else
        aInitialPaintOffset = Point( nXFree, nYFree );
    aInitialPaintOffset += Imp()->CurrPrevwData()->aAdditionalPaintOffset;

    // prepare loop data
    Imp()->CurrPrevwData()->aVisPages.clear();
    const SwPageFrm* pPage = pStartPage;
    sal_uInt16 nCurrCol = nStartCol;
    sal_uInt16 nConsideredRows = 0;
    Point aCurrPaintOffset = aInitialPaintOffset;
    // loop on pages to determine preview background retangles
    while ( pPage &&
            (!bDoesLayoutFits || nConsideredRows < nRows) &&
            //aCurrPaintOffset.X() < aPrevwWinSize.Width() &&
            aCurrPaintOffset.Y() < aPrevwWinSize.Height()
          )
    {
        pPage->Calc();  // don't know, if necessary.

        // consider only pages, which have to be painted.
        if ( nCurrCol < nStartCol )
        {
            // continue with next page and next column
            pPage = static_cast<const SwPageFrm*>(pPage->GetNext());
            ++nCurrCol;
            continue;
        }
        if ( aCurrPaintOffset.X() < aPrevwWinSize.Width() )
        {
            if ( pPage->GetPhyPageNum() == 1 && nCols != 1 && nCurrCol == 1)
            {
                // first page always in 2nd column
                // --> continue with increased paint offset and next column
                aCurrPaintOffset.X() += nColWidth;
                ++nCurrCol;
                continue;
            }

            // calculate data of visible page
            PrevwVisiblePage aVisPage;
            // page frame
            aVisPage.pPage = pPage;
            // size of page frame
            if ( pPage->IsEmptyPage() )
            {
                if ( pPage->GetPhyPageNum() % 2 == 0 )
                    aVisPage.aPageSize = pPage->GetPrev()->Frm().SSize();
                else
                    aVisPage.aPageSize = pPage->GetNext()->Frm().SSize();
            }
            else
                aVisPage.aPageSize = pPage->Frm().SSize();
            // position of page in preview window
            Point aPrevwWinOffset( aCurrPaintOffset );
            if ( aVisPage.aPageSize.Width() < aMaxPageSize.Width() )
                aPrevwWinOffset.X() += ( aMaxPageSize.Width() - aVisPage.aPageSize.Width() ) / 2;
            if ( aVisPage.aPageSize.Height() < aMaxPageSize.Height() )
                aPrevwWinOffset.Y() += ( aMaxPageSize.Height() - aVisPage.aPageSize.Height() ) / 2;
            aVisPage.aPrevwWinPos = aPrevwWinOffset;
            // logic position of page and mapping offset for paint
            if ( pPage->IsEmptyPage() )
            {
                aVisPage.aLogicPos = aVisPage.aPrevwWinPos;
                aVisPage.aMapOffset = Point( 0, 0 );
            }
            else
            {
                aVisPage.aLogicPos = pPage->Frm().Pos();
                aVisPage.aMapOffset = aVisPage.aPrevwWinPos - aVisPage.aLogicPos;
            }

            Imp()->CurrPrevwData()->aVisPages.push_back( aVisPage );
        }

        // prepare data for next loop
        pPage = static_cast<const SwPageFrm*>(pPage->GetNext());
        aCurrPaintOffset.X() += nColWidth;
        ++nCurrCol;
        if ( nCurrCol > nCols )
        {
            ++nConsideredRows;
            aCurrPaintOffset.X() = aInitialPaintOffset.X();
            nCurrCol = 1;
            aCurrPaintOffset.Y() += nRowHeight;
        }
    }
};

/** prepare paint of page preview

    With the valid preview layout settings - calculated and set by method
    <InitPreviewLayout> - the paint of a specific part of the virtual
    preview document is prepared. The corresponding part is given by either
    a start page (parameter <_nProposedStartPageNum>) or a absolute position
    (parameter <_aProposedStartPoint>).
    The accessibility preview will also be updated via a corresponding
    method call.

    @author OD

    @param _nProposedStartPageNum [0..<number of document pages>]
    input parameter - proposed number of page, which should be painted in
    the left-top-corner in the current output device.

    @param _nProposedStartPos [(0,0)..<PreviewDocumentSize>]
    input parameter - proposed absolute position in the virtual preview
    document, which should be painted in the left-top-corner in the current
    output device.

    @param _onStartPageNum
    output parameter - physical number of page, which will be painted in the
    left-top-corner in the current output device.

    @param _onStartPageVirtNum
    output parameter - virtual number of page, which will be painted in the
    left-top-corner in the current output device.

    @param _orDocPreviewPaintRect
    output parameter - rectangle of preview document, which will be painted.

    @return boolean, indicating, if prepare of preview paint was successful.
*/
extern void lcl_CalcAdditionalPaintOffset( CurrentPreviewData* _pPrevwData );
extern void lcl_CalcDocPrevwPaintRect( const CurrentPreviewData* _pPrevwData,
                                       Rectangle& _orDocPrevwPaintRect );

bool ViewShell::PreparePreviewPaint( const sal_uInt16 _nProposedStartPageNum,
                                     const Point      _aProposedStartPos,
                                     const Size*      _pPxWinSize,
                                     sal_uInt16&      _onStartPageNum,
                                     sal_uInt16&      _onStartPageVirtNum,
                                     Rectangle&       _orDocPreviewPaintRect,
                                     const bool       _bPaintPageAtFirstCol
                                   )
{
    sal_uInt16 nProposedStartPageNum = _nProposedStartPageNum;
    const SwRootFrm* pRootFrm = GetLayout();
    // check environment and parameters
    {
        ASSERT( pRootFrm, "No layout - no prepare of preview paint");
        if ( !pRootFrm )
            return false;

        bool bLayoutSettingsValid = Imp()->CurrPrevwData()->bLayoutInfoValid &&
                                    Imp()->CurrPrevwData()->bLayoutSizesValid;
        ASSERT( bLayoutSettingsValid,
                "no valid preview layout info/sizes - no prepare of preview paint");
        if ( !bLayoutSettingsValid )
            return false;

        bool bStartPageRangeValid =
                _nProposedStartPageNum <= Imp()->CurrPrevwData()->nPages;
        ASSERT( bStartPageRangeValid,
                "proposed start page not existing - no prepare of preview paint");
        if ( !bStartPageRangeValid )
            return false;

        bool bStartPosRangeValid =
                _aProposedStartPos.X() <= Imp()->CurrPrevwData()->aPreviewDocSize.Width() &&
                _aProposedStartPos.Y() <= Imp()->CurrPrevwData()->aPreviewDocSize.Height();
        ASSERT( bStartPosRangeValid,
                "proposed start position out of range - no prepare of preview paint");
        if ( !bStartPosRangeValid )
            return false;

        bool bWinSizeValid = _pPxWinSize->Width() != 0 && _pPxWinSize->Height() != 0;
        ASSERT ( bWinSizeValid, "no window size - no prepare of preview paint");
        if ( !bWinSizeValid )
            return false;

        bool bStartInfoValid = _nProposedStartPageNum > 0 ||
                               _aProposedStartPos != Point(0,0);
        if ( !bStartInfoValid )
            nProposedStartPageNum = 1;
    }

    // environment and parameter ok

    // update window size at preview setting data
    Imp()->CurrPrevwData()->aWinSize = GetOut()->PixelToLogic( *(_pPxWinSize) );

    // get preview setting data
    sal_uInt16 nCols = Imp()->CurrPrevwData()->nCols;
    sal_uInt16 nRows = Imp()->CurrPrevwData()->nRows;
    sal_uInt16 nPages = Imp()->CurrPrevwData()->nPages;
    sal_uInt16 nColWidth = static_cast<sal_uInt16>(Imp()->CurrPrevwData()->nColWidth);
    sal_uInt16 nRowHeight = static_cast<sal_uInt16>(Imp()->CurrPrevwData()->nRowHeight);

    sal_uInt16 nStartPageNum, nColOfProposed, nStartCol, nRowOfProposed;
    Point aStartPageOffset;
    Point aPreviewDocOffset;
    if ( nProposedStartPageNum > 0 )
    {
        // determine column and row of proposed start page in virtual preview layout
        sal_uInt16 nTmpCol = (nProposedStartPageNum+1) % nCols;
        nColOfProposed = nTmpCol ? nTmpCol : nCols;
        sal_uInt16 nTmpRow = (nProposedStartPageNum+1) / nCols;
        nRowOfProposed = nTmpCol ? nTmpRow+1 : nTmpRow;
        // determine start page
        if ( _bPaintPageAtFirstCol )
        {
            if ( nProposedStartPageNum == 1 || nRowOfProposed == 1 )
                nStartPageNum = 1;
            else
                nStartPageNum = nProposedStartPageNum - (nColOfProposed-1);
        }
        else
            nStartPageNum = nProposedStartPageNum;
        // set starting column
        if ( _bPaintPageAtFirstCol )
            nStartCol = 1;
        else
            nStartCol = nColOfProposed;
        // page offset == (-1,-1), indicating no offset and paint of free space.
        aStartPageOffset.X() = -1;
        aStartPageOffset.Y() = -1;
        // virtual preview document offset.
        if ( _bPaintPageAtFirstCol )
            aPreviewDocOffset.X() = 0;
        else
        {
            if ( nColOfProposed == 1)
                aPreviewDocOffset.X() = 0;
            else
                aPreviewDocOffset.X() = (nColOfProposed-1) * nColWidth + 1;
        }
        if ( nRowOfProposed == 1)
            aPreviewDocOffset.Y() = 0;
        else
            aPreviewDocOffset.Y() = (nRowOfProposed-1) * nRowHeight + 1;
    }
    else
    {
        // determine column and row of proposed start position
        if ( _aProposedStartPos.X() == 0 )
            nColOfProposed = 1;
        else
        {
            sal_uInt16 nTmpCol = _aProposedStartPos.X() / nColWidth;
            nColOfProposed =
                    (_aProposedStartPos.X() % nColWidth) ? nTmpCol+1 : nTmpCol;
        }
        if ( _aProposedStartPos.Y() == 0 )
            nRowOfProposed = 1;
        else
        {
            sal_uInt16 nTmpRow = _aProposedStartPos.Y() / nRowHeight;
            nRowOfProposed =
                    (_aProposedStartPos.Y() % nRowHeight) ? nTmpRow+1 : nTmpRow;
        }
        // determine start page == page at proposed start position
        if ( nRowOfProposed == 1 && nColOfProposed == 1 )
            nStartPageNum = 1;
        else
            nStartPageNum = (nRowOfProposed-1) * nCols + (nColOfProposed-1);
        // set starting column
        nStartCol = nColOfProposed;
// NOTE: <nStartPageNum> can be greater than <nPages> - consider case later
        // page offset
        aStartPageOffset.X() = (_aProposedStartPos.X() % nColWidth) - nXFree;
        aStartPageOffset.Y() = (_aProposedStartPos.Y() % nRowHeight) - nYFree;
        // virtual preview document offset.
        aPreviewDocOffset = _aProposedStartPos;
    }

    // set paint data at preview settings
    Imp()->CurrPrevwData()->nPaintPhyStartPageNum = nStartPageNum;
    Imp()->CurrPrevwData()->nPaintStartCol = nStartCol;
    Imp()->CurrPrevwData()->nPaintStartRow = nRowOfProposed;
    Imp()->CurrPrevwData()->aPaintStartPageOffset = aStartPageOffset;
    Imp()->CurrPrevwData()->aPaintPreviewDocOffset = aPreviewDocOffset;

    // determine additional paint offset, if preview layout fits into window.
    lcl_CalcAdditionalPaintOffset( Imp()->CurrPrevwData() );

    // determine rectangle to be painted from document preview
    lcl_CalcDocPrevwPaintRect( Imp()->CurrPrevwData(), _orDocPreviewPaintRect );
    Imp()->CurrPrevwData()->aPaintedPrevwDocRect = _orDocPreviewPaintRect;

    // determine pages, which are visible in the preview, and calculate needed
    // paint data
    _CalcVisiblePagesForPreview();

    // validate paint data
    Imp()->CurrPrevwData()->bPaintInfoValid = true;

    // return start page
    _onStartPageNum = nStartPageNum;
    // return virtual page number of start page
    _onStartPageVirtNum = 0;
    if ( nStartPageNum <= nPages )
    {
        const SwPageFrm* pPage = static_cast<const SwPageFrm*>( pRootFrm->Lower() );
        while ( pPage && pPage->GetPhyPageNum() < nStartPageNum )
        {
            pPage = static_cast<const SwPageFrm*>( pPage->GetNext() );
        }
        if ( pPage )
            _onStartPageVirtNum = pPage->GetVirtPageNum();
    }

    return true;
};

void lcl_CalcAdditionalPaintOffset( CurrentPreviewData* _pPrevwData )
{
    Size aWinSize = _pPrevwData->aWinSize;

    SwTwips nPrevwLayoutHeight =
            _pPrevwData->nRows * _pPrevwData->nRowHeight + nYFree;
    if ( nPrevwLayoutHeight <= aWinSize.Height() &&
         _pPrevwData->aPaintStartPageOffset.Y() <= 0 )
    {
        _pPrevwData->bDoesLayoutRowsFitIntoWindow = true;
        _pPrevwData->aAdditionalPaintOffset.Y() =
            (aWinSize.Height() - nPrevwLayoutHeight) / 2;
    }
    else
    {
        _pPrevwData->bDoesLayoutRowsFitIntoWindow = false;
        _pPrevwData->aAdditionalPaintOffset.Y() = 0;
    }

    SwTwips nPrevwLayoutWidth =
            _pPrevwData->nCols * _pPrevwData->nColWidth + nXFree;
    if ( nPrevwLayoutWidth <= aWinSize.Width() &&
         _pPrevwData->aPaintStartPageOffset.X() <= 0 )
    {
        _pPrevwData->bDoesLayoutColsFitIntoWindow = true;
        _pPrevwData->aAdditionalPaintOffset.X() =
                (aWinSize.Width() - nPrevwLayoutWidth) / 2;
    }
    else
    {
        _pPrevwData->bDoesLayoutColsFitIntoWindow = false;
        _pPrevwData->aAdditionalPaintOffset.X() = 0;
    }

}

void lcl_CalcDocPrevwPaintRect( const CurrentPreviewData* _pPrevwData,
                                Rectangle& _orDocPrevwPaintRect )
{
    Point aTopLeftPos = _pPrevwData->aPaintPreviewDocOffset;
    _orDocPrevwPaintRect.SetPos( aTopLeftPos );

    Size aSize;
    aSize.Width() = Min( _pPrevwData->aPreviewDocSize.Width() -
                            aTopLeftPos.X(),
                        _pPrevwData->aWinSize.Width() -
                            _pPrevwData->aAdditionalPaintOffset.X() );
    if ( _pPrevwData->bDoesLayoutRowsFitIntoWindow )
        aSize.Height() = _pPrevwData->nRows * _pPrevwData->nRowHeight + nYFree;
    else
        aSize.Height() = Min( _pPrevwData->aPreviewDocSize.Height() -
                                aTopLeftPos.Y(),
                              _pPrevwData->aWinSize.Height() -
                                _pPrevwData->aAdditionalPaintOffset.Y() );
    _orDocPrevwPaintRect.SetSize( aSize );
}

/** property <DoesPreviewLayoutRowsFitIntoWin> of current preview layout

    OD 03.12.2002 #103492#

    @author OD

    @return boolean, indicating that the rows of the current preview layout
    fit into the current window size.
*/
bool ViewShell::DoesPreviewLayoutRowsFitIntoWindow()
{
    ASSERT( IsPreView(), "method <ViewShell::DoesPreviewLayoutRowsFitIntoWindow called for a non-preview view shell" );
    if ( !IsPreView() )
        return false;

    return Imp()->CurrPrevwData()->bDoesLayoutRowsFitIntoWindow;
}

/** property <DoesPreviewLayoutColsFitIntoWin> of current preview layout

    OD 03.12.2002 #103492#

    @author OD

    @return boolean, indicating that the columns of the current preview layout
    fit into the current window size.
*/
bool ViewShell::DoesPreviewLayoutColsFitIntoWindow()
{
    ASSERT( IsPreView(), "method <ViewShell::DoesPreviewLayoutColsFitIntoWindow called for a non-preview view shell" );
    if ( !IsPreView() )
        return false;

    return Imp()->CurrPrevwData()->bDoesLayoutColsFitIntoWindow;
}
/** calculate start position for new scaling

    OD 04.12.2002 #103492#

    @author OD

    @return Point, start position for new scale
*/
Point ViewShell::GetPreviewStartPosForNewScale( const Fraction& _aNewScale,
                                                const Fraction& _aOldScale,
                                                const Size&     _aNewWinSize )
{
    ASSERT( IsPreView(), "method <ViewShell::GetPreviewStartPosForNewScale called for a non-preview view shell" );
    if ( !IsPreView() )
        return Point(0,0);

    const Rectangle aPaintedPrevwDocRect =
            Imp()->CurrPrevwData()->aPaintedPrevwDocRect;
    Point aNewPaintStartPos = aPaintedPrevwDocRect.TopLeft();
    if ( _aNewScale < _aOldScale )
    {
        // increase paint width by moving start point to left.
        SwTwips nPrevwLayoutWidth = nXFree +
                Imp()->CurrPrevwData()->nCols * Imp()->CurrPrevwData()->nColWidth;
        if ( nPrevwLayoutWidth < _aNewWinSize.Width() )
        {
            aNewPaintStartPos.X() = 0;
        }
        else if ( aPaintedPrevwDocRect.GetWidth() < _aNewWinSize.Width() )
        {
            aNewPaintStartPos.X() -= (_aNewWinSize.Width() - aPaintedPrevwDocRect.GetWidth()) / 2;
            if ( aNewPaintStartPos.X() < 0)
                aNewPaintStartPos.X() = 0;
        }

        if ( !Imp()->CurrPrevwData()->bDoesLayoutRowsFitIntoWindow )
        {
            // increase paint height by moving start point to top.
            SwTwips nPrevwLayoutHeight = nYFree +
                    Imp()->CurrPrevwData()->nRows * Imp()->CurrPrevwData()->nRowHeight;
            if ( nPrevwLayoutHeight < _aNewWinSize.Height() )
            {
                if ( Imp()->CurrPrevwData()->nPaintStartRow == 1)
                    aNewPaintStartPos.Y() = 0;
                else
                    aNewPaintStartPos.Y() =
                        ( (Imp()->CurrPrevwData()->nPaintStartRow - 1) *
                          Imp()->CurrPrevwData()->nRowHeight
                        ) + 1;
            }
            else if ( aPaintedPrevwDocRect.GetHeight() < _aNewWinSize.Height() )
            {
                aNewPaintStartPos.Y() -= (_aNewWinSize.Height() - aPaintedPrevwDocRect.GetHeight()) / 2;
                if ( aNewPaintStartPos.Y() < 0)
                    aNewPaintStartPos.Y() = 0;
            }
        }
    }
    else
    {
        // decrease paint width by moving start point to right
        if ( aPaintedPrevwDocRect.GetWidth() > _aNewWinSize.Width() )
            aNewPaintStartPos.X() += (aPaintedPrevwDocRect.GetWidth() - _aNewWinSize.Width()) / 2;
        // decrease paint height by moving start point to bottom
        if ( aPaintedPrevwDocRect.GetHeight() > _aNewWinSize.Height() )
            aNewPaintStartPos.Y() += (aPaintedPrevwDocRect.GetHeight() - _aNewWinSize.Height()) / 2;
    }
    return aNewPaintStartPos;
}

/** prepares re-paint of preview to bring new selected page into view.

    OD 06.12.2002 #103492#

    @author OD
*/
bool ViewShell::MovePreviewSelectedPage( const sal_uInt16 _nCurrSelectedPage,
                                         const sal_Int16  _nHoriMove,
                                         const sal_Int16  _nVertMove,
                                         sal_uInt16&      _orNewSelectedPage,
                                         sal_uInt16&      _orNewStartPage,
                                         Point&           _orNewStartPos )
{
    ASSERT( IsPreView(), "method <ViewShell::MovePreviewSelectedPage called for a non-preview view shell" );
    if ( !IsPreView() )
        return false;;

    // check parameters
    {
        bool bCurrSelectedPageNumValid = _nCurrSelectedPage > 0 &&
                _nCurrSelectedPage <= Imp()->CurrPrevwData()->nPages;
        ASSERT( bCurrSelectedPageNumValid, "current selected page number out of range" );
        if ( !bCurrSelectedPageNumValid )
            return false;
    }

    // get needed preview layout data
    sal_uInt16 nCols = Imp()->CurrPrevwData()->nCols;
    sal_uInt16 nRows = Imp()->CurrPrevwData()->nRows;
    sal_uInt16 nPages = Imp()->CurrPrevwData()->nPages;

    // determine position of current selected page
    sal_uInt16 nTmpCol = (_nCurrSelectedPage+1) % nCols;
    sal_uInt16 nCurrCol = nTmpCol > 0 ? nTmpCol : nCols;
    sal_uInt16 nCurrRow = (_nCurrSelectedPage+1) / nCols;
    if ( nTmpCol > 0 )
        ++nCurrRow;

    // determine new selected page number
    sal_uInt16 nNewSelectedPageNum = _nCurrSelectedPage;
    {
        if ( _nHoriMove != 0 )
        {
            if ( (nNewSelectedPageNum + _nHoriMove) < 1 )
                nNewSelectedPageNum = 1;
            else if ( (nNewSelectedPageNum + _nHoriMove) > nPages )
                nNewSelectedPageNum = nPages;
            else
                nNewSelectedPageNum += _nHoriMove;
        }
        if ( _nVertMove != 0 )
        {
            if ( (nNewSelectedPageNum + (_nVertMove * nCols)) < 1 )
                nNewSelectedPageNum = 1;
            else if ( (nNewSelectedPageNum + (_nVertMove * nCols)) > nPages )
                nNewSelectedPageNum = nPages;
            else
                nNewSelectedPageNum += ( _nVertMove * nCols );
        }
    }

    sal_uInt16 nNewStartPage = Imp()->CurrPrevwData()->nPaintPhyStartPageNum;
    Point aNewStartPos = Point(0,0);

    if ( !IsPageVisibleInCurrPreview( nNewSelectedPageNum ) )
    {
        if ( _nHoriMove != 0 && _nVertMove != 0 )
        {
            ASSERT( false, "missing implementation for moving preview selected page horizontal AND vertical");
            return false;
        }

        // new selected page has to be brought into view considering current
        // visible preview.
        // determine position of current selected page
        sal_uInt16 nTmpCol = (nNewSelectedPageNum+1) % nCols;
        sal_uInt16 nNewCol = nTmpCol > 0 ? nTmpCol : nCols;
        sal_uInt16 nNewRow = (nNewSelectedPageNum+1) / nCols;
        if ( nTmpCol > 0 )
            ++nNewRow;

        if ( _nHoriMove != 0 && _nVertMove == 0 )
        {
            nNewStartPage = nNewSelectedPageNum;
        }
        else if ( _nHoriMove == 0 && _nVertMove != 0 )
        {
            sal_Int16 nTotalRows = (nPages+1) / nCols;
            if ( (nPages+1) % nCols > 0 )
                ++nTotalRows;
            if ( _nVertMove > 0 &&
                 Imp()->CurrPrevwData()->bDoesLayoutRowsFitIntoWindow &&
                 nCurrRow > nTotalRows - nRows
               )
                nNewStartPage = (nTotalRows - nRows) * nCols;
            else
                nNewStartPage = nNewSelectedPageNum;
        }
    }

    _orNewSelectedPage = nNewSelectedPageNum;
    ShowPreViewSelection( nNewSelectedPageNum );
    _orNewStartPage = nNewStartPage;
    _orNewStartPos = aNewStartPos;

    return true;
}

/** determines, if page with given page number is visible in preview

    OD 05.12.2002 #103492#

    @author OD

    @return boolean, indicating, if page with given page number is visible
    in preview
*/
// predicate (unary function) for stl-algorithm "find_if" needed for method
// <ViewShell::IsPageVisibleInCurrPreview>.
struct EqualsPageNumPred
{
    const sal_uInt16 mnPageNum;
    EqualsPageNumPred( const sal_uInt16 _nPageNum ) : mnPageNum( _nPageNum ) {};
    bool operator() ( const PrevwVisiblePage& _rVisPage )
    {
        return _rVisPage.pPage->GetPhyPageNum() == mnPageNum;
    }
};
bool ViewShell::IsPageVisibleInCurrPreview( const sal_uInt16 _nPageNum )
{
    ASSERT( IsPreView(), "method <ViewShell::IsPageVisibleInCurrPreview called for a non-preview view shell" );
    if ( !IsPreView() )
        return false;

    PrevwVisiblePageConstIter aFoundVisPageIter =
            std::find_if( Imp()->CurrPrevwData()->aVisPages.begin(),
                          Imp()->CurrPrevwData()->aVisPages.end(),
                          EqualsPageNumPred( _nPageNum ) );

    bool bPageVisibleInPrevw =
            aFoundVisPageIter != Imp()->CurrPrevwData()->aVisPages.end();

    return bPageVisibleInPrevw;
}

/** helper method for <ViewShell::PaintPreview(..)>

    OD 03.12.2002 #103492#

    @author OD
*/
void lcl_CalcPrevwBackgrd( const PrevwVisiblePages _aVisPages,
                           SwRegionRects&          _orPrevwBackgrdRegion )
{
    PrevwVisiblePageConstIter aPageIter = _aVisPages.begin();
    while ( aPageIter != _aVisPages.end() )
    {
        _orPrevwBackgrdRegion -=
                SwRect( (*aPageIter).aPrevwWinPos, (*aPageIter).aPageSize );
        ++aPageIter;
    }
}
/** paint prepared preview

    OD 28.11.2002 #103492#

    @author OD

    @param _nSelectedPageNum
    input parameter - physical number of page, which should be painted as
    selected by am extra border in color COL_LIGHTBLUE.

    @param _aOutRect
    input parameter - Twip rectangle of window, which should be painted.

    @return boolean, indicating, if paint of preview was performed
*/
bool ViewShell::PaintPreview( const sal_uInt16 _nSelectedPageNum,
                              const Rectangle  _aOutRect )
{
    const SwRootFrm* pRootFrm = GetLayout();
    // check environment and parameters
    {
        ASSERT( pRootFrm, "No layout - no paint of preview" );
        if ( !pRootFrm )
            return false;

        ASSERT( Imp()->CurrPrevwData()->bPaintInfoValid,
                "invalid preview settings - no paint of preview" );
        if ( !Imp()->CurrPrevwData()->bPaintInfoValid )
            return false;
    }

    // environment and parameter ok

    if ( !GetWin() && !GetOut()->GetConnectMetaFile() )
        return false;;

    SET_CURR_SHELL( this );

    // prepare paint
    if ( Imp()->CurrPrevwData()->aVisPages.size() > 0 )
    {
        Imp()->bFirstPageInvalid = FALSE;
        Imp()->pFirstVisPage =
            const_cast<SwPageFrm*>(Imp()->CurrPrevwData()->aVisPages[0].pPage);
    }

    // paint preview background
    {
        SwRegionRects aPreviewBackgrdRegion( _aOutRect );
        // calculate preview background rectangles
        PrevwVisiblePageConstIter aPageIter =
                Imp()->CurrPrevwData()->aVisPages.begin();
        while ( aPageIter != Imp()->CurrPrevwData()->aVisPages.end() )
        {
            aPreviewBackgrdRegion -=
                    SwRect( (*aPageIter).aPrevwWinPos, (*aPageIter).aPageSize );
            ++aPageIter;
        }
        // paint preview background rectangles
        _PaintDesktop( aPreviewBackgrdRegion );
    }

    // prepare data for paint of pages
    const Rectangle aPxOutRect( GetOut()->LogicToPixel( _aOutRect ) );
    MapMode aMapMode( GetOut()->GetMapMode() );
    MapMode aSavedMapMode = aMapMode;

    Font* pEmptyPgFont = 0;

    Color aSelPgLineColor(COL_LIGHTBLUE);
    const StyleSettings& rSettings = GetWin()->GetSettings().GetStyleSettings();
    if ( rSettings.GetHighContrastMode() )
        aSelPgLineColor = rSettings.GetHighlightTextColor();

    Color aEmptyPgShadowBorderColor = rSettings.GetFieldTextColor();

    {
        PrevwVisiblePageConstIter aPageIter =
                Imp()->CurrPrevwData()->aVisPages.begin();
        while ( aPageIter != Imp()->CurrPrevwData()->aVisPages.end() )
        {
            Rectangle aPageRect( (*aPageIter).aLogicPos, (*aPageIter).aPageSize );
            aMapMode.SetOrigin( (*aPageIter).aMapOffset );
            GetOut()->SetMapMode( aMapMode );
            Rectangle aPxPageRect = GetOut()->LogicToPixel( aPageRect );
            if ( (*aPageIter).pPage->IsEmptyPage() )
            {
                const Color aRetouche( Imp()->GetRetoucheColor() );
                if( GetOut()->GetFillColor() != aRetouche )
                    GetOut()->SetFillColor( aRetouche );
                GetOut()->DrawRect( aPageRect );
                // paint empty page text
                if( !pEmptyPgFont )
                {
                    pEmptyPgFont = new Font;
                    pEmptyPgFont->SetSize( Size( 0, 80 * 20 )); // == 80 pt
                    pEmptyPgFont->SetWeight( WEIGHT_BOLD );
                    pEmptyPgFont->SetStyleName( aEmptyStr );
                    pEmptyPgFont->SetName( String::CreateFromAscii(
                            RTL_CONSTASCII_STRINGPARAM( "Helvetica" )) );
                    pEmptyPgFont->SetFamily( FAMILY_SWISS );
                    pEmptyPgFont->SetTransparent( TRUE );
                    pEmptyPgFont->SetColor( COL_GRAY );
                }
                Font aOldFont( GetOut()->GetFont() );
                GetOut()->SetFont( *pEmptyPgFont );
                GetOut()->DrawText( aPageRect, SW_RESSTR( STR_EMPTYPAGE ),
                                    TEXT_DRAW_VCENTER |
                                    TEXT_DRAW_CENTER |
                                    TEXT_DRAW_CLIP );
                GetOut()->SetFont( aOldFont );
                // paint shadow and border for empty page
                Color aFill( GetOut()->GetFillColor() );
                Color aLine( GetOut()->GetLineColor() );
                GetOut()->SetFillColor( aEmptyPgShadowBorderColor );
                GetOut()->SetLineColor( aEmptyPgShadowBorderColor );
                // bottom shadow
                Rectangle aRect( aPxPageRect.Left()+2, aPxPageRect.Bottom()+1,
                                   aPxPageRect.Right()+2, aPxPageRect.Bottom()+2 );
                aRect = GetOut()->PixelToLogic( aRect );
                GetOut()->DrawRect( aRect );
                // right shadow
                aRect.Left() = aPxPageRect.Right() + 1;
                aRect.Right() = aRect.Left() + 1;
                aRect.Top() = aPxPageRect.Top() + 2;
                aRect.Bottom() = aPxPageRect.Bottom();
                aRect = GetOut()->PixelToLogic( aRect );
                GetOut()->DrawRect( aRect );
                // border
                aRect = GetOut()->PixelToLogic( aPxPageRect );
                GetOut()->SetFillColor( Color( COL_TRANSPARENT ) );
                GetOut()->DrawRect( aRect );
                GetOut()->SetFillColor( aFill );
                GetOut()->SetLineColor( aLine );
            }
            else
            {
                aVisArea = aPageRect;
                Rectangle aPxPaintRect = aPxPageRect;
                if ( aPxOutRect.IsOver( aPxPaintRect) )
                {
                    aPxPaintRect.Intersection( aPxOutRect );
                    Rectangle aPaintRect = GetOut()->PixelToLogic( aPxPaintRect );
                    Paint( aPaintRect );
                }
            }
            if ( (*aPageIter).pPage->GetPhyPageNum() == _nSelectedPageNum )
            {
                Color aFill( GetOut()->GetFillColor() );
                Color aLine( GetOut()->GetLineColor() );
                Rectangle aRect( aPxPageRect.Left()+1, aPxPageRect.Top()+1,
                                   aPxPageRect.Right()-1, aPxPageRect.Bottom()-1 );
                aRect = GetOut()->PixelToLogic( aRect );
                GetOut()->SetFillColor( Color( COL_TRANSPARENT ) );
                GetOut()->SetLineColor( aSelPgLineColor );
                GetOut()->DrawRect( aRect );
                aRect = Rectangle( aPxPageRect.Left()+2, aPxPageRect.Top()+2,
                                   aPxPageRect.Right()-2, aPxPageRect.Bottom()-2 );
                aRect = GetOut()->PixelToLogic( aRect );
                GetOut()->DrawRect( aRect );
                GetOut()->SetFillColor( aFill );
                GetOut()->SetLineColor( aLine );
            }

            ++aPageIter;
        }
    }

    delete pEmptyPgFont;
    GetOut()->SetMapMode( aSavedMapMode );
    aVisArea.Clear();

    return true;
}

void ViewShell::RepaintCoreRect(
        const SwRect& rRect,        // Paint-Rect der Core
        USHORT nRowCol,             // Anzahl Zeilen/Spalten
        USHORT nSttPage,            // Start ab dieser Seite, eine gueltige ??
        const Size& rPageSize )     // MaxSize einer Seite
{
    if( !GetWin() && !GetOut()->GetConnectMetaFile() )
        return;                     //Fuer den Drucker tun wir hier nix

    SET_CURR_SHELL( this );

    const SwRootFrm* pRoot = GetLayout();
    ASSERT( pRoot, "Wo ist mein Layout?" );

    BYTE nRow = BYTE(nRowCol >> 8), nCol = BYTE(nRowCol & 0xff);
    SwPageFrm* pSttPage = ::lcl_GetSttPage( nRow, nCol, nSttPage, pRoot );

    // damit auch die Seiten im Paint angezeigt werden, gleich die richtigen
    // Werte setzen! (siehe auch Bug 24575)
    Imp()->bFirstPageInvalid = FALSE;
    Imp()->pFirstVisPage = pSttPage;

//    const Rectangle aPixRect( GetOut()->LogicToPixel( rRect ) );

    MapMode aMapMode( GetOut()->GetMapMode() );
    MapMode aCalcMapMode( aMapMode );
    aCalcMapMode.SetOrigin( Point() );

    Point aFreePt( nPrevViewXFreePix, nPrevViewYFreePix );
    aFreePt = GetOut()->PixelToLogic( aFreePt, aCalcMapMode );
    Point aCalcPt( aFreePt );
    long nPageHeight = aFreePt.Y() + rPageSize.Height()+1;
    SwPageFrm* pPage = pSttPage;

    // Sonderbehandlung fuer die erste Seite
    // JP 19.08.98: aber nur wenn sie mehrspaltig angezeigt wird
    int bFirstPg = !nSttPage && 1 != nCol;
    for( BYTE nCntRow = 0; pPage && nCntRow < nRow; ++nCntRow )
    {
        aCalcPt.X() = aFreePt.X();
        for( BYTE nCntCol = 0; pPage && nCntCol < nCol; ++nCntCol )
        {
            // Sonderbehandlung fuer 1. Seite, ist immer eine rechte
            if( bFirstPg )
            {
                bFirstPg = FALSE;
                aCalcPt.X() += pPage->Frm().Width()+1 + aFreePt.X();
                continue;
            }

            SwRect aTmpRect( pPage->Frm() );
            if( rRect.IsOver( aTmpRect ))
            {
                Point aPos( aCalcPt );
                aPos -= aTmpRect.Pos();

                aMapMode.SetOrigin( aPos );

                aTmpRect._Intersection( rRect );
                Rectangle aSVRect( GetOut()->
                    LogicToLogic( aTmpRect.SVRect(), &aMapMode, &aCalcMapMode ) );
                GetWin()->Invalidate( aSVRect );
            }
            aCalcPt.X() += pPage->Frm().Width()+1 + aFreePt.X();
            pPage = (SwPageFrm*)pPage->GetNext();
        }
        aCalcPt.Y() += nPageHeight;
    }

    aVisArea.Clear();       // verhinder internes Painten !!
}

    // und jetzt mal raus auf den Drucker
void ViewShell::PrintPreViewPage( SwPrtOptions& rOptions,
                                  USHORT nRowCol, SfxProgress& rProgress,
                                  const SwPagePreViewPrtData* pPrtData )
{
    if( !rOptions.aMulti.GetSelectCount() )
        return;

    // wenn kein Drucker vorhanden ist, wird nicht gedruckt
    SfxPrinter* pPrt = GetPrt();
    if( !pPrt || !pPrt->GetName().Len() )
        return;

// schoen waers gewesen !!! const MultiSelection& rMulti = rOptions.aMulti;
    MultiSelection aMulti( rOptions.aMulti );
    Range aPages( aMulti.FirstSelected(), aMulti.LastSelected() );
    if ( aPages.Max() > USHRT_MAX )
        aPages.Max() = USHRT_MAX;

    ASSERT( aPages.Min() > 0,
            "Seite 0 Drucken?" );
    ASSERT( aPages.Min() <= aPages.Max(),
            "MinSeite groesser MaxSeite." );

    // eine neue Shell fuer den Printer erzeugen
    ViewShell aShell( *this, 0 );

    aShell.pRef = new SfxPrinter( *pPrt );

    SET_CURR_SHELL( &aShell );

    aShell.PrepareForPrint( rOptions );

    // gibt es versteckte Absatzfelder, unnoetig wenn die Absaetze bereits
    // ausgeblendet sind.
    int bHiddenFlds = FALSE;
    SwFieldType* pFldType = 0;
    if ( GetViewOptions()->IsShowHiddenPara() )
    {
        pFldType = GetDoc()->GetSysFldType( RES_HIDDENPARAFLD );
        bHiddenFlds = 0 != pFldType->GetDepends();
        if( bHiddenFlds )
        {
            SwMsgPoolItem aHnt( RES_HIDDENPARA_PRINT );
            pFldType->Modify( &aHnt, 0);
        }
    }

    // Seiten fuers Drucken formatieren
    aShell.CalcPagesForPrint( (USHORT)aPages.Max(), &rProgress );

    USHORT nCopyCnt = rOptions.bCollate ? rOptions.nCopyCount : 1;
    BOOL bStartJob = FALSE;

    for ( USHORT nCnt = 0; nCnt < nCopyCnt; nCnt++ )
    {
        if( rOptions.IsPrintSingleJobs() && rOptions.GetJobName().Len() &&
            ( bStartJob || rOptions.bJobStartet ) )
        {
            pPrt->EndJob();
            rOptions.bJobStartet = TRUE;

            // Reschedule statt Yield, da Yield keine Events abarbeitet
            // und es sonst eine Endlosschleife gibt.
            while( pPrt->IsPrinting() )
                    rProgress.Reschedule();

            rOptions.MakeNextJobName();
            bStartJob = pPrt->StartJob( rOptions.GetJobName() );
        }

        const SwPageFrm *pStPage  = (SwPageFrm*)GetLayout()->Lower();
        const SwFrm     *pEndPage = pStPage;

        for( USHORT i = 1; pStPage && i < (USHORT)aPages.Min(); ++i )
            pStPage = (SwPageFrm*)pStPage->GetNext();

        if( !pStPage )          // dann wars das
        {
            if( bHiddenFlds )
            {
                SwMsgPoolItem aHnt( RES_HIDDENPARA_PRINT );
                pFldType->Modify( &aHnt, 0);
                CalcPagesForPrint( (USHORT)aPages.Max() );
            }
            return;
        }

        // unsere SttPage haben wir, dann die EndPage suchen
        USHORT nFirstPageNo = i;
        USHORT nLastPageNo  = i;
        USHORT nPageNo      = 1;

        pEndPage = pStPage;

        if( pStPage->GetNext() && (i != (USHORT)aPages.Max()) )
        {
            pEndPage = pEndPage->GetNext();
            for( ++i,++nLastPageNo;
                 pEndPage->GetNext() && i < (USHORT)aPages.Max(); ++i )
            {
                pEndPage = pEndPage->GetNext();
                ++nLastPageNo;
            }
        }

        if( rOptions.bPrintReverse )
        {
            const SwFrm *pTmp = pStPage;
            pStPage  = (SwPageFrm*)pEndPage;
            pEndPage = pTmp;
            nPageNo  = nLastPageNo;
        }
        else
            nPageNo = nFirstPageNo;


    /*-----------------01.03.95 13:06-------------------

    // Diese Options nur per Dialog einstellbar

        if( !rOptions.bPaperFromSetup )     // Schacht einstellen.
            pPrt->SetPaperBin( pStPage->GetFmt()->GetPaperBin().
                                    GetBin() );

        // Orientation einstellen: Breiter als Hoch -> Landscape,
        // sonst -> Portrait.
        if( pStPage->GetPageDesc()->GetLandscape() )
            pPrt->SetOrientation( ORIENTATION_LANDSCAPE );
        else
            pPrt->SetOrientation( ORIENTATION_PORTRAIT );

    --------------------------------------------------*/

        // ein Array fuer die Seiten anlegen, die auf eine Drucker-Seite
        // gedruckt werden sollen.
        BYTE nRow = BYTE(nRowCol >> 8), nCol = BYTE(nRowCol & 0xff);
        USHORT nPages = nRow * nCol;
        SwPageFrm** aPageArr = new SwPageFrm* [ nPages ];
        memset( aPageArr, 0, sizeof( SwPageFrm* ) * nPages );
        USHORT nCntPage = 0;

        SwTwips nCalcW = 0, nCalcH = 0, nMaxRowSz = 0, nMaxColSz = 0;

        MapMode aOld( pPrt->GetMapMode() );
        MapMode aMapMode( MAP_TWIP );
        Size aPrtSize( pPrt->PixelToLogic( pPrt->GetPaperSizePixel(), aMapMode ));
        if( pPrtData )
        {
            aPrtSize.Width() -= pPrtData->GetLeftSpace() +
                                pPrtData->GetRightSpace() +
                                ( pPrtData->GetHorzSpace() * (nCol - 1));
            aPrtSize.Height() -= pPrtData->GetTopSpace() +
                                pPrtData->GetBottomSpace() +
                                ( pPrtData->GetVertSpace() * (nRow - 1));
        }

        aMulti.Select( Range( nLastPageNo+1, USHRT_MAX ), FALSE );
        USHORT nSelCount = USHORT((aMulti.GetSelectCount()+nPages-1) / nPages);
        nSelCount *= nCopyCnt;
        USHORT nPrintCount = 1;

        const XubString aTmp( SW_RES( STR_STATSTR_PRINT ) );
        rProgress.SetText( aTmp );
        //HACK, damit die Anzeige nicht durcheinander kommt:
        rProgress.SetState( 1, nSelCount );
        rProgress.SetText( aTmp );
        bStartJob = TRUE;

        while( pStPage )
        {
            // Mag der Anwender noch ?
            rProgress.Reschedule();

            if ( !pPrt->IsJobActive() )
                break;

            if( aMulti.IsSelected( nPageNo ) )
            {
                if( rOptions.bPrintReverse )
                    aPageArr[ nPages - ++nCntPage ] = (SwPageFrm*)pStPage;
                else
                    aPageArr[ nCntPage++ ] = (SwPageFrm*)pStPage;

                const Size& rSz = pStPage->Frm().SSize();
                nCalcW += rSz.Width();
                if( nCalcH < rSz.Height() )
                    nCalcH = rSz.Height();

                if( 0 == (nCntPage % nCol ) ||          // neue Zeile
                    nCntPage == nPages || pStPage == pEndPage )
                {
                    // sollte die Seite nicht gefuellt sein, so erweiter
                    // anhand der letzen Seite. Dadurch hat auch die
                    // letze Seite die richtigen Spalten/Reihen.
                    // BUG: 17695
                    if( pStPage == pEndPage && nCntPage != nPages )
                    {
                        // dann Werte mit der letzen Seite auffuellen
                        if( nCntPage < nCol )
                            nCalcW += rSz.Width() * (nCol - nCntPage);

                        BYTE nRows = (BYTE) ( nCntPage / nCol + 1 );
                        if( nRows < nRow )
                            nCalcH += ( nRow - nRows ) * nCalcH;
                    }

                    if( nMaxColSz < nCalcW )
                        nMaxColSz = nCalcW;
                    nCalcW = 0;
                    nMaxRowSz += nCalcH;
                }

                if( nCntPage == nPages || pStPage == pEndPage )
                {
                    // den MapMode einstellen
                    aMapMode.SetOrigin( Point() );
                    {
                        Fraction aScX( aPrtSize.Width(), nMaxColSz );
                        Fraction aScY( aPrtSize.Height(), nMaxRowSz );

// JP 19.08.98: wird zur Zeit nicht ausgewertet, weil
//              der Font sich nicht enstprechend
//              stretch laesst.
#if 0
                        if( pPrtData && pPrtData->GetStretch() )
                        {
                            // fuer Drawing, damit diese ihre Objecte vernuenftig Painten
                            // koennen, auf "glatte" Prozentwerte setzen
                            long nTmp = (long)(aScY *= Fraction( 1000, 1 ));
                            if( 1 < nTmp ) --nTmp; else nTmp = 1;
                            aScY = Fraction( nTmp, 1000 );

                            nTmp = (long)(aScX *= Fraction( 1000, 1 ));
                            if( 1 < nTmp ) --nTmp; else nTmp = 1;
                            aScX = Fraction( nTmp, 1000 );
                        }
                        else
#endif
                        {
                            if( aScX < aScY )
                                aScY = aScX;

                            // fuer Drawing, damit diese ihre Objecte vernuenftig Painten
                            // koennen, auf "glatte" Prozentwerte setzen
                            aScY *= Fraction( 1000, 1 );
                            long nTmp = (long)aScY;
                            if( 1 < nTmp )
                                --nTmp;
                            else
                                nTmp = 1;
                            aScY = Fraction( nTmp, 1000 );
                            aScX = aScY;
                        }

                        aMapMode.SetScaleY( aScY );
                        aMapMode.SetScaleX( aScX );
                    }

                    Point aPrtOff( pPrt->PixelToLogic(
                                        pPrt->GetPageOffsetPixel(), aMapMode ) );
                    long nPageHeight = (nMaxRowSz / nRow);

                    // dann kann das drucken losgehen
                    rProgress.SetState( nPrintCount++, nSelCount );

                    pPrt->StartPage();
                    Point aCalcPt;
                    SwPageFrm** ppTmpPg = aPageArr;

                    // ist das Array nicht vollsteandig gefuellt ?
                    if( rOptions.bPrintReverse && nCntPage != nPages )
                    {
                        // beim Rueckwaertsdruck alle Seiten nach vorne
                        // verschieben!
                        memmove( ppTmpPg, ppTmpPg + (nPages - nCntPage),
                                    nCntPage * sizeof( SwPageFrm*) );
                        aPageArr[ nCntPage ] = 0;       // Endekennung
                    }

                    long nHOffs = 0, nVOffs = 0, nXStt = 0;
                    if( pPrtData )
                    {
                        const Fraction& rScaleX = aMapMode.GetScaleX();
                        const Fraction& rScaleY = aMapMode.GetScaleY();
                        Fraction aF( pPrtData->GetTopSpace(), 1 );
                        aCalcPt.Y() = aF /= rScaleY;
                        aF = Fraction( pPrtData->GetLeftSpace(), 1 );
                        nXStt = aF /= rScaleX;
                        aF = Fraction( pPrtData->GetVertSpace(), 1 );
                        nVOffs = aF /= rScaleY;
                        aF = Fraction( pPrtData->GetHorzSpace(), 1 );
                        nHOffs = aF /= rScaleX;
                    }

                    for( BYTE nR = 0; *ppTmpPg && nR < nRow; ++nR )
                    {
                        aCalcPt.X() = nXStt;
                        for( BYTE nC = 0; *ppTmpPg && nC < nCol; ++nC )
                        {
                            aShell.Imp()->SetFirstVisPageInvalid();
                            aShell.aVisArea = (*ppTmpPg)->Frm();

                            Point aPos( aCalcPt );
                            aPos -= aShell.aVisArea.Pos();
                            aPos -= aPrtOff;
                            aMapMode.SetOrigin( aPos );
                            pPrt->SetMapMode( aMapMode );
                            (*ppTmpPg)->GetUpper()->Paint( (*ppTmpPg)->Frm() );
                            aCalcPt.X() += nHOffs + (*ppTmpPg)->Frm().Width();
                            ++ppTmpPg;
                        }
                        aCalcPt.Y() += nVOffs + nPageHeight;
                    }
                    pPrt->EndPage();
                    SwPaintQueue::Repaint();

                    memset( aPageArr, 0, sizeof( SwPageFrm* ) * nPages );
                   nCntPage = 0;
                    nMaxRowSz = nMaxColSz = nCalcH = nCalcW = 0;
                }
            }

            if( pStPage == pEndPage )
                pStPage = 0;
            else if( rOptions.bPrintReverse )
            {
                --nPageNo;
                pStPage = (SwPageFrm*)pStPage->GetPrev();
            }
            else
            {
                ++nPageNo;
                pStPage = (SwPageFrm*)pStPage->GetNext();
            }
        }
        pPrt->SetMapMode( aOld );

        if( bHiddenFlds )
        {
            SwMsgPoolItem aHnt( RES_HIDDENPARA_PRINT );
            pFldType->Modify( &aHnt, 0);
            CalcPagesForPrint( (USHORT)aPages.Max() );
        }
        delete[] aPageArr;

        if( bStartJob )
            rOptions.bJobStartet = TRUE;
    }
    pFntCache->Flush();
}

    // Prospektdruck
void ViewShell::PrintProspect( SwPrtOptions& rOptions,
                               SfxProgress& rProgress )
{
    if( !rOptions.aMulti.GetSelectCount() )
        return;

    // wenn kein Drucker vorhanden ist, wird nicht gedruckt
    SfxPrinter* pPrt = GetPrt();
    if( !pPrt || !pPrt->GetName().Len() ||
        ( !rOptions.bPrintLeftPage && !rOptions.bPrintRightPage ))
        return;

    MultiSelection aMulti( rOptions.aMulti );
    Range aPages( aMulti.FirstSelected(), aMulti.LastSelected() );
    if ( aPages.Max() > USHRT_MAX )
        aPages.Max() = USHRT_MAX;

    ASSERT( aPages.Min() > 0,
            "Seite 0 Drucken?" );
    ASSERT( aPages.Min() <= aPages.Max(),
            "MinSeite groesser MaxSeite." );

    // eine neue Shell fuer den Printer erzeugen
    ViewShell aShell( *this, 0 );
    aShell.pRef = new SfxPrinter( *pPrt );

    SET_CURR_SHELL( &aShell );

    aShell.PrepareForPrint( rOptions );

    // gibt es versteckte Absatzfelder, unnoetig wenn die Absaetze bereits
    // ausgeblendet sind.
    int bHiddenFlds = FALSE;
    SwFieldType* pFldType = 0;
    if ( GetViewOptions()->IsShowHiddenPara() )
    {
        pFldType = GetDoc()->GetSysFldType( RES_HIDDENPARAFLD );
        bHiddenFlds = 0 != pFldType->GetDepends();
        if( bHiddenFlds )
        {
            SwMsgPoolItem aHnt( RES_HIDDENPARA_PRINT );
            pFldType->Modify( &aHnt, 0);
        }
    }

    // Seiten fuers Drucken formatieren
    aShell.CalcPagesForPrint( (USHORT)aPages.Max(), &rProgress );

    USHORT nCopyCnt = rOptions.bCollate ? rOptions.nCopyCount : 1;

    const SwPageFrm *pStPage  = (SwPageFrm*)GetLayout()->Lower();

    for( USHORT i = 1; pStPage && i < (USHORT)aPages.Min(); ++i )
        pStPage = (SwPageFrm*)pStPage->GetNext();

    if( !pStPage )          // dann wars das
    {
        if( bHiddenFlds )
        {
            SwMsgPoolItem aHnt( RES_HIDDENPARA_PRINT );
            pFldType->Modify( &aHnt, 0);
            CalcPagesForPrint( (USHORT)aPages.Max() );
        }
        return;
    }

    // unsere SttPage haben wir, dann die EndPage suchen
    SvPtrarr aArr( 255, 255 );
    aArr.Insert( (void*)pStPage, aArr.Count() );

    while( pStPage->GetNext() && i < (USHORT)aPages.Max() )
    {
        pStPage = (SwPageFrm*)pStPage->GetNext();
        if( aMulti.IsSelected( ++i ) )
            aArr.Insert( (void*)pStPage, aArr.Count() );
    }

    // auf Doppelseiten auffuellen
    if( 1 == aArr.Count() )     // eine Seite ist ein Sonderfall
        aArr.Insert( (void*)0, 1 );
    else
    {
        while( aArr.Count() & 3 )
            aArr.Insert( (void*)0, aArr.Count() );

        if( rOptions.bPrintReverse && 4 < aArr.Count() )
        {
            // das Array umsortieren
            // Array:    1 2 3 4 5 6 7 8
            // soll:     3 4 1 2 7 8 5 6
            // Algorhytmus:
            // vordere Haelfte: Austausch von 2 Pointer von Vorne vor die Haelfte
            // hintere Haelfte: Austausch von 2 Pointer von der Haelfte nach hinten

            USHORT nHalf = aArr.Count() / 2;
            USHORT nSwapCount = nHalf / 4;

            VoidPtr* ppArrStt = (VoidPtr*)aArr.GetData();
            VoidPtr* ppArrHalf = (VoidPtr*)aArr.GetData() + nHalf;

            for( int nLoop = 0; nLoop < 2; ++nLoop )
            {
                for( USHORT n = 0; n < nSwapCount; ++n )
                {
                    void* pTmp = *ppArrStt;
                    *ppArrStt++ = *(ppArrHalf-2);
                    *(ppArrHalf-2) = pTmp;

                    pTmp = *ppArrStt;
                    *ppArrStt++ = *--ppArrHalf;
                    *ppArrHalf-- = pTmp;
                }
                ppArrStt = (VoidPtr*)aArr.GetData() + nHalf;
                ppArrHalf = (VoidPtr*)aArr.GetData() + aArr.Count();
            }
        }
    }

    BOOL bStartJob = FALSE;

    for( USHORT nCnt = 0; nCnt < nCopyCnt; nCnt++ )
    {
        if( rOptions.IsPrintSingleJobs() && rOptions.GetJobName().Len() &&
            ( bStartJob || rOptions.bJobStartet ) )
        {
            pPrt->EndJob();
            rOptions.bJobStartet = TRUE;

            // Reschedule statt Yield, da Yield keine Events abarbeitet
            // und es sonst eine Endlosschleife gibt.
            while( pPrt->IsPrinting() )
                    rProgress.Reschedule();

            rOptions.MakeNextJobName();
            bStartJob = pPrt->StartJob( rOptions.GetJobName() );
        }

        // dann sorge mal dafuer, das alle Seiten in der richtigen
        // Reihenfolge stehen:
        USHORT nSPg = 0, nEPg = aArr.Count(), nStep = 1;
        if( 0 == (nEPg & 1 ))      // ungerade gibt es nicht!
            --nEPg;

        if( !rOptions.bPrintLeftPage )
            ++nStep;
        else if( !rOptions.bPrintRightPage )
        {
            ++nStep;
            ++nSPg, --nEPg;
        }

        USHORT nCntPage = (( nEPg - nSPg ) / ( 2 * nStep )) + 1;

        MapMode aOld( pPrt->GetMapMode() );
        MapMode aMapMode( MAP_TWIP );
        Size aPrtSize( pPrt->PixelToLogic( pPrt->GetPaperSizePixel(), aMapMode ) );

        const XubString aTmp( SW_RES( STR_STATSTR_PRINT ) );
        rProgress.SetText( aTmp );
        //HACK, damit die Anzeige nicht durcheinander kommt:
        rProgress.SetState( 1, nCntPage );
        rProgress.SetText( aTmp );

        for( USHORT nPrintCount = 0; nSPg < nEPg &&
                nPrintCount < nCntPage; ++nPrintCount )
        {
            // Mag der Anwender noch ?
            rProgress.Reschedule();

            if ( !pPrt->IsJobActive() )
                break;

            SwTwips nMaxRowSz, nMaxColSz;
            pStPage = (SwPageFrm*)aArr[ nSPg ];
            const SwPageFrm* pNxtPage = nEPg < aArr.Count()
                                ? pNxtPage = (SwPageFrm*)aArr[ nEPg ]
                                : 0;

            if( !pStPage )
            {
                nMaxColSz = 2 * pNxtPage->Frm().SSize().Width();
                nMaxRowSz = pNxtPage->Frm().SSize().Height();
            }
            else if( !pNxtPage )
            {
                nMaxColSz = 2 * pStPage->Frm().SSize().Width();
                nMaxRowSz = pStPage->Frm().SSize().Height();
            }
            else
            {
                nMaxColSz = pNxtPage->Frm().SSize().Width() +
                            pStPage->Frm().SSize().Width();
                nMaxRowSz = Max( pNxtPage->Frm().SSize().Height(),
                                 pStPage->Frm().SSize().Height() );
            }

            if( 0 == ( nSPg & 1 ) )     // diese muss gespiegel werden
            {
                const SwPageFrm* pTmp = pStPage;
                pStPage = pNxtPage;
                pNxtPage = pTmp;
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

            Point aPrtOff( pPrt->PixelToLogic(
                                pPrt->GetPageOffsetPixel(), aMapMode ) );
            Size aTmpPrtSize( pPrt->PixelToLogic(
                                pPrt->GetPaperSizePixel(), aMapMode ) );

            // dann kann das drucken losgehen
            bStartJob = TRUE;
            rProgress.SetState( nPrintCount, nCntPage );

            pPrt->StartPage();

            Point aSttPt;
            for( int nC = 0; nC < 2; ++nC )
            {
                if( pStPage )
                {
                    aShell.Imp()->SetFirstVisPageInvalid();
                    aShell.aVisArea = pStPage->Frm();

                    Point aPos( aSttPt );
                    aPos -= aShell.aVisArea.Pos();
                    aPos -= aPrtOff;
                    aMapMode.SetOrigin( aPos );
                    pPrt->SetMapMode( aMapMode );
                    pStPage->GetUpper()->Paint( pStPage->Frm() );
                }

                pStPage = pNxtPage;
                aSttPt.X() += aTmpPrtSize.Width() / 2;
            }

            pPrt->EndPage();
            SwPaintQueue::Repaint();
            nSPg += nStep;
            nEPg -= nStep;
        }
        pPrt->SetMapMode( aOld );

        if( bHiddenFlds )
        {
            SwMsgPoolItem aHnt( RES_HIDDENPARA_PRINT );
            pFldType->Modify( &aHnt, 0);
            CalcPagesForPrint( (USHORT)aPages.Max() );
        }

        if( bStartJob )
            rOptions.bJobStartet = TRUE;
    }
    pFntCache->Flush();
}


BOOL ViewShell::IsPreViewDocPos(
        Point& rDocPt,              // DocPos die bestimmt und auf
                                    // Layout Korrdination einer View umge-
                                    // rechnet werden soll
        USHORT nRowCol,             // Zeilen/Spalten (Row im Hi, Col im LowByte!)
        USHORT nSttPage,            // Start ab dieser Seite, eine gueltige ??
        const Size& rPageSize       // (Max)Size einer Seite
        )
{
#ifdef DEBUG
    ASSERT( false, "ViewShell::IsPreViewDocPos called [Debug-Assertion by OD]");
#endif
    BOOL bRet = FALSE;
    SET_CURR_SHELL( this );

    const SwRootFrm* pRoot = GetLayout();
    ASSERT( pRoot, "Wo ist mein Layout?" );

    BYTE nRow = BYTE(nRowCol >> 8), nCol = BYTE(nRowCol & 0xff);
    SwPageFrm* pSttPage = ::lcl_GetSttPage( nRow, nCol, nSttPage, pRoot );

    MapMode aMapMode( GetOut()->GetMapMode() );
    MapMode aCalcMapMode( aMapMode );
    aCalcMapMode.SetOrigin( Point() );

    Point aFreePt( nPrevViewXFreePix, nPrevViewYFreePix );
    aFreePt = GetOut()->PixelToLogic( aFreePt, aCalcMapMode );
    Point aCalcPt( aFreePt );
    long nPageHeight = aFreePt.Y() + rPageSize.Height()+1;
    SwPageFrm* pPage = pSttPage;

    // Sonderbehandlung fuer die erste Seite
    // JP 19.08.98: aber nur wenn sie mehrspaltig angezeigt wird
    int bFirstPg = !nSttPage && 1 != nCol;

    for( BYTE nCntRow = 0; pPage && nCntRow < nRow; ++nCntRow )
    {
        aCalcPt.X() = aFreePt.X();
        for( BYTE nCntCol = 0; pPage && nCntCol < nCol; ++nCntCol )
        {
            // Sonderbehandlung fuer 1. Seite, ist immer eine rechte
            if( bFirstPg )
            {
                bFirstPg = FALSE;
                aCalcPt.X() += pPage->Frm().Width()+1 + aFreePt.X();
                continue;
            }

            if( pPage->IsEmptyPage() )
                aCalcPt.X() += rPageSize.Width()+1 + aFreePt.X();
            else
            {
                SwRect aPgRect( aCalcPt, pPage->Frm().SSize() );
                if( aPgRect.IsInside( rDocPt ))
                {
                    // dann den richtigen Point errechnen:
                    Point aOffset( rDocPt );
                    aOffset -= aCalcPt;
                    aOffset += pPage->Frm().Pos();
                    rDocPt = aOffset;
                    bRet = TRUE;
                    break;
                }
                aCalcPt.X() += pPage->Frm().Width()+1 + aFreePt.X();
            }
            pPage = (SwPageFrm*)pPage->GetNext();
        }
        if( bRet )
            break;

        aCalcPt.Y() += nPageHeight;
    }
    return bRet;
}

Size ViewShell::GetPagePreViewPrtMaxSize() const
{
    Size aMaxSize;
    register const SwFrm *pSttPage = GetLayout()->Lower();
    while( pSttPage )
    {
        register const Size& rSize = pSttPage->Frm().SSize();
        if( aMaxSize.Width() < rSize.Width() )
            aMaxSize.Width() = rSize.Width();
        if( aMaxSize.Height() < rSize.Height() )
            aMaxSize.Height() = rSize.Height();
        pSttPage = pSttPage->GetNext();
    }
    return aMaxSize;
}

Point ViewShell::GetPreviewFreePix() const
{
    return Point( nPrevViewXFreePix, nPrevViewYFreePix );
}
