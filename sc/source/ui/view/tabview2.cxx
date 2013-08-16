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


#include "scitems.hxx"
#include <editeng/eeitem.hxx>
#include <vcl/timer.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/app.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/childwin.hxx>

#include "attrib.hxx"
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
#include "scmod.hxx"
#include "tabprotection.hxx"
#include "markdata.hxx"
#include "inputopt.hxx"

namespace {

bool isCellQualified(ScDocument* pDoc, SCCOL nCol, SCROW nRow, SCTAB nTab, bool bSelectLocked, bool bSelectUnlocked)
{
    bool bCellProtected = pDoc->HasAttrib(
        nCol, nRow, nTab, nCol, nRow, nTab, HASATTR_PROTECTED);

    if (bCellProtected && !bSelectLocked)
        return false;

    if (!bCellProtected && !bSelectUnlocked)
        return false;

    return true;
}

void moveCursorByProtRule(
    SCCOL& rCol, SCROW& rRow, SCsCOL nMovX, SCsROW nMovY, SCTAB nTab, ScDocument* pDoc)
{
    bool bSelectLocked = true;
    bool bSelectUnlocked = true;
    ScTableProtection* pTabProtection = pDoc->GetTabProtection(nTab);
    if (pTabProtection && pTabProtection->isProtected())
    {
        bSelectLocked   = pTabProtection->isOptionEnabled(ScTableProtection::SELECT_LOCKED_CELLS);
        bSelectUnlocked = pTabProtection->isOptionEnabled(ScTableProtection::SELECT_UNLOCKED_CELLS);
    }

    if (nMovX > 0)
    {
        for (SCCOL i = 0; i < nMovX && rCol < MAXCOL; ++i)
        {
            if (!isCellQualified(pDoc, rCol+1, rRow, nTab, bSelectLocked, bSelectUnlocked))
                break;
            ++rCol;
        }
    }
    else if (nMovX < 0)
    {
        for (SCCOL i = 0; i > nMovX && rCol > 0; --i)
        {
            if (!isCellQualified(pDoc, rCol-1, rRow, nTab, bSelectLocked, bSelectUnlocked))
                break;
            --rCol;
        }
    }

    if (nMovY > 0)
    {
        for (SCROW i = 0; i < nMovY && rRow < MAXROW; ++i)
        {
            if (!isCellQualified(pDoc, rCol, rRow+1, nTab, bSelectLocked, bSelectUnlocked))
                break;
            ++rRow;
        }
    }
    else if (nMovY < 0)
    {
        for (SCROW i = 0; i > nMovY && rRow > 0; --i)
        {
            if (!isCellQualified(pDoc, rCol, rRow-1, nTab, bSelectLocked, bSelectUnlocked))
                break;
            --rRow;
        }
    }
}

bool checkBoundary(SCCOL& rCol, SCROW& rRow)
{
    bool bGood = true;
    if (rCol < 0)
    {
        rCol = 0;
        bGood = false;
    }
    else if (rCol > MAXCOL)
    {
        rCol = MAXCOL;
        bGood = false;
    }

    if (rRow < 0)
    {
        rRow = 0;
        bGood = false;
    }
    else if (rRow > MAXROW)
    {
        rRow = MAXROW;
        bGood = false;
    }
    return bGood;
}

void moveCursorByMergedCell(
    SCCOL& rCol, SCROW& rRow, SCsCOL nMovX, SCsROW nMovY, SCTAB nTab,
    ScDocument* pDoc, const ScViewData& rViewData)
{
    SCCOL nOrigX = rViewData.GetCurX();
    SCROW nOrigY = rViewData.GetCurY();

    ScTableProtection* pTabProtection = pDoc->GetTabProtection(nTab);
    bool bSelectLocked = true;
    bool bSelectUnlocked = true;
    if (pTabProtection && pTabProtection->isProtected())
    {
        bSelectLocked   = pTabProtection->isOptionEnabled(ScTableProtection::SELECT_LOCKED_CELLS);
        bSelectUnlocked = pTabProtection->isOptionEnabled(ScTableProtection::SELECT_UNLOCKED_CELLS);
    }

    const ScMergeAttr* pMergeAttr = static_cast<const ScMergeAttr*>(
        pDoc->GetAttr(nOrigX, nOrigY, nTab, ATTR_MERGE));

    bool bOriginMerged = false;
    SCsCOL nColSpan = 1;
    SCsROW nRowSpan = 1;
    if (pMergeAttr && pMergeAttr->IsMerged())
    {
        nColSpan = pMergeAttr->GetColMerge();
        nRowSpan = pMergeAttr->GetRowMerge();
        bOriginMerged = true;
    }

    if (nMovX > 0)
    {
        SCCOL nOld = rCol;
        if (bOriginMerged)
        {
            // Original cell is merged.  Push the block end outside the merged region.
            if (nOrigX < MAXCOL && nOrigX < rCol && rCol <= nOrigX + nColSpan - 1)
                rCol = nOrigX + nColSpan;
        }
        else
        {
            pDoc->SkipOverlapped(rCol, rRow, nTab);
        }

        if (nOld < rCol)
        {
            // The block end has moved.  Check the protection setting and move back if needed.
            checkBoundary(rCol, rRow);
            if (!isCellQualified(pDoc, rCol, rRow, nTab, bSelectLocked, bSelectUnlocked))
                --rCol;
        }
    }
    if (nMovX < 0)
    {
        SCCOL nOld = rCol;
        if (bOriginMerged)
        {
            if (nOrigX > 0 && nOrigX <= rCol && rCol < nOrigX + nColSpan - 1)
                // Block end is still within the merged region.  Push it outside.
                rCol = nOrigX - 1;
        }
        else
        {
            pDoc->SkipOverlapped(rCol, rRow, nTab);
        }

        if (nOld > rCol)
        {
            // The block end has moved.  Check the protection setting and move back if needed.
            checkBoundary(rCol, rRow);
            if (!isCellQualified(pDoc, rCol, rRow, nTab, bSelectLocked, bSelectUnlocked))
                ++rCol;
        }
    }
    if (nMovY > 0)
    {
        SCROW nOld = rRow;
        if (bOriginMerged)
        {
            // Original cell is merged.  Push the block end outside the merged region.
            if (nOrigY < MAXROW && nOrigY < rRow && rRow <= nOrigY + nRowSpan - 1)
                rRow = nOrigY + nRowSpan;
        }
        else
        {
            pDoc->SkipOverlapped(rCol, rRow, nTab);
        }

        if (nOld < rRow)
        {
            // The block end has moved.  Check the protection setting and move back if needed.
            checkBoundary(rCol, rRow);
            if (!isCellQualified(pDoc, rCol, rRow, nTab, bSelectLocked, bSelectUnlocked))
                --rRow;
        }
    }
    if (nMovY < 0)
    {
        SCROW nOld = rRow;
        if (bOriginMerged)
        {
            if (nOrigY > 0 && nOrigY <= rRow && rRow < nOrigY + nRowSpan - 1)
                // Block end is still within the merged region.  Push it outside.
                rRow = nOrigY - 1;
        }
        else
        {
            pDoc->SkipOverlapped(rCol, rRow, nTab);
        }

        if (nOld > rRow)
        {
            // The block end has moved.  Check the protection setting and move back if needed.
            checkBoundary(rCol, rRow);
            if (!isCellQualified(pDoc, rCol, rRow, nTab, bSelectLocked, bSelectUnlocked))
                ++rRow;
        }
    }
}

}

