/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#include <tools/debug.hxx>

#include "scmatrix.hxx"
#include "global.hxx"
#include "address.hxx"
#include "formula/errorcodes.hxx"
#include "interpre.hxx"
#include <svl/zforlist.hxx>
#include <tools/stream.hxx>
#include <rtl/math.hxx>

#include <math.h>

#define MDDS_HASH_CONTAINER_BOOST 1
#include <mdds/mixed_type_matrix.hpp>

using ::std::pair;
using ::std::for_each;
using ::std::count_if;
using ::std::advance;
using ::std::unary_function;
using ::mdds::matrix_element_t;

// ============================================================================

namespace {

typedef ::mdds::mixed_type_matrix<String, sal_uInt8> MatrixImplType;

struct ElemEqual : public unary_function<double, bool>
{
    bool operator() (double val) const
    {
        return val == 0.0;
    }
};

struct ElemNotEqual : public unary_function<double, bool>
{
    bool operator() (double val) const
    {
        return val != 0.0;
    }
};

struct ElemGreater : public unary_function<double, bool>
{
    bool operator() (double val) const
    {
        return val > 0.0;
    }
};

struct ElemLess : public unary_function<double, bool>
{
    bool operator() (double val) const
    {
        return val < 0.0;
    }
};

struct ElemGreaterEqual : public unary_function<double, bool>
{
    bool operator() (double val) const
    {
        return val >= 0.0;
    }
};

struct ElemLessEqual : public unary_function<double, bool>
{
    bool operator() (double val) const
    {
        return val <= 0.0;
    }
};

template<typename _Comp>
void compareMatrix(MatrixImplType& rMat)
{
    pair<size_t,size_t> aDim = rMat.size();
    _Comp aComp;
    for (size_t i = 0; i < aDim.first; ++i)
    {
        for (size_t j = 0; j < aDim.second; ++j)
        {
            matrix_element_t eType = rMat.get_type(i, j);
            if (eType != mdds::element_numeric && eType == mdds::element_boolean)
                continue;

            double fVal = rMat.get_numeric(i, j);
            if (!::rtl::math::isFinite(fVal))
                continue;

            rMat.set_boolean(i, j, aComp(fVal));
        }
    }
}

::mdds::matrix_density_t toMddsDensityType(ScMatrix::DensityType eType)
{
    switch (eType)
    {
        case ScMatrix::FILLED_EMPTY:
            return mdds::matrix_density_filled_empty;
        case ScMatrix::FILLED_ZERO:
            return mdds::matrix_density_filled_zero;
        case ScMatrix::SPARSE_EMPTY:
            return mdds::matrix_density_sparse_empty;
        case ScMatrix::SPARSE_ZERO:
            return mdds::matrix_density_sparse_zero;
        default:
            ;
    }

    // default density type
    return mdds::matrix_density_filled_zero;
}

/**
 * Return a numeric value from a matrix element no matter what its type is.
 */
double getNumericValue(const MatrixImplType::element& elem)
{
    switch (elem.m_type)
    {
        case mdds::element_boolean:
            return static_cast<double>(elem.m_boolean);
        case mdds::element_numeric:
            return elem.m_numeric;
        default:
            ;
    }
    return 0.0;
}

}

class ScMatrixImpl
{
    MatrixImplType maMat;
    ScMatrix::DensityType meType;
    ScInterpreter* pErrorInterpreter;
    bool            mbCloneIfConst; // Whether the matrix is cloned with a CloneIfConst() call.

    ScMatrixImpl();
    ScMatrixImpl(const ScMatrixImpl&);
public:
    ScMatrixImpl(SCSIZE nC, SCSIZE nR, ScMatrix::DensityType eType);
    ~ScMatrixImpl();

    void Clear();
    void SetImmutable(bool bVal);
    bool IsImmutable() const;
    void Resize(SCSIZE nC, SCSIZE nR);
    ScMatrix::DensityType GetDensityType() const;
    void SetErrorInterpreter( ScInterpreter* p);
    ScInterpreter* GetErrorInterpreter() const { return pErrorInterpreter; }

