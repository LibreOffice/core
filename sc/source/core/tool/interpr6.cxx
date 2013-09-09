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


#include "interpre.hxx"
#include "columnspanset.hxx"
#include "column.hxx"
#include "document.hxx"
#include "cellvalue.hxx"
#include "dociter.hxx"
#include "mtvcellfunc.hxx"

#include "formula/token.hxx"

using namespace formula;

double const fHalfMachEps = 0.5 * ::std::numeric_limits<double>::epsilon();

// The idea how this group of gamma functions is calculated, is
// based on the Cephes library
// online http://www.moshier.net/#Cephes [called 2008-02]

/** You must ensure fA>0.0 && fX>0.0
    valid results only if fX > fA+1.0
    uses continued fraction with odd items */
double ScInterpreter::GetGammaContFraction( double fA, double fX )
{

    double const fBigInv = ::std::numeric_limits<double>::epsilon();
    double const fBig = 1.0/fBigInv;
    double fCount = 0.0;
    double fNum = 0.0;  // dummy value
    double fY = 1.0 - fA;
    double fDenom = fX + 2.0-fA;
    double fPk = 0.0;   // dummy value
    double fPkm1 = fX + 1.0;
    double fPkm2 = 1.0;
    double fQk = 1.0;   // dummy value
    double fQkm1 = fDenom * fX;
    double fQkm2 = fX;
    double fApprox = fPkm1/fQkm1;
    bool bFinished = false;
    double fR = 0.0;    // dummy value
    do
    {
        fCount = fCount +1.0;
        fY = fY+ 1.0;
        fNum = fY * fCount;
        fDenom = fDenom +2.0;
        fPk = fPkm1 * fDenom  -  fPkm2 * fNum;
        fQk = fQkm1 * fDenom  -  fQkm2 * fNum;
        if (fQk != 0.0)
        {
            fR = fPk/fQk;
            bFinished = (fabs( (fApprox - fR)/fR ) <= fHalfMachEps);
            fApprox = fR;
        }
        fPkm2 = fPkm1;
        fPkm1 = fPk;
        fQkm2 = fQkm1;
        fQkm1 = fQk;
        if (fabs(fPk) > fBig)
        {
            // reduce a fraction does not change the value
            fPkm2 = fPkm2 * fBigInv;
            fPkm1 = fPkm1 * fBigInv;
            fQkm2 = fQkm2 * fBigInv;
            fQkm1 = fQkm1 * fBigInv;
        }
    } while (!bFinished && fCount<10000);
    // most iterations, if fX==fAlpha+1.0; approx sqrt(fAlpha) iterations then
    if (!bFinished)
    {
        SetError(errNoConvergence);
    }
    return fApprox;
}

/** You must ensure fA>0.0 && fX>0.0
    valid results only if fX <= fA+1.0
    uses power series */
double ScInterpreter::GetGammaSeries( double fA, double fX )
{
    double fDenomfactor = fA;
    double fSummand = 1.0/fA;
    double fSum = fSummand;
    int nCount=1;
    do
    {
        fDenomfactor = fDenomfactor + 1.0;
        fSummand = fSummand * fX/fDenomfactor;
        fSum = fSum + fSummand;
        nCount = nCount+1;
    } while ( fSummand/fSum > fHalfMachEps && nCount<=10000);
    // large amount of iterations will be carried out for huge fAlpha, even
    // if fX <= fAlpha+1.0
    if (nCount>10000)
    {
        SetError(errNoConvergence);
    }
    return fSum;
}

/** You must ensure fA>0.0 && fX>0.0) */
double ScInterpreter::GetLowRegIGamma( double fA, double fX )
{
    double fLnFactor = fA * log(fX) - fX - GetLogGamma(fA);
    double fFactor = exp(fLnFactor);    // Do we need more accuracy than exp(ln()) has?
    if (fX>fA+1.0)  // includes fX>1.0; 1-GetUpRegIGamma, continued fraction
        return 1.0 - fFactor * GetGammaContFraction(fA,fX);
    else            // fX<=1.0 || fX<=fA+1.0, series
        return fFactor * GetGammaSeries(fA,fX);
}

