/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: scmatrix.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 15:22:58 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#include <tools/debug.hxx>
#include <math.h>

#include "scmatrix.hxx"
#include "global.hxx"
#include "address.hxx"
#include "errorcodes.hxx"
#include "interpre.hxx"

#ifndef _ZFORLIST_HXX
#include <svtools/zforlist.hxx>
#endif

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

//------------------------------------------------------------------------

void ScMatrix::CreateMatrix(SCSIZE nC, SCSIZE nR)       // nur fuer ctor
{
    pErrorInterpreter = NULL;
    nColCount = nC;
    nRowCount = nR;
    SCSIZE nCount = nColCount * nRowCount;
    if ( !nCount || nCount > GetElementsMax() )
    {
        DBG_ERRORFILE("ScMatrix::CreateMatrix: dimension error");
        nColCount = nRowCount = 1;
        pMat = new ScMatrixValue[1];
        pMat[0].fVal = CreateDoubleError( errStackOverflow);
    }
    else
        pMat = new ScMatrixValue[nCount];
    mnValType = NULL;
    mnNonValue = 0;
}

ScMatrix::~ScMatrix()
{
    DeleteIsString();
    delete [] pMat;
}

ScMatrix* ScMatrix::Clone() const
{
    ScMatrix* pScMat = new ScMatrix( nColCount, nRowCount);
    MatCopy(*pScMat);
    pScMat->SetErrorInterpreter( pErrorInterpreter);    // TODO: really?
    return pScMat;
}

void ScMatrix::SetErrorAtInterpreter( USHORT nError ) const
{
    if ( pErrorInterpreter )
        pErrorInterpreter->SetError( nError);
}

//
//  File format: USHORT columns, USHORT rows, (columns*rows) entries:
//  BYTE type ( CELLTYPE_NONE, CELLTYPE_VALUE, CELLTYPE_STRING ); nothing, double or String
//

ScMatrix::ScMatrix(SvStream& /* rStream */)
        : pErrorInterpreter( NULL)
        , nRefCnt(0)
{
#if SC_ROWLIMIT_STREAM_ACCESS
#error address types changed!
    USHORT nC;
    USHORT nR;

    rStream >> nC;
    rStream >> nR;

    CreateMatrix(nC, nR);
    DBG_ASSERT( pMat, "pMat == NULL" );

    String aMatStr;
    double fVal;
    rtl_TextEncoding eCharSet = rStream.GetStreamCharSet();
    SCSIZE nCount = nColCount * nRowCount;
    SCSIZE nReadCount = (SCSIZE) nC * nR;
    for (SCSIZE i=0; i<nReadCount; i++)
    {
        BYTE nType;
        rStream >> nType;
        if ( nType == CELLTYPE_VALUE )
        {
            if ( i < nCount )
                rStream >> pMat[i].fVal;
            else
                rStream >> fVal;
        }
        else
        {
            // For unknown types read and forget string (upwards compatibility)

            if ( nType != CELLTYPE_NONE )
                rStream.ReadByteString( aMatStr, eCharSet );

            if ( i < nCount )
            {
                if (!mnValType)
                    ResetIsString();        // init string flags
                mnValType[i] = ( nType == CELLTYPE_NONE ? SC_MATVAL_EMPTY : SC_MATVAL_STRING );
                mnNonValue++;

                if ( nType == CELLTYPE_STRING )
                    pMat[i].pS = new String(aMatStr);
                else
                    pMat[i].pS = NULL;
            }
        }
    }
#else
    CreateMatrix(0,0);
#endif // SC_ROWLIMIT_STREAM_ACCESS
}

