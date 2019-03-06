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

#include <rtl/math.hxx>
#include <string.h>
#include <math.h>

#ifdef DEBUG_SC_LUP_DECOMPOSITION
#include <stdio.h>
#endif

#include <unotools/bootstrap.hxx>
#include <svl/zforlist.hxx>

#include <interpre.hxx>
#include <global.hxx>
#include <compiler.hxx>
#include <formulacell.hxx>
#include <document.hxx>
#include <dociter.hxx>
#include <scmatrix.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <cellkeytranslator.hxx>
#include <formulagroup.hxx>

#include <vector>

using ::std::vector;
using namespace formula;

namespace {

struct MatrixAdd
{
    double operator() (const double& lhs, const double& rhs) const
    {
        return ::rtl::math::approxAdd( lhs,rhs);
    }
};

struct MatrixSub
{
    double operator() (const double& lhs, const double& rhs) const
    {
        return ::rtl::math::approxSub( lhs,rhs);
    }
};

struct MatrixMul
{
    double operator() (const double& lhs, const double& rhs) const
    {
        return lhs * rhs;
    }
};

struct MatrixDiv
{
    double operator() (const double& lhs, const double& rhs) const
    {
        return ScInterpreter::div( lhs,rhs);
    }
};

struct MatrixPow
{
    double operator() (const double& lhs, const double& rhs) const
    {
        return ::pow( lhs,rhs);
    }
};

// Multiply n x m Mat A with m x l Mat B to n x l Mat R
void lcl_MFastMult(const ScMatrixRef& pA, const ScMatrixRef& pB, const ScMatrixRef& pR,
                   SCSIZE n, SCSIZE m, SCSIZE l)
{
    double sum;
    for (SCSIZE row = 0; row < n; row++)
    {
        for (SCSIZE col = 0; col < l; col++)
        {   // result element(col, row) =sum[ (row of A) * (column of B)]
            sum = 0.0;
            for (SCSIZE k = 0; k < m; k++)
                sum += pA->GetDouble(k,row) * pB->GetDouble(col,k);
            pR->PutDouble(sum, col, row);
        }
    }
}

}

double ScInterpreter::ScGetGCD(double fx, double fy)
{
    // By ODFF definition GCD(0,a) => a. This is also vital for the code in
    // ScGCD() to work correctly with a preset fy=0.0
    if (fy == 0.0)
        return fx;
    else if (fx == 0.0)
        return fy;
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
        double fx, fy = 0.0;
        ScRange aRange;
        size_t nRefInList = 0;
        while (nGlobalError == FormulaError::NONE && nParamCount-- > 0)
        {
            switch (GetStackType())
            {
                case svDouble :
                case svString:
                case svSingleRef:
                {
                    fx = ::rtl::math::approxFloor( GetDouble());
                    if (fx < 0.0)
                    {
                        PushIllegalArgument();
                        return;
                    }
                    fy = ScGetGCD(fx, fy);
                }
                break;
                case svDoubleRef :
                case svRefList :
                {
                    FormulaError nErr = FormulaError::NONE;
                    PopDoubleRef( aRange, nParamCount, nRefInList);
                    double nCellVal;
                    ScValueIterator aValIter( pDok, aRange, mnSubTotalFlags );
                    if (aValIter.GetFirst(nCellVal, nErr))
                    {
                        do
                        {
                            fx = ::rtl::math::approxFloor( nCellVal);
                            if (fx < 0.0)
                            {
                                PushIllegalArgument();
                                return;
                            }
                            fy = ScGetGCD(fx, fy);
                        } while (nErr == FormulaError::NONE && aValIter.GetNext(nCellVal, nErr));
                    }
                    SetError(nErr);
                }
                break;
                case svMatrix :
                case svExternalSingleRef:
                case svExternalDoubleRef:
                {
                    ScMatrixRef pMat = GetMatrix();
                    if (pMat)
                    {
                        SCSIZE nC, nR;
                        pMat->GetDimensions(nC, nR);
                        if (nC == 0 || nR == 0)
                            SetError(FormulaError::IllegalArgument);
                        else
                        {
                         double nVal = pMat->GetGcd();
                         fy = ScGetGCD(nVal,fy);
                        }
                    }
                }
                break;
                default : SetError(FormulaError::IllegalParameter); break;
            }
        }
        PushDouble(fy);
    }
}

void ScInterpreter:: ScLCM()
{
    short nParamCount = GetByte();
    if ( MustHaveParamCountMin( nParamCount, 1 ) )
    {
        double fx, fy = 1.0;
        ScRange aRange;
        size_t nRefInList = 0;
        while (nGlobalError == FormulaError::NONE && nParamCount-- > 0)
        {
            switch (GetStackType())
            {
                case svDouble :
                case svString:
                case svSingleRef:
                {
                    fx = ::rtl::math::approxFloor( GetDouble());
                    if (fx < 0.0)
                    {
                        PushIllegalArgument();
                        return;
                    }
                    if (fx == 0.0 || fy == 0.0)
                        fy = 0.0;
                    else
                        fy = fx * fy / ScGetGCD(fx, fy);
                }
                break;
                case svDoubleRef :
                case svRefList :
                {
                    FormulaError nErr = FormulaError::NONE;
                    PopDoubleRef( aRange, nParamCount, nRefInList);
                    double nCellVal;
                    ScValueIterator aValIter( pDok, aRange, mnSubTotalFlags );
                    if (aValIter.GetFirst(nCellVal, nErr))
                    {
                        do
                        {
                            fx = ::rtl::math::approxFloor( nCellVal);
                            if (fx < 0.0)
                            {
                                PushIllegalArgument();
                                return;
                            }
                            if (fx == 0.0 || fy == 0.0)
                                fy = 0.0;
                            else
                                fy = fx * fy / ScGetGCD(fx, fy);
                        } while (nErr == FormulaError::NONE && aValIter.GetNext(nCellVal, nErr));
                    }
                    SetError(nErr);
                }
                break;
                case svMatrix :
                case svExternalSingleRef:
                case svExternalDoubleRef:
                {
                    ScMatrixRef pMat = GetMatrix();
                    if (pMat)
                    {
                        SCSIZE nC, nR;
                        pMat->GetDimensions(nC, nR);
                        if (nC == 0 || nR == 0)
                            SetError(FormulaError::IllegalArgument);
                        else
                        {
                         double nVal = pMat->GetLcm();
                         fy = (nVal * fy ) / ScGetGCD(nVal, fy);
                        }
                    }
                }
                break;
                default : SetError(FormulaError::IllegalParameter); break;
            }
        }
        PushDouble(fy);
    }
}

void ScInterpreter::MakeMatNew(ScMatrixRef& rMat, SCSIZE nC, SCSIZE nR)
{
    rMat->SetErrorInterpreter( this);
    // A temporary matrix is mutable and ScMatrix::CloneIfConst() returns the
    // very matrix.
    rMat->SetMutable();
    SCSIZE nCols, nRows;
    rMat->GetDimensions( nCols, nRows);
    if ( nCols != nC || nRows != nR )
    {   // arbitrary limit of elements exceeded
        SetError( FormulaError::MatrixSize);
        rMat.reset();
    }
}

ScMatrixRef ScInterpreter::GetNewMat(SCSIZE nC, SCSIZE nR, bool bEmpty)
{
    ScMatrixRef pMat;
    if (bEmpty)
        pMat = new ScMatrix(nC, nR);
    else
        pMat = new ScMatrix(nC, nR, 0.0);
    MakeMatNew(pMat, nC, nR);
    return pMat;
}

ScMatrixRef ScInterpreter::GetNewMat(SCSIZE nC, SCSIZE nR, const std::vector<double>& rValues)
{
    ScMatrixRef pMat(new ScMatrix(nC, nR, rValues));
    MakeMatNew(pMat, nC, nR);
    return pMat;
}

ScMatrixRef ScInterpreter::CreateMatrixFromDoubleRef( const FormulaToken* pToken,
        SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
        SCCOL nCol2, SCROW nRow2, SCTAB nTab2 )
{
    if (nTab1 != nTab2 || nGlobalError != FormulaError::NONE)
    {
        // Not a 2D matrix.
        SetError(FormulaError::IllegalParameter);
        return nullptr;
    }

    SCSIZE nMatCols = static_cast<SCSIZE>(nCol2 - nCol1 + 1);
    SCSIZE nMatRows = static_cast<SCSIZE>(nRow2 - nRow1 + 1);

    if (!ScMatrix::IsSizeAllocatable( nMatCols, nMatRows))
    {
        SetError(FormulaError::MatrixSize);
        return nullptr;
    }

    ScTokenMatrixMap::const_iterator aIter;
    if (pToken && pTokenMatrixMap && ((aIter = pTokenMatrixMap->find( pToken)) != pTokenMatrixMap->end()))
    {
        /* XXX casting const away here is ugly; ScMatrixToken (to which the
         * result of this function usually is assigned) should not be forced to
         * carry a ScConstMatrixRef though.
         * TODO: a matrix already stored in pTokenMatrixMap should be
         * read-only and have a copy-on-write mechanism. Previously all tokens
         * were modifiable so we're already better than before ... */
        return const_cast<FormulaToken*>((*aIter).second.get())->GetMatrix();
    }

    ScMatrixRef pMat = GetNewMat( nMatCols, nMatRows, true);
    if (!pMat || nGlobalError != FormulaError::NONE)
        return nullptr;

    pDok->FillMatrix(*pMat, nTab1, nCol1, nRow1, nCol2, nRow2);

    if (pToken && pTokenMatrixMap)
        pTokenMatrixMap->emplace(pToken, new ScMatrixToken( pMat));

    return pMat;
}

