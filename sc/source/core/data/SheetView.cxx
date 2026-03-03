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
}

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

    if (nColumn < maSortInfo.mnFirstColumn || nColumn > maSortInfo.mnLastColumn)
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

    if (nColumn < maSortInfo.mnFirstColumn || nColumn > maSortInfo.mnLastColumn)
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

void SheetView::addOrderIndices(SortOrderInfo const& rSortInfo)
{
    if (!moSortOrder)
        moSortOrder.emplace();
    moSortOrder->addOrderIndices(rSortInfo);
}

void SheetView::mergeReorderParameters(ReorderParam const& rReorderParameters)
{
    if (moOriginalReorderParams)
    {
        moOriginalReorderParams->maOrderIndices = mergeOrder(
            moOriginalReorderParams->maOrderIndices, rReorderParameters.maOrderIndices);
    }
    else
    {
        moOriginalReorderParams = rReorderParameters;
    }
}

SCROW SheetView::reverseSortingToDefaultView(SCROW nRow, SCCOL nColumn) const
{
    SCROW nUnsortedRow = nRow;

    // Unsort the sheet view's sort and bring it to the default view's order
    // when the sheet view was created.
    if (moSortOrder)
    {
        nUnsortedRow = moSortOrder->unsort(nRow, nColumn);
    }

    // Resort the default view's sort done since the sheet view creation.
    SCROW nResortedRow = nUnsortedRow;
    if (moOriginalReorderParams)
    {
        ScRange const& rRange = moOriginalReorderParams->maSortRange;

        SortOrderReverser aReverser;
        aReverser.addOrderIndices({ rRange.aStart.Col(),
                                    rRange.aEnd.Col(),
                                    rRange.aStart.Row(),
                                    rRange.aEnd.Row(),
                                    moOriginalReorderParams->maOrderIndices,
                                    {} });

        nResortedRow = aReverser.resort(nUnsortedRow, nColumn);
    }
    return nResortedRow;
}

} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
