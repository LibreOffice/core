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

struct VectorArray
{
    const double* mpArray;
    size_t mnLength;

    VectorArray( const double* pArray, size_t nLength );
};

class FORMULA_DLLPUBLIC SingleVectorRefToken : public FormulaToken
{
    const VectorArray maArray;
public:
    SingleVectorRefToken( const double* pArray, size_t nLength );

    virtual FormulaToken* Clone() const;

    const VectorArray& GetArray() const;
};

/**
 * This token describes a range reference in a vectorized formula
 * calculation context.
 */
class FORMULA_DLLPUBLIC DoubleVectorRefToken : public FormulaToken
{
    std::vector<VectorArray> maArrays;

    size_t mnRowSize;

    bool mbAbsStart:1; /// whether or not the start row position is absolute.
    bool mbAbsEnd:1; /// whether or not the end row position is absolute.

public:
    DoubleVectorRefToken(
        const std::vector<VectorArray>& rArrays, size_t nRowSize, bool bAbsStart, bool bAbsEnd );

    virtual FormulaToken* Clone() const;

    const std::vector<VectorArray>& GetArrays() const;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
