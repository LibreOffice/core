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

#include <scitems.hxx>
#include <svl/zforlist.hxx>
#include <rtl/math.hxx>
#include <sal/log.hxx>
#include <unotools/collatorwrapper.hxx>

#include <com/sun/star/sheet/ConditionOperator2.hpp>

#include <attrib.hxx>
#include <conditio.hxx>
#include <formulacell.hxx>
#include <document.hxx>
#include <compiler.hxx>
#include <rangelst.hxx>
#include <rangenam.hxx>
#include <rangeutl.hxx>
#include <colorscale.hxx>
#include <cellvalue.hxx>
#include <editutil.hxx>
#include <tokenarray.hxx>
#include <fillinfo.hxx>
#include <refupdatecontext.hxx>
#include <formula/errorcodes.hxx>
#include <svl/sharedstring.hxx>
#include <svl/sharedstringpool.hxx>
#include <memory>
#include <numeric>

using namespace formula;

ScFormatEntry::ScFormatEntry(ScDocument* pDoc):
    mpDoc(pDoc)
{
}

bool ScFormatEntry::operator==( const ScFormatEntry& r ) const
{
    return IsEqual(r, false);
}

// virtual
bool ScFormatEntry::IsEqual( const ScFormatEntry& /*r*/, bool /*bIgnoreSrcPos*/ ) const
{
    // By default, return false; this makes sense for all cases except ScConditionEntry
    // As soon as databar and color scale are tested we need to think about the range
    return false;
}

void ScFormatEntry::startRendering()
{
}

void ScFormatEntry::endRendering()
{
}

static bool lcl_HasRelRef( ScDocument* pDoc, const ScTokenArray* pFormula, sal_uInt16 nRecursion = 0 )
{
    if (pFormula)
    {
        FormulaTokenArrayPlainIterator aIter( *pFormula );
        FormulaToken* t;
        for( t = aIter.Next(); t; t = aIter.Next() )
        {
            switch( t->GetType() )
            {
                case svDoubleRef:
                {
                    ScSingleRefData& rRef2 = t->GetDoubleRef()->Ref2;
                    if ( rRef2.IsColRel() || rRef2.IsRowRel() || rRef2.IsTabRel() )
                        return true;
                    [[fallthrough]];
                }

                case svSingleRef:
                {
                    ScSingleRefData& rRef1 = *t->GetSingleRef();
                    if ( rRef1.IsColRel() || rRef1.IsRowRel() || rRef1.IsTabRel() )
                        return true;
                }
                break;

                case svIndex:
                {
                    if( t->GetOpCode() == ocName )      // DB areas always absolute
                        if( ScRangeData* pRangeData = pDoc->FindRangeNameBySheetAndIndex( t->GetSheet(), t->GetIndex()) )
                            if( (nRecursion < 42) && lcl_HasRelRef( pDoc, pRangeData->GetCode(), nRecursion + 1 ) )
                                return true;
                }
                break;

                // #i34474# function result dependent on cell position
                case svByte:
                {
                    switch( t->GetOpCode() )
                    {
                        case ocRow:     // ROW() returns own row index
                        case ocColumn:  // COLUMN() returns own column index
                        case ocSheet:   // SHEET() returns own sheet index
                        case ocCell:    // CELL() may return own cell address
                            return true;
                        default:
                        {
                            // added to avoid warnings
                        }
                    }
                }
                break;

                default:
                {
                    // added to avoid warnings
                }
            }
        }
    }
    return false;
}

namespace {

void start_listen_to(ScFormulaListener& rListener, const ScTokenArray* pTokens, const ScRangeList& rRangeList)
{
    size_t n = rRangeList.size();
    for (size_t i = 0; i < n; ++i)
    {
        const ScRange & rRange = rRangeList[i];
        rListener.addTokenArray(pTokens, rRange);
    }
}

}

void ScConditionEntry::StartListening()
{
    if (!pCondFormat)
        return;

    const ScRangeList& rRanges = pCondFormat->GetRange();
    mpListener->stopListening();
    start_listen_to(*mpListener, pFormula1.get(), rRanges);
    start_listen_to(*mpListener, pFormula2.get(), rRanges);

    mpListener->setCallback([&]() { pCondFormat->DoRepaint();});
}

void ScConditionEntry::SetParent(ScConditionalFormat* pParent)
{
    pCondFormat = pParent;
    StartListening();
}

ScConditionEntry::ScConditionEntry( const ScConditionEntry& r ) :
    ScFormatEntry(r.mpDoc),
    eOp(r.eOp),
    nOptions(r.nOptions),
    nVal1(r.nVal1),
    nVal2(r.nVal2),
    aStrVal1(r.aStrVal1),
    aStrVal2(r.aStrVal2),
    aStrNmsp1(r.aStrNmsp1),
    aStrNmsp2(r.aStrNmsp2),
    eTempGrammar1(r.eTempGrammar1),
    eTempGrammar2(r.eTempGrammar2),
    bIsStr1(r.bIsStr1),
    bIsStr2(r.bIsStr2),
    aSrcPos(r.aSrcPos),
    aSrcString(r.aSrcString),
    bRelRef1(r.bRelRef1),
    bRelRef2(r.bRelRef2),
    bFirstRun(true),
    mpListener(new ScFormulaListener(r.mpDoc)),
    pCondFormat(r.pCondFormat)
{
    // ScTokenArray copy ctor creates a flat copy
    if (r.pFormula1)
        pFormula1.reset( new ScTokenArray( *r.pFormula1 ) );
    if (r.pFormula2)
        pFormula2.reset( new ScTokenArray( *r.pFormula2 ) );

    StartListening();
    // Formula cells are created at IsValid
}

ScConditionEntry::ScConditionEntry( ScDocument* pDocument, const ScConditionEntry& r ) :
    ScFormatEntry(pDocument),
    eOp(r.eOp),
    nOptions(r.nOptions),
    nVal1(r.nVal1),
    nVal2(r.nVal2),
    aStrVal1(r.aStrVal1),
    aStrVal2(r.aStrVal2),
    aStrNmsp1(r.aStrNmsp1),
    aStrNmsp2(r.aStrNmsp2),
    eTempGrammar1(r.eTempGrammar1),
    eTempGrammar2(r.eTempGrammar2),
    bIsStr1(r.bIsStr1),
    bIsStr2(r.bIsStr2),
    aSrcPos(r.aSrcPos),
    aSrcString(r.aSrcString),
    bRelRef1(r.bRelRef1),
    bRelRef2(r.bRelRef2),
    bFirstRun(true),
    mpListener(new ScFormulaListener(pDocument)),
    pCondFormat(r.pCondFormat)
{
    // Real copy of the formulas (for Ref Undo)
    if (r.pFormula1)
        pFormula1 = r.pFormula1->Clone();
    if (r.pFormula2)
        pFormula2 = r.pFormula2->Clone();

    // Formula cells are created at IsValid
    // TODO: But not in the Clipboard! So interpret beforehand!
}

ScConditionEntry::ScConditionEntry( ScConditionMode eOper,
        const OUString& rExpr1, const OUString& rExpr2, ScDocument* pDocument, const ScAddress& rPos,
        const OUString& rExprNmsp1, const OUString& rExprNmsp2,
        FormulaGrammar::Grammar eGrammar1, FormulaGrammar::Grammar eGrammar2 ) :
    ScFormatEntry(pDocument),
    eOp(eOper),
    nOptions(0),
    nVal1(0.0),
    nVal2(0.0),
    aStrNmsp1(rExprNmsp1),
    aStrNmsp2(rExprNmsp2),
    eTempGrammar1(eGrammar1),
    eTempGrammar2(eGrammar2),
    bIsStr1(false),
    bIsStr2(false),
    aSrcPos(rPos),
    bRelRef1(false),
    bRelRef2(false),
    bFirstRun(true),
    mpListener(new ScFormulaListener(pDocument)),
    pCondFormat(nullptr)
{
    Compile( rExpr1, rExpr2, rExprNmsp1, rExprNmsp2, eGrammar1, eGrammar2, false );

    // Formula cells are created at IsValid
}

ScConditionEntry::ScConditionEntry( ScConditionMode eOper,
                                const ScTokenArray* pArr1, const ScTokenArray* pArr2,
                                ScDocument* pDocument, const ScAddress& rPos ) :
    ScFormatEntry(pDocument),
    eOp(eOper),
    nOptions(0),
    nVal1(0.0),
    nVal2(0.0),
    eTempGrammar1(FormulaGrammar::GRAM_DEFAULT),
    eTempGrammar2(FormulaGrammar::GRAM_DEFAULT),
    bIsStr1(false),
    bIsStr2(false),
    aSrcPos(rPos),
    bRelRef1(false),
    bRelRef2(false),
    bFirstRun(true),
    mpListener(new ScFormulaListener(pDocument)),
    pCondFormat(nullptr)
{
    if ( pArr1 )
    {
        pFormula1.reset( new ScTokenArray( *pArr1 ) );
        SimplifyCompiledFormula( pFormula1, nVal1, bIsStr1, aStrVal1 );
        bRelRef1 = lcl_HasRelRef( mpDoc, pFormula1.get() );
    }
    if ( pArr2 )
    {
        pFormula2.reset( new ScTokenArray( *pArr2 ) );
        SimplifyCompiledFormula( pFormula2, nVal2, bIsStr2, aStrVal2 );
        bRelRef2 = lcl_HasRelRef( mpDoc, pFormula2.get() );
    }

    StartListening();

    // Formula cells are created at IsValid
}

