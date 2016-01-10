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

#include <svl/intitem.hxx>
#include <svl/zforlist.hxx>
#include <formula/token.hxx>

#include "document.hxx"
#include "table.hxx"
#include "globstr.hrc"
#include "subtotal.hxx"
#include "docoptio.hxx"
#include "interpre.hxx"
#include "markdata.hxx"
#include "validat.hxx"
#include "scitems.hxx"
#include "stlpool.hxx"
#include "poolhelp.hxx"
#include "detdata.hxx"
#include "patattr.hxx"
#include "chgtrack.hxx"
#include "progress.hxx"
#include "paramisc.hxx"
#include "compiler.hxx"
#include "externalrefmgr.hxx"
#include "colorscale.hxx"
#include "attrib.hxx"
#include "formulacell.hxx"
#include "tokenarray.hxx"
#include "scmatrix.hxx"
#include <tokenstringcontext.hxx>
#include <memory>

using namespace formula;

/** (Goal Seek) Find a value of x that is a root of f(x)

    This function is used internally for the goal seek operation.  It uses the
    Regula Falsi (aka false position) algorithm to find a root of f(x).  The
    start value and the target value are to be given by the user in the
    goal seek dialog.  The f(x) in this case is defined as the formula in the
    formula cell minus target value.  This function may also perform additional
    search in the horizontal directions when the f(x) is discrete in order to
    ensure a non-zero slope necessary for deriving a subsequent x that is
    reasonably close to the root of interest.

    @change 24.10.2004 by Kohei Yoshida (kohei@openoffice.org)

    @see #i28955#

    @change 6 Aug 2013, fdo37341
*/
bool ScDocument::Solver(SCCOL nFCol, SCROW nFRow, SCTAB nFTab,
                        SCCOL nVCol, SCROW nVRow, SCTAB nVTab,
                        const OUString& sValStr, double& nX)
{
    bool bRet = false;
    nX = 0.0;
    if ( ValidColRow( nFCol, nFRow ) && ValidTab( nFTab ) &&
         ValidColRow( nVCol, nVRow ) && ValidTab( nVTab ) &&
         nFTab < static_cast<SCTAB>( maTabs.size() ) && maTabs[nFTab] &&
         nVTab < static_cast<SCTAB>( maTabs.size() ) && maTabs[nVTab] )
    {
        CellType eFType, eVType;
        GetCellType(nFCol, nFRow, nFTab, eFType);
        GetCellType(nVCol, nVRow, nVTab, eVType);
        // #i108005# convert target value to number using default format,
        // as previously done in ScInterpreter::GetDouble
        ScFormulaCell* pFormula = nullptr;
        double fTargetVal = 0.0;
        sal_uInt32 nFIndex = 0;
        if ( eFType == CELLTYPE_FORMULA && eVType == CELLTYPE_VALUE &&
             GetFormatTable()->IsNumberFormat( sValStr, nFIndex, fTargetVal ) )
        {
            ScAddress aFormulaAdr( nFCol, nFRow, nFTab );
            pFormula = GetFormulaCell( aFormulaAdr );
        }
        if (pFormula)
        {
            bool bDoneIteration = false;
            ScAddress aValueAdr( nVCol, nVRow, nVTab );
            double* pVCell = GetValueCell( aValueAdr );

            ScRange aVRange( aValueAdr, aValueAdr );    // for SetDirty
            // Original value to be restored later if necessary
            double fSaveVal = *pVCell;

            const sal_uInt16 nMaxIter = 100;
            const double fEps = 1E-10;
            const double fDelta = 1E-6;

            double fBestX, fXPrev;
            double fBestF, fFPrev;
            fBestX = fXPrev = fSaveVal;

            pFormula->Interpret();
            bool bError = ( pFormula->GetErrCode() != 0 );
            // bError always corresponds with fF

            fFPrev = pFormula->GetValue() - fTargetVal;

            fBestF = fabs( fFPrev );
            if ( fBestF < fDelta )
                bDoneIteration = true;

            double fX = fXPrev + fEps;
            double fF = fFPrev;
            double fSlope;

            sal_uInt16 nIter = 0;

            bool bHorMoveError = false;
            // Conform Regula Falsi Method
            while ( !bDoneIteration && ( nIter++ < nMaxIter ) )
            {
                *pVCell = fX;
                SetDirty( aVRange, false );
                pFormula->Interpret();
                bError = ( pFormula->GetErrCode() != 0 );
                fF = pFormula->GetValue() - fTargetVal;

                if ( fF == fFPrev && !bError )
                {
                    // HORIZONTAL SEARCH: Keep moving x in both directions until the f(x)
                    // becomes different from the previous f(x).  This routine is needed
                    // when a given function is discrete, in which case the resulting slope
                    // may become zero which ultimately causes the goal seek operation
                    // to fail. #i28955#

                    sal_uInt16 nHorIter = 0;
                    const double fHorStepAngle = 5.0;
                    const double fHorMaxAngle = 80.0;
                    int nHorMaxIter = static_cast<int>( fHorMaxAngle / fHorStepAngle );
                    bool bDoneHorMove = false;

                    while ( !bDoneHorMove && !bHorMoveError && nHorIter++ < nHorMaxIter )
                    {
                        double fHorAngle = fHorStepAngle * static_cast<double>( nHorIter );
                        double fHorTangent = ::rtl::math::tan( fHorAngle * F_PI / 180 );

                        sal_uInt16 nIdx = 0;
                        while( nIdx++ < 2 && !bDoneHorMove )
                        {
                            double fHorX;
                            if ( nIdx == 1 )
                                fHorX = fX + fabs( fF ) * fHorTangent;
                            else
                                fHorX = fX - fabs( fF ) * fHorTangent;

                            *pVCell = fHorX;
                            SetDirty( aVRange, false );
                            pFormula->Interpret();
                            bHorMoveError = ( pFormula->GetErrCode() != 0 );
                            if ( bHorMoveError )
                                break;

                            fF = pFormula->GetValue() - fTargetVal;
                            if ( fF != fFPrev )
                            {
                                fX = fHorX;
                                bDoneHorMove = true;
                            }
                        }
                    }
                    if ( !bDoneHorMove )
                        bHorMoveError = true;
                }

                if ( bError )
                {
                    // move closer to last valid value (fXPrev), keep fXPrev & fFPrev
                    double fDiff = ( fXPrev - fX ) / 2;
                    if ( fabs( fDiff ) < fEps )
                        fDiff = ( fDiff < 0.0 ? - fEps : fEps );
                    fX += fDiff;
                }
                else if ( bHorMoveError )
                    break;
                else if ( fabs(fF) < fDelta )
                {
                    // converged to root
                    fBestX = fX;
                    bDoneIteration = true;
                }
                else
                {
                    if ( fabs(fF) + fDelta < fBestF )
                    {
                        fBestX = fX;
                        fBestF = fabs( fF );
                    }

                    if ( ( fXPrev - fX ) != 0 )
                    {
                        fSlope = ( fFPrev - fF ) / ( fXPrev - fX );
                        if ( fabs( fSlope ) < fEps )
                            fSlope = fSlope < 0.0 ? -fEps : fEps;
                    }
                    else
                        fSlope = fEps;

                    fXPrev = fX;
                    fFPrev = fF;
                    fX = fX - ( fF / fSlope );
                }
            }

            // Try a nice rounded input value if possible.
            const double fNiceDelta = ( bDoneIteration && fabs( fBestX ) >= 1e-3 ? 1e-3 : fDelta );
            nX = ::rtl::math::approxFloor( ( fBestX / fNiceDelta ) + 0.5 ) * fNiceDelta;

            if ( bDoneIteration )
            {
                *pVCell = nX;
                SetDirty( aVRange, false );
                pFormula->Interpret();
                if ( fabs( pFormula->GetValue() - fTargetVal ) > fabs( fF ) )
                    nX = fBestX;
                bRet = true;
            }
            else if ( bError || bHorMoveError )
            {
                nX = fBestX;
            }
            *pVCell = fSaveVal;
            SetDirty( aVRange, false );
            pFormula->Interpret();
            if ( !bDoneIteration )
            {
                SetError( nVCol, nVRow, nVTab, NOTAVAILABLE );
            }
        }
        else
        {
            SetError( nVCol, nVRow, nVTab, NOTAVAILABLE );
        }
    }
    return bRet;
}

