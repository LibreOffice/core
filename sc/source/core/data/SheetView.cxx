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
namespace
{
/** Merge sort order arrays, so the order is the same as it would be by doing multiple independent sort operation */
std::vector<SCCOLROW> mergeOrder(std::vector<SCCOLROW> const& rExistingOrder,
                                 std::vector<SCCOLROW> const& rAddedOrder)
{
    size_t nOrderSize = rExistingOrder.size();
    assert(nOrderSize == rAddedOrder.size());

    std::vector<SCCOLROW> aNewOrder(nOrderSize);
    for (size_t nIndex = 0; nIndex < nOrderSize; ++nIndex)
    {
        size_t nSortedIndex = rAddedOrder[nIndex] - 1;
        aNewOrder[nIndex] = rExistingOrder[nSortedIndex];
    }
    return aNewOrder;
}

/** Adjust order indices when rows are inserted within a sorted range. */
void expandOrderIndices(std::vector<SCCOLROW>& rOrder, SCROW nStartRow, SCROW nFirstRow,
                        SCROW nCount)
{
    SCCOLROW nOffset = nStartRow - nFirstRow + 1; // 1-based
    for (auto& rValue : rOrder)
    {
        if (rValue >= nOffset)
            rValue += nCount;
    }
    for (SCROW i = 0; i < nCount; ++i)
        rOrder.push_back(nOffset + i);
}

/** Remove order entries at 1-based nOffset for nCount positions, adjusting remaining values. */
void removeOrderEntries(std::vector<SCCOLROW>& rOrder, SCCOLROW nOffset, SCROW nCount)
{
    std::vector<SCCOLROW> aNewOrder;
    for (auto& rValue : rOrder)
    {
        if (rValue >= nOffset && rValue < nOffset + nCount)
            continue; // deleted
        if (rValue >= nOffset + nCount)
            aNewOrder.push_back(rValue - nCount);
        else
            aNewOrder.push_back(rValue);
    }
    rOrder = std::move(aNewOrder);
}

/** Adjust a row range and order indices for deleted rows.
 *
 * On entire range deletion the order is cleared.
 */
void deleteRowsFromOrderedRange(SCROW& nStart, SCROW& nEnd, std::vector<SCCOLROW>& rOrder,
                                SCROW nDeleteStart, SCROW nDeleteCount)
{
    SCROW nDeleteEnd = nDeleteStart + nDeleteCount - 1;

    // After the range - nothing to do
    if (nDeleteStart > nEnd)
        return;

    // Before the range - shift
    if (nDeleteEnd < nStart)
    {
        nStart -= nDeleteCount;
        nEnd -= nDeleteCount;
        return;
    }

    // Entire range deleted
    if (nDeleteStart <= nStart && nDeleteEnd >= nEnd)
    {
        rOrder.clear();
        return;
    }

    // Partial overlap - compute overlap and update order
    SCROW nOverlapStart = std::max(nDeleteStart, nStart);
    SCROW nOverlapEnd = std::min(nDeleteEnd, nEnd);
    SCROW nDeletedWithin = nOverlapEnd - nOverlapStart + 1;
    SCCOLROW nOffset = nOverlapStart - nStart + 1; // + 1 because offset is 1-based

    removeOrderEntries(rOrder, nOffset, nDeletedWithin);

    if (nDeleteStart <= nStart)
    {
        SCROW nDeletedBefore = nStart - nDeleteStart;
        nStart -= nDeletedBefore;
        nEnd -= nDeleteCount;
    }
    else
    {
        nEnd -= nDeletedWithin;
    }
}
/** Adjust a column range for inserted columns. */
void adjustColumnRangeForInsert(SCCOL& nStart, SCCOL& nEnd, SCCOL nInsertStart, SCCOL nInsertCount)
{
    if (nInsertStart <= nStart)
    {
        nStart += nInsertCount;
        nEnd += nInsertCount;
    }
    else if (nInsertStart <= nEnd)
    {
        nEnd += nInsertCount;
    }
}

/** Adjust a column range for deleted columns.
 *
 * On entire range deletion the order is cleared.
 */
void deleteColumnsFromOrderedRange(SCCOL& nStart, SCCOL& nEnd, std::vector<SCCOLROW>& rOrder,
                                   SCCOL nDeleteStart, SCCOL nDeleteCount)
{
    SCCOL nDeleteEnd = nDeleteStart + nDeleteCount - 1;

    // After the range - nothing to do
    if (nDeleteStart > nEnd)
        return;

    // Before the range - shift
    if (nDeleteEnd < nStart)
    {
        nStart -= nDeleteCount;
        nEnd -= nDeleteCount;
        return;
    }

    // Entire range deleted
    if (nDeleteStart <= nStart && nDeleteEnd >= nEnd)
    {
        rOrder.clear();
        return;
    }

    // Partial overlap
    SCCOL nOverlapStart = std::max(nDeleteStart, nStart);
    SCCOL nOverlapEnd = std::min(nDeleteEnd, nEnd);
    SCCOL nDeletedWithin = nOverlapEnd - nOverlapStart + 1;
    if (nDeleteStart <= nStart)
        nStart = nDeleteStart;
    nEnd -= nDeletedWithin;
}
} // end anonymous namespace

