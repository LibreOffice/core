/*************************************************************************
 *
 *  $RCSfile: tabview2.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2000-09-22 18:32:16 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"

#include <vcl/timer.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/childwin.hxx>

#include "pagedata.hxx"
#include "tabview.hxx"
#include "tabvwsh.hxx"
#include "printfun.hxx"
#include "stlpool.hxx"
#include "docsh.hxx"
#include "gridwin.hxx"
#include "olinewin.hxx"
#include "uiitems.hxx"
#include "sc.hrc"
#include "viewutil.hxx"
#include "colrowba.hxx"
#include "waitoff.hxx"
#include "globstr.hrc"

#define SC_BLOCKMODE_NONE       0
#define SC_BLOCKMODE_NORMAL     1
#define SC_BLOCKMODE_OWN        2



//
//          Markier - Funktionen
//

void ScTabView::PaintMarks(USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow )
{
    if (nStartCol > MAXCOL) nStartCol = MAXCOL;
    if (nStartRow > MAXROW) nStartRow = MAXROW;
    if (nEndCol > MAXCOL) nEndCol = MAXCOL;
    if (nEndRow > MAXROW) nEndRow = MAXROW;

    BOOL bLeft = (nStartCol==0 && nEndCol==MAXCOL);
    BOOL bTop = (nStartRow==0 && nEndRow==MAXROW);

    if (bLeft)
        PaintLeftArea( nStartRow, nEndRow );
    if (bTop)
        PaintTopArea( nStartCol, nEndCol );

    aViewData.GetDocument()->ExtendMerge( nStartCol, nStartRow, nEndCol, nEndRow,
                                            aViewData.GetTabNo() );
    PaintArea( nStartCol, nStartRow, nEndCol, nEndRow, SC_UPDATE_MARKS );
}

BOOL ScTabView::IsMarking( USHORT nCol, USHORT nRow, USHORT nTab ) const
{
    return bIsBlockMode
        && nBlockStartX == nCol
        && nBlockStartY == nRow
        && nBlockStartZ == nTab;
}

void ScTabView::InitOwnBlockMode()
{
    if (!bIsBlockMode)
    {
        //  Wenn keine (alte) Markierung mehr da ist, Anker in SelectionEngine loeschen:

        ScMarkData& rMark = aViewData.GetMarkData();
        if (!rMark.IsMarked() && !rMark.IsMultiMarked())
            GetSelEngine()->CursorPosChanging( FALSE, FALSE );

//      bIsBlockMode = TRUE;
        bIsBlockMode = SC_BLOCKMODE_OWN;            //! Variable umbenennen!
        nBlockStartX = 0;
        nBlockStartY = 0;
        nBlockStartZ = 0;
        nBlockEndX = 0;
        nBlockEndY = 0;
        nBlockEndZ = 0;

        SelectionChanged();     // Status wird mit gesetzer Markierung abgefragt
    }
}

void ScTabView::InitBlockMode( USHORT nCurX, USHORT nCurY, USHORT nCurZ,
                                BOOL bTestNeg, BOOL bCols, BOOL bRows )
{
    if (!bIsBlockMode)
    {
        if (nCurX > MAXCOL) nCurX = MAXCOL;
        if (nCurY > MAXROW) nCurY = MAXROW;

        ScMarkData& rMark = aViewData.GetMarkData();
        USHORT nTab = aViewData.GetTabNo();

        //  Teil von Markierung aufheben?
        if (bTestNeg)
        {
            if ( bCols )
                bBlockNeg = rMark.IsColumnMarked( nCurX );
            else if ( bRows )
                bBlockNeg = rMark.IsRowMarked( nCurY );
            else
                bBlockNeg = rMark.IsCellMarked( nCurX, nCurY );
        }
        else
            bBlockNeg = FALSE;
        rMark.SetMarkNegative(bBlockNeg);

//      bIsBlockMode = TRUE;
        bIsBlockMode = SC_BLOCKMODE_NORMAL;         //! Variable umbenennen!
        bBlockCols = bCols;
        bBlockRows = bRows;
        nBlockStartX = nCurX;
        nBlockStartY = nCurY;
        nBlockStartZ = nCurZ;
        nBlockEndX = nBlockStartX;
        nBlockEndY = nBlockStartY;
        nBlockEndZ = nBlockStartZ;
        if (bBlockCols)
        {
            nBlockStartY = 0;
            nBlockEndY = MAXROW;
        }
        if (bBlockRows)
        {
            nBlockStartX = 0;
            nBlockEndX = MAXCOL;
        }
        rMark.SetMarkArea( ScRange( nBlockStartX,nBlockStartY, nTab, nBlockEndX,nBlockEndY, nTab ) );
        InvertBlockMark( nBlockStartX,nBlockStartY,nBlockEndX,nBlockEndY );
    }
}

void ScTabView::DoneBlockMode( BOOL bContinue )            // Default FALSE
{
    //  Wenn zwischen Tabellen- und Header SelectionEngine gewechselt wird,
    //  wird evtl. DeselectAll gerufen, weil die andere Engine keinen Anker hat.
    //  Mit bMoveIsShift wird verhindert, dass dann die Selektion aufgehoben wird.

    if (bIsBlockMode && !bMoveIsShift)
    {
        ScMarkData& rMark = aViewData.GetMarkData();
        BOOL bFlag = rMark.GetMarkingFlag();
        rMark.SetMarking(FALSE);

        if (bBlockNeg && !bContinue)
            rMark.MarkToMulti();

        if (bContinue)
            rMark.MarkToMulti();
        else
        {
            //  Die Tabelle kann an dieser Stelle ungueltig sein, weil DoneBlockMode
            //  aus SetTabNo aufgerufen wird
            //  (z.B. wenn die aktuelle Tabelle von einer anderen View aus geloescht wird)

            USHORT nTab = aViewData.GetTabNo();
            ScDocument* pDoc = aViewData.GetDocument();
            if ( pDoc->HasTable(nTab) )
                PaintBlock( TRUE );                             // TRUE -> Block loeschen
            else
                rMark.ResetMark();
        }
//      bIsBlockMode = FALSE;
        bIsBlockMode = SC_BLOCKMODE_NONE;           //! Variable umbenennen!

        rMark.SetMarking(bFlag);
        rMark.SetMarkNegative(FALSE);
    }
}

void ScTabView::MarkCursor( USHORT nCurX, USHORT nCurY, USHORT nCurZ, BOOL bCols, BOOL bRows )
{
    if (nCurX > MAXCOL) nCurX = MAXCOL;
    if (nCurY > MAXROW) nCurY = MAXROW;

    if (!bIsBlockMode)
    {
        DBG_ERROR( "MarkCursor nicht im BlockMode" );
        InitBlockMode( nCurX, nCurY, nCurZ, FALSE, bCols, bRows );
    }

    if (bBlockCols)
        nCurY = MAXROW;
    if (bBlockRows)
        nCurX = MAXCOL;

    ScMarkData& rMark = aViewData.GetMarkData();
    DBG_ASSERT(rMark.IsMarked() || rMark.IsMultiMarked(), "MarkCursor, !IsMarked()");
    ScRange aMarkRange;
    rMark.GetMarkArea(aMarkRange);
    if (( aMarkRange.aStart.Col() != nBlockStartX && aMarkRange.aEnd.Col() != nBlockStartX ) ||
        ( aMarkRange.aStart.Row() != nBlockStartY && aMarkRange.aEnd.Row() != nBlockStartY ) ||
        ( bIsBlockMode == SC_BLOCKMODE_OWN ))
    {
        //  Markierung ist veraendert worden
        //  (z.B. MarkToSimple, wenn per negativ alles bis auf ein Rechteck geloescht wurde)
        //  oder nach InitOwnBlockMode wird mit Shift-Klick weitermarkiert...

        BOOL bOldShift = bMoveIsShift;
        bMoveIsShift = FALSE;               //  wirklich umsetzen
        DoneBlockMode(FALSE);               //! direkt Variablen setzen? (-> kein Geflacker)
        bMoveIsShift = bOldShift;

        InitBlockMode( aMarkRange.aStart.Col(), aMarkRange.aStart.Row(),
                        nBlockStartZ, rMark.IsMarkNegative(), bCols, bRows );
    }

    USHORT nOldCurX = nBlockEndX;
    USHORT nOldCurY = nBlockEndY;

    if ( nCurX != nOldCurX || nCurY != nOldCurY )
    {
        USHORT      nTab = nCurZ;

        USHORT      nDrawStartCol;
        USHORT      nDrawStartRow;
        USHORT      nDrawEndCol;
        USHORT      nDrawEndRow;

        ScUpdateRect aRect( nBlockStartX, nBlockStartY, nOldCurX, nOldCurY );
        aRect.SetNew( nBlockStartX, nBlockStartY, nCurX, nCurY );

        BOOL bCont;
        BOOL bDraw = aRect.GetXorDiff( nDrawStartCol, nDrawStartRow,
                                        nDrawEndCol, nDrawEndRow, bCont );
        nBlockEndX = nCurX;
        nBlockEndY = nCurY;
        rMark.SetMarkArea( ScRange( nBlockStartX, nBlockStartY, nTab, nBlockEndX, nBlockEndY, nTab ) );

        if ( bDraw )
        {
//?         PutInOrder( nDrawStartCol, nDrawEndCol );
//?         PutInOrder( nDrawStartRow, nDrawEndRow );

            HideAllCursors();
            InvertBlockMark( nDrawStartCol, nDrawStartRow, nDrawEndCol, nDrawEndRow );
            if (bCont)
            {
                aRect.GetContDiff( nDrawStartCol, nDrawStartRow, nDrawEndCol, nDrawEndRow );
                InvertBlockMark( nDrawStartCol, nDrawStartRow, nDrawEndCol, nDrawEndRow );
            }
            ShowAllCursors();
        }

        aViewData.GetViewShell()->UpdateInputHandler();
//      InvalidateAttribs();
    }
}

//!
//! PaintBlock in zwei Methoden aufteilen: RepaintBlock und RemoveBlock o.ae.
//!

void ScTabView::PaintBlock( BOOL bReset )
{
    ScDocument* pDoc = aViewData.GetDocument();
    ScMarkData& rMark = aViewData.GetMarkData();
    USHORT nTab = aViewData.GetTabNo();
    BOOL bMark = rMark.IsMarked();
    BOOL bMulti = rMark.IsMultiMarked();
    if (bMark || bMulti)
    {
        ScRange aMarkRange;
        HideAllCursors();
        BOOL bWasMulti = bMulti;
        if (bMulti)
        {
            BOOL bFlag = rMark.GetMarkingFlag();
            rMark.SetMarking(FALSE);
            rMark.MarkToMulti();
            rMark.GetMultiMarkArea(aMarkRange);
            rMark.MarkToSimple();
            rMark.SetMarking(bFlag);

            bMark = rMark.IsMarked();
            bMulti = rMark.IsMultiMarked();
        }
        else
            rMark.GetMarkArea(aMarkRange);

        nBlockStartX = aMarkRange.aStart.Col();
        nBlockStartY = aMarkRange.aStart.Row();
        nBlockStartZ = aMarkRange.aStart.Tab();
        nBlockEndX = aMarkRange.aEnd.Col();
        nBlockEndY = aMarkRange.aEnd.Row();
        nBlockEndZ = aMarkRange.aEnd.Tab();

        BOOL bDidReset = FALSE;

        if ( nTab>=nBlockStartZ && nTab<=nBlockEndZ )
        {
            if ( bReset )
            {
                // Invertieren beim Loeschen nur auf aktiver View
                if ( aViewData.IsActive() )
                {
                    USHORT i;
                    if ( bMulti )
                    {
                        for (i=0; i<4; i++)
                            if (pGridWin[i] && pGridWin[i]->IsVisible())
                                pGridWin[i]->InvertSimple( nBlockStartX, nBlockStartY,
                                                            nBlockEndX, nBlockEndY,
                                                            TRUE, TRUE );
                        rMark.ResetMark();
                        bDidReset = TRUE;
                    }
                    else
                    {
                        // (mis)use InvertBlockMark to remove all of the selection
                        // -> set bBlockNeg (like when removing parts of a selection)
                        //    and convert everything to Multi

                        rMark.MarkToMulti();
                        BOOL bOld = bBlockNeg;
                        bBlockNeg = TRUE;
                        // #73130# (negative) MarkArea must be set in case of repaint
                        rMark.SetMarkArea( ScRange( nBlockStartX,nBlockStartY, nTab,
                                                    nBlockEndX,nBlockEndY, nTab ) );

                        InvertBlockMark( nBlockStartX, nBlockStartY, nBlockEndX, nBlockEndY );

                        bBlockNeg = bOld;
                        rMark.ResetMark();

                        bDidReset = TRUE;
                    }

                    //  repaint if controls are touched (#69680# in both cases)

                    Rectangle aMMRect = pDoc->GetMMRect(nBlockStartX,nBlockStartY,nBlockEndX,nBlockEndY, nTab);
                    if (pDoc->HasControl( nTab, aMMRect ))
                    {
                        for (i=0; i<4; i++)
                            if (pGridWin[i] && pGridWin[i]->IsVisible())
                            {
                                //  MapMode muss logischer (1/100mm) sein !!!
                                pDoc->InvalidateControls( pGridWin[i], nTab, aMMRect );
                                pGridWin[i]->Update();
                            }
                    }
                }
            }
            else
                PaintMarks( nBlockStartX, nBlockStartY, nBlockEndX, nBlockEndY );
        }

        if ( bReset && !bDidReset )
            rMark.ResetMark();

        ShowAllCursors();
    }
}

void ScTabView::SelectAll( BOOL bContinue )
{
    ScMarkData& rMark = aViewData.GetMarkData();
    USHORT nTab = aViewData.GetTabNo();

    if (rMark.IsMarked())
    {
        ScRange aMarkRange;
        rMark.GetMarkArea( aMarkRange );
        if ( aMarkRange == ScRange( 0,0,nTab, MAXCOL,MAXROW,nTab ) )
            return;
    }

    DoneBlockMode( bContinue );
    InitBlockMode( 0,0,nTab );
    MarkCursor( MAXCOL,MAXROW,nTab );

    SelectionChanged();
}

void ScTabView::SelectAllTables()
{
    ScDocument* pDoc = aViewData.GetDocument();
    ScMarkData& rMark = aViewData.GetMarkData();
    USHORT nTab = aViewData.GetTabNo();
    USHORT nCount = pDoc->GetTableCount();

    if (nCount>1)
    {
        for (USHORT i=0; i<nCount; i++)
            rMark.SelectTable( i, TRUE );

        //      Markierungen werden per Default nicht pro Tabelle gehalten
//      pDoc->ExtendMarksFromTable( nTab );

        aViewData.GetDocShell()->PostPaintExtras();
        aViewData.GetBindings().Invalidate( FID_FILL_TAB );
    }
}

BOOL lcl_FitsInWindow( double fScaleX, double fScaleY, USHORT nZoom,
                        long nWindowX, long nWindowY, ScDocument* pDoc, USHORT nTab,
                        USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow )
{
    double fZoomFactor = (double)Fraction(nZoom,100);
    fScaleX *= fZoomFactor;
    fScaleY *= fZoomFactor;

    long nBlockX = 0;
    for (USHORT nCol=nStartCol; nCol<=nEndCol; nCol++)
    {
        USHORT nColTwips = pDoc->GetColWidth( nCol, nTab );
        if (nColTwips)
        {
            nBlockX += (long)(nColTwips * fScaleX);
            if (nBlockX > nWindowX)
                return FALSE;
        }
    }
    long nBlockY = 0;
    for (USHORT nRow=nStartRow; nRow<=nEndRow; nRow++)
    {
        USHORT nRowTwips = pDoc->GetRowHeight( nRow, nTab );
        if (nRowTwips)
        {
            nBlockY += (long)(nRowTwips * fScaleY);
            if (nBlockY > nWindowY)
                return FALSE;
        }
    }
    return TRUE;
}

USHORT ScTabView::CalcZoom( SvxZoomType eType, USHORT nOldZoom )
{
    USHORT nZoom = 0; // Ergebnis

    switch ( eType )
    {
        case SVX_ZOOM_PERCENT: // rZoom ist kein besonderer prozentualer Wert
            nZoom = nOldZoom;
            break;

        case SVX_ZOOM_OPTIMAL:  // nZoom entspricht der optimalen Gr"o\se
            {
                ScMarkData& rMark = aViewData.GetMarkData();
                ScDocument* pDoc = aViewData.GetDocument();

                if (!rMark.IsMarked())
                    nZoom = 100;                // nichts markiert
                else
                {
                    USHORT  nTab = aViewData.GetTabNo();
                    ScRange aMarkRange;
                    if ( !aViewData.GetSimpleArea( aMarkRange ) )
                        rMark.GetMultiMarkArea( aMarkRange );

                    USHORT  nStartCol = aMarkRange.aStart.Col();
                    USHORT  nStartRow = aMarkRange.aStart.Row();
                    USHORT  nStartTab = aMarkRange.aStart.Tab();
                    USHORT  nEndCol = aMarkRange.aEnd.Col();
                    USHORT  nEndRow = aMarkRange.aEnd.Row();
                    USHORT  nEndTab = aMarkRange.aEnd.Tab();

                    if ( nTab < nStartTab && nTab > nEndTab )
                        nTab = nStartTab;

                    //  Wegen der Pixel-Rundungs-Arie kann man zuverlaessig nur nachrechnen,
                    //  ob ein bestimmter Zoom in das Fenster passt - darum wird geschachtelt

                    Size aWinSize = pGridWin[aViewData.GetActivePart()]->GetOutputSizePixel();

                    ScDocShell* pDocSh = aViewData.GetDocShell();
                    double nPPTX = ScGlobal::nScreenPPTX / pDocSh->GetOutputFactor();
                    double nPPTY = ScGlobal::nScreenPPTY;

                    USHORT nMin = MINZOOM;
                    USHORT nMax = MAXZOOM;
                    while ( nMax > nMin )
                    {
                        USHORT nTest = (nMin+nMax+1)/2;
                        if ( lcl_FitsInWindow(
                                    nPPTX, nPPTY, nTest, aWinSize.Width(), aWinSize.Height(),
                                    pDoc, nTab, nStartCol, nStartRow, nEndCol, nEndRow ) )
                            nMin = nTest;
                        else
                            nMax = nTest-1;
                    }
                    DBG_ASSERT( nMin == nMax, "Schachtelung ist falsch" );
                    nZoom = nMin;

                    // 1 paar {} wg compiler bug
                    if ( nZoom != nOldZoom )
                    {   for (USHORT i=0; i<2; i++)
                        {
                            aViewData.SetPosX( (ScHSplitPos) i, nStartCol );
                            aViewData.SetPosY( (ScVSplitPos) i, nStartRow );
                        }
                    }
                }
            }
            break;

            case SVX_ZOOM_WHOLEPAGE:    // nZoom entspricht der ganzen Seite oder
            case SVX_ZOOM_PAGEWIDTH:    // nZoom entspricht der Seitenbreite
                {
                    USHORT              nCurTab     = aViewData.GetTabNo();
                    ScDocument*         pDoc        = aViewData.GetDocument();
                    ScStyleSheetPool*   pStylePool  = pDoc->GetStyleSheetPool();
                    SfxStyleSheetBase*  pStyleSheet =
                                            pStylePool->Find( pDoc->GetPageStyle( nCurTab ),
                                                              SFX_STYLE_FAMILY_PAGE );

                    DBG_ASSERT( pStyleSheet, "PageStyle not found :-/" );

                    if ( pStyleSheet )
                    {
                        ScPrintFunc aPrintFunc( aViewData.GetDocShell(),
                                                aViewData.GetViewShell()->GetPrinter(),
                                                nCurTab );

                        Size aPageSize = aPrintFunc.GetDataSize();

                        //  use the size of the largest GridWin for normal split,
                        //  or both combined for frozen panes, with the (document) size
                        //  of the frozen part added to the page size
                        //  (with frozen panes, the size of the individual parts
                        //  depends on the scale that is to be calculated)

                        if ( !pGridWin[SC_SPLIT_BOTTOMLEFT] ) return 0;
                        Size aWinSize = pGridWin[SC_SPLIT_BOTTOMLEFT]->GetOutputSizePixel();
                        ScSplitMode eHMode = aViewData.GetHSplitMode();
                        if ( eHMode != SC_SPLIT_NONE && pGridWin[SC_SPLIT_BOTTOMRIGHT] )
                        {
                            long nOtherWidth = pGridWin[SC_SPLIT_BOTTOMRIGHT]->
                                                        GetOutputSizePixel().Width();
                            if ( eHMode == SC_SPLIT_FIX )
                            {
                                aWinSize.Width() += nOtherWidth;
                                for ( USHORT nCol = aViewData.GetPosX(SC_SPLIT_LEFT);
                                        nCol < aViewData.GetFixPosX(); nCol++ )
                                    aPageSize.Width() += pDoc->GetColWidth( nCol, nCurTab );
                            }
                            else if ( nOtherWidth > aWinSize.Width() )
                                aWinSize.Width() = nOtherWidth;
                        }
                        ScSplitMode eVMode = aViewData.GetVSplitMode();
                        if ( eVMode != SC_SPLIT_NONE && pGridWin[SC_SPLIT_TOPLEFT] )
                        {
                            long nOtherHeight = pGridWin[SC_SPLIT_TOPLEFT]->
                                                        GetOutputSizePixel().Height();
                            if ( eVMode == SC_SPLIT_FIX )
                            {
                                aWinSize.Height() += nOtherHeight;
                                for ( USHORT nRow = aViewData.GetPosY(SC_SPLIT_TOP);
                                        nRow < aViewData.GetFixPosY(); nRow++ )
                                    aPageSize.Height() += pDoc->GetRowHeight( nRow, nCurTab );
                            }
                            else if ( nOtherHeight > aWinSize.Height() )
                                aWinSize.Height() = nOtherHeight;
                        }

                        double nPPTX = ScGlobal::nScreenPPTX / aViewData.GetDocShell()->GetOutputFactor();
                        double nPPTY = ScGlobal::nScreenPPTY;

                        long nZoomX = (long) ( aWinSize.Width() * 100 /
                                               ( aPageSize.Width() * nPPTX ) );
                        long nZoomY = (long) ( aWinSize.Height() * 100 /
                                               ( aPageSize.Height() * nPPTY ) );
                        long nNew = nZoomX;

                        if (eType == SVX_ZOOM_WHOLEPAGE && nZoomY < nNew)
                            nNew = nZoomY;

                        nZoom = (USHORT) nNew;
                    }
                }
                break;

        default:
            DBG_ERROR("Unknown Zoom-Revision");
            nZoom = 0;
    }

    return nZoom;
}

//  wird z.B. gerufen, wenn sich das View-Fenster verschiebt:

void ScTabView::StopMarking()
{
    ScSplitPos eActive = aViewData.GetActivePart();
    if (pGridWin[eActive])
        pGridWin[eActive]->StopMarking();

    ScHSplitPos eH = WhichH(eActive);
    if (pColBar[eH])
        pColBar[eH]->StopMarking();

    ScVSplitPos eV = WhichV(eActive);
    if (pRowBar[eV])
        pRowBar[eV]->StopMarking();
}

void ScTabView::HideNoteMarker()
{
    for (USHORT i=0; i<4; i++)
        if (pGridWin[i] && pGridWin[i]->IsVisible())
            pGridWin[i]->HideNoteMarker();
}

void ScTabView::MakeDrawLayer()
{
    if (!pDrawView)
    {
        aViewData.GetDocShell()->MakeDrawLayer();

        //  pDrawView wird per Notify gesetzt
        DBG_ASSERT(pDrawView,"ScTabView::MakeDrawLayer funktioniert nicht");
    }
}

void ScTabView::ErrorMessage( USHORT nGlobStrId )
{
    StopMarking();      // falls per Focus aus MouseButtonDown aufgerufen

    Window* pParent = aViewData.GetDialogParent();
    ScWaitCursorOff aWaitOff( pParent );
    BOOL bFocus = pParent && pParent->HasFocus();

    if(nGlobStrId==STR_PROTECTIONERR)
    {
        if(aViewData.GetDocShell()->IsReadOnly())
        {
            nGlobStrId=STR_READONLYERR;
        }
    }

    InfoBox aBox( pParent, ScGlobal::GetRscString( nGlobStrId ) );
    aBox.Execute();
    if (bFocus)
        pParent->GrabFocus();
}

Window* ScTabView::GetParentOrChild( USHORT nChildId )
{
    SfxViewFrame* pViewFrm = aViewData.GetViewShell()->GetViewFrame();

    if ( pViewFrm->HasChildWindow(nChildId) )
    {
        SfxChildWindow* pChild = pViewFrm->GetChildWindow(nChildId);
        if (pChild)
        {
            Window* pWin = pChild->GetWindow();
            if (pWin && pWin->IsVisible())
                return pWin;
        }
    }

    return aViewData.GetDialogParent();
}

void ScTabView::UpdatePageBreakData( BOOL bForcePaint )
{
    ScPageBreakData* pNewData = NULL;

    if (aViewData.IsPagebreakMode())
    {
        ScDocShell* pDocSh = aViewData.GetDocShell();
        ScDocument* pDoc = pDocSh->GetDocument();
        USHORT nTab = aViewData.GetTabNo();

        USHORT nCount = pDoc->GetPrintRangeCount(nTab);
        if (!nCount)
            nCount = 1;
        pNewData = new ScPageBreakData(nCount);

        ScPrintFunc aPrintFunc( pDocSh, pDocSh->GetPrinter(), nTab, 0,0,NULL, pNewData );
        //  ScPrintFunc fuellt im ctor die PageBreakData
        if ( nCount > 1 )
        {
            aPrintFunc.ResetBreaks(nTab);
            pNewData->AddPages();
        }

        //  Druckbereiche veraendert?
        if ( bForcePaint || ( pPageBreakData && !pPageBreakData->IsEqual( *pNewData ) ) )
            PaintGrid();
    }

    delete pPageBreakData;
    pPageBreakData = pNewData;
}