/** You must ensure fA>0.0 && fX>0.0) */
double ScInterpreter::GetUpRegIGamma( double fA, double fX )
{

    double fLnFactor= fA*log(fX)-fX-GetLogGamma(fA);
    double fFactor = exp(fLnFactor); //Do I need more accuracy than exp(ln()) has?;
    if (fX>fA+1.0) // includes fX>1.0
            return fFactor * GetGammaContFraction(fA,fX);
    else //fX<=1 || fX<=fA+1, 1-GetLowRegIGamma, series
            return 1.0 -fFactor * GetGammaSeries(fA,fX);
}

/** Gamma distribution, probability density function.
    fLambda is "scale" parameter
    You must ensure fAlpha>0.0 and fLambda>0.0 */
double ScInterpreter::GetGammaDistPDF( double fX, double fAlpha, double fLambda )
{
    if (fX < 0.0)
        return 0.0;     // see ODFF
    else if (fX == 0)
        // in this case 0^0 isn't zero
    {
        if (fAlpha < 1.0)
        {
            SetError(errDivisionByZero);  // should be #DIV/0
            return HUGE_VAL;
        }
        else if (fAlpha == 1)
        {
            return (1.0 / fLambda);
        }
        else
        {
            return 0.0;
        }
    }
    else
    {
        double fXr = fX / fLambda;
        // use exp(ln()) only for large arguments because of less accuracy
        if (fXr > 1.0)
        {
            const double fLogDblMax = log( ::std::numeric_limits<double>::max());
            if (log(fXr) * (fAlpha-1.0) < fLogDblMax && fAlpha < fMaxGammaArgument)
            {
                return pow( fXr, fAlpha-1.0) * exp(-fXr) / fLambda / GetGamma(fAlpha);
            }
            else
            {
                return exp( (fAlpha-1.0) * log(fXr) - fXr - log(fLambda) - GetLogGamma(fAlpha));
            }
        }
        else    // fXr near to zero
        {
            if (fAlpha<fMaxGammaArgument)
            {
                return pow( fXr, fAlpha-1.0) * exp(-fXr) / fLambda / GetGamma(fAlpha);
            }
            else
            {
                return pow( fXr, fAlpha-1.0) * exp(-fXr) / fLambda / exp( GetLogGamma(fAlpha));
            }
        }
    }
}

/** Gamma distribution, cumulative distribution function.
    fLambda is "scale" parameter
    You must ensure fAlpha>0.0 and fLambda>0.0 */
double ScInterpreter::GetGammaDist( double fX, double fAlpha, double fLambda )
{
    if (fX <= 0.0)
        return 0.0;
    else
        return GetLowRegIGamma( fAlpha, fX / fLambda);
}

namespace {

class NumericCellAccumulator
{
    double mfSum;
public:
    NumericCellAccumulator() : mfSum(0.0) {}

    void operator() (size_t, double fVal)
    {
        mfSum += fVal;
    }

    void operator() (size_t, const ScFormulaCell* pCell)
    {
        ScFormulaCell& rCell = const_cast<ScFormulaCell&>(*pCell);
        if (rCell.IsValue())
            mfSum += rCell.GetValue();
    }

    double getSum() const { return mfSum; }
};

class NumericCellCounter
{
    size_t mnCount;
public:
    NumericCellCounter() : mnCount(0) {}

    void operator() (const sc::CellStoreType::value_type& rNode, size_t nOffset, size_t nDataSize)
    {
        switch (rNode.type)
        {
            case sc::element_type_numeric:
                mnCount += nDataSize;
            break;
            case sc::element_type_formula:
            {
                sc::formula_block::const_iterator it = sc::formula_block::begin(*rNode.data);
                std::advance(it, nOffset);
                sc::formula_block::const_iterator itEnd = it;
                std::advance(itEnd, nDataSize);
                for (; it != itEnd; ++it)
                {
                    ScFormulaCell& rCell = const_cast<ScFormulaCell&>(**it);
                    if (rCell.IsValueNoError())
                        ++mnCount;
                }
            }
            break;
            default:
                ;
        }
    }

    size_t getCount() const { return mnCount; }
};

class FuncCount : public sc::ColumnSpanSet::ColumnAction
{
    sc::ColumnBlockConstPosition maPos;
    ScColumn* mpCol;
    size_t mnCount;
    sal_uInt32 mnNumFmt;

public:
    FuncCount() : mnCount(0), mnNumFmt(0) {}

