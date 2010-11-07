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

#ifndef SC_MATRIX_HXX
#define SC_MATRIX_HXX

#include "global.hxx"
#include "formula/intruref.hxx"
#include "formula/errorcodes.hxx"
#include <tools/string.hxx>
#include "scdllapi.h"

class SvStream;
class ScInterpreter;
class SvNumberFormatter;
class ScMatrixImpl;

typedef BYTE ScMatValType;
const ScMatValType SC_MATVAL_VALUE     = 0x00;
const ScMatValType SC_MATVAL_BOOLEAN   = 0x01;
const ScMatValType SC_MATVAL_STRING    = 0x02;
const ScMatValType SC_MATVAL_EMPTY     = SC_MATVAL_STRING | 0x04; // STRING plus flag
const ScMatValType SC_MATVAL_EMPTYPATH = SC_MATVAL_EMPTY | 0x08;  // EMPTY plus flag
const ScMatValType SC_MATVAL_NONVALUE  = SC_MATVAL_EMPTYPATH;     // mask of all non-value bits

struct ScMatrixValue
{
    union {
        double fVal;
        const String* pS;
    };
    ScMatValType nType;

    /// Only valid if ScMatrix methods indicate so!
    const String& GetString() const { return pS ? *pS : EMPTY_STRING; }

    /// Only valid if ScMatrix methods indicate that this is no string!
    USHORT GetError() const         { return GetDoubleErrorValue( fVal); }

    /// Only valid if ScMatrix methods indicate that this is a boolean
    bool GetBoolean() const         { return fVal != 0.0; }

    ScMatrixValue() : pS(NULL), nType(SC_MATVAL_EMPTY) {}

    ScMatrixValue(const ScMatrixValue& r) : nType(r.nType)
    {
        switch (nType)
        {
            case SC_MATVAL_VALUE:
            case SC_MATVAL_BOOLEAN:
                fVal = r.fVal;
            break;
            default:
                pS = r.pS;
        }
    }

    bool operator== (const ScMatrixValue& r) const
    {
        if (nType != r.nType)
            return false;

        switch (nType)
        {
            case SC_MATVAL_VALUE:
            case SC_MATVAL_BOOLEAN:
                return fVal == r.fVal;
            break;
            default:
                ;
        }
        if (!pS)
            return r.pS == NULL;

        return GetString().Equals(r.GetString());
    }

    bool operator!= (const ScMatrixValue& r) const
    {
        return !operator==(r);
    }

    ScMatrixValue& operator= (const ScMatrixValue& r)
    {
        nType = r.nType;
        switch (nType)
        {
            case SC_MATVAL_VALUE:
            case SC_MATVAL_BOOLEAN:
                fVal = r.fVal;
            break;
            default:
                pS = r.pS;
        }
        return *this;
    }
};

/** Matrix representation of double values and strings.

    @ATTENTION: optimized for speed and double values.

    <p> Matrix elements are NOT initialized after construction!

    <p> All methods using an SCSIZE nIndex parameter and all Is...() methods do
    NOT check the range for validity! However, the Put...() and Get...()
    methods using nCol/nRow parameters do check the range.

    <p> Methods using nCol/nRow parameters do replicate a single row vector if
    nRow &gt; 0 and nCol &lt; nColCount, respectively a column vector if nCol
    &gt; 0 and nRow &lt; nRowCount.

    <p> GetString( SCSIZE nIndex ) does not check if there really is a string,
    do this with IsString() first. GetString( SCSIZE nC, SCSIZE nR ) does check
    it and returns and empty string if there is no string. Both GetDouble()
    methods don't check for a string, do this with IsNumeric() or IsString() or
    IsValue() first.

    <p> The GetString( SvNumberFormatter&, ...) methods return the matrix
    element's string if one is present, otherwise the numerical value is
    formatted as a string, or in case of an error the error string is returned.

    <p> PutDouble() does not reset an eventual string! Use
    PutDoubleAndResetString() if that is wanted. Also the FillDouble...()
    methods don't reset strings. As a consequence memory leaks may occur if
    used wrong.
 */
class SC_DLLPUBLIC ScMatrix
{
    ScMatrixImpl*   pImpl;
    mutable ULONG   nRefCnt;    // reference count

    // only delete via Delete()
    ~ScMatrix();

    // not implemented, prevent usage
    ScMatrix( const ScMatrix& );
    ScMatrix& operator=( const ScMatrix&);

public:
    enum DensityType
    {
        FILLED_ZERO,
        FILLED_EMPTY,
        SPARSE_ZERO,
        SPARSE_EMPTY
    };

