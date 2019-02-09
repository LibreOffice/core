/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <cellvalues.hxx>
#include <column.hxx>

#include <cassert>

namespace sc {

namespace {

struct BlockPos
{
    size_t mnStart;
    size_t mnEnd;
};

}

CellValueSpan::CellValueSpan( SCROW nRow1, SCROW nRow2 ) :
    mnRow1(nRow1), mnRow2(nRow2) {}

struct CellValuesImpl
{
    CellStoreType maCells;
    CellTextAttrStoreType maCellTextAttrs;
    CellStoreType::iterator miCellPos;
    CellTextAttrStoreType::iterator miAttrPos;

    CellValuesImpl() = default;

    CellValuesImpl(const CellValuesImpl&) = delete;
    const CellValuesImpl& operator=(const CellValuesImpl&) = delete;
};

CellValues::CellValues() :
    mpImpl(new CellValuesImpl) {}

CellValues::~CellValues()
{
}

void CellValues::transferFrom( ScColumn& rCol, SCROW nRow, size_t nLen )
{
    mpImpl->maCells.resize(nLen);
    mpImpl->maCellTextAttrs.resize(nLen);
    rCol.maCells.transfer(nRow, nRow+nLen-1, mpImpl->maCells, 0);
    rCol.maCellTextAttrs.transfer(nRow, nRow+nLen-1, mpImpl->maCellTextAttrs, 0);
}


void CellValues::copyTo( ScColumn& rCol, SCROW nRow ) const
{
    copyCellsTo(rCol, nRow);
    copyCellTextAttrsTo(rCol, nRow);
}

void CellValues::swapNonEmpty( ScColumn& rCol )
{
    std::vector<BlockPos> aBlocksToSwap;

    // Go through static value blocks and record their positions and sizes.
    for (const auto& rCell : mpImpl->maCells)
    {
        if (rCell.type == sc::element_type_empty)
            continue;

        BlockPos aPos;
        aPos.mnStart = rCell.position;
        aPos.mnEnd = aPos.mnStart + rCell.size - 1;
        aBlocksToSwap.push_back(aPos);
    }

    // Do the swapping.  The undo storage will store the replaced formula cells after this.
    for (const auto& rBlock : aBlocksToSwap)
    {
        rCol.maCells.swap(rBlock.mnStart, rBlock.mnEnd, mpImpl->maCells, rBlock.mnStart);
        rCol.maCellTextAttrs.swap(rBlock.mnStart, rBlock.mnEnd, mpImpl->maCellTextAttrs, rBlock.mnStart);
    }
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

size_t CellValues::size() const
{
    assert(mpImpl->maCells.size() == mpImpl->maCellTextAttrs.size());
    return mpImpl->maCells.size();
}

void CellValues::reset( size_t nSize )
{
    mpImpl->maCells.clear();
    mpImpl->maCells.resize(nSize);
    mpImpl->maCellTextAttrs.clear();
    mpImpl->maCellTextAttrs.resize(nSize);

    mpImpl->miCellPos = mpImpl->maCells.begin();
    mpImpl->miAttrPos = mpImpl->maCellTextAttrs.begin();
}

void CellValues::setValue( size_t nRow, double fVal )
{
    mpImpl->miCellPos = mpImpl->maCells.set(mpImpl->miCellPos, nRow, fVal);
    mpImpl->miAttrPos = mpImpl->maCellTextAttrs.set(mpImpl->miAttrPos, nRow, sc::CellTextAttr());
}

void CellValues::setValue( size_t nRow, const svl::SharedString& rStr )
{
    mpImpl->miCellPos = mpImpl->maCells.set(mpImpl->miCellPos, nRow, rStr);
    mpImpl->miAttrPos = mpImpl->maCellTextAttrs.set(mpImpl->miAttrPos, nRow, sc::CellTextAttr());
}

void CellValues::swap( CellValues& r )
{
    std::swap(mpImpl, r.mpImpl);
}

std::vector<CellValueSpan> CellValues::getNonEmptySpans() const
{
    std::vector<CellValueSpan> aRet;
    for (const auto& rCell : mpImpl->maCells)
    {
        if (rCell.type != element_type_empty)
        {
            // Record this span.
            size_t nRow1 = rCell.position;
            size_t nRow2 = nRow1 + rCell.size - 1;
            aRet.emplace_back(nRow1, nRow2);
        }
    }
    return aRet;
}

void CellValues::copyCellsTo( ScColumn& rCol, SCROW nRow ) const
{
    CellStoreType& rDest = rCol.maCells;
    const CellStoreType& rSrc = mpImpl->maCells;

    // Caller must ensure the destination is long enough.
    assert(rSrc.size() + static_cast<size_t>(nRow) < rDest.size());

    SCROW nCurRow = nRow;
    CellStoreType::iterator itPos = rDest.begin();

    for (const auto& rBlk : rSrc)
    {
        switch (rBlk.type)
        {
            case element_type_numeric:
            {
                numeric_block::const_iterator it = numeric_block::begin(*rBlk.data);
                numeric_block::const_iterator itEnd = numeric_block::end(*rBlk.data);
                itPos = rDest.set(itPos, nCurRow, it, itEnd);
            }
            break;
            case element_type_string:
            {
                string_block::const_iterator it = string_block::begin(*rBlk.data);
                string_block::const_iterator itEnd = string_block::end(*rBlk.data);
                itPos = rDest.set(itPos, nCurRow, it, itEnd);
            }
            break;
            case element_type_edittext:
            {
                edittext_block::const_iterator it = edittext_block::begin(*rBlk.data);
                edittext_block::const_iterator itEnd = edittext_block::end(*rBlk.data);
                std::vector<EditTextObject*> aVals;
                aVals.reserve(rBlk.size);
                for (; it != itEnd; ++it)
                {
                    const EditTextObject* p = *it;
                    aVals.push_back(p->Clone().release());
                }
                itPos = rDest.set(itPos, nCurRow, aVals.begin(), aVals.end());
            }
            break;
            case element_type_formula:
            {
                formula_block::const_iterator it = formula_block::begin(*rBlk.data);
                formula_block::const_iterator itEnd = formula_block::end(*rBlk.data);
                std::vector<ScFormulaCell*> aVals;
                aVals.reserve(rBlk.size);
                for (; it != itEnd; ++it)
                {
                    const ScFormulaCell* p = *it;
                    aVals.push_back(p->Clone());
                }
                itPos = rDest.set(itPos, nCurRow, aVals.begin(), aVals.end());
            }
            break;
            default:
                itPos = rDest.set_empty(itPos, nCurRow, nCurRow+rBlk.size-1);
        }

        nCurRow += rBlk.size;
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

    for (const auto& rBlk : rSrc)
    {
        switch (rBlk.type)
        {
            case element_type_celltextattr:
            {
                celltextattr_block::const_iterator it = celltextattr_block::begin(*rBlk.data);
                celltextattr_block::const_iterator itEnd = celltextattr_block::end(*rBlk.data);
                itPos = rDest.set(itPos, nCurRow, it, itEnd);
            }
            break;
            default:
                itPos = rDest.set_empty(itPos, nCurRow, nCurRow+rBlk.size-1);
        }

        nCurRow += rBlk.size;
    }
}

typedef std::vector<std::unique_ptr<CellValues>> TableType;
typedef std::vector<std::unique_ptr<TableType>> TablesType;

struct TableValues::Impl
{
    ScRange maRange;
    TablesType m_Tables;

    explicit Impl( const ScRange& rRange ) : maRange(rRange)
    {
        size_t nTabs = rRange.aEnd.Tab() - rRange.aStart.Tab() + 1;
        size_t nCols = rRange.aEnd.Col() - rRange.aStart.Col() + 1;

        for (size_t nTab = 0; nTab < nTabs; ++nTab)
        {
            m_Tables.push_back(std::make_unique<TableType>());
            std::unique_ptr<TableType>& rTab2 = m_Tables.back();
            for (size_t nCol = 0; nCol < nCols; ++nCol)
                rTab2->push_back(std::make_unique<CellValues>());
        }
    }

    CellValues* getCellValues( SCTAB nTab, SCCOL nCol )
    {
        if (nTab < maRange.aStart.Tab() || maRange.aEnd.Tab() < nTab)
            // sheet index out of bound.
            return nullptr;
        if (nCol < maRange.aStart.Col() || maRange.aEnd.Col() < nCol)
            // column index out of bound.
            return nullptr;
        size_t nTabOffset = nTab - maRange.aStart.Tab();
        if (nTabOffset >= m_Tables.size())
            return nullptr;
        std::unique_ptr<TableType>& rTab2 = m_Tables[nTab-maRange.aStart.Tab()];
        size_t nColOffset = nCol - maRange.aStart.Col();
        if (nColOffset >= rTab2->size())
            return nullptr;
        return &rTab2.get()[0][nColOffset].get()[0];
    }
};

TableValues::TableValues() :
    mpImpl(new Impl(ScRange(ScAddress::INITIALIZE_INVALID))) {}

TableValues::TableValues( const ScRange& rRange ) :
    mpImpl(new Impl(rRange)) {}

TableValues::~TableValues()
{
}

const ScRange& TableValues::getRange() const
{
    return mpImpl->maRange;
}

void TableValues::swap( SCTAB nTab, SCCOL nCol, CellValues& rColValue )
{
    CellValues* pCol = mpImpl->getCellValues(nTab, nCol);
    if (!pCol)
        return;

    pCol->swap(rColValue);
}

void TableValues::swapNonEmpty( SCTAB nTab, SCCOL nCol, ScColumn& rCol )
{
    CellValues* pCol = mpImpl->getCellValues(nTab, nCol);
    if (!pCol)
        return;

    pCol->swapNonEmpty(rCol);
}

std::vector<CellValueSpan> TableValues::getNonEmptySpans( SCTAB nTab, SCCOL nCol ) const
{
    std::vector<CellValueSpan> aRet;
    CellValues* pCol = mpImpl->getCellValues(nTab, nCol);
    if (pCol)
        aRet = pCol->getNonEmptySpans();

    return aRet;
}

void TableValues::swap( TableValues& rOther )
{
    std::swap(mpImpl, rOther.mpImpl);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
