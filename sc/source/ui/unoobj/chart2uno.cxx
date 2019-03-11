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

#include <memory>
#include <sal/config.h>
#include <sal/log.hxx>

#include <algorithm>
#include <utility>

#include <chart2uno.hxx>
#include <miscuno.hxx>
#include <document.hxx>
#include <formulacell.hxx>
#include <chartpos.hxx>
#include <unonames.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <convuno.hxx>
#include <rangeutl.hxx>
#include <hints.hxx>
#include <unoreflist.hxx>
#include <compiler.hxx>
#include <reftokenhelper.hxx>
#include <chartlis.hxx>
#include <tokenuno.hxx>
#include <docsh.hxx>
#include <cellvalue.hxx>
#include <tokenarray.hxx>
#include <scmatrix.hxx>
#include <brdcst.hxx>

#include <formula/opcode.hxx>
#include <svl/sharedstring.hxx>

#include <sfx2/objsh.hxx>
#include <vcl/svapp.hxx>

#include <com/sun/star/beans/UnknownPropertyException.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart2/data/LabeledDataSequence.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/text/XText.hpp>
#include <comphelper/extract.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>

#include <rtl/math.hxx>

SC_SIMPLE_SERVICE_INFO( ScChart2DataProvider, "ScChart2DataProvider",
        "com.sun.star.chart2.data.DataProvider")
SC_SIMPLE_SERVICE_INFO( ScChart2DataSource, "ScChart2DataSource",
        "com.sun.star.chart2.data.DataSource")
SC_SIMPLE_SERVICE_INFO( ScChart2DataSequence, "ScChart2DataSequence",
        "com.sun.star.chart2.data.DataSequence")

using namespace ::com::sun::star;
using namespace ::formula;
using ::com::sun::star::uno::Sequence;
using ::std::unique_ptr;
using ::std::vector;
using ::std::distance;
using ::std::shared_ptr;

