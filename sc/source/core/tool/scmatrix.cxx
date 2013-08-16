/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include "scmatrix.hxx"
#include "global.hxx"
#include "address.hxx"
#include "formula/errorcodes.hxx"
#include "interpre.hxx"
#include "mtvelements.hxx"

#include <svl/zforlist.hxx>
#include <tools/stream.hxx>
#include <rtl/math.hxx>

#include <math.h>

#include <vector>

#include <mdds/multi_type_matrix.hpp>
#include <mdds/multi_type_vector_types.hpp>
#include <mdds/multi_type_vector_trait.hpp>

#if DEBUG_MATRIX
#include <iostream>
using std::cout;
using std::endl;
#endif

using ::std::pair;
using ::std::for_each;
using ::std::count_if;
using ::std::advance;
using ::std::unary_function;

/**
 * Custom string trait struct to tell mdds::multi_type_matrix about the
 * custom string type and how to handle blocks storing them.
 */
struct custom_string_trait
{
    typedef OUString string_type;
    typedef sc::string_block string_element_block;

    static const mdds::mtv::element_t string_type_identifier = sc::element_type_string;

    typedef mdds::mtv::custom_block_func1<sc::string_block> element_block_func;
};

// ============================================================================

namespace {

typedef mdds::multi_type_matrix<custom_string_trait> MatrixImplType;

struct ElemEqualZero : public unary_function<double, bool>
{
    bool operator() (double val) const
    {
        return val == 0.0;
    }
};

struct ElemNotEqualZero : public unary_function<double, bool>
{
    bool operator() (double val) const
    {
        return val != 0.0;
    }
};

struct ElemGreaterZero : public unary_function<double, bool>
{
    bool operator() (double val) const
    {
        return val > 0.0;
    }
};

struct ElemLessZero : public unary_function<double, bool>
{
    bool operator() (double val) const
    {
        return val < 0.0;
    }
};

struct ElemGreaterEqualZero : public unary_function<double, bool>
{
    bool operator() (double val) const
    {
        return val >= 0.0;
    }
};

struct ElemLessEqualZero : public unary_function<double, bool>
{
    bool operator() (double val) const
    {
        return val <= 0.0;
    }
};

template<typename _Comp>
void compareMatrix(MatrixImplType& rMat)
{
    MatrixImplType::size_pair_type aDim = rMat.size();
    MatrixImplType aNewMat(aDim.row, aDim.column, false); // initialize with boolean block.  faster this way.

    _Comp aComp;
    for (size_t i = 0; i < aDim.row; ++i)
    {
        for (size_t j = 0; j < aDim.column; ++j)
        {
            MatrixImplType::const_position_type aPos = rMat.position(i, j);
            mdds::mtm::element_t eType = rMat.get_type(aPos);
            if (eType != mdds::mtm::element_numeric && eType != mdds::mtm::element_boolean)
                // must be of numeric type (boolean can be numeric).
                continue;

            double fVal = rMat.get_numeric(aPos);
            if (!::rtl::math::isFinite(fVal))
                /* FIXME: this silently skips an error instead of propagating it! */
                continue;

            bool b = aComp(fVal);
            aNewMat.set(i, j, b);
        }
    }
    aNewMat.swap(rMat);
}

}

class ScMatrixImpl
{
    MatrixImplType maMat;
    MatrixImplType maMatFlag;
    ScInterpreter* pErrorInterpreter;
    bool            mbCloneIfConst; // Whether the matrix is cloned with a CloneIfConst() call.

    ScMatrixImpl();
    ScMatrixImpl(const ScMatrixImpl&);
public:
    ScMatrixImpl(SCSIZE nC, SCSIZE nR);
    ScMatrixImpl(SCSIZE nC, SCSIZE nR, double fInitVal);
    ~ScMatrixImpl();

    void Clear();
    void SetImmutable(bool bVal);
    bool IsImmutable() const;
    void Resize(SCSIZE nC, SCSIZE nR);
    void Resize(SCSIZE nC, SCSIZE nR, double fVal);
    void SetErrorInterpreter( ScInterpreter* p);
    ScInterpreter* GetErrorInterpreter() const { return pErrorInterpreter; }

    void GetDimensions( SCSIZE& rC, SCSIZE& rR) const;
    SCSIZE GetElementCount() const;
    bool ValidColRow( SCSIZE nC, SCSIZE nR) const;
    bool ValidColRowReplicated( SCSIZE & rC, SCSIZE & rR ) const;
    bool ValidColRowOrReplicated( SCSIZE & rC, SCSIZE & rR ) const;
    void SetErrorAtInterpreter( sal_uInt16 nError ) const;

    void PutDouble(double fVal, SCSIZE nC, SCSIZE nR);
    void PutDouble( double fVal, SCSIZE nIndex);
    void PutDouble(const double* pArray, size_t nLen, SCSIZE nC, SCSIZE nR);

    void PutString(const OUString& rStr, SCSIZE nC, SCSIZE nR);
    void PutString(const OUString& rStr, SCSIZE nIndex);
    void PutString(const OUString* pArray, size_t nLen, SCSIZE nC, SCSIZE nR);

    void PutEmpty(SCSIZE nC, SCSIZE nR);
    void PutEmptyPath(SCSIZE nC, SCSIZE nR);
    void PutError( sal_uInt16 nErrorCode, SCSIZE nC, SCSIZE nR );
    void PutBoolean(bool bVal, SCSIZE nC, SCSIZE nR);
    sal_uInt16 GetError( SCSIZE nC, SCSIZE nR) const;
    double GetDouble(SCSIZE nC, SCSIZE nR) const;
    double GetDouble( SCSIZE nIndex) const;
    OUString GetString(SCSIZE nC, SCSIZE nR) const;
    OUString GetString( SCSIZE nIndex) const;
    OUString GetString( SvNumberFormatter& rFormatter, SCSIZE nC, SCSIZE nR) const;
    ScMatrixValue Get(SCSIZE nC, SCSIZE nR) const;
    bool IsString( SCSIZE nIndex ) const;
    bool IsString( SCSIZE nC, SCSIZE nR ) const;
    bool IsEmpty( SCSIZE nC, SCSIZE nR ) const;
    bool IsEmptyPath( SCSIZE nC, SCSIZE nR ) const;
    bool IsValue( SCSIZE nIndex ) const;
    bool IsValue( SCSIZE nC, SCSIZE nR ) const;
    bool IsValueOrEmpty( SCSIZE nC, SCSIZE nR ) const;
    bool IsBoolean( SCSIZE nC, SCSIZE nR ) const;
    bool IsNumeric() const;
    void MatCopy(ScMatrixImpl& mRes) const;
    void MatTrans(ScMatrixImpl& mRes) const;
    void FillDouble( double fVal, SCSIZE nC1, SCSIZE nR1, SCSIZE nC2, SCSIZE nR2 );
    void CompareEqual();
    void CompareNotEqual();
    void CompareLess();
    void CompareGreater();
    void CompareLessEqual();
    void CompareGreaterEqual();
    double And() const;
    double Or() const;
    double Xor() const;