    virtual void startColumn(ScColumn* pCol)
    {
        mpCol = pCol;
        mpCol->InitBlockPosition(maPos);
    }

    virtual void execute(SCROW nRow1, SCROW nRow2, bool bVal)
    {
        if (!bVal)
            return;

        NumericCellCounter aFunc;
        maPos.miCellPos = sc::ParseBlock(maPos.miCellPos, mpCol->GetCellStore(), aFunc, nRow1, nRow2);
        mnCount += aFunc.getCount();
        mnNumFmt = mpCol->GetNumberFormat(nRow2);
    };

    size_t getCount() const { return mnCount; }
    sal_uInt32 getNumberFormat() const { return mnNumFmt; }
};

class FuncSum : public sc::ColumnSpanSet::ColumnAction
{
    sc::ColumnBlockConstPosition maPos;
    ScColumn* mpCol;
    double mfSum;
    sal_uInt32 mnNumFmt;

public:
    FuncSum() : mfSum(0.0), mnNumFmt(0) {}

    virtual void startColumn(ScColumn* pCol)
    {
        mpCol = pCol;
        mpCol->InitBlockPosition(maPos);
    }

    virtual void execute(SCROW nRow1, SCROW nRow2, bool bVal)
    {
        if (!bVal)
            return;

        NumericCellAccumulator aFunc;
        maPos.miCellPos = sc::ParseFormulaNumeric(maPos.miCellPos, mpCol->GetCellStore(), nRow1, nRow2, aFunc);
        mfSum += aFunc.getSum();
        mnNumFmt = mpCol->GetNumberFormat(nRow2);
    };

    double getSum() const { return mfSum; }
    sal_uInt32 getNumberFormat() const { return mnNumFmt; }
};

void IterateMatrix(
    const ScMatrixRef& pMat, ScIterFunc eFunc, bool bTextAsZero,
    sal_uLong& rCount, short& rFuncFmtType, double& fRes, double& fMem, bool& bNull)
{
    if (!pMat)
        return;

    rFuncFmtType = NUMBERFORMAT_NUMBER;
    switch (eFunc)
    {
        case ifAVERAGE:
        case ifSUM:
        {
            ScMatrix::IterateResult aRes = pMat->Sum(bTextAsZero);
            if (bNull)
            {
                bNull = false;
                fMem = aRes.mfFirst;
                fRes += aRes.mfRest;
            }
            else
                fRes += aRes.mfFirst + aRes.mfRest;
            rCount += aRes.mnCount;
        }
        break;
        case ifCOUNT:
            rCount += pMat->Count(bTextAsZero);
        break;
        case ifCOUNT2:
            rCount += pMat->Count(true);
        break;
        case ifPRODUCT:
        {
            ScMatrix::IterateResult aRes = pMat->Product(bTextAsZero);
            fRes *= aRes.mfRest;
            rCount += aRes.mnCount;
        }
        break;
        case ifSUMSQ:
        {
            ScMatrix::IterateResult aRes = pMat->SumSquare(bTextAsZero);
            fRes += aRes.mfRest;
            rCount += aRes.mnCount;
        }
        break;
        default:
            ;
    }
}

}

