/*************************************************************************
 *
 *  $RCSfile: dociter.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:14 $
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

#include <svtools/zforlist.hxx>

#include "scitems.hxx"
#include "global.hxx"
#include "dociter.hxx"
#include "document.hxx"
#include "table.hxx"
#include "column.hxx"
#include "cell.hxx"
#include "attarray.hxx"
#include "patattr.hxx"
#include "docoptio.hxx"


// STATIC DATA -----------------------------------------------------------

ScDocumentIterator::ScDocumentIterator( ScDocument* pDocument,
                            USHORT nStartTable, USHORT nEndTable ) :
    pDoc( pDocument ),
    nStartTab( nStartTable ),
    nEndTab( nEndTable )
{
    PutInOrder( nStartTab, nEndTab );
    if (nStartTab > MAXTAB) nStartTab = MAXTAB;
    if (nEndTab > MAXTAB) nEndTab = MAXTAB;

    pDefPattern = pDoc->GetDefPattern();

    nCol = 0;
    nRow = 0;
    nTab = nStartTab;

    nColPos = 0;
    nAttrPos = 0;
}

ScDocumentIterator::~ScDocumentIterator()
{
}

BOOL ScDocumentIterator::GetThisCol()
{
    ScTable*        pTab;
    while ( (pTab = pDoc->pTab[nTab]) == NULL )
    {
        if ( nTab == nEndTab )
        {
            nCol = MAXCOL;
            nRow = MAXROW;
            return FALSE;
        }
        ++nTab;
    }
    ScColumn*       pCol = &pTab->aCol[nCol];
    ScAttrArray*    pAtt = pCol->pAttrArray;

    BOOL bFound = FALSE;
    do
    {
        USHORT nColRow;
        USHORT nAttrEnd;

        do
        {
            nAttrEnd = pAtt->pData[nAttrPos].nRow;
            if (nAttrEnd < nRow)
                ++nAttrPos;
        }
        while (nAttrEnd < nRow);

        do
        {
            nColRow = (nColPos < pCol->nCount) ? pCol->pItems[nColPos].nRow : MAXROW+1;
            if (nColRow < nRow)
                ++nColPos;
        }
        while (nColRow < nRow);

        if (nColRow == nRow)
        {
            bFound   = TRUE;
            pCell    = pCol->pItems[nColPos].pCell;
            pPattern = pAtt->pData[nAttrPos].pPattern;
        }
        else if ( pAtt->pData[nAttrPos].pPattern != pDefPattern )
        {
            bFound = TRUE;
            pCell = NULL;
            pPattern = pAtt->pData[nAttrPos].pPattern;
        }
        else
        {
            nRow = Min( (USHORT)nColRow, (USHORT)(nAttrEnd+1) );
        }
    }
    while (!bFound && nRow <= MAXROW);

    return bFound;
}

BOOL ScDocumentIterator::GetThis()
{
    BOOL bEnd = FALSE;
    BOOL bSuccess = FALSE;

    while ( !bSuccess && !bEnd )
    {
        if ( nRow > MAXROW )
            bSuccess = FALSE;
        else
            bSuccess = GetThisCol();

        if ( !bSuccess )
        {
            ++nCol;
            if (nCol > MAXCOL)
            {
                nCol = 0;
                ++nTab;
                if (nTab > nEndTab)
                    bEnd = TRUE;
            }
            nRow = 0;
            nColPos = 0;
            nAttrPos = 0;
        }
    }

    return !bEnd;
}

BOOL ScDocumentIterator::GetFirst()
{
    nCol = 0;
    nTab = nStartTab;

    nRow = 0;
    nColPos = 0;
    nAttrPos = 0;

    return GetThis();
}

BOOL ScDocumentIterator::GetNext()
{
    ++nRow;

    return GetThis();
}

//------------------------------------------------------------------------

ScBaseCell* ScDocumentIterator::GetCell()
{
    return pCell;
}

const ScPatternAttr* ScDocumentIterator::GetPattern()
{
    return pPattern;
}

void ScDocumentIterator::GetPos( USHORT& rCol, USHORT& rRow, USHORT& rTab )
{
    rCol = nCol;
    rRow = nRow;
    rTab = nTab;
}


//------------------------------------------------------------------------
//------------------------------------------------------------------------
void lcl_IterGetNumberFormat( ULONG& nFormat, const ScAttrArray*& rpArr,
        USHORT& nAttrEndRow, const ScAttrArray* pNewArr, USHORT nRow,
        ScDocument* pDoc )
{
    if ( rpArr != pNewArr || nAttrEndRow < nRow )
    {
        short nPos;
        pNewArr->Search( nRow, nPos );  // nPos 0 gueltig wenn nicht gefunden
        const ScPatternAttr* pPattern = pNewArr->pData[nPos].pPattern;
        nFormat = pPattern->GetNumberFormat( pDoc->GetFormatTable() );
        rpArr = pNewArr;
        nAttrEndRow = pNewArr->pData[nPos].nRow;
    }
}

ScValueIterator::ScValueIterator( ScDocument* pDocument,
                                    USHORT nSCol, USHORT nSRow, USHORT nSTab,
                                    USHORT nECol, USHORT nERow, USHORT nETab,
                                    BOOL bSTotal, BOOL bTextZero ) :
    pDoc( pDocument ),
    nStartCol( nSCol),
    nStartRow( nSRow),
    nStartTab( nSTab ),
    nEndCol( nECol ),
    nEndRow( nERow),
    nEndTab( nETab ),
    bSubTotal(bSTotal),
    nNumFmtType( NUMBERFORMAT_UNDEFINED ),
    nNumFmtIndex(0),
    bNumValid( FALSE ),
    bNextValid( FALSE ),
    bCalcAsShown( pDocument->GetDocOptions().IsCalcAsShown() ),
    bTextAsZero( bTextZero )
{
    PutInOrder( nStartCol, nEndCol);
    PutInOrder( nStartRow, nEndRow);
    PutInOrder( nStartTab, nEndTab );

    if (nStartCol > MAXCOL) nStartCol = MAXCOL;
    if (nEndCol > MAXCOL) nEndCol = MAXCOL;
    if (nStartRow > MAXROW) nStartRow = MAXROW;
    if (nEndRow > MAXROW) nEndRow = MAXROW;
    if (nStartTab > MAXTAB) nStartTab = MAXTAB;
    if (nEndTab > MAXTAB) nEndTab = MAXTAB;

    nCol = nStartCol;
    nRow = nStartRow;
    nTab = nStartTab;

    nColRow = 0;                    // wird bei GetFirst initialisiert

    nNumFormat = 0;                 // werden bei GetNumberFormat initialisiert
    pAttrArray = 0;
    nAttrEndRow = 0;
}

ScValueIterator::ScValueIterator( ScDocument* pDocument, const ScRange& rRange,
            BOOL bSTotal, BOOL bTextZero ) :
    pDoc( pDocument ),
    nStartCol( rRange.aStart.Col() ),
    nStartRow( rRange.aStart.Row() ),
    nStartTab( rRange.aStart.Tab() ),
    nEndCol( rRange.aEnd.Col() ),
    nEndRow( rRange.aEnd.Row() ),
    nEndTab( rRange.aEnd.Tab() ),
    bSubTotal(bSTotal),
    nNumFmtType( NUMBERFORMAT_UNDEFINED ),
    nNumFmtIndex(0),
    bNumValid( FALSE ),
    bNextValid( FALSE ),
    bCalcAsShown( pDocument->GetDocOptions().IsCalcAsShown() ),
    bTextAsZero( bTextZero )
{
    PutInOrder( nStartCol, nEndCol);
    PutInOrder( nStartRow, nEndRow);
    PutInOrder( nStartTab, nEndTab );

    if (nStartCol > MAXCOL) nStartCol = MAXCOL;
    if (nEndCol > MAXCOL) nEndCol = MAXCOL;
    if (nStartRow > MAXROW) nStartRow = MAXROW;
    if (nEndRow > MAXROW) nEndRow = MAXROW;
    if (nStartTab > MAXTAB) nStartTab = MAXTAB;
    if (nEndTab > MAXTAB) nEndTab = MAXTAB;

    nCol = nStartCol;
    nRow = nStartRow;
    nTab = nStartTab;

    nColRow = 0;                    // wird bei GetFirst initialisiert

    nNumFormat = 0;                 // werden bei GetNumberFormat initialisiert
    pAttrArray = 0;
    nAttrEndRow = 0;
}

BOOL ScValueIterator::GetThis(double& rValue, USHORT& rErr)
{
    ScColumn* pCol = &(pDoc->pTab[nTab])->aCol[nCol];
    for (;;)
    {
        if ( nRow > nEndRow )
        {
            nRow = nStartRow;
            do
            {
                nCol++;
                if ( nCol > nEndCol )
                {
                    nCol = nStartCol;
                    nTab++;
                    if ( nTab > nEndTab )
                    {
                        rValue = 0.0;
                        rErr = 0;
                        return FALSE;               // Ende und Aus
                    }
                }
                pCol = &(pDoc->pTab[nTab])->aCol[nCol];
            } while ( pCol->nCount == 0 );
            pCol->Search( nRow, nColRow );
        }

        while (( nColRow < pCol->nCount ) && ( pCol->pItems[nColRow].nRow < nRow ))
            nColRow++;

        if ( nColRow < pCol->nCount && pCol->pItems[nColRow].nRow <= nEndRow )
        {
            nRow = pCol->pItems[nColRow].nRow + 1;
            if ( !bSubTotal || !pDoc->pTab[nTab]->IsFiltered( nRow-1 ) )
            {
                ScBaseCell* pCell = pCol->pItems[nColRow].pCell;
                ++nColRow;
                switch (pCell->GetCellType())
                {
                    case CELLTYPE_VALUE:
                    {
                        bNumValid = FALSE;
                        rValue = ((ScValueCell*)pCell)->GetValue();
                        rErr = 0;
                        --nRow;
                        if ( bCalcAsShown )
                        {
#ifndef WTC
                            lcl_IterGetNumberFormat( nNumFormat,pAttrArray,
#else
                            lcl_IterGetNumberFormat( nNumFormat,
                                (ScAttrArray const *&)pAttrArray,
#endif
                                nAttrEndRow, pCol->pAttrArray, nRow, pDoc );
                            rValue = pDoc->RoundValueAsShown( rValue, nNumFormat );
                        }
                        //
                        //  wenn in der selben Spalte gleich noch eine Value-Cell folgt, die
                        //  auch noch im Block liegt, den Wert jetzt schon holen
                        //
                        if ( nColRow < pCol->nCount &&
                             pCol->pItems[nColRow].nRow <= nEndRow &&
                             pCol->pItems[nColRow].pCell->GetCellType() == CELLTYPE_VALUE &&
                             !bSubTotal )
                        {
                            fNextValue = ((ScValueCell*)pCol->pItems[nColRow].pCell)->GetValue();
                            nNextRow = pCol->pItems[nColRow].nRow;
                            bNextValid = TRUE;
                            if ( bCalcAsShown )
                            {
#ifndef WTC
                                lcl_IterGetNumberFormat( nNumFormat, pAttrArray,
#else
                                lcl_IterGetNumberFormat( nNumFormat,
                                    (ScAttrArray const *&)pAttrArray,
#endif
                                    nAttrEndRow, pCol->pAttrArray, nNextRow, pDoc );
                                fNextValue = pDoc->RoundValueAsShown( fNextValue, nNumFormat );
                            }
                        }

                        return TRUE;                                    // gefunden
                    }
                    break;
                    case CELLTYPE_FORMULA:
                    {
                        if (!bSubTotal || !((ScFormulaCell*)pCell)->IsSubTotal())
                        {
                            rErr = ((ScFormulaCell*)pCell)->GetErrCode();
                            if ( rErr || ((ScFormulaCell*)pCell)->IsValue() )
                            {
                                rValue = ((ScFormulaCell*)pCell)->GetValue();
                                nRow--;
                                bNumValid = FALSE;
                                return TRUE;                            // gefunden
                            }
                        }
                    }
                    break;
                    case CELLTYPE_STRING :
                    case CELLTYPE_EDIT :
                    {
                        if ( bTextAsZero )
                        {
                            rErr = 0;
                            rValue = 0.0;
                            nNumFmtType = NUMBERFORMAT_NUMBER;
                            nNumFmtIndex = 0;
                            bNumValid = TRUE;
                            --nRow;
                            return TRUE;
                        }
                    }
                    break;
                }
            }
        }
        else
            nRow = nEndRow + 1;         // naechste Spalte
    }
}

void ScValueIterator::GetCurNumFmtInfo( short& nType, ULONG& nIndex )
{
    if (!bNumValid)
    {
        const ScColumn* pCol = &(pDoc->pTab[nTab])->aCol[nCol];
        nNumFmtIndex = pCol->GetNumberFormat( nRow );
        if ( (nNumFmtIndex % SV_COUNTRY_LANGUAGE_OFFSET) == 0 )
        {
            const ScBaseCell* pCell;
            USHORT nIdx = nColRow - 1;
            // there might be rearranged something, so be on the safe side
            if ( nIdx < pCol->nCount && pCol->pItems[nIdx].nRow == nRow )
                pCell = pCol->pItems[nIdx].pCell;
            else
            {
                if ( pCol->Search( nRow, nIdx ) )
                    pCell = pCol->pItems[nIdx].pCell;
                else
                    pCell = NULL;
            }
            if ( pCell && pCell->GetCellType() == CELLTYPE_FORMULA )
                ((const ScFormulaCell*)pCell)->GetFormatInfo( nNumFmtType, nNumFmtIndex );
            else
                nNumFmtType = pDoc->GetFormatTable()->GetType( nNumFmtIndex );
        }
        else
            nNumFmtType = pDoc->GetFormatTable()->GetType( nNumFmtIndex );
        bNumValid = TRUE;
    }
    nType = nNumFmtType;
    nIndex = nNumFmtIndex;
}

BOOL ScValueIterator::GetFirst(double& rValue, USHORT& rErr)
{
    nCol = nStartCol;
    nRow = nStartRow;
    nTab = nStartTab;

//  nColRow = 0;
    ScColumn* pCol = &(pDoc->pTab[nTab])->aCol[nCol];
    pCol->Search( nRow, nColRow );

    nNumFormat = 0;                 // werden bei GetNumberFormat initialisiert
    pAttrArray = 0;
    nAttrEndRow = 0;

    return GetThis(rValue, rErr);
}

/*  ist inline:
BOOL ScValueIterator::GetNext(double& rValue, USHORT& rErr)
{
    ++nRow;
    return GetThis(rValue, rErr);
}
*/

