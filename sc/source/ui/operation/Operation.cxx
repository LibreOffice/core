/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/Operation.hxx>
#include <operation/OperationType.hxx>
#include <SheetViewOperationsTester.hxx>
#include <docsh.hxx>
#include <markdata.hxx>
#include <address.hxx>
#include <viewdata.hxx>
#include <SheetViewManager.hxx>
#include <SheetView.hxx>
#include <sal/log.hxx>

#include <dbdata.hxx>
#include <queryparam.hxx>
#include <sortparam.hxx>

namespace sc
{
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
}

Operation::Operation(OperationType eType, bool bRecord, bool bApi)
    : meType(eType)
    , mbApi(bApi)
    , mbRecord(bRecord)
    , mpViewData(ScDocShell::GetViewData())
{
}

ScAddress Operation::convertAddress(ScAddress const& rAddress)
{
    ScViewData* pViewData = mpViewData;

    std::shared_ptr<SheetView> pSheetView = getCurrentSheetView(pViewData);

    // We get a valid pSheetView if we currently are in a sheet view, otherwise we don't need to convert
    if (!pSheetView)
        return rAddress;

    ScAddress aAddress = rAddress;

    // Change the tab number if it is the one from sheet view
    if (aAddress.Tab() == pViewData->CurrentTabForData())
        aAddress.SetTab(pViewData->GetTabNumber());

    // Should be the default view tab
    if (aAddress.Tab() != pViewData->GetTabNumber())
        return aAddress;

    SCCOL nColumn = aAddress.Col();
    SCROW nRow = aAddress.Row();
    SCTAB nTab = aAddress.Tab();

    SCROW nReversedRow = pSheetView->reverseSortingToDefaultView(nRow, nColumn);

    // Check if there was a change
    if (nReversedRow == nRow)
        return aAddress;

    return ScAddress(nColumn, nReversedRow, nTab);
}

ScMarkData Operation::convertMark(ScMarkData const& rMarkData)
{
    ScViewData* pViewData = mpViewData;

    std::shared_ptr<SheetView> pSheetView = getCurrentSheetView(pViewData);

    // We get a valid pSheetView if we currently are in a sheet view, otherwise we don't need to convert
    if (!pSheetView)
        return rMarkData;

    ScMarkData aNewMark(rMarkData);
    aNewMark.MarkToMulti();
    bool bChanged = false;

    SCTAB nDefaultViewTab = pViewData->GetTabNumber();
    SCTAB nSheetViewTab = pViewData->CurrentTabForData();

    // Adjust tab to default view in ranges
    if (nSheetViewTab == aNewMark.GetArea().aStart.Tab()
        && nSheetViewTab == aNewMark.GetArea().aEnd.Tab())
    {
        aNewMark.SetAreaTab(nDefaultViewTab);
        bChanged = true;
    }

    // If sheet view tab is selected, deselect and select default view tab instead
    if (aNewMark.GetTableSelect(nSheetViewTab))
    {
        aNewMark.SelectTable(nSheetViewTab, false);
        aNewMark.SelectTable(nDefaultViewTab, true);
        bChanged = true;
    }

    // Take sorting into account when we convert to default view
    if (aNewMark.GetTableSelect(nDefaultViewTab))
    {
        std::optional<SortOrderReverser> const& oSortOrder = pSheetView->getSortOrder();
        std::optional<ReorderParam> const& oReorderParams = pSheetView->getReorderParameters();
        if (oSortOrder || oReorderParams)
        {
            std::vector<std::pair<SCCOL, SCROW>> aMarkedCells;

            SCROW nRowStart = -1;
            SCROW nRowEnd = -1;
            SCCOL nColumnStart = -1;
            SCCOL nColumnEnd = -1;

            if (oSortOrder)
            {
                SortOrderInfo const& rSortInfo = oSortOrder->maSortInfo;
                nRowStart = rSortInfo.mnFirstRow;
                nRowEnd = rSortInfo.mnLastRow;
                nColumnStart = rSortInfo.mnFirstColumn;
                nColumnEnd = rSortInfo.mnLastColumn;
            }
            else
            {
                ScRange const& rSortRange = oReorderParams->maSortRange;
                nRowStart = rSortRange.aStart.Row();
                nRowEnd = rSortRange.aEnd.Row();
                nColumnStart = rSortRange.aStart.Col();
                nColumnEnd = rSortRange.aEnd.Col();
            }

            for (SCROW nRow = nRowStart; nRow <= nRowEnd; ++nRow)
            {
                for (SCCOL nColumn = nColumnStart; nColumn <= nColumnEnd; ++nColumn)
                {
                    if (aNewMark.IsCellMarked(nColumn, nRow))
                    {
                        // unmark the cell here, but remember which cell it was
                        ScRange aRange(nColumn, nRow, nDefaultViewTab, nColumn, nRow,
                                       nDefaultViewTab);
                        aNewMark.SetMultiMarkArea(aRange, false);
                        aMarkedCells.emplace_back(nColumn, nRow);
                    }
                }
            }
            for (auto & [ nColumn, nRow ] : aMarkedCells)
            {
                // reverse the row and mark the cell again
                SCROW nReversedRow = pSheetView->reverseSortingToDefaultView(nRow, nColumn);
                ScRange aRange(nColumn, nReversedRow, nDefaultViewTab, nColumn, nReversedRow,
                               nDefaultViewTab);
                aNewMark.SetMultiMarkArea(aRange, true);
                bChanged = true;
            }
        }
    }

    if (!bChanged)
        return rMarkData;

    if (bChanged && !aNewMark.HasAnyMultiMarks())
        aNewMark.ResetMark();

    aNewMark.MarkToSimple();

    return aNewMark;
}

