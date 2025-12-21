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
void SortOrderReverser::addOrderIndices(std::vector<SCCOLROW> const& rOrder, SCROW firstRow,
                                        SCROW lastRow)
{
    mnFirstRow = firstRow;
    mnLastRow = lastRow;

    if (maOrder.empty())
    {
        maOrder = rOrder;
    }
    else
    {
        assert(maOrder.size() == rOrder.size());
        std::vector<SCCOLROW> newOrder(maOrder.size());
        for (size_t nIndex = 0; nIndex < maOrder.size(); ++nIndex)
        {
            size_t nSortedIndex = rOrder[nIndex];
            newOrder[nIndex] = maOrder[nSortedIndex - 1];
        }
        maOrder = std::move(newOrder);
    }
}

SCROW SortOrderReverser::unsort(SCROW nRow) const
{
    if (maOrder.empty())
        return nRow;

    if (nRow >= mnFirstRow && nRow <= mnLastRow)
    {
        size_t index = nRow - mnFirstRow;
        auto nUnsortedRow = mnFirstRow + maOrder[index] - 1;
        return nUnsortedRow;
    }
    return nRow;
}

SCROW SortOrderReverser::resort(SCROW nRow) const
{
    if (maOrder.empty())
        return nRow;

    if (nRow >= mnFirstRow && nRow <= mnLastRow)
    {
        SCCOLROW nOrderValue = nRow - mnFirstRow + 1;
        for (size_t nIndex = 0; nIndex < maOrder.size(); ++nIndex)
        {
            if (maOrder[nIndex] == nOrderValue)
                return mnFirstRow + nIndex;
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

void SheetView::addOrderIndices(std::vector<SCCOLROW> const& rOrder, SCROW firstRow, SCROW lastRow)
{
    if (!moSortOrder)
        moSortOrder.emplace();
    moSortOrder->addOrderIndices(rOrder, firstRow, lastRow);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
