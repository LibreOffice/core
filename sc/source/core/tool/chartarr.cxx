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

#include "scitems.hxx"
#include <svl/intitem.hxx>
#include <svl/zforlist.hxx>
#include <float.h>              // DBL_MIN

#include "chartarr.hxx"
#include "document.hxx"
#include "rechead.hxx"
#include "globstr.hrc"
#include "cell.hxx"
#include "docoptio.hxx"

#include <vector>

using ::std::vector;

// -----------------------------------------------------------------------

ScMemChart::ScMemChart(short nCols, short nRows)
{
    nRowCnt = nRows;
    nColCnt = nCols;
    pData   = new double[nColCnt * nRowCnt];

    if (pData)
    {
        double *pFill = pData;

        for (short i = 0; i < nColCnt; i++)
            for (short j = 0; j < nRowCnt; j++)
                *(pFill ++) = 0.0;
    }

    pColText = new String[nColCnt];
    pRowText = new String[nRowCnt];
}

ScMemChart::~ScMemChart()
{
    delete[] pRowText;
    delete[] pColText;
    delete[] pData;
}

// -----------------------------------------------------------------------

ScChartArray::ScChartArray( ScDocument* pDoc, SCTAB nTab,
                    SCCOL nStartColP, SCROW nStartRowP, SCCOL nEndColP, SCROW nEndRowP,
                    const String& rChartName ) :
        aName( rChartName ),
        pDocument( pDoc ),
        aPositioner(pDoc, nTab, nStartColP, nStartRowP, nEndColP, nEndRowP),
        bValid( TRUE )
{
}

ScChartArray::ScChartArray( ScDocument* pDoc, const ScRangeListRef& rRangeList,
                    const String& rChartName ) :
        aName( rChartName ),
        pDocument( pDoc ),
        aPositioner(pDoc, rRangeList),
        bValid( TRUE )
{
}

ScChartArray::ScChartArray( const ScChartArray& rArr ) :
        ScDataObject(),
        aName(rArr.aName),
        pDocument(rArr.pDocument),
        aPositioner(rArr.aPositioner),
        bValid(rArr.bValid)
{
}

ScChartArray::~ScChartArray()
{
}

ScDataObject* ScChartArray::Clone() const
{
    return new ScChartArray(*this);
}

BOOL ScChartArray::operator==(const ScChartArray& rCmp) const
{
    return aPositioner == rCmp.aPositioner
        && aName == rCmp.aName;
}

#ifdef _MSC_VER
#pragma optimize("",off)
#endif

ScMemChart* ScChartArray::CreateMemChart()
{
    ScRangeListRef aRangeListRef(GetRangeList());
    size_t nCount = aRangeListRef->size();
    if ( nCount > 1 )
        return CreateMemChartMulti();
    else if ( nCount == 1 )
    {
        ScRange* pR = aRangeListRef->front();
        if ( pR->aStart.Tab() != pR->aEnd.Tab() )
            return CreateMemChartMulti();
        else
            return CreateMemChartSingle();
    }
    else
        return CreateMemChartMulti();   // kann 0 Range besser ab als Single
}

