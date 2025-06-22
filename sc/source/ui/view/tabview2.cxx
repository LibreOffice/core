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

#include <scitems.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <sfx2/bindings.hxx>
#include <osl/diagnose.h>

#include <attrib.hxx>
#include <pagedata.hxx>
#include <tabview.hxx>
#include <tabvwsh.hxx>
#include <printfun.hxx>
#include <stlpool.hxx>
#include <docsh.hxx>
#include <gridwin.hxx>
#include <sc.hrc>
#include <viewutil.hxx>
#include <colrowba.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <scmod.hxx>
#include <table.hxx>
#include <tabprotection.hxx>
#include <markdata.hxx>
#include <inputopt.hxx>
#include <comphelper/lok.hxx>

namespace {

bool isCellQualified(const ScDocument* pDoc, SCCOL nCol, SCROW nRow, SCTAB nTab, bool bSelectLocked, bool bSelectUnlocked)
{
    bool bCellProtected = pDoc->HasAttrib(
        nCol, nRow, nTab, nCol, nRow, nTab, HasAttrFlags::Protected);

    if (bCellProtected && !bSelectLocked)
        return false;

    if (!bCellProtected && !bSelectUnlocked)
        return false;

    return true;
}

bool areCellsQualified(const ScDocument* pDoc, SCCOL nColStart, SCROW nRowStart, SCCOL nColEnd,
                       SCROW nRowEnd, SCTAB nTab, bool bSelectLocked, bool bSelectUnlocked)
{
    PutInOrder(nColStart, nColEnd);
    PutInOrder(nRowStart, nRowEnd);
    for (SCCOL col = nColStart; col <= nColEnd; ++col)
        for (SCROW row = nRowStart; row <= nRowEnd; ++row)
            if (!isCellQualified(pDoc, col, row, nTab, bSelectLocked, bSelectUnlocked))
                return false;

    return true;
}

void moveCursorByProtRule(
    SCCOL& rCol, SCROW& rRow, SCCOL nMovX, SCROW nMovY, SCTAB nTab, const ScDocument* pDoc)
{
    bool bSelectLocked = true;
    bool bSelectUnlocked = true;
    const ScTableProtection* pTabProtection = pDoc->GetTabProtection(nTab);
    if (pTabProtection && pTabProtection->isProtected())
    {
        bSelectLocked   = pTabProtection->isOptionEnabled(ScTableProtection::SELECT_LOCKED_CELLS);
        bSelectUnlocked = pTabProtection->isOptionEnabled(ScTableProtection::SELECT_UNLOCKED_CELLS);
    }

    if (nMovX > 0)
    {
        for (SCCOL i = 0; i < nMovX && rCol < pDoc->MaxCol(); ++i)
        {
            SCCOL nNewUnhiddenCol = rCol + 1;
            SCCOL nEndCol = 0;
            while(pDoc->ColHidden(nNewUnhiddenCol, nTab, nullptr, &nEndCol))
            {
                if(nNewUnhiddenCol >= pDoc->MaxCol())
                    return;

                i += nEndCol - nNewUnhiddenCol + 1;
                nNewUnhiddenCol = nEndCol +1;
            }

            if (!isCellQualified(pDoc, nNewUnhiddenCol, rRow, nTab, bSelectLocked, bSelectUnlocked))
                break;
            rCol = nNewUnhiddenCol;
        }
    }
    else if (nMovX < 0)
    {
        for (SCCOL i = 0; i > nMovX && rCol > 0; --i)
        {
            SCCOL nNewUnhiddenCol = rCol - 1;
            SCCOL nStartCol = 0;
            while(pDoc->ColHidden(nNewUnhiddenCol, nTab, &nStartCol))
            {
                if(nNewUnhiddenCol <= 0)
                    return;

                i -= nNewUnhiddenCol - nStartCol + 1;
                nNewUnhiddenCol = nStartCol - 1;
            }

            if (!isCellQualified(pDoc, nNewUnhiddenCol, rRow, nTab, bSelectLocked, bSelectUnlocked))
                break;
            rCol = nNewUnhiddenCol;
        }
    }

    if (nMovY > 0)
    {
        for (SCROW i = 0; i < nMovY && rRow < pDoc->MaxRow(); ++i)
        {
            SCROW nNewUnhiddenRow = rRow + 1;
            SCROW nEndRow = 0;
            while(pDoc->RowHidden(nNewUnhiddenRow, nTab, nullptr, &nEndRow))
            {
                if(nNewUnhiddenRow >= pDoc->MaxRow())
                    return;

                i += nEndRow - nNewUnhiddenRow + 1;
                nNewUnhiddenRow = nEndRow + 1;
            }

            if (!isCellQualified(pDoc, rCol, nNewUnhiddenRow, nTab, bSelectLocked, bSelectUnlocked))
                break;
            rRow = nNewUnhiddenRow;
        }
    }
    else if (nMovY < 0)
    {
        for (SCROW i = 0; i > nMovY && rRow > 0; --i)
        {
            SCROW nNewUnhiddenRow = rRow - 1;
            SCROW nStartRow = 0;
            while(pDoc->RowHidden(nNewUnhiddenRow, nTab, &nStartRow))
            {
                if(nNewUnhiddenRow <= 0)
                    return;

                i -= nNewUnhiddenRow - nStartRow + 1;
                nNewUnhiddenRow = nStartRow - 1;
            }

            if (!isCellQualified(pDoc, rCol, nNewUnhiddenRow, nTab, bSelectLocked, bSelectUnlocked))
                break;
            rRow = nNewUnhiddenRow;
        }
    }
}

bool checkBoundary(const ScDocument* pDoc, SCCOL& rCol, SCROW& rRow)
{
    bool bGood = true;
    if (rCol < 0)
    {
        rCol = 0;
        bGood = false;
    }
    else if (rCol > pDoc->MaxCol())
    {
        rCol = pDoc->MaxCol();
        bGood = false;
    }

    if (rRow < 0)
    {
        rRow = 0;
        bGood = false;
    }
    else if (rRow > pDoc->MaxRow())
    {
        rRow = pDoc->MaxRow();
        bGood = false;
    }
    return bGood;
}

void moveRefByCell(SCCOL& rNewX, SCROW& rNewY,
                   SCCOL nMovX, SCROW nMovY, SCTAB nRefTab,
                   const ScDocument& rDoc)
{
    SCCOL nOldX = rNewX;
    SCROW nOldY = rNewY;
    bool bSelectLocked = true;
    bool bSelectUnlocked = true;
    const ScTableProtection* pTabProtection = rDoc.GetTabProtection(nRefTab);
    if (pTabProtection && pTabProtection->isProtected())
    {
        bSelectLocked   = pTabProtection->isOptionEnabled(ScTableProtection::SELECT_LOCKED_CELLS);
        bSelectUnlocked = pTabProtection->isOptionEnabled(ScTableProtection::SELECT_UNLOCKED_CELLS);
    }

    moveCursorByProtRule(rNewX, rNewY, nMovX, nMovY, nRefTab, &rDoc);
    checkBoundary(&rDoc, rNewX, rNewY);

    if (nMovX)
    {
        SCCOL nTempX = rNewX;
        while (rDoc.IsHorOverlapped(nTempX, rNewY, nRefTab))
        {
            nTempX = (nMovX > 0) ? nTempX + 1 : nTempX - 1;
            if (!checkBoundary(&rDoc, nTempX, rNewY))
                break;
        }
        if (isCellQualified(&rDoc, nTempX, rNewY, nRefTab, bSelectLocked, bSelectUnlocked))
            rNewX = nTempX;

        if (nMovX < 0 && rNewX > 0)
        {
            const ScMergeAttr* pMergeAttr = rDoc.GetAttr(rNewX, rNewY, nRefTab, ATTR_MERGE);
            if (pMergeAttr && pMergeAttr->IsMerged() &&
                nOldX >= rNewX &&
                nOldX <= rNewX + pMergeAttr->GetRowMerge() - 1)
                rNewX = rNewX - 1;
        }
    }

    if (nMovY)
    {
        SCROW nTempY = rNewY;
        while (rDoc.IsVerOverlapped(rNewX, nTempY, nRefTab))
        {
            nTempY = (nMovY > 0) ? nTempY + 1 : nTempY - 1;
            if (!checkBoundary(&rDoc, rNewX, nTempY))
                break;
        }
        if (isCellQualified(&rDoc, rNewX, nTempY, nRefTab, bSelectLocked, bSelectUnlocked))
            rNewY = nTempY;

        if (nMovY < 0 && rNewY > 0)
        {
            const ScMergeAttr* pMergeAttr = rDoc.GetAttr(rNewX, rNewY, nRefTab, ATTR_MERGE);
            if (pMergeAttr && pMergeAttr->IsMerged() &&
                nOldY >= rNewY &&
                nOldY <= rNewY + pMergeAttr->GetRowMerge() - 1)
                rNewY = rNewY - 1;
        }
    }

    rDoc.SkipOverlapped(rNewX, rNewY, nRefTab);
}

void moveCursorByMergedCell(SCCOL& rCol, SCROW& rRow, SCCOL nMovX, SCROW nMovY, SCCOL nStartX,
                            SCROW nStartY, SCTAB nTab, const ScDocument* pDoc)
{
    const ScTableProtection* pTabProtection = pDoc->GetTabProtection(nTab);
    bool bSelectLocked = true;
    bool bSelectUnlocked = true;
    if (pTabProtection && pTabProtection->isProtected())
    {
        bSelectLocked   = pTabProtection->isOptionEnabled(ScTableProtection::SELECT_LOCKED_CELLS);
        bSelectUnlocked = pTabProtection->isOptionEnabled(ScTableProtection::SELECT_UNLOCKED_CELLS);
    }

    if (nMovX > 0)
    {
        SCROW rowStart = std::min(rRow, nStartY);
        SCROW rowEnd = std::max(rRow, nStartY);

        for (SCROW i = rowStart; i <= rowEnd && rCol < nStartX;)
        {
            SCCOL tmpCol = rCol;
            while (tmpCol < pDoc->MaxCol() && pDoc->IsHorOverlapped(tmpCol, i, nTab))
                ++tmpCol;
            if (tmpCol != rCol)
            {
                i = rowStart;
                if (tmpCol > nStartX)
                    --tmpCol;
                if (!areCellsQualified(pDoc, rCol + 1, rowStart, tmpCol, rowEnd, nTab,
                                       bSelectLocked, bSelectUnlocked))
                    break;
                rCol = tmpCol;
            }
            else
                ++i;
        }
    }
    else if (nMovX < 0)
    {
        SCROW rowStart = std::min(rRow, nStartY);
        SCROW rowEnd = std::max(rRow, nStartY);

        for (SCROW i = rowStart; i <= rowEnd && rCol > nStartX;)
        {
            SCCOL tmpCol = rCol;
            while (tmpCol >= 0 && pDoc->IsHorOverlapped(tmpCol + 1, i, nTab))
                --tmpCol;
            if (tmpCol != rCol)
            {
                i = rowStart;
                if (tmpCol < nStartX)
                    ++tmpCol;
                if (!areCellsQualified(pDoc, rCol - 1, rowStart, tmpCol, rowEnd, nTab,
                                       bSelectLocked, bSelectUnlocked))
                    break;
                rCol = tmpCol;
            }
            else
                ++i;
        }
    }

    if (nMovY > 0)
    {
        SCCOL colStart = std::min(rCol, nStartX);
        SCCOL colEnd = std::max(rCol, nStartX);

        for (SCCOL i = colStart; i <= colEnd && rRow < nStartY;)
        {
            SCROW tmpRow = rRow;
            while (tmpRow < pDoc->MaxRow() && pDoc->IsVerOverlapped(i, tmpRow, nTab))
                ++tmpRow;
            if (tmpRow != rRow)
            {
                i = colStart;
                if (tmpRow > nStartY)
                    --tmpRow;
                if (!areCellsQualified(pDoc, colStart, rRow + 1, colEnd, tmpRow, nTab,
                                       bSelectLocked, bSelectUnlocked))
                    break;
                rRow = tmpRow;
            }
            else
                ++i;
        }
    }
    else if (nMovY < 0)
    {
        SCCOL colStart = std::min(rCol, nStartX);
        SCCOL colEnd = std::max(rCol, nStartX);

        for (SCCOL i = colStart; i <= colEnd && rRow > nStartY;)
        {
            SCROW tmpRow = rRow;
            while (tmpRow >= 0 && pDoc->IsVerOverlapped(i, tmpRow + 1, nTab))
                --tmpRow;
            if (tmpRow != rRow)
            {
                i = colStart;
                if (tmpRow < nStartY)
                    ++tmpRow;
                if (!areCellsQualified(pDoc, colStart, rRow - 1, colEnd, tmpRow, nTab,
                                       bSelectLocked, bSelectUnlocked))
                    break;
                rRow = tmpRow;
            }
            else
                ++i;
        }
    }
}

void moveCursorToProperSide(SCCOL& rCol, SCROW& rRow, SCCOL nMovX, SCROW nMovY, SCCOL nStartX,
                            SCROW nStartY, SCTAB nTab, const ScDocument* pDoc)
{
    SCCOL tmpCol = rCol;
    SCROW tmpRow = rRow;

    if (nMovX > 0 && nStartX < pDoc->MaxCol() && rCol < nStartX)
    {
        SCROW rowStart = std::min(rRow, nStartY);
        SCROW rowEnd = std::max(rRow, nStartY);
        for (SCROW i = rowStart; i <= rowEnd && tmpCol < nStartX;)
        {
            if (pDoc->IsHorOverlapped(tmpCol + 1, i, nTab))
            {
                do
                {
                    ++tmpCol;
                } while (pDoc->IsHorOverlapped(tmpCol + 1, i, nTab));
                i = rowStart;
            }
            else
                ++i;
        }
        if (tmpCol < nStartX)
            tmpCol = rCol;
    }
    else if (nMovX < 0 && nStartX > 0 && rCol > nStartX)
    {
        SCROW rowStart = std::min(rRow, nStartY);
        SCROW rowEnd = std::max(rRow, nStartY);
        for (SCROW i = rowStart; i <= rowEnd && tmpCol > nStartX;)
        {
            if (pDoc->IsHorOverlapped(tmpCol, i, nTab))
            {
                do
                {
                    --tmpCol;
                } while (pDoc->IsHorOverlapped(tmpCol, i, nTab));
                i = rowStart;
            }
            else
                ++i;
        }
        if (tmpCol > nStartX)
            tmpCol = rCol;
    }

    if (nMovY > 0 && nStartY < pDoc->MaxRow() && rRow < nStartY)
    {
        SCCOL colStart = std::min(rCol, nStartX);
        SCCOL colEnd = std::max(rCol, nStartX);
        for (SCCOL i = colStart; i <= colEnd && tmpRow < nStartY;)
        {
            if (pDoc->IsVerOverlapped(i, tmpRow + 1, nTab))
            {
                do
                {
                    ++tmpRow;
                } while (pDoc->IsVerOverlapped(i, tmpRow + 1, nTab));
                i = colStart;
            }
            else
                ++i;
        }
        if (tmpRow < nStartY)
            tmpRow = rRow;
    }
    else if (nMovY < 0 && nStartY > 0 && rRow > nStartY)
    {
        SCCOL colStart = std::min(rCol, nStartX);
        SCCOL colEnd = std::max(rCol, nStartX);
        for (SCCOL i = colStart; i <= colEnd && tmpRow > nStartY;)
        {
            if (pDoc->IsVerOverlapped(i, tmpRow, nTab))
            {
                do
                {
                    --tmpRow;
                } while (pDoc->IsVerOverlapped(i, tmpRow, nTab));
                i = colStart;
            }
            else
                ++i;
        }
        if (tmpRow > nStartY)
            tmpRow = rRow;
    }

    if (tmpCol != rCol)
        rCol = tmpCol;
    if (tmpRow != rRow)
        rRow = tmpRow;
}
}

