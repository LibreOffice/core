/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/InsertCellsOperation.hxx>

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

#include <comphelper/lok.hxx>
#include <sfx2/app.hxx>
#include <vcl/weld.hxx>
#include <osl/diagnose.h>

namespace
{
/**
 * Check if this insertion attempt would end up cutting one or more pivot
 * tables in half, which is not desirable.
 *
 * @return true if this insertion can be done safely without shearing any
 *         existing pivot tables, false otherwise.
 */
bool canInsertCellsByPivot(const ScRange& rRange, const ScMarkData& rMarkData, InsCellCmd eCmd,
                           const ScDocument& rDoc)
{
    if (!rDoc.HasPivotTable())
        // This document has no pivot tables.
        return true;

    const ScDPCollection* pDPs = rDoc.GetDPCollection();

    ScRange aRange(rRange); // local copy
    switch (eCmd)
    {
        case INS_INSROWS_BEFORE:
        {
            aRange.aStart.SetCol(0);
            aRange.aEnd.SetCol(rDoc.MaxCol());
            [[fallthrough]];
        }
        case INS_CELLSDOWN:
        {
            auto bIntersects = std::any_of(
                rMarkData.begin(), rMarkData.end(), [&pDPs, &aRange](const SCTAB& rTab) {
                    return pDPs->IntersectsTableByColumns(aRange.aStart.Col(), aRange.aEnd.Col(),
                                                          aRange.aStart.Row(), rTab);
                });
            if (bIntersects)
                // This column range cuts through at least one pivot table.  Not good.
                return false;

            // Start row must be either at the top or above any pivot tables.
            if (aRange.aStart.Row() < 0)
                // I don't know how to handle this case.
                return false;

            if (aRange.aStart.Row() == 0)
                // First row is always allowed.
                return true;

            ScRange aTest(aRange);
            aTest.aStart.IncRow(-1); // Test one row up.
            aTest.aEnd.SetRow(aTest.aStart.Row());
            for (const auto& rTab : rMarkData)
            {
                aTest.aStart.SetTab(rTab);
                aTest.aEnd.SetTab(rTab);
                if (pDPs->HasTable(aTest))
                    return false;
            }
        }
        break;
        case INS_INSCOLS_BEFORE:
        {
            aRange.aStart.SetRow(0);
            aRange.aEnd.SetRow(rDoc.MaxRow());
            [[fallthrough]];
        }
        case INS_CELLSRIGHT:
        {
            auto bIntersects = std::any_of(
                rMarkData.begin(), rMarkData.end(), [&pDPs, &aRange](const SCTAB& rTab) {
                    return pDPs->IntersectsTableByRows(aRange.aStart.Col(), aRange.aStart.Row(),
                                                       aRange.aEnd.Row(), rTab);
                });
            if (bIntersects)
                // This column range cuts through at least one pivot table.  Not good.
                return false;

            // Start row must be either at the top or above any pivot tables.
            if (aRange.aStart.Col() < 0)
                // I don't know how to handle this case.
                return false;

            if (aRange.aStart.Col() == 0)
                // First row is always allowed.
                return true;

            ScRange aTest(aRange);
            aTest.aStart.IncCol(-1); // Test one column to the left.
            aTest.aEnd.SetCol(aTest.aStart.Col());
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
InsertCellsOperation::InsertCellsOperation(ScDocShell& rDocShell, ScRange const& rRange,
                                           ScMarkData const* pTabMark, InsCellCmd eCmd,
                                           bool bRecord, bool bApi, bool bPartOfPaste,
                                           size_t nInsertCount)
    : Operation(toOperationType(eCmd), bRecord, bApi)
    , mrDocShell(rDocShell)
    , maRange(rRange)
    , mpTabMark(pTabMark)
    , meCmd(eCmd)
    , mbPartOfPaste(bPartOfPaste)
    , mnInsertCount(nInsertCount)
{
}

OperationType InsertCellsOperation::toOperationType(InsCellCmd eCmd)
{
    switch (eCmd)
    {
        case INS_INSCOLS_BEFORE:
            return OperationType::InsertColumnsBefore;
        case INS_INSCOLS_AFTER:
            return OperationType::InsertColumnsAfter;
        case INS_INSROWS_BEFORE:
            return OperationType::InsertRowsBefore;
        case INS_INSROWS_AFTER:
            return OperationType::InsertRowsAfter;
        case INS_CELLSDOWN:
            return OperationType::InsertCellsDown;
        case INS_CELLSRIGHT:
            return OperationType::InsertCellsRight;
        default:
            return OperationType::Unknown;
    }
}

bool InsertCellsOperation::runImplementation()
{
    ScDocShellModificator aModificator(mrDocShell);
    ScDocument& rDoc = mrDocShell.GetDocument();

    if (mrDocShell.GetDocument().GetChangeTrack()
        && ((meCmd == INS_CELLSDOWN
             && (maRange.aStart.Col() != 0 || maRange.aEnd.Col() != rDoc.MaxCol()))
            || (meCmd == INS_CELLSRIGHT
                && (maRange.aStart.Row() != 0 || maRange.aEnd.Row() != rDoc.MaxRow()))))
    {
        // We should not reach this via UI disabled slots.
        assert(mbApi);
        SAL_WARN("sc.ui", "ScDocFunc::InsertCells - no change-tracking of partial cell shift");
        return false;
    }

    ScRange aTargetRange(maRange);

    // If insertion is for full cols/rows and after the current
    // selection, then shift the range accordingly
    if (meCmd == INS_INSROWS_AFTER)
    {
        ScRange aErrorRange(ScAddress::UNINITIALIZED);
        if (!aTargetRange.Move(0, maRange.aEnd.Row() - maRange.aStart.Row() + 1, 0, aErrorRange,
                               rDoc))
        {
            return false;
        }
    }
    if (meCmd == INS_INSCOLS_AFTER)
    {
        ScRange aErrorRange(ScAddress::UNINITIALIZED);
        if (!aTargetRange.Move(maRange.aEnd.Col() - maRange.aStart.Col() + 1, 0, 0, aErrorRange,
                               rDoc))
        {
            return false;
        }
    }

    SCCOL nStartCol = aTargetRange.aStart.Col();
    SCROW nStartRow = aTargetRange.aStart.Row();
    SCTAB nStartTab = aTargetRange.aStart.Tab();
    SCCOL nEndCol = aTargetRange.aEnd.Col() + mnInsertCount;
    SCROW nEndRow = aTargetRange.aEnd.Row() + mnInsertCount;
    SCTAB nEndTab = aTargetRange.aEnd.Tab();

    if (!rDoc.ValidRow(nStartRow) || !rDoc.ValidRow(nEndRow))
    {
        OSL_FAIL("invalid row in InsertCells");
        return false;
    }

    SCTAB nTabCount = rDoc.GetTableCount();
    SCCOL nPaintStartCol = nStartCol;
    SCROW nPaintStartRow = nStartRow;
    SCCOL nPaintEndCol = nEndCol;
    SCROW nPaintEndRow = nEndRow;
    PaintPartFlags nPaintFlags = PaintPartFlags::Grid;
    bool bSuccess;

    ScTabViewShell* pViewSh = mrDocShell.GetBestViewShell(); //preserve current cursor position
    SCCOL nCursorCol = 0;
    SCROW nCursorRow = 0;
    if (pViewSh)
    {
        nCursorCol = pViewSh->GetViewData().GetCurX();
        nCursorRow = pViewSh->GetViewData().GetCurY();
    }

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

    // Adjust also related scenarios

    SCCOL nMergeTestStartCol = nStartCol;
    SCROW nMergeTestStartRow = nStartRow;
    SCCOL nMergeTestEndCol = nEndCol;
    SCROW nMergeTestEndRow = nEndRow;

    ScRange aExtendMergeRange(aTargetRange);

    if (aTargetRange.aStart == aTargetRange.aEnd
        && rDoc.HasAttrib(aTargetRange, HasAttrFlags::Merged))
    {
        rDoc.ExtendMerge(aExtendMergeRange);
        rDoc.ExtendOverlapped(aExtendMergeRange);
        nMergeTestEndCol = aExtendMergeRange.aEnd.Col();
        nMergeTestEndRow = aExtendMergeRange.aEnd.Row();
        nPaintEndCol = nMergeTestEndCol;
        nPaintEndRow = nMergeTestEndRow;
    }

    if (meCmd == INS_INSROWS_BEFORE || meCmd == INS_INSROWS_AFTER)
    {
        nMergeTestStartCol = 0;
        nMergeTestEndCol = rDoc.MaxCol();
    }
    if (meCmd == INS_INSCOLS_BEFORE || meCmd == INS_INSCOLS_AFTER)
    {
        nMergeTestStartRow = 0;
        nMergeTestEndRow = rDoc.MaxRow();
    }
    if (meCmd == INS_CELLSDOWN)
        nMergeTestEndRow = rDoc.MaxRow();
    if (meCmd == INS_CELLSRIGHT)
        nMergeTestEndCol = rDoc.MaxCol();

    bool bNeedRefresh = false;

    SCCOL nEditTestEndCol = (meCmd == INS_INSCOLS_BEFORE || meCmd == INS_INSCOLS_AFTER)
                                ? rDoc.MaxCol()
                                : nMergeTestEndCol;
    SCROW nEditTestEndRow = (meCmd == INS_INSROWS_BEFORE || meCmd == INS_INSROWS_AFTER)
                                ? rDoc.MaxRow()
                                : nMergeTestEndRow;

    ScEditableTester aTester;
    switch (meCmd)
    {
        case INS_INSCOLS_BEFORE:
            aTester = ScEditableTester::CreateAndTestBlockForAction(
                rDoc, sc::EditAction::InsertColumnsBefore, nMergeTestStartCol, 0, nMergeTestEndCol,
                rDoc.MaxRow(), aMark);
            break;
        case INS_INSCOLS_AFTER:
            aTester = ScEditableTester::CreateAndTestBlockForAction(
                rDoc, sc::EditAction::InsertColumnsAfter, nMergeTestStartCol, 0, nMergeTestEndCol,
                rDoc.MaxRow(), aMark);
            break;
        case INS_INSROWS_BEFORE:
            aTester = ScEditableTester::CreateAndTestBlockForAction(
                rDoc, sc::EditAction::InsertRowsBefore, 0, nMergeTestStartRow, rDoc.MaxCol(),
                nMergeTestEndRow, aMark);
            break;
        case INS_INSROWS_AFTER:
            aTester = ScEditableTester::CreateAndTestBlockForAction(
                rDoc, sc::EditAction::InsertRowsAfter, 0, nMergeTestStartRow, rDoc.MaxCol(),
                nMergeTestEndRow, aMark);
            break;
        default:
            aTester = ScEditableTester::CreateAndTestSelectedBlock(
                rDoc, nMergeTestStartCol, nMergeTestStartRow, nEditTestEndCol, nEditTestEndRow,
                aMark);
            break;
    }

    if (!aTester.IsEditable())
    {
        if (!mbApi)
            mrDocShell.ErrorMessage(aTester.GetMessageId());
        return false;
    }

    // Check if this insertion is allowed with respect to pivot table.
    if (!canInsertCellsByPivot(aTargetRange, aMark, meCmd, rDoc))
    {
        if (!mbApi)
            mrDocShell.ErrorMessage(STR_NO_INSERT_DELETE_OVER_PIVOT_TABLE);
        return false;
    }

    weld::WaitObject aWait(
        ScDocShell::GetActiveDialogParent()); // important due to TrackFormulas at UpdateReference

    ScDocumentUniquePtr pRefUndoDoc;
    std::unique_ptr<ScRefUndoData> pUndoData;
    if (mbRecord)
    {
        pRefUndoDoc.reset(new ScDocument(SCDOCMODE_UNDO));
        pRefUndoDoc->InitUndo(rDoc, 0, nTabCount - 1);

        // pRefUndoDoc is filled in InsertCol / InsertRow

        pUndoData.reset(new ScRefUndoData(&rDoc));

        rDoc.BeginDrawUndo();
    }

    // #i8302 : we unmerge overwhelming ranges, before insertion all the actions are put in the same ListAction
    // the patch comes from mloiseleur and maoyg
    bool bInsertMerge = false;
    std::vector<ScRange> qIncreaseRange;
    OUString aUndo = ScResId(STR_UNDO_INSERTCELLS);
    if (mbRecord)
    {
        ViewShellId nViewShellId(-1);
        if (pViewSh)
            nViewShellId = pViewSh->GetViewShellId();
        mrDocShell.GetUndoManager()->EnterListAction(aUndo, aUndo, 0, nViewShellId);
    }
    std::unique_ptr<ScUndoRemoveMerge> pUndoRemoveMerge;

    for (const SCTAB i : aMark)
    {
        if (i >= nTabCount)
            break;

        if (rDoc.HasAttrib(nMergeTestStartCol, nMergeTestStartRow, i, nMergeTestEndCol,
                           nMergeTestEndRow, i, HasAttrFlags::Merged | HasAttrFlags::Overlapped))
        {
            if (meCmd == INS_CELLSRIGHT)
                bNeedRefresh = true;

            SCCOL nMergeStartCol = nMergeTestStartCol;
            SCROW nMergeStartRow = nMergeTestStartRow;
            SCCOL nMergeEndCol = nMergeTestEndCol;
            SCROW nMergeEndRow = nMergeTestEndRow;

            rDoc.ExtendMerge(nMergeStartCol, nMergeStartRow, nMergeEndCol, nMergeEndRow, i);
            rDoc.ExtendOverlapped(nMergeStartCol, nMergeStartRow, nMergeEndCol, nMergeEndRow, i);

            if ((meCmd == INS_CELLSDOWN
                 && (nMergeStartCol != nMergeTestStartCol || nMergeEndCol != nMergeTestEndCol))
                || (meCmd == INS_CELLSRIGHT
                    && (nMergeStartRow != nMergeTestStartRow || nMergeEndRow != nMergeTestEndRow)))
            {
                if (!mbApi)
                    mrDocShell.ErrorMessage(STR_MSSG_INSERTCELLS_0);
                mrDocShell.GetUndoManager()->LeaveListAction();
                return false;
            }

            SCCOL nTestCol = -1;
            SCROW nTestRow1 = -1;
            SCROW nTestRow2 = -1;

            ScDocAttrIterator aTestIter(rDoc, i, nMergeTestStartCol, nMergeTestStartRow,
                                        nMergeTestEndCol, nMergeTestEndRow);
            ScRange aExtendRange(nMergeTestStartCol, nMergeTestStartRow, i, nMergeTestEndCol,
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
                                qIncreaseRange.push_back(aTestRange);
                                bInsertMerge = true;
                            }
                        }
                    }
                    else
                    {
                        ScRange aMergeRange(aRange.aStart.Col(), aRange.aStart.Row(), i);
                        if (!aExtendRange.Contains(aMergeRange))
                        {
                            qIncreaseRange.push_back(aRange);
                        }
                        bInsertMerge = true;
                    }
                }
            }

            if (bInsertMerge)
            {
                if (meCmd == INS_INSROWS_BEFORE || meCmd == INS_INSROWS_AFTER
                    || meCmd == INS_CELLSDOWN)
                {
                    nStartRow = aExtendMergeRange.aStart.Row();
                    nEndRow = aExtendMergeRange.aEnd.Row();

                    if (meCmd == INS_CELLSDOWN)
                        nEndCol = nMergeTestEndCol;
                    else
                    {
                        nStartCol = 0;
                        nEndCol = rDoc.MaxCol();
                    }
                }
                else if (meCmd == INS_CELLSRIGHT || meCmd == INS_INSCOLS_BEFORE
                         || meCmd == INS_INSCOLS_AFTER)
                {
                    nStartCol = aExtendMergeRange.aStart.Col();
                    nEndCol = aExtendMergeRange.aEnd.Col();
                    if (meCmd == INS_CELLSRIGHT)
                    {
                        nEndRow = nMergeTestEndRow;
                    }
                    else
                    {
                        nStartRow = 0;
                        nEndRow = rDoc.MaxRow();
                    }
                }

                if (!qIncreaseRange.empty())
                {
                    if (mbRecord && !pUndoRemoveMerge)
                    {
                        ScDocumentUniquePtr pUndoDoc(new ScDocument(SCDOCMODE_UNDO));
                        pUndoDoc->InitUndo(rDoc, *aMark.begin(), *aMark.rbegin());
                        pUndoRemoveMerge.reset(
                            new ScUndoRemoveMerge(&mrDocShell, maRange, std::move(pUndoDoc)));
                    }

                    for (const ScRange& aRange : qIncreaseRange)
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
                    mrDocShell.ErrorMessage(STR_MSSG_INSERTCELLS_0);
                mrDocShell.GetUndoManager()->LeaveListAction();
                return false;
            }
        }
    }

    if (mbRecord && pUndoRemoveMerge)
    {
        mrDocShell.GetUndoManager()->AddUndoAction(std::move(pUndoRemoveMerge));
    }

    switch (meCmd)
    {
        case INS_CELLSDOWN:
            bSuccess = rDoc.InsertRow(nStartCol, 0, nEndCol, MAXTAB, nStartRow,
                                      static_cast<SCSIZE>(nEndRow - nStartRow + 1),
                                      pRefUndoDoc.get(), &aFullMark);
            nPaintEndRow = rDoc.MaxRow();
            break;
        case INS_INSROWS_BEFORE:
        case INS_INSROWS_AFTER:
            bSuccess = rDoc.InsertRow(0, 0, rDoc.MaxCol(), MAXTAB, nStartRow,
                                      static_cast<SCSIZE>(nEndRow - nStartRow + 1),
                                      pRefUndoDoc.get(), &aFullMark);
            nPaintStartCol = 0;
            nPaintEndCol = rDoc.MaxCol();
            nPaintEndRow = rDoc.MaxRow();
            nPaintFlags |= PaintPartFlags::Left;
            break;
        case INS_CELLSRIGHT:
            bSuccess = rDoc.InsertCol(nStartRow, 0, nEndRow, MAXTAB, nStartCol,
                                      static_cast<SCSIZE>(nEndCol - nStartCol + 1),
                                      pRefUndoDoc.get(), &aFullMark);
            nPaintEndCol = rDoc.MaxCol();
            break;
        case INS_INSCOLS_BEFORE:
        case INS_INSCOLS_AFTER:
            bSuccess = rDoc.InsertCol(0, 0, rDoc.MaxRow(), MAXTAB, nStartCol,
                                      static_cast<SCSIZE>(nEndCol - nStartCol + 1),
                                      pRefUndoDoc.get(), &aFullMark);
            nPaintStartRow = 0;
            nPaintEndRow = rDoc.MaxRow();
            nPaintEndCol = rDoc.MaxCol();
            nPaintFlags |= PaintPartFlags::Top;
            break;
        default:
            OSL_FAIL("Wrong code at inserting");
            bSuccess = false;
            break;
    }

    if (bSuccess)
    {
        SCTAB nUndoPos = 0;

        if (mbRecord)
        {
            std::unique_ptr<SCTAB[]> pTabs(new SCTAB[nSelCount]);
            std::unique_ptr<SCTAB[]> pScenarios(new SCTAB[nSelCount]);
            nUndoPos = 0;
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

            if (!bInsertMerge)
            {
                mrDocShell.GetUndoManager()->LeaveListAction();
            }

            mrDocShell.GetUndoManager()->AddUndoAction(std::make_unique<ScUndoInsertCells>(
                &mrDocShell, ScRange(nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab),
                nUndoPos, std::move(pTabs), std::move(pScenarios), meCmd, std::move(pRefUndoDoc),
                std::move(pUndoData), mbPartOfPaste));
        }

        // #i8302 : we remerge growing ranges, with the new part inserted

        while (!qIncreaseRange.empty())
        {
            ScRange aRange = qIncreaseRange.back();
            if (!rDoc.HasAttrib(aRange, HasAttrFlags::Overlapped | HasAttrFlags::Merged))
            {
                switch (meCmd)
                {
                    case INS_CELLSDOWN:
                    case INS_INSROWS_BEFORE:
                    case INS_INSROWS_AFTER:
                        aRange.aEnd.IncRow(static_cast<SCCOL>(nEndRow - nStartRow + 1));
                        break;
                    case INS_CELLSRIGHT:
                    case INS_INSCOLS_BEFORE:
                    case INS_INSCOLS_AFTER:
                        aRange.aEnd.IncCol(static_cast<SCCOL>(nEndCol - nStartCol + 1));
                        break;
                    default:
                        break;
                }
                ScCellMergeOption aMergeOption(aRange.aStart.Col(), aRange.aStart.Row(),
                                               aRange.aEnd.Col(), aRange.aEnd.Row());
                aMergeOption.maTabs.insert(aRange.aStart.Tab());
                mrDocShell.GetDocFunc().MergeCells(aMergeOption, false, true, true);
            }
            qIncreaseRange.pop_back();
        }

        if (bInsertMerge)
            mrDocShell.GetUndoManager()->LeaveListAction();

        for (const SCTAB i : aMark)
        {
            if (i >= nTabCount)
                break;

            rDoc.SetDrawPageSize(i);

            if (bNeedRefresh)
                rDoc.ExtendMerge(nMergeTestStartCol, nMergeTestStartRow, nMergeTestEndCol,
                                 nMergeTestEndRow, i, true);
            else
                rDoc.RefreshAutoFilter(nMergeTestStartCol, nMergeTestStartRow, nMergeTestEndCol,
                                       nMergeTestEndRow, i);

            if (meCmd == INS_INSROWS_BEFORE || meCmd == INS_INSCOLS_BEFORE
                || meCmd == INS_INSROWS_AFTER || meCmd == INS_INSCOLS_AFTER)
                rDoc.UpdatePageBreaks(i);

            sal_uInt16 nExtFlags = 0;
            mrDocShell.UpdatePaintExt(nExtFlags, nPaintStartCol, nPaintStartRow, i, nPaintEndCol,
                                      nPaintEndRow, i);

            SCTAB nScenarioCount = 0;

            for (SCTAB j = i + 1; j < nTabCount && rDoc.IsScenario(j); j++)
                nScenarioCount++;

            bool bAdjusted
                = (meCmd == INS_INSROWS_BEFORE || meCmd == INS_INSROWS_AFTER)
                      ? mrDocShell.GetDocFunc().AdjustRowHeight(
                            ScRange(0, nStartRow, i, rDoc.MaxCol(), nEndRow, i + nScenarioCount),
                            true, mbApi)
                      : mrDocShell.GetDocFunc().AdjustRowHeight(ScRange(0, nPaintStartRow, i,
                                                                        rDoc.MaxCol(), nPaintEndRow,
                                                                        i + nScenarioCount),
                                                                true, mbApi);
            if (bAdjusted)
            {
                //  paint only what is not done by AdjustRowHeight
                if (nPaintFlags & PaintPartFlags::Top)
                    mrDocShell.PostPaint(nPaintStartCol, nPaintStartRow, i, nPaintEndCol,
                                         nPaintEndRow, i + nScenarioCount, PaintPartFlags::Top);
            }
            else
                mrDocShell.PostPaint(nPaintStartCol, nPaintStartRow, i, nPaintEndCol, nPaintEndRow,
                                     i + nScenarioCount, nPaintFlags, nExtFlags);
        }
    }
    else
    {
        if (bInsertMerge)
        {
            while (!qIncreaseRange.empty())
            {
                ScRange aRange = qIncreaseRange.back();
                ScCellMergeOption aMergeOption(aRange.aStart.Col(), aRange.aStart.Row(),
                                               aRange.aEnd.Col(), aRange.aEnd.Row());
                mrDocShell.GetDocFunc().MergeCells(aMergeOption, false, true, true);
                qIncreaseRange.pop_back();
            }

            if (pViewSh)
            {
                pViewSh->MarkRange(aTargetRange, false);
                pViewSh->SetCursor(nCursorCol, nCursorRow);
            }
        }

        mrDocShell.GetUndoManager()->LeaveListAction();
        mrDocShell.GetUndoManager()->RemoveLastUndoAction();

        pRefUndoDoc.reset();
        if (!mbApi)
            mrDocShell.ErrorMessage(STR_INSERT_FULL); // column/row full
    }

    // The cursor position needs to be modified earlier than updating
    // any enabled edit view which is triggered by SetDocumentModified below.
    if (bSuccess)
    {
        bool bInsertCols = (meCmd == INS_INSCOLS_BEFORE || meCmd == INS_INSCOLS_AFTER);
        bool bInsertRows = (meCmd == INS_INSROWS_BEFORE || meCmd == INS_INSROWS_AFTER);

        if (bInsertCols)
        {
            pViewSh->OnLOKInsertDeleteColumn(
                maRange.aStart.Col() - (meCmd == INS_INSCOLS_BEFORE ? 1 : 0), 1);
        }

        if (bInsertRows)
        {
            pViewSh->OnLOKInsertDeleteRow(
                maRange.aStart.Row() - (meCmd == INS_INSROWS_BEFORE ? 1 : 0), 1);
        }
    }

    aModificator.SetDocumentModified();

    SfxGetpApp()->Broadcast(SfxHint(SfxHintId::ScAreaLinksChanged));
    return bSuccess;
}

} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
