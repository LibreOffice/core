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

#include "address.hxx"
#include "matrixoperators.hxx"
#include "types.hxx"
#include <formula/errorcodes.hxx>
#include "scdllapi.h"
#include <svl/sharedstring.hxx>
#include <svl/sharedstringpool.hxx>

#include <memory>
#include <utility>
#include <vector>

#define DEBUG_MATRIX 0

class ScInterpreter;
class SvNumberFormatter;
class ScMatrixImpl;
enum class FormulaError : sal_uInt16;

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

/**
 * Matrix data type that can store values of mixed types.  Each element can
 * be one of the following types: numeric, string, boolean, empty, and empty
 * path.
 */
class SC_DLLPUBLIC ScMatrix final
{
    friend class ScMatrixImpl;

    mutable size_t  nRefCnt;    // reference count
    mutable bool    mbCloneIfConst; // Whether the matrix is cloned with a CloneIfConst() call.
    std::unique_ptr<ScMatrixImpl> pImpl;

    ScMatrix( const ScMatrix& ) = delete;
    ScMatrix& operator=( const ScMatrix&) = delete;

public:
    ScMatrix(SCSIZE nC, SCSIZE nR);
    ScMatrix(SCSIZE nC, SCSIZE nR, double fInitVal);
    ScMatrix( size_t nC, size_t nR, const std::vector<double>& rInitVals );
    ~ScMatrix();

    typedef std::function<void(size_t, size_t, double)> DoubleOpFunction;
    typedef std::function<void(size_t, size_t, bool)> BoolOpFunction;
    typedef std::function<void(size_t, size_t, svl::SharedString)> StringOpFunction;
    typedef std::function<void(size_t, size_t)> EmptyOpFunction;

    /**
     * When adding all numerical matrix elements for a scalar result such as
     * summation, the interpreter wants to separate the first non-zero value
     * with the rest of the summed values. This is necessary for better
     * numerical stability, unless we sort all by absolute values before
     * summing (not really an option) or use another algorithm, e.g. Kahan's
     * summation algorithm,
     * https://en.wikipedia.org/wiki/Kahan_summation_algorithm
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

    /** Checks nC or nR for zero and uses GetElementsMax() whether a matrix of
        the size of nC*nR could be allocated. A zero size (both nC and nR zero)
        matrix is allowed for later resize.
     */
    bool static IsSizeAllocatable( SCSIZE nC, SCSIZE nR );

    /// Value or boolean.
    static bool IsValueType( ScMatValType nType )
    {
        return nType <= ScMatValType::Boolean;
    }

    /// Boolean.
    static bool IsBooleanType( ScMatValType nType )
    {
        return nType == ScMatValType::Boolean;
    }

    /// String, empty or empty path, but not value nor boolean.
    static bool IsNonValueType( ScMatValType nType )
    {
        return bool(nType & ScMatValType::NonvalueMask);
    }

    /** String, but not empty or empty path or any other type.
        Not named IsStringType to prevent confusion because previously
        IsNonValueType was named IsStringType. */
    static bool IsRealStringType( ScMatValType nType )
    {
        return (nType & ScMatValType::NonvalueMask) == ScMatValType::String;
    }

    /// Empty, but not empty path or any other type.
    static bool IsEmptyType( ScMatValType nType )
    {
        return (nType & ScMatValType::NonvalueMask) == ScMatValType::Empty;
    }

    /// Empty path, but not empty or any other type.
    static bool IsEmptyPathType( ScMatValType nType )
    {
        return (nType & ScMatValType::NonvalueMask) == ScMatValType::EmptyPath;
    }

    /** Clone the matrix. */
    ScMatrix* Clone() const;

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
    void Resize(SCSIZE nC, SCSIZE nR);

    void Resize(SCSIZE nC, SCSIZE nR, double fVal);

    /** Clone the matrix and extend it to the new size. nNewCols and nNewRows
        MUST be at least of the size of the original matrix. */
    ScMatrix* CloneAndExtend(SCSIZE nNewCols, SCSIZE nNewRows) const;

    void IncRef() const;
    void DecRef() const;

    void SetErrorInterpreter( ScInterpreter* p);
    void GetDimensions( SCSIZE& rC, SCSIZE& rR) const;
    SCSIZE GetElementCount() const;
    bool ValidColRow( SCSIZE nC, SCSIZE nR) const;

    /** For a row vector or column vector, if the position does not point into
        the vector but is a valid column or row offset it is adapted such that
        it points to an element to be replicated, same column row 0 for a row
        vector, same row column 0 for a column vector. Else, for a 2D matrix,
        returns false.
     */
    bool ValidColRowReplicated( SCSIZE & rC, SCSIZE & rR ) const;

