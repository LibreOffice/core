/*************************************************************************
 *
 *  $RCSfile: scmatrix.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 10:15:10 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

const BYTE SC_MATVAL_STRING    = 0x01;
const BYTE SC_MATVAL_EMPTY     = SC_MATVAL_STRING | 0x02; // STRING plus flag
const BYTE SC_MATVAL_EMPTYPATH = SC_MATVAL_EMPTY | 0x04;  // EMPTY plus flag

union MatValue
{
    double fVal;
    String* pS;

    /// Only valid if ScMatrix methods indicate so!
    const String& GetString() const     { return pS ? *pS : EMPTY_STRING; }

    /// Only valid if ScMatrix methods indicate that this is no string!
    USHORT GetError() const         { return GetDoubleErrorValue( fVal); }
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

    PutDouble() does not reset an eventual string! Use
    PutDoubleAndResetString() if that is wanted. Also the FillDouble...()
    methods don't reset strings. As a consequence memory leaks may occur if
    used wrong.
 */
class ScMatrix
{
    MatValue*       pMat;
    BYTE*           bIsString;
    ScInterpreter*  pErrorInterpreter;
    ULONG           nRefCnt;     // reference count
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
        const size_t nMemMax = (((size_t)(~0))-64) / sizeof(MatValue);
        const size_t nArbitraryLimit = 0x80000;  // 512k elements ~= 4MB memory
        return nMemMax < nArbitraryLimit ? nMemMax : nArbitraryLimit;
    }

    /** If nC*nR results in more than GetElementsMax() entries, a 1x1 matrix is
        created instead and a double error value (errStackOverflow) is set.
        Compare nC and nR with a GetDimensions() call to check. */
    ScMatrix( SCSIZE nC, SCSIZE nR) : nRefCnt(0) { CreateMatrix( nC, nR); }
    ScMatrix* Clone() const;

    /// disable refcounting forever, may only be deleted via Delete() afterwards
    inline  void    SetEternalRef()         { nRefCnt = ULONG_MAX; }
    inline  bool    IsEternalRef() const    { return nRefCnt == ULONG_MAX; }
    inline  void    IncRef()
    {
        if ( !IsEternalRef() )
            ++nRefCnt;
    }
    inline  void    DecRef()
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
    void FillDouble( double fVal,
            SCSIZE nC1, SCSIZE nR1, SCSIZE nC2, SCSIZE nR2 );
    /// lower left triangle
    void FillDoubleLowerLeft( double fVal, SCSIZE nC2 );

    /** Use outside ScInterpreter before obtaining the double value of an
        element and passing it's NAN around.
        @returns 0 if no error, else one of err... constants */
    USHORT GetError( SCSIZE nC, SCSIZE nR) const;
    USHORT GetError( SCSIZE nIndex) const
        { return GetDoubleErrorValue( pMat[nIndex].fVal); }

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

    /// @ATTENTION: If bString the MatValue->pS may still be NULL to indicate
    /// an empty string!
    const MatValue* Get( SCSIZE nC, SCSIZE nR, BOOL& bString) const;

    /// @return <TRUE/> if string or empty
    BOOL IsString( SCSIZE nIndex ) const
        { return bIsString && bIsString[nIndex]; }
    /// @return <TRUE/> if string or empty
    BOOL IsString( SCSIZE nC, SCSIZE nR ) const
        { return bIsString && bIsString[ nC * nRowCount + nR ]; }
    BOOL IsEmpty( SCSIZE nIndex ) const
        { return bIsString && ((bIsString[nIndex] & SC_MATVAL_EMPTY) ==
                SC_MATVAL_EMPTY); }
    BOOL IsEmptyPath( SCSIZE nC, SCSIZE nR ) const
        { return bIsString && ((bIsString[ nC * nRowCount + nR ] &
                    SC_MATVAL_EMPTYPATH) == SC_MATVAL_EMPTYPATH); }
    BOOL IsEmptyPath( SCSIZE nIndex ) const
        { return bIsString && ((bIsString[nIndex] & SC_MATVAL_EMPTYPATH) ==
                SC_MATVAL_EMPTYPATH); }
    BOOL IsEmpty( SCSIZE nC, SCSIZE nR ) const
        { return bIsString && ((bIsString[ nC * nRowCount + nR ] &
                    SC_MATVAL_EMPTY) == SC_MATVAL_EMPTY); }
    BOOL IsValue( SCSIZE nIndex ) const
        { return !bIsString || !bIsString[nIndex]; }
    BOOL IsValue( SCSIZE nC, SCSIZE nR ) const
        { return !bIsString || !bIsString[ nC * nRowCount + nR ]; }
    BOOL IsValueOrEmpty( SCSIZE nIndex ) const
        { return !bIsString || !bIsString[nIndex] || ((bIsString[nIndex] &
                    SC_MATVAL_EMPTY) == SC_MATVAL_EMPTY); }
    BOOL IsValueOrEmpty( SCSIZE nC, SCSIZE nR ) const
        { return !bIsString || !bIsString[ nC * nRowCount + nR ] ||
            ((bIsString[ nC * nRowCount + nR ] & SC_MATVAL_EMPTY) ==
             SC_MATVAL_EMPTY); }

    /// @return <TRUE/> if entire matrix is numeric and no strings are contained
    BOOL IsNumeric() const
        { return bIsString == NULL; }

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