void ScTabView::PaintMarks(SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow )
{
    if (!ValidCol(nStartCol)) nStartCol = MAXCOL;
    if (!ValidRow(nStartRow)) nStartRow = MAXROW;
    if (!ValidCol(nEndCol)) nEndCol = MAXCOL;
    if (!ValidRow(nEndRow)) nEndRow = MAXROW;

    bool bLeft = (nStartCol==0 && nEndCol==MAXCOL);
    bool bTop = (nStartRow==0 && nEndRow==MAXROW);

    if (bLeft)
        PaintLeftArea( nStartRow, nEndRow );
    if (bTop)
        PaintTopArea( nStartCol, nEndCol );

    aViewData.GetDocument()->ExtendMerge( nStartCol, nStartRow, nEndCol, nEndRow,
                                            aViewData.GetTabNo() );
    PaintArea( nStartCol, nStartRow, nEndCol, nEndRow, SC_UPDATE_MARKS );
}

bool ScTabView::IsMarking( SCCOL nCol, SCROW nRow, SCTAB nTab ) const
{
    return IsBlockMode()
        && nBlockStartX == nCol
        && nBlockStartY == nRow
        && nBlockStartZ == nTab;
}

void ScTabView::InitOwnBlockMode()
{
    if (!IsBlockMode())
    {
        //  Wenn keine (alte) Markierung mehr da ist, Anker in SelectionEngine loeschen:

        ScMarkData& rMark = aViewData.GetMarkData();
        if (!rMark.IsMarked() && !rMark.IsMultiMarked())
            GetSelEngine()->CursorPosChanging( false, false );

        meBlockMode = Own;
        nBlockStartX = 0;
        nBlockStartY = 0;
        nBlockStartZ = 0;
        nBlockEndX = 0;
        nBlockEndY = 0;
        nBlockEndZ = 0;

        SelectionChanged();     // Status wird mit gesetzer Markierung abgefragt
    }
}

void ScTabView::InitBlockMode( SCCOL nCurX, SCROW nCurY, SCTAB nCurZ,
                               bool bTestNeg, bool bCols, bool bRows )
{
    if (!IsBlockMode())
    {
        if (!ValidCol(nCurX)) nCurX = MAXCOL;
        if (!ValidRow(nCurY)) nCurY = MAXROW;

        ScMarkData& rMark = aViewData.GetMarkData();
        SCTAB nTab = aViewData.GetTabNo();

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
            bBlockNeg = false;
        rMark.SetMarkNegative(bBlockNeg);

        meBlockMode = Normal;
        bBlockCols = bCols;
        bBlockRows = bRows;
        nBlockStartX = nBlockStartXOrig = nCurX;
        nBlockStartY = nBlockStartYOrig = nCurY;
        nBlockStartZ = nCurZ;
        nBlockEndX = nOldCurX = nBlockStartX;
        nBlockEndY = nOldCurY = nBlockStartY;
        nBlockEndZ = nBlockStartZ;

        if (bBlockCols)
        {
            nBlockStartY = nBlockStartYOrig = 0;
            nBlockEndY = MAXROW;
        }

        if (bBlockRows)
        {
            nBlockStartX = nBlockStartXOrig = 0;
            nBlockEndX = MAXCOL;
        }

        rMark.SetMarkArea( ScRange( nBlockStartX,nBlockStartY, nTab, nBlockEndX,nBlockEndY, nTab ) );

        UpdateSelectionOverlay();
    }
}

void ScTabView::DoneBlockMode( bool bContinue )
{
    //  Wenn zwischen Tabellen- und Header SelectionEngine gewechselt wird,
    //  wird evtl. DeselectAll gerufen, weil die andere Engine keinen Anker hat.
    //  Mit bMoveIsShift wird verhindert, dass dann die Selektion aufgehoben wird.

    if (IsBlockMode() && !bMoveIsShift)
    {
        ScMarkData& rMark = aViewData.GetMarkData();
        bool bFlag = rMark.GetMarkingFlag();
        rMark.SetMarking(false);

        if (bBlockNeg && !bContinue)
            rMark.MarkToMulti();

        if (bContinue)
            rMark.MarkToMulti();
        else
        {
            //  Die Tabelle kann an dieser Stelle ungueltig sein, weil DoneBlockMode
            //  aus SetTabNo aufgerufen wird
            //  (z.B. wenn die aktuelle Tabelle von einer anderen View aus geloescht wird)

            SCTAB nTab = aViewData.GetTabNo();
            ScDocument* pDoc = aViewData.GetDocument();
            if ( pDoc->HasTable(nTab) )
                PaintBlock( true );                             // true -> Block loeschen
            else
                rMark.ResetMark();
        }
        meBlockMode = None;

        rMark.SetMarking(bFlag);
        rMark.SetMarkNegative(false);
    }
}