void ScTabView::PaintMarks(SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow )
{
    auto& rDoc = aViewData.GetDocument();
    if (!rDoc.ValidCol(nStartCol)) nStartCol = rDoc.MaxCol();
    if (!rDoc.ValidRow(nStartRow)) nStartRow = rDoc.MaxRow();
    if (!rDoc.ValidCol(nEndCol)) nEndCol = rDoc.MaxCol();
    if (!rDoc.ValidRow(nEndRow)) nEndRow = rDoc.MaxRow();

    bool bLeft = (nStartCol==0 && nEndCol==rDoc.MaxCol());
    bool bTop = (nStartRow==0 && nEndRow==rDoc.MaxRow());

    if (bLeft)
        PaintLeftArea( nStartRow, nEndRow );
    if (bTop)
        PaintTopArea( nStartCol, nEndCol );

    aViewData.GetDocument().ExtendMerge( nStartCol, nStartRow, nEndCol, nEndRow,
                                            aViewData.GetTabNo() );
    PaintArea( nStartCol, nStartRow, nEndCol, nEndRow, ScUpdateMode::Marks );
}

bool ScTabView::IsMarking( SCCOL nCol, SCROW nRow, SCTAB nTab ) const
{
    return IsBlockMode()
        && nBlockStartX == nCol
        && nBlockStartY == nRow
        && nBlockStartZ == nTab;
}

