/*************************************************************************
 *
 *  $RCSfile: table3.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: er $ $Date: 2001-03-14 15:57:39 $
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

#include <tools/solmath.hxx>
#include <unotools/textsearch.hxx>
#include <svtools/zforlist.hxx>
#include <unotools/charclass.hxx>
#include <unotools/collatorwrapper.hxx>
#include <com/sun/star/i18n/CollatorOptions.hpp>
#include <stdlib.h>

#include "table.hxx"
#include "scitems.hxx"
#include "collect.hxx"
#include "attrib.hxx"
#include "cell.hxx"
#include "document.hxx"
#include "globstr.hrc"
#include "global.hxx"
#include "stlpool.hxx"
#include "compiler.hxx"
#include "patattr.hxx"
#include "subtotal.hxx"
#include "docoptio.hxx"
#include "markdata.hxx"
#include "rangelst.hxx"
#include "attarray.hxx"
#include "userlist.hxx"
#include "progress.hxx"

// STATIC DATA -----------------------------------------------------------

const USHORT nMaxSorts = 3;     // maximale Anzahl Sortierkriterien in aSortParam

struct ScSortInfo
{
    ScBaseCell*     pCell;
    USHORT          nOrg;
    DECL_FIXEDMEMPOOL_NEWDEL( ScSortInfo );
};
const USHORT nMemPoolSortInfo = (0x8000 - 64) / sizeof(ScSortInfo);
IMPL_FIXEDMEMPOOL_NEWDEL( ScSortInfo, nMemPoolSortInfo, nMemPoolSortInfo );

// END OF STATIC DATA -----------------------------------------------------


class ScSortInfoArray
{
private:
    ScSortInfo**    pppInfo[nMaxSorts];
    USHORT          nCount;
    USHORT          nStart;
    USHORT          nUsedSorts;

public:
                ScSortInfoArray( USHORT nSorts, USHORT nInd1, USHORT nInd2 ) :
                        nCount( nInd2 - nInd1 + 1 ), nStart( nInd1 ),
                        nUsedSorts( Min( nSorts, nMaxSorts ) )
                    {
                        for ( USHORT nSort = 0; nSort < nUsedSorts; nSort++ )
                        {
                            ScSortInfo** ppInfo = new ScSortInfo* [nCount];
                            for ( USHORT j = 0; j < nCount; j++ )
                                ppInfo[j] = new ScSortInfo;
                            pppInfo[nSort] = ppInfo;
                        }
                    }
                ~ScSortInfoArray()
                    {
                        for ( USHORT nSort = 0; nSort < nUsedSorts; nSort++ )
                        {
                            ScSortInfo** ppInfo = pppInfo[nSort];
                            for ( USHORT j = 0; j < nCount; j++ )
                                delete ppInfo[j];
                            delete [] ppInfo;
                        }
                    }
    ScSortInfo* Get( USHORT nSort, USHORT nInd )
                    { return (pppInfo[nSort])[ nInd - nStart ]; }
    void        Swap( USHORT nInd1, USHORT nInd2 )
                    {
                        USHORT n1 = nInd1 - nStart;
                        USHORT n2 = nInd2 - nStart;
                        for ( USHORT nSort = 0; nSort < nUsedSorts; nSort++ )
                        {
                            ScSortInfo** ppInfo = pppInfo[nSort];
                            ScSortInfo* pTmp = ppInfo[n1];
                            ppInfo[n1] = ppInfo[n2];
                            ppInfo[n2] = pTmp;
                        }
                    }
    USHORT      GetUsedSorts() { return nUsedSorts; }
    ScSortInfo**    GetFirstArray() { return pppInfo[0]; }
    USHORT      GetStart() { return nStart; }
    USHORT      GetCount() { return nCount; }
};

ScSortInfoArray* ScTable::CreateSortInfoArray( USHORT nInd1, USHORT nInd2 )
{
    USHORT nUsedSorts = 1;
    while ( nUsedSorts < nMaxSorts && aSortParam.bDoSort[nUsedSorts] )
        nUsedSorts++;
    ScSortInfoArray* pArray = new ScSortInfoArray( nUsedSorts, nInd1, nInd2 );
    if ( aSortParam.bByRow )
    {
        for ( USHORT nSort = 0; nSort < nUsedSorts; nSort++ )
        {
            USHORT nCol = aSortParam.nField[nSort];
            ScColumn* pCol = &aCol[nCol];
            for ( USHORT nRow = nInd1; nRow <= nInd2; nRow++ )
            {
//2do: FillSortInfo an ScColumn und Array abklappern statt Search in GetCell
                ScSortInfo* pInfo = pArray->Get( nSort, nRow );
                pInfo->pCell = pCol->GetCell( nRow );
                pInfo->nOrg = nRow;
            }
        }
    }
    else
    {
        for ( USHORT nSort = 0; nSort < nUsedSorts; nSort++ )
        {
            USHORT nRow = aSortParam.nField[nSort];
            for ( USHORT nCol = nInd1; nCol <= nInd2; nCol++ )
            {
                ScSortInfo* pInfo = pArray->Get( nSort, nCol );
                pInfo->pCell = GetCell( nCol, nRow );
                pInfo->nOrg = nCol;
            }
        }
    }
    return pArray;
}


BOOL ScTable::IsSortCollatorGlobal() const
{
    return  pSortCollator == ScGlobal::pCollator ||
            pSortCollator == ScGlobal::pCaseCollator;
}


void ScTable::InitSortCollator( const ScSortParam& rPar )
{
    if ( rPar.aCollatorLocale.Language.getLength() )
    {
        if ( !pSortCollator || IsSortCollatorGlobal() )
            pSortCollator = new CollatorWrapper( pDocument->GetServiceManager() );
        pSortCollator->loadCollatorAlgorithm( rPar.aCollatorAlgorithm,
            rPar.aCollatorLocale, (rPar.bCaseSens ? 0 : SC_COLLATOR_IGNORES) );
    }
    else
    {   // SYSTEM
        DestroySortCollator();
        pSortCollator = (rPar.bCaseSens ? ScGlobal::pCaseCollator :
            ScGlobal::pCollator);
    }
}


void ScTable::DestroySortCollator()
{
    if ( pSortCollator )
    {
        if ( !IsSortCollatorGlobal() )
            delete pSortCollator;
        pSortCollator = NULL;
    }
}


void ScTable::SortReorder( ScSortInfoArray* pArray, ScProgress& rProgress )
{
    BOOL bByRow = aSortParam.bByRow;
    USHORT nCount = pArray->GetCount();
    ScSortInfo** ppInfo = pArray->GetFirstArray();
    Table aTable( nCount );
    USHORT nPos;
    for ( nPos = 0; nPos < nCount; nPos++ )
    {
        aTable.Insert( ppInfo[nPos]->nOrg, (void*) ppInfo[nPos] );
    }
    USHORT nDest = pArray->GetStart();
    for ( nPos = 0; nPos < nCount; nPos++, nDest++ )
    {
        USHORT nOrg = ppInfo[nPos]->nOrg;
        if ( nDest != nOrg )
        {
            if ( bByRow )
                SwapRow( nDest, nOrg );
            else
                SwapCol( nDest, nOrg );
            // neue Position des weggeswapten eintragen
            ScSortInfo* p = ppInfo[nPos];
            p->nOrg = nDest;
            p = (ScSortInfo*) aTable.Replace( nDest, (void*) p );
            p->nOrg = nOrg;
            p = (ScSortInfo*) aTable.Replace( nOrg, (void*) p );
            DBG_ASSERT( p == ppInfo[nPos], "SortReorder: nOrg MisMatch" );
        }
        rProgress.SetStateOnPercent( nPos );
    }
}

short ScTable::CompareCell( USHORT nSort,
            ScBaseCell* pCell1, USHORT nCell1Col, USHORT nCell1Row,
            ScBaseCell* pCell2, USHORT nCell2Col, USHORT nCell2Row )
{
    short nRes = 0;

    CellType eType1, eType2;
    if (pCell1)
    {
        eType1 = pCell1->GetCellType();
        if (eType1 == CELLTYPE_NOTE)
            pCell1 = NULL;
    }
    if (pCell2)
    {
        eType2 = pCell2->GetCellType();
        if (eType2 == CELLTYPE_NOTE)
            pCell2 = NULL;
    }

    if (pCell1)
    {
        if (pCell2)
        {
            BOOL bStr1 = ( eType1 != CELLTYPE_VALUE );
            if ( eType1 == CELLTYPE_FORMULA && ((ScFormulaCell*)pCell1)->IsValue() )
                bStr1 = FALSE;
            BOOL bStr2 = ( eType2 != CELLTYPE_VALUE );
            if ( eType2 == CELLTYPE_FORMULA && ((ScFormulaCell*)pCell2)->IsValue() )
                bStr2 = FALSE;

            if ( bStr1 && bStr2 )           // nur Strings untereinander als String vergleichen!
            {
                String aStr1;
                String aStr2;
                if (eType1 == CELLTYPE_STRING)
                    ((ScStringCell*)pCell1)->GetString(aStr1);
                else
                    GetString(nCell1Col, nCell1Row, aStr1);
                if (eType2 == CELLTYPE_STRING)
                    ((ScStringCell*)pCell2)->GetString(aStr2);
                else
                    GetString(nCell2Col, nCell2Row, aStr2);
                BOOL bUserDef = aSortParam.bUserDef;
                if (bUserDef)
                {
                    ScUserListData* pData =
                        (ScUserListData*)(ScGlobal::GetUserList()->At(
                        aSortParam.nUserIndex));
                    if (pData)
                    {
                        if ( aSortParam.bCaseSens )
                            nRes = pData->Compare(aStr1, aStr2);
                        else
                            nRes = pData->ICompare(aStr1, aStr2);
                    }
                    else
                        bUserDef = FALSE;

                }
                if (!bUserDef)
                    nRes = (short) pSortCollator->compareString( aStr1, aStr2 );
            }
            else if ( bStr1 )               // String <-> Zahl
                nRes = 1;                   // Zahl vorne
            else if ( bStr2 )               // Zahl <-> String
                nRes = -1;                  // Zahl vorne
            else                            // Zahlen untereinander
            {
                double nVal1;
                double nVal2;
                if (eType1 == CELLTYPE_VALUE)
                    nVal1 = ((ScValueCell*)pCell1)->GetValue();
                else if (eType1 == CELLTYPE_FORMULA)
                    nVal1 = ((ScFormulaCell*)pCell1)->GetValue();
                else
                    nVal1 = 0;
                if (eType2 == CELLTYPE_VALUE)
                    nVal2 = ((ScValueCell*)pCell2)->GetValue();
                else if (eType2 == CELLTYPE_FORMULA)
                    nVal2 = ((ScFormulaCell*)pCell2)->GetValue();
                else
                    nVal2 = 0;
                if (nVal1 < nVal2)
                    nRes = -1;
                else if (nVal1 > nVal2)
                    nRes = 1;
            }
            if ( !aSortParam.bAscending[nSort] )
                nRes = -nRes;
        }
        else
            nRes = -1;
    }
    else
    {
        if ( pCell2 )
            nRes = 1;
        else
            nRes = 0;                   // beide leer
    }
    return nRes;
}

short ScTable::Compare( ScSortInfoArray* pArray, USHORT nIndex1, USHORT nIndex2 )
{
    short nRes;
    USHORT nSort = 0;
    do
    {
        ScSortInfo* pInfo1 = pArray->Get( nSort, nIndex1 );
        ScSortInfo* pInfo2 = pArray->Get( nSort, nIndex2 );
        if ( aSortParam.bByRow )
            nRes = CompareCell( nSort,
                pInfo1->pCell, aSortParam.nField[nSort], pInfo1->nOrg,
                pInfo2->pCell, aSortParam.nField[nSort], pInfo2->nOrg );
        else
            nRes = CompareCell( nSort,
                pInfo1->pCell, pInfo1->nOrg, aSortParam.nField[nSort],
                pInfo2->pCell, pInfo2->nOrg, aSortParam.nField[nSort] );
    } while ( nRes == 0 && ++nSort < pArray->GetUsedSorts() );
    return nRes;
}

void ScTable::QuickSort( ScSortInfoArray* pArray, short nLo, short nHi )
{
    if ((nHi - nLo) == 1)
    {
        if (Compare(pArray, nLo, nHi) > 0)
            pArray->Swap( nLo, nHi );
    }
    else
    {
        short ni = nLo;
        short nj = nHi;
        do
        {
            while ((ni <= nHi) && (Compare(pArray, ni, nLo)) < 0)
                ni++;
            while ((nj >= nLo) && (Compare(pArray, nLo, nj)) < 0)
                nj--;
            if (ni <= nj)
            {
                if (ni != nj)
                    pArray->Swap( ni, nj );
                ni++;
                nj--;
            }
        } while (ni < nj);
        if ((nj - nLo) < (nHi - ni))
        {
            if (nLo < nj)
                QuickSort(pArray, nLo, nj);
            if (ni < nHi)
                QuickSort(pArray, ni, nHi);
        }
        else
        {
            if (ni < nHi)
                QuickSort(pArray, ni, nHi);
            if (nLo < nj)
                QuickSort(pArray, nLo, nj);
        }
    }
}

void ScTable::SwapCol(USHORT nCol1, USHORT nCol2)
{
    for (USHORT nRow = aSortParam.nRow1; nRow <= aSortParam.nRow2; nRow++)
    {
        aCol[nCol1].SwapCell(nRow, aCol[nCol2]);
        if (aSortParam.bIncludePattern)
        {
            const ScPatternAttr* pPat1 = GetPattern(nCol1, nRow);
            const ScPatternAttr* pPat2 = GetPattern(nCol2, nRow);
            if (pPat1 != pPat2)
            {
                SetPattern(nCol1, nRow, *pPat2, TRUE);
                SetPattern(nCol2, nRow, *pPat1, TRUE);
            }
        }
    }
}

void ScTable::SwapRow(USHORT nRow1, USHORT nRow2)
{
    for (USHORT nCol = aSortParam.nCol1; nCol <= aSortParam.nCol2; nCol++)
    {
        aCol[nCol].SwapRow(nRow1, nRow2);
        if (aSortParam.bIncludePattern)
        {
            const ScPatternAttr* pPat1 = GetPattern(nCol, nRow1);
            const ScPatternAttr* pPat2 = GetPattern(nCol, nRow2);
            if (pPat1 != pPat2)
            {
                SetPattern(nCol, nRow1, *pPat2, TRUE);
                SetPattern(nCol, nRow2, *pPat1, TRUE);
            }
        }
    }
    if (bGlobalKeepQuery && pRowFlags)
    {
        BYTE nFlags1 = pRowFlags[nRow1] & ( CR_HIDDEN | CR_FILTERED );
        BYTE nFlags2 = pRowFlags[nRow2] & ( CR_HIDDEN | CR_FILTERED );
        pRowFlags[nRow1] = (pRowFlags[nRow1] & ~( CR_HIDDEN | CR_FILTERED )) | nFlags2;
        pRowFlags[nRow2] = (pRowFlags[nRow2] & ~( CR_HIDDEN | CR_FILTERED )) | nFlags1;
    }
}

short ScTable::Compare(USHORT nIndex1, USHORT nIndex2)
{
    short nRes;
    USHORT nSort = 0;
    if (aSortParam.bByRow)
    {
        do
        {
            USHORT nCol = aSortParam.nField[nSort];
            ScBaseCell* pCell1 = aCol[nCol].GetCell( nIndex1 );
            ScBaseCell* pCell2 = aCol[nCol].GetCell( nIndex2 );
            nRes = CompareCell( nSort, pCell1, nCol, nIndex1, pCell2, nCol, nIndex2 );
        } while ( nRes == 0 && ++nSort < nMaxSorts && aSortParam.bDoSort[nSort] );
    }
    else
    {
        do
        {
            USHORT nRow = aSortParam.nField[nSort];
            ScBaseCell* pCell1 = aCol[nIndex1].GetCell( nRow );
            ScBaseCell* pCell2 = aCol[nIndex2].GetCell( nRow );
            nRes = CompareCell( nSort, pCell1, nIndex1, nRow, pCell2, nIndex2, nRow );
        } while ( nRes == 0 && ++nSort < nMaxSorts && aSortParam.bDoSort[nSort] );
    }
    return nRes;
}

BOOL ScTable::IsSorted( USHORT nStart, USHORT nEnd )                    // ueber aSortParam
{
    for (USHORT i=nStart; i<nEnd; i++)
    {
        if (Compare( i, i+1 ) > 0)
            return FALSE;
    }
    return TRUE;
}

void ScTable::DecoladeRow( ScSortInfoArray* pArray, USHORT nRow1, USHORT nRow2 )
{
    USHORT nRow;
    USHORT nMax = nRow2 - nRow1;
    for (USHORT i = nRow1; (i + 4) <= nRow2; i += 4)
    {
        nRow = rand() % nMax;
        pArray->Swap(i, nRow1 + nRow);
    }
}

void ScTable::Sort(const ScSortParam& rSortParam, BOOL bKeepQuery)
{
    aSortParam = rSortParam;
    InitSortCollator( rSortParam );
    bGlobalKeepQuery = bKeepQuery;
    if (rSortParam.bByRow)
    {
        USHORT nLastRow = 0;
        for (USHORT nCol = aSortParam.nCol1; nCol <= aSortParam.nCol2; nCol++)
            nLastRow = Max(nLastRow, aCol[nCol].GetLastDataPos());
        nLastRow = Min(nLastRow, aSortParam.nRow2);
        USHORT nRow1 = (rSortParam.bHasHeader ?
            aSortParam.nRow1 + 1 : aSortParam.nRow1);
        if (!IsSorted(nRow1, nLastRow))
        {
            ScProgress aProgress( pDocument->GetDocumentShell(),
                                    ScGlobal::GetRscString(STR_PROGRESS_SORTING), nLastRow - nRow1 );
            ScSortInfoArray* pArray = CreateSortInfoArray( nRow1, nLastRow );
            if ( nLastRow - nRow1 > 255 )
                DecoladeRow( pArray, nRow1, nLastRow );
            QuickSort( pArray, nRow1, nLastRow );
            SortReorder( pArray, aProgress );
            delete pArray;
        }
    }
    else
    {
        USHORT nLastCol;
        for (nLastCol = aSortParam.nCol2;
             (nLastCol > aSortParam.nCol1) && aCol[nLastCol].IsEmptyBlock(aSortParam.nRow1, aSortParam.nRow2); nLastCol--)
        {
        }
        USHORT nCol1 = (rSortParam.bHasHeader ?
            aSortParam.nCol1 + 1 : aSortParam.nCol1);
        if (!IsSorted(nCol1, nLastCol))
        {
            ScProgress aProgress( pDocument->GetDocumentShell(),
                                    ScGlobal::GetRscString(STR_PROGRESS_SORTING), nLastCol - nCol1 );
            ScSortInfoArray* pArray = CreateSortInfoArray( nCol1, nLastCol );
            QuickSort( pArray, nCol1, nLastCol );
            SortReorder( pArray, aProgress );
            delete pArray;
        }
    }
    DestroySortCollator();
}


//      Testen, ob beim Loeschen von Zwischenergebnissen andere Daten mit geloescht werden
//      (fuer Hinweis-Box)

BOOL ScTable::TestRemoveSubTotals( const ScSubTotalParam& rParam )
{
    USHORT nStartCol = rParam.nCol1;
    USHORT nStartRow = rParam.nRow1 + 1;        // Header
    USHORT nEndCol   = rParam.nCol2;
    USHORT nEndRow   = rParam.nRow2;

    USHORT nCol;
    USHORT nRow;
    ScBaseCell* pCell;

    BOOL bWillDelete = FALSE;
    for ( nCol=nStartCol; nCol<=nEndCol && !bWillDelete; nCol++ )
    {
        ScColumnIterator aIter( &aCol[nCol],nStartRow,nEndRow );
        while ( aIter.Next( nRow, pCell ) && !bWillDelete )
        {
            if ( pCell->GetCellType() == CELLTYPE_FORMULA )
                if (((ScFormulaCell*)pCell)->IsSubTotal())
                {
                    for (USHORT nTestCol=0; nTestCol<=MAXCOL; nTestCol++)
                        if (nTestCol<nStartCol || nTestCol>nEndCol)
                            if (aCol[nTestCol].HasDataAt(nRow))
                                bWillDelete = TRUE;
                }
        }
    }
    return bWillDelete;
}

//      alte Ergebnisse loeschen
//      rParam.nRow2 wird veraendert !

void ScTable::RemoveSubTotals( ScSubTotalParam& rParam )
{
    USHORT nStartCol = rParam.nCol1;
    USHORT nStartRow = rParam.nRow1 + 1;        // Header
    USHORT nEndCol   = rParam.nCol2;
    USHORT nEndRow   = rParam.nRow2;            // wird veraendert

    USHORT nCol;
    USHORT nRow;
    ScBaseCell* pCell;

    for ( nCol=nStartCol; nCol<=nEndCol; nCol++ )
    {
        ScColumnIterator aIter( &aCol[nCol],nStartRow,nEndRow );
        while ( aIter.Next( nRow, pCell ) )
        {
            if ( pCell->GetCellType() == CELLTYPE_FORMULA )
                if (((ScFormulaCell*)pCell)->IsSubTotal())
                {
                    SetRowFlags(nRow+1,GetRowFlags(nRow+1)&(~CR_MANUALBREAK));
                    pDocument->DeleteRow( 0,nTab, MAXCOL,nTab, nRow, 1 );
                    --nEndRow;
                    aIter = ScColumnIterator( &aCol[nCol],nRow,nEndRow );
                }
        }
    }

    rParam.nRow2 = nEndRow;                 // neues Ende
}

//  harte Zahlenformate loeschen (fuer Ergebnisformeln)

void lcl_RemoveNumberFormat( ScTable* pTab, USHORT nCol, USHORT nRow )
{
    const ScPatternAttr* pPattern = pTab->GetPattern( nCol, nRow );
    if ( pPattern->GetItemSet().GetItemState( ATTR_VALUE_FORMAT, FALSE )
            == SFX_ITEM_SET )
    {
        ScPatternAttr aNewPattern( *pPattern );
        SfxItemSet& rSet = aNewPattern.GetItemSet();
        rSet.ClearItem( ATTR_VALUE_FORMAT );
        rSet.ClearItem( ATTR_LANGUAGE_FORMAT );
        pTab->SetPattern( nCol, nRow, aNewPattern, TRUE );
    }
}

//      neue Zwischenergebnisse
//      rParam.nRow2 wird veraendert !

BOOL ScTable::DoSubTotals( ScSubTotalParam& rParam )
{
    USHORT nStartCol = rParam.nCol1;
    USHORT nStartRow = rParam.nRow1 + 1;        // Header
    USHORT nEndCol   = rParam.nCol2;
    USHORT nEndRow   = rParam.nRow2;            // wird veraendert
    USHORT i;

    //  Leerzeilen am Ende weglassen,
    //  damit alle Ueberlaeufe (MAXROW) bei InsertRow gefunden werden (#35180#)
    //  Wenn sortiert wurde, sind alle Leerzeilen am Ende.
    USHORT nEmpty = GetEmptyLinesInBlock( nStartCol, nStartRow, nEndCol, nEndRow, DIR_BOTTOM );
    nEndRow -= nEmpty;

    USHORT nLevelCount = 0;             // Anzahl Gruppierungen
    BOOL bDoThis = TRUE;
    for (i=0; i<MAXSUBTOTAL && bDoThis; i++)
        if (rParam.bGroupActive[i])
            nLevelCount = i+1;
        else
            bDoThis = FALSE;

    if (nLevelCount==0)                 // nichts tun
        return TRUE;

    USHORT*         nGroupCol = rParam.nField;  // Spalten nach denen
                                                // gruppiert wird
    USHORT          nResCount;                  // pro Ebene eingestellt
    USHORT*         nResCols;                   // Ergebnis-Spalten
    ScSubTotalFunc* eResFunc;                   // Ergebnis-Funktionen

    //  #44444# Durch (leer) als eigene Kategorie muss immer auf
    //  Teilergebniszeilen aus den anderen Spalten getestet werden
    //  (frueher nur, wenn eine Spalte mehrfach vorkam)
    BOOL bTestPrevSub = ( nLevelCount > 1 );

    USHORT  nLevel;
    BOOL    bChanged;
    USHORT  nRow;
    USHORT  nSubStartRow;
    USHORT  nDestRow;
    USHORT  nFuncStart;
    USHORT  nFuncEnd;
    String  aString;
    String  aSubString;
    String  aOutString;

    BOOL bIgnoreCase = !rParam.bCaseSens;

    String *pCompString[MAXSUBTOTAL];               // Pointer wegen Compiler-Problemen
    for (i=0; i<MAXSUBTOTAL; i++)
        pCompString[i] = new String;

                                //! sortieren?

    ScStyleSheet* pStyle = (ScStyleSheet*) pDocument->GetStyleSheetPool()->Find(
                                ScGlobal::GetRscString(STR_STYLENAME_RESULT), SFX_STYLE_FAMILY_PARA );

    BOOL bSpaceLeft = TRUE;                                         // Erfolg beim Einfuegen?

    for (nLevel=0; nLevel<=nLevelCount && bSpaceLeft; nLevel++)     // incl. Gesamtergebnis
    {
        BOOL bTotal = ( nLevel == nLevelCount );
        USHORT nGroupNo = bTotal ? 0 : (nLevelCount-nLevel-1);

        nResCount   = rParam.nSubTotals[nGroupNo];
        nResCols    = rParam.pSubTotals[nGroupNo];
        eResFunc    = rParam.pFunctions[nGroupNo];

        if (nResCount)                                      // sonst nur sortieren
        {
            for (i=0; i<=nGroupNo; i++)
            {
                GetString( nGroupCol[i], nStartRow, aSubString );
                *pCompString[i] = aSubString;
                if (bIgnoreCase)
                    ScGlobal::pCharClass->toUpper( *pCompString[i] );
            }                                                   // aSubString bleibt auf dem letzten stehen

            BOOL bBlockVis = FALSE;             // Gruppe eingeblendet?
            nSubStartRow = nStartRow;
            for (nRow=nStartRow; nRow<=nEndRow+1 && bSpaceLeft; nRow++)
            {
                if (nRow>nEndRow)
                    bChanged = TRUE;
                else
                {
                    bChanged = FALSE;
                    if (!bTotal)
                    {
                        for (i=0; i<=nGroupNo && !bChanged; i++)
                        {
                            GetString( nGroupCol[i], nRow, aString );
                            if (bIgnoreCase)
                                ScGlobal::pCharClass->toUpper( aString );
                            //  #41427# wenn sortiert, ist "leer" eine eigene Gruppe
                            //  sonst sind leere Zellen unten erlaubt
                            bChanged = ( ( aString.Len() || rParam.bDoSort ) &&
                                            aString != *pCompString[i] );
                        }
                        if ( bChanged && bTestPrevSub )
                        {
                            //  kein Gruppenwechsel bei Teilergebniszeilen

                            ScBaseCell* pCell;
                            for (i=nStartCol; i<=nEndCol && bChanged; i++)
                            {
                                pCell = GetCell( i, nRow );
                                if ( pCell )
                                    if ( pCell->GetCellType() == CELLTYPE_FORMULA )
                                        if (((ScFormulaCell*)pCell)->IsSubTotal())
                                            bChanged = FALSE;
                            }
                        }
                    }
                }
                if ( bChanged )
                {
                    nDestRow   = nRow;
                    nFuncStart = nSubStartRow;
                    nFuncEnd   = nRow-1;
                    ComplRefData aRef;
                    aRef.InitFlags();
                    aRef.Ref1.nTab = nTab;
                    aRef.Ref2.nTab = nTab;

                    bSpaceLeft = pDocument->InsertRow( 0,nTab, MAXCOL,nTab, nDestRow, 1 );
                    DBShowRow( nDestRow, bBlockVis );
                    bBlockVis = FALSE;
                    if (rParam.bPagebreak && nRow < MAXROW && nSubStartRow != nStartRow
                                          && nLevel == 0)
                        SetRowFlags(nSubStartRow,GetRowFlags(nSubStartRow)|CR_MANUALBREAK);
                    if (bSpaceLeft)
                    {
                        if (bTotal)     // "Gesamtergebnis"
                            aOutString = ScGlobal::GetRscString( STR_TABLE_GESAMTERGEBNIS );
                        else
                        {               // " Ergebnis"
                            aOutString = aSubString;
                            if (!aOutString.Len())
                                aOutString = ScGlobal::GetRscString( STR_EMPTYDATA );
                            aOutString += ' ';
                            USHORT nStrId = STR_TABLE_ERGEBNIS;
                            if ( nResCount == 1 )
                                switch ( eResFunc[0] )
                                {
                                    case SUBTOTAL_FUNC_AVE:     nStrId = STR_FUN_TEXT_AVG;      break;
                                    case SUBTOTAL_FUNC_CNT:
                                    case SUBTOTAL_FUNC_CNT2:    nStrId = STR_FUN_TEXT_COUNT;    break;
                                    case SUBTOTAL_FUNC_MAX:     nStrId = STR_FUN_TEXT_MAX;      break;
                                    case SUBTOTAL_FUNC_MIN:     nStrId = STR_FUN_TEXT_MIN;      break;
                                    case SUBTOTAL_FUNC_PROD:    nStrId = STR_FUN_TEXT_PRODUCT;  break;
                                    case SUBTOTAL_FUNC_STD:
                                    case SUBTOTAL_FUNC_STDP:    nStrId = STR_FUN_TEXT_STDDEV;   break;
                                    case SUBTOTAL_FUNC_SUM:     nStrId = STR_FUN_TEXT_SUM;      break;
                                    case SUBTOTAL_FUNC_VAR:
                                    case SUBTOTAL_FUNC_VARP:    nStrId = STR_FUN_TEXT_VAR;      break;
                                }
                            aOutString += ScGlobal::GetRscString( nStrId );
                        }
                        SetString( nGroupCol[nGroupNo], nDestRow, nTab, aOutString );
                        ApplyStyle( nGroupCol[nGroupNo], nDestRow, *pStyle );

                        for (USHORT i=0; i<nResCount; i++)
                        {
                            aRef.Ref1.nCol = nResCols[i];
                            aRef.Ref1.nRow = nFuncStart;
                            aRef.Ref2.nCol = nResCols[i];
                            aRef.Ref2.nRow = nFuncEnd;

                            ScTokenArray aArr;
                            aArr.AddOpCode(ocSubTotal);
                            aArr.AddOpCode(ocOpen);
                            aArr.AddDouble((double) eResFunc[i]);
                            aArr.AddOpCode(ocSep);
                            aArr.AddDoubleReference(aRef);
                            aArr.AddOpCode(ocClose);
                            aArr.AddOpCode(ocStop);
                            ScBaseCell* pCell = new ScFormulaCell
                            ( pDocument, ScAddress( nResCols[i], nDestRow, nTab ), &aArr );
                            PutCell( nResCols[i], nDestRow, pCell );

                            if ( nResCols[i] != nGroupCol[nGroupNo] )
                            {
                                ApplyStyle( nResCols[i], nDestRow, *pStyle );

                                //  Zahlformat loeschen
                                lcl_RemoveNumberFormat( this, nResCols[i], nDestRow );
                            }
                        }

/*                      if (rParam.bPagebreak && nRow < MAXROW)
                        {
                            BYTE nFlags = GetRowFlags( nRow+1 );
                            nFlags |= CR_MANUALBREAK;
                            SetRowFlags( nRow+1, nFlags );
                        }
*/
                        ++nRow;
                        ++nEndRow;
                        nSubStartRow = nRow;
                        for (i=0; i<=nGroupNo; i++)
                        {
                            GetString( nGroupCol[i], nRow, aSubString );
                            *pCompString[i] = aSubString;
                            if (bIgnoreCase)
                                ScGlobal::pCharClass->toUpper( *pCompString[i] );
                        }
                    }
                }
                if (!pRowFlags)
                    bBlockVis = TRUE;
                else
                    if ( (pRowFlags[nRow] & CR_FILTERED) == 0 )
                        bBlockVis = TRUE;
            }
        }
        else
        {
//          DBG_ERROR( "nSubTotals==0 bei DoSubTotals" );
        }
    }

    //!     je nach Einstellung Zwischensummen-Zeilen nach oben verschieben ?

    //!     Outlines direkt erzeugen?

    if (bSpaceLeft)
        DoAutoOutline( nStartCol, nStartRow, nEndCol, nEndRow );

    for (i=0; i<MAXSUBTOTAL; i++)
        delete pCompString[i];

    rParam.nRow2 = nEndRow;                 // neues Ende
    return bSpaceLeft;
}


