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
#include <scmatrix.hxx>
#include <global.hxx>
#include <address.hxx>
#include <formula/errorcodes.hxx>
#include <formula/vectortoken.hxx>
#include <interpre.hxx>
#include <mtvelements.hxx>
#include <compare.hxx>
#include <matrixoperators.hxx>
#include <math.hxx>

#include <svl/zforlist.hxx>
#include <svl/sharedstring.hxx>
#include <tools/stream.hxx>
#include <rtl/math.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>

#include <math.h>

#include <memory>
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
        SvNumFormatType nCurFmtType = SvNumFormatType::ALL;
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

struct ElemEqualZero
{
    double operator() (double val) const
    {
        if (!::rtl::math::isFinite(val))
            return val;
        return val == 0.0 ? 1.0 : 0.0;
    }
};

struct ElemNotEqualZero
{
    double operator() (double val) const
    {
        if (!::rtl::math::isFinite(val))
            return val;
        return val != 0.0 ? 1.0 : 0.0;
    }
};

struct ElemGreaterZero
{
    double operator() (double val) const
    {
        if (!::rtl::math::isFinite(val))
            return val;
        return val > 0.0 ? 1.0 : 0.0;
    }
};

struct ElemLessZero
{
    double operator() (double val) const
    {
        if (!::rtl::math::isFinite(val))
            return val;
        return val < 0.0 ? 1.0 : 0.0;
    }
};

struct ElemGreaterEqualZero
{
    double operator() (double val) const
    {
        if (!::rtl::math::isFinite(val))
            return val;
        return val >= 0.0 ? 1.0 : 0.0;
    }
};

struct ElemLessEqualZero
{
    double operator() (double val) const
    {
        if (!::rtl::math::isFinite(val))
            return val;
        return val <= 0.0 ? 1.0 : 0.0;
    }
};

template<typename Comp>
class CompareMatrixElemFunc
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

    CompareMatrixElemFunc( const CompareMatrixElemFunc& ) = delete;
    CompareMatrixElemFunc& operator= ( const CompareMatrixElemFunc& ) = delete;

    CompareMatrixElemFunc( CompareMatrixElemFunc&& ) = default;
    CompareMatrixElemFunc& operator= ( CompareMatrixElemFunc&& ) = default;

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

    ~ScMatrixImpl() COVERITY_NOEXCEPT_FALSE;

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
    bool IsStringOrEmpty( SCSIZE nIndex ) const;
    bool IsStringOrEmpty( SCSIZE nC, SCSIZE nR ) const;
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
    double GetLcm() const;

    ScMatrixRef CompareMatrix( sc::Compare& rComp, size_t nMatPos, sc::CompareOptions* pOptions ) const;

    void GetDoubleArray( std::vector<double>& rArray, bool bEmptyAsZero ) const;
    void MergeDoubleArrayMultiply( std::vector<double>& rArray ) const;

    template<typename T>
    void ApplyOperation(T aOp, ScMatrixImpl& rMat);

    void ExecuteOperation(const std::pair<size_t, size_t>& rStartPos,
            const std::pair<size_t, size_t>& rEndPos, const ScMatrix::DoubleOpFunction& aDoubleFunc,
            const ScMatrix::BoolOpFunction& aBoolFunc, const ScMatrix::StringOpFunction& aStringFunc,
            const ScMatrix::EmptyOpFunction& aEmptyFunc) const;

    template<typename T>
    std::vector<ScMatrix::IterateResult> ApplyCollectOperation(const std::vector<T>& aOp);

    void MatConcat(SCSIZE nMaxCol, SCSIZE nMaxRow, const ScMatrixRef& xMat1, const ScMatrixRef& xMat2,
            SvNumberFormatter& rFormatter, svl::SharedStringPool& rPool);

#if DEBUG_MATRIX
    void Dump() const;
#endif

private:
    void CalcPosition(SCSIZE nIndex, SCSIZE& rC, SCSIZE& rR) const;
};

static bool bElementsMaxFetched;
static size_t nElementsMax;

/** The maximum number of elements a matrix or the pool may have at runtime.

    @param  nMemory
            If 0, the arbitrary limit of one matrix is returned.
            If >0, the given memory pool divided by the average size of a
            matrix element is returned, which is used to initialize
            nElementsMax.
 */
static size_t GetElementsMax( size_t nMemory )
{
    // Arbitrarily assuming 12 bytes per element, 8 bytes double plus
    // overhead. Stored as an array in an mdds container it's less, but for
    // strings or mixed matrix it can be much more..
    constexpr size_t nPerElem = 12;
    if (nMemory)
        return nMemory / nPerElem;

    // Arbitrarily assuming 1GB memory. Could be dynamic at some point.
    constexpr size_t nMemMax = 0x40000000;
    // With 1GB that's ~85M elements, or 85 whole columns.
    constexpr size_t nElemMax = nMemMax / nPerElem;
    // With MAXROWCOUNT==1048576 and 128 columns => 128M elements, 1.5GB
    constexpr size_t nArbitraryLimit = size_t(MAXROWCOUNT) * 128;
    // With the constant 1GB from above that's the actual value.
    return std::min(nElemMax, nArbitraryLimit);
}