void ScMatrix::Store(SvStream& /* rStream */) const
{
#if SC_ROWLIMIT_STREAM_ACCESS
#error address types changed!
    SCSIZE nCount = nColCount * nRowCount;
    // Don't store matrix with more than USHORT max elements, old versions
    // might get confused in loops for(USHORT i=0; i<nC*nR; i++)
    if ( !pMat || nCount > ((USHORT)(~0)) )
    {
        DBG_ASSERT( pMat, "ScMatrix::Store: pMat == NULL" );
        // We can't store a 0 dimension because old versions rely on some
        // matrix being present, e.g. DDE link results, and old versions didn't
        // create a matrix if dimension was 0. Store an error result.
        rStream << (USHORT) 1;
        rStream << (USHORT) 1;
        rStream << (BYTE) CELLTYPE_VALUE;
        double fVal;
        ::rtl::math::setNan( &fVal );
        rStream << fVal;
        return;
    }

    rStream << (USHORT) nColCount;
#if SC_ROWLIMIT_MORE_THAN_32K
    #error row32k
#endif
    rStream << (USHORT) nRowCount;

    String aMatStr;
    rtl_TextEncoding eCharSet = rStream.GetStreamCharSet();
    for (SCSIZE i=0; i<nCount; i++)
    {
        BYTE nType = CELLTYPE_VALUE;
        if ( mnValType && IsStringType( mnValType[i]))
        {
            if ( pMat[i].pS )
                aMatStr = *pMat[i].pS;
            else
                aMatStr.Erase();

            if ( mnValType[i] == SC_MATVAL_STRING )
                nType = CELLTYPE_STRING;
            else
                nType = CELLTYPE_NONE;
        }
        rStream << nType;
        if ( nType == CELLTYPE_VALUE )
            rStream << pMat[i].fVal;
        else if ( nType == CELLTYPE_STRING )
            rStream.WriteByteString( aMatStr, eCharSet );
    }
#endif // SC_ROWLIMIT_STREAM_ACCESS
}

void ScMatrix::ResetIsString()
{
    SCSIZE nCount = nColCount * nRowCount;
    if (mnValType)
    {
        for (SCSIZE i = 0; i < nCount; i++)
        {
            if ( IsStringType( mnValType[i]))
                delete pMat[i].pS;
        }
    }
    else
        mnValType = new BYTE[nCount];
    memset( mnValType, 0, nCount * sizeof( BYTE ) );
    mnNonValue = 0;
}

void ScMatrix::DeleteIsString()
{
    if ( mnValType )
    {
        SCSIZE nCount = nColCount * nRowCount;
        for ( SCSIZE i = 0; i < nCount; i++ )
        {
            if (IsStringType( mnValType[i]))
                delete pMat[i].pS;
        }
        delete [] mnValType;
        mnValType = NULL;
        mnNonValue = 0;
    }
}

void ScMatrix::PutDouble(double fVal, SCSIZE nC, SCSIZE nR)
{
    if (ValidColRow( nC, nR))
        PutDouble( fVal, CalcOffset( nC, nR) );
    else
    {
        DBG_ERRORFILE("ScMatrix::PutDouble: dimension error");
    }
}

void ScMatrix::PutDoubleAndResetString( double fVal, SCSIZE nC, SCSIZE nR )
{
    if (ValidColRow( nC, nR))
        PutDoubleAndResetString( fVal, CalcOffset( nC, nR) );
    else
    {
        DBG_ERRORFILE("ScMatrix::PutDoubleAndResetString: dimension error");
    }
}

void ScMatrix::PutDoubleAndResetString( double fVal, SCSIZE nIndex )
{
    if ( IsString( nIndex ) )
    {
        delete pMat[nIndex].pS;
        mnValType[nIndex] = 0;
        mnNonValue--;
    }
    PutDouble( fVal, nIndex );
}

void ScMatrix::PutString(const String& rStr, SCSIZE nC, SCSIZE nR)
{
    if (ValidColRow( nC, nR))
        PutString( rStr, CalcOffset( nC, nR) );
    else
    {
        DBG_ERRORFILE("ScMatrix::PutString: dimension error");
    }
}

void ScMatrix::PutString(const String& rStr, SCSIZE nIndex)
{
    if (mnValType == NULL)
        ResetIsString();
    if ( IsStringType( mnValType[nIndex]) && pMat[nIndex].pS )
        *(pMat[nIndex].pS) = rStr;
    else
    {
        pMat[nIndex].pS = new String(rStr);
        mnNonValue++;
    }
    mnValType[nIndex] = SC_MATVAL_STRING;
}

