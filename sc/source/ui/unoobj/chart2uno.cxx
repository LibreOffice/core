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

#include <sal/config.h>

#include <utility>

#include "chart2uno.hxx"
#include "miscuno.hxx"
#include "document.hxx"
#include "formulacell.hxx"
#include "chartpos.hxx"
#include "unonames.hxx"
#include "globstr.hrc"
#include "convuno.hxx"
#include "rangeutl.hxx"
#include "hints.hxx"
#include "unoreflist.hxx"
#include "compiler.hxx"
#include "reftokenhelper.hxx"
#include "chartlis.hxx"
#include "tokenuno.hxx"
#include "docsh.hxx"
#include "cellvalue.hxx"
#include "tokenarray.hxx"
#include "scmatrix.hxx"
#include "ScChart2DataSequence.hxx"
#include "ScChart2DataSource.hxx"
#include <brdcst.hxx>

#include <formula/opcode.hxx>
#include <svl/sharedstring.hxx>

#include <sfx2/objsh.hxx>
#include <vcl/svapp.hxx>

#include <com/sun/star/beans/UnknownPropertyException.hpp>
#include <com/sun/star/chart2/data/LabeledDataSequence.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/text/XText.hpp>
#include <comphelper/extract.hxx>
#include <comphelper/processfactory.hxx>

#include <rtl/math.hxx>
#include <boost/checked_delete.hpp>

SC_SIMPLE_SERVICE_INFO( ScChart2DataProvider, "ScChart2DataProvider",
        "com.sun.star.chart2.data.DataProvider")


using namespace ::com::sun::star;
using namespace ::formula;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::std::unique_ptr;
using ::std::vector;
using ::std::list;
using ::std::distance;
using ::std::unary_function;
using ::boost::shared_ptr;

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

template< typename T >
::com::sun::star::uno::Sequence< T > lcl_VectorToSequence(
    const ::std::vector< T > & rCont )
{
    ::com::sun::star::uno::Sequence< T > aResult( rCont.size());
    ::std::copy( rCont.begin(), rCont.end(), aResult.getArray());
    return aResult;
}

struct lcl_appendTableNumber : public ::std::unary_function< SCTAB, void >
{
    lcl_appendTableNumber( OUStringBuffer & rBuffer ) :
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

OUString lcl_createTableNumberList( const ::std::list< SCTAB > & rTableList )
{
    OUStringBuffer aBuffer;
    ::std::for_each( rTableList.begin(), rTableList.end(), lcl_appendTableNumber( aBuffer ));
    // remove last trailing ' '
    if( !aBuffer.isEmpty() )
        aBuffer.setLength( aBuffer.getLength() - 1 );
    return aBuffer.makeStringAndClear();
}

uno::Reference< frame::XModel > lcl_GetXModel( ScDocument * pDoc )
{
    uno::Reference< frame::XModel > xModel;
    SfxObjectShell * pObjSh( pDoc ? pDoc->GetDocumentShell() : 0 );
    if( pObjSh )
        xModel.set( pObjSh->GetModel());
    return xModel;
}

struct TokenTable : boost::noncopyable
{
    SCROW mnRowCount;
    SCCOL mnColCount;
    vector<FormulaToken*> maTokens;

    void init( SCCOL nColCount, SCROW nRowCount )
    {
        mnColCount = nColCount;
        mnRowCount = nRowCount;
        maTokens.reserve(mnColCount*mnRowCount);
    }
    void clear()
    {
        std::for_each(maTokens.begin(), maTokens.end(), boost::checked_deleter<FormulaToken>());
    }

    void push_back( FormulaToken* pToken )
    {
        maTokens.push_back( pToken );
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

    vector<ScTokenRef>* getColRanges(SCCOL nCol) const;
    vector<ScTokenRef>* getRowRanges(SCROW nRow) const;
    vector<ScTokenRef>* getAllRanges() const;
};

vector<ScTokenRef>* TokenTable::getColRanges(SCCOL nCol) const
{
    if (nCol >= mnColCount)
        return NULL;
    if( mnRowCount<=0 )
        return NULL;

    unique_ptr< vector<ScTokenRef> > pTokens(new vector<ScTokenRef>);
    sal_uInt32 nLast = getIndex(nCol, mnRowCount-1);
    for (sal_uInt32 i = getIndex(nCol, 0); i <= nLast; ++i)
    {
        FormulaToken* p = maTokens[i];
        if (!p)
            continue;

        ScTokenRef pCopy(p->Clone());
        ScRefTokenHelper::join(*pTokens, pCopy, ScAddress());
    }
    return pTokens.release();
}

vector<ScTokenRef>* TokenTable::getRowRanges(SCROW nRow) const
{
    if (nRow >= mnRowCount)
        return NULL;
    if( mnColCount<=0 )
        return NULL;

    unique_ptr< vector<ScTokenRef> > pTokens(new vector<ScTokenRef>);
    sal_uInt32 nLast = getIndex(mnColCount-1, nRow);
    for (sal_uInt32 i = getIndex(0, nRow); i <= nLast; i += mnRowCount)
    {
        FormulaToken* p = maTokens[i];
        if (!p)
            continue;

        ScTokenRef p2(p->Clone());
        ScRefTokenHelper::join(*pTokens, p2, ScAddress());
    }
    return pTokens.release();
}

vector<ScTokenRef>* TokenTable::getAllRanges() const
{
    unique_ptr< vector<ScTokenRef> > pTokens(new vector<ScTokenRef>);
    sal_uInt32 nStop = mnColCount*mnRowCount;
    for (sal_uInt32 i = 0; i < nStop; i++)
    {
        FormulaToken* p = maTokens[i];
        if (!p)
            continue;

        ScTokenRef p2(p->Clone());
        ScRefTokenHelper::join(*pTokens, p2, ScAddress());
    }
    return pTokens.release();
}

typedef std::map<sal_uInt32, FormulaToken*> FormulaTokenMap;
typedef std::map<sal_uInt32, FormulaTokenMap*> FormulaTokenMapMap;

class Chart2PositionMap
{
public:
    Chart2PositionMap(SCCOL nColCount, SCROW nRowCount,
                      bool bFillRowHeader, bool bFillColumnHeader, FormulaTokenMapMap& rCols,
                      ScDocument* pDoc );
    ~Chart2PositionMap();

