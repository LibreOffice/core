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
#include <formula/errorcodes.hxx>
#include "interpre.hxx"
#include "mtvelements.hxx"
#include "compare.hxx"

#include <boost/noncopyable.hpp>
#include <svl/zforlist.hxx>
#include <svl/sharedstring.hxx>
#include <tools/stream.hxx>
#include <rtl/math.hxx>

#include <math.h>

#include <vector>
#include <limits>

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
    typedef svl::SharedString string_type;
    typedef sc::string_block string_element_block;

    static const mdds::mtv::element_t string_type_identifier = sc::element_type_string;

    typedef mdds::mtv::custom_block_func1<sc::string_block> element_block_func;
};

typedef mdds::multi_type_matrix<custom_string_trait> MatrixImplType;

namespace {

struct ElemEqualZero : public unary_function<double, double>
{
    double operator() (double val) const
    {
        if (!::rtl::math::isFinite(val))
            return val;
        return val == 0.0 ? 1.0 : 0.0;
    }
};

struct ElemNotEqualZero : public unary_function<double, double>
{
    double operator() (double val) const
    {
        if (!::rtl::math::isFinite(val))
            return val;
        return val != 0.0 ? 1.0 : 0.0;
    }
};

struct ElemGreaterZero : public unary_function<double, double>
{
    double operator() (double val) const
    {
        if (!::rtl::math::isFinite(val))
            return val;
        return val > 0.0 ? 1.0 : 0.0;
    }
};

struct ElemLessZero : public unary_function<double, double>
{
    double operator() (double val) const
    {
        if (!::rtl::math::isFinite(val))
            return val;
        return val < 0.0 ? 1.0 : 0.0;
    }
};

struct ElemGreaterEqualZero : public unary_function<double, double>
{
    double operator() (double val) const
    {
        if (!::rtl::math::isFinite(val))
            return val;
        return val >= 0.0 ? 1.0 : 0.0;
    }
};

struct ElemLessEqualZero : public unary_function<double, double>
{
    double operator() (double val) const
    {
        if (!::rtl::math::isFinite(val))
            return val;
        return val <= 0.0 ? 1.0 : 0.0;
    }
};

template<typename _Comp>
class CompareMatrixElemFunc : std::unary_function<MatrixImplType::element_block_node_type, void>
{
    static _Comp maComp;

    std::vector<double> maNewMatValues;     // double instead of bool to transport error values
    size_t mnRow;
    size_t mnCol;
public:
    CompareMatrixElemFunc( size_t nRow, size_t nCol ) : mnRow(nRow), mnCol(nCol)
    {
        maNewMatValues.reserve(nRow*nCol);
    }

    void operator() (const MatrixImplType::element_block_node_type& node)
    {
        switch (node.type)
        {
            case mdds::mtm::element_numeric:
            {
                typedef MatrixImplType::numeric_block_type block_type;

                block_type::const_iterator it = block_type::begin(*node.data);
                block_type::const_iterator itEnd = block_type::end(*node.data);
                for (; it != itEnd; ++it)
                {
                    double fVal = *it;
                    maNewMatValues.push_back(maComp(fVal));
                }
            }
            break;
            case mdds::mtm::element_boolean:
            {
                typedef MatrixImplType::boolean_block_type block_type;

                block_type::const_iterator it = block_type::begin(*node.data);
                block_type::const_iterator itEnd = block_type::end(*node.data);
                for (; it != itEnd; ++it)
                {
                    double fVal = *it ? 1.0 : 0.0;
                    maNewMatValues.push_back(maComp(fVal));
                }
            }
            break;
            case mdds::mtm::element_string:
            case mdds::mtm::element_empty:
            default:
                // Fill it with false.
                maNewMatValues.resize(maNewMatValues.size() + node.size, 0.0);
        }
    }

    void swap( MatrixImplType& rMat )
    {
        MatrixImplType aNewMat(mnRow, mnCol, maNewMatValues.begin(), maNewMatValues.end());
        rMat.swap(aNewMat);
    }
};

template<typename _Comp>
_Comp CompareMatrixElemFunc<_Comp>::maComp;

}

/* TODO: it would be good if mdds had get/set<sal_uInt8> additionally to
 * get/set<bool>, we're abusing double here. */
typedef double TMatFlag;
const TMatFlag SC_MATFLAG_EMPTYCELL   = 0.0;
const TMatFlag SC_MATFLAG_EMPTYRESULT = 1.0;
const TMatFlag SC_MATFLAG_EMPTYPATH   = 2.0;

class ScMatrixImpl: private boost::noncopyable
{
    MatrixImplType maMat;
    MatrixImplType maMatFlag;
    ScInterpreter* pErrorInterpreter;
    bool            mbCloneIfConst; // Whether the matrix is cloned with a CloneIfConst() call.

public:
    ScMatrixImpl(SCSIZE nC, SCSIZE nR);
    ScMatrixImpl(SCSIZE nC, SCSIZE nR, double fInitVal);

    ScMatrixImpl( size_t nC, size_t nR, const std::vector<double>& rInitVals );

    ~ScMatrixImpl();

    void Clear();
    void SetImmutable(bool bVal);
    bool IsImmutable() const { return mbCloneIfConst;}
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

    void PutString(const svl::SharedString& rStr, SCSIZE nC, SCSIZE nR);
    void PutString(const svl::SharedString& rStr, SCSIZE nIndex);
    void PutString(const svl::SharedString* pArray, size_t nLen, SCSIZE nC, SCSIZE nR);

