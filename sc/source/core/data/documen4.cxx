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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include <svl/intitem.hxx>
#include <svl/zforlist.hxx>
#include <vcl/sound.hxx>
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

using namespace formula;

// -----------------------------------------------------------------------

// Nach der Regula Falsi Methode
BOOL ScDocument::Solver(SCCOL nFCol, SCROW nFRow, SCTAB nFTab,
                        SCCOL nVCol, SCROW nVRow, SCTAB nVTab,
                        const String& sValStr, double& nX)
{
    BOOL bRet = FALSE;
    nX = 0.0;
    if (ValidColRow(nFCol, nFRow) && ValidColRow(nVCol, nVRow) &&
        VALIDTAB(nFTab) && VALIDTAB(nVTab) && pTab[nFTab] && pTab[nVTab])
    {
        CellType eFType, eVType;
        GetCellType(nFCol, nFRow, nFTab, eFType);
        GetCellType(nVCol, nVRow, nVTab, eVType);
        // CELLTYPE_NOTE: no value, but referenced by formula
        // #i108005# convert target value to number using default format,
        // as previously done in ScInterpreter::GetDouble
        double nTargetVal = 0.0;
        sal_uInt32 nFIndex = 0;
        if (eFType == CELLTYPE_FORMULA && (eVType == CELLTYPE_VALUE || eVType == CELLTYPE_NOTE) &&
            GetFormatTable()->IsNumberFormat(sValStr, nFIndex, nTargetVal))
        {
            ScSingleRefData aRefData;
            aRefData.InitFlags();
            aRefData.nCol = nVCol;
            aRefData.nRow = nVRow;
            aRefData.nTab = nVTab;

            ScTokenArray aArr;
            aArr.AddOpCode( ocBackSolver );
            aArr.AddOpCode( ocOpen );
            aArr.AddSingleReference( aRefData );
            aArr.AddOpCode( ocSep );

            aRefData.nCol = nFCol;
            aRefData.nRow = nFRow;
            aRefData.nTab = nFTab;

            aArr.AddSingleReference( aRefData );
            aArr.AddOpCode( ocSep );
            aArr.AddDouble( nTargetVal );
            aArr.AddOpCode( ocClose );
            aArr.AddOpCode( ocStop );

            ScFormulaCell* pCell = new ScFormulaCell( this, ScAddress(), &aArr );

            if (pCell)
            {
                // FIXME FIXME FIXME this might need to be reworked now that we have formula::FormulaErrorToken and ScFormulaResult, double check !!!
                DBG_ERRORFILE("ScDocument::Solver: -> ScFormulaCell::GetValueAlways might need reimplementation");
                pCell->Interpret();
                USHORT nErrCode = pCell->GetErrCode();
                nX = pCell->GetValueAlways();
                if (nErrCode == 0)                  // kein fehler beim Rechnen
                    bRet = TRUE;
                delete pCell;
            }
        }
    }
    return bRet;
}

void ScDocument::InsertMatrixFormula(SCCOL nCol1, SCROW nRow1,
                                     SCCOL nCol2, SCROW nRow2,
                                     const ScMarkData& rMark,
                                     const String& rFormula,
                                     const ScTokenArray* pArr,
                                     const formula::FormulaGrammar::Grammar eGram )
{
    PutInOrder(nCol1, nCol2);
    PutInOrder(nRow1, nRow2);
    SCTAB i, nTab1;
    SCCOL j;
    SCROW k;
    i = 0;
    BOOL bStop = FALSE;
    while (i <= MAXTAB && !bStop)               // erste markierte Tabelle finden
    {
        if (pTab[i] && rMark.GetTableSelect(i))
            bStop = TRUE;
        else
            i++;
    }
    nTab1 = i;
    if (i == MAXTAB + 1)
    {
        Sound::Beep();
        OSL_FAIL("ScDocument::InsertMatrixFormula Keine Tabelle markiert");
        return;
    }

    ScFormulaCell* pCell;
    ScAddress aPos( nCol1, nRow1, nTab1 );
    if (pArr)
        pCell = new ScFormulaCell( this, aPos, pArr, eGram, MM_FORMULA );
    else
        pCell = new ScFormulaCell( this, aPos, rFormula, eGram, MM_FORMULA );
    pCell->SetMatColsRows( nCol2 - nCol1 + 1, nRow2 - nRow1 + 1 );
    for (i = 0; i <= MAXTAB; i++)
    {
        if (pTab[i] && rMark.GetTableSelect(i))
        {
            if (i == nTab1)
                pTab[i]->PutCell(nCol1, nRow1, pCell);
            else
                pTab[i]->PutCell(nCol1, nRow1, pCell->CloneWithoutNote(*this, ScAddress( nCol1, nRow1, i), SC_CLONECELL_STARTLISTENING));
        }
    }

    ScSingleRefData aRefData;
    aRefData.InitFlags();
    aRefData.nCol = nCol1;
    aRefData.nRow = nRow1;
    aRefData.nTab = nTab1;
    aRefData.SetColRel( TRUE );
    aRefData.SetRowRel( TRUE );
    aRefData.SetTabRel( TRUE );
    aRefData.CalcRelFromAbs( ScAddress( nCol1, nRow1, nTab1 ) );

    ScTokenArray aArr;
    ScToken* t = static_cast<ScToken*>(aArr.AddMatrixSingleReference( aRefData));

    for (i = 0; i <= MAXTAB; i++)
    {
        if (pTab[i] && rMark.GetTableSelect(i))
        {
            pTab[i]->DoColResize( nCol1, nCol2, static_cast<SCSIZE>(nRow2 - nRow1 + 1) );
            if (i != nTab1)
            {
                aRefData.nTab = i;
                aRefData.nRelTab = i - nTab1;
                t->GetSingleRef() = aRefData;
            }
            for (j = nCol1; j <= nCol2; j++)
            {
                for (k = nRow1; k <= nRow2; k++)
                {
                    if (j != nCol1 || k != nRow1)       // nicht in der ersten Zelle
                    {
                        // Array muss geklont werden, damit jede
                        // Zelle ein eigenes Array erhaelt!
                        aPos = ScAddress( j, k, i );
                        t->CalcRelFromAbs( aPos );
                        pCell = new ScFormulaCell( this, aPos, aArr.Clone(), eGram, MM_REFERENCE );
                        pTab[i]->PutCell(j, k, (ScBaseCell*) pCell);
                    }
                }
            }
        }
    }
}

