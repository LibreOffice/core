/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */

#include "columniterator.hxx"
#include "column.hxx"
#include "document.hxx"
#include "table.hxx"

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
        
        checkEndRow();
        return;
    }

    
    for (++miBlockCur; miBlockCur != miBlockEnd; ++miBlockCur)
    {
        if (miBlockCur->type != sc::element_type_celltextattr)
        {
            
            mnCurPos += miBlockCur->size;
            continue;
        }

        getDataIterators(0);
        checkEndRow();
        return;
    }

    
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
    return miDataCur->mnTextWidth;
}

void ScColumnTextWidthIterator::setValue(sal_uInt16 nVal)
{
    OSL_ASSERT(miBlockCur != miBlockEnd && miDataCur != miDataEnd);
    miDataCur->mnTextWidth = nVal;
}

void ScColumnTextWidthIterator::init(SCROW nStartRow, SCROW nEndRow)
{
    if (!ValidRow(nStartRow) || !ValidRow(nEndRow))
        miBlockCur = miBlockEnd;

    size_t nStart = static_cast<size_t>(nStartRow);

    
    size_t nBlockStart = 0, nBlockEnd = 0;
    for (; miBlockCur != miBlockEnd; ++miBlockCur, nBlockStart = nBlockEnd)
    {
        nBlockEnd = nBlockStart + miBlockCur->size; 
        if (nBlockStart <= nStart && nStart < nBlockEnd)
        {
            
            break;
        }
    }

    if (miBlockCur == miBlockEnd)
        
        return;

    
    if (miBlockCur->type == sc::element_type_celltextattr)
    {
        
        size_t nOffsetInBlock = nStart - nBlockStart;
        mnCurPos = nStart;
        getDataIterators(nOffsetInBlock);
        checkEndRow();
        return;
    }

    
    nBlockStart = nBlockEnd;
    ++miBlockCur;

    
    for (; miBlockCur != miBlockEnd; ++miBlockCur, nBlockStart = nBlockEnd)
    {
        nBlockEnd = nBlockStart + miBlockCur->size; 
        if (miBlockCur->type != sc::element_type_celltextattr)
            continue;

        
        mnCurPos = nBlockStart;
        getDataIterators(0);
        checkEndRow();
        return;
    }

    
    OSL_ASSERT(miBlockCur == miBlockEnd);
}

void ScColumnTextWidthIterator::getDataIterators(size_t nOffsetInBlock)
{
    OSL_ENSURE(miBlockCur != miBlockEnd, "block is at end position");
#if 0
    
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
        
        return;

    
    miBlockCur = miBlockEnd;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