void ScDocument::InsertMatrixFormula(SCCOL nCol1, SCROW nRow1,
                                     SCCOL nCol2, SCROW nRow2,
                                     const ScMarkData& rMark,
                                     const OUString& rFormula,
                                     const ScTokenArray* pArr,
                                     const formula::FormulaGrammar::Grammar eGram,
                                     bool bDirtyFlag )
{
    PutInOrder(nCol1, nCol2);
    PutInOrder(nRow1, nRow2);
    nCol2 = std::min<SCCOL>(nCol2, MAXCOL);
    nRow2 = std::min<SCROW>(nRow2, MAXROW);
    if (!rMark.GetSelectCount())
    {
        SAL_WARN("sc", "ScDocument::InsertMatrixFormula: No table marked");
        return;
    }

    SCTAB nTab1 = *rMark.begin();

    ScFormulaCell* pCell;
    ScAddress aPos( nCol1, nRow1, nTab1 );
    if (pArr)
        pCell = new ScFormulaCell(this, aPos, *pArr, eGram, MM_FORMULA);
    else
        pCell = new ScFormulaCell( this, aPos, rFormula, eGram, MM_FORMULA );
    pCell->SetMatColsRows( nCol2 - nCol1 + 1, nRow2 - nRow1 + 1, bDirtyFlag );
    ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
    SCTAB nMax = static_cast<SCTAB>(maTabs.size());
    for (; itr != itrEnd && *itr < nMax; ++itr)
    {
        if (!maTabs[*itr])
            continue;

        if (*itr == nTab1)
        {
            pCell = maTabs[*itr]->SetFormulaCell(nCol1, nRow1, pCell);
            if (!pCell) //NULL if nCol1/nRow1 is invalid, which it can't be here
                break;
        }
        else
            maTabs[*itr]->SetFormulaCell(
                nCol1, nRow1,
                new ScFormulaCell(
                    *pCell, *this, ScAddress(nCol1, nRow1, *itr), SC_CLONECELL_STARTLISTENING));
    }

    ScAddress aBasePos(nCol1, nRow1, nTab1);
    ScSingleRefData aRefData;
    aRefData.InitFlags();
    aRefData.SetColRel( true );
    aRefData.SetRowRel( true );
    aRefData.SetTabRel( true );
    aRefData.SetAddress(aBasePos, aBasePos);

    ScTokenArray aArr; // consists only of one single reference token.
    formula::FormulaToken* t = aArr.AddMatrixSingleReference( aRefData);

    itr = rMark.begin();
    for (; itr != itrEnd && *itr < nMax; ++itr)
    {
        SCTAB nTab = *itr;
        ScTable* pTab = FetchTable(nTab);
        if (!pTab)
            continue;

        if (nTab != nTab1)
        {
            aRefData.SetRelTab(nTab - aBasePos.Tab());
            *t->GetSingleRef() = aRefData;
        }

        for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
        {
            for (SCROW nRow = nRow1; nRow <= nRow2; ++nRow)
            {
                if (nCol == nCol1 && nRow == nRow1)
                    // Skip the base position.
                    continue;

                // Token array must be cloned so that each formula cell receives its own copy.
                aPos = ScAddress(nCol, nRow, nTab);
                // Reference in each cell must point to the origin cell relative to the current cell.
                aRefData.SetAddress(aBasePos, aPos);
                *t->GetSingleRef() = aRefData;
                std::unique_ptr<ScTokenArray> pTokArr(aArr.Clone());
                pCell = new ScFormulaCell(this, aPos, *pTokArr, eGram, MM_REFERENCE);
                pTab->SetFormulaCell(nCol, nRow, pCell);
            }
        }
    }
}