BOOL ScTable::ValidQuery(USHORT nRow, const ScQueryParam& rParam, BOOL* pSpecial)
{
    if (!rParam.GetEntry(0).bDoQuery)
        return TRUE;

    //---------------------------------------------------------------

    const USHORT nFixedBools = 32;
    BOOL aBool[nFixedBools];
    USHORT nEntryCount = rParam.GetEntryCount();
    BOOL* pPasst = ( nEntryCount <= nFixedBools ? &aBool[0] : new BOOL[nEntryCount] );

    short   nPos = -1;
    USHORT  i    = 0;
    BOOL    bMatchWholeCell = pDocument->GetDocOptions().IsMatchWholeCell();
    CollatorWrapper* pCollator = (rParam.bCaseSens ? ScGlobal::pCaseCollator :
        ScGlobal::pCollator);

    while ( (i < nEntryCount) && rParam.GetEntry(i).bDoQuery )
    {
        ScQueryEntry& rEntry = rParam.GetEntry(i);

        BOOL bOk = FALSE;

        if ( pSpecial && pSpecial[i] )
        {
            if (rEntry.nVal == SC_EMPTYFIELDS)
                bOk = !( aCol[rEntry.nField].HasDataAt( nRow ) );
            else // if (rEntry.nVal == SC_NONEMPTYFIELDS)
                bOk = aCol[rEntry.nField].HasDataAt( nRow );
        }
        else if (!rEntry.bQueryByString && HasValueData(rEntry.nField, nRow))
        {   // by Value
            double  nCellVal = GetValue(rEntry.nField, nRow);
            switch (rEntry.eOp)
            {
                case SC_EQUAL :
                    bOk = SolarMath::ApproxEqual( nCellVal, rEntry.nVal );
                    break;
                case SC_LESS :
                    bOk = (nCellVal < rEntry.nVal) && !SolarMath::ApproxEqual( nCellVal, rEntry.nVal );
                    break;
                case SC_GREATER :
                    bOk = (nCellVal > rEntry.nVal) && !SolarMath::ApproxEqual( nCellVal, rEntry.nVal );
                    break;
                case SC_LESS_EQUAL :
                    bOk = (nCellVal < rEntry.nVal) || SolarMath::ApproxEqual( nCellVal, rEntry.nVal );
                    break;
                case SC_GREATER_EQUAL :
                    bOk = (nCellVal > rEntry.nVal) || SolarMath::ApproxEqual( nCellVal, rEntry.nVal );
                    break;
                case SC_NOT_EQUAL :
                    bOk = !SolarMath::ApproxEqual( nCellVal, rEntry.nVal );
                    break;
            }
        }
        else if ( (rEntry.eOp == SC_EQUAL || rEntry.eOp == SC_NOT_EQUAL)
                || (rEntry.bQueryByString
                    && HasStringData(rEntry.nField, nRow))
            )
        {   // by String
            String  aCellStr;
            GetInputString(rEntry.nField, nRow, aCellStr);

            if (rParam.bRegExp && ((rEntry.eOp == SC_EQUAL)
                                || (rEntry.eOp == SC_NOT_EQUAL)) )
            {
                xub_StrLen nStart = 0;
                xub_StrLen nEnd   = aCellStr.Len();
                bOk = (BOOL) rEntry.GetSearchTextPtr( rParam.bCaseSens )
                    ->SearchFrwrd( aCellStr, &nStart, &nEnd );
                // from 614 on, nEnd is behind the found text
                if ( bOk && bMatchWholeCell
                        && (nStart != 0 || nEnd != aCellStr.Len()) )
                    bOk = FALSE;    // RegExp muss ganze Zelle matchen
                if ( rEntry.eOp == SC_NOT_EQUAL )
                    bOk = !bOk;
            }
            else
            {
                if ( rEntry.eOp == SC_EQUAL || rEntry.eOp == SC_NOT_EQUAL )
                {
                    if ( bMatchWholeCell )
                    {
                        bOk = (pCollator->compareString( aCellStr,
                            *rEntry.pStr ) == COMPARE_EQUAL);
                    }
                    else
                    {
                        if ( rParam.bCaseSens )
                            bOk = (aCellStr.Search( *rEntry.pStr ) != STRING_NOTFOUND);
                        else
                        {
                            String aQueryStr( *rEntry.pStr );
                            ScGlobal::pCharClass->toLower( aQueryStr );
                            ScGlobal::pCharClass->toLower( aCellStr );
                            bOk = (aCellStr.Search( aQueryStr ) != STRING_NOTFOUND);
                        }
                    }
                    if ( rEntry.eOp == SC_NOT_EQUAL )
                        bOk = !bOk;
                }
                else
                {
                    sal_Int32 nCompare = pCollator->compareString(
                        aCellStr, *rEntry.pStr );
                    switch (rEntry.eOp)
                    {
                        case SC_LESS :
                            bOk = (nCompare == COMPARE_LESS);
                            break;
                        case SC_GREATER :
                            bOk = (nCompare == COMPARE_GREATER);
                            break;
                        case SC_LESS_EQUAL :
                            bOk = (nCompare == COMPARE_LESS) || (nCompare == COMPARE_EQUAL);
                            break;
                        case SC_GREATER_EQUAL :
                            bOk = (nCompare == COMPARE_GREATER) || (nCompare == COMPARE_EQUAL);
                            break;
                    }
                }
            }
        }

        if (nPos == -1)
        {
            nPos++;
            pPasst[nPos] = bOk;
        }
        else
        {
            if (rEntry.eConnect == SC_AND)
                pPasst[nPos] = pPasst[nPos] && bOk;
            else
            {
                nPos++;
                pPasst[nPos] = bOk;
            }
        }
        i++;
    }

    for ( i=1; (short)i <= nPos; i++ )
        pPasst[0] = pPasst[0] || pPasst[i];

    BOOL bRet = pPasst[0];
    if ( pPasst != &aBool[0] )
        delete [] pPasst;

    return bRet;
}