    SCCOL getDataColCount() const { return mnDataColCount; }
    SCROW getDataRowCount() const { return mnDataRowCount; }

    vector<ScTokenRef>* getLeftUpperCornerRanges() const;
    vector<ScTokenRef>* getAllColHeaderRanges() const;
    vector<ScTokenRef>* getAllRowHeaderRanges() const;

    vector<ScTokenRef>* getColHeaderRanges(SCCOL nChartCol) const;
    vector<ScTokenRef>* getRowHeaderRanges(SCROW nChartRow) const;

    vector<ScTokenRef>* getDataColRanges(SCCOL nCol) const;
    vector<ScTokenRef>* getDataRowRanges(SCROW nRow) const;

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

    if( nHeaderColCount || nHeaderRowCount )
    {
        const SCCOL nInitialHeaderColCount = nHeaderColCount;
        //check whether there is more than one text column or row that should be added to the headers
        SCROW nSmallestValueRowIndex = nAllRowCount;
        bool bFoundValues = false;
        bool bFoundAnything = false;
        FormulaTokenMapMap::const_iterator it1 = rCols.begin();
        for (SCCOL nCol = 0; nCol < nAllColCount; ++nCol)
        {
            if (it1 != rCols.end() && nCol>=nHeaderColCount)
            {
                bool bFoundValuesInRow = false;
                FormulaTokenMap* pCol = it1->second;
                FormulaTokenMap::const_iterator it2 = pCol->begin();
                for (SCROW nRow = 0; !bFoundValuesInRow && nRow < nSmallestValueRowIndex && it2 != pCol->end(); ++nRow)
                {
                    FormulaToken* pToken = it2->second;
                    if (pToken && nRow>=nHeaderRowCount)
                    {
                        ScRange aRange;
                        bool bExternal = false;
                        StackVar eType = pToken->GetType();
                        if( eType==svExternal || eType==svExternalSingleRef || eType==svExternalDoubleRef || eType==svExternalName )
                            bExternal = true;//lllll todo correct?
                        ScTokenRef pSharedToken(pToken->Clone());
                        ScRefTokenHelper::getRangeFromToken(aRange, pSharedToken, ScAddress(), bExternal);
                        SCCOL nCol1=0, nCol2=0;
                        SCROW nRow1=0, nRow2=0;
                        SCTAB nTab1=0, nTab2=0;
                        aRange.GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
                        if (pDoc && pDoc->HasValueData( nCol1, nRow1, nTab1 ))
                        {
                            bFoundValuesInRow = bFoundValues = bFoundAnything = true;
                            nSmallestValueRowIndex = std::min( nSmallestValueRowIndex, nRow );
                        }
                        if( !bFoundAnything )
                        {
                            if (pDoc && pDoc->HasData( nCol1, nRow1, nTab1 ) )
                                bFoundAnything = true;
                        }
                    }
                    ++it2;
                }
                if(!bFoundValues && nHeaderColCount>0)
                    nHeaderColCount++;
            }
            ++it1;
        }
        if( bFoundAnything )
        {
            if(nHeaderRowCount>0)
            {
                if( bFoundValues )
                    nHeaderRowCount = nSmallestValueRowIndex;
                else if( nAllRowCount>1 )
                    nHeaderRowCount = nAllRowCount-1;
            }
        }
        else //if the cells are completely empty, just use single header rows and columns
            nHeaderColCount = nInitialHeaderColCount;
    }

    mnDataColCount = nAllColCount - nHeaderColCount;
    mnDataRowCount = nAllRowCount - nHeaderRowCount;

    maLeftUpperCorner.init(nHeaderColCount,nHeaderRowCount);
    maColHeaders.init(mnDataColCount,nHeaderRowCount);
    maRowHeaders.init(nHeaderColCount,mnDataRowCount);
    maData.init(mnDataColCount,mnDataRowCount);