void SortOrderReverser::addOrderIndices(SortOrderInfo const& rSortInfo)
{
    bool bKeyStatesEqual = std::equal(rSortInfo.maKeyStates.begin(), rSortInfo.maKeyStates.end(),
                                      maSortInfo.maKeyStates.begin(), maSortInfo.maKeyStates.end());

    if (bKeyStatesEqual && maSortInfo.mnFirstRow == rSortInfo.mnFirstRow
        && maSortInfo.mnLastRow == rSortInfo.mnLastRow)
    {
        if (maSortInfo.maOrder.empty())
        {
            // just set the order - no need to merge
            maSortInfo.maOrder = rSortInfo.maOrder;
        }
        else
        {
            maSortInfo.maOrder = mergeOrder(maSortInfo.maOrder, rSortInfo.maOrder);
        }
    }
    else
    {
        maSortInfo = rSortInfo;
    }
}

SCROW SortOrderReverser::unsort(SCROW nRow, SCCOL nColumn) const
{
    if (maSortInfo.maOrder.empty())
        return nRow;

    if (maSortInfo.mnFirstColumn >= 0 && maSortInfo.mnLastColumn >= 0
        && (nColumn < maSortInfo.mnFirstColumn || nColumn > maSortInfo.mnLastColumn))
        return nRow;

    if (nRow < maSortInfo.mnFirstRow || nRow > maSortInfo.mnLastRow)
        return nRow;

    size_t index = nRow - maSortInfo.mnFirstRow;
    auto nUnsortedRow = maSortInfo.mnFirstRow + maSortInfo.maOrder[index] - 1;
    return nUnsortedRow;
}

SCROW SortOrderReverser::resort(SCROW nRow, SCCOL nColumn) const
{
    if (maSortInfo.maOrder.empty())
        return nRow;

    if (maSortInfo.mnFirstColumn >= 0 && maSortInfo.mnLastColumn >= 0
        && (nColumn < maSortInfo.mnFirstColumn || nColumn > maSortInfo.mnLastColumn))
        return nRow;

    if (nRow < maSortInfo.mnFirstRow || nRow > maSortInfo.mnLastRow)
        return nRow;

    SCCOLROW nOrderValue = nRow - maSortInfo.mnFirstRow + 1;
    for (size_t nIndex = 0; nIndex < maSortInfo.maOrder.size(); ++nIndex)
    {
        if (maSortInfo.maOrder[nIndex] == nOrderValue)
            return maSortInfo.mnFirstRow + nIndex;
    }
    return nRow;
}

void SortOrderReverser::insertedRows(SCROW nStartRow, SCROW nRowCount)
{
    if (nStartRow > maSortInfo.mnLastRow + 1)
    {
        // After the sort range - nothing to do.
    }
    else if (nStartRow < maSortInfo.mnFirstRow)
    {
        // Before the sort range - shift the whole range.
        maSortInfo.mnFirstRow += nRowCount;
        maSortInfo.mnLastRow += nRowCount;
    }
    else
    {
        // At start of or within the sort range - expand and update order.
        expandOrderIndices(maSortInfo.maOrder, nStartRow, maSortInfo.mnFirstRow, nRowCount);
        maSortInfo.mnLastRow += nRowCount;
    }
}

