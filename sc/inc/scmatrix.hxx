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

#ifndef INCLUDED_SC_INC_SCMATRIX_HXX
#define INCLUDED_SC_INC_SCMATRIX_HXX

#include "global.hxx"
#include "matrixoperators.hxx"
#include "types.hxx"
#include <formula/errorcodes.hxx>
#include "scdllapi.h"
#include <rtl/ustring.hxx>
#include <svl/sharedstring.hxx>
#include <svl/sharedstringpool.hxx>

#include <functional>
#include <memory>
#include <utility>
#include <vector>
#include <boost/intrusive_ptr.hpp>

#define DEBUG_MATRIX 0

class ScInterpreter;
class SvNumberFormatter;
class ScMatrixImpl;
enum class FormulaError : sal_uInt16;

namespace formula { class DoubleVectorRefToken; }

namespace sc {

struct Compare;
struct CompareOptions;

}

/**
 * Try NOT to use this struct.  This struct should go away in a hopefully
 * not so distant future.
 */
struct ScMatrixValue
{
    double fVal;
    svl::SharedString aStr;
    ScMatValType nType;

    /// Only valid if ScMatrix methods indicate so!
    const svl::SharedString& GetString() const { return aStr; }

    /// Only valid if ScMatrix methods indicate that this is no string!
    FormulaError GetError() const { return GetDoubleErrorValue(fVal); }

    /// Only valid if ScMatrix methods indicate that this is a boolean
    bool GetBoolean() const         { return fVal != 0.0; }

    ScMatrixValue() : fVal(0.0), nType(ScMatValType::Empty) {}

    ScMatrixValue(const ScMatrixValue& r) :
        fVal(r.fVal), aStr(r.aStr), nType(r.nType) {}

    bool operator== (const ScMatrixValue& r) const
    {
        if (nType != r.nType)
            return false;

        switch (nType)
        {
            case ScMatValType::Value:
            case ScMatValType::Boolean:
                return fVal == r.fVal;
            break;
            default:
                ;
        }

        return aStr == r.aStr;
    }

    bool operator!= (const ScMatrixValue& r) const
    {
        return !operator==(r);
    }

    ScMatrixValue& operator= (const ScMatrixValue& r)
    {
        if (this == &r)
            return *this;

        nType = r.nType;
        fVal = r.fVal;
        aStr = r.aStr;
        return *this;
    }
};

/// Abstract base class for ScFullMatrix and ScVectorRefMatrix implementations.
class SC_DLLPUBLIC ScMatrix
{
    mutable size_t  nRefCnt;    // reference count
    mutable bool    mbCloneIfConst; // Whether the matrix is cloned with a CloneIfConst() call.

    ScMatrix( const ScMatrix& ) = delete;
    ScMatrix& operator=( const ScMatrix&) = delete;

protected:
    virtual ~ScMatrix() {}

public:
    enum Op { Add, Sub, Mul, Div };

    typedef std::function<void(size_t, size_t, double)> DoubleOpFunction;
    typedef std::function<void(size_t, size_t, bool)> BoolOpFunction;
    typedef std::function<void(size_t, size_t, svl::SharedString)> StringOpFunction;
    typedef std::function<void(size_t, size_t)> EmptyOpFunction;

    /**
     * When adding all numerical matrix elements for a scalar result such as
     * summation, the interpreter wants to separate the first non-zero value
     * with the rest of the summed values.
     *
     * TODO: Find out if we still need to do this.  If not, we can re-write
     * ScInterpreter::IterateParameters() to make it simpler and remove this
     * struct.
     */
    struct IterateResult
    {
        double mfFirst;
        double mfRest;
        size_t mnCount;

        IterateResult(double fFirst, double fRest, size_t nCount) :
            mfFirst(fFirst), mfRest(fRest), mnCount(nCount) {}

        IterateResult(const IterateResult& r) :
            mfFirst(r.mfFirst), mfRest(r.mfRest), mnCount(r.mnCount) {}
    };

    /// The maximum number of elements a matrix may have at runtime.
    inline static size_t GetElementsMax()
    {
        // TODO: Fix me.
        return 0x08000000;
#if 0
        // Roughly 125MB in total, divided by 8+1 per element => 14M elements.
        const size_t nMemMax = 0x08000000 / (sizeof(ScMatrixValue) + sizeof(ScMatValType));
        // With MAXROWCOUNT==65536 and 128 columns => 8M elements ~72MB.
        const size_t nArbitraryLimit = (size_t)MAXROWCOUNT * 128;
        // Stuffed with a million rows would limit this to 14 columns.
        return nMemMax < nArbitraryLimit ? nMemMax : nArbitraryLimit;
#endif
    }

    /** Checks nC or nR for zero and uses GetElementsMax() whether a matrix of
        the size of nC*nR could be allocated. A zero size (both nC and nR zero)
        matrix is allowed for later resize.
     */
    bool static IsSizeAllocatable( SCSIZE nC, SCSIZE nR );

    /// Value or boolean.
    inline static bool IsValueType( ScMatValType nType )
    {
        return nType <= ScMatValType::Boolean;
    }