void Operation::syncSheetViews()
{
    if (!mpViewData)
        return;

    auto& rDocument = mpViewData->GetDocument();
    SCTAB nTab = mpViewData->GetTabNumber();

    if (rDocument.IsSheetViewHolder(nTab))
        return;

    std::shared_ptr<sc::SheetViewManager> pManager = rDocument.GetSheetViewManager(nTab);
    if (!pManager || pManager->isEmpty())
        return;

    for (auto& rSheetView : pManager->iterateValidSheetViews())
    {
        SCTAB nSheetViewTab = rSheetView.getTableNumber();

        std::optional<ScQueryParam> oQueryParam;
        ScDBData* pNoNameData = rDocument.GetAnonymousDBData(nSheetViewTab);
        if (pNoNameData && pNoNameData->HasAutoFilter())
        {
            if (pNoNameData->HasQueryParam())
            {
                oQueryParam.emplace();
                pNoNameData->GetQueryParam(*oQueryParam);
            }
        }

        rDocument.OverwriteContent(nTab, nSheetViewTab);

        // Reverse the sorting of the default view in the sheet view
        if (auto const& rReorderParameters = rSheetView.getReorderParameters())
        {
            sc::ReorderParam aReorderParameters(*rReorderParameters);
            aReorderParameters.maSortRange.aStart.SetTab(nSheetViewTab);
            aReorderParameters.maSortRange.aEnd.SetTab(nSheetViewTab);
            aReorderParameters.reverse();
            rDocument.Reorder(aReorderParameters);
        }

        auto const& oSortParam = rSheetView.getSortParam();
        if (oSortParam)
        {
            // We need to reset the sort order for the sheet view as we will sort again
            rSheetView.resetSortOrder();
            rDocument.Sort(nSheetViewTab, *oSortParam, false, false, nullptr, nullptr);
        }

        if (oQueryParam)
            rDocument.Query(nSheetViewTab, *oQueryParam, false, false);
    }
}

bool Operation::checkSheetViewProtection()
{
    sc::SheetViewOperationsTester aSheetViewTester(mpViewData);
    return aSheetViewTester.check(meType);
}

bool Operation::run()
{
    SAL_INFO("sc.op", "Running operation '" << operationTypeString(meType) << "'.");
    bool bResult = runImplementation();
    SAL_INFO("sc.op", "Operation '" << operationTypeString(meType)
                                    << (bResult ? "' succeeded." : "' failed."));
    return bResult;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