ScConditionEntry::~ScConditionEntry()
{
}

void ScConditionEntry::SimplifyCompiledFormula( std::unique_ptr<ScTokenArray>& rFormula,
                                                double& rVal,
                                                bool& rIsStr,
                                                OUString& rStrVal )
{
    if ( rFormula->GetLen() == 1 )
    {
        // Single (constant number)?
        FormulaToken* pToken = rFormula->FirstToken();
        if ( pToken->GetOpCode() == ocPush )
        {
            if ( pToken->GetType() == svDouble )
            {
                rVal = pToken->GetDouble();
                rFormula.reset();             // Do not remember as formula
            }
            else if ( pToken->GetType() == svString )
            {
                rIsStr = true;
                rStrVal = pToken->GetString().getString();
                rFormula.reset();             // Do not remember as formula
            }
        }
    }
}

void ScConditionEntry::SetOperation(ScConditionMode eMode)
{
    eOp = eMode;
}

void ScConditionEntry::Compile( const OUString& rExpr1, const OUString& rExpr2,
        const OUString& rExprNmsp1, const OUString& rExprNmsp2,
        FormulaGrammar::Grammar eGrammar1, FormulaGrammar::Grammar eGrammar2, bool bTextToReal )
{
    if ( !rExpr1.isEmpty() || !rExpr2.isEmpty() )
    {
        ScCompiler aComp( mpDoc, aSrcPos );

        if ( !rExpr1.isEmpty() )
        {
            pFormula1.reset();
            aComp.SetGrammar( eGrammar1 );
            if ( mpDoc->IsImportingXML() && !bTextToReal )
            {
                //  temporary formula string as string tokens
                pFormula1.reset( new ScTokenArray );
                pFormula1->AssignXMLString( rExpr1, rExprNmsp1 );
                // bRelRef1 is set when the formula is compiled again (CompileXML)
            }
            else
            {
                pFormula1 = aComp.CompileString( rExpr1, rExprNmsp1 );
                SimplifyCompiledFormula( pFormula1, nVal1, bIsStr1, aStrVal1 );
                bRelRef1 = lcl_HasRelRef( mpDoc, pFormula1.get() );
            }
        }

        if ( !rExpr2.isEmpty() )
        {
            pFormula2.reset();
            aComp.SetGrammar( eGrammar2 );
            if ( mpDoc->IsImportingXML() && !bTextToReal )
            {
                //  temporary formula string as string tokens
                pFormula2.reset( new ScTokenArray );
                pFormula2->AssignXMLString( rExpr2, rExprNmsp2 );
                // bRelRef2 is set when the formula is compiled again (CompileXML)
            }
            else
            {
                pFormula2 = aComp.CompileString( rExpr2, rExprNmsp2 );
                SimplifyCompiledFormula( pFormula2, nVal2, bIsStr2, aStrVal2 );
                bRelRef2 = lcl_HasRelRef( mpDoc, pFormula2.get() );
            }
        }
    }

    StartListening();
}

/**
 * Create formula cells
 */
void ScConditionEntry::MakeCells( const ScAddress& rPos )
{
    if ( !mpDoc->IsClipOrUndo() ) // Never calculate in the Clipboard!
    {
        if ( pFormula1 && !pFCell1 && !bRelRef1 )
        {
            // pFCell1 will hold a flat-copied ScTokenArray sharing ref-counted
            // code tokens with pFormula1
            pFCell1.reset( new ScFormulaCell(mpDoc, rPos, *pFormula1) );
            pFCell1->StartListeningTo( mpDoc );
        }

        if ( pFormula2 && !pFCell2 && !bRelRef2 )
        {
            // pFCell2 will hold a flat-copied ScTokenArray sharing ref-counted
            // code tokens with pFormula2
            pFCell2.reset( new ScFormulaCell(mpDoc, rPos, *pFormula2) );
            pFCell2->StartListeningTo( mpDoc );
        }
    }
}

void ScConditionEntry::SetIgnoreBlank(bool bSet)
{
    // The bit SC_COND_NOBLANKS is set if blanks are not ignored
    // (only of valid)
    if (bSet)
        nOptions &= ~SC_COND_NOBLANKS;
    else
        nOptions |= SC_COND_NOBLANKS;
}

/**
 * Delete formula cells, so we re-compile at the next IsValid
 */
void ScConditionEntry::CompileAll()
{
    pFCell1.reset();
    pFCell2.reset();
}

void ScConditionEntry::CompileXML()
{
    //  First parse the formula source position if it was stored as text
    if ( !aSrcString.isEmpty() )
    {
        ScAddress aNew;
        /* XML is always in OOo:A1 format, although R1C1 would be more amenable
         * to compression */
        if ( aNew.Parse( aSrcString, mpDoc ) & ScRefFlags::VALID )
            aSrcPos = aNew;
        // if the position is invalid, there isn't much we can do at this time
        aSrcString.clear();
    }

    //  Convert the text tokens that were created during XML import into real tokens.
    Compile( GetExpression(aSrcPos, 0, 0, eTempGrammar1),
             GetExpression(aSrcPos, 1, 0, eTempGrammar2),
             aStrNmsp1, aStrNmsp2, eTempGrammar1, eTempGrammar2, true );

    // Importing ocDde/ocWebservice?
    if (pFormula1)
        mpDoc->CheckLinkFormulaNeedingCheck(*pFormula1);
    if (pFormula2)
        mpDoc->CheckLinkFormulaNeedingCheck(*pFormula2);
}

void ScConditionEntry::SetSrcString( const OUString& rNew )
{
    // aSrcString is only evaluated in CompileXML
    SAL_WARN_IF( !mpDoc->IsImportingXML(), "sc", "SetSrcString is only valid for XML import" );

    aSrcString = rNew;
}

void ScConditionEntry::SetFormula1( const ScTokenArray& rArray )
{
    pFormula1.reset();
    if( rArray.GetLen() > 0 )
    {
        pFormula1.reset( new ScTokenArray( rArray ) );
        bRelRef1 = lcl_HasRelRef( mpDoc, pFormula1.get() );
    }

    StartListening();
}

void ScConditionEntry::SetFormula2( const ScTokenArray& rArray )
{
    pFormula2.reset();
    if( rArray.GetLen() > 0 )
    {
        pFormula2.reset( new ScTokenArray( rArray ) );
        bRelRef2 = lcl_HasRelRef( mpDoc, pFormula2.get() );
    }

    StartListening();
}

void ScConditionEntry::UpdateReference( sc::RefUpdateContext& rCxt )
{
    if(pCondFormat)
        aSrcPos = pCondFormat->GetRange().Combine().aStart;
    ScAddress aOldSrcPos = aSrcPos;
    bool bChangedPos = false;
    if (rCxt.meMode == URM_INSDEL && rCxt.maRange.In(aSrcPos))
    {
        ScAddress aErrorPos( ScAddress::UNINITIALIZED );
        if (!aSrcPos.Move(rCxt.mnColDelta, rCxt.mnRowDelta, rCxt.mnTabDelta, aErrorPos))
        {
            assert(!"can't move ScConditionEntry");
        }
        bChangedPos = aSrcPos != aOldSrcPos;
    }

    if (pFormula1)
    {
        sc::RefUpdateResult aRes;
        switch (rCxt.meMode)
        {
            case URM_INSDEL:
                aRes = pFormula1->AdjustReferenceOnShift(rCxt, aOldSrcPos);
            break;
            case URM_MOVE:
                aRes = pFormula1->AdjustReferenceOnMove(rCxt, aOldSrcPos, aSrcPos);
            break;
            default:
                ;
        }

        if (aRes.mbReferenceModified || bChangedPos)
            pFCell1.reset();       // is created again in IsValid
    }

    if (pFormula2)
    {
        sc::RefUpdateResult aRes;
        switch (rCxt.meMode)
        {
            case URM_INSDEL:
                aRes = pFormula2->AdjustReferenceOnShift(rCxt, aOldSrcPos);
            break;
            case URM_MOVE:
                aRes = pFormula2->AdjustReferenceOnMove(rCxt, aOldSrcPos, aSrcPos);
            break;
            default:
                ;
        }

        if (aRes.mbReferenceModified || bChangedPos)
            pFCell2.reset();       // is created again in IsValid
    }

    StartListening();
}

void ScConditionEntry::UpdateInsertTab( sc::RefUpdateInsertTabContext& rCxt )
{
    if (pFormula1)
    {
        pFormula1->AdjustReferenceOnInsertedTab(rCxt, aSrcPos);
        pFCell1.reset();
    }

    if (pFormula2)
    {
        pFormula2->AdjustReferenceOnInsertedTab(rCxt, aSrcPos);
        pFCell2.reset();
    }

    ScRangeUpdater::UpdateInsertTab(aSrcPos, rCxt);
}

void ScConditionEntry::UpdateDeleteTab( sc::RefUpdateDeleteTabContext& rCxt )
{
    if (pFormula1)
    {
        pFormula1->AdjustReferenceOnDeletedTab(rCxt, aSrcPos);
        pFCell1.reset();
    }

    if (pFormula2)
    {
        pFormula2->AdjustReferenceOnDeletedTab(rCxt, aSrcPos);
        pFCell2.reset();
    }

    ScRangeUpdater::UpdateDeleteTab(aSrcPos, rCxt);
    StartListening();
}