    /// Boolean.
    inline static bool IsBooleanType( ScMatValType nType )
    {
        return nType == ScMatValType::Boolean;
    }

    /// String, empty or empty path, but not value nor boolean.
    inline static bool IsNonValueType( ScMatValType nType )
    {
        return bool(nType & ScMatValType::NonvalueMask);
    }

    /** String, but not empty or empty path or any other type.
        Not named IsStringType to prevent confusion because previously
        IsNonValueType was named IsStringType. */
    inline static bool IsRealStringType( ScMatValType nType )
    {
        return (nType & ScMatValType::NonvalueMask) == ScMatValType::String;
    }

    /// Empty, but not empty path or any other type.
    inline static bool IsEmptyType( ScMatValType nType )
    {
        return (nType & ScMatValType::NonvalueMask) == ScMatValType::Empty;
    }

    /// Empty path, but not empty or any other type.
    inline static bool IsEmptyPathType( ScMatValType nType )
    {
        return (nType & ScMatValType::NonvalueMask) == ScMatValType::EmptyPath;
    }

    ScMatrix() : nRefCnt(0), mbCloneIfConst(true) {}

    /** Clone the matrix. */
    virtual ScMatrix* Clone() const = 0;

    /** Clone the matrix if mbCloneIfConst (immutable) is set, otherwise
        return _this_ matrix, to be assigned to a ScMatrixRef. */
    ScMatrix* CloneIfConst();

    /** Set the matrix to mutable for CloneIfConst(), only the interpreter
        should do this and know the consequences. */
    void SetMutable();

    /** Set the matrix to immutable for CloneIfConst(), only the interpreter
        should do this and know the consequences. */
    void SetImmutable() const;

    /**
     * Resize the matrix to specified new dimension.
     */
    virtual void Resize(SCSIZE nC, SCSIZE nR) = 0;

    virtual void Resize(SCSIZE nC, SCSIZE nR, double fVal) = 0;

    /** Clone the matrix and extend it to the new size. nNewCols and nNewRows
        MUST be at least of the size of the original matrix. */
    virtual ScMatrix* CloneAndExtend(SCSIZE nNewCols, SCSIZE nNewRows) const = 0;

    void IncRef() const;
    void DecRef() const;

    virtual void SetErrorInterpreter( ScInterpreter* p) = 0;
    virtual void GetDimensions( SCSIZE& rC, SCSIZE& rR) const = 0;
    virtual SCSIZE GetElementCount() const = 0;
    virtual bool ValidColRow( SCSIZE nC, SCSIZE nR) const = 0;

    /** For a row vector or column vector, if the position does not point into
        the vector but is a valid column or row offset it is adapted such that
        it points to an element to be replicated, same column row 0 for a row
        vector, same row column 0 for a column vector. Else, for a 2D matrix,
        returns false.
     */
    virtual bool ValidColRowReplicated( SCSIZE & rC, SCSIZE & rR ) const = 0;

    /** Checks if the matrix position is within the matrix. If it is not, for a
        row vector or column vector the position is adapted such that it points
        to an element to be replicated, same column row 0 for a row vector,
        same row column 0 for a column vector. Else, for a 2D matrix and
        position not within matrix, returns false.
     */
    virtual bool ValidColRowOrReplicated( SCSIZE & rC, SCSIZE & rR ) const = 0;

    virtual void PutDouble( double fVal, SCSIZE nC, SCSIZE nR) = 0;
    virtual void PutDouble( double fVal, SCSIZE nIndex) = 0;
    virtual void PutDouble(const double* pArray, size_t nLen, SCSIZE nC, SCSIZE nR) = 0;

    virtual void PutString( const svl::SharedString& rStr, SCSIZE nC, SCSIZE nR) = 0;
    virtual void PutString( const svl::SharedString& rStr, SCSIZE nIndex) = 0;
    virtual void PutString( const svl::SharedString* pArray, size_t nLen, SCSIZE nC, SCSIZE nR) = 0;

    virtual void PutEmpty( SCSIZE nC, SCSIZE nR) = 0;

    /// Jump sal_False without path
    virtual void PutEmptyPath( SCSIZE nC, SCSIZE nR) = 0;
    virtual void PutError( FormulaError nErrorCode, SCSIZE nC, SCSIZE nR ) = 0;
    virtual void PutBoolean( bool bVal, SCSIZE nC, SCSIZE nR) = 0;

    virtual void FillDouble( double fVal,
            SCSIZE nC1, SCSIZE nR1, SCSIZE nC2, SCSIZE nR2 ) = 0;

    /** Put a column vector of doubles, starting at row nR, must fit into dimensions. */
    virtual void PutDoubleVector( const ::std::vector< double > & rVec, SCSIZE nC, SCSIZE nR ) = 0;

    /** Put a column vector of strings, starting at row nR, must fit into dimensions. */
    virtual void PutStringVector( const ::std::vector< svl::SharedString > & rVec, SCSIZE nC, SCSIZE nR ) = 0;

    /** Put a column vector of empties, starting at row nR, must fit into dimensions. */
    virtual void PutEmptyVector( SCSIZE nCount, SCSIZE nC, SCSIZE nR ) = 0;

