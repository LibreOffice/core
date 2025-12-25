/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#include "SheetViewTypes.hxx"
#include "SheetView.hxx"
#include "types.hxx"
#include "scdllapi.h"

#include <o3tl/safeint.hxx>
#include <vector>
#include <memory>

class ScTable;

namespace sc
{
/** Manager and the holder of the sheet views for a sheet. */
class SC_DLLPUBLIC SheetViewManager
{
private:
    std::vector<std::shared_ptr<SheetView>> maViews;
    sal_Int32 maNameCounter = 0;

    std::optional<SortOrderReverser> moSortOrder;

    bool isValidSheetViewID(SheetViewID nID) const
    {
        return nID >= 0 && o3tl::make_unsigned(nID) < maViews.size();
    }

    OUString generateName();

public:
    SheetViewManager();

    /** Creates a new sheet view. */
    SheetViewID create(ScTable* pSheetViewTable);

    /** Returns a sheet view for the ID. */
    std::shared_ptr<SheetView> get(SheetViewID nID) const;

    /** True if there are no sheet views. */
    bool isEmpty() const { return maViews.empty(); }

    /** Remove the sheet view with the ID. True if successful. */
    bool remove(SheetViewID nID);

    /** Remove all sheet views. */
    void removeAll();

    /** Return the list of sheet views. */
    std::vector<std::shared_ptr<SheetView>> const& getSheetViews() const { return maViews; }

    /** Calculate the next sheet view ID from the current ID. */
    SheetViewID getNextSheetView(SheetViewID nID);

    /** Calculate the previous sheet view ID from the current ID. */
    SheetViewID getPreviousSheetView(SheetViewID nID);

    void unsyncAllSheetViews();

    static OUString defaultViewName();

    std::optional<SortOrderReverser> const& getSortOrder() const { return moSortOrder; }
    void addOrderIndices(std::vector<SCCOLROW> const& rOrder, SCROW firstRow, SCROW lastRow);
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