double ScInterpreter::IterateParameters( ScIterFunc eFunc, bool bTextAsZero )
{
    short nParamCount = GetByte();
    double fRes = ( eFunc == ifPRODUCT ) ? 1.0 : 0.0;
    double fVal = 0.0;
    double fMem = 0.0; // first numeric value.
    bool bNull = true;
    sal_uLong nCount = 0;
    ScAddress aAdr;
    ScRange aRange;
    size_t nRefInList = 0;
    if ( nGlobalError && ( eFunc == ifCOUNT2 || eFunc == ifCOUNT ) )
        nGlobalError = 0;
    while (nParamCount-- > 0)
    {
        switch (GetStackType())
        {
            case svString:
            {
                if( eFunc == ifCOUNT )
                {
                    String aStr( PopString() );
                    sal_uInt32 nFIndex = 0;                 // damit default Land/Spr.
                    if ( bTextAsZero || pFormatter->IsNumberFormat(aStr, nFIndex, fVal))
                        nCount++;
                }
                else
                {
                    switch ( eFunc )
                    {
                        case ifAVERAGE:
                        case ifSUM:
                        case ifSUMSQ:
                        case ifPRODUCT:
                        {
                            if ( bTextAsZero )
                            {
                                Pop();
                                nCount++;
                                if ( eFunc == ifPRODUCT )
                                    fRes = 0.0;
                            }
                            else
                            {
                                while (nParamCount-- > 0)
                                    Pop();
                                SetError( errNoValue );
                            }
                        }
                        break;
                        default:
                            Pop();
                            nCount++;
                    }
                }
            }
            break;
            case svDouble    :
                fVal = GetDouble();
                nCount++;
                switch( eFunc )
                {
                    case ifAVERAGE:
                    case ifSUM:
                        if ( bNull && fVal != 0.0 )
                        {
                            bNull = false;
                            fMem = fVal;
                        }
                        else
                            fRes += fVal;
                        break;
                    case ifSUMSQ:   fRes += fVal * fVal; break;
                    case ifPRODUCT: fRes *= fVal; break;
                    default: ; // nothing
                }
                nFuncFmtType = NUMBERFORMAT_NUMBER;
                break;
            case svExternalSingleRef:
            {
                ScExternalRefCache::TokenRef pToken;
                ScExternalRefCache::CellFormat aFmt;
                PopExternalSingleRef(pToken, &aFmt);
                if (nGlobalError && (eFunc == ifCOUNT2 || eFunc == ifCOUNT))
                {
                    nGlobalError = 0;
                    if ( eFunc == ifCOUNT2 )
                        ++nCount;
                    break;
                }

                if (!pToken)
                    break;

                StackVar eType = pToken->GetType();
                if (eFunc == ifCOUNT2)
                {
                    if (eType != formula::svEmptyCell)
                        nCount++;
                    if (nGlobalError)
                        nGlobalError = 0;
                }
                else if (eType == formula::svDouble)
                {
                    nCount++;
                    fVal = pToken->GetDouble();
                    if (aFmt.mbIsSet)
                    {
                        nFuncFmtType = aFmt.mnType;
                        nFuncFmtIndex = aFmt.mnIndex;
                    }
                    switch( eFunc )
                    {
                        case ifAVERAGE:
                        case ifSUM:
                            if ( bNull && fVal != 0.0 )
                            {
                                bNull = false;
                                fMem = fVal;
                            }
                            else
                                fRes += fVal;
                            break;
                        case ifSUMSQ:   fRes += fVal * fVal; break;
                        case ifPRODUCT: fRes *= fVal; break;
                        case ifCOUNT:
                            if ( nGlobalError )
                            {
                                nGlobalError = 0;
                                nCount--;
                            }
                            break;
                        default: ; // nothing
                    }
                }
                else if (bTextAsZero && eType == formula::svString)
                {
                    nCount++;
                    if ( eFunc == ifPRODUCT )
                        fRes = 0.0;
                }
            }
            break;
            case svSingleRef :
            {
                PopSingleRef( aAdr );
                if ( nGlobalError && ( eFunc == ifCOUNT2 || eFunc == ifCOUNT ) )
                {
                    nGlobalError = 0;
                    if ( eFunc == ifCOUNT2 )
                        ++nCount;
                    break;
                }
                if (glSubTotal && pDok->RowFiltered( aAdr.Row(), aAdr.Tab()))
                {
                    break;
                }
                ScRefCellValue aCell;
                aCell.assign(*pDok, aAdr);
                if (!aCell.isEmpty())
                {
                    if( eFunc == ifCOUNT2 )
                    {
                        CellType eCellType = aCell.meType;
                        if (eCellType != CELLTYPE_NONE)
                            nCount++;
                        if ( nGlobalError )
                            nGlobalError = 0;
                    }
                    else if (aCell.hasNumeric())
                    {
                        nCount++;
                        fVal = GetCellValue(aAdr, aCell);
                        CurFmtToFuncFmt();
                        switch( eFunc )
                        {
                            case ifAVERAGE:
                            case ifSUM:
                                if ( bNull && fVal != 0.0 )
                                {
                                    bNull = false;
                                    fMem = fVal;
                                }
                                else
                                    fRes += fVal;
                                break;
                            case ifSUMSQ:   fRes += fVal * fVal; break;
                            case ifPRODUCT: fRes *= fVal; break;
                            case ifCOUNT:
                                if ( nGlobalError )
                                {
                                    nGlobalError = 0;
                                    nCount--;
                                }
                                break;
                            default: ; // nothing
                        }
                    }
                    else if (bTextAsZero && aCell.hasString())
                    {
                        nCount++;
                        if ( eFunc == ifPRODUCT )
                            fRes = 0.0;
                    }
                }
            }
            break;
            case svDoubleRef :
            case svRefList :
            {
                PopDoubleRef( aRange, nParamCount, nRefInList);
                if ( nGlobalError && ( eFunc == ifCOUNT2 || eFunc == ifCOUNT ) )
                {
                    nGlobalError = 0;
                    if ( eFunc == ifCOUNT2 )
                        ++nCount;
                    break;
                }
                if( eFunc == ifCOUNT2 )
                {
                    ScCellIterator aIter( pDok, aRange, glSubTotal );
                    for (bool bHas = aIter.first(); bHas; bHas = aIter.next())
                    {
                        if (!aIter.hasEmptyData())
                            ++nCount;
                    }

                    if ( nGlobalError )
                        nGlobalError = 0;
                }
                else
                {
                    ScValueIterator aValIter( pDok, aRange, glSubTotal, bTextAsZero );
                    sal_uInt16 nErr = 0;
                    if (aValIter.GetFirst(fVal, nErr))
                    {
                        // placed the loop on the inside for performance reasons:
                        aValIter.GetCurNumFmtInfo( nFuncFmtType, nFuncFmtIndex );
                        switch( eFunc )
                        {
                            case ifAVERAGE:
                            case ifSUM:
                                    do
                                    {
                                        SetError(nErr);
                                        if ( bNull && fVal != 0.0 )
                                        {
                                            bNull = false;
                                            fMem = fVal;
                                        }
                                        else
                                            fRes += fVal;
                                        nCount++;
                                    }
                                    while (aValIter.GetNext(fVal, nErr));
                                    break;
                            case ifSUMSQ:
                                    do
                                    {
                                        SetError(nErr);
                                        fRes += fVal * fVal;
                                        nCount++;
                                    }
                                    while (aValIter.GetNext(fVal, nErr));
                                    break;
                            case ifPRODUCT:
                                    do
                                    {
                                        SetError(nErr);
                                        fRes *= fVal;
                                        nCount++;
                                    }
                                    while (aValIter.GetNext(fVal, nErr));
                                    break;
                            case ifCOUNT:
                                    do
                                    {
                                        if ( !nErr )
                                            nCount++;
                                    }
                                    while (aValIter.GetNext(fVal, nErr));
                                    break;
                            default: ;  // nothing
                        }
                        SetError( nErr );
                    }
                }
            }
            break;
            case svExternalDoubleRef:
            {
                ScMatrixRef pMat;
                PopExternalDoubleRef(pMat);
                if (nGlobalError)
                    break;

                IterateMatrix(pMat, eFunc, bTextAsZero, nCount, nFuncFmtType, fRes, fMem, bNull);
            }
            break;
            case svMatrix :
            {
                ScMatrixRef pMat = PopMatrix();
                IterateMatrix(pMat, eFunc, bTextAsZero, nCount, nFuncFmtType, fRes, fMem, bNull);
            }
            break;
            case svError:
            {
                PopError();
                if ( eFunc == ifCOUNT )
                {
                    nGlobalError = 0;
                }
                else if ( eFunc == ifCOUNT2 )
                {
                    nCount++;
                    nGlobalError = 0;
                }
            }
            break;
            default :
                while (nParamCount-- > 0)
                    PopError();
                SetError(errIllegalParameter);
        }
    }
    switch( eFunc )
    {
        case ifSUM:     fRes = ::rtl::math::approxAdd( fRes, fMem ); break;
        case ifAVERAGE: fRes = div(::rtl::math::approxAdd( fRes, fMem ), nCount); break;
        case ifCOUNT2:
        case ifCOUNT:   fRes  = nCount; break;
        case ifPRODUCT: if ( !nCount ) fRes = 0.0; break;
        default: ; // nothing
    }
    // Bei Summen etc. macht ein bool-Ergebnis keinen Sinn
    // und Anzahl ist immer Number (#38345#)
    if( eFunc == ifCOUNT || nFuncFmtType == NUMBERFORMAT_LOGICAL )
        nFuncFmtType = NUMBERFORMAT_NUMBER;
    return fRes;
}


