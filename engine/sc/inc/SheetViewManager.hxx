/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#include <iterator>

class ScTable;

namespace sc
{
/** Captured sort data for a default view.*/
struct SC_DLLPUBLIC DefaultViewSortData
{
    SortOrderReverser maSortOrder;
    /// Per sheet view sort data - only populated in captured snapshots.
    std::vector<std::pair<SheetViewID, std::shared_ptr<SheetViewSortData>>> maSheetViewSortData;
};

/** Manager and the holder of the sheet views for a sheet. */
class SC_DLLPUBLIC SheetViewManager
{
private:
    std::vector<std::shared_ptr<SheetView>> maViews;
    size_t mnSheetViewCount = 0;
    sal_Int32 maNameCounter = 0;

    /** Sort data for a default view. Null when no sort. */
    std::shared_ptr<DefaultViewSortData> mpSortData;

    /** Ensure sort data is allocated. */
    DefaultViewSortData& ensureSortData();

    bool isValidSheetViewID(SheetViewID nID) const
    {
        return nID >= 0 && o3tl::make_unsigned(nID) < maViews.size();
    }

    OUString generateName();

    /** Forward iterator over valid (non-null) sheet views only. */
    class SheetViewIterator
    {
    private:
        using VectorIterator = std::vector<std::shared_ptr<SheetView>>::const_iterator;
        VectorIterator maCurrentIt;
        VectorIterator maEndIt;

        void skipEmpty()
        {
            while (maCurrentIt != maEndIt && !(*maCurrentIt))
            {
                maCurrentIt++;
            }
        }

    public:
        SheetViewIterator(VectorIterator aBegin, VectorIterator aEnd)
            : maCurrentIt(aBegin)
            , maEndIt(aEnd)
        {
            skipEmpty();
        }

        SheetView& operator*() const { return **maCurrentIt; }

        SheetViewIterator& operator++()
        {
            maCurrentIt++;
            skipEmpty();
            return *this;
        }

        bool operator==(SheetViewIterator const& rOther) const
        {
            return maCurrentIt == rOther.maCurrentIt;
        }
    };

    /** Wrapper for SheetViewIterator so it is possible to use it with range-for loops. */
    class SheetViewRange
    {
        std::vector<std::shared_ptr<SheetView>> const& mrViews;

    public:
        SheetViewRange(std::vector<std::shared_ptr<SheetView>> const& rViews)
            : mrViews(rViews)
        {
        }

        SheetViewIterator begin() const
        {
            return SheetViewIterator(mrViews.begin(), mrViews.end());
        }

        SheetViewIterator end() const { return SheetViewIterator(mrViews.end(), mrViews.end()); }
    };

public:
    SheetViewManager();

    /** Creates a new sheet view. */
    SheetViewID create(ScTable* pSheetViewTable);

    /** Returns a sheet view for the ID. */
    std::shared_ptr<SheetView> get(SheetViewID nID) const;

    /** True if there are no sheet views. */
    bool isEmpty() const { return mnSheetViewCount == 0; }

    /** Number of sheet views. */
    size_t size() const { return mnSheetViewCount; }

    /** Remove the sheet view with the ID. True if successful. */
    bool remove(SheetViewID nID);

    /** Remove all sheet views. */
    void removeAll();

    /** Returns a range that iterates only over valid (non-null) sheet views. */
    SheetViewRange iterateValidSheetViews() const { return SheetViewRange(maViews); }

    static OUString defaultViewName();

    SortOrderReverser const* getSortOrder() const;
    void addOrderIndices(SortOrderInfo const& rSortInfo);

    void mergeReorderParameters(ReorderParam const& rReorderParameters);

    /** Update when rows are inserted. */
    void insertedRows(SCROW nStartRow, SCROW nRowCount);

    /** Update when rows are deleted. */
    void deletedRows(SCROW nStartRow, SCROW nRowCount);

    /** Update when columns are inserted. */
    void insertedColumns(SCCOL nStartCol, SCCOL nColCount);

    /** Update when columns are deleted. */
    void deletedColumns(SCCOL nStartCol, SCCOL nColCount);

    /** Capture sort data (deep copy).*/
    std::shared_ptr<DefaultViewSortData> captureSortData() const;

    /** Restore sort data from a previous capture. */
    void restoreSortData(std::shared_ptr<DefaultViewSortData> const& pData);
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