ScMatrixRef ScInterpreter::GetMatrix()
{
    ScMatrixRef pMat = nullptr;
    switch (GetRawStackType())
    {
        case svSingleRef :
        {
            ScAddress aAdr;
            PopSingleRef( aAdr );
            pMat = GetNewMat(1, 1);
            if (pMat)
            {
                ScRefCellValue aCell(*pDok, aAdr);
                if (aCell.hasEmptyValue())
                    pMat->PutEmpty(0, 0);
                else if (aCell.hasNumeric())
                    pMat->PutDouble(GetCellValue(aAdr, aCell), 0);
                else
                {
                    svl::SharedString aStr;
                    GetCellString(aStr, aCell);
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
            const formula::FormulaToken* p = sp ? pStack[sp-1] : nullptr;
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
                if ( nGlobalError != FormulaError::NONE )
                {
                    fVal = CreateDoubleError( nGlobalError);
                    nGlobalError = FormulaError::NONE;
                }
                pMat->PutDouble( fVal, 0);
            }
        }
        break;
        case svString :
        {
            svl::SharedString aStr = GetString();
            pMat = GetNewMat( 1, 1);
            if ( pMat )
            {
                if ( nGlobalError != FormulaError::NONE )
                {
                    double fVal = CreateDoubleError( nGlobalError);
                    pMat->PutDouble( fVal, 0);
                    nGlobalError = FormulaError::NONE;
                }
                else
                    pMat->PutString(aStr, 0);
            }
        }
        break;
        case svExternalSingleRef:
        {
            ScExternalRefCache::TokenRef pToken;
            PopExternalSingleRef(pToken);
            pMat = GetNewMat( 1, 1, true);
            if (!pMat)
                break;
            if (nGlobalError != FormulaError::NONE)
            {
                pMat->PutError( nGlobalError, 0, 0);
                nGlobalError = FormulaError::NONE;
                break;
            }
            switch (pToken->GetType())
            {
                case svError:
                    pMat->PutError( pToken->GetError(), 0, 0);
                break;
                case svDouble:
                    pMat->PutDouble( pToken->GetDouble(), 0, 0);
                break;
                case svString:
                    pMat->PutString( pToken->GetString(), 0, 0);
                break;
                default:
                    ;   // nothing, empty element matrix
            }
        }
        break;
        case svExternalDoubleRef:
            PopExternalDoubleRef(pMat);
        break;
        default:
            PopError();
            SetError( FormulaError::IllegalArgument);
        break;
    }
    return pMat;
}

ScMatrixRef ScInterpreter::GetMatrix( short & rParam, size_t & rRefInList )
{
    switch (GetRawStackType())
    {
        case svRefList:
            {
                ScRange aRange( ScAddress::INITIALIZE_INVALID );
                PopDoubleRef( aRange, rParam, rRefInList);
                if (nGlobalError != FormulaError::NONE)
                    return nullptr;

                SCCOL nCol1, nCol2;
                SCROW nRow1, nRow2;
                SCTAB nTab1, nTab2;
                aRange.GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
                return CreateMatrixFromDoubleRef( nullptr, nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
            }
        break;
        default:
            return GetMatrix();
    }
}

sc::RangeMatrix ScInterpreter::GetRangeMatrix()
{
    sc::RangeMatrix aRet;
    switch (GetRawStackType())
    {
        case svMatrix:
            aRet = PopRangeMatrix();
        break;
        default:
            aRet.mpMat = GetMatrix();
    }
    return aRet;
}

void ScInterpreter::ScMatValue()
{
    if ( MustHaveParamCount( GetByte(), 3 ) )
    {
        // 0 to count-1
        // Theoretically we could have GetSize() instead of GetUInt32(), but
        // really, practically ...
        SCSIZE nR = static_cast<SCSIZE>(GetUInt32());
        SCSIZE nC = static_cast<SCSIZE>(GetUInt32());
        if (nGlobalError != FormulaError::NONE)
        {
            PushError( nGlobalError);
            return;
        }
        switch (GetStackType())
        {
            case svSingleRef :
            {
                ScAddress aAdr;
                PopSingleRef( aAdr );
                ScRefCellValue aCell(*pDok, aAdr);
                if (aCell.meType == CELLTYPE_FORMULA)
                {
                    FormulaError nErrCode = aCell.mpFormula->GetErrCode();
                    if (nErrCode != FormulaError::NONE)
                        PushError( nErrCode);
                    else
                    {
                        const ScMatrix* pMat = aCell.mpFormula->GetMatrix();
                        CalculateMatrixValue(pMat,nC,nR);
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
                    ScRefCellValue aCell(*pDok, aAdr);
                    if (aCell.hasNumeric())
                        PushDouble(GetCellValue(aAdr, aCell));
                    else
                    {
                        svl::SharedString aStr;
                        GetCellString(aStr, aCell);
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
                CalculateMatrixValue(pMat.get(),nC,nR);
            }
            break;
            default:
                PopError();
                PushIllegalParameter();
            break;
        }
    }
}
void ScInterpreter::CalculateMatrixValue(const ScMatrix* pMat,SCSIZE nC,SCSIZE nR)
{
    if (pMat)
    {
        SCSIZE nCl, nRw;
        pMat->GetDimensions(nCl, nRw);
        if (nC < nCl && nR < nRw)
        {
            const ScMatrixValue nMatVal = pMat->Get( nC, nR);
            ScMatValType nMatValType = nMatVal.nType;
            if (ScMatrix::IsNonValueType( nMatValType))
                PushString( nMatVal.GetString() );
            else
                PushDouble(nMatVal.fVal);
                // also handles DoubleError
        }
        else
            PushNoValue();
    }
    else
        PushNoValue();
}

void ScInterpreter::ScEMat()
{
    if ( MustHaveParamCount( GetByte(), 1 ) )
    {
        SCSIZE nDim = static_cast<SCSIZE>(GetUInt32());
        if (nGlobalError != FormulaError::NONE || nDim == 0)
            PushIllegalArgument();
        else if (!ScMatrix::IsSizeAllocatable( nDim, nDim))
            PushError( FormulaError::MatrixSize);
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

void ScInterpreter::MEMat(const ScMatrixRef& mM, SCSIZE n)
{
    mM->FillDouble(0.0, 0, 0, n-1, n-1);
    for (SCSIZE i = 0; i < n; i++)
        mM->PutDouble(1.0, i, i);
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
 * permutations occurred, or -1 if odd, which is the sign of the determinant.
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
    // "Recursion" on the diagonal.
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
            double fNum = mA->GetDouble( k, i);
            double fDen = mA->GetDouble( k, k);
            mA->PutDouble( fNum/fDen, k, i);
            for (SCSIZE j = k+1; j < n; ++j)
                mA->PutDouble( ( mA->GetDouble( j, i) * fDen  -
                            fNum * mA->GetDouble( j, k) ) / fDen, j, i);
        }
    }
#ifdef DEBUG_SC_LUP_DECOMPOSITION
    fprintf( stderr, "\n%s\n", "lcl_LUP_decompose(): LU");
    for (SCSIZE i=0; i < n; ++i)
    {
        for (SCSIZE j=0; j < n; ++j)
            fprintf( stderr, "%8.2g  ", mA->GetDouble( j, i));
        fprintf( stderr, "\n%s\n", "");
    }
    fprintf( stderr, "\n%s\n", "lcl_LUP_decompose(): P");
    for (SCSIZE j=0; j < n; ++j)
        fprintf( stderr, "%5u ", (unsigned)P[j]);
    fprintf( stderr, "\n%s\n", "");
#endif

    bool bSingular=false;
    for (SCSIZE i=0; i<n && !bSingular; i++)
        bSingular = (mA->GetDouble(i,i)) == 0.0;
    if (bSingular)
        nSign = 0;

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
#ifdef DEBUG_SC_LUP_DECOMPOSITION
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
        if ( nC != nR || nC == 0 )
            PushIllegalArgument();
        else if (!ScMatrix::IsSizeAllocatable( nC, nR))
            PushError( FormulaError::MatrixSize);
        else
        {
            // LUP decomposition is done inplace, use copy.
            ScMatrixRef xLU = pMat->Clone();
            if (!xLU)
                PushError( FormulaError::CodeOverflow);
            else
            {
                ::std::vector< SCSIZE> P(nR);
                int nDetSign = lcl_LUP_decompose( xLU.get(), nR, P);
                if (!nDetSign)
                    PushInt(0);     // singular matrix
                else
                {
                    // In an LU matrix the determinant is simply the product of
                    // all diagonal elements.
                    double fDet = nDetSign;
                    for (SCSIZE i=0; i < nR; ++i)
                        fDet *= xLU->GetDouble( i, i);
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

        if (ScCalcConfig::isOpenCLEnabled())
        {
            sc::FormulaGroupInterpreter *pInterpreter = sc::FormulaGroupInterpreter::getStatic();
            if (pInterpreter != nullptr)
            {
                ScMatrixRef xResMat = pInterpreter->inverseMatrix(*pMat);
                if (xResMat)
                {
                    PushMatrix(xResMat);
                    return;
                }
            }
        }

        if ( nC != nR || nC == 0 )
            PushIllegalArgument();
        else if (!ScMatrix::IsSizeAllocatable( nC, nR))
            PushError( FormulaError::MatrixSize);
        else
        {
            // LUP decomposition is done inplace, use copy.
            ScMatrixRef xLU = pMat->Clone();
            // The result matrix.
            ScMatrixRef xY = GetNewMat( nR, nR);
            if (!xLU || !xY)
                PushError( FormulaError::CodeOverflow);
            else
            {
                ::std::vector< SCSIZE> P(nR);
                int nDetSign = lcl_LUP_decompose( xLU.get(), nR, P);
                if (!nDetSign)
                    PushIllegalArgument();
                else
                {
                    // Solve equation for each column.
                    ::std::vector< double> B(nR);
                    ::std::vector< double> X(nR);
                    for (SCSIZE j=0; j < nR; ++j)
                    {
                        for (SCSIZE i=0; i < nR; ++i)
                            B[i] = 0.0;
                        B[j] = 1.0;
                        lcl_LUP_solve( xLU.get(), nR, P, B, X);
                        for (SCSIZE i=0; i < nR; ++i)
                            xY->PutDouble( X[i], j, i);
                    }
#ifdef DEBUG_SC_LUP_DECOMPOSITION
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
                    const double fInvEpsilon = 1.0E-7;
                    ScMatrixRef xR = GetNewMat( nR, nR);
                    if (xR)
                    {
                        ScMatrix* pR = xR.get();
                        lcl_MFastMult( pMat, xY.get(), pR, nR, nR, nR);
                        fprintf( stderr, "\n%s\n", "ScMatInv(): mult-identity");
                        for (SCSIZE i=0; i < nR; ++i)
                        {
                            for (SCSIZE j=0; j < nR; ++j)
                            {
                                double fTmp = pR->GetDouble( j, i);
                                fprintf( stderr, "%8.2g  ", fTmp);
                                if (fabs( fTmp - (i == j)) > fInvEpsilon)
                                    SetError( FormulaError::IllegalArgument);
                            }
                        fprintf( stderr, "\n%s\n", "");
                        }
                    }
#endif
                    if (nGlobalError != FormulaError::NONE)
                        PushError( nGlobalError);
                    else
                        PushMatrix( xY);
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

/** Minimum extent of one result matrix dimension.
    For a row or column vector to be replicated the larger matrix dimension is
    returned, else the smaller dimension.
 */
static SCSIZE lcl_GetMinExtent( SCSIZE n1, SCSIZE n2 )
{
    if (n1 == 1)
        return n2;
    else if (n2 == 1)
        return n1;
    else if (n1 < n2)
        return n1;
    else
        return n2;
}

template<class Function>
static ScMatrixRef lcl_MatrixCalculation(
    const ScMatrix& rMat1, const ScMatrix& rMat2, ScInterpreter* pInterpreter)
{
    static const Function Op;

    SCSIZE nC1, nC2, nMinC;
    SCSIZE nR1, nR2, nMinR;
    SCSIZE i, j;
    rMat1.GetDimensions(nC1, nR1);
    rMat2.GetDimensions(nC2, nR2);
    nMinC = lcl_GetMinExtent( nC1, nC2);
    nMinR = lcl_GetMinExtent( nR1, nR2);
    ScMatrixRef xResMat = pInterpreter->GetNewMat(nMinC, nMinR);
    if (xResMat)
    {
        for (i = 0; i < nMinC; i++)
        {
            for (j = 0; j < nMinR; j++)
            {
                bool bVal1 = rMat1.IsValueOrEmpty(i,j);
                bool bVal2 = rMat2.IsValueOrEmpty(i,j);
                FormulaError nErr;
                if (bVal1 && bVal2)
                {
                    double d = Op(rMat1.GetDouble(i,j), rMat2.GetDouble(i,j));
                    xResMat->PutDouble( d, i, j);
                }
                else if (((nErr = rMat1.GetErrorIfNotString(i,j)) != FormulaError::NONE) ||
                         ((nErr = rMat2.GetErrorIfNotString(i,j)) != FormulaError::NONE))
                {
                    xResMat->PutError( nErr, i, j);
                }
                else if ((!bVal1 && rMat1.IsStringOrEmpty(i,j)) || (!bVal2 && rMat2.IsStringOrEmpty(i,j)))
                {
                    FormulaError nError1 = FormulaError::NONE;
                    SvNumFormatType nFmt1 = SvNumFormatType::ALL;
                    double fVal1 = (bVal1 ? rMat1.GetDouble(i,j) :
                            pInterpreter->ConvertStringToValue( rMat1.GetString(i,j).getString(), nError1, nFmt1));

                    FormulaError nError2 = FormulaError::NONE;
                    SvNumFormatType nFmt2 = SvNumFormatType::ALL;
                    double fVal2 = (bVal2 ? rMat2.GetDouble(i,j) :
                            pInterpreter->ConvertStringToValue( rMat2.GetString(i,j).getString(), nError2, nFmt2));

                    if (nError1 != FormulaError::NONE)
                        xResMat->PutError( nError1, i, j);
                    else if (nError2 != FormulaError::NONE)
                        xResMat->PutError( nError2, i, j);
                    else
                    {
                        double d = Op( fVal1, fVal2);
                        xResMat->PutDouble( d, i, j);
                    }
                }
                else
                    xResMat->PutError( FormulaError::NoValue, i, j);
            }
        }
    }
    return xResMat;
}

ScMatrixRef ScInterpreter::MatConcat(const ScMatrixRef& pMat1, const ScMatrixRef& pMat2)
{
    SCSIZE nC1, nC2, nMinC;
    SCSIZE nR1, nR2, nMinR;
    pMat1->GetDimensions(nC1, nR1);
    pMat2->GetDimensions(nC2, nR2);
    nMinC = lcl_GetMinExtent( nC1, nC2);
    nMinR = lcl_GetMinExtent( nR1, nR2);
    ScMatrixRef xResMat = GetNewMat(nMinC, nMinR);
    if (xResMat)
    {
        xResMat->MatConcat(nMinC, nMinR, pMat1, pMat2, *pFormatter, pDok->GetSharedStringPool());
    }
    return xResMat;
}

// for DATE, TIME, DATETIME
static void lcl_GetDiffDateTimeFmtType( SvNumFormatType& nFuncFmt, SvNumFormatType nFmt1, SvNumFormatType nFmt2 )
{
    if ( nFmt1 != SvNumFormatType::UNDEFINED || nFmt2 != SvNumFormatType::UNDEFINED )
    {
        if ( nFmt1 == nFmt2 )
        {
            if ( nFmt1 == SvNumFormatType::TIME || nFmt1 == SvNumFormatType::DATETIME )
                nFuncFmt = SvNumFormatType::TIME;   // times result in time
            // else: nothing special, number (date - date := days)
        }
        else if ( nFmt1 == SvNumFormatType::UNDEFINED )
            nFuncFmt = nFmt2;   // e.g. date + days := date
        else if ( nFmt2 == SvNumFormatType::UNDEFINED )
            nFuncFmt = nFmt1;
        else
        {
            if ( nFmt1 == SvNumFormatType::DATE || nFmt2 == SvNumFormatType::DATE ||
                nFmt1 == SvNumFormatType::DATETIME || nFmt2 == SvNumFormatType::DATETIME )
            {
                if ( nFmt1 == SvNumFormatType::TIME || nFmt2 == SvNumFormatType::TIME )
                    nFuncFmt = SvNumFormatType::DATETIME;   // date + time
            }
        }
    }
}

void ScInterpreter::ScAdd()
{
    CalculateAddSub(false);
}

void ScInterpreter::CalculateAddSub(bool _bSub)
{
    ScMatrixRef pMat1 = nullptr;
    ScMatrixRef pMat2 = nullptr;
    double fVal1 = 0.0, fVal2 = 0.0;
    SvNumFormatType nFmt1, nFmt2;
    nFmt1 = nFmt2 = SvNumFormatType::UNDEFINED;
    SvNumFormatType nFmtCurrencyType = nCurFmtType;
    sal_uLong nFmtCurrencyIndex = nCurFmtIndex;
    SvNumFormatType nFmtPercentType = nCurFmtType;
    if ( GetStackType() == svMatrix )
        pMat2 = GetMatrix();
    else
    {
        fVal2 = GetDouble();
        switch ( nCurFmtType )
        {
            case SvNumFormatType::DATE :
            case SvNumFormatType::TIME :
            case SvNumFormatType::DATETIME :
                nFmt2 = nCurFmtType;
            break;
            case SvNumFormatType::CURRENCY :
                nFmtCurrencyType = nCurFmtType;
                nFmtCurrencyIndex = nCurFmtIndex;
            break;
            case SvNumFormatType::PERCENT :
                nFmtPercentType = SvNumFormatType::PERCENT;
            break;
            default: break;
        }
    }
    if ( GetStackType() == svMatrix )
        pMat1 = GetMatrix();
    else
    {
        fVal1 = GetDouble();
        switch ( nCurFmtType )
        {
            case SvNumFormatType::DATE :
            case SvNumFormatType::TIME :
            case SvNumFormatType::DATETIME :
                nFmt1 = nCurFmtType;
            break;
            case SvNumFormatType::CURRENCY :
                nFmtCurrencyType = nCurFmtType;
                nFmtCurrencyIndex = nCurFmtIndex;
            break;
            case SvNumFormatType::PERCENT :
                nFmtPercentType = SvNumFormatType::PERCENT;
            break;
            default: break;
        }
    }
    if (pMat1 && pMat2)
    {
        ScMatrixRef pResMat;
        if ( _bSub )
        {
            pResMat = lcl_MatrixCalculation<MatrixSub>( *pMat1, *pMat2, this);
        }
        else
        {
            pResMat = lcl_MatrixCalculation<MatrixAdd>( *pMat1, *pMat2, this);
        }

        if (!pResMat)
            PushNoValue();
        else
            PushMatrix(pResMat);
    }
    else if (pMat1 || pMat2)
    {
        double fVal;
        bool bFlag;
        ScMatrixRef pMat = pMat1;
        if (!pMat)
        {
            fVal = fVal1;
            pMat = pMat2;
            bFlag = true;           // double - Matrix
        }
        else
        {
            fVal = fVal2;
            bFlag = false;          // Matrix - double
        }
        SCSIZE nC, nR;
        pMat->GetDimensions(nC, nR);
        ScMatrixRef pResMat = GetNewMat(nC, nR, true);
        if (pResMat)
        {
            if (_bSub)
            {
                pMat->SubOp( bFlag, fVal, *pResMat);
            }
            else
            {
                pMat->AddOp( fVal, *pResMat);
            }
            PushMatrix(pResMat);
        }
        else
            PushIllegalArgument();
    }
    else
    {
        // Determine nFuncFmtType type before PushDouble().
        if ( nFmtCurrencyType == SvNumFormatType::CURRENCY )
        {
            nFuncFmtType = nFmtCurrencyType;
            nFuncFmtIndex = nFmtCurrencyIndex;
        }
        else
        {
            lcl_GetDiffDateTimeFmtType( nFuncFmtType, nFmt1, nFmt2 );
            if (nFmtPercentType == SvNumFormatType::PERCENT && nFuncFmtType == SvNumFormatType::NUMBER)
                nFuncFmtType = SvNumFormatType::PERCENT;
        }
        if ( _bSub )
            PushDouble( ::rtl::math::approxSub( fVal1, fVal2 ) );
        else
            PushDouble( ::rtl::math::approxAdd( fVal1, fVal2 ) );
    }
}

void ScInterpreter::ScAmpersand()
{
    ScMatrixRef pMat1 = nullptr;
    ScMatrixRef pMat2 = nullptr;
    OUString sStr1, sStr2;
    if ( GetStackType() == svMatrix )
        pMat2 = GetMatrix();
    else
        sStr2 = GetString().getString();
    if ( GetStackType() == svMatrix )
        pMat1 = GetMatrix();
    else
        sStr1 = GetString().getString();
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
        OUString sStr;
        bool bFlag;
        ScMatrixRef pMat = pMat1;
        if (!pMat)
        {
            sStr = sStr1;
            pMat = pMat2;
            bFlag = true;           // double - Matrix
        }
        else
        {
            sStr = sStr2;
            bFlag = false;          // Matrix - double
        }
        SCSIZE nC, nR;
        pMat->GetDimensions(nC, nR);
        ScMatrixRef pResMat = GetNewMat(nC, nR);
        if (pResMat)
        {
            if (nGlobalError != FormulaError::NONE)
            {
                for (SCSIZE i = 0; i < nC; ++i)
                    for (SCSIZE j = 0; j < nR; ++j)
                        pResMat->PutError( nGlobalError, i, j);
            }
            else if (bFlag)
            {
                for (SCSIZE i = 0; i < nC; ++i)
                    for (SCSIZE j = 0; j < nR; ++j)
                    {
                        FormulaError nErr = pMat->GetErrorIfNotString( i, j);
                        if (nErr != FormulaError::NONE)
                            pResMat->PutError( nErr, i, j);
                        else
                        {
                            OUString aTmp = sStr;
                            aTmp += pMat->GetString(*pFormatter, i, j).getString();
                            pResMat->PutString(mrStrPool.intern(aTmp), i, j);
                        }
                    }
            }
            else
            {
                for (SCSIZE i = 0; i < nC; ++i)
                    for (SCSIZE j = 0; j < nR; ++j)
                    {
                        FormulaError nErr = pMat->GetErrorIfNotString( i, j);
                        if (nErr != FormulaError::NONE)
                            pResMat->PutError( nErr, i, j);
                        else
                        {
                            OUString aTmp = pMat->GetString(*pFormatter, i, j).getString();
                            aTmp += sStr;
                            pResMat->PutString(mrStrPool.intern(aTmp), i, j);
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
    CalculateAddSub(true);
}

void ScInterpreter::ScMul()
{
    ScMatrixRef pMat1 = nullptr;
    ScMatrixRef pMat2 = nullptr;
    double fVal1 = 0.0, fVal2 = 0.0;
    SvNumFormatType nFmtCurrencyType = nCurFmtType;
    sal_uLong nFmtCurrencyIndex = nCurFmtIndex;
    if ( GetStackType() == svMatrix )
        pMat2 = GetMatrix();
    else
    {
        fVal2 = GetDouble();
        switch ( nCurFmtType )
        {
            case SvNumFormatType::CURRENCY :
                nFmtCurrencyType = nCurFmtType;
                nFmtCurrencyIndex = nCurFmtIndex;
            break;
            default: break;
        }
    }
    if ( GetStackType() == svMatrix )
        pMat1 = GetMatrix();
    else
    {
        fVal1 = GetDouble();
        switch ( nCurFmtType )
        {
            case SvNumFormatType::CURRENCY :
                nFmtCurrencyType = nCurFmtType;
                nFmtCurrencyIndex = nCurFmtIndex;
            break;
            default: break;
        }
    }
    if (pMat1 && pMat2)
    {
        ScMatrixRef pResMat = lcl_MatrixCalculation<MatrixMul>( *pMat1, *pMat2, this);
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
            pMat->MulOp( fVal, *pResMat);
            PushMatrix(pResMat);
        }
        else
            PushIllegalArgument();
    }
    else
    {
        // Determine nFuncFmtType type before PushDouble().
        if ( nFmtCurrencyType == SvNumFormatType::CURRENCY )
        {
            nFuncFmtType = nFmtCurrencyType;
            nFuncFmtIndex = nFmtCurrencyIndex;
        }
        PushDouble(fVal1 * fVal2);
    }
}

void ScInterpreter::ScDiv()
{
    ScMatrixRef pMat1 = nullptr;
    ScMatrixRef pMat2 = nullptr;
    double fVal1 = 0.0, fVal2 = 0.0;
    SvNumFormatType nFmtCurrencyType = nCurFmtType;
    sal_uLong nFmtCurrencyIndex = nCurFmtIndex;
    SvNumFormatType nFmtCurrencyType2 = SvNumFormatType::UNDEFINED;
    if ( GetStackType() == svMatrix )
        pMat2 = GetMatrix();
    else
    {
        fVal2 = GetDouble();
        // do not take over currency, 123kg/456USD is not USD
        nFmtCurrencyType2 = nCurFmtType;
    }
    if ( GetStackType() == svMatrix )
        pMat1 = GetMatrix();
    else
    {
        fVal1 = GetDouble();
        switch ( nCurFmtType )
        {
            case SvNumFormatType::CURRENCY :
                nFmtCurrencyType = nCurFmtType;
                nFmtCurrencyIndex = nCurFmtIndex;
            break;
            default: break;
        }
    }
    if (pMat1 && pMat2)
    {
        ScMatrixRef pResMat = lcl_MatrixCalculation<MatrixDiv>( *pMat1, *pMat2, this);
        if (!pResMat)
            PushNoValue();
        else
            PushMatrix(pResMat);
    }
    else if (pMat1 || pMat2)
    {
        double fVal;
        bool bFlag;
        ScMatrixRef pMat = pMat1;
        if (!pMat)
        {
            fVal = fVal1;
            pMat = pMat2;
            bFlag = true;           // double - Matrix
        }
        else
        {
            fVal = fVal2;
            bFlag = false;          // Matrix - double
        }
        SCSIZE nC, nR;
        pMat->GetDimensions(nC, nR);
        ScMatrixRef pResMat = GetNewMat(nC, nR);
        if (pResMat)
        {
            pMat->DivOp( bFlag, fVal, *pResMat);
            PushMatrix(pResMat);
        }
        else
            PushIllegalArgument();
    }
    else
    {
        // Determine nFuncFmtType type before PushDouble().
        if (    nFmtCurrencyType  == SvNumFormatType::CURRENCY &&
                nFmtCurrencyType2 != SvNumFormatType::CURRENCY)
        {   // even USD/USD is not USD
            nFuncFmtType = nFmtCurrencyType;
            nFuncFmtIndex = nFmtCurrencyIndex;
        }
        PushDouble( div( fVal1, fVal2) );
    }
}

void ScInterpreter::ScPower()
{
    if ( MustHaveParamCount( GetByte(), 2 ) )
        ScPow();
}

void ScInterpreter::ScPow()
{
    ScMatrixRef pMat1 = nullptr;
    ScMatrixRef pMat2 = nullptr;
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
        ScMatrixRef pResMat = lcl_MatrixCalculation<MatrixPow>( *pMat1, *pMat2, this);
        if (!pResMat)
            PushNoValue();
        else
            PushMatrix(pResMat);
    }
    else if (pMat1 || pMat2)
    {
        double fVal;
        bool bFlag;
        ScMatrixRef pMat = pMat1;
        if (!pMat)
        {
            fVal = fVal1;
            pMat = pMat2;
            bFlag = true;           // double - Matrix
        }
        else
        {
            fVal = fVal2;
            bFlag = false;          // Matrix - double
        }
        SCSIZE nC, nR;
        pMat->GetDimensions(nC, nR);
        ScMatrixRef pResMat = GetNewMat(nC, nR);
        if (pResMat)
        {
            pMat->PowOp( bFlag, fVal, *pResMat);
            PushMatrix(pResMat);
        }
        else
            PushIllegalArgument();
    }
    else
    {
        PushDouble( sc::power( fVal1, fVal2));
    }
}

namespace {

class SumValues
{
    double mfSum;
    bool   mbError;
public:
    SumValues() : mfSum(0.0), mbError(false) {}

    void operator() (double f)
    {
        if (mbError)
            return;

        FormulaError nErr = GetDoubleErrorValue(f);
        if (nErr == FormulaError::NONE)
            mfSum += f;
        else if (nErr != FormulaError::ElementNaN)
        {
            // Propagate the first error encountered, ignore "this is not a
            // number" elements.
            mfSum = f;
            mbError = true;
        }
    }

    double getValue() const { return mfSum; }
};

}

void ScInterpreter::ScSumProduct()
{
    short nParamCount = GetByte();
    if ( !MustHaveParamCountMin( nParamCount, 1) )
        return;

    // XXX NOTE: Excel returns #VALUE! for reference list and 0 (why?) for
    // array of references. We calculate the proper individual arrays if sizes
    // match.

    size_t nInRefList = 0;
    ScMatrixRef pMatLast;
    ScMatrixRef pMat;

    pMatLast = GetMatrix( --nParamCount, nInRefList);
    if (!pMatLast)
    {
        PushIllegalParameter();
        return;
    }

    SCSIZE nC, nCLast, nR, nRLast;
    pMatLast->GetDimensions(nCLast, nRLast);
    std::vector<double> aResArray;
    pMatLast->GetDoubleArray(aResArray);

    while (nParamCount--)
    {
        pMat = GetMatrix( nParamCount, nInRefList);
        if (!pMat)
        {
            PushIllegalParameter();
            return;
        }
        pMat->GetDimensions(nC, nR);
        if (nC != nCLast || nR != nRLast)
        {
            PushNoValue();
            return;
        }

        pMat->MergeDoubleArrayMultiply(aResArray);
    }

    double fSum = std::for_each(aResArray.begin(), aResArray.end(), SumValues()).getValue();
    PushDouble(fSum);
}

void ScInterpreter::ScSumX2MY2()
{
    CalculateSumX2MY2SumX2DY2(false);
}
void ScInterpreter::CalculateSumX2MY2SumX2DY2(bool _bSumX2DY2)
{
    if ( !MustHaveParamCount( GetByte(), 2 ) )
        return;

    ScMatrixRef pMat1 = nullptr;
    ScMatrixRef pMat2 = nullptr;
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
            if (!pMat1->IsStringOrEmpty(i,j) && !pMat2->IsStringOrEmpty(i,j))
            {
                fVal = pMat1->GetDouble(i,j);
                fSum += fVal * fVal;
                fVal = pMat2->GetDouble(i,j);
                if ( _bSumX2DY2 )
                    fSum += fVal * fVal;
                else
                    fSum -= fVal * fVal;
            }
    PushDouble(fSum);
}

void ScInterpreter::ScSumX2DY2()
{
    CalculateSumX2MY2SumX2DY2(true);
}

void ScInterpreter::ScSumXMY2()
{
    if ( !MustHaveParamCount( GetByte(), 2 ) )
        return;

    ScMatrixRef pMat2 = GetMatrix();
    ScMatrixRef pMat1 = GetMatrix();
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
    } // if (nC1 != nC2 || nR1 != nR2)
    ScMatrixRef pResMat = lcl_MatrixCalculation<MatrixSub>( *pMat1, *pMat2, this);
    if (!pResMat)
    {
        PushNoValue();
    }
    else
    {
        ScMatrix::IterateResult aRes = pResMat->SumSquare(false);
        double fSum = aRes.mfFirst + aRes.mfRest;
        PushDouble(fSum);
    }
}

void ScInterpreter::ScFrequency()
{
    if ( !MustHaveParamCount( GetByte(), 2 ) )
        return;

    vector<double>  aBinArray;
    vector<long>    aBinIndexOrder;

    GetSortArray( 1, aBinArray, &aBinIndexOrder, false, false );
    SCSIZE nBinSize = aBinArray.size();
    if (nGlobalError != FormulaError::NONE)
    {
        PushNoValue();
        return;
    }

    vector<double>  aDataArray;
    GetSortArray( 1, aDataArray, nullptr, false, false );
    SCSIZE nDataSize = aDataArray.size();

    if (aDataArray.empty() || nGlobalError != FormulaError::NONE)
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

namespace {

// Helper methods for LINEST/LOGEST and TREND/GROWTH
// All matrices must already exist and have the needed size, no control tests
// done. Those methods, which names start with lcl_T, are adapted to case 3,
// where Y (=observed values) is given as row.
// Remember, ScMatrix matrices are zero based, index access (column,row).

// <A;B> over all elements; uses the matrices as vectors of length M
double lcl_GetSumProduct(const ScMatrixRef& pMatA, const ScMatrixRef& pMatB, SCSIZE nM)
{
    double fSum = 0.0;
    for (SCSIZE i=0; i<nM; i++)
        fSum += pMatA->GetDouble(i) * pMatB->GetDouble(i);
    return fSum;
}

// Special version for use within QR decomposition.
// Euclidean norm of column index C starting in row index R;
// matrix A has count N rows.
double lcl_GetColumnEuclideanNorm(const ScMatrixRef& pMatA, SCSIZE nC, SCSIZE nR, SCSIZE nN)
{
    double fNorm = 0.0;
    for (SCSIZE row=nR; row<nN; row++)
        fNorm  += (pMatA->GetDouble(nC,row)) * (pMatA->GetDouble(nC,row));
    return sqrt(fNorm);
}

// Euclidean norm of row index R starting in column index C;
// matrix A has count N columns.
double lcl_TGetColumnEuclideanNorm(const ScMatrixRef& pMatA, SCSIZE nR, SCSIZE nC, SCSIZE nN)
{
    double fNorm = 0.0;
    for (SCSIZE col=nC; col<nN; col++)
        fNorm  += (pMatA->GetDouble(col,nR)) * (pMatA->GetDouble(col,nR));
    return sqrt(fNorm);
}

// Special version for use within QR decomposition.
// Maximum norm of column index C starting in row index R;
// matrix A has count N rows.
double lcl_GetColumnMaximumNorm(const ScMatrixRef& pMatA, SCSIZE nC, SCSIZE nR, SCSIZE nN)
{
    double fNorm = 0.0;
    for (SCSIZE row=nR; row<nN; row++)
    {
        double fVal = fabs(pMatA->GetDouble(nC,row));
        if (fNorm < fVal)
            fNorm = fVal;
    }
    return fNorm;
}

// Maximum norm of row index R starting in col index C;
// matrix A has count N columns.
double lcl_TGetColumnMaximumNorm(const ScMatrixRef& pMatA, SCSIZE nR, SCSIZE nC, SCSIZE nN)
{
    double fNorm = 0.0;
    for (SCSIZE col=nC; col<nN; col++)
    {
        double fVal = fabs(pMatA->GetDouble(col,nR));
        if (fNorm < fVal)
            fNorm = fVal;
    }
    return fNorm;
}

// Special version for use within QR decomposition.
// <A(Ca);B(Cb)> starting in row index R;
// Ca and Cb are indices of columns, matrices A and B have count N rows.
double lcl_GetColumnSumProduct(const ScMatrixRef& pMatA, SCSIZE nCa,
                               const ScMatrixRef& pMatB, SCSIZE nCb, SCSIZE nR, SCSIZE nN)
{
    double fResult = 0.0;
    for (SCSIZE row=nR; row<nN; row++)
        fResult += pMatA->GetDouble(nCa,row) * pMatB->GetDouble(nCb,row);
    return fResult;
}

// <A(Ra);B(Rb)> starting in column index C;
// Ra and Rb are indices of rows, matrices A and B have count N columns.
double lcl_TGetColumnSumProduct(const ScMatrixRef& pMatA, SCSIZE nRa,
                                const ScMatrixRef& pMatB, SCSIZE nRb, SCSIZE nC, SCSIZE nN)
{
    double fResult = 0.0;
    for (SCSIZE col=nC; col<nN; col++)
        fResult += pMatA->GetDouble(col,nRa) * pMatB->GetDouble(col,nRb);
    return fResult;
}

// no mathematical signum, but used to switch between adding and subtracting
double lcl_GetSign(double fValue)
{
    return (fValue >= 0.0 ? 1.0 : -1.0 );
}

/* Calculates a QR decomposition with Householder reflection.
 * For each NxK matrix A exists a decomposition A=Q*R with an orthogonal
 * NxN matrix Q and a NxK matrix R.
 * Q=H1*H2*...*Hk with Householder matrices H. Such a householder matrix can
 * be build from a vector u by H=I-(2/u'u)*(u u'). This vectors u are returned
 * in the columns of matrix A, overwriting the old content.
 * The matrix R has a quadric upper part KxK with values in the upper right
 * triangle and zeros in all other elements. Here the diagonal elements of R
 * are stored in the vector R and the other upper right elements in the upper
 * right of the matrix A.
 * The function returns false, if calculation breaks. But because of round-off
 * errors singularity is often not detected.
 */
bool lcl_CalculateQRdecomposition(const ScMatrixRef& pMatA,
                                  ::std::vector< double>& pVecR, SCSIZE nK, SCSIZE nN)
{
    // ScMatrix matrices are zero based, index access (column,row)
    for (SCSIZE col = 0; col <nK; col++)
    {
        // calculate vector u of the householder transformation
        const double fScale = lcl_GetColumnMaximumNorm(pMatA, col, col, nN);
        if (fScale == 0.0)
        {
            // A is singular
            return false;
        }
        for (SCSIZE row = col; row <nN; row++)
            pMatA->PutDouble( pMatA->GetDouble(col,row)/fScale, col, row);

        const double fEuclid = lcl_GetColumnEuclideanNorm(pMatA, col, col, nN);
        const double fFactor = 1.0/fEuclid/(fEuclid + fabs(pMatA->GetDouble(col,col)));
        const double fSignum = lcl_GetSign(pMatA->GetDouble(col,col));
        pMatA->PutDouble( pMatA->GetDouble(col,col) + fSignum*fEuclid, col,col);
        pVecR[col] = -fSignum * fScale * fEuclid;

        // apply Householder transformation to A
        for (SCSIZE c=col+1; c<nK; c++)
        {
            const double fSum =lcl_GetColumnSumProduct(pMatA, col, pMatA, c, col, nN);
            for (SCSIZE row = col; row <nN; row++)
                pMatA->PutDouble( pMatA->GetDouble(c,row) - fSum * fFactor * pMatA->GetDouble(col,row), c, row);
        }
    }
    return true;
}

// same with transposed matrix A, N is count of columns, K count of rows
bool lcl_TCalculateQRdecomposition(const ScMatrixRef& pMatA,
                                   ::std::vector< double>& pVecR, SCSIZE nK, SCSIZE nN)
{
    double fSum ;
    // ScMatrix matrices are zero based, index access (column,row)
    for (SCSIZE row = 0; row <nK; row++)
    {
        // calculate vector u of the householder transformation
        const double fScale = lcl_TGetColumnMaximumNorm(pMatA, row, row, nN);
        if (fScale == 0.0)
        {
            // A is singular
            return false;
        }
        for (SCSIZE col = row; col <nN; col++)
            pMatA->PutDouble( pMatA->GetDouble(col,row)/fScale, col, row);

        const double fEuclid = lcl_TGetColumnEuclideanNorm(pMatA, row, row, nN);
        const double fFactor = 1.0/fEuclid/(fEuclid + fabs(pMatA->GetDouble(row,row)));
        const double fSignum = lcl_GetSign(pMatA->GetDouble(row,row));
        pMatA->PutDouble( pMatA->GetDouble(row,row) + fSignum*fEuclid, row,row);
        pVecR[row] = -fSignum * fScale * fEuclid;

        // apply Householder transformation to A
        for (SCSIZE r=row+1; r<nK; r++)
        {
            fSum =lcl_TGetColumnSumProduct(pMatA, row, pMatA, r, row, nN);
            for (SCSIZE col = row; col <nN; col++)
                pMatA->PutDouble(
                    pMatA->GetDouble(col,r) - fSum * fFactor * pMatA->GetDouble(col,row), col, r);
        }
    }
    return true;
}

/* Applies a Householder transformation to a column vector Y with is given as
 * Nx1 Matrix. The vector u, from which the Householder transformation is built,
 * is the column part in matrix A, with column index C, starting with row
 * index C. A is the result of the QR decomposition as obtained from
 * lcl_CalculateQRdecomposition.
 */
void lcl_ApplyHouseholderTransformation(const ScMatrixRef& pMatA, SCSIZE nC,
                                        const ScMatrixRef& pMatY, SCSIZE nN)
{
    // ScMatrix matrices are zero based, index access (column,row)
    double fDenominator = lcl_GetColumnSumProduct(pMatA, nC, pMatA, nC, nC, nN);
    double fNumerator = lcl_GetColumnSumProduct(pMatA, nC, pMatY, 0, nC, nN);
    double fFactor = 2.0 * (fNumerator/fDenominator);
    for (SCSIZE row = nC; row < nN; row++)
        pMatY->PutDouble(
            pMatY->GetDouble(row) - fFactor * pMatA->GetDouble(nC,row), row);
}

// Same with transposed matrices A and Y.
void lcl_TApplyHouseholderTransformation(const ScMatrixRef& pMatA, SCSIZE nR,
                                          const ScMatrixRef& pMatY, SCSIZE nN)
{
    // ScMatrix matrices are zero based, index access (column,row)
    double fDenominator = lcl_TGetColumnSumProduct(pMatA, nR, pMatA, nR, nR, nN);
    double fNumerator = lcl_TGetColumnSumProduct(pMatA, nR, pMatY, 0, nR, nN);
    double fFactor = 2.0 * (fNumerator/fDenominator);
    for (SCSIZE col = nR; col < nN; col++)
        pMatY->PutDouble(
          pMatY->GetDouble(col) - fFactor * pMatA->GetDouble(col,nR), col);
}

/* Solve for X in R*X=S using back substitution. The solution X overwrites S.
 * Uses R from the result of the QR decomposition of a NxK matrix A.
 * S is a column vector given as matrix, with at least elements on index
 * 0 to K-1; elements on index>=K are ignored. Vector R must not have zero
 * elements, no check is done.
 */
void lcl_SolveWithUpperRightTriangle(const ScMatrixRef& pMatA,
                        ::std::vector< double>& pVecR, const ScMatrixRef& pMatS,
                        SCSIZE nK, bool bIsTransposed)
{
    // ScMatrix matrices are zero based, index access (column,row)
    SCSIZE row;
    // SCSIZE is never negative, therefore test with rowp1=row+1
    for (SCSIZE rowp1 = nK; rowp1>0; rowp1--)
    {
        row = rowp1-1;
        double fSum = pMatS->GetDouble(row);
        for (SCSIZE col = rowp1; col<nK ; col++)
            if (bIsTransposed)
                fSum -= pMatA->GetDouble(row,col) * pMatS->GetDouble(col);
            else
                fSum -= pMatA->GetDouble(col,row) * pMatS->GetDouble(col);
        pMatS->PutDouble( fSum / pVecR[row] , row);
    }
}

/* Solve for X in R' * X= T using forward substitution. The solution X
 * overwrites T. Uses R from the result of the QR decomposition of a NxK
 * matrix A. T is a column vectors given as matrix, with at least elements on
 * index 0 to K-1; elements on index>=K are ignored. Vector R must not have
 * zero elements, no check is done.
 */
void lcl_SolveWithLowerLeftTriangle(const ScMatrixRef& pMatA,
                                    ::std::vector< double>& pVecR, const ScMatrixRef& pMatT,
                                    SCSIZE nK, bool bIsTransposed)
{
    // ScMatrix matrices are zero based, index access (column,row)
    for (SCSIZE row = 0; row < nK; row++)
    {
        double fSum = pMatT -> GetDouble(row);
        for (SCSIZE col=0; col < row; col++)
        {
            if (bIsTransposed)
                fSum -= pMatA->GetDouble(col,row) * pMatT->GetDouble(col);
            else
                fSum -= pMatA->GetDouble(row,col) * pMatT->GetDouble(col);
        }
        pMatT->PutDouble( fSum / pVecR[row] , row);
    }
}

/* Calculates Z = R * B
 * R is given in matrix A and vector VecR as obtained from the QR
 * decomposition in lcl_CalculateQRdecomposition. B and Z are column vectors
 * given as matrix with at least index 0 to K-1; elements on index>=K are
 * not used.
 */
void lcl_ApplyUpperRightTriangle(const ScMatrixRef& pMatA,
                                 ::std::vector< double>& pVecR, const ScMatrixRef& pMatB,
                                 const ScMatrixRef& pMatZ, SCSIZE nK, bool bIsTransposed)
{
    // ScMatrix matrices are zero based, index access (column,row)
    for (SCSIZE row = 0; row < nK; row++)
    {
        double fSum = pVecR[row] * pMatB->GetDouble(row);
        for (SCSIZE col = row+1; col < nK; col++)
            if (bIsTransposed)
                fSum += pMatA->GetDouble(row,col) * pMatB->GetDouble(col);
            else
                fSum += pMatA->GetDouble(col,row) * pMatB->GetDouble(col);
        pMatZ->PutDouble( fSum, row);
    }
}

double lcl_GetMeanOverAll(const ScMatrixRef& pMat, SCSIZE nN)
{
    double fSum = 0.0;
    for (SCSIZE i=0 ; i<nN; i++)
        fSum += pMat->GetDouble(i);
    return fSum/static_cast<double>(nN);
}

// Calculates means of the columns of matrix X. X is a RxC matrix;
// ResMat is a 1xC matrix (=row).
void lcl_CalculateColumnMeans(const ScMatrixRef& pX, const ScMatrixRef& pResMat,
                              SCSIZE nC, SCSIZE nR)
{
    for (SCSIZE i=0; i < nC; i++)
    {
        double fSum =0.0;
        for (SCSIZE k=0; k < nR; k++)
            fSum += pX->GetDouble(i,k);   // GetDouble(Column,Row)
        pResMat ->PutDouble( fSum/static_cast<double>(nR),i);
    }
}

// Calculates means of the rows of matrix X. X is a RxC matrix;
// ResMat is a Rx1 matrix (=column).
void lcl_CalculateRowMeans(const ScMatrixRef& pX, const ScMatrixRef& pResMat,
                           SCSIZE nC, SCSIZE nR)
{
    for (SCSIZE k=0; k < nR; k++)
    {
        double fSum = 0.0;
        for (SCSIZE i=0; i < nC; i++)
            fSum += pX->GetDouble(i,k);   // GetDouble(Column,Row)
        pResMat ->PutDouble( fSum/static_cast<double>(nC),k);
    }
}

void lcl_CalculateColumnsDelta(const ScMatrixRef& pMat, const ScMatrixRef& pColumnMeans,
                               SCSIZE nC, SCSIZE nR)
{
    for (SCSIZE i = 0; i < nC; i++)
        for (SCSIZE k = 0; k < nR; k++)
            pMat->PutDouble( ::rtl::math::approxSub
                             (pMat->GetDouble(i,k) , pColumnMeans->GetDouble(i) ) , i, k);
}

void lcl_CalculateRowsDelta(const ScMatrixRef& pMat, const ScMatrixRef& pRowMeans,
                            SCSIZE nC, SCSIZE nR)
{
    for (SCSIZE k = 0; k < nR; k++)
        for (SCSIZE i = 0; i < nC; i++)
            pMat->PutDouble( ::rtl::math::approxSub
                             ( pMat->GetDouble(i,k) , pRowMeans->GetDouble(k) ) , i, k);
}

// Case1 = simple regression
// MatX = X - MeanX, MatY = Y - MeanY, y - haty = (y - MeanY) - (haty - MeanY)
// = (y-MeanY)-((slope*x+a)-(slope*MeanX+a)) = (y-MeanY)-slope*(x-MeanX)
double lcl_GetSSresid(const ScMatrixRef& pMatX, const ScMatrixRef& pMatY, double fSlope,
                      SCSIZE nN)
{
    double fSum = 0.0;
    for (SCSIZE i=0; i<nN; i++)
    {
        const double fTemp = pMatY->GetDouble(i) - fSlope * pMatX->GetDouble(i);
        fSum += fTemp * fTemp;
    }
    return fSum;
}

}

// Fill default values in matrix X, transform Y to log(Y) in case LOGEST|GROWTH,
// determine sizes of matrices X and Y, determine kind of regression, clone
// Y in case LOGEST|GROWTH, if constant.
bool ScInterpreter::CheckMatrix(bool _bLOG, sal_uInt8& nCase, SCSIZE& nCX,
                        SCSIZE& nCY, SCSIZE& nRX, SCSIZE& nRY, SCSIZE& M,
                        SCSIZE& N, ScMatrixRef& pMatX, ScMatrixRef& pMatY)
{

    nCX = 0;
    nCY = 0;
    nRX = 0;
    nRY = 0;
    M = 0;
    N = 0;
    pMatY->GetDimensions(nCY, nRY);
    const SCSIZE nCountY = nCY * nRY;
    for ( SCSIZE i = 0; i < nCountY; i++ )
    {
        if (!pMatY->IsValue(i))
        {
            PushIllegalArgument();
            return false;
        }
    }

    if ( _bLOG )
    {
        ScMatrixRef pNewY = pMatY->CloneIfConst();
        for (SCSIZE nElem = 0; nElem < nCountY; nElem++)
        {
            const double fVal = pNewY->GetDouble(nElem);
            if (fVal <= 0.0)
            {
                PushIllegalArgument();
                return false;
            }
            else
                pNewY->PutDouble(log(fVal), nElem);
        }
        pMatY = pNewY;
    }

    if (pMatX)
    {
        pMatX->GetDimensions(nCX, nRX);
        const SCSIZE nCountX = nCX * nRX;
        for ( SCSIZE i = 0; i < nCountX; i++ )
            if (!pMatX->IsValue(i))
            {
                PushIllegalArgument();
                return false;
            }
        if (nCX == nCY && nRX == nRY)
        {
            nCase = 1;                  // simple regression
            M = 1;
            N = nCountY;
        }
        else if (nCY != 1 && nRY != 1)
        {
            PushIllegalArgument();
            return false;
        }
        else if (nCY == 1)
        {
            if (nRX != nRY)
            {
                PushIllegalArgument();
                return false;
            }
            else
            {
                nCase = 2;              // Y is column
                N = nRY;
                M = nCX;
            }
        }
        else if (nCX != nCY)
        {
            PushIllegalArgument();
            return false;
        }
        else
        {
            nCase = 3;                  // Y is row
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
            return false;
        }
        for ( SCSIZE i = 1; i <= nCountY; i++ )
            pMatX->PutDouble(static_cast<double>(i), i-1);
        nCase = 1;
        N = nCountY;
        M = 1;
    }
    return true;
}

// LINEST
void ScInterpreter::ScLinest()
{
    CalculateRGPRKP(false);
}

// LOGEST
void ScInterpreter::ScLogest()
{
    CalculateRGPRKP(true);
}

void ScInterpreter::CalculateRGPRKP(bool _bRKP)
{
    sal_uInt8 nParamCount = GetByte();
    if (!MustHaveParamCount( nParamCount, 1, 4 ))
        return;
    bool bConstant, bStats;

    // optional forth parameter
    if (nParamCount == 4)
        bStats = GetBool();
    else
        bStats = false;

    // The third parameter may not be missing in ODF, if the forth parameter
    // is present. But Excel allows it with default true, we too.
    if (nParamCount >= 3)
    {
        if (IsMissing())
        {
            Pop();
            bConstant = true;
//            PushIllegalParameter(); if ODF behavior is desired
//            return;
        }
        else
            bConstant = GetBool();
    }
    else
        bConstant = true;

    ScMatrixRef pMatX;
    if (nParamCount >= 2)
    {
        if (IsMissing())
        { //In ODF1.2 empty second parameter (which is two ;; ) is allowed
            Pop();
            pMatX = nullptr;
        }
        else
        {
            pMatX = GetMatrix();
        }
    }
    else
        pMatX = nullptr;

    ScMatrixRef pMatY;
    pMatY = GetMatrix();
    if (!pMatY)
    {
        PushIllegalParameter();
        return;
    }

    // 1 = simple; 2 = multiple with Y as column; 3 = multiple with Y as row
    sal_uInt8 nCase;

    SCSIZE nCX, nCY; // number of columns
    SCSIZE nRX, nRY;    //number of rows
    SCSIZE K = 0, N = 0; // K=number of variables X, N=number of data samples
    if (!CheckMatrix(_bRKP,nCase,nCX,nCY,nRX,nRY,K,N,pMatX,pMatY))
    {
        PushIllegalParameter();
        return;
    }

    // Enough data samples?
    if ((bConstant && (N<K+1)) || (!bConstant && (N<K)) || (N<1) || (K<1))
    {
        PushIllegalParameter();
        return;
    }

    ScMatrixRef pResMat;
    if (bStats)
        pResMat = GetNewMat(K+1,5);
    else
        pResMat = GetNewMat(K+1,1);
    if (!pResMat)
    {
        PushError(FormulaError::CodeOverflow);
        return;
    }
    // Fill unused cells in pResMat; order (column,row)
    if (bStats)
    {
        for (SCSIZE i=2; i<K+1; i++)
        {
            pResMat->PutError( FormulaError::NotAvailable, i, 2);
            pResMat->PutError( FormulaError::NotAvailable, i, 3);
            pResMat->PutError( FormulaError::NotAvailable, i, 4);
        }
    }

    // Uses sum(x-MeanX)^2 and not [sum x^2]-N * MeanX^2 in case bConstant.
    // Clone constant matrices, so that Mat = Mat - Mean is possible.
    double fMeanY = 0.0;
    if (bConstant)
    {
        ScMatrixRef pNewX = pMatX->CloneIfConst();
        ScMatrixRef pNewY = pMatY->CloneIfConst();
        if (!pNewX || !pNewY)
        {
            PushError(FormulaError::CodeOverflow);
            return;
        }
        pMatX = pNewX;
        pMatY = pNewY;
        // DeltaY is possible here; DeltaX depends on nCase, so later
        fMeanY = lcl_GetMeanOverAll(pMatY, N);
        for (SCSIZE i=0; i<N; i++)
        {
            pMatY->PutDouble( ::rtl::math::approxSub(pMatY->GetDouble(i),fMeanY), i );
        }
    }

    if (nCase==1)
    {
        // calculate simple regression
        double fMeanX = 0.0;
        if (bConstant)
        {   // Mat = Mat - Mean
            fMeanX = lcl_GetMeanOverAll(pMatX, N);
            for (SCSIZE i=0; i<N; i++)
            {
                pMatX->PutDouble( ::rtl::math::approxSub(pMatX->GetDouble(i),fMeanX), i );
            }
        }
        double fSumXY = lcl_GetSumProduct(pMatX,pMatY,N);
        double fSumX2 = lcl_GetSumProduct(pMatX,pMatX,N);
        if (fSumX2==0.0)
        {
            PushNoValue(); // all x-values are identical
            return;
        }
        double fSlope = fSumXY / fSumX2;
        double fIntercept = 0.0;
        if (bConstant)
            fIntercept = fMeanY - fSlope * fMeanX;
        pResMat->PutDouble(_bRKP ? exp(fIntercept) : fIntercept, 1, 0); //order (column,row)
        pResMat->PutDouble(_bRKP ? exp(fSlope) : fSlope, 0, 0);

        if (bStats)
        {
            double fSSreg = fSlope * fSlope * fSumX2;
            pResMat->PutDouble(fSSreg, 0, 4);

            double fDegreesFreedom =static_cast<double>( bConstant ? N-2 : N-1 );
            pResMat->PutDouble(fDegreesFreedom, 1, 3);

            double fSSresid = lcl_GetSSresid(pMatX,pMatY,fSlope,N);
            pResMat->PutDouble(fSSresid, 1, 4);

            if (fDegreesFreedom == 0.0 || fSSresid == 0.0 || fSSreg == 0.0)
            {   // exact fit; test SSreg too, because SSresid might be
                // unequal zero due to round of errors
                pResMat->PutDouble(0.0, 1, 4); // SSresid
                pResMat->PutError( FormulaError::NotAvailable, 0, 3); // F
                pResMat->PutDouble(0.0, 1, 2); // RMSE
                pResMat->PutDouble(0.0, 0, 1); // SigmaSlope
                if (bConstant)
                    pResMat->PutDouble(0.0, 1, 1); //SigmaIntercept
                else
                    pResMat->PutError( FormulaError::NotAvailable, 1, 1);
                pResMat->PutDouble(1.0, 0, 2); // R^2
            }
            else
            {
                double fFstatistic = (fSSreg / static_cast<double>(K))
                                     / (fSSresid / fDegreesFreedom);
                pResMat->PutDouble(fFstatistic, 0, 3);

                // standard error of estimate
                double fRMSE = sqrt(fSSresid / fDegreesFreedom);
                pResMat->PutDouble(fRMSE, 1, 2);

                double fSigmaSlope = fRMSE / sqrt(fSumX2);
                pResMat->PutDouble(fSigmaSlope, 0, 1);

                if (bConstant)
                {
                    double fSigmaIntercept = fRMSE
                                             * sqrt(fMeanX*fMeanX/fSumX2 + 1.0/static_cast<double>(N));
                    pResMat->PutDouble(fSigmaIntercept, 1, 1);
                }
                else
                {
                    pResMat->PutError( FormulaError::NotAvailable, 1, 1);
                }

                double fR2 = fSSreg / (fSSreg + fSSresid);
                pResMat->PutDouble(fR2, 0, 2);
            }
        }
        PushMatrix(pResMat);
    }
    else // calculate multiple regression;
    {
        // Uses a QR decomposition X = QR. The solution B = (X'X)^(-1) * X' * Y
        // becomes B = R^(-1) * Q' * Y
        if (nCase ==2) // Y is column
        {
            ::std::vector< double> aVecR(N); // for QR decomposition
            // Enough memory for needed matrices?
            ScMatrixRef pMeans = GetNewMat(K, 1); // mean of each column
            ScMatrixRef pMatZ; // for Q' * Y , inter alia
            if (bStats)
                pMatZ = pMatY->Clone(); // Y is used in statistic, keep it
            else
                pMatZ = pMatY; // Y can be overwritten
            ScMatrixRef pSlopes = GetNewMat(1,K); // from b1 to bK
            if (!pMeans || !pMatZ || !pSlopes)
            {
                PushError(FormulaError::CodeOverflow);
                return;
            }
            if (bConstant)
            {
                lcl_CalculateColumnMeans(pMatX, pMeans, K, N);
                lcl_CalculateColumnsDelta(pMatX, pMeans, K, N);
            }
            if (!lcl_CalculateQRdecomposition(pMatX, aVecR, K, N))
            {
                PushNoValue();
                return;
            }
            // Later on we will divide by elements of aVecR, so make sure
            // that they aren't zero.
            bool bIsSingular=false;
            for (SCSIZE row=0; row < K && !bIsSingular; row++)
                bIsSingular = aVecR[row] == 0.0;
            if (bIsSingular)
            {
                PushNoValue();
                return;
            }
            // Z = Q' Y;
            for (SCSIZE col = 0; col < K; col++)
            {
                lcl_ApplyHouseholderTransformation(pMatX, col, pMatZ, N);
            }
            // B = R^(-1) * Q' * Y <=> B = R^(-1) * Z <=> R * B = Z
            // result Z should have zeros for index>=K; if not, ignore values
            for (SCSIZE col = 0; col < K ; col++)
            {
                pSlopes->PutDouble( pMatZ->GetDouble(col), col);
            }
            lcl_SolveWithUpperRightTriangle(pMatX, aVecR, pSlopes, K, false);
            double fIntercept = 0.0;
            if (bConstant)
                fIntercept = fMeanY - lcl_GetSumProduct(pMeans,pSlopes,K);
            // Fill first line in result matrix
            pResMat->PutDouble(_bRKP ? exp(fIntercept) : fIntercept, K, 0 );
            for (SCSIZE i = 0; i < K; i++)
                pResMat->PutDouble(_bRKP ? exp(pSlopes->GetDouble(i))
                                   : pSlopes->GetDouble(i) , K-1-i, 0);

            if (bStats)
            {
                double fSSreg = 0.0;
                double fSSresid = 0.0;
                // re-use memory of Z;
                pMatZ->FillDouble(0.0, 0, 0, 0, N-1);
                // Z = R * Slopes
                lcl_ApplyUpperRightTriangle(pMatX, aVecR, pSlopes, pMatZ, K, false);
                // Z = Q * Z, that is Q * R * Slopes = X * Slopes
                for (SCSIZE colp1 = K; colp1 > 0; colp1--)
                {
                    lcl_ApplyHouseholderTransformation(pMatX, colp1-1, pMatZ,N);
                }
                fSSreg =lcl_GetSumProduct(pMatZ, pMatZ, N);
                // re-use Y for residuals, Y = Y-Z
                for (SCSIZE row = 0; row < N; row++)
                    pMatY->PutDouble(pMatY->GetDouble(row) - pMatZ->GetDouble(row), row);
                fSSresid = lcl_GetSumProduct(pMatY, pMatY, N);
                pResMat->PutDouble(fSSreg, 0, 4);
                pResMat->PutDouble(fSSresid, 1, 4);

                double fDegreesFreedom =static_cast<double>( bConstant ? N-K-1 : N-K );
                pResMat->PutDouble(fDegreesFreedom, 1, 3);

                if (fDegreesFreedom == 0.0 || fSSresid == 0.0 || fSSreg == 0.0)
                {   // exact fit; incl. observed values Y are identical
                    pResMat->PutDouble(0.0, 1, 4); // SSresid
                    // F = (SSreg/K) / (SSresid/df) = #DIV/0!
                    pResMat->PutError( FormulaError::NotAvailable, 0, 3); // F
                    // RMSE = sqrt(SSresid / df) = sqrt(0 / df) = 0
                    pResMat->PutDouble(0.0, 1, 2); // RMSE
                    // SigmaSlope[i] = RMSE * sqrt(matrix[i,i]) = 0 * sqrt(...) = 0
                    for (SCSIZE i=0; i<K; i++)
                        pResMat->PutDouble(0.0, K-1-i, 1);

                    // SigmaIntercept = RMSE * sqrt(...) = 0
                    if (bConstant)
                        pResMat->PutDouble(0.0, K, 1); //SigmaIntercept
                    else
                        pResMat->PutError( FormulaError::NotAvailable, K, 1);

                    //  R^2 = SSreg / (SSreg + SSresid) = 1.0
                    pResMat->PutDouble(1.0, 0, 2); // R^2
                }
                else
                {
                    double fFstatistic = (fSSreg / static_cast<double>(K))
                                         / (fSSresid / fDegreesFreedom);
                    pResMat->PutDouble(fFstatistic, 0, 3);

                    // standard error of estimate = root mean SSE
                    double fRMSE = sqrt(fSSresid / fDegreesFreedom);
                    pResMat->PutDouble(fRMSE, 1, 2);

                    // standard error of slopes
                    // = RMSE * sqrt(diagonal element of (R' R)^(-1) )
                    // standard error of intercept
                    // = RMSE * sqrt( Xmean * (R' R)^(-1) * Xmean' + 1/N)
                    // (R' R)^(-1) = R^(-1) * (R')^(-1). Do not calculate it as
                    // a whole matrix, but iterate over unit vectors.
                    double fSigmaIntercept = 0.0;
                    double fPart; // for Xmean * single column of (R' R)^(-1)
                    for (SCSIZE col = 0; col < K; col++)
                    {
                        //re-use memory of MatZ
                        pMatZ->FillDouble(0.0,0,0,0,K-1); // Z = unit vector e
                        pMatZ->PutDouble(1.0, col);
                        //Solve R' * Z = e
                        lcl_SolveWithLowerLeftTriangle(pMatX, aVecR, pMatZ, K, false);
                        // Solve R * Znew = Zold
                        lcl_SolveWithUpperRightTriangle(pMatX, aVecR, pMatZ, K, false);
                        // now Z is column col in (R' R)^(-1)
                        double fSigmaSlope = fRMSE * sqrt(pMatZ->GetDouble(col));
                        pResMat->PutDouble(fSigmaSlope, K-1-col, 1);
                        // (R' R) ^(-1) is symmetric
                        if (bConstant)
                        {
                            fPart = lcl_GetSumProduct(pMeans, pMatZ, K);
                            fSigmaIntercept += fPart * pMeans->GetDouble(col);
                        }
                    }
                    if (bConstant)
                    {
                        fSigmaIntercept = fRMSE
                                          * sqrt(fSigmaIntercept + 1.0 / static_cast<double>(N));
                        pResMat->PutDouble(fSigmaIntercept, K, 1);
                    }
                    else
                    {
                        pResMat->PutError( FormulaError::NotAvailable, K, 1);
                    }

                    double fR2 = fSSreg / (fSSreg + fSSresid);
                    pResMat->PutDouble(fR2, 0, 2);
                }
            }
            PushMatrix(pResMat);
        }
        else  // nCase == 3, Y is row, all matrices are transposed
        {
            ::std::vector< double> aVecR(N); // for QR decomposition
            // Enough memory for needed matrices?
            ScMatrixRef pMeans = GetNewMat(1, K); // mean of each row
            ScMatrixRef pMatZ; // for Q' * Y , inter alia
            if (bStats)
                pMatZ = pMatY->Clone(); // Y is used in statistic, keep it
            else
                pMatZ = pMatY; // Y can be overwritten
            ScMatrixRef pSlopes = GetNewMat(K,1); // from b1 to bK
            if (!pMeans || !pMatZ || !pSlopes)
            {
                PushError(FormulaError::CodeOverflow);
                return;
            }
            if (bConstant)
            {
                lcl_CalculateRowMeans(pMatX, pMeans, N, K);
                lcl_CalculateRowsDelta(pMatX, pMeans, N, K);
            }

            if (!lcl_TCalculateQRdecomposition(pMatX, aVecR, K, N))
            {
                PushNoValue();
                return;
            }

            // Later on we will divide by elements of aVecR, so make sure
            // that they aren't zero.
            bool bIsSingular=false;
            for (SCSIZE row=0; row < K && !bIsSingular; row++)
                bIsSingular = aVecR[row] == 0.0;
            if (bIsSingular)
            {
                PushNoValue();
                return;
            }
            // Z = Q' Y
            for (SCSIZE row = 0; row < K; row++)
            {
                lcl_TApplyHouseholderTransformation(pMatX, row, pMatZ, N);
            }
            // B = R^(-1) * Q' * Y <=> B = R^(-1) * Z <=> R * B = Z
            // result Z should have zeros for index>=K; if not, ignore values
            for (SCSIZE col = 0; col < K ; col++)
            {
                pSlopes->PutDouble( pMatZ->GetDouble(col), col);
            }
            lcl_SolveWithUpperRightTriangle(pMatX, aVecR, pSlopes, K, true);
            double fIntercept = 0.0;
            if (bConstant)
                fIntercept = fMeanY - lcl_GetSumProduct(pMeans,pSlopes,K);
            // Fill first line in result matrix
            pResMat->PutDouble(_bRKP ? exp(fIntercept) : fIntercept, K, 0 );
            for (SCSIZE i = 0; i < K; i++)
                pResMat->PutDouble(_bRKP ? exp(pSlopes->GetDouble(i))
                                   : pSlopes->GetDouble(i) , K-1-i, 0);

            if (bStats)
            {
                double fSSreg = 0.0;
                double fSSresid = 0.0;
                // re-use memory of Z;
                pMatZ->FillDouble(0.0, 0, 0, N-1, 0);
                // Z = R * Slopes
                lcl_ApplyUpperRightTriangle(pMatX, aVecR, pSlopes, pMatZ, K, true);
                // Z = Q * Z, that is Q * R * Slopes = X * Slopes
                for (SCSIZE rowp1 = K; rowp1 > 0; rowp1--)
                {
                    lcl_TApplyHouseholderTransformation(pMatX, rowp1-1, pMatZ,N);
                }
                fSSreg =lcl_GetSumProduct(pMatZ, pMatZ, N);
                // re-use Y for residuals, Y = Y-Z
                for (SCSIZE col = 0; col < N; col++)
                    pMatY->PutDouble(pMatY->GetDouble(col) - pMatZ->GetDouble(col), col);
                fSSresid = lcl_GetSumProduct(pMatY, pMatY, N);
                pResMat->PutDouble(fSSreg, 0, 4);
                pResMat->PutDouble(fSSresid, 1, 4);

                double fDegreesFreedom =static_cast<double>( bConstant ? N-K-1 : N-K );
                pResMat->PutDouble(fDegreesFreedom, 1, 3);

                if (fDegreesFreedom == 0.0 || fSSresid == 0.0 || fSSreg == 0.0)
                {   // exact fit; incl. case observed values Y are identical
                    pResMat->PutDouble(0.0, 1, 4); // SSresid
                    // F = (SSreg/K) / (SSresid/df) = #DIV/0!
                    pResMat->PutError( FormulaError::NotAvailable, 0, 3); // F
                    // RMSE = sqrt(SSresid / df) = sqrt(0 / df) = 0
                    pResMat->PutDouble(0.0, 1, 2); // RMSE
                    // SigmaSlope[i] = RMSE * sqrt(matrix[i,i]) = 0 * sqrt(...) = 0
                    for (SCSIZE i=0; i<K; i++)
                        pResMat->PutDouble(0.0, K-1-i, 1);

                    // SigmaIntercept = RMSE * sqrt(...) = 0
                    if (bConstant)
                        pResMat->PutDouble(0.0, K, 1); //SigmaIntercept
                    else
                        pResMat->PutError( FormulaError::NotAvailable, K, 1);

                    //  R^2 = SSreg / (SSreg + SSresid) = 1.0
                    pResMat->PutDouble(1.0, 0, 2); // R^2
                }
                else
                {
                    double fFstatistic = (fSSreg / static_cast<double>(K))
                                         / (fSSresid / fDegreesFreedom);
                    pResMat->PutDouble(fFstatistic, 0, 3);

                    // standard error of estimate = root mean SSE
                    double fRMSE = sqrt(fSSresid / fDegreesFreedom);
                    pResMat->PutDouble(fRMSE, 1, 2);

                    // standard error of slopes
                    // = RMSE * sqrt(diagonal element of (R' R)^(-1) )
                    // standard error of intercept
                    // = RMSE * sqrt( Xmean * (R' R)^(-1) * Xmean' + 1/N)
                    // (R' R)^(-1) = R^(-1) * (R')^(-1). Do not calculate it as
                    // a whole matrix, but iterate over unit vectors.
                    // (R' R) ^(-1) is symmetric
                    double fSigmaIntercept = 0.0;
                    double fPart; // for Xmean * single col of (R' R)^(-1)
                    for (SCSIZE row = 0; row < K; row++)
                    {
                        //re-use memory of MatZ
                        pMatZ->FillDouble(0.0,0,0,K-1,0); // Z = unit vector e
                        pMatZ->PutDouble(1.0, row);
                        //Solve R' * Z = e
                        lcl_SolveWithLowerLeftTriangle(pMatX, aVecR, pMatZ, K, true);
                        // Solve R * Znew = Zold
                        lcl_SolveWithUpperRightTriangle(pMatX, aVecR, pMatZ, K, true);
                        // now Z is column col in (R' R)^(-1)
                        double fSigmaSlope = fRMSE * sqrt(pMatZ->GetDouble(row));
                        pResMat->PutDouble(fSigmaSlope, K-1-row, 1);
                        if (bConstant)
                        {
                            fPart = lcl_GetSumProduct(pMeans, pMatZ, K);
                            fSigmaIntercept += fPart * pMeans->GetDouble(row);
                        }
                    }
                    if (bConstant)
                    {
                        fSigmaIntercept = fRMSE
                                          * sqrt(fSigmaIntercept + 1.0 / static_cast<double>(N));
                        pResMat->PutDouble(fSigmaIntercept, K, 1);
                    }
                    else
                    {
                        pResMat->PutError( FormulaError::NotAvailable, K, 1);
                    }

                    double fR2 = fSSreg / (fSSreg + fSSresid);
                    pResMat->PutDouble(fR2, 0, 2);
                }
            }
            PushMatrix(pResMat);
        }
    }
}

void ScInterpreter::ScTrend()
{
    CalculateTrendGrowth(false);
}

void ScInterpreter::ScGrowth()
{
    CalculateTrendGrowth(true);
}

void ScInterpreter::CalculateTrendGrowth(bool _bGrowth)
{
    sal_uInt8 nParamCount = GetByte();
    if (!MustHaveParamCount( nParamCount, 1, 4 ))
        return;

    // optional forth parameter
    bool bConstant;
    if (nParamCount == 4)
        bConstant = GetBool();
    else
        bConstant = true;

    // The third parameter may be missing in ODF, although the forth parameter
    // is present. Default values depend on data not yet read.
    ScMatrixRef pMatNewX;
    if (nParamCount >= 3)
    {
        if (IsMissing())
        {
            Pop();
            pMatNewX = nullptr;
        }
        else
            pMatNewX = GetMatrix();
    }
    else
        pMatNewX = nullptr;

    //In ODF1.2 empty second parameter (which is two ;; ) is allowed
    //Defaults will be set in CheckMatrix
    ScMatrixRef pMatX;
    if (nParamCount >= 2)
    {
        if (IsMissing())
        {
            Pop();
            pMatX = nullptr;
        }
        else
        {
            pMatX = GetMatrix();
        }
    }
    else
        pMatX = nullptr;

    ScMatrixRef pMatY;
    pMatY = GetMatrix();
    if (!pMatY)
    {
        PushIllegalParameter();
        return;
    }

    // 1 = simple; 2 = multiple with Y as column; 3 = multiple with Y as row
    sal_uInt8 nCase;

    SCSIZE nCX, nCY; // number of columns
    SCSIZE nRX, nRY; //number of rows
    SCSIZE K = 0, N = 0; // K=number of variables X, N=number of data samples
    if (!CheckMatrix(_bGrowth,nCase,nCX,nCY,nRX,nRY,K,N,pMatX,pMatY))
    {
        PushIllegalParameter();
        return;
    }

    // Enough data samples?
    if ((bConstant && (N<K+1)) || (!bConstant && (N<K)) || (N<1) || (K<1))
    {
        PushIllegalParameter();
        return;
    }

    // Set default pMatNewX if necessary
    SCSIZE nCXN, nRXN;
    SCSIZE nCountXN;
    if (!pMatNewX)
    {
        nCXN = nCX;
        nRXN = nRX;
        nCountXN = nCXN * nRXN;
        pMatNewX = pMatX->Clone(); // pMatX will be changed to X-meanX
    }
    else
    {
        pMatNewX->GetDimensions(nCXN, nRXN);
        if ((nCase == 2 && K != nCXN) || (nCase == 3 && K != nRXN))
        {
            PushIllegalArgument();
            return;
        }
        nCountXN = nCXN * nRXN;
        for (SCSIZE i = 0; i < nCountXN; i++)
            if (!pMatNewX->IsValue(i))
            {
                PushIllegalArgument();
                return;
            }
    }
    ScMatrixRef pResMat; // size depends on nCase
    if (nCase == 1)
        pResMat = GetNewMat(nCXN,nRXN);
    else
    {
        if (nCase==2)
            pResMat = GetNewMat(1,nRXN);
        else
            pResMat = GetNewMat(nCXN,1);
    }
    if (!pResMat)
    {
        PushError(FormulaError::CodeOverflow);
        return;
    }
    // Uses sum(x-MeanX)^2 and not [sum x^2]-N * MeanX^2 in case bConstant.
    // Clone constant matrices, so that Mat = Mat - Mean is possible.
    double fMeanY = 0.0;
    if (bConstant)
    {
        ScMatrixRef pCopyX = pMatX->CloneIfConst();
        ScMatrixRef pCopyY = pMatY->CloneIfConst();
        if (!pCopyX || !pCopyY)
        {
            PushError(FormulaError::MatrixSize);
            return;
        }
        pMatX = pCopyX;
        pMatY = pCopyY;
        // DeltaY is possible here; DeltaX depends on nCase, so later
        fMeanY = lcl_GetMeanOverAll(pMatY, N);
        for (SCSIZE i=0; i<N; i++)
        {
            pMatY->PutDouble( ::rtl::math::approxSub(pMatY->GetDouble(i),fMeanY), i );
        }
    }

    if (nCase==1)
    {
        // calculate simple regression
        double fMeanX = 0.0;
        if (bConstant)
        {   // Mat = Mat - Mean
            fMeanX = lcl_GetMeanOverAll(pMatX, N);
            for (SCSIZE i=0; i<N; i++)
            {
                pMatX->PutDouble( ::rtl::math::approxSub(pMatX->GetDouble(i),fMeanX), i );
            }
        }
        double fSumXY = lcl_GetSumProduct(pMatX,pMatY,N);
        double fSumX2 = lcl_GetSumProduct(pMatX,pMatX,N);
        if (fSumX2==0.0)
        {
            PushNoValue(); // all x-values are identical
            return;
        }
        double fSlope = fSumXY / fSumX2;
        double fHelp;
        if (bConstant)
        {
            double fIntercept = fMeanY - fSlope * fMeanX;
            for (SCSIZE i = 0; i < nCountXN; i++)
            {
                fHelp = pMatNewX->GetDouble(i)*fSlope + fIntercept;
                pResMat->PutDouble(_bGrowth ? exp(fHelp) : fHelp, i);
            }
        }
        else
        {
            for (SCSIZE i = 0; i < nCountXN; i++)
            {
                fHelp = pMatNewX->GetDouble(i)*fSlope;
                pResMat->PutDouble(_bGrowth ? exp(fHelp) : fHelp, i);
            }
        }
    }
    else // calculate multiple regression;
    {
        if (nCase ==2) // Y is column
        {
            ::std::vector< double> aVecR(N); // for QR decomposition
            // Enough memory for needed matrices?
            ScMatrixRef pMeans = GetNewMat(K, 1); // mean of each column
            ScMatrixRef pSlopes = GetNewMat(1,K); // from b1 to bK
            if (!pMeans || !pSlopes)
            {
                PushError(FormulaError::CodeOverflow);
                return;
            }
            if (bConstant)
            {
                lcl_CalculateColumnMeans(pMatX, pMeans, K, N);
                lcl_CalculateColumnsDelta(pMatX, pMeans, K, N);
            }
            if (!lcl_CalculateQRdecomposition(pMatX, aVecR, K, N))
            {
                PushNoValue();
                return;
            }
            // Later on we will divide by elements of aVecR, so make sure
            // that they aren't zero.
            bool bIsSingular=false;
            for (SCSIZE row=0; row < K && !bIsSingular; row++)
                bIsSingular = aVecR[row] == 0.0;
            if (bIsSingular)
            {
                PushNoValue();
                return;
            }
            // Z := Q' Y; Y is overwritten with result Z
            for (SCSIZE col = 0; col < K; col++)
            {
                lcl_ApplyHouseholderTransformation(pMatX, col, pMatY, N);
            }
            // B = R^(-1) * Q' * Y <=> B = R^(-1) * Z <=> R * B = Z
            // result Z should have zeros for index>=K; if not, ignore values
            for (SCSIZE col = 0; col < K ; col++)
            {
                pSlopes->PutDouble( pMatY->GetDouble(col), col);
            }
            lcl_SolveWithUpperRightTriangle(pMatX, aVecR, pSlopes, K, false);

            // Fill result matrix
            lcl_MFastMult(pMatNewX,pSlopes,pResMat,nRXN,K,1);
            if (bConstant)
            {
                double fIntercept = fMeanY - lcl_GetSumProduct(pMeans,pSlopes,K);
                for (SCSIZE row = 0; row < nRXN; row++)
                    pResMat->PutDouble(pResMat->GetDouble(row)+fIntercept, row);
            }
            if (_bGrowth)
            {
                for (SCSIZE i = 0; i < nRXN; i++)
                    pResMat->PutDouble(exp(pResMat->GetDouble(i)), i);
            }
        }
        else
        { // nCase == 3, Y is row, all matrices are transposed

            ::std::vector< double> aVecR(N); // for QR decomposition
            // Enough memory for needed matrices?
            ScMatrixRef pMeans = GetNewMat(1, K); // mean of each row
            ScMatrixRef pSlopes = GetNewMat(K,1); // row from b1 to bK
            if (!pMeans || !pSlopes)
            {
                PushError(FormulaError::CodeOverflow);
                return;
            }
            if (bConstant)
            {
                lcl_CalculateRowMeans(pMatX, pMeans, N, K);
                lcl_CalculateRowsDelta(pMatX, pMeans, N, K);
            }
            if (!lcl_TCalculateQRdecomposition(pMatX, aVecR, K, N))
            {
                PushNoValue();
                return;
            }
            // Later on we will divide by elements of aVecR, so make sure
            // that they aren't zero.
            bool bIsSingular=false;
            for (SCSIZE row=0; row < K && !bIsSingular; row++)
                bIsSingular = aVecR[row] == 0.0;
            if (bIsSingular)
            {
                PushNoValue();
                return;
            }
            // Z := Q' Y; Y is overwritten with result Z
            for (SCSIZE row = 0; row < K; row++)
            {
                lcl_TApplyHouseholderTransformation(pMatX, row, pMatY, N);
            }
            // B = R^(-1) * Q' * Y <=> B = R^(-1) * Z <=> R * B = Z
            // result Z should have zeros for index>=K; if not, ignore values
            for (SCSIZE col = 0; col < K ; col++)
            {
                pSlopes->PutDouble( pMatY->GetDouble(col), col);
            }
            lcl_SolveWithUpperRightTriangle(pMatX, aVecR, pSlopes, K, true);

            // Fill result matrix
            lcl_MFastMult(pSlopes,pMatNewX,pResMat,1,K,nCXN);
            if (bConstant)
            {
                double fIntercept = fMeanY - lcl_GetSumProduct(pMeans,pSlopes,K);
                for (SCSIZE col = 0; col < nCXN; col++)
                    pResMat->PutDouble(pResMat->GetDouble(col)+fIntercept, col);
            }
            if (_bGrowth)
            {
                for (SCSIZE i = 0; i < nCXN; i++)
                    pResMat->PutDouble(exp(pResMat->GetDouble(i)), i);
            }
        }
    }
    PushMatrix(pResMat);
}

void ScInterpreter::ScMatRef()
{
    // In case it contains relative references resolve them as usual.
    Push( *pCur );
    ScAddress aAdr;
    PopSingleRef( aAdr );

    ScRefCellValue aCell(*pDok, aAdr);

    if (aCell.meType != CELLTYPE_FORMULA)
    {
        PushError( FormulaError::NoRef );
        return;
    }

    if (aCell.mpFormula->IsRunning())
    {
        // Twisted odd corner case where an array element's cell tries to
        // access the top left matrix while it is still running, see tdf#88737
        // This is a hackish workaround, not a general solution, the matrix
        // isn't available anyway and FormulaError::CircularReference would be set.
        PushError( FormulaError::RetryCircular );
        return;
    }

    const ScMatrix* pMat = aCell.mpFormula->GetMatrix();
    if (pMat)
    {
        SCSIZE nCols, nRows;
        pMat->GetDimensions( nCols, nRows );
        SCSIZE nC = static_cast<SCSIZE>(aPos.Col() - aAdr.Col());
        SCSIZE nR = static_cast<SCSIZE>(aPos.Row() - aAdr.Row());
        if ((nCols <= nC && nCols != 1) || (nRows <= nR && nRows != 1))
            PushNA();
        else
        {
            const ScMatrixValue nMatVal = pMat->Get( nC, nR);
            ScMatValType nMatValType = nMatVal.nType;

            if (ScMatrix::IsNonValueType( nMatValType))
            {
                if (ScMatrix::IsEmptyPathType( nMatValType))
                {   // result of empty false jump path
                    nFuncFmtType = SvNumFormatType::LOGICAL;
                    PushInt(0);
                }
                else if (ScMatrix::IsEmptyType( nMatValType))
                {
                    // Not inherited (really?) and display as empty string, not 0.
                    PushTempToken( new ScEmptyCellToken( false, true));
                }
                else
                    PushString( nMatVal.GetString() );
            }
            else
            {
                // Determine nFuncFmtType type before PushDouble().
                pDok->GetNumberFormatInfo(mrContext, nCurFmtType, nCurFmtIndex, aAdr);
                nFuncFmtType = nCurFmtType;
                nFuncFmtIndex = nCurFmtIndex;
                PushDouble(nMatVal.fVal);  // handles DoubleError
            }
        }
    }
    else
    {
        // Determine nFuncFmtType type before PushDouble().
        pDok->GetNumberFormatInfo(mrContext, nCurFmtType, nCurFmtIndex, aAdr);
        nFuncFmtType = nCurFmtType;
        nFuncFmtIndex = nCurFmtIndex;
        // If not a result matrix, obtain the cell value.
        FormulaError nErr = aCell.mpFormula->GetErrCode();
        if (nErr != FormulaError::NONE)
            PushError( nErr );
        else if (aCell.mpFormula->IsValue())
            PushDouble(aCell.mpFormula->GetValue());
        else
        {
            svl::SharedString aVal = aCell.mpFormula->GetString();
            PushString( aVal );
        }
    }
}

void ScInterpreter::ScInfo()
{
    if( MustHaveParamCount( GetByte(), 1 ) )
    {
        OUString aStr = GetString().getString();
        ScCellKeywordTranslator::transKeyword(aStr, ScGlobal::GetLocale(), ocInfo);
        if( aStr == "SYSTEM" )
            PushString( OUString( SC_INFO_OSVERSION ) );
        else if( aStr == "OSVERSION" )
            PushString( OUString( "Windows (32-bit) NT 5.01" ) );
        else if( aStr == "RELEASE" )
            PushString( ::utl::Bootstrap::getBuildIdData( OUString() ) );
        else if( aStr == "NUMFILE" )
            PushDouble( 1 );
        else if( aStr == "RECALC" )
            PushString( ScResId( pDok->GetAutoCalc() ? STR_RECALC_AUTO : STR_RECALC_MANUAL ) );
        else if (aStr == "DIRECTORY" || aStr == "MEMAVAIL" || aStr == "MEMUSED" || aStr == "ORIGIN" || aStr == "TOTMEM")
            PushNA();
        else
            PushIllegalArgument();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
