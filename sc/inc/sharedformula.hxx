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

namespace sc {

class SharedFormulaUtil
{
public:

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
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
