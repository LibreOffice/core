/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <mtvcellfunc.hxx>

namespace sc {

CellStoreType::iterator ProcessFormula(
    const CellStoreType::iterator& it, CellStoreType& rStore, SCROW nRow1, SCROW nRow2,
    std::function<void(size_t,ScFormulaCell*)> aFuncElem )
{
    using FuncType = std::function<void(size_t,ScFormulaCell*)>;
    using ElseFuncType = std::function<void(mdds::mtv::element_t, size_t, size_t)>;

    // empty function for handling the 'else' part.
    static ElseFuncType aFuncElse =
        [](mdds::mtv::element_t,size_t,size_t) {};

    return ProcessElements1<
        CellStoreType, formula_block,
        FuncType, ElseFuncType>(
            it, rStore, nRow1, nRow2, aFuncElem, aFuncElse);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