void ScTabView::InitOwnBlockMode( const ScRange& rMarkRange )
{
    if (IsBlockMode())
        return;

    // when there is no (old) selection anymore, delete anchor in SelectionEngine:
    ScMarkData& rMark = aViewData.GetMarkData();
    if (!rMark.IsMarked() && !rMark.IsMultiMarked())
        GetSelEngine()->CursorPosChanging( false, false );

    meBlockMode = Own;
    nBlockStartX = rMarkRange.aStart.Col();
    nBlockStartY = rMarkRange.aStart.Row();
    nBlockStartZ = rMarkRange.aStart.Tab();
    nBlockEndX = rMarkRange.aEnd.Col();
    nBlockEndY = rMarkRange.aEnd.Row();
    nBlockEndZ = rMarkRange.aEnd.Tab();

    SelectionChanged();     // status is checked with mark set
}

void ScTabView::InitBlockModeHighlight( SCCOL nCurX, SCROW nCurY, SCTAB nCurZ,
                                        bool bCols, bool bRows )
{
    if (meHighlightBlockMode != None)
        return;

    auto& rDoc = aViewData.GetDocument();
    if (!rDoc.ValidCol(nCurX)) nCurX = rDoc.MaxCol();
    if (!rDoc.ValidRow(nCurY)) nCurY = rDoc.MaxRow();

    ScMarkData& rMark = aViewData.GetHighlightData();
    meHighlightBlockMode = Normal;

    SCROW nStartY = nCurY;
    SCCOL nStartX = nCurX;
    SCROW nEndY = nCurY;
    SCCOL nEndX = nCurX;

    if (bCols)
    {
        nStartY = 0;
        nEndY = rDoc.MaxRow();
    }

    if (bRows)
    {
        nStartX = 0;
        nEndX = rDoc.MaxCol();
    }

    rMark.SetMarkArea( ScRange( nStartX, nStartY, nCurZ, nEndX, nEndY, nCurZ ) );
    UpdateHighlightOverlay();
}

void ScTabView::InitBlockMode( SCCOL nCurX, SCROW nCurY, SCTAB nCurZ,
                               bool bTestNeg, bool bCols, bool bRows, bool bForceNeg )
{
    if (IsBlockMode())
        return;

    auto& rDoc = aViewData.GetDocument();
    if (!rDoc.ValidCol(nCurX)) nCurX = rDoc.MaxCol();
    if (!rDoc.ValidRow(nCurY)) nCurY = rDoc.MaxRow();

    ScMarkData& rMark = aViewData.GetMarkData();
    SCTAB nTab = aViewData.GetTabNo();

    //  unmark part?
    if (bForceNeg)
        bBlockNeg = true;
    else if (bTestNeg)
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
        nBlockEndY = rDoc.MaxRow();
    }

    if (bBlockRows)
    {
        nBlockStartX = nBlockStartXOrig = 0;
        nBlockEndX = rDoc.MaxCol();
    }

    rMark.SetMarkArea( ScRange( nBlockStartX,nBlockStartY, nTab, nBlockEndX,nBlockEndY, nTab ) );

    UpdateSelectionOverlay();
}

void ScTabView::DoneBlockModeHighlight( bool bContinue )
{
    if (meHighlightBlockMode == None)
        return;

    ScMarkData& rMark = aViewData.GetHighlightData();
    bool bFlag = rMark.GetMarkingFlag();
    rMark.SetMarking(false);

    if (bContinue)
        rMark.MarkToMulti();
    else
    {
        SCTAB nTab = aViewData.GetTabNo();
        ScDocument& rDoc = aViewData.GetDocument();
        if ( rDoc.HasTable(nTab) )
            rMark.ResetMark();
    }
    meHighlightBlockMode = None;

    rMark.SetMarking(bFlag);
    if (bContinue)
        rMark.SetMarking(false);
}