void ScTable::TopTenQuery( ScQueryParam& rParam )
{
    BOOL bSortCollatorInitialized = FALSE;
    USHORT nEntryCount = rParam.GetEntryCount();
    USHORT nRow1 = (rParam.bHasHeader ? rParam.nRow1 + 1 : rParam.nRow1);
    USHORT nCount = rParam.nRow2 - nRow1 + 1;
    USHORT i;
    for ( i=0; (i<nEntryCount) && (rParam.GetEntry(i).bDoQuery); i++ )
    {
        ScQueryEntry& rEntry = rParam.GetEntry(i);
        switch ( rEntry.eOp )
        {
            case SC_TOPVAL:
            case SC_BOTVAL:
            case SC_TOPPERC:
            case SC_BOTPERC:
            {
                ScSortParam aLocalSortParam( rParam, rEntry.nField );
                if ( !bSortCollatorInitialized )
                {
                    bSortCollatorInitialized = TRUE;
                    InitSortCollator( aLocalSortParam );
                }
                ScSortInfoArray* pArray = CreateSortInfoArray( nRow1, rParam.nRow2 );
                DecoladeRow( pArray, nRow1, rParam.nRow2 );
                QuickSort( pArray, nRow1, rParam.nRow2 );
                ScSortInfo** ppInfo = pArray->GetFirstArray();
                USHORT nValidCount = nCount;
                // keine Note-/Leerzellen zaehlen, sind ans Ende sortiert
                while ( nValidCount && ppInfo[nValidCount-1]->pCell == NULL )
                    nValidCount--;
                // keine Strings zaehlen, sind zwischen Value und Leer
                while ( nValidCount
                  && ppInfo[nValidCount-1]->pCell->HasStringData() )
                    nValidCount--;
                if ( nValidCount )
                {
                    if ( rEntry.bQueryByString )
                    {   // dat wird nix
                        rEntry.bQueryByString = FALSE;
                        rEntry.nVal = 10;   // 10 bzw. 10%
                    }
                    USHORT nVal = (rEntry.nVal >= 1 ? (USHORT)rEntry.nVal : 1);
                    USHORT nOffset = 0;
                    switch ( rEntry.eOp )
                    {
                        case SC_TOPVAL:
                        {
                            rEntry.eOp = SC_GREATER_EQUAL;
                            if ( nVal > nValidCount )
                                nVal = nValidCount;
                            nOffset = nValidCount - nVal;   // 1 <= nVal <= nValidCount
                        }
                        break;
                        case SC_BOTVAL:
                        {
                            rEntry.eOp = SC_LESS_EQUAL;
                            if ( nVal > nValidCount )
                                nVal = nValidCount;
                            nOffset = nVal - 1;     // 1 <= nVal <= nValidCount
                        }
                        break;
                        case SC_TOPPERC:
                        {
                            rEntry.eOp = SC_GREATER_EQUAL;
                            if ( nVal > 100 )
                                nVal = 100;
                            nOffset = nValidCount -
                                (USHORT)((ULONG)nValidCount * nVal / 100);
                            if ( nOffset >= nValidCount )
                                nOffset = nValidCount - 1;
                        }
                        break;
                        case SC_BOTPERC:
                        {
                            rEntry.eOp = SC_LESS_EQUAL;
                            if ( nVal > 100 )
                                nVal = 100;
                            nOffset =
                                (USHORT)((ULONG)nValidCount * nVal / 100);
                            if ( nOffset >= nValidCount )
                                nOffset = nValidCount - 1;
                        }
                        break;
                    }
                    ScBaseCell* pCell = ppInfo[nOffset]->pCell;
                    if ( pCell->HasValueData() )
                    {
                        if ( pCell->GetCellType() == CELLTYPE_VALUE )
                            rEntry.nVal = ((ScValueCell*)pCell)->GetValue();
                        else
                            rEntry.nVal = ((ScFormulaCell*)pCell)->GetValue();
                    }
                    else
                    {
                        DBG_ERRORFILE( "TopTenQuery: pCell kein ValueData" );
                        rEntry.eOp = SC_GREATER_EQUAL;
                        rEntry.nVal = 0;
                    }
                }
                else
                {
                    rEntry.eOp = SC_GREATER_EQUAL;
                    rEntry.bQueryByString = FALSE;
                    rEntry.nVal = 0;
                }
                delete pArray;
            }
        }
    }
    if ( bSortCollatorInitialized )
        DestroySortCollator();
}