void ScConditionEntry::UpdateMoveTab( sc::RefUpdateMoveTabContext& rCxt )
{
    if (pFormula1)
    {
        pFormula1->AdjustReferenceOnMovedTab(rCxt, aSrcPos);
        pFCell1.reset();
    }

    if (pFormula2)
    {
        pFormula2->AdjustReferenceOnMovedTab(rCxt, aSrcPos);
        pFCell2.reset();
    }

    StartListening();
}

static bool lcl_IsEqual( const std::unique_ptr<ScTokenArray>& pArr1, const std::unique_ptr<ScTokenArray>& pArr2 )
{
    // We only compare the non-RPN array
    if ( pArr1 && pArr2 )
        return pArr1->EqualTokens( pArr2.get() );
    else
        return !pArr1 && !pArr2; // Both 0? -> the same
}

// virtual
bool ScConditionEntry::IsEqual( const ScFormatEntry& rOther, bool bIgnoreSrcPos ) const
{
    if (GetType() != rOther.GetType())
        return false;

    const ScConditionEntry& r = static_cast<const ScConditionEntry&>(rOther);

    bool bEq = (eOp == r.eOp && nOptions == r.nOptions &&
                lcl_IsEqual( pFormula1, r.pFormula1 ) &&
                lcl_IsEqual( pFormula2, r.pFormula2 ));

    if (!bIgnoreSrcPos)
    {
        // for formulas, the reference positions must be compared, too
        // (including aSrcString, for inserting the entries during XML import)
        if ( bEq && ( pFormula1 || pFormula2 ) && ( aSrcPos != r.aSrcPos || aSrcString != r.aSrcString ) )
            bEq = false;
    }

    // If not formulas, compare values
    if ( bEq && !pFormula1 && ( nVal1 != r.nVal1 || aStrVal1 != r.aStrVal1 || bIsStr1 != r.bIsStr1 ) )
        bEq = false;
    if ( bEq && !pFormula2 && ( nVal2 != r.nVal2 || aStrVal2 != r.aStrVal2 || bIsStr2 != r.bIsStr2 ) )
        bEq = false;

    return bEq;
}

void ScConditionEntry::Interpret( const ScAddress& rPos )
{
    // Create formula cells
    // Note: New Broadcaster (Note cells) may be inserted into the document!
    if ( ( pFormula1 && !pFCell1 ) || ( pFormula2 && !pFCell2 ) )
        MakeCells( rPos );

    // Evaluate formulas
    bool bDirty = false; // 1 and 2 separate?

    std::unique_ptr<ScFormulaCell> pTemp1;
    ScFormulaCell* pEff1 = pFCell1.get();
    if ( bRelRef1 )
    {
        pTemp1.reset(pFormula1 ? new ScFormulaCell(mpDoc, rPos, *pFormula1) : new ScFormulaCell(mpDoc, rPos));
        pEff1 = pTemp1.get();
    }
    if ( pEff1 )
    {
        if (!pEff1->IsRunning()) // Don't create 522
        {
            //TODO: Query Changed instead of Dirty!
            if (pEff1->GetDirty() && !bRelRef1 && mpDoc->GetAutoCalc())
                bDirty = true;
            if (pEff1->IsValue())
            {
                bIsStr1 = false;
                nVal1 = pEff1->GetValue();
                aStrVal1.clear();
            }
            else
            {
                bIsStr1 = true;
                aStrVal1 = pEff1->GetString().getString();
                nVal1 = 0.0;
            }
        }
    }
    pTemp1.reset();

    std::unique_ptr<ScFormulaCell> pTemp2;
    ScFormulaCell* pEff2 = pFCell2.get(); //@ 1!=2
    if ( bRelRef2 )
    {
        pTemp2.reset(pFormula2 ? new ScFormulaCell(mpDoc, rPos, *pFormula2) : new ScFormulaCell(mpDoc, rPos));
        pEff2 = pTemp2.get();
    }
    if ( pEff2 )
    {
        if (!pEff2->IsRunning()) // Don't create 522
        {
            if (pEff2->GetDirty() && !bRelRef2 && mpDoc->GetAutoCalc())
                bDirty = true;
            if (pEff2->IsValue())
            {
                bIsStr2 = false;
                nVal2 = pEff2->GetValue();
                aStrVal2.clear();
            }
            else
            {
                bIsStr2 = true;
                aStrVal2 = pEff2->GetString().getString();
                nVal2 = 0.0;
            }
        }
    }
    pTemp2.reset();

    // If IsRunning, the last values remain
    if (bDirty && !bFirstRun)
    {
        // Repaint everything for dependent formats
        DataChanged();
    }

    bFirstRun = false;
}

static bool lcl_GetCellContent( ScRefCellValue& rCell, bool bIsStr1, double& rArg, OUString& rArgStr,
        const ScDocument* pDoc )
{

    if (rCell.isEmpty())
        return !bIsStr1;

    bool bVal = true;

    switch (rCell.meType)
    {
        case CELLTYPE_VALUE:
            rArg = rCell.mfValue;
        break;
        case CELLTYPE_FORMULA:
        {
            bVal = rCell.mpFormula->IsValue();
            if (bVal)
                rArg = rCell.mpFormula->GetValue();
            else
                rArgStr = rCell.mpFormula->GetString().getString();
        }
        break;
        case CELLTYPE_STRING:
        case CELLTYPE_EDIT:
            bVal = false;
            if (rCell.meType == CELLTYPE_STRING)
                rArgStr = rCell.mpString->getString();
            else if (rCell.mpEditText)
                rArgStr = ScEditUtil::GetString(*rCell.mpEditText, pDoc);
        break;
        default:
            ;
    }

    return bVal;
}

void ScConditionEntry::FillCache() const
{
    if(!mpCache)
    {
        const ScRangeList& rRanges = pCondFormat->GetRange();
        mpCache.reset(new ScConditionEntryCache);
        size_t nListCount = rRanges.size();
        for( size_t i = 0; i < nListCount; i++ )
        {
            const ScRange & rRange = rRanges[i];
            SCROW nRow = rRange.aEnd.Row();
            SCCOL nCol = rRange.aEnd.Col();
            SCCOL nColStart = rRange.aStart.Col();
            SCROW nRowStart = rRange.aStart.Row();
            SCTAB nTab = rRange.aStart.Tab();

            // temporary fix to workaround slow duplicate entry
            // conditions, prevent to use a whole row
            if(nRow == MAXROW)
            {
                bool bShrunk = false;
                mpDoc->ShrinkToUsedDataArea(bShrunk, nTab, nColStart, nRowStart,
                        nCol, nRow, false);
            }

            for( SCROW r = nRowStart; r <= nRow; r++ )
                for( SCCOL c = nColStart; c <= nCol; c++ )
                {
                    ScRefCellValue aCell(*mpDoc, ScAddress(c, r, nTab));
                    if (aCell.isEmpty())
                        continue;

                    double nVal = 0.0;
                    OUString aStr;
                    if (!lcl_GetCellContent(aCell, false, nVal, aStr, mpDoc))
                    {
                        std::pair<ScConditionEntryCache::StringCacheType::iterator, bool> aResult =
                            mpCache->maStrings.emplace(aStr, 1);

                        if(!aResult.second)
                            aResult.first->second++;
                    }
                    else
                    {
                        std::pair<ScConditionEntryCache::ValueCacheType::iterator, bool> aResult =
                            mpCache->maValues.emplace(nVal, 1);

                        if(!aResult.second)
                            aResult.first->second++;

                        ++(mpCache->nValueItems);
                    }
                }
        }
    }
}

bool ScConditionEntry::IsDuplicate( double nArg, const OUString& rStr ) const
{
    FillCache();

    if(rStr.isEmpty())
    {
        ScConditionEntryCache::ValueCacheType::iterator itr = mpCache->maValues.find(nArg);
        if(itr == mpCache->maValues.end())
            return false;
        else
        {
            return itr->second > 1;
        }
    }
    else
    {
        ScConditionEntryCache::StringCacheType::iterator itr = mpCache->maStrings.find(rStr);
        if(itr == mpCache->maStrings.end())
            return false;
        else
        {
            return itr->second > 1;
        }
    }
}

bool ScConditionEntry::IsTopNElement( double nArg ) const
{
    FillCache();

    if(mpCache->nValueItems <= nVal1)
        return true;

    size_t nCells = 0;
    for(ScConditionEntryCache::ValueCacheType::const_reverse_iterator itr = mpCache->maValues.rbegin(),
            itrEnd = mpCache->maValues.rend(); itr != itrEnd; ++itr)
    {
        if(nCells >= nVal1)
            return false;
        if(itr->first <= nArg)
            return true;
        nCells += itr->second;
    }

    return true;
}

bool ScConditionEntry::IsBottomNElement( double nArg ) const
{
    FillCache();

    if(mpCache->nValueItems <= nVal1)
        return true;

    size_t nCells = 0;
    for(const auto& [rVal, rCount] : mpCache->maValues)
    {
        if(nCells >= nVal1)
            return false;
        if(rVal >= nArg)
            return true;
        nCells += rCount;
    }

    return true;
}