void ScMatrix::PutStringEntry( const String* pStr, BYTE bFlag, SCSIZE nIndex )
{
    DBG_ASSERT( bFlag, "ScMatrix::PutStringEntry: bFlag == 0" );
    if (mnValType == NULL)
        ResetIsString();
    // Make sure all bytes of the union are initialized to be able to access
    // the value with if (IsValueOrEmpty()) GetDouble(). Backup pS first.
    String* pS = pMat[nIndex].pS;
    pMat[nIndex].fVal = 0.0;
    // An EMPTY entry must not have a string pointer therefor.
    DBG_ASSERT( (((bFlag & SC_MATVAL_EMPTY) == SC_MATVAL_EMPTY) && !pStr) || TRUE,
            "ScMatrix::PutStringEntry: pStr passed though EMPTY entry");
    if ( IsStringType( mnValType[nIndex]) && pS )
    {
        if ((bFlag & SC_MATVAL_EMPTY) == SC_MATVAL_EMPTY)
            delete pS, pS = NULL;
        if ( pStr )
            *pS = *pStr;
        else if (pS)
            pS->Erase();
        pMat[nIndex].pS = pS;
    }
    else
    {
        pMat[nIndex].pS = (pStr ? new String(*pStr) : NULL);
        mnNonValue++;
    }
    mnValType[nIndex] = bFlag;
}

void ScMatrix::PutEmpty(SCSIZE nC, SCSIZE nR)
{
    if (ValidColRow( nC, nR))
        PutEmpty( CalcOffset( nC, nR) );
    else
    {
        DBG_ERRORFILE("ScMatrix::PutEmpty: dimension error");
    }
}

void ScMatrix::PutEmpty(SCSIZE nIndex)
{
    if (mnValType == NULL)
        ResetIsString();
    if ( IsStringType( mnValType[nIndex]) && pMat[nIndex].pS )
    {
        delete pMat[nIndex].pS;
    }
    else
    {
        mnNonValue++;
    }
    mnValType[nIndex] = SC_MATVAL_EMPTY;
    pMat[nIndex].pS = NULL;
    pMat[nIndex].fVal = 0.0;
}

void ScMatrix::PutEmptyPath(SCSIZE nC, SCSIZE nR)
{
    if (ValidColRow( nC, nR))
        PutEmptyPath( CalcOffset( nC, nR) );
    else
    {
        DBG_ERRORFILE("ScMatrix::PutEmptyPath: dimension error");
    }
}

void ScMatrix::PutEmptyPath(SCSIZE nIndex)
{
    if (mnValType == NULL)
        ResetIsString();
    if ( IsStringType( mnValType[nIndex]) && pMat[nIndex].pS )
    {
        delete pMat[nIndex].pS;
    }
    else
    {
        mnNonValue++;
    }
    mnValType[nIndex] = SC_MATVAL_EMPTYPATH;
    pMat[nIndex].pS = NULL;
    pMat[nIndex].fVal = 0.0;
}

void ScMatrix::PutBoolean(bool bVal, SCSIZE nC, SCSIZE nR)
{
    if (ValidColRow( nC, nR))
        PutBoolean( bVal, CalcOffset( nC, nR) );
    else
    {
        DBG_ERRORFILE("ScMatrix::PutBoolean: dimension error");
    }
}

void ScMatrix::PutBoolean( bool bVal, SCSIZE nIndex)
{
    if (mnValType == NULL)
        ResetIsString();
    if ( IsStringType( mnValType[nIndex]) && pMat[nIndex].pS )
    {
        delete pMat[nIndex].pS;
        mnNonValue--;
    }

    mnValType[nIndex] = SC_MATVAL_BOOLEAN;
    pMat[nIndex].pS = NULL;
    pMat[nIndex].fVal = bVal ? 1. : 0.;
}

USHORT ScMatrix::GetError( SCSIZE nC, SCSIZE nR) const
{
    if (ValidColRow( nC, nR))
        return GetError( CalcOffset( nC, nR) );
    else
    {
        DBG_ERRORFILE("ScMatrix::GetError: dimension error");
        return 0;   // TODO: do we want an error instead?
    }
}

double ScMatrix::GetDouble(SCSIZE nC, SCSIZE nR) const
{
    if (ValidColRow( nC, nR))
        return GetDouble( CalcOffset( nC, nR) );
    else
    {
        DBG_ERRORFILE("ScMatrix::GetDouble: dimension error");
        return 0.0;
    }
}

