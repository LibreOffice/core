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

#include "scitems.hxx"
#include <sfx2/objsh.hxx>
#include <svl/itemset.hxx>
#include <svl/zforlist.hxx>
#include <rtl/math.hxx>
#include <unotools/collatorwrapper.hxx>

#include <com/sun/star/sheet/ConditionOperator2.hpp>

#include "conditio.hxx"
#include "formulacell.hxx"
#include "document.hxx"
#include "hints.hxx"
#include "compiler.hxx"
#include "rechead.hxx"
#include "rangelst.hxx"
#include "stlpool.hxx"
#include "rangenam.hxx"
#include "colorscale.hxx"
#include "cellvalue.hxx"
#include "editutil.hxx"
#include "tokenarray.hxx"

using namespace formula;
//------------------------------------------------------------------------

ScFormatEntry::ScFormatEntry(ScDocument* pDoc):
    mpDoc(pDoc)
{
}

bool ScFormatEntry::operator==( const ScFormatEntry& r ) const
{
    if(GetType() != r.GetType())
        return false;

    switch(GetType())
    {
        case condformat::CONDITION:
            return static_cast<const ScCondFormatEntry&>(*this) == static_cast<const ScCondFormatEntry&>(r);
        default:
            // TODO: implement also this case
            // actually return false for these cases is not that bad
            // as soon as databar and color scale are tested we need
            // to think about the range
            return false;
    }
}

void ScFormatEntry::startRendering()
{
}

void ScFormatEntry::endRendering()
{
}