USHORT ScTable::Query(ScQueryParam& rParamOrg, BOOL bKeepSub)
{
    StrCollection   aStrCollection;
    StrData*        pStrData = NULL;

    BOOL    bStarted = FALSE;
    BOOL    bOldResult = TRUE;
    USHORT  nOldStart = 0;
    USHORT  nOldEnd = 0;

    USHORT nCount   = 0;
    USHORT nOutRow  = 0;
    USHORT nHeader  = rParamOrg.bHasHeader ? 1 : 0;
    USHORT i        = 0;

    USHORT nEntryCount = rParamOrg.GetEntryCount();

    BOOL* pSpecial = new BOOL[nEntryCount];
    for (i=0; i<nEntryCount; i++)
        pSpecial[i] = FALSE;

    /*
     * Dialog liefert die ausgezeichneten Feldwerte "leer"/"nicht leer"
     * als Konstanten in nVal in Verbindung mit dem Schalter
     * bQueryByString auf FALSE.
     */

    BOOL bTopTen = FALSE;
    for ( i=0; (i<nEntryCount) && (rParamOrg.GetEntry(i).bDoQuery); i++ )
    {
        ScQueryEntry& rEntry = rParamOrg.GetEntry(i);

        if ( rEntry.bQueryByString )
        {
            ULONG nIndex = 0;
            rEntry.bQueryByString = !(pDocument->GetFormatTable()->
                IsNumberFormat( *rEntry.pStr, nIndex, rEntry.nVal ));
        }
        else
        {
            double nTemp = rEntry.nVal;
            if (nTemp == SC_EMPTYFIELDS || nTemp == SC_NONEMPTYFIELDS)
                pSpecial[i] = TRUE;
            // #58736# QueryParam mit !bQueryByString kann per Uno oder zweitem
            // Aufruf per AutoFilter kommen - hier keine Assertion mehr
        }
        if ( !bTopTen )
        {
            switch ( rEntry.eOp )
            {
                case SC_TOPVAL:
                case SC_BOTVAL:
                case SC_TOPPERC:
                case SC_BOTPERC:
                    bTopTen = TRUE;
            }
        }
    }
    ScQueryParam* pTopTenParam;
    if ( bTopTen )
    {   // original Param erhalten und Kopie anpassen
        pTopTenParam = new ScQueryParam( rParamOrg );
        TopTenQuery( *pTopTenParam );
    }
    else
        pTopTenParam = NULL;
    ScQueryParam& rParam = (bTopTen ? *pTopTenParam : rParamOrg);

    if (!rParam.bInplace)
    {
        nOutRow = rParam.nDestRow + nHeader;
        if (nHeader)
            CopyData( rParam.nCol1, rParam.nRow1, rParam.nCol2, rParam.nRow1,
                            rParam.nDestCol, rParam.nDestRow, rParam.nDestTab );
    }

    for (i=rParam.nRow1 + nHeader; i<=rParam.nRow2; i++)
    {
        BOOL bResult;                                   // Filterergebnis
        BOOL bValid = ValidQuery(i, rParam, pSpecial);
        if (!bValid && bKeepSub)                        // Subtotals stehenlassen
        {
            for (USHORT nCol=rParam.nCol1; nCol<=rParam.nCol2 && !bValid; nCol++)
            {
                ScBaseCell* pCell;
                pCell = GetCell( nCol, i );
                if ( pCell )
                    if ( pCell->GetCellType() == CELLTYPE_FORMULA )
                        if (((ScFormulaCell*)pCell)->IsSubTotal())
                            if (RefVisible((ScFormulaCell*)pCell))
                                bValid = TRUE;
            }
        }
        if (bValid)
        {
            if (rParam.bDuplicate)
                bResult = TRUE;
            else
            {
                String aStr;
                for (USHORT j=rParam.nCol1; j <= rParam.nCol2; j++)
                {
                    String aCellStr;
                    GetString(j, i, aCellStr);
                    aStr += aCellStr;
                    aStr += (sal_Unicode)1;
                }
                pStrData = new StrData(aStr);

                BOOL bIsUnique = TRUE;
                if (pStrData)
                    bIsUnique = aStrCollection.Insert(pStrData);
                if (bIsUnique)
                    bResult = TRUE;
                else
                {
                    delete pStrData;
                    bResult = FALSE;
                }
            }
        }
        else
            bResult = FALSE;

        if (rParam.bInplace)
        {
            if (bResult == bOldResult && bStarted)
                nOldEnd = i;
            else
            {
                if (bStarted)
                    DBShowRows(nOldStart,nOldEnd, bOldResult);
                nOldStart = nOldEnd = i;
                bOldResult = bResult;
            }
            bStarted = TRUE;
        }
        else
        {
            if (bResult)
            {
                CopyData( rParam.nCol1,i, rParam.nCol2,i, rParam.nDestCol,nOutRow,rParam.nDestTab );
                ++nOutRow;
            }
        }
        if (bResult)
            ++nCount;
    }

    if (rParam.bInplace && bStarted)
        DBShowRows(nOldStart,nOldEnd, bOldResult);

    delete[] pSpecial;
    if ( pTopTenParam )
        delete pTopTenParam;

    return nCount;
}