bool ScConditionEntry::IsTopNPercent( double nArg ) const
{
    FillCache();

    size_t nCells = 0;
    size_t nLimitCells = static_cast<size_t>(mpCache->nValueItems*nVal1/100);
    for(ScConditionEntryCache::ValueCacheType::const_reverse_iterator itr = mpCache->maValues.rbegin(),
            itrEnd = mpCache->maValues.rend(); itr != itrEnd; ++itr)
    {
        if(nCells >= nLimitCells)
            return false;
        if(itr->first <= nArg)
            return true;
        nCells += itr->second;
    }

    return true;
}

bool ScConditionEntry::IsBottomNPercent( double nArg ) const
{
    FillCache();

    size_t nCells = 0;
    size_t nLimitCells = static_cast<size_t>(mpCache->nValueItems*nVal1/100);
    for(const auto& [rVal, rCount] : mpCache->maValues)
    {
        if(nCells >= nLimitCells)
            return false;
        if(rVal >= nArg)
            return true;
        nCells += rCount;
    }

    return true;
}

bool ScConditionEntry::IsBelowAverage( double nArg, bool bEqual ) const
{
    FillCache();

    double nSum = std::accumulate(mpCache->maValues.begin(), mpCache->maValues.end(), double(0),
        [](const double& rSum, const ScConditionEntryCache::ValueCacheType::value_type& rEntry) {
            return rSum + rEntry.first * rEntry.second; });

    if(bEqual)
        return (nArg <= nSum/mpCache->nValueItems);
    else
        return (nArg < nSum/mpCache->nValueItems);
}

bool ScConditionEntry::IsAboveAverage( double nArg, bool bEqual ) const
{
    FillCache();

    double nSum = std::accumulate(mpCache->maValues.begin(), mpCache->maValues.end(), double(0),
        [](const double& rSum, const ScConditionEntryCache::ValueCacheType::value_type& rEntry) {
            return rSum + rEntry.first * rEntry.second; });

    if(bEqual)
        return (nArg >= nSum/mpCache->nValueItems);
    else
        return (nArg > nSum/mpCache->nValueItems);
}

bool ScConditionEntry::IsError( const ScAddress& rPos ) const
{
    ScRefCellValue rCell(*mpDoc, rPos);

    if (rCell.meType == CELLTYPE_FORMULA)
    {
        if (rCell.mpFormula->GetErrCode() != FormulaError::NONE)
            return true;
    }

    return false;
}

bool ScConditionEntry::IsValid( double nArg, const ScAddress& rPos ) const
{
    // Interpret must already have been called
    if ( bIsStr1 )
    {
        switch( eOp )
        {
            case ScConditionMode::BeginsWith:
            case ScConditionMode::EndsWith:
            case ScConditionMode::ContainsText:
            case ScConditionMode::NotContainsText:
                break;
            case ScConditionMode::NotEqual:
                return true;
            default:
                return false;
        }
    }

    if ( eOp == ScConditionMode::Between || eOp == ScConditionMode::NotBetween )
        if ( bIsStr2 )
            return false;

    double nComp1 = nVal1; // Copy, so that it can be changed
    double nComp2 = nVal2;

    if ( eOp == ScConditionMode::Between || eOp == ScConditionMode::NotBetween )
        if ( nComp1 > nComp2 )
        {
            // Right order for value range
            double nTemp = nComp1; nComp1 = nComp2; nComp2 = nTemp;
        }

    // All corner cases need to be tested with ::rtl::math::approxEqual!
    bool bValid = false;
    switch (eOp)
    {
        case ScConditionMode::NONE:
            break;                  // Always sal_False
        case ScConditionMode::Equal:
            bValid = ::rtl::math::approxEqual( nArg, nComp1 );
            break;
        case ScConditionMode::NotEqual:
            bValid = !::rtl::math::approxEqual( nArg, nComp1 );
            break;
        case ScConditionMode::Greater:
            bValid = ( nArg > nComp1 ) && !::rtl::math::approxEqual( nArg, nComp1 );
            break;
        case ScConditionMode::EqGreater:
            bValid = ( nArg >= nComp1 ) || ::rtl::math::approxEqual( nArg, nComp1 );
            break;
        case ScConditionMode::Less:
            bValid = ( nArg < nComp1 ) && !::rtl::math::approxEqual( nArg, nComp1 );
            break;
        case ScConditionMode::EqLess:
            bValid = ( nArg <= nComp1 ) || ::rtl::math::approxEqual( nArg, nComp1 );
            break;
        case ScConditionMode::Between:
            bValid = ( nArg >= nComp1 && nArg <= nComp2 ) ||
                     ::rtl::math::approxEqual( nArg, nComp1 ) || ::rtl::math::approxEqual( nArg, nComp2 );
            break;
        case ScConditionMode::NotBetween:
            bValid = ( nArg < nComp1 || nArg > nComp2 ) &&
                     !::rtl::math::approxEqual( nArg, nComp1 ) && !::rtl::math::approxEqual( nArg, nComp2 );
            break;
        case ScConditionMode::Duplicate:
        case ScConditionMode::NotDuplicate:
            if( pCondFormat )
            {
                bValid = IsDuplicate( nArg, OUString() );
                if( eOp == ScConditionMode::NotDuplicate )
                    bValid = !bValid;
            }
            break;
        case ScConditionMode::Direct:
            bValid = nComp1 != 0.0;
            break;
        case ScConditionMode::Top10:
            bValid = IsTopNElement( nArg );
            break;
        case ScConditionMode::Bottom10:
            bValid = IsBottomNElement( nArg );
            break;
        case ScConditionMode::TopPercent:
            bValid = IsTopNPercent( nArg );
            break;
        case ScConditionMode::BottomPercent:
            bValid = IsBottomNPercent( nArg );
            break;
        case ScConditionMode::AboveAverage:
        case ScConditionMode::AboveEqualAverage:
            bValid = IsAboveAverage( nArg, eOp == ScConditionMode::AboveEqualAverage );
            break;
        case ScConditionMode::BelowAverage:
        case ScConditionMode::BelowEqualAverage:
            bValid = IsBelowAverage( nArg, eOp == ScConditionMode::BelowEqualAverage );
            break;
        case ScConditionMode::Error:
        case ScConditionMode::NoError:
            bValid = IsError( rPos );
            if( eOp == ScConditionMode::NoError )
                bValid = !bValid;
            break;
        case ScConditionMode::BeginsWith:
            if(aStrVal1.isEmpty())
            {
                OUString aStr = OUString::number(nVal1);
                OUString aStr2 = OUString::number(nArg);
                bValid = aStr2.startsWith(aStr);
            }
            else
            {
                OUString aStr2 = OUString::number(nArg);
                bValid = aStr2.startsWith(aStrVal1);
            }
            break;
        case ScConditionMode::EndsWith:
            if(aStrVal1.isEmpty())
            {
                OUString aStr = OUString::number(nVal1);
                OUString aStr2 = OUString::number(nArg);
                bValid = aStr2.endsWith(aStr);
            }
            else
            {
                OUString aStr2 = OUString::number(nArg);
                bValid = aStr2.endsWith(aStrVal1);
            }
            break;
        case ScConditionMode::ContainsText:
        case ScConditionMode::NotContainsText:
            if(aStrVal1.isEmpty())
            {
                OUString aStr = OUString::number(nVal1);
                OUString aStr2 = OUString::number(nArg);
                bValid = aStr2.indexOf(aStr) != -1;
            }
            else
            {
                OUString aStr2 = OUString::number(nArg);
                bValid = aStr2.indexOf(aStrVal1) != -1;
            }

            if( eOp == ScConditionMode::NotContainsText )
                bValid = !bValid;
            break;
        default:
            SAL_WARN("sc", "unknown operation at ScConditionEntry");
            break;
    }
    return bValid;
}

