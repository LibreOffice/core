/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: interpr5.cxx,v $
 *
 *  $Revision: 1.30 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 15:34:19 $
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

// INCLUDE ---------------------------------------------------------------

#include <string.h>
#include <math.h>
#include <stdio.h>

#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif

#ifndef _UTL_BOOTSTRAP_HXX
#include <unotools/bootstrap.hxx>
#endif
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
#include "cellkeytranslator.hxx"
#include "osversiondef.hxx"

#include <vector>

using ::std::vector;

const double fInvEpsilon = 1.0E-7;

// -----------------------------------------------------------------------

double ScInterpreter::ScGetGCD(double fx, double fy)
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

void ScInterpreter::ScGCD()
{
    short nParamCount = GetByte();
    if ( MustHaveParamCountMin( nParamCount, 1 ) )
    {
        double fSign = 1.0;
        double fx, fy = 0.0;
        bool bFirst = true;
        ScRange aRange;
        size_t nRefInList = 0;
        while (nParamCount-- > 0)
        {
            switch (GetStackType())
            {
                case svDouble :
                case svString:
                case svSingleRef:
                {
                    if (bFirst)
                    {
                        bFirst = false;
                        fy = GetDouble();
                        if (fy < 0.0)
                        {
                            fy *= -1.0;
                            fSign *= -1.0;
                        }
                    }
                    else
                    {
                        fx = GetDouble();
                        if (fx < 0.0)
                        {
                            fx *= -1.0;
                            fSign *= -1.0;
                        }
                        fy = ScGetGCD(fx, fy);
                    }
                }
                break;
                case svDoubleRef :
                case svRefList :
                {
                    USHORT nErr = 0;
                    PopDoubleRef( aRange, nParamCount, nRefInList);
                    double nCellVal;
                    ScValueIterator aValIter(pDok, aRange, glSubTotal);
                    if (aValIter.GetFirst(nCellVal, nErr))
                    {
                        if (bFirst)
                        {
                            bFirst = false;
                            fy = nCellVal;
                            if (fy < 0.0)
                            {
                                fy *= -1.0;
                                fSign *= -1.0;
                            }
                        }
                        else
                        {
                            fx = nCellVal;
                            if (fx < 0.0)
                            {
                                fx *= -1.0;
                                fSign *= -1.0;
                            }
                            fy = ScGetGCD(fx, fy);
                        }
                        while (nErr == 0 && aValIter.GetNext(nCellVal, nErr))
                        {
                            fx = nCellVal;
                            if (fx < 0.0)
                            {
                                fx *= -1.0;
                                fSign *= -1.0;
                            }
                            fy = ScGetGCD(fx, fy);
                        }
                        SetError(nErr);
                    }
                }
                break;
                case svMatrix :
                {
                    ScMatrixRef pMat = PopMatrix();
                    if (pMat)
                    {
                        SCSIZE nC, nR;
                        pMat->GetDimensions(nC, nR);
                        if (nC == 0 || nR == 0)
                            SetError(errIllegalArgument);
                        else
                        {
                            if (!pMat->IsValue(0))
                            {
                                PushIllegalArgument();
                                return;
                            }
                            if (bFirst)
                            {
                                bFirst = false;
                                fy = pMat->GetDouble(0);
                                if (fy < 0.0)
                                {
                                    fy *= -1.0;
                                    fSign *= -1.0;
                                }
                            }
                            else
                            {
                                fx = pMat->GetDouble(0);
                                if (fx < 0.0)
                                {
                                    fx *= -1.0;
                                    fSign *= -1.0;
                                }
                                fy = ScGetGCD(fx, fy);
                            }
                            SCSIZE nCount = nC * nR;
                            for ( SCSIZE j = 1; j < nCount; j++ )
                            {
                                if (!pMat->IsValue(j))
                                {
                                    PushIllegalArgument();
                                    return;
                                }
                                fx = pMat->GetDouble(j);
                                if (fx < 0.0)
                                {
                                    fx *= -1.0;
                                    fSign *= -1.0;
                                }
                                fy = ScGetGCD(fx, fy);
                            }
                        }
                    }
                }
                break;
                default : SetError(errIllegalParameter); break;
            }
        }
        if (bFirst)
            SetError( errIllegalParameter);
        if (fSign == -1.0)
            PushDouble(-fy);
        else
            PushDouble(fy);
    }
}

