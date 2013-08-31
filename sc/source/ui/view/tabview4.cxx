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

#include <vcl/help.hxx>
#include <vcl/svapp.hxx>

#include "tabview.hxx"
#include "document.hxx"
#include "docsh.hxx"
#include "scmod.hxx"
#include "gridwin.hxx"
#include "globstr.hrc"
#include "formulacell.hxx"
#include "dociter.hxx"

extern sal_uInt16 nScFillModeMouseModifier;             // global.cxx

// STATIC DATA -----------------------------------------------------------

//==================================================================

//
// ---  Referenz-Eingabe / Fill-Cursor
//

void ScTabView::HideTip()
{
    if ( nTipVisible )
    {
        Help::HideTip( nTipVisible );
        nTipVisible = 0;
    }
}

void ScTabView::ShowRefTip()
{
    sal_Bool bDone = false;
    if ( aViewData.GetRefType() == SC_REFTYPE_REF && Help::IsQuickHelpEnabled() )
    {
        SCCOL nStartX = aViewData.GetRefStartX();
        SCROW nStartY = aViewData.GetRefStartY();
        SCCOL nEndX   = aViewData.GetRefEndX();
        SCROW nEndY   = aViewData.GetRefEndY();
        if ( nEndX != nStartX || nEndY != nStartY )     // nicht fuer einzelne Zelle
        {
            sal_Bool bLeft = ( nEndX < nStartX );
            sal_Bool bTop  = ( nEndY < nStartY );
            PutInOrder( nStartX, nEndX );
            PutInOrder( nStartY, nEndY );
            SCCOL nCols = nEndX+1-nStartX;
            SCROW nRows = nEndY+1-nStartY;

            OUString aHelp = ScGlobal::GetRscString( STR_QUICKHELP_REF );
            aHelp = aHelp.replaceFirst("%1", OUString::number(nRows) );
            aHelp = aHelp.replaceFirst("%2", OUString::number(nCols) );

            ScSplitPos eWhich = aViewData.GetActivePart();
            Window* pWin = pGridWin[eWhich];
            if ( pWin )
            {
                Point aStart = aViewData.GetScrPos( nStartX, nStartY, eWhich );
                Point aEnd = aViewData.GetScrPos( nEndX+1, nEndY+1, eWhich );

                Point aPos( bLeft ? aStart.X() : ( aEnd.X() + 3 ),
                            bTop ? aStart.Y() : ( aEnd.Y() + 3 ) );
                sal_uInt16 nFlags = ( bLeft ? QUICKHELP_RIGHT : QUICKHELP_LEFT ) |
                                ( bTop ? QUICKHELP_BOTTOM : QUICKHELP_TOP );

                // nicht ueber die editierte Formel
                if ( !bTop && aViewData.HasEditView( eWhich ) &&
                        nEndY+1 == aViewData.GetEditViewRow() )
                {
                    //  dann an der oberen Kante der editierten Zelle ausrichten
                    aPos.Y() -= 2;      // die 3 von oben
                    nFlags = ( nFlags & ~QUICKHELP_TOP ) | QUICKHELP_BOTTOM;
                }

                Rectangle aRect( pWin->OutputToScreenPixel( aPos ), Size(1,1) );

                //! Test, ob geaendert ??

                HideTip();
                nTipVisible = Help::ShowTip( pWin, aRect, aHelp, nFlags );
                bDone = sal_True;
            }
        }
    }

    if (!bDone)
        HideTip();
}

