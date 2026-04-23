/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "scdllapi.h"
#include "types.hxx"
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include "SheetViewTypes.hxx"
#include "sortparam.hxx"
#include <optional>
#include <memory>

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

    /** Update data when rows are inserted. */
    void insertedRows(SCROW nStartRow, SCROW nRowCount);

    /** Update data when rows are deleted. */
    void deletedRows(SCROW nStartRow, SCROW nRowCount);

    /** Update data when columns are inserted. */
    void insertedColumns(SCCOL nStartCol, SCCOL nColCount);

    /** Update data when columns are deleted. */
    void deletedColumns(SCCOL nStartCol, SCCOL nColCount);
};

/** Sort data holder. */
struct SC_DLLPUBLIC SheetViewSortData
{
    SortOrderReverser maSortOrder;
    ReorderParam maOriginalReorderParams;
    ScSortParam maSortParam;
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
    OUString maName;
    SheetViewID mnID;

    // GUIDs
    OString maGUID;
    OString maFilterGUID;

    /** Sort data - nullptr when no sort has been performed. */
    std::shared_ptr<SheetViewSortData> mpSortData;

    /** Ensure sort data is allocated. */
    SheetViewSortData& ensureSortData();

    void adjustReorderParamsForInsertRows(SCROW nStartRow, SCROW nRowCount);
    void adjustReorderParamsForInsertColumns(SCCOL nStartCol, SCCOL nColCount);
    void adjustReorderParamsForDeleteRows(SCROW nStartRow, SCROW nRowCount);
    void adjustReorderParamsForDeleteColumns(SCCOL nStartCol, SCCOL nColCount);

    void adjustSortParamForInsertRows(SCROW nStartRow, SCROW nRowCount);
    void adjustSortParamForInsertColumns(SCCOL nStartCol, SCCOL nColCount);
    void adjustSortParamForDeleteRows(SCROW nStartRow, SCROW nRowCount);
    void adjustSortParamForDeleteColumns(SCCOL nStartCol, SCCOL nColCount);

public:
    SheetView(ScTable* pTable, OUString const& rName, SheetViewID nID);

    ScTable* getTablePointer() const;
    SCTAB getTableNumber() const;

    SheetViewID getID() const { return mnID; }

    OUString const& GetName() const { return maName; }
    void SetName(OUString const& rName) { maName = rName; }

    OString const& GetGUID() const { return maGUID; }
    void SetGUID(OString const& rGUID) { maGUID = rGUID; }

    OString const& GetFilterGUID() const { return maFilterGUID; }
    void SetFilterGUID(OString const& rGUID) { maFilterGUID = rGUID; }

    /** A sheet view is valid if the pointer to the table is set */
    bool isValid() const;

    SortOrderReverser const* getSortOrder() const;
    void resetSortOrder();

    /** Adds or combines the order indices.
     *
     * Adds the indices if none are present, or combines the indices if the order indices
     * were already added previously.
     **/
    void addOrderIndices(SortOrderInfo const& rSortInfo);

    /** Merges the reorder parameters */
    void mergeReorderParameters(ReorderParam const& rReorderParameters);
    ReorderParam const* getReorderParameters() const;
    void restoreReorderParameters(ReorderParam const& rParams);

    /** Reverses the complete (sheet view and default view) sorting order for the input row */
    SCROW reverseSortingToDefaultView(SCROW nRow, SCCOL nColumn) const;

    /** Converts a row from the default view to the row in this sheet view, taking sorting data into account. */
    SCROW reverseDefaultViewToSheetView(SCROW nRow, SCCOL nColumn) const;

    /** Update stored sort ranges when rows are inserted. */
    void insertedRows(SCROW nStartRow, SCROW nRowCount);

    /** Update stored sort ranges when rows are deleted. */
    void deletedRows(SCROW nStartRow, SCROW nRowCount);

    /** Update stored sort ranges when columns are inserted. */
    void insertedColumns(SCCOL nStartCol, SCCOL nColCount);

    /** Update stored sort ranges when columns are deleted. */
    void deletedColumns(SCCOL nStartCol, SCCOL nColCount);

    /** Last used sort parameters */
    ScSortParam const* getSortParam() const;

    /** Remember last used sort parameters when sheet view was sorted. */
    void setSortParam(ScSortParam const& rSortParam);

    /** Reset all sort data. */
    void resetSortData();

    /** Capture current sort state (and deep copy). */
    std::shared_ptr<SheetViewSortData> captureSortData() const;

    /** Restore sort state from a previous capture. */
    void restoreSortData(std::shared_ptr<SheetViewSortData> const& pData);
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