    /** Put a column vector of empty results, starting at row nR, must fit into dimensions. */
    virtual void PutEmptyResultVector( SCSIZE nCount, SCSIZE nC, SCSIZE nR ) = 0;

    /** Put a column vector of empty paths, starting at row nR, must fit into dimensions. */
    virtual void PutEmptyPathVector( SCSIZE nCount, SCSIZE nC, SCSIZE nR ) = 0;

    /** May be used before obtaining the double value of an element to avoid
        passing its NAN around.
        @ATTENTION: MUST NOT be used if the element is a string!
                    Use GetErrorIfNotString() instead if not sure.
        @returns 0 if no error, else one of err... constants */
    virtual FormulaError GetError( SCSIZE nC, SCSIZE nR) const = 0;

    /** Use in ScInterpreter to obtain the error code, if any.
        @returns 0 if no error or string element, else one of err... constants */
    FormulaError GetErrorIfNotString( SCSIZE nC, SCSIZE nR) const
        { return IsValue( nC, nR) ? GetError( nC, nR) : FormulaError::NONE; }

    /// @return 0.0 if empty or empty path, else value or DoubleError.
    virtual double GetDouble( SCSIZE nC, SCSIZE nR) const = 0;
    /// @return 0.0 if empty or empty path, else value or DoubleError.
    virtual double GetDouble( SCSIZE nIndex) const = 0;
    /// @return value or DoubleError or string converted to value.
    virtual double GetDoubleWithStringConversion( SCSIZE nC, SCSIZE nR ) const = 0;

    /// @return empty string if empty or empty path, else string content.
    virtual svl::SharedString GetString( SCSIZE nC, SCSIZE nR) const = 0;
    /// @return empty string if empty or empty path, else string content.
    virtual svl::SharedString GetString( SCSIZE nIndex) const = 0;

    /** @returns the matrix element's string if one is present, otherwise the
        numerical value formatted as string, or in case of an error the error
        string is returned; an empty string for empty, a "FALSE" string for
        empty path. */
    virtual svl::SharedString GetString( SvNumberFormatter& rFormatter, SCSIZE nC, SCSIZE nR) const = 0;

    /// @ATTENTION: If bString the ScMatrixValue->pS may still be NULL to indicate
    /// an empty string!
    virtual ScMatrixValue Get( SCSIZE nC, SCSIZE nR) const = 0;

    /// @return <TRUE/> if string or empty or empty path, in fact non-value.
    virtual bool IsString( SCSIZE nIndex ) const = 0;

    /// @return <TRUE/> if string or empty or empty path, in fact non-value.
    virtual bool IsString( SCSIZE nC, SCSIZE nR ) const = 0;

    /// @return <TRUE/> if empty or empty cell or empty result, not empty path.
    virtual bool IsEmpty( SCSIZE nC, SCSIZE nR ) const = 0;

    /// @return <TRUE/> if empty cell, not empty or empty result or empty path.
    virtual bool IsEmptyCell( SCSIZE nC, SCSIZE nR ) const = 0;

    /// @return <TRUE/> if empty result, not empty or empty cell or empty path.
    virtual bool IsEmptyResult( SCSIZE nC, SCSIZE nR ) const = 0;

    /// @return <TRUE/> if empty path, not empty or empty cell or empty result.
    virtual bool IsEmptyPath( SCSIZE nC, SCSIZE nR ) const = 0;

    /// @return <TRUE/> if value or boolean.
    virtual bool IsValue( SCSIZE nIndex ) const = 0;

    /// @return <TRUE/> if value or boolean.
    virtual bool IsValue( SCSIZE nC, SCSIZE nR ) const = 0;

    /// @return <TRUE/> if value or boolean or empty or empty path.
    virtual bool IsValueOrEmpty( SCSIZE nC, SCSIZE nR ) const = 0;

    /// @return <TRUE/> if boolean.
    virtual bool IsBoolean( SCSIZE nC, SCSIZE nR ) const = 0;

    /// @return <TRUE/> if entire matrix is numeric, including booleans, with no strings or empties
    virtual bool IsNumeric() const = 0;

    virtual void MatTrans( ScMatrix& mRes) const = 0;
    virtual void MatCopy ( ScMatrix& mRes) const = 0;

    // Convert ScInterpreter::CompareMat values (-1,0,1) to boolean values
    virtual void CompareEqual() = 0;
    virtual void CompareNotEqual() = 0;
    virtual void CompareLess() = 0;
    virtual void CompareGreater() = 0;
    virtual void CompareLessEqual() = 0;
    virtual void CompareGreaterEqual() = 0;

    virtual double And() const = 0;       // logical AND of all matrix values, or NAN
    virtual double Or() const = 0;        // logical OR of all matrix values, or NAN
    virtual double Xor() const = 0;       // logical XOR of all matrix values, or NAN

    virtual IterateResult Sum(bool bTextAsZero) const = 0;
    virtual IterateResult SumSquare(bool bTextAsZero) const = 0;
    virtual IterateResult Product(bool bTextAsZero) const = 0;
    virtual size_t Count(bool bCountStrings, bool bCountErrors) const = 0;
    virtual size_t MatchDoubleInColumns(double fValue, size_t nCol1, size_t nCol2) const = 0;
    virtual size_t MatchStringInColumns(const svl::SharedString& rStr, size_t nCol1, size_t nCol2) const = 0;