bool ScTabView::IsBlockMode() const
{
    return meBlockMode != None;
}

void ScTabView::MarkCursor( SCCOL nCurX, SCROW nCurY, SCTAB nCurZ,
                            bool bCols, bool bRows, bool bCellSelection )
{
    if (!ValidCol(nCurX)) nCurX = MAXCOL;
    if (!ValidRow(nCurY)) nCurY = MAXROW;

    if (!IsBlockMode())
    {
        OSL_FAIL( "MarkCursor nicht im BlockMode" );
        InitBlockMode( nCurX, nCurY, nCurZ, false, bCols, bRows );
    }

    if (bCols)
        nCurY = MAXROW;
    if (bRows)
        nCurX = MAXCOL;

    ScMarkData& rMark = aViewData.GetMarkData();
    OSL_ENSURE(rMark.IsMarked() || rMark.IsMultiMarked(), "MarkCursor, !IsMarked()");
    ScRange aMarkRange;
    rMark.GetMarkArea(aMarkRange);
    if (( aMarkRange.aStart.Col() != nBlockStartX && aMarkRange.aEnd.Col() != nBlockStartX ) ||
        ( aMarkRange.aStart.Row() != nBlockStartY && aMarkRange.aEnd.Row() != nBlockStartY ) ||
        ( meBlockMode == Own ))
    {
        //  Markierung ist veraendert worden
        //  (z.B. MarkToSimple, wenn per negativ alles bis auf ein Rechteck geloescht wurde)
        //  oder nach InitOwnBlockMode wird mit Shift-Klick weitermarkiert...

        bool bOldShift = bMoveIsShift;
        bMoveIsShift = false;               //  wirklich umsetzen
        DoneBlockMode(false);               //! direkt Variablen setzen? (-> kein Geflacker)
        bMoveIsShift = bOldShift;

        InitBlockMode( aMarkRange.aStart.Col(), aMarkRange.aStart.Row(),
                        nBlockStartZ, rMark.IsMarkNegative(), bCols, bRows );
    }

    if ( nCurX != nOldCurX || nCurY != nOldCurY )
    {
        // Current cursor has moved

        SCTAB nTab = nCurZ;

        if ( bCellSelection )
        {
            // Expand selection area accordingly when the current selection ends
            // with a merged cell.
            SCsCOL nCurXOffset = 0;
            SCsCOL nBlockStartXOffset = 0;
            SCsROW nCurYOffset = 0;
            SCsROW nBlockStartYOffset = 0;
            bool bBlockStartMerged = false;
            const ScMergeAttr* pMergeAttr = NULL;
            ScDocument* pDocument = aViewData.GetDocument();

            // The following block checks whether or not the "BlockStart" (anchor)
            // cell is merged.  If it's merged, it'll then move the position of the
            // anchor cell to the corner that's diagonally opposite of the
            // direction of a current selection area.  For instance, if a current
            // selection is moving in the upperleft direction, the anchor cell will
            // move to the lower-right corner of the merged anchor cell, and so on.

            pMergeAttr = static_cast<const ScMergeAttr*>(
                pDocument->GetAttr( nBlockStartXOrig, nBlockStartYOrig, nTab, ATTR_MERGE ) );
            if ( pMergeAttr->IsMerged() )
            {
                SCsCOL nColSpan = pMergeAttr->GetColMerge();
                SCsROW nRowSpan = pMergeAttr->GetRowMerge();

                if ( !( nCurX >= nBlockStartXOrig + nColSpan - 1 && nCurY >= nBlockStartYOrig + nRowSpan - 1 ) )
                {
                    nBlockStartX = nCurX >= nBlockStartXOrig ? nBlockStartXOrig : nBlockStartXOrig + nColSpan - 1;
                    nBlockStartY = nCurY >= nBlockStartYOrig ? nBlockStartYOrig : nBlockStartYOrig + nRowSpan - 1;
                    nCurXOffset  = (nCurX >= nBlockStartXOrig && nCurX < nBlockStartXOrig + nColSpan - 1) ?
                        nBlockStartXOrig - nCurX + nColSpan - 1 : 0;
                    nCurYOffset  = (nCurY >= nBlockStartYOrig && nCurY < nBlockStartYOrig + nRowSpan - 1) ?
                        nBlockStartYOrig - nCurY + nRowSpan - 1 : 0;
                    bBlockStartMerged = sal_True;
                }
            }

            // The following block checks whether or not the current cell is
            // merged.  If it is, it'll then set the appropriate X & Y offset
            // values (nCurXOffset & nCurYOffset) such that the selection area will
            // grow by those specified offset amounts.  Note that the values of
            // nCurXOffset/nCurYOffset may also be specified in the previous code
            // block, in which case whichever value is greater will take on.

            pMergeAttr = static_cast<const ScMergeAttr*>(
                pDocument->GetAttr( nCurX, nCurY, nTab, ATTR_MERGE ) );
            if ( pMergeAttr->IsMerged() )
            {
                SCsCOL nColSpan = pMergeAttr->GetColMerge();
                SCsROW nRowSpan = pMergeAttr->GetRowMerge();

                if ( !( nBlockStartX >= nCurX + nColSpan - 1 && nBlockStartY >= nCurY + nRowSpan - 1 ) )
                {
                    if ( nBlockStartX <= nCurX + nColSpan - 1 )
                    {
                        SCsCOL nCurXOffsetTemp = (nCurX < nCurX + nColSpan - 1) ? nColSpan - 1 : 0;
                        nCurXOffset = nCurXOffset > nCurXOffsetTemp ? nCurXOffset : nCurXOffsetTemp;
                    }
                    if ( nBlockStartY <= nCurY + nRowSpan - 1 )
                    {
                        SCsROW nCurYOffsetTemp = (nCurY < nCurY + nRowSpan - 1) ? nRowSpan - 1 : 0;
                        nCurYOffset = nCurYOffset > nCurYOffsetTemp ? nCurYOffset : nCurYOffsetTemp;
                    }
                    if ( !( nBlockStartX <= nCurX && nBlockStartY <= nCurY ) &&
                         !( nBlockStartX > nCurX + nColSpan - 1 && nBlockStartY > nCurY + nRowSpan - 1 ) )
                    {
                        nBlockStartXOffset = (nBlockStartX > nCurX && nBlockStartX <= nCurX + nColSpan - 1) ? nCurX - nBlockStartX : 0;
                        nBlockStartYOffset = (nBlockStartY > nCurY && nBlockStartY <= nCurY + nRowSpan - 1) ? nCurY - nBlockStartY : 0;
                    }
                }
            }
            else
            {
                // The current cell is not merged.  Move the anchor cell to its
                // original position.
                if ( !bBlockStartMerged )
                {
                    nBlockStartX = nBlockStartXOrig;
                    nBlockStartY = nBlockStartYOrig;
                }
            }

            nBlockStartX = nBlockStartX + nBlockStartXOffset >= 0 ? nBlockStartX + nBlockStartXOffset : 0;
            nBlockStartY = nBlockStartY + nBlockStartYOffset >= 0 ? nBlockStartY + nBlockStartYOffset : 0;
            nBlockEndX = nCurX + nCurXOffset > MAXCOL ? MAXCOL : nCurX + nCurXOffset;
            nBlockEndY = nCurY + nCurYOffset > MAXROW ? MAXROW : nCurY + nCurYOffset;
        }
        else
        {
            nBlockEndX = nCurX;
            nBlockEndY = nCurY;
        }

        // Set new selection area
        rMark.SetMarkArea( ScRange( nBlockStartX, nBlockStartY, nTab, nBlockEndX, nBlockEndY, nTab ) );

        UpdateSelectionOverlay();
        SelectionChanged();

        nOldCurX = nCurX;
        nOldCurY = nCurY;

        aViewData.GetViewShell()->UpdateInputHandler();
    }

    if ( !bCols && !bRows )
        aHdrFunc.SetAnchorFlag( false );
}

