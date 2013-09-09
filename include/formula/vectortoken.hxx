/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef FORMULA_VECTORTOKEN_HXX
#define FORMULA_VECTORTOKEN_HXX

#include "formula/token.hxx"

namespace formula {

struct FORMULA_DLLPUBLIC VectorRefArray
{
    union {
        const double* mpNumericArray;
        rtl_uString** mpStringArray;
    };

    bool mbNumeric;

    VectorRefArray();
    VectorRefArray( const double* pArray );
    VectorRefArray( rtl_uString** pArray );
};

/**
 * This token represents a single cell reference in a vectorized formula
 * calculation context.
 */
class FORMULA_DLLPUBLIC SingleVectorRefToken : public FormulaToken
{
    VectorRefArray maArray;
    size_t mnArrayLength;

public:
    SingleVectorRefToken( const double* pArray, size_t nLength );
    SingleVectorRefToken( const VectorRefArray& rArray, size_t nLength );

    virtual FormulaToken* Clone() const;

    const VectorRefArray& GetArray() const;
    size_t GetArrayLength() const;
};

/**
 * This token represents a range reference in a vectorized formula
 * calculation context.
 */
class FORMULA_DLLPUBLIC DoubleVectorRefToken : public FormulaToken
{
    std::vector<VectorRefArray> maArrays;

    size_t mnArrayLength; /// length of all arrays.
    size_t mnRefRowSize; /// original reference row size. The row size may
                         /// change as it goes down the array if either the
                         /// stard or end position is fixed.

    bool mbStartFixed:1; /// whether or not the start row position is absolute.
    bool mbEndFixed:1; /// whether or not the end row position is absolute.

public:
    DoubleVectorRefToken(
        const std::vector<VectorRefArray>& rArrays, size_t nArrayLength, size_t nRefRowSize, bool bStartFixed, bool bEndFixed );

    virtual FormulaToken* Clone() const;

    const std::vector<VectorRefArray>& GetArrays() const;
    size_t GetArrayLength() const;
    size_t GetRefRowSize() const;
    bool IsStartFixed() const;
    bool IsEndFixed() const;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
