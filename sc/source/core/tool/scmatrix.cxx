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

#include <arraysumfunctor.hxx>
#include "scmatrix.hxx"
#include "global.hxx"
#include "address.hxx"
#include <formula/errorcodes.hxx>
#include <formula/vectortoken.hxx>
#include "interpre.hxx"
#include "mtvelements.hxx"
#include "compare.hxx"
#include "matrixoperators.hxx"
#include "math.hxx"

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
using ::std::advance;
using ::std::unary_function;

/**
 * Custom string trait struct to tell mdds::multi_type_matrix about the
 * custom string type and how to handle blocks storing them.
 */
struct matrix_trait
{
    typedef sc::string_block string_element_block;
    typedef sc::uint16_block integer_element_block;

    typedef mdds::mtv::custom_block_func1<sc::string_block> element_block_func;
};

typedef mdds::multi_type_matrix<matrix_trait> MatrixImplType;

namespace {

double convertStringToValue( ScInterpreter* pErrorInterpreter, const OUString& rStr )
{
    if (pErrorInterpreter)
    {
        FormulaError nError = FormulaError::NONE;
        short nCurFmtType = 0;
        double fValue = pErrorInterpreter->ConvertStringToValue( rStr, nError, nCurFmtType);
        if (nError != FormulaError::NONE)
        {
            pErrorInterpreter->SetError( nError);
            return CreateDoubleError( nError);
        }
        return fValue;
    }
    return CreateDoubleError( FormulaError::NoValue);
}

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

template<typename Comp>
class CompareMatrixElemFunc : public std::unary_function<MatrixImplType::element_block_node_type, void>
{
    static Comp maComp;

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

template<typename Comp>
Comp CompareMatrixElemFunc<Comp>::maComp;

}

/* TODO: it would be good if mdds had get/set<sal_uInt8> additionally to
 * get/set<bool>, we're abusing double here. */
typedef double TMatFlag;
const TMatFlag SC_MATFLAG_EMPTYRESULT = 1.0;
const TMatFlag SC_MATFLAG_EMPTYPATH   = 2.0;

class ScMatrixImpl
{
    MatrixImplType maMat;
    MatrixImplType maMatFlag;
    ScInterpreter* pErrorInterpreter;

public:
    ScMatrixImpl(const ScMatrixImpl&) = delete;
    const ScMatrixImpl& operator=(const ScMatrixImpl&) = delete;

    ScMatrixImpl(SCSIZE nC, SCSIZE nR);
    ScMatrixImpl(SCSIZE nC, SCSIZE nR, double fInitVal);

    ScMatrixImpl( size_t nC, size_t nR, const std::vector<double>& rInitVals );

    ~ScMatrixImpl();

    void Clear();
    void Resize(SCSIZE nC, SCSIZE nR);
    void Resize(SCSIZE nC, SCSIZE nR, double fVal);
    void SetErrorInterpreter( ScInterpreter* p);
    ScInterpreter* GetErrorInterpreter() const { return pErrorInterpreter; }

    void GetDimensions( SCSIZE& rC, SCSIZE& rR) const;
    SCSIZE GetElementCount() const;
    bool ValidColRow( SCSIZE nC, SCSIZE nR) const;
    bool ValidColRowReplicated( SCSIZE & rC, SCSIZE & rR ) const;
    bool ValidColRowOrReplicated( SCSIZE & rC, SCSIZE & rR ) const;
    void SetErrorAtInterpreter( FormulaError nError ) const;

    void PutDouble(double fVal, SCSIZE nC, SCSIZE nR);
    void PutDouble( double fVal, SCSIZE nIndex);
    void PutDouble(const double* pArray, size_t nLen, SCSIZE nC, SCSIZE nR);

    void PutString(const svl::SharedString& rStr, SCSIZE nC, SCSIZE nR);
    void PutString(const svl::SharedString& rStr, SCSIZE nIndex);
    void PutString(const svl::SharedString* pArray, size_t nLen, SCSIZE nC, SCSIZE nR);

    void PutEmpty(SCSIZE nC, SCSIZE nR);
    void PutEmptyPath(SCSIZE nC, SCSIZE nR);
    void PutError( FormulaError nErrorCode, SCSIZE nC, SCSIZE nR );
    void PutBoolean(bool bVal, SCSIZE nC, SCSIZE nR);
    FormulaError GetError( SCSIZE nC, SCSIZE nR) const;
    double GetDouble(SCSIZE nC, SCSIZE nR) const;
    double GetDouble( SCSIZE nIndex) const;
    double GetDoubleWithStringConversion(SCSIZE nC, SCSIZE nR) const;
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
    size_t Count(bool bCountStrings, bool bCountErrors) const;
    size_t MatchDoubleInColumns(double fValue, size_t nCol1, size_t nCol2) const;
    size_t MatchStringInColumns(const svl::SharedString& rStr, size_t nCol1, size_t nCol2) const;

    double GetMaxValue( bool bTextAsZero ) const;
    double GetMinValue( bool bTextAsZero ) const;
    double GetGcd() const;

    ScMatrixRef CompareMatrix( sc::Compare& rComp, size_t nMatPos, sc::CompareOptions* pOptions ) const;

    void GetDoubleArray( std::vector<double>& rArray, bool bEmptyAsZero ) const;
    void MergeDoubleArray( std::vector<double>& rArray, ScFullMatrix::Op eOp ) const;
    void AddValues( const ScMatrixImpl& rMat );

    template<typename T>
    void ApplyOperation(T aOp, ScMatrixImpl& rMat);

    void ExecuteOperation(const std::pair<size_t, size_t>& rStartPos,
            const std::pair<size_t, size_t>& rEndPos, const ScFullMatrix::DoubleOpFunction& aDoubleFunc,
            const ScFullMatrix::BoolOpFunction& aBoolFunc, const ScFullMatrix::StringOpFunction& aStringFunc,
            const ScFullMatrix::EmptyOpFunction& aEmptyFunc) const;

    template<typename T>
    std::vector<ScMatrix::IterateResult> ApplyCollectOperation(bool bTextAsZero, const std::vector<std::unique_ptr<T>>& aOp);