ScMemChart* ScChartArray::CreateMemChartSingle()
{
    SCSIZE nCol;
    SCSIZE nRow;

        //
        //  wirkliche Groesse (ohne versteckte Zeilen/Spalten)
        //

    SCCOL nColAdd = HasRowHeaders() ? 1 : 0;
    SCROW nRowAdd = HasColHeaders() ? 1 : 0;

    SCCOL nCol1;
    SCROW nRow1;
    SCTAB nTab1;
    SCCOL nCol2;
    SCROW nRow2;
    SCTAB nTab2;
    ScRangeListRef aRangeListRef(GetRangeList());
    aRangeListRef->front()->GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );

    SCCOL nStrCol = nCol1;      // fuer Beschriftung merken
    SCROW nStrRow = nRow1;
    // Skip hidden columns.
    // TODO: make use of last column value once implemented.
    SCCOL nLastCol = -1;
    while (pDocument->ColHidden(nCol1, nTab1, NULL, &nLastCol))
        ++nCol1;

    // Skip hidden rows.
    SCROW nLastRow = -1;
    if (pDocument->RowHidden(nRow1, nTab1, NULL, &nLastRow))
        nRow1 = nLastRow + 1;

    // falls alles hidden ist, bleibt die Beschriftung am Anfang
    if ( nCol1 <= nCol2 )
    {
        nStrCol = nCol1;
        nCol1 = sal::static_int_cast<SCCOL>( nCol1 + nColAdd );
    }
    if ( nRow1 <= nRow2 )
    {
        nStrRow = nRow1;
        nRow1 = sal::static_int_cast<SCROW>( nRow1 + nRowAdd );
    }

    SCSIZE nTotalCols = ( nCol1 <= nCol2 ? nCol2 - nCol1 + 1 : 0 );
    vector<SCCOL> aCols;
    aCols.reserve(nTotalCols);
    for (SCSIZE i=0; i<nTotalCols; i++)
    {
        SCCOL nThisCol = sal::static_int_cast<SCCOL>(nCol1+i);
        if (!pDocument->ColHidden(nThisCol, nTab1, NULL, &nLastCol))
            aCols.push_back(nThisCol);
    }
    SCSIZE nColCount = aCols.size();

    SCSIZE nTotalRows = ( nRow1 <= nRow2 ? nRow2 - nRow1 + 1 : 0 );
    vector<SCROW> aRows;
    aRows.reserve(nTotalRows);
    if (nRow1 <= nRow2)
    {
        // Get all visible rows between nRow1 and nRow2.
        SCROW nThisRow = nRow1;
        while (nThisRow <= nRow2)
        {
            if (pDocument->RowHidden(nThisRow, nTab1, NULL, &nLastRow))
                nThisRow = nLastRow;
            else
                aRows.push_back(nThisRow);
            ++nThisRow;
        }
    }
    SCSIZE nRowCount = aRows.size();

    // May happen at least with more than 32k rows.
    if (nColCount > SHRT_MAX || nRowCount > SHRT_MAX)
    {
        nColCount = 0;
        nRowCount = 0;
    }

    BOOL bValidData = TRUE;
    if ( !nColCount )
    {
        bValidData = FALSE;
        nColCount = 1;
        aCols.push_back(nStrCol);
    }
    if ( !nRowCount )
    {
        bValidData = FALSE;
        nRowCount = 1;
        aRows.push_back(nStrRow);
    }

        //
        //  Daten
        //

    ScMemChart* pMemChart = new ScMemChart(
            static_cast<short>(nColCount), static_cast<short>(nRowCount) );
    if (pMemChart)
    {
        if ( bValidData )
        {
            BOOL bCalcAsShown = pDocument->GetDocOptions().IsCalcAsShown();
            ScBaseCell* pCell;
            for (nCol=0; nCol<nColCount; nCol++)
            {
                for (nRow=0; nRow<nRowCount; nRow++)
                {
                    double nVal = DBL_MIN;      // Hack fuer Chart, um leere Zellen zu erkennen

                    pDocument->GetCell( aCols[nCol], aRows[nRow], nTab1, pCell );
                    if (pCell)
                    {
                        CellType eType = pCell->GetCellType();
                        if (eType == CELLTYPE_VALUE)
                        {
                            nVal = ((ScValueCell*)pCell)->GetValue();
                            if ( bCalcAsShown && nVal != 0.0 )
                            {
                                sal_uInt32 nFormat;
                                pDocument->GetNumberFormat( aCols[nCol],
                                    aRows[nRow], nTab1, nFormat );
                                nVal = pDocument->RoundValueAsShown( nVal, nFormat );
                            }
                        }
                        else if (eType == CELLTYPE_FORMULA)
                        {
                            ScFormulaCell* pFCell = (ScFormulaCell*)pCell;
                            if ( (pFCell->GetErrCode() == 0) && pFCell->IsValue() )
                                nVal = pFCell->GetValue();
                        }
                    }
                    pMemChart->SetData(static_cast<short>(nCol), static_cast<short>(nRow), nVal);
                }
            }
        }
        else
        {
            //! Flag, dass Daten ungueltig ??

            for (nCol=0; nCol<nColCount; nCol++)
                for (nRow=0; nRow<nRowCount; nRow++)
                    pMemChart->SetData( static_cast<short>(nCol), static_cast<short>(nRow), DBL_MIN );
        }

        //
        //  Spalten-Header
        //

        for (nCol=0; nCol<nColCount; nCol++)
        {
            String aString, aColStr;
            if (HasColHeaders())
                pDocument->GetString( aCols[nCol], nStrRow, nTab1, aString );
            if ( !aString.Len() )
            {
                aString = ScGlobal::GetRscString(STR_COLUMN);
                aString += ' ';
                ScAddress aPos( aCols[ nCol ], 0, 0 );
                aPos.Format( aColStr, SCA_VALID_COL, NULL );
                aString += aColStr;
            }
            pMemChart->SetColText( static_cast<short>(nCol), aString);
        }

        //
        //  Zeilen-Header
        //

        for (nRow=0; nRow<nRowCount; nRow++)
        {
            String aString;
            if (HasRowHeaders())
            {
                ScAddress aAddr( nStrCol, aRows[nRow], nTab1 );
                pDocument->GetString( nStrCol, aRows[nRow], nTab1, aString );
            }
            if ( !aString.Len() )
            {
                aString = ScGlobal::GetRscString(STR_ROW);
                aString += ' ';
                aString += String::CreateFromInt32( aRows[nRow]+1 );
            }
            pMemChart->SetRowText( static_cast<short>(nRow), aString);
        }
    }

    return pMemChart;
}