void ScTabView::StopRefMode()
{
    if (aViewData.IsRefMode())
    {
        aViewData.SetRefMode( false, SC_REFTYPE_NONE );

        HideTip();
        UpdateShrinkOverlay();

        if ( aViewData.GetTabNo() >= aViewData.GetRefStartZ() &&
                aViewData.GetTabNo() <= aViewData.GetRefEndZ() )
        {
            ScDocument* pDoc = aViewData.GetDocument();
            SCCOL nStartX = aViewData.GetRefStartX();
            SCROW nStartY = aViewData.GetRefStartY();
            SCCOL nEndX = aViewData.GetRefEndX();
            SCROW nEndY = aViewData.GetRefEndY();
            if ( nStartX == nEndX && nStartY == nEndY )
                pDoc->ExtendMerge( nStartX, nStartY, nEndX, nEndY, aViewData.GetTabNo() );

            PaintArea( nStartX,nStartY,nEndX,nEndY, SC_UPDATE_MARKS );
        }

        pSelEngine->Reset();
        pSelEngine->SetAddMode( false );        //! sollte das nicht bei Reset passieren?

        ScSplitPos eOld = pSelEngine->GetWhich();
        ScSplitPos eNew = aViewData.GetActivePart();
        if ( eNew != eOld )
        {
            pSelEngine->SetWindow( pGridWin[ eNew ] );
            pSelEngine->SetWhich( eNew );
            pSelEngine->SetVisibleArea( Rectangle(Point(),
                                        pGridWin[eNew]->GetOutputSizePixel()) );
            pGridWin[eOld]->MoveMouseStatus(*pGridWin[eNew]);
        }
    }

    //  AlignToCursor(SC_FOLLOW_NONE): Only switch active part.
    //  This must also be done if no RefMode was active (for RangeFinder dragging),
    //  but if RefMode was set, AlignToCursor must be after SelectionEngine reset,
    //  so the SelectionEngine SetWindow call from AlignToCursor doesn't capture
    //  the mouse again when called from Tracking/MouseButtonUp (#94562#).
    AlignToCursor( aViewData.GetCurX(), aViewData.GetCurY(), SC_FOLLOW_NONE );
}

void ScTabView::DoneRefMode( bool bContinue )
{
    ScDocument* pDoc = aViewData.GetDocument();
    if ( aViewData.GetRefType() == SC_REFTYPE_REF && bContinue )
        SC_MOD()->AddRefEntry();

    sal_Bool bWasRef = aViewData.IsRefMode();
    aViewData.SetRefMode( false, SC_REFTYPE_NONE );

    HideTip();
    UpdateShrinkOverlay();

    //  Paint:
    if ( bWasRef && aViewData.GetTabNo() >= aViewData.GetRefStartZ() &&
                    aViewData.GetTabNo() <= aViewData.GetRefEndZ() )
    {
        SCCOL nStartX = aViewData.GetRefStartX();
        SCROW nStartY = aViewData.GetRefStartY();
        SCCOL nEndX = aViewData.GetRefEndX();
        SCROW nEndY = aViewData.GetRefEndY();
        if ( nStartX == nEndX && nStartY == nEndY )
            pDoc->ExtendMerge( nStartX, nStartY, nEndX, nEndY, aViewData.GetTabNo() );

        PaintArea( nStartX,nStartY,nEndX,nEndY, SC_UPDATE_MARKS );
    }
}