    /// The maximum number of elements a matrix may have at runtime.
    inline static size_t GetElementsMax()
    {
        // Roughly 125MB in total, divided by 8+1 per element => 14M elements.
        const size_t nMemMax = 0x08000000 / (sizeof(ScMatrixValue) + sizeof(ScMatValType));
        // With MAXROWCOUNT==65536 and 128 columns => 8M elements ~72MB.
        const size_t nArbitraryLimit = (size_t)MAXROWCOUNT * 128;
        // Stuffed with a million rows would limit this to 14 columns.
        return nMemMax < nArbitraryLimit ? nMemMax : nArbitraryLimit;
    }

    /// Value or boolean.
    inline static bool IsValueType( ScMatValType nType )
    {
        return nType <= SC_MATVAL_BOOLEAN;
    }

    /// Boolean.
    inline static bool IsBooleanType( ScMatValType nType )
    {
        return nType == SC_MATVAL_BOOLEAN;
    }

    /// String, empty or empty path, but not value nor boolean.
    inline static bool IsNonValueType( ScMatValType nType )
    {
        return (nType & SC_MATVAL_NONVALUE) != 0;
    }

    /** String, but not empty or empty path or any other type.
        Not named IsStringType to prevent confusion because previously
        IsNonValueType was named IsStringType. */
    inline static bool IsRealStringType( ScMatValType nType )
    {
        return (nType & SC_MATVAL_NONVALUE) == SC_MATVAL_STRING;
    }

    /// Empty, but not empty path or any other type.
    inline static bool IsEmptyType( ScMatValType nType )
    {
        return (nType & SC_MATVAL_NONVALUE) == SC_MATVAL_EMPTY;
    }

    /// Empty path, but not empty or any other type.
    inline static bool IsEmptyPathType( ScMatValType nType )
    {
        return (nType & SC_MATVAL_NONVALUE) == SC_MATVAL_EMPTYPATH;
    }

    /** If nC*nR results in more than GetElementsMax() entries, a 1x1 matrix is
        created instead and a double error value (errStackOverflow) is set.
        Compare nC and nR with a GetDimensions() call to check. */
    ScMatrix( SCSIZE nC, SCSIZE nR, DensityType eType = FILLED_ZERO);

    /** Clone the matrix. */
    ScMatrix* Clone( DensityType eType) const;

    /** Clone the matrix if mbCloneIfConst (immutable) is set, otherwise
        return _this_ matrix, to be assigned to a ScMatrixRef. */
    ScMatrix* CloneIfConst();

    /** Set the matrix to (im)mutable for CloneIfConst(), only the interpreter
        should do this and know the consequences. */
    void SetImmutable( bool bVal );

    /**
     * Resize the matrix to specified new dimension.  Note that this operation
     * clears all stored values.
     */
    void Resize( SCSIZE nC, SCSIZE nR);

    /** Clone the matrix and extend it to the new size. nNewCols and nNewRows
        MUST be at least of the size of the original matrix. */
    ScMatrix* CloneAndExtend( SCSIZE nNewCols, SCSIZE nNewRows, DensityType eType) const;

    /// Disable refcounting forever, may only be deleted via Delete() afterwards.
    inline  void    SetEternalRef()         { nRefCnt = ULONG_MAX; }
    inline  bool    IsEternalRef() const    { return nRefCnt == ULONG_MAX; }
    inline  void    IncRef() const
    {
        if ( !IsEternalRef() )
            ++nRefCnt;
    }
    inline  void    DecRef() const
    {
        if ( nRefCnt > 0 && !IsEternalRef() )
            if ( --nRefCnt == 0 )
                delete this;
    }
    inline  void    Delete()
    {
        if ( nRefCnt == 0 || IsEternalRef() )
            delete this;
        else
            --nRefCnt;
    }

    DensityType GetDensityType() const;
    void SetErrorInterpreter( ScInterpreter* p);
    void GetDimensions( SCSIZE& rC, SCSIZE& rR) const;
    SCSIZE GetElementCount() const;
    bool ValidColRow( SCSIZE nC, SCSIZE nR) const;
    SCSIZE CalcOffset( SCSIZE nC, SCSIZE nR) const;

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
    void PutString( const String& rStr, SCSIZE nC, SCSIZE nR);
    void PutString( const String& rStr, SCSIZE nIndex);
    void PutEmpty( SCSIZE nC, SCSIZE nR);
    void PutEmpty( SCSIZE nIndex);
    /// Jump FALSE without path
    void PutEmptyPath( SCSIZE nC, SCSIZE nR);
    void PutEmptyPath( SCSIZE nIndex);
    void PutError( USHORT nErrorCode, SCSIZE nC, SCSIZE nR );
    void PutError( USHORT nErrorCode, SCSIZE nIndex );
    void PutBoolean( bool bVal, SCSIZE nC, SCSIZE nR);
    void PutBoolean( bool bVal, SCSIZE nIndex);