void ScInterpreter:: ScLCM()
{
    short nParamCount = GetByte();
    if ( MustHaveParamCountMin( nParamCount, 1 ) )
    {
        double fSign = 1.0;
        double fx, fy = 0.0;
        bool bFirst = true;
        ScRange aRange;
        size_t nRefInList = 0;
        while (nParamCount-- > 0)
        {
            switch (GetStackType())
            {
                case svDouble :
                case svString:
                case svSingleRef:
                {
                    if (bFirst)
                    {
                        bFirst = false;
                        fy = GetDouble();
                        if (fy < 0.0)
                        {
                            fy *= -1.0;
                            fSign *= -1.0;
                        }
                    }
                    else
                    {
                        fx = GetDouble();
                        if (fx < 0.0)
                        {
                            fx *= -1.0;
                            fSign *= -1.0;
                        }
                        fy = fx * fy / ScGetGCD(fx, fy);
                    }
                }
                break;
                case svDoubleRef :
                case svRefList :
                {
                    USHORT nErr = 0;
                    PopDoubleRef( aRange, nParamCount, nRefInList);
                    double nCellVal;
                    ScValueIterator aValIter(pDok, aRange, glSubTotal);
                    if (aValIter.GetFirst(nCellVal, nErr))
                    {
                        if (bFirst)
                        {
                            bFirst = false;
                            fy = nCellVal;
                            if (fy < 0.0)
                            {
                                fy *= -1.0;
                                fSign *= -1.0;
                            }
                        }
                        else
                        {
                            fx = nCellVal;
                            if (fx < 0.0)
                            {
                                fx *= -1.0;
                                fSign *= -1.0;
                            }
                            fy = fx * fy / ScGetGCD(fx, fy);
                        }
                        while (nErr == 0 && aValIter.GetNext(nCellVal, nErr))
                        {
                            fx = nCellVal;
                            if (fx < 0.0)
                            {
                                fx *= -1.0;
                                fSign *= -1.0;
                            }
                            fy = fx * fy / ScGetGCD(fx, fy);
                        }
                        SetError(nErr);
                    }
                }
                break;
                case svMatrix :
                {
                    ScMatrixRef pMat = PopMatrix();
                    if (pMat)
                    {
                        SCSIZE nC, nR;
                        pMat->GetDimensions(nC, nR);
                        if (nC == 0 || nR == 0)
                            SetError(errIllegalArgument);
                        else
                        {
                            if (!pMat->IsValue(0))
                            {
                                PushIllegalArgument();
                                return;
                            }
                            if (bFirst)
                            {
                                bFirst = false;
                                fy = pMat->GetDouble(0);
                                if (fy < 0.0)
                                {
                                    fy *= -1.0;
                                    fSign *= -1.0;
                                }
                            }
                            else
                            {
                                fx = pMat->GetDouble(0);
                                if (fx < 0.0)
                                {
                                    fx *= -1.0;
                                    fSign *= -1.0;
                                }
                                fy = fx * fy / ScGetGCD(fx, fy);
                            }
                            SCSIZE nCount = nC * nR;
                            for ( SCSIZE j = 1; j < nCount; j++ )
                            {
                                if (!pMat->IsValue(j))
                                {
                                    PushIllegalArgument();
                                    return;
                                }
                                fx = pMat->GetDouble(j);
                                if (fx < 0.0)
                                {
                                    fx *= -1.0;
                                    fSign *= -1.0;
                                }
                                fy = fx * fy / ScGetGCD(fx, fy);
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

ScMatrixRef ScInterpreter::GetNewMat(SCSIZE nC, SCSIZE nR)
{
    ScMatrix* pMat = new ScMatrix( nC, nR);
    pMat->SetErrorInterpreter( this);
    SCSIZE nCols, nRows;
    pMat->GetDimensions( nCols, nRows);
    if ( nCols != nC || nRows != nR )
    {   // arbitray limit of elements exceeded
        SetError( errStackOverflow);
        pMat->Delete();
        pMat = NULL;
    }
    return pMat;
}

ScMatrixRef ScInterpreter::CreateMatrixFromDoubleRef( const ScToken* pToken,
        SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
        SCCOL nCol2, SCROW nRow2, SCTAB nTab2 )
{
    ScMatrixRef pMat = NULL;
    if (nTab1 == nTab2 && !nGlobalError)
    {
        ScTokenMatrixMap::const_iterator aIter;
        if ( static_cast<SCSIZE>(nRow2 - nRow1 + 1) *
                static_cast<SCSIZE>(nCol2 - nCol1 + 1) >
                ScMatrix::GetElementsMax() )
            SetError(errStackOverflow);
        else if (pTokenMatrixMap && ((aIter = pTokenMatrixMap->find( pToken))
                    != pTokenMatrixMap->end()))
            pMat = (*aIter).second->GetMatrix();
        else
        {
            SCSIZE nMatCols = static_cast<SCSIZE>(nCol2 - nCol1 + 1);
            SCSIZE nMatRows = static_cast<SCSIZE>(nRow2 - nRow1 + 1);
            pMat = GetNewMat( nMatCols, nMatRows);
            if (pMat && !nGlobalError)
            {
                // Set position where the next entry is expected.
                SCROW nNextRow = nRow1;
                SCCOL nNextCol = nCol1;
                // Set last position as if there was a previous entry.
                SCROW nThisRow = nRow2;
                SCCOL nThisCol = nCol1 - 1;
                ScCellIterator aCellIter( pDok, nCol1, nRow1, nTab1, nCol2,
                        nRow2, nTab2);
                for (ScBaseCell* pCell = aCellIter.GetFirst(); pCell; pCell =
                        aCellIter.GetNext())
                {
                    nThisCol = aCellIter.GetCol();
                    nThisRow = aCellIter.GetRow();
                    if (nThisCol != nNextCol || nThisRow != nNextRow)
                    {
                        // Fill empty between iterator's positions.
                        for ( ; nNextCol <= nThisCol; ++nNextCol)
                        {
                            SCSIZE nC = nNextCol - nCol1;
                            SCSIZE nMatStopRow = ((nNextCol < nThisCol) ?
                                    nMatRows : nThisRow - nRow1);
                            for (SCSIZE nR = nNextRow - nRow1; nR <
                                    nMatStopRow; ++nR)
                            {
                                pMat->PutEmpty( nC, nR);
                            }
                            nNextRow = nRow1;
                        }
                    }
                    if (nThisRow == nRow2)
                    {
                        nNextCol = nThisCol + 1;
                        nNextRow = nRow1;
                    }
                    else
                    {
                        nNextCol = nThisCol;
                        nNextRow = nThisRow + 1;
                    }
                    if (HasCellEmptyData(pCell))
                    {
                        pMat->PutEmpty( static_cast<SCSIZE>(nThisCol-nCol1),
                                static_cast<SCSIZE>(nThisRow-nRow1));
                    }
                    else if (HasCellValueData(pCell))
                    {
                        ScAddress aAdr( nThisCol, nThisRow, nTab1);
                        double fVal = GetCellValue( aAdr, pCell);
                        if ( nGlobalError )
                        {
                            fVal = CreateDoubleError( nGlobalError);
                            nGlobalError = 0;
                        }
                        pMat->PutDouble( fVal,
                                static_cast<SCSIZE>(nThisCol-nCol1),
                                static_cast<SCSIZE>(nThisRow-nRow1));
                    }
                    else
                    {
                        String aStr;
                        GetCellString( aStr, pCell);
                        if ( nGlobalError )
                        {
                            double fVal = CreateDoubleError( nGlobalError);
                            nGlobalError = 0;
                            pMat->PutDouble( fVal,
                                    static_cast<SCSIZE>(nThisCol-nCol1),
                                    static_cast<SCSIZE>(nThisRow-nRow1));
                        }
                        else
                            pMat->PutString( aStr,
                                    static_cast<SCSIZE>(nThisCol-nCol1),
                                    static_cast<SCSIZE>(nThisRow-nRow1));
                    }
                }
                // Fill empty if iterator's last position wasn't the end.
                if (nThisCol != nCol2 || nThisRow != nRow2)
                {
                    for ( ; nNextCol <= nCol2; ++nNextCol)
                    {
                        SCSIZE nC = nNextCol - nCol1;
                        for (SCSIZE nR = nNextRow - nRow1; nR < nMatRows; ++nR)
                        {
                            pMat->PutEmpty( nC, nR);
                        }
                        nNextRow = nRow1;
                    }
                }
                if (pTokenMatrixMap)
                    pTokenMatrixMap->insert( ScTokenMatrixMap::value_type(
                                pToken, new ScMatrixToken( pMat)));
            }
        }
    }
    else                                // not a 2D matrix
        SetError(errIllegalParameter);
    return pMat;
}


ScMatrixRef ScInterpreter::GetMatrix()
{
    ScMatrixRef pMat = NULL;
    switch (GetRawStackType())
    {
        case svSingleRef :
        {
            ScAddress aAdr;
            PopSingleRef( aAdr );
            pMat = GetNewMat(1, 1);
            if (pMat)
            {
                ScBaseCell* pCell = GetCell( aAdr );
                if (HasCellEmptyData(pCell))
                    pMat->PutEmpty( 0 );
                else if (HasCellValueData(pCell))
                    pMat->PutDouble(GetCellValue(aAdr, pCell), 0);
                else
                {
                    String aStr;
                    GetCellString(aStr, pCell);
                    pMat->PutString(aStr, 0);
                }
            }
        }
        break;
        case svDoubleRef:
        {
            SCCOL nCol1, nCol2;
            SCROW nRow1, nRow2;
            SCTAB nTab1, nTab2;
            const ScToken* p = sp ? pStack[sp-1] : NULL;
            PopDoubleRef(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
            pMat = CreateMatrixFromDoubleRef( p, nCol1, nRow1, nTab1,
                    nCol2, nRow2, nTab2);
        }
        break;
        case svMatrix:
            pMat = PopMatrix();
        break;
        case svError :
        case svMissing :
        case svDouble :
        {
            double fVal = GetDouble();
            pMat = GetNewMat( 1, 1);
            if ( pMat )
            {
                if ( nGlobalError )
                {
                    fVal = CreateDoubleError( nGlobalError);
                    nGlobalError = 0;
                }
                pMat->PutDouble( fVal, 0);
            }
        }
        break;
        case svString :
        {
            String aStr = GetString();
            pMat = GetNewMat( 1, 1);
            if ( pMat )
            {
                if ( nGlobalError )
                {
                    double fVal = CreateDoubleError( nGlobalError);
                    pMat->PutDouble( fVal, 0);
                    nGlobalError = 0;
                }
                else
                    pMat->PutString( aStr, 0);
            }
        }
        break;
        default:
            PopError();
            SetError( errIllegalArgument);
        break;
    }
    return pMat;
}

void ScInterpreter::ScMatValue()
{
    if ( MustHaveParamCount( GetByte(), 3 ) )
    {
        // 0 to count-1
        SCSIZE nR = static_cast<SCSIZE>(::rtl::math::approxFloor(GetDouble()));
        SCSIZE nC = static_cast<SCSIZE>(::rtl::math::approxFloor(GetDouble()));
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
                        PushError( nErrCode);
                    else
                    {
                        const ScMatrix* pMat = ((ScFormulaCell*)pCell)->GetMatrix();
                        if (pMat)
                        {
                            SCSIZE nCl, nRw;
                            pMat->GetDimensions(nCl, nRw);
                            if (nC < nCl && nR < nRw)
                            {
                                ScMatValType nMatValType;
                                const ScMatrixValue* pMatVal = pMat->Get( nC,
                                        nR, nMatValType);
                                if (ScMatrix::IsStringType( nMatValType))
                                    PushString( pMatVal->GetString() );
                                else
                                    PushDouble(pMatVal->fVal);
                                    // also handles DoubleError
                            }
                            else
                                PushNoValue();
                        }
                        else
                            PushNoValue();
                    }
                }
                else
                    PushIllegalParameter();
            }
            break;
            case svDoubleRef :
            {
                SCCOL nCol1;
                SCROW nRow1;
                SCTAB nTab1;
                SCCOL nCol2;
                SCROW nRow2;
                SCTAB nTab2;
                PopDoubleRef(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
                if (nCol2 - nCol1 >= static_cast<SCCOL>(nR) &&
                        nRow2 - nRow1 >= static_cast<SCROW>(nC) &&
                        nTab1 == nTab2)
                {
                    ScAddress aAdr( sal::static_int_cast<SCCOL>( nCol1 + nR ),
                                    sal::static_int_cast<SCROW>( nRow1 + nC ), nTab1 );
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
                    PushNoValue();
            }
            break;
            case svMatrix:
            {
                ScMatrixRef pMat = PopMatrix();
                if (pMat)
                {
                    SCSIZE nCl, nRw;
                    pMat->GetDimensions(nCl, nRw);
                    if (nC < nCl && nR < nRw)
                    {
                        ScMatValType nMatValType;
                        const ScMatrixValue* pMatVal = pMat->Get( nC, nR,
                                nMatValType);
                        if (ScMatrix::IsStringType( nMatValType))
                            PushString( pMatVal->GetString() );
                        else
                            PushDouble(pMatVal->fVal);
                            // also handles DoubleError
                    }
                    else
                        PushNoValue();
                }
                else
                    PushNoValue();
            }
            break;
            default:
                PopError();
                PushIllegalParameter();
            break;
        }
    }
}

void ScInterpreter::ScEMat()
{
    if ( MustHaveParamCount( GetByte(), 1 ) )
    {
        SCSIZE nDim = static_cast<SCSIZE>(::rtl::math::approxFloor(GetDouble()));
        if ( nDim * nDim > ScMatrix::GetElementsMax() || nDim == 0)
            PushIllegalArgument();
        else
        {
            ScMatrixRef pRMat = GetNewMat(nDim, nDim);
            if (pRMat)
            {
                MEMat(pRMat, nDim);
                PushMatrix(pRMat);
            }
            else
                PushIllegalArgument();
        }
    }
}

void ScInterpreter::MEMat(ScMatrix* mM, SCSIZE n)
{
    mM->FillDouble(0.0, 0, 0, n-1, n-1);
    for (SCSIZE i = 0; i < n; i++)
        mM->PutDouble(1.0, i, i);
}

void ScInterpreter::MFastMult(ScMatrix* pA, ScMatrix* pB, ScMatrix* pR,
                              SCSIZE n, SCSIZE m, SCSIZE l)
        // Multipliziert n x m Mat a mit m x l Mat b nach Mat r
{
    double sum;
    for (SCSIZE i = 0; i < n; i++)
    {
        for (SCSIZE j = 0; j < l; j++)
        {
            sum = 0.0;
            for (SCSIZE k = 0; k < m; k++)
                sum += pA->GetDouble(i,k)*pB->GetDouble(k,j);
            pR->PutDouble(sum, i, j);
        }
    }
}

void ScInterpreter::MFastSub(ScMatrix* pA, ScMatrix* pB, ScMatrix* pR,
                              SCSIZE n, SCSIZE m)
        // Subtrahiert n x m Mat a - m x l Mat b nach Mat r
{
    for (SCSIZE i = 0; i < n; i++)
    {
        for (SCSIZE j = 0; j < m; j++)
            pR->PutDouble(pA->GetDouble(i,j) - pB->GetDouble(i,j), i, j);
    }
}

void ScInterpreter::MFastTrans(ScMatrix* pA, ScMatrix* pR,
                              SCSIZE n, SCSIZE m)
        // Transponiert n x m Mat a nach Mat r
{
    for (SCSIZE i = 0; i < n; i++)
        for (SCSIZE j = 0; j < m; j++)
            pR->PutDouble(pA->GetDouble(i, j), j, i);
}


/* Matrix LUP decomposition according to the pseudocode of "Introduction to
 * Algorithms" by Cormen, Leiserson, Rivest, Stein.
 *
 * Added scaling for numeric stability.
 *
 * Given an n x n nonsingular matrix A, find a permutation matrix P, a unit
 * lower-triangular matrix L, and an upper-triangular matrix U such that PA=LU.
 * Compute L and U "in place" in the matrix A, the original content is
 * destroyed. Note that the diagonal elements of the U triangular matrix
 * replace the diagonal elements of the L-unit matrix (that are each ==1). The
 * permutation matrix P is an array, where P[i]=j means that the i-th row of P
 * contains a 1 in column j. Additionally keep track of the number of
 * permutations (row exchanges).
 *
 * Returns 0 if a singular matrix is encountered, else +1 if an even number of
 * permutations occured, or -1 if odd, which is the sign of the determinant.
 * This may be used to calculate the determinant by multiplying the sign with
 * the product of the diagonal elements of the LU matrix.
 */
static int lcl_LUP_decompose( ScMatrix* mA, const SCSIZE n,
        ::std::vector< SCSIZE> & P )
{
    int nSign = 1;
    // Find scale of each row.
    ::std::vector< double> aScale(n);
    for (SCSIZE i=0; i < n; ++i)
    {
        double fMax = 0.0;
        for (SCSIZE j=0; j < n; ++j)
        {
            double fTmp = fabs( mA->GetDouble( j, i));
            if (fMax < fTmp)
                fMax = fTmp;
        }
        if (fMax == 0.0)
            return 0;       // singular matrix
        aScale[i] = 1.0 / fMax;
    }
    // Represent identity permutation, P[i]=i
    for (SCSIZE i=0; i < n; ++i)
        P[i] = i;
    // "Recursion" on the diagonale.
    SCSIZE l = n - 1;
    for (SCSIZE k=0; k < l; ++k)
    {
        // Implicit pivoting. With the scale found for a row, compare values of
        // a column and pick largest.
        double fMax = 0.0;
        double fScale = aScale[k];
        SCSIZE kp = k;
        for (SCSIZE i = k; i < n; ++i)
        {
            double fTmp = fScale * fabs( mA->GetDouble( k, i));
            if (fMax < fTmp)
            {
                fMax = fTmp;
                kp = i;
            }
        }
        if (fMax == 0.0)
            return 0;       // singular matrix
        // Swap rows. The pivot element will be at mA[k,kp] (row,col notation)
        if (k != kp)
        {
            // permutations
            SCSIZE nTmp = P[k];
            P[k]        = P[kp];
            P[kp]       = nTmp;
            nSign       = -nSign;
            // scales
            double fTmp = aScale[k];
            aScale[k]   = aScale[kp];
            aScale[kp]  = fTmp;
            // elements
            for (SCSIZE i=0; i < n; ++i)
            {
                double fMatTmp = mA->GetDouble( i, k);
                mA->PutDouble( mA->GetDouble( i, kp), i, k);
                mA->PutDouble( fMatTmp, i, kp);
            }
        }
        // Compute Schur complement.
        for (SCSIZE i = k+1; i < n; ++i)
        {
            double fTmp = mA->GetDouble( k, i) / mA->GetDouble( k, k);
            mA->PutDouble( fTmp, k, i);
            for (SCSIZE j = k+1; j < n; ++j)
                mA->PutDouble( mA->GetDouble( j, i) - fTmp * mA->GetDouble( j,
                            k), j, i);
        }
    }
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "\n%s\n", "lcl_LUP_decompose(): LU");
    for (SCSIZE i=0; i < n; ++i)
    {
        for (SCSIZE j=0; j < n; ++j)
            fprintf( stderr, "%8.2g  ", mA->GetDouble( j, i));
        fprintf( stderr, "\n%s\n", "");
    }
    fprintf( stderr, "\n%s\n", "lcl_LUP_decompose(): P");
    for (SCSIZE j=0; j < n; ++j)
        fprintf( stderr, "%5u ", P[j]);
    fprintf( stderr, "\n%s\n", "");
#endif
    return nSign;
}


/* Solve a LUP decomposed equation Ax=b. LU is a combined matrix of L and U
 * triangulars and P the permutation vector as obtained from
 * lcl_LUP_decompose(). B is the right-hand side input vector, X is used to
 * return the solution vector.
 */
static void lcl_LUP_solve( const ScMatrix* mLU, const SCSIZE n,
        const ::std::vector< SCSIZE> & P, const ::std::vector< double> & B,
        ::std::vector< double> & X )
{
    SCSIZE nFirst = SCSIZE_MAX;
    // Ax=b => PAx=Pb, with decomposition LUx=Pb.
    // Define y=Ux and solve for y in Ly=Pb using forward substitution.
    for (SCSIZE i=0; i < n; ++i)
    {
        double fSum = B[P[i]];
        // Matrix inversion comes with a lot of zeros in the B vectors, we
        // don't have to do all the computing with results multiplied by zero.
        // Until then, simply lookout for the position of the first nonzero
        // value.
        if (nFirst != SCSIZE_MAX)
        {
            for (SCSIZE j = nFirst; j < i; ++j)
                fSum -= mLU->GetDouble( j, i) * X[j];   // X[j] === y[j]
        }
        else if (fSum)
            nFirst = i;
        X[i] = fSum;                                    // X[i] === y[i]
    }
    // Solve for x in Ux=y using back substitution.
    for (SCSIZE i = n; i--; )
    {
        double fSum = X[i];                             // X[i] === y[i]
        for (SCSIZE j = i+1; j < n; ++j)
            fSum -= mLU->GetDouble( j, i) * X[j];       // X[j] === x[j]
        X[i] = fSum / mLU->GetDouble( i, i);            // X[i] === x[i]
    }
#if OSL_DEBUG_LEVEL >1
    fprintf( stderr, "\n%s\n", "lcl_LUP_solve():");
    for (SCSIZE i=0; i < n; ++i)
        fprintf( stderr, "%8.2g  ", X[i]);
    fprintf( stderr, "%s\n", "");
#endif
}


void ScInterpreter::ScMatDet()
{
    if ( MustHaveParamCount( GetByte(), 1 ) )
    {
        ScMatrixRef pMat = GetMatrix();
        if (!pMat)
        {
            PushIllegalParameter();
            return;
        }
        if ( !pMat->IsNumeric() )
        {
            PushNoValue();
            return;
        }
        SCSIZE nC, nR;
        pMat->GetDimensions(nC, nR);
        if ( nC != nR || nC == 0 || (ULONG) nC * nC > ScMatrix::GetElementsMax() )
            PushIllegalArgument();
        else
        {
            // LUP decomposition is done inplace, use copy.
            ScMatrixRef xLU = pMat->Clone();
            if (!xLU)
                PushError( errCodeOverflow);
            else
            {
                ::std::vector< SCSIZE> P(nR);
                int nDetSign = lcl_LUP_decompose( xLU, nR, P);
                if (!nDetSign)
                    PushInt(0);     // singular matrix
                else
                {
                    // In an LU matrix the determinant is simply the product of
                    // all diagonal elements.
                    double fDet = nDetSign;
                    ScMatrix* pLU = xLU;
                    for (SCSIZE i=0; i < nR; ++i)
                        fDet *= pLU->GetDouble( i, i);
                    PushDouble( fDet);
                }
            }
        }
    }
}

void ScInterpreter::ScMatInv()
{
    if ( MustHaveParamCount( GetByte(), 1 ) )
    {
        ScMatrixRef pMat = GetMatrix();
        if (!pMat)
        {
            PushIllegalParameter();
            return;
        }
        if ( !pMat->IsNumeric() )
        {
            PushNoValue();
            return;
        }
        SCSIZE nC, nR;
        pMat->GetDimensions(nC, nR);
        if ( nC != nR || nC == 0 || (ULONG) nC * nC > ScMatrix::GetElementsMax() )
            PushIllegalArgument();
        else
        {
            // LUP decomposition is done inplace, use copy.
            ScMatrixRef xLU = pMat->Clone();
            // The result matrix.
            ScMatrixRef xY = GetNewMat( nR, nR);
            if (!xLU || !xY)
                PushError( errCodeOverflow);
            else
            {
                ::std::vector< SCSIZE> P(nR);
                int nDetSign = lcl_LUP_decompose( xLU, nR, P);
                if (!nDetSign)
                    PushIllegalArgument();
                else
                {
                    // Solve equation for each column.
                    ScMatrix* pY = xY;
                    ::std::vector< double> B(nR);
                    ::std::vector< double> X(nR);
                    for (SCSIZE j=0; j < nR; ++j)
                    {
                        for (SCSIZE i=0; i < nR; ++i)
                            B[i] = 0.0;
                        B[j] = 1.0;
                        lcl_LUP_solve( xLU, nR, P, B, X);
                        for (SCSIZE i=0; i < nR; ++i)
                            pY->PutDouble( X[i], j, i);
                    }
#if 0
                    /* Possible checks for ill-condition:
                     * 1. Scale matrix, invert scaled matrix. If there are
                     *    elements of the inverted matrix that are several
                     *    orders of magnitude greater than 1 =>
                     *    ill-conditioned.
                     *    Just how much is "several orders"?
                     * 2. Invert the inverted matrix and assess whether the
                     *    result is sufficiently close to the original matrix.
                     *    If not => ill-conditioned.
                     *    Just what is sufficient?
                     * 3. Multiplying the inverse by the original matrix should
                     *    produce a result sufficiently close to the identity
                     *    matrix.
                     *    Just what is sufficient?
                     *
                     * The following is #3.
                     */
                    ScMatrixRef xR = GetNewMat( nR, nR);
                    if (xR)
                    {
                        ScMatrix* pR = xR;
                        MFastMult( pMat, pY, pR, nR, nR, nR);
#if OSL_DEBUG_LEVEL > 1
                        fprintf( stderr, "\n%s\n", "ScMatInv(): mult-identity");
#endif
                        for (SCSIZE i=0; i < nR; ++i)
                        {
                            for (SCSIZE j=0; j < nR; ++j)
                            {
                                double fTmp = pR->GetDouble( j, i);
#if OSL_DEBUG_LEVEL > 1
                                fprintf( stderr, "%8.2g  ", fTmp);
#endif
                                if (fabs( fTmp - (i == j)) > fInvEpsilon)
                                    SetError( errIllegalArgument);
                            }
#if OSL_DEBUG_LEVEL > 1
                        fprintf( stderr, "\n%s\n", "");
#endif
                        }
                    }
#endif
                    if (nGlobalError)
                        PushError( nGlobalError);
                    else
                        PushMatrix( pY);
                }
            }
        }
    }
}

void ScInterpreter::ScMatMult()
{
    if ( MustHaveParamCount( GetByte(), 2 ) )
    {
        ScMatrixRef pMat2 = GetMatrix();
        ScMatrixRef pMat1 = GetMatrix();
        ScMatrixRef pRMat;
        if (pMat1 && pMat2)
        {
            if ( pMat1->IsNumeric() && pMat2->IsNumeric() )
            {
                SCSIZE nC1, nC2;
                SCSIZE nR1, nR2;
                pMat1->GetDimensions(nC1, nR1);
                pMat2->GetDimensions(nC2, nR2);
                if (nC1 != nR2)
                    PushIllegalArgument();
                else
                {
                    pRMat = GetNewMat(nC2, nR1);
                    if (pRMat)
                    {
                        double sum;
                        for (SCSIZE i = 0; i < nR1; i++)
                        {
                            for (SCSIZE j = 0; j < nC2; j++)
                            {
                                sum = 0.0;
                                for (SCSIZE k = 0; k < nC1; k++)
                                {
                                    sum += pMat1->GetDouble(k,i)*pMat2->GetDouble(j,k);
                                }
                                pRMat->PutDouble(sum, j, i);
                            }
                        }
                        PushMatrix(pRMat);
                    }
                    else
                        PushIllegalArgument();
                }
            }
            else
                PushNoValue();
        }
        else
            PushIllegalParameter();
    }
}

void ScInterpreter::ScMatTrans()
{
    if ( MustHaveParamCount( GetByte(), 1 ) )
    {
        ScMatrixRef pMat = GetMatrix();
        ScMatrixRef pRMat;
        if (pMat)
        {
            SCSIZE nC, nR;
            pMat->GetDimensions(nC, nR);
            pRMat = GetNewMat(nR, nC);
            if ( pRMat )
            {
                pMat->MatTrans(*pRMat);
                PushMatrix(pRMat);
            }
            else
                PushIllegalArgument();
        }
        else
            PushIllegalParameter();
    }
}

ScMatrixRef ScInterpreter::MatAdd(ScMatrix* pMat1, ScMatrix* pMat2)
{
    SCSIZE nC1, nC2, nMinC;
    SCSIZE nR1, nR2, nMinR;
    SCSIZE i, j;
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
    ScMatrixRef xResMat = GetNewMat(nMinC, nMinR);
    if (xResMat)
    {
        ScMatrix* pResMat = xResMat;
        for (i = 0; i < nMinC; i++)
        {
            for (j = 0; j < nMinR; j++)
            {
                if (pMat1->IsValueOrEmpty(i,j) && pMat2->IsValueOrEmpty(i,j))
                    pResMat->PutDouble( ::rtl::math::approxAdd( pMat1->GetDouble(i,j),
                                       pMat2->GetDouble(i,j)), i, j);
                else
                    pResMat->PutString(ScGlobal::GetRscString(
                                                    STR_NO_VALUE), i, j);
            }
        }
    }
    return xResMat;
}

ScMatrixRef ScInterpreter::MatSub(ScMatrix* pMat1, ScMatrix* pMat2)
{
    SCSIZE nC1, nC2, nMinC;
    SCSIZE nR1, nR2, nMinR;
    SCSIZE i, j;
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
    ScMatrixRef xResMat = GetNewMat(nMinC, nMinR);
    if (xResMat)
    {
        ScMatrix* pResMat = xResMat;
        for (i = 0; i < nMinC; i++)
        {
            for (j = 0; j < nMinR; j++)
            {
                if (pMat1->IsValueOrEmpty(i,j) && pMat2->IsValueOrEmpty(i,j))
                    pResMat->PutDouble( ::rtl::math::approxSub( pMat1->GetDouble(i,j),
                                       pMat2->GetDouble(i,j)), i, j);
                else
                    pResMat->PutString(ScGlobal::GetRscString(
                                                    STR_NO_VALUE), i, j);
            }
        }
    }
    return xResMat;
}

ScMatrixRef ScInterpreter::MatMul(ScMatrix* pMat1, ScMatrix* pMat2)
{
    SCSIZE nC1, nC2, nMinC;
    SCSIZE nR1, nR2, nMinR;
    SCSIZE i, j;
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
    ScMatrixRef xResMat = GetNewMat(nMinC, nMinR);
    if (xResMat)
    {
        ScMatrix* pResMat = xResMat;
        for (i = 0; i < nMinC; i++)
        {
            for (j = 0; j < nMinR; j++)
            {
                if (pMat1->IsValueOrEmpty(i,j) && pMat2->IsValueOrEmpty(i,j))
                    pResMat->PutDouble(pMat1->GetDouble(i,j) *
                                       pMat2->GetDouble(i,j), i, j);
                else
                    pResMat->PutString(ScGlobal::GetRscString(
                                                    STR_NO_VALUE), i, j);
            }
        }
    }
    return xResMat;
}

ScMatrixRef ScInterpreter::MatDiv(ScMatrix* pMat1, ScMatrix* pMat2)
{
    SCSIZE nC1, nC2, nMinC;
    SCSIZE nR1, nR2, nMinR;
    SCSIZE i, j;
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
    ScMatrixRef xResMat = GetNewMat(nMinC, nMinR);
    if (xResMat)
    {
        ScMatrix* pResMat = xResMat;
        for (i = 0; i < nMinC; i++)
        {
            for (j = 0; j < nMinR; j++)
            {
                if (pMat1->IsValueOrEmpty(i,j) && pMat2->IsValueOrEmpty(i,j))
                    pResMat->PutDouble(
                            div( pMat1->GetDouble(i,j), pMat2->GetDouble(i,j)),
                            i, j);
                else
                    pResMat->PutString(ScGlobal::GetRscString(
                                                    STR_NO_VALUE), i, j);
            }
        }
    }
    return xResMat;
}

ScMatrixRef ScInterpreter::MatPow(ScMatrix* pMat1, ScMatrix* pMat2)
{
    SCSIZE nC1, nC2, nMinC;
    SCSIZE nR1, nR2, nMinR;
    SCSIZE i, j;
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
    ScMatrixRef xResMat = GetNewMat(nMinC, nMinR);
    if (xResMat)
    {
        ScMatrix* pResMat = xResMat;
        for (i = 0; i < nMinC; i++)
        {
            for (j = 0; j < nMinR; j++)
            {
                if (pMat1->IsValueOrEmpty(i,j) && pMat2->IsValueOrEmpty(i,j))
                    pResMat->PutDouble(pow(pMat1->GetDouble(i,j),
                                           pMat2->GetDouble(i,j)), i, j);
                else
                    pResMat->PutString(ScGlobal::GetRscString(
                                                    STR_NO_VALUE), i, j);
            }
        }
    }
    return xResMat;
}

ScMatrixRef ScInterpreter::MatConcat(ScMatrix* pMat1, ScMatrix* pMat2)
{
    SCSIZE nC1, nC2, nMinC;
    SCSIZE nR1, nR2, nMinR;
    SCSIZE i, j;
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
    ScMatrixRef xResMat = GetNewMat(nMinC, nMinR);
    if (xResMat)
    {
        ScMatrix* pResMat = xResMat;
        for (i = 0; i < nMinC; i++)
        {
            for (j = 0; j < nMinR; j++)
            {
                USHORT nErr = pMat1->GetErrorIfNotString( i, j);
                if (!nErr)
                    nErr = pMat2->GetErrorIfNotString( i, j);
                if (nErr)
                    pResMat->PutError( nErr, i, j);
                else
                {
                    String aTmp( pMat1->GetString( *pFormatter, i, j));
                    aTmp += pMat2->GetString( *pFormatter, i, j);
                    pResMat->PutString( aTmp, i, j);
                }
            }
        }
    }
    return xResMat;
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
    ScMatrixRef pMat1 = NULL;
    ScMatrixRef pMat2 = NULL;
    double fVal1 = 0.0, fVal2 = 0.0;
    short nFmt1, nFmt2;
    nFmt1 = nFmt2 = NUMBERFORMAT_UNDEFINED;
    short nFmtCurrencyType = nCurFmtType;
    ULONG nFmtCurrencyIndex = nCurFmtIndex;
    short nFmtPercentType = nCurFmtType;
    if ( GetStackType() == svMatrix )
        pMat2 = GetMatrix();
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
            case NUMBERFORMAT_PERCENT :
                nFmtPercentType = NUMBERFORMAT_PERCENT;
            break;
        }
    }
    if ( GetStackType() == svMatrix )
        pMat1 = GetMatrix();
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
            case NUMBERFORMAT_PERCENT :
                nFmtPercentType = NUMBERFORMAT_PERCENT;
            break;
        }
    }
    if (pMat1 && pMat2)
    {
        ScMatrixRef pResMat = MatAdd(pMat1, pMat2);
        if (!pResMat)
            PushNoValue();
        else
            PushMatrix(pResMat);
    }
    else if (pMat1 || pMat2)
    {
        double fVal;
        ScMatrixRef pMat = pMat1;
        if (!pMat)
        {
            fVal = fVal1;
            pMat = pMat2;
        }
        else
            fVal = fVal2;
        SCSIZE nC, nR;
        pMat->GetDimensions(nC, nR);
        ScMatrixRef pResMat = GetNewMat(nC, nR);
        if (pResMat)
        {
            SCSIZE nCount = nC * nR;
            for ( SCSIZE i = 0; i < nCount; i++ )
            {
                if (pMat->IsValue(i))
                    pResMat->PutDouble( ::rtl::math::approxAdd( pMat->GetDouble(i), fVal), i);
                else
                    pResMat->PutString(ScGlobal::GetRscString(STR_NO_VALUE), i);
            }
            PushMatrix(pResMat);
        }
        else
            PushIllegalArgument();
    }
    else
        PushDouble( ::rtl::math::approxAdd( fVal1, fVal2 ) );
    if ( nFmtCurrencyType == NUMBERFORMAT_CURRENCY )
    {
        nFuncFmtType = nFmtCurrencyType;
        nFuncFmtIndex = nFmtCurrencyIndex;
    }
    else
    {
        lcl_GetDiffDateTimeFmtType( nFuncFmtType, nFmt1, nFmt2 );
        if ( nFmtPercentType == NUMBERFORMAT_PERCENT && nFuncFmtType == NUMBERFORMAT_NUMBER )
            nFuncFmtType = NUMBERFORMAT_PERCENT;
    }
}

void ScInterpreter::ScAmpersand()
{
    ScMatrixRef pMat1 = NULL;
    ScMatrixRef pMat2 = NULL;
    String sStr1, sStr2;
    if ( GetStackType() == svMatrix )
        pMat2 = GetMatrix();
    else
        sStr2 = GetString();
    if ( GetStackType() == svMatrix )
        pMat1 = GetMatrix();
    else
        sStr1 = GetString();
    if (pMat1 && pMat2)
    {
        ScMatrixRef pResMat = MatConcat(pMat1, pMat2);
        if (!pResMat)
            PushNoValue();
        else
            PushMatrix(pResMat);
    }
    else if (pMat1 || pMat2)
    {
        String sStr;
        BOOL bFlag;
        ScMatrixRef pMat = pMat1;
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
        SCSIZE nC, nR;
        pMat->GetDimensions(nC, nR);
        ScMatrixRef pResMat = GetNewMat(nC, nR);
        if (pResMat)
        {
            SCSIZE nCount = nC * nR;
            if (nGlobalError)
            {
                for ( SCSIZE i = 0; i < nCount; i++ )
                    pResMat->PutError( nGlobalError, i);
            }
            else if (bFlag)
            {
                for ( SCSIZE i = 0; i < nCount; i++ )
                {
                    USHORT nErr = pMat->GetErrorIfNotString( i);
                    if (nErr)
                        pResMat->PutError( nErr, i);
                    else
                    {
                        String aTmp( sStr);
                        aTmp += pMat->GetString( *pFormatter, i);
                        pResMat->PutString( aTmp, i);
                    }
                }
            }
            else
            {
                for ( SCSIZE i = 0; i < nCount; i++ )
                {
                    USHORT nErr = pMat->GetErrorIfNotString( i);
                    if (nErr)
                        pResMat->PutError( nErr, i);
                    else
                    {
                        String aTmp( pMat->GetString( *pFormatter, i));
                        aTmp += sStr;
                        pResMat->PutString( aTmp, i);
                    }
                }
            }
            PushMatrix(pResMat);
        }
        else
            PushIllegalArgument();
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
    ScMatrixRef pMat1 = NULL;
    ScMatrixRef pMat2 = NULL;
    double fVal1 = 0.0, fVal2 = 0.0;
    short nFmt1, nFmt2;
    nFmt1 = nFmt2 = NUMBERFORMAT_UNDEFINED;
    short nFmtCurrencyType = nCurFmtType;
    ULONG nFmtCurrencyIndex = nCurFmtIndex;
    short nFmtPercentType = nCurFmtType;
    if ( GetStackType() == svMatrix )
        pMat2 = GetMatrix();
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
            case NUMBERFORMAT_PERCENT :
                nFmtPercentType = NUMBERFORMAT_PERCENT;
            break;
        }
    }
    if ( GetStackType() == svMatrix )
        pMat1 = GetMatrix();
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
            case NUMBERFORMAT_PERCENT :
                nFmtPercentType = NUMBERFORMAT_PERCENT;
            break;
        }
    }
    if (pMat1 && pMat2)
    {
        ScMatrixRef pResMat = MatSub(pMat1, pMat2);
        if (!pResMat)
            PushNoValue();
        else
            PushMatrix(pResMat);
    }
    else if (pMat1 || pMat2)
    {
        double fVal;
        BOOL bFlag;
        ScMatrixRef pMat = pMat1;
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
        SCSIZE nC, nR;
        pMat->GetDimensions(nC, nR);
        ScMatrixRef pResMat = GetNewMat(nC, nR);
        if (pResMat)
        {   // mehr klammern wg. compiler macke
            SCSIZE nCount = nC * nR;
            if (bFlag)
            {   for ( SCSIZE i = 0; i < nCount; i++ )
                {   if (pMat->IsValue(i))
                        pResMat->PutDouble( ::rtl::math::approxSub( fVal, pMat->GetDouble(i)), i);
                    else
                        pResMat->PutString(ScGlobal::GetRscString(STR_NO_VALUE), i);
                }
            }
            else
            {   for ( SCSIZE i = 0; i < nCount; i++ )
                {   if (pMat->IsValue(i))
                        pResMat->PutDouble( ::rtl::math::approxSub( pMat->GetDouble(i), fVal), i);
                    else
                        pResMat->PutString(ScGlobal::GetRscString(STR_NO_VALUE), i);
                }
            }
            PushMatrix(pResMat);
        }
        else
            PushIllegalArgument();
    }
    else
        PushDouble( ::rtl::math::approxSub( fVal1, fVal2 ) );
    if ( nFmtCurrencyType == NUMBERFORMAT_CURRENCY )
    {
        nFuncFmtType = nFmtCurrencyType;
        nFuncFmtIndex = nFmtCurrencyIndex;
    }
    else
    {
        lcl_GetDiffDateTimeFmtType( nFuncFmtType, nFmt1, nFmt2 );
        if ( nFmtPercentType == NUMBERFORMAT_PERCENT && nFuncFmtType == NUMBERFORMAT_NUMBER )
            nFuncFmtType = NUMBERFORMAT_PERCENT;
    }
}