    void PutEmpty(SCSIZE nC, SCSIZE nR);
    void PutEmptyPath(SCSIZE nC, SCSIZE nR);
    void PutError( sal_uInt16 nErrorCode, SCSIZE nC, SCSIZE nR );
    void PutError( sal_uInt16 nErrorCode, SCSIZE nIndex );
    void PutBoolean(bool bVal, SCSIZE nC, SCSIZE nR);
    sal_uInt16 GetError( SCSIZE nC, SCSIZE nR) const;
    double GetDouble(SCSIZE nC, SCSIZE nR) const;
    double GetDouble( SCSIZE nIndex) const;
    svl::SharedString GetString(SCSIZE nC, SCSIZE nR) const;
    svl::SharedString GetString( SCSIZE nIndex) const;
    svl::SharedString GetString( SvNumberFormatter& rFormatter, SCSIZE nC, SCSIZE nR) const;
    ScMatrixValue Get(SCSIZE nC, SCSIZE nR) const;
    bool IsString( SCSIZE nIndex ) const;
    bool IsString( SCSIZE nC, SCSIZE nR ) const;
    bool IsEmpty( SCSIZE nC, SCSIZE nR ) const;
    bool IsEmptyCell( SCSIZE nC, SCSIZE nR ) const;
    bool IsEmptyResult( SCSIZE nC, SCSIZE nR ) const;
    bool IsEmptyPath( SCSIZE nC, SCSIZE nR ) const;
    bool IsValue( SCSIZE nIndex ) const;
    bool IsValue( SCSIZE nC, SCSIZE nR ) const;
    bool IsValueOrEmpty( SCSIZE nC, SCSIZE nR ) const;
    bool IsBoolean( SCSIZE nC, SCSIZE nR ) const;
    bool IsNumeric() const;

    void MatCopy(ScMatrixImpl& mRes) const;
    void MatTrans(ScMatrixImpl& mRes) const;
    void FillDouble( double fVal, SCSIZE nC1, SCSIZE nR1, SCSIZE nC2, SCSIZE nR2 );
    void PutDoubleVector( const ::std::vector< double > & rVec, SCSIZE nC, SCSIZE nR );
    void PutStringVector( const ::std::vector< svl::SharedString > & rVec, SCSIZE nC, SCSIZE nR );
    void PutEmptyVector( SCSIZE nCount, SCSIZE nC, SCSIZE nR );
    void PutEmptyResultVector( SCSIZE nCount, SCSIZE nC, SCSIZE nR );
    void PutEmptyPathVector( SCSIZE nCount, SCSIZE nC, SCSIZE nR );
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
    size_t MatchStringInColumns(const svl::SharedString& rStr, size_t nCol1, size_t nCol2) const;

    double GetMaxValue( bool bTextAsZero ) const;
    double GetMinValue( bool bTextAsZero ) const;

    ScMatrixRef CompareMatrix( sc::Compare& rComp, size_t nMatPos, sc::CompareOptions* pOptions ) const;

    void GetDoubleArray( std::vector<double>& rArray, bool bEmptyAsZero ) const;
    void MergeDoubleArray( std::vector<double>& rArray, ScMatrix::Op eOp ) const;
    void AddValues( const ScMatrixImpl& rMat );

    template<typename T>
    void ApplyOperation(T aOp, ScMatrixImpl& rMat);

#if DEBUG_MATRIX
    void Dump() const;
#endif

private:
    void CalcPosition(SCSIZE nIndex, SCSIZE& rC, SCSIZE& rR) const;
};

ScMatrixImpl::ScMatrixImpl(SCSIZE nC, SCSIZE nR) :
    maMat(nR, nC), maMatFlag(nR, nC, SC_MATFLAG_EMPTYCELL), pErrorInterpreter(NULL), mbCloneIfConst(true) {}

ScMatrixImpl::ScMatrixImpl(SCSIZE nC, SCSIZE nR, double fInitVal) :
    maMat(nR, nC, fInitVal), maMatFlag(nR, nC), pErrorInterpreter(NULL), mbCloneIfConst(true) {}

ScMatrixImpl::ScMatrixImpl( size_t nC, size_t nR, const std::vector<double>& rInitVals ) :
    maMat(nR, nC, rInitVals.begin(), rInitVals.end()), maMatFlag(nR, nC), pErrorInterpreter(NULL), mbCloneIfConst(true) {}

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

void ScMatrixImpl::PutString(const svl::SharedString& rStr, SCSIZE nC, SCSIZE nR)
{
    if (ValidColRow( nC, nR))
        maMat.set(nR, nC, rStr);
    else
    {
        OSL_FAIL("ScMatrixImpl::PutString: dimension error");
    }
}

void ScMatrixImpl::PutString(const svl::SharedString* pArray, size_t nLen, SCSIZE nC, SCSIZE nR)
{
    if (ValidColRow( nC, nR))
        maMat.set(nR, nC, pArray, pArray + nLen);
    else
    {
        OSL_FAIL("ScMatrixImpl::PutString: dimension error");
    }
}

void ScMatrixImpl::PutString(const svl::SharedString& rStr, SCSIZE nIndex)
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
        maMatFlag.set(nR, nC, SC_MATFLAG_EMPTYCELL);
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
        maMatFlag.set(nR, nC, SC_MATFLAG_EMPTYPATH);
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

void ScMatrixImpl::PutError( sal_uInt16 nErrorCode, SCSIZE nIndex )
{
    SCSIZE nC, nR;
    CalcPosition(nIndex, nC, nR);
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

svl::SharedString ScMatrixImpl::GetString(SCSIZE nC, SCSIZE nR) const
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
                return svl::SharedString::getEmptyString();
            case mdds::mtm::element_numeric:
            case mdds::mtm::element_boolean:
                fErr = maMat.get_numeric(aPos);
                //fallthrough
            default:
                OSL_FAIL("ScMatrixImpl::GetString: access error, no string");
        }
        SetErrorAtInterpreter(GetDoubleErrorValue(fErr));
    }
    else
    {
        OSL_FAIL("ScMatrixImpl::GetString: dimension error");
    }
    return svl::SharedString::getEmptyString();
}

svl::SharedString ScMatrixImpl::GetString( SCSIZE nIndex) const
{
    SCSIZE nC, nR;
    CalcPosition(nIndex, nC, nR);
    return GetString(nC, nR);
}