namespace
{
const SfxItemPropertyMapEntry* lcl_GetDataProviderPropertyMap()
{
    static const SfxItemPropertyMapEntry aDataProviderPropertyMap_Impl[] =
    {
        { OUString(SC_UNONAME_INCLUDEHIDDENCELLS), 0, cppu::UnoType<bool>::get(), 0, 0 },
        { OUString(SC_UNONAME_USE_INTERNAL_DATA_PROVIDER), 0, cppu::UnoType<bool>::get(), 0, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    return aDataProviderPropertyMap_Impl;
}

const SfxItemPropertyMapEntry* lcl_GetDataSequencePropertyMap()
{
    static const SfxItemPropertyMapEntry aDataSequencePropertyMap_Impl[] =
    {
        {OUString(SC_UNONAME_HIDDENVALUES), 0, cppu::UnoType<uno::Sequence<sal_Int32>>::get(),                 0, 0 },
        {OUString(SC_UNONAME_ROLE), 0, cppu::UnoType<css::chart2::data::DataSequenceRole>::get(),                  0, 0 },
        {OUString(SC_UNONAME_INCLUDEHIDDENCELLS), 0,        cppu::UnoType<bool>::get(),                  0, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    return aDataSequencePropertyMap_Impl;
}

struct lcl_appendTableNumber
{
    explicit lcl_appendTableNumber( OUStringBuffer & rBuffer ) :
            m_rBuffer( rBuffer )
    {}
    void operator() ( SCTAB nTab )
    {
        // there is no append with SCTAB or sal_Int16
        m_rBuffer.append( static_cast< sal_Int32 >( nTab ));
        m_rBuffer.append( ' ' );
    }
private:
    OUStringBuffer & m_rBuffer;
};

OUString lcl_createTableNumberList( const ::std::vector< SCTAB > & rTableVector )
{
    OUStringBuffer aBuffer;
    ::std::for_each( rTableVector.begin(), rTableVector.end(), lcl_appendTableNumber( aBuffer ));
    // remove last trailing ' '
    if( !aBuffer.isEmpty() )
        aBuffer.setLength( aBuffer.getLength() - 1 );
    return aBuffer.makeStringAndClear();
}

uno::Reference< frame::XModel > lcl_GetXModel( const ScDocument * pDoc )
{
    uno::Reference< frame::XModel > xModel;
    SfxObjectShell * pObjSh( pDoc ? pDoc->GetDocumentShell() : nullptr );
    if( pObjSh )
        xModel.set( pObjSh->GetModel());
    return xModel;
}

struct TokenTable
{
    SCROW mnRowCount;
    SCCOL mnColCount;
    vector<std::unique_ptr<FormulaToken>> maTokens;

    // noncopyable
    TokenTable(const TokenTable&) = delete;
    const TokenTable& operator=(const TokenTable&) = delete;

    TokenTable()
        : mnRowCount(0)
        , mnColCount(0)
    {
    }

    void init( SCCOL nColCount, SCROW nRowCount )
    {
        mnColCount = nColCount;
        mnRowCount = nRowCount;
        maTokens.reserve(mnColCount*mnRowCount);
    }
    void clear()
    {
        for (auto & rToken : maTokens)
            rToken.reset();
    }

    void push_back( std::unique_ptr<FormulaToken> pToken )
    {
        maTokens.push_back( std::move(pToken) );
        OSL_ENSURE( maTokens.size()<= static_cast<sal_uInt32>( mnColCount*mnRowCount ), "too much tokens" );
    }

    sal_uInt32 getIndex(SCCOL nCol, SCROW nRow) const
    {
        OSL_ENSURE( nCol<mnColCount, "wrong column index" );
        OSL_ENSURE( nRow<mnRowCount, "wrong row index" );
        sal_uInt32 nRet = static_cast<sal_uInt32>(nCol*mnRowCount + nRow);
        OSL_ENSURE( maTokens.size()>= static_cast<sal_uInt32>( mnColCount*mnRowCount ), "too few tokens" );
        return nRet;
    }

    vector<ScTokenRef> getColRanges(SCCOL nCol) const;
    vector<ScTokenRef> getRowRanges(SCROW nRow) const;
    vector<ScTokenRef> getAllRanges() const;
};

vector<ScTokenRef> TokenTable::getColRanges(SCCOL nCol) const
{
    if (nCol >= mnColCount)
        return vector<ScTokenRef>();
    if( mnRowCount<=0 )
        return vector<ScTokenRef>();

    vector<ScTokenRef> aTokens;
    sal_uInt32 nLast = getIndex(nCol, mnRowCount-1);
    for (sal_uInt32 i = getIndex(nCol, 0); i <= nLast; ++i)
    {
        FormulaToken* p = maTokens[i].get();
        if (!p)
            continue;

        ScTokenRef pCopy(p->Clone());
        ScRefTokenHelper::join(aTokens, pCopy, ScAddress());
    }
    return aTokens;
}

vector<ScTokenRef> TokenTable::getRowRanges(SCROW nRow) const
{
    if (nRow >= mnRowCount)
        return vector<ScTokenRef>();
    if( mnColCount<=0 )
        return vector<ScTokenRef>();

    vector<ScTokenRef> aTokens;
    sal_uInt32 nLast = getIndex(mnColCount-1, nRow);
    for (sal_uInt32 i = getIndex(0, nRow); i <= nLast; i += mnRowCount)
    {
        FormulaToken* p = maTokens[i].get();
        if (!p)
            continue;

        ScTokenRef p2(p->Clone());
        ScRefTokenHelper::join(aTokens, p2, ScAddress());
    }
    return aTokens;
}

vector<ScTokenRef> TokenTable::getAllRanges() const
{
    vector<ScTokenRef> aTokens;
    sal_uInt32 nStop = mnColCount*mnRowCount;
    for (sal_uInt32 i = 0; i < nStop; i++)
    {
        FormulaToken* p = maTokens[i].get();
        if (!p)
            continue;

        ScTokenRef p2(p->Clone());
        ScRefTokenHelper::join(aTokens, p2, ScAddress());
    }
    return aTokens;
}

typedef std::map<SCROW, std::unique_ptr<FormulaToken>> FormulaTokenMap;
typedef std::map<sal_uInt32, FormulaTokenMap> FormulaTokenMapMap;

class Chart2PositionMap
{
public:
    Chart2PositionMap(SCCOL nColCount, SCROW nRowCount,
                      bool bFillRowHeader, bool bFillColumnHeader, FormulaTokenMapMap& rCols,
                      ScDocument* pDoc );
    ~Chart2PositionMap();

    SCCOL getDataColCount() const { return mnDataColCount; }
    SCROW getDataRowCount() const { return mnDataRowCount; }

    vector<ScTokenRef> getLeftUpperCornerRanges() const;
    vector<ScTokenRef> getAllColHeaderRanges() const;
    vector<ScTokenRef> getAllRowHeaderRanges() const;

    vector<ScTokenRef> getColHeaderRanges(SCCOL nChartCol) const;
    vector<ScTokenRef> getRowHeaderRanges(SCROW nChartRow) const;

    vector<ScTokenRef> getDataColRanges(SCCOL nCol) const;
    vector<ScTokenRef> getDataRowRanges(SCROW nRow) const;

private:
    SCCOL mnDataColCount;
    SCROW mnDataRowCount;

    TokenTable maLeftUpperCorner; //nHeaderColCount*nHeaderRowCount
    TokenTable maColHeaders; //mnDataColCount*nHeaderRowCount
    TokenTable maRowHeaders; //nHeaderColCount*mnDataRowCount
    TokenTable maData;//mnDataColCount*mnDataRowCount
};

Chart2PositionMap::Chart2PositionMap(SCCOL nAllColCount,  SCROW nAllRowCount,
                                     bool bFillRowHeader, bool bFillColumnHeader, FormulaTokenMapMap& rCols, ScDocument* pDoc)
{
    // if bFillRowHeader is true, at least the first column serves as a row header.
    //  If more than one column is pure text all the first pure text columns are used as header.
    // Likewise, if bFillColumnHeader is true, at least the first row serves as a column header.
    //  If more than one row is pure text all the first pure text rows are used as header.

    SCROW nHeaderRowCount = (bFillColumnHeader && nAllColCount && nAllRowCount) ? 1 : 0;
    SCCOL nHeaderColCount = (bFillRowHeader && nAllColCount && nAllRowCount) ? 1 : 0;

    if( pDoc && (nHeaderColCount || nHeaderRowCount ) )
    {
        //check whether there is more than one text column or row that should be added to the headers
        SCROW nMaxHeaderRow = nAllRowCount;
        SCCOL nCol = 0;
        for (auto it = rCols.begin(); it != rCols.end(); ++it, ++nCol)
        {
            // Skip header columns
            if (nCol < nHeaderColCount)
                continue;

            const auto& rCol = *it;

            bool bFoundValuesInCol = false;
            bool bFoundAnythingInCol = false;
            SCROW nRow = 0;
            for (auto it2 = rCol.second.begin(); it2 != rCol.second.end(); ++it2, ++nRow)
            {
                const auto& rCell = *it2;

                // Skip header rows
                if (nRow < nHeaderRowCount || !rCell.second)
                    continue;

                ScRange aRange;
                bool bExternal = false;
                StackVar eType = rCell.second->GetType();
                if( eType==svExternal || eType==svExternalSingleRef || eType==svExternalDoubleRef || eType==svExternalName )
                    bExternal = true;//lllll todo correct?
                ScTokenRef pSharedToken(rCell.second->Clone());
                ScRefTokenHelper::getRangeFromToken(aRange, pSharedToken, ScAddress(), bExternal);
                SCCOL nCol1=0, nCol2=0;
                SCROW nRow1=0, nRow2=0;
                SCTAB nTab1=0, nTab2=0;
                aRange.GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
                if ( pDoc->HasValueData( nCol1, nRow1, nTab1 ) )
                {
                    // Found some numeric data
                    bFoundValuesInCol = true;
                    nMaxHeaderRow = std::min(nMaxHeaderRow, nRow);
                    break;
                }
                if ( pDoc->HasData( nCol1, nRow1, nTab1 ) )
                {
                    // Found some other data (non-numeric)
                    bFoundAnythingInCol = true;
                }
                else
                {
                    // If cell is empty, it belongs to data
                    nMaxHeaderRow = std::min(nMaxHeaderRow, nRow);
                }
            }

            if (nHeaderColCount && !bFoundValuesInCol && bFoundAnythingInCol && nCol == nHeaderColCount)
            {
                // There is no values in row, but some data. And this column is next to header
                // So lets put it to header
                nHeaderColCount++;
            }
        }

        if (nHeaderRowCount)
        {
            nHeaderRowCount = nMaxHeaderRow;
        }
    }

    mnDataColCount = nAllColCount - nHeaderColCount;
    mnDataRowCount = nAllRowCount - nHeaderRowCount;

    maLeftUpperCorner.init(nHeaderColCount,nHeaderRowCount);
    maColHeaders.init(mnDataColCount,nHeaderRowCount);
    maRowHeaders.init(nHeaderColCount,mnDataRowCount);
    maData.init(mnDataColCount,mnDataRowCount);

    FormulaTokenMapMap::iterator it1 = rCols.begin();
    for (SCCOL nCol = 0; nCol < nAllColCount; ++nCol)
    {
        if (it1 != rCols.end())
        {
            FormulaTokenMap& rCol = it1->second;
            FormulaTokenMap::iterator it2 = rCol.begin();
            for (SCROW nRow = 0; nRow < nAllRowCount; ++nRow)
            {
                std::unique_ptr<FormulaToken> pToken;
                if (it2 != rCol.end())
                {
                    pToken = std::move(it2->second);
                    ++it2;
                }

                if( nCol < nHeaderColCount )
                {
                    if( nRow < nHeaderRowCount )
                        maLeftUpperCorner.push_back(std::move(pToken));
                    else
                        maRowHeaders.push_back(std::move(pToken));
                }
                else if( nRow < nHeaderRowCount )
                    maColHeaders.push_back(std::move(pToken));
                else
                    maData.push_back(std::move(pToken));
            }
            ++it1;
        }
    }
}

Chart2PositionMap::~Chart2PositionMap()
{
    maLeftUpperCorner.clear();
    maColHeaders.clear();
    maRowHeaders.clear();
    maData.clear();
}

vector<ScTokenRef> Chart2PositionMap::getLeftUpperCornerRanges() const
{
    return maLeftUpperCorner.getAllRanges();
}
vector<ScTokenRef> Chart2PositionMap::getAllColHeaderRanges() const
{
    return maColHeaders.getAllRanges();
}
vector<ScTokenRef> Chart2PositionMap::getAllRowHeaderRanges() const
{
    return maRowHeaders.getAllRanges();
}
vector<ScTokenRef> Chart2PositionMap::getColHeaderRanges(SCCOL nCol) const
{
    return maColHeaders.getColRanges( nCol);
}
vector<ScTokenRef> Chart2PositionMap::getRowHeaderRanges(SCROW nRow) const
{
    return maRowHeaders.getRowRanges( nRow);
}

vector<ScTokenRef> Chart2PositionMap::getDataColRanges(SCCOL nCol) const
{
    return maData.getColRanges( nCol);
}

vector<ScTokenRef> Chart2PositionMap::getDataRowRanges(SCROW nRow) const
{
    return maData.getRowRanges( nRow);
}

/**
 * Designed to be a drop-in replacement for ScChartPositioner, in order to
 * handle external references.
 */
class Chart2Positioner
{
    enum GlueType
    {
        GLUETYPE_NA,
        GLUETYPE_NONE,
        GLUETYPE_COLS,
        GLUETYPE_ROWS,
        GLUETYPE_BOTH
    };

public:
    Chart2Positioner(const Chart2Positioner&) = delete;
    const Chart2Positioner& operator=(const Chart2Positioner&) = delete;

    Chart2Positioner(ScDocument* pDoc, const vector<ScTokenRef>& rRefTokens) :
        mrRefTokens(rRefTokens),
        meGlue(GLUETYPE_NA),
        mnStartCol(0),
        mnStartRow(0),
        mpDoc(pDoc),
        mbColHeaders(false),
        mbRowHeaders(false),
        mbDummyUpperLeft(false)
    {
    }

    void setHeaders(bool bColHeaders, bool bRowHeaders)
    {
        mbColHeaders = bColHeaders;
        mbRowHeaders = bRowHeaders;
    }

    Chart2PositionMap* getPositionMap()
    {
        createPositionMap();
        return mpPositionMap.get();
    }

private:
    void invalidateGlue();
    void glueState();
    void calcGlueState(SCCOL nCols, SCROW nRows);
    void createPositionMap();

private:
    const vector<ScTokenRef>& mrRefTokens;
    std::unique_ptr<Chart2PositionMap> mpPositionMap;
    GlueType    meGlue;
    SCCOL       mnStartCol;
    SCROW       mnStartRow;
    ScDocument* const mpDoc;
    bool mbColHeaders:1;
    bool mbRowHeaders:1;
    bool mbDummyUpperLeft:1;
};

void Chart2Positioner::invalidateGlue()
{
    meGlue = GLUETYPE_NA;
    mpPositionMap.reset();
}

void Chart2Positioner::glueState()
{
    if (meGlue != GLUETYPE_NA)
        return;

    mbDummyUpperLeft = false;
    if (mrRefTokens.size() <= 1)
    {
        // Source data consists of only one data range.
        const ScTokenRef& p = mrRefTokens.front();
        ScComplexRefData aData;
        if (ScRefTokenHelper::getDoubleRefDataFromToken(aData, p))
        {
            if (aData.Ref1.Tab() == aData.Ref2.Tab())
                meGlue = GLUETYPE_NONE;
            else
                meGlue = GLUETYPE_COLS;
            mnStartCol = aData.Ref1.Col();
            mnStartRow = aData.Ref1.Row();
        }
        else
        {
            invalidateGlue();
            mnStartCol = 0;
            mnStartRow = 0;
        }
        return;
    }

    ScComplexRefData aData;
    ScRefTokenHelper::getDoubleRefDataFromToken(aData, mrRefTokens.front());
    mnStartCol = aData.Ref1.Col();
    mnStartRow = aData.Ref1.Row();

    SCCOL nEndCol = 0;
    SCROW nEndRow = 0;
    for (const auto& rxToken : mrRefTokens)
    {
        ScRefTokenHelper::getDoubleRefDataFromToken(aData, rxToken);
        SCCOLROW n1 = aData.Ref1.Col();
        SCCOLROW n2 = aData.Ref2.Col();
        if (n1 > MAXCOL)
            n1 = MAXCOL;
        if (n2 > MAXCOL)
            n2 = MAXCOL;
        if (n1 < mnStartCol)
            mnStartCol = static_cast<SCCOL>(n1);
        if (n2 > nEndCol)
            nEndCol = static_cast<SCCOL>(n2);

        n1 = aData.Ref1.Row();
        n2 = aData.Ref2.Row();
        if (n1 > MAXROW)
            n1 = MAXROW;
        if (n2 > MAXROW)
            n2 = MAXROW;

        if (n1 < mnStartRow)
            mnStartRow = static_cast<SCROW>(n1);
        if (n2 > nEndRow)
            nEndRow = static_cast<SCROW>(n2);
    }

    if (mnStartCol == nEndCol)
    {
        // All source data is in a single column.
        meGlue = GLUETYPE_ROWS;
        return;
    }

    if (mnStartRow == nEndRow)
    {
        // All source data is in a single row.
        meGlue = GLUETYPE_COLS;
        return;
    }

    // total column size
    SCCOL nC = nEndCol - mnStartCol + 1;

    // total row size
    SCROW nR = nEndRow - mnStartRow + 1;

    // #i103540# prevent invalid vector size
    if ((nC <= 0) || (nR <= 0))
    {
        invalidateGlue();
        mnStartCol = 0;
        mnStartRow = 0;
        return;
    }

    calcGlueState(nC, nR);
}

enum State { Hole = 0, Occupied = 1, Free = 2, Glue = 3 };

void Chart2Positioner::calcGlueState(SCCOL nColSize, SCROW nRowSize)
{
    // TODO: This code can use some space optimization.  Using an array to
    // store individual cell's states is terribly inefficient esp for large
    // data ranges; let's use flat_segment_tree to reduce memory usage here.

    sal_uInt32 nCR = static_cast<sal_uInt32>(nColSize*nRowSize);

    vector<State> aCellStates(nCR, Hole);

    // Mark all referenced cells "occupied".
    for (const auto& rxToken : mrRefTokens)
    {
        ScComplexRefData aData;
        ScRefTokenHelper::getDoubleRefDataFromToken(aData, rxToken);
        SCCOL nCol1 = aData.Ref1.Col() - mnStartCol;
        SCCOL nCol2 = aData.Ref2.Col() - mnStartCol;
        SCROW nRow1 = aData.Ref1.Row() - mnStartRow;
        SCROW nRow2 = aData.Ref2.Row() - mnStartRow;
        for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
            for (SCROW nRow = nRow1; nRow <= nRow2; ++nRow)
            {
                size_t i = nCol*nRowSize + nRow;
                aCellStates[i] = Occupied;
            }
    }

    // If at least one cell in either the first column or first row is empty,
    // we don't glue at all unless the whole column or row is empty; we expect
    // all cells in the first column / row to be fully populated.  If we have
    // empty column or row, then we do glue by the column or row,
    // respectively.

    bool bGlue = true;
    bool bGlueCols = false;
    for (SCCOL nCol = 0; bGlue && nCol < nColSize; ++nCol)
    {
        for (SCROW nRow = 0; bGlue && nRow < nRowSize; ++nRow)
        {
            size_t i = nCol*nRowSize + nRow;
            if (aCellStates[i] == Occupied)
            {
                if (nCol == 0 || nRow == 0)
                    break;

                bGlue = false;
            }
            else
                aCellStates[i] = Free;
        }
        size_t nLast = (nCol+1)*nRowSize - 1; // index for the last cell in the column.
        if (bGlue && aCellStates[nLast] == Free)
        {
            // Whole column is empty.
            aCellStates[nLast] = Glue;
            bGlueCols = true;
        }
    }

    bool bGlueRows = false;
    for (SCROW nRow = 0; bGlue && nRow < nRowSize; ++nRow)
    {
        size_t i = nRow;
        for (SCCOL nCol = 0; bGlue && nCol < nColSize; ++nCol, i += nRowSize)
        {
            if (aCellStates[i] == Occupied)
            {
                if (nCol == 0 || nRow == 0)
                    break;

                bGlue = false;
            }
            else
                aCellStates[i] = Free;
        }
        i = (nColSize-1)*nRowSize + nRow; // index for the row position in the last column.
        if (bGlue && aCellStates[i] == Free)
        {
            // Whole row is empty.
            aCellStates[i] = Glue;
            bGlueRows = true;
        }
    }

    size_t i = 1;
    for (sal_uInt32 n = 1; bGlue && n < nCR; ++n, ++i)
        if (aCellStates[i] == Hole)
            bGlue = false;

    if (bGlue)
    {
        if (bGlueCols && bGlueRows)
            meGlue = GLUETYPE_BOTH;
        else if (bGlueRows)
            meGlue = GLUETYPE_ROWS;
        else
            meGlue = GLUETYPE_COLS;
        if (aCellStates.front() != Occupied)
            mbDummyUpperLeft = true;
    }
    else
        meGlue = GLUETYPE_NONE;
}

void Chart2Positioner::createPositionMap()
{
    if (meGlue == GLUETYPE_NA && mpPositionMap.get())
        mpPositionMap.reset();

    if (mpPositionMap)
        return;

    glueState();

    bool bNoGlue = (meGlue == GLUETYPE_NONE);
    FormulaTokenMapMap aCols;
    SCROW nNoGlueRow = 0;
    for (const ScTokenRef& pToken : mrRefTokens)
    {
        bool bExternal = ScRefTokenHelper::isExternalRef(pToken);
        sal_uInt16 nFileId = bExternal ? pToken->GetIndex() : 0;
        svl::SharedString aTabName = svl::SharedString::getEmptyString();
        if (bExternal)
            aTabName = pToken->GetString();

        ScComplexRefData aData;
        if( !ScRefTokenHelper::getDoubleRefDataFromToken(aData, pToken) )
            break;
        const ScSingleRefData& s = aData.Ref1;
        const ScSingleRefData& e = aData.Ref2;
        SCCOL nCol1 = s.Col(), nCol2 = e.Col();
        SCROW nRow1 = s.Row(), nRow2 = e.Row();
        SCTAB nTab1 = s.Tab(), nTab2 = e.Tab();

        for (SCTAB nTab = nTab1; nTab <= nTab2; ++nTab)
        {
            // columns on secondary sheets are appended; we treat them as if
            // all columns are on the same sheet.  TODO: We can't assume that
            // the column range is 16-bit; remove that restriction.
            sal_uInt32 nInsCol = (static_cast<sal_uInt32>(nTab) << 16) |
                (bNoGlue ? 0 : static_cast<sal_uInt32>(nCol1));

            for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol, ++nInsCol)
            {
                FormulaTokenMap& rCol = aCols[nInsCol];

                auto nInsRow = bNoGlue ? nNoGlueRow : nRow1;
                for (SCROW nRow = nRow1; nRow <= nRow2; ++nRow, ++nInsRow)
                {
                    ScSingleRefData aCellData;
                    aCellData.InitFlags();
                    aCellData.SetFlag3D(true);
                    aCellData.SetColRel(false);
                    aCellData.SetRowRel(false);
                    aCellData.SetTabRel(false);
                    aCellData.SetAbsCol(nCol);
                    aCellData.SetAbsRow(nRow);
                    aCellData.SetAbsTab(nTab);

                    if (rCol.find(nInsRow) == rCol.end())
                    {
                        if (bExternal)
                            rCol[ nInsRow ].reset(new ScExternalSingleRefToken(nFileId, aTabName, aCellData));
                        else
                            rCol[ nInsRow ].reset(new ScSingleRefToken(aCellData));
                    }
                }
            }
        }
        nNoGlueRow += nRow2 - nRow1 + 1;
    }

    bool bFillRowHeader = mbRowHeaders;
    bool bFillColumnHeader = mbColHeaders;

    SCSIZE nAllColCount = static_cast<SCSIZE>(aCols.size());
    SCSIZE nAllRowCount = 0;
    if (!aCols.empty())
    {
        FormulaTokenMap& rCol = aCols.begin()->second;
        if (mbDummyUpperLeft)
            if (rCol.find(0) == rCol.end())
                rCol[ 0 ] = nullptr;        // dummy for labeling
        nAllRowCount = static_cast<SCSIZE>(rCol.size());
    }

    if( nAllColCount!=0 && nAllRowCount!=0 )
    {
        if (bNoGlue)
        {
            FormulaTokenMap& rFirstCol = aCols.begin()->second;
            for (const auto& rFirstColEntry : rFirstCol)
            {
                SCROW nKey = rFirstColEntry.first;
                for (auto& rEntry : aCols)
                {
                    FormulaTokenMap& rCol = rEntry.second;
                    if (rCol.find(nKey) == rCol.end())
                        rCol[ nKey ] = nullptr;
                }
            }
        }
    }
    mpPositionMap.reset(
        new Chart2PositionMap(
            static_cast<SCCOL>(nAllColCount), static_cast<SCROW>(nAllRowCount),
            bFillRowHeader, bFillColumnHeader, aCols, mpDoc));
}

/**
 * Function object to create a range string from a token list.
 */
class Tokens2RangeString
{
public:
    Tokens2RangeString(ScDocument* pDoc, FormulaGrammar::Grammar eGram, sal_Unicode cRangeSep) :
        mpRangeStr(new OUStringBuffer),
        mpDoc(pDoc),
        meGrammar(eGram),
        mcRangeSep(cRangeSep),
        mbFirst(true)
    {
    }

    void operator() (const ScTokenRef& rToken)
    {
        ScCompiler aCompiler(mpDoc, ScAddress(0,0,0), meGrammar);
        OUString aStr;
        aCompiler.CreateStringFromToken(aStr, rToken.get());
        if (mbFirst)
            mbFirst = false;
        else
            mpRangeStr->append(mcRangeSep);
        mpRangeStr->append(aStr);
    }

    void getString(OUString& rStr)
    {
        rStr = mpRangeStr->makeStringAndClear();
    }

private:
    shared_ptr<OUStringBuffer>  mpRangeStr;
    ScDocument*         mpDoc;
    FormulaGrammar::Grammar  meGrammar;
    sal_Unicode         mcRangeSep;
    bool                mbFirst;
};

/**
 * Function object to convert a list of tokens into a string form suitable
 * for ODF export.  In ODF, a range is expressed as
 *
 *   (start cell address):(end cell address)
 *
 * and each address doesn't include any '$' symbols.
 */
class Tokens2RangeStringXML
{
public:
    explicit Tokens2RangeStringXML(ScDocument* pDoc) :
        mpRangeStr(new OUStringBuffer),
        mpDoc(pDoc),
        mbFirst(true)
    {
    }

    void operator() (const ScTokenRef& rToken)
    {
        if (mbFirst)
            mbFirst = false;
        else
            mpRangeStr->append(mcRangeSep);

        ScTokenRef aStart, aEnd;
        bool bValidToken = splitRangeToken(rToken, aStart, aEnd);
        OSL_ENSURE(bValidToken, "invalid token");
        if (!bValidToken)
            return;
        ScCompiler aCompiler(mpDoc, ScAddress(0,0,0), FormulaGrammar::GRAM_ENGLISH);
        {
            OUString aStr;
            aCompiler.CreateStringFromToken(aStr, aStart.get());
            mpRangeStr->append(aStr);
        }
        mpRangeStr->append(mcAddrSep);
        {
            OUString aStr;
            aCompiler.CreateStringFromToken(aStr, aEnd.get());
            mpRangeStr->append(aStr);
        }
    }

    void getString(OUString& rStr)
    {
        rStr = mpRangeStr->makeStringAndClear();
    }

private:
    static bool splitRangeToken(const ScTokenRef& pToken, ScTokenRef& rStart, ScTokenRef& rEnd)
    {
        ScComplexRefData aData;
        bool bIsRefToken = ScRefTokenHelper::getDoubleRefDataFromToken(aData, pToken);
        OSL_ENSURE(bIsRefToken, "invalid token");
        if (!bIsRefToken)
            return false;
        bool bExternal = ScRefTokenHelper::isExternalRef(pToken);
        sal_uInt16 nFileId = bExternal ? pToken->GetIndex() : 0;
        svl::SharedString aTabName = svl::SharedString::getEmptyString();
        if (bExternal)
            aTabName = pToken->GetString();

        // In saving to XML, we don't prepend address with '$'.
        setRelative(aData.Ref1);
        setRelative(aData.Ref2);

        // In XML, the range must explicitly specify sheet name.
        aData.Ref1.SetFlag3D(true);
        aData.Ref2.SetFlag3D(true);

        if (bExternal)
            rStart.reset(new ScExternalSingleRefToken(nFileId, aTabName, aData.Ref1));
        else
            rStart.reset(new ScSingleRefToken(aData.Ref1));

        if (bExternal)
            rEnd.reset(new ScExternalSingleRefToken(nFileId, aTabName, aData.Ref2));
        else
            rEnd.reset(new ScSingleRefToken(aData.Ref2));
        return true;
    }

    static void setRelative(ScSingleRefData& rData)
    {
        rData.SetColRel(true);
        rData.SetRowRel(true);
        rData.SetTabRel(true);
    }

private:
    shared_ptr<OUStringBuffer>  mpRangeStr;
    ScDocument*                 mpDoc;
    static const sal_Unicode    mcRangeSep = ' ';
    static const sal_Unicode    mcAddrSep = ':';
    bool                        mbFirst;
};

void lcl_convertTokensToString(OUString& rStr, const vector<ScTokenRef>& rTokens, ScDocument* pDoc)
{
    const sal_Unicode cRangeSep = ScCompiler::GetNativeSymbolChar(ocSep);
    FormulaGrammar::Grammar eGrammar = pDoc->GetGrammar();
    Tokens2RangeString func(pDoc, eGrammar, cRangeSep);
    func = ::std::for_each(rTokens.begin(), rTokens.end(), func);
    func.getString(rStr);
}

} // anonymous namespace

// DataProvider ==============================================================

ScChart2DataProvider::ScChart2DataProvider( ScDocument* pDoc )
    : m_pDocument( pDoc)
    , m_aPropSet(lcl_GetDataProviderPropertyMap())
    , m_bIncludeHiddenCells( true)
{
    if ( m_pDocument )
        m_pDocument->AddUnoObject( *this);
}

ScChart2DataProvider::~ScChart2DataProvider()
{
    SolarMutexGuard g;

    if ( m_pDocument )
        m_pDocument->RemoveUnoObject( *this);
}

void ScChart2DataProvider::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint)
{
    if ( rHint.GetId() == SfxHintId::Dying )
    {
        m_pDocument = nullptr;
    }
}

sal_Bool SAL_CALL ScChart2DataProvider::createDataSourcePossible( const uno::Sequence< beans::PropertyValue >& aArguments )
{
    SolarMutexGuard aGuard;
    if( ! m_pDocument )
        return false;

    OUString aRangeRepresentation;
    for(sal_Int32 i = 0; i < aArguments.getLength(); ++i)
    {
        if ( aArguments[i].Name == "CellRangeRepresentation" )
        {
            aArguments[i].Value >>= aRangeRepresentation;
        }
    }

    vector<ScTokenRef> aTokens;
    const sal_Unicode cSep = ScCompiler::GetNativeSymbolChar(ocSep);
    ScRefTokenHelper::compileRangeRepresentation(
        aTokens, aRangeRepresentation, m_pDocument, cSep, m_pDocument->GetGrammar(), true);
    return !aTokens.empty();
}

namespace
{

uno::Reference< chart2::data::XLabeledDataSequence > lcl_createLabeledDataSequenceFromTokens(
    vector< ScTokenRef > && aValueTokens, vector< ScTokenRef > && aLabelTokens,
    ScDocument* pDoc, const uno::Reference< chart2::data::XDataProvider >& xDP, bool bIncludeHiddenCells )
{
    uno::Reference< chart2::data::XLabeledDataSequence >  xResult;
    bool bHasValues = !aValueTokens.empty();
    bool bHasLabel = !aLabelTokens.empty();
    if( bHasValues || bHasLabel )
    {
        try
        {
            uno::Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
            if ( xContext.is() )
            {
                xResult.set( chart2::data::LabeledDataSequence::create(xContext), uno::UNO_QUERY_THROW );
            }
            if ( bHasValues )
            {
                uno::Reference< chart2::data::XDataSequence > xSeq( new ScChart2DataSequence( pDoc, xDP, std::move(aValueTokens), bIncludeHiddenCells ) );
                xResult->setValues( xSeq );
            }
            if ( bHasLabel )
            {
                uno::Reference< chart2::data::XDataSequence > xLabelSeq( new ScChart2DataSequence( pDoc, xDP, std::move(aLabelTokens), bIncludeHiddenCells ) );
                xResult->setLabel( xLabelSeq );
            }
        }
        catch( const uno::Exception& )
        {
        }
    }
    return xResult;
}

/**
 * Check the current list of reference tokens, and add the upper left
 * corner of the minimum range that encloses all ranges if certain
 * conditions are met.
 *
 * @param rRefTokens list of reference tokens
 *
 * @return true if the corner was added, false otherwise.
 */
bool lcl_addUpperLeftCornerIfMissing(vector<ScTokenRef>& rRefTokens,
            SCROW nCornerRowCount, SCCOL nCornerColumnCount)
{
    using ::std::max;
    using ::std::min;

    if (rRefTokens.empty())
        return false;

    SCCOL nMinCol = MAXCOLCOUNT;
    SCROW nMinRow = MAXROWCOUNT;
    SCCOL nMaxCol = 0;
    SCROW nMaxRow = 0;
    SCTAB nTab    = 0;

    sal_uInt16 nFileId = 0;
    svl::SharedString aExtTabName;
    bool bExternal = false;

    vector<ScTokenRef>::const_iterator itr = rRefTokens.begin(), itrEnd = rRefTokens.end();

    // Get the first ref token.
    ScTokenRef pToken = *itr;
    switch (pToken->GetType())
    {
        case svSingleRef:
        {
            const ScSingleRefData& rData = *pToken->GetSingleRef();
            nMinCol = rData.Col();
            nMinRow = rData.Row();
            nMaxCol = rData.Col();
            nMaxRow = rData.Row();
            nTab = rData.Tab();
        }
        break;
        case svDoubleRef:
        {
            const ScComplexRefData& rData = *pToken->GetDoubleRef();
            nMinCol = min(rData.Ref1.Col(), rData.Ref2.Col());
            nMinRow = min(rData.Ref1.Row(), rData.Ref2.Row());
            nMaxCol = max(rData.Ref1.Col(), rData.Ref2.Col());
            nMaxRow = max(rData.Ref1.Row(), rData.Ref2.Row());
            nTab = rData.Ref1.Tab();
        }
        break;
        case svExternalSingleRef:
        {
            const ScSingleRefData& rData = *pToken->GetSingleRef();
            nMinCol = rData.Col();
            nMinRow = rData.Row();
            nMaxCol = rData.Col();
            nMaxRow = rData.Row();
            nTab = rData.Tab();
            nFileId = pToken->GetIndex();
            aExtTabName = pToken->GetString();
            bExternal = true;
        }
        break;
        case svExternalDoubleRef:
        {
            const ScComplexRefData& rData = *pToken->GetDoubleRef();
            nMinCol = min(rData.Ref1.Col(), rData.Ref2.Col());
            nMinRow = min(rData.Ref1.Row(), rData.Ref2.Row());
            nMaxCol = max(rData.Ref1.Col(), rData.Ref2.Col());
            nMaxRow = max(rData.Ref1.Row(), rData.Ref2.Row());
            nTab = rData.Ref1.Tab();
            nFileId = pToken->GetIndex();
            aExtTabName = pToken->GetString();
            bExternal = true;
        }
        break;
        default:
            ;
    }

    // Determine the minimum range enclosing all data ranges.  Also make sure
    // that they are all on the same table.

    for (++itr; itr != itrEnd; ++itr)
    {
        pToken = *itr;
        switch (pToken->GetType())
        {
            case svSingleRef:
            {
                const ScSingleRefData& rData = *pToken->GetSingleRef();

                nMinCol = min(nMinCol, rData.Col());
                nMinRow = min(nMinRow, rData.Row());
                nMaxCol = max(nMaxCol, rData.Col());
                nMaxRow = max(nMaxRow, rData.Row());
                if (nTab != rData.Tab() || bExternal)
                    return false;
            }
            break;
            case svDoubleRef:
            {
                const ScComplexRefData& rData = *pToken->GetDoubleRef();

                nMinCol = min(nMinCol, rData.Ref1.Col());
                nMinCol = min(nMinCol, rData.Ref2.Col());
                nMinRow = min(nMinRow, rData.Ref1.Row());
                nMinRow = min(nMinRow, rData.Ref2.Row());

                nMaxCol = max(nMaxCol, rData.Ref1.Col());
                nMaxCol = max(nMaxCol, rData.Ref2.Col());
                nMaxRow = max(nMaxRow, rData.Ref1.Row());
                nMaxRow = max(nMaxRow, rData.Ref2.Row());

                if (nTab != rData.Ref1.Tab() || bExternal)
                    return false;
            }
            break;
            case svExternalSingleRef:
            {
                if (!bExternal)
                    return false;

                if (nFileId != pToken->GetIndex() || aExtTabName != pToken->GetString())
                    return false;

                const ScSingleRefData& rData = *pToken->GetSingleRef();

                nMinCol = min(nMinCol, rData.Col());
                nMinRow = min(nMinRow, rData.Row());
                nMaxCol = max(nMaxCol, rData.Col());
                nMaxRow = max(nMaxRow, rData.Row());
            }
            break;
            case svExternalDoubleRef:
            {
                if (!bExternal)
                    return false;

                if (nFileId != pToken->GetIndex() || aExtTabName != pToken->GetString())
                    return false;

                const ScComplexRefData& rData = *pToken->GetDoubleRef();

                nMinCol = min(nMinCol, rData.Ref1.Col());
                nMinCol = min(nMinCol, rData.Ref2.Col());
                nMinRow = min(nMinRow, rData.Ref1.Row());
                nMinRow = min(nMinRow, rData.Ref2.Row());

                nMaxCol = max(nMaxCol, rData.Ref1.Col());
                nMaxCol = max(nMaxCol, rData.Ref2.Col());
                nMaxRow = max(nMaxRow, rData.Ref1.Row());
                nMaxRow = max(nMaxRow, rData.Ref2.Row());
            }
            break;
            default:
                ;
        }
    }

    if (nMinRow >= nMaxRow || nMinCol >= nMaxCol ||
        nMinRow >= MAXROWCOUNT || nMinCol >= MAXCOLCOUNT ||
        nMaxRow >= MAXROWCOUNT || nMaxCol >= MAXCOLCOUNT)
    {
        // Invalid range.  Bail out.
        return false;
    }

    // Check if the following conditions are met:

    // 1) The upper-left corner cell is not included.
    // 2) The three adjacent cells of that corner cell are included.

    bool bRight = false, bBottom = false, bDiagonal = false;
    for (const auto& rxToken : rRefTokens)
    {
        switch (rxToken->GetType())
        {
            case svSingleRef:
            case svExternalSingleRef:
            {
                const ScSingleRefData& rData = *rxToken->GetSingleRef();
                if (rData.Col() == nMinCol && rData.Row() == nMinRow)
                    // The corner cell is contained.
                    return false;

                if (rData.Col() == nMinCol+nCornerColumnCount && rData.Row() == nMinRow)
                    bRight = true;

                if (rData.Col() == nMinCol && rData.Row() == nMinRow+nCornerRowCount)
                    bBottom = true;

                if (rData.Col() == nMinCol+nCornerColumnCount && rData.Row() == nMinRow+nCornerRowCount)
                    bDiagonal = true;
            }
            break;
            case svDoubleRef:
            case svExternalDoubleRef:
            {
                const ScComplexRefData& rData = *rxToken->GetDoubleRef();
                const ScSingleRefData& r1 = rData.Ref1;
                const ScSingleRefData& r2 = rData.Ref2;
                if (r1.Col() <= nMinCol && nMinCol <= r2.Col() &&
                    r1.Row() <= nMinRow && nMinRow <= r2.Row())
                    // The corner cell is contained.
                    return false;

                if (r1.Col() <= nMinCol+nCornerColumnCount && nMinCol+nCornerColumnCount <= r2.Col() &&
                    r1.Row() <= nMinRow && nMinRow <= r2.Row())
                    bRight = true;

                if (r1.Col() <= nMinCol && nMinCol <= r2.Col() &&
                    r1.Row() <= nMinRow+nCornerRowCount && nMinRow+nCornerRowCount <= r2.Row())
                    bBottom = true;

                if (r1.Col() <= nMinCol+nCornerColumnCount && nMinCol+nCornerColumnCount <= r2.Col() &&
                    r1.Row() <= nMinRow+nCornerRowCount && nMinRow+nCornerRowCount <= r2.Row())
                    bDiagonal = true;
            }
            break;
            default:
                ;
        }
    }

    if (!bRight || !bBottom || !bDiagonal)
        // Not all the adjacent cells are included.  Bail out.
        return false;

    ScSingleRefData aData;
    aData.InitFlags();
    aData.SetFlag3D(true);
    aData.SetAbsCol(nMinCol);
    aData.SetAbsRow(nMinRow);
    aData.SetAbsTab(nTab);

    if( nCornerRowCount==1 && nCornerColumnCount==1 )
    {
        if (bExternal)
        {
            ScTokenRef pCorner(
                new ScExternalSingleRefToken(nFileId, aExtTabName, aData));
            ScRefTokenHelper::join(rRefTokens, pCorner, ScAddress());
        }
        else
        {
            ScTokenRef pCorner(new ScSingleRefToken(aData));
            ScRefTokenHelper::join(rRefTokens, pCorner, ScAddress());
        }
    }
    else
    {
        ScSingleRefData aDataEnd(aData);
        aDataEnd.IncCol(nCornerColumnCount-1);
        aDataEnd.IncRow(nCornerRowCount-1);
        ScComplexRefData r;
        r.Ref1=aData;
        r.Ref2=aDataEnd;
        if (bExternal)
        {
            ScTokenRef pCorner(
                new ScExternalDoubleRefToken(nFileId, aExtTabName, r));
            ScRefTokenHelper::join(rRefTokens, pCorner, ScAddress());
        }
        else
        {
            ScTokenRef pCorner(new ScDoubleRefToken(r));
            ScRefTokenHelper::join(rRefTokens, pCorner, ScAddress());
        }
    }

    return true;
}

#define SHRINK_RANGE_THRESHOLD 10000

class ShrinkRefTokenToDataRange
{
    ScDocument* const mpDoc;
public:
    explicit ShrinkRefTokenToDataRange(ScDocument* pDoc) : mpDoc(pDoc) {}
    void operator() (const ScTokenRef& rRef)
    {
        if (ScRefTokenHelper::isExternalRef(rRef))
            return;

        // Don't assume an ScDoubleRefToken if it isn't. It can be at least an
        // ScSingleRefToken, then there isn't anything to shrink.
        if (rRef->GetType() != svDoubleRef)
            return;

        ScComplexRefData& rData = *rRef->GetDoubleRef();
        ScSingleRefData& s = rData.Ref1;
        ScSingleRefData& e = rData.Ref2;

        if(abs((e.Col()-s.Col())*(e.Row()-s.Row())) < SHRINK_RANGE_THRESHOLD)
            return;

        SCCOL nMinCol = MAXCOL, nMaxCol = 0;
        SCROW nMinRow = MAXROW, nMaxRow = 0;

        // Determine the smallest range that encompasses the data ranges of all sheets.
        SCTAB nTab1 = s.Tab(), nTab2 = e.Tab();
        for (SCTAB nTab = nTab1; nTab <= nTab2; ++nTab)
        {
            SCCOL nCol1 = 0, nCol2 = MAXCOL;
            SCROW nRow1 = 0, nRow2 = MAXROW;
            mpDoc->ShrinkToDataArea(nTab, nCol1, nRow1, nCol2, nRow2);
            nMinCol = std::min(nMinCol, nCol1);
            nMinRow = std::min(nMinRow, nRow1);
            nMaxCol = std::max(nMaxCol, nCol2);
            nMaxRow = std::max(nMaxRow, nRow2);
        }

        // Shrink range to the data range if applicable.
        if (s.Col() < nMinCol)
            s.SetAbsCol(nMinCol);
        if (s.Row() < nMinRow)
            s.SetAbsRow(nMinRow);
        if (e.Col() > nMaxCol)
            e.SetAbsCol(nMaxCol);
        if (e.Row() > nMaxRow)
            e.SetAbsRow(nMaxRow);
    }
};

void shrinkToDataRange(ScDocument* pDoc, vector<ScTokenRef>& rRefTokens)
{
    std::for_each(rRefTokens.begin(), rRefTokens.end(), ShrinkRefTokenToDataRange(pDoc));
}

}

uno::Reference< chart2::data::XDataSource> SAL_CALL
ScChart2DataProvider::createDataSource(
    const uno::Sequence< beans::PropertyValue >& aArguments )
{
    SolarMutexGuard aGuard;
    if ( ! m_pDocument )
        throw uno::RuntimeException();

    uno::Reference< chart2::data::XDataSource> xResult;
    bool bLabel = true;
    bool bCategories = false;
    bool bOrientCol = true;
    OUString aRangeRepresentation;
    uno::Sequence< sal_Int32 > aSequenceMapping;
    bool bTimeBased = false;
    for(sal_Int32 i = 0; i < aArguments.getLength(); ++i)
    {
        if ( aArguments[i].Name == "DataRowSource" )
        {
            chart::ChartDataRowSource eSource = chart::ChartDataRowSource_COLUMNS;
            if( ! (aArguments[i].Value >>= eSource))
            {
                sal_Int32 nSource(0);
                if( aArguments[i].Value >>= nSource )
                    eSource = static_cast< chart::ChartDataRowSource >( nSource );
            }
            bOrientCol = (eSource == chart::ChartDataRowSource_COLUMNS);
        }
        else if ( aArguments[i].Name == "FirstCellAsLabel" )
        {
            bLabel = ::cppu::any2bool(aArguments[i].Value);
        }
        else if ( aArguments[i].Name == "HasCategories" )
        {
            bCategories = ::cppu::any2bool(aArguments[i].Value);
        }
        else if ( aArguments[i].Name == "CellRangeRepresentation" )
        {
            aArguments[i].Value >>= aRangeRepresentation;
        }
        else if ( aArguments[i].Name == "SequenceMapping" )
        {
            aArguments[i].Value >>= aSequenceMapping;
        }
        else if ( aArguments[i].Name == "TimeBased" )
        {
            aArguments[i].Value >>= bTimeBased;
        }
    }

    vector<ScTokenRef> aRefTokens;
    const sal_Unicode cSep = ScCompiler::GetNativeSymbolChar(ocSep);
    ScRefTokenHelper::compileRangeRepresentation(
        aRefTokens, aRangeRepresentation, m_pDocument, cSep, m_pDocument->GetGrammar(), true);
    if (aRefTokens.empty())
        // Invalid range representation.  Bail out.
        throw lang::IllegalArgumentException();

    SCTAB nTimeBasedStart = MAXTAB;
    SCTAB nTimeBasedEnd = 0;
    if(bTimeBased)
    {
        // limit to first sheet
        for(const auto& rxToken : aRefTokens)
        {
            if (rxToken->GetType() != svDoubleRef)
                continue;

            ScComplexRefData& rData = *rxToken->GetDoubleRef();
            ScSingleRefData& s = rData.Ref1;
            ScSingleRefData& e = rData.Ref2;

            nTimeBasedStart = std::min(nTimeBasedStart, s.Tab());
            nTimeBasedEnd = std::min(nTimeBasedEnd, e.Tab());

            if(s.Tab() != e.Tab())
                e.SetAbsTab(s.Tab());
        }
    }

    if(!bTimeBased)
        shrinkToDataRange(m_pDocument, aRefTokens);

    if (bLabel)
        lcl_addUpperLeftCornerIfMissing(aRefTokens, 1, 1); //#i90669#

    bool bColHeaders = (bOrientCol ? bLabel : bCategories );
    bool bRowHeaders = (bOrientCol ? bCategories : bLabel );

    Chart2Positioner aChPositioner(m_pDocument, aRefTokens);
    aChPositioner.setHeaders(bColHeaders, bRowHeaders);

    const Chart2PositionMap* pChartMap = aChPositioner.getPositionMap();
    if (!pChartMap)
        // No chart position map instance.  Bail out.
        return xResult;

    ScChart2DataSource* pDS = nullptr;
    ::std::vector< uno::Reference< chart2::data::XLabeledDataSequence > > aSeqs;

    // Fill Categories
    if( bCategories )
    {
        vector<ScTokenRef> aValueTokens;
        if (bOrientCol)
            aValueTokens = pChartMap->getAllRowHeaderRanges();
        else
            aValueTokens = pChartMap->getAllColHeaderRanges();

        vector<ScTokenRef> aLabelTokens(
                pChartMap->getLeftUpperCornerRanges());

        uno::Reference< chart2::data::XLabeledDataSequence > xCategories = lcl_createLabeledDataSequenceFromTokens(
            std::move(aValueTokens), std::move(aLabelTokens), m_pDocument, this, m_bIncludeHiddenCells ); //ownership of pointers is transferred!
        if ( xCategories.is() )
        {
            aSeqs.push_back( xCategories );
        }
    }

    // Fill Series (values and label)
    sal_Int32 nCount = bOrientCol ? pChartMap->getDataColCount() : pChartMap->getDataRowCount();
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        vector<ScTokenRef> aValueTokens;
        vector<ScTokenRef> aLabelTokens;
        if (bOrientCol)
        {
            aValueTokens = pChartMap->getDataColRanges(static_cast<SCCOL>(i));
            aLabelTokens = pChartMap->getColHeaderRanges(static_cast<SCCOL>(i));
        }
        else
        {
            aValueTokens = pChartMap->getDataRowRanges(static_cast<SCROW>(i));
            aLabelTokens = pChartMap->getRowHeaderRanges(static_cast<SCROW>(i));
        }
        uno::Reference< chart2::data::XLabeledDataSequence > xChartSeries = lcl_createLabeledDataSequenceFromTokens(
            std::move(aValueTokens), std::move(aLabelTokens), m_pDocument, this, m_bIncludeHiddenCells ); //ownership of pointers is transferred!
        if ( xChartSeries.is() && xChartSeries->getValues().is() && xChartSeries->getValues()->getData().getLength() )
        {
            aSeqs.push_back( xChartSeries );
        }
    }

    pDS = new ScChart2DataSource(m_pDocument);

    //reorder labeled sequences according to aSequenceMapping
    ::std::vector< uno::Reference< chart2::data::XLabeledDataSequence > > aSeqVector;
    aSeqVector.reserve(aSeqs.size());
    for (auto const& aSeq : aSeqs)
    {
        aSeqVector.push_back(aSeq);
    }

    for( sal_Int32 nNewIndex = 0; nNewIndex < aSequenceMapping.getLength(); nNewIndex++ )
    {
        // note: assuming that the values in the sequence mapping are always non-negative
        ::std::vector< uno::Reference< chart2::data::XLabeledDataSequence > >::size_type nOldIndex( static_cast< sal_uInt32 >( aSequenceMapping[nNewIndex] ) );
        if( nOldIndex < aSeqVector.size() )
        {
            pDS->AddLabeledSequence( aSeqVector[nOldIndex] );
            aSeqVector[nOldIndex] = nullptr;
        }
    }

    for(const uno::Reference< chart2::data::XLabeledDataSequence >& xSeq : aSeqVector)
    {
        if ( xSeq.is() )
        {
            pDS->AddLabeledSequence( xSeq );
        }
    }

    xResult.set( pDS );
    return xResult;
}

namespace
{

/**
 * Function object to create a list of table numbers from a token list.
 */
class InsertTabNumber
{
public:
    InsertTabNumber() :
        mpTabNumVector(new vector<SCTAB>)
    {
    }