void ScInterpreter::ScMul()
{
    ScMatrixRef pMat1 = NULL;
    ScMatrixRef pMat2 = NULL;
    double fVal1 = 0.0, fVal2 = 0.0;
    short nFmtCurrencyType = nCurFmtType;
    ULONG nFmtCurrencyIndex = nCurFmtIndex;
    if ( GetStackType() == svMatrix )
        pMat2 = GetMatrix();
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
    if ( GetStackType() == svMatrix )
        pMat1 = GetMatrix();
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
        ScMatrixRef pResMat = MatMul(pMat1, pMat2);
        if (!pResMat)
            PushNoValue();
        else
            PushMatrix(pResMat);
    }
    else if (pMat1 || pMat2)
    {
        double fVal;
        ScMatrixRef pMat = pMat1;
        if (!pMat)
        {
            fVal = fVal1;
            pMat = pMat2;
        }
        else
            fVal = fVal2;
        SCSIZE nC, nR;
        pMat->GetDimensions(nC, nR);
        ScMatrixRef pResMat = GetNewMat(nC, nR);
        if (pResMat)
        {
            SCSIZE nCount = nC * nR;
            for ( SCSIZE i = 0; i < nCount; i++ )
                if (pMat->IsValue(i))
                    pResMat->PutDouble(pMat->GetDouble(i)*fVal, i);
                else
                    pResMat->PutString(ScGlobal::GetRscString(STR_NO_VALUE), i);
            PushMatrix(pResMat);
        }
        else
            PushIllegalArgument();
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
    ScMatrixRef pMat1 = NULL;
    ScMatrixRef pMat2 = NULL;
    double fVal1 = 0.0, fVal2 = 0.0;
    short nFmtCurrencyType = nCurFmtType;
    ULONG nFmtCurrencyIndex = nCurFmtIndex;
    short nFmtCurrencyType2 = NUMBERFORMAT_UNDEFINED;
    if ( GetStackType() == svMatrix )
        pMat2 = GetMatrix();
    else
    {
        fVal2 = GetDouble();
        // hier kein Currency uebernehmen, 123kg/456DM sind nicht DM
        nFmtCurrencyType2 = nCurFmtType;
    }
    if ( GetStackType() == svMatrix )
        pMat1 = GetMatrix();
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
        ScMatrixRef pResMat = MatDiv(pMat1, pMat2);
        if (!pResMat)
            PushNoValue();
        else
            PushMatrix(pResMat);
    }
    else if (pMat1 || pMat2)
    {
        double fVal;
        BOOL bFlag;
        ScMatrixRef pMat = pMat1;
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
        SCSIZE nC, nR;
        pMat->GetDimensions(nC, nR);
        ScMatrixRef pResMat = GetNewMat(nC, nR);
        if (pResMat)
        {
            SCSIZE nCount = nC * nR;
            if (bFlag)
            {   for ( SCSIZE i = 0; i < nCount; i++ )
                    if (pMat->IsValue(i))
                        pResMat->PutDouble( div( fVal, pMat->GetDouble(i)), i);
                    else
                        pResMat->PutString(ScGlobal::GetRscString(STR_NO_VALUE), i);
            }
            else
            {   for ( SCSIZE i = 0; i < nCount; i++ )
                    if (pMat->IsValue(i))
                        pResMat->PutDouble( div( pMat->GetDouble(i), fVal), i);
                    else
                        pResMat->PutString(ScGlobal::GetRscString(STR_NO_VALUE), i);
            }
            PushMatrix(pResMat);
        }
        else
            PushIllegalArgument();
    }
    else
    {
        PushDouble( div( fVal1, fVal2) );
    }
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
    ScMatrixRef pMat1 = NULL;
    ScMatrixRef pMat2 = NULL;
    double fVal1 = 0.0, fVal2 = 0.0;
    if ( GetStackType() == svMatrix )
        pMat2 = GetMatrix();
    else
        fVal2 = GetDouble();
    if ( GetStackType() == svMatrix )
        pMat1 = GetMatrix();
    else
        fVal1 = GetDouble();
    if (pMat1 && pMat2)
    {
        ScMatrixRef pResMat = MatPow(pMat1, pMat2);
        if (!pResMat)
            PushNoValue();
        else
            PushMatrix(pResMat);
    }
    else if (pMat1 || pMat2)
    {
        double fVal;
        BOOL bFlag;
        ScMatrixRef pMat = pMat1;
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
        SCSIZE nC, nR;
        pMat->GetDimensions(nC, nR);
        ScMatrixRef pResMat = GetNewMat(nC, nR);
        if (pResMat)
        {
            SCSIZE nCount = nC * nR;
            if (bFlag)
            {   for ( SCSIZE i = 0; i < nCount; i++ )
                    if (pMat->IsValue(i))
                        pResMat->PutDouble(pow(fVal,pMat->GetDouble(i)), i);
                    else
                        pResMat->PutString(ScGlobal::GetRscString(STR_NO_VALUE), i);
            }
            else
            {   for ( SCSIZE i = 0; i < nCount; i++ )
                    if (pMat->IsValue(i))
                        pResMat->PutDouble(pow(pMat->GetDouble(i),fVal), i);
                    else
                        pResMat->PutString(ScGlobal::GetRscString(STR_NO_VALUE), i);
            }
            PushMatrix(pResMat);
        }
        else
            PushIllegalArgument();
    }
    else
        PushDouble(pow(fVal1,fVal2));
}