BOOL ScTable::CreateExcelQuery(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2, ScQueryParam& rQueryParam)
{
    USHORT  i;
    BOOL    bValid = TRUE;
    USHORT* pFields = new USHORT[nCol2-nCol1+1];
    String  aCellStr;
    USHORT  nCol = nCol1;
    DBG_ASSERT( rQueryParam.nTab != USHRT_MAX, "rQueryParam.nTab no value, not bad but no good" );
    USHORT  nDBTab = (rQueryParam.nTab == USHRT_MAX ? nTab : rQueryParam.nTab);
    USHORT  nDBRow1 = rQueryParam.nRow1;
    USHORT  nDBCol2 = rQueryParam.nCol2;
    // Erste Zeile muessen Spaltenkoepfe sein
    while (bValid && (nCol <= nCol2))
    {
        String aQueryStr;
        GetUpperCellString(nCol, nRow1, aQueryStr);
        BOOL bFound = FALSE;
        i = rQueryParam.nCol1;
        while (!bFound && (i <= nDBCol2))
        {
            if ( nTab == nDBTab )
                GetUpperCellString(i, nDBRow1, aCellStr);
            else
                pDocument->GetUpperCellString(i, nDBRow1, nDBTab, aCellStr);
            bFound = (aCellStr == aQueryStr);
            if (!bFound) i++;
        }
        if (bFound)
            pFields[nCol - nCol1] = i;
        else
            bValid = FALSE;
        nCol++;
    }
    if (bValid)
    {
        long nVisible = 0;
        for ( nCol=nCol1; nCol<=nCol2; nCol++ )
            nVisible += aCol[nCol].VisibleCount( nRow1+1, nRow2 );

        if ( nVisible > USHRT_MAX / sizeof(void*) )
        {
            DBG_ERROR("zu viele Filterkritierien");
            nVisible = 0;
        }

        USHORT nNewEntries = (USHORT) nVisible;
        rQueryParam.Resize( nNewEntries );

        USHORT nIndex = 0;
        USHORT nRow = nRow1 + 1;
        while (nRow <= nRow2)
        {
            nCol = nCol1;
            while (nCol <= nCol2)
            {
                GetUpperCellString(nCol, nRow, aCellStr);
                if (aCellStr.Len() > 0)
                {
                    if (nIndex < nNewEntries)
                    {
                        rQueryParam.GetEntry(nIndex).nField = pFields[nCol - nCol1];
                        rQueryParam.FillInExcelSyntax(aCellStr, nIndex);
                        nIndex++;
                        if (nIndex < nNewEntries)
                            rQueryParam.GetEntry(nIndex).eConnect = SC_AND;
                    }
                    else
                        bValid = FALSE;
                }
                nCol++;
            }
            nRow++;
            if (nIndex < nNewEntries)
                rQueryParam.GetEntry(nIndex).eConnect = SC_OR;
        }
    }
    delete pFields;
    return bValid;
}

