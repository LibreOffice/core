/*************************************************************************
 *
 *  $RCSfile: scmatrix.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: er $ $Date: 2001-02-28 14:29:23 $
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

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif

//------------------------------------------------------------------------

void ScMatrix::CreateMatrix(USHORT nC, USHORT nR)       // nur fuer ctor
{
    nAnzCol = nC;
    nAnzRow = nR;
    ULONG nCount = (ULONG) nAnzCol * nAnzRow;
    if ( !nCount || nCount > GetElementsMax() )
    {
        DBG_ERRORFILE("ScMatrix::CreateMatrix: dimension error");
        pMat = NULL;
        nAnzCol = nAnzRow = 0;
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
    ScMatrix* pScMat = new ScMatrix(nAnzCol, nAnzRow);
    MatCopy(*pScMat);
    return pScMat;
}

//
//  Dateiformat: USHORT Spalten, USHORT Zeilen, (Spalten*Zeilen) Eintraege:
//  BYTE Typ ( CELLTYPE_NONE, CELLTYPE_VALUE, CELLTYPE_STRING ); nichts, double oder String
//

ScMatrix::ScMatrix(SvStream& rStream)
{
    USHORT nC, nR;

    rStream >> nC;
    rStream >> nR;

    CreateMatrix(nC, nR);
    DBG_ASSERT( pMat, "pMat == NULL" );

    String aMatStr;
    double fVal;
    rtl_TextEncoding eCharSet = rStream.GetStreamCharSet();
    ULONG nCount = (ULONG) nC * nR;
    for (ULONG i=0; i<nCount; i++)
    {
        BYTE nType;
        rStream >> nType;
        if ( nType == CELLTYPE_VALUE )
        {
            if ( pMat )
                rStream >> pMat[i].fVal;
            else
                rStream >> fVal;
        }
        else
        {
            // For unknown types read and forget string (upwards compatibility)

            if ( nType != CELLTYPE_NONE )
                rStream.ReadByteString( aMatStr, eCharSet );

            if ( pMat )
            {
                if (!bIsString)
                    ResetIsString();                // String-Flags initialisieren
                bIsString[i] = ( CELLTYPE_NONE ? SC_MATVAL_EMPTY : SC_MATVAL_STRING );

                if ( nType == CELLTYPE_STRING )
                    pMat[i].pS = new String(aMatStr);
                else
                    pMat[i].pS = new String;        // leer
            }
        }
    }
}

void ScMatrix::Store(SvStream& rStream) const
{
    if (!pMat)
    {
        rStream << (USHORT) 0;
        rStream << (USHORT) 0;
        DBG_ERRORFILE("ScMatrix::Store: pMat == NULL");
        return;
    }

    rStream << nAnzCol;
    rStream << nAnzRow;

    String aMatStr;
    rtl_TextEncoding eCharSet = rStream.GetStreamCharSet();
    ULONG nCount = (ULONG) nAnzCol * nAnzRow;
    for (ULONG i=0; i<nCount; i++)
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
}

void ScMatrix::ResetIsString()
{
    ULONG nCount = (ULONG) nAnzCol * nAnzRow;
    if (bIsString)
    {
        for (ULONG i = 0; i < nCount; i++)
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
        ULONG nCount = (ULONG) nAnzCol * nAnzRow;
        for ( ULONG i = 0; i < nCount; i++ )
        {
            if ( bIsString[i] )
                delete pMat[i].pS;
        }
        delete [] bIsString;
        bIsString = NULL;
    }
}

void ScMatrix::PutDouble(double fVal, USHORT nC, USHORT nR)
{
    if (nC < nAnzCol && nR < nAnzRow)
        PutDouble( fVal, (ULONG) nC * nAnzRow + nR );
    else
        DBG_ERRORFILE("ScMatrix::PutDouble: dimension error");
}

void ScMatrix::PutDoubleAndResetString( double fVal, USHORT nC, USHORT nR )
{
    if (nC < nAnzCol && nR < nAnzRow)
        PutDoubleAndResetString( fVal, (ULONG) nC * nAnzRow + nR );
    else
        DBG_ERRORFILE("ScMatrix::PutDoubleAndResetString: dimension error");
}

void ScMatrix::PutDoubleAndResetString( double fVal, ULONG nIndex )
{
    if ( IsString( nIndex ) )
    {
        delete pMat[nIndex].pS;
        bIsString[nIndex] = 0;
    }
    PutDouble( fVal, nIndex );
}

void ScMatrix::PutString(const String& rStr, USHORT nC, USHORT nR)
{
    if (nC < nAnzCol && nR < nAnzRow)
        PutString( rStr, (ULONG) nC * nAnzRow + nR );
    else
        DBG_ERRORFILE("ScMatrix::PutString: dimension error");
}

void ScMatrix::PutString(const String& rStr, ULONG nIndex)
{
    if (bIsString == NULL)
        ResetIsString();
    if ( bIsString[nIndex] && pMat[nIndex].pS )
        *(pMat[nIndex].pS) = rStr;
    else
        pMat[nIndex].pS = new String(rStr);
    bIsString[nIndex] = SC_MATVAL_STRING;
}

void ScMatrix::PutStringEntry( const String* pStr, BYTE bFlag, ULONG nIndex )
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

void ScMatrix::PutEmpty(USHORT nC, USHORT nR)
{
    if (nC < nAnzCol && nR < nAnzRow)
        PutEmpty( (ULONG) nC * nAnzRow + nR );
    else
        DBG_ERRORFILE("ScMatrix::PutString: dimension error");
}

void ScMatrix::PutEmpty(ULONG nIndex)
{
    if (bIsString == NULL)
        ResetIsString();
    if ( bIsString[nIndex] && pMat[nIndex].pS )
        delete pMat[nIndex].pS;
    pMat[nIndex].pS = NULL;
    bIsString[nIndex] = SC_MATVAL_EMPTY;
}

double ScMatrix::GetDouble(USHORT nC, USHORT nR) const
{
    if (nC < nAnzCol && nR < nAnzRow)
        return GetDouble( (ULONG) nC * nAnzRow + nR );
    else
    {
        DBG_ERRORFILE("ScMatrix::GetDouble: dimension error");
        return 0.0;
    }
}

const String& ScMatrix::GetString(USHORT nC, USHORT nR) const
{
    if (nC < nAnzCol && nR < nAnzRow)
    {
        ULONG nIndex = (ULONG) nC * nAnzRow + nR;
        if ( IsString( nIndex ) )
            return GetString( nIndex );
        else
            DBG_ERRORFILE("ScMatrix::GetString: access error, no string");
    }
    else
        DBG_ERRORFILE("ScMatrix::GetString: dimension error");
    return ScGlobal::GetEmptyString();
}

const MatValue* ScMatrix::Get(USHORT nC, USHORT nR, BOOL& bString) const
{
    if (nC < nAnzCol && nR < nAnzRow)
    {
        ULONG nIndex = (ULONG) nC * nAnzRow + nR;
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
    if (nAnzCol != mRes.nAnzCol || nAnzRow != mRes.nAnzRow)
    {
        DBG_ERRORFILE("ScMatrix::MatCopy: dimension error");
    }
    else
    {
        if (bIsString)
        {
            mRes.ResetIsString();
            for (USHORT i = 0; i < nAnzCol; i++)
            {
                ULONG nStart = (ULONG) i * nAnzRow;
                for (USHORT j = 0; j < nAnzRow; j++)
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
            ULONG nCount = (ULONG) nAnzCol * nAnzRow;
            for (ULONG i = 0; i < nCount; i++)
                mRes.pMat[i].fVal = pMat[i].fVal;
        }
    }
}

void ScMatrix::MatTrans(ScMatrix& mRes) const
{
    if (nAnzCol != mRes.nAnzRow || nAnzRow != mRes.nAnzCol)
    {
        DBG_ERRORFILE("ScMatrix::MatTrans: dimension error");
    }
    else
    {
        if (bIsString)
        {
            mRes.ResetIsString();
            for ( ULONG i = 0; i < nAnzCol; i++ )
            {
                ULONG nStart = i * nAnzRow;
                for ( ULONG j = 0; j < nAnzRow; j++ )
                {
                    if ( bIsString[nStart+j] )
                        mRes.PutStringEntry( pMat[nStart+j].pS,
                            bIsString[nStart+j], j*mRes.nAnzRow+i );
                    else
                        mRes.pMat[j*mRes.nAnzRow+i].fVal = pMat[nStart+j].fVal;
                }
            }
        }
        else
        {
            mRes.DeleteIsString();
            for ( ULONG i = 0; i < nAnzCol; i++ )
            {
                ULONG nStart = i * nAnzRow;
                for ( ULONG j = 0; j < nAnzRow; j++ )
                {
                    mRes.pMat[j*mRes.nAnzRow+i].fVal = pMat[nStart+j].fVal;
                }
            }
        }
    }
}

void ScMatrix::MatCopyUpperLeft(ScMatrix& mRes) const
{
    if (nAnzCol < mRes.nAnzCol || nAnzRow < mRes.nAnzRow)
    {
        DBG_ERRORFILE("ScMatrix::MatCopyUpperLeft: dimension error");
    }
    else
    {
        if (bIsString)
        {
            mRes.ResetIsString();
            for ( ULONG i = 0; i < mRes.nAnzCol; i++ )
            {
                ULONG nStart = i * nAnzRow;
                for ( ULONG j = 0; j < mRes.nAnzRow; j++ )
                {
                    if ( bIsString[nStart+j] )
                        mRes.PutStringEntry( pMat[nStart+j].pS, bIsString[nStart+j],
                            i*mRes.nAnzRow+j );
                    else
                        mRes.pMat[i*mRes.nAnzRow+j].fVal = pMat[nStart+j].fVal;
                }
            }
        }
        else
        {
            mRes.DeleteIsString();
            for ( ULONG i = 0; i < mRes.nAnzCol; i++ )
            {
                ULONG nStart = i * nAnzRow;
                for ( ULONG j = 0; j < mRes.nAnzRow; j++ )
                {
                    mRes.pMat[i*mRes.nAnzRow+j].fVal = pMat[nStart+j].fVal;
                }
            }
        }
    }
}

void ScMatrix::FillDouble( double fVal, USHORT nC1, USHORT nR1,
                            USHORT nC2, USHORT nR2 )
{
    if (nC2 < nAnzCol && nR2 < nAnzRow)
    {
        if ( nC1 == 0 && nR1 == 0 && nC2 == nAnzCol-1 && nR2 == nAnzRow-1 )
        {
            ULONG nEnd = (ULONG) nAnzCol * nAnzRow;
            for ( ULONG j=0; j<nEnd; j++ )
                pMat[j].fVal = fVal;
        }
        else
        {
            for ( USHORT i=nC1; i<=nC2; i++ )
            {
                ULONG nOff1 = (ULONG) i * nAnzRow + nR1;
                ULONG nOff2 = nOff1 + nR2 - nR1;
                for ( ULONG j=nOff1; j<=nOff2; j++ )
                    pMat[j].fVal = fVal;
            }
        }
    }
    else
        DBG_ERRORFILE("ScMatrix::FillDouble: dimension error");
}

void ScMatrix::FillDoubleLowerLeft( double fVal, USHORT nC2 )
{
    if (nC2 < nAnzCol && nC2 < nAnzRow)
    {
        for ( USHORT i=1; i<=nC2; i++ )
        {
            ULONG nOff1 = (ULONG) i * nAnzRow;
            ULONG nOff2 = nOff1 + i;
            for ( ULONG j=nOff1; j<nOff2; j++ )
                pMat[j].fVal = fVal;
        }
    }
    else
        DBG_ERRORFILE("ScMatrix::FillDoubleLowerLeft: dimension error");
}

void ScMatrix::CompareEqual()
{
    ULONG n = (ULONG) nAnzCol * nAnzRow;
    if ( bIsString )
    {
        for ( ULONG j=0; j<n; j++ )
            if ( !bIsString[j])     // else: #WERT!
                pMat[j].fVal = (pMat[j].fVal == 0.0);
    }
    else
    {
        for ( ULONG j=0; j<n; j++ )
            pMat[j].fVal = (pMat[j].fVal == 0.0);
    }
}

void ScMatrix::CompareNotEqual()
{
    ULONG n = (ULONG) nAnzCol * nAnzRow;
    if ( bIsString )
    {
        for ( ULONG j=0; j<n; j++ )
            if ( !bIsString[j])     // else: #WERT!
                pMat[j].fVal = (pMat[j].fVal != 0.0);
    }
    else
    {
        for ( ULONG j=0; j<n; j++ )
            pMat[j].fVal = (pMat[j].fVal != 0.0);
    }
}

void ScMatrix::CompareLess()
{
    ULONG n = (ULONG) nAnzCol * nAnzRow;
    if ( bIsString )
    {
        for ( ULONG j=0; j<n; j++ )
            if ( !bIsString[j])     // else: #WERT!
                pMat[j].fVal = (pMat[j].fVal < 0.0);
    }
    else
    {
        for ( ULONG j=0; j<n; j++ )
            pMat[j].fVal = (pMat[j].fVal < 0.0);
    }
}

void ScMatrix::CompareGreater()
{
    ULONG n = (ULONG) nAnzCol * nAnzRow;
    if ( bIsString )
    {
        for ( ULONG j=0; j<n; j++ )
            if ( !bIsString[j])     // else: #WERT!
                pMat[j].fVal = (pMat[j].fVal > 0.0);
    }
    else
    {
        for ( ULONG j=0; j<n; j++ )
            pMat[j].fVal = (pMat[j].fVal > 0.0);
    }
}

void ScMatrix::CompareLessEqual()
{
    ULONG n = (ULONG) nAnzCol * nAnzRow;
    if ( bIsString )
    {
        for ( ULONG j=0; j<n; j++ )
            if ( !bIsString[j])     // else: #WERT!
                pMat[j].fVal = (pMat[j].fVal <= 0.0);
    }
    else
    {
        for ( ULONG j=0; j<n; j++ )
            pMat[j].fVal = (pMat[j].fVal <= 0.0);
    }
}

void ScMatrix::CompareGreaterEqual()
{
    ULONG n = (ULONG) nAnzCol * nAnzRow;
    if ( bIsString )
    {
        for ( ULONG j=0; j<n; j++ )
            if ( !bIsString[j])     // else: #WERT!
                pMat[j].fVal = (pMat[j].fVal >= 0.0);
    }
    else
    {
        for ( ULONG j=0; j<n; j++ )
            pMat[j].fVal = (pMat[j].fVal >= 0.0);
    }
}

BOOL ScMatrix::And()
{
    ULONG n = (ULONG) nAnzCol * nAnzRow;
    BOOL bAnd = TRUE;
    if ( bIsString )
    {
        for ( ULONG j=0; bAnd && j<n; j++ )
            if ( bIsString[j] )
                bAnd = FALSE;       // we're assuming a CompareMat
            else
                bAnd = (pMat[j].fVal != 0.0);
    }
    else
    {
        for ( ULONG j=0; bAnd && j<n; j++ )
            bAnd = (pMat[j].fVal != 0.0);
    }
    return bAnd;
}

BOOL ScMatrix::Or()
{
    ULONG n = (ULONG) nAnzCol * nAnzRow;
    BOOL bOr = FALSE;
    if ( bIsString )
    {
        for ( ULONG j=0; !bOr && j<n; j++ )
            if ( !bIsString[j] )
                bOr = (pMat[j].fVal != 0.0);
    }
    else
    {
        for ( ULONG j=0; !bOr && j<n; j++ )
            bOr = (pMat[j].fVal != 0.0);
    }
    return bOr;
}