void ScTabView::DoneBlockMode( bool bContinue )
{
    // When switching between sheet and header SelectionEngine DeselectAll may be called,
    // because the other engine does not have any anchor.
    // bMoveIsShift prevents the selection to be canceled.

    if (!IsBlockMode() || bMoveIsShift)
        return;

    ScMarkData& rMark = aViewData.GetMarkData();
    bool bFlag = rMark.GetMarkingFlag();
    rMark.SetMarking(false);

    if (bBlockNeg && !bContinue)
        rMark.MarkToMulti();

    if (bContinue)
        rMark.MarkToMulti();
    else
    {
        // the sheet may be invalid at this point because DoneBlockMode from SetTabNo is
        // called (for example, when the current sheet is closed from another View)
        SCTAB nTab = aViewData.GetTabNo();
        ScDocument& rDoc = aViewData.GetDocument();
        if ( rDoc.HasTable(nTab) )
            PaintBlock( true );                             // true -> delete block
        else
            rMark.ResetMark();
    }
    meBlockMode = None;

    rMark.SetMarking(bFlag);
    rMark.SetMarkNegative(false);
}

bool ScTabView::IsBlockMode() const
{
    return meBlockMode != None;
}

void ScTabView::MarkCursor( SCCOL nCurX, SCROW nCurY, SCTAB nCurZ,
                            bool bCols, bool bRows, bool bCellSelection )
{
    ScDocument& rDocument = aViewData.GetDocument();
    if (!rDocument.ValidCol(nCurX)) nCurX = rDocument.MaxCol();
    if (!rDocument.ValidRow(nCurY)) nCurY = rDocument.MaxRow();

    if (!IsBlockMode())
    {
        OSL_FAIL( "MarkCursor not in BlockMode" );
        InitBlockMode( nCurX, nCurY, nCurZ, false, bCols, bRows );
    }

    if (bCols)
        nCurY = rDocument.MaxRow();
    if (bRows)
        nCurX = rDocument.MaxCol();

    ScMarkData& rMark = aViewData.GetMarkData();
    OSL_ENSURE(rMark.IsMarked() || rMark.IsMultiMarked(), "MarkCursor, !IsMarked()");
    const ScRange& aMarkRange = rMark.GetMarkArea();
    if (( aMarkRange.aStart.Col() != nBlockStartX && aMarkRange.aEnd.Col() != nBlockStartX ) ||
        ( aMarkRange.aStart.Row() != nBlockStartY && aMarkRange.aEnd.Row() != nBlockStartY ) ||
        ( meBlockMode == Own ))
    {
        //  Mark has been changed
        // (Eg MarkToSimple if by negative everything was erased, except for a rectangle)
        // or after InitOwnBlockMode is further marked with shift-
        bool bOldShift = bMoveIsShift;
        bMoveIsShift = false;          //  really move
        DoneBlockMode();               //! Set variables directly? (-> no flicker)
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
            // Expand selection area accordingly when the current selection cuts
            // through a merged cell.
            ScRange cellSel(nBlockStartXOrig, nBlockStartYOrig, nTab, nCurX, nCurY, nTab);
            cellSel.PutInOrder();
            ScRange oldSel;
            do
            {
                oldSel = cellSel;
                rDocument.ExtendOverlapped(cellSel);
                rDocument.ExtendMerge(cellSel);
            } while (oldSel != cellSel);

            // Preserve the directionality of the selection
            if (nCurX >= nBlockStartXOrig)
            {
                nBlockStartX = cellSel.aStart.Col();
                nBlockEndX = cellSel.aEnd.Col();
            }
            else
            {
                nBlockStartX = cellSel.aEnd.Col();
                nBlockEndX = cellSel.aStart.Col();
            }
            if (nCurY >= nBlockStartYOrig)
            {
                nBlockStartY = cellSel.aStart.Row();
                nBlockEndY = cellSel.aEnd.Row();
            }
            else
            {
                nBlockStartY = cellSel.aEnd.Row();
                nBlockEndY = cellSel.aStart.Row();
            }
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

        nOldCurX = nBlockEndX;
        nOldCurY = nBlockEndY;

        aViewData.GetViewShell()->UpdateInputHandler();
    }

    if ( !bCols && !bRows )
        aHdrFunc.SetAnchorFlag( false );
}

void ScTabView::GetPageMoveEndPosition(SCCOL nMovX, SCROW nMovY, SCCOL& rPageX, SCROW& rPageY)
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

    auto nScrSizeY = SC_SIZE_NONE;
    if (comphelper::LibreOfficeKit::isActive() && aViewData.GetPageUpDownOffset() > 0) {
        nScrSizeY = ScViewData::ToPixel( aViewData.GetPageUpDownOffset(), aViewData.GetPPTX() );
    }

    SCCOL nPageX;
    SCROW nPageY;
    if (nMovX >= 0)
        nPageX = aViewData.CellsAtX( nCurX, 1, eWhichX ) * nMovX;
    else
        nPageX = aViewData.CellsAtX( nCurX, -1, eWhichX ) * nMovX;

    if (nMovY >= 0)
        nPageY = aViewData.CellsAtY( nCurY, 1, eWhichY, nScrSizeY ) * nMovY;
    else
        nPageY = aViewData.CellsAtY( nCurY, -1, eWhichY, nScrSizeY ) * nMovY;

    if (nMovX != 0 && nPageX == 0) nPageX = (nMovX>0) ? 1 : -1;
    if (nMovY != 0 && nPageY == 0) nPageY = (nMovY>0) ? 1 : -1;

    rPageX = nPageX;
    rPageY = nPageY;
}