BOOL ScTable::CreateStarQuery(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2, ScQueryParam& rQueryParam)
{
    BOOL bValid;
    BOOL bFound;
    String aCellStr;
    USHORT nIndex = 0;
    USHORT nRow = nRow1;
    DBG_ASSERT( rQueryParam.nTab != USHRT_MAX, "rQueryParam.nTab no value, not bad but no good" );
    USHORT  nDBTab = (rQueryParam.nTab == USHRT_MAX ? nTab : rQueryParam.nTab);
    USHORT  nDBRow1 = rQueryParam.nRow1;
    USHORT  nDBCol2 = rQueryParam.nCol2;

    USHORT nNewEntries = nRow2-nRow1+1;
    rQueryParam.Resize( nNewEntries );

    do
    {
        ScQueryEntry& rEntry = rQueryParam.GetEntry(nIndex);

        bValid = FALSE;
        // Erste Spalte UND/ODER
        if (nIndex > 0)
        {
            GetUpperCellString(nCol1, nRow, aCellStr);
            if ( aCellStr == ScGlobal::GetRscString(STR_TABLE_UND) )
            {
                rEntry.eConnect = SC_AND;
                bValid = TRUE;
            }
            else if ( aCellStr == ScGlobal::GetRscString(STR_TABLE_ODER) )
            {
                rEntry.eConnect = SC_OR;
                bValid = TRUE;
            }
        }
        // Zweite Spalte FeldName
        if ((nIndex < 1) || bValid)
        {
            bFound = FALSE;
            GetUpperCellString(nCol1 + 1, nRow, aCellStr);
            for (USHORT i=rQueryParam.nCol1; (i <= nDBCol2) && (!bFound); i++)
            {
                String aFieldStr;
                if ( nTab == nDBTab )
                    GetUpperCellString(i, nDBRow1, aFieldStr);
                else
                    pDocument->GetUpperCellString(i, nDBRow1, nDBTab, aFieldStr);
                bFound = (aCellStr == aFieldStr);
                if (bFound)
                {
                    rEntry.nField = i;
                    bValid = TRUE;
                }
                else
                    bValid = FALSE;
            }
        }
        // Dritte Spalte Operator =<>...
        if (bValid)
        {
            bFound = FALSE;
            GetUpperCellString(nCol1 + 2, nRow, aCellStr);
            if (aCellStr.GetChar(0) == '<')
            {
                if (aCellStr.GetChar(1) == '>')
                    rEntry.eOp = SC_NOT_EQUAL;
                else if (aCellStr.GetChar(1) == '=')
                    rEntry.eOp = SC_LESS_EQUAL;
                else
                    rEntry.eOp = SC_LESS;
            }
            else if (aCellStr.GetChar(0) == '>')
            {
                if (aCellStr.GetChar(1) == '=')
                    rEntry.eOp = SC_GREATER_EQUAL;
                else
                    rEntry.eOp = SC_GREATER;
            }
            else if (aCellStr.GetChar(0) == '=')
                rEntry.eOp = SC_EQUAL;

        }
        // Vierte Spalte Wert
        if (bValid)
        {
            GetString(nCol1 + 3, nRow, *rEntry.pStr);
            rEntry.bDoQuery = TRUE;
        }
        nIndex++;
        nRow++;
    }
    while (bValid && (nRow <= nRow2) /* && (nIndex < MAXQUERY) */ );
    return bValid;
}