    virtual double GetMaxValue( bool bTextAsZero ) const = 0;
    virtual double GetMinValue( bool bTextAsZero ) const = 0;
    virtual double GetGcd() const = 0;

    virtual ScMatrixRef CompareMatrix(
        sc::Compare& rComp, size_t nMatPos, sc::CompareOptions* pOptions ) const = 0;

    /**
     * Convert the content of matrix into a linear array of numeric values.
     * String elements are mapped to NaN's and empty elements are mapped to
     * either NaN or zero values.
     *
     * @param bEmptyAsZero if true empty elements are mapped to zero values,
     *                     otherwise they become NaN values.
     */
    virtual void GetDoubleArray( std::vector<double>& rArray, bool bEmptyAsZero = true ) const = 0;
    virtual void MergeDoubleArray( std::vector<double>& rArray, Op eOp ) const = 0;

    virtual void NotOp(ScMatrix& rMat) = 0;
    virtual void NegOp(ScMatrix& rMat) = 0;
    virtual void AddOp(double fVal, ScMatrix& rMat) = 0;
    virtual void SubOp(bool bFlag, double fVal, ScMatrix& rMat) = 0;
    virtual void MulOp(double fVal, ScMatrix& rMat) = 0;
    virtual void DivOp(bool bFlag, double fVal, ScMatrix& rMat) = 0;
    virtual void PowOp(bool bFlag, double fVal, ScMatrix& rMat) = 0;

    virtual std::vector<ScMatrix::IterateResult> Collect(bool bTextAsZero, const std::vector<std::unique_ptr<sc::op::Op>>& aOp) = 0;

    virtual void ExecuteOperation(const std::pair<size_t, size_t>& rStartPos, const std::pair<size_t, size_t>& rEndPos,
            DoubleOpFunction aDoubleFunc, BoolOpFunction aBoolFunc, StringOpFunction aStringFunc,
            EmptyOpFunction aEmptyFunc) const = 0;

    virtual void MatConcat(SCSIZE nMaxCol, SCSIZE nMaxRow, const ScMatrixRef& xMat1, const ScMatrixRef& xMat2,
            SvNumberFormatter& rFormatter, svl::SharedStringPool& rPool) = 0;

#if DEBUG_MATRIX
    virtual void Dump() const = 0;
#endif
};

/**
 * Matrix data type that can store values of mixed types.  Each element can
 * be one of the following types: numeric, string, boolean, empty, and empty
 * path.
 */
class SC_DLLPUBLIC ScFullMatrix : public ScMatrix
{
    friend class ScMatrixImpl;

    std::unique_ptr<ScMatrixImpl> pImpl;

    ScFullMatrix( const ScFullMatrix& ) = delete;
    ScFullMatrix& operator=( const ScFullMatrix&) = delete;

public:

    ScFullMatrix(SCSIZE nC, SCSIZE nR);
    ScFullMatrix(SCSIZE nC, SCSIZE nR, double fInitVal);

    ScFullMatrix( size_t nC, size_t nR, const std::vector<double>& rInitVals );

    virtual ~ScFullMatrix() override;

    /** Clone the matrix. */
    virtual ScMatrix* Clone() const override;

    /**
     * Resize the matrix to specified new dimension.
     */
    virtual void Resize( SCSIZE nC, SCSIZE nR) override;

    virtual void Resize(SCSIZE nC, SCSIZE nR, double fVal) override;

    /** Clone the matrix and extend it to the new size. nNewCols and nNewRows
        MUST be at least of the size of the original matrix. */
    virtual ScMatrix* CloneAndExtend(SCSIZE nNewCols, SCSIZE nNewRows) const override;

    virtual void SetErrorInterpreter( ScInterpreter* p) override;
    virtual void GetDimensions( SCSIZE& rC, SCSIZE& rR) const override;
    virtual SCSIZE GetElementCount() const override;
    virtual bool ValidColRow( SCSIZE nC, SCSIZE nR) const override;

    /** For a row vector or column vector, if the position does not point into
        the vector but is a valid column or row offset it is adapted such that
        it points to an element to be replicated, same column row 0 for a row
        vector, same row column 0 for a column vector. Else, for a 2D matrix,
        returns false.
     */
    virtual bool ValidColRowReplicated( SCSIZE & rC, SCSIZE & rR ) const override;

    /** Checks if the matrix position is within the matrix. If it is not, for a
        row vector or column vector the position is adapted such that it points
        to an element to be replicated, same column row 0 for a row vector,
        same row column 0 for a column vector. Else, for a 2D matrix and
        position not within matrix, returns false.
     */
    virtual bool ValidColRowOrReplicated( SCSIZE & rC, SCSIZE & rR ) const override;

    virtual void PutDouble( double fVal, SCSIZE nC, SCSIZE nR) override;
    virtual void PutDouble( double fVal, SCSIZE nIndex) override;
    virtual void PutDouble(const double* pArray, size_t nLen, SCSIZE nC, SCSIZE nR) override;