void ScTabView::UpdateRef( SCCOL nCurX, SCROW nCurY, SCTAB nCurZ )
{
    ScDocument* pDoc = aViewData.GetDocument();

    if (!aViewData.IsRefMode())
    {
        //  Das kommt vor, wenn bei einem Referenz-Dialog als erstes mit Control in die
        //  die Tabelle geklickt wird. Dann die neue Referenz an den alten Inhalt anhaengen:

        ScModule* pScMod = SC_MOD();
        if (pScMod->IsFormulaMode())
            pScMod->AddRefEntry();

        InitRefMode( nCurX, nCurY, nCurZ, SC_REFTYPE_REF );
    }

    if ( nCurX != aViewData.GetRefEndX() || nCurY != aViewData.GetRefEndY() ||
         nCurZ != aViewData.GetRefEndZ() )
    {
        ScMarkData& rMark = aViewData.GetMarkData();
        SCTAB nTab = aViewData.GetTabNo();

        SCCOL nStartX = aViewData.GetRefStartX();
        SCROW nStartY = aViewData.GetRefStartY();
        SCCOL nEndX = aViewData.GetRefEndX();
        SCROW nEndY = aViewData.GetRefEndY();
        if ( nStartX == nEndX && nStartY == nEndY )
            pDoc->ExtendMerge( nStartX, nStartY, nEndX, nEndY, nTab );
        ScUpdateRect aRect( nStartX, nStartY, nEndX, nEndY );

        aViewData.SetRefEnd( nCurX, nCurY, nCurZ );

        nStartX = aViewData.GetRefStartX();
        nStartY = aViewData.GetRefStartY();
        nEndX = aViewData.GetRefEndX();
        nEndY = aViewData.GetRefEndY();
        if ( nStartX == nEndX && nStartY == nEndY )
            pDoc->ExtendMerge( nStartX, nStartY, nEndX, nEndY, nTab );
        aRect.SetNew( nStartX, nStartY, nEndX, nEndY );

        ScRefType eType = aViewData.GetRefType();
        if ( eType == SC_REFTYPE_REF )
        {
            ScRange aRef(
                    aViewData.GetRefStartX(), aViewData.GetRefStartY(), aViewData.GetRefStartZ(),
                    aViewData.GetRefEndX(), aViewData.GetRefEndY(), aViewData.GetRefEndZ() );
            SC_MOD()->SetReference( aRef, pDoc, &rMark );
            ShowRefTip();
        }
        else if ( eType == SC_REFTYPE_EMBED_LT || eType == SC_REFTYPE_EMBED_RB )
        {
            PutInOrder(nStartX,nEndX);
            PutInOrder(nStartY,nEndY);
            pDoc->SetEmbedded( ScRange(nStartX,nStartY,nTab, nEndX,nEndY,nTab) );
            ScDocShell* pDocSh = aViewData.GetDocShell();
            pDocSh->UpdateOle( &aViewData, sal_True );
            pDocSh->SetDocumentModified();
        }

        SCCOL nPaintStartX;
        SCROW nPaintStartY;
        SCCOL nPaintEndX;
        SCROW nPaintEndY;
        if (aRect.GetDiff( nPaintStartX, nPaintStartY, nPaintEndX, nPaintEndY ))
            PaintArea( nPaintStartX, nPaintStartY, nPaintEndX, nPaintEndY, SC_UPDATE_MARKS );
    }

    //  Tip-Hilfe fuer Auto-Fill

    if ( aViewData.GetRefType() == SC_REFTYPE_FILL && Help::IsQuickHelpEnabled() )
    {
        OUString aHelpStr;
        ScRange aMarkRange;
        aViewData.GetSimpleArea( aMarkRange );
        SCCOL nEndX = aViewData.GetRefEndX();
        SCROW nEndY = aViewData.GetRefEndY();
        ScRange aDelRange;
        if ( aViewData.GetFillMode() == SC_FILL_MATRIX && !(nScFillModeMouseModifier & KEY_MOD1) )
        {
            aHelpStr = ScGlobal::GetRscString( STR_TIP_RESIZEMATRIX );
            SCCOL nCols = nEndX + 1 - aViewData.GetRefStartX(); // Reihenfolge ist richtig
            SCROW nRows = nEndY + 1 - aViewData.GetRefStartY();
            aHelpStr = aHelpStr.replaceFirst("%1", OUString::number(nRows) );
            aHelpStr = aHelpStr.replaceFirst("%2", OUString::number(nCols) );
        }
        else if ( aViewData.GetDelMark( aDelRange ) )
            aHelpStr = ScGlobal::GetRscString( STR_QUICKHELP_DELETE );
        else if ( nEndX != aMarkRange.aEnd.Col() || nEndY != aMarkRange.aEnd.Row() )
            aHelpStr = pDoc->GetAutoFillPreview( aMarkRange, nEndX, nEndY );

        //  je nach Richtung die obere oder untere Ecke:
        SCCOL nAddX = ( nEndX >= aMarkRange.aEnd.Col() ) ? 1 : 0;
        SCROW nAddY = ( nEndY >= aMarkRange.aEnd.Row() ) ? 1 : 0;
        Point aPos = aViewData.GetScrPos( nEndX+nAddX, nEndY+nAddY, aViewData.GetActivePart() );
        aPos.X() += 8;
        aPos.Y() += 4;
        Window* pWin = GetActiveWin();
        if ( pWin )
            aPos = pWin->OutputToScreenPixel( aPos );
        Rectangle aRect( aPos, aPos );
        sal_uInt16 nAlign = QUICKHELP_LEFT|QUICKHELP_TOP;
        Help::ShowQuickHelp(pWin, aRect, aHelpStr, nAlign);
    }
}

void ScTabView::InitRefMode( SCCOL nCurX, SCROW nCurY, SCTAB nCurZ, ScRefType eType, bool bPaint )
{
    ScDocument* pDoc = aViewData.GetDocument();
    ScMarkData& rMark = aViewData.GetMarkData();
    if (!aViewData.IsRefMode())
    {
        aViewData.SetRefMode( sal_True, eType );
        aViewData.SetRefStart( nCurX, nCurY, nCurZ );
        aViewData.SetRefEnd( nCurX, nCurY, nCurZ );

        if (nCurZ == aViewData.GetTabNo() && bPaint)
        {
            SCCOL nStartX = nCurX;
            SCROW nStartY = nCurY;
            SCCOL nEndX = nCurX;
            SCROW nEndY = nCurY;
            pDoc->ExtendMerge( nStartX, nStartY, nEndX, nEndY, aViewData.GetTabNo() );

            //! nur Markierung ueber Inhalte zeichnen!
            PaintArea( nStartX,nStartY,nEndX,nEndY, SC_UPDATE_MARKS );

            //  SetReference ohne Merge-Anpassung
            ScRange aRef( nCurX,nCurY,nCurZ, nCurX,nCurY,nCurZ );
            SC_MOD()->SetReference( aRef, pDoc, &rMark );
        }
    }
}