void ScInterpreter::ScSumProduct()
{
    BYTE nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 1, 30 ) )
        return;

    ScMatrixRef pMat1 = NULL;
    ScMatrixRef pMat2 = NULL;
    ScMatrixRef pMat  = NULL;
    pMat2 = GetMatrix();
    if (!pMat2)
    {
        PushIllegalParameter();
        return;
    }
    SCSIZE nC, nC1;
    SCSIZE nR, nR1;
    pMat2->GetDimensions(nC, nR);
    pMat = pMat2;
    for (USHORT i = 1; i < nParamCount; i++)
    {
        pMat1 = GetMatrix();
        if (!pMat1)
        {
            PushIllegalParameter();
            return;
        }
        pMat1->GetDimensions(nC1, nR1);
        if (nC1 != nC || nR1 != nR)
        {
            PushNoValue();
            return;
        }
        ScMatrixRef pResMat = MatMul(pMat1, pMat);
        if (!pResMat)
        {
            PushNoValue();
            return;
        }
        else
            pMat = pResMat;
    }
    double fSum = 0.0;
    SCSIZE nCount = pMat->GetElementCount();
    for (SCSIZE j = 0; j < nCount; j++)
    {
        if (!pMat->IsString(j))
            fSum += pMat->GetDouble(j);
    }
    PushDouble(fSum);
}

