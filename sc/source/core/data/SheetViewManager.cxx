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

SheetView SheetViewManager::get(SheetViewID nID)
{
    if (nID >= 0 && o3tl::make_unsigned(nID) < maViews.size())
    {
        if (maViews[nID].isValid())
            return maViews[nID];
    }
    return SheetView();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