    void MatConcat(SCSIZE nMaxCol, SCSIZE nMaxRow, const ScMatrixRef& xMat1, const ScMatrixRef& xMat2,
            SvNumberFormatter& rFormatter, svl::SharedStringPool& rPool);

#if DEBUG_MATRIX
    void Dump() const;
#endif

private:
    void CalcPosition(SCSIZE nIndex, SCSIZE& rC, SCSIZE& rR) const;
};

ScMatrixImpl::ScMatrixImpl(SCSIZE nC, SCSIZE nR) :
    maMat(nR, nC), maMatFlag(nR, nC), pErrorInterpreter(nullptr) {}

ScMatrixImpl::ScMatrixImpl(SCSIZE nC, SCSIZE nR, double fInitVal) :
    maMat(nR, nC, fInitVal), maMatFlag(nR, nC), pErrorInterpreter(nullptr) {}

ScMatrixImpl::ScMatrixImpl( size_t nC, size_t nR, const std::vector<double>& rInitVals ) :
    maMat(nR, nC, rInitVals.begin(), rInitVals.end()), maMatFlag(nR, nC), pErrorInterpreter(nullptr) {}

ScMatrixImpl::~ScMatrixImpl()
{
    Clear();
}

void ScMatrixImpl::Clear()
{
    maMat.clear();
    maMatFlag.clear();
}

void ScMatrixImpl::Resize(SCSIZE nC, SCSIZE nR)
{
    if (ScMatrix::IsSizeAllocatable( nC, nR))
    {
        maMat.resize(nR, nC);
        maMatFlag.resize(nR, nC);
    }
    else
    {
        // Invalid matrix size, allocate 1x1 matrix with error value.
        maMat.resize(1, 1, CreateDoubleError( FormulaError::MatrixSize));
        maMatFlag.resize(1, 1);
    }
}

void ScMatrixImpl::Resize(SCSIZE nC, SCSIZE nR, double fVal)
{
    if (ScMatrix::IsSizeAllocatable( nC, nR))
    {
        maMat.resize(nR, nC, fVal);
        maMatFlag.resize(nR, nC);
    }
    else
    {
        // Invalid matrix size, allocate 1x1 matrix with error value.
        maMat.resize(1, 1, CreateDoubleError( FormulaError::StackOverflow));
        maMatFlag.resize(1, 1);
    }
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

void ScMatrixImpl::SetErrorAtInterpreter( FormulaError nError ) const
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
        maMatFlag.set_empty(nR, nC);
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

void ScMatrixImpl::PutError( FormulaError nErrorCode, SCSIZE nC, SCSIZE nR )
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

FormulaError ScMatrixImpl::GetError( SCSIZE nC, SCSIZE nR) const
{
    if (ValidColRowOrReplicated( nC, nR ))
    {
        double fVal = maMat.get_numeric(nR, nC);
        return GetDoubleErrorValue(fVal);
    }
    else
    {
        OSL_FAIL("ScMatrixImpl::GetError: dimension error");
        return FormulaError::NoValue;
    }
}

double ScMatrixImpl::GetDouble(SCSIZE nC, SCSIZE nR) const
{
    if (ValidColRowOrReplicated( nC, nR ))
    {
        double fVal = maMat.get_numeric(nR, nC);
        if ( pErrorInterpreter )
        {
            FormulaError nError = GetDoubleErrorValue(fVal);
            if ( nError != FormulaError::NONE )
                SetErrorAtInterpreter( nError);
        }
        return fVal;
    }
    else
    {
        OSL_FAIL("ScMatrixImpl::GetDouble: dimension error");
        return CreateDoubleError( FormulaError::NoValue);
    }
}

double ScMatrixImpl::GetDouble( SCSIZE nIndex) const
{
    SCSIZE nC, nR;
    CalcPosition(nIndex, nC, nR);
    return GetDouble(nC, nR);
}

double ScMatrixImpl::GetDoubleWithStringConversion(SCSIZE nC, SCSIZE nR) const
{
    ScMatrixValue aMatVal = Get(nC, nR);
    if (aMatVal.nType == ScMatValType::String)
        return convertStringToValue( pErrorInterpreter, aMatVal.aStr.getString());
    return aMatVal.fVal;
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
                SAL_FALLTHROUGH;
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
        return svl::SharedString::getEmptyString();
    }

    double fVal = 0.0;
    MatrixImplType::const_position_type aPos = maMat.position(nR, nC);
    switch (maMat.get_type(aPos))
    {
        case mdds::mtm::element_string:
            return maMat.get_string(aPos);
        case mdds::mtm::element_empty:
        {
            if (maMatFlag.get_numeric(nR, nC) != SC_MATFLAG_EMPTYPATH)
                // not an empty path.
                return svl::SharedString::getEmptyString();

            // result of empty FALSE jump path
            sal_uLong nKey = rFormatter.GetStandardFormat( css::util::NumberFormat::LOGICAL,
                    ScGlobal::eLnge);
            OUString aStr;
            Color* pColor = nullptr;
            rFormatter.GetOutputString( 0.0, nKey, aStr, &pColor);
            return svl::SharedString( aStr);    // string not interned
        }
        case mdds::mtm::element_numeric:
        case mdds::mtm::element_boolean:
            fVal = maMat.get_numeric(aPos);
        break;
        default:
            ;
    }

    FormulaError nError = GetDoubleErrorValue(fVal);
    if (nError != FormulaError::NONE)
    {
        SetErrorAtInterpreter( nError);
        return svl::SharedString( ScGlobal::GetErrorString( nError));   // string not interned
    }

    sal_uLong nKey = rFormatter.GetStandardFormat( css::util::NumberFormat::NUMBER,
            ScGlobal::eLnge);
    OUString aStr;
    rFormatter.GetInputLineString( fVal, nKey, aStr);
    return svl::SharedString( aStr);    // string not interned
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
                aVal.nType = ScMatValType::Boolean;
                aVal.fVal = double(maMat.get_boolean(aPos));
            break;
            case mdds::mtm::element_numeric:
                aVal.nType = ScMatValType::Value;
                aVal.fVal = maMat.get_numeric(aPos);
            break;
            case mdds::mtm::element_string:
                aVal.nType = ScMatValType::String;
                aVal.aStr = maMat.get_string(aPos);
            break;
            case mdds::mtm::element_empty:
                /* TODO: do we need to pass the differentiation of 'empty' and
                 * 'empty result' to the outer world anywhere? */
                switch (maMatFlag.get_type(nR, nC))
                {
                    case mdds::mtm::element_empty:
                        aVal.nType = ScMatValType::Empty;
                    break;
                    case mdds::mtm::element_numeric:
                        aVal.nType = maMatFlag.get<TMatFlag>(nR, nC)
                            == SC_MATFLAG_EMPTYPATH ? ScMatValType::EmptyPath : ScMatValType::Empty;
                    break;
                    default:
                        assert(false);
                }
                aVal.fVal = 0.0;
            break;
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
        maMatFlag.get_numeric(nR, nC) != SC_MATFLAG_EMPTYPATH;
}

bool ScMatrixImpl::IsEmptyCell( SCSIZE nC, SCSIZE nR ) const
{
    // Flag must indicate an 'empty cell' element instead of an
    // 'empty' or 'empty result' or 'empty path' element.
    ValidColRowReplicated( nC, nR );
    return maMat.get_type(nR, nC) == mdds::mtm::element_empty &&
        maMatFlag.get_type(nR, nC) == mdds::mtm::element_empty;
}

bool ScMatrixImpl::IsEmptyResult( SCSIZE nC, SCSIZE nR ) const
{
    // Flag must indicate an 'empty result' element instead of an
    // 'empty' or 'empty cell' or 'empty path' element.
    ValidColRowReplicated( nC, nR );
    return maMat.get_type(nR, nC) == mdds::mtm::element_empty &&
        maMatFlag.get_numeric(nR, nC) == SC_MATFLAG_EMPTYRESULT;
}

bool ScMatrixImpl::IsEmptyPath( SCSIZE nC, SCSIZE nR ) const
{
    // Flag must indicate an 'empty path' element.
    if (ValidColRowOrReplicated( nC, nR ))
        return maMat.get_type(nR, nC) == mdds::mtm::element_empty &&
            maMatFlag.get_numeric(nR, nC) == SC_MATFLAG_EMPTYPATH;
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
        maMatFlag.set_empty(nR, nC, nCount);
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
template <typename Evaluator>
double EvalMatrix(const MatrixImplType& rMat)
{
    Evaluator aEval;
    size_t nRows = rMat.size().row, nCols = rMat.size().column;
    for (size_t i = 0; i < nRows; ++i)
    {
        for (size_t j = 0; j < nCols; ++j)
        {
            MatrixImplType::const_position_type aPos = rMat.position(i, j);
            mdds::mtm::element_t eType = rMat.get_type(aPos);
            if (eType != mdds::mtm::element_numeric && eType != mdds::mtm::element_boolean)
                // assuming a CompareMat this is an error
                return CreateDoubleError(FormulaError::IllegalArgument);

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

template<typename Op>
class WalkElementBlocks
{
    Op maOp;
    ScMatrix::IterateResult maRes;
    bool mbFirst:1;
    bool mbTextAsZero:1;
public:
    WalkElementBlocks(bool bTextAsZero) : maRes(Op::InitVal, Op::InitVal, 0), mbFirst(true), mbTextAsZero(bTextAsZero) {}

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
                    {
                        maOp(maRes.mfRest, *it);
                    }
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
                    {
                        maOp(maRes.mfRest, *it);
                    }
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

template<typename Op>
class WalkElementBlocksMultipleValues
{
    const std::vector<std::unique_ptr<Op>>& maOp;
    std::vector<ScMatrix::IterateResult> maRes;
    bool mbFirst:1;
    bool mbTextAsZero:1;
public:
    WalkElementBlocksMultipleValues(bool bTextAsZero, const std::vector<std::unique_ptr<Op>>& aOp) :
        maOp(aOp), mbFirst(true), mbTextAsZero(bTextAsZero)
    {
        for (const auto& rpOp : maOp)
        {
            maRes.emplace_back(rpOp->mInitVal, rpOp->mInitVal, 0);
        }
        maRes.emplace_back(0.0, 0.0, 0); // count
    }

    const std::vector<ScMatrix::IterateResult>& getResult() const { return maRes; }

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
                        for (auto i = 0u; i < maOp.size(); ++i)
                        {
                            (*maOp[i])(maRes[i].mfFirst, *it);
                        }
                        mbFirst = false;
                    }
                    else
                    {
                        for (auto i = 0u; i < maOp.size(); ++i)
                        {
                            (*maOp[i])(maRes[i].mfRest, *it);
                        }
                    }
                }
                maRes.back().mnCount += node.size;
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
                        for (auto i = 0u; i < maOp.size(); ++i)
                        {
                            (*maOp[i])(maRes[i].mfFirst, *it);
                        }
                        mbFirst = false;
                    }
                    else
                    {
                        for (auto i = 0u; i < maOp.size(); ++i)
                        {
                            (*maOp[i])(maRes[i].mfRest, *it);
                        }
                    }
                }
                maRes.back().mnCount += node.size;
            }
            break;
            case mdds::mtm::element_string:
                if (mbTextAsZero)
                    maRes.back().mnCount += node.size;
            break;
            case mdds::mtm::element_empty:
            default:
                ;
        }
    }
};

class CountElements : public std::unary_function<MatrixImplType::element_block_node_type, void>
{
    size_t mnCount;
    bool mbCountString;
    bool mbCountErrors;
public:
    explicit CountElements(bool bCountString, bool bCountErrors) :
        mnCount(0), mbCountString(bCountString), mbCountErrors(bCountErrors) {}

    size_t getCount() const { return mnCount; }

    void operator() (const MatrixImplType::element_block_node_type& node)
    {
        switch (node.type)
        {
            case mdds::mtm::element_numeric:
                mnCount += node.size;
                if (!mbCountErrors)
                {
                    typedef MatrixImplType::numeric_block_type block_type;

                    block_type::const_iterator it = block_type::begin(*node.data);
                    block_type::const_iterator itEnd = block_type::end(*node.data);
                    for (; it != itEnd; ++it)
                    {
                        if (!::rtl::math::isFinite(*it))
                            --mnCount;
                    }
                }
            break;
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

template<typename Type>
class WalkAndMatchElements : public std::unary_function<MatrixImplType::element_block_node_type, void>
{
    Type maMatchValue;
    MatrixImplType::size_pair_type maSize;
    size_t mnCol1;
    size_t mnCol2;
    size_t mnResult;
    size_t mnIndex;

public:
    WalkAndMatchElements(Type aMatchValue, const MatrixImplType::size_pair_type& aSize, size_t nCol1, size_t nCol2) :
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
        const MatrixImplType::boolean_block_type::const_iterator& itEnd)
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
        const MatrixImplType::boolean_block_type::const_iterator& itEnd)
    {
        // If the array has at least one false value, the minimum value is 0.
        it = std::find(it, itEnd, false);
        return it == itEnd ? 1.0 : 0.0;
    }
};

struct Gcd
{
  static double boolValue(
        MatrixImplType::boolean_block_type::const_iterator it,
        const MatrixImplType::boolean_block_type::const_iterator& itEnd)
    {
        // If the array has at least one true value, the gcdResult is 1.
        it = std::find(it, itEnd, true);
        return it == itEnd ? 0.0 : 1.0;
    }
};

template<typename Op>
class CalcMaxMinValue : public std::unary_function<MatrixImplType::element_block_type, void>
{
    double mfVal;
    bool mbTextAsZero;
    bool mbHasValue;
public:
    CalcMaxMinValue( bool bTextAsZero ) :
        mfVal(Op::init()),
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
                    mfVal = Op::compare(mfVal, *it);

                mbHasValue = true;
            }
            break;
            case mdds::mtm::element_boolean:
            {
                typedef MatrixImplType::boolean_block_type block_type;

                block_type::const_iterator it = block_type::begin(*node.data);
                block_type::const_iterator itEnd = block_type::end(*node.data);
                double fVal = Op::boolValue(it, itEnd);
                mfVal = Op::compare(mfVal, fVal);
                mbHasValue = true;
            }
            break;
            case mdds::mtm::element_string:
            case mdds::mtm::element_empty:
            {
                // empty elements are treated as empty strings.
                if (mbTextAsZero)
                {
                    mfVal = Op::compare(mfVal, 0.0);
                    mbHasValue = true;
                }
            }
            break;
            default:
                ;
        }
    }
};

template<typename Op>
class CalcGcd : public std::unary_function<MatrixImplType::element_block_type,void>
{
     double mfval;

   public:
     CalcGcd():
     mfval((0.0)){}

     double getResult() const { return mfval ;}

