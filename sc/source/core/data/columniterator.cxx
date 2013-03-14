/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "columniterator.hxx"
#include "column.hxx"
#include "document.hxx"
#include "table.hxx"

ScColumnTextWidthIterator::ScColumnTextWidthIterator(ScColumn& rCol, SCROW nStartRow, SCROW nEndRow) :
    mrTextWidths(rCol.maTextWidths),
    mnEnd(static_cast<size_t>(nEndRow)),
    mnCurPos(0),
    miBlockCur(mrTextWidths.begin()),
    miBlockEnd(mrTextWidths.end())
{
    init(nStartRow, nEndRow);
}

ScColumnTextWidthIterator::ScColumnTextWidthIterator(ScDocument& rDoc, const ScAddress& rStartPos, SCROW nEndRow) :
    mrTextWidths(rDoc.maTabs[rStartPos.Tab()]->aCol[rStartPos.Col()].maTextWidths),
    mnEnd(static_cast<size_t>(nEndRow)),
    mnCurPos(0),
    miBlockCur(mrTextWidths.begin()),
    miBlockEnd(mrTextWidths.end())
{
    init(rStartPos.Row(), nEndRow);
}

void ScColumnTextWidthIterator::next()
{
    ++miDataCur;
    ++mnCurPos;

    if (miDataCur != miDataEnd)
    {
        // Stil in the same block. We're good.
        checkEndRow();
        return;
    }

    // Move to the next block.
    for (++miBlockCur; miBlockCur != miBlockEnd; ++miBlockCur)
    {
        if (miBlockCur->type != mdds::mtv::element_type_ushort)
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
    OSL_ASSERT(miBlockCur == miBlockEnd);
}

bool ScColumnTextWidthIterator::hasCell() const
{
    return miBlockCur != miBlockEnd;
}

SCROW ScColumnTextWidthIterator::getPos() const
{
    OSL_ASSERT(miBlockCur != miBlockEnd && miDataCur != miDataEnd);
    return static_cast<SCROW>(mnCurPos);
}

sal_uInt16 ScColumnTextWidthIterator::getValue() const
{
    OSL_ASSERT(miBlockCur != miBlockEnd && miDataCur != miDataEnd);
    return *miDataCur;
}

void ScColumnTextWidthIterator::setValue(sal_uInt16 nVal)
{
    OSL_ASSERT(miBlockCur != miBlockEnd && miDataCur != miDataEnd);
    *miDataCur = nVal;
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
    if (miBlockCur->type == mdds::mtv::element_type_ushort)
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
        if (miBlockCur->type != mdds::mtv::element_type_ushort)
            continue;

        // Found!
        mnCurPos = nBlockStart;
        getDataIterators(0);
        checkEndRow();
        return;
    }

    // Not found.
    OSL_ASSERT(miBlockCur == miBlockEnd);
}

void ScColumnTextWidthIterator::getDataIterators(size_t nOffsetInBlock)
{
    OSL_ENSURE(miBlockCur != miBlockEnd, "block is at end position");
    OSL_ENSURE(miBlockCur->type == mdds::mtv::element_type_ushort,
               "wrong block type - unsigned short block expected.");

    miDataCur = mdds::mtv::ushort_element_block::begin(*miBlockCur->data);
    miDataEnd = mdds::mtv::ushort_element_block::end(*miBlockCur->data);

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