const String& ScMatrix::GetString(SCSIZE nC, SCSIZE nR) const
{
    if (ValidColRow( nC, nR))
    {
        SCSIZE nIndex = CalcOffset( nC, nR);
        if ( IsString( nIndex ) )
            return GetString( nIndex );
        else
        {
            SetErrorAtInterpreter( GetError( nIndex));
            DBG_ERRORFILE("ScMatrix::GetString: access error, no string");
        }
    }
    else
    {
        DBG_ERRORFILE("ScMatrix::GetString: dimension error");
    }
    return ScGlobal::GetEmptyString();
}


String ScMatrix::GetString( SvNumberFormatter& rFormatter, SCSIZE nIndex) const
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


String ScMatrix::GetString( SvNumberFormatter& rFormatter, SCSIZE nC, SCSIZE nR) const
{
    if (ValidColRow( nC, nR))
    {
        SCSIZE nIndex = CalcOffset( nC, nR);
        return GetString( rFormatter, nIndex);
    }
    else
    {
        DBG_ERRORFILE("ScMatrix::GetString: dimension error");
    }
    return String();
}


const ScMatrixValue* ScMatrix::Get(SCSIZE nC, SCSIZE nR, ScMatValType& nType) const
{
    if (ValidColRow( nC, nR))
    {
        SCSIZE nIndex = CalcOffset( nC, nR);
        if (mnValType)
            nType = mnValType[nIndex];
        else
            nType = SC_MATVAL_VALUE;
        return &pMat[nIndex];
    }
    else
    {
        DBG_ERRORFILE("ScMatrix::Get: dimension error");
    }
    nType = SC_MATVAL_EMPTY;
    return NULL;
}

void ScMatrix::MatCopy(ScMatrix& mRes) const
{
    if (nColCount != mRes.nColCount || nRowCount != mRes.nRowCount)
    {
        DBG_ERRORFILE("ScMatrix::MatCopy: dimension error");
    }
    else
    {
        if (mnValType)
        {
            ScMatValType nType;
            mRes.ResetIsString();
            for (SCSIZE i = 0; i < nColCount; i++)
            {
                SCSIZE nStart = i * nRowCount;
                for (SCSIZE j = 0; j < nRowCount; j++)
                {
                    if (IsStringType( (nType = mnValType[nStart+j])))
                        mRes.PutStringEntry( pMat[nStart+j].pS, nType, nStart+j );
                    else
                    {
                        mRes.pMat[nStart+j].fVal = pMat[nStart+j].fVal;
                        mRes.mnValType[nStart+j] = nType;
                    }
                }
            }
        }
        else
        {
            mRes.DeleteIsString();
            SCSIZE nCount = nColCount * nRowCount;
            for (SCSIZE i = 0; i < nCount; i++)
                mRes.pMat[i].fVal = pMat[i].fVal;
        }
    }
}

void ScMatrix::MatTrans(ScMatrix& mRes) const
{
    if (nColCount != mRes.nRowCount || nRowCount != mRes.nColCount)
    {
        DBG_ERRORFILE("ScMatrix::MatTrans: dimension error");
    }
    else
    {
        if (mnValType)
        {
            ScMatValType nType;
            mRes.ResetIsString();
            for ( SCSIZE i = 0; i < nColCount; i++ )
            {
                SCSIZE nStart = i * nRowCount;
                for ( SCSIZE j = 0; j < nRowCount; j++ )
                {
                    if (IsStringType( (nType = mnValType[nStart+j])))
                        mRes.PutStringEntry( pMat[nStart+j].pS, nType, j*mRes.nRowCount+i );
                    else
                    {
                        mRes.pMat[j*mRes.nRowCount+i].fVal = pMat[nStart+j].fVal;
                        mRes.mnValType[j*mRes.nRowCount+i] = nType;
                    }
                }
            }
        }
        else
        {
            mRes.DeleteIsString();
            for ( SCSIZE i = 0; i < nColCount; i++ )
            {
                SCSIZE nStart = i * nRowCount;
                for ( SCSIZE j = 0; j < nRowCount; j++ )
                {
                    mRes.pMat[j*mRes.nRowCount+i].fVal = pMat[nStart+j].fVal;
                }
            }
        }
    }
}

