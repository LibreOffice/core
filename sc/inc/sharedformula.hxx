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
            ScFormulaCell::CompareState eState = pPrev->CompareByTokenArray(*pPrev);
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
            xGroup.reset(new ScFormulaCellGroup);
            xGroup->mnStart = pPrev->aPos.Row();
            xGroup->mnLength = 2;
            xGroup->mbInvariant = (eState == ScFormulaCell::EqualInvariant);
            pPrev->SetCellGroup(xGroup);
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
     * Merge with an existing formula group (if any) located immediately above
     * if the cell at specified position is a formula cell, and its formula
     * tokens are identical to that of the above formula group.
     *
     * @param aPos position of cell to examine.
     */
    static void joinFormulaCellAbove(const CellStoreType::position_type& aPos);
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