    virtual void PutString( const svl::SharedString& rStr, SCSIZE nC, SCSIZE nR) override;
    virtual void PutString( const svl::SharedString& rStr, SCSIZE nIndex) override;
    virtual void PutString( const svl::SharedString* pArray, size_t nLen, SCSIZE nC, SCSIZE nR) override;

    virtual void PutEmpty( SCSIZE nC, SCSIZE nR) override;

    /// Jump sal_False without path
    virtual void PutEmptyPath( SCSIZE nC, SCSIZE nR) override;
    virtual void PutError( FormulaError nErrorCode, SCSIZE nC, SCSIZE nR ) override;
    virtual void PutBoolean( bool bVal, SCSIZE nC, SCSIZE nR) override;

    virtual void FillDouble( double fVal,
            SCSIZE nC1, SCSIZE nR1, SCSIZE nC2, SCSIZE nR2 ) override;

    /** Put a column vector of doubles, starting at row nR, must fit into dimensions. */
    virtual void PutDoubleVector( const ::std::vector< double > & rVec, SCSIZE nC, SCSIZE nR ) override;

    /** Put a column vector of strings, starting at row nR, must fit into dimensions. */
    virtual void PutStringVector( const ::std::vector< svl::SharedString > & rVec, SCSIZE nC, SCSIZE nR ) override;

    /** Put a column vector of empties, starting at row nR, must fit into dimensions. */
    virtual void PutEmptyVector( SCSIZE nCount, SCSIZE nC, SCSIZE nR ) override;

    /** Put a column vector of empty results, starting at row nR, must fit into dimensions. */
    virtual void PutEmptyResultVector( SCSIZE nCount, SCSIZE nC, SCSIZE nR ) override;

    /** Put a column vector of empty paths, starting at row nR, must fit into dimensions. */
    virtual void PutEmptyPathVector( SCSIZE nCount, SCSIZE nC, SCSIZE nR ) override;

    /** May be used before obtaining the double value of an element to avoid
        passing its NAN around.
        @ATTENTION: MUST NOT be used if the element is a string!
                    Use GetErrorIfNotString() instead if not sure.
        @returns 0 if no error, else one of err... constants */
    virtual FormulaError GetError( SCSIZE nC, SCSIZE nR) const override;

    /// @return 0.0 if empty or empty path, else value or DoubleError.
    virtual double GetDouble( SCSIZE nC, SCSIZE nR) const override;
    /// @return 0.0 if empty or empty path, else value or DoubleError.
    virtual double GetDouble( SCSIZE nIndex) const override;
    /// @return value or DoubleError or string converted to value.
    virtual double GetDoubleWithStringConversion( SCSIZE nC, SCSIZE nR ) const override;

    /// @return empty string if empty or empty path, else string content.
    virtual svl::SharedString GetString( SCSIZE nC, SCSIZE nR) const override;
    /// @return empty string if empty or empty path, else string content.
    virtual svl::SharedString GetString( SCSIZE nIndex) const override;

    /** @returns the matrix element's string if one is present, otherwise the
        numerical value formatted as string, or in case of an error the error
        string is returned; an empty string for empty, a "FALSE" string for
        empty path. */
    virtual svl::SharedString GetString( SvNumberFormatter& rFormatter, SCSIZE nC, SCSIZE nR) const override;

    /// @ATTENTION: If bString the ScMatrixValue->pS may still be NULL to indicate
    /// an empty string!
    virtual ScMatrixValue Get( SCSIZE nC, SCSIZE nR) const override;

    /// @return <TRUE/> if string or empty or empty path, in fact non-value.
    virtual bool IsString( SCSIZE nIndex ) const override;

    /// @return <TRUE/> if string or empty or empty path, in fact non-value.
    virtual bool IsString( SCSIZE nC, SCSIZE nR ) const override;

    /// @return <TRUE/> if empty or empty cell or empty result, not empty path.
    virtual bool IsEmpty( SCSIZE nC, SCSIZE nR ) const override;

    /// @return <TRUE/> if empty cell, not empty or empty result or empty path.
    virtual bool IsEmptyCell( SCSIZE nC, SCSIZE nR ) const override;

    /// @return <TRUE/> if empty result, not empty or empty cell or empty path.
    virtual bool IsEmptyResult( SCSIZE nC, SCSIZE nR ) const override;

    /// @return <TRUE/> if empty path, not empty or empty cell or empty result.
    virtual bool IsEmptyPath( SCSIZE nC, SCSIZE nR ) const override;

    /// @return <TRUE/> if value or boolean.
    virtual bool IsValue( SCSIZE nIndex ) const override;

    /// @return <TRUE/> if value or boolean.
    virtual bool IsValue( SCSIZE nC, SCSIZE nR ) const override;

    /// @return <TRUE/> if value or boolean or empty or empty path.
    virtual bool IsValueOrEmpty( SCSIZE nC, SCSIZE nR ) const override;

    /// @return <TRUE/> if boolean.
    virtual bool IsBoolean( SCSIZE nC, SCSIZE nR ) const override;

