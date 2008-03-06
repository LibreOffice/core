/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: scmatrix.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 15:19:59 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SC_MATRIX_HXX
#define SC_MATRIX_HXX

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif
#ifndef SC_INTRUREF_HXX
#include "intruref.hxx"
#endif
#ifndef SC_ERRORCODES_HXX
#include "errorcodes.hxx"
#endif

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

class SvStream;
class ScInterpreter;
class SvNumberFormatter;

typedef BYTE ScMatValType;
const ScMatValType SC_MATVAL_VALUE     = 0x00;
const ScMatValType SC_MATVAL_BOOLEAN   = 0x01;
const ScMatValType SC_MATVAL_STRING    = 0x02;
const ScMatValType SC_MATVAL_EMPTY     = SC_MATVAL_STRING | 0x04; // STRING plus flag
const ScMatValType SC_MATVAL_EMPTYPATH = SC_MATVAL_EMPTY | 0x08;  // EMPTY plus flag

union ScMatrixValue
{
    double fVal;
    String* pS;

    /// Only valid if ScMatrix methods indicate so!
    const String& GetString() const     { return pS ? *pS : EMPTY_STRING; }

    /// Only valid if ScMatrix methods indicate that this is no string!
    USHORT GetError() const         { return GetDoubleErrorValue( fVal); }

    /// Only valid if ScMatrix methods indicate that this is a boolean
    bool GetBoolean() const         { return fVal != 0.; }
};

/** Matrix representation of double values and strings.

    @ATTENTION: optimized for speed and double values.

    Matrix elements are NOT initialized after construction!

    All methods using an SCSIZE nIndex parameter and all Is... methods do NOT
    check the range for validity! However, the Put... and Get... methods using
    nCol/nRow parameters do check the range. <p>

    GetString( SCSIZE nIndex ) does not check if there really is a string, do
    this with IsString() first. GetString( SCSIZE nC, SCSIZE nR ) does check
    it and returns and empty string if there is no string. Both GetDouble()
    methods don't check for a string, do this with IsNumeric() or IsString()
    or IsValue() first. <p>

    The GetString( SvNumberFormatter&, ...) methods return the matrix element's
    string if one is present, otherwise the numerical value is formatted as a
    string, or in case of an error the error string is returned.

    PutDouble() does not reset an eventual string! Use
    PutDoubleAndResetString() if that is wanted. Also the FillDouble...()
    methods don't reset strings. As a consequence memory leaks may occur if
    used wrong.
 */
class ScMatrix
{
    ScMatrixValue*  pMat;
    ScMatValType*   mnValType;
    ULONG           mnNonValue; // how many strings and empties
    ScInterpreter*  pErrorInterpreter;
    mutable ULONG   nRefCnt;    // reference count
    SCSIZE          nColCount;
    SCSIZE          nRowCount;

    void ResetIsString();
    void DeleteIsString();
    void CreateMatrix( SCSIZE nC, SCSIZE nR);

    // pStr may be NULL, bFlag MUST NOT be 0
    void PutStringEntry( const String* pStr, BYTE bFlag, SCSIZE nIndex );

    // only delete via Delete()
    ~ScMatrix();

    // not implemented, prevent usage
    ScMatrix( const ScMatrix& );
    ScMatrix& operator=( const ScMatrix&);

    void SetErrorAtInterpreter( USHORT nError) const;

public:

