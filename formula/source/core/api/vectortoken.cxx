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

VectorRefArray::VectorRefArray() :
    mpNumericArray(nullptr),
    mpStringArray(nullptr),
    mbValid(true) {}

VectorRefArray::VectorRefArray( InitInvalid ) :
    mpNumericArray(nullptr),
    mpStringArray(nullptr),
    mbValid(false) {}

VectorRefArray::VectorRefArray( const double* pArray ) :
    mpNumericArray(pArray),
    mpStringArray(nullptr),
    mbValid(true) {}

VectorRefArray::VectorRefArray( rtl_uString** pArray ) :
    mpNumericArray(nullptr),
    mpStringArray(pArray),
    mbValid(true) {}

VectorRefArray::VectorRefArray( const double* pNumArray, rtl_uString** pStrArray ) :
    mpNumericArray(pNumArray),
    mpStringArray(pStrArray),
    mbValid(true) {}

bool VectorRefArray::isValid() const
{
    return mbValid;
}

SingleVectorRefToken::SingleVectorRefToken( const VectorRefArray& rArray, size_t nReqLength, size_t nArrayLength ) :
    FormulaToken(svSingleVectorRef, ocPush), maArray(rArray), mnRequestedLength(nReqLength), mnArrayLength(nArrayLength)
{
    SAL_INFO("formula.core", "Created SingleVectorRefToken nReqLength=" << nReqLength << " nArrayLength=" << nArrayLength);
}

FormulaToken* SingleVectorRefToken::Clone() const
{
    return new SingleVectorRefToken(maArray, mnRequestedLength, mnArrayLength);
}

const VectorRefArray& SingleVectorRefToken::GetArray() const
{
    return maArray;
}

size_t SingleVectorRefToken::GetArrayLength() const
{
    return mnArrayLength;
}

DoubleVectorRefToken::DoubleVectorRefToken(
    const std::vector<VectorRefArray>& rArrays, size_t nReqLength, size_t nArrayLength,
    size_t nRefRowSize, bool bStartFixed, bool bEndFixed ) :
    FormulaToken(svDoubleVectorRef, ocPush),
    maArrays(rArrays), mnRequestedLength(nReqLength), mnArrayLength(nArrayLength),
    mnRefRowSize(nRefRowSize), mbStartFixed(bStartFixed), mbEndFixed(bEndFixed)
{
    SAL_INFO("formula.core", "Created DoubleVectorRefToken nReqLength=" << nReqLength << " nArrayLength=" << nArrayLength);
}

FormulaToken* DoubleVectorRefToken::Clone() const
{
    return new DoubleVectorRefToken(
        maArrays, mnRequestedLength, mnArrayLength, mnRefRowSize, mbStartFixed, mbEndFixed);
}

const std::vector<VectorRefArray>& DoubleVectorRefToken::GetArrays() const
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