void ScDocument::InsertTableOp(const ScTabOpParam& rParam,  // multiple (repeated?) operation
                               SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                               const ScMarkData& rMark)
{
    PutInOrder(nCol1, nCol2);
    PutInOrder(nRow1, nRow2);
    SCTAB i, nTab1;
    SCCOL j;
    SCROW k;
    i = 0;
    bool bStop = false;
    SCTAB nMax = static_cast<SCTAB>(maTabs.size());
    ScMarkData::const_iterator itr = rMark.begin(), itrEnd = rMark.end();
    for (; itr != itrEnd && *itr < nMax; ++itr)
    {
        if (maTabs[*itr])
        {
            i = *itr;
            bStop = true;
            break;
        }
    }
    nTab1 = i;
    if (!bStop)
    {
        OSL_FAIL("ScDocument::InsertTableOp: No table marked");
        return;
    }

    ScRefAddress aRef;
    OUStringBuffer aForString;
    aForString.append('=');
    aForString.append(ScCompiler::GetNativeSymbol(ocTableOp));
    aForString.append(ScCompiler::GetNativeSymbol( ocOpen));

    const OUString& sSep = ScCompiler::GetNativeSymbol( ocSep);
    if (rParam.meMode == ScTabOpParam::Column) // column only
    {
        aRef.Set( rParam.aRefFormulaCell.GetAddress(), true, false, false );
        aForString.append(aRef.GetRefString(this, nTab1));
        aForString.append(sSep);
        aForString.append(rParam.aRefColCell.GetRefString(this, nTab1));
        aForString.append(sSep);
        aRef.Set( nCol1, nRow1, nTab1, false, true, true );
        aForString.append(aRef.GetRefString(this, nTab1));
        nCol1++;
        nCol2 = std::min( nCol2, (SCCOL)(rParam.aRefFormulaEnd.Col() -
                    rParam.aRefFormulaCell.Col() + nCol1 + 1));
    }
    else if (rParam.meMode == ScTabOpParam::Row) // row only
    {
        aRef.Set( rParam.aRefFormulaCell.GetAddress(), false, true, false );
        aForString.append(aRef.GetRefString(this, nTab1));
        aForString.append(sSep);
        aForString.append(rParam.aRefRowCell.GetRefString(this, nTab1));
        aForString.append(sSep);
        aRef.Set( nCol1, nRow1, nTab1, true, false, true );
        aForString.append(aRef.GetRefString(this, nTab1));
        nRow1++;
        nRow2 = std::min( nRow2, (SCROW)(rParam.aRefFormulaEnd.Row() -
                    rParam.aRefFormulaCell.Row() + nRow1 + 1));
    }
    else // both
    {
        aForString.append(rParam.aRefFormulaCell.GetRefString(this, nTab1));
        aForString.append(sSep);
        aForString.append(rParam.aRefColCell.GetRefString(this, nTab1));
        aForString.append(sSep);
        aRef.Set( nCol1, nRow1 + 1, nTab1, false, true, true );
        aForString.append(aRef.GetRefString(this, nTab1));
        aForString.append(sSep);
        aForString.append(rParam.aRefRowCell.GetRefString(this, nTab1));
        aForString.append(sSep);
        aRef.Set( nCol1 + 1, nRow1, nTab1, true, false, true );
        aForString.append(aRef.GetRefString(this, nTab1));
        nCol1++; nRow1++;
    }
    aForString.append(ScCompiler::GetNativeSymbol( ocClose ));

    ScFormulaCell aRefCell( this, ScAddress( nCol1, nRow1, nTab1 ), aForString.makeStringAndClear(),
           formula::FormulaGrammar::GRAM_NATIVE, MM_NONE );
    for( j = nCol1; j <= nCol2; j++ )
        for( k = nRow1; k <= nRow2; k++ )
            for (i = 0; i < static_cast<SCTAB>(maTabs.size()); i++)
            {
                itr = rMark.begin();
                for (; itr != itrEnd && *itr < nMax; ++itr)
                if( maTabs[*itr] )
                    maTabs[*itr]->SetFormulaCell(
                        j, k, new ScFormulaCell(aRefCell, *this, ScAddress(j, k, *itr), SC_CLONECELL_STARTLISTENING));
            }
}

namespace {

bool setCacheTableReferenced(formula::FormulaToken& rToken, ScExternalRefManager& rRefMgr, const ScAddress& rPos)
{
    switch (rToken.GetType())
    {
        case svExternalSingleRef:
            return rRefMgr.setCacheTableReferenced(
                rToken.GetIndex(), rToken.GetString().getString(), 1);
        case svExternalDoubleRef:
        {
            const ScComplexRefData& rRef = *rToken.GetDoubleRef();
            ScRange aAbs = rRef.toAbs(rPos);
            size_t nSheets = aAbs.aEnd.Tab() - aAbs.aStart.Tab() + 1;
            return rRefMgr.setCacheTableReferenced(
                    rToken.GetIndex(), rToken.GetString().getString(), nSheets);
        }
        case svExternalName:
            /* TODO: external names aren't supported yet, but would
             * have to be marked as well, if so. Mechanism would be
             * different. */
            OSL_FAIL("ScDocument::MarkUsedExternalReferences: implement the svExternalName case!");
        default:
            ;
    }
    return false;
}

}

bool ScDocument::MarkUsedExternalReferences( ScTokenArray& rArr, const ScAddress& rPos )
{
    if (!rArr.GetLen())
        return false;

    ScExternalRefManager* pRefMgr = nullptr;
    rArr.Reset();
    formula::FormulaToken* t = nullptr;
    bool bAllMarked = false;
    while (!bAllMarked && (t = rArr.GetNextReferenceOrName()) != nullptr)
    {
        if (t->IsExternalRef())
        {
            if (!pRefMgr)
                pRefMgr = GetExternalRefManager();

            bAllMarked = setCacheTableReferenced(*t, *pRefMgr, rPos);
        }
        else if (t->GetType() == svIndex)
        {
            // this is a named range.  Check if the range contains an external
            // reference.
            ScRangeData* pRangeData = GetRangeName()->findByIndex(t->GetIndex());
            if (!pRangeData)
                continue;

            ScTokenArray* pArray = pRangeData->GetCode();
            for (t = pArray->First(); t; t = pArray->Next())
            {
                if (!t->IsExternalRef())
                    continue;

                if (!pRefMgr)
                    pRefMgr = GetExternalRefManager();

                bAllMarked = setCacheTableReferenced(*t, *pRefMgr, rPos);
            }
        }
    }
    return bAllMarked;
}

bool ScDocument::GetNextSpellingCell(SCCOL& nCol, SCROW& nRow, SCTAB nTab,
                        bool bInSel, const ScMarkData& rMark) const
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        return maTabs[nTab]->GetNextSpellingCell( nCol, nRow, bInSel, rMark );
    else
        return false;
}

bool ScDocument::GetNextMarkedCell( SCCOL& rCol, SCROW& rRow, SCTAB nTab,
                                        const ScMarkData& rMark )
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        return maTabs[nTab]->GetNextMarkedCell( rCol, rRow, rMark );
    else
        return false;
}

