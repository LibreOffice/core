/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/DeleteCellOperation.hxx>

#include <docfuncutil.hxx>
#include <docfunc.hxx>
#include <markdata.hxx>
#include <editable.hxx>
#include <address.hxx>
#include <viewdata.hxx>
#include <SheetViewManager.hxx>
#include <SheetView.hxx>

#include <memory>

namespace sc
{
DeleteCellOperation::DeleteCellOperation(ScDocFunc& rDocFunc, ScDocShell& rDocShell,
                                         ScAddress const& rPosition, const ScMarkData& rMark,
                                         InsertDeleteFlags nFlags, bool bRecord, bool bApi)
    : Operation(OperationType::DeleteCell, bRecord, bApi)
    , mrDocFunc(rDocFunc)
    , mrDocShell(rDocShell)
    , mrPosition(rPosition)
    , mrMark(rMark)
    , mnFlags(nFlags)
{
}

namespace
{
std::shared_ptr<SheetView> getCurrentSheetView(ScViewData* pViewData)
{
    if (!pViewData)
        return nullptr;

    sc::SheetViewID nID = pViewData->GetSheetViewID();
    if (nID == sc::DefaultSheetViewID)
        return nullptr;

    std::shared_ptr<sc::SheetViewManager> pSheetViewManager
        = pViewData->GetCurrentSheetViewManager();
    if (!pSheetViewManager)
        return nullptr;

    return pSheetViewManager->get(nID);
}

/** Convert address from a sheet view to the address in default view, take sorting into account. */
ScAddress convertAddress(ScAddress const& rAddress)
{
    ScViewData* pViewData = ScDocShell::GetViewData();

    std::shared_ptr<SheetView> pSheetView = getCurrentSheetView(pViewData);
    if (!pSheetView)
        return rAddress;

    std::optional<SortOrderReverser> const& oSortOrder = pSheetView->getSortOrder();
    if (!oSortOrder)
        return rAddress;

    SCTAB nTab = rAddress.Tab();
    SCCOL nColumn = rAddress.Col();
    SCROW nRow = rAddress.Row();

    if (pViewData->GetTabNumber() != nTab)
        return rAddress;

    SCROW nUnsortedRow = oSortOrder->unsort(nRow, nColumn);
    if (nUnsortedRow != nRow)
    {
        return ScAddress(nColumn, nUnsortedRow, nTab);
    }
    return rAddress;
}

/** Convert a mark from a sheet view to the mark in default view, take sorting into account. */
ScMarkData convertMark(ScMarkData const& rMarkData)
{
    ScViewData* pViewData = ScDocShell::GetViewData();

    std::shared_ptr<SheetView> pSheetView = getCurrentSheetView(pViewData);
    if (!pSheetView)
        return rMarkData;

    std::optional<SortOrderReverser> const& oSortOrder = pSheetView->getSortOrder();
    if (!oSortOrder)
        return rMarkData;

    ScMarkData aNewMark(rMarkData);
    aNewMark.MarkToMulti();
    bool bChanged = false;

    for (const SCTAB& nTab : aNewMark)
    {
        if (pViewData->GetTabNumber() != nTab)
            continue;

        std::vector<std::pair<SCCOL, SCROW>> aMarkedCells;
        SortOrderInfo const& rSortInfo = oSortOrder->maSortInfo;
        for (SCROW nRow = rSortInfo.mnFirstRow; nRow <= rSortInfo.mnLastRow; ++nRow)
        {
            for (SCROW nColumn = rSortInfo.mnFirstColumn; nColumn <= rSortInfo.mnLastColumn;
                 ++nColumn)
            {
                if (aNewMark.IsCellMarked(nRow, nColumn))
                {
                    aNewMark.SetMultiMarkArea(ScRange(nColumn, nRow, nTab, nColumn, nRow, nTab),
                                              false);
                    aMarkedCells.emplace_back(nColumn, nRow);
                }
            }
        }
        for (auto & [ nColumn, nRow ] : aMarkedCells)
        {
            SCROW nUnsortedRow = oSortOrder->unsort(nRow, nColumn);
            aNewMark.SetMultiMarkArea(
                ScRange(nColumn, nUnsortedRow, nTab, nColumn, nUnsortedRow, nTab), true);
            bChanged = true;
        }
    }

    if (!bChanged)
        return rMarkData;

    if (bChanged && !aNewMark.HasAnyMultiMarks())
        aNewMark.ResetMark();

    aNewMark.MarkToSimple();

    return aNewMark;
}
}

bool DeleteCellOperation::runImplementation()
{
    ScDocShellModificator aModificator(mrDocShell);

    ScDocument& rDoc = mrDocShell.GetDocument();

    if (mbRecord && !rDoc.IsUndoEnabled())
        mbRecord = false;

    sc::SheetViewOperationsTester aSheetViewTester(ScDocShell::GetViewData());

    // Convert taking sheet view sorting into account
    ScAddress aPosition = convertAddress(mrPosition);
    ScMarkData aMarkData = convertMark(mrMark);

    ScEditableTester aTester = ScEditableTester::CreateAndTestSelectedBlock(
        rDoc, aPosition.Col(), aPosition.Row(), aPosition.Col(), aPosition.Row(), aMarkData);
    if (!aTester.IsEditable())
    {
        mrDocShell.ErrorMessage(aTester.GetMessageId());
        return false;
    }

    // no objects on protected tabs
    bool bObjects = (mnFlags & InsertDeleteFlags::OBJECTS)
                    && !sc::DocFuncUtil::hasProtectedTab(rDoc, aMarkData);

    sal_uInt16 nExtFlags = 0; // extra flags are needed only if attributes are deleted
    if (mnFlags & InsertDeleteFlags::ATTRIB)
        mrDocShell.UpdatePaintExt(nExtFlags, ScRange(aPosition));

    //  order of operations:
    //  1) BeginDrawUndo
    //  2) delete objects (DrawUndo is filled)
    //  3) copy contents for undo
    //  4) delete contents
    //  5) add undo-action

    bool bDrawUndo = bObjects || (mnFlags & InsertDeleteFlags::NOTE); // needed for shown notes
    if (bDrawUndo && mbRecord)
        rDoc.BeginDrawUndo();

    if (bObjects)
        rDoc.DeleteObjectsInArea(aPosition.Col(), aPosition.Row(), aPosition.Col(), aPosition.Row(),
                                 aMarkData);

    // To keep track of all non-empty cells within the deleted area.
    std::shared_ptr<ScSimpleUndo::DataSpansType> pDataSpans;

    ScDocumentUniquePtr pUndoDoc;
    if (mbRecord)
    {
        pUndoDoc = sc::DocFuncUtil::createDeleteContentsUndoDoc(rDoc, aMarkData, ScRange(aPosition),
                                                                mnFlags, false);
        pDataSpans = sc::DocFuncUtil::getNonEmptyCellSpans(rDoc, aMarkData, ScRange(aPosition));
    }

    tools::Long nBefore(mrDocShell.GetTwipWidthHint(aPosition));
    rDoc.DeleteArea(aPosition.Col(), aPosition.Row(), aPosition.Col(), aPosition.Row(), aMarkData,
                    mnFlags);

    if (mbRecord)
    {
        sc::DocFuncUtil::addDeleteContentsUndo(mrDocShell.GetUndoManager(), &mrDocShell, aMarkData,
                                               ScRange(aPosition), std::move(pUndoDoc), mnFlags,
                                               pDataSpans, false, bDrawUndo);
    }

    if (!mrDocFunc.AdjustRowHeight(ScRange(aPosition), true, mbApi))
        mrDocShell.PostPaint(aPosition.Col(), aPosition.Row(), aPosition.Tab(), aPosition.Col(),
                             aPosition.Row(), aPosition.Tab(), PaintPartFlags::Grid, nExtFlags,
                             nBefore);

    if (sc::SheetViewOperationsTester::doesUnsync(meType))
        aSheetViewTester.sync();

    aModificator.SetDocumentModified();

    return true;
}
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