    /// @return <TRUE/> if entire matrix is numeric, including booleans, with no strings or empties
    virtual bool IsNumeric() const override;

    virtual void MatTrans( ScMatrix& mRes) const override;
    virtual void MatCopy ( ScMatrix& mRes) const override;

    // Convert ScInterpreter::CompareMat values (-1,0,1) to boolean values
    virtual void CompareEqual() override;
    virtual void CompareNotEqual() override;
    virtual void CompareLess() override;
    virtual void CompareGreater() override;
    virtual void CompareLessEqual() override;
    virtual void CompareGreaterEqual() override;

    virtual double And() const override;       // logical AND of all matrix values, or NAN
    virtual double Or() const override;        // logical OR of all matrix values, or NAN
    virtual double Xor() const override;       // logical XOR of all matrix values, or NAN

    virtual IterateResult Sum(bool bTextAsZero) const override;
    virtual IterateResult SumSquare(bool bTextAsZero) const override;
    virtual IterateResult Product(bool bTextAsZero) const override;
    virtual size_t Count(bool bCountStrings, bool bCountErrors) const override;
    virtual size_t MatchDoubleInColumns(double fValue, size_t nCol1, size_t nCol2) const override;
    virtual size_t MatchStringInColumns(const svl::SharedString& rStr, size_t nCol1, size_t nCol2) const override;

    virtual double GetMaxValue( bool bTextAsZero ) const override;
    virtual double GetMinValue( bool bTextAsZero ) const override;
    virtual double GetGcd() const override;

    virtual ScMatrixRef CompareMatrix(
        sc::Compare& rComp, size_t nMatPos, sc::CompareOptions* pOptions ) const override;

    /**
     * Convert the content of matrix into a linear array of numeric values.
     * String elements are mapped to NaN's and empty elements are mapped to
     * either NaN or zero values.
     *
     * @param bEmptyAsZero if true empty elements are mapped to zero values,
     *                     otherwise they become NaN values.
     */
    virtual void GetDoubleArray( std::vector<double>& rArray, bool bEmptyAsZero = true ) const override;
    virtual void MergeDoubleArray( std::vector<double>& rArray, Op eOp ) const override;

    virtual void NotOp(ScMatrix& rMat) override;
    virtual void NegOp(ScMatrix& rMat) override;
    virtual void AddOp(double fVal, ScMatrix& rMat) override;
    virtual void SubOp(bool bFlag, double fVal, ScMatrix& rMat) override;
    virtual void MulOp(double fVal, ScMatrix& rMat) override;
    virtual void DivOp(bool bFlag, double fVal, ScMatrix& rMat) override;
    virtual void PowOp(bool bFlag, double fVal, ScMatrix& rMat) override;

    virtual std::vector<ScMatrix::IterateResult> Collect(bool bTextAsZero, const std::vector<std::unique_ptr<sc::op::Op>>& aOp) override;

    virtual void ExecuteOperation(const std::pair<size_t, size_t>& rStartPos, const std::pair<size_t, size_t>& rEndPos,
            DoubleOpFunction aDoubleFunc, BoolOpFunction aBoolFunc, StringOpFunction aStringFunc,
            EmptyOpFunction aEmptyFunc) const override;
    ScFullMatrix& operator+= ( const ScFullMatrix& r );

    virtual void MatConcat(SCSIZE nMaxCol, SCSIZE nMaxRow, const ScMatrixRef& xMat1, const ScMatrixRef& xMat2,
            SvNumberFormatter& rFormatter, svl::SharedStringPool& rPool) override;

#if DEBUG_MATRIX
    virtual void Dump() const override;
#endif
};

class SC_DLLPUBLIC ScVectorRefMatrix : public ScMatrix
{
    const formula::DoubleVectorRefToken* mpToken;
    ScInterpreter* mpErrorInterpreter;

    /// For the operations that are not fully implemented, create a ScFullMatrix, and operate on it.
    std::unique_ptr<ScFullMatrix> mpFullMatrix;

    SCSIZE mnRowStart;
    SCSIZE mnRowSize;

    ScVectorRefMatrix( const ScVectorRefMatrix& ) = delete;
    ScVectorRefMatrix& operator=( const ScVectorRefMatrix&) = delete;

    /// For the operations that are not fully implemented, create a ScFullMatrix, and operate on it.
    ///
    /// Note: This is potentially an expensive operation.
    /// TODO: Implement as much as possible directly using the DoubleVectorRefToken.
    void ensureFullMatrix();

public:

    ScVectorRefMatrix(const formula::DoubleVectorRefToken* pToken, SCSIZE nRowStart, SCSIZE nRowSize);

    virtual ~ScVectorRefMatrix() override;

    /** Clone the matrix. */
    virtual ScMatrix* Clone() const override;

    /**
     * Resize the matrix to specified new dimension.
     */
    virtual void Resize(SCSIZE nC, SCSIZE nR) override;

    virtual void Resize(SCSIZE nC, SCSIZE nR, double fVal) override;

    /** Clone the matrix and extend it to the new size. nNewCols and nNewRows
        MUST be at least of the size of the original matrix. */
    virtual ScMatrix* CloneAndExtend(SCSIZE nNewCols, SCSIZE nNewRows) const override;