svl::SharedString ScMatrixImpl::GetString( SvNumberFormatter& rFormatter, SCSIZE nC, SCSIZE nR) const
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
            return maMat.get_string(aPos).getString();
        case mdds::mtm::element_empty:
        {
            if (maMatFlag.get<TMatFlag>(nR, nC) != SC_MATFLAG_EMPTYPATH)
                // not an empty path.
                return svl::SharedString::getEmptyString();

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
                aVal.fVal = double(maMat.get_boolean(aPos));
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
                /* TODO: do we need to pass the differentiation of 'empty' and
                 * 'empty result' to the outer world anywhere? */
                aVal.nType = maMatFlag.get<TMatFlag>(nR, nC) == SC_MATFLAG_EMPTYPATH ? SC_MATVAL_EMPTYPATH :
                    SC_MATVAL_EMPTY;
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
    // Flag must indicate an 'empty' or 'empty cell' or 'empty result' element,
    // but not an 'empty path' element.
    ValidColRowReplicated( nC, nR );
    return maMat.get_type(nR, nC) == mdds::mtm::element_empty &&
        maMatFlag.get<TMatFlag>(nR, nC) != SC_MATFLAG_EMPTYPATH;
}

bool ScMatrixImpl::IsEmptyCell( SCSIZE nC, SCSIZE nR ) const
{
    // Flag must indicate an 'empty cell' element instead of an
    // 'empty' or 'empty result' or 'empty path' element.
    ValidColRowReplicated( nC, nR );
    return maMat.get_type(nR, nC) == mdds::mtm::element_empty &&
        maMatFlag.get<TMatFlag>(nR, nC) == SC_MATFLAG_EMPTYCELL;
}

bool ScMatrixImpl::IsEmptyResult( SCSIZE nC, SCSIZE nR ) const
{
    // Flag must indicate an 'empty result' element instead of an
    // 'empty' or 'empty cell' or 'empty path' element.
    ValidColRowReplicated( nC, nR );
    return maMat.get_type(nR, nC) == mdds::mtm::element_empty &&
        maMatFlag.get<TMatFlag>(nR, nC) == SC_MATFLAG_EMPTYRESULT;
}