    void GetDimensions( SCSIZE& rC, SCSIZE& rR) const;
    SCSIZE GetElementCount() const;
    bool ValidColRow( SCSIZE nC, SCSIZE nR) const;
    SCSIZE CalcOffset( SCSIZE nC, SCSIZE nR) const;
    bool ValidColRowReplicated( SCSIZE & rC, SCSIZE & rR ) const;
    bool ValidColRowOrReplicated( SCSIZE & rC, SCSIZE & rR ) const;
    void SetErrorAtInterpreter( sal_uInt16 nError ) const;
    void PutDouble(double fVal, SCSIZE nC, SCSIZE nR);
    void PutDouble( double fVal, SCSIZE nIndex);
    void PutString(const String& rStr, SCSIZE nC, SCSIZE nR);
    void PutString(const String& rStr, SCSIZE nIndex);

    void PutEmpty(SCSIZE nC, SCSIZE nR);
    void PutEmptyPath(SCSIZE nC, SCSIZE nR);
    void PutError( sal_uInt16 nErrorCode, SCSIZE nC, SCSIZE nR );
    void PutBoolean(bool bVal, SCSIZE nC, SCSIZE nR);
    sal_uInt16 GetError( SCSIZE nC, SCSIZE nR) const;
    double GetDouble(SCSIZE nC, SCSIZE nR) const;
    double GetDouble( SCSIZE nIndex) const;
    const String& GetString(SCSIZE nC, SCSIZE nR) const;
    const String& GetString( SCSIZE nIndex) const;
    String GetString( SvNumberFormatter& rFormatter, SCSIZE nC, SCSIZE nR) const;
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

    ScMatrix::IterateResult Sum(bool bTextAsZero) const;
    ScMatrix::IterateResult SumSquare(bool bTextAsZero) const;
    ScMatrix::IterateResult Product(bool bTextAsZero) const;
    size_t Count(bool bCountStrings) const;

private:
    void CalcPosition(SCSIZE nIndex, SCSIZE& rC, SCSIZE& rR) const;
};

ScMatrixImpl::ScMatrixImpl(SCSIZE nC, SCSIZE nR, ScMatrix::DensityType eType) :
    maMat(nR, nC, toMddsDensityType(eType)),
    meType(eType),
    mbCloneIfConst(true)
{
}

ScMatrixImpl::~ScMatrixImpl()
{
    Clear();
}

void ScMatrixImpl::Clear()
{
    maMat.clear();
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
}

ScMatrix::DensityType ScMatrixImpl::GetDensityType() const
{
    return meType;
}

void ScMatrixImpl::SetErrorInterpreter( ScInterpreter* p)
{
    pErrorInterpreter = p;
}

void ScMatrixImpl::GetDimensions( SCSIZE& rC, SCSIZE& rR) const
{
    MatrixImplType::size_pair_type aDims = maMat.size();
    rR = aDims.first;
    rC = aDims.second;
}

SCSIZE ScMatrixImpl::GetElementCount() const
{
    MatrixImplType::size_pair_type aDims = maMat.size();
    return aDims.first * aDims.second;
}

bool ScMatrixImpl::ValidColRow( SCSIZE nC, SCSIZE nR) const
{
    MatrixImplType::size_pair_type aDims = maMat.size();
    return nR < aDims.first && nC < aDims.second;
}

SCSIZE ScMatrixImpl::CalcOffset( SCSIZE nC, SCSIZE nR) const
{
    return nC * maMat.size().first + nR;
}

bool ScMatrixImpl::ValidColRowReplicated( SCSIZE & rC, SCSIZE & rR ) const
{
    pair<size_t, size_t> aDims = maMat.size();

    if (aDims.second == 1 && aDims.first == 1)
    {
        rC = 0;
        rR = 0;
        return true;
    }
    else if (aDims.second == 1 && rR < aDims.first)
    {
        // single column matrix.
        rC = 0;
        return true;
    }
    else if (aDims.first == 1 && rC < aDims.second)
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
        maMat.set_numeric(nR, nC, fVal);
    else
    {
        DBG_ERRORFILE("ScMatrixImpl::PutDouble: dimension error");
    }
}

