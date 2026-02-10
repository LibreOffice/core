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
#include "sortparam.hxx"
#include <optional>

class ScTable;

namespace sc
{
/** Contains the information of an performed sort action */
struct SC_DLLPUBLIC SortOrderInfo
{
    SCCOL mnFirstColumn = -1;
    SCCOL mnLastColumn = -1;
    SCROW mnFirstRow = -1;
    SCROW mnLastRow = -1;
    std::vector<SCCOLROW> maOrder; // Remember the sort order
    std::vector<ScSortKeyState> maKeyStates; // Each column sort information
};

/** Stores the sort order and can reverse the sorting of rows (unsorting). */
struct SC_DLLPUBLIC SortOrderReverser
{
public:
    SortOrderInfo maSortInfo;

    /** Reverse the sort for the input row and output the unsorted row.
     *
     * Uses the sort order. The row must be between range of [first row, last row]
     * or it will return the input row without modification.
     **/
    SCROW unsort(SCROW nRow, SCCOL nColumn) const;
    SCROW resort(SCROW nRow, SCCOL nColumn) const;

    /** Adds or combines the order indices.
     *
     * Adds the indices if none are present, or combines the indices if the order indices
     * were already added previously.
     **/
    void addOrderIndices(SortOrderInfo const& rSortInfo);
};

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

    std::optional<SortOrderReverser> moSortOrder;
    SheetViewID mnID;

    std::optional<ReorderParam> moOriginalReorderParams;
    std::optional<ScSortParam> moSortParam;

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

    std::optional<SortOrderReverser> const& getSortOrder() const { return moSortOrder; }
    void resetSortOrder() { moSortOrder.reset(); }

    /** Adds or combines the order indices.
     *
     * Adds the indices if none are present, or combines the indices if the order indices
     * were already added previously.
     **/
    void addOrderIndices(SortOrderInfo const& rSortInfo);

    /** Merges the reorder parameters */
    void mergeReorderParameters(ReorderParam const& rReorderParameters);
    std::optional<ReorderParam> const& getReorderParameters() { return moOriginalReorderParams; }

    /** Reverses the complete (sheet view and default view) sorting order for the input row */
    SCROW reverseSortingToDefaultView(SCROW nRow, SCCOL nColumn) const;

    // Last used sort parameters

    std::optional<ScSortParam> const& getSortPram() { return moSortParam; }

    /// Remember last used sort parameters when sheet view was sorted.
    void setSortParam(ScSortParam const& rSortParam) { moSortParam = rSortParam; }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