void ScInterpreter::ScSumSQ()
{
    PushDouble( IterateParameters( ifSUMSQ ) );
}


void ScInterpreter::ScSum()
{
    short nParamCount = GetByte();
    double fRes = 0.0;
    double fVal = 0.0;
    ScAddress aAdr;
    ScRange aRange;
    size_t nRefInList = 0;
    while (nParamCount-- > 0)
    {
        switch (GetStackType())
        {
            case svString:
            {
                while (nParamCount-- > 0)
                    Pop();
                SetError( errNoValue );
            }
            break;
            case svDouble    :
                fVal = GetDouble();
                fRes += fVal;
                nFuncFmtType = NUMBERFORMAT_NUMBER;
                break;
            case svExternalSingleRef:
            {
                ScExternalRefCache::TokenRef pToken;
                ScExternalRefCache::CellFormat aFmt;
                PopExternalSingleRef(pToken, &aFmt);

                if (!pToken)
                    break;

                StackVar eType = pToken->GetType();
                if (eType == formula::svDouble)
                {
                    fVal = pToken->GetDouble();
                    if (aFmt.mbIsSet)
                    {
                        nFuncFmtType = aFmt.mnType;
                        nFuncFmtIndex = aFmt.mnIndex;
                    }

                    fRes += fVal;
                }
            }
            break;
            case svSingleRef :
            {
                PopSingleRef( aAdr );

                if (glSubTotal && pDok->RowFiltered( aAdr.Row(), aAdr.Tab()))
                {
                    break;
                }
                ScRefCellValue aCell;
                aCell.assign(*pDok, aAdr);
                if (!aCell.isEmpty())
                {
                    if (aCell.hasNumeric())
                    {
                        fVal = GetCellValue(aAdr, aCell);
                        CurFmtToFuncFmt();
                        fRes += fVal;
                    }
                }
            }
            break;
            case svDoubleRef :
            case svRefList :
            {
                PopDoubleRef( aRange, nParamCount, nRefInList);

                sc::ColumnSpanSet aSet(false);
                aSet.set(aRange, true);
                if (glSubTotal)
                    // Skip all filtered rows and subtotal formula cells.
                    pDok->MarkSubTotalCells(aSet, aRange, false);

                FuncSum aAction;
                aSet.executeColumnAction(*pDok, aAction);
                fRes = aAction.getSum();

                // Get the number format of the last iterated cell.
                nFuncFmtIndex = aAction.getNumberFormat();
                nFuncFmtType = pDok->GetFormatTable()->GetType(nFuncFmtIndex);
            }
            break;
            case svExternalDoubleRef:
            {
                ScMatrixRef pMat;
                PopExternalDoubleRef(pMat);
                if (nGlobalError)
                    break;

                sal_uLong nCount = 0;
                double fMem = 0.0;
                bool bNull = true;
                IterateMatrix(pMat, ifSUM, false, nCount, nFuncFmtType, fRes, fMem, bNull);
                fRes += fMem;
            }
            break;
            case svMatrix :
            {
                ScMatrixRef pMat = PopMatrix();
                sal_uLong nCount = 0;
                double fMem = 0.0;
                bool bNull = true;
                IterateMatrix(pMat, ifSUM, false, nCount, nFuncFmtType, fRes, fMem, bNull);
                fRes += fMem;
            }
            break;
            case svError:
            {
                PopError();
            }
            break;
            default :
                while (nParamCount-- > 0)
                    PopError();
                SetError(errIllegalParameter);
        }
    }

    if (nFuncFmtType == NUMBERFORMAT_LOGICAL)
        nFuncFmtType = NUMBERFORMAT_NUMBER;

    PushDouble(fRes);
}