void ScMatrixImpl::PutDouble( double fVal, SCSIZE nIndex)
{
    SCSIZE nC, nR;
    CalcPosition(nIndex, nC, nR);
    PutDouble(fVal, nC, nR);
}

void ScMatrixImpl::PutString(const String& rStr, SCSIZE nC, SCSIZE nR)
{
    if (ValidColRow( nC, nR))
        maMat.set_string(nR, nC, new String(rStr));
    else
    {
        DBG_ERRORFILE("ScMatrixImpl::PutString: dimension error");
    }
}

void ScMatrixImpl::PutString(const String& rStr, SCSIZE nIndex)
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
        maMat.clear_flag(nR, nC); // zero flag to indicate that this is 'empty', not 'empty path'.
    }
    else
    {
        DBG_ERRORFILE("ScMatrixImpl::PutEmpty: dimension error");
    }
}

void ScMatrixImpl::PutEmptyPath(SCSIZE nC, SCSIZE nR)
{
    if (ValidColRow( nC, nR))
    {
        maMat.set_empty(nR, nC);
        maMat.set_flag(nR, nC, 1); // non-zero flag to indicate empty 'path'.
    }
    else
    {
        DBG_ERRORFILE("ScMatrixImpl::PutEmptyPath: dimension error");
    }
}

void ScMatrixImpl::PutError( sal_uInt16 nErrorCode, SCSIZE nC, SCSIZE nR )
{
    maMat.set_numeric(nR, nC, CreateDoubleError(nErrorCode));
}

void ScMatrixImpl::PutBoolean(bool bVal, SCSIZE nC, SCSIZE nR)
{
    if (ValidColRow( nC, nR))
        maMat.set_boolean(nR, nC, bVal);
    else
    {
        DBG_ERRORFILE("ScMatrixImpl::PutBoolean: dimension error");
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
        DBG_ERRORFILE("ScMatrixImpl::GetError: dimension error");
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
        DBG_ERRORFILE("ScMatrixImpl::GetDouble: dimension error");
        return CreateDoubleError( errNoValue);
    }
}

double ScMatrixImpl::GetDouble( SCSIZE nIndex) const
{
    SCSIZE nC, nR;
    CalcPosition(nIndex, nC, nR);
    return GetDouble(nC, nR);
}

const String& ScMatrixImpl::GetString(SCSIZE nC, SCSIZE nR) const
{
    if (ValidColRowOrReplicated( nC, nR ))
    {
        if (maMat.get_type(nR, nC) == ::mdds::element_string)
            return *maMat.get_string(nR, nC);
        else
        {
            SetErrorAtInterpreter( GetError(nC, nR));
            DBG_ERRORFILE("ScMatrixImpl::GetString: access error, no string");
        }
    }
    else
    {
        DBG_ERRORFILE("ScMatrixImpl::GetString: dimension error");
    }
    return ScGlobal::GetEmptyString();
}

const String& ScMatrixImpl::GetString( SCSIZE nIndex) const
{
    SCSIZE nC, nR;
    CalcPosition(nIndex, nC, nR);
    return GetString(nC, nR);
}

String ScMatrixImpl::GetString( SvNumberFormatter& rFormatter, SCSIZE nC, SCSIZE nR) const
{
    if (!ValidColRowOrReplicated( nC, nR ))
    {
        DBG_ERRORFILE("ScMatrixImpl::GetString: dimension error");
        return String();
    }

    if (IsString( nC, nR))
    {
        if (IsEmptyPath( nC, nR))
        {   // result of empty FALSE jump path
            sal_uLong nKey = rFormatter.GetStandardFormat( NUMBERFORMAT_LOGICAL,
                    ScGlobal::eLnge);
            String aStr;
            Color* pColor = NULL;
            rFormatter.GetOutputString( 0.0, nKey, aStr, &pColor);
            return aStr;
        }
        return GetString( nC, nR);
    }

    sal_uInt16 nError = GetError( nC, nR);
    if (nError)
    {
        SetErrorAtInterpreter( nError);
        return ScGlobal::GetErrorString( nError);
    }

    double fVal= GetDouble( nC, nR);
    sal_uLong nKey = rFormatter.GetStandardFormat( NUMBERFORMAT_NUMBER,
            ScGlobal::eLnge);
    String aStr;
    rFormatter.GetInputLineString( fVal, nKey, aStr);
    return aStr;
}

