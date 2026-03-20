/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/DeleteCellsOperation.hxx>

#include <docfunc.hxx>
#include <docsh.hxx>
#include <markdata.hxx>
#include <editable.hxx>
#include <dpobject.hxx>
#include <attrib.hxx>
#include <dociter.hxx>
#include <patattr.hxx>
#include <cellmergeoption.hxx>
#include <tabvwsh.hxx>
#include <undoblk.hxx>
#include <refundo.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <strings.hrc>

#include <SheetViewManager.hxx>

#include <sfx2/app.hxx>
#include <vcl/weld.hxx>
#include <osl/diagnose.h>

namespace
{
/**
 * Check if this deletion attempt would end up cutting one or more pivot
 * tables in half, which is not desirable.
 *
 * @return true if this deletion can be done safely without shearing any
 *         existing pivot tables, false otherwise.
 */
bool canDeleteCellsByPivot(const ScRange& rRange, const ScMarkData& rMarkData, DelCellCmd eCmd,
                           const ScDocument& rDoc)
{
    if (!rDoc.HasPivotTable())
        // This document has no pivot tables.
        return true;

    const ScDPCollection* pDPs = rDoc.GetDPCollection();

    ScRange aRange(rRange); // local copy

    switch (eCmd)
    {
        case DelCellCmd::Rows:
        {
            aRange.aStart.SetCol(0);
            aRange.aEnd.SetCol(rDoc.MaxCol());
            [[fallthrough]];
        }
        case DelCellCmd::CellsUp:
        {
            auto bIntersects = std::any_of(
                rMarkData.begin(), rMarkData.end(), [&pDPs, &aRange](const SCTAB& rTab) {
                    return pDPs->IntersectsTableByColumns(aRange.aStart.Col(), aRange.aEnd.Col(),
                                                          aRange.aStart.Row(), rTab);
                });
            if (bIntersects)
                // This column range cuts through at least one pivot table.  Not good.
                return false;

            ScRange aTest(aRange);
            for (const auto& rTab : rMarkData)
            {
                aTest.aStart.SetTab(rTab);
                aTest.aEnd.SetTab(rTab);
                if (pDPs->HasTable(aTest))
                    return false;
            }
        }
        break;
        case DelCellCmd::Cols:
        {
            aRange.aStart.SetRow(0);
            aRange.aEnd.SetRow(rDoc.MaxRow());
            [[fallthrough]];
        }
        case DelCellCmd::CellsLeft:
        {
            auto bIntersects = std::any_of(
                rMarkData.begin(), rMarkData.end(), [&pDPs, &aRange](const SCTAB& rTab) {
                    return pDPs->IntersectsTableByRows(aRange.aStart.Col(), aRange.aStart.Row(),
                                                       aRange.aEnd.Row(), rTab);
                });
            if (bIntersects)
                // This column range cuts through at least one pivot table.  Not good.
                return false;

            ScRange aTest(aRange);
            for (const auto& rTab : rMarkData)
            {
                aTest.aStart.SetTab(rTab);
                aTest.aEnd.SetTab(rTab);
                if (pDPs->HasTable(aTest))
                    return false;
            }
        }
        break;
        default:;
    }
    return true;
}

} // anonymous namespace