void SortOrderReverser::deletedRows(SCROW nStartRow, SCROW nRowCount)
{
    deleteRowsFromOrderedRange(maSortInfo.mnFirstRow, maSortInfo.mnLastRow, maSortInfo.maOrder,
                               nStartRow, nRowCount);

    // no order indices - range is empty now
    if (maSortInfo.maOrder.empty())
    {
        maSortInfo.mnFirstRow = 0;
        maSortInfo.mnLastRow = 0;
    }
}

void SortOrderReverser::insertedColumns(SCCOL nStartColumn, SCCOL nColumnCount)
{
    adjustColumnRangeForInsert(maSortInfo.mnFirstColumn, maSortInfo.mnLastColumn, nStartColumn,
                               nColumnCount);
}

void SortOrderReverser::deletedColumns(SCCOL nStartColumn, SCCOL nColumnCount)
{
    deleteColumnsFromOrderedRange(maSortInfo.mnFirstColumn, maSortInfo.mnLastColumn,
                                  maSortInfo.maOrder, nStartColumn, nColumnCount);
    if (maSortInfo.maOrder.empty())
    {
        maSortInfo.mnFirstColumn = 0;
        maSortInfo.mnLastColumn = 0;
    }
}

SheetView::SheetView(ScTable* pTable, OUString const& rName, SheetViewID nID)
    : mpTable(pTable)
    , maName(rName)
    , mnID(nID)
{
}

ScTable* SheetView::getTablePointer() const { return mpTable; }

SCTAB SheetView::getTableNumber() const
{
    assert(mpTable);
    return mpTable->GetTab();
}

SheetViewSortData& SheetView::ensureSortData()
{
    if (!mpSortData)
        mpSortData = std::make_shared<SheetViewSortData>();
    return *mpSortData;
}

SortOrderReverser const* SheetView::getSortOrder() const
{
    if (!mpSortData || mpSortData->maSortOrder.maSortInfo.maOrder.empty())
        return nullptr;
    return &mpSortData->maSortOrder;
}

void SheetView::resetSortOrder()
{
    if (mpSortData)
        mpSortData->maSortOrder = SortOrderReverser();
}

void SheetView::addOrderIndices(SortOrderInfo const& rSortInfo)
{
    ensureSortData().maSortOrder.addOrderIndices(rSortInfo);
}

void SheetView::mergeReorderParameters(ReorderParam const& rReorderParameters)
{
    auto& rParams = ensureSortData().maOriginalReorderParams;
    if (!rParams.maOrderIndices.empty())
    {
        rParams.maOrderIndices
            = mergeOrder(rParams.maOrderIndices, rReorderParameters.maOrderIndices);
    }
    else
    {
        rParams = rReorderParameters;
    }
}

ReorderParam const* SheetView::getReorderParameters() const
{
    if (!mpSortData || mpSortData->maOriginalReorderParams.maOrderIndices.empty())
        return nullptr;
    return &mpSortData->maOriginalReorderParams;
}

void SheetView::restoreReorderParameters(ReorderParam const& rParams)
{
    ensureSortData().maOriginalReorderParams = rParams;
}

SCROW SheetView::reverseSortingToDefaultView(SCROW nRow, SCCOL nColumn) const
{
    SCROW nUnsortedRow = nRow;

    // Unsort the sheet view's sort and bring it to the default view's order
    // when the sheet view was created.
    if (mpSortData)
    {
        nUnsortedRow = mpSortData->maSortOrder.unsort(nRow, nColumn);
    }

    // Resort the default view's sort done since the sheet view creation.
    SCROW nResortedRow = nUnsortedRow;
    if (mpSortData && !mpSortData->maOriginalReorderParams.maOrderIndices.empty())
    {
        ScRange const& rRange = mpSortData->maOriginalReorderParams.maSortRange;

        SortOrderReverser aReverser;
        aReverser.addOrderIndices({ rRange.aStart.Col(),
                                    rRange.aEnd.Col(),
                                    rRange.aStart.Row(),
                                    rRange.aEnd.Row(),
                                    mpSortData->maOriginalReorderParams.maOrderIndices,
                                    {} });

        nResortedRow = aReverser.resort(nUnsortedRow, nColumn);
    }
    return nResortedRow;
}

