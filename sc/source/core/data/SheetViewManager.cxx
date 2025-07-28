/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <SheetView.hxx>
#include <table.hxx>

namespace sc
{
SheetViewManager::SheetViewManager() {}

SheetViewID SheetViewManager::create(ScTable* pSheetViewTable)
{
    sal_Int32 nID = maViews.size();
    maViews.emplace_back(pSheetViewTable);
    return SheetViewID(nID);
}

bool SheetViewManager::remove(SheetViewID nID)
{
    if (nID >= 0 && o3tl::make_unsigned(nID) < maViews.size())
    {
        maViews.erase(maViews.begin() + nID);
    }
    return false;
}

SheetView SheetViewManager::get(SheetViewID nID) const
{
    if (nID >= 0 && o3tl::make_unsigned(nID) < maViews.size())
    {
        if (maViews[nID].isValid())
            return maViews[nID];
    }
    return SheetView();
}

/// Calculate the next existing sheet view to use.
SheetViewID SheetViewManager::getNextSheetView(SheetViewID nID)
{
    if (maViews.empty())
        return DefaultSheetViewID;

    // Set to max, so we prevent the for loop to run
    size_t startIndex = std::numeric_limits<size_t>::max();

    // Start with first index and search for the first sheet view in for loop.
    if (nID == DefaultSheetViewID)
    {
        startIndex = 0;
    }
    // If we assume currnet ID is valid, so set the start to current + 1 to search
    // for then next valid sheet view in the for loop.
    else if (nID >= 0 && o3tl::make_unsigned(nID) < maViews.size())
    {
        startIndex = size_t(nID) + 1;
    }

    for (size_t nIndex = startIndex; nIndex < maViews.size(); ++nIndex)
    {
        if (maViews[nIndex].isValid())
            return SheetViewID(nIndex);
    }

    return DefaultSheetViewID;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