    ScMatrix::IterateResult Sum(bool bTextAsZero) const;
    ScMatrix::IterateResult SumSquare(bool bTextAsZero) const;
    ScMatrix::IterateResult Product(bool bTextAsZero) const;
    size_t Count(bool bCountStrings) const;
    size_t MatchDoubleInColumns(double fValue, size_t nCol1, size_t nCol2) const;
    size_t MatchStringInColumns(const OUString& rStr, size_t nCol1, size_t nCol2) const;

    double GetMaxValue( bool bTextAsZero ) const;
    double GetMinValue( bool bTextAsZero ) const;
    void GetDoubleArray( std::vector<double>& rArray ) const;
    void MergeDoubleArray( std::vector<double>& rArray, ScMatrix::Op eOp ) const;

#if DEBUG_MATRIX
    void Dump() const;
#endif

private:
    void CalcPosition(SCSIZE nIndex, SCSIZE& rC, SCSIZE& rR) const;
};

ScMatrixImpl::ScMatrixImpl(SCSIZE nC, SCSIZE nR) :
    maMat(nR, nC), maMatFlag(nR, nC), pErrorInterpreter(NULL), mbCloneIfConst(true) {}

ScMatrixImpl::ScMatrixImpl(SCSIZE nC, SCSIZE nR, double fInitVal) :
    maMat(nR, nC, fInitVal), maMatFlag(nR, nC), pErrorInterpreter(NULL), mbCloneIfConst(true) {}

ScMatrixImpl::~ScMatrixImpl()
{
    Clear();
}

void ScMatrixImpl::Clear()
{
    maMat.clear();
    maMatFlag.clear();
}

void ScMatrixImpl::SetImmutable(bool bVal)
{
    mbCloneIfConst = bVal;
}

bool ScMatrixImpl::IsImmutable() const
{
    return mbCloneIfConst;
}

void ScMatrixImpl::Resize(SCSIZE nC, SCSIZE nR)
{
    maMat.resize(nR, nC);
    maMatFlag.resize(nR, nC);
}

void ScMatrixImpl::Resize(SCSIZE nC, SCSIZE nR, double fVal)
{
    maMat.resize(nR, nC, fVal);
    maMatFlag.resize(nR, nC);
}

void ScMatrixImpl::SetErrorInterpreter( ScInterpreter* p)
{
    pErrorInterpreter = p;
}

void ScMatrixImpl::GetDimensions( SCSIZE& rC, SCSIZE& rR) const
{
    MatrixImplType::size_pair_type aSize = maMat.size();
    rR = aSize.row;
    rC = aSize.column;
}

SCSIZE ScMatrixImpl::GetElementCount() const
{
    MatrixImplType::size_pair_type aSize = maMat.size();
    return aSize.row * aSize.column;
}

bool ScMatrixImpl::ValidColRow( SCSIZE nC, SCSIZE nR) const
{
    MatrixImplType::size_pair_type aSize = maMat.size();
    return nR < aSize.row && nC < aSize.column;
}

bool ScMatrixImpl::ValidColRowReplicated( SCSIZE & rC, SCSIZE & rR ) const
{
    MatrixImplType::size_pair_type aSize = maMat.size();
    if (aSize.column == 1 && aSize.row == 1)
    {
        rC = 0;
        rR = 0;
        return true;
    }
    else if (aSize.column == 1 && rR < aSize.row)
    {
        // single column matrix.
        rC = 0;
        return true;
    }
    else if (aSize.row == 1 && rC < aSize.column)
    {
        // single row matrix.
        rR = 0;
        return true;
    }
    return false;
}

bool ScMatrixImpl::ValidColRowOrReplicated( SCSIZE & rC, SCSIZE & rR ) const
{
    return ValidColRow( rC, rR) || ValidColRowReplicated( rC, rR);
}

void ScMatrixImpl::SetErrorAtInterpreter( sal_uInt16 nError ) const
{
    if ( pErrorInterpreter )
        pErrorInterpreter->SetError( nError);
}

void ScMatrixImpl::PutDouble(double fVal, SCSIZE nC, SCSIZE nR)
{
    if (ValidColRow( nC, nR))
        maMat.set(nR, nC, fVal);
    else
    {
        OSL_FAIL("ScMatrixImpl::PutDouble: dimension error");
    }
}

void ScMatrixImpl::PutDouble(const double* pArray, size_t nLen, SCSIZE nC, SCSIZE nR)
{
    if (ValidColRow( nC, nR))
        maMat.set(nR, nC, pArray, pArray + nLen);
    else
    {
        OSL_FAIL("ScMatrixImpl::PutDouble: dimension error");
    }
}

void ScMatrixImpl::PutDouble( double fVal, SCSIZE nIndex)
{
    SCSIZE nC, nR;
    CalcPosition(nIndex, nC, nR);
    PutDouble(fVal, nC, nR);
}

void ScMatrixImpl::PutString(const OUString& rStr, SCSIZE nC, SCSIZE nR)
{
    if (ValidColRow( nC, nR))
        maMat.set(nR, nC, rStr);
    else
    {
        OSL_FAIL("ScMatrixImpl::PutString: dimension error");
    }
}

void ScMatrixImpl::PutString(const OUString* pArray, size_t nLen, SCSIZE nC, SCSIZE nR)
{
    if (ValidColRow( nC, nR))
        maMat.set(nR, nC, pArray, pArray + nLen);
    else
    {
        OSL_FAIL("ScMatrixImpl::PutString: dimension error");
    }
}

void ScMatrixImpl::PutString(const OUString& rStr, SCSIZE nIndex)
{
    SCSIZE nC, nR;
    CalcPosition(nIndex, nC, nR);
    PutString(rStr, nC, nR);
}

void ScMatrixImpl::PutEmpty(SCSIZE nC, SCSIZE nR)
{
    if (ValidColRow( nC, nR))
    {
        maMat.set_empty(nR, nC);
        maMatFlag.set(nR, nC, false); // zero flag to indicate that this is 'empty', not 'empty path'.
    }
    else
    {
        OSL_FAIL("ScMatrixImpl::PutEmpty: dimension error");
    }
}