BOOL ScTable::CreateQueryParam(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2, ScQueryParam& rQueryParam)
{
    USHORT i, nCount;
    PutInOrder(nCol1, nCol2);
    PutInOrder(nRow1, nRow2);

    nCount = rQueryParam.GetEntryCount();
    for (i=0; i < nCount; i++)
        rQueryParam.GetEntry(i).Clear();

    // Standard QueryTabelle
    BOOL bValid = CreateStarQuery(nCol1, nRow1, nCol2, nRow2, rQueryParam);
    // Excel QueryTabelle
    if (!bValid)
        bValid = CreateExcelQuery(nCol1, nRow1, nCol2, nRow2, rQueryParam);

    nCount = rQueryParam.GetEntryCount();
    if (bValid)
    {
        //  bQueryByString muss gesetzt sein
        for (i=0; i < nCount; i++)
            rQueryParam.GetEntry(i).bQueryByString = TRUE;
    }
    else
    {
        //  nix
        for (i=0; i < nCount; i++)
            rQueryParam.GetEntry(i).Clear();
    }
    return bValid;
}

BOOL ScTable::HasColHeader( USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow )
{
    for (USHORT nCol=nStartCol; nCol<=nEndCol; nCol++)
    {
        CellType eType = GetCellType( nCol, nStartRow );
        if (eType != CELLTYPE_STRING && eType != CELLTYPE_EDIT)
            return FALSE;
    }
    return TRUE;
}

