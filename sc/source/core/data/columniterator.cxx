/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
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

ScColumnTextWidthIterator::ScColumnTextWidthIterator(ScColumn& rCol, SCROW nStartRow, SCROW nEndRow) :
    mrCellTextAttrs(rCol.maCellTextAttrs),
    mnEnd(static_cast<size_t>(nEndRow)),
    mnCurPos(0),
    miBlockCur(mrCellTextAttrs.begin()),
    miBlockEnd(mrCellTextAttrs.end())
{
    init(nStartRow, nEndRow);
}

ScColumnTextWidthIterator::ScColumnTextWidthIterator(ScDocument& rDoc, const ScAddress& rStartPos, SCROW nEndRow) :
    mrCellTextAttrs(rDoc.maTabs[rStartPos.Tab()]->aCol[rStartPos.Col()].maCellTextAttrs),
    mnEnd(static_cast<size_t>(nEndRow)),
    mnCurPos(0),
    miBlockCur(mrCellTextAttrs.begin()),
    miBlockEnd(mrCellTextAttrs.end())
{
    init(rStartPos.Row(), nEndRow);
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

void ScColumnTextWidthIterator::init(SCROW nStartRow, SCROW nEndRow)
{
    if (!ValidRow(nStartRow) || !ValidRow(nEndRow))
        miBlockCur = miBlockEnd;

    size_t nStart = static_cast<size_t>(nStartRow);

    // Locate the start row position.
    size_t nBlockStart = 0, nBlockEnd = 0;
    for (; miBlockCur != miBlockEnd; ++miBlockCur, nBlockStart = nBlockEnd)
    {
        nBlockEnd = nBlockStart + miBlockCur->size; // non-inclusive end point.
        if (nBlockStart <= nStart && nStart < nBlockEnd)
        {
            // Initial block is found!
            break;
        }
    }

    if (miBlockCur == miBlockEnd)
        // Initial block not found for whatever reason... Bail out.
        return;

    // Locate the initial row position within this block.
    if (miBlockCur->type == sc::element_type_celltextattr)
    {
        // This block stores text widths for non-empty cells.
        size_t nOffsetInBlock = nStart - nBlockStart;
        mnCurPos = nStart;
        getDataIterators(nOffsetInBlock);
        checkEndRow();
        return;
    }

    // Current block is not of ushort type.  Skip to the next block.
    nBlockStart = nBlockEnd;
    ++miBlockCur;

    // Look for the first ushort block.
    for (; miBlockCur != miBlockEnd; ++miBlockCur, nBlockStart = nBlockEnd)
    {
        nBlockEnd = nBlockStart + miBlockCur->size; // non-inclusive end point.
        if (miBlockCur->type != sc::element_type_celltextattr)
            continue;

        // Found!
        mnCurPos = nBlockStart;
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

ColumnIterator::~ColumnIterator() {}

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
