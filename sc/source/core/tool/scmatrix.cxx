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

#define _MDDS_HASH_CONTAINER_COMPAT 1
#include <mdds/quad_type_matrix.hpp>

using ::std::pair;
using ::mdds::matrix_element_t;

// ============================================================================

namespace {

typedef ::mdds::quad_type_matrix<String, sal_uInt8> MatrixImplType;

struct ElemEqual : public ::std::unary_function<double, bool>
{
    bool operator() (double val) const
    {
        return val == 0.0;
    }
};

struct ElemNotEqual : public ::std::unary_function<double, bool>
{
    bool operator() (double val) const
    {
        return val != 0.0;
    }
};

struct ElemGreater : public ::std::unary_function<double, bool>
{
    bool operator() (double val) const
    {
        return val > 0.0;
    }
};

struct ElemLess : public ::std::unary_function<double, bool>
{
    bool operator() (double val) const
    {
        return val < 0.0;
    }
};

struct ElemGreaterEqual : public ::std::unary_function<double, bool>
{
    bool operator() (double val) const
    {
        return val >= 0.0;
    }
};

struct ElemLessEqual : public ::std::unary_function<double, bool>
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
    void SetErrorAtInterpreter( USHORT nError ) const;
    void PutDouble(double fVal, SCSIZE nC, SCSIZE nR);
    void PutDouble( double fVal, SCSIZE nIndex);
    void PutString(const String& rStr, SCSIZE nC, SCSIZE nR);
    void PutString(const String& rStr, SCSIZE nIndex);