ScMatrixValue ScMatrixImpl::Get(SCSIZE nC, SCSIZE nR) const
{
    ScMatrixValue aVal;
    if (ValidColRowOrReplicated(nC, nR))
    {
        matrix_element_t eType = maMat.get_type(nR, nC);
        switch (eType)
        {
            case mdds::element_boolean:
                aVal.nType = SC_MATVAL_BOOLEAN;
                aVal.fVal = maMat.get_boolean(nR, nC);
            break;
            case mdds::element_numeric:
                aVal.nType = SC_MATVAL_VALUE;
                aVal.fVal = maMat.get_numeric(nR, nC);
            break;
            case mdds::element_string:
                aVal.nType = SC_MATVAL_STRING;
                aVal.pS = maMat.get_string(nR, nC);
            break;
            case mdds::element_empty:
                // Empty path equals empty plus flag.
                aVal.nType = maMat.get_flag(nR, nC) ? SC_MATVAL_EMPTYPATH : SC_MATVAL_EMPTY;
                aVal.fVal = 0.0;
            default:
                ;
        }
    }
    else
    {
        DBG_ERRORFILE("ScMatrixImpl::Get: dimension error");
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
        case mdds::element_empty:
        case mdds::element_string:
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
    return maMat.get_type(nR, nC) == ::mdds::element_empty && maMat.get_flag(nR, nC) == 0;
}

bool ScMatrixImpl::IsEmptyPath( SCSIZE nC, SCSIZE nR ) const
{
    // 'Empty path' is empty plus non-zero flag.
    ValidColRowReplicated( nC, nR );
    return maMat.get_type(nR, nC) == ::mdds::element_empty && maMat.get_flag(nR, nC) != 0;
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
        case mdds::element_boolean:
        case mdds::element_numeric:
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
        case mdds::element_boolean:
        case mdds::element_numeric:
        case mdds::element_empty:
            return true;
        default:
            ;
    }
    return false;
}

bool ScMatrixImpl::IsBoolean( SCSIZE nC, SCSIZE nR ) const
{
    ValidColRowReplicated( nC, nR );
    return maMat.get_type(nR, nC) == ::mdds::element_boolean;
}

bool ScMatrixImpl::IsNumeric() const
{
    return maMat.numeric();
}

void ScMatrixImpl::MatCopy(ScMatrixImpl& mRes) const
{
    MatrixImplType::size_pair_type s1 = maMat.size(), s2 = mRes.maMat.size();
    if (s1.first > s2.first || s1.second > s2.second)
    {
        // destination matrix is not large enough.
        DBG_ERRORFILE("ScMatrixImpl::MatCopy: dimension error");
        return;
    }

    mRes.maMat.assign(maMat);
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
        for (SCSIZE i = nR1; i <= nR2; ++i)
            for (SCSIZE j = nC1; j <= nC2; ++j)
                maMat.set(i, j, fVal);
    }
    else
    {
        DBG_ERRORFILE("ScMatrixImpl::FillDouble: dimension error");
    }
}

void ScMatrixImpl::CompareEqual()
{
    compareMatrix<ElemEqual>(maMat);
}

void ScMatrixImpl::CompareNotEqual()
{
    compareMatrix<ElemNotEqual>(maMat);
}

void ScMatrixImpl::CompareLess()
{
    compareMatrix<ElemLess>(maMat);
}

void ScMatrixImpl::CompareGreater()
{
    compareMatrix<ElemGreater>(maMat);
}

void ScMatrixImpl::CompareLessEqual()
{
    compareMatrix<ElemLessEqual>(maMat);
}

void ScMatrixImpl::CompareGreaterEqual()
{
    compareMatrix<ElemGreaterEqual>(maMat);
}

