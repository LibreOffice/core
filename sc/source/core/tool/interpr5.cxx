/*************************************************************************
 *
 *  $RCSfile: interpr5.cxx,v $
 *
 *  $Revision: 1.4 $
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

// INCLUDE ---------------------------------------------------------------

#include <string.h>
#include <math.h>

#ifndef _ZFORLIST_HXX //autogen
#include <svtools/zforlist.hxx>
#endif

#include "interpre.hxx"
#include "global.hxx"
#include "compiler.hxx"
#include "cell.hxx"
#include "document.hxx"
#include "dociter.hxx"
#include "scmatrix.hxx"
#include "globstr.hrc"

// STATIC DATA -----------------------------------------------------------

#define SCdEpsilon                1.0E-7


// -----------------------------------------------------------------------

double ScInterpreter::ScGetGGT(double fx, double fy)
{
    if (fy == 0.0 || fx == 0.0)
    {
        SetError(errIllegalArgument);
        return 1.0;
    }
    else
    {
        double fz = fmod(fx, fy);
        while (fz > 0.0)
        {
            fx = fy;
            fy = fz;
            fz = fmod(fx, fy);
        }
        return fy;
    }
}

void ScInterpreter::ScGGT()
{
    BYTE nParamCount = GetByte();
    if ( MustHaveParamCountMin( nParamCount, 1 ) )
    {
        double fSign = 1.0;
        double fx, fy;
        switch (GetStackType())
        {
            case svDouble :
            case svString:
            case svSingleRef:
            {
                fy = GetDouble();
                if (fy < 0.0)
                {
                    fy *= -1.0;
                    fSign *= -1.0;
                }
            }
            break;
            case svDoubleRef :
            {
                ScRange aRange;
                USHORT nErr = 0;
                PopDoubleRef( aRange );
                double nCellVal;
                ScValueIterator aValIter(pDok, aRange, glSubTotal);
                if (aValIter.GetFirst(nCellVal, nErr))
                {
                    fy = nCellVal;
                    if (fy < 0.0)
                    {
                        fy *= -1.0;
                        fSign *= -1.0;
                    }
                    while (nErr == 0 && aValIter.GetNext(nCellVal, nErr))
                    {
                        fx = nCellVal;
                        if (fx < 0.0)
                        {
                            fx *= -1.0;
                            fSign *= -1.0;
                        }
                        fy = ScGetGGT(fx, fy);
                    }
                    SetError(nErr);
                }
                else
                    SetError(errIllegalArgument);
            }
            break;
            case svMatrix :
            {
                ScMatrix* pMat = PopMatrix();
                if (pMat)
                {
                    USHORT nC, nR;
                    pMat->GetDimensions(nC, nR);
                    if (nC == 0 || nR == 0)
                        SetError(errIllegalArgument);
                    else
                    {
                        if (!pMat->IsValue(0))
                        {
                            SetIllegalArgument();
                            return;
                        }
                        fy = pMat->GetDouble(0);
                        if (fy < 0.0)
                        {
                            fy *= -1.0;
                            fSign *= -1.0;
                        }
                        ULONG nCount = (ULONG) nC * nR;
                        for ( ULONG j = 1; j < nCount; j++ )
                        {
                            if (!pMat->IsValue(j))
                            {
                                SetIllegalArgument();
                                return;
                            }
                            fx = pMat->GetDouble(j);
                            if (fx < 0.0)
                            {
                                fx *= -1.0;
                                fSign *= -1.0;
                            }
                            fy = ScGetGGT(fx, fy);
                        }
                    }
                }
            }
            break;
            default : SetError(errIllegalParameter); break;
        }
        ScRange aRange;
        for (short i = 0; i < (short) nParamCount - 1; i++)
        {
            switch (GetStackType())
            {
                case svDouble :
                case svString:
                case svSingleRef:
                {
                    fx = GetDouble();
                    if (fx < 0.0)
                    {
                        fx *= -1.0;
                        fSign *= -1.0;
                    }
                    fy = ScGetGGT(fx, fy);
                }
                break;
                case svDoubleRef :
                {
                    USHORT nErr = 0;
                    PopDoubleRef( aRange );
                    double nCellVal;
                    ScValueIterator aValIter(pDok, aRange, glSubTotal);
                    if (aValIter.GetFirst(nCellVal, nErr))
                    {
                        fx = nCellVal;
                        if (fx < 0.0)
                        {
                            fx *= -1.0;
                            fSign *= -1.0;
                        }
                        fy = ScGetGGT(fx, fy);
                        while (nErr == 0 && aValIter.GetNext(nCellVal, nErr))
                        {
                            fx = nCellVal;
                            if (fx < 0.0)
                            {
                                fx *= -1.0;
                                fSign *= -1.0;
                            }
                            fy = ScGetGGT(fx, fy);
                        }
                        SetError(nErr);
                    }
                    else
                        SetError(errIllegalArgument);
                }
                break;
                case svMatrix :
                {
                    ScMatrix* pMat = PopMatrix();
                    if (pMat)
                    {
                        USHORT nC, nR;
                        pMat->GetDimensions(nC, nR);
                        if (nC == 0 || nR == 0)
                            SetError(errIllegalArgument);
                        else
                        {
                            if (!pMat->IsValue(0))
                            {
                                SetIllegalArgument();
                                return;
                            }
                            fx = pMat->GetDouble(0);
                            if (fx < 0.0)
                            {
                                fx *= -1.0;
                                fSign *= -1.0;
                            }
                            fy = ScGetGGT(fx, fy);
                            ULONG nCount = (ULONG) nC * nR;
                            for ( ULONG j = 1; j < nCount; j++ )
                            {
                                if (!pMat->IsValue(j))
                                {
                                    SetIllegalArgument();
                                    return;
                                }
                                fx = pMat->GetDouble(j);
                                if (fx < 0.0)
                                {
                                    fx *= -1.0;
                                    fSign *= -1.0;
                                }
                                fy = ScGetGGT(fx, fy);
                            }
                        }
                    }
                }
                break;
                default : SetError(errIllegalParameter); break;
            }
        }
        if (fSign == -1.0)
            PushDouble(-fy);
        else
            PushDouble(fy);
    }
}

void ScInterpreter:: ScKGV()
{
    BYTE nParamCount = GetByte();
    if ( MustHaveParamCountMin( nParamCount, 1 ) )
    {
        double fSign = 1.0;
        double fx, fy;
        switch (GetStackType())
        {
            case svDouble :
            case svString:
            case svSingleRef:
            {
                fy = GetDouble();
                if (fy < 0.0)
                {
                    fy *= -1.0;
                    fSign *= -1.0;
                }
            }
            break;
            case svDoubleRef :
            {
                ScRange aRange;
                USHORT nErr = 0;
                PopDoubleRef( aRange );
                double nCellVal;
                ScValueIterator aValIter(pDok, aRange, glSubTotal);
                if (aValIter.GetFirst(nCellVal, nErr))
                {
                    fy = nCellVal;
                    if (fy < 0.0)
                    {
                        fy *= -1.0;
                        fSign *= -1.0;
                    }
                    while (nErr == 0 && aValIter.GetNext(nCellVal, nErr))
                    {
                        fx = nCellVal;
                        if (fx < 0.0)
                        {
                            fx *= -1.0;
                            fSign *= -1.0;
                        }
                        fy = fx * fy / ScGetGGT(fx, fy);
                    }
                    SetError(nErr);
                }
                else
                    SetError(errIllegalArgument);
            }
            break;
            case svMatrix :
            {
                ScMatrix* pMat = PopMatrix();
                if (pMat)
                {
                    USHORT nC, nR;
                    pMat->GetDimensions(nC, nR);
                    if (nC == 0 || nR == 0)
                        SetError(errIllegalArgument);
                    else
                    {
                        if (!pMat->IsValue(0))
                        {
                            SetIllegalArgument();
                            return;
                        }
                        fy = pMat->GetDouble(0);
                        if (fy < 0.0)
                        {
                            fy *= -1.0;
                            fSign *= -1.0;
                        }
                        ULONG nCount = (ULONG) nC * nR;
                        for ( ULONG j = 1; j < nCount; j++ )
                        {
                            if (!pMat->IsValue(j))
                            {
                                SetIllegalArgument();
                                return;
                            }
                            fx = pMat->GetDouble(j);
                            if (fx < 0.0)
                            {
                                fx *= -1.0;
                                fSign *= -1.0;
                            }
                            fy = fx * fy / ScGetGGT(fx, fy);
                        }
                    }
                }
            }
            break;
            default : SetError(errIllegalParameter); break;
        }
        ScRange aRange;
        for (short i = 0; i < (short) nParamCount - 1; i++)
        {
            switch (GetStackType())
            {
                case svDouble :
                case svString:
                case svSingleRef:
                {
                    fx = GetDouble();
                    if (fx < 0.0)
                    {
                        fx *= -1.0;
                        fSign *= -1.0;
                    }
                    fy = fx * fy / ScGetGGT(fx, fy);
                }
                break;
                case svDoubleRef :
                {
                    USHORT nErr = 0;
                    PopDoubleRef( aRange );
                    double nCellVal;
                    ScValueIterator aValIter(pDok, aRange, glSubTotal);
                    if (aValIter.GetFirst(nCellVal, nErr))
                    {
                        fx = nCellVal;
                        if (fx < 0.0)
                        {
                            fx *= -1.0;
                            fSign *= -1.0;
                        }
                        fy = fx * fy / ScGetGGT(fx, fy);
                        while (nErr == 0 && aValIter.GetNext(nCellVal, nErr))
                        {
                            fx = nCellVal;
                            if (fx < 0.0)
                            {
                                fx *= -1.0;
                                fSign *= -1.0;
                            }
                            fy = fx * fy / ScGetGGT(fx, fy);
                        }
                        SetError(nErr);
                    }
                    else
                        SetError(errIllegalArgument);
                }
                break;
                case svMatrix :
                {
                    ScMatrix* pMat = PopMatrix();
                    if (pMat)
                    {
                        USHORT nC, nR;
                        pMat->GetDimensions(nC, nR);
                        if (nC == 0 || nR == 0)
                            SetError(errIllegalArgument);
                        else
                        {
                            if (!pMat->IsValue(0))
                            {
                                SetIllegalArgument();
                                return;
                            }
                            fx = pMat->GetDouble(0);
                            if (fx < 0.0)
                            {
                                fx *= -1.0;
                                fSign *= -1.0;
                            }
                            fy = fx * fy / ScGetGGT(fx, fy);
                            ULONG nCount = (ULONG) nC * nR;
                            for ( ULONG j = 1; j < nCount; j++ )
                            {
                                if (!pMat->IsValue(j))
                                {
                                    SetIllegalArgument();
                                    return;
                                }
                                fx = pMat->GetDouble(j);
                                if (fx < 0.0)
                                {
                                    fx *= -1.0;
                                    fSign *= -1.0;
                                }
                                fy = fx * fy / ScGetGGT(fx, fy);
                            }
                        }
                    }
                }
                break;
                default : SetError(errIllegalParameter); break;
            }
        }
        if (fSign == -1.0)
            PushDouble(-fy);
        else
            PushDouble(fy);
    }
}

ScMatrix* ScInterpreter::GetNewMat(USHORT nC, USHORT nR, USHORT& nMatInd)
{
    if (nMatCount == MAX_ANZ_MAT)
    {
        DBG_ERROR("ScInterpreter::GetNewMat: Matrixueberlauf");
        SetError(errCodeOverflow);
        nMatInd = MAX_ANZ_MAT;
        return NULL;
    }
    else
    {
        if (!bMatDel)                               // beim ersten Mal
        {
            ppTempMatArray = new ScMatrix* [MAX_ANZ_MAT];
            for (USHORT i = 0; i < MAX_ANZ_MAT; i++)
                ppTempMatArray[i] = NULL;
            bMatDel = TRUE;
        }
        ppTempMatArray[nMatCount] = new ScMatrix(nC, nR);
        nMatInd = nMatCount++;
        return ppTempMatArray[nMatInd];
    }
}

void ScInterpreter::ResetNewMat(USHORT nIndex)
{
    if (nIndex < MAX_ANZ_MAT)
    {
        ppTempMatArray[nIndex] = NULL;
        if (nIndex == nMatCount - 1)
            nMatCount--;
    }
}

ScMatrix* ScInterpreter::GetMatrix(USHORT& nMatInd)
{
    ScMatrix* pMat = NULL;
    switch (GetStackType())
    {
        case svSingleRef :
        {
            ScAddress aAdr;
            PopSingleRef( aAdr );
            pMat = GetNewMat(1, 1, nMatInd);
            if (pMat)
            {
                ScBaseCell* pCell = GetCell( aAdr );
                if ( pCell && pCell->GetCellType() != CELLTYPE_NOTE )
                {
                    if (HasCellValueData(pCell))
                        pMat->PutDouble(GetCellValue(aAdr, pCell), 0, 0);
                    else
                    {
                        String aStr;
                        GetCellString(aStr, pCell);
                        pMat->PutString(aStr, 0, 0);
                    }
                }
                else
                    pMat->PutEmpty( 0, 0 );
            }
            else
                SetError(errCodeOverflow);
        }
        break;
        case svDoubleRef:
        {
            USHORT nCol1, nRow1, nTab1, nCol2, nRow2, nTab2;
            PopDoubleRef(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
            if (nTab1 == nTab2)
            {
                USHORT i, j;
                if ( (ULONG) (nRow2 - nRow1 + 1) * (nCol2 - nCol1 + 1) >
                        ScMatrix::GetElementsMax() )
                    SetError(errStackOverflow);
                else
                {
                    pMat = GetNewMat(nCol2 - nCol1 + 1, nRow2 - nRow1 + 1, nMatInd);
                    if (pMat)
                    {
                        ScAddress aAdr( nCol1, nRow1, nTab1 );
                        for (i = nRow1; i <= nRow2; i++)
                        {
                            aAdr.SetRow( i );
                            for (j = nCol1; j <= nCol2; j++)
                            {
                                aAdr.SetCol( j );
                                ScBaseCell* pCell = GetCell( aAdr );
                                if ( pCell && pCell->GetCellType() != CELLTYPE_NOTE )
                                {
                                    if (HasCellValueData(pCell))
                                        pMat->PutDouble(
                                                GetCellValue( aAdr, pCell ),
                                                j-nCol1, i-nRow1);
                                    else
                                    {
                                        String aStr;
                                        GetCellString(aStr, pCell);
                                        pMat->PutString(aStr, j-nCol1, i-nRow1);
                                    }
                                }
                                else
                                    pMat->PutEmpty( j-nCol1, i-nRow1 );
                            }
                        }
                    }
                    else
                        SetError(errCodeOverflow);
                }
            }
            else                                // keine 2D-Matrix
            {
                nMatInd = MAX_ANZ_MAT;
                SetError(errIllegalParameter);
            }
        }
        break;
        case svMatrix:
            pMat = PopMatrix();
            nMatInd = MAX_ANZ_MAT;
        break;
        default:
            Pop();
            nMatInd = MAX_ANZ_MAT;
            SetError(errIllegalParameter);
        break;
    }
    return pMat;
}

void ScInterpreter::ScMatValue()
{
    if ( MustHaveParamCount( GetByte(), 3 ) )
    {
        USHORT nR = (USHORT) SolarMath::ApproxFloor(GetDouble());       // 0 bis nAnz - 1
        USHORT nC = (USHORT) SolarMath::ApproxFloor(GetDouble());       // 0 bis nAnz - 1
        switch (GetStackType())
        {
            case svSingleRef :
            {
                ScAddress aAdr;
                PopSingleRef( aAdr );
                ScBaseCell* pCell = GetCell( aAdr );
                if (pCell && pCell->GetCellType() == CELLTYPE_FORMULA)
                {
                    USHORT nErrCode = ((ScFormulaCell*)pCell)->GetErrCode();
                    if (nErrCode != 0)
                    {
                        SetError(nErrCode);
                        PushInt(0);
                    }
                    else
                    {
                        ScMatrix* pMat;
                        ((ScFormulaCell*)pCell)->GetMatrix(&pMat);
                        if (pMat)
                        {
                            USHORT nCl, nRw;
                            pMat->GetDimensions(nCl, nRw);
                            if (nC < nCl && nR < nRw)
                            {
                                BOOL bIsString;
                                const MatValue* pMatVal = pMat->Get(nC, nR, bIsString);
                                if (bIsString)
                                    PushString( pMatVal->GetString() );
                                else
                                    PushDouble(pMatVal->fVal);
                            }
                            else
                                SetNoValue();
                        }
                        else
                            SetNoValue();
                    }
                }
                else
                    SetIllegalParameter();
            }
            break;
            case svDoubleRef :
            {
                USHORT nCol1, nRow1, nTab1, nCol2, nRow2, nTab2;
                PopDoubleRef(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
                if (nCol2 - nCol1 >= nR && nRow2 - nRow1 >= nC && nTab1 == nTab2)
                {
                    ScAddress aAdr( nCol1 + nR, nRow1 + nC, nTab1 );
                    ScBaseCell* pCell = GetCell( aAdr );
                    if (HasCellValueData(pCell))
                        PushDouble(GetCellValue( aAdr, pCell ));
                    else
                    {
                        String aStr;
                        GetCellString(aStr, pCell);
                        PushString(aStr);
                    }
                }
                else
                    SetNoValue();
            }
            break;
            case svMatrix:
            {
                ScMatrix* pMat = PopMatrix();
                if (pMat)
                {
                    USHORT nCl, nRw;
                    pMat->GetDimensions(nCl, nRw);
                    if (nC < nCl && nR < nRw)
                    {
                        BOOL bIsString;
                        const MatValue* pMatVal = pMat->Get(nC, nR, bIsString);
                        if (bIsString)
                            PushString( pMatVal->GetString() );
                        else
                            PushDouble(pMatVal->fVal);
                    }
                    else
                        SetNoValue();
                }
                else
                    SetNoValue();
            }
            break;
            default:
                Pop();
                SetIllegalParameter();
            break;
        }
    }
}

void ScInterpreter::ScEMat()
{
    if ( MustHaveParamCount( GetByte(), 1 ) )
    {
        ULONG nDim = (ULONG) SolarMath::ApproxFloor(GetDouble());
        if ( nDim * nDim > ScMatrix::GetElementsMax() || nDim == 0)
            SetIllegalArgument();
        else
        {
            USHORT nMatInd;
            ScMatrix* pRMat = GetNewMat((USHORT)nDim, (USHORT)nDim, nMatInd);
            if (pRMat)
            {
                MEMat(pRMat, (USHORT) nDim);
                nRetMat = nMatInd;
                PushMatrix(pRMat);
            }
            else
                SetError(errStackOverflow);
        }
    }
}

void ScInterpreter::MEMat(ScMatrix* mM, USHORT n)
{
    mM->FillDouble(0.0, 0, 0, n-1, n-1);
    for (USHORT i = 0; i < n; i++)
        mM->PutDouble(1.0, i, i);
}

void ScInterpreter::MFastMult(ScMatrix* pA, ScMatrix* pB, ScMatrix* pR,
                              USHORT n, USHORT m, USHORT l)
        // Multipliziert n x m Mat a mit m x l Mat b nach Mat r
{
    double sum;
    for (USHORT i = 0; i < n; i++)
    {
        for (USHORT j = 0; j < l; j++)
        {
            sum = 0.0;
            for (USHORT k = 0; k < m; k++)
                sum += pA->GetDouble(i,k)*pB->GetDouble(k,j);
            pR->PutDouble(sum, i, j);
        }
    }
}

void ScInterpreter::MFastSub(ScMatrix* pA, ScMatrix* pB, ScMatrix* pR,
                              USHORT n, USHORT m)
        // Subtrahiert n x m Mat a - m x l Mat b nach Mat r
{
    for (USHORT i = 0; i < n; i++)
    {
        for (USHORT j = 0; j < m; j++)
            pR->PutDouble(pA->GetDouble(i,j) - pB->GetDouble(i,j), i, j);
    }
}

void ScInterpreter::MFastTrans(ScMatrix* pA, ScMatrix* pR,
                              USHORT n, USHORT m)
        // Transponiert n x m Mat a nach Mat r
{
    for (USHORT i = 0; i < n; i++)
        for (USHORT j = 0; j < m; j++)
            pR->PutDouble(pA->GetDouble(i, j), j, i);
}

BOOL ScInterpreter::MFastBackSubst(ScMatrix* pA, ScMatrix* pR, USHORT n, BOOL bIsUpper)
        // Führt Rückwaertsersetzung der Dreickesmatrix Mat a nach Mat r durch
        // 2 Versionen fuer obere (U)  oder untere (L- Unit) Dreiecksmatrizen
{
    short i, j, k;
    double fSum;
    if (!bIsUpper)                          // L-Matrix, immer invertierbar
    {
        MEMat(pR, n);
        for (i = 1; i < (short) n; i++)
        {
            for (j = 0; j < i; j++)
            {
                fSum = 0.0;
                for (k = 0; k < i; k++)
                    fSum += pA->GetDouble(i,k) * pR->GetDouble(k,j);
                pR->PutDouble(-fSum, i, j);
            }
        }
    }
    else                                    // U-Matrix
    {
        for (i = 0; i < (short) n; i++)                         // Ist invertierbar?
            if (fabs(pA->GetDouble(i,i)) < SCdEpsilon)
                return FALSE;
        pR->FillDoubleLowerLeft(0.0, n-1);                      // untere Haelfte
        pR->PutDouble(1.0/pA->GetDouble(n-1, n-1), n-1, n-1);   // n-1, n-1
        for (i = (short) n-2; i >= 0; i--)
        {
            for (j = (short) n-1; j > i; j--)
            {
                fSum = 0.0;
                for (k = (short) n-1; k > i; k--)
                    fSum += pA->GetDouble(i, k) * pR->GetDouble(k, j);
                pR->PutDouble(-fSum/pA->GetDouble(i, i), i, j);
            }
            fSum = 0.0;                                         // Hauptdiagonale:
            for (k = (short) n-1; k > i; k--)
                fSum += pA->GetDouble(i, k) * pR->GetDouble(k, j);
            pR->PutDouble((1.0-fSum)/pA->GetDouble(i, i), i, i);
        }
    }
    return TRUE;
}

BOOL ScInterpreter::ScMatLUP(ScMatrix* mA, USHORT m, USHORT p,
                             ScMatrix* mL, ScMatrix* mU, ScMatrix* mP,
                             ULONG& rPermutCounter, BOOL& bIsInvertable)
    // Returnwert = False <=> Matrixarray voll
    // BIsInvertable = False: <= mA hat nicht Rang m
{
    USHORT nMatInd1, nMatInd2, nMatInd3, nMatInd4, nMatInd5;
    USHORT i, j;
    if (m == 1)
    {
        mL->PutDouble(1.0,0,0);
        for (j = 0; j < p; j++)
            if (fabs(mA->GetDouble(0, j)) >= SCdEpsilon)
                break;
        if (j == p)
        {
            bIsInvertable = FALSE;
            return TRUE;
        }
        MEMat(mP, p);
        if (j > 0 && j < p)
        {
            mP->PutDouble(0.0, 0, 0);
            mP->PutDouble(1.0, j, 0);
            mP->PutDouble(0.0, j, j);
            mP->PutDouble(1.0, 0, j);
            rPermutCounter++;
        }
        MFastMult(mA, mP, mU, m, p, p);
    }
    else
    {
        USHORT md2 = m/2;
        ScMatrix* mB = GetNewMat(md2, p, nMatInd1);
        ScMatrix* mC = GetNewMat(md2, p, nMatInd2);
        ScMatrix* mL1 = GetNewMat(md2, md2, nMatInd3);
        ScMatrix* mU1 = GetNewMat(md2, p, nMatInd4);
        ScMatrix* mP1 = GetNewMat(p, p, nMatInd5);
        if (!mB || !mC || !mL1 || !mU1 || !mP1 )
            return FALSE;
        for (i = 0; i < md2; i++)
        {
            for (j = 0; j < p; j++)
            {
                mB->PutDouble(mA->GetDouble(i, j), i, j);
                mC->PutDouble(mA->GetDouble(md2+i,j), i, j);
            }
        }
        if (!ScMatLUP(mB, md2, p, mL1, mU1, mP1, rPermutCounter, bIsInvertable))
            return FALSE;
        if (!bIsInvertable)
        {
            ResetNewMat(nMatInd5);
            ResetNewMat(nMatInd4);
            ResetNewMat(nMatInd3);
            ResetNewMat(nMatInd2);
            ResetNewMat(nMatInd1);
            delete mP1;
            delete mU1;
            delete mL1;
            delete mC;
            delete mB;
            return TRUE;
        }
        USHORT nMatInd6, nMatInd7, nMatInd8, nMatInd9, nMatInd10;
        USHORT nMatInd11, nMatInd12, nMatInd13;
        ScMatrix* mE    = GetNewMat(md2, md2,         nMatInd6);
        ScMatrix* mF    = GetNewMat(md2, md2,         nMatInd7);
        ScMatrix* mEInv = GetNewMat(md2, md2,         nMatInd8);
        ScMatrix* mG    = GetNewMat(md2, p,           nMatInd9);
        ScMatrix* mGs   = GetNewMat(md2, p - md2,     nMatInd10);
        ScMatrix* mU2   = GetNewMat(md2, p - md2,     nMatInd11);
        ScMatrix* mP2   = GetNewMat(p - md2, p - md2, nMatInd12);
        if (!mP2 || !mU2 || !mGs|| !mG || !mEInv || !mF || !mE)
            return FALSE;
        MFastTrans(mP1, mP, p, p);              // mP = mP1 hoch  -1
        ScMatrix* mD = mB;                      // mB wird nicht mehr gebraucht
        MFastMult(mC, mP, mD, md2, p, p);
        for (i = 0; i < md2; i++)
        {
            for (j = 0; j < md2; j++)
            {
                mE->PutDouble(mU1->GetDouble(i, j), i, j);
                mF->PutDouble(mD->GetDouble(i, j), i, j);
            }
        }
        BOOL bEInvok = MFastBackSubst(mE, mEInv, md2, TRUE); // MeInv = E hoch -1
        if (!bEInvok)
        {
            bIsInvertable = FALSE;
            ResetNewMat(nMatInd12);
            ResetNewMat(nMatInd11);
            ResetNewMat(nMatInd10);
            ResetNewMat(nMatInd9);
            ResetNewMat(nMatInd8);
            ResetNewMat(nMatInd7);
            ResetNewMat(nMatInd6);
            ResetNewMat(nMatInd5);
            ResetNewMat(nMatInd4);
            ResetNewMat(nMatInd3);
            ResetNewMat(nMatInd2);
            ResetNewMat(nMatInd1);
            delete mP2;
            delete mU2;
            delete mGs;
            delete mG;
            delete mEInv;
            delete mF;
            delete mE;
            delete mP1;
            delete mU1;
            delete mL1;
            delete mC;
            delete mB;
            return TRUE;
        }
        ScMatrix* mFEInv = mE;              // mE wird nicht mehr gebraucht.
        MFastMult(mF, mEInv, mFEInv, md2, md2, md2);
        ScMatrix* mFEInvU1 = mC;            // mC wird nicht mehr gebraucht
        MFastMult(mFEInv, mU1, mFEInvU1, md2, md2, p);
        MFastSub(mD, mFEInvU1, mG, md2, p);
        for (i = 0; i < md2; i++)
        {
            for (j = 0; j < p-md2; j++)
                mGs->PutDouble(mG->GetDouble(i, md2+j), i, j);
        }
        ScMatrix* mL2 = mF;                 // mF wird nicht mehr gebraucht
        if (!ScMatLUP(mGs, md2, p - md2, mL2, mU2, mP2, rPermutCounter, bIsInvertable))
            return FALSE;
        if (!bIsInvertable)
        {
            ResetNewMat(nMatInd12);
            ResetNewMat(nMatInd11);
            ResetNewMat(nMatInd10);
            ResetNewMat(nMatInd9);
            ResetNewMat(nMatInd8);
            ResetNewMat(nMatInd7);
            ResetNewMat(nMatInd6);
            ResetNewMat(nMatInd5);
            ResetNewMat(nMatInd4);
            ResetNewMat(nMatInd3);
            ResetNewMat(nMatInd2);
            ResetNewMat(nMatInd1);
            delete mP2;
            delete mU2;
            delete mGs;
            delete mG;
            delete mEInv;
            delete mF;
            delete mE;
            delete mP1;
            delete mU1;
            delete mL1;
            delete mC;
            delete mB;
            return TRUE;
        }
        ScMatrix* mP3 =  GetNewMat(p, p, nMatInd13);
        if (!mP3)
            return FALSE;
        MEMat(mP3, p);
        for (i = md2; i < p; i++)
        {
            for (j = md2; j < p; j++)
                mP3->PutDouble(mP2->GetDouble(i-md2, j-md2), i, j);
        }
        MFastMult(mP3, mP1, mP, p, p, p);       // Ergebnis P !!
        ScMatrix* mP3Inv = mP1;                 // mP1 wird nicht mehr gebraucht;
        MFastTrans(mP3, mP3Inv, p, p);
        ScMatrix* mH = mD;                      // mD wird nicht mehr gebraucht
        MFastMult(mU1, mP3Inv, mH, md2, p, p);
        MEMat(mL, m);                           // Ergebnis L :
        for (i = 0; i < md2; i++)
        {
            for (j = 0; j < i; j++)
                mL->PutDouble(mL1->GetDouble(i, j), i, j);
        }
        for (i = md2; i < m; i++)
            for (j = md2; j < i; j++)
                mL->PutDouble(mL2->GetDouble(i-md2, j-md2), i, j);
        for (i = md2; i < m; i++)
            for (j = 0; j < md2; j++)
                mL->PutDouble(mFEInv->GetDouble(i-md2, j), i, j);
                                                // Ergebnis U:
        mU->FillDoubleLowerLeft(0.0, m-1);
        for (i = 0; i < md2; i++)
            for (j = i; j < p; j++)
                mU->PutDouble(mH->GetDouble(i, j), i, j);
        for (i = md2; i < m; i++)
            for (j = i; j < p; j++)
                mU->PutDouble(mU2->GetDouble(i - md2, j - md2), i, j);
        ResetNewMat(nMatInd13);             // alle wieder freigeben;
        ResetNewMat(nMatInd12);
        ResetNewMat(nMatInd11);
        ResetNewMat(nMatInd10);
        ResetNewMat(nMatInd9);
        ResetNewMat(nMatInd8);
        ResetNewMat(nMatInd7);
        ResetNewMat(nMatInd6);
        ResetNewMat(nMatInd5);
        ResetNewMat(nMatInd4);
        ResetNewMat(nMatInd3);
        ResetNewMat(nMatInd2);
        ResetNewMat(nMatInd1);
        delete mP3;
        delete mP2;
        delete mU2;
        delete mGs;
        delete mG;
        delete mEInv;
        delete mF;
        delete mE;
        delete mP1;
        delete mU1;
        delete mL1;
        delete mC;
        delete mB;
    }
    return TRUE;
}

void ScInterpreter::ScMatDet()
{
    if ( MustHaveParamCount( GetByte(), 1 ) )
    {
        USHORT nMatInd;
        ScMatrix* pMat = GetMatrix(nMatInd);
        if (!pMat)
        {
            SetIllegalParameter();
            return;
        }
        if ( !pMat->IsNumeric() )
        {
            SetNoValue();
            return;
        }
        USHORT nC, nR;
        pMat->GetDimensions(nC, nR);
        if ( nC != nR || nC == 0 || (ULONG) nC * nC > ScMatrix::GetElementsMax() )
            SetIllegalParameter();
        else
        {
            double fVal = log((double)nC) / log(2.0);
            if (fVal - floor(fVal) != 0.0)
                fVal = floor(fVal) + 1.0;
            USHORT nDim = (USHORT) pow(2.0, fVal);
            USHORT nMatInd1, nMatInd2, nMatInd3;
            USHORT nMatInd4 = MAX_ANZ_MAT;
            ScMatrix* pU = GetNewMat(nDim, nDim, nMatInd1);
            ScMatrix* pL = GetNewMat(nDim, nDim, nMatInd2);
            ScMatrix* pP = GetNewMat(nDim, nDim, nMatInd3);
            ScMatrix* pA;
            if (nC == nDim)
                pA = pMat;
            else
            {
                pA = GetNewMat(nDim, nDim, nMatInd4);
                MEMat(pA, nDim);
                for (USHORT i = 0; i < nC; i++)
                    for (USHORT j = 0; j < nC; j++)
                    {
                        pA->PutDouble(pMat->GetDouble(i, j), i, j);
                    }
            }
            ULONG nPermutCounter = 0;
            BOOL bIsInvertable = TRUE;
            BOOL bOk = ScMatLUP(pA, nDim, nDim, pL, pU, pP,
                                nPermutCounter, bIsInvertable);
            ResetNewMat(nMatInd4);
            ResetNewMat(nMatInd3);
            ResetNewMat(nMatInd2);
            if (nC != nDim)
                delete pA;
            delete pP;
            delete pL;
            if (bOk)
            {
                if (!bIsInvertable)
                    PushInt(0);
                else
                {
                    double fDet = 1.0;
                    for (USHORT i = 0; i < nC; i++)
                        fDet *= pU->GetDouble(i, i);
                    if (nPermutCounter % 2 != 0)
                        fDet *= -1.0;
                    PushDouble(fDet);
                }
            }
            else
            {
                SetError(errCodeOverflow);
                PushInt(0);
            }
            ResetNewMat(nMatInd1);
            delete pU;
        }
    }
}

void ScInterpreter::ScMatInv()
{
    if ( MustHaveParamCount( GetByte(), 1 ) )
    {
        USHORT nMatInd;
        ScMatrix* pMat = GetMatrix(nMatInd);
        if (!pMat)
        {
            SetIllegalParameter();
            return;
        }
        if ( !pMat->IsNumeric() )
        {
            SetNoValue();
            return;
        }
        USHORT nC, nR;
        pMat->GetDimensions(nC, nR);
        if ( nC != nR || nC == 0 || (ULONG) nC * nC > ScMatrix::GetElementsMax() )
            SetIllegalParameter();
        else
        {
            double fVal = log((double)nC) / log(2.0);
            if (fVal - floor(fVal) != 0.0)
                fVal = floor(fVal) + 1.0;
            USHORT nDim = (USHORT) pow(2.0, fVal);
            USHORT nMatInd1, nMatInd2, nMatInd3;
            USHORT nMatInd4 = MAX_ANZ_MAT;
            ScMatrix* pU = GetNewMat(nDim, nDim, nMatInd1);
            ScMatrix* pL = GetNewMat(nDim, nDim, nMatInd2);
            ScMatrix* pP = GetNewMat(nDim, nDim, nMatInd3);
            ScMatrix* pA;
            if (nC == nDim)
                pA = pMat;
            else
            {
                pA = GetNewMat(nDim, nDim, nMatInd4);
                MEMat(pA, nDim);
                for (USHORT i = 0; i < nC; i++)
                    for (USHORT j = 0; j < nC; j++)
                    {
                        pA->PutDouble(pMat->GetDouble(i, j), i, j);
                    }
            }
            ULONG nPermutCounter = 0;
            BOOL bIsInvertable = TRUE;
            BOOL bOk = ScMatLUP(pA, nDim, nDim, pL, pU, pP,
                                nPermutCounter, bIsInvertable);
            if (bOk)
            {
                if (!bIsInvertable)
                    SetNoValue();
                else
                {
                    USHORT nMatInd5;
                    ScMatrix* pUInv = GetNewMat(nDim, nDim, nMatInd5);
                    if (!pUInv)
                        PushInt(0);
                    else
                    {
                        bOk = MFastBackSubst(pU, pUInv, nDim, TRUE);
                        if (!bOk)
                            SetNoValue();
                        else
                        {
                            ScMatrix* pPInv = pU;
                            MFastTrans(pP, pPInv, nDim, nDim);
                            ScMatrix* pPInvUInv = pP;
                            MFastMult(pPInv, pUInv, pPInvUInv, nDim, nDim, nDim);
                            ScMatrix* pLInv = pPInv;
                            MFastBackSubst(pL, pLInv, nDim, FALSE);
                            if (nDim == nC)
                                MFastMult(pPInvUInv, pLInv, pMat, nDim, nDim, nDim);
                            else
                            {
                                MFastMult(pPInvUInv, pLInv, pL, nDim, nDim, nDim);
                                for (USHORT i = 0; i < nC; i++)
                                    for (USHORT j = 0; j < nC; j++)
                                        pMat->PutDouble(pL->GetDouble(i, j), i, j);
                            }
                            PushMatrix(pMat);
                            if (nMatInd != MAX_ANZ_MAT) // vorher neue Matrix
                                nRetMat = nMatInd;      // sonst nRetMat wie vorher
                            ResetNewMat(nMatInd5);
                            delete pUInv;
                        }
                    }
                }
            }
            else
            {
                SetError(errCodeOverflow);
                PushInt(0);
            }
            ResetNewMat(nMatInd4);
            ResetNewMat(nMatInd3);
            ResetNewMat(nMatInd2);
            ResetNewMat(nMatInd1);
            if (nC != nDim)
                delete pA;
            delete pP;
            delete pL;
            delete pU;
        }
    }
}

void ScInterpreter::ScMatMult()
{
    if ( MustHaveParamCount( GetByte(), 2 ) )
    {
        USHORT nMatInd1, nMatInd2;
        ScMatrix* pMat2 = GetMatrix(nMatInd2);
        ScMatrix* pMat1 = GetMatrix(nMatInd1);
        ScMatrix* pRMat;
        if (pMat1 && pMat2)
        {
            if ( pMat1->IsNumeric() && pMat2->IsNumeric() )
            {
                USHORT nC1, nR1, nC2, nR2;
                pMat1->GetDimensions(nC1, nR1);
                pMat2->GetDimensions(nC2, nR2);
                if (nC1 != nR2)
                    SetIllegalParameter();
                else
                {
                    USHORT nMatInd;
                    pRMat = GetNewMat(nC2, nR1, nMatInd);       // Matrixabsicherung
                    if (pRMat)
                    {
                        double sum;
                        for (USHORT i = 0; i < nR1; i++)
                        {
                            for (USHORT j = 0; j < nC2; j++)
                            {
                                sum = 0.0;
                                for (USHORT k = 0; k < nC1; k++)
                                {
                                    sum += pMat1->GetDouble(k,i)*pMat2->GetDouble(j,k);
                                }
                                pRMat->PutDouble(sum, j, i);
                            }
                        }
                        PushMatrix(pRMat);
                        nRetMat = nMatInd;
                    }
                    else
                        SetNoValue();
                }
            }
            else
                SetNoValue();
        }
        else
            SetIllegalParameter();
    }
}

void ScInterpreter::ScMatTrans()
{
    if ( MustHaveParamCount( GetByte(), 1 ) )
    {
        USHORT nMatInd, nMatInd1;
        ScMatrix* pMat = GetMatrix(nMatInd);
        ScMatrix* pRMat;
        if (pMat)
        {
            USHORT nC, nR;
            pMat->GetDimensions(nC, nR);
            pRMat = GetNewMat(nR, nC, nMatInd1);
            pMat->MatTrans(*pRMat);
            PushMatrix(pRMat);
            nRetMat = nMatInd1;
        }
        else
            SetIllegalParameter();
    }
}

ScMatrix* ScInterpreter::MatAdd(ScMatrix* pMat1, ScMatrix* pMat2)
{
    USHORT nC1, nR1, nC2, nR2, nMinC, nMinR, i, j;
    pMat1->GetDimensions(nC1, nR1);
    pMat2->GetDimensions(nC2, nR2);
    if (nC1 < nC2)
        nMinC = nC1;
    else
        nMinC = nC2;
    if (nR1 < nR2)
        nMinR = nR1;
    else
        nMinR = nR2;
    USHORT nMatInd;
    ScMatrix* pResMat = GetNewMat(nMinC, nMinR, nMatInd);
    if (pResMat)
    {
        for (i = 0; i < nMinC; i++)
        {
            for (j = 0; j < nMinR; j++)
            {
                if (!pMat1->IsString(i,j) && !pMat2->IsString(i,j))
                    pResMat->PutDouble( SolarMath::ApproxAdd( pMat1->GetDouble(i,j),
                                       pMat2->GetDouble(i,j)), i, j);
                else
                    pResMat->PutString(ScGlobal::GetRscString(
                                                    STR_NO_VALUE), i, j);
            }
        }
        nRetMat = nMatInd;
    }
    return pResMat;
}

ScMatrix* ScInterpreter::MatSub(ScMatrix* pMat1, ScMatrix* pMat2)
{
    USHORT nC1, nR1, nC2, nR2, nMinC, nMinR, i, j;
    pMat1->GetDimensions(nC1, nR1);
    pMat2->GetDimensions(nC2, nR2);
    if (nC1 < nC2)
        nMinC = nC1;
    else
        nMinC = nC2;
    if (nR1 < nR2)
        nMinR = nR1;
    else
        nMinR = nR2;
    USHORT nMatInd;
    ScMatrix* pResMat = GetNewMat(nMinC, nMinR, nMatInd);
    if (pResMat)
    {
        for (i = 0; i < nMinC; i++)
        {
            for (j = 0; j < nMinR; j++)
            {
                if (!pMat1->IsString(i,j) && !pMat2->IsString(i,j))
                    pResMat->PutDouble( SolarMath::ApproxSub( pMat1->GetDouble(i,j),
                                       pMat2->GetDouble(i,j)), i, j);
                else
                    pResMat->PutString(ScGlobal::GetRscString(
                                                    STR_NO_VALUE), i, j);
            }
        }
        nRetMat = nMatInd;
    }
    return pResMat;
}

ScMatrix* ScInterpreter::MatMul(ScMatrix* pMat1, ScMatrix* pMat2)
{
    USHORT nC1, nR1, nC2, nR2, nMinC, nMinR, i, j;
    pMat1->GetDimensions(nC1, nR1);
    pMat2->GetDimensions(nC2, nR2);
    if (nC1 < nC2)
        nMinC = nC1;
    else
        nMinC = nC2;
    if (nR1 < nR2)
        nMinR = nR1;
    else
        nMinR = nR2;
    USHORT nMatInd;
    ScMatrix* pResMat = GetNewMat(nMinC, nMinR, nMatInd);
    if (pResMat)
    {
        for (i = 0; i < nMinC; i++)
        {
            for (j = 0; j < nMinR; j++)
            {
                if (!pMat1->IsString(i,j) && !pMat2->IsString(i,j))
                    pResMat->PutDouble(pMat1->GetDouble(i,j) *
                                       pMat2->GetDouble(i,j), i, j);
                else
                    pResMat->PutString(ScGlobal::GetRscString(
                                                    STR_NO_VALUE), i, j);
            }
        }
        nRetMat = nMatInd;
    }
    return pResMat;
}

ScMatrix* ScInterpreter::MatDiv(ScMatrix* pMat1, ScMatrix* pMat2)
{
    USHORT nC1, nR1, nC2, nR2, nMinC, nMinR, i, j;
    pMat1->GetDimensions(nC1, nR1);
    pMat2->GetDimensions(nC2, nR2);
    if (nC1 < nC2)
        nMinC = nC1;
    else
        nMinC = nC2;
    if (nR1 < nR2)
        nMinR = nR1;
    else
        nMinR = nR2;
    USHORT nMatInd;
    ScMatrix* pResMat = GetNewMat(nMinC, nMinR, nMatInd);
    if (pResMat)
    {
        for (i = 0; i < nMinC; i++)
        {
            for (j = 0; j < nMinR; j++)
            {
                if (!pMat1->IsString(i,j) && !pMat2->IsString(i,j))
                    pResMat->PutDouble(pMat1->GetDouble(i,j) /
                                       pMat2->GetDouble(i,j), i, j);
                else
                    pResMat->PutString(ScGlobal::GetRscString(
                                                    STR_NO_VALUE), i, j);
            }
        }
        nRetMat = nMatInd;
    }
    return pResMat;
}

ScMatrix* ScInterpreter::MatPow(ScMatrix* pMat1, ScMatrix* pMat2)
{
    USHORT nC1, nR1, nC2, nR2, nMinC, nMinR, i, j;
    pMat1->GetDimensions(nC1, nR1);
    pMat2->GetDimensions(nC2, nR2);
    if (nC1 < nC2)
        nMinC = nC1;
    else
        nMinC = nC2;
    if (nR1 < nR2)
        nMinR = nR1;
    else
        nMinR = nR2;
    USHORT nMatInd;
    ScMatrix* pResMat = GetNewMat(nMinC, nMinR, nMatInd);
    if (pResMat)
    {
        for (i = 0; i < nMinC; i++)
        {
            for (j = 0; j < nMinR; j++)
            {
                if (!pMat1->IsString(i,j) && !pMat2->IsString(i,j))
                    pResMat->PutDouble(pow(pMat1->GetDouble(i,j),
                                           pMat2->GetDouble(i,j)), i, j);
                else
                    pResMat->PutString(ScGlobal::GetRscString(
                                                    STR_NO_VALUE), i, j);
            }
        }
        nRetMat = nMatInd;
    }
    return pResMat;
}

ScMatrix* ScInterpreter::MatConcat(ScMatrix* pMat1, ScMatrix* pMat2)
{
    USHORT nC1, nR1, nC2, nR2, nMinC, nMinR, i, j;
    pMat1->GetDimensions(nC1, nR1);
    pMat2->GetDimensions(nC2, nR2);
    if (nC1 < nC2)
        nMinC = nC1;
    else
        nMinC = nC2;
    if (nR1 < nR2)
        nMinR = nR1;
    else
        nMinR = nR2;
    USHORT nMatInd;
    ScMatrix* pResMat = GetNewMat(nMinC, nMinR, nMatInd);
    if (pResMat)
    {
        for (i = 0; i < nMinC; i++)
        {
            for (j = 0; j < nMinR; j++)
            {
                if (pMat1->IsString(i,j) && pMat2->IsString(i,j))
                {
                    String aTmp( pMat1->GetString(i,j) );
                    aTmp += pMat2->GetString(i,j);
                    pResMat->PutString( aTmp , i, j);
                }
                else
                    pResMat->PutString(ScGlobal::GetRscString(
                                                    STR_NO_VALUE), i, j);
            }
        }
        nRetMat = nMatInd;
    }
    return pResMat;
}


// fuer DATE, TIME, DATETIME
void lcl_GetDiffDateTimeFmtType( short& nFuncFmt, short nFmt1, short nFmt2 )
{
    if ( nFmt1 != NUMBERFORMAT_UNDEFINED || nFmt2 != NUMBERFORMAT_UNDEFINED )
    {
        if ( nFmt1 == nFmt2 )
        {
            if ( nFmt1 == NUMBERFORMAT_TIME || nFmt1 == NUMBERFORMAT_DATETIME )
                nFuncFmt = NUMBERFORMAT_TIME;   // Zeiten ergeben Zeit
            // else: nichts besonderes, Zahl (Datum - Datum := Tage)
        }
        else if ( nFmt1 == NUMBERFORMAT_UNDEFINED )
            nFuncFmt = nFmt2;   // z.B. Datum + Tage := Datum
        else if ( nFmt2 == NUMBERFORMAT_UNDEFINED )
            nFuncFmt = nFmt1;
        else
        {
            if ( nFmt1 == NUMBERFORMAT_DATE || nFmt2 == NUMBERFORMAT_DATE ||
                nFmt1 == NUMBERFORMAT_DATETIME || nFmt2 == NUMBERFORMAT_DATETIME )
            {
                if ( nFmt1 == NUMBERFORMAT_TIME || nFmt2 == NUMBERFORMAT_TIME )
                    nFuncFmt = NUMBERFORMAT_DATETIME;   // Datum + Zeit
            }
        }
    }
}


void ScInterpreter::ScAdd()
{
    ScMatrix* pMat1 = NULL;
    ScMatrix* pMat2 = NULL;
    double fVal1, fVal2;
    USHORT nMatInd1, nMatInd2;
    short nFmt1, nFmt2;
    nFmt1 = nFmt2 = NUMBERFORMAT_UNDEFINED;
    short nFmtCurrencyType = nCurFmtType;
    ULONG nFmtCurrencyIndex = nCurFmtIndex;
    MatrixDoubleRefToMatrix();
    if ( GetStackType() == svMatrix )
        pMat2 = GetMatrix(nMatInd2);
    else
    {
        fVal2 = GetDouble();
        switch ( nCurFmtType )
        {
            case NUMBERFORMAT_DATE :
            case NUMBERFORMAT_TIME :
            case NUMBERFORMAT_DATETIME :
                nFmt2 = nCurFmtType;
            break;
            case NUMBERFORMAT_CURRENCY :
                nFmtCurrencyType = nCurFmtType;
                nFmtCurrencyIndex = nCurFmtIndex;
            break;
        }
    }
    MatrixDoubleRefToMatrix();
    if ( GetStackType() == svMatrix )
        pMat1 = GetMatrix(nMatInd1);
    else
    {
        fVal1 = GetDouble();
        switch ( nCurFmtType )
        {
            case NUMBERFORMAT_DATE :
            case NUMBERFORMAT_TIME :
            case NUMBERFORMAT_DATETIME :
                nFmt1 = nCurFmtType;
            break;
            case NUMBERFORMAT_CURRENCY :
                nFmtCurrencyType = nCurFmtType;
                nFmtCurrencyIndex = nCurFmtIndex;
            break;
        }
    }
    if (pMat1 && pMat2)
    {
        ScMatrix* pResMat = MatAdd(pMat1, pMat2);
        if (!pResMat)
            SetNoValue();
        else
            PushMatrix(pResMat);
    }
    else if (pMat1 || pMat2)
    {
        double fVal;
        ScMatrix* pMat = pMat1;
        if (!pMat)
        {
            fVal = fVal1;
            pMat = pMat2;
        }
        else
            fVal = fVal2;
        USHORT nC, nR;
        pMat->GetDimensions(nC, nR);
        USHORT nMatInd;
        ScMatrix* pResMat = GetNewMat(nC, nR, nMatInd);
        if (pResMat)
        {
            ULONG nCount = (ULONG) nC * nR;
            for ( ULONG i = 0; i < nCount; i++ )
            {
                if (pMat->IsValue(i))
                    pResMat->PutDouble( SolarMath::ApproxAdd( pMat->GetDouble(i), fVal), i);
                else
                    pResMat->PutString(ScGlobal::GetRscString(STR_NO_VALUE), i);
            }
            PushMatrix(pResMat);
            nRetMat = nMatInd;
        }
        else
            SetNoValue();
    }
    else
        PushDouble( SolarMath::ApproxAdd( fVal1, fVal2 ) );
    if ( nFmtCurrencyType == NUMBERFORMAT_CURRENCY )
    {
        nFuncFmtType = nFmtCurrencyType;
        nFuncFmtIndex = nFmtCurrencyIndex;
    }
    else
        lcl_GetDiffDateTimeFmtType( nFuncFmtType, nFmt1, nFmt2 );
}

void ScInterpreter::ScAmpersand()
{
    ScMatrix* pMat1 = NULL;
    ScMatrix* pMat2 = NULL;
    String sStr1, sStr2;
    USHORT nMatInd1, nMatInd2;
    MatrixDoubleRefToMatrix();
    if ( GetStackType() == svMatrix )
        pMat2 = GetMatrix(nMatInd2);
    else
        sStr2 = GetString();
    MatrixDoubleRefToMatrix();
    if ( GetStackType() == svMatrix )
        pMat1 = GetMatrix(nMatInd1);
    else
        sStr1 = GetString();
    if (pMat1 && pMat2)
    {
        ScMatrix* pResMat = MatConcat(pMat1, pMat2);
        if (!pResMat)
            SetNoValue();
        else
            PushMatrix(pResMat);
    }
    else if (pMat1 || pMat2)
    {
        String sStr;
        BOOL bFlag;
        ScMatrix* pMat = pMat1;
        if (!pMat)
        {
            sStr = sStr1;
            pMat = pMat2;
            bFlag = TRUE;           // double - Matrix
        }
        else
        {
            sStr = sStr2;
            bFlag = FALSE;          // Matrix - double
        }
        USHORT nC, nR;
        pMat->GetDimensions(nC, nR);
        USHORT nMatInd;
        ScMatrix* pResMat = GetNewMat(nC, nR, nMatInd);
        if (pResMat)
        {
            ULONG nCount = (ULONG) nC * nR;
            if (bFlag)
            {
                for ( ULONG i = 0; i < nCount; i++ )
                    if (!pMat->IsValue(i))
                    {
                        String sS = sStr;
                        sS += pMat->GetString(i);
                        pResMat->PutString(sS, i);
                    }
                    else
                        pResMat->PutString(ScGlobal::GetRscString(STR_NO_VALUE), i);
            }
            else
            {
                for ( ULONG i = 0; i < nCount; i++ )
                    if (!pMat->IsValue(i))
                    {
                        String sS = pMat->GetString(i);
                        sS += sStr;
                        pResMat->PutString(sS, i);
                    }
                    else
                        pResMat->PutString(ScGlobal::GetRscString(STR_NO_VALUE), i);
            }
            PushMatrix(pResMat);
            nRetMat = nMatInd;
        }
        else
            SetNoValue();
    }
    else
    {
        if ( CheckStringResultLen( sStr1, sStr2 ) )
            sStr1 += sStr2;
        PushString(sStr1);
    }
}

void ScInterpreter::ScSub()
{
    ScMatrix* pMat1 = NULL;
    ScMatrix* pMat2 = NULL;
    double fVal1, fVal2;
    USHORT nMatInd1, nMatInd2;
    short nFmt1, nFmt2;
    nFmt1 = nFmt2 = NUMBERFORMAT_UNDEFINED;
    short nFmtCurrencyType = nCurFmtType;
    ULONG nFmtCurrencyIndex = nCurFmtIndex;
    MatrixDoubleRefToMatrix();
    if ( GetStackType() == svMatrix )
        pMat2 = GetMatrix(nMatInd2);
    else
    {
        fVal2 = GetDouble();
        switch ( nCurFmtType )
        {
            case NUMBERFORMAT_DATE :
            case NUMBERFORMAT_TIME :
            case NUMBERFORMAT_DATETIME :
                nFmt2 = nCurFmtType;
            break;
            case NUMBERFORMAT_CURRENCY :
                nFmtCurrencyType = nCurFmtType;
                nFmtCurrencyIndex = nCurFmtIndex;
            break;
        }
    }
    MatrixDoubleRefToMatrix();
    if ( GetStackType() == svMatrix )
        pMat1 = GetMatrix(nMatInd1);
    else
    {
        fVal1 = GetDouble();
        switch ( nCurFmtType )
        {
            case NUMBERFORMAT_DATE :
            case NUMBERFORMAT_TIME :
            case NUMBERFORMAT_DATETIME :
                nFmt1 = nCurFmtType;
            break;
            case NUMBERFORMAT_CURRENCY :
                nFmtCurrencyType = nCurFmtType;
                nFmtCurrencyIndex = nCurFmtIndex;
            break;
        }
    }
    if (pMat1 && pMat2)
    {
        ScMatrix* pResMat = MatSub(pMat1, pMat2);
        if (!pResMat)
            SetNoValue();
        else
            PushMatrix(pResMat);
    }
    else if (pMat1 || pMat2)
    {
        double fVal;
        BOOL bFlag;
        ScMatrix* pMat = pMat1;
        if (!pMat)
        {
            fVal = fVal1;
            pMat = pMat2;
            bFlag = TRUE;           // double - Matrix
        }
        else
        {
            fVal = fVal2;
            bFlag = FALSE;          // Matrix - double
        }
        USHORT nC, nR;
        pMat->GetDimensions(nC, nR);
        USHORT nMatInd;
        ScMatrix* pResMat = GetNewMat(nC, nR, nMatInd);
        if (pResMat)
        {   // mehr klammern wg. compiler macke
            ULONG nCount = (ULONG) nC * nR;
            if (bFlag)
            {   for ( ULONG i = 0; i < nCount; i++ )
                {   if (pMat->IsValue(i))
                        pResMat->PutDouble( SolarMath::ApproxSub( fVal, pMat->GetDouble(i)), i);
                    else
                        pResMat->PutString(ScGlobal::GetRscString(STR_NO_VALUE), i);
                }
            }
            else
            {   for ( ULONG i = 0; i < nCount; i++ )
                {   if (pMat->IsValue(i))
                        pResMat->PutDouble( SolarMath::ApproxSub( pMat->GetDouble(i), fVal), i);
                    else
                        pResMat->PutString(ScGlobal::GetRscString(STR_NO_VALUE), i);
                }
            }
            PushMatrix(pResMat);
            nRetMat = nMatInd;
        }
        else
            SetNoValue();
    }
    else
        PushDouble( SolarMath::ApproxSub( fVal1, fVal2 ) );
    if ( nFmtCurrencyType == NUMBERFORMAT_CURRENCY )
    {
        nFuncFmtType = nFmtCurrencyType;
        nFuncFmtIndex = nFmtCurrencyIndex;
    }
    else
        lcl_GetDiffDateTimeFmtType( nFuncFmtType, nFmt1, nFmt2 );
}

void ScInterpreter::ScMul()
{
    ScMatrix* pMat1 = NULL;
    ScMatrix* pMat2 = NULL;
    double fVal1, fVal2;
    USHORT nMatInd1, nMatInd2;
    short nFmtCurrencyType = nCurFmtType;
    ULONG nFmtCurrencyIndex = nCurFmtIndex;
    MatrixDoubleRefToMatrix();
    if ( GetStackType() == svMatrix )
        pMat2 = GetMatrix(nMatInd2);
    else
    {
        fVal2 = GetDouble();
        switch ( nCurFmtType )
        {
            case NUMBERFORMAT_CURRENCY :
                nFmtCurrencyType = nCurFmtType;
                nFmtCurrencyIndex = nCurFmtIndex;
            break;
        }
    }
    MatrixDoubleRefToMatrix();
    if ( GetStackType() == svMatrix )
        pMat1 = GetMatrix(nMatInd1);
    else
    {
        fVal1 = GetDouble();
        switch ( nCurFmtType )
        {
            case NUMBERFORMAT_CURRENCY :
                nFmtCurrencyType = nCurFmtType;
                nFmtCurrencyIndex = nCurFmtIndex;
            break;
        }
    }
    if (pMat1 && pMat2)
    {
        ScMatrix* pResMat = MatMul(pMat1, pMat2);
        if (!pResMat)
            SetNoValue();
        else
            PushMatrix(pResMat);
    }
    else if (pMat1 || pMat2)
    {
        double fVal;
        ScMatrix* pMat = pMat1;
        if (!pMat)
        {
            fVal = fVal1;
            pMat = pMat2;
        }
        else
            fVal = fVal2;
        USHORT nC, nR;
        pMat->GetDimensions(nC, nR);
        USHORT nMatInd;
        ScMatrix* pResMat = GetNewMat(nC, nR, nMatInd);
        if (pResMat)
        {
            ULONG nCount = (ULONG) nC * nR;
            for ( ULONG i = 0; i < nCount; i++ )
                if (pMat->IsValue(i))
                    pResMat->PutDouble(pMat->GetDouble(i)*fVal, i);
                else
                    pResMat->PutString(ScGlobal::GetRscString(STR_NO_VALUE), i);
            PushMatrix(pResMat);
            nRetMat = nMatInd;
        }
        else
            SetNoValue();
    }
    else
        PushDouble(fVal1 * fVal2);
    if ( nFmtCurrencyType == NUMBERFORMAT_CURRENCY )
    {
        nFuncFmtType = nFmtCurrencyType;
        nFuncFmtIndex = nFmtCurrencyIndex;
    }
}

void ScInterpreter::ScDiv()
{
    ScMatrix* pMat1 = NULL;
    ScMatrix* pMat2 = NULL;
    double fVal1, fVal2;
    USHORT nMatInd1, nMatInd2;
    short nFmtCurrencyType = nCurFmtType;
    ULONG nFmtCurrencyIndex = nCurFmtIndex;
    short nFmtCurrencyType2 = NUMBERFORMAT_UNDEFINED;
    MatrixDoubleRefToMatrix();
    if ( GetStackType() == svMatrix )
        pMat2 = GetMatrix(nMatInd2);
    else
    {
        fVal2 = GetDouble();
        // hier kein Currency uebernehmen, 123kg/456DM sind nicht DM
        nFmtCurrencyType2 = nCurFmtType;
    }
    MatrixDoubleRefToMatrix();
    if ( GetStackType() == svMatrix )
        pMat1 = GetMatrix(nMatInd1);
    else
    {
        fVal1 = GetDouble();
        switch ( nCurFmtType )
        {
            case NUMBERFORMAT_CURRENCY :
                nFmtCurrencyType = nCurFmtType;
                nFmtCurrencyIndex = nCurFmtIndex;
            break;
        }
    }
    if (pMat1 && pMat2)
    {
        ScMatrix* pResMat = MatDiv(pMat1, pMat2);
        if (!pResMat)
            SetNoValue();
        else
            PushMatrix(pResMat);
    }
    else if (pMat1 || pMat2)
    {
        double fVal;
        BOOL bFlag;
        ScMatrix* pMat = pMat1;
        if (!pMat)
        {
            fVal = fVal1;
            pMat = pMat2;
            bFlag = TRUE;           // double - Matrix
        }
        else
        {
            fVal = fVal2;
            bFlag = FALSE;          // Matrix - double
        }
        USHORT nC, nR;
        pMat->GetDimensions(nC, nR);
        USHORT nMatInd;
        ScMatrix* pResMat = GetNewMat(nC, nR, nMatInd);
        if (pResMat)
        {
            ULONG nCount = (ULONG) nC * nR;
            if (bFlag)
            {   for ( ULONG i = 0; i < nCount; i++ )
                    if (pMat->IsValue(i))
                        pResMat->PutDouble(fVal / pMat->GetDouble(i), i);
                    else
                        pResMat->PutString(ScGlobal::GetRscString(STR_NO_VALUE), i);
            }
            else
            {   for ( ULONG i = 0; i < nCount; i++ )
                    if (pMat->IsValue(i))
                        pResMat->PutDouble(pMat->GetDouble(i) / fVal, i);
                    else
                        pResMat->PutString(ScGlobal::GetRscString(STR_NO_VALUE), i);
            }
            PushMatrix(pResMat);
            nRetMat = nMatInd;
        }
        else
            SetNoValue();
    }
    else
        PushDouble(fVal1 / fVal2);
    if ( nFmtCurrencyType == NUMBERFORMAT_CURRENCY && nFmtCurrencyType2 != NUMBERFORMAT_CURRENCY )
    {   // auch DM/DM ist nicht DM bzw. DEM/EUR nicht DEM
        nFuncFmtType = nFmtCurrencyType;
        nFuncFmtIndex = nFmtCurrencyIndex;
    }
}

void ScInterpreter::ScPower()
{
    if ( MustHaveParamCount( GetByte(), 2 ) )
        ScPow();
}

void ScInterpreter::ScPow()
{
    ScMatrix* pMat1 = NULL;
    ScMatrix* pMat2 = NULL;
    double fVal1, fVal2;
    USHORT nMatInd1, nMatInd2;
    MatrixDoubleRefToMatrix();
    if ( GetStackType() == svMatrix )
        pMat2 = GetMatrix(nMatInd2);
    else
        fVal2 = GetDouble();
    MatrixDoubleRefToMatrix();
    if ( GetStackType() == svMatrix )
        pMat1 = GetMatrix(nMatInd1);
    else
        fVal1 = GetDouble();
    if (pMat1 && pMat2)
    {
        ScMatrix* pResMat = MatPow(pMat1, pMat2);
        if (!pResMat)
            SetNoValue();
        else
            PushMatrix(pResMat);
    }
    else if (pMat1 || pMat2)
    {
        double fVal;
        BOOL bFlag;
        ScMatrix* pMat = pMat1;
        if (!pMat)
        {
            fVal = fVal1;
            pMat = pMat2;
            bFlag = TRUE;           // double - Matrix
        }
        else
        {
            fVal = fVal2;
            bFlag = FALSE;          // Matrix - double
        }
        USHORT nC, nR;
        pMat->GetDimensions(nC, nR);
        USHORT nMatInd;
        ScMatrix* pResMat = GetNewMat(nC, nR, nMatInd);
        if (pResMat)
        {
            ULONG nCount = (ULONG) nC * nR;
            if (bFlag)
            {   for ( ULONG i = 0; i < nCount; i++ )
                    if (pMat->IsValue(i))
                        pResMat->PutDouble(pow(fVal,pMat->GetDouble(i)), i);
                    else
                        pResMat->PutString(ScGlobal::GetRscString(STR_NO_VALUE), i);
            }
            else
            {   for ( ULONG i = 0; i < nCount; i++ )
                    if (pMat->IsValue(i))
                        pResMat->PutDouble(pow(pMat->GetDouble(i),fVal), i);
                    else
                        pResMat->PutString(ScGlobal::GetRscString(STR_NO_VALUE), i);
            }
            PushMatrix(pResMat);
            nRetMat = nMatInd;
        }
        else
            SetNoValue();
    }
    else
        PushDouble(pow(fVal1,fVal2));
}

void ScInterpreter::ScSumProduct()
{
    BYTE nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 1, 30 ) )
        return;

    ScMatrix* pMat1 = NULL;
    ScMatrix* pMat2 = NULL;
    ScMatrix* pMat  = NULL;
    USHORT nMatInd1, nMatInd2, i, j;
    pMat2 = GetMatrix(nMatInd2);
    if (!pMat2)
    {
        SetIllegalParameter();
        return;
    }
    USHORT nC, nR, nC1, nR1;
    pMat2->GetDimensions(nC, nR);
    pMat = pMat2;
    for (i = 1; i < nParamCount; i++)
    {
        pMat1 = GetMatrix(nMatInd1);
        if (!pMat1)
        {
            SetIllegalParameter();
            return;
        }
        pMat1->GetDimensions(nC1, nR1);
        if (nC1 != nC || nR1 != nR)
        {
            SetNoValue();
            return;
        }
        ScMatrix* pResMat = MatMul(pMat1, pMat);
        if (!pResMat)
        {
            SetNoValue();
            return;
        }
        else
            pMat = pResMat;
    }
    double fSum = 0.0;
    for (i = 0; i < nC; i++)
        for (j = 0; j < nR; j++)
            if (!pMat->IsString(i,j))
                fSum += pMat->GetDouble(i,j);
    PushDouble(fSum);
}

void ScInterpreter::ScSumX2MY2()
{
    if ( !MustHaveParamCount( GetByte(), 2 ) )
        return;

    ScMatrix* pMat1 = NULL;
    ScMatrix* pMat2 = NULL;
    USHORT nMatInd1, nMatInd2, i, j;
    pMat2 = GetMatrix(nMatInd2);
    pMat1 = GetMatrix(nMatInd1);
    if (!pMat2 || !pMat1)
    {
        SetIllegalParameter();
        return;
    }
    USHORT nC2, nR2, nC1, nR1;
    pMat2->GetDimensions(nC2, nR2);
    pMat1->GetDimensions(nC1, nR1);
    if (nC1 != nC2 || nR1 != nR2)
    {
        SetNoValue();
        return;
    }
    double fVal, fSum = 0.0;
    for (i = 0; i < nC1; i++)
        for (j = 0; j < nR1; j++)
            if (!pMat1->IsString(i,j) && !pMat2->IsString(i,j))
            {
                fVal = pMat1->GetDouble(i,j);
                fSum += fVal * fVal;
                fVal = pMat2->GetDouble(i,j);
                fSum -= fVal * fVal;
            }
    PushDouble(fSum);
}

void ScInterpreter::ScSumX2DY2()
{
    if ( !MustHaveParamCount( GetByte(), 2 ) )
        return;

    ScMatrix* pMat1 = NULL;
    ScMatrix* pMat2 = NULL;
    USHORT nMatInd1, nMatInd2, i, j;
    pMat2 = GetMatrix(nMatInd2);
    pMat1 = GetMatrix(nMatInd1);
    if (!pMat2 || !pMat1)
    {
        SetIllegalParameter();
        return;
    }
    USHORT nC2, nR2, nC1, nR1;
    pMat2->GetDimensions(nC2, nR2);
    pMat1->GetDimensions(nC1, nR1);
    if (nC1 != nC2 || nR1 != nR2)
    {
        SetNoValue();
        return;
    }
    double fVal, fSum = 0.0;
    for (i = 0; i < nC1; i++)
        for (j = 0; j < nR1; j++)
            if (!pMat1->IsString(i,j) && !pMat2->IsString(i,j))
            {
                fVal = pMat1->GetDouble(i,j);
                fSum += fVal * fVal;
                fVal = pMat2->GetDouble(i,j);
                fSum += fVal * fVal;
            }
    PushDouble(fSum);
}

void ScInterpreter::ScSumXMY2()
{
    if ( !MustHaveParamCount( GetByte(), 2 ) )
        return;

    ScMatrix* pMat1 = NULL;
    ScMatrix* pMat2 = NULL;
    USHORT nMatInd1, nMatInd2, i, j;
    pMat2 = GetMatrix(nMatInd2);
    pMat1 = GetMatrix(nMatInd1);
    if (!pMat2 || !pMat1)
    {
        SetIllegalParameter();
        return;
    }
    USHORT nC2, nR2, nC1, nR1;
    pMat2->GetDimensions(nC2, nR2);
    pMat1->GetDimensions(nC1, nR1);
    if (nC1 != nC2 || nR1 != nR2)
    {
        SetNoValue();
        return;
    }
    ScMatrix* pResMat = MatSub(pMat1, pMat2);
    if (!pResMat)
    {
        SetNoValue();
        return;
    }
    else
    {
        double fVal, fSum = 0.0;
        for (i = 0; i < nC1; i++)
            for (j = 0; j < nR1; j++)
                if (!pResMat->IsString(i,j))
                {
                    fVal = pResMat->GetDouble(i,j);
                    fSum += fVal * fVal;
                }
        PushDouble(fSum);
    }
}

void ScInterpreter::ScFrequency()
{
    if ( !MustHaveParamCount( GetByte(), 2 ) )
        return;

    USHORT nMatInd;
    double* pSortArray1 = NULL;
    ULONG nSize1 = 0;
    GetSortArray(1, &pSortArray1, nSize1);
    if (nGlobalError)
        SetNoValue();
    double* pSortArray2 = NULL;
    ULONG nSize2 = 0;
    GetSortArray(1, &pSortArray2, nSize2);
    if (!pSortArray2 || nSize2 == 0 || nGlobalError)
    {
        if (pSortArray1)
            delete pSortArray1;
        if (pSortArray2)
            delete pSortArray2;
        SetNoValue();
        return;
    }
    ScMatrix* pResMat = GetNewMat(1,(USHORT)nSize1+1,nMatInd);
    if (!pResMat)
    {
        if (pSortArray1)
            delete pSortArray1;
        if (pSortArray2)
            delete pSortArray2;
        SetNoValue();
        return;
    }

    USHORT j;
    ULONG i = 0;
    ULONG nCount;
    for (j = 0; j < nSize1; j++)
    {
        nCount = 0;
        while (i < nSize2 && pSortArray2[i] <= pSortArray1[j])
        {
            nCount++;
            i++;
        }
        pResMat->PutDouble((double) nCount, 0, j);
    }
    pResMat->PutDouble((double) (nSize2-i), 0, j);
    if (pSortArray1)
        delete pSortArray1;
    if (pSortArray2)
        delete pSortArray2;
    PushMatrix(pResMat);
    nRetMat = nMatInd;
}

BOOL ScInterpreter::RGetVariances( ScMatrix* pV, ScMatrix* pX,
        USHORT nC, USHORT nR, BOOL bSwapColRow, BOOL bZeroConstant )
{   // multiple Regression: Varianzen der Koeffizienten
    // bSwapColRow==TRUE : Koeffizienten in Zeilen statt Spalten angeordnet
    USHORT i, j, k, nMatInd;
    double sum;
    ScMatrix* pC = GetNewMat(nC, nC, nMatInd);
    if ( !pC )
        return FALSE;
    // X transformiert mit X multipziert, X'X Matrix
    if ( !bZeroConstant )
    {   // in der X-Designmatrix existiert ein gedachtes X0j==1
        if ( bSwapColRow )
        {
            for ( i=0; i<nC; i++ )
            {
                for ( j=0; j<nC; j++ )
                {
                    sum = 0.0;
                    for ( k=0; k<nR; k++ )
                    {
                        sum += (j==0 ? 1 : pX->GetDouble(k,j-1))
                            * (i==0 ? 1 : pX->GetDouble(k,i-1));
                    }
                    pC->PutDouble(sum, i, j);
                }
            }
        }
        else
        {
            for ( i=0; i<nC; i++ )
            {
                for ( j=0; j<nC; j++ )
                {
                    sum = 0.0;
                    for ( k=0; k<nR; k++ )
                    {
                        sum += (j==0 ? 1 : pX->GetDouble(j-1,k))
                            * (i==0 ? 1 : pX->GetDouble(i-1,k));
                    }
                    pC->PutDouble(sum, i, j);
                }
            }
        }
    }
    else
    {
        if ( bSwapColRow )
        {
            for ( i=0; i<nC; i++ )
            {
                for ( j=0; j<nC; j++ )
                {
                    sum = 0.0;
                    for ( k=0; k<nR; k++ )
                    {
                        sum += pX->GetDouble(k,j) * pX->GetDouble(k,i);
                    }
                    pC->PutDouble(sum, i, j);
                }
            }
        }
        else
        {
            for ( i=0; i<nC; i++ )
            {
                for ( j=0; j<nC; j++ )
                {
                    sum = 0.0;
                    for ( k=0; k<nR; k++ )
                    {
                        sum += pX->GetDouble(j,k) * pX->GetDouble(i,k);
                    }
                    pC->PutDouble(sum, i, j);
                }
            }
        }
    }
    // X'X Inverse
    BOOL bOk = TRUE;
    USHORT nErr = nGlobalError;
    PushMatrix(pC);
    BYTE nTmp = cPar;
    cPar = 1;
    ScMatInv();
    cPar = nTmp;
    if ( nGlobalError )
    {
        nGlobalError = nErr;
        bOk = FALSE;
    }
    else
    {
        Pop();      // pC bleibt erhalten
        // Varianzen auf der Diagonalen, andere sind Kovarianzen
        for (i = 0; i < nC; i++)
            pV->PutDouble(pC->GetDouble(i, i), i);
    }
    delete pC;
    ResetNewMat(nMatInd);
    return bOk;
}

void ScInterpreter::ScRGP()
{
    BYTE nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 1, 4 ) )
        return;
    BOOL bConstant, bStats;
    if (nParamCount == 4)
        bStats = GetBool();
    else
        bStats = FALSE;
    if (nParamCount >= 3)
        bConstant = GetBool();
    else
        bConstant = TRUE;
    USHORT nMatInd1, nMatInd2;
    ScMatrix* pMatX;
    ScMatrix* pMatY;
    if (nParamCount >= 2)
        pMatX = GetMatrix(nMatInd2);
    else
        pMatX = NULL;
    pMatY = GetMatrix(nMatInd1);
    if (!pMatY)
    {
        SetIllegalParameter();
        return;
    }
    BYTE nCase;                         // 1 = normal, 2,3 = mehrfach
    USHORT nCX, nRX, nCY, nRY, M, N;
    pMatY->GetDimensions(nCY, nRY);
    ULONG nCountY = (ULONG) nCY * nRY;
    for ( ULONG i = 0; i < nCountY; i++ )
        if (!pMatY->IsValue(i))
        {
            SetIllegalArgument();
            return;
        }
    if (pMatX)
    {
        pMatX->GetDimensions(nCX, nRX);
        ULONG nCountX = (ULONG) nCX * nRX;
        for ( ULONG i = 0; i < nCountX; i++ )
            if (!pMatX->IsValue(i))
            {
                SetIllegalArgument();
                return;
            }
        if (nCX == nCY && nRX == nRY)
            nCase = 1;                  // einfache Regression
        else if (nCY != 1 && nRY != 1)
        {
            SetIllegalParameter();
            return;
        }
        else if (nCY == 1)
        {
            if (nRX != nRY)
            {
                SetIllegalParameter();
                return;
            }
            else
            {
                nCase = 2;              // zeilenweise
                N = nRY;
                M = nCX;
            }
        }
        else if (nCX != nCY)
        {
            SetIllegalParameter();
            return;
        }
        else
        {
            nCase = 3;                  // spaltenweise
            N = nCY;
            M = nRX;
        }
    }
    else
    {
        pMatX = GetNewMat(nCY, nRY, nMatInd2);
        if (!pMatX)
        {
            SetIllegalParameter();
            return;
        }
        for ( ULONG i = 1; i <= nCountY; i++ )
            pMatX->PutDouble((double)i, i-1);
        nCase = 1;
    }
    USHORT nMatInd;
    ScMatrix* pResMat;
    if (nCase == 1)
    {
        if (!bStats)
            pResMat = GetNewMat(2,1,nMatInd);
        else
            pResMat = GetNewMat(2,5,nMatInd);
        if (!pResMat)
        {
            SetIllegalParameter();
            return;
        }
        double fCount   = 0.0;
        double fSumX    = 0.0;
        double fSumSqrX = 0.0;
        double fSumY    = 0.0;
        double fSumSqrY = 0.0;
        double fSumXY   = 0.0;
        double fValX, fValY;
        for (USHORT i = 0; i < nCY; i++)
            for (USHORT j = 0; j < nRY; j++)
            {
                fValX = pMatX->GetDouble(i,j);
                fValY = pMatY->GetDouble(i,j);
                fSumX    += fValX;
                fSumSqrX += fValX * fValX;
                fSumY    += fValY;
                fSumSqrY += fValY * fValY;
                fSumXY   += fValX*fValY;
                fCount++;
            }
        if (fCount < 1.0)
            SetNoValue();
        else
        {
            double f1 = fCount*fSumXY-fSumX*fSumY;
            double fX = fCount*fSumSqrX-fSumX*fSumX;
            double b, m;
            if (bConstant)
            {
                b = fSumY/fCount - f1/fX*fSumX/fCount;
                m = f1/fX;
            }
            else
            {
                b = 0.0;
                m = fSumXY/fSumSqrX;
            }
            pResMat->PutDouble(m, 0, 0);
            pResMat->PutDouble(b, 1, 0);
            if (bStats)
            {
                double fY = fCount*fSumSqrY-fSumY*fSumY;
                double fSyx = fSumSqrY-b*fSumY-m*fSumXY;
                double fR2 = f1*f1/(fX*fY);
                pResMat->PutDouble (fR2, 0, 2);
                if (fCount < 3.0)
                {
                    pResMat->PutString(ScGlobal::GetRscString(STR_NO_VALUE), 0, 1 );
                    pResMat->PutString(ScGlobal::GetRscString(STR_NO_VALUE), 1, 1 );
                    pResMat->PutString(ScGlobal::GetRscString(STR_NO_VALUE), 1, 2 );
                    pResMat->PutString(ScGlobal::GetRscString(STR_NO_VALUE), 0, 3 );
                }
                else
                {
                    pResMat->PutDouble(sqrt(fSyx*fCount/(fX*(fCount-2.0))), 0, 1);
                    pResMat->PutDouble(sqrt(fSyx*fSumSqrX/fX/(fCount-2.0)), 1, 1);
                    pResMat->PutDouble(
                        sqrt((fCount*fSumSqrY - fSumY*fSumY - f1*f1/fX)/
                             (fCount*(fCount-2.0))), 1, 2);
                    if (fR2 == 1.0)
                        pResMat->PutString(ScGlobal::GetRscString(STR_NO_VALUE), 0, 3 );
                    else
                        pResMat->PutDouble(fR2*(fCount-2.0)/(1.0-fR2), 0, 3);
                }
                pResMat->PutDouble(((double)(nCY*nRY))-2.0, 1, 3);
                pResMat->PutDouble(fY/fCount-fSyx, 0, 4);
                pResMat->PutDouble(fSyx, 1, 4);
            }
        }
    }
    else
    {
        USHORT nMatInd10, nMatInd12, nMatInd13, i, j, k;
        if (!bStats)
            pResMat = GetNewMat(M+1,1,nMatInd);
        else
            pResMat = GetNewMat(M+1,5,nMatInd);
        if (!pResMat)
        {
            SetIllegalParameter();
            return;
        }
        BOOL bVariancesOk = TRUE;
        ScMatrix* pQ = GetNewMat(M+1, M+2, nMatInd10);
        ScMatrix* pE = GetNewMat(M+2, 1, nMatInd12);
        ScMatrix* pV = GetNewMat(M+1, 1, nMatInd13);
        pE->PutDouble(0.0, M+1);
        pQ->FillDouble(0.0, 0, 0, M, M+1);
        if (nCase == 2)
        {
            for (k = 0; k < N; k++)
            {
                double Yk = pMatY->GetDouble(k);
                pE->PutDouble( pE->GetDouble(M+1)+Yk*Yk, M+1 );
                double sumYk = pQ->GetDouble(0, M+1) + Yk;
                pQ->PutDouble( sumYk, 0, M+1 );
                pE->PutDouble( sumYk, 0 );
                for (i = 0; i < M; i++)
                {
                    double Xik = pMatX->GetDouble(i,k);
                    double sumXik = pQ->GetDouble(0, i+1) + Xik;
                    pQ->PutDouble( sumXik, 0, i+1);
                    pQ->PutDouble( sumXik, i+1, 0);
                    double sumXikYk = pQ->GetDouble(i+1, M+1) + Xik * Yk;
                    pQ->PutDouble( sumXikYk, i+1, M+1);
                    pE->PutDouble( sumXikYk, i+1);
                    for (j = i; j < M; j++)
                    {
                        double sumXikXjk = pQ->GetDouble(j+1, i+1) +
                             Xik * pMatX->GetDouble(j,k);
                        pQ->PutDouble( sumXikXjk, j+1, i+1);
                        pQ->PutDouble( sumXikXjk, i+1, j+1);
                    }
                }
            }
        }
        else
        {
            for (k = 0; k < N; k++)
            {
                double Yk = pMatY->GetDouble(k);
                pE->PutDouble( pE->GetDouble(M+1)+Yk*Yk, M+1 );
                double sumYk = pQ->GetDouble(0, M+1) + Yk;
                pQ->PutDouble( sumYk, 0, M+1 );
                pE->PutDouble( sumYk, 0 );
                for (i = 0; i < M; i++)
                {
                    double Xki = pMatX->GetDouble(k,i);
                    double sumXki = pQ->GetDouble(0, i+1) + Xki;
                    pQ->PutDouble( sumXki, 0, i+1);
                    pQ->PutDouble( sumXki, i+1, 0);
                    double sumXkiYk = pQ->GetDouble(i+1, M+1) + Xki * Yk;
                    pQ->PutDouble( sumXkiYk, i+1, M+1);
                    pE->PutDouble( sumXkiYk, i+1);
                    for (j = i; j < M; j++)
                    {
                        double sumXkiXkj = pQ->GetDouble(j+1, i+1) +
                             Xki * pMatX->GetDouble(k,j);
                        pQ->PutDouble( sumXkiXkj, j+1, i+1);
                        pQ->PutDouble( sumXkiXkj, i+1, j+1);
                    }
                }
            }
        }
        pQ->PutDouble((double)N, 0, 0);
        if (bConstant)
        {
            USHORT S, L;
            for (S = 0; S < M+1; S++)
            {
                i = S;
                while (i < M+1 && pQ->GetDouble(i, S) == 0.0)
                    i++;
                if (i >= M+1)
                {
                    SetNoValue();
                    delete pQ;
                    delete pE;
                    delete pV;
                    ResetNewMat(nMatInd13);
                    ResetNewMat(nMatInd12);
                    ResetNewMat(nMatInd10);
                    return;
                }
                double fVal;
                for (L = 0; L < M+2; L++)
                {
                    fVal = pQ->GetDouble(S, L);
                    pQ->PutDouble(pQ->GetDouble(i, L), S, L);
                    pQ->PutDouble(fVal, i, L);
                }
                fVal = 1.0/pQ->GetDouble(S, S);
                for (L = 0; L < M+2; L++)
                    pQ->PutDouble(pQ->GetDouble(S, L)*fVal, S, L);
                for (i = 0; i < M+1; i++)
                {
                    if (i != S)
                    {
                        fVal = -pQ->GetDouble(i, S);
                        for (L = 0; L < M+2; L++)
                            pQ->PutDouble(
                                pQ->GetDouble(i,L)+fVal*pQ->GetDouble(S,L),i,L);
                    }
                }
            }
        }
        else
        {
            USHORT S, L;
            for (S = 1; S < M+1; S++)
            {
                i = S;
                while (i < M+1 && pQ->GetDouble(i, S) == 0.0)
                    i++;
                if (i >= M+1)
                {
                    SetNoValue();
                    delete pQ;
                    delete pE;
                    delete pV;
                    ResetNewMat(nMatInd13);
                    ResetNewMat(nMatInd12);
                    ResetNewMat(nMatInd10);
                    return;
                }
                double fVal;
                for (L = 1; L < M+2; L++)
                {
                    fVal = pQ->GetDouble(S, L);
                    pQ->PutDouble(pQ->GetDouble(i, L), S, L);
                    pQ->PutDouble(fVal, i, L);
                }
                fVal = 1.0/pQ->GetDouble(S, S);
                for (L = 1; L < M+2; L++)
                    pQ->PutDouble(pQ->GetDouble(S, L)*fVal, S, L);
                for (i = 1; i < M+1; i++)
                {
                    if (i != S)
                    {
                        fVal = -pQ->GetDouble(i, S);
                        for (L = 1; L < M+2; L++)
                            pQ->PutDouble(
                                pQ->GetDouble(i,L)+fVal*pQ->GetDouble(S,L),i,L);
                    }
                }
                pQ->PutDouble(0.0, 0, M+1);     // Konstante b
            }
        }
        // mn ... m1, b
        for (i = 0; i < M+1; i++)
            pResMat->PutDouble(pQ->GetDouble(M-i,M+1), i, 0);
        if (bStats)
        {
            // pE[0]    := Sigma i=1...n (Yi)
            // pE[k]    := Sigma i=1...n (Xki*Yi)
            // pE[M+1]  := Sigma i=1...n (Yi**2)
            // pQ[0,M+1]:= B
            // pQ[k,M+1]:= Mk
            double fSQR, fSQT, fSQE;
            fSQT = pE->GetDouble(M+1)
                - pE->GetDouble(0) * pE->GetDouble(0) / (double)N;
            fSQR = pE->GetDouble(M+1);
            for (i = 0; i < M+1; i++)
                fSQR -= pQ->GetDouble(i, M+1) * pE->GetDouble(i);
            fSQE = fSQT-fSQR;
            // r2 (Bestimmtheitsmass, 0...1)
            if (fSQT == 0.0)
                pResMat->PutString(ScGlobal::GetRscString(STR_NO_VALUE), 0, 2);
            else
                pResMat->PutDouble (fSQE/fSQT, 0, 2);
            // ssReg (Regressions-Quadratsumme)
            pResMat->PutDouble(fSQE, 0, 4);
            // ssResid (Residual-Quadratsumme, Summe der Abweichungsquadrate)
            pResMat->PutDouble(fSQR, 1, 4);
            for (i = 2; i < 5; i++)
                for (j = 2; j < M+1; j++)
                    pResMat->PutString(ScGlobal::GetRscString(STR_NV_STR), j, i);
            if (bConstant)
            {
                if (N-M-1 == 0)
                {
                    pResMat->PutString(ScGlobal::GetRscString(STR_NO_VALUE), 1, 2);
                    for (i = 0; i < M+1; i++)
                        pResMat->PutString(ScGlobal::GetRscString(STR_NO_VALUE), i, 1);
                }
                else
                {
                    double fSE2 = fSQR/(N-M-1);
                    // sey (Standardfehler des Schaetzwertes y)
                    pResMat->PutDouble(sqrt(fSE2), 1, 2);
                    // sen...se1 (Standardfehler der Koeffizienten mn...m1)
                    // seb (Standardfehler der Konstanten b)
                    if ( RGetVariances( pV, pMatX, M+1, N, nCase != 2, FALSE ) )
                    {
                        for (i = 0; i < M+1; i++)
                            pResMat->PutDouble( sqrt(fSE2 * pV->GetDouble(i)), M-i, 1 );
                    }
                    else
                    {
                        for (i = 0; i < M+1; i++)
                            pResMat->PutString(ScGlobal::GetRscString(STR_NV_STR), i, 1);
                    }
                }
                // F (F-Statistik)
                if (fSQR == 0.0)
                    pResMat->PutString(ScGlobal::GetRscString(STR_NO_VALUE), 0, 3);
                else
                    pResMat->PutDouble(((double)(N-M-1))*fSQE/fSQR/((double)M),0, 3);
                // df (Freiheitsgrad)
                pResMat->PutDouble(((double)(N-M-1)), 1, 3);
            }
            else
            {
                if (N-M == 0)
                {
                    pResMat->PutString(ScGlobal::GetRscString(STR_NO_VALUE), 1, 2);
                    for (i = 0; i < M+1; i++)
                        pResMat->PutString(ScGlobal::GetRscString(STR_NO_VALUE), i, 1);
                }
                else
                {
                    double fSE2 = fSQR/(N-M);
                    pResMat->PutDouble(sqrt(fSE2), 1, 2);
                    if ( RGetVariances( pV, pMatX, M, N, nCase != 2, TRUE ) )
                    {
                        for (i = 0; i < M; i++)
                            pResMat->PutDouble( sqrt(fSE2 * pV->GetDouble(i)), M-i-1, 1 );
                        pResMat->PutString(ScGlobal::GetRscString(STR_NV_STR), M, 1);
                    }
                    else
                    {
                        for (i = 0; i < M+1; i++)
                            pResMat->PutString(ScGlobal::GetRscString(STR_NV_STR), i, 1);
                    }
                }
                if (fSQR == 0.0)
                    pResMat->PutString(ScGlobal::GetRscString(STR_NO_VALUE), 0, 3);
                else
                    pResMat->PutDouble(((double)(N-M))*fSQE/fSQR/((double)M),0, 3);
                pResMat->PutDouble(((double)(N-M)), 1, 3);
            }
        }
        delete pQ;
        delete pE;
        delete pV;
        ResetNewMat(nMatInd13);
        ResetNewMat(nMatInd12);
        ResetNewMat(nMatInd10);
    }
    PushMatrix(pResMat);
    nRetMat = nMatInd;
}

void ScInterpreter::ScRKP()
{
    BYTE nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 1, 4 ) )
        return;
    BOOL bConstant, bStats;
    if (nParamCount == 4)
        bStats = GetBool();
    else
        bStats = FALSE;
    if (nParamCount >= 3)
        bConstant = GetBool();
    else
        bConstant = TRUE;
    USHORT nMatInd1, nMatInd2;
    ScMatrix* pMatX;
    ScMatrix* pMatY;
    if (nParamCount >= 2)
        pMatX = GetMatrix(nMatInd2);
    else
        pMatX = NULL;
    pMatY = GetMatrix(nMatInd1);
    if (!pMatY)
    {
        SetIllegalParameter();
        return;
    }
    BYTE nCase;                         // 1 = normal, 2,3 = mehrfach
    USHORT nCX, nRX, nCY, nRY, M, N;
    pMatY->GetDimensions(nCY, nRY);
    ULONG nCountY = (ULONG) nCY * nRY;
    ULONG i;
    for (i = 0; i < nCountY; i++)
        if (!pMatY->IsValue(i))
        {
            SetIllegalArgument();
            return;
        }
    for (i = 0; i < nCountY; i++)
    {
        if (pMatY->GetDouble(i) <= 0.0)
        {
            SetIllegalArgument();
            return;
        }
        else
            pMatY->PutDouble(log(pMatY->GetDouble(i)), i);
    }
    if (pMatX)
    {
        pMatX->GetDimensions(nCX, nRX);
        ULONG nCountX = (ULONG) nCX * nRX;
        for (i = 0; i < nCountX; i++)
            if (!pMatX->IsValue(i))
            {
                SetIllegalArgument();
                return;
            }
        if (nCX == nCY && nRX == nRY)
            nCase = 1;                  // einfache Regression
        else if (nCY != 1 && nRY != 1)
        {
            SetIllegalParameter();
            return;
        }
        else if (nCY == 1)
        {
            if (nRX != nRY)
            {
                SetIllegalParameter();
                return;
            }
            else
            {
                nCase = 2;              // zeilenweise
                N = nRY;
                M = nCX;
            }
        }
        else if (nCX != nCY)
        {
            SetIllegalParameter();
            return;
        }
        else
        {
            nCase = 3;                  // spaltenweise
            N = nCY;
            M = nRX;
        }
    }
    else
    {
        pMatX = GetNewMat(nCY, nRY, nMatInd2);
        if (!pMatX)
        {
            SetIllegalParameter();
            return;
        }
        for ( ULONG i = 1; i <= nCountY; i++ )
            pMatX->PutDouble((double)i, i-1);
        nCase = 1;
    }
    USHORT nMatInd;
    ScMatrix* pResMat;
    if (nCase == 1)
    {
        if (!bStats)
            pResMat = GetNewMat(2,1,nMatInd);
        else
            pResMat = GetNewMat(2,5,nMatInd);
        if (!pResMat)
        {
            SetIllegalParameter();
            return;
        }
        double fCount   = 0.0;
        double fSumX    = 0.0;
        double fSumSqrX = 0.0;
        double fSumY    = 0.0;
        double fSumSqrY = 0.0;
        double fSumXY   = 0.0;
        double fValX, fValY;
        for (USHORT i = 0; i < nCY; i++)
            for (USHORT j = 0; j < nRY; j++)
            {
                fValX = pMatX->GetDouble(i,j);
                fValY = pMatY->GetDouble(i,j);
                fSumX    += fValX;
                fSumSqrX += fValX * fValX;
                fSumY    += fValY;
                fSumSqrY += fValY * fValY;
                fSumXY   += fValX*fValY;
                fCount++;
            }
        if (fCount < 1.0)
            SetNoValue();
        else
        {
            double f1 = fCount*fSumXY-fSumX*fSumY;
            double fX = fCount*fSumSqrX-fSumX*fSumX;
            double b, m;
            if (bConstant)
            {
                b = fSumY/fCount - f1/fX*fSumX/fCount;
                m = f1/fX;
            }
            else
            {
                b = 0.0;
                m = fSumXY/fSumSqrX;
            }
            pResMat->PutDouble(exp(m), 0, 0);
            pResMat->PutDouble(exp(b), 1, 0);
            if (bStats)
            {
                double fY = fCount*fSumSqrY-fSumY*fSumY;
                double fSyx = fSumSqrY-b*fSumY-m*fSumXY;
                double fR2 = f1*f1/(fX*fY);
                pResMat->PutDouble (fR2, 0, 2);
                if (fCount < 3.0)
                {
                    pResMat->PutString(ScGlobal::GetRscString(STR_NO_VALUE), 0, 1 );
                    pResMat->PutString(ScGlobal::GetRscString(STR_NO_VALUE), 1, 1 );
                    pResMat->PutString(ScGlobal::GetRscString(STR_NO_VALUE), 1, 2 );
                    pResMat->PutString(ScGlobal::GetRscString(STR_NO_VALUE), 0, 3 );
                }
                else
                {
                    pResMat->PutDouble(sqrt(fSyx*fCount/(fX*(fCount-2.0))), 0, 1);
                    pResMat->PutDouble(sqrt(fSyx*fSumSqrX/fX/(fCount-2.0)), 1, 1);
                    pResMat->PutDouble(
                        sqrt((fCount*fSumSqrY - fSumY*fSumY - f1*f1/fX)/
                             (fCount*(fCount-2.0))), 1, 2);
                    if (fR2 == 1.0)
                        pResMat->PutString(ScGlobal::GetRscString(STR_NO_VALUE), 0, 3 );
                    else
                        pResMat->PutDouble(fR2*(fCount-2.0)/(1.0-fR2), 0, 3);
                }
                pResMat->PutDouble(((double)(nCY*nRY))-2.0, 1, 3);
                pResMat->PutDouble(fY/fCount-fSyx, 0, 4);
                pResMat->PutDouble(fSyx, 1, 4);
            }
        }
    }
    else
    {
        USHORT nMatInd10, nMatInd12, nMatInd13, i, j, k;
        if (!bStats)
            pResMat = GetNewMat(M+1,1,nMatInd);
        else
            pResMat = GetNewMat(M+1,5,nMatInd);
        if (!pResMat)
        {
            SetIllegalParameter();
            return;
        }
        ScMatrix* pQ = GetNewMat(M+1, M+2, nMatInd10);
        ScMatrix* pE = GetNewMat(M+2, 1, nMatInd12);
        ScMatrix* pV = GetNewMat(M+1, 1, nMatInd13);
        pE->PutDouble(0.0, M+1);
        pQ->FillDouble(0.0, 0, 0, M, M+1);
        if (nCase == 2)
        {
            for (k = 0; k < N; k++)
            {
                double Yk = pMatY->GetDouble(k);
                pE->PutDouble( pE->GetDouble(M+1)+Yk*Yk, M+1 );
                double sumYk = pQ->GetDouble(0, M+1) + Yk;
                pQ->PutDouble( sumYk, 0, M+1 );
                pE->PutDouble( sumYk, 0 );
                for (i = 0; i < M; i++)
                {
                    double Xik = pMatX->GetDouble(i,k);
                    double sumXik = pQ->GetDouble(0, i+1) + Xik;
                    pQ->PutDouble( sumXik, 0, i+1);
                    pQ->PutDouble( sumXik, i+1, 0);
                    double sumXikYk = pQ->GetDouble(i+1, M+1) + Xik * Yk;
                    pQ->PutDouble( sumXikYk, i+1, M+1);
                    pE->PutDouble( sumXikYk, i+1);
                    for (j = i; j < M; j++)
                    {
                        double sumXikXjk = pQ->GetDouble(j+1, i+1) +
                             Xik * pMatX->GetDouble(j,k);
                        pQ->PutDouble( sumXikXjk, j+1, i+1);
                        pQ->PutDouble( sumXikXjk, i+1, j+1);
                    }
                }
            }
        }
        else
        {
            for (k = 0; k < N; k++)
            {
                double Yk = pMatY->GetDouble(k);
                pE->PutDouble( pE->GetDouble(M+1)+Yk*Yk, M+1 );
                double sumYk = pQ->GetDouble(0, M+1) + Yk;
                pQ->PutDouble( sumYk, 0, M+1 );
                pE->PutDouble( sumYk, 0 );
                for (i = 0; i < M; i++)
                {
                    double Xki = pMatX->GetDouble(k,i);
                    double sumXki = pQ->GetDouble(0, i+1) + Xki;
                    pQ->PutDouble( sumXki, 0, i+1);
                    pQ->PutDouble( sumXki, i+1, 0);
                    double sumXkiYk = pQ->GetDouble(i+1, M+1) + Xki * Yk;
                    pQ->PutDouble( sumXkiYk, i+1, M+1);
                    pE->PutDouble( sumXkiYk, i+1);
                    for (j = i; j < M; j++)
                    {
                        double sumXkiXkj = pQ->GetDouble(j+1, i+1) +
                             Xki * pMatX->GetDouble(k,j);
                        pQ->PutDouble( sumXkiXkj, j+1, i+1);
                        pQ->PutDouble( sumXkiXkj, i+1, j+1);
                    }
                }
            }
        }
        pQ->PutDouble((double)N, 0, 0);
        if (bConstant)
        {
            USHORT S, L;
            for (S = 0; S < M+1; S++)
            {
                i = S;
                while (i < M+1 && pQ->GetDouble(i, S) == 0.0)
                    i++;
                if (i >= M+1)
                {
                    SetNoValue();
                    delete pQ;
                    delete pE;
                    delete pV;
                    ResetNewMat(nMatInd13);
                    ResetNewMat(nMatInd12);
                    ResetNewMat(nMatInd10);
                    return;
                }
                double fVal;
                for (L = 0; L < M+2; L++)
                {
                    fVal = pQ->GetDouble(S, L);
                    pQ->PutDouble(pQ->GetDouble(i, L), S, L);
                    pQ->PutDouble(fVal, i, L);
                }
                fVal = 1.0/pQ->GetDouble(S, S);
                for (L = 0; L < M+2; L++)
                    pQ->PutDouble(pQ->GetDouble(S, L)*fVal, S, L);
                for (i = 0; i < M+1; i++)
                {
                    if (i != S)
                    {
                        fVal = -pQ->GetDouble(i, S);
                        for (L = 0; L < M+2; L++)
                            pQ->PutDouble(
                                pQ->GetDouble(i,L)+fVal*pQ->GetDouble(S,L),i,L);
                    }
                }
            }
        }
        else
        {
            USHORT S, L;
            for (S = 1; S < M+1; S++)
            {
                i = S;
                while (i < M+1 && pQ->GetDouble(i, S) == 0.0)
                    i++;
                if (i >= M+1)
                {
                    SetNoValue();
                    delete pQ;
                    delete pE;
                    delete pV;
                    ResetNewMat(nMatInd13);
                    ResetNewMat(nMatInd12);
                    ResetNewMat(nMatInd10);
                    return;
                }
                double fVal;
                for (L = 1; L < M+2; L++)
                {
                    fVal = pQ->GetDouble(S, L);
                    pQ->PutDouble(pQ->GetDouble(i, L), S, L);
                    pQ->PutDouble(fVal, i, L);
                }
                fVal = 1.0/pQ->GetDouble(S, S);
                for (L = 1; L < M+2; L++)
                    pQ->PutDouble(pQ->GetDouble(S, L)*fVal, S, L);
                for (i = 1; i < M+1; i++)
                {
                    if (i != S)
                    {
                        fVal = -pQ->GetDouble(i, S);
                        for (L = 1; L < M+2; L++)
                            pQ->PutDouble(
                                pQ->GetDouble(i,L)+fVal*pQ->GetDouble(S,L),i,L);
                    }
                }
                pQ->PutDouble(0.0, 0, M+1);
            }
        }
        for (i = 0; i < M+1; i++)
            pResMat->PutDouble(exp(pQ->GetDouble(M-i,M+1)), i, 0);
        if (bStats)
        {
            double fSQR, fSQT, fSQE;
            fSQT = pE->GetDouble(M+1)-pE->GetDouble(0)*pE->GetDouble(0)/((double)N);
            fSQR = pE->GetDouble(M+1);
            for (i = 0; i < M+1; i++)
                fSQR += -pQ->GetDouble(i, M+1)*pE->GetDouble(i);
            fSQE = fSQT-fSQR;
            if (fSQT == 0.0)
                pResMat->PutString(ScGlobal::GetRscString(STR_NO_VALUE), 0, 2);
            else
                pResMat->PutDouble (fSQE/fSQT, 0, 2);
            pResMat->PutDouble(fSQE, 0, 4);
            pResMat->PutDouble(fSQR, 1, 4);
            for (i = 2; i < 5; i++)
                for (j = 2; j < M+1; j++)
                    pResMat->PutString(ScGlobal::GetRscString(STR_NV_STR), j, i);
            if (bConstant)
            {
                if (N-M-1 == 0)
                {
                    pResMat->PutString(ScGlobal::GetRscString(STR_NO_VALUE), 1, 2);
                    for (i = 0; i < M+1; i++)
                        pResMat->PutString(ScGlobal::GetRscString(STR_NO_VALUE), i, 1);
                }
                else
                {
                    double fSE2 = fSQR/(N-M-1);
                    pResMat->PutDouble(sqrt(fSE2), 1, 2);
                    if ( RGetVariances( pV, pMatX, M+1, N, nCase != 2, FALSE ) )
                    {
                        for (i = 0; i < M+1; i++)
                            pResMat->PutDouble( sqrt(fSE2 * pV->GetDouble(i)), M-i, 1 );
                    }
                    else
                    {
                        for (i = 0; i < M+1; i++)
                            pResMat->PutString(ScGlobal::GetRscString(STR_NV_STR), i, 1);
                    }
                }
                if (fSQR == 0.0)
                    pResMat->PutString(ScGlobal::GetRscString(STR_NO_VALUE), 0, 3);
                else
                    pResMat->PutDouble(((double)(N-M-1))*fSQE/fSQR/((double)M),0, 3);
                pResMat->PutDouble(((double)(N-M-1)), 1, 3);
            }
            else
            {
                if (N-M == 0)
                {
                    pResMat->PutString(ScGlobal::GetRscString(STR_NO_VALUE), 1, 2);
                    for (i = 0; i < M+1; i++)
                        pResMat->PutString(ScGlobal::GetRscString(STR_NO_VALUE), i, 1);
                }
                else
                {
                    double fSE2 = fSQR/(N-M);
                    pResMat->PutDouble(sqrt(fSE2), 1, 2);
                    if ( RGetVariances( pV, pMatX, M, N, nCase != 2, TRUE ) )
                    {
                        for (i = 0; i < M; i++)
                            pResMat->PutDouble( sqrt(fSE2 * pV->GetDouble(i)), M-i-1, 1 );
                        pResMat->PutString(ScGlobal::GetRscString(STR_NV_STR), M, 1);
                    }
                    else
                    {
                        for (i = 0; i < M+1; i++)
                            pResMat->PutString(ScGlobal::GetRscString(STR_NV_STR), i, 1);
                    }
                }
                if (fSQR == 0.0)
                    pResMat->PutString(ScGlobal::GetRscString(STR_NO_VALUE), 0, 3);
                else
                    pResMat->PutDouble(((double)(N-M))*fSQE/fSQR/((double)M),0, 3);
                pResMat->PutDouble(((double)(N-M)), 1, 3);
            }
        }
        delete pQ;
        delete pE;
        delete pV;
        ResetNewMat(nMatInd13);
        ResetNewMat(nMatInd12);
        ResetNewMat(nMatInd10);
    }
    PushMatrix(pResMat);
    nRetMat = nMatInd;
}


void ScInterpreter::ScTrend()
{
    BYTE nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 1, 4 ) )
        return;
    BOOL bConstant;
    if (nParamCount == 4)
        bConstant = GetBool();
    else
        bConstant = TRUE;
    USHORT nMatInd1, nMatInd2, nMatInd3;
    ScMatrix* pMatX;
    ScMatrix* pMatY;
    ScMatrix* pMatNewX;
    if (nParamCount >= 3)
        pMatNewX = GetMatrix(nMatInd3);
    else
        pMatNewX = NULL;
    if (nParamCount >= 2)
        pMatX = GetMatrix(nMatInd2);
    else
        pMatX = NULL;
    pMatY = GetMatrix(nMatInd1);
    if (!pMatY)
    {
        SetIllegalParameter();
        return;
    }
    BYTE nCase;                         // 1 = normal, 2,3 = mehrfach
    USHORT nCX, nRX, nCY, nRY, M, N;
    pMatY->GetDimensions(nCY, nRY);
    ULONG nCountY = (ULONG) nCY * nRY;
    ULONG i;
    for (i = 0; i < nCountY; i++)
        if (!pMatY->IsValue(i))
        {
            SetIllegalArgument();
            return;
        }
    if (pMatX)
    {
        pMatX->GetDimensions(nCX, nRX);
        ULONG nCountX = (ULONG) nCX * nRX;
        for (i = 0; i < nCountX; i++)
            if (!pMatX->IsValue(i))
            {
                SetIllegalArgument();
                return;
            }
        if (nCX == nCY && nRX == nRY)
            nCase = 1;                  // einfache Regression
        else if (nCY != 1 && nRY != 1)
        {
            SetIllegalParameter();
            return;
        }
        else if (nCY == 1)
        {
            if (nRX != nRY)
            {
                SetIllegalParameter();
                return;
            }
            else
            {
                nCase = 2;              // zeilenweise
                N = nRY;
                M = nCX;
            }
        }
        else if (nCX != nCY)
        {
            SetIllegalParameter();
            return;
        }
        else
        {
            nCase = 3;                  // spaltenweise
            N = nCY;
            M = nRX;
        }
    }
    else
    {
        pMatX = GetNewMat(nCY, nRY, nMatInd2);
        nCX = nCY;
        nRX = nRY;
        if (!pMatX)
        {
            SetIllegalParameter();
            return;
        }
        for (i = 1; i <= nCountY; i++)
            pMatX->PutDouble((double)i, i-1);
        nCase = 1;
    }
    USHORT nCXN, nRXN;
    ULONG nCountXN;
    if (!pMatNewX)
    {
        nCXN = nCX;
        nRXN = nRX;
        nCountXN = (ULONG) nCXN * nRXN;
        pMatNewX = pMatX;
    }
    else
    {
        pMatNewX->GetDimensions(nCXN, nRXN);
        if ((nCase == 2 && nCX != nCXN) || (nCase == 3 && nRX != nRXN))
        {
            SetIllegalArgument();
            return;
        }
        nCountXN = (ULONG) nCXN * nRXN;
        for ( ULONG i = 0; i < nCountXN; i++ )
            if (!pMatNewX->IsValue(i))
            {
                SetIllegalArgument();
                return;
            }
    }
    USHORT nMatInd;
    ScMatrix* pResMat;
    if (nCase == 1)
    {
        double fCount   = 0.0;
        double fSumX    = 0.0;
        double fSumSqrX = 0.0;
        double fSumY    = 0.0;
        double fSumSqrY = 0.0;
        double fSumXY   = 0.0;
        double fValX, fValY;
        for (USHORT i = 0; i < nCY; i++)
            for (USHORT j = 0; j < nRY; j++)
            {
                fValX = pMatX->GetDouble(i,j);
                fValY = pMatY->GetDouble(i,j);
                fSumX    += fValX;
                fSumSqrX += fValX * fValX;
                fSumY    += fValY;
                fSumSqrY += fValY * fValY;
                fSumXY   += fValX*fValY;
                fCount++;
            }
        if (fCount < 1.0)
        {
            SetNoValue();
            return;
        }
        else
        {
            double f1 = fCount*fSumXY-fSumX*fSumY;
            double fX = fCount*fSumSqrX-fSumX*fSumX;
            double b, m;
            if (bConstant)
            {
                b = fSumY/fCount - f1/fX*fSumX/fCount;
                m = f1/fX;
            }
            else
            {
                b = 0.0;
                m = fSumXY/fSumSqrX;
            }
            pResMat = GetNewMat(nCXN, nRXN, nMatInd);
            if (!pResMat)
            {
                SetIllegalParameter();
                return;
            }
            for (i = 0; i < nCountXN; i++)
                pResMat->PutDouble(pMatNewX->GetDouble(i)*m+b, i);
        }
    }
    else
    {
        USHORT nMatInd10, nMatInd12, i, j, k;
        ScMatrix* pQ = GetNewMat(M+1, M+2, nMatInd10);
        ScMatrix* pE = GetNewMat(M+2, 1, nMatInd12);
        pE->PutDouble(0.0, M+1);
        pQ->FillDouble(0.0, 0, 0, M, M+1);
        if (nCase == 2)
        {
            for (k = 0; k < N; k++)
            {
                pE->PutDouble(
                    pE->GetDouble(M+1)+pMatY->GetDouble(k)*pMatY->GetDouble(k), M+1);
                pQ->PutDouble(pQ->GetDouble(0, M+1) + pMatY->GetDouble(k), 0,   M+1);
                pE->PutDouble(pQ->GetDouble(0, M+1), 0);
                for (i = 0; i < M; i++)
                {
                    pQ->PutDouble(pQ->GetDouble(0, i+1)+pMatX->GetDouble(i,k), 0, i+1);
                    pQ->PutDouble(pQ->GetDouble(0, i+1), i+1, 0);
                    pQ->PutDouble(pQ->GetDouble(i+1, M+1) +
                             pMatX->GetDouble(i,k)*pMatY->GetDouble(k), i+1, M+1);
                    pE->PutDouble(pQ->GetDouble(i+1, M+1), i+1);
                    for (j = i; j < M; j++)
                    {
                        pQ->PutDouble(pQ->GetDouble(j+1, i+1) +
                             pMatX->GetDouble(i,k)*pMatX->GetDouble(j,k), j+1, i+1);
                        pQ->PutDouble(pQ->GetDouble(j+1, i+1), i+1, j+1);
                    }
                }
            }
        }
        else
        {
            for (k = 0; k < N; k++)
            {
                pE->PutDouble(
                    pE->GetDouble(M+1)+pMatY->GetDouble(k)*pMatY->GetDouble(k), M+1);
                pQ->PutDouble(pQ->GetDouble(0, M+1) + pMatY->GetDouble(k), 0,   M+1);
                pE->PutDouble(pQ->GetDouble(0, M+1), 0);
                for (i = 0; i < M; i++)
                {
                    pQ->PutDouble(pQ->GetDouble(0, i+1)+pMatX->GetDouble(k,i), 0, i+1);
                    pQ->PutDouble(pQ->GetDouble(0, i+1), i+1, 0);
                    pQ->PutDouble(pQ->GetDouble(i+1, M+1) +
                             pMatX->GetDouble(k,i)*pMatY->GetDouble(k), i+1, M+1);
                    pE->PutDouble(pQ->GetDouble(i+1, M+1), i+1);
                    for (j = i; j < M; j++)
                    {
                        pQ->PutDouble(pQ->GetDouble(j+1, i+1) +
                             pMatX->GetDouble(k, i)*pMatX->GetDouble(k, j), j+1, i+1);
                        pQ->PutDouble(pQ->GetDouble(j+1, i+1), i+1, j+1);
                    }
                }
            }
        }
        pQ->PutDouble((double)N, 0, 0);
        if (bConstant)
        {
            USHORT S, L;
            for (S = 0; S < M+1; S++)
            {
                i = S;
                while (i < M+1 && pQ->GetDouble(i, S) == 0.0)
                    i++;
                if (i >= M+1)
                {
                    SetNoValue();
                    delete pQ;
                    delete pE;
                    ResetNewMat(nMatInd12);
                    ResetNewMat(nMatInd10);
                    return;
                }
                double fVal;
                for (L = 0; L < M+2; L++)
                {
                    fVal = pQ->GetDouble(S, L);
                    pQ->PutDouble(pQ->GetDouble(i, L), S, L);
                    pQ->PutDouble(fVal, i, L);
                }
                fVal = 1.0/pQ->GetDouble(S, S);
                for (L = 0; L < M+2; L++)
                    pQ->PutDouble(pQ->GetDouble(S, L)*fVal, S, L);
                for (i = 0; i < M+1; i++)
                {
                    if (i != S)
                    {
                        fVal = -pQ->GetDouble(i, S);
                        for (L = 0; L < M+2; L++)
                            pQ->PutDouble(
                                pQ->GetDouble(i,L)+fVal*pQ->GetDouble(S,L),i,L);
                    }
                }
            }
        }
        else
        {
            USHORT S, L;
            for (S = 1; S < M+1; S++)
            {
                i = S;
                while (i < M+1 && pQ->GetDouble(i, S) == 0.0)
                    i++;
                if (i >= M+1)
                {
                    SetNoValue();
                    delete pQ;
                    delete pE;
                    ResetNewMat(nMatInd12);
                    ResetNewMat(nMatInd10);
                    return;
                }
                double fVal;
                for (L = 1; L < M+2; L++)
                {
                    fVal = pQ->GetDouble(S, L);
                    pQ->PutDouble(pQ->GetDouble(i, L), S, L);
                    pQ->PutDouble(fVal, i, L);
                }
                fVal = 1.0/pQ->GetDouble(S, S);
                for (L = 1; L < M+2; L++)
                    pQ->PutDouble(pQ->GetDouble(S, L)*fVal, S, L);
                for (i = 1; i < M+1; i++)
                {
                    if (i != S)
                    {
                        fVal = -pQ->GetDouble(i, S);
                        for (L = 1; L < M+2; L++)
                            pQ->PutDouble(
                                pQ->GetDouble(i,L)+fVal*pQ->GetDouble(S,L),i,L);
                    }
                }
                pQ->PutDouble(0.0, 0, M+1);
            }
        }
        if (nCase == 2)
        {
            pResMat = GetNewMat(1, nRXN, nMatInd);
            if (!pResMat)
            {
                SetIllegalParameter();
                return;
            }
            double fVal;
            for (i = 0; i < nRXN; i++)
            {
                fVal = pQ->GetDouble(0, M+1);
                for (j = 0; j < M; j++)
                    fVal += pQ->GetDouble(j+1, M+1)*pMatNewX->GetDouble(j, i);
                pResMat->PutDouble(fVal, i);
            }
        }
        else
        {
            pResMat = GetNewMat(nCXN, 1, nMatInd);
            if (!pResMat)
            {
                SetIllegalParameter();
                return;
            }
            double fVal;
            for (i = 0; i < nCXN; i++)
            {
                fVal = pQ->GetDouble(0, M+1);
                for (j = 0; j < M; j++)
                    fVal += pQ->GetDouble(j+1, M+1)*pMatNewX->GetDouble(i, j);
                pResMat->PutDouble(fVal, i);
            }
        }
        delete pQ;
        delete pE;
        ResetNewMat(nMatInd12);
        ResetNewMat(nMatInd10);
    }
    PushMatrix(pResMat);
    nRetMat = nMatInd;
}

void ScInterpreter::ScGrowth()
{
    BYTE nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 1, 4 ) )
        return;
    BOOL bConstant;
    if (nParamCount == 4)
        bConstant = GetBool();
    else
        bConstant = TRUE;
    USHORT nMatInd1, nMatInd2, nMatInd3;
    ScMatrix* pMatX;
    ScMatrix* pMatY;
    ScMatrix* pMatNewX;
    if (nParamCount >= 3)
        pMatNewX = GetMatrix(nMatInd3);
    else
        pMatNewX = NULL;
    if (nParamCount >= 2)
        pMatX = GetMatrix(nMatInd2);
    else
        pMatX = NULL;
    pMatY = GetMatrix(nMatInd1);
    if (!pMatY)
    {
        SetIllegalParameter();
        return;
    }
    BYTE nCase;                         // 1 = normal, 2,3 = mehrfach
    USHORT nCX, nRX, nCY, nRY, M, N;
    pMatY->GetDimensions(nCY, nRY);
    ULONG nCountY = (ULONG) nCY * nRY;
    ULONG i;
    for (i = 0; i < nCountY; i++)
        if (!pMatY->IsValue(i))
        {
            SetIllegalArgument();
            return;
        }
    for (i = 0; i < nCountY; i++)
    {
        if (pMatY->GetDouble(i) <= 0.0)
        {
            SetIllegalArgument();
            return;
        }
        else
            pMatY->PutDouble(log(pMatY->GetDouble(i)), i);
    }
    if (pMatX)
    {
        pMatX->GetDimensions(nCX, nRX);
        ULONG nCountX = (ULONG) nCX * nRX;
        for ( ULONG i = 0; i < nCountX; i++ )
            if (!pMatX->IsValue(i))
            {
                SetIllegalArgument();
                return;
            }
        if (nCX == nCY && nRX == nRY)
            nCase = 1;                  // einfache Regression
        else if (nCY != 1 && nRY != 1)
        {
            SetIllegalParameter();
            return;
        }
        else if (nCY == 1)
        {
            if (nRX != nRY)
            {
                SetIllegalParameter();
                return;
            }
            else
            {
                nCase = 2;              // zeilenweise
                N = nRY;
                M = nCX;
            }
        }
        else if (nCX != nCY)
        {
            SetIllegalParameter();
            return;
        }
        else
        {
            nCase = 3;                  // spaltenweise
            N = nCY;
            M = nRX;
        }
    }
    else
    {
        pMatX = GetNewMat(nCY, nRY, nMatInd2);
        nCX = nCY;
        nRX = nRY;
        if (!pMatX)
        {
            SetIllegalParameter();
            return;
        }
        for (i = 1; i <= nCountY; i++)
            pMatX->PutDouble((double)i, i-1);
        nCase = 1;
    }
    USHORT nCXN, nRXN;
    ULONG nCountXN;
    if (!pMatNewX)
    {
        nCXN = nCX;
        nRXN = nRX;
        nCountXN = (ULONG) nCXN * nRXN;
        pMatNewX = pMatX;
    }
    else
    {
        pMatNewX->GetDimensions(nCXN, nRXN);
        if ((nCase == 2 && nCX != nCXN) || (nCase == 3 && nRX != nRXN))
        {
            SetIllegalArgument();
            return;
        }
        nCountXN = (ULONG) nCXN * nRXN;
        for ( ULONG i = 0; i < nCountXN; i++ )
            if (!pMatNewX->IsValue(i))
            {
                SetIllegalArgument();
                return;
            }
    }
    USHORT nMatInd;
    ScMatrix* pResMat;
    if (nCase == 1)
    {
        double fCount   = 0.0;
        double fSumX    = 0.0;
        double fSumSqrX = 0.0;
        double fSumY    = 0.0;
        double fSumSqrY = 0.0;
        double fSumXY   = 0.0;
        double fValX, fValY;
        for (USHORT i = 0; i < nCY; i++)
            for (USHORT j = 0; j < nRY; j++)
            {
                fValX = pMatX->GetDouble(i,j);
                fValY = pMatY->GetDouble(i,j);
                fSumX    += fValX;
                fSumSqrX += fValX * fValX;
                fSumY    += fValY;
                fSumSqrY += fValY * fValY;
                fSumXY   += fValX*fValY;
                fCount++;
            }
        if (fCount < 1.0)
        {
            SetNoValue();
            return;
        }
        else
        {
            double f1 = fCount*fSumXY-fSumX*fSumY;
            double fX = fCount*fSumSqrX-fSumX*fSumX;
            double b, m;
            if (bConstant)
            {
                b = fSumY/fCount - f1/fX*fSumX/fCount;
                m = f1/fX;
            }
            else
            {
                b = 0.0;
                m = fSumXY/fSumSqrX;
            }
            pResMat = GetNewMat(nCXN, nRXN, nMatInd);
            if (!pResMat)
            {
                SetIllegalParameter();
                return;
            }
            for (i = 0; i < nCountXN; i++)
                pResMat->PutDouble(exp(pMatNewX->GetDouble(i)*m+b), i);
        }
    }
    else
    {
        USHORT nMatInd10, nMatInd12, i, j, k;
        ScMatrix* pQ = GetNewMat(M+1, M+2, nMatInd10);
        ScMatrix* pE = GetNewMat(M+2, 1, nMatInd12);
        pE->PutDouble(0.0, M+1);
        pQ->FillDouble(0.0, 0, 0, M, M+1);
        if (nCase == 2)
        {
            for (k = 0; k < N; k++)
            {
                pE->PutDouble(
                    pE->GetDouble(M+1)+pMatY->GetDouble(k)*pMatY->GetDouble(k), M+1);
                pQ->PutDouble(pQ->GetDouble(0, M+1) + pMatY->GetDouble(k), 0,   M+1);
                pE->PutDouble(pQ->GetDouble(0, M+1), 0);
                for (i = 0; i < M; i++)
                {
                    pQ->PutDouble(pQ->GetDouble(0, i+1)+pMatX->GetDouble(i,k), 0, i+1);
                    pQ->PutDouble(pQ->GetDouble(0, i+1), i+1, 0);
                    pQ->PutDouble(pQ->GetDouble(i+1, M+1) +
                             pMatX->GetDouble(i,k)*pMatY->GetDouble(k), i+1, M+1);
                    pE->PutDouble(pQ->GetDouble(i+1, M+1), i+1);
                    for (j = i; j < M; j++)
                    {
                        pQ->PutDouble(pQ->GetDouble(j+1, i+1) +
                             pMatX->GetDouble(i,k)*pMatX->GetDouble(j,k), j+1, i+1);
                        pQ->PutDouble(pQ->GetDouble(j+1, i+1), i+1, j+1);
                    }
                }
            }
        }
        else
        {
            for (k = 0; k < N; k++)
            {
                pE->PutDouble(
                    pE->GetDouble(M+1)+pMatY->GetDouble(k)*pMatY->GetDouble(k), M+1);
                pQ->PutDouble(pQ->GetDouble(0, M+1) + pMatY->GetDouble(k), 0,   M+1);
                pE->PutDouble(pQ->GetDouble(0, M+1), 0);
                for (i = 0; i < M; i++)
                {
                    pQ->PutDouble(pQ->GetDouble(0, i+1)+pMatX->GetDouble(k,i), 0, i+1);
                    pQ->PutDouble(pQ->GetDouble(0, i+1), i+1, 0);
                    pQ->PutDouble(pQ->GetDouble(i+1, M+1) +
                             pMatX->GetDouble(k,i)*pMatY->GetDouble(k), i+1, M+1);
                    pE->PutDouble(pQ->GetDouble(i+1, M+1), i+1);
                    for (j = i; j < M; j++)
                    {
                        pQ->PutDouble(pQ->GetDouble(j+1, i+1) +
                             pMatX->GetDouble(k, i)*pMatX->GetDouble(k, j), j+1, i+1);
                        pQ->PutDouble(pQ->GetDouble(j+1, i+1), i+1, j+1);
                    }
                }
            }
        }
        pQ->PutDouble((double)N, 0, 0);
        if (bConstant)
        {
            USHORT S, L;
            for (S = 0; S < M+1; S++)
            {
                i = S;
                while (i < M+1 && pQ->GetDouble(i, S) == 0.0)
                    i++;
                if (i >= M+1)
                {
                    SetNoValue();
                    delete pQ;
                    delete pE;
                    ResetNewMat(nMatInd12);
                    ResetNewMat(nMatInd10);
                    return;
                }
                double fVal;
                for (L = 0; L < M+2; L++)
                {
                    fVal = pQ->GetDouble(S, L);
                    pQ->PutDouble(pQ->GetDouble(i, L), S, L);
                    pQ->PutDouble(fVal, i, L);
                }
                fVal = 1.0/pQ->GetDouble(S, S);
                for (L = 0; L < M+2; L++)
                    pQ->PutDouble(pQ->GetDouble(S, L)*fVal, S, L);
                for (i = 0; i < M+1; i++)
                {
                    if (i != S)
                    {
                        fVal = -pQ->GetDouble(i, S);
                        for (L = 0; L < M+2; L++)
                            pQ->PutDouble(
                                pQ->GetDouble(i,L)+fVal*pQ->GetDouble(S,L),i,L);
                    }
                }
            }
        }
        else
        {
            USHORT S, L;
            for (S = 1; S < M+1; S++)
            {
                i = S;
                while (i < M+1 && pQ->GetDouble(i, S) == 0.0)
                    i++;
                if (i >= M+1)
                {
                    SetNoValue();
                    delete pQ;
                    delete pE;
                    ResetNewMat(nMatInd12);
                    ResetNewMat(nMatInd10);
                    return;
                }
                double fVal;
                for (L = 1; L < M+2; L++)
                {
                    fVal = pQ->GetDouble(S, L);
                    pQ->PutDouble(pQ->GetDouble(i, L), S, L);
                    pQ->PutDouble(fVal, i, L);
                }
                fVal = 1.0/pQ->GetDouble(S, S);
                for (L = 1; L < M+2; L++)
                    pQ->PutDouble(pQ->GetDouble(S, L)*fVal, S, L);
                for (i = 1; i < M+1; i++)
                {
                    if (i != S)
                    {
                        fVal = -pQ->GetDouble(i, S);
                        for (L = 1; L < M+2; L++)
                            pQ->PutDouble(
                                pQ->GetDouble(i,L)+fVal*pQ->GetDouble(S,L),i,L);
                    }
                }
                pQ->PutDouble(0.0, 0, M+1);
            }
        }
        if (nCase == 2)
        {
            pResMat = GetNewMat(1, nRXN, nMatInd);
            if (!pResMat)
            {
                SetIllegalParameter();
                return;
            }
            double fVal;
            for (i = 0; i < nRXN; i++)
            {
                fVal = pQ->GetDouble(0, M+1);
                for (j = 0; j < M; j++)
                    fVal += pQ->GetDouble(j+1, M+1)*pMatNewX->GetDouble(j, i);
                pResMat->PutDouble(exp(fVal), i);
            }
        }
        else
        {
            pResMat = GetNewMat(nCXN, 1, nMatInd);
            if (!pResMat)
            {
                SetIllegalParameter();
                return;
            }
            double fVal;
            for (i = 0; i < nCXN; i++)
            {
                fVal = pQ->GetDouble(0, M+1);
                for (j = 0; j < M; j++)
                    fVal += pQ->GetDouble(j+1, M+1)*pMatNewX->GetDouble(i, j);
                pResMat->PutDouble(exp(fVal), i);
            }
        }
        delete pQ;
        delete pE;
        ResetNewMat(nMatInd12);
        ResetNewMat(nMatInd10);
    }
    PushMatrix(pResMat);
    nRetMat = nMatInd;
}

void ScInterpreter::ScMatRef()
{
    // Falls Deltarefs drin sind...
    Push( (ScToken&) *pCur );
    ScAddress aAdr;
    PopSingleRef( aAdr );
    ScFormulaCell* pCell = (ScFormulaCell*) GetCell( aAdr );
    if( pCell && pCell->GetCellType() == CELLTYPE_FORMULA )
    {
        ScMatrix* pMat;
        pCell->GetMatrix( &pMat );
        if( pMat )
        {
            USHORT nCl, nRw;
            pMat->GetDimensions( nCl, nRw );
            USHORT nC = aPos.Col() - aAdr.Col();
            USHORT nR = aPos.Row() - aAdr.Row();
            if (nC < nCl && nR < nRw)
            {
                BOOL bIsString;
                const MatValue* pMatVal = pMat->Get(nC, nR, bIsString);
                if (bIsString)
                    PushString( pMatVal->GetString() );
                else
                {
                    PushDouble(pMatVal->fVal);
                    pDok->GetNumberFormatInfo( nCurFmtType, nCurFmtIndex, aAdr, *pCell );
                    nFuncFmtType = nCurFmtType;
                    nFuncFmtIndex = nCurFmtIndex;
                }
            }
            else
                SetNV();
        }
        else
        {
            // Ist gar keine Ergebnis-Matrix, dann bitte den Wert holen!
            USHORT nErr = pCell->GetErrCode();
            SetError( nErr );
            if( pCell->IsValue() )
                PushDouble( pCell->GetValue() );
            else
            {
                String aVal;
                pCell->GetString( aVal );
                PushString( aVal );
            }
            pDok->GetNumberFormatInfo( nCurFmtType, nCurFmtIndex, aAdr, *pCell );
            nFuncFmtType = nCurFmtType;
            nFuncFmtIndex = nCurFmtIndex;
        }
    }
    else
        SetError( errNoRef );
}