void SheetView::adjustReorderParamsForInsertRows(SCROW nStartRow, SCROW nRowCount)
{
    if (!mpSortData || mpSortData->maOriginalReorderParams.maOrderIndices.empty())
        return;

    ScRange& rRange = mpSortData->maOriginalReorderParams.maSortRange;
    if (nStartRow < rRange.aStart.Row())
    {
        rRange.aStart.IncRow(nRowCount);
        rRange.aEnd.IncRow(nRowCount);
    }
    else if (nStartRow <= rRange.aEnd.Row() + 1)
    {
        expandOrderIndices(mpSortData->maOriginalReorderParams.maOrderIndices, nStartRow,
                           rRange.aStart.Row(), nRowCount);
        rRange.aEnd.IncRow(nRowCount);
    }
}

void SheetView::adjustSortParamForInsertRows(SCROW nStartRow, SCROW nRowCount)
{
    if (!mpSortData)
        return;

    if (nStartRow < mpSortData->maSortParam.nRow1)
    {
        mpSortData->maSortParam.nRow1 += nRowCount;
        mpSortData->maSortParam.nRow2 += nRowCount;
    }
    else if (nStartRow <= mpSortData->maSortParam.nRow2 + 1)
    {
        mpSortData->maSortParam.nRow2 += nRowCount;
    }
}

void SheetView::insertedRows(SCROW nStartRow, SCROW nRowCount)
{
    if (mpSortData)
        mpSortData->maSortOrder.insertedRows(nStartRow, nRowCount);

    // Update the reorder params that track the default view's sort since sheet view creation
    adjustReorderParamsForInsertRows(nStartRow, nRowCount);

    // Update the stored sort parameters (the range the sheet view was sorted on)
    adjustSortParamForInsertRows(nStartRow, nRowCount);
}

void SheetView::adjustReorderParamsForDeleteRows(SCROW nStartRow, SCROW nRowCount)
{
    if (!mpSortData || mpSortData->maOriginalReorderParams.maOrderIndices.empty())
        return;

    ScRange& rRange = mpSortData->maOriginalReorderParams.maSortRange;
    SCROW nRangeStart = rRange.aStart.Row();
    SCROW nRangeEnd = rRange.aEnd.Row();
    deleteRowsFromOrderedRange(nRangeStart, nRangeEnd,
                               mpSortData->maOriginalReorderParams.maOrderIndices, nStartRow,
                               nRowCount);
    if (mpSortData->maOriginalReorderParams.maOrderIndices.empty())
    {
        mpSortData->maOriginalReorderParams = ReorderParam();
    }
    else
    {
        rRange.aStart.SetRow(nRangeStart);
        rRange.aEnd.SetRow(nRangeEnd);
    }
}

void SheetView::adjustSortParamForDeleteRows(SCROW nStartRow, SCROW nRowCount)
{
    if (!mpSortData)
        return;

    std::vector<SCCOLROW> aUnused;
    deleteRowsFromOrderedRange(mpSortData->maSortParam.nRow1, mpSortData->maSortParam.nRow2,
                               aUnused, nStartRow, nRowCount);
}

void SheetView::deletedRows(SCROW nStartRow, SCROW nRowCount)
{
    // Update the sheet view's own sort order
    if (mpSortData)
        mpSortData->maSortOrder.deletedRows(nStartRow, nRowCount);

    // Update the reorder params that track the default view's sort since sheet view creation
    adjustReorderParamsForDeleteRows(nStartRow, nRowCount);

    // Update the stored sort parameters (the range the sheet view was sorted on)
    adjustSortParamForDeleteRows(nStartRow, nRowCount);
}

ScSortParam const* SheetView::getSortParam() const
{
    if (!mpSortData || !mpSortData->maSortParam.GetSortKeyCount()
        || !mpSortData->maSortParam.maKeyState[0].bDoSort)
        return nullptr;
    return &mpSortData->maSortParam;
}

void SheetView::setSortParam(ScSortParam const& rSortParam)
{
    ensureSortData().maSortParam = rSortParam;
}

void SheetView::adjustReorderParamsForInsertColumns(SCCOL nStartColumn, SCCOL nColumnCount)
{
    if (!mpSortData || mpSortData->maOriginalReorderParams.maOrderIndices.empty())
        return;

    ScRange& rRange = mpSortData->maOriginalReorderParams.maSortRange;
    SCCOL nColumnStart = rRange.aStart.Col();
    SCCOL nColumnEnd = rRange.aEnd.Col();
    adjustColumnRangeForInsert(nColumnStart, nColumnEnd, nStartColumn, nColumnCount);
    rRange.aStart.SetCol(nColumnStart);
    rRange.aEnd.SetCol(nColumnEnd);
}