void ScInterpreter::ScProduct()
{
    PushDouble( IterateParameters( ifPRODUCT ) );
}


void ScInterpreter::ScAverage( bool bTextAsZero )
{
    PushDouble( IterateParameters( ifAVERAGE, bTextAsZero ) );
}

void ScInterpreter::ScCount()
{
    short nParamCount = GetByte();
    double fVal = 0.0;
    sal_uLong nCount = 0;
    ScAddress aAdr;
    ScRange aRange;
    size_t nRefInList = 0;
    if (nGlobalError)
        nGlobalError = 0;

    while (nParamCount-- > 0)
    {
        switch (GetRawStackType())
        {
            case svString:
            {
                String aStr( PopString() );
                sal_uInt32 nFIndex = 0;                 // damit default Land/Spr.
                if (pFormatter->IsNumberFormat(aStr, nFIndex, fVal))
                    nCount++;
            }
            break;
            case svDouble    :
                nCount++;
                nFuncFmtType = NUMBERFORMAT_NUMBER;
                break;
            case svExternalSingleRef:
            {
                ScExternalRefCache::TokenRef pToken;
                ScExternalRefCache::CellFormat aFmt;
                PopExternalSingleRef(pToken, &aFmt);
                if (nGlobalError)
                {
                    nGlobalError = 0;
                    break;
                }

                if (!pToken)
                    break;

                StackVar eType = pToken->GetType();
                if (eType == formula::svDouble)
                {
                    nCount++;
                    if (aFmt.mbIsSet)
                    {
                        nFuncFmtType = aFmt.mnType;
                        nFuncFmtIndex = aFmt.mnIndex;
                    }

                    if (nGlobalError)
                    {
                        nGlobalError = 0;
                        nCount--;
                    }
                }
            }
            break;
            case svSingleRef :
            {
                PopSingleRef( aAdr );
                if (nGlobalError)
                {
                    nGlobalError = 0;
                    break;
                }
                if (glSubTotal && pDok->RowFiltered( aAdr.Row(), aAdr.Tab()))
                {
                    break;
                }
                ScRefCellValue aCell;
                aCell.assign(*pDok, aAdr);
                if (!aCell.isEmpty())
                {
                    if (aCell.hasNumeric())
                    {
                        nCount++;
                        CurFmtToFuncFmt();
                        if (nGlobalError)
                        {
                            nGlobalError = 0;
                            nCount--;
                        }
                    }
                }
            }
            break;
            case svDoubleRef :
            case svRefList :
            {
                PopDoubleRef( aRange, nParamCount, nRefInList);
                if (nGlobalError)
                {
                    nGlobalError = 0;
                    break;
                }

                sc::ColumnSpanSet aSet(false);
                aSet.set(aRange, true);
                if (glSubTotal)
                    // Skip all filtered rows and subtotal formula cells.
                    pDok->MarkSubTotalCells(aSet, aRange, false);

                FuncCount aAction;
                aSet.executeColumnAction(*pDok, aAction);
                nCount = aAction.getCount();

                // Get the number format of the last iterated cell.
                nFuncFmtIndex = aAction.getNumberFormat();
                nFuncFmtType = pDok->GetFormatTable()->GetType(nFuncFmtIndex);
            }
            break;
            case svExternalDoubleRef:
            {
                ScMatrixRef pMat;
                PopExternalDoubleRef(pMat);
                if (nGlobalError)
                    break;

                double fMem = 0.0, fRes = 0.0;
                bool bNull = true;
                IterateMatrix(pMat, ifCOUNT, false, nCount, nFuncFmtType, fRes, fMem, bNull);
            }
            break;
            case svMatrix :
            {
                ScMatrixRef pMat = PopMatrix();
                double fMem = 0.0, fRes = 0.0;
                bool bNull = true;
                IterateMatrix(pMat, ifCOUNT, false, nCount, nFuncFmtType, fRes, fMem, bNull);
            }
            break;
            case svError:
            {
                PopError();
                nGlobalError = 0;
            }
            break;
            default :
                while (nParamCount-- > 0)
                    PopError();
                SetError(errIllegalParameter);
        }
    }

    nFuncFmtType = NUMBERFORMAT_NUMBER;

    PushDouble(nCount);
}


void ScInterpreter::ScCount2()
{
    PushDouble( IterateParameters( ifCOUNT2 ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