    /// The maximum number of elements a matrix may have at runtime
    inline static size_t GetElementsMax()
    {
        const size_t nMemMax = (((size_t)(~0))-64) / sizeof(ScMatrixValue);
        const size_t nArbitraryLimit = 0x80000;  // 512k elements ~= 4MB memory
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
    inline static bool IsStringType( ScMatValType nType )
    {
        return (nType & SC_MATVAL_STRING) != 0;
    }

    /// Empty, but not empty path or any other type.
    inline static bool IsEmptyType( ScMatValType nType )
    {
        return (nType & SC_MATVAL_EMPTY) == SC_MATVAL_EMPTY;
    }

    /// Empty path, but not empty or any other type.
    inline static bool IsEmptyPathType( ScMatValType nType )
    {
        return (nType & SC_MATVAL_EMPTYPATH) == SC_MATVAL_EMPTYPATH;
    }

    /** If nC*nR results in more than GetElementsMax() entries, a 1x1 matrix is
        created instead and a double error value (errStackOverflow) is set.
        Compare nC and nR with a GetDimensions() call to check. */
    ScMatrix( SCSIZE nC, SCSIZE nR) : nRefCnt(0) { CreateMatrix( nC, nR); }
    ScMatrix* Clone() const;

    /// disable refcounting forever, may only be deleted via Delete() afterwards
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

    void SetErrorInterpreter( ScInterpreter* p)
        { pErrorInterpreter = p; }

    ScMatrix( SvStream& rStream);
    void Store( SvStream& rStream) const;

    void GetDimensions( SCSIZE& rC, SCSIZE& rR) const
        { rC = nColCount; rR = nRowCount; };
    SCSIZE GetElementCount() const
        { return nColCount * nRowCount; }
    inline bool ValidColRow( SCSIZE nC, SCSIZE nR) const
        { return nC < nColCount && nR < nRowCount; }
    inline SCSIZE CalcOffset( SCSIZE nC, SCSIZE nR) const
        { return nC * nRowCount + nR; }

    void PutDouble( double fVal, SCSIZE nC, SCSIZE nR);
    void PutDouble( double fVal, SCSIZE nIndex)
        { pMat[nIndex].fVal = fVal; }
    void PutDoubleAndResetString( double fVal, SCSIZE nC, SCSIZE nR );
    void PutDoubleAndResetString( double fVal, SCSIZE nIndex );
    void PutString( const String& rStr, SCSIZE nC, SCSIZE nR);
    void PutString( const String& rStr, SCSIZE nIndex);
    void PutEmpty( SCSIZE nC, SCSIZE nR);
    void PutEmpty( SCSIZE nIndex);
    /// Jump FALSE without path
    void PutEmptyPath( SCSIZE nC, SCSIZE nR);
    void PutEmptyPath( SCSIZE nIndex);
    void PutError( USHORT nErrorCode, SCSIZE nC, SCSIZE nR )
        { PutDouble( CreateDoubleError( nErrorCode ), nC, nR ); }
    void PutError( USHORT nErrorCode, SCSIZE nIndex )
        { PutDouble( CreateDoubleError( nErrorCode ), nIndex ); }
    void PutBoolean( bool bVal, SCSIZE nC, SCSIZE nR);
    void PutBoolean( bool bVal, SCSIZE nIndex);

    void FillDouble( double fVal,
            SCSIZE nC1, SCSIZE nR1, SCSIZE nC2, SCSIZE nR2 );
    /// lower left triangle
    void FillDoubleLowerLeft( double fVal, SCSIZE nC2 );

    /** May be used before obtaining the double value of an element to avoid
        passing its NAN around.
        @ATTENTION: MUST NOT be used if the element is a string!
        @returns 0 if no error, else one of err... constants */
    USHORT GetError( SCSIZE nC, SCSIZE nR) const;
    USHORT GetError( SCSIZE nIndex) const
        { return pMat[nIndex].GetError(); }

    /** Use in ScInterpreter to obtain the error code, if any.
        @returns 0 if no error or string element, else one of err... constants */
    USHORT GetErrorIfNotString( SCSIZE nC, SCSIZE nR) const
        { return IsValue( nC, nR) ? GetError( nC, nR) : 0; }
    USHORT GetErrorIfNotString( SCSIZE nIndex) const
        { return IsValue( nIndex) ? GetError( nIndex) : 0; }

    /// @return 0.0 if empty
    double GetDouble( SCSIZE nC, SCSIZE nR) const;
    /// @return 0.0 if empty
    double GetDouble( SCSIZE nIndex) const
    {
        if ( pErrorInterpreter )
        {
            USHORT nError = GetDoubleErrorValue( pMat[nIndex].fVal);
            if ( nError )
                SetErrorAtInterpreter( nError);
        }
        return pMat[nIndex].fVal;
    }
    const String& GetString( SCSIZE nC, SCSIZE nR) const;
    const String& GetString( SCSIZE nIndex) const
        { return pMat[nIndex].GetString(); }

    String GetString( SvNumberFormatter& rFormatter, SCSIZE nIndex) const;
    String GetString( SvNumberFormatter& rFormatter, SCSIZE nC, SCSIZE nR) const;

    /// @ATTENTION: If bString the ScMatrixValue->pS may still be NULL to indicate
    /// an empty string!
    const ScMatrixValue* Get( SCSIZE nC, SCSIZE nR, ScMatValType& nType) const;

    /// @return <TRUE/> if string or empty
    BOOL IsString( SCSIZE nIndex ) const
        { return mnValType && IsStringType( mnValType[nIndex]); }
    /// @return <TRUE/> if string or empty
    BOOL IsString( SCSIZE nC, SCSIZE nR ) const
        { return mnValType && IsStringType( mnValType[ nC * nRowCount + nR ]); }
    BOOL IsEmpty( SCSIZE nIndex ) const
        { return mnValType && ((mnValType[nIndex] & SC_MATVAL_EMPTY) == SC_MATVAL_EMPTY); }
    BOOL IsEmptyPath( SCSIZE nC, SCSIZE nR ) const
        { return mnValType && ((mnValType[ nC * nRowCount + nR ] & SC_MATVAL_EMPTYPATH) == SC_MATVAL_EMPTYPATH); }
    BOOL IsEmptyPath( SCSIZE nIndex ) const
        { return mnValType && ((mnValType[nIndex] & SC_MATVAL_EMPTYPATH) == SC_MATVAL_EMPTYPATH); }
    BOOL IsEmpty( SCSIZE nC, SCSIZE nR ) const
        { return mnValType && ((mnValType[ nC * nRowCount + nR ] & SC_MATVAL_EMPTY) == SC_MATVAL_EMPTY); }
    BOOL IsValue( SCSIZE nIndex ) const
        { return !mnValType || IsValueType( mnValType[nIndex]); }
    BOOL IsValue( SCSIZE nC, SCSIZE nR ) const
        { return !mnValType || IsValueType( mnValType[ nC * nRowCount + nR ]); }
    BOOL IsValueOrEmpty( SCSIZE nIndex ) const
        { return !mnValType || IsValueType( mnValType[nIndex] ) ||
            ((mnValType[nIndex] & SC_MATVAL_EMPTY) == SC_MATVAL_EMPTY); }
    BOOL IsValueOrEmpty( SCSIZE nC, SCSIZE nR ) const
        { return !mnValType || IsValueType( mnValType[ nC * nRowCount + nR ]) ||
            ((mnValType[ nC * nRowCount + nR ] & SC_MATVAL_EMPTY) ==
             SC_MATVAL_EMPTY); }
    BOOL IsBoolean( SCSIZE nIndex ) const
        { return mnValType && IsBooleanType( mnValType[nIndex]); }
    BOOL IsBoolean( SCSIZE nC, SCSIZE nR ) const
        { return mnValType && IsBooleanType( mnValType[ nC * nRowCount + nR ]); }

    /// @return <TRUE/> if entire matrix is numeric, including booleans, with no strings or empties
    BOOL IsNumeric() const
        { return 0 == mnNonValue; }

    void MatTrans( ScMatrix& mRes) const;
    void MatCopy ( ScMatrix& mRes) const;
    /** Copy upper left of this matrix to mRes matrix.
        This matrix's dimensions must be greater than the mRes matrix
        dimensions. */
    void MatCopyUpperLeft( ScMatrix& mRes) const;

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


typedef ScSimpleIntrusiveReference< class ScMatrix > ScMatrixRef;
typedef ScSimpleIntrusiveReference< const class ScMatrix > ScConstMatrixRef;


// Old values as used up to SO52.
// The overall elements count had to be <= SC_MAX_MAT_DIM * SC_MAX_MAT_DIM.
// Don't use except maybe for file format compatibility.
// In any other case use ScMatrix::GetElementsMax() instead.
#ifdef WIN
#define SC_OLD_MAX_MAT_DIM            64
#else
#define SC_OLD_MAX_MAT_DIM           128
#endif
#define SC_OLD_MAX_MAT_ELEMENTS     ((SC_OLD_MAX_MAT_DIM) * (SC_OLD_MAX_MAT_DIM))


#endif

