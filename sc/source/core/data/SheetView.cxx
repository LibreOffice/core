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
void SortOrderReverser::addOrderIndices(SortOrderInfo const& rSortInfo)
{
    bool bKeyStatesEqual = std::equal(rSortInfo.maKeyStates.begin(), rSortInfo.maKeyStates.begin(),
                                      maSortInfo.maKeyStates.begin());

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
            // merge the order
            size_t nOrderSize = maSortInfo.maOrder.size();
            assert(nOrderSize == rSortInfo.maOrder.size());

            std::vector<SCCOLROW> aNewOrder(nOrderSize);
            for (size_t nIndex = 0; nIndex < nOrderSize; ++nIndex)
            {
                size_t nSortedIndex = rSortInfo.maOrder[nIndex];
                aNewOrder[nIndex] = maSortInfo.maOrder[nSortedIndex - 1];
            }
            maSortInfo.maOrder = aNewOrder;
        }
    }
    else
    {
        maSortInfo = rSortInfo;
    }
}

SCROW SortOrderReverser::unsort(SCROW nRow) const
{
    if (maSortInfo.maOrder.empty())
        return nRow;

    if (nRow >= maSortInfo.mnFirstRow && nRow <= maSortInfo.mnLastRow)
    {
        size_t index = nRow - maSortInfo.mnFirstRow;
        auto nUnsortedRow = maSortInfo.mnFirstRow + maSortInfo.maOrder[index] - 1;
        return nUnsortedRow;
    }
    return nRow;
}

SCROW SortOrderReverser::resort(SCROW nRow) const
{
    if (maSortInfo.maOrder.empty())
        return nRow;

    if (nRow >= maSortInfo.mnFirstRow && nRow <= maSortInfo.mnLastRow)
    {
        SCCOLROW nOrderValue = nRow - maSortInfo.mnFirstRow + 1;
        for (size_t nIndex = 0; nIndex < maSortInfo.maOrder.size(); ++nIndex)
        {
            if (maSortInfo.maOrder[nIndex] == nOrderValue)
                return maSortInfo.mnFirstRow + nIndex;
        }
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