    void operator() (const ScTokenRef& pToken) const
    {
        if (!ScRefTokenHelper::isRef(pToken))
            return;

        const ScSingleRefData& r = *pToken->GetSingleRef();
        mpTabNumVector->push_back(r.Tab());
    }

    void getVector(vector<SCTAB>& rVector)
    {
        mpTabNumVector->swap(rVector);
    }
private:
    shared_ptr< vector<SCTAB> > mpTabNumVector;
};

class RangeAnalyzer
{
public:
    RangeAnalyzer();
    void initRangeAnalyzer( const vector<ScTokenRef>& rTokens );
    void analyzeRange( sal_Int32& rnDataInRows, sal_Int32& rnDataInCols,
            bool& rbRowSourceAmbiguous ) const;
    bool inSameSingleRow( const RangeAnalyzer& rOther );
    bool inSameSingleColumn( const RangeAnalyzer& rOther );
    SCROW getRowCount() { return mnRowCount; }
    SCCOL getColumnCount() { return mnColumnCount; }

private:
    bool mbEmpty;
    bool mbAmbiguous;
    SCROW mnRowCount;
    SCCOL mnColumnCount;

    SCCOL mnStartColumn;
    SCROW mnStartRow;
};

RangeAnalyzer::RangeAnalyzer()
    : mbEmpty(true)
    , mbAmbiguous(false)
    , mnRowCount(0)
    , mnColumnCount(0)
    , mnStartColumn(-1)
    , mnStartRow(-1)
{
}

void RangeAnalyzer::initRangeAnalyzer( const vector<ScTokenRef>& rTokens )
{
    mnRowCount=0;
    mnColumnCount=0;
    mnStartColumn = -1;
    mnStartRow = -1;
    mbAmbiguous=false;
    if( rTokens.empty() )
    {
        mbEmpty=true;
        return;
    }
    mbEmpty=false;

    for (const ScTokenRef& aRefToken : rTokens)
    {
        StackVar eVar = aRefToken->GetType();
        if (eVar == svDoubleRef || eVar == svExternalDoubleRef)
        {
            const ScComplexRefData& r = *aRefToken->GetDoubleRef();
            if (r.Ref1.Tab() == r.Ref2.Tab())
            {
                mnColumnCount = std::max<SCCOL>(mnColumnCount, static_cast<SCCOL>(abs(r.Ref2.Col() - r.Ref1.Col())+1));
                mnRowCount = std::max<SCROW>(mnRowCount, static_cast<SCROW>(abs(r.Ref2.Row() - r.Ref1.Row())+1));
                if( mnStartColumn == -1 )
                {
                    mnStartColumn = r.Ref1.Col();
                    mnStartRow = r.Ref1.Row();
                }
                else
                {
                    if (mnStartColumn != r.Ref1.Col() && mnStartRow != r.Ref1.Row())
                        mbAmbiguous=true;
                }
            }
            else
                mbAmbiguous=true;
        }
        else if (eVar == svSingleRef || eVar == svExternalSingleRef)
        {
            const ScSingleRefData& r = *aRefToken->GetSingleRef();
            mnColumnCount = std::max<SCCOL>( mnColumnCount, 1);
            mnRowCount = std::max<SCROW>( mnRowCount, 1);
            if( mnStartColumn == -1 )
            {
                mnStartColumn = r.Col();
                mnStartRow = r.Row();
            }
            else
            {
                if (mnStartColumn != r.Col() && mnStartRow != r.Row())
                    mbAmbiguous=true;
            }
        }
        else
            mbAmbiguous=true;
    }
}

void RangeAnalyzer::analyzeRange( sal_Int32& rnDataInRows,
                                     sal_Int32& rnDataInCols,
                                     bool& rbRowSourceAmbiguous ) const
{
    if(!mbEmpty && !mbAmbiguous)
    {
        if( mnRowCount==1 && mnColumnCount>1 )
            ++rnDataInRows;
        else if( mnColumnCount==1 && mnRowCount>1 )
            ++rnDataInCols;
        else if( mnRowCount>1 && mnColumnCount>1 )
            rbRowSourceAmbiguous = true;
    }
    else if( !mbEmpty )
        rbRowSourceAmbiguous = true;
}

bool RangeAnalyzer::inSameSingleRow( const RangeAnalyzer& rOther )
{
    return mnStartRow==rOther.mnStartRow &&
        mnRowCount==1 && rOther.mnRowCount==1;
}

bool RangeAnalyzer::inSameSingleColumn( const RangeAnalyzer& rOther )
{
    return mnStartColumn==rOther.mnStartColumn &&
        mnColumnCount==1 && rOther.mnColumnCount==1;
}

std::pair<OUString, OUString> constructKey(const uno::Reference< chart2::data::XLabeledDataSequence>& xNew)
{
    std::pair<OUString, OUString> aKey;
    if( xNew->getLabel().is() )
        aKey.first = xNew->getLabel()->getSourceRangeRepresentation();
    if( xNew->getValues().is() )
        aKey.second = xNew->getValues()->getSourceRangeRepresentation();
    return aKey;
}


} //end anonymous namespace

uno::Sequence< beans::PropertyValue > SAL_CALL ScChart2DataProvider::detectArguments(
    const uno::Reference< chart2::data::XDataSource >& xDataSource )
{
    ::std::vector< beans::PropertyValue > aResult;
    bool bRowSourceDetected = false;
    bool bFirstCellAsLabel = false;
    bool bHasCategories = false;
    OUString sRangeRep;

    bool bHasCategoriesLabels = false;
    vector<ScTokenRef> aAllCategoriesValuesTokens;
    vector<ScTokenRef> aAllSeriesLabelTokens;

    chart::ChartDataRowSource eRowSource = chart::ChartDataRowSource_COLUMNS;

    vector<ScTokenRef> aAllTokens;

    // parse given data source and collect infos
    {
        SolarMutexGuard aGuard;
        OSL_ENSURE( m_pDocument, "No Document -> no detectArguments" );
        if(!m_pDocument ||!xDataSource.is())
            return comphelper::containerToSequence( aResult );

        sal_Int32 nDataInRows = 0;
        sal_Int32 nDataInCols = 0;
        bool bRowSourceAmbiguous = false;

        Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > aSequences( xDataSource->getDataSequences());
        const sal_Int32 nCount( aSequences.getLength());
        RangeAnalyzer aPrevLabel,aPrevValues;
        for( sal_Int32 nIdx=0; nIdx<nCount; ++nIdx )
        {
            uno::Reference< chart2::data::XLabeledDataSequence > xLS(aSequences[nIdx]);
            if( xLS.is() )
            {
                bool bThisIsCategories = false;
                if(!bHasCategories)
                {
                    uno::Reference< beans::XPropertySet > xSeqProp( xLS->getValues(), uno::UNO_QUERY );
                    OUString aRole;
                    if( xSeqProp.is() && (xSeqProp->getPropertyValue("Role") >>= aRole) &&
                        aRole == "categories" )
                        bThisIsCategories = bHasCategories = true;
                }

                RangeAnalyzer aLabel,aValues;
                // label
                uno::Reference< chart2::data::XDataSequence > xLabel( xLS->getLabel());
                if( xLabel.is())
                {
                    bFirstCellAsLabel = true;
                    vector<ScTokenRef> aTokens;
                    const sal_Unicode cSep = ScCompiler::GetNativeSymbolChar(ocSep);
                    ScRefTokenHelper::compileRangeRepresentation(
                        aTokens, xLabel->getSourceRangeRepresentation(), m_pDocument, cSep, m_pDocument->GetGrammar(), true);
                    aLabel.initRangeAnalyzer(aTokens);
                    for (const auto& rxToken : aTokens)
                    {
                        ScRefTokenHelper::join(aAllTokens, rxToken, ScAddress());
                        if(!bThisIsCategories)
                            ScRefTokenHelper::join(aAllSeriesLabelTokens, rxToken, ScAddress());
                    }
                    if(bThisIsCategories)
                        bHasCategoriesLabels=true;
                }
                // values
                uno::Reference< chart2::data::XDataSequence > xValues( xLS->getValues());
                if( xValues.is())
                {
                    vector<ScTokenRef> aTokens;
                    const sal_Unicode cSep = ScCompiler::GetNativeSymbolChar(ocSep);
                    ScRefTokenHelper::compileRangeRepresentation(
                        aTokens, xValues->getSourceRangeRepresentation(), m_pDocument, cSep, m_pDocument->GetGrammar(), true);
                    aValues.initRangeAnalyzer(aTokens);
                    for (const auto& rxToken : aTokens)
                    {
                        ScRefTokenHelper::join(aAllTokens, rxToken, ScAddress());
                        if(bThisIsCategories)
                            ScRefTokenHelper::join(aAllCategoriesValuesTokens, rxToken, ScAddress());
                    }
                }
                //detect row source
                if(!bThisIsCategories || nCount==1) //categories might span multiple rows *and* columns, so they should be used for detection only if nothing else is available
                {
                    if (!bRowSourceAmbiguous)
                    {
                        aValues.analyzeRange(nDataInRows,nDataInCols,bRowSourceAmbiguous);
                        aLabel.analyzeRange(nDataInRows,nDataInCols,bRowSourceAmbiguous);
                        if (nDataInRows > 1 && nDataInCols > 1)
                            bRowSourceAmbiguous = true;
                        else if( !bRowSourceAmbiguous && !nDataInRows && !nDataInCols )
                        {
                            if( aValues.inSameSingleColumn( aLabel ) )
                                nDataInCols++;
                            else if( aValues.inSameSingleRow( aLabel ) )
                                nDataInRows++;
                            else
                            {
                                //#i86188# also detect a single column split into rows correctly
                                if( aValues.inSameSingleColumn( aPrevValues ) )
                                    nDataInRows++;
                                else if( aValues.inSameSingleRow( aPrevValues ) )
                                    nDataInCols++;
                                else if( aLabel.inSameSingleColumn( aPrevLabel ) )
                                    nDataInRows++;
                                else if( aLabel.inSameSingleRow( aPrevLabel ) )
                                    nDataInCols++;
                            }
                        }
                    }
                }
                aPrevValues=aValues;
                aPrevLabel=aLabel;
            }
        }

        if (!bRowSourceAmbiguous)
        {
            bRowSourceDetected = true;
            eRowSource = ( nDataInCols > 0
                           ? chart::ChartDataRowSource_COLUMNS
                           : chart::ChartDataRowSource_ROWS );
        }
        else
        {
            // set DataRowSource to the better of the two ambiguities
            eRowSource = ( nDataInRows > nDataInCols
                           ? chart::ChartDataRowSource_ROWS
                           : chart::ChartDataRowSource_COLUMNS );
        }

    }

    // TableNumberList
    {
        vector<SCTAB> aTableNumVector;
        InsertTabNumber func;
        func = ::std::for_each(aAllTokens.begin(), aAllTokens.end(), func);
        func.getVector(aTableNumVector);
        aResult.emplace_back( "TableNumberList", -1,
                                  uno::makeAny( lcl_createTableNumberList( aTableNumVector ) ),
                                  beans::PropertyState_DIRECT_VALUE );
    }

    // DataRowSource (calculated before)
    if( bRowSourceDetected )
    {
        aResult.emplace_back( "DataRowSource", -1,
                                  uno::makeAny( eRowSource ), beans::PropertyState_DIRECT_VALUE );
    }

    // HasCategories
    if( bRowSourceDetected )
    {
        aResult.emplace_back( "HasCategories", -1,
                                  uno::makeAny( bHasCategories ), beans::PropertyState_DIRECT_VALUE );
    }

    // FirstCellAsLabel
    if( bRowSourceDetected )
    {
        aResult.emplace_back( "FirstCellAsLabel", -1,
                                  uno::makeAny( bFirstCellAsLabel ), beans::PropertyState_DIRECT_VALUE );
    }

    // Add the left upper corner to the range if it is missing.
    if (bRowSourceDetected && bFirstCellAsLabel && bHasCategories && !bHasCategoriesLabels )
    {
        RangeAnalyzer aTop,aLeft;
        if( eRowSource==chart::ChartDataRowSource_COLUMNS )
        {
            aTop.initRangeAnalyzer(aAllSeriesLabelTokens);
            aLeft.initRangeAnalyzer(aAllCategoriesValuesTokens);
        }
        else
        {
            aTop.initRangeAnalyzer(aAllCategoriesValuesTokens);
            aLeft.initRangeAnalyzer(aAllSeriesLabelTokens);
        }
        lcl_addUpperLeftCornerIfMissing(aAllTokens, aTop.getRowCount(), aLeft.getColumnCount());//e.g. #i91212#
    }

    // Get range string.
    lcl_convertTokensToString(sRangeRep, aAllTokens, m_pDocument);

    // add cell range property
    aResult.emplace_back( "CellRangeRepresentation", -1,
                              uno::makeAny( sRangeRep ), beans::PropertyState_DIRECT_VALUE );

    //Sequence Mapping
    bool const bSequencesReordered = true;//todo detect this above or detect this sequence mapping cheaper ...
    if( bSequencesReordered && bRowSourceDetected )
    {
        bool bDifferentIndexes = false;

        std::vector< sal_Int32 > aSequenceMappingVector;

        uno::Reference< chart2::data::XDataSource > xCompareDataSource;
        try
        {
            xCompareDataSource.set( createDataSource( comphelper::containerToSequence( aResult ) ) );
        }
        catch( const lang::IllegalArgumentException & )
        {
            // creation of data source to compare didn't work, so we cannot
            // create a sequence mapping
        }

        if( xDataSource.is() && xCompareDataSource.is() )
        {
            const uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence> >& aOldSequences =
                xCompareDataSource->getDataSequences();
            const uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence> >& aNewSequences =
                xDataSource->getDataSequences();

            std::map<std::pair<OUString, OUString>,sal_Int32> aOldEntryToIndex;
            for( sal_Int32 nIndex = 0, n = aOldSequences.getLength(); nIndex < n; nIndex++ )
            {
                const uno::Reference< chart2::data::XLabeledDataSequence>& xOld( aOldSequences[nIndex] );
                if( xOld.is() )
                {
                    std::pair<OUString, OUString> aKey = constructKey(xOld);
                    aOldEntryToIndex[aKey] = nIndex;
                }
            }

            for( sal_Int32 nNewIndex = 0, n = aNewSequences.getLength(); nNewIndex < n; nNewIndex++ )
            {
                const uno::Reference< chart2::data::XLabeledDataSequence>& xNew( aNewSequences[nNewIndex] );
                if( !xNew.is() )
                    continue;

                std::pair<OUString, OUString> aKey = constructKey(xNew);
                if (aOldEntryToIndex.find(aKey) == aOldEntryToIndex.end())
                    continue;

                sal_Int32 nOldIndex = aOldEntryToIndex[aKey];
                if( nOldIndex != nNewIndex )
                    bDifferentIndexes = true;

                aSequenceMappingVector.push_back(nOldIndex);
            }
        }

        if( bDifferentIndexes && !aSequenceMappingVector.empty() )
        {
            aResult.emplace_back( "SequenceMapping", -1,
                    uno::makeAny( comphelper::containerToSequence(aSequenceMappingVector) )
                    , beans::PropertyState_DIRECT_VALUE );
        }
    }

    return comphelper::containerToSequence( aResult );
}

sal_Bool SAL_CALL ScChart2DataProvider::createDataSequenceByRangeRepresentationPossible( const OUString& aRangeRepresentation )
{
    SolarMutexGuard aGuard;
    if( ! m_pDocument )
        return false;

    vector<ScTokenRef> aTokens;
    const sal_Unicode cSep = ScCompiler::GetNativeSymbolChar(ocSep);
    ScRefTokenHelper::compileRangeRepresentation(
        aTokens, aRangeRepresentation, m_pDocument, cSep, m_pDocument->GetGrammar(), true);
    return !aTokens.empty();
}

uno::Reference< chart2::data::XDataSequence > SAL_CALL
    ScChart2DataProvider::createDataSequenceByRangeRepresentation(
    const OUString& aRangeRepresentation )
{
    SolarMutexGuard aGuard;
    uno::Reference< chart2::data::XDataSequence > xResult;

    OSL_ENSURE( m_pDocument, "No Document -> no createDataSequenceByRangeRepresentation" );
    if(!m_pDocument || aRangeRepresentation.isEmpty())
        return xResult;

    vector<ScTokenRef> aRefTokens;
    const sal_Unicode cSep = ScCompiler::GetNativeSymbolChar(ocSep);
    ScRefTokenHelper::compileRangeRepresentation(
        aRefTokens, aRangeRepresentation, m_pDocument, cSep, m_pDocument->GetGrammar(), true);
    if (aRefTokens.empty())
        return xResult;

    shrinkToDataRange(m_pDocument, aRefTokens);

    xResult.set(new ScChart2DataSequence(m_pDocument, this, std::move(aRefTokens), m_bIncludeHiddenCells));

    return xResult;
}

uno::Reference<chart2::data::XDataSequence> SAL_CALL
ScChart2DataProvider::createDataSequenceByValueArray(
    const OUString& /*aRole*/, const OUString& /*aRangeRepresentation*/ )
{
    return uno::Reference<chart2::data::XDataSequence>();
}

uno::Reference< sheet::XRangeSelection > SAL_CALL ScChart2DataProvider::getRangeSelection()
{
    uno::Reference< sheet::XRangeSelection > xResult;

    uno::Reference< frame::XModel > xModel( lcl_GetXModel( m_pDocument ));
    if( xModel.is())
        xResult.set( xModel->getCurrentController(), uno::UNO_QUERY );

    return xResult;
}

sal_Bool SAL_CALL ScChart2DataProvider::createDataSequenceByFormulaTokensPossible(
    const Sequence<sheet::FormulaToken>& aTokens )
{
    if (aTokens.getLength() <= 0)
        return false;

    ScTokenArray aCode;
    if (!ScTokenConversion::ConvertToTokenArray(*m_pDocument, aCode, aTokens))
        return false;

    sal_uInt16 n = aCode.GetLen();
    if (!n)
        return false;

    formula::FormulaTokenArrayPlainIterator aIter(aCode);
    const formula::FormulaToken* pFirst = aIter.First();
    const formula::FormulaToken* pLast = aCode.GetArray()[n-1];
    for (const formula::FormulaToken* p = aIter.First(); p; p = aIter.Next())
    {
        switch (p->GetType())
        {
            case svSep:
            {
                switch (p->GetOpCode())
                {
                    case ocSep:
                        // separators are allowed.
                    break;
                    case ocOpen:
                        if (p != pFirst)
                            // open paran is allowed only as the first token.
                            return false;
                    break;
                    case ocClose:
                        if (p != pLast)
                            // close paren is allowed only as the last token.
                            return false;
                    break;
                    default:
                        return false;
                }
            }
            break;
            case svSingleRef:
            case svDoubleRef:
            case svExternalSingleRef:
            case svExternalDoubleRef:
            break;
            default:
                return false;
        }
    }

    return true;
}

uno::Reference<chart2::data::XDataSequence> SAL_CALL
ScChart2DataProvider::createDataSequenceByFormulaTokens(
    const Sequence<sheet::FormulaToken>& aTokens )
{
    uno::Reference<chart2::data::XDataSequence> xResult;
    if (aTokens.getLength() <= 0)
        return xResult;

    ScTokenArray aCode;
    if (!ScTokenConversion::ConvertToTokenArray(*m_pDocument, aCode, aTokens))
        return xResult;

    sal_uInt16 n = aCode.GetLen();
    if (!n)
        return xResult;

    vector<ScTokenRef> aRefTokens;
    formula::FormulaTokenArrayPlainIterator aIter(aCode);
    const formula::FormulaToken* pFirst = aIter.First();
    const formula::FormulaToken* pLast = aCode.GetArray()[n-1];
    for (const formula::FormulaToken* p = aIter.First(); p; p = aIter.Next())
    {
        switch (p->GetType())
        {
            case svSep:
            {
                switch (p->GetOpCode())
                {
                    case ocSep:
                        // separators are allowed.
                    break;
                    case ocOpen:
                        if (p != pFirst)
                            // open paran is allowed only as the first token.
                            throw lang::IllegalArgumentException();
                    break;
                    case ocClose:
                        if (p != pLast)
                            // close paren is allowed only as the last token.
                            throw lang::IllegalArgumentException();
                    break;
                    default:
                        throw lang::IllegalArgumentException();
                }
            }
            break;
            case svString:
            case svSingleRef:
            case svDoubleRef:
            case svExternalSingleRef:
            case svExternalDoubleRef:
            {
                ScTokenRef pNew(p->Clone());
                aRefTokens.push_back(pNew);
            }
            break;
            default:
                throw lang::IllegalArgumentException();
        }
    }

    if (aRefTokens.empty())
        return xResult;

    shrinkToDataRange(m_pDocument, aRefTokens);

    xResult.set(new ScChart2DataSequence(m_pDocument, this, std::move(aRefTokens), m_bIncludeHiddenCells));
    return xResult;
}

// XRangeXMLConversion ---------------------------------------------------

OUString SAL_CALL ScChart2DataProvider::convertRangeToXML( const OUString& sRangeRepresentation )
{
    OUString aRet;
    if (!m_pDocument)
        return aRet;

    if (sRangeRepresentation.isEmpty())
        // Empty data range is allowed.
        return aRet;

    vector<ScTokenRef> aRefTokens;
    const sal_Unicode cSep = ScCompiler::GetNativeSymbolChar(ocSep);
    ScRefTokenHelper::compileRangeRepresentation(
        aRefTokens, sRangeRepresentation, m_pDocument, cSep, m_pDocument->GetGrammar(), true);
    if (aRefTokens.empty())
        throw lang::IllegalArgumentException();

    Tokens2RangeStringXML converter(m_pDocument);
    converter = ::std::for_each(aRefTokens.begin(), aRefTokens.end(), converter);
    converter.getString(aRet);

    return aRet;
}

OUString SAL_CALL ScChart2DataProvider::convertRangeFromXML( const OUString& sXMLRange )
{
    if (!m_pDocument)
    {
        // #i74062# When loading flat XML, this is called before the referenced sheets are in the document,
        // so the conversion has to take place directly with the strings, without looking up the sheets.

        OUStringBuffer sRet;
        sal_Int32 nOffset = 0;
        while( nOffset >= 0 )
        {
            OUString sToken;
            ScRangeStringConverter::GetTokenByOffset( sToken, sXMLRange, nOffset );
            if( nOffset >= 0 )
            {
                // convert one address (remove dots)

                OUString aUIString(sToken);

                sal_Int32 nIndex = ScRangeStringConverter::IndexOf( sToken, ':', 0 );
                if ( nIndex >= 0 && nIndex < aUIString.getLength() - 1 &&
                        aUIString[nIndex + 1] == '.' )
                    aUIString = aUIString.replaceAt( nIndex + 1, 1, "" );

                if ( aUIString[0] == '.' )
                    aUIString = aUIString.copy( 1 );

                if( !sRet.isEmpty() )
                    sRet.append( ';' );
                sRet.append( aUIString );
            }
        }

        return sRet.makeStringAndClear();
    }

    OUString aRet;
    ScRangeStringConverter::GetStringFromXMLRangeString(aRet, sXMLRange, m_pDocument);
    return aRet;
}

// DataProvider XPropertySet -------------------------------------------------

uno::Reference< beans::XPropertySetInfo> SAL_CALL
ScChart2DataProvider::getPropertySetInfo()
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( m_aPropSet.getPropertyMap() );
    return aRef;
}

