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
    if (!pSheetView)
        return rAddress;

    ScAddress aAddress = rAddress;
    SCTAB nTab = aAddress.Tab();

    // Change the tab number if it is the one from sheet view
    if (pViewData->CurrentTabForData() == nTab)
        nTab = pViewData->GetTabNumber();
    aAddress.SetTab(nTab);

    std::optional<SortOrderReverser> const& oSortOrder = pSheetView->getSortOrder();
    if (!oSortOrder)
        return aAddress;

    SCCOL nColumn = aAddress.Col();
    SCROW nRow = aAddress.Row();

    if (pViewData->GetTabNumber() != nTab)
        return aAddress;

    SCROW nReversedRow = pSheetView->reverseSortingToDefaultView(nRow, nColumn);

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

    std::optional<SortOrderReverser> const& oSortOrder = pSheetView->getSortOrder();
    if (!oSortOrder)
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
        std::vector<std::pair<SCCOL, SCROW>> aMarkedCells;
        SortOrderInfo const& rSortInfo = oSortOrder->maSortInfo;
        for (SCROW nRow = rSortInfo.mnFirstRow; nRow <= rSortInfo.mnLastRow; ++nRow)
        {
            for (SCROW nColumn = rSortInfo.mnFirstColumn; nColumn <= rSortInfo.mnLastColumn;
                 ++nColumn)
            {
                if (aNewMark.IsCellMarked(nColumn, nRow))
                {
                    ScRange aRange(nColumn, nRow, nDefaultViewTab, nColumn, nRow, nDefaultViewTab);
                    aNewMark.SetMultiMarkArea(aRange, false);
                    aMarkedCells.emplace_back(nColumn, nRow);
                }
            }
        }
        for (auto & [ nColumn, nRow ] : aMarkedCells)
        {
            SCROW nReversedRow = pSheetView->reverseSortingToDefaultView(nRow, nColumn);
            ScRange aRange(nColumn, nReversedRow, nDefaultViewTab, nColumn, nReversedRow,
                           nDefaultViewTab);
            aNewMark.SetMultiMarkArea(aRange, true);
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

bool Operation::checkSheetViewProtection()
{
    sc::SheetViewOperationsTester aSheetViewTester(ScDocShell::GetViewData());
    return aSheetViewTester.check(meType);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