    virtual void SetErrorInterpreter(ScInterpreter* p) override;
    virtual void GetDimensions(SCSIZE& rC, SCSIZE& rR) const override;
    virtual SCSIZE GetElementCount() const override;
    virtual bool ValidColRow( SCSIZE nC, SCSIZE nR) const override;

    /** For a row vector or column vector, if the position does not point into
        the vector but is a valid column or row offset it is adapted such that
        it points to an element to be replicated, same column row 0 for a row
        vector, same row column 0 for a column vector. Else, for a 2D matrix,
        returns false.
     */
    virtual bool ValidColRowReplicated(SCSIZE & rC, SCSIZE & rR) const override;

    /** Checks if the matrix position is within the matrix. If it is not, for a
        row vector or column vector the position is adapted such that it points
        to an element to be replicated, same column row 0 for a row vector,
        same row column 0 for a column vector. Else, for a 2D matrix and
        position not within matrix, returns false.
     */
    virtual bool ValidColRowOrReplicated(SCSIZE & rC, SCSIZE & rR) const override;

    virtual void PutDouble(double fVal, SCSIZE nC, SCSIZE nR) override;
    virtual void PutDouble(double fVal, SCSIZE nIndex) override;
    virtual void PutDouble(const double* pArray, size_t nLen, SCSIZE nC, SCSIZE nR) override;

    virtual void PutString(const svl::SharedString& rStr, SCSIZE nC, SCSIZE nR) override;
    virtual void PutString(const svl::SharedString& rStr, SCSIZE nIndex) override;
    virtual void PutString(const svl::SharedString* pArray, size_t nLen, SCSIZE nC, SCSIZE nR) override;

    virtual void PutEmpty(SCSIZE nC, SCSIZE nR) override;

    /// Jump sal_False without path
    virtual void PutEmptyPath(SCSIZE nC, SCSIZE nR) override;
    virtual void PutError(FormulaError nErrorCode, SCSIZE nC, SCSIZE nR ) override;
    virtual void PutBoolean(bool bVal, SCSIZE nC, SCSIZE nR) override;

    virtual void FillDouble(double fVal, SCSIZE nC1, SCSIZE nR1, SCSIZE nC2, SCSIZE nR2) override;

    /** Put a column vector of doubles, starting at row nR, must fit into dimensions. */
    virtual void PutDoubleVector(const ::std::vector< double > & rVec, SCSIZE nC, SCSIZE nR) override;

    /** Put a column vector of strings, starting at row nR, must fit into dimensions. */
    virtual void PutStringVector(const ::std::vector< svl::SharedString > & rVec, SCSIZE nC, SCSIZE nR) override;

    /** Put a column vector of empties, starting at row nR, must fit into dimensions. */
    virtual void PutEmptyVector(SCSIZE nCount, SCSIZE nC, SCSIZE nR) override;

    /** Put a column vector of empty results, starting at row nR, must fit into dimensions. */
    virtual void PutEmptyResultVector(SCSIZE nCount, SCSIZE nC, SCSIZE nR) override;

    /** Put a column vector of empty paths, starting at row nR, must fit into dimensions. */
    virtual void PutEmptyPathVector(SCSIZE nCount, SCSIZE nC, SCSIZE nR) override;

    /** May be used before obtaining the double value of an element to avoid
        passing its NAN around.
        @ATTENTION: MUST NOT be used if the element is a string!
                    Use GetErrorIfNotString() instead if not sure.
        @returns 0 if no error, else one of err... constants */
    virtual FormulaError GetError(SCSIZE nC, SCSIZE nR) const override;

    /// @return 0.0 if empty or empty path, else value or DoubleError.
    virtual double GetDouble(SCSIZE nC, SCSIZE nR) const override;
    /// @return 0.0 if empty or empty path, else value or DoubleError.
    virtual double GetDouble(SCSIZE nIndex) const override;
    /// @return value or DoubleError or string converted to value.
    virtual double GetDoubleWithStringConversion( SCSIZE nC, SCSIZE nR ) const override;

    /// @return empty string if empty or empty path, else string content.
    virtual svl::SharedString GetString(SCSIZE nC, SCSIZE nR) const override;
    /// @return empty string if empty or empty path, else string content.
    virtual svl::SharedString GetString(SCSIZE nIndex) const override;

    /** @returns the matrix element's string if one is present, otherwise the
        numerical value formatted as string, or in case of an error the error
        string is returned; an empty string for empty, a "FALSE" string for
        empty path. */
    virtual svl::SharedString GetString(SvNumberFormatter& rFormatter, SCSIZE nC, SCSIZE nR) const override;

    /// @ATTENTION: If bString the ScMatrixValue->pS may still be NULL to indicate
    /// an empty string!
    virtual ScMatrixValue Get(SCSIZE nC, SCSIZE nR) const override;

    /// @return <TRUE/> if string or empty or empty path, in fact non-value.
    virtual bool IsString(SCSIZE nIndex) const override;

    /// @return <TRUE/> if string or empty or empty path, in fact non-value.
    virtual bool IsString(SCSIZE nC, SCSIZE nR) const override;

