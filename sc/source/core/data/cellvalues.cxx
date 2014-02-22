/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */

#include <cellvalues.hxx>
#include <column.hxx>

#include <cassert>
#include <boost/noncopyable.hpp>

namespace sc {

struct CellValuesImpl : boost::noncopyable
{
    CellStoreType maCells;
};

CellValues::CellValues() :
    mpImpl(new CellValuesImpl) {}

CellValues::~CellValues()
{
    delete mpImpl;
}

void CellValues::transferFrom( ScColumn& rCol, SCROW nRow, size_t nLen )
{
    mpImpl->maCells.resize(nLen);
    rCol.maCells.transfer(nRow, nRow+nLen-1, mpImpl->maCells, 0);
}

void CellValues::copyTo( ScColumn& rCol, SCROW nRow ) const
{
    CellStoreType& rDest = rCol.maCells;
    const CellStoreType& rSrc = mpImpl->maCells;

    
    assert(rSrc.size() + static_cast<size_t>(nRow) < rDest.size());

    SCROW nCurRow = nRow;
    CellStoreType::iterator itPos = rDest.begin();

    CellStoreType::const_iterator itBlk = rSrc.begin(), itBlkEnd = rSrc.end();
    for (; itBlk != itBlkEnd; ++itBlk)
    {
        switch (itBlk->type)
        {
            case element_type_numeric:
            {
                numeric_block::const_iterator it = numeric_block::begin(*itBlk->data);
                numeric_block::const_iterator itEnd = numeric_block::end(*itBlk->data);
                itPos = rDest.set(itPos, nCurRow, it, itEnd);
            }
            break;
            case element_type_string:
            {
                string_block::const_iterator it = string_block::begin(*itBlk->data);
                string_block::const_iterator itEnd = string_block::end(*itBlk->data);
                itPos = rDest.set(itPos, nCurRow, it, itEnd);
            }
            break;
            case element_type_edittext:
            {
                edittext_block::const_iterator it = edittext_block::begin(*itBlk->data);
                edittext_block::const_iterator itEnd = edittext_block::end(*itBlk->data);
                std::vector<EditTextObject*> aVals;
                aVals.reserve(itBlk->size);
                for (; it != itEnd; ++it)
                {
                    const EditTextObject* p = *it;
                    aVals.push_back(p->Clone());
                }
                itPos = rDest.set(itPos, nCurRow, aVals.begin(), aVals.end());
            }
            break;
            case element_type_formula:
            {
                formula_block::const_iterator it = formula_block::begin(*itBlk->data);
                formula_block::const_iterator itEnd = formula_block::end(*itBlk->data);
                std::vector<ScFormulaCell*> aVals;
                aVals.reserve(itBlk->size);
                for (; it != itEnd; ++it)
                {
                    const ScFormulaCell* p = *it;
                    aVals.push_back(p->Clone());
                }
                itPos = rDest.set(itPos, nCurRow, aVals.begin(), aVals.end());
            }
            break;
            default:
                itPos = rDest.set_empty(itPos, nCurRow, nCurRow+itBlk->size-1);
        }

        nCurRow += itBlk->size;
    }
}

void CellValues::assign( const std::vector<double>& rVals )
{
    mpImpl->maCells.resize(rVals.size());
    mpImpl->maCells.set(0, rVals.begin(), rVals.end());
}

size_t CellValues::size() const
{
    return mpImpl->maCells.size();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