//------------------------------------------------------------------------
//------------------------------------------------------------------------

ScQueryValueIterator::ScQueryValueIterator(ScDocument* pDocument, USHORT nTable, const ScQueryParam& rParam) :
    pDoc( pDocument ),
    nTab( nTable),
    aParam (rParam),
    nNumFmtType( NUMBERFORMAT_UNDEFINED ),
    nNumFmtIndex(0),
    bCalcAsShown( pDocument->GetDocOptions().IsCalcAsShown() )
{
    nCol = aParam.nCol1;
    nRow = aParam.nRow1;
    nColRow = 0;                    // wird bei GetFirst initialisiert
    USHORT i;
    USHORT nCount = aParam.GetEntryCount();
    for (i=0; (i<nCount) && (aParam.GetEntry(i).bDoQuery); i++)
    {
        ScQueryEntry& rEntry = aParam.GetEntry(i);
        ULONG nIndex = 0;
        rEntry.bQueryByString =
                !(pDoc->GetFormatTable()->IsNumberFormat(*rEntry.pStr, nIndex, rEntry.nVal));
    }
    nNumFormat = 0;                 // werden bei GetNumberFormat initialisiert
    pAttrArray = 0;
    nAttrEndRow = 0;
}

BOOL ScQueryValueIterator::GetThis(double& rValue, USHORT& rErr)
{
    ScColumn* pCol = &(pDoc->pTab[nTab])->aCol[nCol];
    for ( ;; )
    {
        if ( nRow > aParam.nRow2 )
        {
            nRow = aParam.nRow1;
            if (aParam.bHasHeader)
                nRow++;
            do
            {
                nCol++;
                if ( nCol > aParam.nCol2 )
                {
                    rValue = 0.0;
                    rErr = 0;
                    return FALSE;               // Ende und Aus
                }
                pCol = &(pDoc->pTab[nTab])->aCol[nCol];
            } while ( pCol->nCount == 0 );
            pCol->Search( nRow, nColRow );
        }

        while ( (nColRow < pCol->nCount) && (pCol->pItems[nColRow].nRow < nRow) )
            nColRow++;

        if ( nColRow < pCol->nCount && pCol->pItems[nColRow].nRow <= aParam.nRow2 )
        {
            nRow = pCol->pItems[nColRow].nRow;
            if ((pDoc->pTab[nTab])->ValidQuery(nRow, aParam))
            {
                ScBaseCell* pCell = pCol->pItems[nColRow].pCell;
                switch (pCell->GetCellType())
                {
                    case CELLTYPE_VALUE:
                        {
                            rValue = ((ScValueCell*)pCell)->GetValue();
                            if ( bCalcAsShown )
                            {
#if ! ( defined WTC || defined IRIX || defined ICC || defined HPUX || defined C50 )
                                lcl_IterGetNumberFormat( nNumFormat, pAttrArray,
#else
                                lcl_IterGetNumberFormat( nNumFormat,
                                    (ScAttrArray const *&)pAttrArray,
#endif
                                    nAttrEndRow, pCol->pAttrArray, nRow, pDoc );
                                rValue = pDoc->RoundValueAsShown( rValue, nNumFormat );
                            }
                            nNumFmtType = NUMBERFORMAT_NUMBER;
                            nNumFmtIndex = 0;
                            rErr = 0;
                            return TRUE;        // gefunden
                        }
                        break;
                    case CELLTYPE_FORMULA:
                        {
                            if (((ScFormulaCell*)pCell)->IsValue())
                            {
                                rValue = ((ScFormulaCell*)pCell)->GetValue();
                                pDoc->GetNumberFormatInfo( nNumFmtType,
                                    nNumFmtIndex, ScAddress( nCol, nRow, nTab ),
                                    *((ScFormulaCell*)pCell) );
                                rErr = ((ScFormulaCell*)pCell)->GetErrCode();
                                return TRUE;    // gefunden
                            }
                            else
                                nRow++;
                        }
                        break;
                    default:
                        nRow++;
                        break;
                }
            }
            else
                nRow++;
        }
        else
            nRow = aParam.nRow2 + 1; // Naechste Spalte
    }
    return FALSE;
}

BOOL ScQueryValueIterator::GetFirst(double& rValue, USHORT& rErr)
{
    nCol = aParam.nCol1;
    nRow = aParam.nRow1;
    if (aParam.bHasHeader)
        nRow++;
//  nColRow = 0;
    ScColumn* pCol = &(pDoc->pTab[nTab])->aCol[nCol];
    pCol->Search( nRow, nColRow );
    return GetThis(rValue, rErr);
}

BOOL ScQueryValueIterator::GetNext(double& rValue, USHORT& rErr)
{
    ++nRow;
    return GetThis(rValue, rErr);
}

//-------------------------------------------------------------------------------

ScCellIterator::ScCellIterator( ScDocument* pDocument,
                                USHORT nSCol, USHORT nSRow, USHORT nSTab,
                                USHORT nECol, USHORT nERow, USHORT nETab, BOOL bSTotal ) :
    pDoc( pDocument ),
    nStartCol( nSCol),
    nStartRow( nSRow),
    nStartTab( nSTab ),
    nEndCol( nECol ),
    nEndRow( nERow),
    nEndTab( nETab ),
    bSubTotal(bSTotal)

{
    PutInOrder( nStartCol, nEndCol);
    PutInOrder( nStartRow, nEndRow);
    PutInOrder( nStartTab, nEndTab );

    if (nStartCol > MAXCOL) nStartCol = MAXCOL;
    if (nEndCol > MAXCOL) nEndCol = MAXCOL;
    if (nStartRow > MAXROW) nStartRow = MAXROW;
    if (nEndRow > MAXROW) nEndRow = MAXROW;
    if (nStartTab > MAXTAB) nStartTab = MAXTAB;
    if (nEndTab > MAXTAB) nEndTab = MAXTAB;

    while (nEndTab>0 && !pDoc->pTab[nEndTab])
        --nEndTab;                                      // nur benutzte Tabellen
    if (nStartTab>nEndTab)
        nStartTab = nEndTab;

    nCol = nStartCol;
    nRow = nStartRow;
    nTab = nStartTab;
    nColRow = 0;                    // wird bei GetFirst initialisiert

    if (!pDoc->pTab[nTab])
    {
        DBG_ERROR("Tabelle nicht gefunden");
        nStartCol = nCol = MAXCOL+1;
        nStartRow = nRow = MAXROW+1;
        nStartTab = nTab = MAXTAB+1;    // -> Abbruch bei GetFirst
    }
}

ScCellIterator::ScCellIterator
    ( ScDocument* pDocument, const ScRange& rRange, BOOL bSTotal ) :
    pDoc( pDocument ),
    nStartCol( rRange.aStart.Col() ),
    nStartRow( rRange.aStart.Row() ),
    nStartTab( rRange.aStart.Tab() ),
    nEndCol( rRange.aEnd.Col() ),
    nEndRow( rRange.aEnd.Row() ),
    nEndTab( rRange.aEnd.Tab() ),
    bSubTotal(bSTotal)

{
    PutInOrder( nStartCol, nEndCol);
    PutInOrder( nStartRow, nEndRow);
    PutInOrder( nStartTab, nEndTab );

    if (nStartCol > MAXCOL) nStartCol = MAXCOL;
    if (nEndCol > MAXCOL) nEndCol = MAXCOL;
    if (nStartRow > MAXROW) nStartRow = MAXROW;
    if (nEndRow > MAXROW) nEndRow = MAXROW;
    if (nStartTab > MAXTAB) nStartTab = MAXTAB;
    if (nEndTab > MAXTAB) nEndTab = MAXTAB;

    while (nEndTab>0 && !pDoc->pTab[nEndTab])
        --nEndTab;                                      // nur benutzte Tabellen
    if (nStartTab>nEndTab)
        nStartTab = nEndTab;

    nCol = nStartCol;
    nRow = nStartRow;
    nTab = nStartTab;
    nColRow = 0;                    // wird bei GetFirst initialisiert

    if (!pDoc->pTab[nTab])
    {
        DBG_ERROR("Tabelle nicht gefunden");
        nStartCol = nCol = MAXCOL+1;
        nStartRow = nRow = MAXROW+1;
        nStartTab = nTab = MAXTAB+1;    // -> Abbruch bei GetFirst
    }
}

ScBaseCell* ScCellIterator::GetThis()
{
    ScColumn* pCol = &(pDoc->pTab[nTab])->aCol[nCol];
    for ( ;; )
    {
        if ( nRow > nEndRow )
        {
            nRow = nStartRow;
            do
            {
                nCol++;
                if ( nCol > nEndCol )
                {
                    nCol = nStartCol;
                    nTab++;
                    if ( nTab > nEndTab )
                        return NULL;                // Ende und Aus
                }
                pCol = &(pDoc->pTab[nTab])->aCol[nCol];
            } while ( pCol->nCount == 0 );
            pCol->Search( nRow, nColRow );
        }

        while ( (nColRow < pCol->nCount) && (pCol->pItems[nColRow].nRow < nRow) )
            nColRow++;

        if ( nColRow < pCol->nCount && pCol->pItems[nColRow].nRow <= nEndRow )
        {
            nRow = pCol->pItems[nColRow].nRow;
            if ( !bSubTotal || !pDoc->pTab[nTab]->IsFiltered( nRow ) )
            {
                ScBaseCell* pCell = pCol->pItems[nColRow].pCell;

                if ( bSubTotal && pCell->GetCellType() == CELLTYPE_FORMULA
                                && ((ScFormulaCell*)pCell)->IsSubTotal() )
                    nRow++;             // Sub-Total-Zeilen nicht
                else
                    return pCell;       // gefunden
            }
            else
                nRow++;
        }
        else
            nRow = nEndRow + 1; // Naechste Spalte
    }
}

ScBaseCell* ScCellIterator::GetFirst()
{
    if ( nTab > MAXTAB )
        return NULL;
    nCol = nStartCol;
    nRow = nStartRow;
    nTab = nStartTab;
//  nColRow = 0;
    ScColumn* pCol = &(pDoc->pTab[nTab])->aCol[nCol];
    pCol->Search( nRow, nColRow );
    return GetThis();
}

ScBaseCell* ScCellIterator::GetNext()
{
    ++nRow;
    return GetThis();
}

//-------------------------------------------------------------------------------

ScQueryCellIterator::ScQueryCellIterator(ScDocument* pDocument, USHORT nTable,
             const ScQueryParam& rParam, BOOL bMod ) :
    pDoc( pDocument ),
    nTab( nTable),
    aParam (rParam),
    bAdvanceQuery( FALSE )
{
    nCol = aParam.nCol1;
    nRow = aParam.nRow1;
    nColRow = 0;                    // wird bei GetFirst initialisiert
    USHORT i;
    if (bMod)                               // sonst schon eingetragen
    {
        for (i=0; (i<MAXQUERY) && (aParam.GetEntry(i).bDoQuery); i++)
        {
            ScQueryEntry& rEntry = aParam.GetEntry(i);
            ULONG nIndex = 0;
            rEntry.bQueryByString =
                     !(pDoc->GetFormatTable()->IsNumberFormat(*rEntry.pStr,
                                                              nIndex, rEntry.nVal));
        }
    }
    nNumFormat = 0;                 // werden bei GetNumberFormat initialisiert
    pAttrArray = 0;
    nAttrEndRow = 0;
}

ScBaseCell* ScQueryCellIterator::GetThis()
{
    ScColumn* pCol = &(pDoc->pTab[nTab])->aCol[nCol];
    for ( ;; )
    {
        if ( nRow > aParam.nRow2 )
        {
            nRow = aParam.nRow1;
            if (aParam.bHasHeader)
                nRow++;
            do
            {
                if ( bAdvanceQuery )
                    AdvanceQueryParamEntryField();
                if ( ++nCol > aParam.nCol2 )
                    return NULL;                // Ende und Aus
                pCol = &(pDoc->pTab[nTab])->aCol[nCol];
            } while ( pCol->nCount == 0 );
            pCol->Search( nRow, nColRow );
        }

        while ( nColRow < pCol->nCount && pCol->pItems[nColRow].nRow < nRow )
            nColRow++;

        if ( nColRow < pCol->nCount && pCol->pItems[nColRow].nRow <= aParam.nRow2 )
        {
            if ( pCol->pItems[nColRow].pCell->GetCellType() == CELLTYPE_NOTE )
                nRow++;
            else
            {
                nRow = pCol->pItems[nColRow].nRow;
                if ((pDoc->pTab[nTab])->ValidQuery(nRow, aParam))
                    return pCol->pItems[nColRow].pCell;     // gefunden
                else
                    nRow++;
            }
        }
        else
            nRow = aParam.nRow2 + 1; // Naechste Spalte
    }
}

ScBaseCell* ScQueryCellIterator::GetFirst()
{
    nCol = aParam.nCol1;
    nRow = aParam.nRow1;
    if (aParam.bHasHeader)
        nRow++;
//  nColRow = 0;
    ScColumn* pCol = &(pDoc->pTab[nTab])->aCol[nCol];
    pCol->Search( nRow, nColRow );
    return GetThis();
}

ScBaseCell* ScQueryCellIterator::GetNext()
{
    ++nRow;
    return GetThis();
}

ULONG ScQueryCellIterator::GetNumberFormat()
{
    ScColumn* pCol = &(pDoc->pTab[nTab])->aCol[nCol];
#if ! ( defined WTC || defined IRIX  || defined ICC || defined HPUX || defined C50 )
    lcl_IterGetNumberFormat( nNumFormat, pAttrArray,
#else
    lcl_IterGetNumberFormat( nNumFormat,
        (ScAttrArray const *&)pAttrArray,
#endif
        nAttrEndRow, pCol->pAttrArray, nRow, pDoc );
    return nNumFormat;
}

void ScQueryCellIterator::AdvanceQueryParamEntryField()
{
    USHORT nEntries = aParam.GetEntryCount();
    for ( USHORT j = 0; j < nEntries; j++  )
    {
        ScQueryEntry& rEntry = aParam.GetEntry( j );
        if ( rEntry.bDoQuery )
        {
            if ( rEntry.nField < MAXCOL )
                rEntry.nField++;
            else
            {
                DBG_ERRORFILE( "AdvanceQueryParamEntryField: ++rEntry.nField > MAXCOL" );
            }
        }
        else
            break;  // for
    }
}

//-------------------------------------------------------------------------------

ScHorizontalCellIterator::ScHorizontalCellIterator(ScDocument* pDocument, USHORT nTable,
                                    USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2 ) :
    pDoc( pDocument ),
    nTab( nTable ),
    nStartCol( nCol1 ),
    nEndCol( nCol2 ),
    nEndRow( nRow2 ),
    nCol( nCol1 ),
    nRow( nRow1 ),
    bMore( TRUE )
{
    USHORT i;
    USHORT nIndex;

    pNextRows = new USHORT[ nCol2-nCol1+1 ];
    pNextIndices = new USHORT[ nCol2-nCol1+1 ];

    for (i=nStartCol; i<=nEndCol; i++)
    {
        ScColumn* pCol = &pDoc->pTab[nTab]->aCol[i];

        pCol->Search( nRow1, nIndex );
        if ( nIndex < pCol->nCount )
        {
            pNextRows[i-nStartCol] = pCol->pItems[nIndex].nRow;
            pNextIndices[i-nStartCol] = nIndex;
        }
        else
        {
            pNextRows[i-nStartCol] = MAXROW+1;      // nichts gefunden
            pNextIndices[i-nStartCol] = MAXROW+1;
        }
    }

    if (pNextRows[0] != nRow1)
        Advance();
}

ScHorizontalCellIterator::~ScHorizontalCellIterator()
{
    delete [] pNextRows;
    delete [] pNextIndices;
}

ScBaseCell* ScHorizontalCellIterator::GetNext( USHORT& rCol, USHORT& rRow )
{
    if ( bMore )
    {
        rCol = nCol;
        rRow = nRow;

        ScColumn* pCol = &pDoc->pTab[nTab]->aCol[nCol];
        USHORT nIndex = pNextIndices[nCol-nStartCol];
        DBG_ASSERT( nIndex < pCol->nCount, "ScHorizontalCellIterator::GetNext: nIndex out of range" );
        ScBaseCell* pCell = pCol->pItems[nIndex].pCell;
        if ( ++nIndex < pCol->nCount )
        {
            pNextRows[nCol-nStartCol] = pCol->pItems[nIndex].nRow;
            pNextIndices[nCol-nStartCol] = nIndex;
        }
        else
        {
            pNextRows[nCol-nStartCol] = MAXROW+1;       // nichts gefunden
            pNextIndices[nCol-nStartCol] = MAXROW+1;
        }

        Advance();
        return pCell;
    }
    else
        return NULL;
}

BOOL ScHorizontalCellIterator::ReturnNext( USHORT& rCol, USHORT& rRow )
{
    rCol = nCol;
    rRow = nRow;
    return bMore;
}

void ScHorizontalCellIterator::Advance()
{
    BOOL bFound = FALSE;
    USHORT i;

    for (i=nCol+1; i<=nEndCol && !bFound; i++)
        if (pNextRows[i-nStartCol] == nRow)
        {
            nCol = i;
            bFound = TRUE;
        }

    if (!bFound)
    {
        USHORT nMinRow = MAXROW+1;
        for (i=nStartCol; i<=nEndCol; i++)
            if (pNextRows[i-nStartCol] < nMinRow)
            {
                nCol = i;
                nMinRow = pNextRows[i-nStartCol];
            }

        if (nMinRow <= nEndRow)
        {
            nRow = nMinRow;
            bFound = TRUE;
        }
    }

    if ( !bFound )
        bMore = FALSE;
}

//-------------------------------------------------------------------------------

ScHorizontalAttrIterator::ScHorizontalAttrIterator( ScDocument* pDocument, USHORT nTable,
                            USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2 ) :
    pDoc( pDocument ),
    nTab( nTable ),
    nStartCol( nCol1 ),
    nStartRow( nRow1 ),
    nEndCol( nCol2 ),
    nEndRow( nRow2 )
{
    DBG_ASSERT( pDoc->pTab[nTab], "Tabelle nicht da" );

    USHORT i;

    nRow = nStartRow;
    nCol = nStartCol;
    bRowEmpty = FALSE;

    pIndices    = new USHORT[nEndCol-nStartCol+1];
    pNextEnd    = new USHORT[nEndCol-nStartCol+1];
    ppPatterns  = new const ScPatternAttr*[nEndCol-nStartCol+1];

    USHORT nSkipTo = MAXROW;
    BOOL bEmpty = TRUE;
    for (i=nStartCol; i<=nEndCol; i++)
    {
        USHORT nPos = i - nStartCol;
        ScAttrArray* pArray = pDoc->pTab[nTab]->aCol[i].pAttrArray;
        DBG_ASSERT( pArray, "pArray == 0" );

        short s;
        pArray->Search( nStartRow, s );
        USHORT nIndex = (USHORT) s;

        const ScPatternAttr* pPattern = pArray->pData[nIndex].pPattern;
        USHORT nThisEnd = pArray->pData[nIndex].nRow;
        if ( IsDefaultItem( pPattern ) )
        {
            pPattern = NULL;
            if ( nThisEnd < nSkipTo )
                nSkipTo = nThisEnd;         // nSkipTo kann gleich hier gesetzt werden
        }
        else
            bEmpty = FALSE;                 // Attribute gefunden

        pIndices[nPos] = nIndex;
        pNextEnd[nPos] = nThisEnd;
        ppPatterns[nPos] = pPattern;
    }

    if (bEmpty)
        nRow = nSkipTo;                     // bis zum naechsten Bereichsende ueberspringen
    bRowEmpty = bEmpty;
}

ScHorizontalAttrIterator::~ScHorizontalAttrIterator()
{
    delete[] (ScPatternAttr**)ppPatterns;
    delete[] pNextEnd;
    delete[] pIndices;
}

const ScPatternAttr* ScHorizontalAttrIterator::GetNext( USHORT& rCol1, USHORT& rCol2, USHORT& rRow )
{
    for (;;)
    {
        if (!bRowEmpty)
        {
            // in dieser Zeile suchen

            while ( nCol <= nEndCol && !ppPatterns[nCol-nStartCol] )
                ++nCol;

            if ( nCol <= nEndCol )
            {
                const ScPatternAttr* pPat = ppPatterns[nCol-nStartCol];
                rRow = nRow;
                rCol1 = nCol;
                while ( nCol < nEndCol && ppPatterns[nCol+1-nStartCol] == pPat )
                    ++nCol;
                rCol2 = nCol;
                ++nCol;                 // hochzaehlen fuer naechsten Aufruf
                return pPat;            // gefunden
            }
        }

        // naechste Zeile

        ++nRow;
        if ( nRow > nEndRow )       // schon am Ende?
            return NULL;            // nichts gefunden

        BOOL bEmpty = TRUE;
        for (USHORT i=nStartCol; i<=nEndCol; i++)
        {
            USHORT nPos = i-nStartCol;
            if ( pNextEnd[nPos] < nRow )
            {
                ScAttrArray* pArray = pDoc->pTab[nTab]->aCol[i].pAttrArray;

                USHORT nIndex = ++pIndices[nPos];
                if ( nIndex < pArray->nCount )
                {
                    const ScPatternAttr* pPattern = pArray->pData[nIndex].pPattern;
                    USHORT nThisEnd = pArray->pData[nIndex].nRow;
                    if ( IsDefaultItem( pPattern ) )
                        pPattern = NULL;
                    else
                        bEmpty = FALSE;                 // Attribute gefunden

                    pNextEnd[nPos] = nThisEnd;
                    ppPatterns[nPos] = pPattern;

                    DBG_ASSERT( pNextEnd[nPos] >= nRow, "Reihenfolge durcheinander" );
                }
                else
                {
                    DBG_ERROR("AttrArray reicht nicht bis MAXROW");
                    pNextEnd[nPos] = MAXROW;
                    ppPatterns[nPos] = NULL;
                }
            }
            else if ( ppPatterns[nPos] )
                bEmpty = FALSE;                         // Bereich noch nicht zuende
        }

        if (bEmpty)
        {
            USHORT nCount = nEndCol-nStartCol+1;
            USHORT nSkipTo = pNextEnd[0];               // naechstes Bereichsende suchen
            for (i=1; i<nCount; i++)
                if ( pNextEnd[i] < nSkipTo )
                    nSkipTo = pNextEnd[i];
            nRow = nSkipTo;                             // leere Zeilen ueberspringen
        }
        bRowEmpty = bEmpty;
        nCol = nStartCol;           // wieder links anfangen
    }

    return NULL;
}

//-------------------------------------------------------------------------------

inline BOOL IsGreater( USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2 )
{
    return ( nRow1 > nRow2 ) || ( nRow1 == nRow2 && nCol1 > nCol2 );
}

ScUsedAreaIterator::ScUsedAreaIterator( ScDocument* pDocument, USHORT nTable,
                            USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2 ) :
    aCellIter( pDocument, nTable, nCol1, nRow1, nCol2, nRow2 ),
    aAttrIter( pDocument, nTable, nCol1, nRow1, nCol2, nRow2 ),
    nNextCol( nCol1 ),
    nNextRow( nRow1 )
{
    pCell    = aCellIter.GetNext( nCellCol, nCellRow );
    pPattern = aAttrIter.GetNext( nAttrCol1, nAttrCol2, nAttrRow );
}

ScUsedAreaIterator::~ScUsedAreaIterator()
{
}

BOOL ScUsedAreaIterator::GetNext()
{
    //  Iteratoren weiterzaehlen

    if ( pCell && IsGreater( nNextCol, nNextRow, nCellCol, nCellRow ) )
        pCell = aCellIter.GetNext( nCellCol, nCellRow );

    while ( pCell && pCell->GetCellType() == CELLTYPE_NOTE && !pCell->GetNotePtr() )
        pCell = aCellIter.GetNext( nCellCol, nCellRow );

    if ( pPattern && IsGreater( nNextCol, nNextRow, nAttrCol2, nAttrRow ) )
        pPattern = aAttrIter.GetNext( nAttrCol1, nAttrCol2, nAttrRow );

    if ( pPattern && nAttrRow == nNextRow && nAttrCol1 < nNextCol )
        nAttrCol1 = nNextCol;

    //  naechsten Abschnitt heraussuchen

    BOOL bFound = TRUE;
    BOOL bUseCell = FALSE;

    if ( pCell && pPattern )
    {
        if ( IsGreater( nCellCol, nCellRow, nAttrCol1, nAttrRow ) )     // vorne nur Attribute ?
        {
            pFoundCell = NULL;
            pFoundPattern = pPattern;
            nFoundRow = nAttrRow;
            nFoundStartCol = nAttrCol1;
            if ( nCellRow == nAttrRow && nCellCol <= nAttrCol2 )        // auch Zelle im Bereich ?
                nFoundEndCol = nCellCol - 1;                            // nur bis vor der Zelle
            else
                nFoundEndCol = nAttrCol2;                               // alles
        }
        else
        {
            bUseCell = TRUE;
            if ( nAttrRow == nCellRow && nAttrCol1 == nCellCol )        // Attribute auf der Zelle ?
                pFoundPattern = pPattern;
            else
                pFoundPattern = NULL;
        }
    }
    else if ( pCell )                   // nur Zelle -> direkt uebernehmen
    {
        pFoundPattern = NULL;
        bUseCell = TRUE;                // Position von Zelle
    }
    else if ( pPattern )                // nur Attribute -> direkt uebernehmen
    {
        pFoundCell = NULL;
        pFoundPattern = pPattern;
        nFoundRow = nAttrRow;
        nFoundStartCol = nAttrCol1;
        nFoundEndCol = nAttrCol2;
    }
    else                                // gar nichts
        bFound = FALSE;

    if ( bUseCell )                     // Position von Zelle
    {
        pFoundCell = pCell;
        nFoundRow = nCellRow;
        nFoundStartCol = nFoundEndCol = nCellCol;
    }

    if (bFound)
    {
        nNextRow = nFoundRow;
        nNextCol = nFoundEndCol + 1;
    }

    return bFound;
}

//-------------------------------------------------------------------------------

ScDocAttrIterator::ScDocAttrIterator(ScDocument* pDocument, USHORT nTable,
                                    USHORT nCol1, USHORT nRow1,
                                    USHORT nCol2, USHORT nRow2) :
    pDoc( pDocument ),
    nTab( nTable ),
    nCol( nCol1 ),
    nEndCol( nCol2 ),
    nStartRow( nRow1 ),
    nEndRow( nRow2 )
{
    if ( nTab<=MAXTAB && pDoc->pTab[nTab] )
        pColIter = pDoc->pTab[nTab]->aCol[nCol].CreateAttrIterator( nStartRow, nEndRow );
    else
        pColIter = NULL;
}

ScDocAttrIterator::~ScDocAttrIterator()
{
    delete pColIter;
}

const ScPatternAttr* ScDocAttrIterator::GetNext( USHORT& rCol, USHORT& rRow1, USHORT& rRow2 )
{
    while ( pColIter )
    {
        const ScPatternAttr* pPattern = pColIter->Next( rRow1, rRow2 );
        if ( pPattern )
        {
            rCol = nCol;
            return pPattern;
        }

        delete pColIter;
        ++nCol;
        if ( nCol <= nEndCol )
            pColIter = pDoc->pTab[nTab]->aCol[nCol].CreateAttrIterator( nStartRow, nEndRow );
        else
            pColIter = NULL;
    }
    return NULL;        // is nix mehr
}

//-------------------------------------------------------------------------------

ScAttrRectIterator::ScAttrRectIterator(ScDocument* pDocument, USHORT nTable,
                                    USHORT nCol1, USHORT nRow1,
                                    USHORT nCol2, USHORT nRow2) :
    pDoc( pDocument ),
    nTab( nTable ),
    nEndCol( nCol2 ),
    nStartRow( nRow1 ),
    nEndRow( nRow2 ),
    nIterStartCol( nCol1 ),
    nIterEndCol( nCol1 )
{
    if ( nTab<=MAXTAB && pDoc->pTab[nTab] )
    {
        pColIter = pDoc->pTab[nTab]->aCol[nIterStartCol].CreateAttrIterator( nStartRow, nEndRow );
        while ( nIterEndCol < nEndCol &&
                pDoc->pTab[nTab]->aCol[nIterEndCol].IsAllAttrEqual(
                    pDoc->pTab[nTab]->aCol[nIterEndCol+1], nStartRow, nEndRow ) )
            ++nIterEndCol;
    }
    else
        pColIter = NULL;
}

ScAttrRectIterator::~ScAttrRectIterator()
{
    delete pColIter;
}

void ScAttrRectIterator::DataChanged()
{
    if (pColIter)
    {
        USHORT nNextRow = pColIter->GetNextRow();
        delete pColIter;
        pColIter = pDoc->pTab[nTab]->aCol[nIterStartCol].CreateAttrIterator( nNextRow, nEndRow );
    }
}

const ScPatternAttr* ScAttrRectIterator::GetNext( USHORT& rCol1, USHORT& rCol2,
                                                    USHORT& rRow1, USHORT& rRow2 )
{
    while ( pColIter )
    {
        const ScPatternAttr* pPattern = pColIter->Next( rRow1, rRow2 );
        if ( pPattern )
        {
            rCol1 = nIterStartCol;
            rCol2 = nIterEndCol;
            return pPattern;
        }

        delete pColIter;
        nIterStartCol = nIterEndCol+1;
        if ( nIterStartCol <= nEndCol )
        {
            nIterEndCol = nIterStartCol;
            pColIter = pDoc->pTab[nTab]->aCol[nIterStartCol].CreateAttrIterator( nStartRow, nEndRow );
            while ( nIterEndCol < nEndCol &&
                    pDoc->pTab[nTab]->aCol[nIterEndCol].IsAllAttrEqual(
                        pDoc->pTab[nTab]->aCol[nIterEndCol+1], nStartRow, nEndRow ) )
                ++nIterEndCol;
        }
        else
            pColIter = NULL;
    }
    return NULL;        // is nix mehr
}