void ScMatrix::MatCopyUpperLeft(ScMatrix& mRes) const
{
    if (nColCount < mRes.nColCount || nRowCount < mRes.nRowCount)
    {
        DBG_ERRORFILE("ScMatrix::MatCopyUpperLeft: dimension error");
    }
    else
    {
        if (mnValType)
        {
            ScMatValType nType;
            mRes.ResetIsString();
            for ( SCSIZE i = 0; i < mRes.nColCount; i++ )
            {
                SCSIZE nStart = i * nRowCount;
                for ( SCSIZE j = 0; j < mRes.nRowCount; j++ )
                {
                    if ( IsStringType( (nType = mnValType[nStart+j]) ))
                        mRes.PutStringEntry( pMat[nStart+j].pS, nType,
                            i*mRes.nRowCount+j );
                    else
                    {
                        mRes.pMat[i*mRes.nRowCount+j].fVal = pMat[nStart+j].fVal;
                        mRes.mnValType[i*mRes.nRowCount+j] = nType;
                    }
                }
            }
        }
        else
        {
            mRes.DeleteIsString();
            for ( SCSIZE i = 0; i < mRes.nColCount; i++ )
            {
                SCSIZE nStart = i * nRowCount;
                for ( SCSIZE j = 0; j < mRes.nRowCount; j++ )
                {
                    mRes.pMat[i*mRes.nRowCount+j].fVal = pMat[nStart+j].fVal;
                }
            }
        }
    }
}

void ScMatrix::FillDouble( double fVal, SCSIZE nC1, SCSIZE nR1, SCSIZE nC2, SCSIZE nR2 )
{
    if (ValidColRow( nC1, nR1) && ValidColRow( nC2, nR2))
    {
        if ( nC1 == 0 && nR1 == 0 && nC2 == nColCount-1 && nR2 == nRowCount-1 )
        {
            SCSIZE nEnd = nColCount * nRowCount;
            for ( SCSIZE j=0; j<nEnd; j++ )
                pMat[j].fVal = fVal;
        }
        else
        {
            for ( SCSIZE i=nC1; i<=nC2; i++ )
            {
                SCSIZE nOff1 = i * nRowCount + nR1;
                SCSIZE nOff2 = nOff1 + nR2 - nR1;
                for ( SCSIZE j=nOff1; j<=nOff2; j++ )
                    pMat[j].fVal = fVal;
            }
        }
    }
    else
    {
        DBG_ERRORFILE("ScMatrix::FillDouble: dimension error");
    }
}

void ScMatrix::FillDoubleLowerLeft( double fVal, SCSIZE nC2 )
{
    if (ValidColRow( nC2, nC2))
    {
        for ( SCSIZE i=1; i<=nC2; i++ )
        {
            SCSIZE nOff1 = i * nRowCount;
            SCSIZE nOff2 = nOff1 + i;
            for ( SCSIZE j=nOff1; j<nOff2; j++ )
                pMat[j].fVal = fVal;
        }
    }
    else
    {
        DBG_ERRORFILE("ScMatrix::FillDoubleLowerLeft: dimension error");
    }
}

void ScMatrix::CompareEqual()
{
    SCSIZE n = nColCount * nRowCount;
    if ( mnValType )
    {
        for ( SCSIZE j=0; j<n; j++ )
            if ( IsValueType( mnValType[j]) )               // else: #WERT!
                if ( ::rtl::math::isFinite( pMat[j].fVal))  // else: DoubleError
                    pMat[j].fVal = (pMat[j].fVal == 0.0);
    }
    else
    {
        for ( SCSIZE j=0; j<n; j++ )
            if ( ::rtl::math::isFinite( pMat[j].fVal))  // else: DoubleError
                pMat[j].fVal = (pMat[j].fVal == 0.0);
    }
}

void ScMatrix::CompareNotEqual()
{
    SCSIZE n = nColCount * nRowCount;
    if ( mnValType )
    {
        for ( SCSIZE j=0; j<n; j++ )
            if ( IsValueType( mnValType[j]) )               // else: #WERT!
                if ( ::rtl::math::isFinite( pMat[j].fVal))  // else: DoubleError
                    pMat[j].fVal = (pMat[j].fVal != 0.0);
    }
    else
    {
        for ( SCSIZE j=0; j<n; j++ )
            if ( ::rtl::math::isFinite( pMat[j].fVal))  // else: DoubleError
                pMat[j].fVal = (pMat[j].fVal != 0.0);
    }
}