void ScTabView::SetScrollBar( ScrollBar& rScroll, long nRangeMax, long nVisible, long nPos, bool bLayoutRTL )
{
    if ( nVisible == 0 )
        nVisible = 1;       // #i59893# don't use visible size 0

    rScroll.SetRange( Range( 0, nRangeMax ) );
    rScroll.SetVisibleSize( nVisible );
    rScroll.SetThumbPos( nPos );

    rScroll.EnableRTL( bLayoutRTL );
}

long ScTabView::GetScrollBarPos( ScrollBar& rScroll )
{
    return rScroll.GetThumbPos();
}

//  UpdateScrollBars - sichtbaren Bereich und Scrollweite der Scrollbars einstellen

static long lcl_UpdateBar( ScrollBar& rScroll, SCCOLROW nSize )        // Size = (komplette) Zellen
{
    long nOldPos;
    long nNewPos;

    nOldPos = rScroll.GetThumbPos();
    rScroll.SetPageSize( static_cast<long>(nSize) );
    nNewPos = rScroll.GetThumbPos();
#ifndef UNX
    rScroll.SetPageSize( 1 );               // immer moeglich !
#endif

    return nNewPos - nOldPos;
}

static long lcl_GetScrollRange( SCCOLROW nDocEnd, SCCOLROW nPos, SCCOLROW nVis, SCCOLROW nMax, SCCOLROW nStart )
{
    // get the end (positive) of a scroll bar range that always starts at 0

    ++nVis;
    ++nMax;     // for partially visible cells
    SCCOLROW nEnd = std::max(nDocEnd, (SCCOLROW)(nPos+nVis)) + nVis;
    if (nEnd > nMax)
        nEnd = nMax;

    return ( nEnd - nStart );       // for range starting at 0
}

void ScTabView::UpdateScrollBars()
{
    long        nDiff;
    sal_Bool        bTop =   ( aViewData.GetVSplitMode() != SC_SPLIT_NONE );
    sal_Bool        bRight = ( aViewData.GetHSplitMode() != SC_SPLIT_NONE );
    ScDocument* pDoc = aViewData.GetDocument();
    SCTAB       nTab = aViewData.GetTabNo();
    bool        bLayoutRTL = pDoc->IsLayoutRTL( nTab );
    SCCOL       nUsedX;
    SCROW       nUsedY;
    pDoc->GetTableArea( nTab, nUsedX, nUsedY );     //! cachen !!!!!!!!!!!!!!!

    SCCOL nVisXL = 0;
    SCCOL nVisXR = 0;
    SCROW nVisYB = 0;
    SCROW nVisYT = 0;

    SCCOL nStartX = 0;
    SCROW nStartY = 0;
    if (aViewData.GetHSplitMode()==SC_SPLIT_FIX)
        nStartX = aViewData.GetFixPosX();
    if (aViewData.GetVSplitMode()==SC_SPLIT_FIX)
        nStartY = aViewData.GetFixPosY();

    nVisXL = aViewData.VisibleCellsX( SC_SPLIT_LEFT );
    long nMaxXL = lcl_GetScrollRange( nUsedX, aViewData.GetPosX(SC_SPLIT_LEFT), nVisXL, MAXCOL, 0 );
    SetScrollBar( aHScrollLeft, nMaxXL, nVisXL, aViewData.GetPosX( SC_SPLIT_LEFT ), bLayoutRTL );

    nVisYB = aViewData.VisibleCellsY( SC_SPLIT_BOTTOM );
    long nMaxYB = lcl_GetScrollRange( nUsedY, aViewData.GetPosY(SC_SPLIT_BOTTOM), nVisYB, MAXROW, nStartY );
    SetScrollBar( aVScrollBottom, nMaxYB, nVisYB, aViewData.GetPosY( SC_SPLIT_BOTTOM ) - nStartY, bLayoutRTL );

    if (bRight)
    {
        nVisXR = aViewData.VisibleCellsX( SC_SPLIT_RIGHT );
        long nMaxXR = lcl_GetScrollRange( nUsedX, aViewData.GetPosX(SC_SPLIT_RIGHT), nVisXR, MAXCOL, nStartX );
        SetScrollBar( aHScrollRight, nMaxXR, nVisXR, aViewData.GetPosX( SC_SPLIT_RIGHT ) - nStartX, bLayoutRTL );
    }

    if (bTop)
    {
        nVisYT = aViewData.VisibleCellsY( SC_SPLIT_TOP );
        long nMaxYT = lcl_GetScrollRange( nUsedY, aViewData.GetPosY(SC_SPLIT_TOP), nVisYT, MAXROW, 0 );
        SetScrollBar( aVScrollTop, nMaxYT, nVisYT, aViewData.GetPosY( SC_SPLIT_TOP ), bLayoutRTL );
    }

    //      Bereich testen

    nDiff = lcl_UpdateBar( aHScrollLeft, nVisXL );
    if (nDiff) ScrollX( nDiff, SC_SPLIT_LEFT );
    if (bRight)
    {
        nDiff = lcl_UpdateBar( aHScrollRight, nVisXR );
        if (nDiff) ScrollX( nDiff, SC_SPLIT_RIGHT );
    }

    nDiff = lcl_UpdateBar( aVScrollBottom, nVisYB );
    if (nDiff) ScrollY( nDiff, SC_SPLIT_BOTTOM );
    if (bTop)
    {
        nDiff = lcl_UpdateBar( aVScrollTop, nVisYT );
        if (nDiff) ScrollY( nDiff, SC_SPLIT_TOP );
    }

    //      set visible area for online spelling

    if ( aViewData.IsActive() )
    {
        if (UpdateVisibleRange())
            SC_MOD()->AnythingChanged();                // if visible area has changed
    }
}

