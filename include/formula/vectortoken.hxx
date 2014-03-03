/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_FORMULA_VECTORTOKEN_HXX
#define INCLUDED_FORMULA_VECTORTOKEN_HXX

#include <formula/token.hxx>

namespace formula {

/**
 * Single unit of vector reference consists of two physical arrays.
 *
 * <p>If the whole data array consists of only numeric values, mpStringArray
 * will be NULL, and NaN values in the numeric array represent empty
 * cells.</p>
 *
 * <p>If the whole data array consists of only string values, mpNumericArray
 * will be NULL, and NULL values in the string array represent empty
 * cells.</p>
 *
 * <p>If the data array consists of numeric and string values, then both
 * mpNumericArray and mpStringArray will be non-NULL, and a string cell will
 * be represented by a non-NULL pointer value in the string array.  If the
 * string value is NULL, check the corresponding value in the numeric array.
 * If the value in the numeric array is NaN, it's an empty cell, otherwise
 * it's a numeric cell.</p>
 */
struct FORMULA_DLLPUBLIC VectorRefArray
{
    const double* mpNumericArray;
    rtl_uString** mpStringArray;

    VectorRefArray();
    VectorRefArray( const double* pArray );
    VectorRefArray( rtl_uString** pArray );
    VectorRefArray( const double* pNumArray, rtl_uString** pStrArray );

    bool isValid() const;
};

/**
 * This token represents a single cell reference in a vectorized formula
 * calculation context.
 */
class FORMULA_DLLPUBLIC SingleVectorRefToken : public FormulaToken
{
    VectorRefArray maArray;
    size_t mnRequestedLength;
    size_t mnArrayLength;

public:
    SingleVectorRefToken( const VectorRefArray& rArray, size_t nReqLength, size_t nArrayLength );

    virtual FormulaToken* Clone() const;

    inline const VectorRefArray& GetArray() const { return maArray; }
    inline size_t GetArrayLength() const { return mnArrayLength; }
};

/**
 * This token represents a range reference in a vectorized formula
 * calculation context.
 */
class FORMULA_DLLPUBLIC DoubleVectorRefToken : public FormulaToken
{
    std::vector<VectorRefArray> maArrays;

    size_t mnRequestedLength; /// requested length of all arrays which include trailing empty region.
    size_t mnArrayLength; /// length of all arrays which does not include trailing empty region.
    size_t mnRefRowSize; /// original reference row size. The row size may
                         /// change as it goes down the array if either the
                         /// stard or end position is fixed.

    bool mbStartFixed:1; /// whether or not the start row position is absolute.
    bool mbEndFixed:1; /// whether or not the end row position is absolute.

public:
    DoubleVectorRefToken(
        const std::vector<VectorRefArray>& rArrays, size_t nReqLength, size_t nArrayLength,
        size_t nRefRowSize, bool bStartFixed, bool bEndFixed );

    virtual FormulaToken* Clone() const;

    inline const std::vector<VectorRefArray>& GetArrays() const { return maArrays; }
    inline size_t GetArrayLength() const { return mnArrayLength; }
    inline size_t GetRefRowSize() const { return mnRefRowSize; }
    inline bool IsStartFixed() const { return mbStartFixed; }
    inline bool IsEndFixed() const { return mbEndFixed; }
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
