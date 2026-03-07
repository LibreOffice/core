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

void SheetViewManager::addOrderIndices(SortOrderInfo const& rSortInfo)
{
    if (!moSortOrder)
        moSortOrder.emplace();
    moSortOrder->addOrderIndices(rSortInfo);
}

void SheetViewManager::mergeReorderParameters(ReorderParam const& rReorderParameters)
{
    for (auto& rSheetView : iterateValidSheetViews())
    {
        rSheetView.mergeReorderParameters(rReorderParameters);
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
