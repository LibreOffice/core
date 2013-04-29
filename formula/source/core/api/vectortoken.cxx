/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "formula/vectortoken.hxx"

namespace formula {

VectorArray::VectorArray( const double* pArray, size_t nLength ) :
    mpArray(pArray), mnLength(nLength) {}

SingleVectorRefToken::SingleVectorRefToken( const double* pArray, size_t nLength ) :
    FormulaToken(svSingleVectorRef, ocPush), maArray(pArray, nLength) {}

FormulaToken* SingleVectorRefToken::Clone() const
{
    return new SingleVectorRefToken(maArray.mpArray, maArray.mnLength);
}

const VectorArray& SingleVectorRefToken::GetArray() const
{
    return maArray;
}

DoubleVectorRefToken::DoubleVectorRefToken(
    const std::vector<VectorArray>& rArrays, size_t nRowSize, bool bAbsStart, bool bAbsEnd ) :
    FormulaToken(svDoubleVectorRef, ocPush),
    maArrays(rArrays), mnRowSize(nRowSize), mbAbsStart(bAbsStart), mbAbsEnd(bAbsEnd) {}

FormulaToken* DoubleVectorRefToken::Clone() const
{
    return new DoubleVectorRefToken(maArrays, mnRowSize, mbAbsStart, mbAbsEnd);
}

const std::vector<VectorArray>& DoubleVectorRefToken::GetArrays() const
{
    return maArrays;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
