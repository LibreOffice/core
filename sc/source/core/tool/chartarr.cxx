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
#include <svl/intitem.hxx>
#include <svl/zforlist.hxx>
#include <float.h>

#include "chartarr.hxx"
#include "document.hxx"
#include "rechead.hxx"
#include "globstr.hrc"
#include "formulacell.hxx"
#include "docoptio.hxx"

#include <comphelper/stl_types.hxx>
#include <o3tl/make_unique.hxx>

#include <vector>

using ::std::vector;

ScMemChart::ScMemChart(SCCOL nCols, SCROW nRows)
{
    nRowCnt = nRows;
    nColCnt = nCols;
    pData   = new double[nColCnt * nRowCnt];

    double *pFill = pData;

    for (SCCOL i = 0; i < nColCnt; i++)
        for (SCROW j = 0; j < nRowCnt; j++)
            *(pFill ++) = 0.0;

    pColText = new OUString[nColCnt];
    pRowText = new OUString[nRowCnt];
}

ScMemChart::~ScMemChart()
{
    delete[] pRowText;
    delete[] pColText;
    delete[] pData;
}

ScChartArray::ScChartArray( ScDocument* pDoc, SCTAB nTab,
                    SCCOL nStartColP, SCROW nStartRowP, SCCOL nEndColP, SCROW nEndRowP,
                    const OUString& rChartName ) :
        aName( rChartName ),
        pDocument( pDoc ),
        aPositioner(pDoc, nTab, nStartColP, nStartRowP, nEndColP, nEndRowP),
        bValid( true )
{
}

ScChartArray::ScChartArray(
    ScDocument* pDoc, const ScRangeListRef& rRangeList, const OUString& rChartName ) :
    aName( rChartName ),
    pDocument( pDoc ),
    aPositioner(pDoc, rRangeList),
    bValid( true ) {}

ScChartArray::ScChartArray( const ScChartArray& rArr ) :
    aName(rArr.aName),
    pDocument(rArr.pDocument),
    aPositioner(rArr.aPositioner),
    bValid(rArr.bValid) {}

ScChartArray::~ScChartArray() {}

bool ScChartArray::operator==(const ScChartArray& rCmp) const
{
    return aPositioner == rCmp.aPositioner
        && aName == rCmp.aName;
}

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
        return CreateMemChartMulti(); // Can handle 0 range better than Single
}

namespace {

double getCellValue( ScDocument& rDoc, const ScAddress& rPos, double fDefault, bool bCalcAsShown )
{
    double fRet = fDefault;

    CellType eType = rDoc.GetCellType(rPos);
    switch (eType)
    {
        case CELLTYPE_VALUE:
        {
            fRet = rDoc.GetValue(rPos);
            if (bCalcAsShown && fRet != 0.0)
            {
                sal_uInt32 nFormat = rDoc.GetNumberFormat(rPos);
                fRet = rDoc.RoundValueAsShown(fRet, nFormat);
            }
        }
        break;
        case CELLTYPE_FORMULA:
        {
            ScFormulaCell* pFCell = rDoc.GetFormulaCell(rPos);
            if (pFCell && !pFCell->GetErrCode() && pFCell->IsValue())
                fRet = pFCell->GetValue();
        }
        break;
        default:
            ;
    }
    return fRet;
}

}