    /** Checks if the matrix position is within the matrix. If it is not, for a
        row vector or column vector the position is adapted such that it points
        to an element to be replicated, same column row 0 for a row vector,
        same row column 0 for a column vector. Else, for a 2D matrix and
        position not within matrix, returns false.
     */
    bool ValidColRowOrReplicated( SCSIZE & rC, SCSIZE & rR ) const;

    void PutDouble( double fVal, SCSIZE nC, SCSIZE nR);
    void PutDouble( double fVal, SCSIZE nIndex);
    void PutDouble(const double* pArray, size_t nLen, SCSIZE nC, SCSIZE nR);

    void PutString( const svl::SharedString& rStr, SCSIZE nC, SCSIZE nR) ;
    void PutString( const svl::SharedString& rStr, SCSIZE nIndex) ;
    void PutString( const svl::SharedString* pArray, size_t nLen, SCSIZE nC, SCSIZE nR) ;

    void PutEmpty( SCSIZE nC, SCSIZE nR);

    /// Jump sal_False without path
    void PutEmptyPath( SCSIZE nC, SCSIZE nR) ;
    void PutError( FormulaError nErrorCode, SCSIZE nC, SCSIZE nR ) ;
    void PutBoolean( bool bVal, SCSIZE nC, SCSIZE nR) ;

    void FillDouble( double fVal,
            SCSIZE nC1, SCSIZE nR1, SCSIZE nC2, SCSIZE nR2 ) ;

    /** Put a column vector of doubles, starting at row nR, must fit into dimensions. */
    void PutDoubleVector( const ::std::vector< double > & rVec, SCSIZE nC, SCSIZE nR ) ;

    /** Put a column vector of strings, starting at row nR, must fit into dimensions. */
    void PutStringVector( const ::std::vector< svl::SharedString > & rVec, SCSIZE nC, SCSIZE nR ) ;

    /** Put a column vector of empties, starting at row nR, must fit into dimensions. */
    void PutEmptyVector( SCSIZE nCount, SCSIZE nC, SCSIZE nR ) ;

    /** Put a column vector of empty results, starting at row nR, must fit into dimensions. */
    void PutEmptyResultVector( SCSIZE nCount, SCSIZE nC, SCSIZE nR ) ;

    /** Put a column vector of empty paths, starting at row nR, must fit into dimensions. */
    void PutEmptyPathVector( SCSIZE nCount, SCSIZE nC, SCSIZE nR ) ;

    /** May be used before obtaining the double value of an element to avoid
        passing its NAN around.
        @ATTENTION: MUST NOT be used if the element is a string!
                    Use GetErrorIfNotString() instead if not sure.
        @returns 0 if no error, else one of err... constants */
    FormulaError GetError( SCSIZE nC, SCSIZE nR) const ;

    /** Use in ScInterpreter to obtain the error code, if any.
        @returns 0 if no error or string element, else one of err... constants */
    FormulaError GetErrorIfNotString( SCSIZE nC, SCSIZE nR) const
        { return IsValue( nC, nR) ? GetError( nC, nR) : FormulaError::NONE; }

    /// @return 0.0 if empty or empty path, else value or DoubleError.
    double GetDouble( SCSIZE nC, SCSIZE nR) const ;
    /// @return 0.0 if empty or empty path, else value or DoubleError.
    double GetDouble( SCSIZE nIndex) const ;
    /// @return value or DoubleError or string converted to value.
    double GetDoubleWithStringConversion( SCSIZE nC, SCSIZE nR ) const ;

    /// @return empty string if empty or empty path, else string content.
    svl::SharedString GetString( SCSIZE nC, SCSIZE nR) const ;
    /// @return empty string if empty or empty path, else string content.
    svl::SharedString GetString( SCSIZE nIndex) const ;

    /** @returns the matrix element's string if one is present, otherwise the
        numerical value formatted as string, or in case of an error the error
        string is returned; an empty string for empty, a "FALSE" string for
        empty path. */
    svl::SharedString GetString( SvNumberFormatter& rFormatter, SCSIZE nC, SCSIZE nR) const ;

    /// @ATTENTION: If bString the ScMatrixValue->pS may still be NULL to indicate
    /// an empty string!
    ScMatrixValue Get( SCSIZE nC, SCSIZE nR) const ;

    /** @return <TRUE/> if string or any empty, empty cell, empty result, empty
        path, in fact non-value. */
    bool IsStringOrEmpty( SCSIZE nIndex ) const ;

