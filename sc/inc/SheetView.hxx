/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "scdllapi.h"
#include "types.hxx"
#include <rtl/ustring.hxx>
#include "SheetViewTypes.hxx"

class ScTable;

namespace sc
{
/** Stores information of a sheet view.
 *
 * A sheet view is a special view of a sheet that can be filtered and sorted
 * independently from other views of the sheet.
 **/
class SC_DLLPUBLIC SheetView
{
private:
    ScTable* mpTable = nullptr;
    bool mbSynced = true;
    OUString maName;

    SCROW mnFirstRow;
    SCROW mnLastRow;
    std::vector<SCCOLROW> maOrder;
    SheetViewID mnID;

public:
    SheetView(ScTable* pTable, OUString const& rName, SheetViewID nID);

    ScTable* getTablePointer() const;
    SCTAB getTableNumber() const;
    SheetViewID getID() const { return mnID; }

    OUString const& GetName() { return maName; }

    /** A sheet view is valid if the pointer to the table is set */
    bool isValid() const;

    /** Mark the sheet view as unsynced (not synced). */
    void unsync() { mbSynced = false; }

    /** Is the sheet view synced with its default view. */
    bool isSynced() const { return mbSynced; }

    void addOrderIndices(std::vector<SCCOLROW> const& rOrder, SCROW firstRow, SCROW lastRow);
    SCROW unsort(SCROW nRow) const;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