bool ScDocument::ReplaceStyle(const SvxSearchItem& rSearchItem,
                              SCCOL nCol, SCROW nRow, SCTAB nTab,
                              ScMarkData& rMark,
                              bool bIsUndoP)
{
    if (nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        return maTabs[nTab]->ReplaceStyle(rSearchItem, nCol, nRow, rMark, bIsUndoP);
    else
        return false;
}

void ScDocument::CompileDBFormula()
{
    sc::CompileFormulaContext aCxt(this);
    TableContainer::iterator it = maTabs.begin();
    for (;it != maTabs.end(); ++it)
    {
        if (*it)
            (*it)->CompileDBFormula(aCxt);
    }
}

void ScDocument::CompileColRowNameFormula()
{
    sc::CompileFormulaContext aCxt(this);
    TableContainer::iterator it = maTabs.begin();
    for (;it != maTabs.end(); ++it)
    {
        if (*it)
            (*it)->CompileColRowNameFormula(aCxt);
    }
}

void ScDocument::InvalidateTableArea()
{
    TableContainer::iterator it = maTabs.begin();
    for (;it != maTabs.end() && *it; ++it)
    {
        (*it)->InvalidateTableArea();
        if ( (*it)->IsScenario() )
            (*it)->InvalidateScenarioRanges();
    }
}

sal_Int32 ScDocument::GetMaxStringLen( SCTAB nTab, SCCOL nCol,
        SCROW nRowStart, SCROW nRowEnd, rtl_TextEncoding eCharSet ) const
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        return maTabs[nTab]->GetMaxStringLen( nCol, nRowStart, nRowEnd, eCharSet );
    else
        return 0;
}