void SAL_CALL ScChart2DataProvider::setPropertyValue(
        const OUString& rPropertyName, const uno::Any& rValue)
{
    if ( rPropertyName != SC_UNONAME_INCLUDEHIDDENCELLS )
        throw beans::UnknownPropertyException();

    if ( !(rValue >>= m_bIncludeHiddenCells))
        throw lang::IllegalArgumentException();

}

uno::Any SAL_CALL ScChart2DataProvider::getPropertyValue(
        const OUString& rPropertyName)
{
    uno::Any aRet;
    if ( rPropertyName == SC_UNONAME_INCLUDEHIDDENCELLS )
        aRet <<= m_bIncludeHiddenCells;
    else if (rPropertyName == SC_UNONAME_USE_INTERNAL_DATA_PROVIDER)
    {
        // This is a read-only property.
        aRet <<= m_pDocument->PastingDrawFromOtherDoc();
    }
    else
        throw beans::UnknownPropertyException();
    return aRet;
}

void SAL_CALL ScChart2DataProvider::addPropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener>& /*xListener*/)
{
    OSL_FAIL( "Not yet implemented" );
}

void SAL_CALL ScChart2DataProvider::removePropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener>& /*rListener*/)
{
    OSL_FAIL( "Not yet implemented" );
}