void ScMatrixImpl::PutEmptyPath(SCSIZE nC, SCSIZE nR)
{
    if (ValidColRow( nC, nR))
    {
        maMat.set_empty(nR, nC);
        maMatFlag.set(nR, nC, true); // non-zero flag to indicate empty 'path'.
    }
    else
    {
        OSL_FAIL("ScMatrixImpl::PutEmptyPath: dimension error");
    }
}

void ScMatrixImpl::PutError( sal_uInt16 nErrorCode, SCSIZE nC, SCSIZE nR )
{
    maMat.set(nR, nC, CreateDoubleError(nErrorCode));
}

void ScMatrixImpl::PutBoolean(bool bVal, SCSIZE nC, SCSIZE nR)
{
    if (ValidColRow( nC, nR))
        maMat.set(nR, nC, bVal);
    else
    {
        OSL_FAIL("ScMatrixImpl::PutBoolean: dimension error");
    }
}

sal_uInt16 ScMatrixImpl::GetError( SCSIZE nC, SCSIZE nR) const
{
    if (ValidColRowOrReplicated( nC, nR ))
    {
        double fVal = maMat.get_numeric(nR, nC);
        return GetDoubleErrorValue(fVal);
    }
    else
    {
        OSL_FAIL("ScMatrixImpl::GetError: dimension error");
        return errNoValue;
    }
}

double ScMatrixImpl::GetDouble(SCSIZE nC, SCSIZE nR) const
{
    if (ValidColRowOrReplicated( nC, nR ))
    {
        double fVal = maMat.get_numeric(nR, nC);
        if ( pErrorInterpreter )
        {
            sal_uInt16 nError = GetDoubleErrorValue(fVal);
            if ( nError )
                SetErrorAtInterpreter( nError);
        }
        return fVal;
    }
    else
    {
        OSL_FAIL("ScMatrixImpl::GetDouble: dimension error");
        return CreateDoubleError( errNoValue);
    }
}

double ScMatrixImpl::GetDouble( SCSIZE nIndex) const
{
    SCSIZE nC, nR;
    CalcPosition(nIndex, nC, nR);
    return GetDouble(nC, nR);
}

OUString ScMatrixImpl::GetString(SCSIZE nC, SCSIZE nR) const
{
    if (ValidColRowOrReplicated( nC, nR ))
    {
        double fErr = 0.0;
        MatrixImplType::const_position_type aPos = maMat.position(nR, nC);
        switch (maMat.get_type(aPos))
        {
            case mdds::mtm::element_string:
                return maMat.get_string(aPos);
            case mdds::mtm::element_empty:
                return EMPTY_OUSTRING;
            case mdds::mtm::element_numeric:
            case mdds::mtm::element_boolean:
                OSL_FAIL("ScMatrixImpl::GetString: access error, no string");
                fErr = maMat.get_numeric(aPos);
            default:
                OSL_FAIL("ScMatrixImpl::GetString: access error, no string");
        }
        SetErrorAtInterpreter(GetDoubleErrorValue(fErr));
    }
    else
    {
        OSL_FAIL("ScMatrixImpl::GetString: dimension error");
    }
    return EMPTY_OUSTRING;
}

OUString ScMatrixImpl::GetString( SCSIZE nIndex) const
{
    SCSIZE nC, nR;
    CalcPosition(nIndex, nC, nR);
    return GetString(nC, nR);
}

OUString ScMatrixImpl::GetString( SvNumberFormatter& rFormatter, SCSIZE nC, SCSIZE nR) const
{
    if (!ValidColRowOrReplicated( nC, nR ))
    {
        OSL_FAIL("ScMatrixImpl::GetString: dimension error");
        return OUString();
    }

    double fVal = 0.0;
    MatrixImplType::const_position_type aPos = maMat.position(nR, nC);
    switch (maMat.get_type(aPos))
    {
        case mdds::mtm::element_string:
            return maMat.get_string(aPos);
        case mdds::mtm::element_empty:
        {
            if (!maMatFlag.get<bool>(nR, nC))
                // not an empty path.
                break;

            // result of empty FALSE jump path
            sal_uLong nKey = rFormatter.GetStandardFormat( NUMBERFORMAT_LOGICAL,
                    ScGlobal::eLnge);
            OUString aStr;
            Color* pColor = NULL;
            rFormatter.GetOutputString( 0.0, nKey, aStr, &pColor);
            return aStr;
        }
        case mdds::mtm::element_numeric:
        case mdds::mtm::element_boolean:
            fVal = maMat.get_numeric(aPos);
        break;
        default:
            ;
    }

    sal_uInt16 nError = GetDoubleErrorValue(fVal);
    if (nError)
    {
        SetErrorAtInterpreter( nError);
        return ScGlobal::GetErrorString( nError);
    }

    sal_uLong nKey = rFormatter.GetStandardFormat( NUMBERFORMAT_NUMBER,
            ScGlobal::eLnge);
    OUString aStr;
    rFormatter.GetInputLineString( fVal, nKey, aStr);
    return aStr;
}

ScMatrixValue ScMatrixImpl::Get(SCSIZE nC, SCSIZE nR) const
{
    ScMatrixValue aVal;
    if (ValidColRowOrReplicated(nC, nR))
    {
        MatrixImplType::const_position_type aPos = maMat.position(nR, nC);
        mdds::mtm::element_t eType = maMat.get_type(aPos);
        switch (eType)
        {
            case mdds::mtm::element_boolean:
                aVal.nType = SC_MATVAL_BOOLEAN;
                aVal.fVal = maMat.get_boolean(aPos);
            break;
            case mdds::mtm::element_numeric:
                aVal.nType = SC_MATVAL_VALUE;
                aVal.fVal = maMat.get_numeric(aPos);
            break;
            case mdds::mtm::element_string:
                aVal.nType = SC_MATVAL_STRING;
                aVal.aStr = maMat.get_string(aPos);
            break;
            case mdds::mtm::element_empty:
                // Empty path equals empty plus flag.
                aVal.nType = maMatFlag.get<bool>(nR, nC) ? SC_MATVAL_EMPTYPATH : SC_MATVAL_EMPTY;
                aVal.fVal = 0.0;
            default:
                ;
        }
    }
    else
    {
        OSL_FAIL("ScMatrixImpl::Get: dimension error");
    }
    return aVal;
}

bool ScMatrixImpl::IsString( SCSIZE nIndex ) const
{
    SCSIZE nC, nR;
    CalcPosition(nIndex, nC, nR);
    return IsString(nC, nR);
}

bool ScMatrixImpl::IsString( SCSIZE nC, SCSIZE nR ) const
{
    ValidColRowReplicated( nC, nR );
    switch (maMat.get_type(nR, nC))
    {
        case mdds::mtm::element_empty:
        case mdds::mtm::element_string:
            return true;
        default:
            ;
    }
    return false;
}

