/*************************************************************************
 *
 *  $RCSfile: viewpg.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: ama $ $Date: 2002-05-22 11:36:44 $
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

const SwTwips nXFree = 142;     // == 0.25 cm
const SwTwips nYFree = 142;
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
        USHORT& rVirtPageNo )       // virtuelle SeitenNummer
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

    // was machen wir, wenn der MapMode unter einer
    // gewissen Schwelle (z.B. 10% (WW6)) ist ???

    GetOut()->SetMapMode( aMapMode );
    return nSttPage;
}

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
    Color aLineColor(COL_BLUE);
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
        if( rOptions.bSinglePrtJobs && rOptions.GetJobName().Len() &&
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
        if( rOptions.bSinglePrtJobs && rOptions.GetJobName().Len() &&
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