void ScDocument::InsertTableOp(const ScTabOpParam& rParam,      // Mehrfachoperation
                               SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                               const ScMarkData& rMark)
{
    PutInOrder(nCol1, nCol2);
    PutInOrder(nRow1, nRow2);
    SCTAB i, nTab1;
    SCCOL j;
    SCROW k;
    i = 0;
    BOOL bStop = FALSE;
    while (i <= MAXTAB && !bStop)               // erste markierte Tabelle finden
    {
        if (pTab[i] && rMark.GetTableSelect(i))
            bStop = TRUE;
        else
            i++;
    }
    nTab1 = i;
    if (i == MAXTAB + 1)
    {
        Sound::Beep();
        OSL_FAIL("ScDocument::InsertTableOp: Keine Tabelle markiert");
        return;
    }

    ScRefAddress aRef;
    String aForString = '=';
    aForString += ScCompiler::GetNativeSymbol(ocTableOp);
    aForString += ScCompiler::GetNativeSymbol( ocOpen);

    const String& sSep = ScCompiler::GetNativeSymbol( ocSep);
    if (rParam.nMode == 0)                          // nur Spalte
    {
        aRef.Set( rParam.aRefFormulaCell.GetAddress(), TRUE, FALSE, FALSE );
        aForString += aRef.GetRefString(this, nTab1);
        aForString += sSep;
        aForString += rParam.aRefColCell.GetRefString(this, nTab1);
        aForString += sSep;
        aRef.Set( nCol1, nRow1, nTab1, FALSE, TRUE, TRUE );
        aForString += aRef.GetRefString(this, nTab1);
        nCol1++;
        nCol2 = Min( nCol2, (SCCOL)(rParam.aRefFormulaEnd.Col() -
                    rParam.aRefFormulaCell.Col() + nCol1 + 1));
    }
    else if (rParam.nMode == 1)                 // nur zeilenweise
    {
        aRef.Set( rParam.aRefFormulaCell.GetAddress(), FALSE, TRUE, FALSE );
        aForString += aRef.GetRefString(this, nTab1);
        aForString += sSep;
        aForString += rParam.aRefRowCell.GetRefString(this, nTab1);
        aForString += sSep;
        aRef.Set( nCol1, nRow1, nTab1, TRUE, FALSE, TRUE );
        aForString += aRef.GetRefString(this, nTab1);
        nRow1++;
        nRow2 = Min( nRow2, (SCROW)(rParam.aRefFormulaEnd.Row() -
                    rParam.aRefFormulaCell.Row() + nRow1 + 1));
    }
    else                    // beides
    {
        aForString += rParam.aRefFormulaCell.GetRefString(this, nTab1);
        aForString += sSep;
        aForString += rParam.aRefColCell.GetRefString(this, nTab1);
        aForString += sSep;
        aRef.Set( nCol1, nRow1 + 1, nTab1, FALSE, TRUE, TRUE );
        aForString += aRef.GetRefString(this, nTab1);
        aForString += sSep;
        aForString += rParam.aRefRowCell.GetRefString(this, nTab1);
        aForString += sSep;
        aRef.Set( nCol1 + 1, nRow1, nTab1, TRUE, FALSE, TRUE );
        aForString += aRef.GetRefString(this, nTab1);
        nCol1++; nRow1++;
    }
    aForString += ScCompiler::GetNativeSymbol( ocClose);

    ScFormulaCell aRefCell( this, ScAddress( nCol1, nRow1, nTab1 ), aForString,
           formula::FormulaGrammar::GRAM_NATIVE, MM_NONE );
    for( j = nCol1; j <= nCol2; j++ )
        for( k = nRow1; k <= nRow2; k++ )
            for (i = 0; i <= MAXTAB; i++)
                if( pTab[i] && rMark.GetTableSelect(i) )
                    pTab[i]->PutCell( j, k, aRefCell.CloneWithoutNote( *this, ScAddress( j, k, i ), SC_CLONECELL_STARTLISTENING ) );
}