bool ScMatrixImpl::IsEmpty( SCSIZE nC, SCSIZE nR ) const
{
    // Flag must be zero for this to be an empty element, instead of being an
    // empty path element.
    ValidColRowReplicated( nC, nR );
    return maMat.get_type(nR, nC) == mdds::mtm::element_empty && !maMatFlag.get<bool>(nR, nC);
}

bool ScMatrixImpl::IsEmptyPath( SCSIZE nC, SCSIZE nR ) const
{
    // 'Empty path' is empty plus non-zero flag.
    if (ValidColRowOrReplicated( nC, nR ))
        return maMat.get_type(nR, nC) == mdds::mtm::element_empty && maMatFlag.get<bool>(nR, nC);
    else
        return true;
}

bool ScMatrixImpl::IsValue( SCSIZE nIndex ) const
{
    SCSIZE nC, nR;
    CalcPosition(nIndex, nC, nR);
    return IsValue(nC, nR);
}

bool ScMatrixImpl::IsValue( SCSIZE nC, SCSIZE nR ) const
{
    ValidColRowReplicated(nC, nR);
    switch (maMat.get_type(nR, nC))
    {
        case mdds::mtm::element_boolean:
        case mdds::mtm::element_numeric:
            return true;
        default:
            ;
    }
    return false;
}

bool ScMatrixImpl::IsValueOrEmpty( SCSIZE nC, SCSIZE nR ) const
{
    ValidColRowReplicated(nC, nR);
    switch (maMat.get_type(nR, nC))
    {
        case mdds::mtm::element_boolean:
        case mdds::mtm::element_numeric:
        case mdds::mtm::element_empty:
            return true;
        default:
            ;
    }
    return false;
}

bool ScMatrixImpl::IsBoolean( SCSIZE nC, SCSIZE nR ) const
{
    ValidColRowReplicated( nC, nR );
    return maMat.get_type(nR, nC) == mdds::mtm::element_boolean;
}

bool ScMatrixImpl::IsNumeric() const
{
    return maMat.numeric();
}

void ScMatrixImpl::MatCopy(ScMatrixImpl& mRes) const
{
    if (maMat.size().row > mRes.maMat.size().row || maMat.size().column > mRes.maMat.size().column)
    {
        // destination matrix is not large enough.
        OSL_FAIL("ScMatrixImpl::MatCopy: dimension error");
        return;
    }

    mRes.maMat.copy(maMat);
}

void ScMatrixImpl::MatTrans(ScMatrixImpl& mRes) const
{
    mRes.maMat = maMat;
    mRes.maMat.transpose();
}

void ScMatrixImpl::FillDouble( double fVal, SCSIZE nC1, SCSIZE nR1, SCSIZE nC2, SCSIZE nR2 )
{
    if (ValidColRow( nC1, nR1) && ValidColRow( nC2, nR2))
    {
        for (SCSIZE j = nC1; j <= nC2; ++j)
        {
            // Passing value array is much faster.
            std::vector<double> aVals(nR2-nR1+1, fVal);
            maMat.set(nR1, j, aVals.begin(), aVals.end());
        }
    }
    else
    {
        OSL_FAIL("ScMatrixImpl::FillDouble: dimension error");
    }
}

void ScMatrixImpl::CompareEqual()
{
    compareMatrix<ElemEqualZero>(maMat);
}

void ScMatrixImpl::CompareNotEqual()
{
    compareMatrix<ElemNotEqualZero>(maMat);
}

void ScMatrixImpl::CompareLess()
{
    compareMatrix<ElemLessZero>(maMat);
}

void ScMatrixImpl::CompareGreater()
{
    compareMatrix<ElemGreaterZero>(maMat);
}

void ScMatrixImpl::CompareLessEqual()
{
    compareMatrix<ElemLessEqualZero>(maMat);
}

void ScMatrixImpl::CompareGreaterEqual()
{
    compareMatrix<ElemGreaterEqualZero>(maMat);
}

namespace {

struct AndEvaluator
{
    bool mbResult;
    void operate(double fVal) { mbResult &= (fVal != 0.0); }
    bool result() const { return mbResult; }
    AndEvaluator() : mbResult(true) {}
};

struct OrEvaluator
{
    bool mbResult;
    void operate(double fVal) { mbResult |= (fVal != 0.0); }
    bool result() const { return mbResult; }
    OrEvaluator() : mbResult(false) {}
};

struct XorEvaluator
{
    bool mbResult;
    void operate(double fVal) { mbResult ^= (fVal != 0.0); }
    bool result() const { return mbResult; }
    XorEvaluator() : mbResult(false) {}
};

// Do not short circuit logical operations, in case there are error values
// these need to be propagated even if the result was determined earlier.
template <typename _Evaluator>
double EvalMatrix(const MatrixImplType& rMat)
{
    _Evaluator aEval;
    size_t nRows = rMat.size().row, nCols = rMat.size().column;
    for (size_t i = 0; i < nRows; ++i)
    {
        for (size_t j = 0; j < nCols; ++j)
        {
            MatrixImplType::const_position_type aPos = rMat.position(i, j);
            mdds::mtm::element_t eType = rMat.get_type(aPos);
            if (eType != mdds::mtm::element_numeric && eType != mdds::mtm::element_boolean)
                // assuming a CompareMat this is an error
                return CreateDoubleError(errIllegalArgument);

            double fVal = rMat.get_numeric(aPos);
            if (!::rtl::math::isFinite(fVal))
                // DoubleError
                return fVal;

            aEval.operate(fVal);
        }
    }
    return aEval.result();
}

}

double ScMatrixImpl::And() const
{
    // All elements must be of value type.
    // True only if all the elements have non-zero values.
    return EvalMatrix<AndEvaluator>(maMat);
}

double ScMatrixImpl::Or() const
{
    // All elements must be of value type.
    // True if at least one element has a non-zero value.
    return EvalMatrix<OrEvaluator>(maMat);
}

double ScMatrixImpl::Xor() const
{
    // All elements must be of value type.
    // True if an odd number of elements have a non-zero value.
    return EvalMatrix<XorEvaluator>(maMat);
}

namespace {

struct SumOp
{
    static const int InitVal = 0;

    void operator() (double& rAccum, double fVal)
    {
        rAccum += fVal;
    }
};

struct SumSquareOp
{
    static const int InitVal = 0;

    void operator() (double& rAccum, double fVal)
    {
        rAccum += fVal*fVal;
    }
};

struct ProductOp
{
    static const int InitVal = 1;