void SheetView::adjustSortParamForInsertColumns(SCCOL nStartColumn, SCCOL nColumnCount)
{
    if (!mpSortData)
        return;

    adjustColumnRangeForInsert(mpSortData->maSortParam.nCol1, mpSortData->maSortParam.nCol2,
                               nStartColumn, nColumnCount);

    // We need to adjust key state - columns shift by one
    for (auto& rKey : mpSortData->maSortParam.maKeyState)
    {
        if (rKey.bDoSort && rKey.nField >= nStartColumn)
            rKey.nField += nColumnCount;
    }
}

void SheetView::insertedColumns(SCCOL nStartColumn, SCCOL nColumnCount)
{
    // Update the sheet view's own sort order
    if (mpSortData)
        mpSortData->maSortOrder.insertedColumns(nStartColumn, nColumnCount);

    // Update the reorder params that track the default view's sort since sheet view creation
    adjustReorderParamsForInsertColumns(nStartColumn, nColumnCount);

    // Update the stored sort parameters (the range the sheet view was sorted on)
    adjustSortParamForInsertColumns(nStartColumn, nColumnCount);
}

void SheetView::adjustReorderParamsForDeleteColumns(SCCOL nStartColumn, SCCOL nColumnCount)
{
    if (!mpSortData || mpSortData->maOriginalReorderParams.maOrderIndices.empty())
        return;

    ScRange& rRange = mpSortData->maOriginalReorderParams.maSortRange;
    SCCOL nStart = rRange.aStart.Col();
    SCCOL nEnd = rRange.aEnd.Col();
    deleteColumnsFromOrderedRange(nStart, nEnd, mpSortData->maOriginalReorderParams.maOrderIndices,
                                  nStartColumn, nColumnCount);
    if (mpSortData->maOriginalReorderParams.maOrderIndices.empty())
    {
        mpSortData->maOriginalReorderParams = ReorderParam();
    }
    else
    {
        rRange.aStart.SetCol(nStart);
        rRange.aEnd.SetCol(nEnd);
    }
}

void SheetView::adjustSortParamForDeleteColumns(SCCOL nStartColumn, SCCOL nColumnCount)
{
    if (!mpSortData)
        return;

    SCCOL nDeleteEnd = nStartColumn + nColumnCount - 1;

    std::vector<SCCOLROW> aUnused;
    deleteColumnsFromOrderedRange(mpSortData->maSortParam.nCol1, mpSortData->maSortParam.nCol2,
                                  aUnused, nStartColumn, nColumnCount);

    // We need to adjust key state - columns shift by one
    for (auto& rKey : mpSortData->maSortParam.maKeyState)
    {
        if (rKey.bDoSort && rKey.nField >= nStartColumn)
        {
            if (rKey.nField <= nDeleteEnd)
                rKey.bDoSort = false; // sort key column was deleted
            else
                rKey.nField -= nColumnCount;
        }
    }
}

void SheetView::deletedColumns(SCCOL nStartColumn, SCCOL nColumnCount)
{
    // Update the sheet view's own sort order
    if (mpSortData)
        mpSortData->maSortOrder.deletedColumns(nStartColumn, nColumnCount);

    // Update the reorder params that track the default view's sort since sheet view creation
    adjustReorderParamsForDeleteColumns(nStartColumn, nColumnCount);

    // Update the stored sort parameters (the range the sheet view was sorted on)
    adjustSortParamForDeleteColumns(nStartColumn, nColumnCount);
}

void SheetView::resetSortData() { mpSortData.reset(); }

std::shared_ptr<SheetViewSortData> SheetView::captureSortData() const
{
    if (!mpSortData)
        return nullptr;
    return std::make_shared<SheetViewSortData>(*mpSortData);
}

void SheetView::restoreSortData(std::shared_ptr<SheetViewSortData> const& pData)
{
    if (pData)
        mpSortData = std::make_shared<SheetViewSortData>(*pData);
    else
        mpSortData.reset();
}

} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
