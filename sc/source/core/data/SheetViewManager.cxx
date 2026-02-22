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
    return nID;
}

bool SheetViewManager::remove(SheetViewID nID)
{
    if (isValidSheetViewID(nID))
    {
        // We only reset the value and not actually remove if from the vector, because the SheetViewID
        // also represents the index in the vector. If we removed the value it would make all the
        // following indices / SheetViewIDs returning the wrong SheetView.
        maViews[nID].reset();
        return true;
    }
    return false;
}

void SheetViewManager::removeAll() { maViews.clear(); }

std::shared_ptr<SheetView> SheetViewManager::get(SheetViewID nID) const
{
    if (isValidSheetViewID(nID))
    {
        return maViews[nID];
    }
    return std::shared_ptr<SheetView>();
}

/// Calculate the next existing sheet view to use.
SheetViewID SheetViewManager::getNextSheetView(SheetViewID nID)
{
    if (nID != DefaultSheetViewID && nID < 0)
        return InvalidSheetViewID;

    if (maViews.empty())
        return DefaultSheetViewID;

    // Set to max, so we prevent the for loop to run
    sal_Int32 startIndex = std::numeric_limits<sal_Int32>::max();

    // Start with first index and search for the first sheet view in for loop.
    if (nID == DefaultSheetViewID)
    {
        startIndex = 0;
    }
    // If we assume current ID is valid, so set the start to current + 1 to search
    // for then next valid sheet view in the for loop.
    else if (isValidSheetViewID(nID))
    {
        startIndex = sal_Int32(nID) + 1;
    }

    for (sal_Int32 nIndex = startIndex; nIndex < sal_Int32(maViews.size()); ++nIndex)
    {
        if (maViews[nIndex])
            return SheetViewID(nIndex);
    }

    return DefaultSheetViewID;
}

/// Calculate the previous existing sheet view to use.
SheetViewID SheetViewManager::getPreviousSheetView(SheetViewID nID)
{
    if (nID != DefaultSheetViewID && nID < 0)
        return InvalidSheetViewID;

    if (maViews.empty())
        return DefaultSheetViewID;

    // Set to -1, so we prevent the for loop to run
    sal_Int32 startIndex = -1;

    // Start with first index and search for the first sheet view in for loop.
    if (nID == DefaultSheetViewID)
    {
        startIndex = sal_Int32(maViews.size()) - 1;
    }
    // If we assume current ID is valid, so set the start to current + 1 to search
    // for then next valid sheet view in the for loop.
    else if (isValidSheetViewID(nID))
    {
        startIndex = sal_Int32(nID) - 1;
    }

    for (sal_Int32 nIndex = startIndex; nIndex >= 0; --nIndex)
    {
        if (maViews[nIndex])
            return SheetViewID(nIndex);
    }

    return DefaultSheetViewID;
}

void SheetViewManager::unsyncAllSheetViews()
{
    for (auto const& pSheetView : maViews)
    {
        if (!pSheetView)
        {
            continue;
        }

        pSheetView->unsync();
    }
}

OUString SheetViewManager::generateName()
{
    maNameCounter++;
    OUString aTemplate = ScResId(STR_SHEET_VIEW_TEMPORARY_NAME_TEMPLATE);
    return aTemplate.replaceAll("%1", OUString::number(maNameCounter));
}

OUString SheetViewManager::defaultViewName() { return ScResId(STR_SHEET_VIEW_DEFAULT_VIEW_NAME); }

void SheetViewManager::addOrderIndices(std::vector<SCCOLROW> const& rOrder, SCROW nFirstRow,
                                       SCROW nLastRow)
{
    if (!moSortOrder)
        moSortOrder.emplace();
    moSortOrder->addOrderIndices(rOrder, nFirstRow, nLastRow);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
