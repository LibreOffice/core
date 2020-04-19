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

void moveCursorByProtRule(
    SCCOL& rCol, SCROW& rRow, SCCOL nMovX, SCROW nMovY, SCTAB nTab, const ScDocument* pDoc)
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

void moveCursorByMergedCell(
    SCCOL& rCol, SCROW& rRow, SCCOL nMovX, SCROW nMovY, SCTAB nTab,
    const ScDocument* pDoc, const ScViewData& rViewData)
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

    const ScMergeAttr* pMergeAttr = pDoc->GetAttr(nOrigX, nOrigY, nTab, ATTR_MERGE);

    bool bOriginMerged = false;
    SCCOL nColSpan = 1;
    SCROW nRowSpan = 1;
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
            if (nOrigX < pDoc->MaxCol() && nOrigX < rCol && rCol <= nOrigX + nColSpan - 1)
                rCol = nOrigX + nColSpan;
        }
        else
        {
            pDoc->SkipOverlapped(rCol, rRow, nTab);
        }

        if (nOld < rCol)
        {
            // The block end has moved.  Check the protection setting and move back if needed.
            checkBoundary(pDoc, rCol, rRow);
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
            checkBoundary(pDoc, rCol, rRow);
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
            if (nOrigY < pDoc->MaxRow() && nOrigY < rRow && rRow <= nOrigY + nRowSpan - 1)
                rRow = nOrigY + nRowSpan;
        }
        else
        {
            pDoc->SkipOverlapped(rCol, rRow, nTab);
        }

        if (nOld < rRow)
        {
            // The block end has moved.  Check the protection setting and move back if needed.
            checkBoundary(pDoc, rCol, rRow);
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
            checkBoundary(pDoc, rCol, rRow);
            if (!isCellQualified(pDoc, rCol, rRow, nTab, bSelectLocked, bSelectUnlocked))
                ++rRow;
        }
    }
}

}

