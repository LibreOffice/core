/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_SHAREDFORMULA_HXX
#define SC_SHAREDFORMULA_HXX

#include "formulacell.hxx"
#include "mtvelements.hxx"

#include <vector>

namespace sc {

class SharedFormulaUtil
{
public:

    /**
     * Group formula cells stored in the passed container. The formula cells
     * in the container are assumed to be all <b>non-shared</b>.
     */
    template<typename _Iter>
    static void groupFormulaCells(const _Iter& itBeg, const _Iter& itEnd)
    {
        _Iter it = itBeg;
        ScFormulaCell* pPrev = *it;
        ScFormulaCell* pCur = NULL;
        for (++it; it != itEnd; ++it, pPrev = pCur)
        {
            pCur = *it;
            ScFormulaCell::CompareState eState = pCur->CompareByTokenArray(*pPrev);
            if (eState == ScFormulaCell::NotEqual)
                continue;

            ScFormulaCellGroupRef xGroup = pPrev->GetCellGroup();
            if (xGroup)
            {
                // Extend the group.
                ++xGroup->mnLength;
                pCur->SetCellGroup(xGroup);
                continue;
            }

            // Create a new group.
            xGroup = pPrev->CreateCellGroup(pPrev->aPos.Row(), 2, eState == ScFormulaCell::EqualInvariant);
            pCur->SetCellGroup(xGroup);
        }
    }

    /**
     * Split existing shared formula range at specified position. The cell at
     * specified position becomes the top cell of the lower shared formula
     * range after this call.  This method does nothing if the cell at
     * specified position is not a formula cell.
     *
     * @param aPos position of cell to examine.
     */
    static void splitFormulaCellGroup(const CellStoreType::position_type& aPos);

    /**
     * Split existing shared formula ranges at specified row positions.
     *
     * @param rCells cell storage container
     * @param rBounds row positions at which to split existing shared formula
     *                ranges. Note that this method will directly modify this
     *                parameter to sort and remove duplicates.
     */
    static void splitFormulaCellGroups(CellStoreType& rCells, std::vector<SCROW>& rBounds);

    /**
     * See if two specified adjacent formula cells can be merged, and if they
     * can, merge them into the same group.
     *
     * @param rPos position object of the first cell
     * @param rCell1 first cell
     * @param rCell2 second cell located immediately below the first cell.
     */
    static void joinFormulaCells(
        const CellStoreType::position_type& rPos, ScFormulaCell& rCell1, ScFormulaCell& rCell2);
    /**
     * Merge with an existing formula group (if any) located immediately above
     * if the cell at specified position is a formula cell, and its formula
     * tokens are identical to that of the above formula group.
     *
     * @param aPos position of cell to examine.
     */
    static void joinFormulaCellAbove(const CellStoreType::position_type& aPos);

    /**
     * Turn a shared formula cell into a non-shared one, and split it off from
     * the adjacent formula cell groups.
     *
     * @param aPos position of cell to examine
     * @param rCell formula cell instance
     */
    static void unshareFormulaCell(const CellStoreType::position_type& aPos, ScFormulaCell& rCell);
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
