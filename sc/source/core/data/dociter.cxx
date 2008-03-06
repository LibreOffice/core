/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dociter.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 15:25:36 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

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
#include "cellform.hxx"


// STATIC DATA -----------------------------------------------------------

ScDocumentIterator::ScDocumentIterator( ScDocument* pDocument,
                            SCTAB nStartTable, SCTAB nEndTable ) :
    pDoc( pDocument ),
    nStartTab( nStartTable ),
    nEndTab( nEndTable )
{
    PutInOrder( nStartTab, nEndTab );
    if (!ValidTab(nStartTab)) nStartTab = MAXTAB;
    if (!ValidTab(nEndTab)) nEndTab = MAXTAB;

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
        SCROW nColRow;
        SCROW nAttrEnd;

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
            nRow = Min( (SCROW)nColRow, (SCROW)(nAttrEnd+1) );
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

void ScDocumentIterator::GetPos( SCCOL& rCol, SCROW& rRow, SCTAB& rTab )
{
    rCol = nCol;
    rRow = nRow;
    rTab = nTab;
}


//------------------------------------------------------------------------
//------------------------------------------------------------------------
void lcl_IterGetNumberFormat( ULONG& nFormat, const ScAttrArray*& rpArr,
        SCROW& nAttrEndRow, const ScAttrArray* pNewArr, SCROW nRow,
        ScDocument* pDoc )
{
    if ( rpArr != pNewArr || nAttrEndRow < nRow )
    {
        SCSIZE nPos;
        pNewArr->Search( nRow, nPos );  // nPos 0 gueltig wenn nicht gefunden
        const ScPatternAttr* pPattern = pNewArr->pData[nPos].pPattern;
        nFormat = pPattern->GetNumberFormat( pDoc->GetFormatTable() );
        rpArr = pNewArr;
        nAttrEndRow = pNewArr->pData[nPos].nRow;
    }
}

ScValueIterator::ScValueIterator( ScDocument* pDocument,
                                    SCCOL nSCol, SCROW nSRow, SCTAB nSTab,
                                    SCCOL nECol, SCROW nERow, SCTAB nETab,
                                    BOOL bSTotal, BOOL bTextZero ) :
    pDoc( pDocument ),
    nNumFmtIndex(0),
    nStartCol( nSCol),
    nStartRow( nSRow),
    nStartTab( nSTab ),
    nEndCol( nECol ),
    nEndRow( nERow),
    nEndTab( nETab ),
    nNumFmtType( NUMBERFORMAT_UNDEFINED ),
    bNumValid( FALSE ),
    bSubTotal(bSTotal),
    bNextValid( FALSE ),
    bCalcAsShown( pDocument->GetDocOptions().IsCalcAsShown() ),
    bTextAsZero( bTextZero )
{
    PutInOrder( nStartCol, nEndCol);
    PutInOrder( nStartRow, nEndRow);
    PutInOrder( nStartTab, nEndTab );

    if (!ValidCol(nStartCol)) nStartCol = MAXCOL;
    if (!ValidCol(nEndCol)) nEndCol = MAXCOL;
    if (!ValidRow(nStartRow)) nStartRow = MAXROW;
    if (!ValidRow(nEndRow)) nEndRow = MAXROW;
    if (!ValidTab(nStartTab)) nStartTab = MAXTAB;
    if (!ValidTab(nEndTab)) nEndTab = MAXTAB;

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
    nNumFmtIndex(0),
    nStartCol( rRange.aStart.Col() ),
    nStartRow( rRange.aStart.Row() ),
    nStartTab( rRange.aStart.Tab() ),
    nEndCol( rRange.aEnd.Col() ),
    nEndRow( rRange.aEnd.Row() ),
    nEndTab( rRange.aEnd.Tab() ),
    nNumFmtType( NUMBERFORMAT_UNDEFINED ),
    bNumValid( FALSE ),
    bSubTotal(bSTotal),
    bNextValid( FALSE ),
    bCalcAsShown( pDocument->GetDocOptions().IsCalcAsShown() ),
    bTextAsZero( bTextZero )
{
    PutInOrder( nStartCol, nEndCol);
    PutInOrder( nStartRow, nEndRow);
    PutInOrder( nStartTab, nEndTab );

    if (!ValidCol(nStartCol)) nStartCol = MAXCOL;
    if (!ValidCol(nEndCol)) nEndCol = MAXCOL;
    if (!ValidRow(nStartRow)) nStartRow = MAXROW;
    if (!ValidRow(nEndRow)) nEndRow = MAXROW;
    if (!ValidTab(nStartTab)) nStartTab = MAXTAB;
    if (!ValidTab(nEndTab)) nEndTab = MAXTAB;

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
                        // rValue = 0.0;    //! do not change caller's value!
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
                            lcl_IterGetNumberFormat( nNumFormat, pAttrArray,
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
                                lcl_IterGetNumberFormat( nNumFormat, pAttrArray,
                                    nAttrEndRow, pCol->pAttrArray, nNextRow, pDoc );
                                fNextValue = pDoc->RoundValueAsShown( fNextValue, nNumFormat );
                            }
                        }

                        return TRUE;                                    // gefunden
                    }
//                    break;
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
                    default:
                    {
                        // added to avoid warnings
                    }
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
            SCSIZE nIdx = nColRow - 1;
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

ScQueryValueIterator::ScQueryValueIterator(ScDocument* pDocument, SCTAB nTable, const ScQueryParam& rParam) :
    aParam (rParam),
    pDoc( pDocument ),
    nNumFmtIndex(0),
    nTab( nTable),
    nNumFmtType( NUMBERFORMAT_UNDEFINED ),
    bCalcAsShown( pDocument->GetDocOptions().IsCalcAsShown() )
{
    nCol = aParam.nCol1;
    nRow = aParam.nRow1;
    nColRow = 0;                    // wird bei GetFirst initialisiert
    SCSIZE i;
    SCSIZE nCount = aParam.GetEntryCount();
    for (i=0; (i<nCount) && (aParam.GetEntry(i).bDoQuery); i++)
    {
        ScQueryEntry& rEntry = aParam.GetEntry(i);
        sal_uInt32 nIndex = 0;
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
    SCCOLROW nFirstQueryField = aParam.GetEntry(0).nField;
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
                    // rValue = 0.0;    // do not change caller's value!
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
            ScBaseCell* pCell = pCol->pItems[nColRow].pCell;
            if ( (pDoc->pTab[nTab])->ValidQuery( nRow, aParam, NULL,
                    (nCol == static_cast<SCCOL>(nFirstQueryField) ? pCell : NULL) ) )
            {
                switch (pCell->GetCellType())
                {
                    case CELLTYPE_VALUE:
                        {
                            rValue = ((ScValueCell*)pCell)->GetValue();
                            if ( bCalcAsShown )
                            {
                                lcl_IterGetNumberFormat( nNumFormat, pAttrArray,
                                    nAttrEndRow, pCol->pAttrArray, nRow, pDoc );
                                rValue = pDoc->RoundValueAsShown( rValue, nNumFormat );
                            }
                            nNumFmtType = NUMBERFORMAT_NUMBER;
                            nNumFmtIndex = 0;
                            rErr = 0;
                            return TRUE;        // gefunden
                        }
//                        break;
                    case CELLTYPE_FORMULA:
                        {
                            if (((ScFormulaCell*)pCell)->IsValue())
                            {
                                rValue = ((ScFormulaCell*)pCell)->GetValue();
                                pDoc->GetNumberFormatInfo( nNumFmtType,
                                    nNumFmtIndex, ScAddress( nCol, nRow, nTab ),
                                    pCell );
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
//    return FALSE;
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
                                SCCOL nSCol, SCROW nSRow, SCTAB nSTab,
                                SCCOL nECol, SCROW nERow, SCTAB nETab, BOOL bSTotal ) :
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

    if (!ValidCol(nStartCol)) nStartCol = MAXCOL;
    if (!ValidCol(nEndCol)) nEndCol = MAXCOL;
    if (!ValidRow(nStartRow)) nStartRow = MAXROW;
    if (!ValidRow(nEndRow)) nEndRow = MAXROW;
    if (!ValidTab(nStartTab)) nStartTab = MAXTAB;
    if (!ValidTab(nEndTab)) nEndTab = MAXTAB;

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

    if (!ValidCol(nStartCol)) nStartCol = MAXCOL;
    if (!ValidCol(nEndCol)) nEndCol = MAXCOL;
    if (!ValidRow(nStartRow)) nStartRow = MAXROW;
    if (!ValidRow(nEndRow)) nEndRow = MAXROW;
    if (!ValidTab(nStartTab)) nStartTab = MAXTAB;
    if (!ValidTab(nEndTab)) nEndTab = MAXTAB;

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
    if ( !ValidTab(nTab) )
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

ScQueryCellIterator::ScQueryCellIterator(ScDocument* pDocument, SCTAB nTable,
             const ScQueryParam& rParam, BOOL bMod ) :
    aParam (rParam),
    pDoc( pDocument ),
    nTab( nTable),
    nStopOnMismatch( nStopOnMismatchDisabled ),
    nTestEqualCondition( nTestEqualConditionDisabled ),
    bAdvanceQuery( FALSE ),
    bIgnoreMismatchOnLeadingStrings( FALSE )
{
    nCol = aParam.nCol1;
    nRow = aParam.nRow1;
    nColRow = 0;                    // wird bei GetFirst initialisiert
    SCSIZE i;
    if (bMod)                               // sonst schon eingetragen
    {
        for (i=0; (i<MAXQUERY) && (aParam.GetEntry(i).bDoQuery); i++)
        {
            ScQueryEntry& rEntry = aParam.GetEntry(i);
            sal_uInt32 nIndex = 0;
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
    const ScQueryEntry& rEntry = aParam.GetEntry(0);
    SCCOLROW nFirstQueryField = rEntry.nField;
    bool bAllStringIgnore = bIgnoreMismatchOnLeadingStrings &&
        !rEntry.bQueryByString;
    bool bFirstStringIgnore = bIgnoreMismatchOnLeadingStrings &&
        !aParam.bHasHeader && rEntry.bQueryByString &&
        ((aParam.bByRow && nRow == aParam.nRow1) ||
         (!aParam.bByRow && nCol == aParam.nCol1));
    for ( ;; )
    {
        if ( nRow > aParam.nRow2 )
        {
            nRow = aParam.nRow1;
            if (aParam.bHasHeader && aParam.bByRow)
                nRow++;
            do
            {
                if ( ++nCol > aParam.nCol2 )
                    return NULL;                // Ende und Aus
                if ( bAdvanceQuery )
                {
                    AdvanceQueryParamEntryField();
                    nFirstQueryField = rEntry.nField;
                }
                pCol = &(pDoc->pTab[nTab])->aCol[nCol];
            } while ( pCol->nCount == 0 );
            pCol->Search( nRow, nColRow );
            bFirstStringIgnore = bIgnoreMismatchOnLeadingStrings &&
                !aParam.bHasHeader && rEntry.bQueryByString &&
                aParam.bByRow;
        }

        while ( nColRow < pCol->nCount && pCol->pItems[nColRow].nRow < nRow )
            nColRow++;

        if ( nColRow < pCol->nCount &&
                (nRow = pCol->pItems[nColRow].nRow) <= aParam.nRow2 )
        {
            ScBaseCell* pCell = pCol->pItems[nColRow].pCell;
            if ( pCell->GetCellType() == CELLTYPE_NOTE )
                ++nRow;
            else if (bAllStringIgnore && pCell->HasStringData())
                ++nRow;
            else
            {
                BOOL bTestEqualCondition;
                if ( (pDoc->pTab[nTab])->ValidQuery( nRow, aParam, NULL,
                        (nCol == static_cast<SCCOL>(nFirstQueryField) ? pCell : NULL),
                        (nTestEqualCondition ? &bTestEqualCondition : NULL) ) )
                {
                    if ( nTestEqualCondition && bTestEqualCondition )
                        nTestEqualCondition |= nTestEqualConditionMatched;
                    return pCell;     // found
                }
                else if ( nStopOnMismatch )
                {
                    // Yes, even a mismatch may have a fulfilled equal
                    // condition if regular expressions were involved and
                    // SC_LESS_EQUAL or SC_GREATER_EQUAL were queried.
                    if ( nTestEqualCondition && bTestEqualCondition )
                    {
                        nTestEqualCondition |= nTestEqualConditionMatched;
                        nStopOnMismatch |= nStopOnMismatchOccured;
                        return NULL;
                    }
                    bool bStop;
                    if (bFirstStringIgnore)
                    {
                        if (pCell->HasStringData())
                        {
                            ++nRow;
                            bStop = false;
                        }
                        else
                            bStop = true;
                    }
                    else
                        bStop = true;
                    if (bStop)
                    {
                        nStopOnMismatch |= nStopOnMismatchOccured;
                        return NULL;
                    }
                }
                else
                    nRow++;
            }
        }
        else
            nRow = aParam.nRow2 + 1; // Naechste Spalte
        bFirstStringIgnore = false;
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
    if ( nStopOnMismatch )
        nStopOnMismatch = nStopOnMismatchEnabled;
    if ( nTestEqualCondition )
        nTestEqualCondition = nTestEqualConditionEnabled;
    return GetThis();
}

ULONG ScQueryCellIterator::GetNumberFormat()
{
    ScColumn* pCol = &(pDoc->pTab[nTab])->aCol[nCol];
    lcl_IterGetNumberFormat( nNumFormat, pAttrArray,
        nAttrEndRow, pCol->pAttrArray, nRow, pDoc );
    return nNumFormat;
}

void ScQueryCellIterator::AdvanceQueryParamEntryField()
{
    SCSIZE nEntries = aParam.GetEntryCount();
    for ( SCSIZE j = 0; j < nEntries; j++  )
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


BOOL ScQueryCellIterator::FindEqualOrSortedLastInRange( SCCOL& nFoundCol,
        SCROW& nFoundRow, BOOL bSearchForEqualAfterMismatch,
        BOOL bIgnoreMismatchOnLeadingStringsP )
{
    nFoundCol = MAXCOL+1;
    nFoundRow = MAXROW+1;
    SetStopOnMismatch( TRUE );      // assume sorted keys
    SetTestEqualCondition( TRUE );
    bIgnoreMismatchOnLeadingStrings = bIgnoreMismatchOnLeadingStringsP;
    bool bRegExp = aParam.bRegExp && aParam.GetEntry(0).bQueryByString;
    bool bBinary = !bRegExp && aParam.bByRow && (aParam.GetEntry(0).eOp ==
            SC_LESS_EQUAL || aParam.GetEntry(0).eOp == SC_GREATER_EQUAL);
    if (bBinary ? (BinarySearch() ? GetThis() : 0) : GetFirst())
    {
        // First equal entry or last smaller than (greater than) entry.
        SCSIZE nColRowSave;
        ScBaseCell* pNext = 0;
        do
        {
            nFoundCol = GetCol();
            nFoundRow = GetRow();
            nColRowSave = nColRow;
        } while ( !IsEqualConditionFulfilled() && (pNext = GetNext()) != NULL );
        // There may be no pNext but equal condition fulfilled if regular
        // expressions are involved. Keep the found entry and proceed.
        if (!pNext && !IsEqualConditionFulfilled())
        {
            // Step back to last in range and adjust position markers for
            // GetNumberFormat() or similar.
            nCol = nFoundCol;
            nRow = nFoundRow;
            nColRow = nColRowSave;
        }
    }
    if ( IsEqualConditionFulfilled() )
    {
        // Position on last equal entry.
        SCSIZE nEntries = aParam.GetEntryCount();
        for ( SCSIZE j = 0; j < nEntries; j++  )
        {
            ScQueryEntry& rEntry = aParam.GetEntry( j );
            if ( rEntry.bDoQuery )
            {
                switch ( rEntry.eOp )
                {
                    case SC_LESS_EQUAL :
                    case SC_GREATER_EQUAL :
                        rEntry.eOp = SC_EQUAL;
                    break;
                    default:
                    {
                        // added to avoid warnings
                    }
                }
            }
            else
                break;  // for
        }
        SCSIZE nColRowSave;
        bIgnoreMismatchOnLeadingStrings = FALSE;
        SetTestEqualCondition( FALSE );
        do
        {
            nFoundCol = GetCol();
            nFoundRow = GetRow();
            nColRowSave = nColRow;
        } while (GetNext());
        // Step back conditions same as above
        nCol = nFoundCol;
        nRow = nFoundRow;
        nColRow = nColRowSave;
        return TRUE;
    }
    if ( (bSearchForEqualAfterMismatch || aParam.bRegExp) &&
            StoppedOnMismatch() )
    {
        // Assume found entry to be the last value less than respectively
        // greater than the query. But keep on searching for an equal match.
        SCSIZE nEntries = aParam.GetEntryCount();
        for ( SCSIZE j = 0; j < nEntries; j++  )
        {
            ScQueryEntry& rEntry = aParam.GetEntry( j );
            if ( rEntry.bDoQuery )
            {
                switch ( rEntry.eOp )
                {
                    case SC_LESS_EQUAL :
                    case SC_GREATER_EQUAL :
                        rEntry.eOp = SC_EQUAL;
                    break;
                    default:
                    {
                        // added to avoid warnings
                    }
                }
            }
            else
                break;  // for
        }
        SetStopOnMismatch( FALSE );
        SetTestEqualCondition( FALSE );
        if (GetNext())
        {
            // Last of a consecutive area, avoid searching the entire parameter
            // range as it is a real performance bottleneck in case of regular
            // expressions.
            SCSIZE nColRowSave;
            do
            {
                nFoundCol = GetCol();
                nFoundRow = GetRow();
                nColRowSave = nColRow;
                SetStopOnMismatch( TRUE );
            } while (GetNext());
            nCol = nFoundCol;
            nRow = nFoundRow;
            nColRow = nColRowSave;
        }
    }
    return (nFoundCol <= MAXCOL) && (nFoundRow <= MAXROW);
}


ScBaseCell* ScQueryCellIterator::BinarySearch()
{
    nCol = aParam.nCol1;
    ScColumn* pCol = &(pDoc->pTab[nTab])->aCol[nCol];
    if (!pCol->nCount)
        return 0;

    ScBaseCell* pCell;
    SCSIZE nHi, nLo;
    CollatorWrapper* pCollator = (aParam.bCaseSens ? ScGlobal::pCaseCollator :
        ScGlobal::pCollator);
    SvNumberFormatter& rFormatter = *(pDoc->GetFormatTable());
    const ScQueryEntry& rEntry = aParam.GetEntry(0);
    bool bLessEqual = rEntry.eOp == SC_LESS_EQUAL;
    bool bByString = rEntry.bQueryByString;
    bool bAllStringIgnore = bIgnoreMismatchOnLeadingStrings && !bByString;
    bool bFirstStringIgnore = bIgnoreMismatchOnLeadingStrings &&
        !aParam.bHasHeader && bByString;

    nRow = aParam.nRow1;
    if (aParam.bHasHeader)
        nRow++;
    const ColEntry* pItems = pCol->pItems;
    if (pCol->Search( nRow, nLo ) && bFirstStringIgnore &&
            pItems[nLo].pCell->HasStringData())
    {
        String aCellStr;
        ULONG nFormat = pCol->GetNumberFormat( pItems[nLo].nRow);
        ScCellFormat::GetInputString( pItems[nLo].pCell, nFormat, aCellStr,
                rFormatter);
        sal_Int32 nTmp = pCollator->compareString( aCellStr, *rEntry.pStr);
        if ((rEntry.eOp == SC_LESS_EQUAL && nTmp > 0) ||
                (rEntry.eOp == SC_GREATER_EQUAL && nTmp < 0) ||
                (rEntry.eOp == SC_EQUAL && nTmp != 0))
            ++nLo;
    }
    if (!pCol->Search( aParam.nRow2, nHi ) && nHi>0)
        --nHi;
    while (bAllStringIgnore && nLo <= nHi && nLo < pCol->nCount &&
            pItems[nLo].pCell->HasStringData())
        ++nLo;

    // Bookkeeping values for breaking up the binary search in case the data
    // range isn't strictly sorted.
    SCSIZE nLastInRange = nLo;
    SCSIZE nFirstLastInRange = nLastInRange;
    double fLastInRangeValue = bLessEqual ?
        -(::std::numeric_limits<double>::max()) :
            ::std::numeric_limits<double>::max();
    String aLastInRangeString;
    if (!bLessEqual)
        aLastInRangeString.Assign( sal_Unicode(0xFFFF));
    if (nLastInRange < pCol->nCount)
    {
        pCell = pItems[nLastInRange].pCell;
        if (pCell->HasStringData())
        {
            ULONG nFormat = pCol->GetNumberFormat( pItems[nLastInRange].nRow);
            ScCellFormat::GetInputString( pCell, nFormat, aLastInRangeString,
                    rFormatter);
        }
        else
        {
            switch ( pCell->GetCellType() )
            {
                case CELLTYPE_VALUE :
                    fLastInRangeValue =
                        static_cast<ScValueCell*>(pCell)->GetValue();
                    break;
                case CELLTYPE_FORMULA :
                    fLastInRangeValue =
                        static_cast<ScFormulaCell*>(pCell)->GetValue();
                    break;
                default:
                {
                    // added to avoid warnings
                }
            }
        }
    }

    sal_Int32 nRes = 0;
    bool bFound = false;
    bool bDone = false;
    while (nLo <= nHi && !bDone)
    {
        SCSIZE nMid = (nLo+nHi)/2;
        SCSIZE i = nMid;
        while (i <= nHi && pItems[i].pCell->GetCellType() == CELLTYPE_NOTE)
            ++i;
        if (i > nHi)
        {
            if (nMid > 0)
                nHi = nMid - 1;
            else
                bDone = true;
            continue;   // while
        }
        BOOL bStr = pItems[i].pCell->HasStringData();
        nRes = 0;
        // compares are content<query:-1, content>query:1
        // Cell value comparison similar to ScTable::ValidQuery()
        if (!bStr && !bByString)
        {
            double nCellVal;
            pCell = pItems[i].pCell;
            switch ( pCell->GetCellType() )
            {
                case CELLTYPE_VALUE :
                    nCellVal = static_cast<ScValueCell*>(pCell)->GetValue();
                    break;
                case CELLTYPE_FORMULA :
                    nCellVal = static_cast<ScFormulaCell*>(pCell)->GetValue();
                    break;
                default:
                    nCellVal = 0.0;
            }
            if ((nCellVal < rEntry.nVal) && !::rtl::math::approxEqual(
                        nCellVal, rEntry.nVal))
            {
                nRes = -1;
                if (bLessEqual)
                {
                    if (fLastInRangeValue < nCellVal)
                    {
                        fLastInRangeValue = nCellVal;
                        nLastInRange = i;
                    }
                    else if (fLastInRangeValue > nCellVal)
                    {
                        // not strictly sorted, continue with GetThis()
                        nLastInRange = nFirstLastInRange;
                        bDone = true;
                    }
                }
            }
            else if ((nCellVal > rEntry.nVal) && !::rtl::math::approxEqual(
                        nCellVal, rEntry.nVal))
            {
                nRes = 1;
                if (!bLessEqual)
                {
                    if (fLastInRangeValue > nCellVal)
                    {
                        fLastInRangeValue = nCellVal;
                        nLastInRange = i;
                    }
                    else if (fLastInRangeValue < nCellVal)
                    {
                        // not strictly sorted, continue with GetThis()
                        nLastInRange = nFirstLastInRange;
                        bDone = true;
                    }
                }
            }
        }
        else if (bStr && bByString)
        {
            String aCellStr;
            ULONG nFormat = pCol->GetNumberFormat( pItems[i].nRow);
            ScCellFormat::GetInputString( pItems[i].pCell, nFormat, aCellStr,
                    rFormatter);
            nRes = pCollator->compareString( aCellStr, *rEntry.pStr);
            if (nRes < 0 && bLessEqual)
            {
                sal_Int32 nTmp = pCollator->compareString( aLastInRangeString,
                        aCellStr);
                if (nTmp < 0)
                {
                    aLastInRangeString = aCellStr;
                    nLastInRange = i;
                }
                else if (nTmp > 0)
                {
                    // not strictly sorted, continue with GetThis()
                    nLastInRange = nFirstLastInRange;
                    bDone = true;
                }
            }
            else if (nRes > 0 && !bLessEqual)
            {
                sal_Int32 nTmp = pCollator->compareString( aLastInRangeString,
                        aCellStr);
                if (nTmp > 0)
                {
                    aLastInRangeString = aCellStr;
                    nLastInRange = i;
                }
                else if (nTmp < 0)
                {
                    // not strictly sorted, continue with GetThis()
                    nLastInRange = nFirstLastInRange;
                    bDone = true;
                }
            }
        }
        else if (!bStr && bByString)
        {
            nRes = -1;  // numeric < string
            if (bLessEqual)
                nLastInRange = i;
        }
        else // if (bStr && !bByString)
        {
            nRes = 1;   // string > numeric
            if (!bLessEqual)
                nLastInRange = i;
        }
        if (nRes < 0)
        {
            if (bLessEqual)
                nLo = nMid + 1;
            else    // assumed to be SC_GREATER_EQUAL
            {
                if (nMid > 0)
                    nHi = nMid - 1;
                else
                    bDone = true;
            }
        }
        else if (nRes > 0)
        {
            if (bLessEqual)
            {
                if (nMid > 0)
                    nHi = nMid - 1;
                else
                    bDone = true;
            }
            else    // assumed to be SC_GREATER_EQUAL
                nLo = nMid + 1;
        }
        else
        {
            nLo = i;
            bDone = bFound = true;
        }
    }
    if (!bFound)
    {
        // If all hits didn't result in a moving limit there's something
        // strange, e.g. data range not properly sorted, or only identical
        // values encountered, which doesn't mean there aren't any others in
        // between.. leave it to GetThis(). The condition for this would be
        // if (nLastInRange == nFirstLastInRange) nLo = nFirstLastInRange;
        // Else, in case no exact match was found, we step back for a
        // subsequent GetThis() to find the last in range. Effectively this is
        // --nLo with nLastInRange == nLo-1. Both conditions combined yield:
        nLo = nLastInRange;
    }
    if (nLo < pCol->nCount && pCol->pItems[nLo].nRow <= aParam.nRow2)
    {
        nRow = pItems[nLo].nRow;
        pCell = pItems[nLo].pCell;
        nColRow = nLo;
    }
    else
    {
        nRow = aParam.nRow2 + 1;
        pCell = 0;
        nColRow = pCol->nCount - 1;
    }
    return pCell;
}


//-------------------------------------------------------------------------------

ScHorizontalCellIterator::ScHorizontalCellIterator(ScDocument* pDocument, SCTAB nTable,
                                    SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 ) :
    pDoc( pDocument ),
    nTab( nTable ),
    nStartCol( nCol1 ),
    nEndCol( nCol2 ),
    nEndRow( nRow2 ),
    nCol( nCol1 ),
    nRow( nRow1 ),
    bMore( TRUE )
{
    SCCOL i;
    SCSIZE nIndex;

    pNextRows = new SCROW[ nCol2-nCol1+1 ];
    pNextIndices = new SCSIZE[ nCol2-nCol1+1 ];

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
            pNextRows[i-nStartCol] = MAXROWCOUNT;       // nichts gefunden
            pNextIndices[i-nStartCol] = MAXROWCOUNT;
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

ScBaseCell* ScHorizontalCellIterator::GetNext( SCCOL& rCol, SCROW& rRow )
{
    if ( bMore )
    {
        rCol = nCol;
        rRow = nRow;

        ScColumn* pCol = &pDoc->pTab[nTab]->aCol[nCol];
        SCSIZE nIndex = pNextIndices[nCol-nStartCol];
        DBG_ASSERT( nIndex < pCol->nCount, "ScHorizontalCellIterator::GetNext: nIndex out of range" );
        ScBaseCell* pCell = pCol->pItems[nIndex].pCell;
        if ( ++nIndex < pCol->nCount )
        {
            pNextRows[nCol-nStartCol] = pCol->pItems[nIndex].nRow;
            pNextIndices[nCol-nStartCol] = nIndex;
        }
        else
        {
            pNextRows[nCol-nStartCol] = MAXROWCOUNT;        // nichts gefunden
            pNextIndices[nCol-nStartCol] = MAXROWCOUNT;
        }

        Advance();
        return pCell;
    }
    else
        return NULL;
}

BOOL ScHorizontalCellIterator::ReturnNext( SCCOL& rCol, SCROW& rRow )
{
    rCol = nCol;
    rRow = nRow;
    return bMore;
}

void ScHorizontalCellIterator::Advance()
{
    BOOL bFound = FALSE;
    SCCOL i;

    for (i=nCol+1; i<=nEndCol && !bFound; i++)
        if (pNextRows[i-nStartCol] == nRow)
        {
            nCol = i;
            bFound = TRUE;
        }

    if (!bFound)
    {
        SCROW nMinRow = MAXROW+1;
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

ScHorizontalAttrIterator::ScHorizontalAttrIterator( ScDocument* pDocument, SCTAB nTable,
                            SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 ) :
    pDoc( pDocument ),
    nTab( nTable ),
    nStartCol( nCol1 ),
    nStartRow( nRow1 ),
    nEndCol( nCol2 ),
    nEndRow( nRow2 )
{
    DBG_ASSERT( pDoc->pTab[nTab], "Tabelle nicht da" );

    SCCOL i;

    nRow = nStartRow;
    nCol = nStartCol;
    bRowEmpty = FALSE;

    pIndices    = new SCSIZE[nEndCol-nStartCol+1];
    pNextEnd    = new SCROW[nEndCol-nStartCol+1];
    ppPatterns  = new const ScPatternAttr*[nEndCol-nStartCol+1];

    SCROW nSkipTo = MAXROW;
    BOOL bEmpty = TRUE;
    for (i=nStartCol; i<=nEndCol; i++)
    {
        SCCOL nPos = i - nStartCol;
        ScAttrArray* pArray = pDoc->pTab[nTab]->aCol[i].pAttrArray;
        DBG_ASSERT( pArray, "pArray == 0" );

        SCSIZE nIndex;
        pArray->Search( nStartRow, nIndex );

        const ScPatternAttr* pPattern = pArray->pData[nIndex].pPattern;
        SCROW nThisEnd = pArray->pData[nIndex].nRow;
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

const ScPatternAttr* ScHorizontalAttrIterator::GetNext( SCCOL& rCol1, SCCOL& rCol2, SCROW& rRow )
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
        SCCOL i;

        for ( i = nStartCol; i <= nEndCol; i++)
        {
            SCCOL nPos = i-nStartCol;
            if ( pNextEnd[nPos] < nRow )
            {
                ScAttrArray* pArray = pDoc->pTab[nTab]->aCol[i].pAttrArray;

                SCSIZE nIndex = ++pIndices[nPos];
                if ( nIndex < pArray->nCount )
                {
                    const ScPatternAttr* pPattern = pArray->pData[nIndex].pPattern;
                    SCROW nThisEnd = pArray->pData[nIndex].nRow;
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
            SCCOL nCount = nEndCol-nStartCol+1;
            SCROW nSkipTo = pNextEnd[0];                // naechstes Bereichsende suchen
            for (i=1; i<nCount; i++)
                if ( pNextEnd[i] < nSkipTo )
                    nSkipTo = pNextEnd[i];
            nRow = nSkipTo;                             // leere Zeilen ueberspringen
        }
        bRowEmpty = bEmpty;
        nCol = nStartCol;           // wieder links anfangen
    }

//    return NULL;
}

//-------------------------------------------------------------------------------

inline BOOL IsGreater( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 )
{
    return ( nRow1 > nRow2 ) || ( nRow1 == nRow2 && nCol1 > nCol2 );
}

ScUsedAreaIterator::ScUsedAreaIterator( ScDocument* pDocument, SCTAB nTable,
                            SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 ) :
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

ScDocAttrIterator::ScDocAttrIterator(ScDocument* pDocument, SCTAB nTable,
                                    SCCOL nCol1, SCROW nRow1,
                                    SCCOL nCol2, SCROW nRow2) :
    pDoc( pDocument ),
    nTab( nTable ),
    nEndCol( nCol2 ),
    nStartRow( nRow1 ),
    nEndRow( nRow2 ),
    nCol( nCol1 )
{
    if ( ValidTab(nTab) && pDoc->pTab[nTab] )
        pColIter = pDoc->pTab[nTab]->aCol[nCol].CreateAttrIterator( nStartRow, nEndRow );
    else
        pColIter = NULL;
}

ScDocAttrIterator::~ScDocAttrIterator()
{
    delete pColIter;
}

const ScPatternAttr* ScDocAttrIterator::GetNext( SCCOL& rCol, SCROW& rRow1, SCROW& rRow2 )
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

ScAttrRectIterator::ScAttrRectIterator(ScDocument* pDocument, SCTAB nTable,
                                    SCCOL nCol1, SCROW nRow1,
                                    SCCOL nCol2, SCROW nRow2) :
    pDoc( pDocument ),
    nTab( nTable ),
    nEndCol( nCol2 ),
    nStartRow( nRow1 ),
    nEndRow( nRow2 ),
    nIterStartCol( nCol1 ),
    nIterEndCol( nCol1 )
{
    if ( ValidTab(nTab) && pDoc->pTab[nTab] )
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
        SCROW nNextRow = pColIter->GetNextRow();
        delete pColIter;
        pColIter = pDoc->pTab[nTab]->aCol[nIterStartCol].CreateAttrIterator( nNextRow, nEndRow );
    }
}

const ScPatternAttr* ScAttrRectIterator::GetNext( SCCOL& rCol1, SCCOL& rCol2,
                                                    SCROW& rRow1, SCROW& rRow2 )
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