namespace {

struct AndEvaluator
{
    bool isBadElem(double fVal) const { return fVal == 0; }
    bool returnOnElem() const { return false; }
    bool returnOnAllElems() const { return true; }
};

struct OrEvaluator
{
    bool isBadElem(double fVal) const { return fVal != 0; }
    bool returnOnElem() const { return true; }
    bool returnOnAllElems() const { return false; }
};

template <typename _Evaluator>
bool EvalMatrix(const MatrixImplType& rMat)
{
    _Evaluator aEval;
    pair<size_t,size_t> aDim = rMat.size();
    size_t nRows = aDim.first, nCols = aDim.second;
    for (size_t i = 0; i < nRows; ++i)
    {
        for (size_t j = 0; j < nCols; ++j)
        {
            matrix_element_t eType = rMat.get_type(i, j);
            if (eType != mdds::element_numeric && eType == mdds::element_boolean)
                // assuming a CompareMat this is an error
                return CreateDoubleError(errIllegalArgument);

            double fVal = rMat.get_numeric(i, j);
            if (!::rtl::math::isFinite(fVal))
                // DoubleError
                return fVal;

            if (aEval.isBadElem(fVal))
                return aEval.returnOnElem();
        }
    }
    return aEval.returnOnAllElems();
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

namespace {

/**
 * Function object to sum all numeric elements (including boolean).  It
 * stores the first non-zero element value into maRes.mfFirst while the rest
 * into maRes.mfRest.  This weird requirement comes from
 * ScInterpreter::IterateParameters.
 */
class SumElements : public unary_function<void, MatrixImplType::element>
{
    ScMatrix::IterateResult maRes;
    bool mbTextAsZero;
public:
    SumElements(bool bTextAsZero) : maRes(0.0, 0.0, 0), mbTextAsZero(bTextAsZero) {}

    ScMatrix::IterateResult getResult() const { return maRes; }
    void operator() (const MatrixImplType::element& elem)
    {
        switch (elem.m_type)
        {
            case mdds::element_boolean:
                if (elem.m_boolean)
                {
                    if (maRes.mfFirst)
                        maRes.mfFirst = 1.0;
                    else
                        maRes.mfRest += 1.0;
                }
                ++maRes.mnCount;
            break;
            case mdds::element_numeric:
                if (elem.m_numeric != 0.0)
                {
                    if (maRes.mfFirst)
                        maRes.mfFirst = elem.m_numeric;
                    else
                        maRes.mfRest += elem.m_numeric;
                }
                ++maRes.mnCount;
            break;
            case mdds::element_string:
                if (mbTextAsZero)
                    ++maRes.mnCount;
            default:
                ;
        }
    }
};

class SumSquareElements : public unary_function<void, MatrixImplType::element>
{
    ScMatrix::IterateResult maRes;
    bool mbTextAsZero;
public:
    SumSquareElements(bool bTextAsZero) : maRes(0.0, 0.0, 0), mbTextAsZero(bTextAsZero) {}
    ScMatrix::IterateResult getResult() const { return maRes; }
    void operator() (const MatrixImplType::element& elem)
    {
        if (elem.m_type == ::mdds::element_empty)
            return;

        if (elem.m_type == ::mdds::element_string)
        {
            if (mbTextAsZero)
                ++maRes.mnCount;
            return;
        }

        double val = getNumericValue(elem);
        maRes.mfRest += val*val;
        ++maRes.mnCount;
    }
};

/**
 * Multiply all boolean and numeric elements.  It skips empty elements, and
 * optionally string elements if specified.  When text as zero option is
 * specified, it treats string elements as if they have values of zero.
 */
class MultiplyElements : public unary_function<void, MatrixImplType::element>
{
    ScMatrix::IterateResult maRes;
    bool mbTextAsZero;
public:
    MultiplyElements(bool bTextAsZero) : maRes(0.0, 1.0, 0), mbTextAsZero(bTextAsZero) {}
    ScMatrix::IterateResult getResult() const { return maRes; }

    void operator() (const MatrixImplType::element& elem)
    {
        if (elem.m_type == ::mdds::element_string)
        {
            ++maRes.mnCount;
            if (mbTextAsZero)
                maRes.mfRest = 0.0;
        }
        else if (elem.m_type != ::mdds::element_empty)
        {
            ++maRes.mnCount;
            maRes.mfRest *= getNumericValue(elem);
        }
    }
};

/**
 * Predicate for counting only boolean, numeric, and optionally string
 * elements.
 */
class CountNonEmptyElements : public unary_function<bool, MatrixImplType::element>
{
    const bool mbCountString;
public:
    CountNonEmptyElements(bool bCountString) : mbCountString(bCountString) {}
    bool operator() (const MatrixImplType::element& elem) const
    {
        switch (elem.m_type)
        {
            case mdds::element_boolean:
            case mdds::element_numeric:
                return true;
            case mdds::element_string:
                return mbCountString;
            default:
                ;
        }
        return false;
    }
};

}

ScMatrix::IterateResult ScMatrixImpl::Sum(bool bTextAsZero) const
{
    return for_each(maMat.begin(), maMat.end(), SumElements(bTextAsZero)).getResult();
}

ScMatrix::IterateResult ScMatrixImpl::SumSquare(bool bTextAsZero) const
{
    return for_each(maMat.begin(), maMat.end(), SumSquareElements(bTextAsZero)).getResult();
}

ScMatrix::IterateResult ScMatrixImpl::Product(bool bTextAsZero) const
{
    return for_each(maMat.begin(), maMat.end(), MultiplyElements(bTextAsZero)).getResult();
}

size_t ScMatrixImpl::Count(bool bCountStrings) const
{
    return count_if(maMat.begin(), maMat.end(), CountNonEmptyElements(bCountStrings));
}

void ScMatrixImpl::CalcPosition(SCSIZE nIndex, SCSIZE& rC, SCSIZE& rR) const
{
    SCSIZE nRowSize = maMat.size().first;
    rC = nIndex / nRowSize;
    rR = nIndex - rC*nRowSize;
}

// ============================================================================

ScMatrix::ScMatrix( SCSIZE nC, SCSIZE nR, DensityType eType) :
    pImpl(new ScMatrixImpl(nC, nR, eType)),
    nRefCnt(0)
{
}
ScMatrix::~ScMatrix()
{
    delete pImpl;
}

ScMatrix* ScMatrix::Clone() const
{
    return Clone(GetDensityType());
}

ScMatrix* ScMatrix::Clone( DensityType eType) const
{
    SCSIZE nC, nR;
    pImpl->GetDimensions(nC, nR);
    ScMatrix* pScMat = new ScMatrix(nC, nR, eType);
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

ScMatrix* ScMatrix::CloneAndExtend( SCSIZE nNewCols, SCSIZE nNewRows, DensityType eType ) const
{
    ScMatrix* pScMat = new ScMatrix( nNewCols, nNewRows, eType);
    MatCopy(*pScMat);
    pScMat->SetErrorInterpreter(pImpl->GetErrorInterpreter());
    return pScMat;
}

ScMatrix::DensityType ScMatrix::GetDensityType() const
{
    return pImpl->GetDensityType();
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

SCSIZE ScMatrix::CalcOffset( SCSIZE nC, SCSIZE nR) const
{
    return pImpl->CalcOffset(nC, nR);
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

void ScMatrix::PutString(const String& rStr, SCSIZE nC, SCSIZE nR)
{
    pImpl->PutString(rStr, nC, nR);
}

void ScMatrix::PutString(const String& rStr, SCSIZE nIndex)
{
    pImpl->PutString(rStr, nIndex);
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

const String& ScMatrix::GetString(SCSIZE nC, SCSIZE nR) const
{
    return pImpl->GetString(nC, nR);
}

const String& ScMatrix::GetString( SCSIZE nIndex) const
{
    return pImpl->GetString(nIndex);
}

String ScMatrix::GetString( SvNumberFormatter& rFormatter, SCSIZE nC, SCSIZE nR) const
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
