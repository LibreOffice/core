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

VectorRefArray::VectorRefArray() : mpNumericArray(NULL), mpStringArray(NULL) {}
VectorRefArray::VectorRefArray( const double* pArray ) : mpNumericArray(pArray), mpStringArray(NULL) {}
VectorRefArray::VectorRefArray( rtl_uString** pArray ) : mpNumericArray(NULL), mpStringArray(pArray) {}
VectorRefArray::VectorRefArray( const double* pNumArray, rtl_uString** pStrArray ) :
    mpNumericArray(pNumArray), mpStringArray(pStrArray) {}

bool VectorRefArray::isValid() const
{
    return mpNumericArray || mpStringArray;
}

SingleVectorRefToken::SingleVectorRefToken( const double* pArray, size_t nReqLength, size_t nArrayLength ) :
    FormulaToken(svSingleVectorRef, ocPush), maArray(pArray), mnRequestedLength(nReqLength), mnArrayLength(nArrayLength) {}

SingleVectorRefToken::SingleVectorRefToken( const VectorRefArray& rArray, size_t nReqLength, size_t nArrayLength ) :
    FormulaToken(svSingleVectorRef, ocPush), maArray(rArray), mnRequestedLength(nReqLength), mnArrayLength(nArrayLength) {}

FormulaToken* SingleVectorRefToken::Clone() const
{
    return new SingleVectorRefToken(maArray, mnRequestedLength, mnArrayLength);
}

const VectorRefArray& SingleVectorRefToken::GetArray() const
{
    return maArray;
}

size_t SingleVectorRefToken::GetRequestedArrayLength() const
{
    return mnRequestedLength;
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
    mnRefRowSize(nRefRowSize), mbStartFixed(bStartFixed), mbEndFixed(bEndFixed) {}

FormulaToken* DoubleVectorRefToken::Clone() const
{
    return new DoubleVectorRefToken(
        maArrays, mnRequestedLength, mnArrayLength, mnRefRowSize, mbStartFixed, mbEndFixed);
}

const std::vector<VectorRefArray>& DoubleVectorRefToken::GetArrays() const
{
    return maArrays;
}

size_t DoubleVectorRefToken::GetRequestedArrayLength() const
{
    return mnRequestedLength;
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