ScMemChart* ScChartArray::CreateMemChartMulti()
{
    SCSIZE nColCount = GetPositionMap()->GetColCount();
    SCSIZE nRowCount = GetPositionMap()->GetRowCount();

    // May happen at least with more than 32k rows.
    if (nColCount > SHRT_MAX || nRowCount > SHRT_MAX)
    {
        nColCount = 0;
        nRowCount = 0;
    }

    BOOL bValidData = TRUE;
    if ( !nColCount )
    {
        bValidData = FALSE;
        nColCount = 1;
    }
    if ( !nRowCount )
    {
        bValidData = FALSE;
        nRowCount = 1;
    }

    //
    //  Daten
    //

    ScMemChart* pMemChart = new ScMemChart(
            static_cast<short>(nColCount), static_cast<short>(nRowCount) );
    if (pMemChart)
    {
        SCSIZE nCol = 0;
        SCSIZE nRow = 0;
        BOOL bCalcAsShown = pDocument->GetDocOptions().IsCalcAsShown();
        ULONG nIndex = 0;
        if (bValidData)
        {
            for ( nCol = 0; nCol < nColCount; nCol++ )
            {
                for ( nRow = 0; nRow < nRowCount; nRow++, nIndex++ )
                {
                    double nVal = DBL_MIN;      // Hack fuer Chart, um leere Zellen zu erkennen
                    const ScAddress* pPos = GetPositionMap()->GetPosition( nIndex );
                    if ( pPos )
                    {   // sonst: Luecke
                        ScBaseCell* pCell = pDocument->GetCell( *pPos );
                        if (pCell)
                        {
                            CellType eType = pCell->GetCellType();
                            if (eType == CELLTYPE_VALUE)
                            {
                                nVal = ((ScValueCell*)pCell)->GetValue();
                                if ( bCalcAsShown && nVal != 0.0 )
                                {
                                    ULONG nFormat = pDocument->GetNumberFormat( *pPos );
                                    nVal = pDocument->RoundValueAsShown( nVal, nFormat );
                                }
                            }
                            else if (eType == CELLTYPE_FORMULA)
                            {
                                ScFormulaCell* pFCell = (ScFormulaCell*)pCell;
                                if ( (pFCell->GetErrCode() == 0) && pFCell->IsValue() )
                                    nVal = pFCell->GetValue();
                            }
                        }
                    }
                    pMemChart->SetData(static_cast<short>(nCol), static_cast<short>(nRow), nVal);
                }
            }
        }
        else
        {
            for ( nRow = 0; nRow < nRowCount; nRow++, nIndex++ )
            {
                double nVal = DBL_MIN;      // Hack fuer Chart, um leere Zellen zu erkennen
                const ScAddress* pPos = GetPositionMap()->GetPosition( nIndex );
                if ( pPos )
                {   // sonst: Luecke
                    ScBaseCell* pCell = pDocument->GetCell( *pPos );
                    if (pCell)
                    {
                        CellType eType = pCell->GetCellType();
                        if (eType == CELLTYPE_VALUE)
                        {
                            nVal = ((ScValueCell*)pCell)->GetValue();
                            if ( bCalcAsShown && nVal != 0.0 )
                            {
                                ULONG nFormat = pDocument->GetNumberFormat( *pPos );
                                nVal = pDocument->RoundValueAsShown( nVal, nFormat );
                            }
                        }
                        else if (eType == CELLTYPE_FORMULA)
                        {
                            ScFormulaCell* pFCell = (ScFormulaCell*)pCell;
                            if ( (pFCell->GetErrCode() == 0) && pFCell->IsValue() )
                                nVal = pFCell->GetValue();
                        }
                    }
                }
                pMemChart->SetData(static_cast<short>(nCol), static_cast<short>(nRow), nVal);
            }
        }

//2do: Beschriftung bei Luecken

        //
        //  Spalten-Header
        //

        SCCOL nPosCol = 0;
        for ( nCol = 0; nCol < nColCount; nCol++ )
        {
            String aString, aColStr;
            const ScAddress* pPos = GetPositionMap()->GetColHeaderPosition( static_cast<SCCOL>(nCol) );
            if ( HasColHeaders() && pPos )
                pDocument->GetString(
                    pPos->Col(), pPos->Row(), pPos->Tab(), aString );
            if ( !aString.Len() )
            {
                aString = ScGlobal::GetRscString(STR_COLUMN);
                aString += ' ';
                if ( pPos )
                    nPosCol = pPos->Col() + 1;
                else
                    nPosCol++;
                ScAddress aPos( nPosCol - 1, 0, 0 );
                aPos.Format( aColStr, SCA_VALID_COL, NULL );
                aString += aColStr;
            }
            pMemChart->SetColText( static_cast<short>(nCol), aString);
        }

        //
        //  Zeilen-Header
        //

        SCROW nPosRow = 0;
        for ( nRow = 0; nRow < nRowCount; nRow++ )
        {
            String aString;
            const ScAddress* pPos = GetPositionMap()->GetRowHeaderPosition( nRow );
            if ( HasRowHeaders() && pPos )
            {
                pDocument->GetString(
                    pPos->Col(), pPos->Row(), pPos->Tab(), aString );
            }
            if ( !aString.Len() )
            {
                aString = ScGlobal::GetRscString(STR_ROW);
                aString += ' ';
                if ( pPos )
                    nPosRow = pPos->Row() + 1;
                else
                    nPosRow++;
                aString += String::CreateFromInt32( nPosRow );
            }
            pMemChart->SetRowText( static_cast<short>(nRow), aString);
        }
    }

    return pMemChart;
}

#ifdef _MSC_VER
#pragma optimize("",on)
#endif


//
//              Collection
//

ScDataObject*   ScChartCollection::Clone() const
{
    return new ScChartCollection(*this);
}

BOOL ScChartCollection::operator==(const ScChartCollection& rCmp) const
{
    if (nCount != rCmp.nCount)
        return FALSE;

    for (USHORT i=0; i<nCount; i++)
        if (!((*(const ScChartArray*)pItems[i]) == (*(const ScChartArray*)rCmp.pItems[i])))
            return FALSE;

    return TRUE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