bool ScDocument::MarkUsedExternalReferences( ScTokenArray & rArr )
{
    bool bAllMarked = false;
    if (rArr.GetLen())
    {
        ScExternalRefManager* pRefMgr = NULL;
        rArr.Reset();
        ScToken* t;
        while (!bAllMarked && (t = static_cast<ScToken*>(rArr.GetNextReferenceOrName())) != NULL)
        {
            if (t->IsExternalRef())
            {
                if (!pRefMgr)
                    pRefMgr = GetExternalRefManager();
                switch (t->GetType())
                {
                    case svExternalSingleRef:
                        bAllMarked = pRefMgr->setCacheTableReferenced(
                                t->GetIndex(), t->GetString(), 1);
                        break;
                    case svExternalDoubleRef:
                        {
                            const ScComplexRefData& rRef = t->GetDoubleRef();
                            size_t nSheets = rRef.Ref2.nTab - rRef.Ref1.nTab + 1;
                            bAllMarked = pRefMgr->setCacheTableReferenced(
                                    t->GetIndex(), t->GetString(), nSheets);
                        }
                        break;
                    case svExternalName:
                        /* TODO: external names aren't supported yet, but would
                         * have to be marked as well, if so. Mechanism would be
                         * different. */
                        DBG_ERRORFILE("ScDocument::MarkUsedExternalReferences: implement the svExternalName case!");
                        break;
                    default: break;
                }
            }
        }
    }
    return bAllMarked;
}

BOOL ScDocument::GetNextSpellingCell(SCCOL& nCol, SCROW& nRow, SCTAB nTab,
                        BOOL bInSel, const ScMarkData& rMark) const
{
    if (ValidTab(nTab) && pTab[nTab])
        return pTab[nTab]->GetNextSpellingCell( nCol, nRow, bInSel, rMark );
    else
        return FALSE;
}

BOOL ScDocument::GetNextMarkedCell( SCCOL& rCol, SCROW& rRow, SCTAB nTab,
                                        const ScMarkData& rMark )
{
    if (ValidTab(nTab) && pTab[nTab])
        return pTab[nTab]->GetNextMarkedCell( rCol, rRow, rMark );
    else
        return FALSE;
}

BOOL ScDocument::ReplaceStyle(const SvxSearchItem& rSearchItem,
                              SCCOL nCol, SCROW nRow, SCTAB nTab,
                              ScMarkData& rMark,
                              BOOL bIsUndoP)
{
    if (pTab[nTab])
        return pTab[nTab]->ReplaceStyle(rSearchItem, nCol, nRow, rMark, bIsUndoP);
    else
        return FALSE;
}

void ScDocument::CompileDBFormula()
{
    for (SCTAB i=0; i<=MAXTAB; i++)
    {
        if (pTab[i]) pTab[i]->CompileDBFormula();
    }
}

void ScDocument::CompileDBFormula( BOOL bCreateFormulaString )
{
    for (SCTAB i=0; i<=MAXTAB; i++)
    {
        if (pTab[i]) pTab[i]->CompileDBFormula( bCreateFormulaString );
    }
}

void ScDocument::CompileNameFormula( BOOL bCreateFormulaString )
{
    if ( pCondFormList )
        pCondFormList->CompileAll();    // nach ScNameDlg noetig

    for (SCTAB i=0; i<=MAXTAB; i++)
    {
        if (pTab[i]) pTab[i]->CompileNameFormula( bCreateFormulaString );
    }
}

void ScDocument::CompileColRowNameFormula()
{
    for (SCTAB i=0; i<=MAXTAB; i++)
    {
        if (pTab[i]) pTab[i]->CompileColRowNameFormula();
    }
}

void ScDocument::DoColResize( SCTAB nTab, SCCOL nCol1, SCCOL nCol2, SCSIZE nAdd )
{
    if (ValidTab(nTab) && pTab[nTab])
        pTab[nTab]->DoColResize( nCol1, nCol2, nAdd );
    else
    {
        OSL_FAIL("DoColResize: falsche Tabelle");
    }
}

void ScDocument::InvalidateTableArea()
{
    for (SCTAB nTab=0; nTab<=MAXTAB && pTab[nTab]; nTab++)
    {
        pTab[nTab]->InvalidateTableArea();
        if ( pTab[nTab]->IsScenario() )
            pTab[nTab]->InvalidateScenarioRanges();
    }
}

sal_Int32 ScDocument::GetMaxStringLen( SCTAB nTab, SCCOL nCol,
        SCROW nRowStart, SCROW nRowEnd, CharSet eCharSet ) const
{
    if (ValidTab(nTab) && pTab[nTab])
        return pTab[nTab]->GetMaxStringLen( nCol, nRowStart, nRowEnd, eCharSet );
    else
        return 0;
}

xub_StrLen ScDocument::GetMaxNumberStringLen( sal_uInt16& nPrecision, SCTAB nTab,
                                    SCCOL nCol,
                                    SCROW nRowStart, SCROW nRowEnd ) const
{
    if (ValidTab(nTab) && pTab[nTab])
        return pTab[nTab]->GetMaxNumberStringLen( nPrecision, nCol,
            nRowStart, nRowEnd );
    else
        return 0;
}