void SAL_CALL ScChart2DataProvider::addVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener>& /*rListener*/)
{
    OSL_FAIL( "Not yet implemented" );
}

void SAL_CALL ScChart2DataProvider::removeVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener>& /*rListener*/ )
{
    OSL_FAIL( "Not yet implemented" );
}

// DataSource ================================================================

ScChart2DataSource::ScChart2DataSource( ScDocument* pDoc)
    : m_pDocument( pDoc)
{
    if ( m_pDocument )
        m_pDocument->AddUnoObject( *this);
}

ScChart2DataSource::~ScChart2DataSource()
{
    SolarMutexGuard g;

    if ( m_pDocument )
        m_pDocument->RemoveUnoObject( *this);
}

void ScChart2DataSource::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint)
{
    if ( rHint.GetId() == SfxHintId::Dying )
    {
        m_pDocument = nullptr;
    }
}

uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence> > SAL_CALL
ScChart2DataSource::getDataSequences()
{
    SolarMutexGuard aGuard;
    return comphelper::containerToSequence(m_aLabeledSequences);
}

void ScChart2DataSource::AddLabeledSequence(const uno::Reference < chart2::data::XLabeledDataSequence >& xNew)
{
    m_aLabeledSequences.push_back(xNew);
}

// DataSequence ==============================================================