    void operator() (double& rAccum, double fVal)
    {
        rAccum *= fVal;
    }
};

template<typename _Op>
class WalkElementBlocks : std::unary_function<MatrixImplType::element_block_node_type, void>
{
    _Op maOp;

    ScMatrix::IterateResult maRes;
    bool mbFirst:1;
    bool mbTextAsZero:1;
public:
    WalkElementBlocks(bool bTextAsZero) : maRes(0.0, _Op::InitVal, 0), mbFirst(true), mbTextAsZero(bTextAsZero) {}

    const ScMatrix::IterateResult& getResult() const { return maRes; }

    void operator() (const MatrixImplType::element_block_node_type& node)
    {
        switch (node.type)
        {
            case mdds::mtm::element_numeric:
            {
                mdds::mtv::numeric_element_block::const_iterator it = mdds::mtv::numeric_element_block::begin(*node.data);
                mdds::mtv::numeric_element_block::const_iterator itEnd = mdds::mtv::numeric_element_block::end(*node.data);
                for (; it != itEnd; ++it)
                {
                    if (mbFirst)
                    {
                        maOp(maRes.mfFirst, *it);
                        mbFirst = false;
                    }
                    else
                        maOp(maRes.mfRest, *it);
                }
                maRes.mnCount += node.size;
            }
            break;
            case mdds::mtm::element_boolean:
            {
                mdds::mtv::boolean_element_block::const_iterator it = mdds::mtv::boolean_element_block::begin(*node.data);
                mdds::mtv::boolean_element_block::const_iterator itEnd = mdds::mtv::boolean_element_block::end(*node.data);
                for (; it != itEnd; ++it)
                {
                    if (mbFirst)
                    {
                        maOp(maRes.mfFirst, *it);
                        mbFirst = false;
                    }
                    else
                        maOp(maRes.mfRest, *it);
                }
                maRes.mnCount += node.size;
            }
            break;
            case mdds::mtm::element_string:
                if (mbTextAsZero)
                    maRes.mnCount += node.size;
            break;
            case mdds::mtm::element_empty:
            default:
                ;
        }
    }
};

class CountElements : std::unary_function<MatrixImplType::element_block_node_type, void>
{
    size_t mnCount;
    bool mbCountString;
public:
    CountElements(bool bCountString) : mnCount(0), mbCountString(bCountString) {}

    size_t getCount() const { return mnCount; }

    void operator() (const MatrixImplType::element_block_node_type& node)
    {
        switch (node.type)
        {
            case mdds::mtm::element_numeric:
            case mdds::mtm::element_boolean:
                mnCount += node.size;
            break;
            case mdds::mtm::element_string:
                if (mbCountString)
                    mnCount += node.size;
            break;
            case mdds::mtm::element_empty:
            default:
                ;
        }
    }
};


template<typename vType>
class WalkAndMatchElements : std::unary_function<MatrixImplType::element_block_node_type, void>
{
    vType maMatchValue;
    MatrixImplType::size_pair_type maSize;
    size_t mnCol1, mnCol2;
    size_t mnResult, mnIndex;

public:
    WalkAndMatchElements(vType aMatchValue, const MatrixImplType::size_pair_type& aSize, size_t nCol1, size_t nCol2)
        : maMatchValue(aMatchValue), maSize(aSize), mnCol1(nCol1), mnCol2(nCol2), mnResult(size_t_MAX), mnIndex(0) {}

    size_t getMatching() const { return mnResult; }

    size_t compare(const MatrixImplType::element_block_node_type& node) const;

    void operator() (const MatrixImplType::element_block_node_type& node)
    {
        // early exit if match aleady found
        if (mnResult != SCSIZE_MAX)
            return;

        // limit lookup to the requested columns
        if (mnIndex >= ( mnCol1 * maSize.row ) &&
            mnIndex <= ( ( mnCol2 + 1 ) * maSize.row ) )
        {
            mnResult = compare(node);
        }

        mnIndex += node.size;
    }
};

template<>
size_t WalkAndMatchElements<double>::compare(const MatrixImplType::element_block_node_type& node) const {
    size_t nCount = 0;
    switch (node.type)
    {
        case mdds::mtm::element_numeric:
        {
            mdds::mtv::numeric_element_block::const_iterator it = mdds::mtv::numeric_element_block::begin(*node.data);
            mdds::mtv::numeric_element_block::const_iterator itEnd = mdds::mtv::numeric_element_block::end(*node.data);
            for (; it != itEnd; ++it, nCount++)
            {
                if (*it == maMatchValue)
                {
                    return mnIndex + nCount;
                }
            }
            break;
        }
        case mdds::mtm::element_boolean:
        {
            mdds::mtv::boolean_element_block::const_iterator it = mdds::mtv::boolean_element_block::begin(*node.data);
            mdds::mtv::boolean_element_block::const_iterator itEnd = mdds::mtv::boolean_element_block::end(*node.data);
            for (; it != itEnd; ++it, nCount++)
            {
                if (*it == maMatchValue)
                {
                    return mnIndex + nCount;
                }
            }
            break;
        }
        break;
        case mdds::mtm::element_string:
        case mdds::mtm::element_empty:
        default:
            ;
    }
    return SCSIZE_MAX;
}

template<>
size_t WalkAndMatchElements<OUString>::compare(const MatrixImplType::element_block_node_type& node) const {
    size_t nCount = 0;
    switch (node.type)
    {
        case mdds::mtm::element_string:
        {
            MatrixImplType::string_block_type::const_iterator it = MatrixImplType::string_block_type::begin(*node.data);
            MatrixImplType::string_block_type::const_iterator itEnd = MatrixImplType::string_block_type::end(*node.data);
            for (; it != itEnd; ++it, ++nCount)
            {
                if (ScGlobal::GetpTransliteration()->isEqual(*it, maMatchValue))
                {
                    return mnIndex + nCount;
                }
            }
            break;
        }
        case mdds::mtm::element_boolean:
        case mdds::mtm::element_numeric:
        case mdds::mtm::element_empty:
        default:
            ;
    }
    return SCSIZE_MAX;
}


struct MaxOp
{
    static double init() { return -std::numeric_limits<double>::max(); }
    static double compare(double left, double right)
    {
        return std::max(left, right);
    }

    static double boolValue(
        mdds::mtv::boolean_element_block::const_iterator it,
        mdds::mtv::boolean_element_block::const_iterator itEnd)
    {
        // If the array has at least one true value, the maximum value is 1.
        it = std::find(it, itEnd, true);
        return it == itEnd ? 0.0 : 1.0;
    }
};

struct MinOp
{
    static double init() { return std::numeric_limits<double>::max(); }
    static double compare(double left, double right)
    {
        return std::min(left, right);
    }

