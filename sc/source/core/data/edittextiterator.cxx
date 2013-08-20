/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "edittextiterator.hxx"
#include "document.hxx"
#include "table.hxx"
#include "column.hxx"

namespace sc {

EditTextIterator::EditTextIterator( const ScDocument& rDoc, SCTAB nTab ) :
    mrTable(*rDoc.maTabs.at(nTab)),
    mpCol(&mrTable.aCol[0]),
    mpColEnd(mpCol + static_cast<size_t>(MAXCOLCOUNT)),
    mpCells(&mpCol->maCells),
    maPos(mpCells->position(0)),
    miEnd(mpCells->end())
{
}

const EditTextObject* EditTextIterator::seek()
{
    while (maPos.first->type != sc::element_type_edittext)
    {
        incBlock();
        if (maPos.first == miEnd)
        {
            // Move to the next column.
            ++mpCol;
            if (mpCol == mpColEnd)
                // No more columns.
                return NULL;

            mpCells = &mpCol->maCells;
            maPos = mpCells->position(0);
            miEnd = mpCells->end();
        }
    }

    // We are on the right block type.
    return sc::edittext_block::at(*maPos.first->data, maPos.second);
}

void EditTextIterator::incPos()
{
    if (maPos.second + 1 < maPos.first->size)
        // Increment within the block.
        ++maPos.second;
    else
        incBlock();
}

void EditTextIterator::incBlock()
{
    ++maPos.first;
    maPos.second = 0;
}

const EditTextObject* EditTextIterator::first()
{
    mpCol = &mrTable.aCol[0];
    mpColEnd = mpCol + static_cast<size_t>(MAXCOLCOUNT);
    mpCells = &mpCol->maCells;
    maPos = mpCells->position(0);
    miEnd = mpCells->end();
    return seek();
}

const EditTextObject* EditTextIterator::next()
{
    if (maPos.first == miEnd)
        return NULL;

    incPos();
    return seek();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