bool ScConditionEntry::IsValidStr( const OUString& rArg, const ScAddress& rPos ) const
{
    bool bValid = false;
    // Interpret must already have been called
    if ( eOp == ScConditionMode::Direct ) // Formula is independent from the content
        return nVal1 != 0.0;

    if ( eOp == ScConditionMode::Duplicate || eOp == ScConditionMode::NotDuplicate )
    {
        if( pCondFormat && !rArg.isEmpty() )
        {
            bValid = IsDuplicate( 0.0, rArg );
            if( eOp == ScConditionMode::NotDuplicate )
                bValid = !bValid;
            return bValid;
        }
    }

    // If number contains condition, always false, except for "not equal".
    if ( !bIsStr1 && (eOp != ScConditionMode::Error && eOp != ScConditionMode::NoError) )
        return ( eOp == ScConditionMode::NotEqual );
    if ( eOp == ScConditionMode::Between || eOp == ScConditionMode::NotBetween )
        if ( !bIsStr2 )
            return false;

    OUString aUpVal1( aStrVal1 ); //TODO: As a member? (Also set in Interpret)
    OUString aUpVal2( aStrVal2 );

    if ( eOp == ScConditionMode::Between || eOp == ScConditionMode::NotBetween )
        if (ScGlobal::GetCollator()->compareString( aUpVal1, aUpVal2 ) > 0)
        {
            // Right order for value range
            OUString aTemp( aUpVal1 ); aUpVal1 = aUpVal2; aUpVal2 = aTemp;
        }

    switch ( eOp )
    {
        case ScConditionMode::Equal:
            bValid = (ScGlobal::GetCollator()->compareString(
                rArg, aUpVal1 ) == 0);
        break;
        case ScConditionMode::NotEqual:
            bValid = (ScGlobal::GetCollator()->compareString(
                rArg, aUpVal1 ) != 0);
        break;
        case ScConditionMode::TopPercent:
        case ScConditionMode::BottomPercent:
        case ScConditionMode::Top10:
        case ScConditionMode::Bottom10:
        case ScConditionMode::AboveAverage:
        case ScConditionMode::BelowAverage:
            return false;
        case ScConditionMode::Error:
        case ScConditionMode::NoError:
            bValid = IsError( rPos );
            if(eOp == ScConditionMode::NoError)
                bValid = !bValid;
        break;
        case ScConditionMode::BeginsWith:
            bValid = rArg.startsWith(aUpVal1);
        break;
        case ScConditionMode::EndsWith:
            bValid = rArg.endsWith(aUpVal1);
        break;
        case ScConditionMode::ContainsText:
        case ScConditionMode::NotContainsText:
            bValid = rArg.indexOf(aUpVal1) != -1;
            if(eOp == ScConditionMode::NotContainsText)
                bValid = !bValid;
        break;
        default:
        {
            sal_Int32 nCompare = ScGlobal::GetCollator()->compareString(
                rArg, aUpVal1 );
            switch ( eOp )
            {
                case ScConditionMode::Greater:
                    bValid = ( nCompare > 0 );
                    break;
                case ScConditionMode::EqGreater:
                    bValid = ( nCompare >= 0 );
                    break;
                case ScConditionMode::Less:
                    bValid = ( nCompare < 0 );
                    break;
                case ScConditionMode::EqLess:
                    bValid = ( nCompare <= 0 );
                    break;
                case ScConditionMode::Between:
                case ScConditionMode::NotBetween:
                    //  Test for NOTBETWEEN:
                    bValid = ( nCompare < 0 ||
                        ScGlobal::GetCollator()->compareString( rArg,
                        aUpVal2 ) > 0 );
                    if ( eOp == ScConditionMode::Between )
                        bValid = !bValid;
                    break;
                //  ScConditionMode::Direct already handled above
                default:
                    SAL_WARN("sc", "unknown operation in ScConditionEntry");
                    bValid = false;
                    break;
            }
        }
    }
    return bValid;
}

bool ScConditionEntry::IsCellValid( ScRefCellValue& rCell, const ScAddress& rPos ) const
{
    const_cast<ScConditionEntry*>(this)->Interpret(rPos); // Evaluate formula

    if ( eOp == ScConditionMode::Direct )
        return nVal1 != 0.0;

    double nArg = 0.0;
    OUString aArgStr;
    bool bVal = lcl_GetCellContent( rCell, bIsStr1, nArg, aArgStr, mpDoc );
    if (bVal)
        return IsValid( nArg, rPos );
    else
        return IsValidStr( aArgStr, rPos );
}

OUString ScConditionEntry::GetExpression( const ScAddress& rCursor, sal_uInt16 nIndex,
                                        sal_uInt32 nNumFmt,
                                        const FormulaGrammar::Grammar eGrammar ) const
{
    assert( nIndex <= 1);
    OUString aRet;

    if ( FormulaGrammar::isEnglish( eGrammar) && nNumFmt == 0 )
        nNumFmt = mpDoc->GetFormatTable()->GetStandardIndex( LANGUAGE_ENGLISH_US );

    if ( nIndex==0 )
    {
        if ( pFormula1 )
        {
            ScCompiler aComp(mpDoc, rCursor, *pFormula1, eGrammar);
            OUStringBuffer aBuffer;
            aComp.CreateStringFromTokenArray( aBuffer );
            aRet = aBuffer.makeStringAndClear();
        }
        else if (bIsStr1)
        {
            aRet = "\"";
            aRet += aStrVal1;
            aRet += "\"";
        }
        else
            mpDoc->GetFormatTable()->GetInputLineString(nVal1, nNumFmt, aRet);
    }
    else if ( nIndex==1 )
    {
        if ( pFormula2 )
        {
            ScCompiler aComp(mpDoc, rCursor, *pFormula2, eGrammar);
            OUStringBuffer aBuffer;
            aComp.CreateStringFromTokenArray( aBuffer );
            aRet = aBuffer.makeStringAndClear();
        }
        else if (bIsStr2)
        {
            aRet = "\"";
            aRet += aStrVal2;
            aRet += "\"";
        }
        else
            mpDoc->GetFormatTable()->GetInputLineString(nVal2, nNumFmt, aRet);
    }

    return aRet;
}

ScTokenArray* ScConditionEntry::CreateFlatCopiedTokenArray( sal_uInt16 nIndex ) const
{
    assert(nIndex <= 1);
    ScTokenArray* pRet = nullptr;

    if ( nIndex==0 )
    {
        if ( pFormula1 )
            pRet = new ScTokenArray( *pFormula1 );
        else
        {
            pRet = new ScTokenArray();
            if (bIsStr1)
            {
                svl::SharedStringPool& rSPool = mpDoc->GetSharedStringPool();
                pRet->AddString(rSPool.intern(aStrVal1));
            }
            else
                pRet->AddDouble( nVal1 );
        }
    }
    else if ( nIndex==1 )
    {
        if ( pFormula2 )
            pRet = new ScTokenArray( *pFormula2 );
        else
        {
            pRet = new ScTokenArray();
            if (bIsStr2)
            {
                svl::SharedStringPool& rSPool = mpDoc->GetSharedStringPool();
                pRet->AddString(rSPool.intern(aStrVal2));
            }
            else
                pRet->AddDouble( nVal2 );
        }
    }

    return pRet;
}

/**
 * Return a position that's adjusted to allow textual representation
 * of expressions if possible
 */
ScAddress ScConditionEntry::GetValidSrcPos() const
{
    SCTAB nMinTab = aSrcPos.Tab();
    SCTAB nMaxTab = nMinTab;

    for (sal_uInt16 nPass = 0; nPass < 2; nPass++)
    {
        ScTokenArray* pFormula = nPass ? pFormula2.get() : pFormula1.get();
        if (pFormula)
        {
            for ( auto t: pFormula->References() )
            {
                ScSingleRefData& rRef1 = *t->GetSingleRef();
                ScAddress aAbs = rRef1.toAbs(aSrcPos);
                if (!rRef1.IsTabDeleted())
                {
                    if (aAbs.Tab() < nMinTab)
                        nMinTab = aAbs.Tab();
                    if (aAbs.Tab() > nMaxTab)
                        nMaxTab = aAbs.Tab();
                }
                if ( t->GetType() == svDoubleRef )
                {
                    ScSingleRefData& rRef2 = t->GetDoubleRef()->Ref2;
                    aAbs = rRef2.toAbs(aSrcPos);
                    if (!rRef2.IsTabDeleted())
                    {
                        if (aAbs.Tab() < nMinTab)
                            nMinTab = aAbs.Tab();
                        if (aAbs.Tab() > nMaxTab)
                            nMaxTab = aAbs.Tab();
                    }
                }
            }
        }
    }

    ScAddress aValidPos = aSrcPos;
    SCTAB nTabCount = mpDoc->GetTableCount();
    if ( nMaxTab >= nTabCount && nMinTab > 0 )
        aValidPos.SetTab( aSrcPos.Tab() - nMinTab ); // so the lowest tab ref will be on 0

    if ( aValidPos.Tab() >= nTabCount )
        aValidPos.SetTab( nTabCount - 1 );  // ensure a valid position even if some references will be invalid

    return aValidPos;
}

void ScConditionEntry::DataChanged() const
{
    //FIXME: Nothing so far
}

bool ScConditionEntry::MarkUsedExternalReferences() const
{
    bool bAllMarked = false;
    for (sal_uInt16 nPass = 0; !bAllMarked && nPass < 2; nPass++)
    {
        ScTokenArray* pFormula = nPass ? pFormula2.get() : pFormula1.get();
        if (pFormula)
            bAllMarked = mpDoc->MarkUsedExternalReferences(*pFormula, aSrcPos);
    }
    return bAllMarked;
}

ScFormatEntry* ScConditionEntry::Clone(ScDocument* pDoc) const
{
    return new ScConditionEntry(pDoc, *this);
}

ScConditionMode ScConditionEntry::GetModeFromApi(css::sheet::ConditionOperator nOperation)
{
    ScConditionMode eMode = ScConditionMode::NONE;
    switch (static_cast<sal_Int32>(nOperation))
    {
        case css::sheet::ConditionOperator2::EQUAL:
            eMode = ScConditionMode::Equal;
            break;
        case css::sheet::ConditionOperator2::LESS:
            eMode = ScConditionMode::Less;
            break;
        case css::sheet::ConditionOperator2::GREATER:
            eMode = ScConditionMode::Greater;
            break;
        case css::sheet::ConditionOperator2::LESS_EQUAL:
            eMode = ScConditionMode::EqLess;
            break;
        case css::sheet::ConditionOperator2::GREATER_EQUAL:
            eMode = ScConditionMode::EqGreater;
            break;
        case css::sheet::ConditionOperator2::NOT_EQUAL:
            eMode = ScConditionMode::NotEqual;
            break;
        case css::sheet::ConditionOperator2::BETWEEN:
            eMode = ScConditionMode::Between;
            break;
        case css::sheet::ConditionOperator2::NOT_BETWEEN:
            eMode = ScConditionMode::NotBetween;
            break;
        case css::sheet::ConditionOperator2::FORMULA:
            eMode = ScConditionMode::Direct;
            break;
        case css::sheet::ConditionOperator2::DUPLICATE:
            eMode = ScConditionMode::Duplicate;
            break;
        case css::sheet::ConditionOperator2::NOT_DUPLICATE:
            eMode = ScConditionMode::NotDuplicate;
            break;
        default:
            break;
    }
    return eMode;
}

