/*************************************************************************
 *
 *  $RCSfile: scmatrix.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:18 $
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

//------------------------------------------------------------------------

void ScMatrix::CreateMatrix(USHORT nC, USHORT nR)       // nur fuer ctor
{
    nAnzCol = nC;
    nAnzRow = nR;
    if (nC == 0 || nR == 0)
    {
        DBG_ERROR("ScMatrix::CreateMatrix: Dimensionsfehler");
        pMat = NULL;
    }
    else
        pMat = new MatValue[nC*nR];
    bIsString = NULL;
}

ScMatrix::~ScMatrix()
{
    if (bIsString)
    {
        USHORT nCount = nAnzCol * nAnzRow;
        for (USHORT i = 0; i < nCount; i++)
            if (bIsString[i])
                delete pMat[i].pS;
        delete [] bIsString;
    }
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
    rtl_TextEncoding eCharSet = rStream.GetStreamCharSet();
    USHORT nCount = nC * nR;
    for (USHORT i=0; i<nCount; i++)
    {
        BYTE nType;
        rStream >> nType;
        if ( nType == CELLTYPE_VALUE )
            rStream >> pMat[i].fVal;
        else
        {
                //  fuer unbekannte Typen String lesen und vergessen (Leer-String)
                //  (Aufwaerts-Kompatibilitaet)

            if ( nType != CELLTYPE_NONE )
                rStream.ReadByteString( aMatStr, eCharSet );

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

void ScMatrix::Store(SvStream& rStream) const
{
    if (!pMat)
    {
        rStream << (USHORT) 0;
        rStream << (USHORT) 0;
        DBG_ERROR("ScMatrix::Store: pMat == NULL");
        return;
    }

    rStream << nAnzCol;
    rStream << nAnzRow;

    String aMatStr;
    rtl_TextEncoding eCharSet = rStream.GetStreamCharSet();
    USHORT nCount = nAnzCol * nAnzRow;
    for (USHORT i=0; i<nCount; i++)
    {
        BYTE nType = CELLTYPE_VALUE;
        if ( bIsString && bIsString[i] )
        {
            if ( pMat[i].pS )
                aMatStr = *pMat[i].pS;
            else
            {
                DBG_ERROR("ScMatrix::Store: pS == NULL");
                aMatStr.Erase();
            }

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
    USHORT nCount = nAnzCol * nAnzRow;
    if (bIsString)
    {
        for (USHORT i = 0; i < nCount; i++)
            if (bIsString[i])
                delete pMat[i].pS;
    }
    else
        bIsString = new BYTE[nCount];
    memset( bIsString, 0, nCount * sizeof( BYTE ) );
}

void ScMatrix::PutDouble(double fVal, USHORT nC, USHORT nR)
{
    if (nC < nAnzCol && nR < nAnzRow)
        pMat[nC*nAnzRow+nR].fVal = fVal;
    else
        DBG_ERROR("ScMatrix::PutDouble: Dimensionsfehler");
}

void ScMatrix::PutString(const String& rStr, USHORT nC, USHORT nR)
{
    if (nC < nAnzCol && nR < nAnzRow)
    {
        if (bIsString == NULL)
            ResetIsString();
        bIsString[nC*nAnzRow+nR] = SC_MATVAL_STRING;
        pMat[nC*nAnzRow+nR].pS = new String(rStr);
    }
    else
        DBG_ERROR("ScMatrix::PutString: Dimensionsfehler");
}

void ScMatrix::PutString(const String& rStr, USHORT nIndex)
{
    if (bIsString == NULL)
        ResetIsString();
    bIsString[nIndex] = SC_MATVAL_STRING;
    pMat[nIndex].pS = new String(rStr);
}

void ScMatrix::PutEmpty(USHORT nC, USHORT nR)
{
    if (nC < nAnzCol && nR < nAnzRow)
    {
        if (bIsString == NULL)
            ResetIsString();
        bIsString[nC*nAnzRow+nR] = SC_MATVAL_EMPTY;
        pMat[nC*nAnzRow+nR].pS = new String;
    }
    else
        DBG_ERROR("ScMatrix::PutString: Dimensionsfehler");
}

void ScMatrix::PutEmpty(USHORT nIndex)
{
    if (bIsString == NULL)
        ResetIsString();
    bIsString[nIndex] = SC_MATVAL_EMPTY;
    pMat[nIndex].pS = new String;
}

double ScMatrix::GetDouble(USHORT nC, USHORT nR) const
{
    if (nC < nAnzCol && nR < nAnzRow)
        return pMat[nC*nAnzRow+nR].fVal;
    else
    {
        DBG_ERROR("ScMatrix::GetDouble: Dimensionsfehler");
        return 0.0;
    }
}

const String& ScMatrix::GetString(USHORT nC, USHORT nR) const
{
    if (nC < nAnzCol && nR < nAnzRow)
    {
        if (bIsString && bIsString[nC*nAnzRow+nR])
            return *(pMat[nC*nAnzRow+nR].pS);
        else
            DBG_ERROR("ScMatrix::GetString: Zugriffsfehler");
    }
    else
        DBG_ERROR("ScMatrix::GetString: Dimensionsfehler");
    return ScGlobal::GetEmptyString();
}

const MatValue* ScMatrix::Get(USHORT nC, USHORT nR, BOOL& bString) const
{
    if (nC < nAnzCol && nR < nAnzRow)
    {
        if (bIsString && bIsString[nC*nAnzRow+nR])
            bString = TRUE;
        else
            bString = FALSE;
        return &pMat[nC*nAnzRow+nR];
    }
    else
        DBG_ERROR("ScMatrix::Get: Dimensionsfehler");
    return NULL;
}

void ScMatrix::MatCopy(ScMatrix& mRes) const
{
    if (nAnzCol != mRes.nAnzCol || nAnzRow != mRes.nAnzRow)
    {
        DBG_ERROR("ScMatrix::MatCopy: Dimensionsfehler");
    }
    else
    {
        if (bIsString)
        {
            mRes.ResetIsString();
            for (USHORT i = 0; i < nAnzCol; i++)
            {
                for (USHORT j = 0; j < nAnzRow; j++)
                {
                    if (bIsString && bIsString[i*nAnzRow+j])
                    {
                        mRes.pMat[i*mRes.nAnzRow+j].pS =
                                    new String(*(pMat[i*nAnzRow+j].pS));
                        mRes.bIsString[i*mRes.nAnzRow+j] = bIsString[i*nAnzRow+j];
                    }
                    else
                        mRes.pMat[i*mRes.nAnzRow+j].fVal =
                                    pMat[i*nAnzRow+j].fVal;
                }
            }
        }
        else
        {
            for (USHORT i = 0; i < nAnzCol*nAnzRow; i++)
                mRes.pMat[i].fVal = pMat[i].fVal;
        }
    }
}

void ScMatrix::MatTrans(ScMatrix& mRes) const
{
    if (nAnzCol != mRes.nAnzRow || nAnzRow != mRes.nAnzCol)
    {
        DBG_ERROR("ScMatrix::MatTrans: Dimensionsfehler");
    }
    else
    {
        if (bIsString)
        {
            mRes.ResetIsString();
            for (USHORT i = 0; i < nAnzCol; i++)
            {
                for (USHORT j = 0; j < nAnzRow; j++)
                {
                    if (bIsString && bIsString[i*nAnzRow+j])
                    {
                        mRes.pMat[j*mRes.nAnzRow+i].pS =
                                     new String(*(pMat[i*nAnzRow+j].pS));
                        mRes.bIsString[j*mRes.nAnzRow+i] = bIsString[i*nAnzRow+j];
                    }
                    else
                        mRes.pMat[j*mRes.nAnzRow+i].fVal =
                                    pMat[i*nAnzRow+j].fVal;
                }
            }
        }
        else
        {
            for (USHORT i = 0; i < nAnzCol; i++)
            {
                for (USHORT j = 0; j < nAnzRow; j++)
                    mRes.pMat[j*mRes.nAnzRow+i].fVal =
                                    pMat[i*nAnzRow+j].fVal;
            }
        }
    }
}

void ScMatrix::MatCopyUpperLeft(ScMatrix& mRes) const
{
    if (nAnzCol < mRes.nAnzCol || nAnzRow < mRes.nAnzRow)
    {
        DBG_ERROR("ScMatrix::MatCopyUpperLeft : Dimensionsfehler");
    }
    else
    {
        if (bIsString)
        {
            mRes.ResetIsString();
            for (USHORT i = 0; i < mRes.nAnzCol; i++)
            {
                for (USHORT j = 0; j < mRes.nAnzRow; j++)
                {
                    if (bIsString && bIsString[i*nAnzRow+j])
                    {
                        mRes.pMat[i*mRes.nAnzRow+j].pS =
                                    new String(*(pMat[i*nAnzRow+j].pS));
                        mRes.bIsString[i*mRes.nAnzRow+j] = bIsString[i*nAnzRow+j];
                    }
                    else
                        mRes.pMat[i*mRes.nAnzRow+j].fVal =
                                    pMat[i*nAnzRow+j].fVal;
                }
            }
        }
        else
        {
            for (USHORT i = 0; i < mRes.nAnzCol; i++)
            {
                for (USHORT j = 0; j < mRes.nAnzRow; j++)
                    mRes.pMat[i*mRes.nAnzRow+j].fVal =
                                    pMat[i*nAnzRow+j].fVal;
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
            USHORT nEnd = nAnzCol * nAnzRow;
            for ( USHORT j=0; j<nEnd; j++ )
                pMat[j].fVal = fVal;
        }
        else
        {
            for ( USHORT i=nC1; i<=nC2; i++ )
            {
                USHORT nOff1 = i * nAnzRow + nR1;
                USHORT nOff2 = nOff1 + nR2 - nR1;
                for ( USHORT j=nOff1; j<=nOff2; j++ )
                    pMat[j].fVal = fVal;
            }
        }
    }
    else
        DBG_ERROR("ScMatrix::FillDouble: Dimensionsfehler");
}

void ScMatrix::FillDoubleLowerLeft( double fVal, USHORT nC2 )
{
    if (nC2 < nAnzCol && nC2 < nAnzRow)
    {
        for ( USHORT i=1; i<=nC2; i++ )
        {
            USHORT nOff1 = i * nAnzRow;
            USHORT nOff2 = nOff1 + i;
            for ( USHORT j=nOff1; j<nOff2; j++ )
                pMat[j].fVal = fVal;
        }
    }
    else
        DBG_ERROR("ScMatrix::FillDoubleLowerLeft: Dimensionsfehler");
}

void ScMatrix::CompareEqual()
{
    USHORT n = nAnzCol * nAnzRow;
    if ( bIsString )
    {
        for ( USHORT j=0; j<n; j++ )
            if ( !bIsString[j])     // else: #WERT!
                pMat[j].fVal = (pMat[j].fVal == 0.0);
    }
    else
    {
        for ( USHORT j=0; j<n; j++ )
            pMat[j].fVal = (pMat[j].fVal == 0.0);
    }
}

void ScMatrix::CompareNotEqual()
{
    USHORT n = nAnzCol * nAnzRow;
    if ( bIsString )
    {
        for ( USHORT j=0; j<n; j++ )
            if ( !bIsString[j])     // else: #WERT!
                pMat[j].fVal = (pMat[j].fVal != 0.0);
    }
    else
    {
        for ( USHORT j=0; j<n; j++ )
            pMat[j].fVal = (pMat[j].fVal != 0.0);
    }
}

void ScMatrix::CompareLess()
{
    USHORT n = nAnzCol * nAnzRow;
    if ( bIsString )
    {
        for ( USHORT j=0; j<n; j++ )
            if ( !bIsString[j])     // else: #WERT!
                pMat[j].fVal = (pMat[j].fVal < 0.0);
    }
    else
    {
        for ( USHORT j=0; j<n; j++ )
            pMat[j].fVal = (pMat[j].fVal < 0.0);
    }
}

void ScMatrix::CompareGreater()
{
    USHORT n = nAnzCol * nAnzRow;
    if ( bIsString )
    {
        for ( USHORT j=0; j<n; j++ )
            if ( !bIsString[j])     // else: #WERT!
                pMat[j].fVal = (pMat[j].fVal > 0.0);
    }
    else
    {
        for ( USHORT j=0; j<n; j++ )
            pMat[j].fVal = (pMat[j].fVal > 0.0);
    }
}

void ScMatrix::CompareLessEqual()
{
    USHORT n = nAnzCol * nAnzRow;
    if ( bIsString )
    {
        for ( USHORT j=0; j<n; j++ )
            if ( !bIsString[j])     // else: #WERT!
                pMat[j].fVal = (pMat[j].fVal <= 0.0);
    }
    else
    {
        for ( USHORT j=0; j<n; j++ )
            pMat[j].fVal = (pMat[j].fVal <= 0.0);
    }
}

void ScMatrix::CompareGreaterEqual()
{
    USHORT n = nAnzCol * nAnzRow;
    if ( bIsString )
    {
        for ( USHORT j=0; j<n; j++ )
            if ( !bIsString[j])     // else: #WERT!
                pMat[j].fVal = (pMat[j].fVal >= 0.0);
    }
    else
    {
        for ( USHORT j=0; j<n; j++ )
            pMat[j].fVal = (pMat[j].fVal >= 0.0);
    }
}

BOOL ScMatrix::And()
{
    USHORT n = nAnzCol * nAnzRow;
    BOOL bAnd = TRUE;
    if ( bIsString )
    {
        for ( USHORT j=0; bAnd && j<n; j++ )
            if ( bIsString[j] )
                bAnd = FALSE;       // wir gehen hier von einer CompareMat aus
            else
                bAnd &= (pMat[j].fVal != 0.0);
    }
    else
    {
        for ( USHORT j=0; bAnd && j<n; j++ )
            bAnd &= (pMat[j].fVal != 0.0);
    }
    return bAnd;
}

BOOL ScMatrix::Or()
{
    USHORT n = nAnzCol * nAnzRow;
    BOOL bOr = FALSE;
    if ( bIsString )
    {
        for ( USHORT j=0; !bOr && j<n; j++ )
            if ( !bIsString[j] )
                bOr |= (pMat[j].fVal != 0.0);
    }
    else
    {
        for ( USHORT j=0; !bOr && j<n; j++ )
            bOr |= (pMat[j].fVal != 0.0);
    }
    return bOr;
}