void ScMatrix::CompareLess()
{
    SCSIZE n = nColCount * nRowCount;
    if ( mnValType )
    {
        for ( SCSIZE j=0; j<n; j++ )
            if ( IsValueType( mnValType[j]) )               // else: #WERT!
                if ( ::rtl::math::isFinite( pMat[j].fVal))  // else: DoubleError
                    pMat[j].fVal = (pMat[j].fVal < 0.0);
    }
    else
    {
        for ( SCSIZE j=0; j<n; j++ )
            if ( ::rtl::math::isFinite( pMat[j].fVal))  // else: DoubleError
                pMat[j].fVal = (pMat[j].fVal < 0.0);
    }
}

void ScMatrix::CompareGreater()
{
    SCSIZE n = nColCount * nRowCount;
    if ( mnValType )
    {
        for ( SCSIZE j=0; j<n; j++ )
            if ( IsValueType( mnValType[j]) )               // else: #WERT!
                if ( ::rtl::math::isFinite( pMat[j].fVal))  // else: DoubleError
                    pMat[j].fVal = (pMat[j].fVal > 0.0);
    }
    else
    {
        for ( SCSIZE j=0; j<n; j++ )
            if ( ::rtl::math::isFinite( pMat[j].fVal))  // else: DoubleError
                pMat[j].fVal = (pMat[j].fVal > 0.0);
    }
}

void ScMatrix::CompareLessEqual()
{
    SCSIZE n = nColCount * nRowCount;
    if ( mnValType )
    {
        for ( SCSIZE j=0; j<n; j++ )
            if ( IsValueType( mnValType[j]) )               // else: #WERT!
                if ( ::rtl::math::isFinite( pMat[j].fVal))  // else: DoubleError
                    pMat[j].fVal = (pMat[j].fVal <= 0.0);
    }
    else
    {
        for ( SCSIZE j=0; j<n; j++ )
            if ( ::rtl::math::isFinite( pMat[j].fVal))  // else: DoubleError
                pMat[j].fVal = (pMat[j].fVal <= 0.0);
    }
}

void ScMatrix::CompareGreaterEqual()
{
    SCSIZE n = nColCount * nRowCount;
    if ( mnValType )
    {
        for ( SCSIZE j=0; j<n; j++ )
            if ( IsValueType( mnValType[j]) )               // else: #WERT!
                if ( ::rtl::math::isFinite( pMat[j].fVal))  // else: DoubleError
                    pMat[j].fVal = (pMat[j].fVal >= 0.0);
    }
    else
    {
        for ( SCSIZE j=0; j<n; j++ )
            if ( ::rtl::math::isFinite( pMat[j].fVal))  // else: DoubleError
                pMat[j].fVal = (pMat[j].fVal >= 0.0);
    }
}

double ScMatrix::And()
{
    SCSIZE n = nColCount * nRowCount;
    bool bAnd = true;
    if ( mnValType )
    {
        for ( SCSIZE j=0; bAnd && j<n; j++ )
        {
            if ( !IsValueType( mnValType[j]) )
            {   // assuming a CompareMat this is an error
                return CreateDoubleError( errIllegalArgument );
            }
            else if ( ::rtl::math::isFinite( pMat[j].fVal))
                bAnd = (pMat[j].fVal != 0.0);
            else
                return pMat[j].fVal;    // DoubleError
        }
    }
    else
    {
        for ( SCSIZE j=0; bAnd && j<n; j++ )
        {
            if ( ::rtl::math::isFinite( pMat[j].fVal))
                bAnd = (pMat[j].fVal != 0.0);
            else
                return pMat[j].fVal;    // DoubleError
        }
    }
    return bAnd;
}

double ScMatrix::Or()
{
    SCSIZE n = nColCount * nRowCount;
    bool bOr = false;
    if ( mnValType )
    {
        for ( SCSIZE j=0; !bOr && j<n; j++ )
            if ( !IsValueType( mnValType[j]) )
            {   // assuming a CompareMat this is an error
                return CreateDoubleError( errIllegalArgument );
            }
            else if ( ::rtl::math::isFinite( pMat[j].fVal))
                bOr = (pMat[j].fVal != 0.0);
            else
                return pMat[j].fVal;    // DoubleError
    }
    else
    {
        for ( SCSIZE j=0; !bOr && j<n; j++ )
            if ( ::rtl::math::isFinite( pMat[j].fVal))
                bOr = (pMat[j].fVal != 0.0);
            else
                return pMat[j].fVal;    // DoubleError
    }
    return bOr;
}