    void PutEmpty(SCSIZE nC, SCSIZE nR);
    void PutEmptyPath(SCSIZE nC, SCSIZE nR);
    void PutEmptyPath(SCSIZE nIndex);
    void PutError( USHORT nErrorCode, SCSIZE nC, SCSIZE nR );
    void PutError( USHORT nErrorCode, SCSIZE nIndex );
    void PutBoolean(bool bVal, SCSIZE nC, SCSIZE nR);
    void PutBoolean( bool bVal, SCSIZE nIndex);
    USHORT GetError( SCSIZE nC, SCSIZE nR) const;
    USHORT GetError( SCSIZE nIndex) const;
    double GetDouble(SCSIZE nC, SCSIZE nR) const;
    double GetDouble( SCSIZE nIndex) const;
    const String& GetString(SCSIZE nC, SCSIZE nR) const;
    const String& GetString( SCSIZE nIndex) const;
    String GetString( SvNumberFormatter& rFormatter, SCSIZE nIndex) const;
    String GetString( SvNumberFormatter& rFormatter, SCSIZE nC, SCSIZE nR) const;
    ScMatrixValue Get(SCSIZE nC, SCSIZE nR) const;
    BOOL IsString( SCSIZE nIndex ) const;
    BOOL IsString( SCSIZE nC, SCSIZE nR ) const;
    BOOL IsEmpty( SCSIZE nIndex ) const;
    BOOL IsEmpty( SCSIZE nC, SCSIZE nR ) const;
    BOOL IsEmptyPath( SCSIZE nC, SCSIZE nR ) const;
    BOOL IsEmptyPath( SCSIZE nIndex ) const;
    BOOL IsValue( SCSIZE nIndex ) const;
    BOOL IsValue( SCSIZE nC, SCSIZE nR ) const;
    BOOL IsValueOrEmpty( SCSIZE nC, SCSIZE nR ) const;
    BOOL IsBoolean( SCSIZE nC, SCSIZE nR ) const;
    BOOL IsNumeric() const;
    void MatCopy(ScMatrixImpl& mRes) const;
    void MatTrans(ScMatrixImpl& mRes) const;
    void FillDouble( double fVal, SCSIZE nC1, SCSIZE nR1, SCSIZE nC2, SCSIZE nR2 );
    void CompareEqual();
    void CompareNotEqual();
    void CompareLess();
    void CompareGreater();
    void CompareLessEqual();
    void CompareGreaterEqual();
    double And();
    double Or();

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
    Clear();
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

void ScMatrixImpl::SetErrorAtInterpreter( USHORT nError ) const
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

void ScMatrixImpl::PutEmptyPath(SCSIZE nIndex)
{
    SCSIZE nC, nR;
    CalcPosition(nIndex, nC, nR);
    PutEmptyPath(nC, nR);
}

void ScMatrixImpl::PutError( USHORT nErrorCode, SCSIZE nC, SCSIZE nR )
{
    maMat.set_numeric(nR, nC, CreateDoubleError(nErrorCode));
}

void ScMatrixImpl::PutError( USHORT nErrorCode, SCSIZE nIndex )
{
    SCSIZE nC, nR;
    CalcPosition(nIndex, nC, nR);
    PutError(nErrorCode, nC, nR);
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

void ScMatrixImpl::PutBoolean( bool bVal, SCSIZE nIndex)
{
    SCSIZE nC, nR;
    CalcPosition(nIndex, nC, nR);
    PutBoolean(bVal, nC, nR);
}

USHORT ScMatrixImpl::GetError( SCSIZE nC, SCSIZE nR) const
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

USHORT ScMatrixImpl::GetError( SCSIZE nIndex) const
{
    SCSIZE nC, nR;
    CalcPosition(nIndex, nC, nR);
    return GetError(nC, nR);
}

double ScMatrixImpl::GetDouble(SCSIZE nC, SCSIZE nR) const
{
    if (ValidColRowOrReplicated( nC, nR ))
    {
        double fVal = maMat.get_numeric(nR, nC);
        if ( pErrorInterpreter )
        {
            USHORT nError = GetDoubleErrorValue(fVal);
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

String ScMatrixImpl::GetString( SvNumberFormatter& rFormatter, SCSIZE nIndex) const
{
    if (IsString( nIndex))
    {
        if (IsEmptyPath( nIndex))
        {   // result of empty FALSE jump path
            ULONG nKey = rFormatter.GetStandardFormat( NUMBERFORMAT_LOGICAL,
                    ScGlobal::eLnge);
            String aStr;
            Color* pColor = NULL;
            rFormatter.GetOutputString( 0.0, nKey, aStr, &pColor);
            return aStr;
        }
        return GetString( nIndex );
    }

    USHORT nError = GetError( nIndex);
    if (nError)
    {
        SetErrorAtInterpreter( nError);
        return ScGlobal::GetErrorString( nError);
    }

    double fVal= GetDouble( nIndex);
    ULONG nKey = rFormatter.GetStandardFormat( NUMBERFORMAT_NUMBER,
            ScGlobal::eLnge);
    String aStr;
    rFormatter.GetInputLineString( fVal, nKey, aStr);
    return aStr;
}

String ScMatrixImpl::GetString( SvNumberFormatter& rFormatter, SCSIZE nC, SCSIZE nR) const
{
    if (ValidColRowOrReplicated( nC, nR ))
    {
        SCSIZE nIndex = CalcOffset( nC, nR);
        return GetString( rFormatter, nIndex);
    }
    else
    {
        DBG_ERRORFILE("ScMatrixImpl::GetString: dimension error");
    }
    return String();
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
            default:
                ; // no action is needed for an empty element.
        }
    }
    else
    {
        DBG_ERRORFILE("ScMatrixImpl::Get: dimension error");
    }
    return aVal;
}

BOOL ScMatrixImpl::IsString( SCSIZE nIndex ) const
{
    SCSIZE nC, nR;
    CalcPosition(nIndex, nC, nR);
    return IsString(nC, nR);
}

BOOL ScMatrixImpl::IsString( SCSIZE nC, SCSIZE nR ) const
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
BOOL ScMatrixImpl::IsEmpty( SCSIZE nIndex ) const
{
    SCSIZE nC, nR;
    CalcPosition(nIndex, nC, nR);
    return IsEmpty(nC, nR);
}

BOOL ScMatrixImpl::IsEmpty( SCSIZE nC, SCSIZE nR ) const
{
    // Flag must be zero for this to be an empty element, instead of being an
    // empty path element.
    ValidColRowReplicated( nC, nR );
    return maMat.get_type(nR, nC) == ::mdds::element_empty && maMat.get_flag(nR, nC) == 0;
}

BOOL ScMatrixImpl::IsEmptyPath( SCSIZE nC, SCSIZE nR ) const
{
    // 'Empty path' is empty plus non-zero flag.
    ValidColRowReplicated( nC, nR );
    return maMat.get_type(nR, nC) == ::mdds::element_empty && maMat.get_flag(nR, nC) != 0;
}

BOOL ScMatrixImpl::IsEmptyPath( SCSIZE nIndex ) const
{
    SCSIZE nC, nR;
    CalcPosition(nIndex, nC, nR);
    return IsEmptyPath(nC, nR);
}

BOOL ScMatrixImpl::IsValue( SCSIZE nIndex ) const
{
    SCSIZE nC, nR;
    CalcPosition(nIndex, nC, nR);
    return IsValue(nC, nR);
}

BOOL ScMatrixImpl::IsValue( SCSIZE nC, SCSIZE nR ) const
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

BOOL ScMatrixImpl::IsValueOrEmpty( SCSIZE nC, SCSIZE nR ) const
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

BOOL ScMatrixImpl::IsBoolean( SCSIZE nC, SCSIZE nR ) const
{
    ValidColRowReplicated( nC, nR );
    return maMat.get_type(nR, nC) == ::mdds::element_boolean;
}

BOOL ScMatrixImpl::IsNumeric() const
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
        for (SCSIZE i = nR1; i < nR2; ++i)
            for (SCSIZE j = nC1; j < nC2; ++j)
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

double ScMatrixImpl::And()
{
    // All elements must be of value type.
    // True only if all the elements have non-zero values.
    pair<size_t,size_t> aDim = maMat.size();
    size_t nRows = aDim.first, nCols = aDim.second;
    for (size_t i = 0; i < nRows; ++i)
    {
        for (size_t j = 0; j < nCols; ++j)
        {
            matrix_element_t eType = maMat.get_type(i, j);
            if (eType != mdds::element_numeric && eType == mdds::element_boolean)
                // assuming a CompareMat this is an error
                return CreateDoubleError(errIllegalArgument);

            double fVal = maMat.get_numeric(i, j);
            if (!::rtl::math::isFinite(fVal))
                // DoubleError
                return fVal;

            if (fVal == 0.0)
                return false;
        }
    }
    return true;
}

double ScMatrixImpl::Or()
{
    // All elements must be of value type.
    // True if at least one element has a non-zero value.
    pair<size_t,size_t> aDim = maMat.size();
    size_t nRows = aDim.first, nCols = aDim.second;
    for (size_t i = 0; i < nRows; ++i)
    {
        for (size_t j = 0; j < nCols; ++j)
        {
            matrix_element_t eType = maMat.get_type(i, j);
            if (eType != mdds::element_numeric && eType == mdds::element_boolean)
                // assuming a CompareMat this is an error
                return CreateDoubleError(errIllegalArgument);

            double fVal = maMat.get_numeric(i, j);
            if (!::rtl::math::isFinite(fVal))
                // DoubleError
                return fVal;

            if (fVal != 0.0)
                return true;
        }
    }
    return false;
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
    return (pImpl->IsImmutable() || IsEternalRef()) ? Clone() : this;
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

void ScMatrix::PutEmptyPath(SCSIZE nIndex)
{
    pImpl->PutEmptyPath(nIndex);
}

void ScMatrix::PutError( USHORT nErrorCode, SCSIZE nC, SCSIZE nR )
{
    pImpl->PutError(nErrorCode, nC, nR);
}

void ScMatrix::PutError( USHORT nErrorCode, SCSIZE nIndex )
{
    pImpl->PutError(nErrorCode, nIndex);
}

void ScMatrix::PutBoolean(bool bVal, SCSIZE nC, SCSIZE nR)
{
    pImpl->PutBoolean(bVal, nC, nR);
}

void ScMatrix::PutBoolean( bool bVal, SCSIZE nIndex)
{
    pImpl->PutBoolean(bVal, nIndex);
}

USHORT ScMatrix::GetError( SCSIZE nC, SCSIZE nR) const
{
    return pImpl->GetError(nC, nR);
}

USHORT ScMatrix::GetError( SCSIZE nIndex) const
{
    return pImpl->GetError(nIndex);
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

String ScMatrix::GetString( SvNumberFormatter& rFormatter, SCSIZE nIndex) const
{
    return pImpl->GetString(rFormatter, nIndex);
}

String ScMatrix::GetString( SvNumberFormatter& rFormatter, SCSIZE nC, SCSIZE nR) const
{
    return pImpl->GetString(rFormatter, nC, nR);
}

ScMatrixValue ScMatrix::Get(SCSIZE nC, SCSIZE nR) const
{
    return pImpl->Get(nC, nR);
}

BOOL ScMatrix::IsString( SCSIZE nIndex ) const
{
    return pImpl->IsString(nIndex);
}

BOOL ScMatrix::IsString( SCSIZE nC, SCSIZE nR ) const
{
    return pImpl->IsString(nC, nR);
}

BOOL ScMatrix::IsEmpty( SCSIZE nIndex ) const
{
    return pImpl->IsEmpty(nIndex);
}

BOOL ScMatrix::IsEmpty( SCSIZE nC, SCSIZE nR ) const
{
    return pImpl->IsEmpty(nC, nR);
}

BOOL ScMatrix::IsEmptyPath( SCSIZE nC, SCSIZE nR ) const
{
    return pImpl->IsEmptyPath(nC, nR);
}

BOOL ScMatrix::IsEmptyPath( SCSIZE nIndex ) const
{
    return pImpl->IsEmptyPath(nIndex);
}

BOOL ScMatrix::IsValue( SCSIZE nIndex ) const
{
    return pImpl->IsValue(nIndex);
}

BOOL ScMatrix::IsValue( SCSIZE nC, SCSIZE nR ) const
{
    return pImpl->IsValue(nC, nR);
}

BOOL ScMatrix::IsValueOrEmpty( SCSIZE nC, SCSIZE nR ) const
{
    return pImpl->IsValueOrEmpty(nC, nR);
}

BOOL ScMatrix::IsBoolean( SCSIZE nC, SCSIZE nR ) const
{
    return pImpl->IsBoolean(nC, nR);
}

BOOL ScMatrix::IsNumeric() const
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

double ScMatrix::And()
{
    return pImpl->And();
}

double ScMatrix::Or()
{
    return pImpl->Or();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