ScChart2DataSequence::Item::Item() :
    mfValue(0.0), mbIsValue(false)
{
    ::rtl::math::setNan(&mfValue);
}

ScChart2DataSequence::HiddenRangeListener::HiddenRangeListener(ScChart2DataSequence& rParent) :
    mrParent(rParent)
{
}

ScChart2DataSequence::HiddenRangeListener::~HiddenRangeListener()
{
}

void ScChart2DataSequence::HiddenRangeListener::notify()
{
    mrParent.setDataChangedHint(true);
}

ScChart2DataSequence::ScChart2DataSequence( ScDocument* pDoc,
        const uno::Reference < chart2::data::XDataProvider >& xDP,
        vector<ScTokenRef>&& rTokens,
        bool bIncludeHiddenCells )
    : m_bIncludeHiddenCells( bIncludeHiddenCells)
    , m_nObjectId( 0 )
    , m_pDocument( pDoc)
    , m_aTokens(std::move(rTokens))
    , m_xDataProvider( xDP)
    , m_aPropSet(lcl_GetDataSequencePropertyMap())
    , m_bGotDataChangedHint(false)
    , m_bExtDataRebuildQueued(false)
    , mbTimeBased(false)
    , mnTimeBasedStart(0)
    , mnTimeBasedEnd(0)
    , mnCurrentTab(0)
{
    if ( m_pDocument )
    {
        m_pDocument->AddUnoObject( *this);
        m_nObjectId = m_pDocument->GetNewUnoId();
    }
    // FIXME: real implementation of identifier and it's mapping to ranges.
    // Reuse ScChartListener?

    // BM: don't use names of named ranges but the UI range strings
//  String  aStr;
//  rRangeList->Format( aStr, ScRefFlags::RANGE_ABS_3D, m_pDocument );
//    m_aIdentifier = aStr;

//      m_aIdentifier = "ID_";
//      static sal_Int32 nID = 0;
//      m_aIdentifier += OUString::valueOf( ++nID);
}

ScChart2DataSequence::~ScChart2DataSequence()
{
    SolarMutexGuard g;

    if ( m_pDocument )
    {
        m_pDocument->RemoveUnoObject( *this);
        if (m_pHiddenListener)
        {
            ScChartListenerCollection* pCLC = m_pDocument->GetChartListenerCollection();
            if (pCLC)
                pCLC->EndListeningHiddenRange(m_pHiddenListener.get());
        }
        StopListeningToAllExternalRefs();
    }

    m_pValueListener.reset();
}

void ScChart2DataSequence::RefChanged()
{
    if( m_pValueListener && !m_aValueListeners.empty() )
    {
        m_pValueListener->EndListeningAll();

        if( m_pDocument )
        {
            ScChartListenerCollection* pCLC = nullptr;
            if (m_pHiddenListener)
            {
                pCLC = m_pDocument->GetChartListenerCollection();
                if (pCLC)
                    pCLC->EndListeningHiddenRange(m_pHiddenListener.get());
            }

            for (const auto& rxToken : m_aTokens)
            {
                ScRange aRange;
                if (!ScRefTokenHelper::getRangeFromToken(aRange, rxToken, ScAddress()))
                    continue;

                m_pDocument->StartListeningArea(aRange, false, m_pValueListener.get());
                if (pCLC)
                    pCLC->StartListeningHiddenRange(aRange, m_pHiddenListener.get());
            }
        }
    }
}

void ScChart2DataSequence::BuildDataCache()
{
    m_bExtDataRebuildQueued = false;

    if (!m_aDataArray.empty())
        return;

    StopListeningToAllExternalRefs();

    ::std::vector<sal_Int32> aHiddenValues;
    sal_Int32 nDataCount = 0;

    for (const auto& rxToken : m_aTokens)
    {
        if (ScRefTokenHelper::isExternalRef(rxToken))
        {
            nDataCount += FillCacheFromExternalRef(rxToken);
        }
        else
        {
            ScRange aRange;
            if (!ScRefTokenHelper::getRangeFromToken(aRange, rxToken, ScAddress()))
                continue;

            SCCOL nLastCol = -1;
            SCROW nLastRow = -1;
            for (SCTAB nTab = aRange.aStart.Tab(); nTab <= aRange.aEnd.Tab(); ++nTab)
            {
                for (SCCOL nCol = aRange.aStart.Col(); nCol <= aRange.aEnd.Col(); ++nCol)
                {
                    for (SCROW nRow = aRange.aStart.Row(); nRow <= aRange.aEnd.Row(); ++nRow)
                    {
                        bool bColHidden = m_pDocument->ColHidden(nCol, nTab, nullptr, &nLastCol);
                        bool bRowHidden = m_pDocument->RowHidden(nRow, nTab, nullptr, &nLastRow);

                        if (bColHidden || bRowHidden)
                        {
                            // hidden cell
                            aHiddenValues.push_back(nDataCount-1);

                            if( !m_bIncludeHiddenCells )
                                continue;
                        }

                        Item aItem;

                        ScAddress aAdr(nCol, nRow, nTab);
                        aItem.maString = m_pDocument->GetString(aAdr);

                        ScRefCellValue aCell(*m_pDocument, aAdr);
                        switch (aCell.meType)
                        {
                            case CELLTYPE_VALUE:
                                aItem.mfValue = aCell.getValue();
                                aItem.mbIsValue = true;
                            break;
                            case CELLTYPE_FORMULA:
                            {
                                ScFormulaCell* pFCell = aCell.mpFormula;
                                FormulaError nErr = pFCell->GetErrCode();
                                if (nErr != FormulaError::NONE)
                                    break;

                                if (pFCell->IsValue())
                                {
                                    aItem.mfValue = pFCell->GetValue();
                                    aItem.mbIsValue = true;
                                }
                            }
                            break;
                            case CELLTYPE_EDIT:
                            case CELLTYPE_NONE:
                            case CELLTYPE_STRING:
                            default:
                                ; // do nothing
                        }

                        aItem.mAddress = ScAddress(nCol, nRow, nTab);

                        m_aDataArray.push_back(aItem);
                        ++nDataCount;
                    }
                }
            }
        }
    }

    // convert the hidden cell list to sequence.
    m_aHiddenValues.realloc(aHiddenValues.size());
    std::copy(
        aHiddenValues.begin(), aHiddenValues.end(), m_aHiddenValues.begin());

    // Clear the data series cache when the array is re-built.
    m_aMixedDataCache.realloc(0);
}

