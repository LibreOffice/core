/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "scitems.hxx"
#include <sfx2/objsh.hxx>
#include <svl/itemset.hxx>
#include <svl/zforlist.hxx>
#include <rtl/math.hxx>
#include <unotools/collatorwrapper.hxx>

#include <com/sun/star/sheet/ConditionOperator2.hpp>

#include "conditio.hxx"
#include "cell.hxx"
#include "document.hxx"
#include "hints.hxx"
#include "compiler.hxx"
#include "rechead.hxx"
#include "rangelst.hxx"
#include "stlpool.hxx"
#include "rangenam.hxx"
#include "colorscale.hxx"

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
            break;
        default:
            // TODO: implement also this case
            // actually return false for these cases is not that bad
            // as soon as databar and color scale are tested we need
            // to think about the range
            return false;
    }

    return true;
}

bool lcl_HasRelRef( ScDocument* pDoc, ScTokenArray* pFormula, sal_uInt16 nRecursion = 0 )
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
        const String& rExpr1, const String& rExpr2, ScDocument* pDocument, const ScAddress& rPos,
        const String& rExprNmsp1, const String& rExprNmsp2,
        FormulaGrammar::Grammar eGrammar1, FormulaGrammar::Grammar eGrammar2 ) :
    ScFormatEntry(pDocument),
    eOp(eOper),
    nOptions(0),    // spaeter...
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
    nOptions(0),    // spaeter...
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