static bool lcl_HasRelRef( ScDocument* pDoc, ScTokenArray* pFormula, sal_uInt16 nRecursion = 0 )
{
    if (pFormula)
    {
        pFormula->Reset();
        FormulaToken* t;
        for( t = pFormula->Next(); t; t = pFormula->Next() )
        {
            switch( t->GetType() )
            {
                case svDoubleRef:
                {
                    ScSingleRefData& rRef2 = static_cast<ScToken*>(t)->GetDoubleRef().Ref2;
                    if ( rRef2.IsColRel() || rRef2.IsRowRel() || rRef2.IsTabRel() )
                        return true;
                }
                // fall through

                case svSingleRef:
                {
                    ScSingleRefData& rRef1 = static_cast<ScToken*>(t)->GetSingleRef();
                    if ( rRef1.IsColRel() || rRef1.IsRowRel() || rRef1.IsTabRel() )
                        return true;
                }
                break;

                case svIndex:
                {
                    if( t->GetOpCode() == ocName )      // DB areas always absolute
                        if( ScRangeData* pRangeData = pDoc->GetRangeName()->findByIndex( t->GetIndex() ) )
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
                        case ocTable:   // SHEET() returns own sheet index
                        case ocCell:    // CELL() may return own cell address
                            return true;
//                        break;
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
    pFormula1(NULL),
    pFormula2(NULL),
    aSrcPos(r.aSrcPos),
    aSrcString(r.aSrcString),
    pFCell1(NULL),
    pFCell2(NULL),
    bRelRef1(r.bRelRef1),
    bRelRef2(r.bRelRef2),
    bFirstRun(true),
    pCondFormat(r.pCondFormat)
{
    //  ScTokenArray copy ctor erzeugt flache Kopie

    if (r.pFormula1)
        pFormula1 = new ScTokenArray( *r.pFormula1 );
    if (r.pFormula2)
        pFormula2 = new ScTokenArray( *r.pFormula2 );

    //  Formelzellen werden erst bei IsValid angelegt
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
    pFormula1(NULL),
    pFormula2(NULL),
    aSrcPos(r.aSrcPos),
    aSrcString(r.aSrcString),
    pFCell1(NULL),
    pFCell2(NULL),
    bRelRef1(r.bRelRef1),
    bRelRef2(r.bRelRef2),
    bFirstRun(true),
    pCondFormat(r.pCondFormat)
{
    // echte Kopie der Formeln (fuer Ref-Undo)

    if (r.pFormula1)
        pFormula1 = r.pFormula1->Clone();
    if (r.pFormula2)
        pFormula2 = r.pFormula2->Clone();

    //  Formelzellen werden erst bei IsValid angelegt
    //! im Clipboard nicht - dann vorher interpretieren !!!
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
    pFormula1(NULL),
    pFormula2(NULL),
    aSrcPos(rPos),
    pFCell1(NULL),
    pFCell2(NULL),
    bRelRef1(false),
    bRelRef2(false),
    bFirstRun(true),
    pCondFormat(NULL)
{
    Compile( rExpr1, rExpr2, rExprNmsp1, rExprNmsp2, eGrammar1, eGrammar2, false );

    //  Formelzellen werden erst bei IsValid angelegt
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
    pFormula1(NULL),
    pFormula2(NULL),
    aSrcPos(rPos),
    pFCell1(NULL),
    pFCell2(NULL),
    bRelRef1(false),
    bRelRef2(false),
    bFirstRun(true),
    pCondFormat(NULL)
{
    if ( pArr1 )
    {
        pFormula1 = new ScTokenArray( *pArr1 );
        if ( pFormula1->GetLen() == 1 )
        {
            // einzelne (konstante Zahl) ?
            FormulaToken* pToken = pFormula1->First();
            if ( pToken->GetOpCode() == ocPush )
            {
                if ( pToken->GetType() == svDouble )
                {
                    nVal1 = pToken->GetDouble();
                    DELETEZ(pFormula1);             // nicht als Formel merken
                }
                else if ( pToken->GetType() == svString )
                {
                    bIsStr1 = true;
                    aStrVal1 = pToken->GetString();
                    DELETEZ(pFormula1);             // nicht als Formel merken
                }
            }
        }
        bRelRef1 = lcl_HasRelRef( mpDoc, pFormula1 );
    }
    if ( pArr2 )
    {
        pFormula2 = new ScTokenArray( *pArr2 );
        if ( pFormula2->GetLen() == 1 )
        {
            // einzelne (konstante Zahl) ?
            FormulaToken* pToken = pFormula2->First();
            if ( pToken->GetOpCode() == ocPush )
            {
                if ( pToken->GetType() == svDouble )
                {
                    nVal2 = pToken->GetDouble();
                    DELETEZ(pFormula2);             // nicht als Formel merken
                }
                else if ( pToken->GetType() == svString )
                {
                    bIsStr2 = true;
                    aStrVal2 = pToken->GetString();
                    DELETEZ(pFormula2);             // nicht als Formel merken
                }
            }
        }
        bRelRef2 = lcl_HasRelRef( mpDoc, pFormula2 );
    }

    //  formula cells are created at IsValid
}

ScConditionEntry::~ScConditionEntry()
{
    delete pFCell1;
    delete pFCell2;

    delete pFormula1;
    delete pFormula2;
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
            aComp.SetGrammar( eGrammar1 );
            if ( mpDoc->IsImportingXML() && !bTextToReal )
            {
                //  temporary formula string as string tokens
                //! merge with lcl_ScDocFunc_CreateTokenArrayXML
                pFormula1 = new ScTokenArray;
                pFormula1->AddStringXML( rExpr1 );
                // bRelRef1 is set when the formula is compiled again (CompileXML)
            }
            else
            {
                pFormula1 = aComp.CompileString( rExpr1, rExprNmsp1 );
                if ( pFormula1->GetLen() == 1 )
                {
                    // einzelne (konstante Zahl) ?
                    FormulaToken* pToken = pFormula1->First();
                    if ( pToken->GetOpCode() == ocPush )
                    {
                        if ( pToken->GetType() == svDouble )
                        {
                            nVal1 = pToken->GetDouble();
                            DELETEZ(pFormula1);             // nicht als Formel merken
                        }
                        else if ( pToken->GetType() == svString )
                        {
                            bIsStr1 = true;
                            aStrVal1 = pToken->GetString();
                            DELETEZ(pFormula1);             // nicht als Formel merken
                        }
                    }
                }
                bRelRef1 = lcl_HasRelRef( mpDoc, pFormula1 );
            }
        }

        if ( !rExpr2.isEmpty() )
        {
            aComp.SetGrammar( eGrammar2 );
            if ( mpDoc->IsImportingXML() && !bTextToReal )
            {
                //  temporary formula string as string tokens
                //! merge with lcl_ScDocFunc_CreateTokenArrayXML
                pFormula2 = new ScTokenArray;
                pFormula2->AddStringXML( rExpr2 );
                // bRelRef2 is set when the formula is compiled again (CompileXML)
            }
            else
            {
                pFormula2 = aComp.CompileString( rExpr2, rExprNmsp2 );
                if ( pFormula2->GetLen() == 1 )
                {
                    // einzelne (konstante Zahl) ?
                    FormulaToken* pToken = pFormula2->First();
                    if ( pToken->GetOpCode() == ocPush )
                    {
                        if ( pToken->GetType() == svDouble )
                        {
                            nVal2 = pToken->GetDouble();
                            DELETEZ(pFormula2);             // nicht als Formel merken
                        }
                        else if ( pToken->GetType() == svString )
                        {
                            bIsStr2 = true;
                            aStrVal2 = pToken->GetString();
                            DELETEZ(pFormula2);             // nicht als Formel merken
                        }
                    }
                }
                bRelRef2 = lcl_HasRelRef( mpDoc, pFormula2 );
            }
        }
    }
}

void ScConditionEntry::MakeCells( const ScAddress& rPos )           // Formelzellen anlegen
{
    if ( !mpDoc->IsClipOrUndo() )            // nie im Clipboard rechnen!
    {
        if ( pFormula1 && !pFCell1 && !bRelRef1 )
        {
            pFCell1 = new ScFormulaCell( mpDoc, rPos, pFormula1 );
            pFCell1->StartListeningTo( mpDoc );
        }

        if ( pFormula2 && !pFCell2 && !bRelRef2 )
        {
            pFCell2 = new ScFormulaCell( mpDoc, rPos, pFormula2 );
            pFCell2->StartListeningTo( mpDoc );
        }
    }
}

void ScConditionEntry::SetIgnoreBlank(bool bSet)
{
    //  Das Bit SC_COND_NOBLANKS wird gesetzt, wenn Blanks nicht ignoriert werden
    //  (nur bei Gueltigkeit)

    if (bSet)
        nOptions &= ~SC_COND_NOBLANKS;
    else
        nOptions |= SC_COND_NOBLANKS;
}

void ScConditionEntry::CompileAll()
{
    //  Formelzellen loeschen, dann wird beim naechsten IsValid neu kompiliert

    DELETEZ(pFCell1);
    DELETEZ(pFCell2);
}

void ScConditionEntry::CompileXML()
{
    //  First parse the formula source position if it was stored as text

    if ( !aSrcString.isEmpty() )
    {
        ScAddress aNew;
        /* XML is always in OOo:A1 format, although R1C1 would be more amenable
         * to compression */
        if ( aNew.Parse( aSrcString, mpDoc ) & SCA_VALID )
            aSrcPos = aNew;
        // if the position is invalid, there isn't much we can do at this time
        aSrcString = OUString();
    }

    //  Convert the text tokens that were created during XML import into real tokens.

    Compile( GetExpression(aSrcPos, 0, 0, eTempGrammar1),
             GetExpression(aSrcPos, 1, 0, eTempGrammar2),
             aStrNmsp1, aStrNmsp2, eTempGrammar1, eTempGrammar2, true );
}

void ScConditionEntry::SetSrcString( const OUString& rNew )
{
    // aSrcString is only evaluated in CompileXML
    SAL_WARN_IF( !mpDoc->IsImportingXML(), "sc", "SetSrcString is only valid for XML import" );

    aSrcString = rNew;
}

void ScConditionEntry::SetFormula1( const ScTokenArray& rArray )
{
    DELETEZ( pFormula1 );
    if( rArray.GetLen() > 0 )
    {
        pFormula1 = new ScTokenArray( rArray );
        bRelRef1 = lcl_HasRelRef( mpDoc, pFormula1 );
    }
}

void ScConditionEntry::SetFormula2( const ScTokenArray& rArray )
{
    DELETEZ( pFormula2 );
    if( rArray.GetLen() > 0 )
    {
        pFormula2 = new ScTokenArray( rArray );
        bRelRef2 = lcl_HasRelRef( mpDoc, pFormula2 );
    }
}

static void lcl_CondUpdateInsertTab( ScTokenArray& rCode, SCTAB nInsTab, bool& rChanged, SCTAB nTabs )
{
    //  Insert table: only update absolute table references.
    //  (Similar to ScCompiler::UpdateInsertTab with bIsName=true, result is the same as for named ranges)
    //  For deleting, ScCompiler::UpdateDeleteTab is used because of the handling of invalid references.

    rCode.Reset();
    ScToken* p = static_cast<ScToken*>(rCode.GetNextReference());
    while( p )
    {
        ScSingleRefData& rRef1 = p->GetSingleRef();
        if (!rRef1.IsTabRel() && nInsTab <= rRef1.Tab())
        {
            rRef1.SetAbsTab(rRef1.Tab() + nTabs);
            rChanged = true;
        }
        if( p->GetType() == svDoubleRef )
        {
            ScSingleRefData& rRef2 = p->GetDoubleRef().Ref2;
            if (!rRef2.IsTabRel() && nInsTab <= rRef2.Tab())
            {
                rRef2.SetAbsTab(rRef2.Tab() + nTabs);
                rChanged = true;
            }
        }
        p = static_cast<ScToken*>(rCode.GetNextReference());
    }
}

void ScConditionEntry::UpdateReference( UpdateRefMode eUpdateRefMode,
                                const ScRange& rRange, SCsCOL nDx, SCsROW nDy, SCsTAB nDz )
{
    bool bInsertTab = ( eUpdateRefMode == URM_INSDEL && nDz >= 1 );
    bool bDeleteTab = ( eUpdateRefMode == URM_INSDEL && nDz <= -1 );
    if(pCondFormat)
        aSrcPos = pCondFormat->GetRange().Combine().aStart;
    ScAddress aOldSrcPos = aSrcPos;
    bool bChangedPos = false;
    if(eUpdateRefMode == URM_INSDEL && rRange.In(aSrcPos))
    {
        aSrcPos.Move(nDx, nDy, nDz);
        bChangedPos = aSrcPos != aOldSrcPos;
    }

    if (pFormula1)
    {
        bool bChanged1 = false;
        if ( bInsertTab )
            lcl_CondUpdateInsertTab( *pFormula1, rRange.aStart.Tab(), bChanged1, nDz );
        else
        {
            if ( bDeleteTab )
                pFormula1->AdjustReferenceOnDeletedTab(rRange.aStart.Tab(), static_cast<SCTAB>(-1*nDz), aSrcPos);
            else
            {
                ScCompiler aComp( mpDoc, aSrcPos, *pFormula1 );
                aComp.SetGrammar(mpDoc->GetGrammar());
                bool bSizeChanged;
                aComp.UpdateReference( eUpdateRefMode, aOldSrcPos, rRange, nDx,
                        nDy, nDz, bChanged1, bSizeChanged );
            }
        }

        if (bChanged1 || bChangedPos)
            DELETEZ(pFCell1);       // is created again in IsValid
    }
    if (pFormula2)
    {
        bool bChanged2 = false;
        if ( bInsertTab )
            lcl_CondUpdateInsertTab( *pFormula2, rRange.aStart.Tab(), bChanged2, nDz );
        else
        {
            if ( bDeleteTab )
                pFormula2->AdjustReferenceOnDeletedTab(rRange.aStart.Tab(), static_cast<SCTAB>(-1*nDz), aSrcPos);
            else
            {
                ScCompiler aComp( mpDoc, aSrcPos, *pFormula2);
                aComp.SetGrammar(mpDoc->GetGrammar());
                bool bSizeChanged;
                aComp.UpdateReference( eUpdateRefMode, aOldSrcPos, rRange, nDx,
                        nDy, nDz, bChanged2, bSizeChanged );
            }
        }

        if (bChanged2 || bChangedPos)
            DELETEZ(pFCell2);       // is created again in IsValid
    }
}

void ScConditionEntry::UpdateMoveTab( SCTAB nOldPos, SCTAB nNewPos )
{
    if (pFormula1)
    {
        pFormula1->AdjustReferenceOnMovedTab(nOldPos, nNewPos, aSrcPos);
        DELETEZ(pFCell1);
    }

    if (pFormula2)
    {
        pFormula2->AdjustReferenceOnMovedTab(nOldPos, nNewPos, aSrcPos);
        DELETEZ(pFCell2);
    }
}

//! als Vergleichsoperator ans TokenArray ???

static bool lcl_IsEqual( const ScTokenArray* pArr1, const ScTokenArray* pArr2 )
{
    //  verglichen wird nur das nicht-UPN Array

    if ( pArr1 && pArr2 )
    {
        sal_uInt16 nLen = pArr1->GetLen();
        if ( pArr2->GetLen() != nLen )
            return false;

        FormulaToken** ppToken1 = pArr1->GetArray();
        FormulaToken** ppToken2 = pArr2->GetArray();
        for (sal_uInt16 i=0; i<nLen; i++)
        {
            if ( ppToken1[i] != ppToken2[i] &&
                 !(*ppToken1[i] == *ppToken2[i]) )
                return false;                       // Unterschied
        }
        return true;                    // alle Eintraege gleich
    }
    else
        return !pArr1 && !pArr2;        // beide 0 -> gleich
}

int ScConditionEntry::operator== ( const ScConditionEntry& r ) const
{
    bool bEq = (eOp == r.eOp && nOptions == r.nOptions &&
                lcl_IsEqual( pFormula1, r.pFormula1 ) &&
                lcl_IsEqual( pFormula2, r.pFormula2 ));
    if (bEq)
    {
        // for formulas, the reference positions must be compared, too
        // (including aSrcString, for inserting the entries during XML import)
        if ( ( pFormula1 || pFormula2 ) && ( aSrcPos != r.aSrcPos || aSrcString != r.aSrcString ) )
            bEq = false;

        //  wenn keine Formeln, Werte vergleichen
        if ( !pFormula1 && ( nVal1 != r.nVal1 || aStrVal1 != r.aStrVal1 || bIsStr1 != r.bIsStr1 ) )
            bEq = false;
        if ( !pFormula2 && ( nVal2 != r.nVal2 || aStrVal2 != r.aStrVal2 || bIsStr2 != r.bIsStr2 ) )
            bEq = false;
    }

    return bEq;
}

void ScConditionEntry::Interpret( const ScAddress& rPos )
{
    //  Formelzellen anlegen
    //  dabei koennen neue Broadcaster (Note-Zellen) ins Dokument eingefuegt werden !!!!

    if ( ( pFormula1 && !pFCell1 ) || ( pFormula2 && !pFCell2 ) )
        MakeCells( rPos );

    //  Formeln auswerten

    bool bDirty = false;        //! 1 und 2 getrennt ???

    ScFormulaCell* pTemp1 = NULL;
    ScFormulaCell* pEff1 = pFCell1;
    if ( bRelRef1 )
    {
        pTemp1 = new ScFormulaCell( mpDoc, rPos, pFormula1 );    // ohne Listening
        pEff1 = pTemp1;
    }
    if ( pEff1 )
    {
        if (!pEff1->IsRunning())        // keine 522 erzeugen
        {
            //! Changed statt Dirty abfragen !!!
            if (pEff1->GetDirty() && !bRelRef1 && mpDoc->GetAutoCalc())
                bDirty = true;
            if (pEff1->IsValue())
            {
                bIsStr1 = false;
                nVal1 = pEff1->GetValue();
                aStrVal1 = OUString();
            }
            else
            {
                bIsStr1 = true;
                aStrVal1 = pEff1->GetString();
                nVal1 = 0.0;
            }
        }
    }
    delete pTemp1;

    ScFormulaCell* pTemp2 = NULL;
    ScFormulaCell* pEff2 = pFCell2; //@ 1!=2
    if ( bRelRef2 )
    {
        pTemp2 = new ScFormulaCell( mpDoc, rPos, pFormula2 );    // ohne Listening
        pEff2 = pTemp2;
    }
    if ( pEff2 )
    {
        if (!pEff2->IsRunning())        // keine 522 erzeugen
        {
            if (pEff2->GetDirty() && !bRelRef2 && mpDoc->GetAutoCalc())
                bDirty = true;
            if (pEff2->IsValue())
            {
                bIsStr2 = false;
                nVal2 = pEff2->GetValue();
                aStrVal2 = OUString();
            }
            else
            {
                bIsStr2 = true;
                aStrVal2 = pEff2->GetString();
                nVal2 = 0.0;
            }
        }
    }
    delete pTemp2;

    //  wenn IsRunning, bleiben die letzten Werte erhalten

    if (bDirty && !bFirstRun)
    {
        //  bei bedingten Formaten neu painten

        DataChanged( NULL );    // alles
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
                rArgStr = rCell.mpFormula->GetString();
        }
        break;
        case CELLTYPE_STRING:
        case CELLTYPE_EDIT:
            bVal = false;
            if (rCell.meType == CELLTYPE_STRING)
                rArgStr = *rCell.mpString;
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
            const ScRange *aRange = rRanges[i];
            SCROW nRow = aRange->aEnd.Row();
            SCCOL nCol = aRange->aEnd.Col();
            SCCOL nColStart = aRange->aStart.Col();
            SCROW nRowStart = aRange->aStart.Row();
            SCTAB nTab = aRange->aStart.Tab();

            // temporary fix to workaorund slow duplicate entry
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
                    ScRefCellValue aCell;
                    aCell.assign(*mpDoc, ScAddress(c, r, nTab));
                    if (aCell.isEmpty())
                        continue;

                    double nVal = 0.0;
                    OUString aStr;
                    if (!lcl_GetCellContent(aCell, false, nVal, aStr, mpDoc))
                    {
                        std::pair<ScConditionEntryCache::StringCacheType::iterator, bool> aResult =
                            mpCache->maStrings.insert(
                                ScConditionEntryCache::StringCacheType::value_type(aStr, 1));

                        if(!aResult.second)
                            aResult.first->second++;
                    }
                    else
                    {
                        std::pair<ScConditionEntryCache::ValueCacheType::iterator, bool> aResult =
                            mpCache->maValues.insert(
                                ScConditionEntryCache::ValueCacheType::value_type(nVal, 1));

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
            if(itr->second > 1)
                return true;
            else
                return false;
        }
    }
    else
    {
        ScConditionEntryCache::StringCacheType::iterator itr = mpCache->maStrings.find(rStr);
        if(itr == mpCache->maStrings.end())
            return false;
        else
        {
            if(itr->second > 1)
                return true;
            else
                return false;
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
    for(ScConditionEntryCache::ValueCacheType::const_iterator itr = mpCache->maValues.begin(),
            itrEnd = mpCache->maValues.end(); itr != itrEnd; ++itr)
    {
        if(nCells >= nVal1)
            return false;
        if(itr->first >= nArg)
            return true;
        nCells += itr->second;
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
    for(ScConditionEntryCache::ValueCacheType::const_iterator itr = mpCache->maValues.begin(),
            itrEnd = mpCache->maValues.end(); itr != itrEnd; ++itr)
    {
        if(nCells >= nLimitCells)
            return false;
        if(itr->first >= nArg)
            return true;
        nCells += itr->second;
    }

    return true;
}

bool ScConditionEntry::IsBelowAverage( double nArg, bool bEqual ) const
{
    FillCache();

    double nSum = 0;
    for(ScConditionEntryCache::ValueCacheType::const_iterator itr = mpCache->maValues.begin(),
            itrEnd = mpCache->maValues.end(); itr != itrEnd; ++itr)
    {
        nSum += itr->first * itr->second;
    }

    if(bEqual)
        return (nArg <= nSum/mpCache->nValueItems);
    else
        return (nArg < nSum/mpCache->nValueItems);
}

bool ScConditionEntry::IsAboveAverage( double nArg, bool bEqual ) const
{
    FillCache();

    double nSum = 0;
    for(ScConditionEntryCache::ValueCacheType::const_iterator itr = mpCache->maValues.begin(),
            itrEnd = mpCache->maValues.end(); itr != itrEnd; ++itr)
    {
        nSum += itr->first * itr->second;
    }

    if(bEqual)
        return (nArg >= nSum/mpCache->nValueItems);
    else
        return (nArg > nSum/mpCache->nValueItems);
}

bool ScConditionEntry::IsError( const ScAddress& rPos ) const
{
    switch (mpDoc->GetCellType(rPos))
    {
        case CELLTYPE_VALUE:
            return false;
        case CELLTYPE_FORMULA:
        {
            ScFormulaCell* pFormulaCell = const_cast<ScFormulaCell*>(mpDoc->GetFormulaCell(rPos));
            if(pFormulaCell->GetErrCode())
                return true;
        }
        case CELLTYPE_STRING:
        case CELLTYPE_EDIT:
            return false;
        default:
            break;
    }
    return false;
}

bool ScConditionEntry::IsValid( double nArg, const ScAddress& rPos ) const
{
    //  Interpret muss schon gerufen sein

    if ( bIsStr1 )
    {
        switch( eOp )
        {
            case SC_COND_BEGINS_WITH:
            case SC_COND_ENDS_WITH:
            case SC_COND_CONTAINS_TEXT:
            case SC_COND_NOT_CONTAINS_TEXT:
                break;
            case SC_COND_NOTEQUAL:
                return true;
            default:
                return false;
        }
    }

    if ( eOp == SC_COND_BETWEEN || eOp == SC_COND_NOTBETWEEN )
        if ( bIsStr2 )
            return false;

    double nComp1 = nVal1;      // Kopie, damit vertauscht werden kann
    double nComp2 = nVal2;

    if ( eOp == SC_COND_BETWEEN || eOp == SC_COND_NOTBETWEEN )
        if ( nComp1 > nComp2 )
        {
            //  richtige Reihenfolge fuer Wertebereich
            double nTemp = nComp1; nComp1 = nComp2; nComp2 = nTemp;
        }

    //  Alle Grenzfaelle muessen per ::rtl::math::approxEqual getestet werden!

    bool bValid = false;
    switch (eOp)
    {
        case SC_COND_NONE:
            break;                  // immer sal_False;
        case SC_COND_EQUAL:
            bValid = ::rtl::math::approxEqual( nArg, nComp1 );
            break;
        case SC_COND_NOTEQUAL:
            bValid = !::rtl::math::approxEqual( nArg, nComp1 );
            break;
        case SC_COND_GREATER:
            bValid = ( nArg > nComp1 ) && !::rtl::math::approxEqual( nArg, nComp1 );
            break;
        case SC_COND_EQGREATER:
            bValid = ( nArg >= nComp1 ) || ::rtl::math::approxEqual( nArg, nComp1 );
            break;
        case SC_COND_LESS:
            bValid = ( nArg < nComp1 ) && !::rtl::math::approxEqual( nArg, nComp1 );
            break;
        case SC_COND_EQLESS:
            bValid = ( nArg <= nComp1 ) || ::rtl::math::approxEqual( nArg, nComp1 );
            break;
        case SC_COND_BETWEEN:
            bValid = ( nArg >= nComp1 && nArg <= nComp2 ) ||
                     ::rtl::math::approxEqual( nArg, nComp1 ) || ::rtl::math::approxEqual( nArg, nComp2 );
            break;
        case SC_COND_NOTBETWEEN:
            bValid = ( nArg < nComp1 || nArg > nComp2 ) &&
                     !::rtl::math::approxEqual( nArg, nComp1 ) && !::rtl::math::approxEqual( nArg, nComp2 );
            break;
        case SC_COND_DUPLICATE:
        case SC_COND_NOTDUPLICATE:
            if( pCondFormat )
            {
                bValid = IsDuplicate( nArg, OUString() );
                if( eOp == SC_COND_NOTDUPLICATE )
                    bValid = !bValid;
            }
            break;
        case SC_COND_DIRECT:
            bValid = !::rtl::math::approxEqual( nComp1, 0.0 );
            break;
        case SC_COND_TOP10:
            bValid = IsTopNElement( nArg );
            break;
        case SC_COND_BOTTOM10:
            bValid = IsBottomNElement( nArg );
            break;
        case SC_COND_TOP_PERCENT:
            bValid = IsTopNPercent( nArg );
            break;
        case SC_COND_BOTTOM_PERCENT:
            bValid = IsBottomNPercent( nArg );
            break;
        case SC_COND_ABOVE_AVERAGE:
        case SC_COND_ABOVE_EQUAL_AVERAGE:
            bValid = IsAboveAverage( nArg, eOp == SC_COND_ABOVE_EQUAL_AVERAGE );
            break;
        case SC_COND_BELOW_AVERAGE:
        case SC_COND_BELOW_EQUAL_AVERAGE:
            bValid = IsBelowAverage( nArg, eOp == SC_COND_BELOW_EQUAL_AVERAGE );
            break;
        case SC_COND_ERROR:
        case SC_COND_NOERROR:
            bValid = IsError( rPos );
            if( eOp == SC_COND_NOERROR )
                bValid = !bValid;
            break;
        case SC_COND_BEGINS_WITH:
            if(aStrVal1.isEmpty())
            {
                OUString aStr = OUString::valueOf(nVal1);
                OUString aStr2 = OUString::valueOf(nArg);
                bValid = aStr2.indexOf(aStr) == 0;
            }
            else
            {
                OUString aStr2 = OUString::valueOf(nArg);
                bValid = aStr2.indexOf(aStrVal1) == 0;
            }
            break;
        case SC_COND_ENDS_WITH:
            if(aStrVal1.isEmpty())
            {
                OUString aStr = OUString::valueOf(nVal1);
                OUString aStr2 = OUString::valueOf(nArg);
                bValid = aStr2.endsWith(aStr) == 0;
            }
            else
            {
                OUString aStr2 = OUString::valueOf(nArg);
                bValid = aStr2.endsWith(aStrVal1) == 0;
            }
            break;
        case SC_COND_CONTAINS_TEXT:
        case SC_COND_NOT_CONTAINS_TEXT:
            if(aStrVal1.isEmpty())
            {
                OUString aStr = OUString::valueOf(nVal1);
                OUString aStr2 = OUString::valueOf(nArg);
                bValid = aStr2.indexOf(aStr) != -1;
            }
            else
            {
                OUString aStr2 = OUString::valueOf(nArg);
                bValid = aStr2.indexOf(aStrVal1) != -1;
            }

            if( eOp == SC_COND_NOT_CONTAINS_TEXT )
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
    //  Interpret muss schon gerufen sein

    if ( eOp == SC_COND_DIRECT )                // Formel ist unabhaengig vom Inhalt
        return !::rtl::math::approxEqual( nVal1, 0.0 );

    if ( eOp == SC_COND_DUPLICATE || eOp == SC_COND_NOTDUPLICATE )
    {
        if( pCondFormat && !rArg.isEmpty() )
        {
            bValid = IsDuplicate( 0.0, rArg );
            if( eOp == SC_COND_NOTDUPLICATE )
                bValid = !bValid;
            return bValid;
        }
    }

    // If number contains condition, always false, except for "not equal".

    if ( !bIsStr1 && (eOp != SC_COND_ERROR && eOp != SC_COND_NOERROR) )
        return ( eOp == SC_COND_NOTEQUAL );
    if ( eOp == SC_COND_BETWEEN || eOp == SC_COND_NOTBETWEEN )
        if ( !bIsStr2 )
            return false;

    OUString aUpVal1( aStrVal1 );     //! als Member? (dann auch in Interpret setzen)
    OUString aUpVal2( aStrVal2 );

    if ( eOp == SC_COND_BETWEEN || eOp == SC_COND_NOTBETWEEN )
        if ( ScGlobal::GetCollator()->compareString( aUpVal1, aUpVal2 )
                == COMPARE_GREATER )
        {
            //  richtige Reihenfolge fuer Wertebereich
            OUString aTemp( aUpVal1 ); aUpVal1 = aUpVal2; aUpVal2 = aTemp;
        }

    switch ( eOp )
    {
        case SC_COND_EQUAL:
            bValid = (ScGlobal::GetCollator()->compareString(
                rArg, aUpVal1 ) == COMPARE_EQUAL);
        break;
        case SC_COND_NOTEQUAL:
            bValid = (ScGlobal::GetCollator()->compareString(
                rArg, aUpVal1 ) != COMPARE_EQUAL);
        break;
        case SC_COND_TOP_PERCENT:
        case SC_COND_BOTTOM_PERCENT:
        case SC_COND_TOP10:
        case SC_COND_BOTTOM10:
        case SC_COND_ABOVE_AVERAGE:
        case SC_COND_BELOW_AVERAGE:
            return false;
        case SC_COND_ERROR:
        case SC_COND_NOERROR:
            bValid = IsError( rPos );
            if(eOp == SC_COND_NOERROR)
                bValid = !bValid;
        break;
        case SC_COND_BEGINS_WITH:
            bValid = rArg.indexOf(aUpVal1) == 0;
        break;
        case SC_COND_ENDS_WITH:
            bValid = rArg.endsWith(aUpVal1);
        break;
        case SC_COND_CONTAINS_TEXT:
        case SC_COND_NOT_CONTAINS_TEXT:
            bValid = rArg.indexOf(aUpVal1) != -1;
            if(eOp == SC_COND_NOT_CONTAINS_TEXT)
                bValid = !bValid;
        break;
        default:
        {
            sal_Int32 nCompare = ScGlobal::GetCollator()->compareString(
                rArg, aUpVal1 );
            switch ( eOp )
            {
                case SC_COND_GREATER:
                    bValid = ( nCompare == COMPARE_GREATER );
                    break;
                case SC_COND_EQGREATER:
                    bValid = ( nCompare == COMPARE_EQUAL || nCompare == COMPARE_GREATER );
                    break;
                case SC_COND_LESS:
                    bValid = ( nCompare == COMPARE_LESS );
                    break;
                case SC_COND_EQLESS:
                    bValid = ( nCompare == COMPARE_EQUAL || nCompare == COMPARE_LESS );
                    break;
                case SC_COND_BETWEEN:
                case SC_COND_NOTBETWEEN:
                    //  Test auf NOTBETWEEN:
                    bValid = ( nCompare == COMPARE_LESS ||
                        ScGlobal::GetCollator()->compareString( rArg,
                        aUpVal2 ) == COMPARE_GREATER );
                    if ( eOp == SC_COND_BETWEEN )
                        bValid = !bValid;
                    break;
                //  SC_COND_DIRECT schon oben abgefragt
                default:
                    SAL_WARN("sc", "unbekannte Operation bei ScConditionEntry");
                    bValid = false;
                    break;
            }
        }
    }
    return bValid;
}

bool ScConditionEntry::IsCellValid( ScRefCellValue& rCell, const ScAddress& rPos ) const
{
    ((ScConditionEntry*)this)->Interpret(rPos);         // Formeln auswerten

    double nArg = 0.0;
    OUString aArgStr;
    bool bVal = lcl_GetCellContent( rCell, bIsStr1, nArg, aArgStr, mpDoc );
    if (bVal)
        return IsValid( nArg, rPos );
    else
        return IsValidStr( aArgStr, rPos );
}

OUString ScConditionEntry::GetExpression( const ScAddress& rCursor, sal_uInt16 nIndex,
                                        sal_uLong nNumFmt,
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
            ScCompiler aComp(mpDoc, rCursor, *pFormula1);
            aComp.SetGrammar(eGrammar);
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
            ScCompiler aComp(mpDoc, rCursor, *pFormula2);
            aComp.SetGrammar(eGrammar);
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

ScTokenArray* ScConditionEntry::CreateTokenArry( sal_uInt16 nIndex ) const
{
    assert(nIndex <= 1);
    ScTokenArray* pRet = NULL;
    ScAddress aAddr;

    if ( nIndex==0 )
    {
        if ( pFormula1 )
            pRet = new ScTokenArray( *pFormula1 );
        else
        {
            pRet = new ScTokenArray();
            if (bIsStr1)
                pRet->AddString( aStrVal1 );
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
                pRet->AddString( aStrVal2 );
            else
                pRet->AddDouble( nVal2 );
        }
    }

    return pRet;
}

void ScConditionEntry::SourceChanged( const ScAddress& rChanged )
{
    for (sal_uInt16 nPass = 0; nPass < 2; nPass++)
    {
        ScTokenArray* pFormula = nPass ? pFormula2 : pFormula1;
        if (pFormula)
        {
            pFormula->Reset();
            ScToken* t;
            while ( ( t = static_cast<ScToken*>(pFormula->GetNextReference()) ) != NULL )
            {
                SingleDoubleRefProvider aProv( *t );
                if ( aProv.Ref1.IsColRel() || aProv.Ref1.IsRowRel() || aProv.Ref1.IsTabRel() ||
                     aProv.Ref2.IsColRel() || aProv.Ref2.IsRowRel() || aProv.Ref2.IsTabRel() )
                {
                    //  absolut muss getroffen sein, relativ bestimmt Bereich

                    bool bHit = true;
                    SCsCOL nCol1;
                    SCsROW nRow1;
                    SCsTAB nTab1;
                    SCsCOL nCol2;
                    SCsROW nRow2;
                    SCsTAB nTab2;

                    if ( aProv.Ref1.IsColRel() )
                        nCol2 = rChanged.Col() - aProv.Ref1.Col();
                    else
                    {
                        bHit &= (rChanged.Col() >= aProv.Ref1.Col());
                        nCol2 = MAXCOL;
                    }
                    if ( aProv.Ref1.IsRowRel() )
                        nRow2 = rChanged.Row() - aProv.Ref1.Row();
                    else
                    {
                        bHit &= ( rChanged.Row() >= aProv.Ref1.Row() );
                        nRow2 = MAXROW;
                    }
                    if ( aProv.Ref1.IsTabRel() )
                        nTab2 = rChanged.Tab() - aProv.Ref1.Tab();
                    else
                    {
                        bHit &= (rChanged.Tab() >= aProv.Ref1.Tab());
                        nTab2 = MAXTAB;
                    }

                    if ( aProv.Ref2.IsColRel() )
                        nCol1 = rChanged.Col() - aProv.Ref2.Col();
                    else
                    {
                        bHit &= ( rChanged.Col() <= aProv.Ref2.Col() );
                        nCol1 = 0;
                    }
                    if ( aProv.Ref2.IsRowRel() )
                        nRow1 = rChanged.Row() - aProv.Ref2.Row();
                    else
                    {
                        bHit &= (rChanged.Row() <= aProv.Ref2.Row());
                        nRow1 = 0;
                    }
                    if ( aProv.Ref2.IsTabRel() )
                        nTab1 = rChanged.Tab() - aProv.Ref2.Tab();
                    else
                    {
                        bHit &= (rChanged.Tab() <= aProv.Ref2.Tab());
                        nTab1 = 0;
                    }

                    if ( bHit )
                    {
                        //! begrenzen

                        ScRange aPaint( nCol1,nRow1,nTab1, nCol2,nRow2,nTab2 );

                        //  kein Paint, wenn es nur die Zelle selber ist
                        if ( aPaint.IsValid() && (aPaint.aStart != rChanged || aPaint.aEnd != rChanged ))
                            DataChanged( &aPaint );
                    }
                }
            }
        }
    }
}

ScAddress ScConditionEntry::GetValidSrcPos() const
{
    // return a position that's adjusted to allow textual representation of expressions if possible

    SCTAB nMinTab = aSrcPos.Tab();
    SCTAB nMaxTab = nMinTab;

    for (sal_uInt16 nPass = 0; nPass < 2; nPass++)
    {
        ScTokenArray* pFormula = nPass ? pFormula2 : pFormula1;
        if (pFormula)
        {
            pFormula->Reset();
            ScToken* t;
            while ( ( t = static_cast<ScToken*>(pFormula->GetNextReference()) ) != NULL )
            {
                ScSingleRefData& rRef1 = t->GetSingleRef();
                if ( rRef1.IsTabRel() && !rRef1.IsTabDeleted() )
                {
                    if ( rRef1.nTab < nMinTab )
                        nMinTab = rRef1.nTab;
                    if ( rRef1.nTab > nMaxTab )
                        nMaxTab = rRef1.nTab;
                }
                if ( t->GetType() == svDoubleRef )
                {
                    ScSingleRefData& rRef2 = t->GetDoubleRef().Ref2;
                    if ( rRef2.IsTabRel() && !rRef2.IsTabDeleted() )
                    {
                        if ( rRef2.nTab < nMinTab )
                            nMinTab = rRef2.nTab;
                        if ( rRef2.nTab > nMaxTab )
                            nMaxTab = rRef2.nTab;
                    }
                }
            }
        }
    }

    ScAddress aValidPos = aSrcPos;
    SCTAB nTabCount = mpDoc->GetTableCount();
    if ( nMaxTab >= nTabCount && nMinTab > 0 )
        aValidPos.SetTab( aSrcPos.Tab() - nMinTab );    // so the lowest tab ref will be on 0

    if ( aValidPos.Tab() >= nTabCount )
        aValidPos.SetTab( nTabCount - 1 );  // ensure a valid position even if some references will be invalid

    return aValidPos;
}

void ScConditionEntry::DataChanged( const ScRange* /* pModified */ ) const
{
    // nix
}

bool ScConditionEntry::MarkUsedExternalReferences() const
{
    bool bAllMarked = false;
    for (sal_uInt16 nPass = 0; !bAllMarked && nPass < 2; nPass++)
    {
        ScTokenArray* pFormula = nPass ? pFormula2 : pFormula1;
        if (pFormula)
            bAllMarked = mpDoc->MarkUsedExternalReferences( *pFormula);
    }
    return bAllMarked;
}

ScFormatEntry* ScConditionEntry::Clone(ScDocument* pDoc) const
{
    return new ScConditionEntry(pDoc, *this);
}

ScConditionMode ScConditionEntry::GetModeFromApi(sal_Int32 nOperation)
{
    ScConditionMode eMode = SC_COND_NONE;
    switch (nOperation)
    {
        case com::sun::star::sheet::ConditionOperator2::EQUAL:
            eMode = SC_COND_EQUAL;
            break;
        case com::sun::star::sheet::ConditionOperator2::LESS:
            eMode = SC_COND_LESS;
            break;
        case com::sun::star::sheet::ConditionOperator2::GREATER:
            eMode = SC_COND_GREATER;
            break;
        case com::sun::star::sheet::ConditionOperator2::LESS_EQUAL:
            eMode = SC_COND_EQLESS;
            break;
        case com::sun::star::sheet::ConditionOperator2::GREATER_EQUAL:
            eMode = SC_COND_EQGREATER;
            break;
        case com::sun::star::sheet::ConditionOperator2::NOT_EQUAL:
            eMode = SC_COND_NOTEQUAL;
            break;
        case com::sun::star::sheet::ConditionOperator2::BETWEEN:
            eMode = SC_COND_BETWEEN;
            break;
        case com::sun::star::sheet::ConditionOperator2::NOT_BETWEEN:
            eMode = SC_COND_NOTBETWEEN;
            break;
        case com::sun::star::sheet::ConditionOperator2::FORMULA:
            eMode = SC_COND_DIRECT;
            break;
        case com::sun::star::sheet::ConditionOperator2::DUPLICATE:
            eMode = SC_COND_DUPLICATE;
            break;
        case com::sun::star::sheet::ConditionOperator2::NOT_DUPLICATE:
            eMode = SC_COND_NOTDUPLICATE;
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

//------------------------------------------------------------------------

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

int ScCondFormatEntry::operator== ( const ScCondFormatEntry& r ) const
{
    return ScConditionEntry::operator==( r ) &&
            aStyleName == r.aStyleName;
}

ScCondFormatEntry::~ScCondFormatEntry()
{
}

void ScCondFormatEntry::DataChanged( const ScRange* pModified ) const
{
    if ( pCondFormat )
        pCondFormat->DoRepaint( pModified );
}

ScFormatEntry* ScCondFormatEntry::Clone( ScDocument* pDoc ) const
{
    return new ScCondFormatEntry( pDoc, *this );
}

//------------------------------------------------------------------------

ScCondDateFormatEntry::ScCondDateFormatEntry( ScDocument* pDoc ):
    ScFormatEntry( pDoc )
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
    CellType eCellType = mpDoc->GetCellType(rPos);

    if (eCellType == CELLTYPE_NONE)
        // empty cell.
        return false;

    if (eCellType != CELLTYPE_VALUE && eCellType != CELLTYPE_FORMULA)
        // non-numerical cell.
        return false;

    if( !mpCache )
        mpCache.reset( new Date( Date::SYSTEM ) );

    const Date& rActDate = *mpCache;
    SvNumberFormatter* pFormatter = mpDoc->GetFormatTable();
    long nCurrentDate = rActDate - *(pFormatter->GetNullDate());

    double nVal = mpDoc->GetValue(rPos);
    long nCellDate = (long) ::rtl::math::approxFloor(nVal);
    Date aCellDate = *(pFormatter->GetNullDate());
    aCellDate += (long) ::rtl::math::approxFloor(nVal);

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
            if( rActDate.GetDayOfWeek() != SUNDAY )
            {
                Date aBegin(rActDate - 8 - static_cast<long>(rActDate.GetDayOfWeek()));
                Date aEnd(rActDate - 2 -static_cast<long>(rActDate.GetDayOfWeek()));
                return aCellDate.IsBetween( aBegin, aEnd );
            }
            else
            {
                Date aBegin(rActDate - 8);
                Date aEnd(rActDate - 1);
                return aCellDate.IsBetween( aBegin, aEnd );
            }
            break;
        case condformat::THISWEEK:
            if( rActDate.GetDayOfWeek() != SUNDAY )
            {
                Date aBegin(rActDate - 1 - static_cast<long>(rActDate.GetDayOfWeek()));
                Date aEnd(rActDate + 5 - static_cast<long>(rActDate.GetDayOfWeek()));
                return aCellDate.IsBetween( aBegin, aEnd );
            }
            else
            {
                Date aEnd( rActDate + 6);
                return aCellDate.IsBetween( rActDate, aEnd );
            }
            break;
        case condformat::NEXTWEEK:
            if( rActDate.GetDayOfWeek() != SUNDAY )
            {
                return aCellDate.IsBetween( rActDate + 6 - static_cast<long>(rActDate.GetDayOfWeek()), rActDate + 12 - static_cast<long>(rActDate.GetDayOfWeek()) );
            }
            else
            {
                return aCellDate.IsBetween( rActDate + 7, rActDate + 13 );
            }
            break;
        case condformat::LASTMONTH:
            if( rActDate.GetMonth() == 1 )
            {
                if( aCellDate.GetMonth() == 12 && rActDate.GetYear() == aCellDate.GetYear() + 1 )
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
            if( rActDate.GetYear() == aCellDate.GetYear() + 1 )
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

condformat::ScCondFormatDateType ScCondDateFormatEntry::GetDateType() const
{
    return meType;
}

const OUString& ScCondDateFormatEntry::GetStyleName() const
{
    return maStyleName;
}

void ScCondDateFormatEntry::SetStyleName( const OUString& rStyleName )
{
    maStyleName = rStyleName;
}

ScFormatEntry* ScCondDateFormatEntry::Clone( ScDocument* pDoc ) const
{
    return new ScCondDateFormatEntry( pDoc, *this );
}

bool ScCondDateFormatEntry::operator==( const ScFormatEntry& r ) const
{
    if(r.GetType() != condformat::DATE)
        return false;

    const ScCondDateFormatEntry& rEntry = static_cast<const ScCondDateFormatEntry&>(r);

    if(rEntry.meType != meType)
        return false;

    return rEntry.maStyleName == maStyleName;
}

void ScCondDateFormatEntry::startRendering()
{
    mpCache.reset();
}

void ScCondDateFormatEntry::endRendering()
{
    mpCache.reset();
}

//------------------------------------------------------------------------

ScConditionalFormat::ScConditionalFormat(sal_uInt32 nNewKey, ScDocument* pDocument) :
    pDoc( pDocument ),
    nKey( nNewKey )
{
}

ScConditionalFormat* ScConditionalFormat::Clone(ScDocument* pNewDoc) const
{
    // echte Kopie der Formeln (fuer Ref-Undo / zwischen Dokumenten)

    if (!pNewDoc)
        pNewDoc = pDoc;

    ScConditionalFormat* pNew = new ScConditionalFormat(nKey, pNewDoc);

    for (CondFormatContainer::const_iterator itr = maEntries.begin(); itr != maEntries.end(); ++itr)
    {
        ScFormatEntry* pNewEntry = itr->Clone(pNewDoc);
        pNew->maEntries.push_back( pNewEntry );
        pNewEntry->SetParent(pNew);
    }
    pNew->AddRange( maRanges );

    return pNew;
}

bool ScConditionalFormat::EqualEntries( const ScConditionalFormat& r ) const
{
    if( size() != r.size())
        return false;

    //! auf gleiche Eintraege in anderer Reihenfolge testen ???

    for (sal_uInt16 i=0; i<size(); i++)
        if ( ! (maEntries == r.maEntries ) )
            return false;

    // right now don't check for same range
    // we only use this method to merge same conditional formats from
    // old ODF data structure
    return true;
}

void ScConditionalFormat::AddRange( const ScRangeList& rRanges )
{
    maRanges = rRanges;
}

void ScConditionalFormat::AddEntry( ScFormatEntry* pNew )
{
    maEntries.push_back(pNew);
    pNew->SetParent(this);
}

bool ScConditionalFormat::IsEmpty() const
{
    return maEntries.empty();
}

size_t ScConditionalFormat::size() const
{
    return maEntries.size();
}

ScConditionalFormat::~ScConditionalFormat()
{
}

const ScFormatEntry* ScConditionalFormat::GetEntry( sal_uInt16 nPos ) const
{
    if ( nPos < size() )
        return &maEntries[nPos];
    else
        return NULL;
}

const OUString& ScConditionalFormat::GetCellStyle( ScRefCellValue& rCell, const ScAddress& rPos ) const
{
    for (CondFormatContainer::const_iterator itr = maEntries.begin(); itr != maEntries.end(); ++itr)
    {
        if(itr->GetType() == condformat::CONDITION)
        {
            const ScCondFormatEntry& rEntry = static_cast<const ScCondFormatEntry&>(*itr);
            if (rEntry.IsCellValid(rCell, rPos))
                return rEntry.GetStyle();
        }
        else if(itr->GetType() == condformat::DATE)
        {
            const ScCondDateFormatEntry& rEntry = static_cast<const ScCondDateFormatEntry&>(*itr);
            if (rEntry.IsValid( rPos ))
                return rEntry.GetStyleName();
        }
    }

    return EMPTY_OUSTRING;
}

ScCondFormatData ScConditionalFormat::GetData( ScRefCellValue& rCell, const ScAddress& rPos ) const
{
    ScCondFormatData aData;
    for(CondFormatContainer::const_iterator itr = maEntries.begin(); itr != maEntries.end(); ++itr)
    {
        if(itr->GetType() == condformat::CONDITION && aData.aStyleName.isEmpty())
        {
            const ScCondFormatEntry& rEntry = static_cast<const ScCondFormatEntry&>(*itr);
            if (rEntry.IsCellValid(rCell, rPos))
                aData.aStyleName = rEntry.GetStyle();
        }
        else if(itr->GetType() == condformat::COLORSCALE && !aData.pColorScale)
        {
            const ScColorScaleFormat& rEntry = static_cast<const ScColorScaleFormat&>(*itr);
            aData.pColorScale = rEntry.GetColor(rPos);
        }
        else if(itr->GetType() == condformat::DATABAR && !aData.pDataBar)
        {
            const ScDataBarFormat& rEntry = static_cast<const ScDataBarFormat&>(*itr);
            aData.pDataBar = rEntry.GetDataBarInfo(rPos);
        }
        else if(itr->GetType() == condformat::ICONSET && !aData.pIconSet)
        {
            const ScIconSetFormat& rEntry = static_cast<const ScIconSetFormat&>(*itr);
            aData.pIconSet = rEntry.GetIconSetInfo(rPos);
        }
        else if(itr->GetType() == condformat::DATE && aData.aStyleName.isEmpty())
        {
            const ScCondDateFormatEntry& rEntry = static_cast<const ScCondDateFormatEntry&>(*itr);
            if ( rEntry.IsValid( rPos ) )
                aData.aStyleName = rEntry.GetStyleName();
        }
    }
    return aData;
}

void ScConditionalFormat::DoRepaint( const ScRange* pModified )
{
    if(pModified)
    {
        if(maRanges.Intersects(*pModified))
            pDoc->RepaintRange(*pModified);
    }
    else
    {
        // all conditional format cells
        pDoc->RepaintRange( maRanges );
    }
}

void ScConditionalFormat::CompileAll()
{
    for(CondFormatContainer::iterator itr = maEntries.begin(); itr != maEntries.end(); ++itr)
        if(itr->GetType() == condformat::CONDITION)
            static_cast<ScCondFormatEntry&>(*itr).CompileAll();
}

void ScConditionalFormat::CompileXML()
{
    for(CondFormatContainer::iterator itr = maEntries.begin(); itr != maEntries.end(); ++itr)
        if(itr->GetType() == condformat::CONDITION)
            static_cast<ScCondFormatEntry&>(*itr).CompileXML();
}

void ScConditionalFormat::UpdateReference( UpdateRefMode eUpdateRefMode,
                                const ScRange& rRange, SCsCOL nDx, SCsROW nDy, SCsTAB nDz, bool bCopyAsMove )
{
    for(CondFormatContainer::iterator itr = maEntries.begin(); itr != maEntries.end(); ++itr)
        itr->UpdateReference(eUpdateRefMode, rRange, nDx, nDy, nDz);

    if( eUpdateRefMode == URM_COPY && bCopyAsMove )
        maRanges.UpdateReference( URM_MOVE, pDoc, rRange, nDx, nDy, nDz );
    else
        maRanges.UpdateReference( eUpdateRefMode, pDoc, rRange, nDx, nDy, nDz );
}

void ScConditionalFormat::DeleteArea( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 )
{
    SCTAB nTab = maRanges[0]->aStart.Tab();
    maRanges.DeleteArea( nCol1, nRow1, nTab, nCol2, nRow2, nTab );
}

void ScConditionalFormat::RenameCellStyle(const OUString& rOld, const OUString& rNew)
{
    for(CondFormatContainer::iterator itr = maEntries.begin(); itr != maEntries.end(); ++itr)
        if(itr->GetType() == condformat::CONDITION)
        {
            ScCondFormatEntry& rFormat = static_cast<ScCondFormatEntry&>(*itr);
            if(rFormat.GetStyle() == rOld)
                rFormat.UpdateStyleName( rNew );
        }
}

void ScConditionalFormat::UpdateMoveTab( SCTAB nOldPos, SCTAB nNewPos )
{
    size_t n = maRanges.size();
    SCTAB nMinTab = std::min<SCTAB>(nOldPos, nNewPos);
    SCTAB nMaxTab = std::max<SCTAB>(nOldPos, nNewPos);
    for(size_t i = 0; i < n; ++i)
    {
        ScRange* pRange = maRanges[i];
        SCTAB nTab = pRange->aStart.Tab();
        if(nTab < nMinTab || nTab > nMaxTab)
        {
            continue;
        }

        if(nTab == nOldPos)
        {
            pRange->aStart.SetTab(nNewPos);
            pRange->aEnd.SetTab(nNewPos);
            continue;
        }

        if(nNewPos < nOldPos)
        {
            pRange->aStart.IncTab();
            pRange->aEnd.IncTab();
        }
        else
        {
            pRange->aStart.IncTab(-1);
            pRange->aEnd.IncTab(-1);
        }
    }

    for(CondFormatContainer::iterator itr = maEntries.begin(); itr != maEntries.end(); ++itr)
        itr->UpdateMoveTab( nOldPos, nNewPos );
}

void ScConditionalFormat::SourceChanged( const ScAddress& rAddr )
{
    for(CondFormatContainer::iterator itr = maEntries.begin(); itr != maEntries.end(); ++itr)
    {
        condformat::ScFormatEntryType eEntryType = itr->GetType();
        if( eEntryType == condformat::CONDITION)
        {
            ScCondFormatEntry& rFormat = static_cast<ScCondFormatEntry&>(*itr);
            rFormat.SourceChanged( rAddr );
        }
        else if( eEntryType == condformat::COLORSCALE ||
                eEntryType == condformat::DATABAR ||
                eEntryType == condformat::ICONSET )
        {
            ScColorFormat& rFormat = static_cast<ScColorFormat&>(*itr);
            if(rFormat.NeedsRepaint())
            {
                // we need to repaint the whole range anyway
                DoRepaint(NULL);
                return;
            }
        }
    }
}

bool ScConditionalFormat::MarkUsedExternalReferences() const
{
    bool bAllMarked = false;
    for(CondFormatContainer::const_iterator itr = maEntries.begin(); itr != maEntries.end() && !bAllMarked; ++itr)
        if(itr->GetType() == condformat::CONDITION)
        {
            const ScCondFormatEntry& rFormat = static_cast<const ScCondFormatEntry&>(*itr);
            bAllMarked = rFormat.MarkUsedExternalReferences();
        }

    return bAllMarked;
}

void ScConditionalFormat::startRendering()
{
    for(CondFormatContainer::iterator itr = maEntries.begin(); itr != maEntries.end(); ++itr)
    {
        itr->startRendering();
    }
}

void ScConditionalFormat::endRendering()
{
    for(CondFormatContainer::iterator itr = maEntries.begin(); itr != maEntries.end(); ++itr)
    {
        itr->endRendering();
    }
}

//------------------------------------------------------------------------

ScConditionalFormatList::ScConditionalFormatList(const ScConditionalFormatList& rList)
{
    for(const_iterator itr = rList.begin(); itr != rList.end(); ++itr)
        InsertNew( itr->Clone() );
}

ScConditionalFormatList::ScConditionalFormatList(ScDocument* pDoc, const ScConditionalFormatList& rList)
{
    for(const_iterator itr = rList.begin(); itr != rList.end(); ++itr)
        InsertNew( itr->Clone(pDoc) );
}

void ScConditionalFormatList::InsertNew( ScConditionalFormat* pNew )
{
    maConditionalFormats.insert(pNew);
}

bool ScConditionalFormatList::operator==( const ScConditionalFormatList& r ) const
{
    // fuer Ref-Undo - interne Variablen werden nicht verglichen

    sal_uInt16 nCount = size();
    bool bEqual = ( nCount == r.size() );
    const_iterator locIterator = begin();
    for(const_iterator itr = r.begin(); itr != r.end() && bEqual; ++itr, ++locIterator)
        if ( !locIterator->EqualEntries(*itr) )         // Eintraege unterschiedlich ?
            bEqual = false;

    return bEqual;
}

ScConditionalFormat* ScConditionalFormatList::GetFormat( sal_uInt32 nKey )
{
    //! binaer suchen

    for( iterator itr = begin(); itr != end(); ++itr)
        if (itr->GetKey() == nKey)
            return &(*itr);

    SAL_WARN("sc", "ScConditionalFormatList: Entry not found");
    return NULL;
}

const ScConditionalFormat* ScConditionalFormatList::GetFormat( sal_uInt32 nKey ) const
{
    //! binaer suchen

    for ( const_iterator itr = begin(); itr != end(); ++itr)
        if (itr->GetKey() == nKey)
            return &(*itr);

    SAL_WARN("sc", "ScConditionalFormatList: Entry not found");
    return NULL;
}

void ScConditionalFormatList::CompileAll()
{
    for( iterator itr = begin(); itr != end(); ++itr)
        itr->CompileAll();
}

void ScConditionalFormatList::CompileXML()
{
    for( iterator itr = begin(); itr != end(); ++itr)
        itr->CompileXML();
}

void ScConditionalFormatList::UpdateReference( UpdateRefMode eUpdateRefMode,
                                const ScRange& rRange, SCsCOL nDx, SCsROW nDy, SCsTAB nDz )
{
    for( iterator itr = begin(); itr != end(); ++itr)
        itr->UpdateReference( eUpdateRefMode, rRange, nDx, nDy, nDz );

    if( eUpdateRefMode == URM_INSDEL )
    {
        // need to check which must be deleted
        iterator itr = begin();
        while(itr != end())
        {
            if(itr->GetRange().empty())
                maConditionalFormats.erase(itr++);
            else
                ++itr;
        }
    }
}

void ScConditionalFormatList::RenameCellStyle( const OUString& rOld, const OUString& rNew )
{
    for( iterator itr = begin(); itr != end(); ++itr)
        itr->RenameCellStyle(rOld,rNew);
}

void ScConditionalFormatList::UpdateMoveTab( SCTAB nOldPos, SCTAB nNewPos )
{
    for( iterator itr = begin(); itr != end(); ++itr)
        itr->UpdateMoveTab( nOldPos, nNewPos );
}

bool ScConditionalFormatList::CheckAllEntries()
{
    bool bValid = true;
    //
    // need to check which must be deleted
    iterator itr = begin();
    while(itr != end())
    {
        if(itr->GetRange().empty())
        {
            bValid = false;
            maConditionalFormats.erase(itr++);
        }
        else
            ++itr;
    }

    return bValid;
}

void ScConditionalFormatList::DeleteArea( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 )
{
    for( iterator itr = begin(); itr != end(); ++itr)
        itr->DeleteArea( nCol1, nRow1, nCol2, nRow2 );

    CheckAllEntries();
}

void ScConditionalFormatList::SourceChanged( const ScAddress& rAddr )
{
    for( iterator itr = begin(); itr != end(); ++itr)
        itr->SourceChanged( rAddr );
}

ScConditionalFormatList::iterator ScConditionalFormatList::begin()
{
    return maConditionalFormats.begin();
}

ScConditionalFormatList::const_iterator ScConditionalFormatList::begin() const
{
    return maConditionalFormats.begin();
}

ScConditionalFormatList::iterator ScConditionalFormatList::end()
{
    return maConditionalFormats.end();
}

ScConditionalFormatList::const_iterator ScConditionalFormatList::end() const
{
    return maConditionalFormats.end();
}

size_t ScConditionalFormatList::size() const
{
    return maConditionalFormats.size();
}

void ScConditionalFormatList::erase( sal_uLong nIndex )
{
    for( iterator itr = begin(); itr != end(); ++itr )
    {
        if( itr->GetKey() == nIndex )
        {
            maConditionalFormats.erase(itr);
            break;
        }
    }
}

void ScConditionalFormatList::startRendering()
{
    for(iterator itr = begin(); itr != end(); ++itr)
    {
        itr->startRendering();
    }
}

void ScConditionalFormatList::endRendering()
{
    for(iterator itr = begin(); itr != end(); ++itr)
    {
        itr->endRendering();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