    /// @return <TRUE/> if empty or empty cell or empty result, not empty path.
    virtual bool IsEmpty(SCSIZE nC, SCSIZE nR) const override;

    /// @return <TRUE/> if empty cell, not empty or empty result or empty path.
    virtual bool IsEmptyCell(SCSIZE nC, SCSIZE nR) const override;

    /// @return <TRUE/> if empty result, not empty or empty cell or empty path.
    virtual bool IsEmptyResult(SCSIZE nC, SCSIZE nR) const override;

    /// @return <TRUE/> if empty path, not empty or empty cell or empty result.
    virtual bool IsEmptyPath(SCSIZE nC, SCSIZE nR) const override;

    /// @return <TRUE/> if value or boolean.
    virtual bool IsValue(SCSIZE nIndex) const override;

    /// @return <TRUE/> if value or boolean.
    virtual bool IsValue(SCSIZE nC, SCSIZE nR) const override;

    /// @return <TRUE/> if value or boolean or empty or empty path.
    virtual bool IsValueOrEmpty(SCSIZE nC, SCSIZE nR) const override;

    /// @return <TRUE/> if boolean.
    virtual bool IsBoolean(SCSIZE nC, SCSIZE nR) const override;

    /// @return <TRUE/> if entire matrix is numeric, including booleans, with no strings or empties
    virtual bool IsNumeric() const override;

    virtual void MatTrans(ScMatrix& mRes) const override;
    virtual void MatCopy (ScMatrix& mRes) const override;

    // Convert ScInterpreter::CompareMat values (-1,0,1) to boolean values
    virtual void CompareEqual() override;
    virtual void CompareNotEqual() override;
    virtual void CompareLess() override;
    virtual void CompareGreater() override;
    virtual void CompareLessEqual() override;
    virtual void CompareGreaterEqual() override;

    virtual double And() const override;       // logical AND of all matrix values, or NAN
    virtual double Or() const override;        // logical OR of all matrix values, or NAN
    virtual double Xor() const override;       // logical XOR of all matrix values, or NAN

    virtual IterateResult Sum(bool bTextAsZero) const override;
    virtual IterateResult SumSquare(bool bTextAsZero) const override;
    virtual IterateResult Product(bool bTextAsZero) const override;
    virtual size_t Count(bool bCountStrings, bool bCountErrors) const override;
    virtual size_t MatchDoubleInColumns(double fValue, size_t nCol1, size_t nCol2) const override;
    virtual size_t MatchStringInColumns(const svl::SharedString& rStr, size_t nCol1, size_t nCol2) const override;

    virtual double GetMaxValue(bool bTextAsZero) const override;
    virtual double GetMinValue(bool bTextAsZero) const override;
    virtual double GetGcd() const override;

    virtual ScMatrixRef CompareMatrix(sc::Compare& rComp, size_t nMatPos, sc::CompareOptions* pOptions) const override;

    /**
     * Convert the content of matrix into a linear array of numeric values.
     * String elements are mapped to NaN's and empty elements are mapped to
     * either NaN or zero values.
     *
     * @param bEmptyAsZero if true empty elements are mapped to zero values,
     *                     otherwise they become NaN values.
     */
    virtual void GetDoubleArray(std::vector<double>& rVector, bool bEmptyAsZero = true) const override;
    virtual void MergeDoubleArray(std::vector<double>& rVector, Op eOp) const override;

    virtual void NotOp(ScMatrix& rMat) override;
    virtual void NegOp(ScMatrix& rMat) override;
    virtual void AddOp(double fVal, ScMatrix& rMat) override;
    virtual void SubOp(bool bFlag, double fVal, ScMatrix& rMat) override;
    virtual void MulOp(double fVal, ScMatrix& rMat) override;
    virtual void DivOp(bool bFlag, double fVal, ScMatrix& rMat) override;
    virtual void PowOp(bool bFlag, double fVal, ScMatrix& rMat) override;

    virtual std::vector<ScMatrix::IterateResult> Collect(bool bTextAsZero, const std::vector<std::unique_ptr<sc::op::Op>>& aOp) override;

    virtual void ExecuteOperation(const std::pair<size_t, size_t>& rStartPos, const std::pair<size_t, size_t>& rEndPos,
            DoubleOpFunction aDoubleFunc, BoolOpFunction aBoolFunc, StringOpFunction aStringFunc,
            EmptyOpFunction aEmptyFunc) const override;

    ScVectorRefMatrix& operator+=(const ScVectorRefMatrix& r);

    virtual void MatConcat(SCSIZE nMaxCol, SCSIZE nMaxRow, const ScMatrixRef& xMat1, const ScMatrixRef& xMat2,
            SvNumberFormatter& rFormatter, svl::SharedStringPool& rPool) override;

#if DEBUG_MATRIX
    virtual void Dump() const override
    {
    }
#endif
};

inline void intrusive_ptr_add_ref(const ScMatrix* p)
{
    p->IncRef();
}

inline void intrusive_ptr_release(const ScMatrix* p)
{
    p->DecRef();
}

#endif // INCLUDED_SC_INC_SCMATRIX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
