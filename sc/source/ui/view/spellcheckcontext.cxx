/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "spellcheckcontext.hxx"

namespace sc {

size_t SpellCheckContext::CellPos::Hash::operator() (const CellPos& rPos) const
{
    size_t nVal = rPos.mnCol;
    nVal = nVal << 4;
    nVal += rPos.mnRow;
    return nVal;
}

SpellCheckContext::CellPos::CellPos() : mnCol(0), mnRow(0) {}
SpellCheckContext::CellPos::CellPos(SCCOL nCol, SCROW nRow) : mnCol(nCol), mnRow(nRow) {}

void SpellCheckContext::CellPos::setInvalid()
{
    mnCol = -1;
    mnRow = -1;
}

bool SpellCheckContext::CellPos::isValid() const
{
    return mnCol >= 0 && mnRow >= 0;
}

void SpellCheckContext::CellPos::reset()
{
    mnCol = 0;
    mnRow = 0;
}

bool SpellCheckContext::CellPos::operator< (const CellPos& r) const
{
    if (mnCol != r.mnCol)
        return mnCol < r.mnCol;

    return mnRow < r.mnRow;
}

bool SpellCheckContext::CellPos::operator== (const CellPos& r) const
{
    return mnCol == r.mnCol && mnRow == r.mnRow;
}

SpellCheckContext::SpellCheckContext()
{
}

bool SpellCheckContext::isMisspelled( SCCOL nCol, SCROW nRow ) const
{
    return maMisspellCells.count(CellPos(nCol, nRow)) > 0;
}

const std::vector<editeng::MisspellRanges>* SpellCheckContext::getMisspellRanges(
    SCCOL nCol, SCROW nRow ) const
{
    CellMapType::const_iterator it = maMisspellCells.find(CellPos(nCol,nRow));
    if (it == maMisspellCells.end())
        return NULL;

    return &it->second;
}

void SpellCheckContext::setMisspellRanges(
    SCCOL nCol, SCROW nRow, const std::vector<editeng::MisspellRanges>* pRanges )
{
    CellPos aPos(nCol, nRow);
    CellMapType::iterator it = maMisspellCells.find(aPos);

    if (pRanges)
    {
        if (it == maMisspellCells.end())
            maMisspellCells.insert(CellMapType::value_type(aPos, *pRanges));
        else
            it->second = *pRanges;
    }
    else
    {
        if (it != maMisspellCells.end())
            maMisspellCells.erase(it);
    }
}

void SpellCheckContext::reset()
{
    maPos.reset();
    maMisspellCells.clear();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