void ScConditionEntry::startRendering()
{
    mpCache.reset();
}

void ScConditionEntry::endRendering()
{
    mpCache.reset();
}

bool ScConditionEntry::NeedsRepaint() const
{
    return mpListener->NeedsRepaint();
}

ScCondFormatEntry::ScCondFormatEntry( ScConditionMode eOper,
                                        const OUString& rExpr1, const OUString& rExpr2,
                                        ScDocument* pDocument, const ScAddress& rPos,
                                        const OUString& rStyle,
                                        const OUString& rExprNmsp1, const OUString& rExprNmsp2,
                                        FormulaGrammar::Grammar eGrammar1,
                                        FormulaGrammar::Grammar eGrammar2 ) :
    ScConditionEntry( eOper, rExpr1, rExpr2, pDocument, rPos, rExprNmsp1, rExprNmsp2, eGrammar1, eGrammar2 ),
    aStyleName( rStyle )
{
}

ScCondFormatEntry::ScCondFormatEntry( ScConditionMode eOper,
                                        const ScTokenArray* pArr1, const ScTokenArray* pArr2,
                                        ScDocument* pDocument, const ScAddress& rPos,
                                        const OUString& rStyle ) :
    ScConditionEntry( eOper, pArr1, pArr2, pDocument, rPos ),
    aStyleName( rStyle )
{
}

ScCondFormatEntry::ScCondFormatEntry( const ScCondFormatEntry& r ) :
    ScConditionEntry( r ),
    aStyleName( r.aStyleName )
{
}

ScCondFormatEntry::ScCondFormatEntry( ScDocument* pDocument, const ScCondFormatEntry& r ) :
    ScConditionEntry( pDocument, r ),
    aStyleName( r.aStyleName )
{
}

// virtual
bool ScCondFormatEntry::IsEqual( const ScFormatEntry& r, bool bIgnoreSrcPos ) const
{
    return ScConditionEntry::IsEqual(r, bIgnoreSrcPos) &&
        (aStyleName == static_cast<const ScCondFormatEntry&>(r).aStyleName);
}

ScCondFormatEntry::~ScCondFormatEntry()
{
}

void ScCondFormatEntry::DataChanged() const
{
    if ( pCondFormat )
        pCondFormat->DoRepaint();
}

ScFormatEntry* ScCondFormatEntry::Clone( ScDocument* pDoc ) const
{
    return new ScCondFormatEntry( pDoc, *this );
}

void ScConditionEntry::CalcAll()
{
    if (pFCell1 || pFCell2)
    {
        if (pFCell1)
            pFCell1->SetDirty();
        if (pFCell2)
            pFCell2->SetDirty();
        pCondFormat->DoRepaint();
    }
}

ScCondDateFormatEntry::ScCondDateFormatEntry( ScDocument* pDoc )
    : ScFormatEntry( pDoc )
    , meType(condformat::TODAY)
{
}

ScCondDateFormatEntry::ScCondDateFormatEntry( ScDocument* pDoc, const ScCondDateFormatEntry& rFormat ):
    ScFormatEntry( pDoc ),
    meType( rFormat.meType ),
    maStyleName( rFormat.maStyleName )
{
}

bool ScCondDateFormatEntry::IsValid( const ScAddress& rPos ) const
{
    ScRefCellValue rCell(*mpDoc, rPos);

    if (!rCell.hasNumeric())
        // non-numerical cell.
        return false;

    if( !mpCache )
        mpCache.reset( new Date( Date::SYSTEM ) );

    const Date& rActDate = *mpCache;
    SvNumberFormatter* pFormatter = mpDoc->GetFormatTable();
    sal_Int32 nCurrentDate = rActDate - pFormatter->GetNullDate();

    double nVal = rCell.getValue();
    sal_Int32 nCellDate = static_cast<sal_Int32>(::rtl::math::approxFloor(nVal));
    Date aCellDate = pFormatter->GetNullDate();
    aCellDate.AddDays(nCellDate);

    switch(meType)
    {
        case condformat::TODAY:
            if( nCurrentDate == nCellDate )
                return true;
            break;
        case condformat::TOMORROW:
            if( nCurrentDate == nCellDate -1 )
                return true;
            break;
        case condformat::YESTERDAY:
            if( nCurrentDate == nCellDate + 1)
                return true;
            break;
        case condformat::LAST7DAYS:
            if( nCurrentDate >= nCellDate && nCurrentDate - 7 < nCellDate )
                return true;
            break;
        case condformat::LASTWEEK:
            {
                const DayOfWeek eDay = rActDate.GetDayOfWeek();
                if( eDay != SUNDAY )
                {
                    Date aBegin(rActDate - (8 + static_cast<sal_Int32>(eDay)));
                    Date aEnd(rActDate - (2 + static_cast<sal_Int32>(eDay)));
                    return aCellDate.IsBetween( aBegin, aEnd );
                }
                else
                {
                    Date aBegin(rActDate - 8);
                    Date aEnd(rActDate - 1);
                    return aCellDate.IsBetween( aBegin, aEnd );
                }
            }
            break;
        case condformat::THISWEEK:
            {
                const DayOfWeek eDay = rActDate.GetDayOfWeek();
                if( eDay != SUNDAY )
                {
                    Date aBegin(rActDate - (1 + static_cast<sal_Int32>(eDay)));
                    Date aEnd(rActDate + (5 - static_cast<sal_Int32>(eDay)));
                    return aCellDate.IsBetween( aBegin, aEnd );
                }
                else
                {
                    Date aEnd( rActDate + 6);
                    return aCellDate.IsBetween( rActDate, aEnd );
                }
            }
            break;
        case condformat::NEXTWEEK:
            {
                const DayOfWeek eDay = rActDate.GetDayOfWeek();
                if( eDay != SUNDAY )
                {
                    return aCellDate.IsBetween( rActDate + (6 - static_cast<sal_Int32>(eDay)),
                            rActDate + (12 - static_cast<sal_Int32>(eDay)) );
                }
                else
                {
                    return aCellDate.IsBetween( rActDate + 7, rActDate + 13 );
                }
            }
            break;
        case condformat::LASTMONTH:
            if( rActDate.GetMonth() == 1 )
            {
                if( aCellDate.GetMonth() == 12 && rActDate.GetYear() == aCellDate.GetNextYear() )
                    return true;
            }
            else if( rActDate.GetYear() == aCellDate.GetYear() )
            {
                if( rActDate.GetMonth() == aCellDate.GetMonth() + 1)
                    return true;
            }
            break;
        case condformat::THISMONTH:
            if( rActDate.GetYear() == aCellDate.GetYear() )
            {
                if( rActDate.GetMonth() == aCellDate.GetMonth() )
                    return true;
            }
            break;
        case condformat::NEXTMONTH:
            if( rActDate.GetMonth() == 12 )
            {
                if( aCellDate.GetMonth() == 1 && rActDate.GetYear() == aCellDate.GetYear() - 1 )
                    return true;
            }
            else if( rActDate.GetYear() == aCellDate.GetYear() )
            {
                if( rActDate.GetMonth() == aCellDate.GetMonth() - 1)
                    return true;
            }
            break;
        case condformat::LASTYEAR:
            if( rActDate.GetYear() == aCellDate.GetNextYear() )
                return true;
            break;
        case condformat::THISYEAR:
            if( rActDate.GetYear() == aCellDate.GetYear() )
                return true;
            break;
        case condformat::NEXTYEAR:
            if( rActDate.GetYear() == aCellDate.GetYear() - 1 )
                return true;
            break;
    }

    return false;
}

void ScCondDateFormatEntry::SetDateType( condformat::ScCondFormatDateType eType )
{
    meType = eType;
}

void ScCondDateFormatEntry::SetStyleName( const OUString& rStyleName )
{
    maStyleName = rStyleName;
}

ScFormatEntry* ScCondDateFormatEntry::Clone( ScDocument* pDoc ) const
{
    return new ScCondDateFormatEntry( pDoc, *this );
}

void ScCondDateFormatEntry::startRendering()
{
    mpCache.reset();
}

void ScCondDateFormatEntry::endRendering()
{
    mpCache.reset();
}

ScConditionalFormat::ScConditionalFormat(sal_uInt32 nNewKey, ScDocument* pDocument) :
    pDoc( pDocument ),
    nKey( nNewKey )
{
}

std::unique_ptr<ScConditionalFormat> ScConditionalFormat::Clone(ScDocument* pNewDoc) const
{
    // Real copy of the formula (for Ref Undo/between documents)
    if (!pNewDoc)
        pNewDoc = pDoc;

    std::unique_ptr<ScConditionalFormat> pNew(new ScConditionalFormat(nKey, pNewDoc));
    pNew->SetRange( maRanges );     // prerequisite for listeners

    for (const auto& rxEntry : maEntries)
    {
        ScFormatEntry* pNewEntry = rxEntry->Clone(pNewDoc);
        pNew->maEntries.push_back( std::unique_ptr<ScFormatEntry>(pNewEntry) );
        pNewEntry->SetParent(pNew.get());
    }

    return pNew;
}