    FormulaTokenMapMap::const_iterator it1 = rCols.begin();
    for (SCCOL nCol = 0; nCol < nAllColCount; ++nCol)
    {
        if (it1 != rCols.end())
        {
            FormulaTokenMap* pCol = it1->second;
            FormulaTokenMap::const_iterator it2 = pCol->begin();
            for (SCROW nRow = 0; nRow < nAllRowCount; ++nRow)
            {
                FormulaToken* pToken = NULL;
                if (it2 != pCol->end())
                {
                    pToken = it2->second;
                    ++it2;
                }

                if( nCol < nHeaderColCount )
                {
                    if( nRow < nHeaderRowCount )
                        maLeftUpperCorner.push_back(pToken);
                    else
                        maRowHeaders.push_back(pToken);
                }
                else if( nRow < nHeaderRowCount )
                    maColHeaders.push_back(pToken);
                else
                    maData.push_back(pToken);
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

vector<ScTokenRef>* Chart2PositionMap::getLeftUpperCornerRanges() const
{
    return maLeftUpperCorner.getAllRanges();
}
vector<ScTokenRef>* Chart2PositionMap::getAllColHeaderRanges() const
{
    return maColHeaders.getAllRanges();
}
vector<ScTokenRef>* Chart2PositionMap::getAllRowHeaderRanges() const
{
    return maRowHeaders.getAllRanges();
}
vector<ScTokenRef>* Chart2PositionMap::getColHeaderRanges(SCCOL nCol) const
{
    return maColHeaders.getColRanges( nCol);
}
vector<ScTokenRef>* Chart2PositionMap::getRowHeaderRanges(SCROW nRow) const
{
    return maRowHeaders.getRowRanges( nRow);
}

vector<ScTokenRef>* Chart2PositionMap::getDataColRanges(SCCOL nCol) const
{
    return maData.getColRanges( nCol);
}

vector<ScTokenRef>* Chart2PositionMap::getDataRowRanges(SCROW nRow) const
{
    return maData.getRowRanges( nRow);
}

/**
 * Designed to be a drop-in replacement for ScChartPositioner, in order to
 * handle external references.
 */
class Chart2Positioner : boost::noncopyable
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
    Chart2Positioner(ScDocument* pDoc, const vector<ScTokenRef>& rRefTokens) :
        mrRefTokens(rRefTokens),
        mpPositionMap(NULL),
        meGlue(GLUETYPE_NA),
        mnStartCol(0),
        mnStartRow(0),
        mpDoc(pDoc),
        mbColHeaders(false),
        mbRowHeaders(false),
        mbDummyUpperLeft(false)
    {
    }

    ~Chart2Positioner()
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
    boost::scoped_ptr<Chart2PositionMap> mpPositionMap;
    GlueType    meGlue;
    SCCOL       mnStartCol;
    SCROW       mnStartRow;
    ScDocument* mpDoc;
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
    for (vector<ScTokenRef>::const_iterator itr = mrRefTokens.begin(), itrEnd = mrRefTokens.end()
         ; itr != itrEnd; ++itr)
    {
        ScRefTokenHelper::getDoubleRefDataFromToken(aData, *itr);
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
    for (vector<ScTokenRef>::const_iterator itr = mrRefTokens.begin(), itrEnd = mrRefTokens.end();
          itr != itrEnd; ++itr)
    {
        ScComplexRefData aData;
        ScRefTokenHelper::getDoubleRefDataFromToken(aData, *itr);
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

    if (mpPositionMap.get())
        return;

    glueState();

    bool bNoGlue = (meGlue == GLUETYPE_NONE);
    unique_ptr<FormulaTokenMapMap> pCols(new FormulaTokenMapMap);
    FormulaTokenMap* pCol = NULL;
    SCROW nNoGlueRow = 0;
    for (vector<ScTokenRef>::const_iterator itr = mrRefTokens.begin(), itrEnd = mrRefTokens.end();
          itr != itrEnd; ++itr)
    {
        const ScTokenRef& pToken = *itr;

        bool bExternal = ScRefTokenHelper::isExternalRef(pToken);
        sal_uInt16 nFileId = bExternal ? pToken->GetIndex() : 0;
        svl::SharedString aTabName = svl::SharedString::getEmptyString();
        if (bExternal)
            aTabName = pToken->GetString();

        ScComplexRefData aData;
        if( !ScRefTokenHelper::getDoubleRefDataFromToken(aData, *itr) )
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
                FormulaTokenMapMap::const_iterator it = pCols->find(nInsCol);
                if (it == pCols->end())
                {
                    pCol = new FormulaTokenMap;
                    (*pCols)[ nInsCol ] = pCol;
                }
                else
                    pCol = it->second;

                sal_uInt32 nInsRow = static_cast<sal_uInt32>(bNoGlue ? nNoGlueRow : nRow1);
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

                    if (pCol->find(nInsRow) == pCol->end())
                    {
                        if (bExternal)
                            (*pCol)[ nInsRow ] = new ScExternalSingleRefToken(nFileId, aTabName, aCellData);
                        else
                            (*pCol)[ nInsRow ] = new ScSingleRefToken(aCellData);
                    }
                }
            }
        }
        nNoGlueRow += nRow2 - nRow1 + 1;
    }

    bool bFillRowHeader = mbRowHeaders;
    bool bFillColumnHeader = mbColHeaders;

    SCSIZE nAllColCount = static_cast<SCSIZE>(pCols->size());
    SCSIZE nAllRowCount = 0;
    if (!pCols->empty())
    {
        pCol = pCols->begin()->second;
        if (mbDummyUpperLeft)
            if (pCol->find(0) == pCol->end())
                (*pCol)[ 0 ] = NULL;        // Dummy fuer Beschriftung
        nAllRowCount = static_cast<SCSIZE>(pCol->size());
    }

    if( nAllColCount!=0 && nAllRowCount!=0 )
    {
        if (bNoGlue)
        {
            FormulaTokenMap* pFirstCol = pCols->begin()->second;
            for (FormulaTokenMap::const_iterator it1 = pFirstCol->begin(); it1 != pFirstCol->end(); ++it1)
            {
                sal_uInt32 nKey = it1->first;
                for (FormulaTokenMapMap::const_iterator it2 = pCols->begin(); it2 != pCols->end(); ++it2)
                {
                    pCol = it2->second;
                    if (pCol->find(nKey) == pCol->end())
                        (*pCol)[ nKey ] = NULL;
                }
            }
        }
    }
    mpPositionMap.reset(
        new Chart2PositionMap(
            static_cast<SCCOL>(nAllColCount), static_cast<SCROW>(nAllRowCount),
            bFillRowHeader, bFillColumnHeader, *pCols, mpDoc));

    // Destroy all column instances.
    for (FormulaTokenMapMap::const_iterator it = pCols->begin(); it != pCols->end(); ++it)
    {
        pCol = it->second;
        delete pCol;
    }
}

/**
 * Function object to convert a list of tokens into a string form suitable
 * for ODF export.  In ODF, a range is expressed as
 *
 *   (start cell address):(end cell address)
 *
 * and each address doesn't include any '$' symbols.
 */
class Tokens2RangeStringXML : public unary_function<ScTokenRef, void>
{
public:
    Tokens2RangeStringXML(ScDocument* pDoc) :
        mpRangeStr(new OUStringBuffer),
        mpDoc(pDoc),
        mcRangeSep(' '),
        mcAddrSep(':'),
        mbFirst(true)
    {
    }

    Tokens2RangeStringXML(const Tokens2RangeStringXML& r) :
        mpRangeStr(r.mpRangeStr),
        mpDoc(r.mpDoc),
        mcRangeSep(r.mcRangeSep),
        mcAddrSep(r.mcAddrSep),
        mbFirst(r.mbFirst)
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
        ScCompiler aCompiler(mpDoc, ScAddress(0,0,0));
        aCompiler.SetGrammar(FormulaGrammar::GRAM_ENGLISH);
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
    ScDocument*         mpDoc;
    sal_Unicode         mcRangeSep;
    sal_Unicode         mcAddrSep;
    bool                mbFirst;
};

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
    const SfxSimpleHint* pSimpleHint = dynamic_cast<const SfxSimpleHint*>(&rHint);
    if ( pSimpleHint && pSimpleHint->GetId() == SFX_HINT_DYING )
    {
        m_pDocument = NULL;
    }
}

sal_Bool SAL_CALL ScChart2DataProvider::createDataSourcePossible( const uno::Sequence< beans::PropertyValue >& aArguments )
    throw (uno::RuntimeException, std::exception)
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

Reference< chart2::data::XLabeledDataSequence > lcl_createLabeledDataSequenceFromTokens(
    unique_ptr< vector< ScTokenRef > > && pValueTokens, unique_ptr< vector< ScTokenRef > > && pLabelTokens,
    ScDocument* pDoc, const Reference< chart2::data::XDataProvider >& xDP, bool bIncludeHiddenCells )
{
    Reference< chart2::data::XLabeledDataSequence >  xResult;
    bool bHasValues = pValueTokens.get() && !pValueTokens->empty();
    bool bHasLabel = pLabelTokens.get() && !pLabelTokens->empty();
    if( bHasValues || bHasLabel )
    {
        try
        {
            Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
            if ( xContext.is() )
            {
                xResult.set( chart2::data::LabeledDataSequence::create(xContext), uno::UNO_QUERY_THROW );
            }
            if ( bHasValues )
            {
                Reference< chart2::data::XDataSequence > xSeq( new ScChart2DataSequence( pDoc, xDP, pValueTokens.release(), bIncludeHiddenCells ) );
                xResult->setValues( xSeq );
            }
            if ( bHasLabel )
            {
                Reference< chart2::data::XDataSequence > xLabelSeq( new ScChart2DataSequence( pDoc, xDP, pLabelTokens.release(), bIncludeHiddenCells ) );
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
            SCROW nCornerRowCount=1, SCCOL nCornerColumnCount=1)
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
    for (itr = rRefTokens.begin(); itr != itrEnd; ++itr)
    {
        pToken = *itr;
        switch (pToken->GetType())
        {
            case svSingleRef:
            case svExternalSingleRef:
            {
                const ScSingleRefData& rData = *pToken->GetSingleRef();
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
                const ScComplexRefData& rData = *pToken->GetDoubleRef();
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

class ShrinkRefTokenToDataRange : std::unary_function<ScTokenRef, void>
{
    ScDocument* mpDoc;
public:
    ShrinkRefTokenToDataRange(ScDocument* pDoc) : mpDoc(pDoc) {}
    void operator() (ScTokenRef& rRef)
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
    throw( lang::IllegalArgumentException, uno::RuntimeException, std::exception)
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
                    eSource = (static_cast< chart::ChartDataRowSource >( nSource ));
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
        for(vector<ScTokenRef>::iterator itr = aRefTokens.begin(),
                itrEnd = aRefTokens.end(); itr != itrEnd; ++itr)
        {
            if ((*itr)->GetType() != svDoubleRef)
                continue;

            ScComplexRefData& rData = *(*itr)->GetDoubleRef();
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
        lcl_addUpperLeftCornerIfMissing(aRefTokens); //#i90669#

    bool bColHeaders = (bOrientCol ? bLabel : bCategories );
    bool bRowHeaders = (bOrientCol ? bCategories : bLabel );

    Chart2Positioner aChPositioner(m_pDocument, aRefTokens);
    aChPositioner.setHeaders(bColHeaders, bRowHeaders);

    const Chart2PositionMap* pChartMap = aChPositioner.getPositionMap();
    if (!pChartMap)
        // No chart position map instance.  Bail out.
        return xResult;

    ScChart2DataSource* pDS = NULL;
    ::std::list< Reference< chart2::data::XLabeledDataSequence > > aSeqs;

    // Fill Categories
    if( bCategories )
    {
        unique_ptr< vector<ScTokenRef> > pValueTokens;
        if (bOrientCol)
            pValueTokens.reset(pChartMap->getAllRowHeaderRanges());
        else
            pValueTokens.reset(pChartMap->getAllColHeaderRanges());

        unique_ptr< vector<ScTokenRef> > pLabelTokens(
                pChartMap->getLeftUpperCornerRanges());

        Reference< chart2::data::XLabeledDataSequence > xCategories = lcl_createLabeledDataSequenceFromTokens(
            std::move(pValueTokens), std::move(pLabelTokens), m_pDocument, this, m_bIncludeHiddenCells ); //ownership of pointers is transferred!
        if ( xCategories.is() )
        {
            aSeqs.push_back( xCategories );
        }
    }

    // Fill Serieses (values and label)
    sal_Int32 nCount = bOrientCol ? pChartMap->getDataColCount() : pChartMap->getDataRowCount();
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        unique_ptr< vector<ScTokenRef> > pValueTokens;
        unique_ptr< vector<ScTokenRef> > pLabelTokens;
        if (bOrientCol)
        {
            pValueTokens.reset(pChartMap->getDataColRanges(static_cast<SCCOL>(i)));
            pLabelTokens.reset(pChartMap->getColHeaderRanges(static_cast<SCCOL>(i)));
        }
        else
        {
            pValueTokens.reset(pChartMap->getDataRowRanges(static_cast<SCROW>(i)));
            pLabelTokens.reset(pChartMap->getRowHeaderRanges(static_cast<SCROW>(i)));
        }
        Reference< chart2::data::XLabeledDataSequence > xChartSeries = lcl_createLabeledDataSequenceFromTokens(
            std::move(pValueTokens), std::move(pLabelTokens), m_pDocument, this, m_bIncludeHiddenCells ); //ownership of pointers is transferred!
        if ( xChartSeries.is() && xChartSeries->getValues().is() && xChartSeries->getValues()->getData().getLength() )
        {
            aSeqs.push_back( xChartSeries );
        }
    }

    pDS = new ScChart2DataSource(m_pDocument);
    ::std::list< Reference< chart2::data::XLabeledDataSequence > >::iterator aItr( aSeqs.begin() );
    ::std::list< Reference< chart2::data::XLabeledDataSequence > >::iterator aEndItr( aSeqs.end() );

    //reorder labeled sequences according to aSequenceMapping
    ::std::vector< Reference< chart2::data::XLabeledDataSequence > > aSeqVector;
    while(aItr != aEndItr)
    {
        aSeqVector.push_back(*aItr);
        ++aItr;
    }

    ::std::map< sal_Int32, Reference< chart2::data::XLabeledDataSequence > > aSequenceMap;
    for( sal_Int32 nNewIndex = 0; nNewIndex < aSequenceMapping.getLength(); nNewIndex++ )
    {
        // note: assuming that the values in the sequence mapping are always non-negative
        ::std::vector< Reference< chart2::data::XLabeledDataSequence > >::size_type nOldIndex( static_cast< sal_uInt32 >( aSequenceMapping[nNewIndex] ) );
        if( nOldIndex < aSeqVector.size() )
        {
            pDS->AddLabeledSequence( aSeqVector[nOldIndex] );
            aSeqVector[nOldIndex] = 0;
        }
    }

    ::std::vector< Reference< chart2::data::XLabeledDataSequence > >::iterator aVectorItr( aSeqVector.begin() );
    ::std::vector< Reference< chart2::data::XLabeledDataSequence > >::iterator aVectorEndItr( aSeqVector.end() );
    while(aVectorItr != aVectorEndItr)
    {
        Reference< chart2::data::XLabeledDataSequence > xSeq( *aVectorItr );
        if ( xSeq.is() )
        {
            pDS->AddLabeledSequence( xSeq );
        }
        ++aVectorItr;
    }

    xResult.set( pDS );
    return xResult;
}

namespace
{

/**
 * Function object to create a list of table numbers from a token list.
 */
class InsertTabNumber : public unary_function<ScTokenRef, void>
{
public:
    InsertTabNumber() :
        mpTabNumList(new list<SCTAB>())
    {
    }

    InsertTabNumber(const InsertTabNumber& r) :
        mpTabNumList(r.mpTabNumList)
    {
    }

    void operator() (const ScTokenRef& pToken) const
    {
        if (!ScRefTokenHelper::isRef(pToken))
            return;

        const ScSingleRefData& r = *pToken->GetSingleRef();
        mpTabNumList->push_back(r.Tab());
    }

    void getList(list<SCTAB>& rList)
    {
        mpTabNumList->swap(rList);
    }
private:
    shared_ptr< list<SCTAB> > mpTabNumList;
};

class RangeAnalyzer
{
public:
    RangeAnalyzer();
    void initRangeAnalyzer( const vector<ScTokenRef>& rTokens );
    void analyzeRange( sal_Int32& rnDataInRows, sal_Int32& rnDataInCols,
            bool& rbRowSourceAmbiguous ) const;
    bool inSameSingleRow( RangeAnalyzer& rOther );
    bool inSameSingleColumn( RangeAnalyzer& rOther );
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

    vector<ScTokenRef>::const_iterator itr = rTokens.begin(), itrEnd = rTokens.end();
    for (; itr != itrEnd ; ++itr)
    {
        ScTokenRef aRefToken = *itr;
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

bool RangeAnalyzer::inSameSingleRow( RangeAnalyzer& rOther )
{
    if( mnStartRow==rOther.mnStartRow &&
        mnRowCount==1 && rOther.mnRowCount==1 )
        return true;
    return false;
}

bool RangeAnalyzer::inSameSingleColumn( RangeAnalyzer& rOther )
{
    if( mnStartColumn==rOther.mnStartColumn &&
        mnColumnCount==1 && rOther.mnColumnCount==1 )
        return true;
    return false;
}

} //end anonymous namespace

uno::Sequence< beans::PropertyValue > SAL_CALL ScChart2DataProvider::detectArguments(
    const uno::Reference< chart2::data::XDataSource >& xDataSource )
    throw (uno::RuntimeException, std::exception)
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
            return lcl_VectorToSequence( aResult );

        sal_Int32 nDataInRows = 0;
        sal_Int32 nDataInCols = 0;
        bool bRowSourceAmbiguous = false;

        Sequence< Reference< chart2::data::XLabeledDataSequence > > aSequences( xDataSource->getDataSequences());
        const sal_Int32 nCount( aSequences.getLength());
        RangeAnalyzer aPrevLabel,aPrevValues;
        for( sal_Int32 nIdx=0; nIdx<nCount; ++nIdx )
        {
            Reference< chart2::data::XLabeledDataSequence > xLS(aSequences[nIdx]);
            if( xLS.is() )
            {
                bool bThisIsCategories = false;
                if(!bHasCategories)
                {
                    Reference< beans::XPropertySet > xSeqProp( xLS->getValues(), uno::UNO_QUERY );
                    OUString aRole;
                    if( xSeqProp.is() && (xSeqProp->getPropertyValue("Role") >>= aRole) &&
                        aRole == "categories" )
                        bThisIsCategories = bHasCategories = true;
                }

                RangeAnalyzer aLabel,aValues;
                // label
                Reference< chart2::data::XDataSequence > xLabel( xLS->getLabel());
                if( xLabel.is())
                {
                    bFirstCellAsLabel = true;
                    vector<ScTokenRef> aTokens;
                    const sal_Unicode cSep = ScCompiler::GetNativeSymbolChar(ocSep);
                    ScRefTokenHelper::compileRangeRepresentation(
                        aTokens, xLabel->getSourceRangeRepresentation(), m_pDocument, cSep, m_pDocument->GetGrammar(), true);
                    aLabel.initRangeAnalyzer(aTokens);
                    vector<ScTokenRef>::const_iterator itr = aTokens.begin(), itrEnd = aTokens.end();
                    for (; itr != itrEnd; ++itr)
                    {
                        ScRefTokenHelper::join(aAllTokens, *itr, ScAddress());
                        if(!bThisIsCategories)
                            ScRefTokenHelper::join(aAllSeriesLabelTokens, *itr, ScAddress());
                    }
                    if(bThisIsCategories)
                        bHasCategoriesLabels=true;
                }
                // values
                Reference< chart2::data::XDataSequence > xValues( xLS->getValues());
                if( xValues.is())
                {
                    vector<ScTokenRef> aTokens;
                    const sal_Unicode cSep = ScCompiler::GetNativeSymbolChar(ocSep);
                    ScRefTokenHelper::compileRangeRepresentation(
                        aTokens, xValues->getSourceRangeRepresentation(), m_pDocument, cSep, m_pDocument->GetGrammar(), true);
                    aValues.initRangeAnalyzer(aTokens);
                    vector<ScTokenRef>::const_iterator itr = aTokens.begin(), itrEnd = aTokens.end();
                    for (; itr != itrEnd; ++itr)
                    {
                        ScRefTokenHelper::join(aAllTokens, *itr, ScAddress());
                        if(bThisIsCategories)
                            ScRefTokenHelper::join(aAllCategoriesValuesTokens, *itr, ScAddress());
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
            eRowSource = ( nDataInRows > 0
                           ? chart::ChartDataRowSource_ROWS
                           : chart::ChartDataRowSource_COLUMNS );
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
        list<SCTAB> aTableNumList;
        InsertTabNumber func;
        func = ::std::for_each(aAllTokens.begin(), aAllTokens.end(), func);
        func.getList(aTableNumList);
        aResult.push_back(
            beans::PropertyValue( OUString("TableNumberList"), -1,
                                  uno::makeAny( lcl_createTableNumberList( aTableNumList ) ),
                                  beans::PropertyState_DIRECT_VALUE ));
    }

    // DataRowSource (calculated before)
    if( bRowSourceDetected )
    {
        aResult.push_back(
            beans::PropertyValue( OUString("DataRowSource"), -1,
                                  uno::makeAny( eRowSource ), beans::PropertyState_DIRECT_VALUE ));
    }

    // HasCategories
    if( bRowSourceDetected )
    {
        aResult.push_back(
            beans::PropertyValue( OUString("HasCategories"), -1,
                                  uno::makeAny( bHasCategories ), beans::PropertyState_DIRECT_VALUE ));
    }

    // FirstCellAsLabel
    if( bRowSourceDetected )
    {
        aResult.push_back(
            beans::PropertyValue( OUString("FirstCellAsLabel"), -1,
                                  uno::makeAny( bFirstCellAsLabel ), beans::PropertyState_DIRECT_VALUE ));
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
    ScChart2DataSequence::convertTokensToString(sRangeRep, aAllTokens, m_pDocument);

    // add cell range property
    aResult.push_back(
        beans::PropertyValue( OUString("CellRangeRepresentation"), -1,
                              uno::makeAny( sRangeRep ), beans::PropertyState_DIRECT_VALUE ));

    //Sequence Mapping
    bool bSequencesReordered = true;//todo detect this above or detect this sequence mapping cheaper ...
    if( bSequencesReordered && bRowSourceDetected )
    {
        bool bDifferentIndexes = false;

        std::vector< sal_Int32 > aSequenceMappingVector;

        uno::Reference< chart2::data::XDataSource > xCompareDataSource;
        try
        {
            xCompareDataSource.set( this->createDataSource( lcl_VectorToSequence( aResult ) ) );
        }
        catch( const lang::IllegalArgumentException & )
        {
            // creation of data source to compare didn't work, so we cannot
            // create a sequence mapping
        }

        if( xDataSource.is() && xCompareDataSource.is() )
        {
            uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence> > aOldSequences(
                xCompareDataSource->getDataSequences() );
            uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > aNewSequences(
                xDataSource->getDataSequences());

            OUString aOldLabel;
            OUString aNewLabel;
            OUString aOldValues;
            OUString aNewValues;
            OUString aEmpty;

            for( sal_Int32 nNewIndex = 0; nNewIndex < aNewSequences.getLength(); nNewIndex++ )
            {
                uno::Reference< chart2::data::XLabeledDataSequence> xNew( aNewSequences[nNewIndex] );
                for( sal_Int32 nOldIndex = 0; nOldIndex < aOldSequences.getLength(); nOldIndex++ )
                {
                    uno::Reference< chart2::data::XLabeledDataSequence> xOld( aOldSequences[nOldIndex] );

                    if( xOld.is() && xNew.is() )
                    {
                        aOldLabel = aNewLabel = aOldValues = aNewValues = aEmpty;
                        if( xOld.is() && xOld->getLabel().is() )
                            aOldLabel = xOld->getLabel()->getSourceRangeRepresentation();
                        if( xNew.is() && xNew->getLabel().is() )
                            aNewLabel = xNew->getLabel()->getSourceRangeRepresentation();
                        if( xOld.is() && xOld->getValues().is() )
                            aOldValues = xOld->getValues()->getSourceRangeRepresentation();
                        if( xNew.is() && xNew->getValues().is() )
                            aNewValues = xNew->getValues()->getSourceRangeRepresentation();

                        if( aOldLabel.equals(aNewLabel)
                            && ( aOldValues.equals(aNewValues) ) )
                        {
                            if( nOldIndex!=nNewIndex )
                                bDifferentIndexes = true;
                            aSequenceMappingVector.push_back(nOldIndex);
                            break;
                        }
                    }
                }
            }
        }

        if( bDifferentIndexes && !aSequenceMappingVector.empty() )
        {
            aResult.push_back(
                beans::PropertyValue( OUString("SequenceMapping"), -1,
                    uno::makeAny( lcl_VectorToSequence(aSequenceMappingVector) )
                    , beans::PropertyState_DIRECT_VALUE ));
        }
    }

    return lcl_VectorToSequence( aResult );
}

sal_Bool SAL_CALL ScChart2DataProvider::createDataSequenceByRangeRepresentationPossible( const OUString& aRangeRepresentation )
    throw (uno::RuntimeException, std::exception)
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
    throw (lang::IllegalArgumentException,
           uno::RuntimeException, std::exception)
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

    // ScChart2DataSequence manages the life cycle of pRefTokens.
    vector<ScTokenRef>* pRefTokens = new vector<ScTokenRef>();
    pRefTokens->swap(aRefTokens);
    xResult.set(new ScChart2DataSequence(m_pDocument, this, pRefTokens, m_bIncludeHiddenCells));

    return xResult;
}

uno::Reference<chart2::data::XDataSequence> SAL_CALL
ScChart2DataProvider::createDataSequenceByValueArray(
    const OUString& /*aRole*/, const OUString& /*aRangeRepresentation*/ )
    throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
{
    return uno::Reference<chart2::data::XDataSequence>();
}

uno::Reference< sheet::XRangeSelection > SAL_CALL ScChart2DataProvider::getRangeSelection()
    throw (uno::RuntimeException, std::exception)
{
    uno::Reference< sheet::XRangeSelection > xResult;

    uno::Reference< frame::XModel > xModel( lcl_GetXModel( m_pDocument ));
    if( xModel.is())
        xResult.set( xModel->getCurrentController(), uno::UNO_QUERY );

    return xResult;
}

sal_Bool SAL_CALL ScChart2DataProvider::createDataSequenceByFormulaTokensPossible(
    const Sequence<sheet::FormulaToken>& aTokens )
        throw (uno::RuntimeException, std::exception)
{
    if (aTokens.getLength() <= 0)
        return false;

    ScTokenArray aCode;
    if (!ScTokenConversion::ConvertToTokenArray(*m_pDocument, aCode, aTokens))
        return false;

    sal_uInt16 n = aCode.GetLen();
    if (!n)
        return false;

    const formula::FormulaToken* pFirst = aCode.First();
    const formula::FormulaToken* pLast = aCode.GetArray()[n-1];
    for (const formula::FormulaToken* p = aCode.First(); p; p = aCode.Next())
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

Reference<chart2::data::XDataSequence> SAL_CALL
ScChart2DataProvider::createDataSequenceByFormulaTokens(
    const Sequence<sheet::FormulaToken>& aTokens )
        throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception)
{
    Reference<chart2::data::XDataSequence> xResult;
    if (aTokens.getLength() <= 0)
        return xResult;

    ScTokenArray aCode;
    if (!ScTokenConversion::ConvertToTokenArray(*m_pDocument, aCode, aTokens))
        return xResult;

    sal_uInt16 n = aCode.GetLen();
    if (!n)
        return xResult;

    vector<ScTokenRef> aRefTokens;
    const formula::FormulaToken* pFirst = aCode.First();
    const formula::FormulaToken* pLast = aCode.GetArray()[n-1];
    for (const formula::FormulaToken* p = aCode.First(); p; p = aCode.Next())
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

    // ScChart2DataSequence manages the life cycle of pRefTokens.
    vector<ScTokenRef>* pRefTokens = new vector<ScTokenRef>();
    pRefTokens->swap(aRefTokens);
    xResult.set(new ScChart2DataSequence(m_pDocument, this, pRefTokens, m_bIncludeHiddenCells));
    return xResult;
}

// XRangeXMLConversion ---------------------------------------------------

OUString SAL_CALL ScChart2DataProvider::convertRangeToXML( const OUString& sRangeRepresentation )
    throw ( uno::RuntimeException, lang::IllegalArgumentException, std::exception )
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
    throw ( uno::RuntimeException, lang::IllegalArgumentException, std::exception )
{
    const sal_Unicode cSep = ' ';
    const sal_Unicode cQuote = '\'';

    if (!m_pDocument)
    {
        // #i74062# When loading flat XML, this is called before the referenced sheets are in the document,
        // so the conversion has to take place directly with the strings, without looking up the sheets.

        OUStringBuffer sRet;
        sal_Int32 nOffset = 0;
        while( nOffset >= 0 )
        {
            OUString sToken;
            ScRangeStringConverter::GetTokenByOffset( sToken, sXMLRange, nOffset, cSep, cQuote );
            if( nOffset >= 0 )
            {
                // convert one address (remove dots)

                OUString aUIString(sToken);

                sal_Int32 nIndex = ScRangeStringConverter::IndexOf( sToken, ':', 0, cQuote );
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
ScChart2DataProvider::getPropertySetInfo() throw( uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( m_aPropSet.getPropertyMap() );
    return aRef;
}

void SAL_CALL ScChart2DataProvider::setPropertyValue(
        const OUString& rPropertyName, const uno::Any& rValue)
            throw( beans::UnknownPropertyException,
                    beans::PropertyVetoException,
                    lang::IllegalArgumentException,
                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    if ( rPropertyName == SC_UNONAME_INCLUDEHIDDENCELLS )
    {
        if ( !(rValue >>= m_bIncludeHiddenCells))
            throw lang::IllegalArgumentException();
    }
    else
        throw beans::UnknownPropertyException();
}

uno::Any SAL_CALL ScChart2DataProvider::getPropertyValue(
        const OUString& rPropertyName)
            throw( beans::UnknownPropertyException,
                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
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
            throw( beans::UnknownPropertyException,
                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    OSL_FAIL( "Not yet implemented" );
}

void SAL_CALL ScChart2DataProvider::removePropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener>& /*rListener*/)
            throw( beans::UnknownPropertyException,
                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    OSL_FAIL( "Not yet implemented" );
}

void SAL_CALL ScChart2DataProvider::addVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener>& /*rListener*/)
            throw( beans::UnknownPropertyException,
                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    OSL_FAIL( "Not yet implemented" );
}

void SAL_CALL ScChart2DataProvider::removeVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener>& /*rListener*/ )
            throw( beans::UnknownPropertyException,
                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    OSL_FAIL( "Not yet implemented" );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