void ScTabView::GetAreaMoveEndPosition(SCCOL nMovX, SCROW nMovY, ScFollowMode eMode,
                                       SCCOL& rAreaX, SCROW& rAreaY, ScFollowMode& rMode,
                                       bool bInteractiveByUser)
{
    SCCOL nNewX = -1;
    SCROW nNewY = -1;

    // current cursor position.
    SCCOL nCurX = aViewData.GetCurX();
    SCROW nCurY = aViewData.GetCurY();

    ScModule* pScModule = ScModule::get();
    bool bLegacyCellSelection = pScModule->GetInputOptions().GetLegacyCellSelection();
    bool bIncrementallyExpandToDocLimits(false);

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
        // cool#6931 on ctrl+[right/down] don't immediately leap to the far limits of the document when no more data,
        // instead jump a generous block of emptiness. Limit to direct interaction by user and the simple
        // case.
        bIncrementallyExpandToDocLimits = bInteractiveByUser && (nMovX == 1 || nMovY == 1) &&
                                          !bLegacyCellSelection && comphelper::LibreOfficeKit::isActive();
    }

    ScDocument& rDoc = aViewData.GetDocument();
    SCTAB nTab = aViewData.GetTabNo();

    // FindAreaPos knows only -1 or 1 as direction
    SCCOL nVirtualX = bLegacyCellSelection ? nNewX : nCurX;
    SCROW nVirtualY = bLegacyCellSelection ? nNewY : nCurY;

    SCCOLROW i;
    if ( nMovX > 0 )
        for ( i=0; i<nMovX; i++ )
            rDoc.FindAreaPos( nNewX, nVirtualY, nTab,  SC_MOVE_RIGHT );
    if ( nMovX < 0 )
        for ( i=0; i<-nMovX; i++ )
            rDoc.FindAreaPos( nNewX, nVirtualY, nTab,  SC_MOVE_LEFT );
    if ( nMovY > 0 )
        for ( i=0; i<nMovY; i++ )
            rDoc.FindAreaPos( nVirtualX, nNewY, nTab,  SC_MOVE_DOWN );
    if ( nMovY < 0 )
        for ( i=0; i<-nMovY; i++ )
            rDoc.FindAreaPos( nVirtualX, nNewY, nTab,  SC_MOVE_UP );

    if (eMode==SC_FOLLOW_JUMP)                  // bottom right do not show too much grey
    {
        if (nMovX != 0 && nNewX == rDoc.MaxCol())
        {
            eMode = SC_FOLLOW_LINE;
            if (bIncrementallyExpandToDocLimits)
            {
                if (const ScTable* pTab = rDoc.FetchTable(nTab))
                {
                    if (!pTab->HasData(nNewX, nCurY))
                    {
                        SCCOL nLastUsedCol(0);
                        SCROW nLastUsedRow(0);
                        rDoc.GetPrintArea(nTab, nLastUsedCol, nLastUsedRow);
                        SCCOL nJumpFrom = std::max(nCurX, nLastUsedCol);
                        nNewX = ((nJumpFrom / 13) + 2) * 13 - 1;
                    }
                }
            }
        }
        if (nMovY != 0 && nNewY == rDoc.MaxRow())
        {
            eMode = SC_FOLLOW_LINE;
            if (bIncrementallyExpandToDocLimits)
            {
                if (const ScTable* pTab = rDoc.FetchTable(nTab))
                {
                    if (!pTab->HasData(nCurX, nNewY))
                    {
                        SCCOL nLastUsedCol(0);
                        SCROW nLastUsedRow(0);
                        rDoc.GetPrintArea(nTab, nLastUsedCol, nLastUsedRow);
                        SCROW nJumpFrom = std::max(nCurY, nLastUsedRow);
                        nNewY = ((nJumpFrom / 500) + 2) * 500 - 1;
                    }
                }
            }
        }
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

void ScTabView::SkipCursorHorizontal(SCCOL& rCurX, SCROW& rCurY, SCCOL nOldX, SCCOL nMovX)
{
    ScDocument& rDoc = aViewData.GetDocument();
    SCTAB nTab = aViewData.GetTabNo();

    bool bSkipProtected = false, bSkipUnprotected = false;
    const ScTableProtection* pProtect = rDoc.GetTabProtection(nTab);
    if (pProtect && pProtect->isProtected())
    {
        bSkipProtected   = !pProtect->isOptionEnabled(ScTableProtection::SELECT_LOCKED_CELLS);
        bSkipUnprotected = !pProtect->isOptionEnabled(ScTableProtection::SELECT_UNLOCKED_CELLS);
    }

    bool bSkipCell = false;
    bool bHFlip = false;
    // If a number of last columns are hidden, search up to and including the first of them,
    // because after it nothing changes.
    SCCOL nMaxCol;
    if(rDoc.ColHidden(rDoc.MaxCol(), nTab, &nMaxCol))
        ++nMaxCol;
    else
        nMaxCol = rDoc.MaxCol();
    // Search also at least up to and including the first unallocated column (all unallocated columns
    // share a set of attrs).
    nMaxCol = std::max( nMaxCol, std::min<SCCOL>( rDoc.GetAllocatedColumnsCount(nTab) + 1, rDoc.MaxCol()));
    do
    {
        bSkipCell = rDoc.ColHidden(rCurX, nTab) || rDoc.IsHorOverlapped(rCurX, rCurY, nTab);
        if (bSkipProtected && !bSkipCell)
            bSkipCell = rDoc.HasAttrib(rCurX, rCurY, nTab, rCurX, rCurY, nTab, HasAttrFlags::Protected);
        if (bSkipUnprotected && !bSkipCell)
            bSkipCell = !rDoc.HasAttrib(rCurX, rCurY, nTab, rCurX, rCurY, nTab, HasAttrFlags::Protected);

        if (bSkipCell)
        {
            if (rCurX <= 0 || rCurX >= nMaxCol)
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

    if (rDoc.IsVerOverlapped(rCurX, rCurY, nTab))
    {
        aViewData.SetOldCursor(rCurX, rCurY);
        while (rDoc.IsVerOverlapped(rCurX, rCurY, nTab))
            --rCurY;
    }
}

void ScTabView::SkipCursorVertical(SCCOL& rCurX, SCROW& rCurY, SCROW nOldY, SCROW nMovY)
{
    ScDocument& rDoc = aViewData.GetDocument();
    SCTAB nTab = aViewData.GetTabNo();

    bool bSkipProtected = false, bSkipUnprotected = false;
    const ScTableProtection* pProtect = rDoc.GetTabProtection(nTab);
    if (pProtect && pProtect->isProtected())
    {
        bSkipProtected   = !pProtect->isOptionEnabled(ScTableProtection::SELECT_LOCKED_CELLS);
        bSkipUnprotected = !pProtect->isOptionEnabled(ScTableProtection::SELECT_UNLOCKED_CELLS);
    }

    bool bSkipCell = false;
    bool bVFlip = false;
    // Avoid repeated calls to RowHidden(), IsVerOverlapped() and HasAttrib().
    SCROW nFirstSameHiddenRow = -1;
    SCROW nLastSameHiddenRow = -1;
    bool bRowHidden = false;
    SCROW nFirstSameIsVerOverlapped = -1;
    SCROW nLastSameIsVerOverlapped = -1;
    bool bIsVerOverlapped = false;
    SCROW nFirstSameHasAttribRow = -1;
    SCROW nLastSameHasAttribRow = -1;
    bool bHasAttribProtected = false;
    do
    {
        if( rCurY < nFirstSameHiddenRow || rCurY > nLastSameHiddenRow )
            bRowHidden = rDoc.RowHidden(rCurY, nTab, &nFirstSameHiddenRow, &nLastSameHiddenRow);
        bSkipCell = bRowHidden;
        if( !bSkipCell )
        {
            if( rCurY < nFirstSameIsVerOverlapped || rCurY > nLastSameIsVerOverlapped )
                bIsVerOverlapped = rDoc.IsVerOverlapped(rCurX, rCurY, nTab, &nFirstSameIsVerOverlapped, &nLastSameIsVerOverlapped);
            bSkipCell = bIsVerOverlapped;
        }
        if (bSkipProtected && !bSkipCell)
        {
            if( rCurY < nFirstSameHasAttribRow || rCurY > nLastSameHasAttribRow )
                bHasAttribProtected = rDoc.HasAttrib(rCurX, rCurY, nTab, HasAttrFlags::Protected,
                                                     &nFirstSameHasAttribRow, &nLastSameHasAttribRow);
            bSkipCell = bHasAttribProtected;
        }
        if (bSkipUnprotected && !bSkipCell)
        {
            if( rCurY < nFirstSameHasAttribRow || rCurY > nLastSameHasAttribRow )
                bHasAttribProtected = rDoc.HasAttrib(rCurX, rCurY, nTab, HasAttrFlags::Protected,
                                                     &nFirstSameHasAttribRow, &nLastSameHasAttribRow);
            bSkipCell = !bHasAttribProtected;
        }

        if (bSkipCell)
        {
            if (rCurY <= 0 || rCurY >= rDoc.MaxRow())
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

    if (rDoc.IsHorOverlapped(rCurX, rCurY, nTab))
    {
        aViewData.SetOldCursor(rCurX, rCurY);
        while (rDoc.IsHorOverlapped(rCurX, rCurY, nTab))
            --rCurX;
    }
}

void ScTabView::ExpandBlock(SCCOL nMovX, SCROW nMovY, ScFollowMode eMode)
{
    if (!nMovX && !nMovY)
        // Nothing to do.  Bail out.
        return;

    ScTabViewShell* pViewShell = aViewData.GetViewShell();
    bool bRefInputMode = pViewShell && pViewShell->IsRefInputMode();
    if (bRefInputMode && !aViewData.IsRefMode())
        // initialize formula reference mode if it hasn't already.
        InitRefMode(aViewData.GetCurX(), aViewData.GetCurY(), aViewData.GetTabNo(), SC_REFTYPE_REF);

    ScDocument& rDoc = aViewData.GetDocument();

    if (aViewData.IsRefMode())
    {
        // formula reference mode

        SCCOL nNewX = aViewData.GetRefEndX();
        SCROW nNewY = aViewData.GetRefEndY();
        SCTAB nRefTab = aViewData.GetRefEndZ();

        moveRefByCell(nNewX, nNewY, nMovX, nMovY, nRefTab, rDoc);

        UpdateRef(nNewX, nNewY, nRefTab);
        SCCOL nTargetCol = nNewX;
        SCROW nTargetRow = nNewY;
        if (((aViewData.GetRefStartX() == 0) || (aViewData.GetRefStartY() == 0)) &&
            ((nNewX != rDoc.MaxCol()) || (nNewY != rDoc.MaxRow())))
        {
            // Row selection
            if ((aViewData.GetRefStartX() == 0) && (nNewX == rDoc.MaxCol()))
                nTargetCol = aViewData.GetCurX();
            // Column selection
            if ((aViewData.GetRefStartY() == 0) && (nNewY == rDoc.MaxRow()))
                nTargetRow = aViewData.GetCurY();
        }
        AlignToCursor(nTargetCol, nTargetRow, eMode);
    }
    else
    {
        // normal selection mode

        SCTAB nTab = aViewData.GetTabNo();
        SCCOL nOrigX = aViewData.GetCurX();
        SCROW nOrigY = aViewData.GetCurY();

        // Note that the origin position *never* moves during selection.

        if (!IsBlockMode())
        {
            InitBlockMode(nOrigX, nOrigY, nTab, true);
            const ScMergeAttr* pMergeAttr = rDoc.GetAttr(nOrigX, nOrigY, nTab, ATTR_MERGE);
            if (pMergeAttr && pMergeAttr->IsMerged())
            {
                nBlockEndX = nOrigX + pMergeAttr->GetColMerge() - 1;
                nBlockEndY = nOrigY + pMergeAttr->GetRowMerge() - 1;
            }
        }

        moveCursorToProperSide(nBlockEndX, nBlockEndY, nMovX, nMovY, nBlockStartX, nBlockStartY,
                               nTab, &rDoc);
        moveCursorByProtRule(nBlockEndX, nBlockEndY, nMovX, nMovY, nTab, &rDoc);
        checkBoundary(&rDoc, nBlockEndX, nBlockEndY);
        moveCursorByMergedCell(nBlockEndX, nBlockEndY, nMovX, nMovY, nBlockStartX, nBlockStartY,
                               nTab, &rDoc);
        checkBoundary(&rDoc, nBlockEndX, nBlockEndY);

        MarkCursor(nBlockEndX, nBlockEndY, nTab, false, false, true);

        // Check if the entire row(s) or column(s) are selected.
        ScSplitPos eActive = aViewData.GetActivePart();
        bool bRowSelected = (nBlockStartX == 0 && nBlockEndX == rDoc.MaxCol());
        bool bColSelected = (nBlockStartY == 0 && nBlockEndY == rDoc.MaxRow());
        SCCOL nAlignX = bRowSelected ? aViewData.GetPosX(WhichH(eActive)) : nBlockEndX;
        SCROW nAlignY = bColSelected ? aViewData.GetPosY(WhichV(eActive)) : nBlockEndY;
        AlignToCursor(nAlignX, nAlignY, eMode);

        SelectionChanged();
    }
}

void ScTabView::ExpandBlockPage(SCCOL nMovX, SCROW nMovY)
{
    SCCOL nPageX;
    SCROW nPageY;
    GetPageMoveEndPosition(nMovX, nMovY, nPageX, nPageY);
    ExpandBlock(nPageX, nPageY, SC_FOLLOW_FIX);
}

void ScTabView::ExpandBlockArea(SCCOL nMovX, SCROW nMovY)
{
    SCCOL nAreaX;
    SCROW nAreaY;
    ScFollowMode eMode;
    GetAreaMoveEndPosition(nMovX, nMovY, SC_FOLLOW_JUMP, nAreaX, nAreaY, eMode);
    ExpandBlock(nAreaX, nAreaY, eMode);
}

void ScTabView::UpdateCopySourceOverlay()
{
    for (VclPtr<ScGridWindow> & pWin : pGridWin)
        if (pWin && pWin->IsVisible())
            pWin->UpdateCopySourceOverlay();
}

void ScTabView::UpdateSelectionOverlay()
{
    for (VclPtr<ScGridWindow> & pWin : pGridWin)
        if ( pWin && pWin->IsVisible() )
            pWin->UpdateSelectionOverlay();
}

void ScTabView::UpdateHighlightOverlay()
{
    for (VclPtr<ScGridWindow> & pWin : pGridWin)
        if ( pWin && pWin->IsVisible() )
            pWin->UpdateHighlightOverlay();
}

void ScTabView::UpdateShrinkOverlay()
{
    for (VclPtr<ScGridWindow> & pWin : pGridWin)
        if ( pWin && pWin->IsVisible() )
            pWin->UpdateShrinkOverlay();
}

void ScTabView::UpdateAllOverlays()
{
    for (VclPtr<ScGridWindow> & pWin : pGridWin)
        if ( pWin && pWin->IsVisible() )
            pWin->UpdateAllOverlays();
}

//!
//! divide PaintBlock into two methods: RepaintBlock and RemoveBlock or similar
//!

void ScTabView::PaintBlock( bool bReset )
{
    ScMarkData& rMark = aViewData.GetMarkData();
    SCTAB nTab = aViewData.GetTabNo();
    bool bMulti = rMark.IsMultiMarked();
    if (!(rMark.IsMarked() || bMulti))
        return;

    ScRange aMarkRange;
    HideAllCursors();
    if (bMulti)
    {
        bool bFlag = rMark.GetMarkingFlag();
        rMark.SetMarking(false);
        rMark.MarkToMulti();
        aMarkRange = rMark.GetMultiMarkArea();
        rMark.MarkToSimple();
        rMark.SetMarking(bFlag);
    }
    else
        aMarkRange = rMark.GetMarkArea();

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
            // Inverting when deleting only on active View
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

void ScTabView::SelectAll( bool bContinue )
{
    ScDocument& rDoc = aViewData.GetDocument();
    ScMarkData& rMark = aViewData.GetMarkData();
    SCTAB nTab = aViewData.GetTabNo();

    if (rMark.IsMarked())
    {
        if ( rMark.GetMarkArea() == ScRange( 0,0,nTab, rDoc.MaxCol(),rDoc.MaxRow(),nTab ) )
            return;
    }

    DoneBlockMode( bContinue );
    InitBlockMode( 0,0,nTab );
    MarkCursor( rDoc.MaxCol(),rDoc.MaxRow(),nTab );

    SelectionChanged();
}

void ScTabView::SelectAllTables()
{
    ScDocument& rDoc = aViewData.GetDocument();
    ScMarkData& rMark = aViewData.GetMarkData();
    SCTAB nCount = rDoc.GetTableCount();

    if (nCount>1)
    {
        for (SCTAB i=0; i<nCount; i++)
            rMark.SelectTable( i, true );

        aViewData.GetDocShell().PostPaintExtras();
        SfxBindings& rBind = aViewData.GetBindings();
        rBind.Invalidate( FID_FILL_TAB );
        rBind.Invalidate( FID_TAB_DESELECTALL );
    }
}

void ScTabView::DeselectAllTables()
{
    ScDocument& rDoc = aViewData.GetDocument();
    ScMarkData& rMark = aViewData.GetMarkData();
    SCTAB nTab = aViewData.GetTabNo();
    SCTAB nCount = rDoc.GetTableCount();

    for (SCTAB i=0; i<nCount; i++)
        rMark.SelectTable( i, ( i == nTab ) );

    aViewData.GetDocShell().PostPaintExtras();
    SfxBindings& rBind = aViewData.GetBindings();
    rBind.Invalidate( FID_FILL_TAB );
    rBind.Invalidate( FID_TAB_DESELECTALL );
}

static bool lcl_FitsInWindow( double fScaleX, double fScaleY, sal_uInt16 nZoom,
                        tools::Long nWindowX, tools::Long nWindowY, const ScDocument* pDoc, SCTAB nTab,
                        SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                        SCCOL nFixPosX, SCROW nFixPosY )
{
    double fZoomFactor = static_cast<double>(Fraction(nZoom,100));
    fScaleX *= fZoomFactor;
    fScaleY *= fZoomFactor;

    tools::Long nBlockX = 0;
    SCCOL nCol;
    for (nCol=0; nCol<nFixPosX; nCol++)
    {
        //  for frozen panes, add both parts
        sal_uInt16 nColTwips = pDoc->GetColWidth( nCol, nTab );
        if (nColTwips)
        {
            nBlockX += static_cast<tools::Long>(nColTwips * fScaleX);
            if (nBlockX > nWindowX)
                return false;
        }
    }
    for (nCol=nStartCol; nCol<=nEndCol; nCol++)
    {
        sal_uInt16 nColTwips = pDoc->GetColWidth( nCol, nTab );
        if (nColTwips)
        {
            nBlockX += static_cast<tools::Long>(nColTwips * fScaleX);
            if (nBlockX > nWindowX)
                return false;
        }
    }

    tools::Long nBlockY = 0;
    for (SCROW nRow = 0; nRow <= nFixPosY-1; ++nRow)
    {
        if (pDoc->RowHidden(nRow, nTab))
            continue;

        //  for frozen panes, add both parts
        sal_uInt16 nRowTwips = pDoc->GetRowHeight(nRow, nTab);
        if (nRowTwips)
        {
            nBlockY += static_cast<tools::Long>(nRowTwips * fScaleY);
            if (nBlockY > nWindowY)
                return false;
        }
    }
    for (SCROW nRow = nStartRow; nRow <= nEndRow; ++nRow)
    {
        sal_uInt16 nRowTwips = pDoc->GetRowHeight(nRow, nTab);
        if (nRowTwips)
        {
            nBlockY += static_cast<tools::Long>(nRowTwips * fScaleY);
            if (nBlockY > nWindowY)
                return false;
        }
    }

    return true;
}

sal_uInt16 ScTabView::CalcZoom( SvxZoomType eType, sal_uInt16 nOldZoom )
{
    sal_uInt16 nZoom = 100;

    switch ( eType )
    {
        case SvxZoomType::PERCENT: // rZoom is no particular percent value
            nZoom = nOldZoom;
            break;

        case SvxZoomType::OPTIMAL:  // nZoom corresponds to the optimal size
            {
                ScMarkData& rMark = aViewData.GetMarkData();
                ScDocument& rDoc = aViewData.GetDocument();

                if (!rMark.IsMarked() && !rMark.IsMultiMarked())
                    nZoom = 100;                // nothing selected
                else
                {
                    SCTAB   nTab = aViewData.GetTabNo();
                    ScRange aMarkRange;
                    if ( aViewData.GetSimpleArea( aMarkRange ) != SC_MARK_SIMPLE )
                        aMarkRange = rMark.GetMultiMarkArea();

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
                            aWinSize.AdjustWidth(GetGridWidth( SC_SPLIT_LEFT ) );
                        if ( nFixPosY != 0 )
                            aWinSize.AdjustHeight(GetGridHeight( SC_SPLIT_TOP ) );

                        ScDocShell& rDocSh = aViewData.GetDocShell();
                        double nPPTX = ScGlobal::nScreenPPTX / rDocSh.GetOutputFactor();
                        double nPPTY = ScGlobal::nScreenPPTY;

                        sal_uInt16 nMin = MINZOOM;
                        sal_uInt16 nMax = MAXZOOM;
                        while ( nMax > nMin )
                        {
                            sal_uInt16 nTest = (nMin+nMax+1)/2;
                            if ( lcl_FitsInWindow(
                                        nPPTX, nPPTY, nTest, aWinSize.Width(), aWinSize.Height(),
                                        &rDoc, nTab, nStartCol, nStartRow, nEndCol, nEndRow,
                                        nFixPosX, nFixPosY ) )
                                nMin = nTest;
                            else
                                nMax = nTest-1;
                        }
                        OSL_ENSURE( nMin == nMax, "Nesting is wrong" );
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

            case SvxZoomType::WHOLEPAGE:    // nZoom corresponds to the whole page or
            case SvxZoomType::PAGEWIDTH:    // nZoom corresponds to the page width
                {
                    SCTAB               nCurTab     = aViewData.GetTabNo();
                    ScDocument&         rDoc        = aViewData.GetDocument();
                    ScStyleSheetPool*   pStylePool  = rDoc.GetStyleSheetPool();
                    SfxStyleSheetBase*  pStyleSheet =
                                            pStylePool->Find( rDoc.GetPageStyle( nCurTab ),
                                                              SfxStyleFamily::Page );

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

                        if (!pGridWin[SC_SPLIT_BOTTOMLEFT])
                            return nZoom;

                        Size aWinSize = pGridWin[SC_SPLIT_BOTTOMLEFT]->GetOutputSizePixel();
                        ScSplitMode eHMode = aViewData.GetHSplitMode();
                        if ( eHMode != SC_SPLIT_NONE && pGridWin[SC_SPLIT_BOTTOMRIGHT] )
                        {
                            tools::Long nOtherWidth = pGridWin[SC_SPLIT_BOTTOMRIGHT]->
                                                        GetOutputSizePixel().Width();
                            if ( eHMode == SC_SPLIT_FIX )
                            {
                                aWinSize.AdjustWidth(nOtherWidth );
                                for ( SCCOL nCol = aViewData.GetPosX(SC_SPLIT_LEFT);
                                        nCol < aViewData.GetFixPosX(); nCol++ )
                                    aPageSize.AdjustWidth(rDoc.GetColWidth( nCol, nCurTab ) );
                            }
                            else if ( nOtherWidth > aWinSize.Width() )
                                aWinSize.setWidth( nOtherWidth );
                        }
                        ScSplitMode eVMode = aViewData.GetVSplitMode();
                        if ( eVMode != SC_SPLIT_NONE && pGridWin[SC_SPLIT_TOPLEFT] )
                        {
                            tools::Long nOtherHeight = pGridWin[SC_SPLIT_TOPLEFT]->
                                                        GetOutputSizePixel().Height();
                            if ( eVMode == SC_SPLIT_FIX )
                            {
                                aWinSize.AdjustHeight(nOtherHeight );
                                aPageSize.AdjustHeight(rDoc.GetRowHeight(
                                        aViewData.GetPosY(SC_SPLIT_TOP),
                                        aViewData.GetFixPosY()-1, nCurTab) );
                            }
                            else if ( nOtherHeight > aWinSize.Height() )
                                aWinSize.setHeight( nOtherHeight );
                        }

                        double nPPTX = ScGlobal::nScreenPPTX / aViewData.GetDocShell().GetOutputFactor();
                        double nPPTY = ScGlobal::nScreenPPTY;

                        tools::Long nZoomX = static_cast<tools::Long>( aWinSize.Width() * 100 /
                                               ( aPageSize.Width() * nPPTX ) );
                        tools::Long nZoomY = static_cast<tools::Long>( aWinSize.Height() * 100 /
                                               ( aPageSize.Height() * nPPTY ) );

                        if (nZoomX > 0)
                            nZoom = static_cast<sal_uInt16>(nZoomX);

                        if (eType == SvxZoomType::WHOLEPAGE && nZoomY > 0 && nZoomY < nZoom)
                            nZoom = static_cast<sal_uInt16>(nZoomY);
                    }
                }
                break;

        default:
            OSL_FAIL("Unknown Zoom-Revision");
    }

    return nZoom;
}

// is called for instance when the view window is shifted:

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

void ScTabView::HideNoteOverlay()
{
    for (VclPtr<ScGridWindow> & pWin : pGridWin)
        if (pWin && pWin->IsVisible())
            pWin->HideNoteOverlay();
}

void ScTabView::MakeDrawLayer()
{
    if (pDrawView)
        return;

    aViewData.GetDocShell().MakeDrawLayer();

    // pDrawView is set per Notify
    OSL_ENSURE(pDrawView,"ScTabView::MakeDrawLayer does not work");

    for(VclPtr<ScGridWindow> & pWin : pGridWin)
    {
        if(pWin)
        {
            pWin->DrawLayerCreated();
        }
    }
}

IMPL_STATIC_LINK_NOARG(ScTabView, InstallLOKNotifierHdl, void*, vcl::ILibreOfficeKitNotifier*)
{
    return GetpApp();
}

void ScTabView::ErrorMessage(TranslateId pGlobStrId)
{
    if (ScModule::get()->IsInExecuteDrop())
    {
        // #i28468# don't show error message when called from Drag&Drop, silently abort instead
        return;
    }

    StopMarking();      // if called by Focus from MouseButtonDown

    weld::Window* pParent = aViewData.GetDialogParent();
    weld::WaitObject aWaitOff( pParent );
    bool bFocus = pParent && pParent->has_focus();

    if (pGlobStrId && pGlobStrId == STR_PROTECTIONERR)
    {
        if (aViewData.GetDocShell().IsReadOnly())
        {
            pGlobStrId = STR_READONLYERR;
        }
    }

    m_xMessageBox.reset(Application::CreateMessageDialog(pParent,
                                                         VclMessageType::Info, VclButtonsType::Ok,
                                                         ScResId(pGlobStrId)));

    if (comphelper::LibreOfficeKit::isActive())
        m_xMessageBox->SetInstallLOKNotifierHdl(LINK(this, ScTabView, InstallLOKNotifierHdl));

    weld::Window* pGrabOnClose = bFocus ? pParent : nullptr;
    m_xMessageBox->runAsync(m_xMessageBox, [this, pGrabOnClose](sal_Int32 /*nResult*/) {
        m_xMessageBox.reset();
        if (pGrabOnClose)
            pGrabOnClose->grab_focus();
    });
}

void ScTabView::UpdatePageBreakData( bool bForcePaint )
{
    std::unique_ptr<ScPageBreakData> pNewData;

    if (aViewData.IsPagebreakMode())
    {
        ScDocShell& rDocSh = aViewData.GetDocShell();
        ScDocument& rDoc = rDocSh.GetDocument();
        SCTAB nTab = aViewData.GetTabNo();

        sal_uInt16 nCount = rDoc.GetPrintRangeCount(nTab);
        if (!nCount)
            nCount = 1;
        pNewData.reset( new ScPageBreakData(nCount) );

        ScPrintFunc aPrintFunc( rDocSh, rDocSh.GetPrinter(), nTab, 0,0,nullptr, nullptr, pNewData.get() );
        // ScPrintFunc fills the PageBreakData in ctor
        if ( nCount > 1 )
        {
            aPrintFunc.ResetBreaks(nTab);
            pNewData->AddPages();
        }

        // print area changed?
        if ( bForcePaint || ( pPageBreakData && !( *pPageBreakData == *pNewData ) ) )
            PaintGrid();
    }

    pPageBreakData = std::move(pNewData);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
