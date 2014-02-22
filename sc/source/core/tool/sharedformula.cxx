/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */

#include "sharedformula.hxx"
#include "calcmacros.hxx"
#include "tokenarray.hxx"

namespace sc {

void SharedFormulaUtil::splitFormulaCellGroup(const CellStoreType::position_type& aPos)
{
    SCROW nRow = aPos.first->position + aPos.second;

    if (aPos.first->type != sc::element_type_formula)
        
        return;

    if (aPos.second == 0)
        
        return;

    sc::formula_block::iterator it = sc::formula_block::begin(*aPos.first->data);
    std::advance(it, aPos.second);
    ScFormulaCell& rTop = **it;
    if (!rTop.IsShared())
        
        return;

    if (nRow == rTop.GetSharedTopRow())
        
        return;

    ScFormulaCellGroupRef xGroup = rTop.GetCellGroup();

    SCROW nLength2 = xGroup->mpTopCell->aPos.Row() + xGroup->mnLength - nRow;
    ScFormulaCellGroupRef xGroup2;
    if (nLength2 > 1)
    {
        xGroup2.reset(new ScFormulaCellGroup);
        xGroup2->mbInvariant = xGroup->mbInvariant;
        xGroup2->mpTopCell = &rTop;
        xGroup2->mnLength = nLength2;
        xGroup2->mpCode = xGroup->mpCode->Clone();
    }

    xGroup->mnLength = nRow - xGroup->mpTopCell->aPos.Row();
    if (xGroup->mnLength == 1)
    {
        
        ScFormulaCellGroupRef xNone;
        ScFormulaCell& rPrev = *sc::formula_block::at(*aPos.first->data, aPos.second-1);
        rPrev.SetCellGroup(xNone);
    }

    
#if DEBUG_COLUMN_STORAGE
    if (xGroup2->mpTopCell->aPos.Row() + xGroup2->mnLength > aPos.first->position + aPos.first->size)
    {
        cerr << "ScColumn::SplitFormulaCellGroup: Shared formula region goes beyond the formula block. Not good." << endl;
        cerr.flush();
        abort();
    }
#endif
    sc::formula_block::iterator itEnd = it;
    std::advance(itEnd, nLength2);
    for (; it != itEnd; ++it)
    {
        ScFormulaCell& rCell = **it;
        rCell.SetCellGroup(xGroup2);
    }
}

void SharedFormulaUtil::splitFormulaCellGroups(CellStoreType& rCells, std::vector<SCROW>& rBounds)
{
    if (rBounds.empty())
        return;

    
    std::sort(rBounds.begin(), rBounds.end());
    std::vector<SCROW>::iterator it = std::unique(rBounds.begin(), rBounds.end());
    rBounds.erase(it, rBounds.end());

    it = rBounds.begin();
    SCROW nRow = *it;
    CellStoreType::position_type aPos = rCells.position(nRow);
    if (aPos.first == rCells.end())
        return;

    splitFormulaCellGroup(aPos);
    std::vector<SCROW>::iterator itEnd = rBounds.end();
    for (++it; it != itEnd; ++it)
    {
        nRow = *it;
        aPos = rCells.position(aPos.first, nRow);
        if (aPos.first == rCells.end())
            return;

        splitFormulaCellGroup(aPos);
    }
}

void SharedFormulaUtil::joinFormulaCells(const CellStoreType::position_type& rPos, ScFormulaCell& rCell1, ScFormulaCell& rCell2)
{
    ScFormulaCell::CompareState eState = rCell1.CompareByTokenArray(rCell2);
    if (eState == ScFormulaCell::NotEqual)
        return;

    
    ScFormulaCellGroupRef xGroup1 = rCell1.GetCellGroup();
    ScFormulaCellGroupRef xGroup2 = rCell2.GetCellGroup();
    if (xGroup1)
    {
        if (xGroup2)
        {
            
            if (xGroup1.get() == xGroup2.get())
                
                return;

            
            xGroup1->mnLength += xGroup2->mnLength;
            size_t nOffset = rPos.second + 1; 
            for (size_t i = 0, n = xGroup2->mnLength; i < n; ++i)
            {
                ScFormulaCell& rCell = *sc::formula_block::at(*rPos.first->data, nOffset+i);
                rCell.SetCellGroup(xGroup1);
            }
        }
        else
        {
            
            rCell2.SetCellGroup(xGroup1);
            ++xGroup1->mnLength;
        }
    }
    else
    {
        if (xGroup2)
        {
            
            rCell1.SetCellGroup(xGroup2);
            xGroup2->mpTopCell = &rCell1;
            ++xGroup2->mnLength;
        }
        else
        {
            
            assert(rCell1.aPos.Row() == (SCROW)(rPos.first->position + rPos.second));
            xGroup1 = rCell1.CreateCellGroup(2, eState == ScFormulaCell::EqualInvariant);
            rCell2.SetCellGroup(xGroup1);
        }
    }
}

void SharedFormulaUtil::joinFormulaCellAbove(const CellStoreType::position_type& aPos)
{
    if (aPos.first->type != sc::element_type_formula)
        
        return;

    if (aPos.second == 0)
        
        
        return;

    ScFormulaCell& rPrev = *sc::formula_block::at(*aPos.first->data, aPos.second-1);
    ScFormulaCell& rCell = *sc::formula_block::at(*aPos.first->data, aPos.second);
    sc::CellStoreType::position_type aPosPrev = aPos;
    --aPosPrev.second;
    joinFormulaCells(aPosPrev, rPrev, rCell);
}

void SharedFormulaUtil::unshareFormulaCell(const CellStoreType::position_type& aPos, ScFormulaCell& rCell)
{
    if (!rCell.IsShared())
        return;

    ScFormulaCellGroupRef xNone;
    sc::CellStoreType::iterator it = aPos.first;

    
    if (rCell.aPos.Row() == rCell.GetSharedTopRow())
    {
        
        ScFormulaCellGroupRef xGroup = rCell.GetCellGroup();
        if (xGroup->mnLength == 2)
        {
            
#if DEBUG_COLUMN_STORAGE
            if (aPos.second+1 >= aPos.first->size)
            {
                cerr << "ScColumn::UnshareFormulaCell: There is no next formula cell but there should be!" << endl;
                cerr.flush();
                abort();
            }
#endif
            ScFormulaCell& rNext = *sc::formula_block::at(*it->data, aPos.second+1);
            rNext.SetCellGroup(xNone);
        }
        else
        {
            
            ScFormulaCell& rNext = *sc::formula_block::at(*it->data, aPos.second+1);
            --xGroup->mnLength;
            xGroup->mpTopCell = &rNext;
        }
    }
    else if (rCell.aPos.Row() == rCell.GetSharedTopRow() + rCell.GetSharedLength() - 1)
    {
        
        ScFormulaCellGroupRef xGroup = rCell.GetCellGroup();
        if (xGroup->mnLength == 2)
        {
            
#if DEBUG_COLUMN_STORAGE
            if (aPos.second == 0)
            {
                cerr << "ScColumn::UnshareFormulaCell: There is no previous formula cell but there should be!" << endl;
                cerr.flush();
                abort();
            }
#endif
            ScFormulaCell& rPrev = *sc::formula_block::at(*it->data, aPos.second-1);
            rPrev.SetCellGroup(xNone);
        }
        else
        {
            
            --xGroup->mnLength;
        }
    }
    else
    {
        
        ScFormulaCellGroupRef xGroup = rCell.GetCellGroup();
        SCROW nEndRow = xGroup->mpTopCell->aPos.Row() + xGroup->mnLength - 1;
        xGroup->mnLength = rCell.aPos.Row() - xGroup->mpTopCell->aPos.Row(); 
        if (xGroup->mnLength == 1)
        {
            
#if DEBUG_COLUMN_STORAGE
            if (aPos.second == 0)
            {
                cerr << "ScColumn::UnshareFormulaCell: There is no previous formula cell but there should be!" << endl;
                cerr.flush();
                abort();
            }
#endif
            ScFormulaCell& rPrev = *sc::formula_block::at(*it->data, aPos.second-1);
            rPrev.SetCellGroup(xNone);
        }

        SCROW nLength2 = nEndRow - rCell.aPos.Row();
        if (nLength2 >= 2)
        {
            ScFormulaCellGroupRef xGroup2;
            xGroup2.reset(new ScFormulaCellGroup);
            ScFormulaCell& rNext = *sc::formula_block::at(*it->data, aPos.second+1);
            xGroup2->mpTopCell = &rNext;
            xGroup2->mnLength = nLength2;
            xGroup2->mbInvariant = xGroup->mbInvariant;
            xGroup2->mpCode = xGroup->mpCode->Clone();
#if DEBUG_COLUMN_STORAGE
            if (xGroup2->mpTopCell->aPos.Row() + xGroup2->mnLength > it->position + it->size)
            {
                cerr << "ScColumn::UnshareFormulaCell: Shared formula region goes beyond the formula block. Not good." << endl;
                cerr.flush();
                abort();
            }
#endif
            sc::formula_block::iterator itCell = sc::formula_block::begin(*it->data);
            std::advance(itCell, aPos.second+1);
            sc::formula_block::iterator itCellEnd = itCell;
            std::advance(itCellEnd, xGroup2->mnLength);
            for (; itCell != itCellEnd; ++itCell)
            {
                ScFormulaCell& rCell2 = **itCell;
                rCell2.SetCellGroup(xGroup2);
            }
        }
        else
        {
            
            sc::formula_block::iterator itCell = sc::formula_block::begin(*it->data);
            std::advance(itCell, aPos.second+1);
            ScFormulaCell& rCell2 = **itCell;
            rCell2.SetCellGroup(xNone);
        }
    }

    rCell.SetCellGroup(xNone);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