#ifndef HDR_SLIDERSIZE
#define HDR_SLIDERSIZE      2
#endif

void ScTabView::InvertHorizontal( ScVSplitPos eWhich, long nDragPos )
{
    for (sal_uInt16 i=0; i<4; i++)
        if (WhichV((ScSplitPos)i)==eWhich)
        {
            ScGridWindow* pWin = pGridWin[i];
            if (pWin)
            {
                Rectangle aRect( 0,nDragPos, pWin->GetOutputSizePixel().Width()-1,nDragPos+HDR_SLIDERSIZE-1 );
                pWin->Update();
                pWin->DoInvertRect( aRect );    // Pixel
            }
        }
}

void ScTabView::InvertVertical( ScHSplitPos eWhich, long nDragPos )
{
    for (sal_uInt16 i=0; i<4; i++)
        if (WhichH((ScSplitPos)i)==eWhich)
        {
            ScGridWindow* pWin = pGridWin[i];
            if (pWin)
            {
                Rectangle aRect( nDragPos,0, nDragPos+HDR_SLIDERSIZE-1,pWin->GetOutputSizePixel().Height()-1 );
                pWin->Update();
                pWin->DoInvertRect( aRect );    // Pixel
            }
        }
}

//==================================================================

void ScTabView::InterpretVisible()
{
    //  make sure all visible cells are interpreted,
    //  so the next paint will not execute a macro function

    ScDocument* pDoc = aViewData.GetDocument();
    if ( !pDoc->GetAutoCalc() )
        return;

    SCTAB nTab = aViewData.GetTabNo();
    for (sal_uInt16 i=0; i<4; i++)
    {
        //  rely on gridwin pointers to find used panes
        //  no IsVisible test in case the whole view is not yet shown

        if (pGridWin[i])
        {
            ScHSplitPos eHWhich = WhichH( ScSplitPos(i) );
            ScVSplitPos eVWhich = WhichV( ScSplitPos(i) );

            SCCOL   nX1 = aViewData.GetPosX( eHWhich );
            SCROW   nY1 = aViewData.GetPosY( eVWhich );
            SCCOL   nX2 = nX1 + aViewData.VisibleCellsX( eHWhich );
            SCROW   nY2 = nY1 + aViewData.VisibleCellsY( eVWhich );

            if (nX2 > MAXCOL) nX2 = MAXCOL;
            if (nY2 > MAXROW) nY2 = MAXROW;

            pDoc->InterpretDirtyCells(ScRange(nX1, nY1, nTab, nX2, nY2, nTab));
        }
    }

    // #i65047# repaint during the above loop may have set the bNeedsRepaint flag
    CheckNeedsRepaint();
}





/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
