/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_INC_SHAREDFORMULA_HXX
#define INCLUDED_SC_INC_SHAREDFORMULA_HXX

#include "formulacell.hxx"
#include "mtvelements.hxx"

#include <vector>

#define USE_FORMULA_GROUP_LISTENER 1

namespace sc {

class StartListeningContext;

class SharedFormulaUtil
{
public:

    /**
     * Group formula cells stored in the passed container. The formula cells
     * in the container are assumed to be all <b>non-shared</b>.
     */
    template<typename Iter>
    static void groupFormulaCells(const Iter& itBeg, const Iter& itEnd)
    {
        Iter it = itBeg;
        ScFormulaCell* pPrev = *it;
        ScFormulaCell* pCur = nullptr;
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
            xGroup = pPrev->CreateCellGroup(2, eState == ScFormulaCell::EqualInvariant);
            pCur->SetCellGroup(xGroup);
        }
    }

    /** Get shared formula top cell from position, if any, else nullptr. */
    static const ScFormulaCell* getSharedTopFormulaCell(const CellStoreType::position_type& aPos);

    /**
     * Split existing shared formula range at specified position. The cell at
     * specified position becomes the top cell of the lower shared formula
     * range after this call.  This method does nothing if the cell at
     * specified position is not a formula cell.
     *
     * @param aPos position of cell to examine.
     * @param pCxt context to be used, if any, may be nullptr.
     *
     * @return TRUE if there indeed was a split, else FALSE (e.g. split
     *         position was top or bottom cell or no formula group).
     */
    static bool splitFormulaCellGroup(const CellStoreType::position_type& aPos, sc::EndListeningContext* pCxt);

    /**
     * Split existing shared formula ranges at specified row positions.
     *
     * @param rCells cell storage container
     * @param rBounds row positions at which to split existing shared formula
     *                ranges. Note that this method will directly modify this
     *                parameter to sort and remove duplicates.
     *
     * @return TRUE if there indeed was a split, else FALSE (e.g. split
     *         positions were only top or bottom cells or no formula group).
     */
    static bool splitFormulaCellGroups(CellStoreType& rCells, std::vector<SCROW>& rBounds);

    /**
     * See if two specified adjacent formula cells can be merged, and if they
     * can, merge them into the same group.
     *
     * @param rPos position object of the first cell
     * @param rCell1 first cell
     * @param rCell2 second cell located immediately below the first cell.
     *
     * @return true if the cells are merged, false otherwise.  If the two
     *         cells already belong to the same group, it returns false.
     */
    static bool joinFormulaCells(
        const CellStoreType::position_type& rPos, ScFormulaCell& rCell1, ScFormulaCell& rCell2 );
    /**
     * Merge with an existing formula group (if any) located immediately above
     * if the cell at specified position is a formula cell, and its formula
     * tokens are identical to that of the above formula group.
     *
     * @param aPos position of cell to examine.
     *
     * @return true if the cells are merged, false otherwise.  If the two
     *         cells already belong to the same group, it returns false.
     */
    static bool joinFormulaCellAbove( const CellStoreType::position_type& aPos );

    /**
     * Turn a shared formula cell into a non-shared one, and split it off from
     * the adjacent formula cell groups.
     *
     * @param aPos position of cell to examine
     * @param rCell formula cell instance
     */
    static void unshareFormulaCell(const CellStoreType::position_type& aPos, ScFormulaCell& rCell);

    /**
     * Make specified formula cells non-shared ones, and split them off from
     * their respective adjacent formula cell groups.
     *
     * @param rCells cell storage container
     * @param rRows row positions at which to unshare formula cells.
     */
    static void unshareFormulaCells(CellStoreType& rCells, std::vector<SCROW>& rRows);

    /**
     * Have all formula cells belonging to a group start listening to their
     * references.
     *
     * @param rCxt context object.
     * @param ppSharedTop memory position of the pointer of the topmost
     *                    formula cell instance in the cell storage.  The
     *                    caller is responsible for ensuring that it is indeed
     *                    the topmost cell of a shared formula group.
     */
    static void startListeningAsGroup( StartListeningContext& rCxt, ScFormulaCell** ppSharedTop );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
