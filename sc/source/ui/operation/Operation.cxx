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

ScMarkData Operation::convertMark(ScMarkData const& rMarkData)
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
                if (aNewMark.IsCellMarked(nColumn, nRow))
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

bool Operation::checkSheetViewProtection()
{
    sc::SheetViewOperationsTester aSheetViewTester(ScDocShell::GetViewData());
    return aSheetViewTester.check(meType);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
