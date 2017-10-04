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

#include <interpre.hxx>
#include <columnspanset.hxx>
#include <column.hxx>
#include <document.hxx>
#include <cellvalue.hxx>
#include <dociter.hxx>
#include <mtvcellfunc.hxx>
#include <scmatrix.hxx>

#include <arraysumfunctor.hxx>

#include <formula/token.hxx>

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
    double fY = 1.0 - fA;
    double fDenom = fX + 2.0-fA;
    double fPkm1 = fX + 1.0;
    double fPkm2 = 1.0;
    double fQkm1 = fDenom * fX;
    double fQkm2 = fX;
    double fApprox = fPkm1/fQkm1;
    bool bFinished = false;
    do
    {
        fCount = fCount +1.0;
        fY = fY+ 1.0;
        const double fNum = fY * fCount;
        fDenom = fDenom +2.0;
        double fPk = fPkm1 * fDenom  -  fPkm2 * fNum;
        const double fQk = fQkm1 * fDenom  -  fQkm2 * fNum;
        if (fQk != 0.0)
        {
            const double fR = fPk/fQk;
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
        SetError(FormulaError::NoConvergence);
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
        SetError(FormulaError::NoConvergence);
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
            SetError(FormulaError::DivisionByZero);  // should be #DIV/0
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

class NumericCellAccumulator
{
    double mfFirst;
    double mfRest;
    FormulaError mnError;

public:
    NumericCellAccumulator() : mfFirst(0.0), mfRest(0.0), mnError(FormulaError::NONE) {}

    void operator() (const sc::CellStoreType::value_type& rNode, size_t nOffset, size_t nDataSize)
    {
        switch (rNode.type)
        {
            case sc::element_type_numeric:
            {
                const double *p = &sc::numeric_block::at(*rNode.data, nOffset);
                size_t i = 0;

                // Store the first non-zero value in mfFirst (for some reason).
                if (!mfFirst)
                {
                    for (i = 0; i < nDataSize; ++i)
                    {
                        if (!mfFirst)
                            mfFirst = p[i];
                        else
                            break;
                    }
                }
                p += i;
                nDataSize -= i;
                if (nDataSize == 0)
                    return;

                sc::ArraySumFunctor functor(p, nDataSize);

                mfRest += functor();
                break;
            }

            case sc::element_type_formula:
            {
                sc::formula_block::const_iterator it = sc::formula_block::begin(*rNode.data);
                std::advance(it, nOffset);
                sc::formula_block::const_iterator itEnd = it;
                std::advance(itEnd, nDataSize);
                for (; it != itEnd; ++it)
                {
                    double fVal = 0.0;
                    FormulaError nErr = FormulaError::NONE;
                    ScFormulaCell& rCell = *(*it);
                    if (!rCell.GetErrorOrValue(nErr, fVal))
                        // The cell has neither error nor value.  Perhaps string result.
                        continue;

                    if (nErr != FormulaError::NONE)
                    {
                        // Cell has error - skip all the rest
                        mnError = nErr;
                        return;
                    }

                    if ( !mfFirst )
                        mfFirst = fVal;
                    else
                        mfRest += fVal;
                }
            }
            break;
            default:
                ;
        }
    }

    FormulaError getError() const { return mnError; }
    double getFirst() const { return mfFirst; }
    double getRest() const { return mfRest; }
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
                    ScFormulaCell& rCell = **it;
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
    const ScInterpreterContext& mrContext;
    sc::ColumnBlockConstPosition maPos;
    ScColumn* mpCol;
    size_t mnCount;
    sal_uInt32 mnNumFmt;

public:
    FuncCount(const ScInterpreterContext& rContext) : mrContext(rContext), mpCol(nullptr), mnCount(0), mnNumFmt(0) {}

    virtual void startColumn(ScColumn* pCol) override
    {
        mpCol = pCol;
        mpCol->InitBlockPosition(maPos);
    }

    virtual void execute(SCROW nRow1, SCROW nRow2, bool bVal) override
    {
        if (!bVal)
            return;

        NumericCellCounter aFunc;
        maPos.miCellPos = sc::ParseBlock(maPos.miCellPos, mpCol->GetCellStore(), aFunc, nRow1, nRow2);
        mnCount += aFunc.getCount();
        mnNumFmt = mpCol->GetNumberFormat(mrContext, nRow2);
    };

    size_t getCount() const { return mnCount; }
    sal_uInt32 getNumberFormat() const { return mnNumFmt; }
};

class FuncSum : public sc::ColumnSpanSet::ColumnAction
{
    const ScInterpreterContext& mrContext;
    sc::ColumnBlockConstPosition maPos;
    ScColumn* mpCol;
    double mfSum;
    FormulaError mnError;
    sal_uInt32 mnNumFmt;

public:
    FuncSum(const ScInterpreterContext& rContext) : mrContext(rContext), mpCol(nullptr), mfSum(0.0), mnError(FormulaError::NONE), mnNumFmt(0) {}

    virtual void startColumn(ScColumn* pCol) override
    {
        mpCol = pCol;
        mpCol->InitBlockPosition(maPos);
    }

    virtual void execute(SCROW, SCROW, bool) override {}

    virtual void executeSum(SCROW nRow1, SCROW nRow2, bool bVal, double& fMem ) override
    {
        if (!bVal)
            return;

        if (mnError != FormulaError::NONE)
            return;

        NumericCellAccumulator aFunc;
        maPos.miCellPos = sc::ParseBlock(maPos.miCellPos, mpCol->GetCellStore(), aFunc, nRow1, nRow2);
        mnError = aFunc.getError();
        if (mnError != FormulaError::NONE)
            return;

        if ( fMem )
            mfSum += aFunc.getFirst() + aFunc.getRest();
        else
        {
            fMem = aFunc.getFirst();
            mfSum += aFunc.getRest();
        }

        mnNumFmt = mpCol->GetNumberFormat(mrContext, nRow2);
    };

    FormulaError getError() const { return mnError; }
    double getSum() const { return mfSum; }
    sal_uInt32 getNumberFormat() const { return mnNumFmt; }
};

void IterateMatrix(
    const ScMatrixRef& pMat, ScIterFunc eFunc, bool bTextAsZero,
    sal_uLong& rCount, short& rFuncFmtType, double& fRes, double& fMem )
{
    if (!pMat)
        return;

    // TODO fdo73148 take mnSubTotalFlags into account
    rFuncFmtType = css::util::NumberFormat::NUMBER;
    switch (eFunc)
    {
        case ifAVERAGE:
        case ifSUM:
        {
            ScMatrix::IterateResult aRes = pMat->Sum(bTextAsZero);
            // If the first value is a NaN, it probably means it was an empty cell,
            // and should be treated as zero.
            if ( !rtl::math::isFinite(aRes.mfFirst) )
            {
                sal_uInt32 nErr = reinterpret_cast< sal_math_Double * >(&aRes.mfFirst)->nan_parts.fraction_lo;
                if (nErr & 0xffff0000)
                {
                    aRes.mfFirst = 0;
                }
            }
            if ( fMem )
                fRes += aRes.mfFirst + aRes.mfRest;
            else
            {
                fMem = aRes.mfFirst;
                fRes += aRes.mfRest;
            }
            rCount += aRes.mnCount;
        }
        break;
        case ifCOUNT:
            rCount += pMat->Count(bTextAsZero, false);  // do not count error values
        break;
        case ifCOUNT2:
            rCount += pMat->Count(true, true);          // do count error values
        break;
        case ifPRODUCT:
        {
            ScMatrix::IterateResult aRes = pMat->Product(bTextAsZero);
            fRes *= aRes.mfFirst;
            fRes *= aRes.mfRest;
            rCount += aRes.mnCount;
        }
        break;
        case ifSUMSQ:
        {
            ScMatrix::IterateResult aRes = pMat->SumSquare(bTextAsZero);
            fRes += aRes.mfFirst;
            fRes += aRes.mfRest;
            rCount += aRes.mnCount;
        }
        break;
        default:
            ;
    }
}

size_t ScInterpreter::GetRefListArrayMaxSize( short nParamCount )
{
    size_t nSize = 0;
    if (bMatrixFormula || pCur->IsInForceArray())
    {
        for (short i=1; i <= nParamCount; ++i)
        {
            if (GetStackType(i) == svRefList)
            {
                const ScRefListToken* p = dynamic_cast<const ScRefListToken*>(pStack[sp - i]);
                if (p && p->IsArrayResult() && p->GetRefList()->size() > nSize)
                    nSize = p->GetRefList()->size();
            }
        }
    }
    return nSize;
}

static double lcl_IterResult( ScIterFunc eFunc, double fRes, double fMem, sal_uLong nCount )
{
    switch( eFunc )
    {
        case ifSUM:
            fRes = ::rtl::math::approxAdd( fRes, fMem );
        break;
        case ifAVERAGE:
            fRes = sc::div( ::rtl::math::approxAdd( fRes, fMem ), nCount);
        break;
        case ifCOUNT2:
        case ifCOUNT:
            fRes = nCount;
        break;
        case ifPRODUCT:
            if ( !nCount )
                fRes = 0.0;
        break;
        default:
            ; // nothing
    }
    return fRes;
}

void ScInterpreter::IterateParameters( ScIterFunc eFunc, bool bTextAsZero )
{
    short nParamCount = GetByte();
    const SCSIZE nMatRows = GetRefListArrayMaxSize( nParamCount);
    ScMatrixRef xResMat, xResCount;
    auto ResInitVal = [eFunc]()
    {
        return (eFunc == ifPRODUCT) ? 1.0 : 0.0;
    };
    double fRes = ResInitVal();
    double fVal = 0.0;
    double fMem = 0.0;  // first numeric value != 0.0
    sal_uLong nCount = 0;
    ScAddress aAdr;
    ScRange aRange;
    size_t nRefInList = 0;
    size_t nRefArrayPos = std::numeric_limits<size_t>::max();
    if ( nGlobalError != FormulaError::NONE && ( eFunc == ifCOUNT2 || eFunc == ifCOUNT ||
         ( mnSubTotalFlags & SubtotalFlags::IgnoreErrVal ) ) )
        nGlobalError = FormulaError::NONE;
    while (nParamCount-- > 0)
    {
        switch (GetStackType())
        {
            case svString:
            {
                if( eFunc == ifCOUNT )
                {
                    OUString aStr = PopString().getString();
                    if ( bTextAsZero )
                        nCount++;
                    else
                    {
                        // Only check if string can be converted to number, no
                        // error propagation.
                        FormulaError nErr = nGlobalError;
                        nGlobalError = FormulaError::NONE;
                        ConvertStringToValue( aStr );
                        if (nGlobalError == FormulaError::NONE)
                            ++nCount;
                        nGlobalError = nErr;
                    }
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
                                SetError( FormulaError::NoValue );
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
                        if ( fMem )
                            fRes += fVal;
                        else
                            fMem = fVal;
                        break;
                    case ifSUMSQ:   fRes += fVal * fVal; break;
                    case ifPRODUCT: fRes *= fVal; break;
                    default: ; // nothing
                }
                nFuncFmtType = css::util::NumberFormat::NUMBER;
                break;
            case svExternalSingleRef:
            {
                ScExternalRefCache::TokenRef pToken;
                ScExternalRefCache::CellFormat aFmt;
                PopExternalSingleRef(pToken, &aFmt);
                if ( nGlobalError != FormulaError::NONE && ( eFunc == ifCOUNT2 || eFunc == ifCOUNT ||
                     ( mnSubTotalFlags & SubtotalFlags::IgnoreErrVal ) ) )
                {
                    nGlobalError = FormulaError::NONE;
                    if ( eFunc == ifCOUNT2 && !( mnSubTotalFlags & SubtotalFlags::IgnoreErrVal ) )
                        ++nCount;
                    break;
                }

                if (!pToken)
                    break;

                StackVar eType = pToken->GetType();
                if (eFunc == ifCOUNT2)
                {
                    if ( eType != svEmptyCell &&
                         ( ( pToken->GetOpCode() != ocSubTotal &&
                             pToken->GetOpCode() != ocAggregate ) ||
                           ( mnSubTotalFlags & SubtotalFlags::IgnoreNestedStAg ) ) )
                        nCount++;
                    if (nGlobalError != FormulaError::NONE)
                        nGlobalError = FormulaError::NONE;
                }
                else if (eType == svDouble)
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
                            if ( fMem )
                                fRes += fVal;
                            else
                                fMem = fVal;
                            break;
                        case ifSUMSQ:   fRes += fVal * fVal; break;
                        case ifPRODUCT: fRes *= fVal; break;
                        case ifCOUNT:
                            if ( nGlobalError != FormulaError::NONE )
                            {
                                nGlobalError = FormulaError::NONE;
                                nCount--;
                            }
                            break;
                        default: ; // nothing
                    }
                }
                else if (bTextAsZero && eType == svString)
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
                if (nGlobalError == FormulaError::NoRef)
                {
                    PushError( FormulaError::NoRef);
                    return;
                }

                if ( nGlobalError != FormulaError::NONE && ( eFunc == ifCOUNT2 || eFunc == ifCOUNT ||
                     ( mnSubTotalFlags & SubtotalFlags::IgnoreErrVal ) ) )
                {
                    nGlobalError = FormulaError::NONE;
                    if ( eFunc == ifCOUNT2 && !( mnSubTotalFlags & SubtotalFlags::IgnoreErrVal ) )
                        ++nCount;
                    break;
                }
                if ( ( mnSubTotalFlags & SubtotalFlags::IgnoreFiltered ) &&
                     pDok->RowFiltered( aAdr.Row(), aAdr.Tab() ) )
                {
                    break;
                }
                ScRefCellValue aCell(*pDok, aAdr);
                if (!aCell.isEmpty())
                {
                    if( eFunc == ifCOUNT2 )
                    {
                        CellType eCellType = aCell.meType;
                        if ( eCellType != CELLTYPE_NONE )
                            nCount++;
                        if ( nGlobalError != FormulaError::NONE )
                            nGlobalError = FormulaError::NONE;
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
                                if ( fMem )
                                    fRes += fVal;
                                else
                                    fMem = fVal;
                                break;
                            case ifSUMSQ:   fRes += fVal * fVal; break;
                            case ifPRODUCT: fRes *= fVal; break;
                            case ifCOUNT:
                                if ( nGlobalError != FormulaError::NONE )
                                {
                                    nGlobalError = FormulaError::NONE;
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
            case svRefList :
            {
                const ScRefListToken* p = dynamic_cast<const ScRefListToken*>(pStack[sp-1]);
                if (p && p->IsArrayResult())
                {
                    nRefArrayPos = nRefInList;
                    if ((eFunc == ifSUM || eFunc == ifAVERAGE) && fMem != 0.0)
                    {
                        fRes = rtl::math::approxAdd( fRes, fMem);
                        fMem = 0.0;
                    }
                    // The "one value to all references of an array" seems to
                    // be what Excel does if there are other types than just
                    // arrays of references.
                    if (!xResMat)
                    {
                        // Create and init all elements with current value.
                        assert(nMatRows > 0);
                        xResMat = GetNewMat( 1, nMatRows, true);
                        xResMat->FillDouble( fRes, 0,0, 0,nMatRows-1);
                        if (eFunc != ifSUM)
                        {
                            xResCount = GetNewMat( 1, nMatRows, true);
                            xResCount->FillDouble( nCount, 0,0, 0,nMatRows-1);
                        }
                    }
                    else
                    {
                        // Current value and values from vector are operands
                        // for each vector position.
                        if (nCount && xResCount)
                        {
                            for (SCSIZE i=0; i < nMatRows; ++i)
                            {
                                xResCount->PutDouble( xResCount->GetDouble(0,i) + nCount, 0,i);
                            }
                        }
                        if (fRes != ResInitVal())
                        {
                            for (SCSIZE i=0; i < nMatRows; ++i)
                            {
                                double fVecRes = xResMat->GetDouble(0,i);
                                if (eFunc == ifPRODUCT)
                                    fVecRes *= fRes;
                                else
                                    fVecRes += fRes;
                                xResMat->PutDouble( fVecRes, 0,i);
                            }
                        }
                    }
                    fRes = ResInitVal();
                    nCount = 0;
                }
            }
            SAL_FALLTHROUGH;
            case svDoubleRef :
            {
                PopDoubleRef( aRange, nParamCount, nRefInList);
                if (nGlobalError == FormulaError::NoRef)
                {
                    PushError( FormulaError::NoRef);
                    return;
                }

                if ( nGlobalError != FormulaError::NONE && ( eFunc == ifCOUNT2 || eFunc == ifCOUNT ||
                     ( mnSubTotalFlags & SubtotalFlags::IgnoreErrVal ) ) )
                {
                    nGlobalError = FormulaError::NONE;
                    if ( eFunc == ifCOUNT2 && !( mnSubTotalFlags & SubtotalFlags::IgnoreErrVal ) )
                        ++nCount;
                    if ( eFunc == ifCOUNT2 || eFunc == ifCOUNT )
                        break;
                }
                if( eFunc == ifCOUNT2 )
                {
                    ScCellIterator aIter( pDok, aRange, mnSubTotalFlags );
                    for (bool bHas = aIter.first(); bHas; bHas = aIter.next())
                    {
                        if ( !aIter.hasEmptyData() )
                        {
                            ++nCount;
                        }
                    }

                    if ( nGlobalError != FormulaError::NONE )
                        nGlobalError = FormulaError::NONE;
                }
                else if ( ( eFunc == ifSUM || eFunc == ifCOUNT ) && mnSubTotalFlags == SubtotalFlags::NONE )
                {
                    sc::ColumnSpanSet aSet( false );
                    aSet.set( aRange, true );

                    if ( eFunc == ifSUM )
                    {
                        FuncSum aAction(mrContext);
                        aSet.executeColumnAction( *pDok, aAction, fMem );
                        FormulaError nErr = aAction.getError();
                        if ( nErr != FormulaError::NONE )
                        {
                            PushError( nErr );
                            return;
                        }
                        fRes += aAction.getSum();

                        // Get the number format of the last iterated cell.
                        nFuncFmtIndex = aAction.getNumberFormat();
                    }
                    else
                    {
                        FuncCount aAction(mrContext);
                        aSet.executeColumnAction(*pDok, aAction);
                        nCount += aAction.getCount();

                        // Get the number format of the last iterated cell.
                        nFuncFmtIndex = aAction.getNumberFormat();
                    }

                    nFuncFmtType = mrContext.GetFormatTable()->GetType( nFuncFmtIndex );
                }
                else
                {
                    ScValueIterator aValIter( pDok, aRange, mnSubTotalFlags, bTextAsZero );
                    FormulaError nErr = FormulaError::NONE;
                    if (aValIter.GetFirst(fVal, nErr))
                    {
                        // placed the loop on the inside for performance reasons:
                        aValIter.GetCurNumFmtInfo( mrContext, nFuncFmtType, nFuncFmtIndex );
                        switch( eFunc )
                        {
                            case ifAVERAGE:
                            case ifSUM:
                                    if ( mnSubTotalFlags & SubtotalFlags::IgnoreErrVal )
                                    {
                                        do
                                        {
                                            if ( nErr == FormulaError::NONE )
                                            {
                                                SetError(nErr);
                                                if ( fMem )
                                                    fRes += fVal;
                                                else
                                                    fMem = fVal;
                                                nCount++;
                                            }
                                        }
                                        while (aValIter.GetNext(fVal, nErr));
                                    }
                                    else
                                    {
                                        do
                                        {
                                            SetError(nErr);
                                            if ( fMem )
                                                fRes += fVal;
                                            else
                                                fMem = fVal;
                                            nCount++;
                                        }
                                        while (aValIter.GetNext(fVal, nErr));
                                    }
                                    break;
                            case ifSUMSQ:
                                    if ( mnSubTotalFlags & SubtotalFlags::IgnoreErrVal )
                                    {
                                        do
                                        {
                                            if ( nErr == FormulaError::NONE )
                                            {
                                                SetError(nErr);
                                                fRes += fVal * fVal;
                                                nCount++;
                                            }
                                        }
                                        while (aValIter.GetNext(fVal, nErr));
                                    }
                                    else
                                    {
                                        do
                                        {
                                            SetError(nErr);
                                            fRes += fVal * fVal;
                                            nCount++;
                                        }
                                        while (aValIter.GetNext(fVal, nErr));
                                    }
                                    break;
                            case ifPRODUCT:
                                    do
                                    {
                                        if ( !( nErr != FormulaError::NONE && ( mnSubTotalFlags & SubtotalFlags::IgnoreErrVal ) ) )
                                        {
                                            SetError(nErr);
                                            fRes *= fVal;
                                            nCount++;
                                        }
                                    }
                                    while (aValIter.GetNext(fVal, nErr));
                                    break;
                            case ifCOUNT:
                                    do
                                    {
                                        if ( nErr == FormulaError::NONE )
                                            nCount++;
                                    }
                                    while (aValIter.GetNext(fVal, nErr));
                                    break;
                            default: ;  // nothing
                        }
                        SetError( nErr );
                    }
                }
                if (nRefArrayPos != std::numeric_limits<size_t>::max())
                {
                    // Update vector element with current value.
                    if ((eFunc == ifSUM || eFunc == ifAVERAGE) && fMem != 0.0)
                    {
                        fRes = rtl::math::approxAdd( fRes, fMem);
                        fMem = 0.0;
                    }
                    if (xResCount)
                        xResCount->PutDouble( xResCount->GetDouble(0,nRefArrayPos) + nCount, 0,nRefArrayPos);
                    double fVecRes = xResMat->GetDouble(0,nRefArrayPos);
                    if (eFunc == ifPRODUCT)
                        fVecRes *= fRes;
                    else
                        fVecRes += fRes;
                    xResMat->PutDouble( fVecRes, 0,nRefArrayPos);
                    // Reset.
                    fRes = ResInitVal();
                    nCount = 0;
                    nRefArrayPos = std::numeric_limits<size_t>::max();
                }
            }
            break;
            case svExternalDoubleRef:
            {
                ScMatrixRef pMat;
                PopExternalDoubleRef(pMat);
                if ( nGlobalError != FormulaError::NONE && !( mnSubTotalFlags & SubtotalFlags::IgnoreErrVal ) )
                    break;

                IterateMatrix( pMat, eFunc, bTextAsZero, nCount, nFuncFmtType, fRes, fMem );
            }
            break;
            case svMatrix :
            {
                ScMatrixRef pMat = PopMatrix();

                IterateMatrix( pMat, eFunc, bTextAsZero, nCount, nFuncFmtType, fRes, fMem );
            }
            break;
            case svError:
            {
                PopError();
                if ( eFunc == ifCOUNT || ( mnSubTotalFlags & SubtotalFlags::IgnoreErrVal ) )
                {
                    nGlobalError = FormulaError::NONE;
                }
                else if ( eFunc == ifCOUNT2 && !( mnSubTotalFlags & SubtotalFlags::IgnoreErrVal ) )
                {
                    nCount++;
                    nGlobalError = FormulaError::NONE;
                }
            }
            break;
            default :
                while (nParamCount-- > 0)
                    PopError();
                SetError(FormulaError::IllegalParameter);
        }
    }

    // A boolean return type makes no sense on sums et al.
    // Counts are always numbers.
    if( nFuncFmtType == css::util::NumberFormat::LOGICAL || eFunc == ifCOUNT || eFunc == ifCOUNT2 )
        nFuncFmtType = css::util::NumberFormat::NUMBER;

    if (xResMat)
    {
        // Include value of last non-references-array type and calculate final result.
        for (SCSIZE i=0; i < nMatRows; ++i)
        {
            sal_uLong nVecCount = (xResCount ? nCount + xResCount->GetDouble(0,i) : nCount);
            double fVecRes = xResMat->GetDouble(0,i);
            if (eFunc == ifPRODUCT)
                fVecRes *= fRes;
            else
                fVecRes += fRes;
            fVecRes = lcl_IterResult( eFunc, fVecRes, fMem, nVecCount);
            xResMat->PutDouble( fVecRes, 0,i);
        }
        PushMatrix( xResMat);
    }
    else
    {
        PushDouble( lcl_IterResult( eFunc, fRes, fMem, nCount));
    }
}

void ScInterpreter::ScSumSQ()
{
    IterateParameters( ifSUMSQ );
}

void ScInterpreter::ScSum()
{
    IterateParameters( ifSUM );
}

void ScInterpreter::ScProduct()
{
    IterateParameters( ifPRODUCT );
}

void ScInterpreter::ScAverage( bool bTextAsZero )
{
    IterateParameters( ifAVERAGE, bTextAsZero );
}

void ScInterpreter::ScCount()
{
    IterateParameters( ifCOUNT );
}

void ScInterpreter::ScCount2()
{
    IterateParameters( ifCOUNT2 );
}

void ScInterpreter::ScRawSubtract()
{
    short nParamCount = GetByte();
    if (!MustHaveParamCountMin( nParamCount, 2))
        return;

    // Fish the 1st parameter from the stack and push it on top.
    const FormulaToken* p = pStack[ sp - nParamCount ];
    PushWithoutError( *p );
    // Obtain the minuend.
    double fRes = GetDouble();

    while (nGlobalError == FormulaError::NONE && nParamCount-- > 1)
    {
        // Simple single values without matrix support.
        fRes -= GetDouble();
    }
    while (nParamCount-- > 0)
        PopError();

    PushDouble( fRes);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