ScMatrixImpl::ScMatrixImpl(SCSIZE nC, SCSIZE nR) :
    maMat(nR, nC), maMatFlag(nR, nC), pErrorInterpreter(nullptr)
{
    nElementsMax -= GetElementCount();
}

ScMatrixImpl::ScMatrixImpl(SCSIZE nC, SCSIZE nR, double fInitVal) :
    maMat(nR, nC, fInitVal), maMatFlag(nR, nC), pErrorInterpreter(nullptr)
{
    nElementsMax -= GetElementCount();
}

ScMatrixImpl::ScMatrixImpl( size_t nC, size_t nR, const std::vector<double>& rInitVals ) :
    maMat(nR, nC, rInitVals.begin(), rInitVals.end()), maMatFlag(nR, nC), pErrorInterpreter(nullptr)
{
    nElementsMax -= GetElementCount();
}

ScMatrixImpl::~ScMatrixImpl() COVERITY_NOEXCEPT_FALSE
{
    nElementsMax += GetElementCount();
    Clear();
}

void ScMatrixImpl::Clear()
{
    maMat.clear();
    maMatFlag.clear();
}

void ScMatrixImpl::Resize(SCSIZE nC, SCSIZE nR)
{
    nElementsMax += GetElementCount();
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
    nElementsMax -= GetElementCount();
}

void ScMatrixImpl::Resize(SCSIZE nC, SCSIZE nR, double fVal)
{
    nElementsMax += GetElementCount();
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
    nElementsMax -= GetElementCount();
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
                [[fallthrough]];
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
            sal_uInt32 nKey = rFormatter.GetStandardFormat( SvNumFormatType::LOGICAL,
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

    sal_uInt32 nKey = rFormatter.GetStandardFormat( SvNumFormatType::NUMBER,
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

bool ScMatrixImpl::IsStringOrEmpty( SCSIZE nIndex ) const
{
    SCSIZE nC, nR;
    CalcPosition(nIndex, nC, nR);
    return IsStringOrEmpty(nC, nR);
}

bool ScMatrixImpl::IsStringOrEmpty( SCSIZE nC, SCSIZE nR ) const
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
    aFunc = maMat.walk(std::move(aFunc));
    aFunc.swap(maMat);
}

void ScMatrixImpl::CompareNotEqual()
{
    MatrixImplType::size_pair_type aSize = maMat.size();
    CompareMatrixElemFunc<ElemNotEqualZero> aFunc(aSize.row, aSize.column);
    aFunc = maMat.walk(std::move(aFunc));
    aFunc.swap(maMat);
}

void ScMatrixImpl::CompareLess()
{
    MatrixImplType::size_pair_type aSize = maMat.size();
    CompareMatrixElemFunc<ElemLessZero> aFunc(aSize.row, aSize.column);
    aFunc = maMat.walk(std::move(aFunc));
    aFunc.swap(maMat);
}

void ScMatrixImpl::CompareGreater()
{
    MatrixImplType::size_pair_type aSize = maMat.size();
    CompareMatrixElemFunc<ElemGreaterZero> aFunc(aSize.row, aSize.column);
    aFunc = maMat.walk(std::move(aFunc));
    aFunc.swap(maMat);
}

void ScMatrixImpl::CompareLessEqual()
{
    MatrixImplType::size_pair_type aSize = maMat.size();
    CompareMatrixElemFunc<ElemLessEqualZero> aFunc(aSize.row, aSize.column);
    aFunc = maMat.walk(std::move(aFunc));
    aFunc.swap(maMat);
}

void ScMatrixImpl::CompareGreaterEqual()
{
    MatrixImplType::size_pair_type aSize = maMat.size();
    CompareMatrixElemFunc<ElemGreaterEqualZero> aFunc(aSize.row, aSize.column);
    aFunc = maMat.walk(std::move(aFunc));
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
    const std::vector<Op>* mpOp;
    std::vector<ScMatrix::IterateResult> maRes;
    bool mbFirst:1;
public:
    WalkElementBlocksMultipleValues(const std::vector<Op>& aOp) :
        mpOp(&aOp), mbFirst(true)
    {
        for (const auto& rpOp : *mpOp)
        {
            maRes.emplace_back(rpOp.mInitVal, rpOp.mInitVal, 0);
        }
        maRes.emplace_back(0.0, 0.0, 0); // count
    }

    WalkElementBlocksMultipleValues( const WalkElementBlocksMultipleValues& ) = delete;
    WalkElementBlocksMultipleValues& operator= ( const WalkElementBlocksMultipleValues& ) = delete;

    WalkElementBlocksMultipleValues( WalkElementBlocksMultipleValues&& r ) :
        mpOp(r.mpOp), maRes(std::move(r.maRes)), mbFirst(r.mbFirst) {}

    WalkElementBlocksMultipleValues& operator= ( WalkElementBlocksMultipleValues&& r )
    {
        mpOp = r.mpOp;
        maRes = std::move(r.maRes);
        mbFirst = r.mbFirst;
        return *this;
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
                        for (size_t i = 0u; i < mpOp->size(); ++i)
                        {
                            (*mpOp)[i](maRes[i].mfFirst, *it);
                        }
                        mbFirst = false;
                    }
                    else
                    {
                        for (size_t i = 0u; i < mpOp->size(); ++i)
                        {
                            (*mpOp)[i](maRes[i].mfRest, *it);
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
                        for (size_t i = 0u; i < mpOp->size(); ++i)
                        {
                            (*mpOp)[i](maRes[i].mfFirst, *it);
                        }
                        mbFirst = false;
                    }
                    else
                    {
                        for (size_t i = 0u; i < mpOp->size(); ++i)
                        {
                            (*mpOp)[i](maRes[i].mfRest, *it);
                        }
                    }
                }
                maRes.back().mnCount += node.size;
            }
            break;
            case mdds::mtm::element_string:
            case mdds::mtm::element_empty:
            default:
                ;
        }
    }
};