void ScConditionEntry::Compile( const String& rExpr1, const String& rExpr2,
        const String& rExprNmsp1, const String& rExprNmsp2,
        FormulaGrammar::Grammar eGrammar1, FormulaGrammar::Grammar eGrammar2, bool bTextToReal )
{
    if ( rExpr1.Len() || rExpr2.Len() )
    {
        ScCompiler aComp( mpDoc, aSrcPos );

        if ( rExpr1.Len() )
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

        if ( rExpr2.Len() )
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

    if ( aSrcString.Len() )
    {
        ScAddress aNew;
        /* XML is always in OOo:A1 format, although R1C1 would be more amenable
         * to compression */
        if ( aNew.Parse( aSrcString, mpDoc ) & SCA_VALID )
            aSrcPos = aNew;
        // if the position is invalid, there isn't much we can do at this time
        aSrcString.Erase();
    }

    //  Convert the text tokens that were created during XML import into real tokens.

    Compile( GetExpression(aSrcPos, 0, 0, eTempGrammar1),
             GetExpression(aSrcPos, 1, 0, eTempGrammar2),
             aStrNmsp1, aStrNmsp2, eTempGrammar1, eTempGrammar2, true );
}

void ScConditionEntry::SetSrcString( const String& rNew )
{
    // aSrcString is only evaluated in CompileXML
    OSL_ENSURE( mpDoc->IsImportingXML(), "SetSrcString is only valid for XML import" );

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

void lcl_CondUpdateInsertTab( ScTokenArray& rCode, SCTAB nInsTab, SCTAB nPosTab, bool& rChanged, SCTAB nTabs )
{
    //  Insert table: only update absolute table references.
    //  (Similar to ScCompiler::UpdateInsertTab with bIsName=true, result is the same as for named ranges)
    //  For deleting, ScCompiler::UpdateDeleteTab is used because of the handling of invalid references.

    rCode.Reset();
    ScToken* p = static_cast<ScToken*>(rCode.GetNextReference());
    while( p )
    {
        ScSingleRefData& rRef1 = p->GetSingleRef();
        if ( !rRef1.IsTabRel() && nInsTab <= rRef1.nTab )
        {
            rRef1.nTab += nTabs;
            rRef1.nRelTab = rRef1.nTab - nPosTab;
            rChanged = true;
        }
        if( p->GetType() == svDoubleRef )
        {
            ScSingleRefData& rRef2 = p->GetDoubleRef().Ref2;
            if ( !rRef2.IsTabRel() && nInsTab <= rRef2.nTab )
            {
                rRef2.nTab += nTabs;
                rRef2.nRelTab = rRef2.nTab - nPosTab;
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

    bool bChanged1 = false;
    bool bChanged2 = false;

    if (pFormula1)
    {
        if ( bInsertTab )
            lcl_CondUpdateInsertTab( *pFormula1, rRange.aStart.Tab(), aSrcPos.Tab(), bChanged1, nDz );
        else
        {
            ScCompiler aComp( mpDoc, aSrcPos, *pFormula1 );
            aComp.SetGrammar(mpDoc->GetGrammar());
            if ( bDeleteTab )
                aComp.UpdateDeleteTab( rRange.aStart.Tab(), false, true, bChanged1, static_cast<SCTAB>(-1 * nDz) );
            else
                aComp.UpdateNameReference( eUpdateRefMode, rRange, nDx, nDy, nDz, bChanged1 );
        }

        if (bChanged1)
            DELETEZ(pFCell1);       // is created again in IsValid
    }
    if (pFormula2)
    {
        if ( bInsertTab )
            lcl_CondUpdateInsertTab( *pFormula2, rRange.aStart.Tab(), aSrcPos.Tab(), bChanged2, nDz );
        else
        {
            ScCompiler aComp( mpDoc, aSrcPos, *pFormula2);
            aComp.SetGrammar(mpDoc->GetGrammar());
            if ( bDeleteTab )
                aComp.UpdateDeleteTab( rRange.aStart.Tab(), false, true, bChanged2, static_cast<SCTAB>(-1*nDz) );
            else
                aComp.UpdateNameReference( eUpdateRefMode, rRange, nDx, nDy, nDz, bChanged2 );
        }

        if (bChanged2)
            DELETEZ(pFCell2);       // is created again in IsValid
    }
}

void ScConditionEntry::UpdateMoveTab( SCTAB nOldPos, SCTAB nNewPos )
{
    if (pFormula1)
    {
        ScCompiler aComp( mpDoc, aSrcPos, *pFormula1);
        aComp.SetGrammar(mpDoc->GetGrammar());
        aComp.UpdateMoveTab(nOldPos, nNewPos, true );
        DELETEZ(pFCell1);
    }
    if (pFormula2)
    {
        ScCompiler aComp( mpDoc, aSrcPos, *pFormula2);
        aComp.SetGrammar(mpDoc->GetGrammar());
        aComp.UpdateMoveTab(nOldPos, nNewPos, true );
        DELETEZ(pFCell2);
    }
}

//! als Vergleichsoperator ans TokenArray ???

bool lcl_IsEqual( const ScTokenArray* pArr1, const ScTokenArray* pArr2 )
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
            if (pEff1->GetDirty() && !bRelRef1)
                bDirty = true;
            if (pEff1->IsValue())
            {
                bIsStr1 = false;
                nVal1 = pEff1->GetValue();
                aStrVal1.Erase();
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
            if (pEff2->GetDirty() && !bRelRef2)
                bDirty = true;
            if (pEff2->IsValue())
            {
                bIsStr2 = false;
                nVal2 = pEff2->GetValue();
                aStrVal2.Erase();
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

static bool lcl_GetCellContent( ScBaseCell* pCell, bool bIsStr1, double& rArg, String& rArgStr )
{
    bool bVal = true;

    if ( pCell )
    {
        CellType eType = pCell->GetCellType();
        switch( eType )
        {
            case CELLTYPE_VALUE:
                rArg = ((ScValueCell*)pCell)->GetValue();
                break;
            case CELLTYPE_FORMULA:
                {
                    ScFormulaCell* pFCell = (ScFormulaCell*)pCell;
                    bVal = pFCell->IsValue();
                    if (bVal)
                        rArg = pFCell->GetValue();
                    else
                        rArgStr = pFCell->GetString();
                }
                break;
            case CELLTYPE_STRING:
            case CELLTYPE_EDIT:
                bVal = false;
                if ( eType == CELLTYPE_STRING )
                    rArgStr = ((ScStringCell*)pCell)->GetString();
                else
                    rArgStr = ((ScEditCell*)pCell)->GetString();
                break;

            default:
                pCell = NULL;           // Note-Zellen wie leere
                break;
        }
    }

    if( !pCell )
        if( bIsStr1 )
            bVal = false;               // leere Zellen je nach Bedingung

    return bVal;
}

static bool lcl_IsDuplicate( ScDocument *pDoc, double nArg, const String& rStr, const ScAddress& rAddr, const ScRangeList& rRanges )
{
    size_t nListCount = rRanges.size();
    for( size_t i = 0; i < nListCount; i++ )
    {
        const ScRange *aRange = rRanges[i];
        SCROW nRow = aRange->aEnd.Row();
        SCCOL nCol = aRange->aEnd.Col();
        for( SCROW r = aRange->aStart.Row(); r <= nRow; r++ )
            for( SCCOL c = aRange->aStart.Col(); c <= nCol; c++ )
            {
                double nVal = 0.0;
                ScBaseCell *pCell = NULL;
                String aStr;

                if( c == rAddr.Col() && r == rAddr.Row() )
                    continue;
                pDoc->GetCell( c, r, rAddr.Tab(), pCell );
                if( !pCell )
                    continue;

                if( !lcl_GetCellContent( pCell, false, nVal, aStr ) )
                {
                    if( rStr.Len() &&
                        ( ScGlobal::GetCollator()->compareString( rStr, aStr ) == COMPARE_EQUAL ) )
                        return true;
                }
                else
                {
                    if( !rStr.Len() && ::rtl::math::approxEqual( nArg, nVal ) )
                        return true;
                }
            }
    }
    return false;
}

bool ScConditionEntry::IsValid( double nArg, const ScAddress& rAddr ) const
{
    //  Interpret muss schon gerufen sein

    if ( bIsStr1 )
    {
        // wenn auf String getestet wird, bei Zahlen immer sal_False, ausser bei "ungleich"

        return ( eOp == SC_COND_NOTEQUAL );
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
                const ScRangeList& aRanges = pCondFormat->GetRange();
                bValid = lcl_IsDuplicate( mpDoc, nArg, String(), rAddr, aRanges );
                if( eOp == SC_COND_NOTDUPLICATE )
                    bValid = !bValid;
            }
            break;
        case SC_COND_DIRECT:
            bValid = !::rtl::math::approxEqual( nComp1, 0.0 );
            break;
        default:
            OSL_FAIL("unbekannte Operation bei ScConditionEntry");
            break;
    }
    return bValid;
}

bool ScConditionEntry::IsValidStr( const String& rArg, const ScAddress& rAddr ) const
{
    bool bValid = false;
    //  Interpret muss schon gerufen sein

    if ( eOp == SC_COND_DIRECT )                // Formel ist unabhaengig vom Inhalt
        return !::rtl::math::approxEqual( nVal1, 0.0 );

    if ( eOp == SC_COND_DUPLICATE || eOp == SC_COND_NOTDUPLICATE )
    {
        if( pCondFormat && rArg.Len() )
        {
            const ScRangeList& aRanges = pCondFormat->GetRange();
            bValid = lcl_IsDuplicate( mpDoc, 0.0, rArg, rAddr, aRanges );
            if( eOp == SC_COND_NOTDUPLICATE )
                bValid = !bValid;
            return bValid;
        }
    }

    //  Wenn Bedingung Zahl enthaelt, immer FALSE, ausser bei "ungleich"

    if ( !bIsStr1 )
        return ( eOp == SC_COND_NOTEQUAL );
    if ( eOp == SC_COND_BETWEEN || eOp == SC_COND_NOTBETWEEN )
        if ( !bIsStr2 )
            return false;

    String aUpVal1( aStrVal1 );     //! als Member? (dann auch in Interpret setzen)
    String aUpVal2( aStrVal2 );

    if ( eOp == SC_COND_BETWEEN || eOp == SC_COND_NOTBETWEEN )
        if ( ScGlobal::GetCollator()->compareString( aUpVal1, aUpVal2 )
                == COMPARE_GREATER )
        {
            //  richtige Reihenfolge fuer Wertebereich
            String aTemp( aUpVal1 ); aUpVal1 = aUpVal2; aUpVal2 = aTemp;
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
                    OSL_FAIL("unbekannte Operation bei ScConditionEntry");
                    bValid = false;
                    break;
            }
        }
    }
    return bValid;
}

bool ScConditionEntry::IsCellValid( ScBaseCell* pCell, const ScAddress& rPos ) const
{
    ((ScConditionEntry*)this)->Interpret(rPos);         // Formeln auswerten

    double nArg = 0.0;
    String aArgStr;
    bool bVal = lcl_GetCellContent( pCell, bIsStr1, nArg, aArgStr );
    if (bVal)
        return IsValid( nArg, rPos );
    else
        return IsValidStr( aArgStr, rPos );
}

String ScConditionEntry::GetExpression( const ScAddress& rCursor, sal_uInt16 nIndex,
                                        sal_uLong nNumFmt,
                                        const FormulaGrammar::Grammar eGrammar ) const
{
    String aRet;

    if ( FormulaGrammar::isEnglish( eGrammar) && nNumFmt == 0 )
        nNumFmt = mpDoc->GetFormatTable()->GetStandardIndex( LANGUAGE_ENGLISH_US );

    if ( nIndex==0 )
    {
        if ( pFormula1 )
        {
            ScCompiler aComp(mpDoc, rCursor, *pFormula1);
            aComp.SetGrammar(eGrammar);
            aComp.CreateStringFromTokenArray( aRet );
        }
        else if (bIsStr1)
        {
            aRet = '"';
            aRet += aStrVal1;
            aRet += '"';
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
            aComp.CreateStringFromTokenArray( aRet );
        }
        else if (bIsStr2)
        {
            aRet = '"';
            aRet += aStrVal2;
            aRet += '"';
        }
        else
            mpDoc->GetFormatTable()->GetInputLineString(nVal2, nNumFmt, aRet);
    }
    else
    {
        OSL_FAIL("GetExpression: falscher Index");
    }

    return aRet;
}

ScTokenArray* ScConditionEntry::CreateTokenArry( sal_uInt16 nIndex ) const
{
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
                pRet->AddString( aStrVal1.GetBuffer() );
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
                pRet->AddString( aStrVal2.GetBuffer() );
            else
                pRet->AddDouble( nVal2 );
        }
    }
    else
    {
        OSL_FAIL("GetExpression: falscher Index");
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
                        nCol2 = rChanged.Col() - aProv.Ref1.nRelCol;
                    else
                    {
                        bHit &= ( rChanged.Col() >= aProv.Ref1.nCol );
                        nCol2 = MAXCOL;
                    }
                    if ( aProv.Ref1.IsRowRel() )
                        nRow2 = rChanged.Row() - aProv.Ref1.nRelRow;
                    else
                    {
                        bHit &= ( rChanged.Row() >= aProv.Ref1.nRow );
                        nRow2 = MAXROW;
                    }
                    if ( aProv.Ref1.IsTabRel() )
                        nTab2 = rChanged.Tab() - aProv.Ref1.nRelTab;
                    else
                    {
                        bHit &= ( rChanged.Tab() >= aProv.Ref1.nTab );
                        nTab2 = MAXTAB;
                    }

                    if ( aProv.Ref2.IsColRel() )
                        nCol1 = rChanged.Col() - aProv.Ref2.nRelCol;
                    else
                    {
                        bHit &= ( rChanged.Col() <= aProv.Ref2.nCol );
                        nCol1 = 0;
                    }
                    if ( aProv.Ref2.IsRowRel() )
                        nRow1 = rChanged.Row() - aProv.Ref2.nRelRow;
                    else
                    {
                        bHit &= ( rChanged.Row() <= aProv.Ref2.nRow );
                        nRow1 = 0;
                    }
                    if ( aProv.Ref2.IsTabRel() )
                        nTab1 = rChanged.Tab() - aProv.Ref2.nRelTab;
                    else
                    {
                        bHit &= ( rChanged.Tab() <= aProv.Ref2.nTab );
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
//------------------------------------------------------------------------

ScCondFormatEntry::ScCondFormatEntry( ScConditionMode eOper,
                                        const String& rExpr1, const String& rExpr2,
                                        ScDocument* pDocument, const ScAddress& rPos,
                                        const String& rStyle,
                                        const String& rExprNmsp1, const String& rExprNmsp2,
                                        FormulaGrammar::Grammar eGrammar1,
                                        FormulaGrammar::Grammar eGrammar2 ) :
    ScConditionEntry( eOper, rExpr1, rExpr2, pDocument, rPos, rExprNmsp1, rExprNmsp2, eGrammar1, eGrammar2 ),
    aStyleName( rStyle )
{
}

ScCondFormatEntry::ScCondFormatEntry( ScConditionMode eOper,
                                        const ScTokenArray* pArr1, const ScTokenArray* pArr2,
                                        ScDocument* pDocument, const ScAddress& rPos,
                                        const String& rStyle ) :
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

ScConditionalFormat::ScConditionalFormat(sal_uInt32 nNewKey, ScDocument* pDocument) :
    pDoc( pDocument ),
    nKey( nNewKey )
{
}

ScConditionalFormat::ScConditionalFormat(const ScConditionalFormat& r) :
    pDoc( r.pDoc ),
    nKey( r.nKey ),
    maRanges( r.maRanges )
{
    for (CondFormatContainer::const_iterator itr = r.maEntries.begin(); itr != r.maEntries.end(); ++itr)
    {
        ScFormatEntry* pNewEntry = itr->Clone(r.pDoc);
        maEntries.push_back( pNewEntry );
        pNewEntry->SetParent(this);
    }
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

const String& ScConditionalFormat::GetCellStyle( ScBaseCell* pCell, const ScAddress& rPos ) const
{
    for (CondFormatContainer::const_iterator itr = maEntries.begin(); itr != maEntries.end(); ++itr)
    {
        if(itr->GetType() == condformat::CONDITION)
        {
            const ScCondFormatEntry& rEntry = static_cast<const ScCondFormatEntry&>(*itr);
            if ( rEntry.IsCellValid( pCell, rPos ) )
                return rEntry.GetStyle();
        }
    }

    return EMPTY_STRING;
}

ScCondFormatData ScConditionalFormat::GetData( ScBaseCell* pCell, const ScAddress& rPos ) const
{
    ScCondFormatData aData;
    for(CondFormatContainer::const_iterator itr = maEntries.begin(); itr != maEntries.end(); ++itr)
    {
        if(itr->GetType() == condformat::CONDITION && aData.aStyleName.isEmpty())
        {
            const ScCondFormatEntry& rEntry = static_cast<const ScCondFormatEntry&>(*itr);
            if ( rEntry.IsCellValid( pCell, rPos ) )
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
    }
    return aData;
}


#if DUMP_FORMAT_INFO
void ScConditionalFormat::dumpInfo() const
{
    for(CondFormatContainer::const_iterator itr = maEntries.begin(); itr != maEntries.end(); ++itr)
    {
        itr->dumpInfo();
    }
}
#endif

void ScConditionalFormat::DoRepaint( const ScRange* pModified )
{
    if(pModified)
        pDoc->RepaintRange(*pModified);
    else
    {
        // all conditional format cells
        size_t n = maRanges.size();
        for(size_t i = 0; i < n; ++i)
            pDoc->RepaintRange(*maRanges[i]);
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
                                const ScRange& rRange, SCsCOL nDx, SCsROW nDy, SCsTAB nDz )
{
    maRanges.UpdateReference( eUpdateRefMode, pDoc, rRange, nDx, nDy, nDz );
    for(CondFormatContainer::iterator itr = maEntries.begin(); itr != maEntries.end(); ++itr)
        itr->UpdateReference(eUpdateRefMode, rRange, nDx, nDy, nDz);
}

void ScConditionalFormat::DeleteArea( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 )
{
    SCTAB nTab = maRanges[0]->aStart.Tab();
    maRanges.DeleteArea( nCol1, nRow1, nTab, nCol2, nRow2, nTab );
}

void ScConditionalFormat::RenameCellStyle(const String& rOld, const String& rNew)
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
        if(itr->GetType() == condformat::CONDITION)
        {
            ScCondFormatEntry& rFormat = static_cast<ScCondFormatEntry&>(*itr);
            rFormat.SourceChanged( rAddr );
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

    OSL_FAIL("ScConditionalFormatList: Eintrag nicht gefunden");
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

void ScConditionalFormatList::RenameCellStyle( const String& rOld, const String& rNew )
{
    for( iterator itr = begin(); itr != end(); ++itr)
        itr->RenameCellStyle(rOld,rNew);
}

void ScConditionalFormatList::UpdateMoveTab( SCTAB nOldPos, SCTAB nNewPos )
{
    for( iterator itr = begin(); itr != end(); ++itr)
        itr->UpdateMoveTab( nOldPos, nNewPos );
}

void ScConditionalFormatList::DeleteArea( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 )
{
    for( iterator itr = begin(); itr != end(); ++itr)
        itr->DeleteArea( nCol1, nRow1, nCol2, nRow2 );

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