bool ScConditionalFormat::EqualEntries( const ScConditionalFormat& r, bool bIgnoreSrcPos ) const
{
    if( size() != r.size())
        return false;

    //TODO: Test for same entries in reverse order?
    if (! std::equal(maEntries.begin(), maEntries.end(), r.maEntries.begin(),
        [&bIgnoreSrcPos](const std::unique_ptr<ScFormatEntry>& p1, const std::unique_ptr<ScFormatEntry>& p2) -> bool
            {
                return p1->IsEqual(*p2, bIgnoreSrcPos);
            }))
        return false;

    // right now don't check for same range
    // we only use this method to merge same conditional formats from
    // old ODF data structure
    return true;
}

void ScConditionalFormat::SetRange( const ScRangeList& rRanges )
{
    maRanges = rRanges;
    SAL_WARN_IF(maRanges.empty(), "sc", "the conditional format range is empty! will result in a crash later!");
}

void ScConditionalFormat::AddEntry( ScFormatEntry* pNew )
{
    maEntries.push_back( std::unique_ptr<ScFormatEntry>(pNew));
    pNew->SetParent(this);
}

void ScConditionalFormat::RemoveEntry(size_t n)
{
    if (n < maEntries.size())
    {
        maEntries.erase(maEntries.begin() + n);
        DoRepaint();
    }
}

bool ScConditionalFormat::IsEmpty() const
{
    return maEntries.empty();
}

size_t ScConditionalFormat::size() const
{
    return maEntries.size();
}

ScDocument* ScConditionalFormat::GetDocument()
{
    return pDoc;
}

ScConditionalFormat::~ScConditionalFormat()
{
}

const ScFormatEntry* ScConditionalFormat::GetEntry( sal_uInt16 nPos ) const
{
    if ( nPos < size() )
        return maEntries[nPos].get();
    else
        return nullptr;
}

const OUString& ScConditionalFormat::GetCellStyle( ScRefCellValue& rCell, const ScAddress& rPos ) const
{
    for (const auto& rxEntry : maEntries)
    {
        if(rxEntry->GetType() == ScFormatEntry::Type::Condition)
        {
            const ScCondFormatEntry& rEntry = static_cast<const ScCondFormatEntry&>(*rxEntry);
            if (rEntry.IsCellValid(rCell, rPos))
                return rEntry.GetStyle();
        }
        else if(rxEntry->GetType() == ScFormatEntry::Type::Date)
        {
            const ScCondDateFormatEntry& rEntry = static_cast<const ScCondDateFormatEntry&>(*rxEntry);
            if (rEntry.IsValid( rPos ))
                return rEntry.GetStyleName();
        }
    }

    return EMPTY_OUSTRING;
}

ScCondFormatData ScConditionalFormat::GetData( ScRefCellValue& rCell, const ScAddress& rPos ) const
{
    ScCondFormatData aData;
    for(const auto& rxEntry : maEntries)
    {
        if(rxEntry->GetType() == ScFormatEntry::Type::Condition && aData.aStyleName.isEmpty())
        {
            const ScCondFormatEntry& rEntry = static_cast<const ScCondFormatEntry&>(*rxEntry);
            if (rEntry.IsCellValid(rCell, rPos))
                aData.aStyleName = rEntry.GetStyle();
        }
        else if(rxEntry->GetType() == ScFormatEntry::Type::Colorscale && !aData.mxColorScale)
        {
            const ScColorScaleFormat& rEntry = static_cast<const ScColorScaleFormat&>(*rxEntry);
            aData.mxColorScale = rEntry.GetColor(rPos);
        }
        else if(rxEntry->GetType() == ScFormatEntry::Type::Databar && !aData.pDataBar)
        {
            const ScDataBarFormat& rEntry = static_cast<const ScDataBarFormat&>(*rxEntry);
            aData.pDataBar = rEntry.GetDataBarInfo(rPos);
        }
        else if(rxEntry->GetType() == ScFormatEntry::Type::Iconset && !aData.pIconSet)
        {
            const ScIconSetFormat& rEntry = static_cast<const ScIconSetFormat&>(*rxEntry);
            aData.pIconSet = rEntry.GetIconSetInfo(rPos);
        }
        else if(rxEntry->GetType() == ScFormatEntry::Type::Date && aData.aStyleName.isEmpty())
        {
            const ScCondDateFormatEntry& rEntry = static_cast<const ScCondDateFormatEntry&>(*rxEntry);
            if ( rEntry.IsValid( rPos ) )
                aData.aStyleName = rEntry.GetStyleName();
        }
    }
    return aData;
}

void ScConditionalFormat::DoRepaint()
{
    // all conditional format cells
    pDoc->RepaintRange( maRanges );
}

void ScConditionalFormat::CompileAll()
{
    for(auto& rxEntry : maEntries)
        if(rxEntry->GetType() == ScFormatEntry::Type::Condition)
            static_cast<ScCondFormatEntry&>(*rxEntry).CompileAll();
}

void ScConditionalFormat::CompileXML()
{
    for(auto& rxEntry : maEntries)
        if(rxEntry->GetType() == ScFormatEntry::Type::Condition)
            static_cast<ScCondFormatEntry&>(*rxEntry).CompileXML();
}

void ScConditionalFormat::UpdateReference( sc::RefUpdateContext& rCxt, bool bCopyAsMove )
{
    for(auto& rxEntry : maEntries)
        rxEntry->UpdateReference(rCxt);

    if (rCxt.meMode == URM_COPY && bCopyAsMove)
        maRanges.UpdateReference(URM_MOVE, pDoc, rCxt.maRange, rCxt.mnColDelta, rCxt.mnRowDelta, rCxt.mnTabDelta);
    else
        maRanges.UpdateReference(rCxt.meMode, pDoc, rCxt.maRange, rCxt.mnColDelta, rCxt.mnRowDelta, rCxt.mnTabDelta);
}

void ScConditionalFormat::InsertRow(SCTAB nTab, SCCOL nColStart, SCCOL nColEnd, SCROW nRowPos, SCSIZE nSize)
{
    maRanges.InsertRow(nTab, nColStart, nColEnd, nRowPos, nSize);
}

void ScConditionalFormat::InsertCol(SCTAB nTab, SCROW nRowStart, SCROW nRowEnd, SCCOL nColPos, SCSIZE nSize)
{
    maRanges.InsertCol(nTab, nRowStart, nRowEnd, nColPos, nSize);
}

void ScConditionalFormat::UpdateInsertTab( sc::RefUpdateInsertTabContext& rCxt )
{
    for (size_t i = 0, n = maRanges.size(); i < n; ++i)
    {
        // We assume that the start and end sheet indices are equal.
        ScRange & rRange = maRanges[i];
        SCTAB nTab = rRange.aStart.Tab();

        if (nTab < rCxt.mnInsertPos)
            // Unaffected.
            continue;

        rRange.aStart.IncTab(rCxt.mnSheets);
        rRange.aEnd.IncTab(rCxt.mnSheets);
    }

    for (auto& rxEntry : maEntries)
        rxEntry->UpdateInsertTab(rCxt);
}

void ScConditionalFormat::UpdateDeleteTab( sc::RefUpdateDeleteTabContext& rCxt )
{
    for (size_t i = 0, n = maRanges.size(); i < n; ++i)
    {
        // We assume that the start and end sheet indices are equal.
        ScRange & rRange = maRanges[i];
        SCTAB nTab = rRange.aStart.Tab();

        if (nTab < rCxt.mnDeletePos)
            // Left of the deleted sheet(s).  Unaffected.
            continue;

        if (nTab <= rCxt.mnDeletePos+rCxt.mnSheets-1)
        {
            // On the deleted sheet(s).
            rRange.aStart.SetTab(-1);
            rRange.aEnd.SetTab(-1);
            continue;
        }

        // Right of the deleted sheet(s).  Adjust the sheet indices.
        rRange.aStart.IncTab(-1*rCxt.mnSheets);
        rRange.aEnd.IncTab(-1*rCxt.mnSheets);
    }

    for (auto& rxEntry : maEntries)
        rxEntry->UpdateDeleteTab(rCxt);
}

void ScConditionalFormat::UpdateMoveTab( sc::RefUpdateMoveTabContext& rCxt )
{
    size_t n = maRanges.size();
    SCTAB nMinTab = std::min<SCTAB>(rCxt.mnOldPos, rCxt.mnNewPos);
    SCTAB nMaxTab = std::max<SCTAB>(rCxt.mnOldPos, rCxt.mnNewPos);
    for(size_t i = 0; i < n; ++i)
    {
        ScRange & rRange = maRanges[i];
        SCTAB nTab = rRange.aStart.Tab();
        if(nTab < nMinTab || nTab > nMaxTab)
        {
            continue;
        }

        if (nTab == rCxt.mnOldPos)
        {
            rRange.aStart.SetTab(rCxt.mnNewPos);
            rRange.aEnd.SetTab(rCxt.mnNewPos);
            continue;
        }

        if (rCxt.mnNewPos < rCxt.mnOldPos)
        {
            rRange.aStart.IncTab();
            rRange.aEnd.IncTab();
        }
        else
        {
            rRange.aStart.IncTab(-1);
            rRange.aEnd.IncTab(-1);
        }
    }

    for (auto& rxEntry : maEntries)
        rxEntry->UpdateMoveTab(rCxt);
}

