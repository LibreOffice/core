/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cellvalues.hxx>
#include <column.hxx>
#include <cellvalue.hxx>
#include <sharedformula.hxx>

#include <cassert>
#include <boost/noncopyable.hpp>

namespace sc {

struct CellValuesImpl : boost::noncopyable
{
    CellStoreType maCells;
    CellTextAttrStoreType maCellTextAttrs;
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
    mpImpl->maCellTextAttrs.resize(nLen);
    rCol.maCells.transfer(nRow, nRow+nLen-1, mpImpl->maCells, 0);
    rCol.maCellTextAttrs.transfer(nRow, nRow+nLen-1, mpImpl->maCellTextAttrs, 0);
}

void CellValues::transferTo( ScColumn& rCol, SCROW nRow )
{
    assert(mpImpl->maCells.size() == mpImpl->maCellTextAttrs.size());

    size_t nLen = mpImpl->maCells.size();
    mpImpl->maCells.transfer(0, nLen-1, rCol.maCells, nRow);
    mpImpl->maCellTextAttrs.transfer(0, nLen-1, rCol.maCellTextAttrs, nRow);
}

void CellValues::copyTo( ScColumn& rCol, SCROW nRow ) const
{
    copyCellsTo(rCol, nRow);
    copyCellTextAttrsTo(rCol, nRow);
}

void CellValues::assign( const std::vector<double>& rVals )
{
    mpImpl->maCells.resize(rVals.size());
    mpImpl->maCells.set(0, rVals.begin(), rVals.end());

    // Set default text attributes.
    std::vector<CellTextAttr> aDefaults(rVals.size(), CellTextAttr());
    mpImpl->maCellTextAttrs.resize(rVals.size());
    mpImpl->maCellTextAttrs.set(0, aDefaults.begin(), aDefaults.end());
}

void CellValues::append( ScRefCellValue& rVal, const CellTextAttr* pAttr, const ScAddress& rPos )
{
    assert(mpImpl->maCells.size() == mpImpl->maCellTextAttrs.size());

    size_t n = mpImpl->maCells.size();

    bool bAppendAttr = true;

    switch (rVal.meType)
    {
        case CELLTYPE_STRING:
        {
            mpImpl->maCells.resize(n+1);
            mpImpl->maCells.set(n, *rVal.mpString);
        }
        break;
        case CELLTYPE_VALUE:
        {
            mpImpl->maCells.resize(n+1);
            mpImpl->maCells.set(n, rVal.mfValue);
        }
        break;
        case CELLTYPE_EDIT:
        {
            mpImpl->maCells.resize(n+1);
            mpImpl->maCells.set(n, rVal.mpEditText->Clone());
        }
        break;
        case CELLTYPE_FORMULA:
        {
            mpImpl->maCells.resize(n+1);
            CellStoreType::iterator itBlk = mpImpl->maCells.set(n, rVal.mpFormula->Clone(rPos));

            size_t nOffset = n - itBlk->position;
            CellStoreType::position_type aPos(itBlk, nOffset);
            SharedFormulaUtil::joinFormulaCellAbove(aPos);
        }
        break;
        default:
            bAppendAttr = false;
    }

    if (bAppendAttr)
    {
        mpImpl->maCellTextAttrs.resize(n+1);

        if (pAttr)
            mpImpl->maCellTextAttrs.set(n, *pAttr);
        else
            mpImpl->maCellTextAttrs.set(n, CellTextAttr());
    }
}

size_t CellValues::size() const
{
    assert(mpImpl->maCells.size() == mpImpl->maCellTextAttrs.size());
    return mpImpl->maCells.size();
}

void CellValues::copyCellsTo( ScColumn& rCol, SCROW nRow ) const
{
    CellStoreType& rDest = rCol.maCells;
    const CellStoreType& rSrc = mpImpl->maCells;

    // Caller must ensure the destination is long enough.
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

void CellValues::copyCellTextAttrsTo( ScColumn& rCol, SCROW nRow ) const
{
    CellTextAttrStoreType& rDest = rCol.maCellTextAttrs;
    const CellTextAttrStoreType& rSrc = mpImpl->maCellTextAttrs;

    // Caller must ensure the destination is long enough.
    assert(rSrc.size() + static_cast<size_t>(nRow) < rDest.size());

    SCROW nCurRow = nRow;
    CellTextAttrStoreType::iterator itPos = rDest.begin();

    CellTextAttrStoreType::const_iterator itBlk = rSrc.begin(), itBlkEnd = rSrc.end();
    for (; itBlk != itBlkEnd; ++itBlk)
    {
        switch (itBlk->type)
        {
            case element_type_celltextattr:
            {
                celltextattr_block::const_iterator it = celltextattr_block::begin(*itBlk->data);
                celltextattr_block::const_iterator itEnd = celltextattr_block::end(*itBlk->data);
                itPos = rDest.set(itPos, nCurRow, it, itEnd);
            }
            break;
            default:
                itPos = rDest.set_empty(itPos, nCurRow, nCurRow+itBlk->size-1);
        }

        nCurRow += itBlk->size;
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