    static double boolValue(
        mdds::mtv::boolean_element_block::const_iterator it,
        mdds::mtv::boolean_element_block::const_iterator itEnd)
    {
        // If the array has at least one false value, the minimum value is 0.
        it = std::find(it, itEnd, false);
        return it == itEnd ? 1.0 : 0.0;
    }
};

template<typename _Op>
class CalcMaxMinValue : std::unary_function<MatrixImplType::element_block_type, void>
{
    double mfVal;
    bool mbTextAsZero;
public:
    CalcMaxMinValue( bool bTextAsZero ) :
        mfVal(_Op::init()),
        mbTextAsZero(bTextAsZero) {}

    double getValue() const { return mfVal; }

    void operator() (const MatrixImplType::element_block_node_type& node)
    {
        using namespace mdds::mtv;

        switch (node.type)
        {
            case mdds::mtm::element_numeric:
            {
                numeric_element_block::const_iterator it = numeric_element_block::begin(*node.data);
                numeric_element_block::const_iterator itEnd = numeric_element_block::end(*node.data);
                for (; it != itEnd; ++it)
                    mfVal = _Op::compare(mfVal, *it);
            }
            break;
            case mdds::mtm::element_boolean:
            {
                boolean_element_block::const_iterator it = boolean_element_block::begin(*node.data);
                boolean_element_block::const_iterator itEnd = boolean_element_block::end(*node.data);
                double fVal = _Op::boolValue(it, itEnd);
                mfVal = _Op::compare(mfVal, fVal);
            }
            break;
            case mdds::mtm::element_string:
            case mdds::mtm::element_empty:
            {
                // empty elements are treated as empty strings.
                if (mbTextAsZero)
                    mfVal = _Op::compare(mfVal, 0.0);
            }
            break;
            default:
                ;
        }
    }
};

class ToDoubleArray : std::unary_function<MatrixImplType::element_block_type, void>
{
    std::vector<double> maArray;
    std::vector<double>::iterator miPos;
    double mfNaN;
public:
    ToDoubleArray(size_t nSize) : maArray(nSize, 0.0), miPos(maArray.begin())
    {
        rtl::math::setNan(&mfNaN);
    }

    void operator() (const MatrixImplType::element_block_node_type& node)
    {
        using namespace mdds::mtv;

        switch (node.type)
        {
            case mdds::mtm::element_numeric:
            {
                numeric_element_block::const_iterator it = numeric_element_block::begin(*node.data);
                numeric_element_block::const_iterator itEnd = numeric_element_block::end(*node.data);
                for (; it != itEnd; ++it, ++miPos)
                    *miPos = *it;
            }
            break;
            case mdds::mtm::element_boolean:
            {
                boolean_element_block::const_iterator it = boolean_element_block::begin(*node.data);
                boolean_element_block::const_iterator itEnd = boolean_element_block::end(*node.data);
                for (; it != itEnd; ++it, ++miPos)
                    *miPos = *it ? 1.0 : 0.0;
            }
            break;
            case mdds::mtm::element_string:
            {
                for (size_t i = 0; i < node.size; ++i, ++miPos)
                    *miPos = mfNaN;
            }
            break;
            case mdds::mtm::element_empty:
                std::advance(miPos, node.size);
            default:
                ;
        }
    }

    void swap(std::vector<double>& rOther)
    {
        maArray.swap(rOther);
    }
};

struct ArrayMul : public std::binary_function<double, double, double>
{
    double operator() (const double& lhs, const double& rhs) const
    {
        return lhs * rhs;
    }
};

template<typename _Op>
class MergeDoubleArrayFunc : std::unary_function<MatrixImplType::element_block_type, void>
{
    std::vector<double>& mrArray;
    std::vector<double>::iterator miPos;
    double mfNaN;
public:
    MergeDoubleArrayFunc(std::vector<double>& rArray) : mrArray(rArray), miPos(mrArray.begin())
    {
        rtl::math::setNan(&mfNaN);
    }

    void operator() (const MatrixImplType::element_block_node_type& node)
    {
        using namespace mdds::mtv;
        static _Op op;

        switch (node.type)
        {
            case mdds::mtm::element_numeric:
            {
                numeric_element_block::const_iterator it = numeric_element_block::begin(*node.data);
                numeric_element_block::const_iterator itEnd = numeric_element_block::end(*node.data);
                for (; it != itEnd; ++it, ++miPos)
                {
                    if (rtl::math::isNan(*miPos))
                        continue;

                    *miPos = op(*miPos, *it);
                }
            }
            break;
            case mdds::mtm::element_boolean:
            {
                boolean_element_block::const_iterator it = boolean_element_block::begin(*node.data);
                boolean_element_block::const_iterator itEnd = boolean_element_block::end(*node.data);
                for (; it != itEnd; ++it, ++miPos)
                {
                    if (rtl::math::isNan(*miPos))
                        continue;

                    *miPos = op(*miPos, *it ? 1.0 : 0.0);
                }
            }
            break;
            case mdds::mtm::element_string:
            {
                for (size_t i = 0; i < node.size; ++i, ++miPos)
                    *miPos = mfNaN;
            }
            break;
            case mdds::mtm::element_empty:
                std::advance(miPos, node.size);
            default:
                ;
        }
    }
};

}

ScMatrix::IterateResult ScMatrixImpl::Sum(bool bTextAsZero) const
{
    WalkElementBlocks<SumOp> aFunc(bTextAsZero);
    maMat.walk(aFunc);
    return aFunc.getResult();
}

ScMatrix::IterateResult ScMatrixImpl::SumSquare(bool bTextAsZero) const
{
    WalkElementBlocks<SumSquareOp> aFunc(bTextAsZero);
    maMat.walk(aFunc);
    return aFunc.getResult();
}

ScMatrix::IterateResult ScMatrixImpl::Product(bool bTextAsZero) const
{
    WalkElementBlocks<ProductOp> aFunc(bTextAsZero);
    maMat.walk(aFunc);
    ScMatrix::IterateResult aRes = aFunc.getResult();
    return aRes;
}

size_t ScMatrixImpl::Count(bool bCountStrings) const
{
    CountElements aFunc(bCountStrings);
    maMat.walk(aFunc);
    return aFunc.getCount();
}

size_t ScMatrixImpl::MatchDoubleInColumns(double fValue, size_t nCol1, size_t nCol2) const
{
    WalkAndMatchElements<double> aFunc(fValue, maMat.size(), nCol1, nCol2);
    maMat.walk(aFunc);
    return aFunc.getMatching();
}

