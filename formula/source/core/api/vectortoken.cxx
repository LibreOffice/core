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

SingleVectorRefToken::SingleVectorRefToken( const double* pArray, size_t nLength ) :
    FormulaToken(svSingleVectorRef, ocPush), mpArray(pArray), mnArrayLength(nLength) {}

FormulaToken* SingleVectorRefToken::Clone() const
{
    return new SingleVectorRefToken(mpArray, mnArrayLength);
}

const double* SingleVectorRefToken::GetArray() const
{
    return mpArray;
}

size_t SingleVectorRefToken::GetArrayLength() const
{
    return mnArrayLength;
}

DoubleVectorRefToken::DoubleVectorRefToken(
    const std::vector<const double*>& rArrays, size_t nArrayLength, size_t nRefRowSize, bool bStartFixed, bool bEndFixed ) :
    FormulaToken(svDoubleVectorRef, ocPush),
    maArrays(rArrays), mnArrayLength(nArrayLength), mnRefRowSize(nRefRowSize), mbStartFixed(bStartFixed), mbEndFixed(bEndFixed) {}

FormulaToken* DoubleVectorRefToken::Clone() const
{
    return new DoubleVectorRefToken(maArrays, mnArrayLength, mnRefRowSize, mbStartFixed, mbEndFixed);
}

const std::vector<const double*>& DoubleVectorRefToken::GetArrays() const
{
    return maArrays;
}

size_t DoubleVectorRefToken::GetArrayLength() const
{
    return mnArrayLength;
}

size_t DoubleVectorRefToken::GetRefRowSize() const
{
    return mnRefRowSize;
}

bool DoubleVectorRefToken::IsStartFixed() const
{
    return mbStartFixed;
}

bool DoubleVectorRefToken::IsEndFixed() const
{
    return mbEndFixed;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