void ScConditionalFormat::DeleteArea( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 )
{
    if (maRanges.empty())
        return;

    SCTAB nTab = maRanges[0].aStart.Tab();
    maRanges.DeleteArea( nCol1, nRow1, nTab, nCol2, nRow2, nTab );
}

void ScConditionalFormat::RenameCellStyle(const OUString& rOld, const OUString& rNew)
{
    for(const auto& rxEntry : maEntries)
        if(rxEntry->GetType() == ScFormatEntry::Type::Condition)
        {
            ScCondFormatEntry& rFormat = static_cast<ScCondFormatEntry&>(*rxEntry);
            if(rFormat.GetStyle() == rOld)
                rFormat.UpdateStyleName( rNew );
        }
}

bool ScConditionalFormat::MarkUsedExternalReferences() const
{
    bool bAllMarked = false;
    for(const auto& rxEntry : maEntries)
        if(rxEntry->GetType() == ScFormatEntry::Type::Condition)
        {
            const ScCondFormatEntry& rFormat = static_cast<const ScCondFormatEntry&>(*rxEntry);
            bAllMarked = rFormat.MarkUsedExternalReferences();
            if (bAllMarked)
                break;
        }

    return bAllMarked;
}

void ScConditionalFormat::startRendering()
{
    for(auto& rxEntry : maEntries)
    {
        rxEntry->startRendering();
    }
}

void ScConditionalFormat::endRendering()
{
    for(auto& rxEntry : maEntries)
    {
        rxEntry->endRendering();
    }
}

void ScConditionalFormat::CalcAll()
{
    for(const auto& rxEntry : maEntries)
    {
        if (rxEntry->GetType() == ScFormatEntry::Type::Condition)
        {
            ScCondFormatEntry& rFormat = static_cast<ScCondFormatEntry&>(*rxEntry);
            rFormat.CalcAll();
        }
    }
}

ScConditionalFormatList::ScConditionalFormatList(const ScConditionalFormatList& rList)
{
    for(const auto& rxFormat : rList)
        InsertNew( rxFormat->Clone() );
}

ScConditionalFormatList::ScConditionalFormatList(ScDocument* pDoc, const ScConditionalFormatList& rList)
{
    for(const auto& rxFormat : rList)
        InsertNew( rxFormat->Clone(pDoc) );
}

void ScConditionalFormatList::InsertNew( std::unique_ptr<ScConditionalFormat> pNew )
{
    m_ConditionalFormats.insert(std::move(pNew));
}

ScConditionalFormat* ScConditionalFormatList::GetFormat( sal_uInt32 nKey )
{
    auto itr = m_ConditionalFormats.find(nKey);
    if (itr != m_ConditionalFormats.end())
        return itr->get();

    SAL_WARN("sc", "ScConditionalFormatList: Entry not found");
    return nullptr;
}

const ScConditionalFormat* ScConditionalFormatList::GetFormat( sal_uInt32 nKey ) const
{
    auto itr = m_ConditionalFormats.find(nKey);
    if (itr != m_ConditionalFormats.end())
        return itr->get();

    SAL_WARN("sc", "ScConditionalFormatList: Entry not found");
    return nullptr;
}

void ScConditionalFormatList::CompileAll()
{
    for (auto const& it : m_ConditionalFormats)
    {
        it->CompileAll();
    }
}

void ScConditionalFormatList::CompileXML()
{
    for (auto const& it : m_ConditionalFormats)
    {
        it->CompileXML();
    }
}

void ScConditionalFormatList::UpdateReference( sc::RefUpdateContext& rCxt )
{
    for (auto const& it : m_ConditionalFormats)
    {
        it->UpdateReference(rCxt);
    }

    if (rCxt.meMode == URM_INSDEL)
    {
        // need to check which must be deleted
        CheckAllEntries();
    }
}

void ScConditionalFormatList::InsertRow(SCTAB nTab, SCCOL nColStart, SCCOL nColEnd, SCROW nRowPos, SCSIZE nSize)
{
    for (auto const& it : m_ConditionalFormats)
    {
        it->InsertRow(nTab, nColStart, nColEnd, nRowPos, nSize);
    }
}

void ScConditionalFormatList::InsertCol(SCTAB nTab, SCROW nRowStart, SCROW nRowEnd, SCCOL nColPos, SCSIZE nSize)
{
    for (auto const& it : m_ConditionalFormats)
    {
        it->InsertCol(nTab, nRowStart, nRowEnd, nColPos, nSize);
    }
}

void ScConditionalFormatList::UpdateInsertTab( sc::RefUpdateInsertTabContext& rCxt )
{
    for (auto const& it : m_ConditionalFormats)
    {
        it->UpdateInsertTab(rCxt);
    }
}

void ScConditionalFormatList::UpdateDeleteTab( sc::RefUpdateDeleteTabContext& rCxt )
{
    for (auto const& it : m_ConditionalFormats)
    {
        it->UpdateDeleteTab(rCxt);
    }
}

void ScConditionalFormatList::UpdateMoveTab( sc::RefUpdateMoveTabContext& rCxt )
{
    for (auto const& it : m_ConditionalFormats)
    {
        it->UpdateMoveTab(rCxt);
    }
}

void ScConditionalFormatList::RenameCellStyle( const OUString& rOld, const OUString& rNew )
{
    for (auto const& it : m_ConditionalFormats)
    {
        it->RenameCellStyle(rOld, rNew);
    }
}

bool ScConditionalFormatList::CheckAllEntries(const Link<ScConditionalFormat*,void>& rLink)
{
    bool bValid = true;

    // need to check which must be deleted
    iterator itr = m_ConditionalFormats.begin();
    while(itr != m_ConditionalFormats.end())
    {
        if ((*itr)->GetRange().empty())
        {
            bValid = false;
            if (rLink.IsSet())
                rLink.Call(itr->get());
            itr = m_ConditionalFormats.erase(itr);
        }
        else
            ++itr;
    }

    return bValid;
}

void ScConditionalFormatList::DeleteArea( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 )
{
    for (auto& rxFormat : m_ConditionalFormats)
        rxFormat->DeleteArea( nCol1, nRow1, nCol2, nRow2 );

    CheckAllEntries();
}

ScConditionalFormatList::iterator ScConditionalFormatList::begin()
{
    return m_ConditionalFormats.begin();
}

ScConditionalFormatList::const_iterator ScConditionalFormatList::begin() const
{
    return m_ConditionalFormats.begin();
}

ScConditionalFormatList::iterator ScConditionalFormatList::end()
{
    return m_ConditionalFormats.end();
}

ScConditionalFormatList::const_iterator ScConditionalFormatList::end() const
{
    return m_ConditionalFormats.end();
}

ScRangeList ScConditionalFormatList::GetCombinedRange() const
{
    ScRangeList aRange;
    for (auto& itr: m_ConditionalFormats)
    {
        const ScRangeList& rRange = itr->GetRange();
        for (size_t i = 0, n = rRange.size(); i < n; ++i)
        {
            aRange.Join(rRange[i]);
        }
    }
    return aRange;
}

void ScConditionalFormatList::RemoveFromDocument(ScDocument* pDoc) const
{
    ScRangeList aRange = GetCombinedRange();
    ScMarkData aMark;
    aMark.MarkFromRangeList(aRange, true);
    sal_uInt16 const pItems[2] = { sal_uInt16(ATTR_CONDITIONAL),0};
    pDoc->ClearSelectionItems(pItems, aMark);
}

void ScConditionalFormatList::AddToDocument(ScDocument* pDoc) const
{
    for (auto& itr: m_ConditionalFormats)
    {
        const ScRangeList& rRange = itr->GetRange();
        if (rRange.empty())
            continue;

        SCTAB nTab = rRange.front().aStart.Tab();
        pDoc->AddCondFormatData(rRange, nTab, itr->GetKey());
    }
}

size_t ScConditionalFormatList::size() const
{
    return m_ConditionalFormats.size();
}

bool ScConditionalFormatList::empty() const
{
    return m_ConditionalFormats.empty();
}

void ScConditionalFormatList::erase( sal_uLong nIndex )
{
    auto itr = m_ConditionalFormats.find(nIndex);
    if (itr != end())
        m_ConditionalFormats.erase(itr);
}

void ScConditionalFormatList::startRendering()
{
    for (auto const& it : m_ConditionalFormats)
    {
        it->startRendering();
    }
}

void ScConditionalFormatList::endRendering()
{
    for (auto const& it : m_ConditionalFormats)
    {
        it->endRendering();
    }
}

void ScConditionalFormatList::clear()
{
    m_ConditionalFormats.clear();
}

sal_uInt32 ScConditionalFormatList::getMaxKey() const
{
    if (m_ConditionalFormats.empty())
        return 0;
    return (*m_ConditionalFormats.rbegin())->GetKey();
}

void ScConditionalFormatList::CalcAll()
{
    for (const auto& aEntry : m_ConditionalFormats)
    {
        aEntry->CalcAll();
    }

}

ScCondFormatData::ScCondFormatData() {}

ScCondFormatData::ScCondFormatData(ScCondFormatData&&) = default;

ScCondFormatData::~ScCondFormatData() {}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