void ScChart2DataSequence::RebuildDataCache()
{
    if (!m_bExtDataRebuildQueued)
    {
        m_aDataArray.clear();
        m_pDocument->BroadcastUno(ScHint(SfxHintId::ScDataChanged, ScAddress()));
        m_bExtDataRebuildQueued = true;
        m_bGotDataChangedHint = true;
    }
}

sal_Int32 ScChart2DataSequence::FillCacheFromExternalRef(const ScTokenRef& pToken)
{
    ScExternalRefManager* pRefMgr = m_pDocument->GetExternalRefManager();
    ScRange aRange;
    if (!ScRefTokenHelper::getRangeFromToken(aRange, pToken, ScAddress(), true))
        return 0;

    sal_uInt16 nFileId = pToken->GetIndex();
    OUString aTabName = pToken->GetString().getString();
    ScExternalRefCache::TokenArrayRef pArray = pRefMgr->getDoubleRefTokens(nFileId, aTabName, aRange, nullptr);
    if (!pArray)
        // no external data exists for this range.
        return 0;

    // Start listening for this external document.
    ExternalRefListener* pExtRefListener = GetExtRefListener();
    pRefMgr->addLinkListener(nFileId, pExtRefListener);
    pExtRefListener->addFileId(nFileId);

    ScExternalRefCache::TableTypeRef pTable = pRefMgr->getCacheTable(nFileId, aTabName, false);
    sal_Int32 nDataCount = 0;
    FormulaTokenArrayPlainIterator aIter(*pArray);
    for (FormulaToken* p = aIter.First(); p; p = aIter.Next())
    {
        // Cached external range is always represented as a single
        // matrix token, although that might change in the future when
        // we introduce a new token type to store multi-table range
        // data.

        if (p->GetType() != svMatrix)
        {
            OSL_FAIL("Cached array is not a matrix token.");
            continue;
        }

        const ScMatrix* pMat = p->GetMatrix();
        SCSIZE nCSize, nRSize;
        pMat->GetDimensions(nCSize, nRSize);
        for (SCSIZE nC = 0; nC < nCSize; ++nC)
        {
            for (SCSIZE nR = 0; nR < nRSize; ++nR)
            {
                if (pMat->IsValue(nC, nR) || pMat->IsBoolean(nC, nR))
                {
                    Item aItem;

                    aItem.mbIsValue = true;
                    aItem.mfValue = pMat->GetDouble(nC, nR);

                    SvNumberFormatter* pFormatter = m_pDocument->GetFormatTable();
                    if (pFormatter)
                    {
                        const double fVal = aItem.mfValue;
                        Color* pColor = nullptr;
                        sal_uInt32 nFmt = 0;
                        if (pTable)
                        {
                            // Get the correct format index from the cache.
                            SCCOL nCol = aRange.aStart.Col() + static_cast<SCCOL>(nC);
                            SCROW nRow = aRange.aStart.Row() + static_cast<SCROW>(nR);
                            pTable->getCell(nCol, nRow, &nFmt);
                        }
                        pFormatter->GetOutputString(fVal, nFmt, aItem.maString, &pColor);
                    }

                    m_aDataArray.push_back(aItem);
                    ++nDataCount;
                }
                else if (pMat->IsStringOrEmpty(nC, nR))
                {
                    Item aItem;

                    aItem.mbIsValue = false;
                    aItem.maString = pMat->GetString(nC, nR).getString();

                    m_aDataArray.emplace_back(aItem);
                    ++nDataCount;
                }
            }
        }
    }
    return nDataCount;
}

void ScChart2DataSequence::UpdateTokensFromRanges(const ScRangeList& rRanges)
{
    if (!m_pRangeIndices.get())
        return;

    for ( size_t i = 0, nCount = rRanges.size(); i < nCount; ++i )
    {
        ScTokenRef pToken;
        const ScRange & rRange = rRanges[i];

        ScRefTokenHelper::getTokenFromRange(pToken, rRange);
        sal_uInt32 nOrigPos = (*m_pRangeIndices)[i];
        m_aTokens[nOrigPos] = pToken;
    }

    RefChanged();

    // any change of the range address is broadcast to value (modify) listeners
    if ( !m_aValueListeners.empty() )
        m_bGotDataChangedHint = true;
}

ScChart2DataSequence::ExternalRefListener* ScChart2DataSequence::GetExtRefListener()
{
    if (!m_pExtRefListener.get())
        m_pExtRefListener.reset(new ExternalRefListener(*this, m_pDocument));

    return m_pExtRefListener.get();
}

void ScChart2DataSequence::StopListeningToAllExternalRefs()
{
    if (!m_pExtRefListener.get())
        return;

    const std::unordered_set<sal_uInt16>& rFileIds = m_pExtRefListener->getAllFileIds();
    ScExternalRefManager* pRefMgr = m_pDocument->GetExternalRefManager();
    for (const auto& rFileId : rFileIds)
        pRefMgr->removeLinkListener(rFileId, m_pExtRefListener.get());

    m_pExtRefListener.reset();
}

void ScChart2DataSequence::CopyData(const ScChart2DataSequence& r)
{
    if (!m_pDocument)
    {
        OSL_FAIL("document instance is nullptr!?");
        return;
    }

    std::vector<Item> aDataArray(r.m_aDataArray);
    m_aDataArray.swap(aDataArray);

    m_aHiddenValues = r.m_aHiddenValues;
    m_aRole = r.m_aRole;

    if (r.m_pRangeIndices.get())
        m_pRangeIndices.reset(new vector<sal_uInt32>(*r.m_pRangeIndices));

    if (r.m_pExtRefListener.get())
    {
        // Re-register all external files that the old instance was
        // listening to.

        ScExternalRefManager* pRefMgr = m_pDocument->GetExternalRefManager();
        m_pExtRefListener.reset(new ExternalRefListener(*this, m_pDocument));
        const std::unordered_set<sal_uInt16>& rFileIds = r.m_pExtRefListener->getAllFileIds();
        for (const auto& rFileId : rFileIds)
        {
            pRefMgr->addLinkListener(rFileId, m_pExtRefListener.get());
            m_pExtRefListener->addFileId(rFileId);
        }
    }
}

void ScChart2DataSequence::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint)
{
    if ( dynamic_cast<const ScUpdateRefHint*>(&rHint) )
    {
        // Create a range list from the token list, have the range list
        // updated, and bring the change back to the token list.

        ScRangeList aRanges;
        m_pRangeIndices.reset(new vector<sal_uInt32>);
        vector<ScTokenRef>::const_iterator itrBeg = m_aTokens.begin(), itrEnd = m_aTokens.end();
        for (vector<ScTokenRef>::const_iterator itr = itrBeg ;itr != itrEnd; ++itr)
        {
            if (!ScRefTokenHelper::isExternalRef(*itr))
            {
                ScRange aRange;
                ScRefTokenHelper::getRangeFromToken(aRange, *itr, ScAddress());
                aRanges.push_back(aRange);
                sal_uInt32 nPos = distance(itrBeg, itr);
                m_pRangeIndices->push_back(nPos);
            }
        }

        OSL_ENSURE(m_pRangeIndices->size() == aRanges.size(),
                   "range list and range index list have different sizes.");

        unique_ptr<ScRangeList> pUndoRanges;
        if ( m_pDocument->HasUnoRefUndo() )
            pUndoRanges.reset(new ScRangeList(aRanges));

        const ScUpdateRefHint& rRef = static_cast<const ScUpdateRefHint&>(rHint);
        bool bChanged = aRanges.UpdateReference(
            rRef.GetMode(), m_pDocument, rRef.GetRange(), rRef.GetDx(), rRef.GetDy(), rRef.GetDz());

        if (bChanged)
        {
            OSL_ENSURE(m_pRangeIndices->size() == aRanges.size(),
                       "range list and range index list have different sizes after the reference update.");

            // Bring the change back from the range list to the token list.
            UpdateTokensFromRanges(aRanges);

            if (pUndoRanges)
                m_pDocument->AddUnoRefChange(m_nObjectId, *pUndoRanges);
        }
    }
    else if ( dynamic_cast<const ScUnoRefUndoHint*>(&rHint) )
    {
        const ScUnoRefUndoHint& rUndoHint = static_cast<const ScUnoRefUndoHint&>(rHint);

        do
        {
            if (rUndoHint.GetObjectId() != m_nObjectId)
                break;

            // The hint object provides the old ranges.  Restore the old state
            // from these ranges.

            if (!m_pRangeIndices.get() || m_pRangeIndices->empty())
            {
                OSL_FAIL(" faulty range indices");
                break;
            }

            const ScRangeList& rRanges = rUndoHint.GetRanges();

            size_t nCount = rRanges.size();
            if (nCount != m_pRangeIndices->size())
            {
                OSL_FAIL("range count and range index count differ.");
                break;
            }

            UpdateTokensFromRanges(rRanges);
        }
        while (false);
    }
    else
    {
        const SfxHintId nId = rHint.GetId();
        if ( nId ==SfxHintId::Dying )
        {
            m_pDocument = nullptr;
        }
        else if ( nId == SfxHintId::DataChanged )
        {
            // delayed broadcast as in ScCellRangesBase

            if ( m_bGotDataChangedHint && m_pDocument )
            {
                m_aDataArray.clear();
                lang::EventObject aEvent;
                aEvent.Source.set(static_cast<cppu::OWeakObject*>(this));

                if( m_pDocument )
                {
                    for (uno::Reference<util::XModifyListener> & xListener: m_aValueListeners)
                        m_pDocument->AddUnoListenerCall( xListener, aEvent );
                }

                m_bGotDataChangedHint = false;
            }
        }
        else if ( nId == SfxHintId::ScCalcAll )
        {
            // broadcast from DoHardRecalc - set m_bGotDataChangedHint
            // (SfxHintId::DataChanged follows separately)

            if ( !m_aValueListeners.empty() )
                m_bGotDataChangedHint = true;
        }
        else if (nId == SfxHintId::ScClearCache)
        {
            // necessary after import
            m_aDataArray.clear();
        }
    }
}

IMPL_LINK( ScChart2DataSequence, ValueListenerHdl, const SfxHint&, rHint, void )
{
    if ( m_pDocument && (rHint.GetId() == SfxHintId::ScDataChanged) )
    {
        //  This may be called several times for a single change, if several formulas
        //  in the range are notified. So only a flag is set that is checked when
        //  SfxHintId::DataChanged is received.

        setDataChangedHint(true);
    }
}

ScChart2DataSequence::ExternalRefListener::ExternalRefListener(
    ScChart2DataSequence& rParent, ScDocument* pDoc) :
    ScExternalRefManager::LinkListener(),
    mrParent(rParent),
    mpDoc(pDoc)
{
}

ScChart2DataSequence::ExternalRefListener::~ExternalRefListener()
{
    if (!mpDoc || mpDoc->IsInDtorClear())
        // The document is being destroyed.  Do nothing.
        return;

    // Make sure to remove all pointers to this object.
    mpDoc->GetExternalRefManager()->removeLinkListener(this);
}

void ScChart2DataSequence::ExternalRefListener::notify(sal_uInt16 nFileId, ScExternalRefManager::LinkUpdateType eType)
{
    switch (eType)
    {
        case ScExternalRefManager::LINK_MODIFIED:
        {
            if (maFileIds.count(nFileId))
                // We are listening to this external document.
                mrParent.RebuildDataCache();
        }
        break;
        case ScExternalRefManager::LINK_BROKEN:
            maFileIds.erase(nFileId);
        break;
    }
}

void ScChart2DataSequence::ExternalRefListener::addFileId(sal_uInt16 nFileId)
{
    maFileIds.insert(nFileId);
}

uno::Sequence< uno::Any> SAL_CALL ScChart2DataSequence::getData()
{
    SolarMutexGuard aGuard;
    if ( !m_pDocument)
        throw uno::RuntimeException();

    BuildDataCache();

    if (!m_aMixedDataCache.getLength())
    {
        // Build a cache for the 1st time...

        sal_Int32 nCount = m_aDataArray.size();
        m_aMixedDataCache.realloc(nCount);
        uno::Any* pArr = m_aMixedDataCache.getArray();
        for (const Item &rItem : m_aDataArray)
        {
            if (rItem.mbIsValue)
                *pArr <<= rItem.mfValue;
            else if (rItem.maString.isEmpty())
            {
                ScRefCellValue aCell(*m_pDocument, rItem.mAddress);
                if (aCell.isEmpty())
                   *pArr = uno::Any();
                else
                   *pArr <<= rItem.maString;
            }
            else
                *pArr <<= rItem.maString;
            ++pArr;
        }
    }
    return m_aMixedDataCache;
}

// XNumericalDataSequence --------------------------------------------------

uno::Sequence< double > SAL_CALL ScChart2DataSequence::getNumericalData()
{
    SolarMutexGuard aGuard;
    if ( !m_pDocument)
        throw uno::RuntimeException();

    BuildDataCache();

    double fNAN;
    ::rtl::math::setNan(&fNAN);

    sal_Int32 nCount = m_aDataArray.size();
    uno::Sequence<double> aSeq(nCount);
    double* pArr = aSeq.getArray();
    for (const Item& rItem : m_aDataArray)
    {
        *pArr = rItem.mbIsValue ? rItem.mfValue : fNAN;
        ++pArr;
    }

    return aSeq;
}

// XTextualDataSequence --------------------------------------------------

uno::Sequence< OUString > SAL_CALL ScChart2DataSequence::getTextualData()
{
    SolarMutexGuard aGuard;
    uno::Sequence<OUString> aSeq;
    if ( !m_pDocument )
        throw uno::RuntimeException();

    BuildDataCache();

    sal_Int32 nCount = m_aDataArray.size();
    if ( nCount > 0 )
    {
        aSeq =  uno::Sequence<OUString>(nCount);
        OUString* pArr = aSeq.getArray();
        for (const Item& rItem : m_aDataArray)
        {
            *pArr = rItem.maString;
            ++pArr;
        }
    }
    else if ( m_aTokens.front() )
    {
        if( m_aTokens.front()->GetType() == svString )
        {
            aSeq = uno::Sequence<OUString>(1);
            aSeq[0] = m_aTokens.front()->GetString().getString();
        }
    }

    return aSeq;
}