namespace sc
{
DeleteCellsOperation::DeleteCellsOperation(ScDocShell& rDocShell, ScRange const& rRange,
                                           ScMarkData const* pTabMark, DelCellCmd eCmd, bool bApi)
    : Operation(toOperationType(eCmd), true, bApi)
    , mrDocShell(rDocShell)
    , maRange(rRange)
    , mpTabMark(pTabMark)
    , meCmd(eCmd)
{
}

OperationType DeleteCellsOperation::toOperationType(DelCellCmd eCmd)
{
    switch (eCmd)
    {
        case DelCellCmd::Cols:
            return OperationType::DeleteColumns;
        case DelCellCmd::Rows:
            return OperationType::DeleteRows;
        case DelCellCmd::CellsLeft:
            return OperationType::DeleteCellsLeft;
        case DelCellCmd::CellsUp:
            return OperationType::DeleteCellsUp;
        default:
            return OperationType::Unknown;
    }
}

bool DeleteCellsOperation::canRunTheOperation() const
{
    return !isInputOnSheetViewAutoFilter(maRange);
}

bool DeleteCellsOperation::runImplementation()
{
    ScDocShellModificator aModificator(mrDocShell);
    ScDocument& rDoc = mrDocShell.GetDocument();

    if (rDoc.GetChangeTrack()
        && ((meCmd == DelCellCmd::CellsUp
             && (maRange.aStart.Col() != 0 || maRange.aEnd.Col() != rDoc.MaxCol()))
            || (meCmd == DelCellCmd::CellsLeft
                && (maRange.aStart.Row() != 0 || maRange.aEnd.Row() != rDoc.MaxRow()))))
    {
        // We should not reach this via UI disabled slots.
        assert(mbApi);
        SAL_WARN("sc.ui", "ScDocFunc::DeleteCells - no change-tracking of partial cell shift");
        return false;
    }

    SCCOL nStartCol = maRange.aStart.Col();
    SCROW nStartRow = maRange.aStart.Row();
    SCTAB nStartTab = maRange.aStart.Tab();
    SCCOL nEndCol = maRange.aEnd.Col();
    SCROW nEndRow = maRange.aEnd.Row();
    SCTAB nEndTab = maRange.aEnd.Tab();

    if (!rDoc.ValidRow(nStartRow) || !rDoc.ValidRow(nEndRow))
    {
        OSL_FAIL("invalid row in DeleteCells");
        return false;
    }

    SCTAB nTabCount = rDoc.GetTableCount();
    SCCOL nPaintStartCol = nStartCol;
    SCROW nPaintStartRow = nStartRow;
    SCCOL nPaintEndCol = nEndCol;
    SCROW nPaintEndRow = nEndRow;
    PaintPartFlags nPaintFlags = PaintPartFlags::Grid;

    if (mbRecord && !rDoc.IsUndoEnabled())
        mbRecord = false;

    ScMarkData aMark(rDoc.GetSheetLimits());
    if (mpTabMark)
        aMark = *mpTabMark;
    else
    {
        SCTAB nCount = 0;
        for (SCTAB i = 0; i < nTabCount; i++)
        {
            if (!rDoc.IsScenario(i))
            {
                nCount++;
                if (nCount == nEndTab + 1)
                {
                    aMark.SelectTable(i, true);
                    break;
                }
            }
        }
    }

    ScMarkData aFullMark(aMark); // including scenario sheets
    for (const auto& rTab : aMark)
    {
        if (rTab >= nTabCount)
            break;

        for (SCTAB j = rTab + 1; j < nTabCount && rDoc.IsScenario(j); j++)
            aFullMark.SelectTable(j, true);
    }

    SCTAB nSelCount = aMark.GetSelectCount();

    SCCOL nUndoStartCol = nStartCol;
    SCROW nUndoStartRow = nStartRow;
    SCCOL nUndoEndCol = nEndCol;
    SCROW nUndoEndRow = nEndRow;

    ScRange aExtendMergeRange(maRange);

    if (maRange.aStart == maRange.aEnd && rDoc.HasAttrib(maRange, HasAttrFlags::Merged))
    {
        rDoc.ExtendMerge(aExtendMergeRange);
        rDoc.ExtendOverlapped(aExtendMergeRange);
        nUndoEndCol = aExtendMergeRange.aEnd.Col();
        nUndoEndRow = aExtendMergeRange.aEnd.Row();
        nPaintEndCol = nUndoEndCol;
        nPaintEndRow = nUndoEndRow;
    }

    if (meCmd == DelCellCmd::Rows)
    {
        nUndoStartCol = 0;
        nUndoEndCol = rDoc.MaxCol();
    }
    if (meCmd == DelCellCmd::Cols)
    {
        nUndoStartRow = 0;
        nUndoEndRow = rDoc.MaxRow();
    }
    // Test for cell protection

    SCCOL nEditTestEndX = nUndoEndCol;
    if (meCmd == DelCellCmd::Cols || meCmd == DelCellCmd::CellsLeft)
        nEditTestEndX = rDoc.MaxCol();
    SCROW nEditTestEndY = nUndoEndRow;
    if (meCmd == DelCellCmd::Rows || meCmd == DelCellCmd::CellsUp)
        nEditTestEndY = rDoc.MaxRow();

    ScEditableTester aTester;
    switch (meCmd)
    {
        case DelCellCmd::Cols:
            aTester = ScEditableTester::CreateAndTestBlockForAction(
                rDoc, sc::EditAction::DeleteColumns, nUndoStartCol, 0, nUndoEndCol, rDoc.MaxRow(),
                aMark);
            break;
        case DelCellCmd::Rows:
            aTester = ScEditableTester::CreateAndTestBlockForAction(
                rDoc, sc::EditAction::DeleteRows, 0, nUndoStartRow, rDoc.MaxCol(), nUndoEndRow,
                aMark);
            break;
        default:
            aTester = ScEditableTester::CreateAndTestSelectedBlock(
                rDoc, nUndoStartCol, nUndoStartRow, nEditTestEndX, nEditTestEndY, aMark);
    }

    if (!aTester.IsEditable())
    {
        if (!mbApi)
            mrDocShell.ErrorMessage(aTester.GetMessageId());
        return false;
    }

    if (!canDeleteCellsByPivot(maRange, aMark, meCmd, rDoc))
    {
        if (!mbApi)
            mrDocShell.ErrorMessage(STR_NO_INSERT_DELETE_OVER_PIVOT_TABLE);
        return false;
    }
    // Test for merged cells

    SCCOL nMergeTestEndCol = (meCmd == DelCellCmd::CellsLeft) ? rDoc.MaxCol() : nUndoEndCol;
    SCROW nMergeTestEndRow = (meCmd == DelCellCmd::CellsUp) ? rDoc.MaxRow() : nUndoEndRow;
    SCCOL nExtendStartCol = nUndoStartCol;
    SCROW nExtendStartRow = nUndoStartRow;
    bool bNeedRefresh = false;

    //Issue 8302 want to be able to insert into the middle of merged cells
    //the patch comes from maoyg
    ::std::vector<ScRange> qDecreaseRange;
    bool bDeletingMerge = false;
    OUString aUndo = ScResId(STR_UNDO_DELETECELLS);
    if (mbRecord)
    {
        ViewShellId nViewShellId(-1);
        if (ScTabViewShell* pViewSh = ScTabViewShell::GetActiveViewShell())
            nViewShellId = pViewSh->GetViewShellId();
        mrDocShell.GetUndoManager()->EnterListAction(aUndo, aUndo, 0, nViewShellId);
    }
    std::unique_ptr<ScUndoRemoveMerge> pUndoRemoveMerge;

    for (const SCTAB i : aMark)
    {
        if (i >= nTabCount)
            break;

        if (rDoc.HasAttrib(nUndoStartCol, nUndoStartRow, i, nMergeTestEndCol, nMergeTestEndRow, i,
                           HasAttrFlags::Merged | HasAttrFlags::Overlapped))
        {
            SCCOL nMergeStartCol = nUndoStartCol;
            SCROW nMergeStartRow = nUndoStartRow;
            SCCOL nMergeEndCol = nMergeTestEndCol;
            SCROW nMergeEndRow = nMergeTestEndRow;

            rDoc.ExtendMerge(nMergeStartCol, nMergeStartRow, nMergeEndCol, nMergeEndRow, i);
            rDoc.ExtendOverlapped(nMergeStartCol, nMergeStartRow, nMergeEndCol, nMergeEndRow, i);
            if ((meCmd == DelCellCmd::CellsUp
                 && (nMergeStartCol != nUndoStartCol || nMergeEndCol != nMergeTestEndCol))
                || (meCmd == DelCellCmd::CellsLeft
                    && (nMergeStartRow != nUndoStartRow || nMergeEndRow != nMergeTestEndRow)))
            {
                if (!mbApi)
                    mrDocShell.ErrorMessage(STR_MSSG_DELETECELLS_0);
                mrDocShell.GetUndoManager()->LeaveListAction();
                return false;
            }

            nExtendStartCol = nMergeStartCol;
            nExtendStartRow = nMergeStartRow;
            SCCOL nTestCol = -1;
            SCROW nTestRow1 = -1;
            SCROW nTestRow2 = -1;

            ScDocAttrIterator aTestIter(rDoc, i, nUndoStartCol, nUndoStartRow, nMergeTestEndCol,
                                        nMergeTestEndRow);
            ScRange aExtendRange(nUndoStartCol, nUndoStartRow, i, nMergeTestEndCol,
                                 nMergeTestEndRow, i);
            const ScPatternAttr* pPattern = nullptr;
            while ((pPattern = aTestIter.GetNext(nTestCol, nTestRow1, nTestRow2)) != nullptr)
            {
                const ScMergeAttr& rMergeFlag = pPattern->GetItem(ATTR_MERGE);
                const ScMergeFlagAttr& rMergeFlagAttr = pPattern->GetItem(ATTR_MERGE_FLAG);
                ScMF nNewFlags = rMergeFlagAttr.GetValue() & (ScMF::Hor | ScMF::Ver);
                if (rMergeFlag.IsMerged() || nNewFlags == ScMF::Hor || nNewFlags == ScMF::Ver)
                {
                    ScRange aRange(nTestCol, nTestRow1, i);
                    rDoc.ExtendOverlapped(aRange);
                    rDoc.ExtendMerge(aRange, true);

                    if (nTestRow1 < nTestRow2 && nNewFlags == ScMF::Hor)
                    {
                        for (SCROW nTestRow = nTestRow1; nTestRow <= nTestRow2; nTestRow++)
                        {
                            ScRange aTestRange(nTestCol, nTestRow, i);
                            rDoc.ExtendOverlapped(aTestRange);
                            rDoc.ExtendMerge(aTestRange, true);
                            ScRange aMergeRange(aTestRange.aStart.Col(), aTestRange.aStart.Row(),
                                                i);
                            if (!aExtendRange.Contains(aMergeRange))
                            {
                                qDecreaseRange.push_back(aTestRange);
                                bDeletingMerge = true;
                            }
                        }
                    }
                    else
                    {
                        ScRange aMergeRange(aRange.aStart.Col(), aRange.aStart.Row(), i);
                        if (!aExtendRange.Contains(aMergeRange))
                        {
                            qDecreaseRange.push_back(aRange);
                        }
                        bDeletingMerge = true;
                    }
                }
            }

            if (bDeletingMerge)
            {
                if (meCmd == DelCellCmd::Rows || meCmd == DelCellCmd::CellsUp)
                {
                    nStartRow = aExtendMergeRange.aStart.Row();
                    nEndRow = aExtendMergeRange.aEnd.Row();
                    bNeedRefresh = true;

                    if (meCmd == DelCellCmd::CellsUp)
                    {
                        nEndCol = aExtendMergeRange.aEnd.Col();
                    }
                    else
                    {
                        nStartCol = 0;
                        nEndCol = rDoc.MaxCol();
                    }
                }
                else if (meCmd == DelCellCmd::CellsLeft || meCmd == DelCellCmd::Cols)
                {
                    nStartCol = aExtendMergeRange.aStart.Col();
                    nEndCol = aExtendMergeRange.aEnd.Col();
                    if (meCmd == DelCellCmd::CellsLeft)
                    {
                        nEndRow = aExtendMergeRange.aEnd.Row();
                        bNeedRefresh = true;
                    }
                    else
                    {
                        nStartRow = 0;
                        nEndRow = rDoc.MaxRow();
                    }
                }

                if (!qDecreaseRange.empty())
                {
                    if (mbRecord && !pUndoRemoveMerge)
                    {
                        ScDocumentUniquePtr pUndoDoc(new ScDocument(SCDOCMODE_UNDO));
                        pUndoDoc->InitUndo(rDoc, *aMark.begin(), *aMark.rbegin());
                        pUndoRemoveMerge.reset(
                            new ScUndoRemoveMerge(&mrDocShell, maRange, std::move(pUndoDoc)));
                    }

                    for (const ScRange& aRange : qDecreaseRange)
                    {
                        if (rDoc.HasAttrib(aRange, HasAttrFlags::Overlapped | HasAttrFlags::Merged))
                        {
                            mrDocShell.GetDocFunc().UnmergeCells(aRange, mbRecord,
                                                                 pUndoRemoveMerge.get());
                        }
                    }
                }
            }
            else
            {
                if (!mbApi)
                    mrDocShell.ErrorMessage(STR_MSSG_DELETECELLS_0);
                mrDocShell.GetUndoManager()->LeaveListAction();
                return false;
            }
        }
    }

    if (mbRecord && pUndoRemoveMerge)
    {
        mrDocShell.GetUndoManager()->AddUndoAction(std::move(pUndoRemoveMerge));
    }

    //     do it

    weld::WaitObject aWait(
        ScDocShell::
            GetActiveDialogParent()); // important because of TrackFormulas in UpdateReference

    ScUndoDeleteCells* pUndoDeleteCells = nullptr;
    ScDocumentUniquePtr pUndoDoc;
    std::unique_ptr<ScDocument> pRefUndoDoc;
    std::unique_ptr<ScRefUndoData> pUndoData;
    if (mbRecord)
    {
        // With the fix for #101329#, UpdateRef always puts cells into pRefUndoDoc at their old
        // position, so it's no longer necessary to copy more than the deleted range into pUndoDoc.

        pUndoDoc.reset(new ScDocument(SCDOCMODE_UNDO));
        pUndoDoc->InitUndo(rDoc, 0, nTabCount - 1, (meCmd == DelCellCmd::Cols),
                           (meCmd == DelCellCmd::Rows));
        for (const auto& rTab : aMark)
        {
            if (rTab >= nTabCount)
                break;

            SCTAB nScenarioCount = 0;

            for (SCTAB j = rTab + 1; j < nTabCount && rDoc.IsScenario(j); j++)
                nScenarioCount++;

            rDoc.CopyToDocument(
                nUndoStartCol, nUndoStartRow, rTab, nUndoEndCol, nUndoEndRow, rTab + nScenarioCount,
                InsertDeleteFlags::ALL | InsertDeleteFlags::NOCAPTIONS, false, *pUndoDoc);
        }

        pRefUndoDoc.reset(new ScDocument(SCDOCMODE_UNDO));
        pRefUndoDoc->InitUndo(rDoc, 0, nTabCount - 1);

        pUndoData.reset(new ScRefUndoData(&rDoc));

        rDoc.BeginDrawUndo();
    }

    sal_uInt16 nExtFlags = 0;
    for (const auto& rTab : aMark)
    {
        if (rTab >= nTabCount)
            break;

        mrDocShell.UpdatePaintExt(nExtFlags, nStartCol, nStartRow, rTab, nEndCol, nEndRow, rTab);
    }

    switch (meCmd)
    {
        case DelCellCmd::CellsUp:
        case DelCellCmd::CellsLeft:
            rDoc.DeleteObjectsInArea(nStartCol, nStartRow, nEndCol, nEndRow, aMark, true);
            break;
        case DelCellCmd::Rows:
            rDoc.DeleteObjectsInArea(0, nStartRow, rDoc.MaxCol(), nEndRow, aMark, true);
            break;
        case DelCellCmd::Cols:
            rDoc.DeleteObjectsInArea(nStartCol, 0, nEndCol, rDoc.MaxRow(), aMark, true);
            break;
        default:
            break;
    }

    bool bUndoOutline = false;
    switch (meCmd)
    {
        case DelCellCmd::CellsUp:
            rDoc.DeleteRow(nStartCol, 0, nEndCol, MAXTAB, nStartRow,
                           static_cast<SCSIZE>(nEndRow - nStartRow + 1), pRefUndoDoc.get(), nullptr,
                           &aFullMark);
            nPaintEndRow = rDoc.MaxRow();
            break;
        case DelCellCmd::Rows:
            rDoc.DeleteRow(0, 0, rDoc.MaxCol(), MAXTAB, nStartRow,
                           static_cast<SCSIZE>(nEndRow - nStartRow + 1), pRefUndoDoc.get(),
                           &bUndoOutline, &aFullMark);
            nPaintStartCol = 0;
            nPaintEndCol = rDoc.MaxCol();
            nPaintEndRow = rDoc.MaxRow();
            nPaintFlags |= PaintPartFlags::Left;
            break;
        case DelCellCmd::CellsLeft:
            rDoc.DeleteCol(nStartRow, 0, nEndRow, MAXTAB, nStartCol,
                           static_cast<SCSIZE>(nEndCol - nStartCol + 1), pRefUndoDoc.get(), nullptr,
                           &aFullMark);
            nPaintEndCol = rDoc.MaxCol();
            break;
        case DelCellCmd::Cols:
            rDoc.DeleteCol(0, 0, rDoc.MaxRow(), MAXTAB, nStartCol,
                           static_cast<SCSIZE>(nEndCol - nStartCol + 1), pRefUndoDoc.get(),
                           &bUndoOutline, &aFullMark);
            nPaintStartRow = 0;
            nPaintEndRow = rDoc.MaxRow();
            nPaintEndCol = rDoc.MaxCol();
            nPaintFlags |= PaintPartFlags::Top;
            break;
        default:
            OSL_FAIL("Wrong code at deleting");
            break;
    }

    //! Test if the size of outline has changed

    if (mbRecord)
    {
        for (const auto& rTab : aFullMark)
        {
            if (rTab >= nTabCount)
                break;

            pRefUndoDoc->DeleteAreaTab(nUndoStartCol, nUndoStartRow, nUndoEndCol, nUndoEndRow, rTab,
                                       InsertDeleteFlags::ALL);
        }

        //  for all sheets, so that formulas can be copied
        pUndoDoc->AddUndoTab(0, nTabCount - 1);

        //  copy with bColRowFlags=false (#54194#)
        pRefUndoDoc->CopyToDocument(0, 0, 0, rDoc.MaxCol(), rDoc.MaxRow(), MAXTAB,
                                    InsertDeleteFlags::FORMULA, false, *pUndoDoc, nullptr, false);
        pRefUndoDoc.reset();

        std::unique_ptr<SCTAB[]> pTabs(new SCTAB[nSelCount]);
        std::unique_ptr<SCTAB[]> pScenarios(new SCTAB[nSelCount]);
        SCTAB nUndoPos = 0;

        for (const auto& rTab : aMark)
        {
            if (rTab >= nTabCount)
                break;

            SCTAB nCount = 0;
            for (SCTAB j = rTab + 1; j < nTabCount && rDoc.IsScenario(j); j++)
                nCount++;

            pScenarios[nUndoPos] = nCount;
            pTabs[nUndoPos] = rTab;
            nUndoPos++;
        }

        if (!bDeletingMerge)
        {
            mrDocShell.GetUndoManager()->LeaveListAction();
        }

        auto pUndoAction = std::make_unique<ScUndoDeleteCells>(
            &mrDocShell, ScRange(nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab),
            nUndoPos, std::move(pTabs), std::move(pScenarios), meCmd, std::move(pUndoDoc),
            std::move(pUndoData));
        pUndoDeleteCells = pUndoAction.get();
        mrDocShell.GetUndoManager()->AddUndoAction(std::move(pUndoAction));
    }

    // #i8302 want to be able to insert into the middle of merged cells
    // the patch comes from maoyg

    while (!qDecreaseRange.empty())
    {
        ScRange aRange = qDecreaseRange.back();

        sal_Int32 nDecreaseRowCount = 0;
        sal_Int32 nDecreaseColCount = 0;
        if (meCmd == DelCellCmd::CellsUp || meCmd == DelCellCmd::Rows)
        {
            if (nStartRow >= aRange.aStart.Row() && nStartRow <= aRange.aEnd.Row()
                && nEndRow >= aRange.aStart.Row() && nEndRow <= aRange.aEnd.Row())
                nDecreaseRowCount = nEndRow - nStartRow + 1;
            else if (nStartRow >= aRange.aStart.Row() && nStartRow <= aRange.aEnd.Row()
                     && nEndRow >= aRange.aStart.Row() && nEndRow >= aRange.aEnd.Row())
                nDecreaseRowCount = aRange.aEnd.Row() - nStartRow + 1;
            else if (nStartRow >= aRange.aStart.Row() && nStartRow >= aRange.aEnd.Row()
                     && nEndRow >= aRange.aStart.Row() && nEndRow <= aRange.aEnd.Row())
                nDecreaseRowCount = aRange.aEnd.Row() - nEndRow + 1;
        }
        else if (meCmd == DelCellCmd::CellsLeft || meCmd == DelCellCmd::Cols)
        {
            if (nStartCol >= aRange.aStart.Col() && nStartCol <= aRange.aEnd.Col()
                && nEndCol >= aRange.aStart.Col() && nEndCol <= aRange.aEnd.Col())
                nDecreaseColCount = nEndCol - nStartCol + 1;
            else if (nStartCol >= aRange.aStart.Col() && nStartCol <= aRange.aEnd.Col()
                     && nEndCol >= aRange.aStart.Col() && nEndCol >= aRange.aEnd.Col())
                nDecreaseColCount = aRange.aEnd.Col() - nStartCol + 1;
            else if (nStartCol >= aRange.aStart.Col() && nStartCol >= aRange.aEnd.Col()
                     && nEndCol >= aRange.aStart.Col() && nEndCol <= aRange.aEnd.Col())
                nDecreaseColCount = aRange.aEnd.Col() - nEndCol + 1;
        }

        switch (meCmd)
        {
            case DelCellCmd::CellsUp:
            case DelCellCmd::Rows:
                aRange.aEnd.SetRow(static_cast<SCCOL>(aRange.aEnd.Row() - nDecreaseRowCount));
                break;
            case DelCellCmd::CellsLeft:
            case DelCellCmd::Cols:
                aRange.aEnd.SetCol(static_cast<SCCOL>(aRange.aEnd.Col() - nDecreaseColCount));
                break;
            default:
                break;
        }

        if (!rDoc.HasAttrib(aRange, HasAttrFlags::Overlapped | HasAttrFlags::Merged))
        {
            ScCellMergeOption aMergeOption(aRange);
            mrDocShell.GetDocFunc().MergeCells(aMergeOption, false, true, true);
        }
        qDecreaseRange.pop_back();
    }

    if (bDeletingMerge)
        mrDocShell.GetUndoManager()->LeaveListAction();

    if (meCmd == DelCellCmd::Cols || meCmd == DelCellCmd::CellsLeft)
        nMergeTestEndCol = rDoc.MaxCol();
    if (meCmd == DelCellCmd::Rows || meCmd == DelCellCmd::CellsUp)
        nMergeTestEndRow = rDoc.MaxRow();
    if (bNeedRefresh)
    {
        // #i51445# old merge flag attributes must be deleted also for single cells,
        // not only for whole columns/rows

        ScPatternAttr aPattern(rDoc.getCellAttributeHelper());
        aPattern.GetItemSet().Put(ScMergeFlagAttr());

        rDoc.ApplyPatternArea(nExtendStartCol, nExtendStartRow, nMergeTestEndCol, nMergeTestEndRow,
                              aMark, aPattern);

        for (const auto& rTab : aMark)
        {
            if (rTab >= nTabCount)
                break;

            SCTAB nScenarioCount = 0;

            for (SCTAB j = rTab + 1; j < nTabCount && rDoc.IsScenario(j); j++)
                nScenarioCount++;

            ScRange aMergedRange(nExtendStartCol, nExtendStartRow, rTab, nMergeTestEndCol,
                                 nMergeTestEndRow, rTab + nScenarioCount);
            rDoc.ExtendMerge(aMergedRange, true);
        }
    }

    for (const auto& rTab : aMark)
    {
        if (rTab >= nTabCount)
            break;

        rDoc.RefreshAutoFilter(nExtendStartCol, nExtendStartRow, nMergeTestEndCol, nMergeTestEndRow,
                               rTab);
    }

    for (const auto& rTab : aMark)
    {
        if (rTab >= nTabCount)
            break;

        rDoc.SetDrawPageSize(rTab);

        if (meCmd == DelCellCmd::Cols || meCmd == DelCellCmd::Rows)
            rDoc.UpdatePageBreaks(rTab);

        mrDocShell.UpdatePaintExt(nExtFlags, nPaintStartCol, nPaintStartRow, rTab, nPaintEndCol,
                                  nPaintEndRow, rTab);

        SCTAB nScenarioCount = 0;

        for (SCTAB j = rTab + 1; j < nTabCount && rDoc.IsScenario(j); j++)
            nScenarioCount++;

        //  delete entire rows: do not adjust
        if (meCmd == DelCellCmd::Rows
            || !mrDocShell.GetDocFunc().AdjustRowHeight(ScRange(0, nPaintStartRow, rTab,
                                                                rDoc.MaxCol(), nPaintEndRow,
                                                                rTab + nScenarioCount),
                                                        true, mbApi))
            mrDocShell.PostPaint(nPaintStartCol, nPaintStartRow, rTab, nPaintEndCol, nPaintEndRow,
                                 rTab + nScenarioCount, nPaintFlags, nExtFlags);
        else
        {
            //  paint only what is not done by AdjustRowHeight
            if (nExtFlags & SC_PF_LINES)
                ScDocFunc::PaintAbove(mrDocShell,
                                      ScRange(nPaintStartCol, nPaintStartRow, rTab, nPaintEndCol,
                                              nPaintEndRow, rTab + nScenarioCount));
            if (nPaintFlags & PaintPartFlags::Top)
                mrDocShell.PostPaint(nPaintStartCol, nPaintStartRow, rTab, nPaintEndCol,
                                     nPaintEndRow, rTab + nScenarioCount, PaintPartFlags::Top);
        }
    }

    // The cursor position needs to be modified earlier than updating
    // any enabled edit view which is triggered by SetDocumentModified below.
    ScTabViewShell* pViewSh = mrDocShell.GetBestViewShell();
    if (pViewSh)
    {
        bool bDeleteCols = (meCmd == DelCellCmd::Cols);
        bool bDeleteRows = (meCmd == DelCellCmd::Rows);

        if (bDeleteCols)
        {
            pViewSh->OnLOKInsertDeleteColumn(maRange.aStart.Col(),
                                             -1 * (maRange.aEnd.Col() - maRange.aStart.Col() + 1));
        }
        if (bDeleteRows)
        {
            pViewSh->OnLOKInsertDeleteRow(maRange.aStart.Row(),
                                          -1 * (maRange.aEnd.Row() - maRange.aStart.Row() + 1));
        }

        // Update sheet view sort ranges to account for deleted rows/columns
        if (bDeleteRows || bDeleteCols)
        {
            SCTAB nDefaultViewTab = rDoc.GetDefaultViewTableNumber(nStartTab);
            std::shared_ptr<sc::SheetViewManager> pManager
                = rDoc.GetSheetViewManager(nDefaultViewTab);
            if (pManager)
            {
                auto pSortDataBefore = pManager->captureSortData();

                if (bDeleteRows)
                    pManager->deletedRows(nStartRow, nEndRow - nStartRow + 1);
                if (bDeleteCols)
                    pManager->deletedColumns(nStartCol, nEndCol - nStartCol + 1);

                auto pSortDataAfter = pManager->captureSortData();

                if (mbRecord && pUndoDeleteCells)
                {
                    pUndoDeleteCells->setDefaultViewContext(
                        nDefaultViewTab, std::move(pSortDataBefore), std::move(pSortDataAfter));
                }
            }
        }

        syncSheetViews();
    }

    aModificator.SetDocumentModified();

    SfxGetpApp()->Broadcast(SfxHint(SfxHintId::ScAreaLinksChanged));

    return true;
}

} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