size_t ScMatrixImpl::MatchStringInColumns(const OUString& rStr, size_t nCol1, size_t nCol2) const
{
    WalkAndMatchElements<OUString> aFunc(rStr, maMat.size(), nCol1, nCol2);
    maMat.walk(aFunc);
    return aFunc.getMatching();
}

double ScMatrixImpl::GetMaxValue( bool bTextAsZero ) const
{
    CalcMaxMinValue<MaxOp> aFunc(bTextAsZero);
    maMat.walk(aFunc);
    return aFunc.getValue();
}

double ScMatrixImpl::GetMinValue( bool bTextAsZero ) const
{
    CalcMaxMinValue<MinOp> aFunc(bTextAsZero);
    maMat.walk(aFunc);
    return aFunc.getValue();
}

void ScMatrixImpl::GetDoubleArray( std::vector<double>& rArray ) const
{
    MatrixImplType::size_pair_type aSize = maMat.size();
    ToDoubleArray aFunc(aSize.row*aSize.column);
    maMat.walk(aFunc);
    aFunc.swap(rArray);
}

void ScMatrixImpl::MergeDoubleArray( std::vector<double>& rArray, ScMatrix::Op eOp ) const
{
    MatrixImplType::size_pair_type aSize = maMat.size();
    size_t nSize = aSize.row*aSize.column;
    if (nSize != rArray.size())
        return;

    switch (eOp)
    {
        case ScMatrix::Mul:
        {
            MergeDoubleArrayFunc<ArrayMul> aFunc(rArray);
            maMat.walk(aFunc);
        }
        break;
        default:
            ;
    }
}

#if DEBUG_MATRIX
void ScMatrixImpl::Dump() const
{
    cout << "-- matrix content" << endl;
    SCSIZE nCols, nRows;
    GetDimensions(nCols, nRows);
    for (SCSIZE nRow = 0; nRow < nRows; ++nRow)
    {
        for (SCSIZE nCol = 0; nCol < nCols; ++nCol)
        {
            cout << "  row=" << nRow << ", col=" << nCol << " : ";
            switch (maMat.get_type(nRow, nCol))
            {
                case mdds::mtm::element_string:
                    cout << "string (" << maMat.get_string(nRow, nCol) << ")";
                break;
                case mdds::mtm::element_numeric:
                    cout << "numeric (" << maMat.get_numeric(nRow, nCol) << ")";
                break;
                case mdds::mtm::element_boolean:
                    cout << "boolean (" << maMat.get_boolean(nRow, nCol) << ")";
                break;
                case mdds::mtm::element_empty:
                    cout << "empty";
                break;
                default:
                    ;
            }

            cout << endl;
        }
    }
}
#endif

void ScMatrixImpl::CalcPosition(SCSIZE nIndex, SCSIZE& rC, SCSIZE& rR) const
{
    SCSIZE nRowSize = maMat.size().row;
    SAL_WARN_IF( !nRowSize, "sc", "ScMatrixImpl::CalcPosition: 0 rows!");
    rC = nRowSize > 1 ? nIndex / nRowSize : nIndex;
    rR = nIndex - rC*nRowSize;
}

// ============================================================================

void ScMatrix::IncRef() const
{
    ++nRefCnt;
}

void ScMatrix::DecRef() const
{
    --nRefCnt;
    if (nRefCnt == 0)
        delete this;
}

ScMatrix::ScMatrix( SCSIZE nC, SCSIZE nR) :
    pImpl(new ScMatrixImpl(nC, nR)), nRefCnt(0)
{
    SAL_WARN_IF( !nC, "sc", "ScMatrix with 0 columns!");
    SAL_WARN_IF( !nR, "sc", "ScMatrix with 0 rows!");
}

ScMatrix::ScMatrix(SCSIZE nC, SCSIZE nR, double fInitVal) :
    pImpl(new ScMatrixImpl(nC, nR, fInitVal)), nRefCnt(0)
{
    SAL_WARN_IF( !nC, "sc", "ScMatrix with 0 columns!");
    SAL_WARN_IF( !nR, "sc", "ScMatrix with 0 rows!");
}

ScMatrix::~ScMatrix()
{
    delete pImpl;
}

ScMatrix* ScMatrix::Clone() const
{
    SCSIZE nC, nR;
    pImpl->GetDimensions(nC, nR);
    ScMatrix* pScMat = new ScMatrix(nC, nR);
    MatCopy(*pScMat);
    pScMat->SetErrorInterpreter(pImpl->GetErrorInterpreter());    // TODO: really?
    return pScMat;
}

ScMatrix* ScMatrix::CloneIfConst()
{
    return pImpl->IsImmutable() ? Clone() : this;
}

void ScMatrix::SetImmutable( bool bVal )
{
    pImpl->SetImmutable(bVal);
}

void ScMatrix::Resize( SCSIZE nC, SCSIZE nR)
{
    pImpl->Resize(nC, nR);
}

void ScMatrix::Resize(SCSIZE nC, SCSIZE nR, double fVal)
{
    pImpl->Resize(nC, nR, fVal);
}

ScMatrix* ScMatrix::CloneAndExtend(SCSIZE nNewCols, SCSIZE nNewRows) const
{
    ScMatrix* pScMat = new ScMatrix(nNewCols, nNewRows);
    MatCopy(*pScMat);
    pScMat->SetErrorInterpreter(pImpl->GetErrorInterpreter());
    return pScMat;
}

void ScMatrix::SetErrorInterpreter( ScInterpreter* p)
{
    pImpl->SetErrorInterpreter(p);
}

void ScMatrix::GetDimensions( SCSIZE& rC, SCSIZE& rR) const
{
    pImpl->GetDimensions(rC, rR);
}

SCSIZE ScMatrix::GetElementCount() const
{
    return pImpl->GetElementCount();
}

bool ScMatrix::ValidColRow( SCSIZE nC, SCSIZE nR) const
{
    return pImpl->ValidColRow(nC, nR);
}

bool ScMatrix::ValidColRowReplicated( SCSIZE & rC, SCSIZE & rR ) const
{
    return pImpl->ValidColRowReplicated(rC, rR);
}

bool ScMatrix::ValidColRowOrReplicated( SCSIZE & rC, SCSIZE & rR ) const
{
    return ValidColRow( rC, rR) || ValidColRowReplicated( rC, rR);
}

void ScMatrix::PutDouble(double fVal, SCSIZE nC, SCSIZE nR)
{
    pImpl->PutDouble(fVal, nC, nR);
}

void ScMatrix::PutDouble( double fVal, SCSIZE nIndex)
{
    pImpl->PutDouble(fVal, nIndex);
}