void ScInterpreter::ScSumX2MY2()
{
    if ( !MustHaveParamCount( GetByte(), 2 ) )
        return;

    ScMatrixRef pMat1 = NULL;
    ScMatrixRef pMat2 = NULL;
    SCSIZE i, j;
    pMat2 = GetMatrix();
    pMat1 = GetMatrix();
    if (!pMat2 || !pMat1)
    {
        PushIllegalParameter();
        return;
    }
    SCSIZE nC1, nC2;
    SCSIZE nR1, nR2;
    pMat2->GetDimensions(nC2, nR2);
    pMat1->GetDimensions(nC1, nR1);
    if (nC1 != nC2 || nR1 != nR2)
    {
        PushNoValue();
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

    ScMatrixRef pMat1 = NULL;
    ScMatrixRef pMat2 = NULL;
    SCSIZE i, j;
    pMat2 = GetMatrix();
    pMat1 = GetMatrix();
    if (!pMat2 || !pMat1)
    {
        PushIllegalParameter();
        return;
    }
    SCSIZE nC1, nC2;
    SCSIZE nR1, nR2;
    pMat2->GetDimensions(nC2, nR2);
    pMat1->GetDimensions(nC1, nR1);
    if (nC1 != nC2 || nR1 != nR2)
    {
        PushNoValue();
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

    ScMatrixRef pMat1 = NULL;
    ScMatrixRef pMat2 = NULL;
    pMat2 = GetMatrix();
    pMat1 = GetMatrix();
    if (!pMat2 || !pMat1)
    {
        PushIllegalParameter();
        return;
    }
    SCSIZE nC1, nC2;
    SCSIZE nR1, nR2;
    pMat2->GetDimensions(nC2, nR2);
    pMat1->GetDimensions(nC1, nR1);
    if (nC1 != nC2 || nR1 != nR2)
    {
        PushNoValue();
        return;
    }
    ScMatrixRef pResMat = MatSub(pMat1, pMat2);
    if (!pResMat)
    {
        PushNoValue();
        return;
    }
    else
    {
        double fVal, fSum = 0.0;
        SCSIZE nCount = pResMat->GetElementCount();
        for (SCSIZE i = 0; i < nCount; i++)
            if (!pResMat->IsString(i))
            {
                fVal = pResMat->GetDouble(i);
                fSum += fVal * fVal;
            }
        PushDouble(fSum);
    }
}

void ScInterpreter::ScFrequency()
{
    if ( !MustHaveParamCount( GetByte(), 2 ) )
        return;

    vector<double>  aBinArray;
    vector<long>    aBinIndexOrder;

    GetSortArray(1, aBinArray, &aBinIndexOrder);
    SCSIZE nBinSize = aBinArray.size();
    if (nGlobalError)
    {
        PushNoValue();
        return;
    }

    vector<double>  aDataArray;
    GetSortArray(1, aDataArray);
    SCSIZE nDataSize = aDataArray.size();

    if (aDataArray.empty() || nGlobalError)
    {
        PushNoValue();
        return;
    }
    ScMatrixRef pResMat = GetNewMat(1, nBinSize+1);
    if (!pResMat)
    {
        PushIllegalArgument();
        return;
    }

    if (nBinSize != aBinIndexOrder.size())
    {
        PushIllegalArgument();
        return;
    }

    SCSIZE j;
    SCSIZE i = 0;
    for (j = 0; j < nBinSize; ++j)
    {
        SCSIZE nCount = 0;
        while (i < nDataSize && aDataArray[i] <= aBinArray[j])
        {
            ++nCount;
            ++i;
        }
        pResMat->PutDouble(static_cast<double>(nCount), aBinIndexOrder[j]);
    }
    pResMat->PutDouble(static_cast<double>(nDataSize-i), j);
    PushMatrix(pResMat);
}

BOOL ScInterpreter::RGetVariances( ScMatrix* pV, ScMatrix* pX,
        SCSIZE nC, SCSIZE nR, BOOL bSwapColRow, BOOL bZeroConstant )
{   // multiple Regression: Varianzen der Koeffizienten
    // bSwapColRow==TRUE : Koeffizienten in Zeilen statt Spalten angeordnet
    SCSIZE i, j, k;
    double sum;
    ScMatrixRef pC = GetNewMat(nC, nC);
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
        // #i61216# ScMatInv no longer modifies the original matrix, so just calling Pop() doesn't work
        pC = PopMatrix();
        if ( pC.Is() )
        {
            // Varianzen auf der Diagonalen, andere sind Kovarianzen
            for (i = 0; i < nC; i++)
                pV->PutDouble(pC->GetDouble(i, i), i);
        }
    }
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
    ScMatrixRef pMatX;
    ScMatrixRef pMatY;
    if (nParamCount >= 2)
        pMatX = GetMatrix();
    else
        pMatX = NULL;
    pMatY = GetMatrix();
    if (!pMatY)
    {
        PushIllegalParameter();
        return;
    }
    BYTE nCase;                         // 1 = normal, 2,3 = mehrfach
    SCSIZE nCX, nCY;
    SCSIZE nRX, nRY;
    SCSIZE M = 0, N = 0;
    pMatY->GetDimensions(nCY, nRY);
    SCSIZE nCountY = nCY * nRY;
    for ( SCSIZE i = 0; i < nCountY; i++ )
        if (!pMatY->IsValue(i))
        {
            PushIllegalArgument();
            return;
        }
    if (pMatX)
    {
        pMatX->GetDimensions(nCX, nRX);
        SCSIZE nCountX = nCX * nRX;
        for ( SCSIZE i = 0; i < nCountX; i++ )
            if (!pMatX->IsValue(i))
            {
                PushIllegalArgument();
                return;
            }
        if (nCX == nCY && nRX == nRY)
            nCase = 1;                  // einfache Regression
        else if (nCY != 1 && nRY != 1)
        {
            PushIllegalArgument();
            return;
        }
        else if (nCY == 1)
        {
            if (nRX != nRY)
            {
                PushIllegalArgument();
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
            PushIllegalArgument();
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
        pMatX = GetNewMat(nCY, nRY);
        if (!pMatX)
        {
            PushIllegalArgument();
            return;
        }
        for ( SCSIZE i = 1; i <= nCountY; i++ )
            pMatX->PutDouble((double)i, i-1);
        nCase = 1;
    }
    ScMatrixRef pResMat;
    if (nCase == 1)
    {
        if (!bStats)
            pResMat = GetNewMat(2,1);
        else
            pResMat = GetNewMat(2,5);
        if (!pResMat)
        {
            PushIllegalArgument();
            return;
        }
        double fCount   = 0.0;
        double fSumX    = 0.0;
        double fSumSqrX = 0.0;
        double fSumY    = 0.0;
        double fSumSqrY = 0.0;
        double fSumXY   = 0.0;
        double fValX, fValY;
        for (SCSIZE i = 0; i < nCY; i++)
            for (SCSIZE j = 0; j < nRY; j++)
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
            PushNoValue();
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
        SCSIZE i, j, k;
        if (!bStats)
            pResMat = GetNewMat(M+1,1);
        else
            pResMat = GetNewMat(M+1,5);
        if (!pResMat)
        {
            PushIllegalArgument();
            return;
        }
        ScMatrixRef pQ = GetNewMat(M+1, M+2);
        ScMatrixRef pE = GetNewMat(M+2, 1);
        ScMatrixRef pV = GetNewMat(M+1, 1);
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
                        double fVal = pMatX->GetDouble(j,k);
                        double sumXikXjk = pQ->GetDouble(j+1, i+1) +
                             Xik * fVal;
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
                        double fVal = pMatX->GetDouble(k,j);
                        double sumXkiXkj = pQ->GetDouble(j+1, i+1) +
                             Xki * fVal;
                        pQ->PutDouble( sumXkiXkj, j+1, i+1);
                        pQ->PutDouble( sumXkiXkj, i+1, j+1);
                    }
                }
            }
        }
        pQ->PutDouble((double)N, 0, 0);
        if (bConstant)
        {
            SCSIZE S, L;
            for (S = 0; S < M+1; S++)
            {
                i = S;
                while (i < M+1 && pQ->GetDouble(i, S) == 0.0)
                    i++;
                if (i >= M+1)
                {
                    PushNoValue();
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
            SCSIZE S, L;
            for (S = 1; S < M+1; S++)
            {
                i = S;
                while (i < M+1 && pQ->GetDouble(i, S) == 0.0)
                    i++;
                if (i >= M+1)
                {
                    PushNoValue();
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
    }
    PushMatrix(pResMat);
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
    ScMatrixRef pMatX;
    ScMatrixRef pMatY;
    if (nParamCount >= 2)
        pMatX = GetMatrix();
    else
        pMatX = NULL;
    pMatY = GetMatrix();
    if (!pMatY)
    {
        PushIllegalParameter();
        return;
    }
    BYTE nCase;                         // 1 = normal, 2,3 = mehrfach
    SCSIZE nCX, nCY;
    SCSIZE nRX, nRY;
    SCSIZE M = 0, N = 0;
    pMatY->GetDimensions(nCY, nRY);
    SCSIZE nCountY = nCY * nRY;
    SCSIZE nElem;
    for (nElem = 0; nElem < nCountY; nElem++)
        if (!pMatY->IsValue(nElem))
        {
            PushIllegalArgument();
            return;
        }
    for (nElem = 0; nElem < nCountY; nElem++)
    {
        double fVal = pMatY->GetDouble(nElem);
        if (fVal <= 0.0)
        {
            PushIllegalArgument();
            return;
        }
        else
            pMatY->PutDouble(log(pMatY->GetDouble(nElem)), nElem);
    }
    if (pMatX)
    {
        pMatX->GetDimensions(nCX, nRX);
        SCSIZE nCountX = nCX * nRX;
        for (nElem = 0; nElem < nCountX; nElem++)
            if (!pMatX->IsValue(nElem))
            {
                PushIllegalArgument();
                return;
            }
        if (nCX == nCY && nRX == nRY)
            nCase = 1;                  // einfache Regression
        else if (nCY != 1 && nRY != 1)
        {
            PushIllegalArgument();
            return;
        }
        else if (nCY == 1)
        {
            if (nRX != nRY)
            {
                PushIllegalArgument();
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
            PushIllegalArgument();
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
        pMatX = GetNewMat(nCY, nRY);
        if (!pMatX)
        {
            PushIllegalArgument();
            return;
        }
        for ( SCSIZE i = 1; i <= nCountY; i++ )
            pMatX->PutDouble((double)i, i-1);
        nCase = 1;
    }
    ScMatrixRef pResMat;
    if (nCase == 1)
    {
        if (!bStats)
            pResMat = GetNewMat(2,1);
        else
            pResMat = GetNewMat(2,5);
        if (!pResMat)
        {
            PushIllegalArgument();
            return;
        }
        double fCount   = 0.0;
        double fSumX    = 0.0;
        double fSumSqrX = 0.0;
        double fSumY    = 0.0;
        double fSumSqrY = 0.0;
        double fSumXY   = 0.0;
        double fValX, fValY;
        for (SCSIZE i = 0; i < nCY; i++)
            for (SCSIZE j = 0; j < nRY; j++)
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
            PushNoValue();
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
        SCSIZE i, j, k;
        if (!bStats)
            pResMat = GetNewMat(M+1,1);
        else
            pResMat = GetNewMat(M+1,5);
        if (!pResMat)
        {
            PushIllegalArgument();
            return;
        }
        ScMatrixRef pQ = GetNewMat(M+1, M+2);
        ScMatrixRef pE = GetNewMat(M+2, 1);
        ScMatrixRef pV = GetNewMat(M+1, 1);
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
            SCSIZE S, L;
            for (S = 0; S < M+1; S++)
            {
                i = S;
                while (i < M+1 && pQ->GetDouble(i, S) == 0.0)
                    i++;
                if (i >= M+1)
                {
                    PushNoValue();
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
            SCSIZE S, L;
            for (S = 1; S < M+1; S++)
            {
                i = S;
                while (i < M+1 && pQ->GetDouble(i, S) == 0.0)
                    i++;
                if (i >= M+1)
                {
                    PushNoValue();
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
    }
    PushMatrix(pResMat);
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
    ScMatrixRef pMatX;
    ScMatrixRef pMatY;
    ScMatrixRef pMatNewX;
    if (nParamCount >= 3)
        pMatNewX = GetMatrix();
    else
        pMatNewX = NULL;
    if (nParamCount >= 2)
        pMatX = GetMatrix();
    else
        pMatX = NULL;
    pMatY = GetMatrix();
    if (!pMatY)
    {
        PushIllegalParameter();
        return;
    }
    BYTE nCase;                         // 1 = normal, 2,3 = mehrfach
    SCSIZE nCX, nCY;
    SCSIZE nRX, nRY;
    SCSIZE M = 0, N = 0;
    pMatY->GetDimensions(nCY, nRY);
    SCSIZE nCountY = nCY * nRY;
    SCSIZE nElem;
    for (nElem = 0; nElem < nCountY; nElem++)
        if (!pMatY->IsValue(nElem))
        {
            PushIllegalArgument();
            return;
        }
    if (pMatX)
    {
        pMatX->GetDimensions(nCX, nRX);
        SCSIZE nCountX = nCX * nRX;
        for (nElem = 0; nElem < nCountX; nElem++)
            if (!pMatX->IsValue(nElem))
            {
                PushIllegalArgument();
                return;
            }
        if (nCX == nCY && nRX == nRY)
            nCase = 1;                  // einfache Regression
        else if (nCY != 1 && nRY != 1)
        {
            PushIllegalArgument();
            return;
        }
        else if (nCY == 1)
        {
            if (nRX != nRY)
            {
                PushIllegalArgument();
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
            PushIllegalArgument();
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
        pMatX = GetNewMat(nCY, nRY);
        nCX = nCY;
        nRX = nRY;
        if (!pMatX)
        {
            PushIllegalArgument();
            return;
        }
        for (nElem = 1; nElem <= nCountY; nElem++)
            pMatX->PutDouble((double)nElem, nElem-1);
        nCase = 1;
    }
    SCSIZE nCXN, nRXN;
    SCSIZE nCountXN;
    if (!pMatNewX)
    {
        nCXN = nCX;
        nRXN = nRX;
        nCountXN = nCXN * nRXN;
        pMatNewX = pMatX;
    }
    else
    {
        pMatNewX->GetDimensions(nCXN, nRXN);
        if ((nCase == 2 && nCX != nCXN) || (nCase == 3 && nRX != nRXN))
        {
            PushIllegalArgument();
            return;
        }
        nCountXN = nCXN * nRXN;
        for ( SCSIZE i = 0; i < nCountXN; i++ )
            if (!pMatNewX->IsValue(i))
            {
                PushIllegalArgument();
                return;
            }
    }
    ScMatrixRef pResMat;
    if (nCase == 1)
    {
        double fCount   = 0.0;
        double fSumX    = 0.0;
        double fSumSqrX = 0.0;
        double fSumY    = 0.0;
        double fSumSqrY = 0.0;
        double fSumXY   = 0.0;
        double fValX, fValY;
        SCSIZE i;
        for (i = 0; i < nCY; i++)
            for (SCSIZE j = 0; j < nRY; j++)
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
            PushNoValue();
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
            pResMat = GetNewMat(nCXN, nRXN);
            if (!pResMat)
            {
                PushIllegalArgument();
                return;
            }
            for (i = 0; i < nCountXN; i++)
                pResMat->PutDouble(pMatNewX->GetDouble(i)*m+b, i);
        }
    }
    else
    {
        SCSIZE i, j, k;
        ScMatrixRef pQ = GetNewMat(M+1, M+2);
        ScMatrixRef pE = GetNewMat(M+2, 1);
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
            SCSIZE S, L;
            for (S = 0; S < M+1; S++)
            {
                i = S;
                while (i < M+1 && pQ->GetDouble(i, S) == 0.0)
                    i++;
                if (i >= M+1)
                {
                    PushNoValue();
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
            SCSIZE S, L;
            for (S = 1; S < M+1; S++)
            {
                i = S;
                while (i < M+1 && pQ->GetDouble(i, S) == 0.0)
                    i++;
                if (i >= M+1)
                {
                    PushNoValue();
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
            pResMat = GetNewMat(1, nRXN);
            if (!pResMat)
            {
                PushIllegalArgument();
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
            pResMat = GetNewMat(nCXN, 1);
            if (!pResMat)
            {
                PushIllegalArgument();
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
    }
    PushMatrix(pResMat);
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
    ScMatrixRef pMatX;
    ScMatrixRef pMatY;
    ScMatrixRef pMatNewX;
    if (nParamCount >= 3)
        pMatNewX = GetMatrix();
    else
        pMatNewX = NULL;
    if (nParamCount >= 2)
        pMatX = GetMatrix();
    else
        pMatX = NULL;
    pMatY = GetMatrix();
    if (!pMatY)
    {
        PushIllegalParameter();
        return;
    }
    BYTE nCase;                         // 1 = normal, 2,3 = mehrfach
    SCSIZE nCX, nCY;
    SCSIZE nRX, nRY;
    SCSIZE M = 0, N = 0;
    pMatY->GetDimensions(nCY, nRY);
    SCSIZE nCountY = nCY * nRY;
    SCSIZE nElem;
    for (nElem = 0; nElem < nCountY; nElem++)
    {
        if (!pMatY->IsValue(nElem))
        {
            PushIllegalArgument();
            return;
        }
    }
    for (nElem = 0; nElem < nCountY; nElem++)
    {
        if (pMatY->GetDouble(nElem) <= 0.0)
        {
            PushIllegalArgument();
            return;
        }
        else
            pMatY->PutDouble(log(pMatY->GetDouble(nElem)), nElem);
    }
    if (pMatX)
    {
        pMatX->GetDimensions(nCX, nRX);
        SCSIZE nCountX = nCX * nRX;
        for ( SCSIZE i = 0; i < nCountX; i++ )
            if (!pMatX->IsValue(i))
            {
                PushIllegalArgument();
                return;
            }
        if (nCX == nCY && nRX == nRY)
            nCase = 1;                  // einfache Regression
        else if (nCY != 1 && nRY != 1)
        {
            PushIllegalArgument();
            return;
        }
        else if (nCY == 1)
        {
            if (nRX != nRY)
            {
                PushIllegalArgument();
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
            PushIllegalArgument();
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
        pMatX = GetNewMat(nCY, nRY);
        nCX = nCY;
        nRX = nRY;
        if (!pMatX)
        {
            PushIllegalArgument();
            return;
        }
        for (SCSIZE i = 1; i <= nCountY; i++)
            pMatX->PutDouble((double)i, i-1);
        nCase = 1;
    }
    SCSIZE nCXN, nRXN;
    SCSIZE nCountXN;
    if (!pMatNewX)
    {
        nCXN = nCX;
        nRXN = nRX;
        nCountXN = nCXN * nRXN;
        pMatNewX = pMatX;
    }
    else
    {
        pMatNewX->GetDimensions(nCXN, nRXN);
        if ((nCase == 2 && nCX != nCXN) || (nCase == 3 && nRX != nRXN))
        {
            PushIllegalArgument();
            return;
        }
        nCountXN = nCXN * nRXN;
        for ( SCSIZE i = 0; i < nCountXN; i++ )
            if (!pMatNewX->IsValue(i))
            {
                PushIllegalArgument();
                return;
            }
    }
    ScMatrixRef pResMat;
    if (nCase == 1)
    {
        double fCount   = 0.0;
        double fSumX    = 0.0;
        double fSumSqrX = 0.0;
        double fSumY    = 0.0;
        double fSumSqrY = 0.0;
        double fSumXY   = 0.0;
        double fValX, fValY;
        for (SCSIZE i = 0; i < nCY; i++)
            for (SCSIZE j = 0; j < nRY; j++)
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
            PushNoValue();
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
            pResMat = GetNewMat(nCXN, nRXN);
            if (!pResMat)
            {
                PushIllegalArgument();
                return;
            }
            for (SCSIZE i = 0; i < nCountXN; i++)
                pResMat->PutDouble(exp(pMatNewX->GetDouble(i)*m+b), i);
        }
    }
    else
    {
        SCSIZE i, j, k;
        ScMatrixRef pQ = GetNewMat(M+1, M+2);
        ScMatrixRef pE = GetNewMat(M+2, 1);
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
            SCSIZE S, L;
            for (S = 0; S < M+1; S++)
            {
                i = S;
                while (i < M+1 && pQ->GetDouble(i, S) == 0.0)
                    i++;
                if (i >= M+1)
                {
                    PushNoValue();
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
            SCSIZE S, L;
            for (S = 1; S < M+1; S++)
            {
                i = S;
                while (i < M+1 && pQ->GetDouble(i, S) == 0.0)
                    i++;
                if (i >= M+1)
                {
                    PushNoValue();
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
            pResMat = GetNewMat(1, nRXN);
            if (!pResMat)
            {
                PushIllegalArgument();
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
            pResMat = GetNewMat(nCXN, 1);
            if (!pResMat)
            {
                PushIllegalArgument();
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
    }
    PushMatrix(pResMat);
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
        const ScMatrix* pMat = pCell->GetMatrix();
        if( pMat )
        {
            SCSIZE nCl, nRw;
            pMat->GetDimensions( nCl, nRw );
            SCSIZE nC = static_cast<SCSIZE>(aPos.Col() - aAdr.Col());
            SCSIZE nR = static_cast<SCSIZE>(aPos.Row() - aAdr.Row());
            if (nC < nCl && nR < nRw)
            {
                ScMatValType nMatValType;
                const ScMatrixValue* pMatVal = pMat->Get( nC, nR, nMatValType);
                if (ScMatrix::IsStringType( nMatValType))
                {
                    if (ScMatrix::IsEmptyType( nMatValType))
                    {
                        // Not inherited (really?) and display as empty string, not 0.
                        PushTempToken( new ScEmptyCellToken( false, true));
                    }
                    else if (ScMatrix::IsEmptyPathType( nMatValType))
                    {   // result of empty FALSE jump path
                        nFuncFmtType = NUMBERFORMAT_LOGICAL;
                        PushInt(0);
                    }
                    else
                        PushString( pMatVal->GetString() );
                }
                else
                {
                    PushDouble(pMatVal->fVal);  // handles DoubleError
                    pDok->GetNumberFormatInfo( nCurFmtType, nCurFmtIndex, aAdr, pCell );
                    nFuncFmtType = nCurFmtType;
                    nFuncFmtIndex = nCurFmtIndex;
                }
            }
            else
                PushNA();
        }
        else
        {
            // If not a result matrix, obtain the cell value.
            USHORT nErr = pCell->GetErrCode();
            if (nErr)
                PushError( nErr );
            else if( pCell->IsValue() )
                PushDouble( pCell->GetValue() );
            else
            {
                String aVal;
                pCell->GetString( aVal );
                PushString( aVal );
            }
            pDok->GetNumberFormatInfo( nCurFmtType, nCurFmtIndex, aAdr, pCell );
            nFuncFmtType = nCurFmtType;
            nFuncFmtIndex = nCurFmtIndex;
        }
    }
    else
        PushError( errNoRef );
}

void ScInterpreter::ScInfo()
{
    if( MustHaveParamCount( GetByte(), 1 ) )
    {
        String aStr = GetString();
        ScCellKeywordTranslator::transKeyword(aStr, ScGlobal::pLocale, ocInfo);
        if( aStr.EqualsAscii( "SYSTEM" ) )
            PushString( String( RTL_CONSTASCII_USTRINGPARAM( SC_INFO_OSVERSION ) ) );
        else if( aStr.EqualsAscii( "OSVERSION" ) )
            PushString( String( RTL_CONSTASCII_USTRINGPARAM( "Windows (32-bit) NT 5.01" ) ) );
        else if( aStr.EqualsAscii( "RELEASE" ) )
            PushString( ::utl::Bootstrap::getBuildIdData( ::rtl::OUString() ) );
        else if( aStr.EqualsAscii( "NUMFILE" ) )
            PushDouble( 1 );
        else if( aStr.EqualsAscii( "RECALC" ) )
            PushString( ScGlobal::GetRscString( pDok->GetAutoCalc() ? STR_RECALC_AUTO : STR_RECALC_MANUAL ) );
        else
            PushIllegalArgument();
    }
}
