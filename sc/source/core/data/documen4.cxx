/*************************************************************************
 *
 *  $RCSfile: documen4.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: er $ $Date: 2001-02-28 14:31:18 $
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

#include <svtools/intitem.hxx>
#include <svtools/zforlist.hxx>
#include <vcl/sound.hxx>

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

// -----------------------------------------------------------------------

// Nach der Regula Falsi Methode
BOOL ScDocument::Solver(USHORT nFCol, USHORT nFRow, USHORT nFTab,
                        USHORT nVCol, USHORT nVRow, USHORT nVTab,
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
        // CELLTYPE_NOTE: kein Value aber von Formel referiert
        if (eFType == CELLTYPE_FORMULA && (eVType == CELLTYPE_VALUE
                || eVType == CELLTYPE_NOTE) )
        {
            SingleRefData aRefData;
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
            aArr.AddString( sValStr.GetBuffer() );
            aArr.AddOpCode( ocClose );
            aArr.AddOpCode( ocStop );

            ScFormulaCell* pCell = new ScFormulaCell( this, ScAddress(), &aArr );

            if (pCell)
            {
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

void ScDocument::InsertMatrixFormula(USHORT nCol1, USHORT nRow1,
                                     USHORT nCol2, USHORT nRow2,
                                     const ScMarkData& rMark,
                                     const String& rFormula,
                                     const ScTokenArray* pArr )
{
    PutInOrder(nCol1, nCol2);
    PutInOrder(nRow1, nRow2);
    USHORT i, j, k, nTab1;
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
        DBG_ERROR("ScDocument::InsertMatrixFormula Keine Tabelle markiert");
        return;
    }

    ScFormulaCell* pCell;
    ScAddress aPos( nCol1, nRow1, nTab1 );
    if (pArr)
        pCell = new ScFormulaCell( this, aPos, pArr, MM_FORMULA );
    else
        pCell = new ScFormulaCell( this, aPos, rFormula, MM_FORMULA );
    pCell->SetMatColsRows( nCol2 - nCol1 + 1, nRow2 - nRow1 + 1 );
    for (i = 0; i <= MAXTAB; i++)
    {
        if (pTab[i] && rMark.GetTableSelect(i))
        {
            if (i == nTab1)
                pTab[i]->PutCell(nCol1, nRow1, pCell);
            else
                pTab[i]->PutCell(nCol1, nRow1, pCell->Clone(this, ScAddress( nCol1, nRow1, i)));
        }
    }

    SingleRefData aRefData;
    aRefData.InitFlags();
    aRefData.nCol = nCol1;
    aRefData.nRow = nRow1;
    aRefData.nTab = nTab1;
    aRefData.SetColRel( TRUE );
    aRefData.SetRowRel( TRUE );
    aRefData.SetTabRel( TRUE );
    aRefData.CalcRelFromAbs( ScAddress( nCol1, nRow1, nTab1 ) );

    ScTokenArray aArr;
    ScToken* t = aArr.AddSingleReference(aRefData);
    t->NewOpCode( ocMatRef );

    for (i = 0; i <= MAXTAB; i++)
    {
        if (pTab[i] && rMark.GetTableSelect(i))
        {
            pTab[i]->DoColResize( nCol1, nCol2, nRow2 - nRow1 + 1 );
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
                        pCell = new ScFormulaCell( this, aPos, aArr.Clone(), MM_REFERENCE );
                        pTab[i]->PutCell(j, k, (ScBaseCell*) pCell);
                    }
                }
            }
        }
    }
}

void ScDocument::InsertTableOp(const ScTabOpParam& rParam,      // Mehrfachoperation
                               USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                               const ScMarkData& rMark)
{
    PutInOrder(nCol1, nCol2);
    PutInOrder(nRow1, nRow2);
    USHORT i, j, k, nTab1;
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
        DBG_ERROR("ScDocument::InsertTableOp: Keine Tabelle markiert");
        return;
    }
    String aForString = '=';
    if (ScCompiler::pSymbolTableNative)
        aForString += ScCompiler::pSymbolTableNative[SC_OPCODE_TABLE_OP];
    aForString += '(';
    if (rParam.nMode == 0)                          // nur Spalte
    {
        for (i = nCol1+1; i <= nCol2; i++)
        {
            k = i - nCol1 - 1;
            if (rParam.aRefFormulaCell.GetCol()+k <= rParam.aRefFormulaEnd.GetCol())
            {
                ScRefTripel aTrip;
                aTrip.Put( rParam.aRefFormulaCell.GetCol()+k, rParam.aRefFormulaCell.GetRow(),
                    rParam.aRefFormulaCell.GetTab(), TRUE, FALSE, FALSE );
                String aFString = aForString;
                aFString += aTrip.GetRefString(this, nTab1);
                aFString += ';';
                aFString += rParam.aRefColCell.GetRefString(this, nTab1);
                aFString += ';';
                for (j = nRow1; j <= nRow2; j++)
                {
                    aTrip.Put( nCol1, j, nTab1, FALSE, TRUE, TRUE );
                    String aFormula = aFString;
                    aFormula += aTrip.GetRefString(this, nTab1);
                    aFormula += ')';
                    ScFormulaCell* pCell = new ScFormulaCell(this, ScAddress( i, j, nTab1 ),
                                                 aFormula, 0l);
                    for (k = 0; k <= MAXTAB; k++)
                    {
                        if (pTab[k] && rMark.GetTableSelect(k))
                        {
                            if (k == nTab1)
                                pTab[k]->PutCell(i, j, pCell);
                            else
                                pTab[k]->PutCell(i, j, pCell->Clone(this, ScAddress( i, j, k)));
                        }
                    }
                }
            }
        }
    }
    else if (rParam.nMode == 1)                 // nur zeilenweise
    {
        for (j = nRow1+1; j <= nRow2; j++)
        {
            k = j - nRow1 - 1;
            if (rParam.aRefFormulaCell.GetRow()+k <= rParam.aRefFormulaEnd.GetRow())
            {
                ScRefTripel aTrip;
                aTrip.Put( rParam.aRefFormulaCell.GetCol(), rParam.aRefFormulaCell.GetRow()+k,
                    rParam.aRefFormulaCell.GetTab(), FALSE, TRUE, FALSE );
                String aFString = aForString;
                aFString += aTrip.GetRefString(this, nTab1);
                aFString += ';';
                aFString += rParam.aRefRowCell.GetRefString(this, nTab1);
                aFString += ';';
                for (i = nCol1; i <= nCol2; i++)
                {
                    aTrip.Put( i, nRow1, nTab1, TRUE, FALSE, TRUE );
                    String aFormula = aFString;
                    aFormula += aTrip.GetRefString(this, nTab1);
                    aFormula += ')';
                    ScFormulaCell* pCell = new ScFormulaCell(this, ScAddress( i, j, nTab1 ),
                                                 aFormula, 0l);
                    for (k = 0; k <= MAXTAB; k++)
                    {
                        if (pTab[k] && rMark.GetTableSelect(k))
                        {
                            if (k == nTab1)
                                pTab[k]->PutCell(i, j, pCell);
                            else
                                pTab[k]->PutCell(i, j, pCell->Clone(this, ScAddress(i, j, k)));
                        }
                    }
                }
            }
        }
    }
    else                    // beides
    {
        for (i = nCol1+1; i <= nCol2; i++)
        {
            ScRefTripel aTrip;
            String aFString = aForString;
            aFString += rParam.aRefFormulaCell.GetRefString(this, nTab1);
            aFString += ';';
            aFString += rParam.aRefColCell.GetRefString(this, nTab1);
            aFString += ';';
            for (j = nRow1+1; j <= nRow2; j++)
            {
                aTrip.Put( nCol1, j, nTab1, FALSE, TRUE, TRUE );
                String aFormula = aFString;
                aFormula += aTrip.GetRefString(this, nTab1);
                aFormula += ';';
                aFormula += rParam.aRefRowCell.GetRefString(this, nTab1);
                aFormula += ';';
                aTrip.Put( i, nRow1, nTab1, TRUE, FALSE, TRUE );
                aFormula += aTrip.GetRefString(this, nTab1);
                aFormula += ')';
                ScFormulaCell* pCell = new ScFormulaCell(this, ScAddress( i, j, nTab1 ),
                                             aFormula );
                for (k = 0; k <= MAXTAB; k++)
                {
                    if (pTab[k] && rMark.GetTableSelect(k))
                    {
                        if (k == nTab1)
                            pTab[k]->PutCell(i, j, pCell);
                        else
                            pTab[k]->PutCell(i, j, pCell->Clone(this, ScAddress( i, j, k)));
                    }
                }
            }
        }
    }
}

USHORT ScDocument::GetErrorData( USHORT nCol, USHORT nRow, USHORT nTab ) const
{
    if (nTab<=MAXTAB && pTab[nTab])
        return pTab[nTab]->GetErrorData( nCol, nRow );
    else
        return 0;
}

BOOL ScDocument::GetNextSpellingCell(USHORT& nCol, USHORT& nRow, USHORT nTab,
                        BOOL bInSel, const ScMarkData& rMark) const
{
    if (nTab<=MAXTAB && pTab[nTab])
        return pTab[nTab]->GetNextSpellingCell( nCol, nRow, bInSel, rMark );
    else
        return FALSE;
}

BOOL ScDocument::GetNextMarkedCell( USHORT& rCol, USHORT& rRow, USHORT nTab,
                                        const ScMarkData& rMark )
{
    if (nTab<=MAXTAB && pTab[nTab])
        return pTab[nTab]->GetNextMarkedCell( rCol, rRow, rMark );
    else
        return FALSE;
}

BOOL ScDocument::ReplaceStyle(const SvxSearchItem& rSearchItem,
                              USHORT nCol, USHORT nRow, USHORT nTab,
                              ScMarkData& rMark,
                              BOOL bIsUndo)
{
    if (pTab[nTab])
        return pTab[nTab]->ReplaceStyle(rSearchItem, nCol, nRow, rMark, bIsUndo);
    else
        return FALSE;
}

void ScDocument::CompileDBFormula()
{
    for (USHORT i=0; i<=MAXTAB; i++)
    {
        if (pTab[i]) pTab[i]->CompileDBFormula();
    }
}

void ScDocument::CompileDBFormula( BOOL bCreateFormulaString )
{
    for (USHORT i=0; i<=MAXTAB; i++)
    {
        if (pTab[i]) pTab[i]->CompileDBFormula( bCreateFormulaString );
    }
}

void ScDocument::CompileNameFormula( BOOL bCreateFormulaString )
{
    if ( pCondFormList )
        pCondFormList->CompileAll();    // nach ScNameDlg noetig

    for (USHORT i=0; i<=MAXTAB; i++)
    {
        if (pTab[i]) pTab[i]->CompileNameFormula( bCreateFormulaString );
    }
}

void ScDocument::CompileColRowNameFormula()
{
    for (USHORT i=0; i<=MAXTAB; i++)
    {
        if (pTab[i]) pTab[i]->CompileColRowNameFormula();
    }
}

void ScDocument::DoColResize( USHORT nTab, USHORT nCol1, USHORT nCol2, USHORT nAdd )
{
    if (nTab<=MAXTAB && pTab[nTab])
        pTab[nTab]->DoColResize( nCol1, nCol2, nAdd );
    else
        DBG_ERROR("DoColResize: falsche Tabelle");
}

void ScDocument::InvalidateTableArea()
{
    for (USHORT nTab=0; nTab<=MAXTAB && pTab[nTab]; nTab++)
    {
        pTab[nTab]->InvalidateTableArea();
        if ( pTab[nTab]->IsScenario() )
            pTab[nTab]->InvalidateScenarioRanges();
    }
}

xub_StrLen ScDocument::GetMaxStringLen( USHORT nTab, USHORT nCol,
                                    USHORT nRowStart, USHORT nRowEnd ) const
{
    if (nTab<=MAXTAB && pTab[nTab])
        return pTab[nTab]->GetMaxStringLen( nCol, nRowStart, nRowEnd );
    else
        return 0;
}

xub_StrLen ScDocument::GetMaxNumberStringLen( USHORT& nPrecision, USHORT nTab,
                                    USHORT nCol,
                                    USHORT nRowStart, USHORT nRowEnd ) const
{
    if (nTab<=MAXTAB && pTab[nTab])
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

    USHORT nStartCol = aSingle.aStart.Col();
    USHORT nStartRow = aSingle.aStart.Row();
    USHORT nEndCol = aSingle.aEnd.Col();
    USHORT nEndRow = aSingle.aEnd.Row();

    for (USHORT nTab=0; nTab<=MAXTAB && !aData.bError; nTab++)
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
        if ( nFormat )
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
                        nPrecision -= (short)floor( log10( fVal ) );
                    else if ( fVal < 0.0 )
                        nPrecision -= (short)floor( log10( -fVal ) );
                    break;
                }
            }
        }
        else
            nPrecision = (short)GetDocOptions().GetStdPrecision();
        double fRound = SolarMath::Round( fVal, nPrecision );
        if ( SolarMath::ApproxEqual( fVal, fRound ) )
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
                        USHORT nCol, USHORT nRow, USHORT nTab, USHORT nWhich ) const
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
    DBG_ERROR("kein Pattern");
    return NULL;
}

const SfxItemSet* ScDocument::GetCondResult( USHORT nCol, USHORT nRow, USHORT nTab ) const
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
                            USHORT nCol, USHORT nRow, USHORT nTab ) const
{
    ULONG nIndex = ((const SfxUInt32Item*)GetAttr(nCol,nRow,nTab,ATTR_CONDITIONAL))->GetValue();
    if (nIndex)
    {
        if (pCondFormList)
            return pCondFormList->GetFormat( nIndex );
        else
            DBG_ERROR("pCondFormList ist 0");
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
    for (USHORT i=0; i<=MAXTAB && pTab[i]; i++)
        pTab[i]->FindConditionalFormat( nKey, rRanges );
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

void ScDocument::SetConditionalUsed( ULONG nKey )   // aus dem Speichern der Tabellen
{
    if ( nKey && pCondFormList )        // nKey==0 -> noop
    {
        ScConditionalFormat* pForm = pCondFormList->GetFormat( nKey );
        if (pForm)
            pForm->SetUsed(TRUE);
    }
}

void ScDocument::SetValidationUsed( ULONG nKey )    // aus dem Speichern der Tabellen
{
    if ( nKey && pValidationList )      // nKey==0 -> noop
    {
        ScValidationData* pData = pValidationList->GetData( nKey );
        if (pData)
            pData->SetUsed(TRUE);
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


USHORT ScDocument::RowDifferences( USHORT nThisRow, USHORT nThisTab,
                                    ScDocument& rOtherDoc, USHORT nOtherRow, USHORT nOtherTab,
                                    USHORT nMaxCol, USHORT* pOtherCols )
{
    USHORT nDif = 0;
    USHORT nUsed = 0;
    for (USHORT nThisCol=0; nThisCol<=nMaxCol; nThisCol++)
    {
        USHORT nOtherCol;
        if ( pOtherCols )
            nOtherCol = pOtherCols[nThisCol];
        else
            nOtherCol = nThisCol;

        if (nOtherCol <= MAXCOL)    // nur Spalten vergleichen, die in beiden Dateien sind
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

    if (nUsed)
        return (nDif*64)/nUsed;         // max.256 (SC_DOCCOMP_MAXDIFF)

    DBG_ASSERT(!nDif,"Diff ohne Used");
    return 0;
}

USHORT ScDocument::ColDifferences( USHORT nThisCol, USHORT nThisTab,
                                    ScDocument& rOtherDoc, USHORT nOtherCol, USHORT nOtherTab,
                                    USHORT nMaxRow, USHORT* pOtherRows )
{
    //! optimieren mit Iterator oder so

    USHORT nDif = 0;
    USHORT nUsed = 0;
    for (USHORT nThisRow=0; nThisRow<=nMaxRow; nThisRow++)
    {
        USHORT nOtherRow;
        if ( pOtherRows )
            nOtherRow = pOtherRows[nThisRow];
        else
            nOtherRow = nThisRow;

        if (nOtherRow <= MAXROW)    // nur Zeilen vergleichen, die in beiden Dateien sind
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

    if (nUsed)
        return (nDif*64)/nUsed;         // max.256

    DBG_ASSERT(!nDif,"Diff ohne Used");
    return 0;
}

void ScDocument::FindOrder( USHORT* pOtherRows, USHORT nThisEndRow, USHORT nOtherEndRow,
                            BOOL bColumns, ScDocument& rOtherDoc, USHORT nThisTab, USHORT nOtherTab,
                            USHORT nEndCol, USHORT* pTranslate, ScProgress* pProgress, ULONG nProAdd )
{
    //  bColumns=TRUE: Zeilen sind Spalten und umgekehrt

    USHORT nMaxCont;                        // wieviel weiter
    USHORT nMinGood;                        // was ist ein Treffer (incl.)
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


    USHORT nOtherRow = 0;
    USHORT nComp;
    USHORT nThisRow;
    BOOL bTotal = FALSE;        // ueber verschiedene nThisRow beibehalten
    USHORT nUnknown = 0;
    for (nThisRow = 0; nThisRow <= nThisEndRow; nThisRow++)
    {
        USHORT nTempOther = nOtherRow;
        BOOL bFound = FALSE;
        USHORT nBest = SC_DOCCOMP_MAXDIFF;
        USHORT nMax = Min( nOtherEndRow, (USHORT)( nTempOther + nMaxCont + nUnknown ) );
        for (USHORT i=nTempOther; i<=nMax && nBest; i++)    // bei 0 abbrechen
        {
            if (bColumns)
                nComp = ColDifferences( nThisRow, nThisTab, rOtherDoc, i, nOtherTab, nEndCol, pTranslate );
            else
                nComp = RowDifferences( nThisRow, nThisTab, rOtherDoc, i, nOtherTab, nEndCol, pTranslate );
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
            pOtherRows[nThisRow] = USHRT_MAX;
            ++nUnknown;
        }

        if (pProgress)
            pProgress->SetStateOnPercent(nProAdd+nThisRow);
    }

    //  Bloecke ohne Uebereinstimmung ausfuellen

    USHORT nFillStart = 0;
    USHORT nFillPos = 0;
    BOOL bInFill = FALSE;
    for (nThisRow = 0; nThisRow <= nThisEndRow+1; nThisRow++)
    {
        USHORT nThisOther = ( nThisRow <= nThisEndRow ) ? pOtherRows[nThisRow] : (nOtherEndRow+1);
        if ( nThisOther <= MAXROW )
        {
            if ( bInFill )
            {
                if ( nThisOther > nFillStart )      // ist was zu verteilen da?
                {
                    USHORT nDiff1 = nThisOther - nFillStart;
                    USHORT nDiff2 = nThisRow   - nFillPos;
                    USHORT nMinDiff = Min(nDiff1, nDiff2);
                    for (USHORT i=0; i<nMinDiff; i++)
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

    USHORT nThisCount = GetTableCount();
    USHORT nOtherCount = rOtherDoc.GetTableCount();
    USHORT* pOtherTabs = new USHORT[nThisCount];
    USHORT nThisTab;

    //  Tabellen mit gleichen Namen vergleichen
    String aThisName;
    String aOtherName;
    for (nThisTab=0; nThisTab<nThisCount; nThisTab++)
    {
        USHORT nOtherTab = USHRT_MAX;
        if (!IsScenario(nThisTab))  // Szenarien weglassen
        {
            GetName( nThisTab, aThisName );
            for (USHORT nTemp=0; nTemp<nOtherCount && nOtherTab>MAXTAB; nTemp++)
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
    USHORT nFillStart = 0;
    USHORT nFillPos = 0;
    BOOL bInFill = FALSE;
    for (nThisTab = 0; nThisTab <= nThisCount; nThisTab++)
    {
        USHORT nThisOther = ( nThisTab < nThisCount ) ? pOtherTabs[nThisTab] : nOtherCount;
        if ( nThisOther <= MAXTAB )
        {
            if ( bInFill )
            {
                if ( nThisOther > nFillStart )      // ist was zu verteilen da?
                {
                    USHORT nDiff1 = nThisOther - nFillStart;
                    USHORT nDiff2 = nThisTab   - nFillPos;
                    USHORT nMinDiff = Min(nDiff1, nDiff2);
                    for (USHORT i=0; i<nMinDiff; i++)
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
        USHORT nOtherTab = pOtherTabs[nThisTab];
        if ( nOtherTab <= MAXTAB )
        {
            USHORT nThisEndCol = 0;
            USHORT nThisEndRow = 0;
            USHORT nOtherEndCol = 0;
            USHORT nOtherEndRow = 0;
            GetCellArea( nThisTab, nThisEndCol, nThisEndRow );
            rOtherDoc.GetCellArea( nOtherTab, nOtherEndCol, nOtherEndRow );
            USHORT nEndCol = Max(nThisEndCol, nOtherEndCol);
            USHORT nEndRow = Max(nThisEndRow, nOtherEndRow);
            USHORT nThisCol, nThisRow;
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

            USHORT* pTempRows = new USHORT[nThisEndRow+1];
            USHORT* pOtherRows = new USHORT[nThisEndRow+1];
            USHORT* pOtherCols = new USHORT[nThisEndCol+1];

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
                if (pTempRows[nThisRow] <= MAXROW)
                    nMatch1 += SC_DOCCOMP_MAXDIFF -
                               RowDifferences( nThisRow, nThisTab, rOtherDoc, pTempRows[nThisRow],
                                                nOtherTab, nEndCol, NULL );

            ULONG nMatch2 = 0;  // pOtherRows, pOtherCols
            for (nThisRow = 0; nThisRow<=nThisEndRow; nThisRow++)
                if (pOtherRows[nThisRow] <= MAXROW)
                    nMatch2 += SC_DOCCOMP_MAXDIFF -
                               RowDifferences( nThisRow, nThisTab, rOtherDoc, pOtherRows[nThisRow],
                                                nOtherTab, nThisEndCol, pOtherCols );

            if ( nMatch1 >= nMatch2 )           // ohne Spalten ?
            {
                //  Spalten zuruecksetzen
                for (nThisCol = 0; nThisCol<=nThisEndCol; nThisCol++)
                    pOtherCols[nThisCol] = nThisCol;

                //  Zeilenarrays vertauschen (geloescht werden sowieso beide)
                USHORT* pSwap = pTempRows;
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

            USHORT nLastOtherCol = nOtherEndCol + 1;
            //  nThisEndCol ... 0
            for ( nThisCol = nThisEndCol+1; nThisCol > 0; )
            {
                --nThisCol;
                USHORT nOtherCol = pOtherCols[nThisCol];
                if ( nOtherCol <= MAXCOL && nOtherCol+1 < nLastOtherCol )
                {
                    // Luecke -> geloescht
                    ScRange aDelRange( nOtherCol+1, 0, nOtherTab,
                                        nLastOtherCol-1, MAXROW, nOtherTab );
                    pChangeTrack->AppendDeleteRange( aDelRange, &rOtherDoc, n1, n2 );
                }
                if ( nOtherCol > MAXCOL )                       // eingefuegt
                {
                    //  zusammenfassen
                    if ( nThisCol == nThisEndCol || pOtherCols[nThisCol+1] <= MAXCOL )
                    {
                        USHORT nFirstNew = nThisCol;
                        while ( nFirstNew > 0 && pOtherCols[nFirstNew-1] > MAXCOL )
                            --nFirstNew;
                        USHORT nDiff = nThisCol - nFirstNew;
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

            USHORT nLastOtherRow = nOtherEndRow + 1;
            //  nThisEndRow ... 0
            for ( nThisRow = nThisEndRow+1; nThisRow > 0; )
            {
                --nThisRow;
                USHORT nOtherRow = pOtherRows[nThisRow];
                if ( nOtherRow <= MAXROW && nOtherRow+1 < nLastOtherRow )
                {
                    // Luecke -> geloescht
                    ScRange aDelRange( 0, nOtherRow+1, nOtherTab,
                                        MAXCOL, nLastOtherRow-1, nOtherTab );
                    pChangeTrack->AppendDeleteRange( aDelRange, &rOtherDoc, n1, n2 );
                }
                if ( nOtherRow > MAXROW )                       // eingefuegt
                {
                    //  zusammenfassen
                    if ( nThisRow == nThisEndRow || pOtherRows[nThisRow+1] <= MAXROW )
                    {
                        USHORT nFirstNew = nThisRow;
                        while ( nFirstNew > 0 && pOtherRows[nFirstNew-1] > MAXROW )
                            --nFirstNew;
                        USHORT nDiff = nThisRow - nFirstNew;
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
                USHORT nOtherRow = pOtherRows[nThisRow];
                for (USHORT nThisCol = 0; nThisCol <= nThisEndCol; nThisCol++)
                {
                    USHORT nOtherCol = pOtherCols[nThisCol];
                    ScAddress aThisPos( nThisCol, nThisRow, nThisTab );
                    const ScBaseCell* pThisCell = GetCell( aThisPos );
                    const ScBaseCell* pOtherCell = NULL;
                    if ( nOtherCol <= MAXCOL && nOtherRow <= MAXROW )
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





