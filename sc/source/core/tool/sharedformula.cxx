/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sharedformula.hxx"
#include "calcmacros.hxx"

namespace sc {

void SharedFormulaUtil::splitFormulaCellGroup(const CellStoreType::position_type& aPos)
{
    SCROW nRow = aPos.first->position + aPos.second;

    if (aPos.first->type != sc::element_type_formula)
        // Not a formula cell block.
        return;

    if (aPos.second == 0)
        // Split position coincides with the block border. Nothing to do.
        return;

    sc::formula_block::iterator it = sc::formula_block::begin(*aPos.first->data);
    std::advance(it, aPos.second);
    ScFormulaCell& rTop = **it;
    if (!rTop.IsShared())
        // Not a shared formula.
        return;

    if (nRow == rTop.GetSharedTopRow())
        // Already the top cell of a shared group.
        return;

    ScFormulaCellGroupRef xGroup = rTop.GetCellGroup();

    ScFormulaCellGroupRef xGroup2(new ScFormulaCellGroup);
    xGroup2->mbInvariant = xGroup->mbInvariant;
    xGroup2->mnStart = nRow;
    xGroup2->mnLength = xGroup->mnStart + xGroup->mnLength - nRow;

    xGroup->mnLength = nRow - xGroup->mnStart;

    // Apply the lower group object to the lower cells.
#if DEBUG_COLUMN_STORAGE
        if (xGroup2->mnStart + xGroup2->mnLength > aPos.first->position + aPos.first->size)
        {
            cerr << "ScColumn::SplitFormulaCellGroup: Shared formula region goes beyond the formula block. Not good." << endl;
            cerr.flush();
            abort();
        }
#endif
    sc::formula_block::iterator itEnd = it;
    std::advance(itEnd, xGroup2->mnLength);
    for (; it != itEnd; ++it)
    {
        ScFormulaCell& rCell = **it;
        rCell.SetCellGroup(xGroup2);
    }
}

namespace {

void joinFormulaCells(const sc::CellStoreType::position_type& rPos, ScFormulaCell& rCell1, ScFormulaCell& rCell2)
{
    ScFormulaCell::CompareState eState = rCell1.CompareByTokenArray(rCell2);
    if (eState == ScFormulaCell::NotEqual)
        return;

    SCROW nRow = rPos.first->position + rPos.second;

    // Formula tokens equal those of the previous formula cell.
    ScFormulaCellGroupRef xGroup1 = rCell1.GetCellGroup();
    ScFormulaCellGroupRef xGroup2 = rCell2.GetCellGroup();
    if (xGroup1)
    {
        if (xGroup2)
        {
            // Both cell 1 and cell 2 are shared. Merge them together.
            if (xGroup1.get() == xGroup2.get())
                // They belong to the same group.
                return;

            // Set the group object from cell 1 to all cells in group 2.
            xGroup1->mnLength += xGroup2->mnLength;
            size_t nOffset = rPos.second + 1; // position of cell 2
            for (size_t i = 0, n = xGroup2->mnLength; i < n; ++i)
            {
                ScFormulaCell& rCell = *sc::formula_block::at(*rPos.first->data, nOffset+i);
                rCell.SetCellGroup(xGroup1);
            }
        }
        else
        {
            // cell 1 is shared but cell 2 is not.
            rCell2.SetCellGroup(xGroup1);
            ++xGroup1->mnLength;
        }
    }
    else
    {
        if (xGroup2)
        {
            // cell 1 is not shared, but cell 2 is already shared.
            rCell1.SetCellGroup(xGroup2);
            xGroup2->mnStart = nRow;
            ++xGroup2->mnLength;
        }
        else
        {
            // neither cells are shared.
            xGroup1.reset(new ScFormulaCellGroup);
            xGroup1->mnStart = nRow;
            xGroup1->mbInvariant = (eState == ScFormulaCell::EqualInvariant);
            xGroup1->mnLength = 2;

            rCell1.SetCellGroup(xGroup1);
            rCell2.SetCellGroup(xGroup1);
        }
    }
}

}

void SharedFormulaUtil::joinFormulaCellAbove(const CellStoreType::position_type& aPos)
{
    if (aPos.first->type != sc::element_type_formula)
        // This is not a formula cell.
        return;

    if (aPos.second == 0)
        // This cell is already the top cell in a formula block; the previous
        // cell is not a formula cell.
        return;

    ScFormulaCell& rPrev = *sc::formula_block::at(*aPos.first->data, aPos.second-1);
    ScFormulaCell& rCell = *sc::formula_block::at(*aPos.first->data, aPos.second);
    sc::CellStoreType::position_type aPosPrev = aPos;
    --aPosPrev.second;
    joinFormulaCells(aPosPrev, rPrev, rCell);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