void ScTabView::GetPageMoveEndPosition(SCsCOL nMovX, SCsROW nMovY, SCsCOL& rPageX, SCsROW& rPageY)
{
    SCCOL nCurX;
    SCROW nCurY;
    if (aViewData.IsRefMode())
    {
        nCurX = aViewData.GetRefEndX();
        nCurY = aViewData.GetRefEndY();
    }
    else if (IsBlockMode())
    {
        // block end position.
        nCurX = nBlockEndX;
        nCurY = nBlockEndY;
    }
    else
    {
        // cursor position
        nCurX = aViewData.GetCurX();
        nCurY = aViewData.GetCurY();
    }

    ScSplitPos eWhich = aViewData.GetActivePart();
    ScHSplitPos eWhichX = WhichH( eWhich );
    ScVSplitPos eWhichY = WhichV( eWhich );

    SCsCOL nPageX;
    SCsROW nPageY;
    if (nMovX >= 0)
        nPageX = ((SCsCOL) aViewData.CellsAtX( nCurX, 1, eWhichX )) * nMovX;
    else
        nPageX = ((SCsCOL) aViewData.CellsAtX( nCurX, -1, eWhichX )) * nMovX;

    if (nMovY >= 0)
        nPageY = ((SCsROW) aViewData.CellsAtY( nCurY, 1, eWhichY )) * nMovY;
    else
        nPageY = ((SCsROW) aViewData.CellsAtY( nCurY, -1, eWhichY )) * nMovY;

    if (nMovX != 0 && nPageX == 0) nPageX = (nMovX>0) ? 1 : -1;
    if (nMovY != 0 && nPageY == 0) nPageY = (nMovY>0) ? 1 : -1;

    rPageX = nPageX;
    rPageY = nPageY;
}

void ScTabView::GetAreaMoveEndPosition(SCsCOL nMovX, SCsROW nMovY, ScFollowMode eMode,
                                       SCsCOL& rAreaX, SCsROW& rAreaY, ScFollowMode& rMode)
{
    SCCOL nNewX = -1;
    SCROW nNewY = -1;

    // current cursor position.
    SCCOL nCurX = aViewData.GetCurX();
    SCROW nCurY = aViewData.GetCurY();

    if (aViewData.IsRefMode())
    {
        nNewX = aViewData.GetRefEndX();
        nNewY = aViewData.GetRefEndY();
        nCurX = aViewData.GetRefStartX();
        nCurY = aViewData.GetRefStartY();
    }
    else if (IsBlockMode())
    {
        // block end position.
        nNewX = nBlockEndX;
        nNewY = nBlockEndY;
    }
    else
    {
        nNewX = nCurX;
        nNewY = nCurY;
    }

    ScDocument* pDoc = aViewData.GetDocument();
    SCTAB nTab = aViewData.GetTabNo();

    //  FindAreaPos kennt nur -1 oder 1 als Richtung
    ScModule* pScModule = SC_MOD();
    bool bLegacyCellSelection = pScModule->GetInputOptions().GetLegacyCellSelection();
    SCCOL nVirtualX = bLegacyCellSelection ? nNewX : nCurX;
    SCROW nVirtualY = bLegacyCellSelection ? nNewY : nCurY;

    SCsCOLROW i;
    if ( nMovX > 0 )
        for ( i=0; i<nMovX; i++ )
            pDoc->FindAreaPos( nNewX, nVirtualY, nTab,  SC_MOVE_RIGHT );
    if ( nMovX < 0 )
        for ( i=0; i<-nMovX; i++ )
            pDoc->FindAreaPos( nNewX, nVirtualY, nTab,  SC_MOVE_LEFT );
    if ( nMovY > 0 )
        for ( i=0; i<nMovY; i++ )
            pDoc->FindAreaPos( nVirtualX, nNewY, nTab,  SC_MOVE_DOWN );
    if ( nMovY < 0 )
        for ( i=0; i<-nMovY; i++ )
            pDoc->FindAreaPos( nVirtualX, nNewY, nTab,  SC_MOVE_UP );

    if (eMode==SC_FOLLOW_JUMP)                  // unten/rechts nicht zuviel grau anzeigen
    {
        if (nMovX != 0 && nNewX == MAXCOL)
            eMode = SC_FOLLOW_LINE;
        if (nMovY != 0 && nNewY == MAXROW)
            eMode = SC_FOLLOW_LINE;
    }

    if (aViewData.IsRefMode())
    {
        rAreaX = nNewX - aViewData.GetRefEndX();
        rAreaY = nNewY - aViewData.GetRefEndY();
    }
    else if (IsBlockMode())
    {
        rAreaX = nNewX - nBlockEndX;
        rAreaY = nNewY - nBlockEndY;
    }
    else
    {
        rAreaX = nNewX - nCurX;
        rAreaY = nNewY - nCurY;
    }
    rMode = eMode;
}