void ScMatrix::PutDouble(const double* pArray, size_t nLen, SCSIZE nC, SCSIZE nR)
{
    pImpl->PutDouble(pArray, nLen, nC, nR);
}

void ScMatrix::PutString(const OUString& rStr, SCSIZE nC, SCSIZE nR)
{
    pImpl->PutString(rStr, nC, nR);
}

void ScMatrix::PutString(const OUString& rStr, SCSIZE nIndex)
{
    pImpl->PutString(rStr, nIndex);
}

void ScMatrix::PutString(const OUString* pArray, size_t nLen, SCSIZE nC, SCSIZE nR)
{
    pImpl->PutString(pArray, nLen, nC, nR);
}

void ScMatrix::PutEmpty(SCSIZE nC, SCSIZE nR)
{
    pImpl->PutEmpty(nC, nR);
}

void ScMatrix::PutEmptyPath(SCSIZE nC, SCSIZE nR)
{
    pImpl->PutEmptyPath(nC, nR);
}

void ScMatrix::PutError( sal_uInt16 nErrorCode, SCSIZE nC, SCSIZE nR )
{
    pImpl->PutError(nErrorCode, nC, nR);
}

void ScMatrix::PutBoolean(bool bVal, SCSIZE nC, SCSIZE nR)
{
    pImpl->PutBoolean(bVal, nC, nR);
}

sal_uInt16 ScMatrix::GetError( SCSIZE nC, SCSIZE nR) const
{
    return pImpl->GetError(nC, nR);
}

double ScMatrix::GetDouble(SCSIZE nC, SCSIZE nR) const
{
    return pImpl->GetDouble(nC, nR);
}

double ScMatrix::GetDouble( SCSIZE nIndex) const
{
    return pImpl->GetDouble(nIndex);
}

OUString ScMatrix::GetString(SCSIZE nC, SCSIZE nR) const
{
    return pImpl->GetString(nC, nR);
}

OUString ScMatrix::GetString( SCSIZE nIndex) const
{
    return pImpl->GetString(nIndex);
}

OUString ScMatrix::GetString( SvNumberFormatter& rFormatter, SCSIZE nC, SCSIZE nR) const
{
    return pImpl->GetString(rFormatter, nC, nR);
}

ScMatrixValue ScMatrix::Get(SCSIZE nC, SCSIZE nR) const
{
    return pImpl->Get(nC, nR);
}

sal_Bool ScMatrix::IsString( SCSIZE nIndex ) const
{
    return pImpl->IsString(nIndex);
}

sal_Bool ScMatrix::IsString( SCSIZE nC, SCSIZE nR ) const
{
    return pImpl->IsString(nC, nR);
}

sal_Bool ScMatrix::IsEmpty( SCSIZE nC, SCSIZE nR ) const
{
    return pImpl->IsEmpty(nC, nR);
}

sal_Bool ScMatrix::IsEmptyPath( SCSIZE nC, SCSIZE nR ) const
{
    return pImpl->IsEmptyPath(nC, nR);
}

sal_Bool ScMatrix::IsValue( SCSIZE nIndex ) const
{
    return pImpl->IsValue(nIndex);
}

sal_Bool ScMatrix::IsValue( SCSIZE nC, SCSIZE nR ) const
{
    return pImpl->IsValue(nC, nR);
}

sal_Bool ScMatrix::IsValueOrEmpty( SCSIZE nC, SCSIZE nR ) const
{
    return pImpl->IsValueOrEmpty(nC, nR);
}

sal_Bool ScMatrix::IsBoolean( SCSIZE nC, SCSIZE nR ) const
{
    return pImpl->IsBoolean(nC, nR);
}

sal_Bool ScMatrix::IsNumeric() const
{
    return pImpl->IsNumeric();
}

void ScMatrix::MatCopy(ScMatrix& mRes) const
{
    pImpl->MatCopy(*mRes.pImpl);
}

void ScMatrix::MatTrans(ScMatrix& mRes) const
{
    pImpl->MatTrans(*mRes.pImpl);
}

void ScMatrix::FillDouble( double fVal, SCSIZE nC1, SCSIZE nR1, SCSIZE nC2, SCSIZE nR2 )
{
    pImpl->FillDouble(fVal, nC1, nR1, nC2, nR2);
}

void ScMatrix::CompareEqual()
{
    pImpl->CompareEqual();
}

void ScMatrix::CompareNotEqual()
{
    pImpl->CompareNotEqual();
}

void ScMatrix::CompareLess()
{
    pImpl->CompareLess();
}

void ScMatrix::CompareGreater()
{
    pImpl->CompareGreater();
}

void ScMatrix::CompareLessEqual()
{
    pImpl->CompareLessEqual();
}

void ScMatrix::CompareGreaterEqual()
{
    pImpl->CompareGreaterEqual();
}

double ScMatrix::And() const
{
    return pImpl->And();
}

double ScMatrix::Or() const
{
    return pImpl->Or();
}

double ScMatrix::Xor() const
{
    return pImpl->Xor();
}

ScMatrix::IterateResult ScMatrix::Sum(bool bTextAsZero) const
{
    return pImpl->Sum(bTextAsZero);
}

ScMatrix::IterateResult ScMatrix::SumSquare(bool bTextAsZero) const
{
    return pImpl->SumSquare(bTextAsZero);
}

ScMatrix::IterateResult ScMatrix::Product(bool bTextAsZero) const
{
    return pImpl->Product(bTextAsZero);
}

size_t ScMatrix::Count(bool bCountStrings) const
{
    return pImpl->Count(bCountStrings);
}

size_t ScMatrix::MatchDoubleInColumns(double fValue, size_t nCol1, size_t nCol2) const {
    return pImpl->MatchDoubleInColumns(fValue, nCol1, nCol2);
}

size_t ScMatrix::MatchStringInColumns(const OUString& rStr, size_t nCol1, size_t nCol2) const {
    return pImpl->MatchStringInColumns(rStr, nCol1, nCol2);
}

double ScMatrix::GetMaxValue( bool bTextAsZero ) const
{
    return pImpl->GetMaxValue(bTextAsZero);
}

double ScMatrix::GetMinValue( bool bTextAsZero ) const
{
    return pImpl->GetMinValue(bTextAsZero);
}

void ScMatrix::GetDoubleArray( std::vector<double>& rArray ) const
{
    pImpl->GetDoubleArray(rArray);
}

void ScMatrix::MergeDoubleArray( std::vector<double>& rArray, Op eOp ) const
{
    pImpl->MergeDoubleArray(rArray, eOp);
}

#if DEBUG_MATRIX
void ScMatrix::Dump() const
{
    pImpl->Dump();
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

