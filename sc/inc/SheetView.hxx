/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#include <vector>
#include "SheetViewTypes.hxx"
#include "types.hxx"

class ScTable;

namespace sc
{
/** Stores information of a sheet view.
 *
 * A sheet view is a special view of a sheet that can be filtered and sorted
 * independently from other views of the sheet.
 **/
class SheetView
{
private:
    ScTable* mpTable = nullptr;

public:
    SheetView() = default;
    SheetView(ScTable* pTable);

    ScTable* getTablePointer() const;
    SCTAB getTableNumber() const;

    /** A sheet view is valid if the pointer to the table is set */
    bool isValid() const;
};

/** Manager and the holder of the sheet views for a sheet. */
class SheetViewManager
{
private:
    std::vector<SheetView> maViews;

public:
    SheetViewManager();

    /** Creates a new sheet view. */
    SheetViewID create(ScTable* pSheetViewTable);

    /** Returns a sheet view for the ID. */
    SheetView get(SheetViewID nID) const;
    bool isEmpty() const { return maViews.empty(); }
    bool remove(SheetViewID nID);
    std::vector<SheetView> const& getSheetViews() const { return maViews; }
    SheetViewID getNextSheetView(SheetViewID nID);
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
