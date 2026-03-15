/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <SheetViewManager.hxx>
#include <table.hxx>
#include <globstr.hrc>
#include <scresid.hxx>

namespace sc
{
SheetViewManager::SheetViewManager() {}

SheetViewID SheetViewManager::create(ScTable* pSheetViewTable)
{
    SheetViewID nID(maViews.size());
    maViews.emplace_back(std::make_shared<SheetView>(pSheetViewTable, generateName(), nID));
    mnSheetViewCount++;
    return nID;
}

bool SheetViewManager::remove(SheetViewID nID)
{
    if (!isValidSheetViewID(nID))
        return false;

    // It's probably a bug if we want to remove a non-existent sheet view.
    SAL_WARN_IF(!maViews[nID], "sc", "Removing a non-existing sheet view.");

    if (!maViews[nID])
        return false;

    // We only reset the value and not actually remove if from the vector, because the SheetViewID
    // also represents the index in the vector. If we removed the value it would make all the
    // following indices / SheetViewIDs returning the wrong SheetView.
    maViews[nID].reset();
    mnSheetViewCount--;
    return true;
}

void SheetViewManager::removeAll()
{
    maViews.clear();
    mnSheetViewCount = 0;
}

std::shared_ptr<SheetView> SheetViewManager::get(SheetViewID nID) const
{
    if (isValidSheetViewID(nID))
    {
        return maViews[nID];
    }
    return std::shared_ptr<SheetView>();
}

OUString SheetViewManager::generateName()
{
    maNameCounter++;
    OUString aTemplate = ScResId(STR_SHEET_VIEW_TEMPORARY_NAME_TEMPLATE);
    return aTemplate.replaceAll("%1", OUString::number(maNameCounter));
}

OUString SheetViewManager::defaultViewName() { return ScResId(STR_SHEET_VIEW_DEFAULT_VIEW_NAME); }

DefaultViewSortData& SheetViewManager::ensureSortData()
{
    if (!mpSortData)
        mpSortData = std::make_shared<DefaultViewSortData>();
    return *mpSortData;
}

SortOrderReverser const* SheetViewManager::getSortOrder() const
{
    if (!mpSortData || mpSortData->maSortOrder.maSortInfo.maOrder.empty())
        return nullptr;
    return &mpSortData->maSortOrder;
}

void SheetViewManager::addOrderIndices(SortOrderInfo const& rSortInfo)
{
    ensureSortData().maSortOrder.addOrderIndices(rSortInfo);
}

void SheetViewManager::mergeReorderParameters(ReorderParam const& rReorderParameters)
{
    for (auto& rSheetView : iterateValidSheetViews())
    {
        rSheetView.mergeReorderParameters(rReorderParameters);
    }
}

void SheetViewManager::insertedRows(SCROW nStartRow, SCROW nRowCount)
{
    if (mpSortData)
        mpSortData->maSortOrder.insertedRows(nStartRow, nRowCount);

    for (auto& rSheetView : iterateValidSheetViews())
    {
        rSheetView.insertedRows(nStartRow, nRowCount);
    }
}

void SheetViewManager::deletedRows(SCROW nStartRow, SCROW nRowCount)
{
    if (mpSortData)
        mpSortData->maSortOrder.deletedRows(nStartRow, nRowCount);

    for (auto& rSheetView : iterateValidSheetViews())
    {
        rSheetView.deletedRows(nStartRow, nRowCount);
    }
}

void SheetViewManager::insertedColumns(SCCOL nStartCol, SCCOL nColCount)
{
    if (mpSortData)
        mpSortData->maSortOrder.insertedColumns(nStartCol, nColCount);

    for (auto& rSheetView : iterateValidSheetViews())
    {
        rSheetView.insertedColumns(nStartCol, nColCount);
    }
}

void SheetViewManager::deletedColumns(SCCOL nStartCol, SCCOL nColCount)
{
    if (mpSortData)
        mpSortData->maSortOrder.deletedColumns(nStartCol, nColCount);

    for (auto& rSheetView : iterateValidSheetViews())
    {
        rSheetView.deletedColumns(nStartCol, nColCount);
    }
}

std::shared_ptr<DefaultViewSortData> SheetViewManager::captureSortData() const
{
    auto pSortDataCopy = std::make_shared<DefaultViewSortData>();
    if (mpSortData)
        pSortDataCopy->maSortOrder = mpSortData->maSortOrder;

    // Capture each SheetView's full sort data
    for (auto const& rSheetView : iterateValidSheetViews())
    {
        pSortDataCopy->maSheetViewSortData.emplace_back(rSheetView.getID(),
                                                        rSheetView.captureSortData());
    }
    return pSortDataCopy;
}

void SheetViewManager::restoreSortData(std::shared_ptr<DefaultViewSortData> const& pData)
{
    if (!pData)
    {
        mpSortData.reset();
    }
    else
    {
        mpSortData = std::make_shared<DefaultViewSortData>();
        mpSortData->maSortOrder = pData->maSortOrder;
    }

    // Restore each SheetView's full sort data
    if (!pData)
        return;

    for (auto const & [ nID, pSortData ] : pData->maSheetViewSortData)
    {
        if (isValidSheetViewID(nID))
        {
            auto pSheetView = get(nID);
            if (pSheetView)
                pSheetView->restoreSortData(pSortData);
        }
    }
}

} // end sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