void ScTabView::SkipCursorHorizontal(SCsCOL& rCurX, SCsROW& rCurY, SCsCOL nOldX, SCsROW nMovX)
{
    ScDocument* pDoc = aViewData.GetDocument();
    SCTAB nTab = aViewData.GetTabNo();

    bool bSkipProtected = false, bSkipUnprotected = false;
    ScTableProtection* pProtect = pDoc->GetTabProtection(nTab);
    if (pProtect && pProtect->isProtected())
    {
        bSkipProtected   = !pProtect->isOptionEnabled(ScTableProtection::SELECT_LOCKED_CELLS);
        bSkipUnprotected = !pProtect->isOptionEnabled(ScTableProtection::SELECT_UNLOCKED_CELLS);
    }

    bool bSkipCell = false;
    bool bHFlip = false;
    do
    {
        bSkipCell = pDoc->ColHidden(rCurX, nTab) || pDoc->IsHorOverlapped(rCurX, rCurY, nTab);
        if (bSkipProtected && !bSkipCell)
            bSkipCell = pDoc->HasAttrib(rCurX, rCurY, nTab, rCurX, rCurY, nTab, HASATTR_PROTECTED);
        if (bSkipUnprotected && !bSkipCell)
            bSkipCell = !pDoc->HasAttrib(rCurX, rCurY, nTab, rCurX, rCurY, nTab, HASATTR_PROTECTED);

        if (bSkipCell)
        {
            if (rCurX <= 0 || rCurX >= MAXCOL)
            {
                if (bHFlip)
                {
                    rCurX = nOldX;
                    bSkipCell = false;
                }
                else
                {
                    nMovX = -nMovX;
                    if (nMovX > 0)
                        ++rCurX;
                    else
                        --rCurX;
                    bHFlip = true;
                }
            }
            else
                if (nMovX > 0)
                    ++rCurX;
                else
                    --rCurX;
        }
    }
    while (bSkipCell);

    if (pDoc->IsVerOverlapped(rCurX, rCurY, nTab))
    {
        aViewData.SetOldCursor(rCurX, rCurY);
        while (pDoc->IsVerOverlapped(rCurX, rCurY, nTab))
            --rCurY;
    }
}

void ScTabView::SkipCursorVertical(SCsCOL& rCurX, SCsROW& rCurY, SCsROW nOldY, SCsROW nMovY)
{
    ScDocument* pDoc = aViewData.GetDocument();
    SCTAB nTab = aViewData.GetTabNo();

    bool bSkipProtected = false, bSkipUnprotected = false;
    ScTableProtection* pProtect = pDoc->GetTabProtection(nTab);
    if (pProtect && pProtect->isProtected())
    {
        bSkipProtected   = !pProtect->isOptionEnabled(ScTableProtection::SELECT_LOCKED_CELLS);
        bSkipUnprotected = !pProtect->isOptionEnabled(ScTableProtection::SELECT_UNLOCKED_CELLS);
    }

    bool bSkipCell = false;
    bool bVFlip = false;
    do
    {
        SCROW nLastRow = -1;
        bSkipCell = pDoc->RowHidden(rCurY, nTab, NULL, &nLastRow) || pDoc->IsVerOverlapped( rCurX, rCurY, nTab );
        if (bSkipProtected && !bSkipCell)
            bSkipCell = pDoc->HasAttrib(rCurX, rCurY, nTab, rCurX, rCurY, nTab, HASATTR_PROTECTED);
        if (bSkipUnprotected && !bSkipCell)
            bSkipCell = !pDoc->HasAttrib(rCurX, rCurY, nTab, rCurX, rCurY, nTab, HASATTR_PROTECTED);

        if (bSkipCell)
        {
            if (rCurY <= 0 || rCurY >= MAXROW)
            {
                if (bVFlip)
                {
                    rCurY = nOldY;
                    bSkipCell = false;
                }
                else
                {
                    nMovY = -nMovY;
                    if (nMovY > 0)
                        ++rCurY;
                    else
                        --rCurY;
                    bVFlip = true;
                }
            }
            else
                if (nMovY > 0)
                    ++rCurY;
                else
                    --rCurY;
        }
    }
    while (bSkipCell);

    if (pDoc->IsHorOverlapped(rCurX, rCurY, nTab))
    {
        aViewData.SetOldCursor(rCurX, rCurY);
        while (pDoc->IsHorOverlapped(rCurX, rCurY, nTab))
            --rCurX;
    }
}