     void operator() ( const MatrixImplType::element_block_node_type& node)
     {
       switch(node.type)
       {
          case mdds::mtm::element_numeric:
          {
             typedef MatrixImplType::numeric_block_type block_type;
             block_type::const_iterator it = block_type::begin(*node.data);
             block_type::const_iterator itEnd = block_type::end(*node.data);

             for(;it != itEnd;++it)
              {
                if(*it < 0 )
                 mfval = CreateDoubleError(FormulaError::IllegalArgument);
                else
                 mfval = ScInterpreter::ScGetGCD(*it,mfval);
              }
           }
          break;
          case mdds::mtm::element_boolean:
          {
              typedef MatrixImplType::boolean_block_type block_type;
              block_type::const_iterator it = block_type::begin(*node.data);
              block_type::const_iterator itEnd = block_type::end(*node.data);

              mfval = Op::boolValue(it, itEnd);
          }
          break;
          case mdds::mtm::element_empty:
          case mdds::mtm::element_string:
          {
              mfval = CreateDoubleError(FormulaError::IllegalArgument);
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

    SAL_WARN("sc.core",  "evaluate: unhandled comparison operator: " << (int)eOp);
    return CreateDoubleError( FormulaError::UnknownState);
}

class CompareMatrixFunc : public std::unary_function<MatrixImplType::element_block_type, void>
{
    sc::Compare& mrComp;
    size_t mnMatPos;
    sc::CompareOptions* mpOptions;
    std::vector<double> maResValues;    // double instead of bool to transport error values

    void compare()
    {
        double fVal = sc::CompareFunc( mrComp, mpOptions);
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

/**
 * Left-hand side is a matrix while the right-hand side is a numeric value.
 */
class CompareMatrixToNumericFunc : public std::unary_function<MatrixImplType::element_block_type, void>
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

class ToDoubleArray : public std::unary_function<MatrixImplType::element_block_type, void>
{
    std::vector<double> maArray;
    std::vector<double>::iterator miPos;
    double mfNaN;
    bool mbEmptyAsZero;

public:
    ToDoubleArray( size_t nSize, bool bEmptyAsZero ) :
        maArray(nSize, 0.0), miPos(maArray.begin()), mbEmptyAsZero(bEmptyAsZero)
    {
        mfNaN = CreateDoubleError( FormulaError::ElementNaN);
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

template<typename Op>
class MergeDoubleArrayFunc : public std::unary_function<MatrixImplType::element_block_type, void>
{
    std::vector<double>& mrArray;
    std::vector<double>::iterator miPos;
    double mfNaN;
public:
    MergeDoubleArrayFunc(std::vector<double>& rArray) : mrArray(rArray), miPos(mrArray.begin())
    {
        mfNaN = CreateDoubleError( FormulaError::ElementNaN);
    }

    void operator() (const MatrixImplType::element_block_node_type& node)
    {
        using namespace mdds::mtv;
        static Op op;

        switch (node.type)
        {
            case mdds::mtm::element_numeric:
            {
                numeric_element_block::const_iterator it = numeric_element_block::begin(*node.data);
                numeric_element_block::const_iterator itEnd = numeric_element_block::end(*node.data);
                for (; it != itEnd; ++it, ++miPos)
                {
                    if (GetDoubleErrorValue(*miPos) == FormulaError::ElementNaN)
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
                    if (GetDoubleErrorValue(*miPos) == FormulaError::ElementNaN)
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
                    if (GetDoubleErrorValue(*miPos) == FormulaError::ElementNaN)
                        continue;

                    *miPos = op(*miPos, 0.0);
                }
            }
            break;
            default:
                ;
        }
    }
};

}

namespace {

template<typename TOp>
ScMatrix::IterateResult GetValueWithCount(bool bTextAsZero, const MatrixImplType& maMat)
{
    WalkElementBlocks<TOp> aFunc(bTextAsZero);
    maMat.walk(aFunc);
    return aFunc.getResult();
}

}

ScMatrix::IterateResult ScMatrixImpl::Sum(bool bTextAsZero) const
{
    return GetValueWithCount<sc::op::Sum>(bTextAsZero, maMat);
}

ScMatrix::IterateResult ScMatrixImpl::SumSquare(bool bTextAsZero) const
{
    return GetValueWithCount<sc::op::SumSquare>(bTextAsZero, maMat);
}

ScMatrix::IterateResult ScMatrixImpl::Product(bool bTextAsZero) const
{
    return GetValueWithCount<sc::op::Product>(bTextAsZero, maMat);
}

size_t ScMatrixImpl::Count(bool bCountStrings, bool bCountErrors) const
{
    CountElements aFunc(bCountStrings, bCountErrors);
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

double ScMatrixImpl::GetGcd() const
{
    CalcGcd<Gcd> aFunc;
    maMat.walk(aFunc);
    return aFunc.getResult();
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

            return ScMatrixRef(new ScFullMatrix(aSize.column, aSize.row, rResVal));
        }
    }

    CompareMatrixFunc aFunc(nSize, rComp, nMatPos, pOptions);
    maMat.walk(aFunc);

    // We assume the result matrix has the same dimension as this matrix.
    const std::vector<double>& rResVal = aFunc.getValues();
    if (nSize != rResVal.size())
        ScMatrixRef();

    return ScMatrixRef(new ScFullMatrix(aSize.column, aSize.row, rResVal));
}

void ScMatrixImpl::GetDoubleArray( std::vector<double>& rArray, bool bEmptyAsZero ) const
{
    MatrixImplType::size_pair_type aSize = maMat.size();
    ToDoubleArray aFunc(aSize.row*aSize.column, bEmptyAsZero);
    maMat.walk(aFunc);
    aFunc.swap(rArray);
}

void ScMatrixImpl::MergeDoubleArray( std::vector<double>& rArray, ScFullMatrix::Op eOp ) const
{
    MatrixImplType::size_pair_type aSize = maMat.size();
    size_t nSize = aSize.row*aSize.column;
    if (nSize != rArray.size())
        return;

    switch (eOp)
    {
        case ScFullMatrix::Mul:
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

namespace Op {

template<typename T>
struct return_type
{
    typedef T type;
};

template<>
struct return_type<bool>
{
    typedef double type;
};

template<>
struct return_type<char>
{
    typedef svl::SharedString type;
};

}

template<typename T, typename U, typename return_type>
struct wrapped_iterator
{
    typedef ::std::bidirectional_iterator_tag iterator_category;
    typedef typename T::const_iterator::value_type old_value_type;
    typedef return_type value_type;
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

    wrapped_iterator(typename T::const_iterator const & it_, U const & aOp):
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

template<typename T, typename U, typename return_type>
struct MatrixIteratorWrapper
{
private:
    typename T::const_iterator m_itBegin;
    typename T::const_iterator m_itEnd;
    U maOp;
public:
    MatrixIteratorWrapper(typename T::const_iterator const & itBegin, typename T::const_iterator const & itEnd, U const & aOp):
        m_itBegin(itBegin),
        m_itEnd(itEnd),
        maOp(aOp)
    {
    }

    wrapped_iterator<T, U, return_type> begin()
    {
        return wrapped_iterator<T, U, return_type>(m_itBegin, maOp);
    }

    wrapped_iterator<T, U, return_type> end()
    {
        return wrapped_iterator<T, U, return_type>(m_itEnd, maOp);
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
    MatrixOpWrapper(MatrixImplType& rMat, T const & aOp):
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
                MatrixIteratorWrapper<block_type, T, typename T::number_value_type> aFunc(it, itEnd, maOp);
                pos = mrMat.set(pos,aFunc.begin(), aFunc.end());
            }
            break;
            case mdds::mtm::element_boolean:
            {
                typedef MatrixImplType::boolean_block_type block_type;

                block_type::const_iterator it = block_type::begin(*node.data);
                block_type::const_iterator itEnd = block_type::end(*node.data);

                MatrixIteratorWrapper<block_type, T, typename T::number_value_type> aFunc(it, itEnd, maOp);
                pos = mrMat.set(pos, aFunc.begin(), aFunc.end());
            }
            break;
            case mdds::mtm::element_string:
            {
                typedef MatrixImplType::string_block_type block_type;

                block_type::const_iterator it = block_type::begin(*node.data);
                block_type::const_iterator itEnd = block_type::end(*node.data);

                MatrixIteratorWrapper<block_type, T, typename T::number_value_type> aFunc(it, itEnd, maOp);
                pos = mrMat.set(pos, aFunc.begin(), aFunc.end());
            }
            break;
            case mdds::mtm::element_empty:
            {
                if (maOp.useFunctionForEmpty())
                {
                    std::vector<char> aVec(node.size);
                    MatrixIteratorWrapper<std::vector<char>, T, typename T::number_value_type>
                        aFunc(aVec.begin(), aVec.end(), maOp);
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

template<typename T>
std::vector<ScMatrix::IterateResult> ScMatrixImpl::ApplyCollectOperation(bool bTextAsZero, const std::vector<std::unique_ptr<T>>& aOp)
{
    WalkElementBlocksMultipleValues<T> aFunc(bTextAsZero, aOp);
    maMat.walk(aFunc);
    return aFunc.getResult();
}

namespace {

class WalkElementBlockOperation
{
public:

    WalkElementBlockOperation(size_t nRowSize, size_t /*nColSize*/,
            ScFullMatrix::DoubleOpFunction const & aDoubleFunc,
            ScFullMatrix::BoolOpFunction const & aBoolFunc,
            ScFullMatrix::StringOpFunction const & aStringFunc,
            ScFullMatrix::EmptyOpFunction const & aEmptyFunc):
        mnRowSize(nRowSize),
        mnRowPos(0),
        mnColPos(0),
        maDoubleFunc(aDoubleFunc),
        maBoolFunc(aBoolFunc),
        maStringFunc(aStringFunc),
        maEmptyFunc(aEmptyFunc)
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
                std::advance(it, node.offset);
                block_type::const_iterator itEnd = it;
                std::advance(itEnd, node.size);
                for (auto itr = it; itr != itEnd; ++itr)
                {
                    maDoubleFunc(mnRowPos, mnColPos, *itr);
                    ++mnRowPos;
                    if (mnRowPos >= mnRowSize)
                    {
                        mnRowPos = 0;
                        ++mnColPos;
                    }
                }
            }
            break;
            case mdds::mtm::element_string:
            {
                typedef MatrixImplType::string_block_type block_type;

                block_type::const_iterator it = block_type::begin(*node.data);
                std::advance(it, node.offset);
                block_type::const_iterator itEnd = it;
                std::advance(itEnd, node.size);
                for (auto itr = it; itr != itEnd; ++itr)
                {
                    maStringFunc(mnRowPos, mnColPos, *itr);
                    ++mnRowPos;
                    if (mnRowPos >= mnRowSize)
                    {
                        mnRowPos = 0;
                        ++mnColPos;
                    }
                }
            }
            break;
            case mdds::mtm::element_boolean:
            {
                typedef MatrixImplType::boolean_block_type block_type;

                block_type::const_iterator it = block_type::begin(*node.data);
                std::advance(it, node.offset);
                block_type::const_iterator itEnd = it;
                std::advance(itEnd, node.size);
                for (auto itr = it; itr != itEnd; ++itr)
                {
                    maBoolFunc(mnRowPos, mnColPos, *itr);
                    ++mnRowPos;
                    if (mnRowPos >= mnRowSize)
                    {
                        mnRowPos = 0;
                        ++mnColPos;
                    }
                }
            }
            break;
            case mdds::mtm::element_empty:
            {
                for (size_t i=0; i < node.size; ++i)
                {
                    maEmptyFunc(mnRowPos, mnColPos);
                    ++mnRowPos;
                    if (mnRowPos >= mnRowSize)
                    {
                        mnRowPos = 0;
                        ++mnColPos;
                    }
                }
            }
            break;
            case mdds::mtm::element_integer:
            {
                SAL_WARN("sc.core","WalkElementBlockOperation - unhandled element_integer");
                // No function (yet?), but advance row and column count.
                mnColPos += node.size / mnRowSize;
                mnRowPos += node.size % mnRowSize;
                if (mnRowPos >= mnRowSize)
                {
                    mnRowPos = 0;
                    ++mnColPos;
                }
            }
            break;
        }
    }

private:

    size_t mnRowSize;
    size_t mnRowPos;
    size_t mnColPos;

    ScFullMatrix::DoubleOpFunction maDoubleFunc;
    ScFullMatrix::BoolOpFunction maBoolFunc;
    ScFullMatrix::StringOpFunction maStringFunc;
    ScFullMatrix::EmptyOpFunction maEmptyFunc;
};

}

void ScMatrixImpl::ExecuteOperation(const std::pair<size_t, size_t>& rStartPos,
        const std::pair<size_t, size_t>& rEndPos, const ScMatrix::DoubleOpFunction& aDoubleFunc,
        const ScMatrix::BoolOpFunction& aBoolFunc, const ScMatrix::StringOpFunction& aStringFunc,
        const ScMatrix::EmptyOpFunction& aEmptyFunc) const
{
    WalkElementBlockOperation aFunc(maMat.size().row, maMat.size().column,
            aDoubleFunc, aBoolFunc, aStringFunc, aEmptyFunc);
    maMat.walk(aFunc, MatrixImplType::size_pair_type(rStartPos.first, rStartPos.second),
            MatrixImplType::size_pair_type(rEndPos.first, rEndPos.second));
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
    SAL_WARN_IF( !nRowSize, "sc.core", "ScMatrixImpl::CalcPosition: 0 rows!");
    rC = nRowSize > 1 ? nIndex / nRowSize : nIndex;
    rR = nIndex - rC*nRowSize;
}

namespace {

size_t get_index(SCSIZE nMaxRow, SCSIZE /*nMaxCol*/, size_t nRow, size_t nCol, size_t nRowOffset, size_t nColOffset)
{
    return nMaxRow * (nCol + nColOffset) + nRow + nRowOffset;
}

}

void ScMatrixImpl::MatConcat(SCSIZE nMaxCol, SCSIZE nMaxRow, const ScMatrixRef& xMat1, const ScMatrixRef& xMat2,
            SvNumberFormatter& rFormatter, svl::SharedStringPool& rStringPool)
{
    SCSIZE nC1, nC2;
    SCSIZE nR1, nR2;
    xMat1->GetDimensions(nC1, nR1);
    xMat2->GetDimensions(nC2, nR2);

    sal_uLong nKey = rFormatter.GetStandardFormat( css::util::NumberFormat::NUMBER,
            ScGlobal::eLnge);

    std::vector<OUString> aString(nMaxCol * nMaxRow);
    std::vector<bool> aValid(nMaxCol * nMaxRow, true);
    std::vector<FormulaError> nErrors(nMaxCol * nMaxRow,FormulaError::NONE);

    size_t nRowOffset = 0;
    size_t nColOffset = 0;
    std::function<void(size_t, size_t, double)> aDoubleFunc =
        [&](size_t nRow, size_t nCol, double nVal)
        {
            FormulaError nErr = GetDoubleErrorValue(nVal);
            if (nErr != FormulaError::NONE)
            {
                aValid[get_index(nMaxRow, nMaxCol, nRow, nCol, nRowOffset, nColOffset)] = false;
                nErrors[get_index(nMaxRow, nMaxCol, nRow, nCol, nRowOffset, nColOffset)] = nErr;
                return;
            }
            OUString aStr;
            rFormatter.GetInputLineString( nVal, nKey, aStr);
            aString[get_index(nMaxRow, nMaxCol, nRow, nCol, nRowOffset, nColOffset)] = aString[get_index(nMaxRow, nMaxCol, nRow, nCol, nRowOffset, nColOffset)] + aStr;
        };

    std::function<void(size_t, size_t, bool)> aBoolFunc =
        [&](size_t nRow, size_t nCol, bool nVal)
        {
            OUString aStr;
            rFormatter.GetInputLineString( nVal ? 1.0 : 0.0, nKey, aStr);
            aString[get_index(nMaxRow, nMaxCol, nRow, nCol, nRowOffset, nColOffset)] = aString[get_index(nMaxRow, nMaxCol, nRow, nCol, nRowOffset, nColOffset)] + aStr;
        };

    std::function<void(size_t, size_t, const svl::SharedString&)> aStringFunc =
        [&](size_t nRow, size_t nCol, const svl::SharedString& aStr)
        {
            aString[get_index(nMaxRow, nMaxCol, nRow, nCol, nRowOffset, nColOffset)] = aString[get_index(nMaxRow, nMaxCol, nRow, nCol, nRowOffset, nColOffset)] + aStr.getString();
        };

    std::function<void(size_t, size_t)> aEmptyFunc =
        [&](size_t /*nRow*/, size_t /*nCol*/)
        {
            // Nothing. Concatenating an empty string to an existing string.
        };


    if (nC1 == 1 || nR1 == 1)
    {
        size_t nRowRep = nR1 == 1 ? nMaxRow : 1;
        size_t nColRep = nC1 == 1 ? nMaxCol : 1;

        for (size_t i = 0; i < nRowRep; ++i)
        {
            nRowOffset = i;
            for (size_t j = 0; j < nColRep; ++j)
            {
                nColOffset = j;
                xMat1->ExecuteOperation(
                        std::pair<size_t, size_t>(0, 0),
                        std::pair<size_t, size_t>(std::min(nR1, nMaxRow) - 1, std::min(nC1, nMaxCol) - 1),
                        aDoubleFunc, aBoolFunc, aStringFunc, aEmptyFunc);
            }
        }
    }
    else
        xMat1->ExecuteOperation(
                std::pair<size_t, size_t>(0, 0),
                std::pair<size_t, size_t>(nMaxRow - 1, nMaxCol - 1),
                aDoubleFunc, aBoolFunc, aStringFunc, aEmptyFunc);

    std::vector<svl::SharedString> aSharedString(nMaxCol*nMaxRow);

    std::function<void(size_t, size_t, double)> aDoubleFunc2 =
        [&](size_t nRow, size_t nCol, double nVal)
        {
            FormulaError nErr = GetDoubleErrorValue(nVal);
            if (nErr != FormulaError::NONE)
            {
                aValid[get_index(nMaxRow, nMaxCol, nRow, nCol, nRowOffset, nColOffset)] = false;
                nErrors[get_index(nMaxRow, nMaxCol, nRow, nCol, nRowOffset, nColOffset)] = nErr;
                return;
            }
            OUString aStr;
            rFormatter.GetInputLineString( nVal, nKey, aStr);
            aSharedString[get_index(nMaxRow, nMaxCol, nRow, nCol, nRowOffset, nColOffset)] = rStringPool.intern(aString[get_index(nMaxRow, nMaxCol, nRow, nCol, nRowOffset, nColOffset)] + aStr);
        };

    std::function<void(size_t, size_t, bool)> aBoolFunc2 =
        [&](size_t nRow, size_t nCol, bool nVal)
        {
            OUString aStr;
            rFormatter.GetInputLineString( nVal ? 1.0 : 0.0, nKey, aStr);
            aSharedString[get_index(nMaxRow, nMaxCol, nRow, nCol, nRowOffset, nColOffset)] = rStringPool.intern(aString[get_index(nMaxRow, nMaxCol, nRow, nCol, nRowOffset, nColOffset)] + aStr);
        };

    std::function<void(size_t, size_t, const svl::SharedString&)> aStringFunc2 =
        [&](size_t nRow, size_t nCol, const svl::SharedString& aStr)
        {
            aSharedString[get_index(nMaxRow, nMaxCol, nRow, nCol, nRowOffset, nColOffset)] =
                rStringPool.intern(aString[get_index(nMaxRow, nMaxCol, nRow, nCol, nRowOffset, nColOffset)] + aStr.getString());
        };

    std::function<void(size_t, size_t)> aEmptyFunc2 =
        [&](size_t nRow, size_t nCol)
        {
            aSharedString[get_index(nMaxRow, nMaxCol, nRow, nCol, nRowOffset, nColOffset)] =
                rStringPool.intern(aString[get_index(nMaxRow, nMaxCol, nRow, nCol, nRowOffset, nColOffset)]);
        };

    nRowOffset = 0;
    nColOffset = 0;
    if (nC2 == 1 || nR2 == 1)
    {
        size_t nRowRep = nR2 == 1 ? nMaxRow : 1;
        size_t nColRep = nC2 == 1 ? nMaxCol : 1;

        for (size_t i = 0; i < nRowRep; ++i)
        {
            nRowOffset = i;
            for (size_t j = 0; j < nColRep; ++j)
            {
                nColOffset = j;
                xMat2->ExecuteOperation(
                        std::pair<size_t, size_t>(0, 0),
                        std::pair<size_t, size_t>(std::min(nR2, nMaxRow) - 1, std::min(nC2, nMaxCol) - 1),
                        aDoubleFunc2, aBoolFunc2, aStringFunc2, aEmptyFunc2);
            }
        }
    }
    else
        xMat2->ExecuteOperation(
                std::pair<size_t, size_t>(0, 0),
                std::pair<size_t, size_t>(nMaxRow - 1, nMaxCol - 1),
                aDoubleFunc2, aBoolFunc2, aStringFunc2, aEmptyFunc2);

    aString.clear();

    MatrixImplType::position_type pos = maMat.position(0, 0);
    for (SCSIZE i = 0; i < nMaxCol; ++i)
    {
        for (SCSIZE j = 0; j < nMaxRow && i < nMaxCol; ++j)
        {
            if (aValid[nMaxRow * i + j])
            {
                auto itr = aValid.begin();
                std::advance(itr, nMaxRow * i + j);
                auto itrEnd = std::find(itr, aValid.end(), false);
                size_t nSteps = std::distance(itr, itrEnd);
                auto itrStr = aSharedString.begin();
                std::advance(itrStr, nMaxRow * i + j);
                auto itrEndStr = itrStr;
                std::advance(itrEndStr, nSteps);
                pos = maMat.set(pos, itrStr, itrEndStr);
                size_t nColSteps = nSteps / nMaxRow;
                i += nColSteps;
                j += nSteps % nMaxRow;
                if (j >= nMaxRow)
                {
                    j -= nMaxRow;
                    ++i;
                }
            }
            else
            {
                pos = maMat.set(pos, CreateDoubleError(nErrors[nMaxRow * i + j]));
            }
            pos = MatrixImplType::next_position(pos);
        }
    }
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

bool ScMatrix::IsSizeAllocatable( SCSIZE nC, SCSIZE nR )
{
    SAL_WARN_IF( !nC, "sc.core", "ScMatrix with 0 columns!");
    SAL_WARN_IF( !nR, "sc.core", "ScMatrix with 0 rows!");
    // 0-size matrix is valid, it could be resized later.
    if ((nC && !nR) || (!nC && nR))
    {
        SAL_WARN( "sc.core", "ScMatrix one-dimensional zero: " << nC << " columns * " << nR << " rows");
        return false;
    }
    if (nC && nR && (nC > (ScMatrix::GetElementsMax() / nR)))
    {
        SAL_WARN( "sc.core", "ScMatrix overflow: " << nC << " columns * " << nR << " rows");
        return false;
    }
    return true;
}

ScFullMatrix::ScFullMatrix( SCSIZE nC, SCSIZE nR) :
    ScMatrix()
{
    if (ScMatrix::IsSizeAllocatable( nC, nR))
        pImpl.reset( new ScMatrixImpl( nC, nR));
    else
        // Invalid matrix size, allocate 1x1 matrix with error value.
        pImpl.reset( new ScMatrixImpl( 1,1, CreateDoubleError( FormulaError::MatrixSize)));
}

ScFullMatrix::ScFullMatrix(SCSIZE nC, SCSIZE nR, double fInitVal) :
    ScMatrix()
{
    if (ScMatrix::IsSizeAllocatable( nC, nR))
        pImpl.reset( new ScMatrixImpl( nC, nR, fInitVal));
    else
        // Invalid matrix size, allocate 1x1 matrix with error value.
        pImpl.reset( new ScMatrixImpl( 1,1, CreateDoubleError( FormulaError::MatrixSize)));
}

ScFullMatrix::ScFullMatrix( size_t nC, size_t nR, const std::vector<double>& rInitVals ) :
    ScMatrix()
{
    if (ScMatrix::IsSizeAllocatable( nC, nR))
        pImpl.reset( new ScMatrixImpl( nC, nR, rInitVals));
    else
        // Invalid matrix size, allocate 1x1 matrix with error value.
        pImpl.reset( new ScMatrixImpl( 1,1, CreateDoubleError( FormulaError::MatrixSize)));
}

ScFullMatrix::~ScFullMatrix()
{
}

ScMatrix* ScFullMatrix::Clone() const
{
    SCSIZE nC, nR;
    pImpl->GetDimensions(nC, nR);
    ScMatrix* pScMat = new ScFullMatrix(nC, nR);
    MatCopy(*pScMat);
    pScMat->SetErrorInterpreter(pImpl->GetErrorInterpreter());    // TODO: really?
    return pScMat;
}

ScMatrix* ScMatrix::CloneIfConst()
{
    return mbCloneIfConst ? Clone() : this;
}

void ScMatrix::SetMutable()
{
    mbCloneIfConst = false;
}

void ScMatrix::SetImmutable() const
{
    mbCloneIfConst = true;
}

void ScFullMatrix::Resize( SCSIZE nC, SCSIZE nR)
{
    pImpl->Resize(nC, nR);
}

void ScFullMatrix::Resize(SCSIZE nC, SCSIZE nR, double fVal)
{
    pImpl->Resize(nC, nR, fVal);
}

ScMatrix* ScFullMatrix::CloneAndExtend(SCSIZE nNewCols, SCSIZE nNewRows) const
{
    ScMatrix* pScMat = new ScFullMatrix(nNewCols, nNewRows);
    MatCopy(*pScMat);
    pScMat->SetErrorInterpreter(pImpl->GetErrorInterpreter());
    return pScMat;
}

void ScFullMatrix::SetErrorInterpreter( ScInterpreter* p)
{
    pImpl->SetErrorInterpreter(p);
}

void ScFullMatrix::GetDimensions( SCSIZE& rC, SCSIZE& rR) const
{
    pImpl->GetDimensions(rC, rR);
}

SCSIZE ScFullMatrix::GetElementCount() const
{
    return pImpl->GetElementCount();
}

bool ScFullMatrix::ValidColRow( SCSIZE nC, SCSIZE nR) const
{
    return pImpl->ValidColRow(nC, nR);
}

bool ScFullMatrix::ValidColRowReplicated( SCSIZE & rC, SCSIZE & rR ) const
{
    return pImpl->ValidColRowReplicated(rC, rR);
}

bool ScFullMatrix::ValidColRowOrReplicated( SCSIZE & rC, SCSIZE & rR ) const
{
    return ValidColRow( rC, rR) || ValidColRowReplicated( rC, rR);
}

void ScFullMatrix::PutDouble(double fVal, SCSIZE nC, SCSIZE nR)
{
    pImpl->PutDouble(fVal, nC, nR);
}

void ScFullMatrix::PutDouble( double fVal, SCSIZE nIndex)
{
    pImpl->PutDouble(fVal, nIndex);
}

void ScFullMatrix::PutDouble(const double* pArray, size_t nLen, SCSIZE nC, SCSIZE nR)
{
    pImpl->PutDouble(pArray, nLen, nC, nR);
}

void ScFullMatrix::PutString(const svl::SharedString& rStr, SCSIZE nC, SCSIZE nR)
{
    pImpl->PutString(rStr, nC, nR);
}

void ScFullMatrix::PutString(const svl::SharedString& rStr, SCSIZE nIndex)
{
    pImpl->PutString(rStr, nIndex);
}

void ScFullMatrix::PutString(const svl::SharedString* pArray, size_t nLen, SCSIZE nC, SCSIZE nR)
{
    pImpl->PutString(pArray, nLen, nC, nR);
}

void ScFullMatrix::PutEmpty(SCSIZE nC, SCSIZE nR)
{
    pImpl->PutEmpty(nC, nR);
}

void ScFullMatrix::PutEmptyPath(SCSIZE nC, SCSIZE nR)
{
    pImpl->PutEmptyPath(nC, nR);
}

void ScFullMatrix::PutError( FormulaError nErrorCode, SCSIZE nC, SCSIZE nR )
{
    pImpl->PutError(nErrorCode, nC, nR);
}

void ScFullMatrix::PutBoolean(bool bVal, SCSIZE nC, SCSIZE nR)
{
    pImpl->PutBoolean(bVal, nC, nR);
}

FormulaError ScFullMatrix::GetError( SCSIZE nC, SCSIZE nR) const
{
    return pImpl->GetError(nC, nR);
}

double ScFullMatrix::GetDouble(SCSIZE nC, SCSIZE nR) const
{
    return pImpl->GetDouble(nC, nR);
}

double ScFullMatrix::GetDouble( SCSIZE nIndex) const
{
    return pImpl->GetDouble(nIndex);
}

double ScFullMatrix::GetDoubleWithStringConversion(SCSIZE nC, SCSIZE nR) const
{
    return pImpl->GetDoubleWithStringConversion(nC, nR);
}

svl::SharedString ScFullMatrix::GetString(SCSIZE nC, SCSIZE nR) const
{
    return pImpl->GetString(nC, nR);
}

svl::SharedString ScFullMatrix::GetString( SCSIZE nIndex) const
{
    return pImpl->GetString(nIndex);
}

svl::SharedString ScFullMatrix::GetString( SvNumberFormatter& rFormatter, SCSIZE nC, SCSIZE nR) const
{
    return pImpl->GetString(rFormatter, nC, nR);
}

ScMatrixValue ScFullMatrix::Get(SCSIZE nC, SCSIZE nR) const
{
    return pImpl->Get(nC, nR);
}

bool ScFullMatrix::IsString( SCSIZE nIndex ) const
{
    return pImpl->IsString(nIndex);
}

bool ScFullMatrix::IsString( SCSIZE nC, SCSIZE nR ) const
{
    return pImpl->IsString(nC, nR);
}

bool ScFullMatrix::IsEmpty( SCSIZE nC, SCSIZE nR ) const
{
    return pImpl->IsEmpty(nC, nR);
}

bool ScFullMatrix::IsEmptyCell( SCSIZE nC, SCSIZE nR ) const
{
    return pImpl->IsEmptyCell(nC, nR);
}

bool ScFullMatrix::IsEmptyResult( SCSIZE nC, SCSIZE nR ) const
{
    return pImpl->IsEmptyResult(nC, nR);
}

bool ScFullMatrix::IsEmptyPath( SCSIZE nC, SCSIZE nR ) const
{
    return pImpl->IsEmptyPath(nC, nR);
}

bool ScFullMatrix::IsValue( SCSIZE nIndex ) const
{
    return pImpl->IsValue(nIndex);
}

bool ScFullMatrix::IsValue( SCSIZE nC, SCSIZE nR ) const
{
    return pImpl->IsValue(nC, nR);
}

bool ScFullMatrix::IsValueOrEmpty( SCSIZE nC, SCSIZE nR ) const
{
    return pImpl->IsValueOrEmpty(nC, nR);
}

bool ScFullMatrix::IsBoolean( SCSIZE nC, SCSIZE nR ) const
{
    return pImpl->IsBoolean(nC, nR);
}

bool ScFullMatrix::IsNumeric() const
{
    return pImpl->IsNumeric();
}

void ScFullMatrix::MatCopy(ScMatrix& mRes) const
{
    // FIXME
    ScFullMatrix* pMatrix = dynamic_cast<ScFullMatrix*>(&mRes);
    assert(pMatrix);
    pImpl->MatCopy(*pMatrix->pImpl);
}

void ScFullMatrix::MatTrans(ScMatrix& mRes) const
{
    // FIXME
    ScFullMatrix* pMatrix = dynamic_cast<ScFullMatrix*>(&mRes);
    assert(pMatrix);
    pImpl->MatTrans(*pMatrix->pImpl);
}

void ScFullMatrix::FillDouble( double fVal, SCSIZE nC1, SCSIZE nR1, SCSIZE nC2, SCSIZE nR2 )
{
    pImpl->FillDouble(fVal, nC1, nR1, nC2, nR2);
}

void ScFullMatrix::PutDoubleVector( const ::std::vector< double > & rVec, SCSIZE nC, SCSIZE nR )
{
    pImpl->PutDoubleVector(rVec, nC, nR);
}

void ScFullMatrix::PutStringVector( const ::std::vector< svl::SharedString > & rVec, SCSIZE nC, SCSIZE nR )
{
    pImpl->PutStringVector(rVec, nC, nR);
}

void ScFullMatrix::PutEmptyVector( SCSIZE nCount, SCSIZE nC, SCSIZE nR )
{
    pImpl->PutEmptyVector(nCount, nC, nR);
}

void ScFullMatrix::PutEmptyResultVector( SCSIZE nCount, SCSIZE nC, SCSIZE nR )
{
    pImpl->PutEmptyResultVector(nCount, nC, nR);
}

void ScFullMatrix::PutEmptyPathVector( SCSIZE nCount, SCSIZE nC, SCSIZE nR )
{
    pImpl->PutEmptyPathVector(nCount, nC, nR);
}

void ScFullMatrix::CompareEqual()
{
    pImpl->CompareEqual();
}

void ScFullMatrix::CompareNotEqual()
{
    pImpl->CompareNotEqual();
}

void ScFullMatrix::CompareLess()
{
    pImpl->CompareLess();
}

void ScFullMatrix::CompareGreater()
{
    pImpl->CompareGreater();
}

void ScFullMatrix::CompareLessEqual()
{
    pImpl->CompareLessEqual();
}

void ScFullMatrix::CompareGreaterEqual()
{
    pImpl->CompareGreaterEqual();
}

double ScFullMatrix::And() const
{
    return pImpl->And();
}

double ScFullMatrix::Or() const
{
    return pImpl->Or();
}

double ScFullMatrix::Xor() const
{
    return pImpl->Xor();
}

ScMatrix::IterateResult ScFullMatrix::Sum(bool bTextAsZero) const
{
    return pImpl->Sum(bTextAsZero);
}

ScMatrix::IterateResult ScFullMatrix::SumSquare(bool bTextAsZero) const
{
    return pImpl->SumSquare(bTextAsZero);
}

ScMatrix::IterateResult ScFullMatrix::Product(bool bTextAsZero) const
{
    return pImpl->Product(bTextAsZero);
}

size_t ScFullMatrix::Count(bool bCountStrings, bool bCountErrors) const
{
    return pImpl->Count(bCountStrings, bCountErrors);
}

size_t ScFullMatrix::MatchDoubleInColumns(double fValue, size_t nCol1, size_t nCol2) const
{
    return pImpl->MatchDoubleInColumns(fValue, nCol1, nCol2);
}

size_t ScFullMatrix::MatchStringInColumns(const svl::SharedString& rStr, size_t nCol1, size_t nCol2) const
{
    return pImpl->MatchStringInColumns(rStr, nCol1, nCol2);
}

double ScFullMatrix::GetMaxValue( bool bTextAsZero ) const
{
    return pImpl->GetMaxValue(bTextAsZero);
}

double ScFullMatrix::GetMinValue( bool bTextAsZero ) const
{
    return pImpl->GetMinValue(bTextAsZero);
}

double ScFullMatrix::GetGcd() const
{
    return pImpl->GetGcd();
}

ScMatrixRef ScFullMatrix::CompareMatrix(
    sc::Compare& rComp, size_t nMatPos, sc::CompareOptions* pOptions ) const
{
    return pImpl->CompareMatrix(rComp, nMatPos, pOptions);
}

void ScFullMatrix::GetDoubleArray( std::vector<double>& rArray, bool bEmptyAsZero ) const
{
    pImpl->GetDoubleArray(rArray, bEmptyAsZero);
}

void ScFullMatrix::MergeDoubleArray( std::vector<double>& rArray, Op eOp ) const
{
    pImpl->MergeDoubleArray(rArray, eOp);
}

namespace matop {

/**
 * COp struct is used in MatOp class to provide (through template specialization)
 * different actions for empty entries in a matrix.
 */
template <typename T, typename S>
struct COp {};

template <typename T>
struct COp<T, svl::SharedString>
{
    const svl::SharedString& operator()(char, T /*aOp*/, double /*a*/, double /*b*/, const svl::SharedString& rString) const
    {
        return rString;
    }
};

template <typename T>
struct COp<T, double>
{
    double operator()(char, T aOp, double a, double b, const svl::SharedString& /*rString*/) const
    {
        return aOp( a, b);
    }
};

/** A template for operations where operands are supposed to be numeric.
    A non-numeric (string) operand leads to the configured conversion to number
    method being called if in interpreter context and an FormulaError::NoValue DoubleError
    if conversion was not possible, else to an unconditional FormulaError::NoValue
    DoubleError.
    An empty operand evaluates to 0.
    XXX: semantically TEmptyRes and types other than number_value_type are
    unused, but this template could serve as a basis for future enhancements.
 */
template<typename TOp, typename TEmptyRes=double, typename TRet=double>
struct MatOp
{
private:
    TOp maOp;
    ScInterpreter* mpErrorInterpreter;
    svl::SharedString maString;
    double mfVal;
    COp<TOp, TEmptyRes> maCOp;

public:
    typedef TEmptyRes empty_value_type;
    typedef TRet number_value_type;
    typedef svl::SharedString string_value_type;

    MatOp( TOp aOp, ScInterpreter* pErrorInterpreter,
            double fVal = 0.0, const svl::SharedString& rString = svl::SharedString() ):
        maOp(aOp),
        mpErrorInterpreter(pErrorInterpreter),
        maString(rString),
        mfVal(fVal)
    {
        if (mpErrorInterpreter)
        {
            FormulaError nErr = mpErrorInterpreter->GetError();
            if (nErr != FormulaError::NONE)
                mfVal = CreateDoubleError( nErr);
        }
    }

    TRet operator()(double fVal) const
    {
        return maOp(fVal, mfVal);
    }

    TRet operator()(bool bVal) const
    {
        return maOp((double)bVal, mfVal);
    }

    double operator()(const svl::SharedString& rStr) const
    {
        return convertStringToValue( mpErrorInterpreter, rStr.getString());
    }

    TEmptyRes operator()(char) const
    {
        return maCOp(char{}, maOp, 0, mfVal, maString);
    }

    static bool useFunctionForEmpty()
    {
        return true;
    }
};

}

void ScFullMatrix::NotOp( ScMatrix& rMat)
{
    auto not_ = [](double a, double){return double(a == 0.0);};
    matop::MatOp<decltype(not_), double> aOp(not_, pImpl->GetErrorInterpreter());
    // FIXME
    ScFullMatrix* pMatrix = dynamic_cast<ScFullMatrix*>(&rMat);
    assert(pMatrix);
    pImpl->ApplyOperation(aOp, *pMatrix->pImpl);
}

void ScFullMatrix::NegOp( ScMatrix& rMat)
{
    auto neg_ = [](double a, double){return -a;};
    matop::MatOp<decltype(neg_), double> aOp(neg_, pImpl->GetErrorInterpreter());
    // FIXME
    ScFullMatrix* pMatrix = dynamic_cast<ScFullMatrix*>(&rMat);
    assert(pMatrix);
    pImpl->ApplyOperation(aOp, *pMatrix->pImpl);
}

void ScFullMatrix::AddOp( double fVal, ScMatrix& rMat)
{
    auto add_ = [](double a, double b){return a + b;};
    matop::MatOp<decltype(add_)> aOp(add_, pImpl->GetErrorInterpreter(), fVal);
    // FIXME
    ScFullMatrix* pMatrix = dynamic_cast<ScFullMatrix*>(&rMat);
    assert(pMatrix);
    pImpl->ApplyOperation(aOp, *pMatrix->pImpl);
}

void ScFullMatrix::SubOp( bool bFlag, double fVal, ScMatrix& rMat)
{
    if (bFlag)
    {
        auto sub_ = [](double a, double b){return b - a;};
        matop::MatOp<decltype(sub_)> aOp(sub_, pImpl->GetErrorInterpreter(), fVal);
        // FIXME
        ScFullMatrix* pMatrix = dynamic_cast<ScFullMatrix*>(&rMat);
        assert(pMatrix);
        pImpl->ApplyOperation(aOp, *pMatrix->pImpl);
    }
    else
    {
        auto sub_ = [](double a, double b){return a - b;};
        matop::MatOp<decltype(sub_)> aOp(sub_, pImpl->GetErrorInterpreter(), fVal);
        // FIXME
        ScFullMatrix* pMatrix = dynamic_cast<ScFullMatrix*>(&rMat);
        assert(pMatrix);
        pImpl->ApplyOperation(aOp, *pMatrix->pImpl);
    }
}

void ScFullMatrix::MulOp( double fVal, ScMatrix& rMat)
{
    auto mul_ = [](double a, double b){return a * b;};
    matop::MatOp<decltype(mul_)> aOp(mul_, pImpl->GetErrorInterpreter(), fVal);
    // FIXME
    ScFullMatrix* pMatrix = dynamic_cast<ScFullMatrix*>(&rMat);
    assert(pMatrix);
    pImpl->ApplyOperation(aOp, *pMatrix->pImpl);
}

void ScFullMatrix::DivOp( bool bFlag, double fVal, ScMatrix& rMat)
{
    if (bFlag)
    {
        auto div_ = [](double a, double b){return sc::div(b, a);};
        matop::MatOp<decltype(div_)> aOp(div_, pImpl->GetErrorInterpreter(), fVal);
        // FIXME
        ScFullMatrix* pMatrix = dynamic_cast<ScFullMatrix*>(&rMat);
        assert(pMatrix);
        pImpl->ApplyOperation(aOp, *pMatrix->pImpl);
    }
    else
    {
        auto div_ = [](double a, double b){return sc::div(a, b);};
        matop::MatOp<decltype(div_)> aOp(div_, pImpl->GetErrorInterpreter(), fVal);
        // FIXME
        ScFullMatrix* pMatrix = dynamic_cast<ScFullMatrix*>(&rMat);
        assert(pMatrix);
        pImpl->ApplyOperation(aOp, *pMatrix->pImpl);
    }
}

void ScFullMatrix::PowOp( bool bFlag, double fVal, ScMatrix& rMat)
{
    if (bFlag)
    {
        auto pow_ = [](double a, double b){return pow(b, a);};
        matop::MatOp<decltype(pow_)> aOp(pow_, pImpl->GetErrorInterpreter(), fVal);
        // FIXME
        ScFullMatrix* pMatrix = dynamic_cast<ScFullMatrix*>(&rMat);
        assert(pMatrix);
        pImpl->ApplyOperation(aOp, *pMatrix->pImpl);
    }
    else
    {
        auto pow_ = [](double a, double b){return pow(a, b);};
        matop::MatOp<decltype(pow_)> aOp(pow_, pImpl->GetErrorInterpreter(), fVal);
        // FIXME
        ScFullMatrix* pMatrix = dynamic_cast<ScFullMatrix*>(&rMat);
        assert(pMatrix);
        pImpl->ApplyOperation(aOp, *pMatrix->pImpl);
    }
}

void ScFullMatrix::ExecuteOperation(const std::pair<size_t, size_t>& rStartPos,
        const std::pair<size_t, size_t>& rEndPos, DoubleOpFunction aDoubleFunc,
        BoolOpFunction aBoolFunc, StringOpFunction aStringFunc, EmptyOpFunction aEmptyFunc) const
{
    pImpl->ExecuteOperation(rStartPos, rEndPos, aDoubleFunc, aBoolFunc, aStringFunc, aEmptyFunc);
}

std::vector<ScMatrix::IterateResult> ScFullMatrix::Collect(bool bTextAsZero, const std::vector<std::unique_ptr<sc::op::Op>>& aOp)
{
    return pImpl->ApplyCollectOperation(bTextAsZero, aOp);
}

ScFullMatrix& ScFullMatrix::operator+= ( const ScFullMatrix& r )
{
    pImpl->AddValues(*r.pImpl);
    return *this;
}

#if DEBUG_MATRIX
void ScFullMatrix::Dump() const
{
    pImpl->Dump();
}
#endif

namespace {

/**
 * Input double array consists of segments of NaN's and normal values.
 * Insert only the normal values into the matrix while skipping the NaN's.
 */
void fillMatrix( ScMatrix& rMat, size_t nCol, const double* pNums, size_t nLen )
{
    const double* pNum = pNums;
    const double* pNumEnd = pNum + nLen;
    const double* pNumHead = nullptr;
    for (; pNum != pNumEnd; ++pNum)
    {
        if (!rtl::math::isNan(*pNum))
        {
            if (!pNumHead)
                // Store the first non-NaN position.
                pNumHead = pNum;

            continue;
        }

        if (pNumHead)
        {
            // Flush this non-NaN segment to the matrix.
            rMat.PutDouble(pNumHead, pNum - pNumHead, nCol, pNumHead - pNums);
            pNumHead = nullptr;
        }
    }

    if (pNumHead)
    {
        // Flush last non-NaN segment to the matrix.
        rMat.PutDouble(pNumHead, pNum - pNumHead, nCol, pNumHead - pNums);
    }
}

void flushStrSegment(
    ScMatrix& rMat, size_t nCol, rtl_uString** pHead, rtl_uString** pCur, rtl_uString** pTop )
{
    size_t nOffset = pHead - pTop;
    std::vector<svl::SharedString> aStrs;
    aStrs.reserve(pCur - pHead);
    for (; pHead != pCur; ++pHead)
        aStrs.push_back(svl::SharedString(*pHead, *pHead));

    rMat.PutString(&aStrs[0], aStrs.size(), nCol, nOffset);
}

void fillMatrix( ScMatrix& rMat, size_t nCol, rtl_uString** pStrs, size_t nLen )
{
    rtl_uString** p = pStrs;
    rtl_uString** pEnd = p + nLen;
    rtl_uString** pHead = nullptr;
    for (; p != pEnd; ++p)
    {
        if (*p)
        {
            if (!pHead)
                // Store the first non-empty string position.
                pHead = p;

            continue;
        }

        if (pHead)
        {
            // Flush this non-empty segment to the matrix.
            flushStrSegment(rMat, nCol, pHead, p, pStrs);
            pHead = nullptr;
        }
    }

    if (pHead)
    {
        // Flush last non-empty segment to the matrix.
        flushStrSegment(rMat, nCol, pHead, p, pStrs);
    }
}

void fillMatrix( ScMatrix& rMat, size_t nCol, const double* pNums, rtl_uString** pStrs, size_t nLen )
{
    if (!pStrs)
    {
        fillMatrix(rMat, nCol, pNums, nLen);
        return;
    }

    const double* pNum = pNums;
    const double* pNumHead = nullptr;
    rtl_uString** pStr = pStrs;
    rtl_uString** pStrEnd = pStr + nLen;
    rtl_uString** pStrHead = nullptr;

    for (; pStr != pStrEnd; ++pStr, ++pNum)
    {
        if (*pStr)
        {
            // String cell exists.

            if (pNumHead)
            {
                // Flush this numeric segment to the matrix.
                rMat.PutDouble(pNumHead, pNum - pNumHead, nCol, pNumHead - pNums);
                pNumHead = nullptr;
            }

            if (!pStrHead)
                // Store the first non-empty string position.
                pStrHead = pStr;

            continue;
        }

        // No string cell. Check the numeric cell value.

        if (pStrHead)
        {
            // Flush this non-empty string segment to the matrix.
            flushStrSegment(rMat, nCol, pStrHead, pStr, pStrs);
            pStrHead = nullptr;
        }

        if (!rtl::math::isNan(*pNum))
        {
            // Numeric cell exists.
            if (!pNumHead)
                // Store the first non-NaN position.
                pNumHead = pNum;

            continue;
        }

        // it's a NaN, need to flush the non-NaN segment if it exists

        if (pNumHead)
        {
            // Flush this non-NaN segment to the matrix.
            rMat.PutDouble(pNumHead, pNum - pNumHead, nCol, pNumHead - pNums);
            pNumHead = nullptr;
        }
    }

    if (pStrHead)
    {
        // Flush the last non-empty segment to the matrix.
        flushStrSegment(rMat, nCol, pStrHead, pStr, pStrs);
    }
    else if (pNumHead)
    {
        // Flush the last numeric segment to the matrix.
        rMat.PutDouble(pNumHead, pNum - pNumHead, nCol, pNumHead - pNums);
    }
}

} // anonymous namespace

void ScVectorRefMatrix::ensureFullMatrix()
{
    if (mpFullMatrix)
        return;

    const std::vector<formula::VectorRefArray>& rArrays = mpToken->GetArrays();
    size_t nColSize = rArrays.size();
    mpFullMatrix.reset(new ScFullMatrix(nColSize, mnRowSize));

    if (mpErrorInterpreter)
        mpFullMatrix->SetErrorInterpreter(mpErrorInterpreter);

    size_t nRowSize = mnRowSize;
    size_t nRowEnd = mnRowStart + mnRowSize;
    size_t nDataRowEnd = mpToken->GetArrayLength();

    if (mnRowStart >= nDataRowEnd)
        return;

    if (nRowEnd > nDataRowEnd)
    {
        // Data array is shorter than the row size of the reference. Truncate
        // it to the data.
        nRowSize -= nRowEnd - nDataRowEnd;
    }

    for (size_t nCol = 0; nCol < nColSize; ++nCol)
    {
        const formula::VectorRefArray& rArray = rArrays[nCol];
        if (rArray.mpStringArray)
        {
            if (rArray.mpNumericArray)
            {
                // Mixture of string and numeric values.
                const double* pNums = rArray.mpNumericArray;
                pNums += mnRowStart;
                rtl_uString** pStrs = rArray.mpStringArray;
                pStrs += mnRowStart;
                fillMatrix(*mpFullMatrix, nCol, pNums, pStrs, nRowSize);
            }
            else
            {
                // String cells only.
                rtl_uString** pStrs = rArray.mpStringArray;
                pStrs += mnRowStart;
                fillMatrix(*mpFullMatrix, nCol, pStrs, nRowSize);
            }
        }
        else if (rArray.mpNumericArray)
        {
            // Numeric cells only.
            const double* pNums = rArray.mpNumericArray;
            pNums += mnRowStart;
            fillMatrix(*mpFullMatrix, nCol, pNums, nRowSize);
        }
    }
}

ScVectorRefMatrix::ScVectorRefMatrix(const formula::DoubleVectorRefToken* pToken, SCSIZE nRowStart, SCSIZE nRowSize)
    : ScMatrix()
    , mpToken(pToken)
    , mpErrorInterpreter(nullptr)
    , mnRowStart(nRowStart)
    , mnRowSize(nRowSize)
{
}

ScVectorRefMatrix::~ScVectorRefMatrix()
{
}

ScMatrix* ScVectorRefMatrix::Clone() const
{
    const_cast<ScVectorRefMatrix*>(this)->ensureFullMatrix();
    return mpFullMatrix->Clone();
}

void ScVectorRefMatrix::Resize(SCSIZE nC, SCSIZE nR)
{
    ensureFullMatrix();
    mpFullMatrix->Resize(nC, nR);
}

void ScVectorRefMatrix::Resize(SCSIZE nC, SCSIZE nR, double fVal)
{
    ensureFullMatrix();
    mpFullMatrix->Resize(nC, nR, fVal);
}

ScMatrix* ScVectorRefMatrix::CloneAndExtend(SCSIZE nNewCols, SCSIZE nNewRows) const
{
    const_cast<ScVectorRefMatrix*>(this)->ensureFullMatrix();
    return mpFullMatrix->CloneAndExtend(nNewCols, nNewRows);
}

void ScVectorRefMatrix::SetErrorInterpreter(ScInterpreter* p)
{
    if (mpFullMatrix)
    {
        mpFullMatrix->SetErrorInterpreter(p);
        return;
    }

    mpErrorInterpreter = p;
}

void ScVectorRefMatrix::GetDimensions(SCSIZE& rC, SCSIZE& rR) const
{
    if (mpFullMatrix)
    {
        mpFullMatrix->GetDimensions(rC, rR);
        return;
    }

    rC = mpToken->GetArrays().size();
    rR = mnRowSize;
}

SCSIZE ScVectorRefMatrix::GetElementCount() const
{
    const_cast<ScVectorRefMatrix*>(this)->ensureFullMatrix();
    return mpFullMatrix->GetElementCount();
}

bool ScVectorRefMatrix::ValidColRow(SCSIZE nC, SCSIZE nR) const
{
    const_cast<ScVectorRefMatrix*>(this)->ensureFullMatrix();
    return mpFullMatrix->ValidColRow(nC, nR);
}

bool ScVectorRefMatrix::ValidColRowReplicated(SCSIZE & rC, SCSIZE & rR) const
{
    const_cast<ScVectorRefMatrix*>(this)->ensureFullMatrix();
    return mpFullMatrix->ValidColRowReplicated(rC, rR);
}

bool ScVectorRefMatrix::ValidColRowOrReplicated(SCSIZE & rC, SCSIZE & rR) const
{
    const_cast<ScVectorRefMatrix*>(this)->ensureFullMatrix();
    return mpFullMatrix->ValidColRowOrReplicated(rC, rR);
}

void ScVectorRefMatrix::PutDouble(double fVal, SCSIZE nC, SCSIZE nR)
{
    ensureFullMatrix();
    mpFullMatrix->PutDouble(fVal, nC, nR);
}

void ScVectorRefMatrix::PutDouble(double fVal, SCSIZE nIndex)
{
    ensureFullMatrix();
    mpFullMatrix->PutDouble(fVal, nIndex);
}

void ScVectorRefMatrix::PutDouble(const double* pArray, size_t nLen, SCSIZE nC, SCSIZE nR)
{
    ensureFullMatrix();
    mpFullMatrix->PutDouble(pArray, nLen, nC, nR);
}

void ScVectorRefMatrix::PutString(const svl::SharedString& rStr, SCSIZE nC, SCSIZE nR)
{
    ensureFullMatrix();
    mpFullMatrix->PutString(rStr, nC, nR);
}

void ScVectorRefMatrix::PutString(const svl::SharedString& rStr, SCSIZE nIndex)
{
    ensureFullMatrix();
    mpFullMatrix->PutString(rStr, nIndex);
}

void ScVectorRefMatrix::PutString(const svl::SharedString* pArray, size_t nLen, SCSIZE nC, SCSIZE nR)
{
    ensureFullMatrix();
    mpFullMatrix->PutString(pArray, nLen, nC, nR);
}

void ScVectorRefMatrix::PutEmpty(SCSIZE nC, SCSIZE nR)
{
    ensureFullMatrix();
    mpFullMatrix->PutEmpty(nC, nR);
}

void ScVectorRefMatrix::PutEmptyPath(SCSIZE nC, SCSIZE nR)
{
    ensureFullMatrix();
    mpFullMatrix->PutEmptyPath(nC, nR);
}

void ScVectorRefMatrix::PutError(FormulaError nErrorCode, SCSIZE nC, SCSIZE nR)
{
    ensureFullMatrix();
    mpFullMatrix->PutError(nErrorCode, nC, nR);
}

void ScVectorRefMatrix::PutBoolean(bool bVal, SCSIZE nC, SCSIZE nR)
{
    ensureFullMatrix();
    mpFullMatrix->PutBoolean(bVal, nC, nR);
}

void ScVectorRefMatrix::FillDouble(double fVal, SCSIZE nC1, SCSIZE nR1, SCSIZE nC2, SCSIZE nR2)
{
    ensureFullMatrix();
    mpFullMatrix->FillDouble(fVal, nC1, nR1, nC2, nR2);
}

void ScVectorRefMatrix::PutDoubleVector(const ::std::vector< double > & rVec, SCSIZE nC, SCSIZE nR)
{
    ensureFullMatrix();
    mpFullMatrix->PutDoubleVector(rVec, nC, nR);
}

void ScVectorRefMatrix::PutStringVector(const ::std::vector< svl::SharedString > & rVec, SCSIZE nC, SCSIZE nR)
{
    ensureFullMatrix();
    mpFullMatrix->PutStringVector(rVec, nC, nR);
}

void ScVectorRefMatrix::PutEmptyVector(SCSIZE nCount, SCSIZE nC, SCSIZE nR)
{
    ensureFullMatrix();
    mpFullMatrix->PutEmptyVector(nCount, nC, nR);
}

void ScVectorRefMatrix::PutEmptyResultVector(SCSIZE nCount, SCSIZE nC, SCSIZE nR)
{
    ensureFullMatrix();
    mpFullMatrix->PutEmptyResultVector(nCount, nC, nR);
}

void ScVectorRefMatrix::PutEmptyPathVector(SCSIZE nCount, SCSIZE nC, SCSIZE nR)
{
    ensureFullMatrix();
    mpFullMatrix->PutEmptyPathVector(nCount, nC, nR);
}

FormulaError ScVectorRefMatrix::GetError(SCSIZE nC, SCSIZE nR) const
{
    const_cast<ScVectorRefMatrix*>(this)->ensureFullMatrix();
    return mpFullMatrix->GetError(nC, nR);
}

double ScVectorRefMatrix::GetDouble(SCSIZE nC, SCSIZE nR) const
{
    const_cast<ScVectorRefMatrix*>(this)->ensureFullMatrix();
    return mpFullMatrix->GetDouble(nC, nR);
}

double ScVectorRefMatrix::GetDouble(SCSIZE nIndex) const
{
    const_cast<ScVectorRefMatrix*>(this)->ensureFullMatrix();
    return mpFullMatrix->GetDouble(nIndex);
}

double ScVectorRefMatrix::GetDoubleWithStringConversion(SCSIZE nC, SCSIZE nR) const
{
    const_cast<ScVectorRefMatrix*>(this)->ensureFullMatrix();
    return mpFullMatrix->GetDoubleWithStringConversion(nC, nR);
}

svl::SharedString ScVectorRefMatrix::GetString(SCSIZE nC, SCSIZE nR) const
{
    const_cast<ScVectorRefMatrix*>(this)->ensureFullMatrix();
    return mpFullMatrix->GetString(nC, nR);
}

svl::SharedString ScVectorRefMatrix::GetString(SCSIZE nIndex) const
{
    const_cast<ScVectorRefMatrix*>(this)->ensureFullMatrix();
    return mpFullMatrix->GetString(nIndex);
}

svl::SharedString ScVectorRefMatrix::GetString(SvNumberFormatter& rFormatter, SCSIZE nC, SCSIZE nR) const
{
    const_cast<ScVectorRefMatrix*>(this)->ensureFullMatrix();
    return mpFullMatrix->GetString(rFormatter, nC, nR);
}

ScMatrixValue ScVectorRefMatrix::Get(SCSIZE nC, SCSIZE nR) const
{
    const_cast<ScVectorRefMatrix*>(this)->ensureFullMatrix();
    return mpFullMatrix->Get(nC, nR);
}

bool ScVectorRefMatrix::IsString(SCSIZE nIndex) const
{
    const_cast<ScVectorRefMatrix*>(this)->ensureFullMatrix();
    return mpFullMatrix->IsString(nIndex);
}

bool ScVectorRefMatrix::IsString(SCSIZE nC, SCSIZE nR) const
{
    const_cast<ScVectorRefMatrix*>(this)->ensureFullMatrix();
    return mpFullMatrix->IsString(nC, nR);
}

bool ScVectorRefMatrix::IsEmpty(SCSIZE nC, SCSIZE nR) const
{
    const_cast<ScVectorRefMatrix*>(this)->ensureFullMatrix();
    return mpFullMatrix->IsEmpty(nC, nR);
}

bool ScVectorRefMatrix::IsEmptyCell(SCSIZE nC, SCSIZE nR) const
{
    const_cast<ScVectorRefMatrix*>(this)->ensureFullMatrix();
    return mpFullMatrix->IsEmptyCell(nC, nR);
}

bool ScVectorRefMatrix::IsEmptyResult(SCSIZE nC, SCSIZE nR) const
{
    const_cast<ScVectorRefMatrix*>(this)->ensureFullMatrix();
    return mpFullMatrix->IsEmptyResult(nC, nR);
}

bool ScVectorRefMatrix::IsEmptyPath(SCSIZE nC, SCSIZE nR) const
{
    const_cast<ScVectorRefMatrix*>(this)->ensureFullMatrix();
    return mpFullMatrix->IsEmptyPath(nC, nR);
}

bool ScVectorRefMatrix::IsValue(SCSIZE nIndex) const
{
    const_cast<ScVectorRefMatrix*>(this)->ensureFullMatrix();
    return mpFullMatrix->IsValue(nIndex);
}

bool ScVectorRefMatrix::IsValue(SCSIZE nC, SCSIZE nR) const
{
    const_cast<ScVectorRefMatrix*>(this)->ensureFullMatrix();
    return mpFullMatrix->IsValue(nC, nR);
}

bool ScVectorRefMatrix::IsValueOrEmpty(SCSIZE nC, SCSIZE nR) const
{
    const_cast<ScVectorRefMatrix*>(this)->ensureFullMatrix();
    return mpFullMatrix->IsValueOrEmpty(nC, nR);
}

bool ScVectorRefMatrix::IsBoolean(SCSIZE nC, SCSIZE nR) const
{
    const_cast<ScVectorRefMatrix*>(this)->ensureFullMatrix();
    return mpFullMatrix->IsBoolean(nC, nR);
}

bool ScVectorRefMatrix::IsNumeric() const
{
    const_cast<ScVectorRefMatrix*>(this)->ensureFullMatrix();
    return mpFullMatrix->IsNumeric();
}

void ScVectorRefMatrix::MatTrans(ScMatrix& mRes) const
{
    const_cast<ScVectorRefMatrix*>(this)->ensureFullMatrix();
    mpFullMatrix->MatTrans(mRes);
}

void ScVectorRefMatrix::MatCopy(ScMatrix& mRes) const
{
    const_cast<ScVectorRefMatrix*>(this)->ensureFullMatrix();
    mpFullMatrix->MatCopy(mRes);
}

void ScVectorRefMatrix::CompareEqual()
{
    ensureFullMatrix();
    mpFullMatrix->CompareEqual();
}

void ScVectorRefMatrix::CompareNotEqual()
{
    ensureFullMatrix();
    mpFullMatrix->CompareNotEqual();
}

void ScVectorRefMatrix::CompareLess()
{
    ensureFullMatrix();
    mpFullMatrix->CompareLess();
}

void ScVectorRefMatrix::CompareGreater()
{
    ensureFullMatrix();
    mpFullMatrix->CompareGreater();
}

void ScVectorRefMatrix::CompareLessEqual()
{
    ensureFullMatrix();
    mpFullMatrix->CompareLessEqual();
}

void ScVectorRefMatrix::CompareGreaterEqual()
{
    ensureFullMatrix();
    mpFullMatrix->CompareGreaterEqual();
}

double ScVectorRefMatrix::And() const
{
    const_cast<ScVectorRefMatrix*>(this)->ensureFullMatrix();
    return mpFullMatrix->And();
}

double ScVectorRefMatrix::Or() const
{
    const_cast<ScVectorRefMatrix*>(this)->ensureFullMatrix();
    return mpFullMatrix->Or();
}

double ScVectorRefMatrix::Xor() const
{
    const_cast<ScVectorRefMatrix*>(this)->ensureFullMatrix();
    return mpFullMatrix->Xor();
}

ScMatrix::IterateResult ScVectorRefMatrix::Sum(bool bTextAsZero) const
{
    if (mpFullMatrix)
        return mpFullMatrix->Sum(bTextAsZero);

    const std::vector<formula::VectorRefArray>& rArrays = mpToken->GetArrays();
    size_t nDataSize = mnRowSize;

    if (mnRowStart >= mpToken->GetArrayLength())
    {
        return ScMatrix::IterateResult(0.0, 0.0, 0);
    }
    else if (nDataSize > mpToken->GetArrayLength() - mnRowStart)
    {
        nDataSize = mpToken->GetArrayLength() - mnRowStart;
    }

    double mfFirst = 0.0;
    double mfRest = 0.0;
    for (const formula::VectorRefArray& rArray : rArrays)
    {
        if (rArray.mpStringArray)
        {
            // FIXME operate directly on the array too
            const_cast<ScVectorRefMatrix*>(this)->ensureFullMatrix();
            return mpFullMatrix->Sum(bTextAsZero);
        }
        else if (rArray.mpNumericArray)
        {
            // Numeric cells only.
            const double* p = rArray.mpNumericArray + mnRowStart;
            size_t i = 0;

            // Store the first non-zero value in mfFirst (for some reason).
            if (!mfFirst)
            {
                for (i = 0; i < nDataSize; ++i)
                {
                    if (!mfFirst)
                        mfFirst = p[i];
                    else
                        break;
                }
            }
            p += i;
            if (i == nDataSize)
                continue;

            sc::ArraySumFunctor functor(p, nDataSize-i);

            mfRest += functor();
        }
    }

    return ScMatrix::IterateResult(mfFirst, mfRest, mpToken->GetArrays().size()*nDataSize);
}

ScMatrix::IterateResult ScVectorRefMatrix::SumSquare(bool bTextAsZero) const
{
    const_cast<ScVectorRefMatrix*>(this)->ensureFullMatrix();
    return mpFullMatrix->SumSquare(bTextAsZero);
}

ScMatrix::IterateResult ScVectorRefMatrix::Product(bool bTextAsZero) const
{
    const_cast<ScVectorRefMatrix*>(this)->ensureFullMatrix();
    return mpFullMatrix->Product(bTextAsZero);
}

size_t ScVectorRefMatrix::Count(bool bCountStrings, bool bCountErrors) const
{
    const_cast<ScVectorRefMatrix*>(this)->ensureFullMatrix();
    return mpFullMatrix->Count(bCountStrings, bCountErrors);
}

size_t ScVectorRefMatrix::MatchDoubleInColumns(double fValue, size_t nCol1, size_t nCol2) const
{
    const_cast<ScVectorRefMatrix*>(this)->ensureFullMatrix();
    return mpFullMatrix->MatchDoubleInColumns(fValue, nCol1, nCol2);
}

size_t ScVectorRefMatrix::MatchStringInColumns(const svl::SharedString& rStr, size_t nCol1, size_t nCol2) const
{
    const_cast<ScVectorRefMatrix*>(this)->ensureFullMatrix();
    return mpFullMatrix->MatchStringInColumns(rStr, nCol1, nCol2);
}

double ScVectorRefMatrix::GetMaxValue(bool bTextAsZero) const
{
    const_cast<ScVectorRefMatrix*>(this)->ensureFullMatrix();
    return mpFullMatrix->GetMaxValue(bTextAsZero);
}

double ScVectorRefMatrix::GetMinValue(bool bTextAsZero) const
{
    const_cast<ScVectorRefMatrix*>(this)->ensureFullMatrix();
    return mpFullMatrix->GetMinValue(bTextAsZero);
}

double ScVectorRefMatrix::GetGcd() const
{
    const_cast<ScVectorRefMatrix*>(this)->ensureFullMatrix();
    return mpFullMatrix->GetGcd();
}

ScMatrixRef ScVectorRefMatrix::CompareMatrix(sc::Compare& rComp, size_t nMatPos, sc::CompareOptions* pOptions) const
{
    const_cast<ScVectorRefMatrix*>(this)->ensureFullMatrix();
    return mpFullMatrix->CompareMatrix(rComp, nMatPos, pOptions);
}

void ScVectorRefMatrix::GetDoubleArray(std::vector<double>& rVector, bool bEmptyAsZero) const
{
    const_cast<ScVectorRefMatrix*>(this)->ensureFullMatrix();
    mpFullMatrix->GetDoubleArray(rVector, bEmptyAsZero);
}

void ScVectorRefMatrix::MergeDoubleArray(std::vector<double>& rVector, Op eOp) const
{
    const_cast<ScVectorRefMatrix*>(this)->ensureFullMatrix();
    mpFullMatrix->MergeDoubleArray(rVector, eOp);
}

void ScVectorRefMatrix::NotOp(ScMatrix& rMat)
{
    ensureFullMatrix();
    mpFullMatrix->NotOp(rMat);
}

void ScVectorRefMatrix::NegOp(ScMatrix& rMat)
{
    ensureFullMatrix();
    mpFullMatrix->NegOp(rMat);
}

void ScVectorRefMatrix::AddOp(double fVal, ScMatrix& rMat)
{
    ensureFullMatrix();
    mpFullMatrix->AddOp(fVal, rMat);
}

void ScVectorRefMatrix::SubOp(bool bFlag, double fVal, ScMatrix& rMat)
{
    ensureFullMatrix();
    mpFullMatrix->SubOp(bFlag, fVal, rMat);
}

void ScVectorRefMatrix::MulOp(double fVal, ScMatrix& rMat)
{
    ensureFullMatrix();
    mpFullMatrix->MulOp(fVal, rMat);
}

void ScVectorRefMatrix::DivOp(bool bFlag, double fVal, ScMatrix& rMat)
{
    ensureFullMatrix();
    mpFullMatrix->DivOp(bFlag, fVal, rMat);
}

void ScVectorRefMatrix::PowOp(bool bFlag, double fVal, ScMatrix& rMat)
{
    ensureFullMatrix();
    mpFullMatrix->PowOp(bFlag, fVal, rMat);
}

std::vector<ScMatrix::IterateResult> ScVectorRefMatrix::Collect(bool bTextAsZero, const std::vector<std::unique_ptr<sc::op::Op>>& aOp)
{
    ensureFullMatrix();
    return mpFullMatrix->Collect(bTextAsZero, aOp);
}

void ScVectorRefMatrix::ExecuteOperation(const std::pair<size_t, size_t>& rStartPos,
        const std::pair<size_t, size_t>& rEndPos, DoubleOpFunction aDoubleFunc,
        BoolOpFunction aBoolFunc, StringOpFunction aStringFunc, EmptyOpFunction aEmptyFunc) const
{
    const_cast<ScVectorRefMatrix*>(this)->ensureFullMatrix();
    mpFullMatrix->ExecuteOperation(rStartPos, rEndPos, aDoubleFunc, aBoolFunc, aStringFunc, aEmptyFunc);
}

void ScFullMatrix::MatConcat(SCSIZE nMaxCol, SCSIZE nMaxRow,
        const ScMatrixRef& xMat1, const ScMatrixRef& xMat2, SvNumberFormatter& rFormatter, svl::SharedStringPool& rPool)
{
    pImpl->MatConcat(nMaxCol, nMaxRow, xMat1, xMat2, rFormatter, rPool);
}

void ScVectorRefMatrix::MatConcat(SCSIZE nMaxCol, SCSIZE nMaxRow,
        const ScMatrixRef& xMat1, const ScMatrixRef& xMat2, SvNumberFormatter& rFormatter, svl::SharedStringPool& rPool)
{
    ensureFullMatrix();
    mpFullMatrix->MatConcat(nMaxCol, nMaxRow, xMat1, xMat2, rFormatter, rPool);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
