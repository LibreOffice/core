/*************************************************************************
 *
 *  $RCSfile: scmatrix.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 10:39:39 $
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

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------------

#include <tools/debug.hxx>
#include <math.h>

#include "scmatrix.hxx"
#include "global.hxx"
#include "address.hxx"
#include "errorcodes.hxx"
#include "interpre.hxx"

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
        pMat = new MatValue[1];
        pMat[0].fVal = CreateDoubleError( errStackOverflow);
    }
    else
        pMat = new MatValue[nCount];
    bIsString = NULL;
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

ScMatrix::ScMatrix(SvStream& rStream)
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
                if (!bIsString)
                    ResetIsString();        // init string flags
                bIsString[i] = ( nType == CELLTYPE_NONE ? SC_MATVAL_EMPTY : SC_MATVAL_STRING );

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

void ScMatrix::Store(SvStream& rStream) const
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
        if ( bIsString && bIsString[i] )
        {
            if ( pMat[i].pS )
                aMatStr = *pMat[i].pS;
            else
                aMatStr.Erase();

            if ( bIsString[i] == SC_MATVAL_STRING )
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
    if (bIsString)
    {
        for (SCSIZE i = 0; i < nCount; i++)
        {
            if ( bIsString[i] )
                delete pMat[i].pS;
        }
    }
    else
        bIsString = new BYTE[nCount];
    memset( bIsString, 0, nCount * sizeof( BYTE ) );
}

void ScMatrix::DeleteIsString()
{
    if ( bIsString )
    {
        SCSIZE nCount = nColCount * nRowCount;
        for ( SCSIZE i = 0; i < nCount; i++ )
        {
            if ( bIsString[i] )
                delete pMat[i].pS;
        }
        delete [] bIsString;
        bIsString = NULL;
    }
}

void ScMatrix::PutDouble(double fVal, SCSIZE nC, SCSIZE nR)
{
    if (ValidColRow( nC, nR))
        PutDouble( fVal, CalcOffset( nC, nR) );
    else
        DBG_ERRORFILE("ScMatrix::PutDouble: dimension error");
}

void ScMatrix::PutDoubleAndResetString( double fVal, SCSIZE nC, SCSIZE nR )
{
    if (ValidColRow( nC, nR))
        PutDoubleAndResetString( fVal, CalcOffset( nC, nR) );
    else
        DBG_ERRORFILE("ScMatrix::PutDoubleAndResetString: dimension error");
}

void ScMatrix::PutDoubleAndResetString( double fVal, SCSIZE nIndex )
{
    if ( IsString( nIndex ) )
    {
        delete pMat[nIndex].pS;
        bIsString[nIndex] = 0;
    }
    PutDouble( fVal, nIndex );
}

void ScMatrix::PutString(const String& rStr, SCSIZE nC, SCSIZE nR)
{
    if (ValidColRow( nC, nR))
        PutString( rStr, CalcOffset( nC, nR) );
    else
        DBG_ERRORFILE("ScMatrix::PutString: dimension error");
}

void ScMatrix::PutString(const String& rStr, SCSIZE nIndex)
{
    if (bIsString == NULL)
        ResetIsString();
    if ( bIsString[nIndex] && pMat[nIndex].pS )
        *(pMat[nIndex].pS) = rStr;
    else
        pMat[nIndex].pS = new String(rStr);
    bIsString[nIndex] = SC_MATVAL_STRING;
}

void ScMatrix::PutStringEntry( const String* pStr, BYTE bFlag, SCSIZE nIndex )
{
    DBG_ASSERT( bFlag, "ScMatrix::PutStringEntry: bFlag == 0" );
    if (bIsString == NULL)
        ResetIsString();
    if ( bIsString[nIndex] && pMat[nIndex].pS )
    {
        if ( pStr )
            *(pMat[nIndex].pS) = *pStr;
        else
            pMat[nIndex].pS->Erase();
    }
    else
        pMat[nIndex].pS = (pStr ? new String(*pStr) : NULL);
    bIsString[nIndex] = bFlag;
}

void ScMatrix::PutEmpty(SCSIZE nC, SCSIZE nR)
{
    if (ValidColRow( nC, nR))
        PutEmpty( CalcOffset( nC, nR) );
    else
        DBG_ERRORFILE("ScMatrix::PutEmpty: dimension error");
}

void ScMatrix::PutEmpty(SCSIZE nIndex)
{
    if (bIsString == NULL)
        ResetIsString();
    if ( bIsString[nIndex] && pMat[nIndex].pS )
        delete pMat[nIndex].pS;
    bIsString[nIndex] = SC_MATVAL_EMPTY;
    pMat[nIndex].pS = NULL;
    pMat[nIndex].fVal = 0.0;
}

void ScMatrix::PutEmptyPath(SCSIZE nC, SCSIZE nR)
{
    if (ValidColRow( nC, nR))
        PutEmptyPath( CalcOffset( nC, nR) );
    else
        DBG_ERRORFILE("ScMatrix::PutEmptyPath: dimension error");
}

void ScMatrix::PutEmptyPath(SCSIZE nIndex)
{
    if (bIsString == NULL)
        ResetIsString();
    if ( bIsString[nIndex] && pMat[nIndex].pS )
        delete pMat[nIndex].pS;
    bIsString[nIndex] = SC_MATVAL_EMPTYPATH;
    pMat[nIndex].pS = NULL;
    pMat[nIndex].fVal = 0.0;
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
            DBG_ERRORFILE("ScMatrix::GetString: access error, no string");
    }
    else
        DBG_ERRORFILE("ScMatrix::GetString: dimension error");
    return ScGlobal::GetEmptyString();
}

const MatValue* ScMatrix::Get(SCSIZE nC, SCSIZE nR, BOOL& bString) const
{
    if (ValidColRow( nC, nR))
    {
        SCSIZE nIndex = CalcOffset( nC, nR);
        if (bIsString && bIsString[nIndex])
            bString = TRUE;
        else
            bString = FALSE;
        return &pMat[nIndex];
    }
    else
        DBG_ERRORFILE("ScMatrix::Get: dimension error");
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
        if (bIsString)
        {
            mRes.ResetIsString();
            for (SCSIZE i = 0; i < nColCount; i++)
            {
                SCSIZE nStart = i * nRowCount;
                for (SCSIZE j = 0; j < nRowCount; j++)
                {
                    if ( bIsString[nStart+j] )
                        mRes.PutStringEntry( pMat[nStart+j].pS,
                            bIsString[nStart+j], nStart+j );
                    else
                        mRes.pMat[nStart+j].fVal = pMat[nStart+j].fVal;
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
        if (bIsString)
        {
            mRes.ResetIsString();
            for ( SCSIZE i = 0; i < nColCount; i++ )
            {
                SCSIZE nStart = i * nRowCount;
                for ( SCSIZE j = 0; j < nRowCount; j++ )
                {
                    if ( bIsString[nStart+j] )
                        mRes.PutStringEntry( pMat[nStart+j].pS,
                            bIsString[nStart+j], j*mRes.nRowCount+i );
                    else
                        mRes.pMat[j*mRes.nRowCount+i].fVal = pMat[nStart+j].fVal;
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
        if (bIsString)
        {
            mRes.ResetIsString();
            for ( SCSIZE i = 0; i < mRes.nColCount; i++ )
            {
                SCSIZE nStart = i * nRowCount;
                for ( SCSIZE j = 0; j < mRes.nRowCount; j++ )
                {
                    if ( bIsString[nStart+j] )
                        mRes.PutStringEntry( pMat[nStart+j].pS, bIsString[nStart+j],
                            i*mRes.nRowCount+j );
                    else
                        mRes.pMat[i*mRes.nRowCount+j].fVal = pMat[nStart+j].fVal;
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
        DBG_ERRORFILE("ScMatrix::FillDouble: dimension error");
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
        DBG_ERRORFILE("ScMatrix::FillDoubleLowerLeft: dimension error");
}

void ScMatrix::CompareEqual()
{
    SCSIZE n = nColCount * nRowCount;
    if ( bIsString )
    {
        for ( SCSIZE j=0; j<n; j++ )
            if ( !bIsString[j])     // else: #WERT!
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
    if ( bIsString )
    {
        for ( SCSIZE j=0; j<n; j++ )
            if ( !bIsString[j])     // else: #WERT!
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
    if ( bIsString )
    {
        for ( SCSIZE j=0; j<n; j++ )
            if ( !bIsString[j])     // else: #WERT!
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
    if ( bIsString )
    {
        for ( SCSIZE j=0; j<n; j++ )
            if ( !bIsString[j])     // else: #WERT!
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
    if ( bIsString )
    {
        for ( SCSIZE j=0; j<n; j++ )
            if ( !bIsString[j])     // else: #WERT!
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
    if ( bIsString )
    {
        for ( SCSIZE j=0; j<n; j++ )
            if ( !bIsString[j])     // else: #WERT!
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
    if ( bIsString )
    {
        for ( SCSIZE j=0; bAnd && j<n; j++ )
        {
            if ( bIsString[j] )
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
    if ( bIsString )
    {
        for ( SCSIZE j=0; !bOr && j<n; j++ )
            if ( bIsString[j] )
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