BOOL ScDocument::GetSelectionFunction( ScSubTotalFunc eFunc,
                                        const ScAddress& rCursor, const ScMarkData& rMark,
                                        double& rResult )
{
    ScFunctionData aData(eFunc);

    ScRange aSingle( rCursor );
    if ( rMark.IsMarked() )
        rMark.GetMarkArea(aSingle);

    SCCOL nStartCol = aSingle.aStart.Col();
    SCROW nStartRow = aSingle.aStart.Row();
    SCCOL nEndCol = aSingle.aEnd.Col();
    SCROW nEndRow = aSingle.aEnd.Row();

    for (SCTAB nTab=0; nTab<=MAXTAB && !aData.bError; nTab++)
        if (pTab[nTab] && rMark.GetTableSelect(nTab))
            pTab[nTab]->UpdateSelectionFunction( aData,
                            nStartCol, nStartRow, nEndCol, nEndRow, rMark );

            //! rMark an UpdateSelectionFunction uebergeben !!!!!

    if (!aData.bError)
        switch (eFunc)
        {
            case SUBTOTAL_FUNC_SUM:
                rResult = aData.nVal;
                break;
            case SUBTOTAL_FUNC_CNT:
            case SUBTOTAL_FUNC_CNT2:
                rResult = aData.nCount;
                break;
            case SUBTOTAL_FUNC_AVE:
                if (aData.nCount)
                    rResult = aData.nVal / (double) aData.nCount;
                else
                    aData.bError = TRUE;
                break;
            case SUBTOTAL_FUNC_MAX:
            case SUBTOTAL_FUNC_MIN:
                if (aData.nCount)
                    rResult = aData.nVal;
                else
                    aData.bError = TRUE;
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

double ScDocument::RoundValueAsShown( double fVal, ULONG nFormat )
{
    short nType;
    if ( (nType = GetFormatTable()->GetType( nFormat )) != NUMBERFORMAT_DATE
      && nType != NUMBERFORMAT_TIME && nType != NUMBERFORMAT_DATETIME )
    {
        short nPrecision;
        if ((nFormat % SV_COUNTRY_LANGUAGE_OFFSET) != 0)
        {
            nPrecision = (short)GetFormatTable()->GetFormatPrecision( nFormat );
            switch ( nType )
            {
                case NUMBERFORMAT_PERCENT:      // 0,41% == 0,0041
                    nPrecision += 2;
                    break;
                case NUMBERFORMAT_SCIENTIFIC:   // 1,23e-3 == 0,00123
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
            return fVal;        // durch Rundung hoechstens Fehler
        else
            return fRound;
    }
    else
        return fVal;
}

//
//          bedingte Formate und Gueltigkeitsbereiche
//

ULONG ScDocument::AddCondFormat( const ScConditionalFormat& rNew )
{
    if (rNew.IsEmpty())
        return 0;                   // leer ist immer 0

    if (!pCondFormList)
        pCondFormList = new ScConditionalFormatList;

    ULONG nMax = 0;
    USHORT nCount = pCondFormList->Count();
    for (USHORT i=0; i<nCount; i++)
    {
        const ScConditionalFormat* pForm = (*pCondFormList)[i];
        ULONG nKey = pForm->GetKey();
        if ( pForm->EqualEntries( rNew ) )
            return nKey;
        if ( nKey > nMax )
            nMax = nKey;
    }

    // Der Aufruf kann aus ScPatternAttr::PutInPool kommen, darum Clone (echte Kopie)

    ULONG nNewKey = nMax + 1;
    ScConditionalFormat* pInsert = rNew.Clone(this);
    pInsert->SetKey( nNewKey );
    pCondFormList->InsertNew( pInsert );
    return nNewKey;
}

ULONG ScDocument::AddValidationEntry( const ScValidationData& rNew )
{
    if (rNew.IsEmpty())
        return 0;                   // leer ist immer 0

    if (!pValidationList)
        pValidationList = new ScValidationDataList;

    ULONG nMax = 0;
    USHORT nCount = pValidationList->Count();
    for (USHORT i=0; i<nCount; i++)
    {
        const ScValidationData* pData = (*pValidationList)[i];
        ULONG nKey = pData->GetKey();
        if ( pData->EqualEntries( rNew ) )
            return nKey;
        if ( nKey > nMax )
            nMax = nKey;
    }

    // Der Aufruf kann aus ScPatternAttr::PutInPool kommen, darum Clone (echte Kopie)

    ULONG nNewKey = nMax + 1;
    ScValidationData* pInsert = rNew.Clone(this);
    pInsert->SetKey( nNewKey );
    pValidationList->InsertNew( pInsert );
    return nNewKey;
}

const SfxPoolItem* ScDocument::GetEffItem(
                        SCCOL nCol, SCROW nRow, SCTAB nTab, USHORT nWhich ) const
{
    const ScPatternAttr* pPattern = GetPattern( nCol, nRow, nTab );
    if ( pPattern )
    {
        const SfxItemSet& rSet = pPattern->GetItemSet();
        const SfxPoolItem* pItem;
        if ( rSet.GetItemState( ATTR_CONDITIONAL, TRUE, &pItem ) == SFX_ITEM_SET )
        {
            ULONG nIndex = ((const SfxUInt32Item*)pItem)->GetValue();
            if (nIndex && pCondFormList)
            {
                const ScConditionalFormat* pForm = pCondFormList->GetFormat( nIndex );
                if ( pForm )
                {
                    ScBaseCell* pCell = ((ScDocument*)this)->GetCell(ScAddress(nCol,nRow,nTab));
                    String aStyle = pForm->GetCellStyle( pCell, ScAddress(nCol, nRow, nTab) );
                    if (aStyle.Len())
                    {
                        SfxStyleSheetBase* pStyleSheet = xPoolHelper->GetStylePool()->Find(
                                                                aStyle, SFX_STYLE_FAMILY_PARA );
                        if ( pStyleSheet && pStyleSheet->GetItemSet().GetItemState(
                                                nWhich, TRUE, &pItem ) == SFX_ITEM_SET )
                            return pItem;
                    }
                }
            }
        }
        return &rSet.Get( nWhich );
    }
    OSL_FAIL("kein Pattern");
    return NULL;
}

const SfxItemSet* ScDocument::GetCondResult( SCCOL nCol, SCROW nRow, SCTAB nTab ) const
{
    const ScConditionalFormat* pForm = GetCondFormat( nCol, nRow, nTab );
    if ( pForm )
    {
        ScBaseCell* pCell = ((ScDocument*)this)->GetCell(ScAddress(nCol,nRow,nTab));
        String aStyle = pForm->GetCellStyle( pCell, ScAddress(nCol, nRow, nTab) );
        if (aStyle.Len())
        {
            SfxStyleSheetBase* pStyleSheet = xPoolHelper->GetStylePool()->Find( aStyle, SFX_STYLE_FAMILY_PARA );
            if ( pStyleSheet )
                return &pStyleSheet->GetItemSet();
            // if style is not there, treat like no condition
        }
    }
    return NULL;
}

const ScConditionalFormat* ScDocument::GetCondFormat(
                            SCCOL nCol, SCROW nRow, SCTAB nTab ) const
{
    ULONG nIndex = ((const SfxUInt32Item*)GetAttr(nCol,nRow,nTab,ATTR_CONDITIONAL))->GetValue();
    if (nIndex)
    {
        if (pCondFormList)
            return pCondFormList->GetFormat( nIndex );
        else
        {
            OSL_FAIL("pCondFormList ist 0");
        }
    }

    return NULL;
}

const ScValidationData* ScDocument::GetValidationEntry( ULONG nIndex ) const
{
    if ( pValidationList )
        return pValidationList->GetData( nIndex );
    else
        return NULL;
}

void ScDocument::FindConditionalFormat( ULONG nKey, ScRangeList& rRanges )
{
    for (SCTAB i=0; i<=MAXTAB && pTab[i]; i++)
        pTab[i]->FindConditionalFormat( nKey, rRanges );
}

void ScDocument::FindConditionalFormat( ULONG nKey, ScRangeList& rRanges, SCTAB nTab )
{
    if(VALIDTAB(nTab) && pTab[nTab])
        pTab[nTab]->FindConditionalFormat( nKey, rRanges );
}

void ScDocument::ConditionalChanged( ULONG nKey )
{
    if ( nKey && pCondFormList && !bIsClip && !bIsUndo )        // nKey==0 -> noop
    {
        ScConditionalFormat* pForm = pCondFormList->GetFormat( nKey );
        if (pForm)
            pForm->InvalidateArea();
    }
}

void ScDocument::SetCondFormList(ScConditionalFormatList* pNew)
{
    if (pCondFormList)
    {
        pCondFormList->DeleteAndDestroy( 0, pCondFormList->Count() );
        delete pCondFormList;
    }

    pCondFormList = pNew;
}

//------------------------------------------------------------------------

BOOL ScDocument::HasDetectiveOperations() const
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
    delete pDetOpList;      // loescht auch die Eintraege
    pDetOpList = NULL;
}

void ScDocument::SetDetOpList(ScDetOpList* pNew)
{
    delete pDetOpList;      // loescht auch die Eintraege
    pDetOpList = pNew;
}

//------------------------------------------------------------------------
//
//      Vergleich von Dokumenten
//
//------------------------------------------------------------------------

//  Pfriemel-Faktoren
#define SC_DOCCOMP_MAXDIFF  256
#define SC_DOCCOMP_MINGOOD  128
#define SC_DOCCOMP_COLUMNS  10
#define SC_DOCCOMP_ROWS     100


USHORT ScDocument::RowDifferences( SCROW nThisRow, SCTAB nThisTab,
                                    ScDocument& rOtherDoc, SCROW nOtherRow, SCTAB nOtherTab,
                                    SCCOL nMaxCol, SCCOLROW* pOtherCols )
{
    ULONG nDif = 0;
    ULONG nUsed = 0;
    for (SCCOL nThisCol=0; nThisCol<=nMaxCol; nThisCol++)
    {
        SCCOL nOtherCol;
        if ( pOtherCols )
            nOtherCol = static_cast<SCCOL>(pOtherCols[nThisCol]);
        else
            nOtherCol = nThisCol;

        if (ValidCol(nOtherCol))    // nur Spalten vergleichen, die in beiden Dateien sind
        {
            const ScBaseCell* pThisCell = GetCell( ScAddress( nThisCol, nThisRow, nThisTab ) );
            const ScBaseCell* pOtherCell = rOtherDoc.GetCell( ScAddress( nOtherCol, nOtherRow, nOtherTab ) );
            if (!ScBaseCell::CellEqual( pThisCell, pOtherCell ))
            {
                if ( pThisCell && pOtherCell )
                    nDif += 3;
                else
                    nDif += 4;      // Inhalt <-> leer zaehlt mehr
            }

            if ( ( pThisCell  && pThisCell->GetCellType()!=CELLTYPE_NOTE ) ||
                 ( pOtherCell && pOtherCell->GetCellType()!=CELLTYPE_NOTE ) )
                ++nUsed;
        }
    }

    if (nUsed > 0)
        return static_cast<USHORT>((nDif*64)/nUsed);            // max.256 (SC_DOCCOMP_MAXDIFF)

    DBG_ASSERT(!nDif,"Diff ohne Used");
    return 0;
}

USHORT ScDocument::ColDifferences( SCCOL nThisCol, SCTAB nThisTab,
                                    ScDocument& rOtherDoc, SCCOL nOtherCol, SCTAB nOtherTab,
                                    SCROW nMaxRow, SCCOLROW* pOtherRows )
{
    //! optimieren mit Iterator oder so

    ULONG nDif = 0;
    ULONG nUsed = 0;
    for (SCROW nThisRow=0; nThisRow<=nMaxRow; nThisRow++)
    {
        SCROW nOtherRow;
        if ( pOtherRows )
            nOtherRow = pOtherRows[nThisRow];
        else
            nOtherRow = nThisRow;

        if (ValidRow(nOtherRow))    // nur Zeilen vergleichen, die in beiden Dateien sind
        {
            const ScBaseCell* pThisCell = GetCell( ScAddress( nThisCol, nThisRow, nThisTab ) );
            const ScBaseCell* pOtherCell = rOtherDoc.GetCell( ScAddress( nOtherCol, nOtherRow, nOtherTab ) );
            if (!ScBaseCell::CellEqual( pThisCell, pOtherCell ))
            {
                if ( pThisCell && pOtherCell )
                    nDif += 3;
                else
                    nDif += 4;      // Inhalt <-> leer zaehlt mehr
            }

            if ( ( pThisCell  && pThisCell->GetCellType()!=CELLTYPE_NOTE ) ||
                 ( pOtherCell && pOtherCell->GetCellType()!=CELLTYPE_NOTE ) )
                ++nUsed;
        }
    }

    if (nUsed > 0)
        return static_cast<USHORT>((nDif*64)/nUsed);    // max.256

    DBG_ASSERT(!nDif,"Diff ohne Used");
    return 0;
}

void ScDocument::FindOrder( SCCOLROW* pOtherRows, SCCOLROW nThisEndRow, SCCOLROW nOtherEndRow,
                            BOOL bColumns, ScDocument& rOtherDoc, SCTAB nThisTab, SCTAB nOtherTab,
                            SCCOLROW nEndCol, SCCOLROW* pTranslate, ScProgress* pProgress, ULONG nProAdd )
{
    //  bColumns=TRUE: Zeilen sind Spalten und umgekehrt

    SCCOLROW nMaxCont;                      // wieviel weiter
    SCCOLROW nMinGood;                      // was ist ein Treffer (incl.)
    if ( bColumns )
    {
        nMaxCont = SC_DOCCOMP_COLUMNS;      // 10 Spalten
        nMinGood = SC_DOCCOMP_MINGOOD;
        //! Extra Durchgang mit nMinGood = 0 ????
    }
    else
    {
        nMaxCont = SC_DOCCOMP_ROWS;         // 100 Zeilen
        nMinGood = SC_DOCCOMP_MINGOOD;
    }
    BOOL bUseTotal = bColumns && !pTranslate;       // nur beim ersten Durchgang


    SCCOLROW nOtherRow = 0;
    USHORT nComp;
    SCCOLROW nThisRow;
    BOOL bTotal = FALSE;        // ueber verschiedene nThisRow beibehalten
    SCCOLROW nUnknown = 0;
    for (nThisRow = 0; nThisRow <= nThisEndRow; nThisRow++)
    {
        SCCOLROW nTempOther = nOtherRow;
        BOOL bFound = FALSE;
        USHORT nBest = SC_DOCCOMP_MAXDIFF;
        SCCOLROW nMax = Min( nOtherEndRow, static_cast<SCCOLROW>(( nTempOther + nMaxCont + nUnknown )) );
        for (SCCOLROW i=nTempOther; i<=nMax && nBest>0; i++)    // bei 0 abbrechen
        {
            if (bColumns)
                nComp = ColDifferences( static_cast<SCCOL>(nThisRow), nThisTab, rOtherDoc, static_cast<SCCOL>(i), nOtherTab, nEndCol, pTranslate );
            else
                nComp = RowDifferences( nThisRow, nThisTab, rOtherDoc, i, nOtherTab, static_cast<SCCOL>(nEndCol), pTranslate );
            if ( nComp < nBest && ( nComp <= nMinGood || bTotal ) )
            {
                nTempOther = i;
                nBest = nComp;
                bFound = TRUE;
            }
            if ( nComp < SC_DOCCOMP_MAXDIFF || bFound )
                bTotal = FALSE;
            else if ( i == nTempOther && bUseTotal )
                bTotal = TRUE;                          // nur ganz oben
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
            pProgress->SetStateOnPercent(nProAdd+static_cast<ULONG>(nThisRow));
    }

    //  Bloecke ohne Uebereinstimmung ausfuellen

    SCROW nFillStart = 0;
    SCROW nFillPos = 0;
    BOOL bInFill = FALSE;
    for (nThisRow = 0; nThisRow <= nThisEndRow+1; nThisRow++)
    {
        SCROW nThisOther = ( nThisRow <= nThisEndRow ) ? pOtherRows[nThisRow] : (nOtherEndRow+1);
        if ( ValidRow(nThisOther) )
        {
            if ( bInFill )
            {
                if ( nThisOther > nFillStart )      // ist was zu verteilen da?
                {
                    SCROW nDiff1 = nThisOther - nFillStart;
                    SCROW nDiff2 = nThisRow   - nFillPos;
                    SCROW nMinDiff = Min(nDiff1, nDiff2);
                    for (SCROW i=0; i<nMinDiff; i++)
                        pOtherRows[nFillPos+i] = nFillStart+i;
                }

                bInFill = FALSE;
            }
            nFillStart = nThisOther + 1;
            nFillPos = nThisRow + 1;
        }
        else
            bInFill = TRUE;
    }
}

void ScDocument::CompareDocument( ScDocument& rOtherDoc )
{
    if (!pChangeTrack)
        return;

    SCTAB nThisCount = GetTableCount();
    SCTAB nOtherCount = rOtherDoc.GetTableCount();
    SCTAB* pOtherTabs = new SCTAB[nThisCount];
    SCTAB nThisTab;

    //  Tabellen mit gleichen Namen vergleichen
    String aThisName;
    String aOtherName;
    for (nThisTab=0; nThisTab<nThisCount; nThisTab++)
    {
        SCTAB nOtherTab = SCTAB_MAX;
        if (!IsScenario(nThisTab))  // Szenarien weglassen
        {
            GetName( nThisTab, aThisName );
            for (SCTAB nTemp=0; nTemp<nOtherCount && nOtherTab>MAXTAB; nTemp++)
                if (!rOtherDoc.IsScenario(nTemp))
                {
                    rOtherDoc.GetName( nTemp, aOtherName );
                    if ( aThisName == aOtherName )
                        nOtherTab = nTemp;
                }
        }
        pOtherTabs[nThisTab] = nOtherTab;
    }
    //  auffuellen, damit einzeln umbenannte Tabellen nicht wegfallen
    SCTAB nFillStart = 0;
    SCTAB nFillPos = 0;
    BOOL bInFill = FALSE;
    for (nThisTab = 0; nThisTab <= nThisCount; nThisTab++)
    {
        SCTAB nThisOther = ( nThisTab < nThisCount ) ? pOtherTabs[nThisTab] : nOtherCount;
        if ( ValidTab(nThisOther) )
        {
            if ( bInFill )
            {
                if ( nThisOther > nFillStart )      // ist was zu verteilen da?
                {
                    SCTAB nDiff1 = nThisOther - nFillStart;
                    SCTAB nDiff2 = nThisTab   - nFillPos;
                    SCTAB nMinDiff = Min(nDiff1, nDiff2);
                    for (SCTAB i=0; i<nMinDiff; i++)
                        if ( !IsScenario(nFillPos+i) && !rOtherDoc.IsScenario(nFillStart+i) )
                            pOtherTabs[nFillPos+i] = nFillStart+i;
                }

                bInFill = FALSE;
            }
            nFillStart = nThisOther + 1;
            nFillPos = nThisTab + 1;
        }
        else
            bInFill = TRUE;
    }

    //
    //  Tabellen in der gefundenen Reihenfolge vergleichen
    //

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
            SCCOL nEndCol = Max(nThisEndCol, nOtherEndCol);
            SCROW nEndRow = Max(nThisEndRow, nOtherEndRow);
            SCCOL nThisCol;
            SCROW nThisRow;
            ULONG n1,n2;    // fuer AppendDeleteRange

            //! ein Progress ueber alle Tabellen ???
            String aTabName;
            GetName( nThisTab, aTabName );
            String aTemplate = ScGlobal::GetRscString(STR_PROGRESS_COMPARING);
            String aProText = aTemplate.GetToken( 0, '#' );
            aProText += aTabName;
            aProText += aTemplate.GetToken( 1, '#' );
            ScProgress aProgress( GetDocumentShell(),
                                        aProText, 3*nThisEndRow );  // 2x FindOrder, 1x hier
            long nProgressStart = 2*nThisEndRow;                    // start fuer hier

            SCCOLROW* pTempRows = new SCCOLROW[nThisEndRow+1];
            SCCOLROW* pOtherRows = new SCCOLROW[nThisEndRow+1];
            SCCOLROW* pOtherCols = new SCCOLROW[nThisEndCol+1];

            //  eingefuegte/geloeschte Spalten/Zeilen finden:
            //  Zwei Versuche:
            //  1) Original Zeilen vergleichen                          (pTempRows)
            //  2) Original Spalten vergleichen                         (pOtherCols)
            //     mit dieser Spaltenreihenfolge Zeilen vergleichen     (pOtherRows)

            //! Spalten vergleichen zweimal mit unterschiedlichem nMinGood ???

            // 1
            FindOrder( pTempRows, nThisEndRow, nOtherEndRow, FALSE,
                        rOtherDoc, nThisTab, nOtherTab, nEndCol, NULL, &aProgress, 0 );
            // 2
            FindOrder( pOtherCols, nThisEndCol, nOtherEndCol, TRUE,
                        rOtherDoc, nThisTab, nOtherTab, nEndRow, NULL, NULL, 0 );
            FindOrder( pOtherRows, nThisEndRow, nOtherEndRow, FALSE,
                        rOtherDoc, nThisTab, nOtherTab, nThisEndCol,
                        pOtherCols, &aProgress, nThisEndRow );

            ULONG nMatch1 = 0;  // pTempRows, keine Spalten
            for (nThisRow = 0; nThisRow<=nThisEndRow; nThisRow++)
                if (ValidRow(pTempRows[nThisRow]))
                    nMatch1 += SC_DOCCOMP_MAXDIFF -
                               RowDifferences( nThisRow, nThisTab, rOtherDoc, pTempRows[nThisRow],
                                                nOtherTab, nEndCol, NULL );

            ULONG nMatch2 = 0;  // pOtherRows, pOtherCols
            for (nThisRow = 0; nThisRow<=nThisEndRow; nThisRow++)
                if (ValidRow(pOtherRows[nThisRow]))
                    nMatch2 += SC_DOCCOMP_MAXDIFF -
                               RowDifferences( nThisRow, nThisTab, rOtherDoc, pOtherRows[nThisRow],
                                                nOtherTab, nThisEndCol, pOtherCols );

            if ( nMatch1 >= nMatch2 )           // ohne Spalten ?
            {
                //  Spalten zuruecksetzen
                for (nThisCol = 0; nThisCol<=nThisEndCol; nThisCol++)
                    pOtherCols[nThisCol] = nThisCol;

                //  Zeilenarrays vertauschen (geloescht werden sowieso beide)
                SCCOLROW* pSwap = pTempRows;
                pTempRows = pOtherRows;
                pOtherRows = pSwap;
            }
            else
            {
                //  bleibt bei pOtherCols, pOtherRows
            }


            //  Change-Actions erzeugen
            //  1) Spalten von rechts
            //  2) Zeilen von unten
            //  3) einzelne Zellen in normaler Reihenfolge

            //  Actions fuer eingefuegte/geloeschte Spalten

            SCCOL nLastOtherCol = static_cast<SCCOL>(nOtherEndCol + 1);
            //  nThisEndCol ... 0
            for ( nThisCol = nThisEndCol+1; nThisCol > 0; )
            {
                --nThisCol;
                SCCOL nOtherCol = static_cast<SCCOL>(pOtherCols[nThisCol]);
                if ( ValidCol(nOtherCol) && nOtherCol+1 < nLastOtherCol )
                {
                    // Luecke -> geloescht
                    ScRange aDelRange( nOtherCol+1, 0, nOtherTab,
                                        nLastOtherCol-1, MAXROW, nOtherTab );
                    pChangeTrack->AppendDeleteRange( aDelRange, &rOtherDoc, n1, n2 );
                }
                if ( nOtherCol > MAXCOL )                       // eingefuegt
                {
                    //  zusammenfassen
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
            if ( nLastOtherCol > 0 )                            // ganz oben geloescht
            {
                ScRange aDelRange( 0, 0, nOtherTab,
                                    nLastOtherCol-1, MAXROW, nOtherTab );
                pChangeTrack->AppendDeleteRange( aDelRange, &rOtherDoc, n1, n2 );
            }

            //  Actions fuer eingefuegte/geloeschte Zeilen

            SCROW nLastOtherRow = nOtherEndRow + 1;
            //  nThisEndRow ... 0
            for ( nThisRow = nThisEndRow+1; nThisRow > 0; )
            {
                --nThisRow;
                SCROW nOtherRow = pOtherRows[nThisRow];
                if ( ValidRow(nOtherRow) && nOtherRow+1 < nLastOtherRow )
                {
                    // Luecke -> geloescht
                    ScRange aDelRange( 0, nOtherRow+1, nOtherTab,
                                        MAXCOL, nLastOtherRow-1, nOtherTab );
                    pChangeTrack->AppendDeleteRange( aDelRange, &rOtherDoc, n1, n2 );
                }
                if ( nOtherRow > MAXROW )                       // eingefuegt
                {
                    //  zusammenfassen
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
            if ( nLastOtherRow > 0 )                            // ganz oben geloescht
            {
                ScRange aDelRange( 0, 0, nOtherTab,
                                    MAXCOL, nLastOtherRow-1, nOtherTab );
                pChangeTrack->AppendDeleteRange( aDelRange, &rOtherDoc, n1, n2 );
            }

            //  Zeilen durchgehen um einzelne Zellen zu finden

            for (nThisRow = 0; nThisRow <= nThisEndRow; nThisRow++)
            {
                SCROW nOtherRow = pOtherRows[nThisRow];
                for (nThisCol = 0; nThisCol <= nThisEndCol; nThisCol++)
                {
                    SCCOL nOtherCol = static_cast<SCCOL>(pOtherCols[nThisCol]);
                    ScAddress aThisPos( nThisCol, nThisRow, nThisTab );
                    const ScBaseCell* pThisCell = GetCell( aThisPos );
                    const ScBaseCell* pOtherCell = NULL;
                    if ( ValidCol(nOtherCol) && ValidRow(nOtherRow) )
                    {
                        ScAddress aOtherPos( nOtherCol, nOtherRow, nOtherTab );
                        pOtherCell = rOtherDoc.GetCell( aOtherPos );
                    }
                    if ( !ScBaseCell::CellEqual( pThisCell, pOtherCell ) )
                    {
                        ScRange aRange( aThisPos );
                        ScChangeActionContent* pAction = new ScChangeActionContent( aRange );
                        pAction->SetOldValue( pOtherCell, &rOtherDoc, this );
                        pAction->SetNewValue( pThisCell, this );
                        pChangeTrack->Append( pAction );
                    }
                }
                aProgress.SetStateOnPercent(nProgressStart+nThisRow);
            }

            delete[] pOtherCols;
            delete[] pOtherRows;
            delete[] pTempRows;
        }
    }

    //! Inhalt von eingefuegten / geloeschten Tabellen ???
    //! Aktionen fuer eingefuegte / geloeschte Tabellen ???

    delete[] pOtherTabs;
}





/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