    void FillDouble( double fVal,
            SCSIZE nC1, SCSIZE nR1, SCSIZE nC2, SCSIZE nR2 );

    /** May be used before obtaining the double value of an element to avoid
        passing its NAN around.
        @ATTENTION: MUST NOT be used if the element is a string!
                    Use GetErrorIfNotString() instead if not sure.
        @returns 0 if no error, else one of err... constants */
    USHORT GetError( SCSIZE nC, SCSIZE nR) const;
    USHORT GetError( SCSIZE nIndex) const;

    /** Use in ScInterpreter to obtain the error code, if any.
        @returns 0 if no error or string element, else one of err... constants */
    USHORT GetErrorIfNotString( SCSIZE nC, SCSIZE nR) const
        { return IsValue( nC, nR) ? GetError( nC, nR) : 0; }
    USHORT GetErrorIfNotString( SCSIZE nIndex) const
        { return IsValue( nIndex) ? GetError( nIndex) : 0; }

    /// @return 0.0 if empty or empty path, else value or DoubleError.
    double GetDouble( SCSIZE nC, SCSIZE nR) const;
    /// @return 0.0 if empty or empty path, else value or DoubleError.
    double GetDouble( SCSIZE nIndex) const;

    /// @return empty string if empty or empty path, else string content.
    const String& GetString( SCSIZE nC, SCSIZE nR) const;
    /// @return empty string if empty or empty path, else string content.
    const String& GetString( SCSIZE nIndex) const;

    /** @returns the matrix element's string if one is present, otherwise the
        numerical value formatted as string, or in case of an error the error
        string is returned; an empty string for empty, a "FALSE" string for
        empty path. */
    String GetString( SvNumberFormatter& rFormatter, SCSIZE nC, SCSIZE nR) const;
    String GetString( SvNumberFormatter& rFormatter, SCSIZE nIndex) const;

    /// @ATTENTION: If bString the ScMatrixValue->pS may still be NULL to indicate
    /// an empty string!
    ScMatrixValue Get( SCSIZE nC, SCSIZE nR) const;

    /// @return <TRUE/> if string or empty or empty path, in fact non-value.
    BOOL IsString( SCSIZE nIndex ) const;

    /// @return <TRUE/> if string or empty or empty path, in fact non-value.
    BOOL IsString( SCSIZE nC, SCSIZE nR ) const;

    /// @return <TRUE/> if empty or empty path.
    BOOL IsEmpty( SCSIZE nIndex ) const;

    /// @return <TRUE/> if empty or empty path.
    BOOL IsEmpty( SCSIZE nC, SCSIZE nR ) const;

    /// @return <TRUE/> if empty path.
    BOOL IsEmptyPath( SCSIZE nC, SCSIZE nR ) const;

    /// @return <TRUE/> if empty path.
    BOOL IsEmptyPath( SCSIZE nIndex ) const;

    /// @return <TRUE/> if value or boolean.
    BOOL IsValue( SCSIZE nIndex ) const;

    /// @return <TRUE/> if value or boolean.
    BOOL IsValue( SCSIZE nC, SCSIZE nR ) const;

    /// @return <TRUE/> if value or boolean or empty or empty path.
    BOOL IsValueOrEmpty( SCSIZE nIndex ) const;

    /// @return <TRUE/> if value or boolean or empty or empty path.
    BOOL IsValueOrEmpty( SCSIZE nC, SCSIZE nR ) const;

    /// @return <TRUE/> if boolean.
    BOOL IsBoolean( SCSIZE nIndex ) const;

    /// @return <TRUE/> if boolean.
    BOOL IsBoolean( SCSIZE nC, SCSIZE nR ) const;

    /// @return <TRUE/> if entire matrix is numeric, including booleans, with no strings or empties
    BOOL IsNumeric() const;

    void MatTrans( ScMatrix& mRes) const;
    void MatCopy ( ScMatrix& mRes) const;

    // Convert ScInterpreter::CompareMat values (-1,0,1) to boolean values
    void CompareEqual();
    void CompareNotEqual();
    void CompareLess();
    void CompareGreater();
    void CompareLessEqual();
    void CompareGreaterEqual();

    double And();       // logical AND of all matrix values, or NAN
    double Or();        // logical OR of all matrix values, or NAN

    // All other matrix functions  MatMult, MInv, ...  are in ScInterpreter
    // to be numerically safe.
};


typedef formula::SimpleIntrusiveReference< class ScMatrix > ScMatrixRef;
typedef formula::SimpleIntrusiveReference< const class ScMatrix > ScConstMatrixRef;


#endif  // SC_MATRIX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