void ScTabView::ExpandBlock(SCsCOL nMovX, SCsROW nMovY, ScFollowMode eMode)
{
    if (!nMovX && !nMovY)
        // Nothing to do.  Bail out.
        return;

    ScTabViewShell* pViewShell = aViewData.GetViewShell();
    bool bRefInputMode = pViewShell && pViewShell->IsRefInputMode();
    if (bRefInputMode && !aViewData.IsRefMode())
        // initialize formula reference mode if it hasn't already.
        InitRefMode(aViewData.GetCurX(), aViewData.GetCurY(), aViewData.GetTabNo(), SC_REFTYPE_REF);

    ScDocument* pDoc = aViewData.GetDocument();

    if (aViewData.IsRefMode())
    {
        // formula reference mode

        SCCOL nNewX = aViewData.GetRefEndX();
        SCROW nNewY = aViewData.GetRefEndY();
        SCTAB nRefTab = aViewData.GetRefEndZ();

        bool bSelectLocked = true;
        bool bSelectUnlocked = true;
        ScTableProtection* pTabProtection = pDoc->GetTabProtection(nRefTab);
        if (pTabProtection && pTabProtection->isProtected())
        {
            bSelectLocked   = pTabProtection->isOptionEnabled(ScTableProtection::SELECT_LOCKED_CELLS);
            bSelectUnlocked = pTabProtection->isOptionEnabled(ScTableProtection::SELECT_UNLOCKED_CELLS);
        }

        moveCursorByProtRule(nNewX, nNewY, nMovX, nMovY, nRefTab, pDoc);
        checkBoundary(nNewX, nNewY);

        if (nMovX)
        {
            SCCOL nTempX = nNewX;
            while (pDoc->IsHorOverlapped(nTempX, nNewY, nRefTab))
            {
                if (nMovX > 0)
                    ++nTempX;
                else
                    --nTempX;
                if (!checkBoundary(nTempX, nNewY))
                    break;
            }
            if (isCellQualified(pDoc, nTempX, nNewY, nRefTab, bSelectLocked, bSelectUnlocked))
                nNewX = nTempX;
        }

        if (nMovY)
        {
            SCROW nTempY = nNewY;
            while (pDoc->IsVerOverlapped(nNewX, nTempY, nRefTab))
            {
                if (nMovY > 0)
                    ++nTempY;
                else
                    --nTempY;
                if (!checkBoundary(nNewX, nTempY))
                    break;
            }
            if (isCellQualified(pDoc, nNewX, nTempY, nRefTab, bSelectLocked, bSelectUnlocked))
                nNewY = nTempY;
        }

        pDoc->SkipOverlapped(nNewX, nNewY, nRefTab);
        UpdateRef(nNewX, nNewY, nRefTab);
        AlignToCursor(nNewX, nNewY, eMode);
    }
    else
    {
        // normal selection mode

        SCTAB nTab = aViewData.GetTabNo();
        SCCOL nOrigX = aViewData.GetCurX();
        SCROW nOrigY = aViewData.GetCurY();

        // Note that the origin position *never* moves during selection.

        if (!IsBlockMode())
            InitBlockMode(nOrigX, nOrigY, nTab, true);

        moveCursorByProtRule(nBlockEndX, nBlockEndY, nMovX, nMovY, nTab, pDoc);
        checkBoundary(nBlockEndX, nBlockEndY);
        moveCursorByMergedCell(nBlockEndX, nBlockEndY, nMovX, nMovY, nTab, pDoc, aViewData);
        checkBoundary(nBlockEndX, nBlockEndY);

        MarkCursor(nBlockEndX, nBlockEndY, nTab, false, false, true);

        // Check if the entire row(s) or column(s) are selected.
        ScSplitPos eActive = aViewData.GetActivePart();
        bool bRowSelected = (nBlockStartX == 0 && nBlockEndX == MAXCOL);
        bool bColSelected = (nBlockStartY == 0 && nBlockEndY == MAXROW);
        SCsCOL nAlignX = bRowSelected ? aViewData.GetPosX(WhichH(eActive)) : nBlockEndX;
        SCsROW nAlignY = bColSelected ? aViewData.GetPosY(WhichV(eActive)) : nBlockEndY;
        AlignToCursor(nAlignX, nAlignY, eMode);

        SelectionChanged();
    }
}

void ScTabView::ExpandBlockPage(SCsCOL nMovX, SCsROW nMovY)
{
    SCsCOL nPageX;
    SCsROW nPageY;
    GetPageMoveEndPosition(nMovX, nMovY, nPageX, nPageY);
    ExpandBlock(nPageX, nPageY, SC_FOLLOW_FIX);
}

void ScTabView::ExpandBlockArea(SCsCOL nMovX, SCsROW nMovY)
{
    SCsCOL nAreaX;
    SCsROW nAreaY;
    ScFollowMode eMode;
    GetAreaMoveEndPosition(nMovX, nMovY, SC_FOLLOW_JUMP, nAreaX, nAreaY, eMode);
    ExpandBlock(nAreaX, nAreaY, eMode);
}

void ScTabView::UpdateCopySourceOverlay()
{
    for (sal_uInt8 i = 0; i < 4; ++i)
        if (pGridWin[i] && pGridWin[i]->IsVisible())
            pGridWin[i]->UpdateCopySourceOverlay();
}

void ScTabView::UpdateSelectionOverlay()
{
    for (sal_uInt16 i=0; i<4; i++)
        if ( pGridWin[i] && pGridWin[i]->IsVisible() )
            pGridWin[i]->UpdateSelectionOverlay();
}

void ScTabView::UpdateShrinkOverlay()
{
    for (sal_uInt16 i=0; i<4; i++)
        if ( pGridWin[i] && pGridWin[i]->IsVisible() )
            pGridWin[i]->UpdateShrinkOverlay();
}

void ScTabView::UpdateAllOverlays()
{
    for (sal_uInt16 i=0; i<4; i++)
        if ( pGridWin[i] && pGridWin[i]->IsVisible() )
            pGridWin[i]->UpdateAllOverlays();
}

//!
//! PaintBlock in zwei Methoden aufteilen: RepaintBlock und RemoveBlock o.ae.
//!