class CountElements
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
class WalkAndMatchElements
{
    Type maMatchValue;
    size_t mnStartIndex;
    size_t mnStopIndex;
    size_t mnResult;
    size_t mnIndex;

public:
    WalkAndMatchElements(Type aMatchValue, const MatrixImplType::size_pair_type& aSize, size_t nCol1, size_t nCol2) :
        maMatchValue(aMatchValue),
        mnStartIndex( nCol1 * aSize.row ),
        mnStopIndex( (nCol2 + 1) * aSize.row ),
        mnResult(ResultNotSet),
        mnIndex(0)
    {
        assert( nCol1 < aSize.column && nCol2 < aSize.column);
    }

    size_t getMatching() const { return mnResult; }

    size_t getRemainingCount() const
    {
        return mnIndex < mnStopIndex ? mnStopIndex - mnIndex : 0;
    }

    size_t compare(const MatrixImplType::element_block_node_type& node) const;

    void operator() (const MatrixImplType::element_block_node_type& node)
    {
        // early exit if match already found
        if (mnResult != ResultNotSet)
            return;

        // limit lookup to the requested columns
        if (mnStartIndex <= mnIndex && getRemainingCount() > 0)
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
            const size_t nRemaining = getRemainingCount();
            for (; it != itEnd && nCount < nRemaining; ++it, ++nCount)
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
            const size_t nRemaining = getRemainingCount();
            for (; it != itEnd && nCount < nRemaining; ++it, ++nCount)
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
            const size_t nRemaining = getRemainingCount();
            for (; it != itEnd && nCount < nRemaining; ++it, ++nCount)
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
        if (!rtl::math::isFinite(left))
            return left;
        if (!rtl::math::isFinite(right))
            return right;
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
        if (!rtl::math::isFinite(left))
            return left;
        if (!rtl::math::isFinite(right))
            return right;
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

struct Lcm
{
    static double init() { return 1.0; }
    static double calculate(double fx,double fy)
    {
        return (fx*fy)/ScInterpreter::ScGetGCD(fx,fy);
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
    static double init() { return 0.0; }
    static double calculate(double fx,double fy)
    {
        return ScInterpreter::ScGetGCD(fx,fy);
    }

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
class CalcMaxMinValue
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
class CalcGcdLcm
{
    double mfval;

public:
    CalcGcdLcm() : mfval(Op::init()) {}

    double getResult() const { return mfval; }

    void operator() ( const MatrixImplType::element_block_node_type& node )
    {
        switch (node.type)
        {
            case mdds::mtm::element_numeric:
                {
                    typedef MatrixImplType::numeric_block_type block_type;
                    block_type::const_iterator it = block_type::begin(*node.data);
                    block_type::const_iterator itEnd = block_type::end(*node.data);

                    for ( ; it != itEnd; ++it)
                    {
                        if (*it < 0.0)
                            mfval = CreateDoubleError(FormulaError::IllegalArgument);
                        else
                            mfval = ::rtl::math::approxFloor( Op::calculate(*it,mfval));
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

double evaluate( double fVal, ScQueryOp eOp )
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

    SAL_WARN("sc.core",  "evaluate: unhandled comparison operator: " << static_cast<int>(eOp));
    return CreateDoubleError( FormulaError::UnknownState);
}

class CompareMatrixFunc
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

    CompareMatrixFunc( const CompareMatrixFunc& ) = delete;
    CompareMatrixFunc& operator= ( const CompareMatrixFunc& ) = delete;

    CompareMatrixFunc( CompareMatrixFunc&& r ) :
        mrComp(r.mrComp),
        mnMatPos(r.mnMatPos),
        mpOptions(r.mpOptions),
        maResValues(std::move(r.maResValues)) {}

    CompareMatrixFunc& operator= ( CompareMatrixFunc&& r )
    {
        mrComp = r.mrComp;
        mnMatPos = r.mnMatPos;
        mpOptions = r.mpOptions;
        maResValues = std::move(r.maResValues);
        return *this;
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
class CompareMatrixToNumericFunc
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

    CompareMatrixToNumericFunc( const CompareMatrixToNumericFunc& ) = delete;
    CompareMatrixToNumericFunc& operator= ( const CompareMatrixToNumericFunc& ) = delete;

    CompareMatrixToNumericFunc( CompareMatrixToNumericFunc&& r ) :
        mrComp(r.mrComp),
        mfRightValue(r.mfRightValue),
        mpOptions(r.mpOptions),
        maResValues(std::move(r.maResValues)) {}

    CompareMatrixToNumericFunc& operator= ( CompareMatrixToNumericFunc&& r )
    {
        mrComp = r.mrComp;
        mfRightValue = r.mfRightValue;
        mpOptions = r.mpOptions;
        maResValues = std::move(r.maResValues);
        return *this;
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
                    sc::Compare::Cell& rCell = mrComp.maCells[0];
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

class ToDoubleArray
{
    std::vector<double> maArray;
    std::vector<double>::iterator miPos;
    double mfNaN;
    bool mbEmptyAsZero;

    void moveArray( ToDoubleArray& r )
    {
        // Re-create the iterator from the new array after the array has been
        // moved, to ensure that the iterator points to a valid array
        // position.
        size_t n = std::distance(r.maArray.begin(), r.miPos);
        maArray = std::move(r.maArray);
        miPos = maArray.begin();
        std::advance(miPos, n);
    }

public:
    ToDoubleArray( size_t nSize, bool bEmptyAsZero ) :
        maArray(nSize, 0.0), miPos(maArray.begin()), mbEmptyAsZero(bEmptyAsZero)
    {
        mfNaN = CreateDoubleError( FormulaError::ElementNaN);
    }

    ToDoubleArray( const ToDoubleArray& ) = delete;
    ToDoubleArray& operator= ( const ToDoubleArray& ) = delete;

    ToDoubleArray( ToDoubleArray&& r ) :
        mfNaN(r.mfNaN), mbEmptyAsZero(r.mbEmptyAsZero)
    {
        moveArray(r);
    }

    ToDoubleArray& operator= ( ToDoubleArray&& r )
    {
        mfNaN = r.mfNaN;
        mbEmptyAsZero = r.mbEmptyAsZero;
        moveArray(r);
        return *this;
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

struct ArrayMul
{
    double operator() (const double& lhs, const double& rhs) const
    {
        return lhs * rhs;
    }
};

template<typename Op>
class MergeDoubleArrayFunc
{
    std::vector<double>::iterator miPos;
    double mfNaN;
public:
    MergeDoubleArrayFunc(std::vector<double>& rArray) : miPos(rArray.begin())
    {
        mfNaN = CreateDoubleError( FormulaError::ElementNaN);
    }

    MergeDoubleArrayFunc( const MergeDoubleArrayFunc& ) = delete;
    MergeDoubleArrayFunc& operator= ( const MergeDoubleArrayFunc& ) = delete;

    MergeDoubleArrayFunc( MergeDoubleArrayFunc&& ) = default;
    MergeDoubleArrayFunc& operator= ( MergeDoubleArrayFunc&& ) = default;

    void operator() (const MatrixImplType::element_block_node_type& node)
    {
        using namespace mdds::mtv;
        static const Op op;

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
    aFunc = maMat.walk(aFunc);
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
    aFunc = maMat.walk(aFunc);
    return aFunc.getCount();
}

size_t ScMatrixImpl::MatchDoubleInColumns(double fValue, size_t nCol1, size_t nCol2) const
{
    WalkAndMatchElements<double> aFunc(fValue, maMat.size(), nCol1, nCol2);
    aFunc = maMat.walk(aFunc);
    return aFunc.getMatching();
}

size_t ScMatrixImpl::MatchStringInColumns(const svl::SharedString& rStr, size_t nCol1, size_t nCol2) const
{
    WalkAndMatchElements<svl::SharedString> aFunc(rStr, maMat.size(), nCol1, nCol2);
    aFunc = maMat.walk(aFunc);
    return aFunc.getMatching();
}

double ScMatrixImpl::GetMaxValue( bool bTextAsZero ) const
{
    CalcMaxMinValue<MaxOp> aFunc(bTextAsZero);
    aFunc = maMat.walk(aFunc);
    return aFunc.getValue();
}

double ScMatrixImpl::GetMinValue( bool bTextAsZero ) const
{
    CalcMaxMinValue<MinOp> aFunc(bTextAsZero);
    aFunc = maMat.walk(aFunc);
    return aFunc.getValue();
}

double ScMatrixImpl::GetGcd() const
{
    CalcGcdLcm<Gcd> aFunc;
    aFunc = maMat.walk(aFunc);
    return aFunc.getResult();
}

double ScMatrixImpl::GetLcm() const
{
    CalcGcdLcm<Lcm> aFunc;
    aFunc = maMat.walk(aFunc);
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
            aFunc = maMat.walk(std::move(aFunc));

            // We assume the result matrix has the same dimension as this matrix.
            const std::vector<double>& rResVal = aFunc.getValues();
            if (nSize != rResVal.size())
                ScMatrixRef();

            return ScMatrixRef(new ScMatrix(aSize.column, aSize.row, rResVal));
        }
    }

    CompareMatrixFunc aFunc(nSize, rComp, nMatPos, pOptions);
    aFunc = maMat.walk(std::move(aFunc));

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
    aFunc = maMat.walk(std::move(aFunc));
    aFunc.swap(rArray);
}

void ScMatrixImpl::MergeDoubleArrayMultiply( std::vector<double>& rArray ) const
{
    MatrixImplType::size_pair_type aSize = maMat.size();
    size_t nSize = aSize.row*aSize.column;
    if (nSize != rArray.size())
        return;

    MergeDoubleArrayFunc<ArrayMul> aFunc(rArray);
    maMat.walk(std::move(aFunc));
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
    typename T::const_iterator const m_itBegin;
    typename T::const_iterator const m_itEnd;
    U const maOp;
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
    const T* mpOp;

public:
    MatrixOpWrapper(MatrixImplType& rMat, const T& aOp):
        mrMat(rMat),
        pos(rMat.position(0,0)),
        mpOp(&aOp)
    {
    }

    MatrixOpWrapper( const MatrixOpWrapper& r ) : mrMat(r.mrMat), pos(r.pos), mpOp(r.mpOp) {}

    MatrixOpWrapper& operator= ( const MatrixOpWrapper& r )
    {
        mrMat = r.mrMat;
        pos = r.pos;
        mpOp = r.mpOp;
        return *this;
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
                MatrixIteratorWrapper<block_type, T, typename T::number_value_type> aFunc(it, itEnd, *mpOp);
                pos = mrMat.set(pos,aFunc.begin(), aFunc.end());
            }
            break;
            case mdds::mtm::element_boolean:
            {
                typedef MatrixImplType::boolean_block_type block_type;

                block_type::const_iterator it = block_type::begin(*node.data);
                block_type::const_iterator itEnd = block_type::end(*node.data);

                MatrixIteratorWrapper<block_type, T, typename T::number_value_type> aFunc(it, itEnd, *mpOp);
                pos = mrMat.set(pos, aFunc.begin(), aFunc.end());
            }
            break;
            case mdds::mtm::element_string:
            {
                typedef MatrixImplType::string_block_type block_type;

                block_type::const_iterator it = block_type::begin(*node.data);
                block_type::const_iterator itEnd = block_type::end(*node.data);

                MatrixIteratorWrapper<block_type, T, typename T::number_value_type> aFunc(it, itEnd, *mpOp);
                pos = mrMat.set(pos, aFunc.begin(), aFunc.end());
            }
            break;
            case mdds::mtm::element_empty:
            {
                if (mpOp->useFunctionForEmpty())
                {
                    std::vector<char> aVec(node.size);
                    MatrixIteratorWrapper<std::vector<char>, T, typename T::number_value_type>
                        aFunc(aVec.begin(), aVec.end(), *mpOp);
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
std::vector<ScMatrix::IterateResult> ScMatrixImpl::ApplyCollectOperation(const std::vector<T>& aOp)
{
    WalkElementBlocksMultipleValues<T> aFunc(aOp);
    aFunc = maMat.walk(std::move(aFunc));
    return aFunc.getResult();
}

namespace {

struct ElementBlock
{
    ElementBlock(size_t nRowSize,
            ScMatrix::DoubleOpFunction const & aDoubleFunc,
            ScMatrix::BoolOpFunction const & aBoolFunc,
            ScMatrix::StringOpFunction const & aStringFunc,
            ScMatrix::EmptyOpFunction const & aEmptyFunc):
        mnRowSize(nRowSize),
        mnRowPos(0),
        mnColPos(0),
        maDoubleFunc(aDoubleFunc),
        maBoolFunc(aBoolFunc),
        maStringFunc(aStringFunc),
        maEmptyFunc(aEmptyFunc)
    {
    }

    size_t mnRowSize;
    size_t mnRowPos;
    size_t mnColPos;

    ScMatrix::DoubleOpFunction maDoubleFunc;
    ScMatrix::BoolOpFunction maBoolFunc;
    ScMatrix::StringOpFunction maStringFunc;
    ScMatrix::EmptyOpFunction maEmptyFunc;
};

class WalkElementBlockOperation
{
public:

    WalkElementBlockOperation(ElementBlock& rElementBlock)
        : mrElementBlock(rElementBlock)
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
                    mrElementBlock.maDoubleFunc(mrElementBlock.mnRowPos, mrElementBlock.mnColPos, *itr);
                    ++mrElementBlock.mnRowPos;
                    if (mrElementBlock.mnRowPos >= mrElementBlock.mnRowSize)
                    {
                        mrElementBlock.mnRowPos = 0;
                        ++mrElementBlock.mnColPos;
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
                    mrElementBlock.maStringFunc(mrElementBlock.mnRowPos, mrElementBlock.mnColPos, *itr);
                    ++mrElementBlock.mnRowPos;
                    if (mrElementBlock.mnRowPos >= mrElementBlock.mnRowSize)
                    {
                        mrElementBlock.mnRowPos = 0;
                        ++mrElementBlock.mnColPos;
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
                    mrElementBlock.maBoolFunc(mrElementBlock.mnRowPos, mrElementBlock.mnColPos, *itr);
                    ++mrElementBlock.mnRowPos;
                    if (mrElementBlock.mnRowPos >= mrElementBlock.mnRowSize)
                    {
                        mrElementBlock.mnRowPos = 0;
                        ++mrElementBlock.mnColPos;
                    }
                }
            }
            break;
            case mdds::mtm::element_empty:
            {
                for (size_t i=0; i < node.size; ++i)
                {
                    mrElementBlock.maEmptyFunc(mrElementBlock.mnRowPos, mrElementBlock.mnColPos);
                    ++mrElementBlock.mnRowPos;
                    if (mrElementBlock.mnRowPos >= mrElementBlock.mnRowSize)
                    {
                        mrElementBlock.mnRowPos = 0;
                        ++mrElementBlock.mnColPos;
                    }
                }
            }
            break;
            case mdds::mtm::element_integer:
            {
                SAL_WARN("sc.core","WalkElementBlockOperation - unhandled element_integer");
                // No function (yet?), but advance row and column count.
                mrElementBlock.mnColPos += node.size / mrElementBlock.mnRowSize;
                mrElementBlock.mnRowPos += node.size % mrElementBlock.mnRowSize;
                if (mrElementBlock.mnRowPos >= mrElementBlock.mnRowSize)
                {
                    mrElementBlock.mnRowPos = 0;
                    ++mrElementBlock.mnColPos;
                }
            }
            break;
        }
    }

private:

    ElementBlock& mrElementBlock;
};

}

void ScMatrixImpl::ExecuteOperation(const std::pair<size_t, size_t>& rStartPos,
        const std::pair<size_t, size_t>& rEndPos, const ScMatrix::DoubleOpFunction& aDoubleFunc,
        const ScMatrix::BoolOpFunction& aBoolFunc, const ScMatrix::StringOpFunction& aStringFunc,
        const ScMatrix::EmptyOpFunction& aEmptyFunc) const
{
    ElementBlock aPayload(maMat.size().row, aDoubleFunc, aBoolFunc, aStringFunc, aEmptyFunc);
    WalkElementBlockOperation aFunc(aPayload);
    maMat.walk(
        aFunc,
        MatrixImplType::size_pair_type(rStartPos.first, rStartPos.second),
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

size_t get_index(SCSIZE nMaxRow, size_t nRow, size_t nCol, size_t nRowOffset, size_t nColOffset)
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

    sal_uInt32 nKey = rFormatter.GetStandardFormat( SvNumFormatType::NUMBER,
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
                aValid[get_index(nMaxRow, nRow, nCol, nRowOffset, nColOffset)] = false;
                nErrors[get_index(nMaxRow, nRow, nCol, nRowOffset, nColOffset)] = nErr;
                return;
            }
            OUString aStr;
            rFormatter.GetInputLineString( nVal, nKey, aStr);
            aString[get_index(nMaxRow, nRow, nCol, nRowOffset, nColOffset)] = aString[get_index(nMaxRow, nRow, nCol, nRowOffset, nColOffset)] + aStr;
        };

    std::function<void(size_t, size_t, bool)> aBoolFunc =
        [&](size_t nRow, size_t nCol, bool nVal)
        {
            OUString aStr;
            rFormatter.GetInputLineString( nVal ? 1.0 : 0.0, nKey, aStr);
            aString[get_index(nMaxRow, nRow, nCol, nRowOffset, nColOffset)] = aString[get_index(nMaxRow, nRow, nCol, nRowOffset, nColOffset)] + aStr;
        };

    std::function<void(size_t, size_t, const svl::SharedString&)> aStringFunc =
        [&](size_t nRow, size_t nCol, const svl::SharedString& aStr)
        {
            aString[get_index(nMaxRow, nRow, nCol, nRowOffset, nColOffset)] = aString[get_index(nMaxRow, nRow, nCol, nRowOffset, nColOffset)] + aStr.getString();
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
                aValid[get_index(nMaxRow, nRow, nCol, nRowOffset, nColOffset)] = false;
                nErrors[get_index(nMaxRow, nRow, nCol, nRowOffset, nColOffset)] = nErr;
                return;
            }
            OUString aStr;
            rFormatter.GetInputLineString( nVal, nKey, aStr);
            aSharedString[get_index(nMaxRow, nRow, nCol, nRowOffset, nColOffset)] = rStringPool.intern(aString[get_index(nMaxRow, nRow, nCol, nRowOffset, nColOffset)] + aStr);
        };

    std::function<void(size_t, size_t, bool)> aBoolFunc2 =
        [&](size_t nRow, size_t nCol, bool nVal)
        {
            OUString aStr;
            rFormatter.GetInputLineString( nVal ? 1.0 : 0.0, nKey, aStr);
            aSharedString[get_index(nMaxRow, nRow, nCol, nRowOffset, nColOffset)] = rStringPool.intern(aString[get_index(nMaxRow, nRow, nCol, nRowOffset, nColOffset)] + aStr);
        };

    std::function<void(size_t, size_t, const svl::SharedString&)> aStringFunc2 =
        [&](size_t nRow, size_t nCol, const svl::SharedString& aStr)
        {
            aSharedString[get_index(nMaxRow, nRow, nCol, nRowOffset, nColOffset)] =
                rStringPool.intern(aString[get_index(nMaxRow, nRow, nCol, nRowOffset, nColOffset)] + aStr.getString());
        };

    std::function<void(size_t, size_t)> aEmptyFunc2 =
        [&](size_t nRow, size_t nCol)
        {
            aSharedString[get_index(nMaxRow, nRow, nCol, nRowOffset, nColOffset)] =
                rStringPool.intern(aString[get_index(nMaxRow, nRow, nCol, nRowOffset, nColOffset)]);
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
    if (!nC || !nR)
        return true;

    if (!bElementsMaxFetched)
    {
        const char* pEnv = std::getenv("SC_MAX_MATRIX_ELEMENTS");
        if (pEnv)
        {
            // Environment specifies the overall elements pool.
            nElementsMax = std::atoi(pEnv);
        }
        else
        {
            // GetElementsMax() uses an (~arbitrary) elements limit.
            // The actual allocation depends on the types of individual matrix
            // elements and is averaged for type double.
#if SAL_TYPES_SIZEOFPOINTER < 8
            // Assume 1GB memory could be consumed by matrices.
            constexpr size_t nMemMax = 0x40000000;
#else
            // Assume 6GB memory could be consumed by matrices.
            constexpr size_t nMemMax = 0x180000000;
#endif
            nElementsMax = GetElementsMax( nMemMax);
        }
        bElementsMaxFetched = true;
    }

    if (nC > (nElementsMax / nR))
    {
        SAL_WARN( "sc.core", "ScMatrix overflow: " << nC << " columns * " << nR << " rows");
        return false;
    }
    return true;
}

ScMatrix::ScMatrix( SCSIZE nC, SCSIZE nR) :
    nRefCnt(0), mbCloneIfConst(true)
{
    if (ScMatrix::IsSizeAllocatable( nC, nR))
        pImpl.reset( new ScMatrixImpl( nC, nR));
    else
        // Invalid matrix size, allocate 1x1 matrix with error value.
        pImpl.reset( new ScMatrixImpl( 1,1, CreateDoubleError( FormulaError::MatrixSize)));
}

ScMatrix::ScMatrix(SCSIZE nC, SCSIZE nR, double fInitVal) :
    nRefCnt(0), mbCloneIfConst(true)
{
    if (ScMatrix::IsSizeAllocatable( nC, nR))
        pImpl.reset( new ScMatrixImpl( nC, nR, fInitVal));
    else
        // Invalid matrix size, allocate 1x1 matrix with error value.
        pImpl.reset( new ScMatrixImpl( 1,1, CreateDoubleError( FormulaError::MatrixSize)));
}

ScMatrix::ScMatrix( size_t nC, size_t nR, const std::vector<double>& rInitVals ) :
    nRefCnt(0), mbCloneIfConst(true)
{
    if (ScMatrix::IsSizeAllocatable( nC, nR))
        pImpl.reset( new ScMatrixImpl( nC, nR, rInitVals));
    else
        // Invalid matrix size, allocate 1x1 matrix with error value.
        pImpl.reset( new ScMatrixImpl( 1,1, CreateDoubleError( FormulaError::MatrixSize)));
}

ScMatrix::~ScMatrix()
{
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

void ScMatrix::PutError( FormulaError nErrorCode, SCSIZE nC, SCSIZE nR )
{
    pImpl->PutError(nErrorCode, nC, nR);
}

void ScMatrix::PutBoolean(bool bVal, SCSIZE nC, SCSIZE nR)
{
    pImpl->PutBoolean(bVal, nC, nR);
}

FormulaError ScMatrix::GetError( SCSIZE nC, SCSIZE nR) const
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

double ScMatrix::GetDoubleWithStringConversion(SCSIZE nC, SCSIZE nR) const
{
    return pImpl->GetDoubleWithStringConversion(nC, nR);
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

bool ScMatrix::IsStringOrEmpty( SCSIZE nIndex ) const
{
    return pImpl->IsStringOrEmpty(nIndex);
}

bool ScMatrix::IsStringOrEmpty( SCSIZE nC, SCSIZE nR ) const
{
    return pImpl->IsStringOrEmpty(nC, nR);
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

void ScMatrix::MatCopy(const ScMatrix& mRes) const
{
    pImpl->MatCopy(*mRes.pImpl);
}

void ScMatrix::MatTrans(const ScMatrix& mRes) const
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

size_t ScMatrix::Count(bool bCountStrings, bool bCountErrors) const
{
    return pImpl->Count(bCountStrings, bCountErrors);
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

double ScMatrix::GetGcd() const
{
    return pImpl->GetGcd();
}

double ScMatrix::GetLcm() const
{
    return pImpl->GetLcm();
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

void ScMatrix::MergeDoubleArrayMultiply( std::vector<double>& rArray ) const
{
    pImpl->MergeDoubleArrayMultiply(rArray);
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
    typedef TRet number_value_type;

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
        return maOp(static_cast<double>(bVal), mfVal);
    }

    double operator()(const svl::SharedString& rStr) const
    {
        return convertStringToValue( mpErrorInterpreter, rStr.getString());
    }

    TEmptyRes operator()(char) const
    {
        return maCOp({}, maOp, 0, mfVal, maString);
    }

    static bool useFunctionForEmpty()
    {
        return true;
    }
};

}

void ScMatrix::NotOp( const ScMatrix& rMat)
{
    auto not_ = [](double a, double){return double(a == 0.0);};
    matop::MatOp<decltype(not_), double> aOp(not_, pImpl->GetErrorInterpreter());
    pImpl->ApplyOperation(aOp, *rMat.pImpl);
}

void ScMatrix::NegOp( const ScMatrix& rMat)
{
    auto neg_ = [](double a, double){return -a;};
    matop::MatOp<decltype(neg_), double> aOp(neg_, pImpl->GetErrorInterpreter());
    pImpl->ApplyOperation(aOp, *rMat.pImpl);
}

void ScMatrix::AddOp( double fVal, const ScMatrix& rMat)
{
    auto add_ = [](double a, double b){return a + b;};
    matop::MatOp<decltype(add_)> aOp(add_, pImpl->GetErrorInterpreter(), fVal);
    pImpl->ApplyOperation(aOp, *rMat.pImpl);
}

void ScMatrix::SubOp( bool bFlag, double fVal, const ScMatrix& rMat)
{
    if (bFlag)
    {
        auto sub_ = [](double a, double b){return b - a;};
        matop::MatOp<decltype(sub_)> aOp(sub_, pImpl->GetErrorInterpreter(), fVal);
        pImpl->ApplyOperation(aOp, *rMat.pImpl);
    }
    else
    {
        auto sub_ = [](double a, double b){return a - b;};
        matop::MatOp<decltype(sub_)> aOp(sub_, pImpl->GetErrorInterpreter(), fVal);
        pImpl->ApplyOperation(aOp, *rMat.pImpl);
    }
}

void ScMatrix::MulOp( double fVal, const ScMatrix& rMat)
{
    auto mul_ = [](double a, double b){return a * b;};
    matop::MatOp<decltype(mul_)> aOp(mul_, pImpl->GetErrorInterpreter(), fVal);
    pImpl->ApplyOperation(aOp, *rMat.pImpl);
}

void ScMatrix::DivOp( bool bFlag, double fVal, const ScMatrix& rMat)
{
    if (bFlag)
    {
        auto div_ = [](double a, double b){return sc::div(b, a);};
        matop::MatOp<decltype(div_)> aOp(div_, pImpl->GetErrorInterpreter(), fVal);
        pImpl->ApplyOperation(aOp, *rMat.pImpl);
    }
    else
    {
        auto div_ = [](double a, double b){return sc::div(a, b);};
        matop::MatOp<decltype(div_)> aOp(div_, pImpl->GetErrorInterpreter(), fVal);
        pImpl->ApplyOperation(aOp, *rMat.pImpl);
    }
}

void ScMatrix::PowOp( bool bFlag, double fVal, const ScMatrix& rMat)
{
    if (bFlag)
    {
        auto pow_ = [](double a, double b){return sc::power(b, a);};
        matop::MatOp<decltype(pow_)> aOp(pow_, pImpl->GetErrorInterpreter(), fVal);
        pImpl->ApplyOperation(aOp, *rMat.pImpl);
    }
    else
    {
        auto pow_ = [](double a, double b){return sc::power(a, b);};
        matop::MatOp<decltype(pow_)> aOp(pow_, pImpl->GetErrorInterpreter(), fVal);
        pImpl->ApplyOperation(aOp, *rMat.pImpl);
    }
}

void ScMatrix::ExecuteOperation(const std::pair<size_t, size_t>& rStartPos,
        const std::pair<size_t, size_t>& rEndPos, DoubleOpFunction aDoubleFunc,
        BoolOpFunction aBoolFunc, StringOpFunction aStringFunc, EmptyOpFunction aEmptyFunc) const
{
    pImpl->ExecuteOperation(rStartPos, rEndPos, aDoubleFunc, aBoolFunc, aStringFunc, aEmptyFunc);
}

std::vector<ScMatrix::IterateResult> ScMatrix::Collect(const std::vector<sc::op::Op>& aOp)
{
    return pImpl->ApplyCollectOperation(aOp);
}

#if DEBUG_MATRIX
void ScMatrix::Dump() const
{
    pImpl->Dump();
}
#endif

void ScMatrix::MatConcat(SCSIZE nMaxCol, SCSIZE nMaxRow,
        const ScMatrixRef& xMat1, const ScMatrixRef& xMat2, SvNumberFormatter& rFormatter, svl::SharedStringPool& rPool)
{
    pImpl->MatConcat(nMaxCol, nMaxRow, xMat1, xMat2, rFormatter, rPool);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
