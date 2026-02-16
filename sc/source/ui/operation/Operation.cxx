/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/Operation.hxx>
#include <SheetViewOperationsTester.hxx>
#include <docsh.hxx>
#include <markdata.hxx>
#include <address.hxx>
#include <viewdata.hxx>
#include <SheetViewManager.hxx>
#include <SheetView.hxx>
#include <sal/log.hxx>

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

ScAddress Operation::convertAddress(ScAddress const& rAddress)
{
    ScViewData* pViewData = ScDocShell::GetViewData();

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
    ScViewData* pViewData = ScDocShell::GetViewData();

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
    }

    // If sheet view tab is selected, deselect and select default view tab instead
    if (aNewMark.GetTableSelect(nSheetViewTab))
    {
        aNewMark.SelectTable(nSheetViewTab, false);
        aNewMark.SelectTable(nDefaultViewTab, true);
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
                ScRange const& aSortRange = oReorderParams->maSortRange;
                nRowStart = aSortRange.aStart.Row();
                nRowEnd = aSortRange.aEnd.Row();
                nColumnStart = aSortRange.aStart.Col();
                nColumnEnd = aSortRange.aEnd.Col();
            }

            for (SCROW nRow = nRowStart; nRow <= nRowEnd; ++nRow)
            {
                for (SCROW nColumn = nColumnStart; nColumn <= nColumnEnd; ++nColumn)
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

bool Operation::checkSheetViewProtection()
{
    sc::SheetViewOperationsTester aSheetViewTester(ScDocShell::GetViewData());
    return aSheetViewTester.check(meType);
}

bool Operation::run()
{
    SAL_INFO("sc.op",
             "Running operation '" << SheetViewOperationsTester::operationName(meType) << "'.");
    bool bResult = runImplementation();
    SAL_INFO("sc.op", "Operation '" << SheetViewOperationsTester::operationName(meType)
                                    << (bResult ? "' succeded." : "' failed."));
    return bResult;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