    /** @return <TRUE/> if string or any empty, empty cell, empty result, empty
        path, in fact non-value. */
    bool IsStringOrEmpty( SCSIZE nC, SCSIZE nR ) const ;

    /// @return <TRUE/> if empty or empty cell or empty result, not empty path.
    bool IsEmpty( SCSIZE nC, SCSIZE nR ) const ;

    /// @return <TRUE/> if empty cell, not empty or empty result or empty path.
    bool IsEmptyCell( SCSIZE nC, SCSIZE nR ) const ;

    /// @return <TRUE/> if empty result, not empty or empty cell or empty path.
    bool IsEmptyResult( SCSIZE nC, SCSIZE nR ) const ;

    /// @return <TRUE/> if empty path, not empty or empty cell or empty result.
    bool IsEmptyPath( SCSIZE nC, SCSIZE nR ) const ;

    /// @return <TRUE/> if value or boolean.
    bool IsValue( SCSIZE nIndex ) const ;

    /// @return <TRUE/> if value or boolean.
    bool IsValue( SCSIZE nC, SCSIZE nR ) const ;

    /// @return <TRUE/> if value or boolean or empty or empty path.
    bool IsValueOrEmpty( SCSIZE nC, SCSIZE nR ) const ;

    /// @return <TRUE/> if boolean.
    bool IsBoolean( SCSIZE nC, SCSIZE nR ) const ;

    /// @return <TRUE/> if entire matrix is numeric, including booleans, with no strings or empties
    bool IsNumeric() const ;

    void MatTrans( const ScMatrix& mRes) const ;
    void MatCopy ( const ScMatrix& mRes) const ;

    // Convert ScInterpreter::CompareMat values (-1,0,1) to boolean values
    void CompareEqual() ;
    void CompareNotEqual() ;
    void CompareLess() ;
    void CompareGreater() ;
    void CompareLessEqual() ;
    void CompareGreaterEqual() ;

    double And() const ;       // logical AND of all matrix values, or NAN
    double Or() const ;        // logical OR of all matrix values, or NAN
    double Xor() const ;       // logical XOR of all matrix values, or NAN

    IterateResult Sum(bool bTextAsZero) const ;
    IterateResult SumSquare(bool bTextAsZero) const ;
    IterateResult Product(bool bTextAsZero) const ;
    size_t Count(bool bCountStrings, bool bCountErrors) const ;
    size_t MatchDoubleInColumns(double fValue, size_t nCol1, size_t nCol2) const ;
    size_t MatchStringInColumns(const svl::SharedString& rStr, size_t nCol1, size_t nCol2) const ;

    double GetMaxValue( bool bTextAsZero ) const ;
    double GetMinValue( bool bTextAsZero ) const ;
    double GetGcd() const ;
    double GetLcm() const ;

    ScMatrixRef CompareMatrix(
        sc::Compare& rComp, size_t nMatPos, sc::CompareOptions* pOptions ) const ;

    /**
     * Convert the content of matrix into a linear array of numeric values.
     * String elements are mapped to NaN's and empty elements are mapped to
     * either NaN or zero values.
     *
     * @param bEmptyAsZero if true empty elements are mapped to zero values,
     *                     otherwise they become NaN values.
     */
    void GetDoubleArray( std::vector<double>& rArray, bool bEmptyAsZero = true ) const ;
    void MergeDoubleArrayMultiply( std::vector<double>& rArray ) const ;

    void NotOp(const ScMatrix& rMat) ;
    void NegOp(const ScMatrix& rMat) ;
    void AddOp(double fVal, const ScMatrix& rMat) ;
    void SubOp(bool bFlag, double fVal, const ScMatrix& rMat) ;
    void MulOp(double fVal, const ScMatrix& rMat) ;
    void DivOp(bool bFlag, double fVal, const ScMatrix& rMat) ;
    void PowOp(bool bFlag, double fVal, const ScMatrix& rMat) ;

    std::vector<ScMatrix::IterateResult> Collect(const std::vector<sc::op::Op>& aOp) ;

    void ExecuteOperation(const std::pair<size_t, size_t>& rStartPos, const std::pair<size_t, size_t>& rEndPos,
            DoubleOpFunction aDoubleFunc, BoolOpFunction aBoolFunc, StringOpFunction aStringFunc,
            EmptyOpFunction aEmptyFunc) const ;

    void MatConcat(SCSIZE nMaxCol, SCSIZE nMaxRow, const ScMatrixRef& xMat1, const ScMatrixRef& xMat2,
            SvNumberFormatter& rFormatter, svl::SharedStringPool& rPool) ;

#if DEBUG_MATRIX
    void Dump() const;
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
