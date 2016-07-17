/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "mtvelements.hxx"
#include "globalnames.hxx"
#include "document.hxx"
#include "cellvalue.hxx"
#include "column.hxx"

namespace sc {

CellStoreEvent::CellStoreEvent() : mpCol(nullptr) {}

CellStoreEvent::CellStoreEvent(ScColumn* pCol) : mpCol(pCol) {}

void CellStoreEvent::element_block_acquired(const mdds::mtv::base_element_block* block)
{
    if (!mpCol)
        return;

    switch (mdds::mtv::get_block_type(*block))
    {
        case sc::element_type_formula:
            ++mpCol->mnBlkCountFormula;
            break;
        default:
            ;
    }
}

void CellStoreEvent::element_block_released(const mdds::mtv::base_element_block* block)
{
    if (!mpCol)
        return;

    switch (mdds::mtv::get_block_type(*block))
    {
        case sc::element_type_formula:
            --mpCol->mnBlkCountFormula;
            break;
        default:
            ;
    }
}

ColumnBlockPositionSet::ColumnBlockPositionSet(ScDocument& rDoc) : mrDoc(rDoc) {}

ColumnBlockPosition* ColumnBlockPositionSet::getBlockPosition(SCTAB nTab, SCCOL nCol)
{
    osl::MutexGuard aGuard(&maMtxTables);

    TablesType::iterator itTab = maTables.find(nTab);
    if (itTab == maTables.end())
    {
        std::pair<TablesType::iterator,bool> r =
            maTables.insert(TablesType::value_type(nTab, ColumnsType()));
        if (!r.second)
            // insertion failed.
            return nullptr;

        itTab = r.first;
    }

    ColumnsType& rCols = itTab->second;

    ColumnsType::iterator it = rCols.find(nCol);
    if (it != rCols.end())
        // Block position for this column has already been fetched.
        return &it->second;

    std::pair<ColumnsType::iterator,bool> r =
        rCols.insert(
            ColumnsType::value_type(nCol, ColumnBlockPosition()));

    if (!r.second)
        // insertion failed.
        return nullptr;

    it = r.first;

    if (!mrDoc.InitColumnBlockPosition(it->second, nTab, nCol))
        return nullptr;

    return &it->second;
}

void ColumnBlockPositionSet::clear()
{
    osl::MutexGuard aGuard(&maMtxTables);
    maTables.clear();
}

ScRefCellValue toRefCell( const sc::CellStoreType::const_iterator& itPos, size_t nOffset )
{
    switch (itPos->type)
    {
        case sc::element_type_numeric:
            // Numeric cell
            return ScRefCellValue(sc::numeric_block::at(*itPos->data, nOffset));
        case sc::element_type_string:
            // String cell
            return ScRefCellValue(&sc::string_block::at(*itPos->data, nOffset));
        case sc::element_type_edittext:
            // Edit cell
            return ScRefCellValue(sc::edittext_block::at(*itPos->data, nOffset));
        break;
        case sc::element_type_formula:
            // Formula cell
            return ScRefCellValue(sc::formula_block::at(*itPos->data, nOffset));
        default:
            ;
    }

    return ScRefCellValue();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
