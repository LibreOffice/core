/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <columniterator.hxx>
#include <column.hxx>
#include <document.hxx>
#include <table.hxx>

#include <osl/diagnose.h>

ScColumnTextWidthIterator::ScColumnTextWidthIterator(const ScDocument& rDoc, ScColumn& rCol, SCROW nStartRow, SCROW nEndRow) :
    mnEnd(static_cast<size_t>(nEndRow)),
    mnCurPos(0)
{
    init(rDoc, rCol.maCellTextAttrs, nStartRow, nEndRow);
}

ScColumnTextWidthIterator::ScColumnTextWidthIterator(const ScDocument& rDoc, const ScAddress& rStartPos, SCROW nEndRow) :
    mnEnd(static_cast<size_t>(nEndRow)),
    mnCurPos(0)
{
    auto & rCellTextAttrs = rDoc.maTabs[rStartPos.Tab()]->aCol[rStartPos.Col()].maCellTextAttrs;
    init(rDoc, rCellTextAttrs, rStartPos.Row(), nEndRow);
}

void ScColumnTextWidthIterator::next()
{
    ++miDataCur;
    ++mnCurPos;

    if (miDataCur != miDataEnd)
    {
        // Still in the same block. We're good.
        checkEndRow();
        return;
    }

    // Move to the next block.
    for (++miBlockCur; miBlockCur != miBlockEnd; ++miBlockCur)
    {
        if (miBlockCur->type != sc::element_type_celltextattr)
        {
            // We don't iterator over this block.
            mnCurPos += miBlockCur->size;
            continue;
        }

        getDataIterators(0);
        checkEndRow();
        return;
    }

    // Reached the end.
    assert(miBlockCur == miBlockEnd);
}

bool ScColumnTextWidthIterator::hasCell() const
{
    return miBlockCur != miBlockEnd;
}

SCROW ScColumnTextWidthIterator::getPos() const
{
    assert(miBlockCur != miBlockEnd && miDataCur != miDataEnd);
    return static_cast<SCROW>(mnCurPos);
}

sal_uInt16 ScColumnTextWidthIterator::getValue() const
{
    assert(miBlockCur != miBlockEnd && miDataCur != miDataEnd);
    return miDataCur->mnTextWidth;
}

void ScColumnTextWidthIterator::setValue(sal_uInt16 nVal)
{
    assert(miBlockCur != miBlockEnd && miDataCur != miDataEnd);
    miDataCur->mnTextWidth = nVal;
}

void ScColumnTextWidthIterator::init(const ScDocument& rDoc, sc::CellTextAttrStoreType& rAttrs,
                                     SCROW nStartRow, SCROW nEndRow)
{
    miBlockEnd = rAttrs.end();
    miBlockCur = miBlockEnd;

    if (!rDoc.ValidRow(nStartRow) || !rDoc.ValidRow(nEndRow))
        return;

    size_t nStart = static_cast<size_t>(nStartRow);
    if (nStart >= rAttrs.size())
        return;

    // Locate the block containing the start row. The store keeps the start row of every block,
    // so this is a binary search, and its cost does not grow with the start row.
    sc::CellTextAttrStoreType::position_type aPos = rAttrs.position(nStart);
    miBlockCur = aPos.first;
    if (miBlockCur == miBlockEnd)
        return;

    // Locate the initial row position within this block.
    if (miBlockCur->type == sc::element_type_celltextattr)
    {
        // This block stores text widths for non-empty cells.
        mnCurPos = nStart;
        getDataIterators(aPos.second);
        checkEndRow();
        return;
    }

    // Current block is not of ushort type.  Look for the first ushort block
    // below it.
    for (++miBlockCur; miBlockCur != miBlockEnd; ++miBlockCur)
    {
        if (miBlockCur->type != sc::element_type_celltextattr)
            continue;

        // Found!
        mnCurPos = miBlockCur->position;
        getDataIterators(0);
        checkEndRow();
        return;
    }

    // Not found.
    assert(miBlockCur == miBlockEnd);
}

void ScColumnTextWidthIterator::getDataIterators(size_t nOffsetInBlock)
{
    OSL_ENSURE(miBlockCur != miBlockEnd, "block is at end position");
#if 0
    // Does not compile
    OSL_ENSURE(miBlockCur->type == sc::celltextattr_block,
               "wrong block type - unsigned short block expected.");
#endif
    miDataCur = sc::celltextattr_block::begin(*miBlockCur->data);
    miDataEnd = sc::celltextattr_block::end(*miBlockCur->data);

    std::advance(miDataCur, nOffsetInBlock);
}

void ScColumnTextWidthIterator::checkEndRow()
{
    if (mnCurPos <= mnEnd)
        // We're still good.
        return;

    // We're below the end position. End the iteration.
    miBlockCur = miBlockEnd;
}

namespace sc {

ColumnIterator::ColumnIterator( const CellStoreType& rCells, SCROW nRow1, SCROW nRow2 ) :
    maPos(rCells.position(nRow1)),
    maPosEnd(rCells.position(maPos.first, nRow2)),
    mbComplete(false)
{
}

void ColumnIterator::next()
{
    if ( maPos == maPosEnd)
        mbComplete = true;
    else
        maPos = CellStoreType::next_position(maPos);
}

SCROW ColumnIterator::getRow() const
{
    return CellStoreType::logical_position(maPos);
}

bool ColumnIterator::hasCell() const
{
    return !mbComplete;
}

mdds::mtv::element_t ColumnIterator::getType() const
{
    return maPos.first->type;
}

ScRefCellValue ColumnIterator::getCell() const
{
    return toRefCell(maPos.first, maPos.second);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