BOOL ScTable::HasRowHeader( USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow )
{
    for (USHORT nRow=nStartRow; nRow<=nEndRow; nRow++)
    {
        CellType eType = GetCellType( nStartCol, nRow );
        if (eType != CELLTYPE_STRING && eType != CELLTYPE_EDIT)
            return FALSE;
    }
    return TRUE;
}

void ScTable::GetFilterEntries(USHORT nCol, USHORT nRow1, USHORT nRow2, TypedStrCollection& rStrings)
{
    aCol[nCol].GetFilterEntries( nRow1, nRow2, rStrings );
}

BOOL ScTable::GetDataEntries(USHORT nCol, USHORT nRow, TypedStrCollection& rStrings, BOOL bLimit)
{
    return aCol[nCol].GetDataEntries( nRow, rStrings, bLimit );
}

long ScTable::GetCellCount() const
{
    long nCellCount = 0;

    for ( USHORT nCol=0; nCol<=MAXCOL; nCol++ )
        nCellCount += aCol[nCol].GetCellCount();

    return nCellCount;
}

long ScTable::GetWeightedCount() const
{
    long nCellCount = 0;

    for ( USHORT nCol=0; nCol<=MAXCOL; nCol++ )
        if ( aCol[nCol].GetCellCount() )                    // GetCellCount ist inline
            nCellCount += aCol[nCol].GetWeightedCount();

    return nCellCount;
}

ULONG ScTable::GetCodeCount() const
{
    ULONG nCodeCount = 0;

    for ( USHORT nCol=0; nCol<=MAXCOL; nCol++ )
        if ( aCol[nCol].GetCellCount() )                    // GetCellCount ist inline
            nCodeCount += aCol[nCol].GetCodeCount();

    return nCodeCount;
}

xub_StrLen ScTable::GetMaxStringLen( USHORT nCol,
                                USHORT nRowStart, USHORT nRowEnd ) const
{
    if ( nCol <= MAXCOL )
        return aCol[nCol].GetMaxStringLen( nRowStart, nRowEnd );
    else
        return 0;
}

xub_StrLen ScTable::GetMaxNumberStringLen( USHORT& nPrecision, USHORT nCol,
                                USHORT nRowStart, USHORT nRowEnd ) const
{
    if ( nCol <= MAXCOL )
        return aCol[nCol].GetMaxNumberStringLen( nPrecision, nRowStart, nRowEnd );
    else
        return 0;
}

void ScTable::UpdateSelectionFunction( ScFunctionData& rData,
                        USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow,
                        const ScMarkData& rMark )
{
    //  Cursor neben einer Markierung nicht beruecksichtigen:
    //! nur noch MarkData uebergeben, Cursorposition ggf. hineinselektieren!!!
    BOOL bSingle = ( rMark.IsMarked() || !rMark.IsMultiMarked() );

    // Mehrfachselektion:

    USHORT nCol;
    if ( rMark.IsMultiMarked() )
        for (nCol=0; nCol<=MAXCOL && !rData.bError; nCol++)
            if ( !pColFlags || !( pColFlags[nCol] & CR_HIDDEN ) )
                aCol[nCol].UpdateSelectionFunction( rMark, rData, pRowFlags,
                                                    bSingle && ( nCol >= nStartCol && nCol <= nEndCol ),
                                                    nStartRow, nEndRow );

    //  Einfachselektion (oder Cursor) nur wenn nicht negativ (und s.o.):

    if ( bSingle && !rMark.IsMarkNegative() )
        for (nCol=nStartCol; nCol<=nEndCol && !rData.bError; nCol++)
            if ( !pColFlags || !( pColFlags[nCol] & CR_HIDDEN ) )
                aCol[nCol].UpdateAreaFunction( rData, pRowFlags, nStartRow, nEndRow );
}

void ScTable::FindConditionalFormat( ULONG nKey, ScRangeList& rList )
{
    USHORT nStartRow, nEndRow;
    for (USHORT nCol=0; nCol<=MAXCOL; nCol++)
    {
        ScAttrIterator* pIter = aCol[nCol].CreateAttrIterator( 0, MAXCOL );
        const ScPatternAttr* pPattern = pIter->Next( nStartRow, nEndRow );
        while (pPattern)
        {
            if (((SfxUInt32Item&)pPattern->GetItem(ATTR_CONDITIONAL)).GetValue() == nKey)
                rList.Join( ScRange(nCol,nStartRow,nTab, nCol,nEndRow,nTab) );
            pPattern = pIter->Next( nStartRow, nEndRow );
        }
        delete pIter;
    }
}