sal_Int32 ScDocument::GetMaxNumberStringLen( sal_uInt16& nPrecision, SCTAB nTab,
                                    SCCOL nCol,
                                    SCROW nRowStart, SCROW nRowEnd ) const
{
    if (ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        return maTabs[nTab]->GetMaxNumberStringLen( nPrecision, nCol,
            nRowStart, nRowEnd );
    else
        return 0;
}

bool ScDocument::GetSelectionFunction( ScSubTotalFunc eFunc,
                                        const ScAddress& rCursor, const ScMarkData& rMark,
                                        double& rResult )
{
    ScFunctionData aData(eFunc);

    ScMarkData aMark(rMark);
    aMark.MarkToMulti();
    if (!aMark.IsMultiMarked() && !aMark.IsCellMarked(rCursor.Col(), rCursor.Row()))
        aMark.SetMarkArea(rCursor);

    SCTAB nMax = static_cast<SCTAB>(maTabs.size());
    ScMarkData::const_iterator itr = aMark.begin(), itrEnd = aMark.end();

    for (; itr != itrEnd && *itr < nMax && !aData.bError; ++itr)
        if (maTabs[*itr])
            maTabs[*itr]->UpdateSelectionFunction(aData, aMark);

            //TODO: pass rMark to UpdateSelection Function !!!!!

    if (!aData.bError)
        switch (eFunc)
        {
            case SUBTOTAL_FUNC_SUM:
                rResult = aData.nVal;
                break;
            case SUBTOTAL_FUNC_SELECTION_COUNT:
                rResult = aData.nCount;
                break;
            case SUBTOTAL_FUNC_CNT:
            case SUBTOTAL_FUNC_CNT2:
                rResult = aData.nCount;
                break;
            case SUBTOTAL_FUNC_AVE:
                if (aData.nCount)
                    rResult = aData.nVal / (double) aData.nCount;
                else
                    aData.bError = true;
                break;
            case SUBTOTAL_FUNC_MAX:
            case SUBTOTAL_FUNC_MIN:
                if (aData.nCount)
                    rResult = aData.nVal;
                else
                    aData.bError = true;
                break;
            default:
            {
                // added to avoid warnings
            }
        }

    if (aData.bError)
        rResult = 0.0;

    return !aData.bError;
}

double ScDocument::RoundValueAsShown( double fVal, sal_uInt32 nFormat ) const
{
    short nType;
    if ( (nType = GetFormatTable()->GetType( nFormat )) != css::util::NumberFormat::DATE
      && nType != css::util::NumberFormat::TIME && nType != css::util::NumberFormat::DATETIME )
    {
        short nPrecision;
        if ((nFormat % SV_COUNTRY_LANGUAGE_OFFSET) != 0)
        {
            nPrecision = (short)GetFormatTable()->GetFormatPrecision( nFormat );
            switch ( nType )
            {
                case css::util::NumberFormat::PERCENT:      // 0.41% == 0.0041
                    nPrecision += 2;
                    break;
                case css::util::NumberFormat::SCIENTIFIC:   // 1.23e-3 == 0.00123
                {
                    if ( fVal > 0.0 )
                        nPrecision = sal::static_int_cast<short>( nPrecision - (short)floor( log10( fVal ) ) );
                    else if ( fVal < 0.0 )
                        nPrecision = sal::static_int_cast<short>( nPrecision - (short)floor( log10( -fVal ) ) );
                    break;
                }
            }
        }
        else
        {
            nPrecision = (short)GetDocOptions().GetStdPrecision();
            // #i115512# no rounding for automatic decimals
            if (nPrecision == static_cast<short>(SvNumberFormatter::UNLIMITED_PRECISION))
                return fVal;
        }
        double fRound = ::rtl::math::round( fVal, nPrecision );
        if ( ::rtl::math::approxEqual( fVal, fRound ) )
            return fVal;        // rounding might introduce some error
        else
            return fRound;
    }
    else
        return fVal;
}

// conditional formats and validation ranges

sal_uLong ScDocument::AddCondFormat( ScConditionalFormat* pNew, SCTAB nTab )
{
    if(!pNew)
        return 0;

    if(ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        return maTabs[nTab]->AddCondFormat( pNew );

    return 0;
}

sal_uLong ScDocument::AddValidationEntry( const ScValidationData& rNew )
{
    if (rNew.IsEmpty())
        return 0;                   // empty is always 0

    if (!pValidationList)
        pValidationList = new ScValidationDataList;

    sal_uLong nMax = 0;
    for( ScValidationDataList::iterator it = pValidationList->begin(); it != pValidationList->end(); ++it )
    {
        const ScValidationData* pData = *it;
        sal_uLong nKey = pData->GetKey();
        if ( pData->EqualEntries( rNew ) )
            return nKey;
        if ( nKey > nMax )
            nMax = nKey;
    }

    // might be called from ScPatternAttr::PutInPool; thus clone (real copy)

    sal_uLong nNewKey = nMax + 1;
    ScValidationData* pInsert = rNew.Clone(this);
    pInsert->SetKey( nNewKey );
    pValidationList->InsertNew( pInsert );
    return nNewKey;
}

const SfxPoolItem* ScDocument::GetEffItem(
                        SCCOL nCol, SCROW nRow, SCTAB nTab, sal_uInt16 nWhich ) const
{
    const ScPatternAttr* pPattern = GetPattern( nCol, nRow, nTab );
    if ( pPattern )
    {
        const SfxItemSet& rSet = pPattern->GetItemSet();
        const SfxPoolItem* pItem;
        if ( rSet.GetItemState( ATTR_CONDITIONAL, true, &pItem ) == SfxItemState::SET )
        {
            const std::vector<sal_uInt32>& rIndex = static_cast<const ScCondFormatItem&>(pPattern->GetItem(ATTR_CONDITIONAL)).GetCondFormatData();
            ScConditionalFormatList* pCondFormList = GetCondFormList( nTab );
            if (!rIndex.empty() && pCondFormList)
            {
                for(std::vector<sal_uInt32>::const_iterator itr = rIndex.begin(), itrEnd = rIndex.end();
                        itr != itrEnd; ++itr)
                {
                    const ScConditionalFormat* pForm = pCondFormList->GetFormat( *itr );
                    if ( pForm )
                    {
                        ScAddress aPos(nCol, nRow, nTab);
                        ScRefCellValue aCell(const_cast<ScDocument&>(*this), aPos);
                        OUString aStyle = pForm->GetCellStyle(aCell, aPos);
                        if (!aStyle.isEmpty())
                        {
                            SfxStyleSheetBase* pStyleSheet = xPoolHelper->GetStylePool()->Find(
                                    aStyle, SFX_STYLE_FAMILY_PARA );
                            if ( pStyleSheet && pStyleSheet->GetItemSet().GetItemState(
                                        nWhich, true, &pItem ) == SfxItemState::SET )
                                return pItem;
                        }
                    }
                }
            }
        }
        return &rSet.Get( nWhich );
    }
    OSL_FAIL("no pattern");
    return nullptr;
}

const SfxItemSet* ScDocument::GetCondResult( SCCOL nCol, SCROW nRow, SCTAB nTab ) const
{
    ScConditionalFormatList* pFormatList = GetCondFormList(nTab);
    if (!pFormatList)
        return nullptr;

    ScAddress aPos(nCol, nRow, nTab);
    ScRefCellValue aCell(const_cast<ScDocument&>(*this), aPos);
    const ScPatternAttr* pPattern = GetPattern( nCol, nRow, nTab );
    const std::vector<sal_uInt32>& rIndex =
        static_cast<const ScCondFormatItem&>(pPattern->GetItem(ATTR_CONDITIONAL)).GetCondFormatData();

    return GetCondResult(aCell, aPos, *pFormatList, rIndex);
}

const SfxItemSet* ScDocument::GetCondResult(
    ScRefCellValue& rCell, const ScAddress& rPos, const ScConditionalFormatList& rList,
    const std::vector<sal_uInt32>& rIndex ) const
{
    std::vector<sal_uInt32>::const_iterator itr = rIndex.begin(), itrEnd = rIndex.end();
    for (; itr != itrEnd; ++itr)
    {
        const ScConditionalFormat* pForm = rList.GetFormat(*itr);
        if (!pForm)
            continue;

        const OUString& aStyle = pForm->GetCellStyle(rCell, rPos);
        if (!aStyle.isEmpty())
        {
            SfxStyleSheetBase* pStyleSheet =
                xPoolHelper->GetStylePool()->Find(aStyle, SFX_STYLE_FAMILY_PARA);

            if (pStyleSheet)
                return &pStyleSheet->GetItemSet();

            // if style is not there, treat like no condition
        }
    }

    return nullptr;
}

ScConditionalFormat* ScDocument::GetCondFormat(
                            SCCOL nCol, SCROW nRow, SCTAB nTab ) const
{
    sal_uInt32 nIndex = 0;
    const std::vector<sal_uInt32>& rCondFormats = static_cast<const ScCondFormatItem*>(GetAttr(nCol, nRow, nTab, ATTR_CONDITIONAL))->GetCondFormatData();

    if(!rCondFormats.empty())
        nIndex = rCondFormats[0];

    if (nIndex)
    {
        ScConditionalFormatList* pCondFormList = GetCondFormList(nTab);
        if (pCondFormList)
            return pCondFormList->GetFormat( nIndex );
        else
        {
            OSL_FAIL("pCondFormList is 0");
        }
    }

    return nullptr;
}

ScConditionalFormatList* ScDocument::GetCondFormList(SCTAB nTab) const
{
    if(ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        return maTabs[nTab]->GetCondFormList();

    return nullptr;
}

void ScDocument::SetCondFormList( ScConditionalFormatList* pList, SCTAB nTab )
{
    if(ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        maTabs[nTab]->SetCondFormList(pList);
}

const ScValidationData* ScDocument::GetValidationEntry( sal_uLong nIndex ) const
{
    if ( pValidationList )
        return pValidationList->GetData( nIndex );
    else
        return nullptr;
}

void ScDocument::DeleteConditionalFormat(sal_uLong nOldIndex, SCTAB nTab)
{
    if(ValidTab(nTab) && nTab < static_cast<SCTAB>(maTabs.size()) && maTabs[nTab])
        maTabs[nTab]->DeleteConditionalFormat(nOldIndex);
}

bool ScDocument::HasDetectiveOperations() const
{
    return pDetOpList && pDetOpList->Count();
}

void ScDocument::AddDetectiveOperation( const ScDetOpData& rData )
{
    if (!pDetOpList)
        pDetOpList = new ScDetOpList;

    pDetOpList->Append( new ScDetOpData( rData ) );
}

void ScDocument::ClearDetectiveOperations()
{
    delete pDetOpList;      // deletes also the entries
    pDetOpList = nullptr;
}

void ScDocument::SetDetOpList(ScDetOpList* pNew)
{
    delete pDetOpList;      // deletes also the entries
    pDetOpList = pNew;
}

// Comparison of Documents

//  Pfriemel-Factors
#define SC_DOCCOMP_MAXDIFF  256
#define SC_DOCCOMP_MINGOOD  128
#define SC_DOCCOMP_COLUMNS  10
#define SC_DOCCOMP_ROWS     100

sal_uInt16 ScDocument::RowDifferences( SCROW nThisRow, SCTAB nThisTab,
                                    ScDocument& rOtherDoc, SCROW nOtherRow, SCTAB nOtherTab,
                                    SCCOL nMaxCol, SCCOLROW* pOtherCols )
{
    sal_uLong nDif = 0;
    sal_uLong nUsed = 0;
    for (SCCOL nThisCol=0; nThisCol<=nMaxCol; nThisCol++)
    {
        SCCOL nOtherCol;
        if ( pOtherCols )
            nOtherCol = static_cast<SCCOL>(pOtherCols[nThisCol]);
        else
            nOtherCol = nThisCol;

        if (ValidCol(nOtherCol))    // only compare columns that are common to both docs
        {
            ScRefCellValue aThisCell(*this, ScAddress(nThisCol, nThisRow, nThisTab));
            ScRefCellValue aOtherCell(rOtherDoc, ScAddress(nOtherCol, nOtherRow, nOtherTab));
            if (!aThisCell.equalsWithoutFormat(aOtherCell))
            {
                if (!aThisCell.isEmpty() && !aOtherCell.isEmpty())
                    nDif += 3;
                else
                    nDif += 4;      // content <-> empty counts more
            }

            if (!aThisCell.isEmpty() || !aOtherCell.isEmpty())
                ++nUsed;
        }
    }

    if (nUsed > 0)
        return static_cast<sal_uInt16>((nDif*64)/nUsed);            // max.256 (SC_DOCCOMP_MAXDIFF)

    OSL_ENSURE(!nDif,"Diff without Used");
    return 0;
}

sal_uInt16 ScDocument::ColDifferences( SCCOL nThisCol, SCTAB nThisTab,
                                    ScDocument& rOtherDoc, SCCOL nOtherCol, SCTAB nOtherTab,
                                    SCROW nMaxRow, SCCOLROW* pOtherRows )
{

    //TODO: optimize e.g. with iterator?

    sal_uLong nDif = 0;
    sal_uLong nUsed = 0;
    for (SCROW nThisRow=0; nThisRow<=nMaxRow; nThisRow++)
    {
        SCROW nOtherRow;
        if ( pOtherRows )
            nOtherRow = pOtherRows[nThisRow];
        else
            nOtherRow = nThisRow;

        if (ValidRow(nOtherRow))    // only compare rows that are common to both docs
        {
            ScRefCellValue aThisCell(*this, ScAddress(nThisCol, nThisRow, nThisTab));
            ScRefCellValue aOtherCell(rOtherDoc, ScAddress(nOtherCol, nOtherRow, nOtherTab));
            if (!aThisCell.equalsWithoutFormat(aOtherCell))
            {
                if (!aThisCell.isEmpty() && !aOtherCell.isEmpty())
                    nDif += 3;
                else
                    nDif += 4;      // content <-> empty counts more
            }

            if (!aThisCell.isEmpty() || !aOtherCell.isEmpty())
                ++nUsed;
        }
    }

    if (nUsed > 0)
        return static_cast<sal_uInt16>((nDif*64)/nUsed);    // max.256

    OSL_ENSURE(!nDif,"Diff without Used");
    return 0;
}

void ScDocument::FindOrder( SCCOLROW* pOtherRows, SCCOLROW nThisEndRow, SCCOLROW nOtherEndRow,
                            bool bColumns, ScDocument& rOtherDoc, SCTAB nThisTab, SCTAB nOtherTab,
                            SCCOLROW nEndCol, SCCOLROW* pTranslate, ScProgress* pProgress, sal_uLong nProAdd )
{
    //  bColumns=true: rows are columns and vice versa

    SCCOLROW nMaxCont;                      // continue by how much
    SCCOLROW nMinGood;                      // what is a hit (incl.)
    if ( bColumns )
    {
        nMaxCont = SC_DOCCOMP_COLUMNS;      // 10 columns
        nMinGood = SC_DOCCOMP_MINGOOD;

        //TODO: additional pass with nMinGood = 0 ????

    }
    else
    {
        nMaxCont = SC_DOCCOMP_ROWS;         // 100 rows
        nMinGood = SC_DOCCOMP_MINGOOD;
    }
    bool bUseTotal = bColumns && !pTranslate;       // only for the 1st pass

    SCCOLROW nOtherRow = 0;
    sal_uInt16 nComp;
    SCCOLROW nThisRow;
    bool bTotal = false;        // hold for several nThisRow
    SCCOLROW nUnknown = 0;
    for (nThisRow = 0; nThisRow <= nThisEndRow; nThisRow++)
    {
        SCCOLROW nTempOther = nOtherRow;
        bool bFound = false;
        sal_uInt16 nBest = SC_DOCCOMP_MAXDIFF;
        SCCOLROW nMax = std::min( nOtherEndRow, static_cast<SCCOLROW>(( nTempOther + nMaxCont + nUnknown )) );
        for (SCCOLROW i=nTempOther; i<=nMax && nBest>0; i++)    // stop at 0
        {
            if (bColumns)
                nComp = ColDifferences( static_cast<SCCOL>(nThisRow), nThisTab, rOtherDoc, static_cast<SCCOL>(i), nOtherTab, nEndCol, pTranslate );
            else
                nComp = RowDifferences( nThisRow, nThisTab, rOtherDoc, i, nOtherTab, static_cast<SCCOL>(nEndCol), pTranslate );
            if ( nComp < nBest && ( nComp <= nMinGood || bTotal ) )
            {
                nTempOther = i;
                nBest = nComp;
                bFound = true;
            }
            if ( nComp < SC_DOCCOMP_MAXDIFF || bFound )
                bTotal = false;
            else if ( i == nTempOther && bUseTotal )
                bTotal = true;                          // only at the very top
        }
        if ( bFound )
        {
            pOtherRows[nThisRow] = nTempOther;
            nOtherRow = nTempOther + 1;
            nUnknown = 0;
        }
        else
        {
            pOtherRows[nThisRow] = SCROW_MAX;
            ++nUnknown;
        }

        if (pProgress)
            pProgress->SetStateOnPercent(nProAdd+static_cast<sal_uLong>(nThisRow));
    }

    // fill in blocks that don't match

    SCROW nFillStart = 0;
    SCROW nFillPos = 0;
    bool bInFill = false;
    for (nThisRow = 0; nThisRow <= nThisEndRow+1; nThisRow++)
    {
        SCROW nThisOther = ( nThisRow <= nThisEndRow ) ? pOtherRows[nThisRow] : (nOtherEndRow+1);
        if ( ValidRow(nThisOther) )
        {
            if ( bInFill )
            {
                if ( nThisOther > nFillStart )      // is there something to distribute?
                {
                    SCROW nDiff1 = nThisOther - nFillStart;
                    SCROW nDiff2 = nThisRow   - nFillPos;
                    SCROW nMinDiff = std::min(nDiff1, nDiff2);
                    for (SCROW i=0; i<nMinDiff; i++)
                        pOtherRows[nFillPos+i] = nFillStart+i;
                }

                bInFill = false;
            }
            nFillStart = nThisOther + 1;
            nFillPos = nThisRow + 1;
        }
        else
            bInFill = true;
    }
}

void ScDocument::CompareDocument( ScDocument& rOtherDoc )
{
    if (!pChangeTrack)
        return;

    SCTAB nThisCount = GetTableCount();
    SCTAB nOtherCount = rOtherDoc.GetTableCount();
    std::unique_ptr<SCTAB[]> pOtherTabs(new SCTAB[nThisCount]);
    SCTAB nThisTab;

    //  compare tables with identical names
    OUString aThisName;
    OUString aOtherName;
    for (nThisTab=0; nThisTab<nThisCount; nThisTab++)
    {
        SCTAB nOtherTab = SCTAB_MAX;
        if (!IsScenario(nThisTab))  // skip scenarios
        {
            GetName( nThisTab, aThisName );
            for (SCTAB nTemp=0; nTemp<nOtherCount && nOtherTab>MAXTAB; nTemp++)
                if (!rOtherDoc.IsScenario(nTemp))
                {
                    rOtherDoc.GetName( nTemp, aOtherName );
                    if ( aThisName.equals(aOtherName) )
                        nOtherTab = nTemp;
                }
        }
        pOtherTabs[nThisTab] = nOtherTab;
    }
    //  fill in, so that un-named tables don't get lost
    SCTAB nFillStart = 0;
    SCTAB nFillPos = 0;
    bool bInFill = false;
    for (nThisTab = 0; nThisTab <= nThisCount; nThisTab++)
    {
        SCTAB nThisOther = ( nThisTab < nThisCount ) ? pOtherTabs[nThisTab] : nOtherCount;
        if ( ValidTab(nThisOther) )
        {
            if ( bInFill )
            {
                if ( nThisOther > nFillStart )      // is there something to distribute?
                {
                    SCTAB nDiff1 = nThisOther - nFillStart;
                    SCTAB nDiff2 = nThisTab   - nFillPos;
                    SCTAB nMinDiff = std::min(nDiff1, nDiff2);
                    for (SCTAB i=0; i<nMinDiff; i++)
                        if ( !IsScenario(nFillPos+i) && !rOtherDoc.IsScenario(nFillStart+i) )
                            pOtherTabs[nFillPos+i] = nFillStart+i;
                }

                bInFill = false;
            }
            nFillStart = nThisOther + 1;
            nFillPos = nThisTab + 1;
        }
        else
            bInFill = true;
    }

    //  compare tables in the original order

    for (nThisTab=0; nThisTab<nThisCount; nThisTab++)
    {
        SCTAB nOtherTab = pOtherTabs[nThisTab];
        if ( ValidTab(nOtherTab) )
        {
            SCCOL nThisEndCol = 0;
            SCROW nThisEndRow = 0;
            SCCOL nOtherEndCol = 0;
            SCROW nOtherEndRow = 0;
            GetCellArea( nThisTab, nThisEndCol, nThisEndRow );
            rOtherDoc.GetCellArea( nOtherTab, nOtherEndCol, nOtherEndRow );
            SCCOL nEndCol = std::max(nThisEndCol, nOtherEndCol);
            SCROW nEndRow = std::max(nThisEndRow, nOtherEndRow);
            SCCOL nThisCol;
            SCROW nThisRow;
            sal_uLong n1,n2;    // for AppendDeleteRange

            //TODO: one Progress over all tables ???

            OUString aTabName;
            GetName( nThisTab, aTabName );
            OUString aTemplate = ScGlobal::GetRscString(STR_PROGRESS_COMPARING);
            sal_Int32 nIndex = 0;
            OUStringBuffer aProText = aTemplate.getToken( 0, '#', nIndex );
            aProText.append(aTabName);
            nIndex = 0;
            aProText.append(aTemplate.getToken( 1, '#', nIndex ));
            ScProgress aProgress( GetDocumentShell(),
                                        aProText.makeStringAndClear(), 3*nThisEndRow );  // 2x FindOrder, 1x here
            long nProgressStart = 2*nThisEndRow;                    // start for here

            std::unique_ptr<SCCOLROW[]> pTempRows(new SCCOLROW[nThisEndRow+1]);
            std::unique_ptr<SCCOLROW[]> pOtherRows(new SCCOLROW[nThisEndRow+1]);
            std::unique_ptr<SCCOLROW[]> pOtherCols(new SCCOLROW[nThisEndCol+1]);

            //  find inserted/deleted columns/rows:
            //  Two attempts:
            //  1) compare original rows                    (pTempRows)
            //  2) compare original columns                 (pOtherCols)
            //     with this column order compare rows      (pOtherRows)

            //TODO: compare columns twice with different nMinGood ???

            // 1
            FindOrder( pTempRows.get(), nThisEndRow, nOtherEndRow, false,
                        rOtherDoc, nThisTab, nOtherTab, nEndCol, nullptr, &aProgress, 0 );
            // 2
            FindOrder( pOtherCols.get(), nThisEndCol, nOtherEndCol, true,
                        rOtherDoc, nThisTab, nOtherTab, nEndRow, nullptr, nullptr, 0 );
            FindOrder( pOtherRows.get(), nThisEndRow, nOtherEndRow, false,
                        rOtherDoc, nThisTab, nOtherTab, nThisEndCol,
                       pOtherCols.get(), &aProgress, nThisEndRow );

            sal_uLong nMatch1 = 0;  // pTempRows, no columns
            for (nThisRow = 0; nThisRow<=nThisEndRow; nThisRow++)
                if (ValidRow(pTempRows[nThisRow]))
                    nMatch1 += SC_DOCCOMP_MAXDIFF -
                               RowDifferences( nThisRow, nThisTab, rOtherDoc, pTempRows[nThisRow],
                                                nOtherTab, nEndCol, nullptr );

            sal_uLong nMatch2 = 0;  // pOtherRows, pOtherCols
            for (nThisRow = 0; nThisRow<=nThisEndRow; nThisRow++)
                if (ValidRow(pOtherRows[nThisRow]))
                    nMatch2 += SC_DOCCOMP_MAXDIFF -
                               RowDifferences( nThisRow, nThisTab, rOtherDoc, pOtherRows[nThisRow],
                                               nOtherTab, nThisEndCol, pOtherCols.get() );

            if ( nMatch1 >= nMatch2 )           // without columns ?
            {
                //  reset columns
                for (nThisCol = 0; nThisCol<=nThisEndCol; nThisCol++)
                    pOtherCols[nThisCol] = nThisCol;

                //  swap row-arrays (they get both deleted anyway)
                pTempRows.swap(pOtherRows);
            }
            else
            {
                //  remains for pOtherCols, pOtherRows
            }

            //  Generate Change-Actions
            //  1) columns from the right
            //  2) rows from below
            //  3) single cells in normal order

            //  Actions for inserted/deleted columns

            SCCOL nLastOtherCol = static_cast<SCCOL>(nOtherEndCol + 1);
            //  nThisEndCol ... 0
            for ( nThisCol = nThisEndCol+1; nThisCol > 0; )
            {
                --nThisCol;
                SCCOL nOtherCol = static_cast<SCCOL>(pOtherCols[nThisCol]);
                if ( ValidCol(nOtherCol) && nOtherCol+1 < nLastOtherCol )
                {
                    // gap -> deleted
                    ScRange aDelRange( nOtherCol+1, 0, nOtherTab,
                                        nLastOtherCol-1, MAXROW, nOtherTab );
                    pChangeTrack->AppendDeleteRange( aDelRange, &rOtherDoc, n1, n2 );
                }
                if ( nOtherCol > MAXCOL )                       // inserted
                {
                    //  combine
                    if ( nThisCol == nThisEndCol || ValidCol(static_cast<SCCOL>(pOtherCols[nThisCol+1])) )
                    {
                        SCCOL nFirstNew = static_cast<SCCOL>(nThisCol);
                        while ( nFirstNew > 0 && pOtherCols[nFirstNew-1] > MAXCOL )
                            --nFirstNew;
                        SCCOL nDiff = nThisCol - nFirstNew;
                        ScRange aRange( nLastOtherCol, 0, nOtherTab,
                                        nLastOtherCol+nDiff, MAXROW, nOtherTab );
                        pChangeTrack->AppendInsert( aRange );
                    }
                }
                else
                    nLastOtherCol = nOtherCol;
            }
            if ( nLastOtherCol > 0 )                            // deleted at the very top
            {
                ScRange aDelRange( 0, 0, nOtherTab,
                                    nLastOtherCol-1, MAXROW, nOtherTab );
                pChangeTrack->AppendDeleteRange( aDelRange, &rOtherDoc, n1, n2 );
            }

            //  Actions for inserted/deleted rows

            SCROW nLastOtherRow = nOtherEndRow + 1;
            //  nThisEndRow ... 0
            for ( nThisRow = nThisEndRow+1; nThisRow > 0; )
            {
                --nThisRow;
                SCROW nOtherRow = pOtherRows[nThisRow];
                if ( ValidRow(nOtherRow) && nOtherRow+1 < nLastOtherRow )
                {
                    // gap -> deleted
                    ScRange aDelRange( 0, nOtherRow+1, nOtherTab,
                                        MAXCOL, nLastOtherRow-1, nOtherTab );
                    pChangeTrack->AppendDeleteRange( aDelRange, &rOtherDoc, n1, n2 );
                }
                if ( nOtherRow > MAXROW )                       // inserted
                {
                    //  combine
                    if ( nThisRow == nThisEndRow || ValidRow(pOtherRows[nThisRow+1]) )
                    {
                        SCROW nFirstNew = nThisRow;
                        while ( nFirstNew > 0 && pOtherRows[nFirstNew-1] > MAXROW )
                            --nFirstNew;
                        SCROW nDiff = nThisRow - nFirstNew;
                        ScRange aRange( 0, nLastOtherRow, nOtherTab,
                                        MAXCOL, nLastOtherRow+nDiff, nOtherTab );
                        pChangeTrack->AppendInsert( aRange );
                    }
                }
                else
                    nLastOtherRow = nOtherRow;
            }
            if ( nLastOtherRow > 0 )                            // deleted at the very top
            {
                ScRange aDelRange( 0, 0, nOtherTab,
                                    MAXCOL, nLastOtherRow-1, nOtherTab );
                pChangeTrack->AppendDeleteRange( aDelRange, &rOtherDoc, n1, n2 );
            }

             //  walk rows to find single cells

            for (nThisRow = 0; nThisRow <= nThisEndRow; nThisRow++)
            {
                SCROW nOtherRow = pOtherRows[nThisRow];
                for (nThisCol = 0; nThisCol <= nThisEndCol; nThisCol++)
                {
                    SCCOL nOtherCol = static_cast<SCCOL>(pOtherCols[nThisCol]);
                    ScAddress aThisPos( nThisCol, nThisRow, nThisTab );
                    ScCellValue aThisCell;
                    aThisCell.assign(*this, aThisPos);
                    ScCellValue aOtherCell; // start empty
                    if ( ValidCol(nOtherCol) && ValidRow(nOtherRow) )
                    {
                        ScAddress aOtherPos( nOtherCol, nOtherRow, nOtherTab );
                        aOtherCell.assign(rOtherDoc, aOtherPos);
                    }

                    if (!aThisCell.equalsWithoutFormat(aOtherCell))
                    {
                        ScRange aRange( aThisPos );
                        ScChangeActionContent* pAction = new ScChangeActionContent( aRange );
                        pAction->SetOldValue(aOtherCell, &rOtherDoc, this);
                        pAction->SetNewValue(aThisCell, this);
                        pChangeTrack->Append( pAction );
                    }
                }
                aProgress.SetStateOnPercent(nProgressStart+nThisRow);
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