OUString SAL_CALL ScChart2DataSequence::getSourceRangeRepresentation()
{
    SolarMutexGuard aGuard;
    OUString aStr;
    OSL_ENSURE( m_pDocument, "No Document -> no SourceRangeRepresentation" );
    if (m_pDocument)
        lcl_convertTokensToString(aStr, m_aTokens, m_pDocument);

    return aStr;
}

namespace {

/**
 * This function object is used to accumulatively count the numbers of
 * columns and rows in all reference tokens.
 */
class AccumulateRangeSize
{
public:
    AccumulateRangeSize() :
        mnCols(0), mnRows(0) {}

    void operator() (const ScTokenRef& pToken)
    {
        ScRange r;
        bool bExternal = ScRefTokenHelper::isExternalRef(pToken);
        ScRefTokenHelper::getRangeFromToken(r, pToken, ScAddress(), bExternal);
        r.PutInOrder();
        mnCols += r.aEnd.Col() - r.aStart.Col() + 1;
        mnRows += r.aEnd.Row() - r.aStart.Row() + 1;
    }

    SCCOL getCols() const { return mnCols; }
    SCROW getRows() const { return mnRows; }
private:
    SCCOL mnCols;
    SCROW mnRows;
};

/**
 * This function object is used to generate label strings from a list of
 * reference tokens.
 */
class GenerateLabelStrings
{
public:
    GenerateLabelStrings(sal_Int32 nSize, chart2::data::LabelOrigin eOrigin, bool bColumn) :
        mpLabels(new Sequence<OUString>(nSize)),
        meOrigin(eOrigin),
        mnCount(0),
        mbColumn(bColumn) {}

    void operator() (const ScTokenRef& pToken)
    {
        bool bExternal = ScRefTokenHelper::isExternalRef(pToken);
        ScRange aRange;
        ScRefTokenHelper::getRangeFromToken(aRange, pToken, ScAddress(), bExternal);
        OUString* pArr = mpLabels->getArray();
        if (mbColumn)
        {
            for (SCCOL nCol = aRange.aStart.Col(); nCol <= aRange.aEnd.Col(); ++nCol)
            {
                if ( meOrigin != chart2::data::LabelOrigin_LONG_SIDE)
                {
                    OUString aString = ScResId(STR_COLUMN);
                    aString += " ";
                    ScAddress aPos( nCol, 0, 0 );
                    OUString aColStr(aPos.Format(ScRefFlags::COL_VALID));
                    aString += aColStr;
                    pArr[mnCount] = aString;
                }
                else //only indices for categories
                    pArr[mnCount] = OUString::number( mnCount+1 );
                ++mnCount;
            }
        }
        else
        {
            for (sal_Int32 nRow = aRange.aStart.Row(); nRow <= aRange.aEnd.Row(); ++nRow)
            {
                if (meOrigin != chart2::data::LabelOrigin_LONG_SIDE)
                {
                    OUString aString = ScResId(STR_ROW) +
                                       " " + OUString::number( nRow+1 );
                    pArr[mnCount] = aString;
                }
                else //only indices for categories
                    pArr[mnCount] = OUString::number( mnCount+1 );
                ++mnCount;
            }
        }
    }

    const Sequence<OUString>& getLabels() const { return *mpLabels; }

private:
    shared_ptr< Sequence<OUString> >    mpLabels;
    chart2::data::LabelOrigin           meOrigin;
    sal_Int32                           mnCount;
    bool                                mbColumn;
};

}

uno::Sequence< OUString > SAL_CALL ScChart2DataSequence::generateLabel(chart2::data::LabelOrigin eOrigin)
{
    SolarMutexGuard aGuard;
    if ( !m_pDocument)
        throw uno::RuntimeException();

    // Determine the total size of all ranges.
    AccumulateRangeSize func;
    func = ::std::for_each(m_aTokens.begin(), m_aTokens.end(), func);
    SCCOL nCols = func.getCols();
    SCROW nRows = func.getRows();

    // Determine whether this is column-major or row-major.
    bool bColumn = true;
    if ((eOrigin == chart2::data::LabelOrigin_SHORT_SIDE) ||
        (eOrigin == chart2::data::LabelOrigin_LONG_SIDE))
    {
        if (nRows > nCols)
        {
            bColumn = eOrigin == chart2::data::LabelOrigin_SHORT_SIDE;
        }
        else if (nCols > nRows)
        {
            bColumn = eOrigin != chart2::data::LabelOrigin_SHORT_SIDE;
        }
        else
            return Sequence<OUString>();
    }

    // Generate label strings based on the info so far.
    sal_Int32 nCount = bColumn ? nCols : nRows;
    GenerateLabelStrings genLabels(nCount, eOrigin, bColumn);
    genLabels = ::std::for_each(m_aTokens.begin(), m_aTokens.end(), genLabels);
    Sequence<OUString> aSeq = genLabels.getLabels();

    return aSeq;
}

namespace {

sal_uInt32 getDisplayNumberFormat(const ScDocument* pDoc, const ScAddress& rPos)
{
    sal_uInt32 nFormat = pDoc->GetNumberFormat(rPos); // original format from cell.
    return nFormat;
}

}

::sal_Int32 SAL_CALL ScChart2DataSequence::getNumberFormatKeyByIndex( ::sal_Int32 nIndex )
{
    SolarMutexGuard aGuard;
    BuildDataCache();

    if (nIndex == -1)
    {
        // return format of first non-empty cell
        // TODO: use nicer heuristic
        for (const Item& rItem : m_aDataArray)
        {
            ScRefCellValue aCell(*m_pDocument, rItem.mAddress);
            if (!aCell.isEmpty() && aCell.hasNumeric())
            {
                return static_cast<sal_Int32>(getDisplayNumberFormat(m_pDocument, rItem.mAddress));
            }
        }

        // we could not find a non-empty cell
        return 0;
    }

    if (nIndex < 0 || nIndex >= static_cast<sal_Int32>(m_aDataArray.size()))
    {
        SAL_WARN("sc.ui", "Passed invalid index to getNumberFormatKeyByIndex(). Will return default value '0'.");
        return 0;
    }

    return static_cast<sal_Int32>(getDisplayNumberFormat(m_pDocument, m_aDataArray.at(nIndex).mAddress));
}

// XCloneable ================================================================

uno::Reference< util::XCloneable > SAL_CALL ScChart2DataSequence::createClone()
{
    SolarMutexGuard aGuard;

    // Clone tokens.
    vector<ScTokenRef> aTokensNew;
    aTokensNew.reserve(m_aTokens.size());
    for (const auto& rxToken : m_aTokens)
    {
        ScTokenRef p(rxToken->Clone());
        aTokensNew.push_back(p);
    }

    rtl::Reference<ScChart2DataSequence> p(new ScChart2DataSequence(m_pDocument, m_xDataProvider, std::move(aTokensNew), m_bIncludeHiddenCells));
    p->CopyData(*this);
    uno::Reference< util::XCloneable > xClone(p.get());

    return xClone;
}

// XModifyBroadcaster ========================================================

void SAL_CALL ScChart2DataSequence::addModifyListener( const uno::Reference< util::XModifyListener >& aListener )
{
    // like ScCellRangesBase::addModifyListener
    SolarMutexGuard aGuard;
    if (m_aTokens.empty())
        return;

    ScRangeList aRanges;
    ScRefTokenHelper::getRangeListFromTokens(aRanges, m_aTokens, ScAddress());
    m_aValueListeners.emplace_back( aListener );

    if ( m_aValueListeners.size() == 1 )
    {
        if (!m_pValueListener)
            m_pValueListener.reset(new ScLinkListener( LINK( this, ScChart2DataSequence, ValueListenerHdl ) ));

        if (!m_pHiddenListener)
            m_pHiddenListener.reset(new HiddenRangeListener(*this));

        if( m_pDocument )
        {
            ScChartListenerCollection* pCLC = m_pDocument->GetChartListenerCollection();
            for (const auto& rxToken : m_aTokens)
            {
                ScRange aRange;
                if (!ScRefTokenHelper::getRangeFromToken(aRange, rxToken, ScAddress()))
                    continue;

                m_pDocument->StartListeningArea( aRange, false, m_pValueListener.get() );
                if (pCLC)
                    pCLC->StartListeningHiddenRange(aRange, m_pHiddenListener.get());
            }
        }

        acquire();  // don't lose this object (one ref for all listeners)
    }
}

void SAL_CALL ScChart2DataSequence::removeModifyListener( const uno::Reference< util::XModifyListener >& aListener )
{
    // like ScCellRangesBase::removeModifyListener

    SolarMutexGuard aGuard;
    if (m_aTokens.empty())
        return;

    rtl::Reference<ScChart2DataSequence> aSelfHold(this);      // in case the listeners have the last ref

    sal_uInt16 nCount = m_aValueListeners.size();
    for ( sal_uInt16 n=nCount; n--; )
    {
        uno::Reference<util::XModifyListener>& rObj = m_aValueListeners[n];
        if ( rObj == aListener )
        {
            m_aValueListeners.erase( m_aValueListeners.begin() + n );

            if ( m_aValueListeners.empty() )
            {
                if (m_pValueListener)
                    m_pValueListener->EndListeningAll();

                if (m_pHiddenListener.get() && m_pDocument)
                {
                    ScChartListenerCollection* pCLC = m_pDocument->GetChartListenerCollection();
                    if (pCLC)
                        pCLC->EndListeningHiddenRange(m_pHiddenListener.get());
                }

                release();      // release the ref for the listeners
            }

            break;
        }
    }
}

// DataSequence XPropertySet -------------------------------------------------

uno::Reference< beans::XPropertySetInfo> SAL_CALL
ScChart2DataSequence::getPropertySetInfo()
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( m_aPropSet.getPropertyMap() );
    return aRef;
}

void SAL_CALL ScChart2DataSequence::setPropertyValue(
        const OUString& rPropertyName, const uno::Any& rValue)
{
    if ( rPropertyName == SC_UNONAME_ROLE )
    {
        if ( !(rValue >>= m_aRole))
            throw lang::IllegalArgumentException();
    }
    else if ( rPropertyName == SC_UNONAME_INCLUDEHIDDENCELLS )
    {
        bool bOldValue = m_bIncludeHiddenCells;
        if ( !(rValue >>= m_bIncludeHiddenCells))
            throw lang::IllegalArgumentException();
        if( bOldValue != m_bIncludeHiddenCells )
            m_aDataArray.clear();//data array is dirty now
    }
    else if( rPropertyName == "TimeBased" )
    {
        bool bTimeBased = mbTimeBased;
        rValue>>= bTimeBased;
        mbTimeBased = bTimeBased;
    }
    else
        throw beans::UnknownPropertyException();
    // TODO: support optional properties
}

uno::Any SAL_CALL ScChart2DataSequence::getPropertyValue(const OUString& rPropertyName)
{
    uno::Any aRet;
    if ( rPropertyName == SC_UNONAME_ROLE )
        aRet <<= m_aRole;
    else if ( rPropertyName == SC_UNONAME_INCLUDEHIDDENCELLS )
        aRet <<= m_bIncludeHiddenCells;
    else if ( rPropertyName == SC_UNONAME_HIDDENVALUES )
    {
        // This property is read-only thus cannot be set externally via
        // setPropertyValue(...).
        BuildDataCache();
        aRet <<= m_aHiddenValues;
    }
    else if (rPropertyName == SC_UNONAME_TIME_BASED)
    {
        aRet <<= mbTimeBased;
    }
    else if (rPropertyName == SC_UNONAME_HAS_STRING_LABEL)
    {
        // Read-only property.  It returns whether or not the label value is a
        // direct user input, rather than an indirect reference.
        bool bHasStringLabel = false;
        if (m_aTokens.size() == 1)
        {
            const formula::FormulaToken& rToken = *m_aTokens[0];
            bHasStringLabel = rToken.GetType() == formula::svString;
        }
        aRet <<= bHasStringLabel;
    }
    else
        throw beans::UnknownPropertyException();
    // TODO: support optional properties
    return aRet;
}

void SAL_CALL ScChart2DataSequence::addPropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener>& /*xListener*/)
{
    // FIXME: real implementation
    OSL_FAIL( "Not yet implemented" );
}

void SAL_CALL ScChart2DataSequence::removePropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener>& /*rListener*/)
{
    // FIXME: real implementation
    OSL_FAIL( "Not yet implemented" );
}

void SAL_CALL ScChart2DataSequence::addVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener>& /*rListener*/)
{
    // FIXME: real implementation
    OSL_FAIL( "Not yet implemented" );
}

void SAL_CALL ScChart2DataSequence::removeVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener>& /*rListener*/)
{
    // FIXME: real implementation
    OSL_FAIL( "Not yet implemented" );
}

void ScChart2DataSequence::setDataChangedHint(bool b)
{
    m_bGotDataChangedHint = b;
}

sal_Bool ScChart2DataSequence::switchToNext(sal_Bool bWrap)
{
    if(!mbTimeBased)
        return true;

    if(mnCurrentTab >= mnTimeBasedEnd)
    {
        if(bWrap)
            setToPointInTime(0);
        return false;
    }

    for(const auto& rxToken : m_aTokens)
    {
        if (rxToken->GetType() != svDoubleRef)
            continue;

        ScComplexRefData& rData = *rxToken->GetDoubleRef();
        ScSingleRefData& s = rData.Ref1;
        ScSingleRefData& e = rData.Ref2;

        s.IncTab(1);
        e.IncTab(1);
    }

    ++mnCurrentTab;

    RebuildDataCache();

    return true;
}

void ScChart2DataSequence::setRange(sal_Int32 nStart, sal_Int32 nEnd)
{
    mnTimeBasedStart = nStart;
    mnTimeBasedEnd = nEnd;
    mnCurrentTab = mnTimeBasedStart;
}

sal_Bool ScChart2DataSequence::setToPointInTime(sal_Int32 nPoint)
{
    if(nPoint > mnTimeBasedEnd - mnTimeBasedStart)
        return false;

    SCTAB nTab = mnTimeBasedStart + nPoint;
    for(const auto& rxToken : m_aTokens)
    {
        if (rxToken->GetType() != svDoubleRef)
            continue;

        ScComplexRefData& rData = *rxToken->GetDoubleRef();
        ScSingleRefData& s = rData.Ref1;
        ScSingleRefData& e = rData.Ref2;

        s.SetAbsTab(nTab);
        e.SetAbsTab(nTab);
    }

    mnCurrentTab = nTab;

    RebuildDataCache();

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