ScMemChart* ScChartArray::CreateMemChartSingle()
{
    SCSIZE nCol;
    SCSIZE nRow;

        //  real size (without hidden rows/columns)

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

    SCCOL nStrCol = nCol1; // remember for labeling
    SCROW nStrRow = nRow1;
    // Skip hidden columns.
    // TODO: make use of last column value once implemented.
    SCCOL nLastCol = -1;
    while (pDocument->ColHidden(nCol1, nTab1, nullptr, &nLastCol))
        ++nCol1;

    // Skip hidden rows.
    SCROW nLastRow = -1;
    if (pDocument->RowHidden(nRow1, nTab1, nullptr, &nLastRow))
        nRow1 = nLastRow + 1;

    // if everything is hidden then the label remains at the beginning
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
        if (!pDocument->ColHidden(nThisCol, nTab1, nullptr, &nLastCol))
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
            if (pDocument->RowHidden(nThisRow, nTab1, nullptr, &nLastRow))
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

    bool bValidData = true;
    if ( !nColCount )
    {
        bValidData = false;
        nColCount = 1;
        aCols.push_back(nStrCol);
    }
    if ( !nRowCount )
    {
        bValidData = false;
        nRowCount = 1;
        aRows.push_back(nStrRow);
    }

    //  Data
    ScMemChart* pMemChart = new ScMemChart( nColCount, nRowCount );

    if ( bValidData )
    {
        bool bCalcAsShown = pDocument->GetDocOptions().IsCalcAsShown();
        for (nCol=0; nCol<nColCount; nCol++)
        {
            for (nRow=0; nRow<nRowCount; nRow++)
            {
                // DBL_MIN is a Hack for Chart to recognize empty cells.
                ScAddress aPos(aCols[nCol], aRows[nRow], nTab1);
                double nVal = getCellValue(*pDocument, aPos, DBL_MIN, bCalcAsShown);
                pMemChart->SetData(nCol, nRow, nVal);
            }
        }
    }
    else
    {
        // Flag marking data as invalid?
        for (nCol=0; nCol<nColCount; nCol++)
            for (nRow=0; nRow<nRowCount; nRow++)
                pMemChart->SetData( nCol, nRow, DBL_MIN );
    }

    //  Column Header

    for (nCol=0; nCol<nColCount; nCol++)
    {
        OUString aString;
        if (HasColHeaders())
            aString = pDocument->GetString(aCols[nCol], nStrRow, nTab1);
        if (aString.isEmpty())
        {
            OUStringBuffer aBuf;
            aBuf.append(ScGlobal::GetRscString(STR_COLUMN));
            aBuf.append(' ');

            ScAddress aPos( aCols[ nCol ], 0, 0 );
            aBuf.append(aPos.Format(SCA_VALID_COL));

            aString = aBuf.makeStringAndClear();
        }
        pMemChart->SetColText( nCol, aString);
    }

    //  Row Header

    for (nRow=0; nRow<nRowCount; nRow++)
    {
        OUString aString;
        if (HasRowHeaders())
        {
            ScAddress aAddr( nStrCol, aRows[nRow], nTab1 );
            aString = pDocument->GetString(nStrCol, aRows[nRow], nTab1);
        }
        if (aString.isEmpty())
        {
            OUStringBuffer aBuf;
            aBuf.append(ScGlobal::GetRscString(STR_ROW));
            aBuf.append(' ');
            aBuf.append(static_cast<sal_Int32>(aRows[nRow]+1));
            aString = aBuf.makeStringAndClear();
        }
        pMemChart->SetRowText( nRow, aString);
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

    bool bValidData = true;
    if ( !nColCount )
    {
        bValidData = false;
        nColCount = 1;
    }
    if ( !nRowCount )
    {
        bValidData = false;
        nRowCount = 1;
    }

    //  Data
    ScMemChart* pMemChart = new ScMemChart( nColCount, nRowCount );

    SCSIZE nCol = 0;
    SCSIZE nRow = 0;
    bool bCalcAsShown = pDocument->GetDocOptions().IsCalcAsShown();
    sal_uLong nIndex = 0;
    if (bValidData)
    {
        for ( nCol = 0; nCol < nColCount; nCol++ )
        {
            for ( nRow = 0; nRow < nRowCount; nRow++, nIndex++ )
            {
                double nVal = DBL_MIN; // Hack for Chart to recognize empty cells
                const ScAddress* pPos = GetPositionMap()->GetPosition( nIndex );
                if (pPos)
                    // otherwise: Gap
                    nVal = getCellValue(*pDocument, *pPos, DBL_MIN, bCalcAsShown);

                pMemChart->SetData(nCol, nRow, nVal);
            }
        }
    }
    else
    {
        for ( nRow = 0; nRow < nRowCount; nRow++, nIndex++ )
        {
            double nVal = DBL_MIN; // Hack for Chart to recognize empty cells
            const ScAddress* pPos = GetPositionMap()->GetPosition( nIndex );
            if (pPos)
                // otherwise: Gap
                nVal = getCellValue(*pDocument, *pPos, DBL_MIN, bCalcAsShown);

            pMemChart->SetData(nCol, nRow, nVal);
        }
    }

    //TODO: Label when gaps

    //  Column header

    SCCOL nPosCol = 0;
    for ( nCol = 0; nCol < nColCount; nCol++ )
    {
        OUString aString;
        const ScAddress* pPos = GetPositionMap()->GetColHeaderPosition( static_cast<SCCOL>(nCol) );
        if ( HasColHeaders() && pPos )
            aString = pDocument->GetString(pPos->Col(), pPos->Row(), pPos->Tab());

        if (aString.isEmpty())
        {
            OUStringBuffer aBuf(ScGlobal::GetRscString(STR_COLUMN));
            aBuf.append(' ');
            if ( pPos )
                nPosCol = pPos->Col() + 1;
            else
                nPosCol++;
            ScAddress aPos( nPosCol - 1, 0, 0 );
            aBuf.append(aPos.Format(SCA_VALID_COL));
            aString = aBuf.makeStringAndClear();
        }
        pMemChart->SetColText( nCol, aString);
    }

    //  Row header

    SCROW nPosRow = 0;
    for ( nRow = 0; nRow < nRowCount; nRow++ )
    {
        OUString aString;
        const ScAddress* pPos = GetPositionMap()->GetRowHeaderPosition( nRow );
        if ( HasRowHeaders() && pPos )
            aString = pDocument->GetString(pPos->Col(), pPos->Row(), pPos->Tab());

        if (aString.isEmpty())
        {
            OUStringBuffer aBuf(ScGlobal::GetRscString(STR_ROW));
            aBuf.append(' ');
            if ( pPos )
                nPosRow = pPos->Row() + 1;
            else
                nPosRow++;
            aBuf.append(static_cast<sal_Int32>(nPosRow));
            aString = aBuf.makeStringAndClear();
        }
        pMemChart->SetRowText( nRow, aString);
    }

    return pMemChart;
}

ScChartCollection::ScChartCollection() {}
ScChartCollection::ScChartCollection(const ScChartCollection& r)
{
    for (auto const& it : r.m_Data)
    {
        m_Data.push_back(o3tl::make_unique<ScChartArray>(*it));
    }
}

void ScChartCollection::push_back(ScChartArray* p)
{
    m_Data.push_back(std::unique_ptr<ScChartArray>(p));
}

void ScChartCollection::clear()
{
    m_Data.clear();
}

size_t ScChartCollection::size() const
{
    return m_Data.size();
}

bool ScChartCollection::empty() const
{
    return m_Data.empty();
}

ScChartArray* ScChartCollection::operator[](size_t nIndex)
{
    if (m_Data.size() <= nIndex)
        return nullptr;
    return m_Data[nIndex].get();
}

const ScChartArray* ScChartCollection::operator[](size_t nIndex) const
{
    if (m_Data.size() <= nIndex)
        return nullptr;
    return m_Data[nIndex].get();
}

bool ScChartCollection::operator==(const ScChartCollection& rCmp) const
{
    return ::comphelper::ContainerUniquePtrEquals(m_Data, rCmp.m_Data);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