void ScTabView::PaintMarks(SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow )
{
    auto pDoc = aViewData.GetDocument();
    if (!pDoc->ValidCol(nStartCol)) nStartCol = pDoc->MaxCol();
    if (!pDoc->ValidRow(nStartRow)) nStartRow = pDoc->MaxRow();
    if (!pDoc->ValidCol(nEndCol)) nEndCol = pDoc->MaxCol();
    if (!pDoc->ValidRow(nEndRow)) nEndRow = pDoc->MaxRow();

    bool bLeft = (nStartCol==0 && nEndCol==pDoc->MaxCol());
    bool bTop = (nStartRow==0 && nEndRow==pDoc->MaxRow());

    if (bLeft)
        PaintLeftArea( nStartRow, nEndRow );
    if (bTop)
        PaintTopArea( nStartCol, nEndCol );

    aViewData.GetDocument()->ExtendMerge( nStartCol, nStartRow, nEndCol, nEndRow,
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

void ScTabView::InitOwnBlockMode()
{
    if (!IsBlockMode())
    {
        // when there is no (old) selection anymore, delete anchor in SelectionEngine:
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

        SelectionChanged();     // status is checked with mark set
    }
}

void ScTabView::InitBlockMode( SCCOL nCurX, SCROW nCurY, SCTAB nCurZ,
                               bool bTestNeg, bool bCols, bool bRows, bool bForceNeg )
{
    if (!IsBlockMode())
    {
        auto pDoc = aViewData.GetDocument();
        if (!pDoc->ValidCol(nCurX)) nCurX = pDoc->MaxCol();
        if (!pDoc->ValidRow(nCurY)) nCurY = pDoc->MaxRow();

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
            nBlockEndY = pDoc->MaxRow();
        }

        if (bBlockRows)
        {
            nBlockStartX = nBlockStartXOrig = 0;
            nBlockEndX = pDoc->MaxCol();
        }

        rMark.SetMarkArea( ScRange( nBlockStartX,nBlockStartY, nTab, nBlockEndX,nBlockEndY, nTab ) );

        UpdateSelectionOverlay();
    }
}

void ScTabView::DoneBlockMode( bool bContinue )
{
    // When switching between sheet and header SelectionEngine DeselectAll may be called,
    // because the other engine does not have any anchor.
    // bMoveIsShift prevents the selection to be canceled.

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
            // the sheet may be invalid at this point because DoneBlockMode from SetTabNo is
            // called (for example, when the current sheet is closed from another View)
            SCTAB nTab = aViewData.GetTabNo();
            ScDocument* pDoc = aViewData.GetDocument();
            if ( pDoc->HasTable(nTab) )
                PaintBlock( true );                             // true -> delete block
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
    ScDocument* pDocument = aViewData.GetDocument();
    if (!pDocument->ValidCol(nCurX)) nCurX = pDocument->MaxCol();
    if (!pDocument->ValidRow(nCurY)) nCurY = pDocument->MaxRow();

    if (!IsBlockMode())
    {
        OSL_FAIL( "MarkCursor not in BlockMode" );
        InitBlockMode( nCurX, nCurY, nCurZ, false, bCols, bRows );
    }

    if (bCols)
        nCurY = pDocument->MaxRow();
    if (bRows)
        nCurX = pDocument->MaxCol();

    ScMarkData& rMark = aViewData.GetMarkData();
    OSL_ENSURE(rMark.IsMarked() || rMark.IsMultiMarked(), "MarkCursor, !IsMarked()");
    ScRange aMarkRange;
    rMark.GetMarkArea(aMarkRange);
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
            // Expand selection area accordingly when the current selection ends
            // with a merged cell.
            SCCOL nCurXOffset = 0;
            SCCOL nBlockStartXOffset = 0;
            SCROW nCurYOffset = 0;
            SCROW nBlockStartYOffset = 0;
            bool bBlockStartMerged = false;

            // The following block checks whether or not the "BlockStart" (anchor)
            // cell is merged.  If it's merged, it'll then move the position of the
            // anchor cell to the corner that's diagonally opposite of the
            // direction of a current selection area.  For instance, if a current
            // selection is moving in the upperleft direction, the anchor cell will
            // move to the lower-right corner of the merged anchor cell, and so on.

            const ScMergeAttr* pMergeAttr =
                pDocument->GetAttr( nBlockStartXOrig, nBlockStartYOrig, nTab, ATTR_MERGE );
            if ( pMergeAttr->IsMerged() )
            {
                SCCOL nColSpan = pMergeAttr->GetColMerge();
                SCROW nRowSpan = pMergeAttr->GetRowMerge();

                if ( nCurX < nBlockStartXOrig + nColSpan - 1 || nCurY < nBlockStartYOrig + nRowSpan - 1 )
                {
                    nBlockStartX = nCurX >= nBlockStartXOrig ? nBlockStartXOrig : nBlockStartXOrig + nColSpan - 1;
                    nBlockStartY = nCurY >= nBlockStartYOrig ? nBlockStartYOrig : nBlockStartYOrig + nRowSpan - 1;
                    nCurXOffset  = (nCurX >= nBlockStartXOrig && nCurX < nBlockStartXOrig + nColSpan - 1) ?
                        nBlockStartXOrig - nCurX + nColSpan - 1 : 0;
                    nCurYOffset  = (nCurY >= nBlockStartYOrig && nCurY < nBlockStartYOrig + nRowSpan - 1) ?
                        nBlockStartYOrig - nCurY + nRowSpan - 1 : 0;
                    bBlockStartMerged = true;
                }
            }

            // The following block checks whether or not the current cell is
            // merged.  If it is, it'll then set the appropriate X & Y offset
            // values (nCurXOffset & nCurYOffset) such that the selection area will
            // grow by those specified offset amounts.  Note that the values of
            // nCurXOffset/nCurYOffset may also be specified in the previous code
            // block, in which case whichever value is greater will take on.

            pMergeAttr = pDocument->GetAttr( nCurX, nCurY, nTab, ATTR_MERGE );
            if ( pMergeAttr->IsMerged() )
            {
                SCCOL nColSpan = pMergeAttr->GetColMerge();
                SCROW nRowSpan = pMergeAttr->GetRowMerge();

                if ( nBlockStartX < nCurX + nColSpan - 1 || nBlockStartY < nCurY + nRowSpan - 1 )
                {
                    if ( nBlockStartX <= nCurX + nColSpan - 1 )
                    {
                        SCCOL nCurXOffsetTemp = (nCurX < nCurX + nColSpan - 1) ? nColSpan - 1 : 0;
                        nCurXOffset = std::max(nCurXOffset, nCurXOffsetTemp);
                    }
                    if ( nBlockStartY <= nCurY + nRowSpan - 1 )
                    {
                        SCROW nCurYOffsetTemp = (nCurY < nCurY + nRowSpan - 1) ? nRowSpan - 1 : 0;
                        nCurYOffset = std::max(nCurYOffset, nCurYOffsetTemp);
                    }
                    if ( ( nBlockStartX > nCurX || nBlockStartY > nCurY ) &&
                         ( nBlockStartX <= nCurX + nColSpan - 1 || nBlockStartY <= nCurY + nRowSpan - 1 ) )
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
            nBlockEndX = std::min<SCCOL>(nCurX + nCurXOffset, pDocument->MaxCol());
            nBlockEndY = std::min(nCurY + nCurYOffset, pDocument->MaxRow());
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

    sal_uInt16 nScrSizeY = SC_SIZE_NONE;
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
                                       SCCOL& rAreaX, SCROW& rAreaY, ScFollowMode& rMode)
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

    // FindAreaPos knows only -1 or 1 as direction
    ScModule* pScModule = SC_MOD();
    bool bLegacyCellSelection = pScModule->GetInputOptions().GetLegacyCellSelection();
    SCCOL nVirtualX = bLegacyCellSelection ? nNewX : nCurX;
    SCROW nVirtualY = bLegacyCellSelection ? nNewY : nCurY;

    SCCOLROW i;
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

    if (eMode==SC_FOLLOW_JUMP)                  // bottom right do not show too much grey
    {
        if (nMovX != 0 && nNewX == pDoc->MaxCol())
            eMode = SC_FOLLOW_LINE;
        if (nMovY != 0 && nNewY == pDoc->MaxRow())
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

void ScTabView::SkipCursorHorizontal(SCCOL& rCurX, SCROW& rCurY, SCCOL nOldX, SCCOL nMovX)
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
    auto nMaxCol = pDoc->ClampToAllocatedColumns(nTab, pDoc->MaxCol());
    do
    {
        bSkipCell = pDoc->ColHidden(rCurX, nTab) || pDoc->IsHorOverlapped(rCurX, rCurY, nTab);
        if (bSkipProtected && !bSkipCell)
            bSkipCell = pDoc->HasAttrib(rCurX, rCurY, nTab, rCurX, rCurY, nTab, HasAttrFlags::Protected);
        if (bSkipUnprotected && !bSkipCell)
            bSkipCell = !pDoc->HasAttrib(rCurX, rCurY, nTab, rCurX, rCurY, nTab, HasAttrFlags::Protected);

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

    if (pDoc->IsVerOverlapped(rCurX, rCurY, nTab))
    {
        aViewData.SetOldCursor(rCurX, rCurY);
        while (pDoc->IsVerOverlapped(rCurX, rCurY, nTab))
            --rCurY;
    }
}

void ScTabView::SkipCursorVertical(SCCOL& rCurX, SCROW& rCurY, SCROW nOldY, SCROW nMovY)
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
        bSkipCell = pDoc->RowHidden(rCurY, nTab, nullptr, &nLastRow) || pDoc->IsVerOverlapped( rCurX, rCurY, nTab );
        if (bSkipProtected && !bSkipCell)
            bSkipCell = pDoc->HasAttrib(rCurX, rCurY, nTab, rCurX, rCurY, nTab, HasAttrFlags::Protected);
        if (bSkipUnprotected && !bSkipCell)
            bSkipCell = !pDoc->HasAttrib(rCurX, rCurY, nTab, rCurX, rCurY, nTab, HasAttrFlags::Protected);

        if (bSkipCell)
        {
            if (rCurY <= 0 || rCurY >= pDoc->MaxRow())
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
        checkBoundary(pDoc, nNewX, nNewY);

        if (nMovX)
        {
            SCCOL nTempX = nNewX;
            while (pDoc->IsHorOverlapped(nTempX, nNewY, nRefTab))
            {
                if (nMovX > 0)
                    ++nTempX;
                else
                    --nTempX;
                if (!checkBoundary(pDoc, nTempX, nNewY))
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
                if (!checkBoundary(pDoc, nNewX, nTempY))
                    break;
            }
            if (isCellQualified(pDoc, nNewX, nTempY, nRefTab, bSelectLocked, bSelectUnlocked))
                nNewY = nTempY;
        }

        pDoc->SkipOverlapped(nNewX, nNewY, nRefTab);
        UpdateRef(nNewX, nNewY, nRefTab);
        SCCOL nTargetCol = nNewX;
        SCROW nTargetRow = nNewY;
        if (((aViewData.GetRefStartX() == 0) || (aViewData.GetRefStartY() == 0)) &&
            ((nNewX != pDoc->MaxCol()) || (nNewY != pDoc->MaxRow())))
        {
            // Row selection
            if ((aViewData.GetRefStartX() == 0) && (nNewX == pDoc->MaxCol()))
                nTargetCol = aViewData.GetCurX();
            // Column selection
            if ((aViewData.GetRefStartY() == 0) && (nNewY == pDoc->MaxRow()))
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
            InitBlockMode(nOrigX, nOrigY, nTab, true);

        moveCursorByProtRule(nBlockEndX, nBlockEndY, nMovX, nMovY, nTab, pDoc);
        checkBoundary(pDoc, nBlockEndX, nBlockEndY);
        moveCursorByMergedCell(nBlockEndX, nBlockEndY, nMovX, nMovY, nTab, pDoc, aViewData);
        checkBoundary(pDoc, nBlockEndX, nBlockEndY);

        MarkCursor(nBlockEndX, nBlockEndY, nTab, false, false, true);

        // Check if the entire row(s) or column(s) are selected.
        ScSplitPos eActive = aViewData.GetActivePart();
        bool bRowSelected = (nBlockStartX == 0 && nBlockEndX == pDoc->MaxCol());
        bool bColSelected = (nBlockStartY == 0 && nBlockEndY == pDoc->MaxRow());
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
}

void ScTabView::SelectAll( bool bContinue )
{
    ScDocument* pDoc = aViewData.GetDocument();
    ScMarkData& rMark = aViewData.GetMarkData();
    SCTAB nTab = aViewData.GetTabNo();

    if (rMark.IsMarked())
    {
        ScRange aMarkRange;
        rMark.GetMarkArea( aMarkRange );
        if ( aMarkRange == ScRange( 0,0,nTab, pDoc->MaxCol(),pDoc->MaxRow(),nTab ) )
            return;
    }

    DoneBlockMode( bContinue );
    InitBlockMode( 0,0,nTab );
    MarkCursor( pDoc->MaxCol(),pDoc->MaxRow(),nTab );

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
                        long nWindowX, long nWindowY, const ScDocument* pDoc, SCTAB nTab,
                        SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                        SCCOL nFixPosX, SCROW nFixPosY )
{
    double fZoomFactor = static_cast<double>(Fraction(nZoom,100));
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
            nBlockX += static_cast<long>(nColTwips * fScaleX);
            if (nBlockX > nWindowX)
                return false;
        }
    }
    for (nCol=nStartCol; nCol<=nEndCol; nCol++)
    {
        sal_uInt16 nColTwips = pDoc->GetColWidth( nCol, nTab );
        if (nColTwips)
        {
            nBlockX += static_cast<long>(nColTwips * fScaleX);
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
            nBlockY += static_cast<long>(nRowTwips * fScaleY);
            if (nBlockY > nWindowY)
                return false;
        }
    }
    for (SCROW nRow = nStartRow; nRow <= nEndRow; ++nRow)
    {
        sal_uInt16 nRowTwips = pDoc->GetRowHeight(nRow, nTab);
        if (nRowTwips)
        {
            nBlockY += static_cast<long>(nRowTwips * fScaleY);
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
                            aWinSize.AdjustWidth(GetGridWidth( SC_SPLIT_LEFT ) );
                        if ( nFixPosY != 0 )
                            aWinSize.AdjustHeight(GetGridHeight( SC_SPLIT_TOP ) );

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
                    ScDocument*         pDoc        = aViewData.GetDocument();
                    ScStyleSheetPool*   pStylePool  = pDoc->GetStyleSheetPool();
                    SfxStyleSheetBase*  pStyleSheet =
                                            pStylePool->Find( pDoc->GetPageStyle( nCurTab ),
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
                            long nOtherWidth = pGridWin[SC_SPLIT_BOTTOMRIGHT]->
                                                        GetOutputSizePixel().Width();
                            if ( eHMode == SC_SPLIT_FIX )
                            {
                                aWinSize.AdjustWidth(nOtherWidth );
                                for ( SCCOL nCol = aViewData.GetPosX(SC_SPLIT_LEFT);
                                        nCol < aViewData.GetFixPosX(); nCol++ )
                                    aPageSize.AdjustWidth(pDoc->GetColWidth( nCol, nCurTab ) );
                            }
                            else if ( nOtherWidth > aWinSize.Width() )
                                aWinSize.setWidth( nOtherWidth );
                        }
                        ScSplitMode eVMode = aViewData.GetVSplitMode();
                        if ( eVMode != SC_SPLIT_NONE && pGridWin[SC_SPLIT_TOPLEFT] )
                        {
                            long nOtherHeight = pGridWin[SC_SPLIT_TOPLEFT]->
                                                        GetOutputSizePixel().Height();
                            if ( eVMode == SC_SPLIT_FIX )
                            {
                                aWinSize.AdjustHeight(nOtherHeight );
                                aPageSize.AdjustHeight(pDoc->GetRowHeight(
                                        aViewData.GetPosY(SC_SPLIT_TOP),
                                        aViewData.GetFixPosY()-1, nCurTab) );
                            }
                            else if ( nOtherHeight > aWinSize.Height() )
                                aWinSize.setHeight( nOtherHeight );
                        }

                        double nPPTX = ScGlobal::nScreenPPTX / aViewData.GetDocShell()->GetOutputFactor();
                        double nPPTY = ScGlobal::nScreenPPTY;

                        long nZoomX = static_cast<long>( aWinSize.Width() * 100 /
                                               ( aPageSize.Width() * nPPTX ) );
                        long nZoomY = static_cast<long>( aWinSize.Height() * 100 /
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

void ScTabView::HideNoteMarker()
{
    for (VclPtr<ScGridWindow> & pWin : pGridWin)
        if (pWin && pWin->IsVisible())
            pWin->HideNoteMarker();
}

void ScTabView::MakeDrawLayer()
{
    if (!pDrawView)
    {
        aViewData.GetDocShell()->MakeDrawLayer();

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
}

void ScTabView::ErrorMessage(const char* pGlobStrId)
{
    if ( SC_MOD()->IsInExecuteDrop() )
    {
        // #i28468# don't show error message when called from Drag&Drop, silently abort instead
        return;
    }

    StopMarking();      // if called by Focus from MouseButtonDown

    weld::Window* pParent = aViewData.GetDialogParent();
    weld::WaitObject aWaitOff( pParent );
    bool bFocus = pParent && pParent->has_focus();

    if (pGlobStrId && strcmp(pGlobStrId, STR_PROTECTIONERR) == 0)
    {
        if (aViewData.GetDocShell()->IsReadOnly())
        {
            pGlobStrId = STR_READONLYERR;
        }
    }

    std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(pParent,
                                                  VclMessageType::Info, VclButtonsType::Ok,
                                                  ScResId(pGlobStrId)));
    xInfoBox->run();

    if (bFocus)
        pParent->grab_focus();
}

void ScTabView::UpdatePageBreakData( bool bForcePaint )
{
    std::unique_ptr<ScPageBreakData> pNewData;

    if (aViewData.IsPagebreakMode())
    {
        ScDocShell* pDocSh = aViewData.GetDocShell();
        ScDocument& rDoc = pDocSh->GetDocument();
        SCTAB nTab = aViewData.GetTabNo();

        sal_uInt16 nCount = rDoc.GetPrintRangeCount(nTab);
        if (!nCount)
            nCount = 1;
        pNewData.reset( new ScPageBreakData(nCount) );

        ScPrintFunc aPrintFunc( pDocSh, pDocSh->GetPrinter(), nTab, 0,0,nullptr, nullptr, pNewData.get() );
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