void ScTabView::PaintBlock( bool bReset )
{
    ScMarkData& rMark = aViewData.GetMarkData();
    SCTAB nTab = aViewData.GetTabNo();
    bool bMulti = rMark.IsMultiMarked();
    if (rMark.IsMarked() || bMulti)
    {
        ScRange aMarkRange;
        HideAllCursors();
        if (bMulti)
        {
            bool bFlag = rMark.GetMarkingFlag();
            rMark.SetMarking(false);
            rMark.MarkToMulti();
            rMark.GetMultiMarkArea(aMarkRange);
            rMark.MarkToSimple();
            rMark.SetMarking(bFlag);
        }
        else
            rMark.GetMarkArea(aMarkRange);

        nBlockStartX = aMarkRange.aStart.Col();
        nBlockStartY = aMarkRange.aStart.Row();
        nBlockStartZ = aMarkRange.aStart.Tab();
        nBlockEndX = aMarkRange.aEnd.Col();
        nBlockEndY = aMarkRange.aEnd.Row();
        nBlockEndZ = aMarkRange.aEnd.Tab();

        bool bDidReset = false;

        if ( nTab>=nBlockStartZ && nTab<=nBlockEndZ )
        {
            if ( bReset )
            {
                // Invertieren beim Loeschen nur auf aktiver View
                if ( aViewData.IsActive() )
                {
                    rMark.ResetMark();
                    UpdateSelectionOverlay();
                    bDidReset = true;
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

void ScTabView::SelectAll( bool bContinue )
{
    ScMarkData& rMark = aViewData.GetMarkData();
    SCTAB nTab = aViewData.GetTabNo();

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
    SCTAB nCount = pDoc->GetTableCount();

    if (nCount>1)
    {
        for (SCTAB i=0; i<nCount; i++)
            rMark.SelectTable( i, true );

        aViewData.GetDocShell()->PostPaintExtras();
        SfxBindings& rBind = aViewData.GetBindings();
        rBind.Invalidate( FID_FILL_TAB );
        rBind.Invalidate( FID_TAB_DESELECTALL );
    }
}

void ScTabView::DeselectAllTables()
{
    ScDocument* pDoc = aViewData.GetDocument();
    ScMarkData& rMark = aViewData.GetMarkData();
    SCTAB nTab = aViewData.GetTabNo();
    SCTAB nCount = pDoc->GetTableCount();

    for (SCTAB i=0; i<nCount; i++)
        rMark.SelectTable( i, ( i == nTab ) );

    aViewData.GetDocShell()->PostPaintExtras();
    SfxBindings& rBind = aViewData.GetBindings();
    rBind.Invalidate( FID_FILL_TAB );
    rBind.Invalidate( FID_TAB_DESELECTALL );
}

static bool lcl_FitsInWindow( double fScaleX, double fScaleY, sal_uInt16 nZoom,
                        long nWindowX, long nWindowY, ScDocument* pDoc, SCTAB nTab,
                        SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                        SCCOL nFixPosX, SCROW nFixPosY )
{
    double fZoomFactor = (double)Fraction(nZoom,100);
    fScaleX *= fZoomFactor;
    fScaleY *= fZoomFactor;

    long nBlockX = 0;
    SCCOL nCol;
    for (nCol=0; nCol<nFixPosX; nCol++)
    {
        //  for frozen panes, add both parts
        sal_uInt16 nColTwips = pDoc->GetColWidth( nCol, nTab );
        if (nColTwips)
        {
            nBlockX += (long)(nColTwips * fScaleX);
            if (nBlockX > nWindowX)
                return false;
        }
    }
    for (nCol=nStartCol; nCol<=nEndCol; nCol++)
    {
        sal_uInt16 nColTwips = pDoc->GetColWidth( nCol, nTab );
        if (nColTwips)
        {
            nBlockX += (long)(nColTwips * fScaleX);
            if (nBlockX > nWindowX)
                return false;
        }
    }

    long nBlockY = 0;
    for (SCROW nRow = 0; nRow <= nFixPosY-1; ++nRow)
    {
        if (pDoc->RowHidden(nRow, nTab))
            continue;

        //  for frozen panes, add both parts
        sal_uInt16 nRowTwips = pDoc->GetRowHeight(nRow, nTab);
        if (nRowTwips)
        {
            nBlockY += (long)(nRowTwips * fScaleY);
            if (nBlockY > nWindowY)
                return false;
        }
    }
    for (SCROW nRow = nStartRow; nRow <= nEndRow; ++nRow)
    {
        sal_uInt16 nRowTwips = pDoc->GetRowHeight(nRow, nTab);
        if (nRowTwips)
        {
            nBlockY += (long)(nRowTwips * fScaleY);
            if (nBlockY > nWindowY)
                return false;
        }
    }

    return true;
}

sal_uInt16 ScTabView::CalcZoom( SvxZoomType eType, sal_uInt16 nOldZoom )
{
    sal_uInt16 nZoom = 0; // Ergebnis

    switch ( eType )
    {
        case SVX_ZOOM_PERCENT: // rZoom ist kein besonderer prozentualer Wert
            nZoom = nOldZoom;
            break;

        case SVX_ZOOM_OPTIMAL:  // nZoom entspricht der optimalen Gr"o\se
            {
                ScMarkData& rMark = aViewData.GetMarkData();
                ScDocument* pDoc = aViewData.GetDocument();

                if (!rMark.IsMarked() && !rMark.IsMultiMarked())
                    nZoom = 100;                // nothing selected
                else
                {
                    SCTAB   nTab = aViewData.GetTabNo();
                    ScRange aMarkRange;
                    if ( aViewData.GetSimpleArea( aMarkRange ) != SC_MARK_SIMPLE )
                        rMark.GetMultiMarkArea( aMarkRange );

                    SCCOL   nStartCol = aMarkRange.aStart.Col();
                    SCROW   nStartRow = aMarkRange.aStart.Row();
                    SCTAB   nStartTab = aMarkRange.aStart.Tab();
                    SCCOL   nEndCol = aMarkRange.aEnd.Col();
                    SCROW   nEndRow = aMarkRange.aEnd.Row();
                    SCTAB   nEndTab = aMarkRange.aEnd.Tab();

                    if ( nTab < nStartTab && nTab > nEndTab )
                        nTab = nStartTab;

                    ScSplitPos eUsedPart = aViewData.GetActivePart();

                    SCCOL nFixPosX = 0;
                    SCROW nFixPosY = 0;
                    if ( aViewData.GetHSplitMode() == SC_SPLIT_FIX )
                    {
                        //  use right part
                        eUsedPart = (WhichV(eUsedPart)==SC_SPLIT_TOP) ? SC_SPLIT_TOPRIGHT : SC_SPLIT_BOTTOMRIGHT;
                        nFixPosX = aViewData.GetFixPosX();
                        if ( nStartCol < nFixPosX )
                            nStartCol = nFixPosX;
                    }
                    if ( aViewData.GetVSplitMode() == SC_SPLIT_FIX )
                    {
                        //  use bottom part
                        eUsedPart = (WhichH(eUsedPart)==SC_SPLIT_LEFT) ? SC_SPLIT_BOTTOMLEFT : SC_SPLIT_BOTTOMRIGHT;
                        nFixPosY = aViewData.GetFixPosY();
                        if ( nStartRow < nFixPosY )
                            nStartRow = nFixPosY;
                    }

                    if (pGridWin[eUsedPart])
                    {
                        //  Because scale is rounded to pixels, the only reliable way to find
                        //  the right scale is to check if a zoom fits

                        Size aWinSize = pGridWin[eUsedPart]->GetOutputSizePixel();

                        //  for frozen panes, use sum of both parts for calculation

                        if ( nFixPosX != 0 )
                            aWinSize.Width() += GetGridWidth( SC_SPLIT_LEFT );
                        if ( nFixPosY != 0 )
                            aWinSize.Height() += GetGridHeight( SC_SPLIT_TOP );

                        ScDocShell* pDocSh = aViewData.GetDocShell();
                        double nPPTX = ScGlobal::nScreenPPTX / pDocSh->GetOutputFactor();
                        double nPPTY = ScGlobal::nScreenPPTY;

                        sal_uInt16 nMin = MINZOOM;
                        sal_uInt16 nMax = MAXZOOM;
                        while ( nMax > nMin )
                        {
                            sal_uInt16 nTest = (nMin+nMax+1)/2;
                            if ( lcl_FitsInWindow(
                                        nPPTX, nPPTY, nTest, aWinSize.Width(), aWinSize.Height(),
                                        pDoc, nTab, nStartCol, nStartRow, nEndCol, nEndRow,
                                        nFixPosX, nFixPosY ) )
                                nMin = nTest;
                            else
                                nMax = nTest-1;
                        }
                        OSL_ENSURE( nMin == nMax, "Schachtelung ist falsch" );
                        nZoom = nMin;

                        if ( nZoom != nOldZoom )
                        {
                            // scroll to block only in active split part
                            // (the part for which the size was calculated)

                            if ( nStartCol <= nEndCol )
                                aViewData.SetPosX( WhichH(eUsedPart), nStartCol );
                            if ( nStartRow <= nEndRow )
                                aViewData.SetPosY( WhichV(eUsedPart), nStartRow );
                        }
                    }
                }
            }
            break;

            case SVX_ZOOM_WHOLEPAGE:    // nZoom entspricht der ganzen Seite oder
            case SVX_ZOOM_PAGEWIDTH:    // nZoom entspricht der Seitenbreite
                {
                    SCTAB               nCurTab     = aViewData.GetTabNo();
                    ScDocument*         pDoc        = aViewData.GetDocument();
                    ScStyleSheetPool*   pStylePool  = pDoc->GetStyleSheetPool();
                    SfxStyleSheetBase*  pStyleSheet =
                                            pStylePool->Find( pDoc->GetPageStyle( nCurTab ),
                                                              SFX_STYLE_FAMILY_PAGE );

                    OSL_ENSURE( pStyleSheet, "PageStyle not found :-/" );

                    if ( pStyleSheet )
                    {
                        ScPrintFunc aPrintFunc( aViewData.GetDocShell(),
                                                aViewData.GetViewShell()->GetPrinter(true),
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
                                for ( SCCOL nCol = aViewData.GetPosX(SC_SPLIT_LEFT);
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
                                aPageSize.Height() += pDoc->GetRowHeight(
                                        aViewData.GetPosY(SC_SPLIT_TOP),
                                        aViewData.GetFixPosY()-1, nCurTab);
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

                        nZoom = (sal_uInt16) nNew;
                    }
                }
                break;

        default:
            OSL_FAIL("Unknown Zoom-Revision");
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
    for (sal_uInt16 i=0; i<4; i++)
        if (pGridWin[i] && pGridWin[i]->IsVisible())
            pGridWin[i]->HideNoteMarker();
}

void ScTabView::MakeDrawLayer()
{
    if (!pDrawView)
    {
        aViewData.GetDocShell()->MakeDrawLayer();

        //  pDrawView wird per Notify gesetzt
        OSL_ENSURE(pDrawView,"ScTabView::MakeDrawLayer funktioniert nicht");

        // #114409#
        for(sal_uInt16 a(0); a < 4; a++)
        {
            if(pGridWin[a])
            {
                pGridWin[a]->DrawLayerCreated();
            }
        }
    }
}

void ScTabView::ErrorMessage( sal_uInt16 nGlobStrId )
{
    if ( SC_MOD()->IsInExecuteDrop() )
    {
        // #i28468# don't show error message when called from Drag&Drop, silently abort instead
        return;
    }

    StopMarking();      // falls per Focus aus MouseButtonDown aufgerufen

    Window* pParent = aViewData.GetDialogParent();
    ScWaitCursorOff aWaitOff( pParent );
    bool bFocus = pParent && pParent->HasFocus();

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

Window* ScTabView::GetParentOrChild( sal_uInt16 nChildId )
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

void ScTabView::UpdatePageBreakData( bool bForcePaint )
{
    ScPageBreakData* pNewData = NULL;

    if (aViewData.IsPagebreakMode())
    {
        ScDocShell* pDocSh = aViewData.GetDocShell();
        ScDocument* pDoc = pDocSh->GetDocument();
        SCTAB nTab = aViewData.GetTabNo();

        sal_uInt16 nCount = pDoc->GetPrintRangeCount(nTab);
        if (!nCount)
            nCount = 1;
        pNewData = new ScPageBreakData(nCount);

        ScPrintFunc aPrintFunc( pDocSh, pDocSh->GetPrinter(), nTab, 0,0,NULL, NULL, pNewData );
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



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