bool ScMatrixImpl::IsEmptyPath( SCSIZE nC, SCSIZE nR ) const
{
    // Flag must indicate an 'empty path' element.
    if (ValidColRowOrReplicated( nC, nR ))
        return maMat.get_type(nR, nC) == mdds::mtm::element_empty &&
            maMatFlag.get<TMatFlag>(nR, nC) == SC_MATFLAG_EMPTYPATH;
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

void ScMatrixImpl::PutDoubleVector( const ::std::vector< double > & rVec, SCSIZE nC, SCSIZE nR )
{
    if (!rVec.empty() && ValidColRow( nC, nR) && ValidColRow( nC, nR + rVec.size() - 1))
    {
        maMat.set(nR, nC, rVec.begin(), rVec.end());
    }
    else
    {
        OSL_FAIL("ScMatrixImpl::PutDoubleVector: dimension error");
    }
}

void ScMatrixImpl::PutStringVector( const ::std::vector< svl::SharedString > & rVec, SCSIZE nC, SCSIZE nR )
{
    if (!rVec.empty() && ValidColRow( nC, nR) && ValidColRow( nC, nR + rVec.size() - 1))
    {
        maMat.set(nR, nC, rVec.begin(), rVec.end());
    }
    else
    {
        OSL_FAIL("ScMatrixImpl::PutStringVector: dimension error");
    }
}

void ScMatrixImpl::PutEmptyVector( SCSIZE nCount, SCSIZE nC, SCSIZE nR )
{
    if (nCount && ValidColRow( nC, nR) && ValidColRow( nC, nR + nCount - 1))
    {
        maMat.set_empty(nR, nC, nCount);
        // Flag to indicate that this is 'empty', not 'empty result' or 'empty path'.
        std::vector<TMatFlag> aVals(nCount, SC_MATFLAG_EMPTYCELL);
        maMatFlag.set(nR, nC, aVals.begin(), aVals.end());
    }
    else
    {
        OSL_FAIL("ScMatrixImpl::PutEmptyVector: dimension error");
    }
}

void ScMatrixImpl::PutEmptyResultVector( SCSIZE nCount, SCSIZE nC, SCSIZE nR )
{
    if (nCount && ValidColRow( nC, nR) && ValidColRow( nC, nR + nCount - 1))
    {
        maMat.set_empty(nR, nC, nCount);
        // Flag to indicate that this is 'empty result', not 'empty' or 'empty path'.
        std::vector<TMatFlag> aVals(nCount, SC_MATFLAG_EMPTYRESULT);
        maMatFlag.set(nR, nC, aVals.begin(), aVals.end());
    }
    else
    {
        OSL_FAIL("ScMatrixImpl::PutEmptyResultVector: dimension error");
    }
}

void ScMatrixImpl::PutEmptyPathVector( SCSIZE nCount, SCSIZE nC, SCSIZE nR )
{
    if (nCount && ValidColRow( nC, nR) && ValidColRow( nC, nR + nCount - 1))
    {
        maMat.set_empty(nR, nC, nCount);
        // Flag to indicate 'empty path'.
        std::vector<TMatFlag> aVals(nCount, SC_MATFLAG_EMPTYPATH);
        maMatFlag.set(nR, nC, aVals.begin(), aVals.end());
    }
    else
    {
        OSL_FAIL("ScMatrixImpl::PutEmptyPathVector: dimension error");
    }
}

void ScMatrixImpl::CompareEqual()
{
    MatrixImplType::size_pair_type aSize = maMat.size();
    CompareMatrixElemFunc<ElemEqualZero> aFunc(aSize.row, aSize.column);
    maMat.walk(aFunc);
    aFunc.swap(maMat);
}

void ScMatrixImpl::CompareNotEqual()
{
    MatrixImplType::size_pair_type aSize = maMat.size();
    CompareMatrixElemFunc<ElemNotEqualZero> aFunc(aSize.row, aSize.column);
    maMat.walk(aFunc);
    aFunc.swap(maMat);
}

void ScMatrixImpl::CompareLess()
{
    MatrixImplType::size_pair_type aSize = maMat.size();
    CompareMatrixElemFunc<ElemLessZero> aFunc(aSize.row, aSize.column);
    maMat.walk(aFunc);
    aFunc.swap(maMat);
}

void ScMatrixImpl::CompareGreater()
{
    MatrixImplType::size_pair_type aSize = maMat.size();
    CompareMatrixElemFunc<ElemGreaterZero> aFunc(aSize.row, aSize.column);
    maMat.walk(aFunc);
    aFunc.swap(maMat);
}

void ScMatrixImpl::CompareLessEqual()
{
    MatrixImplType::size_pair_type aSize = maMat.size();
    CompareMatrixElemFunc<ElemLessEqualZero> aFunc(aSize.row, aSize.column);
    maMat.walk(aFunc);
    aFunc.swap(maMat);
}

void ScMatrixImpl::CompareGreaterEqual()
{
    MatrixImplType::size_pair_type aSize = maMat.size();
    CompareMatrixElemFunc<ElemGreaterEqualZero> aFunc(aSize.row, aSize.column);
    maMat.walk(aFunc);
    aFunc.swap(maMat);
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
    static const double InitVal;

    void operator() (double& rAccum, double fVal)
    {
        rAccum += fVal;
    }
};

const double SumOp::InitVal = 0.0;

struct SumSquareOp
{
    static const double InitVal;

    void operator() (double& rAccum, double fVal)
    {
        rAccum += fVal*fVal;
    }
};

const double SumSquareOp::InitVal = 0.0;

struct ProductOp
{
    static const double InitVal;

    void operator() (double& rAccum, double fVal)
    {
        rAccum *= fVal;
    }
};

const double ProductOp::InitVal = 1.0;

template<typename _Op>
class WalkElementBlocks : std::unary_function<MatrixImplType::element_block_node_type, void>
{
    _Op maOp;

    ScMatrix::IterateResult maRes;
    bool mbFirst:1;
    bool mbTextAsZero:1;
public:
    WalkElementBlocks(bool bTextAsZero) : maRes(_Op::InitVal, _Op::InitVal, 0), mbFirst(true), mbTextAsZero(bTextAsZero) {}

    const ScMatrix::IterateResult& getResult() const { return maRes; }

    void operator() (const MatrixImplType::element_block_node_type& node)
    {
        switch (node.type)
        {
            case mdds::mtm::element_numeric:
            {
                typedef MatrixImplType::numeric_block_type block_type;

                block_type::const_iterator it = block_type::begin(*node.data);
                block_type::const_iterator itEnd = block_type::end(*node.data);
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
                typedef MatrixImplType::boolean_block_type block_type;

                block_type::const_iterator it = block_type::begin(*node.data);
                block_type::const_iterator itEnd = block_type::end(*node.data);
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

const size_t ResultNotSet = std::numeric_limits<size_t>::max();

template<typename _Type>
class WalkAndMatchElements : std::unary_function<MatrixImplType::element_block_node_type, void>
{
    _Type maMatchValue;
    MatrixImplType::size_pair_type maSize;
    size_t mnCol1;
    size_t mnCol2;
    size_t mnResult;
    size_t mnIndex;

public:
    WalkAndMatchElements(_Type aMatchValue, const MatrixImplType::size_pair_type& aSize, size_t nCol1, size_t nCol2) :
        maMatchValue(aMatchValue),
        maSize(aSize),
        mnCol1(nCol1),
        mnCol2(nCol2),
        mnResult(ResultNotSet),
        mnIndex(0) {}

    size_t getMatching() const { return mnResult; }

    size_t compare(const MatrixImplType::element_block_node_type& node) const;

    void operator() (const MatrixImplType::element_block_node_type& node)
    {
        // early exit if match already found
        if (mnResult != ResultNotSet)
            return;

        // limit lookup to the requested columns
        if ((mnCol1 * maSize.row) <= mnIndex && mnIndex < ((mnCol2 + 1) * maSize.row))
        {
            mnResult = compare(node);
        }

        mnIndex += node.size;
    }
};

template<>
size_t WalkAndMatchElements<double>::compare(const MatrixImplType::element_block_node_type& node) const
{
    size_t nCount = 0;
    switch (node.type)
    {
        case mdds::mtm::element_numeric:
        {
            typedef MatrixImplType::numeric_block_type block_type;

            block_type::const_iterator it = block_type::begin(*node.data);
            block_type::const_iterator itEnd = block_type::end(*node.data);
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
            typedef MatrixImplType::boolean_block_type block_type;

            block_type::const_iterator it = block_type::begin(*node.data);
            block_type::const_iterator itEnd = block_type::end(*node.data);
            for (; it != itEnd; ++it, ++nCount)
            {
                if (int(*it) == maMatchValue)
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
    return ResultNotSet;
}

template<>
size_t WalkAndMatchElements<svl::SharedString>::compare(const MatrixImplType::element_block_node_type& node) const
{
    switch (node.type)
    {
        case mdds::mtm::element_string:
        {
            size_t nCount = 0;
            typedef MatrixImplType::string_block_type block_type;

            block_type::const_iterator it = block_type::begin(*node.data);
            block_type::const_iterator itEnd = block_type::end(*node.data);
            for (; it != itEnd; ++it, ++nCount)
            {
                if (it->getDataIgnoreCase() == maMatchValue.getDataIgnoreCase())
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
    return ResultNotSet;
}

struct MaxOp
{
    static double init() { return -std::numeric_limits<double>::max(); }
    static double compare(double left, double right)
    {
        return std::max(left, right);
    }

    static double boolValue(
        MatrixImplType::boolean_block_type::const_iterator it,
        MatrixImplType::boolean_block_type::const_iterator itEnd)
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
        MatrixImplType::boolean_block_type::const_iterator it,
        MatrixImplType::boolean_block_type::const_iterator itEnd)
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
    bool mbHasValue;
public:
    CalcMaxMinValue( bool bTextAsZero ) :
        mfVal(_Op::init()),
        mbTextAsZero(bTextAsZero),
        mbHasValue(false) {}

    double getValue() const { return mbHasValue ? mfVal : 0.0; }

    void operator() (const MatrixImplType::element_block_node_type& node)
    {

        switch (node.type)
        {
            case mdds::mtm::element_numeric:
            {
                typedef MatrixImplType::numeric_block_type block_type;

                block_type::const_iterator it = block_type::begin(*node.data);
                block_type::const_iterator itEnd = block_type::end(*node.data);
                for (; it != itEnd; ++it)
                    mfVal = _Op::compare(mfVal, *it);

                mbHasValue = true;
            }
            break;
            case mdds::mtm::element_boolean:
            {
                typedef MatrixImplType::boolean_block_type block_type;

                block_type::const_iterator it = block_type::begin(*node.data);
                block_type::const_iterator itEnd = block_type::end(*node.data);
                double fVal = _Op::boolValue(it, itEnd);
                mfVal = _Op::compare(mfVal, fVal);
                mbHasValue = true;
            }
            break;
            case mdds::mtm::element_string:
            case mdds::mtm::element_empty:
            {
                // empty elements are treated as empty strings.
                if (mbTextAsZero)
                {
                    mfVal = _Op::compare(mfVal, 0.0);
                    mbHasValue = true;
                }
            }
            break;
            default:
                ;
        }
    }
};

inline double evaluate( double fVal, ScQueryOp eOp )
{
    if (!rtl::math::isFinite(fVal))
        return fVal;

    switch (eOp)
    {
        case SC_EQUAL:
            return fVal == 0.0 ? 1.0 : 0.0;
        case SC_LESS:
            return fVal < 0.0 ? 1.0 : 0.0;
        case SC_GREATER:
            return fVal > 0.0 ? 1.0 : 0.0;
        break;
        case SC_LESS_EQUAL:
            return fVal <= 0.0 ? 1.0 : 0.0;
        break;
        case SC_GREATER_EQUAL:
            return fVal >= 0.0 ? 1.0 : 0.0;
        break;
        case SC_NOT_EQUAL:
            return fVal != 0.0 ? 1.0 : 0.0;
        break;
        default:
            ;
    }

    OSL_TRACE( "evaluate: unhandled comparison operator: %d", (int)eOp);
    return CreateDoubleError( errUnknownState);
}

class CompareMatrixFunc : std::unary_function<MatrixImplType::element_block_type, void>
{
    sc::Compare& mrComp;
    size_t mnMatPos;
    sc::CompareOptions* mpOptions;
    std::vector<double> maResValues;    // double instead of bool to transport error values

    void compare()
    {
        double fVal = sc::CompareFunc(mrComp.maCells[0], mrComp.maCells[1], mrComp.mbIgnoreCase, mpOptions);
        maResValues.push_back(evaluate(fVal, mrComp.meOp));
    }

public:
    CompareMatrixFunc( size_t nResSize, sc::Compare& rComp, size_t nMatPos, sc::CompareOptions* pOptions ) :
        mrComp(rComp), mnMatPos(nMatPos), mpOptions(pOptions)
    {
        maResValues.reserve(nResSize);
    }

    void operator() (const MatrixImplType::element_block_node_type& node)
    {
        sc::Compare::Cell& rCell = mrComp.maCells[mnMatPos];

        switch (node.type)
        {
            case mdds::mtm::element_numeric:
            {
                typedef MatrixImplType::numeric_block_type block_type;

                block_type::const_iterator it = block_type::begin(*node.data);
                block_type::const_iterator itEnd = block_type::end(*node.data);
                for (; it != itEnd; ++it)
                {
                    rCell.mbValue = true;
                    rCell.mbEmpty = false;
                    rCell.mfValue = *it;
                    compare();
                }
            }
            break;
            case mdds::mtm::element_boolean:
            {
                typedef MatrixImplType::boolean_block_type block_type;

                block_type::const_iterator it = block_type::begin(*node.data);
                block_type::const_iterator itEnd = block_type::end(*node.data);
                for (; it != itEnd; ++it)
                {
                    rCell.mbValue = true;
                    rCell.mbEmpty = false;
                    rCell.mfValue = double(*it);
                    compare();
                }
            }
            break;
            case mdds::mtm::element_string:
            {
                typedef MatrixImplType::string_block_type block_type;

                block_type::const_iterator it = block_type::begin(*node.data);
                block_type::const_iterator itEnd = block_type::end(*node.data);
                for (; it != itEnd; ++it)
                {
                    const svl::SharedString& rStr = *it;
                    rCell.mbValue = false;
                    rCell.mbEmpty = false;
                    rCell.maStr = rStr;
                    compare();
                }
            }
            break;
            case mdds::mtm::element_empty:
            {
                rCell.mbValue = false;
                rCell.mbEmpty = true;
                rCell.maStr = svl::SharedString::getEmptyString();
                for (size_t i = 0; i < node.size; ++i)
                    compare();
            }
            default:
                ;
        }
    }

    const std::vector<double>& getValues() const
    {
        return maResValues;
    }
};

/**
 * Left-hand side is a matrix while the right-hand side is a numeric value.
 */
class CompareMatrixToNumericFunc : std::unary_function<MatrixImplType::element_block_type, void>
{
    sc::Compare& mrComp;
    double mfRightValue;
    sc::CompareOptions* mpOptions;
    std::vector<double> maResValues;    // double instead of bool to transport error values

    void compare()
    {
        double fVal = sc::CompareFunc(mrComp.maCells[0], mfRightValue, mpOptions);
        maResValues.push_back(evaluate(fVal, mrComp.meOp));
    }

    void compareLeftNumeric( double fLeftVal )
    {
        double fVal = sc::CompareFunc(fLeftVal, mfRightValue);
        maResValues.push_back(evaluate(fVal, mrComp.meOp));
    }

    void compareLeftEmpty( size_t nSize )
    {
        double fVal = sc::CompareEmptyToNumericFunc(mfRightValue);
        bool bRes = evaluate(fVal, mrComp.meOp);
        maResValues.resize(maResValues.size() + nSize, bRes ? 1.0 : 0.0);
    }

public:
    CompareMatrixToNumericFunc( size_t nResSize, sc::Compare& rComp, double fRightValue, sc::CompareOptions* pOptions ) :
        mrComp(rComp), mfRightValue(fRightValue), mpOptions(pOptions)
    {
        maResValues.reserve(nResSize);
    }

    void operator() (const MatrixImplType::element_block_node_type& node)
    {
        sc::Compare::Cell& rCell = mrComp.maCells[0];

        switch (node.type)
        {
            case mdds::mtm::element_numeric:
            {
                typedef MatrixImplType::numeric_block_type block_type;

                block_type::const_iterator it = block_type::begin(*node.data);
                block_type::const_iterator itEnd = block_type::end(*node.data);
                for (; it != itEnd; ++it)
                    compareLeftNumeric(*it);
            }
            break;
            case mdds::mtm::element_boolean:
            {
                typedef MatrixImplType::boolean_block_type block_type;

                block_type::const_iterator it = block_type::begin(*node.data);
                block_type::const_iterator itEnd = block_type::end(*node.data);
                for (; it != itEnd; ++it)
                    compareLeftNumeric(double(*it));
            }
            break;
            case mdds::mtm::element_string:
            {
                typedef MatrixImplType::string_block_type block_type;

                block_type::const_iterator it = block_type::begin(*node.data);
                block_type::const_iterator itEnd = block_type::end(*node.data);
                for (; it != itEnd; ++it)
                {
                    const svl::SharedString& rStr = *it;
                    rCell.mbValue = false;
                    rCell.mbEmpty = false;
                    rCell.maStr = rStr;
                    compare();
                }
            }
            break;
            case mdds::mtm::element_empty:
                compareLeftEmpty(node.size);
            break;
            default:
                ;
        }
    }

    const std::vector<double>& getValues() const
    {
        return maResValues;
    }
};

class ToDoubleArray : std::unary_function<MatrixImplType::element_block_type, void>
{
    std::vector<double> maArray;
    std::vector<double>::iterator miPos;
    double mfNaN;
    bool mbEmptyAsZero;

public:
    ToDoubleArray( size_t nSize, bool bEmptyAsZero ) :
        maArray(nSize, 0.0), miPos(maArray.begin()), mbEmptyAsZero(bEmptyAsZero)
    {
        mfNaN = CreateDoubleError( errElementNaN);
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
            {
                if (mbEmptyAsZero)
                {
                    std::advance(miPos, node.size);
                    return;
                }

                for (size_t i = 0; i < node.size; ++i, ++miPos)
                    *miPos = mfNaN;
            }
            break;
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
        mfNaN = CreateDoubleError( errElementNaN);
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
                    if (GetDoubleErrorValue(*miPos) == errElementNaN)
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
                    if (GetDoubleErrorValue(*miPos) == errElementNaN)
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
            {
                // Empty element is equivalent of having a numeric value of 0.0.
                for (size_t i = 0; i < node.size; ++i, ++miPos)
                {
                    if (GetDoubleErrorValue(*miPos) == errElementNaN)
                        continue;

                    *miPos = op(*miPos, 0.0);
                }
            }
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

size_t ScMatrixImpl::MatchStringInColumns(const svl::SharedString& rStr, size_t nCol1, size_t nCol2) const
{
    WalkAndMatchElements<svl::SharedString> aFunc(rStr, maMat.size(), nCol1, nCol2);
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

ScMatrixRef ScMatrixImpl::CompareMatrix(
    sc::Compare& rComp, size_t nMatPos, sc::CompareOptions* pOptions ) const
{
    MatrixImplType::size_pair_type aSize = maMat.size();
    size_t nSize = aSize.column * aSize.row;
    if (nMatPos == 0)
    {
        if (rComp.maCells[1].mbValue && !rComp.maCells[1].mbEmpty)
        {
            // Matrix on the left, and a numeric value on the right.  Use a
            // function object that has much less branching for much better
            // performance.
            CompareMatrixToNumericFunc aFunc(nSize, rComp, rComp.maCells[1].mfValue, pOptions);
            maMat.walk(aFunc);

            // We assume the result matrix has the same dimension as this matrix.
            const std::vector<double>& rResVal = aFunc.getValues();
            if (nSize != rResVal.size())
                ScMatrixRef();

            return ScMatrixRef(new ScMatrix(aSize.column, aSize.row, rResVal));
        }
    }

    CompareMatrixFunc aFunc(nSize, rComp, nMatPos, pOptions);
    maMat.walk(aFunc);

    // We assume the result matrix has the same dimension as this matrix.
    const std::vector<double>& rResVal = aFunc.getValues();
    if (nSize != rResVal.size())
        ScMatrixRef();

    return ScMatrixRef(new ScMatrix(aSize.column, aSize.row, rResVal));
}

void ScMatrixImpl::GetDoubleArray( std::vector<double>& rArray, bool bEmptyAsZero ) const
{
    MatrixImplType::size_pair_type aSize = maMat.size();
    ToDoubleArray aFunc(aSize.row*aSize.column, bEmptyAsZero);
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

void ScMatrixImpl::AddValues( const ScMatrixImpl& rMat )
{
    const MatrixImplType& rOther = rMat.maMat;
    MatrixImplType::size_pair_type aSize = maMat.size();
    if (aSize != rOther.size())
        // Geometry must match.
        return;

    // For now, we only add two matricies if and only if 1) the receiving
    // matrix consists only of one numeric block, and 2) the other matrix
    // consists of either one numeric block or one boolean block.  In the
    // future, we may want to be more flexible support matricies that consist
    // of multiple blocks.

    MatrixImplType::position_type aPos1 = maMat.position(0, 0);
    MatrixImplType::const_position_type aPos2 = rOther.position(0, 0);
    if (MatrixImplType::to_mtm_type(aPos1.first->type) != mdds::mtm::element_numeric)
        return;

    if (aPos1.first->size != aPos2.first->size)
        return;

    if (aPos1.first->size != aSize.row * aSize.column)
        return;

    MatrixImplType::numeric_block_type::iterator it =
        MatrixImplType::numeric_block_type::begin(*aPos1.first->data);
    MatrixImplType::numeric_block_type::iterator itEnd =
        MatrixImplType::numeric_block_type::end(*aPos1.first->data);

    switch (MatrixImplType::to_mtm_type(aPos2.first->type))
    {
        case mdds::mtm::element_boolean:
        {
            MatrixImplType::boolean_block_type::iterator it2 =
                MatrixImplType::boolean_block_type::begin(*aPos2.first->data);

            for (; it != itEnd; ++it, ++it2)
                *it += *it2;
        }
        break;
        case mdds::mtm::element_numeric:
        {
            MatrixImplType::numeric_block_type::iterator it2 =
                MatrixImplType::numeric_block_type::begin(*aPos2.first->data);

            for (; it != itEnd; ++it, ++it2)
                *it += *it2;
        }
        break;
        default:
            ;
    }
}

template<typename T, typename U>
struct wrapped_iterator
{
    typedef ::std::bidirectional_iterator_tag iterator_category;
    typedef double value_type;
    typedef value_type* pointer;
    typedef value_type& reference;
    typedef typename T::const_iterator::difference_type difference_type;

    typename T::const_iterator it;
    mutable value_type val;
    U maOp;

private:

    value_type calcVal() const
    {
        return maOp(*it);
    }

public:

    wrapped_iterator(typename T::const_iterator it_, U aOp):
        it(it_),
        val(value_type()),
        maOp(aOp)
    {
    }

    wrapped_iterator(const wrapped_iterator& r):
        it(r.it),
        val(r.val),
        maOp(r.maOp)
    {
    }

    wrapped_iterator& operator=(const wrapped_iterator& r)
    {
        it = r.it;
        return *this;
    }

    bool operator==(const wrapped_iterator& r) const
    {
        return it == r.it;
    }

    bool operator!=(const wrapped_iterator& r) const
    {
        return !operator==(r);
    }

    wrapped_iterator& operator++()
    {
        ++it;

        return *this;
    }

    wrapped_iterator& operator--()
    {
        --it;

        return *this;
    }

    value_type& operator*() const
    {
        val = calcVal();
        return val;
    }

    pointer operator->() const
    {
        val = calcVal();
        return &val;
    }
};

template<typename T, typename U>
struct MatrixIteratorWrapper
{
private:
    typename T::const_iterator m_itBegin;
    typename T::const_iterator m_itEnd;
    U maOp;
public:
    MatrixIteratorWrapper(typename T::const_iterator itBegin, typename T::const_iterator itEnd, U aOp):
        m_itBegin(itBegin),
        m_itEnd(itEnd),
        maOp(aOp)
    {
    }

    wrapped_iterator<T, U> begin()
    {
        return wrapped_iterator<T, U>(m_itBegin, maOp);
    }

    wrapped_iterator<T, U> end()
    {
        return wrapped_iterator<T, U>(m_itEnd, maOp);
    }
};

namespace {

MatrixImplType::position_type increment_position(const MatrixImplType::position_type& pos, size_t n)
{
    MatrixImplType::position_type ret = pos;
    do
    {
        if (ret.second + n < ret.first->size)
        {
            ret.second += n;
            break;
        }
        else
        {
            n -= (ret.first->size - ret.second);
            ++ret.first;
            ret.second = 0;
        }
    }
    while (n > 0);
    return ret;
}

}

template<typename T>
struct MatrixOpWrapper
{
private:
    MatrixImplType& mrMat;
    MatrixImplType::position_type pos;
    T maOp;

public:
    MatrixOpWrapper(MatrixImplType& rMat, T aOp):
        mrMat(rMat),
        pos(rMat.position(0,0)),
        maOp(aOp)
    {
    }

    void operator()(const MatrixImplType::element_block_node_type& node)
    {
        switch (node.type)
        {
            case mdds::mtm::element_numeric:
            {
                typedef MatrixImplType::numeric_block_type block_type;

                block_type::const_iterator it = block_type::begin(*node.data);
                block_type::const_iterator itEnd = block_type::end(*node.data);
                MatrixIteratorWrapper<block_type, T> aFunc(it, itEnd, maOp);
                pos = mrMat.set(pos,aFunc.begin(), aFunc.end());
            }
            break;
            case mdds::mtm::element_boolean:
            {
                typedef MatrixImplType::boolean_block_type block_type;

                block_type::const_iterator it = block_type::begin(*node.data);
                block_type::const_iterator itEnd = block_type::end(*node.data);

                MatrixIteratorWrapper<block_type, T> aFunc(it, itEnd, maOp);
                pos = mrMat.set(pos, aFunc.begin(), aFunc.end());
            }
            break;
            case mdds::mtm::element_string:
            {
                typedef MatrixImplType::string_block_type block_type;

                block_type::const_iterator it = block_type::begin(*node.data);
                block_type::const_iterator itEnd = block_type::end(*node.data);

                MatrixIteratorWrapper<block_type, T> aFunc(it, itEnd, maOp);
                pos = mrMat.set(pos, aFunc.begin(), aFunc.end());
            }
            break;
            case mdds::mtm::element_empty:
            {
                if (maOp.useFunctionForEmpty())
                {
                    std::vector<char> aVec(node.size);
                    MatrixIteratorWrapper<std::vector<char>, T> aFunc(aVec.begin(), aVec.end(), maOp);
                    pos = mrMat.set(pos, aFunc.begin(), aFunc.end());
                }
            }
            break;
            default:
                ;
        }
        pos = increment_position(pos, node.size);
    }
};

template<typename T>
void ScMatrixImpl::ApplyOperation(T aOp, ScMatrixImpl& rMat)
{
    MatrixOpWrapper<T> aFunc(rMat.maMat, aOp);
    maMat.walk(aFunc);
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
                    cout << "string (" << maMat.get_string(nRow, nCol).getString() << ")";
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

ScMatrix::ScMatrix( size_t nC, size_t nR, const std::vector<double>& rInitVals ) :
    pImpl(new ScMatrixImpl(nC, nR, rInitVals)), nRefCnt(0)
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

void ScMatrix::PutString(const svl::SharedString& rStr, SCSIZE nC, SCSIZE nR)
{
    pImpl->PutString(rStr, nC, nR);
}

void ScMatrix::PutString(const svl::SharedString& rStr, SCSIZE nIndex)
{
    pImpl->PutString(rStr, nIndex);
}

void ScMatrix::PutString(const svl::SharedString* pArray, size_t nLen, SCSIZE nC, SCSIZE nR)
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

void ScMatrix::PutError( sal_uInt16 nErrorCode, SCSIZE nIndex )
{
    pImpl->PutError(nErrorCode, nIndex);
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

svl::SharedString ScMatrix::GetString(SCSIZE nC, SCSIZE nR) const
{
    return pImpl->GetString(nC, nR);
}

svl::SharedString ScMatrix::GetString( SCSIZE nIndex) const
{
    return pImpl->GetString(nIndex);
}

svl::SharedString ScMatrix::GetString( SvNumberFormatter& rFormatter, SCSIZE nC, SCSIZE nR) const
{
    return pImpl->GetString(rFormatter, nC, nR);
}

ScMatrixValue ScMatrix::Get(SCSIZE nC, SCSIZE nR) const
{
    return pImpl->Get(nC, nR);
}

bool ScMatrix::IsString( SCSIZE nIndex ) const
{
    return pImpl->IsString(nIndex);
}

bool ScMatrix::IsString( SCSIZE nC, SCSIZE nR ) const
{
    return pImpl->IsString(nC, nR);
}

bool ScMatrix::IsEmpty( SCSIZE nC, SCSIZE nR ) const
{
    return pImpl->IsEmpty(nC, nR);
}

bool ScMatrix::IsEmptyCell( SCSIZE nC, SCSIZE nR ) const
{
    return pImpl->IsEmptyCell(nC, nR);
}

bool ScMatrix::IsEmptyResult( SCSIZE nC, SCSIZE nR ) const
{
    return pImpl->IsEmptyResult(nC, nR);
}

bool ScMatrix::IsEmptyPath( SCSIZE nC, SCSIZE nR ) const
{
    return pImpl->IsEmptyPath(nC, nR);
}

bool ScMatrix::IsValue( SCSIZE nIndex ) const
{
    return pImpl->IsValue(nIndex);
}

bool ScMatrix::IsValue( SCSIZE nC, SCSIZE nR ) const
{
    return pImpl->IsValue(nC, nR);
}

bool ScMatrix::IsValueOrEmpty( SCSIZE nC, SCSIZE nR ) const
{
    return pImpl->IsValueOrEmpty(nC, nR);
}

bool ScMatrix::IsBoolean( SCSIZE nC, SCSIZE nR ) const
{
    return pImpl->IsBoolean(nC, nR);
}

bool ScMatrix::IsNumeric() const
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

void ScMatrix::PutDoubleVector( const ::std::vector< double > & rVec, SCSIZE nC, SCSIZE nR )
{
    pImpl->PutDoubleVector(rVec, nC, nR);
}

void ScMatrix::PutStringVector( const ::std::vector< svl::SharedString > & rVec, SCSIZE nC, SCSIZE nR )
{
    pImpl->PutStringVector(rVec, nC, nR);
}

void ScMatrix::PutEmptyVector( SCSIZE nCount, SCSIZE nC, SCSIZE nR )
{
    pImpl->PutEmptyVector(nCount, nC, nR);
}

void ScMatrix::PutEmptyResultVector( SCSIZE nCount, SCSIZE nC, SCSIZE nR )
{
    pImpl->PutEmptyResultVector(nCount, nC, nR);
}

void ScMatrix::PutEmptyPathVector( SCSIZE nCount, SCSIZE nC, SCSIZE nR )
{
    pImpl->PutEmptyPathVector(nCount, nC, nR);
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

size_t ScMatrix::MatchDoubleInColumns(double fValue, size_t nCol1, size_t nCol2) const
{
    return pImpl->MatchDoubleInColumns(fValue, nCol1, nCol2);
}

size_t ScMatrix::MatchStringInColumns(const svl::SharedString& rStr, size_t nCol1, size_t nCol2) const
{
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

ScMatrixRef ScMatrix::CompareMatrix(
    sc::Compare& rComp, size_t nMatPos, sc::CompareOptions* pOptions ) const
{
    return pImpl->CompareMatrix(rComp, nMatPos, pOptions);
}

void ScMatrix::GetDoubleArray( std::vector<double>& rArray, bool bEmptyAsZero ) const
{
    pImpl->GetDoubleArray(rArray, bEmptyAsZero);
}

void ScMatrix::MergeDoubleArray( std::vector<double>& rArray, Op eOp ) const
{
    pImpl->MergeDoubleArray(rArray, eOp);
}

namespace {

struct AddOp
{
private:
    double mnVal;
    double mnError;

public:

    AddOp(double nVal):
        mnVal(nVal),
        mnError( CreateDoubleError( errNoValue))
    {
    }

    double operator()(double nVal) const
    {
        return nVal + mnVal;
    }

    double operator()(bool bVal) const
    {
        return mnVal + (double)bVal;
    }

    double operator()(const svl::SharedString&) const
    {
        return mnError;
    }

    double operator()(char) const
    {
        return mnVal;   // mnVal + 0.0
    }

    bool useFunctionForEmpty() const
    {
        return true;
    }
};

struct SubOp
{
private:
    double mnVal;
    double mnError;

public:

    SubOp(double nVal):
        mnVal(nVal),
        mnError( CreateDoubleError( errNoValue))
    {
    }

    double operator()(double nVal) const
    {
        return mnVal - nVal;
    }

    double operator()(bool bVal) const
    {
        return mnVal - (double)bVal;
    }

    double operator()(const svl::SharedString&) const
    {
        return mnError;
    }

    double operator()(char) const
    {
        return mnVal;   // mnVal - 0.0
    }

    bool useFunctionForEmpty() const
    {
        return true;
    }
};

}

void ScMatrix::SubAddOp(bool bSub, double fVal, ScMatrix& rMat)
{
    if(bSub)
    {
        SubOp aOp(fVal);
        pImpl->ApplyOperation(aOp, *rMat.pImpl);
    }
    else
    {
        AddOp aOp(fVal);
        pImpl->ApplyOperation(aOp, *rMat.pImpl);
    }
}

ScMatrix& ScMatrix::operator+= ( const ScMatrix& r )
{
    pImpl->AddValues(*r.pImpl);
    return *this;
}

#if DEBUG_MATRIX
void ScMatrix::Dump() const
{
    pImpl->Dump();
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
