/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: viewpg.cxx,v $
 * $Revision: 1.31 $
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
#include "precompiled_sw.hxx"


#include <hintids.hxx>
#include <vcl/window.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/progress.hxx>
#include <pvprtdat.hxx>
#include <viewsh.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <viewimp.hxx>
#include <viewopt.hxx>
#include <swprtopt.hxx> // SwPrtOptions
#include <fldbas.hxx>
#include <ptqueue.hxx>
#include <swregion.hxx>
#include <hints.hxx>
#include <fntcache.hxx>

#ifndef _STATSTR_HRC
#include <statstr.hrc>  // Text fuer SfxProgress
#endif
#ifndef _COMCORE_HRC
#include <comcore.hrc>
#endif

#include <IDocumentFieldsAccess.hxx>
#include <IDocumentDeviceAccess.hxx>

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
void ViewShell::AdjustOptionsForPagePreview( const SwPrtOptions &_rPrintOptions )
{
    if ( !IsPreView() )
    {
        ASSERT( false, "view shell doesn't belongs to a page preview - no adjustment of its view options");
        return;
    }

    PrepareForPrint( _rPrintOptions );

    return;
}

// output print preview on printer
// OD 05.05.2003 #i14016# - consider empty pages on calculation of scaling
// and on calculation of paint offset.
void ViewShell::PrintPreViewPage( SwPrtOptions& rOptions,
                                  USHORT nRowCol, SfxProgress& rProgress,
                                  const SwPagePreViewPrtData* pPrtData )
{
    if( !rOptions.aMulti.GetSelectCount() )
        return;

    // wenn kein Drucker vorhanden ist, wird nicht gedruckt
    SfxPrinter* pPrt = getIDocumentDeviceAccess()->getPrinter(false);
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
    if ( &GetRefDev() == pPrt )
        aShell.mpTmpRef = new SfxPrinter( *pPrt );

    SET_CURR_SHELL( &aShell );

    aShell.PrepareForPrint( rOptions );

    // gibt es versteckte Absatzfelder, unnoetig wenn die Absaetze bereits
    // ausgeblendet sind.
    int bHiddenFlds = FALSE;
    SwFieldType* pFldType = 0;
    if ( GetViewOptions()->IsShowHiddenPara() )
    {
        pFldType = getIDocumentFieldsAccess()->GetSysFldType( RES_HIDDENPARAFLD );
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

    const bool bPrintEmptyPages = rOptions.IsPrintEmptyPages();

    for ( USHORT nCnt = 0; nCnt < nCopyCnt; nCnt++ )
    {
        if( rOptions.IsPrintSingleJobs() && rOptions.GetJobName().Len() &&
            ( bStartJob || rOptions.bJobStartet ) )
        {
#ifdef TL_NOT_NOW /*TLPDF*/
            pPrt->EndJob();
#endif
            rOptions.bJobStartet = TRUE;

            // Reschedule statt Yield, da Yield keine Events abarbeitet
            // und es sonst eine Endlosschleife gibt.
            while( pPrt->IsPrinting() )
                    rProgress.Reschedule();

            rOptions.MakeNextJobName();
#ifdef TL_NOT_NOW /*TLPDF*/
            // bStartJob = pPrt->StartJob( rOptions.GetJobName() );
#endif
        }

        const SwPageFrm *pStPage  = (SwPageFrm*)GetLayout()->Lower();
        const SwFrm     *pEndPage = pStPage;
        USHORT i;

        for( i = 1; pStPage && i < (USHORT)aPages.Min(); ++i )
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
        nSelCount = nSelCount * nCopyCnt;
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

            // --> FME 2005-12-12 #b6354161# Feature - Print empty pages
            // --> OD 2007-10-22 #i79738# - correct condition:
            // always consider, if page is selected for print.
            if( aMulti.IsSelected( nPageNo ) &&
                ( bPrintEmptyPages || !pStPage->IsEmptyPage() ) )
            // <--
            {
                if( rOptions.bPrintReverse )
                    aPageArr[ nPages - ++nCntPage ] = (SwPageFrm*)pStPage;
                else
                    aPageArr[ nCntPage++ ] = (SwPageFrm*)pStPage;

                // OD 05.05.2003 #i14016# - consider empty pages on calculation
                // of page size, used for calculation of scaling.
                Size aPageSize;
                if ( pStPage->IsEmptyPage() )
                {
                    if ( pStPage->GetPhyPageNum() % 2 == 0 )
                        aPageSize = pStPage->GetPrev()->Frm().SSize();
                    else
                        aPageSize = pStPage->GetNext()->Frm().SSize();
                }
                else
                {
                    aPageSize = pStPage->Frm().SSize();
                }
                nCalcW += aPageSize.Width();
                if( nCalcH < aPageSize.Height() )
                    nCalcH = aPageSize.Height();

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
                            nCalcW += aPageSize.Width() * (nCol - nCntPage);

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

#ifdef TL_NOT_NOW /*TLPDF*/
                    pPrt->StartPage();
#endif
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
                            // OD 05.05.2003 #i14016# - consider empty pages
                            // on calculation of the paint offset for the next page.
                            aCalcPt.X() += nHOffs;
                            if ( (*ppTmpPg)->IsEmptyPage() )
                            {
                                if ( (*ppTmpPg)->GetPhyPageNum() % 2 == 0 )
                                    aCalcPt.X() += (*ppTmpPg)->GetPrev()->Frm().SSize().Width();
                                else
                                    aCalcPt.X() += (*ppTmpPg)->GetNext()->Frm().SSize().Width();
                            }
                            else
                            {
                                aCalcPt.X() += (*ppTmpPg)->Frm().Width();
                            }
                            ++ppTmpPg;
                        }
                        aCalcPt.Y() += nVOffs + nPageHeight;
                    }
#ifdef TL_NOT_NOW /*TLPDF*/
                    pPrt->EndPage();
#endif
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

// print brochure
// OD 05.05.2003 #i14016# - consider empty pages on calculation of the scaling
// for a page to be printed.
void ViewShell::PrintProspect( SwPrtOptions& rOptions,
                               SfxProgress& rProgress , BOOL bRTL)
{
    if( !rOptions.aMulti.GetSelectCount() )
        return;

    // wenn kein Drucker vorhanden ist, wird nicht gedruckt
    SfxPrinter* pPrt = getIDocumentDeviceAccess()->getPrinter(false);
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
    if ( &GetRefDev() == pPrt )
        aShell.mpTmpRef = new SfxPrinter( *pPrt );

    SET_CURR_SHELL( &aShell );

    aShell.PrepareForPrint( rOptions );

    // gibt es versteckte Absatzfelder, unnoetig wenn die Absaetze bereits
    // ausgeblendet sind.
    int bHiddenFlds = FALSE;
    SwFieldType* pFldType = 0;
    if ( GetViewOptions()->IsShowHiddenPara() )
    {
        pFldType = getIDocumentFieldsAccess()->GetSysFldType( RES_HIDDENPARAFLD );
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
    USHORT i;

    for( i = 1; pStPage && i < (USHORT)aPages.Min(); ++i )
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
#ifdef TL_NOT_NOW /*TLPDF*/
            pPrt->EndJob();
#endif
            rOptions.bJobStartet = TRUE;

            // Reschedule statt Yield, da Yield keine Events abarbeitet
            // und es sonst eine Endlosschleife gibt.
            while( pPrt->IsPrinting() )
                    rProgress.Reschedule();

            rOptions.MakeNextJobName();
            DBG_ERROR( "StartJob does not exist anymore" );
            // bStartJob = pPrt->StartJob( rOptions.GetJobName() );
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
                                ? (SwPageFrm*)aArr[ nEPg ]
                                : 0;

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
                nMaxColSz = aNxtPageSize.Width() +
                            aSttPageSize.Width();
                nMaxRowSz = Max( aNxtPageSize.Height(),
                                 aSttPageSize.Height() );
            }

            short nRtlOfs = bRTL ? 1 : 0;
            if( 0 == (( nSPg + nRtlOfs) & 1 ) )     // switch for odd number in LTR, even number in RTL
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

#ifdef TL_NOT_NOW /*TLPDF*/
            pPrt->StartPage();
#endif

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

#ifdef TL_NOT_NOW /*TLPDF*/
            pPrt->EndPage();
#endif
            SwPaintQueue::Repaint();
            nSPg = nSPg + nStep;
            nEPg = nEPg - nStep;
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
