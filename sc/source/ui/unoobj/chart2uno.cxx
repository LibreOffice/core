/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: chart2uno.cxx,v $
 * $Revision: 1.11 $
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

#include "chart2uno.hxx"
#include "miscuno.hxx"
#include "document.hxx"
#include "unoguard.hxx"
#include "cell.hxx"
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

#include <sfx2/objsh.hxx>
#include <tools/table.hxx>

#include <com/sun/star/beans/UnknownPropertyException.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/text/XText.hpp>
#include <comphelper/extract.hxx>

#include <vector>
#include <list>
#include <rtl/math.hxx>

SC_SIMPLE_SERVICE_INFO( ScChart2DataProvider, "ScChart2DataProvider",
        "com.sun.star.chart2.data.DataProvider")
SC_SIMPLE_SERVICE_INFO( ScChart2DataSource, "ScChart2DataSource",
        "com.sun.star.chart2.data.DataSource")
SC_SIMPLE_SERVICE_INFO( ScChart2LabeledDataSequence, "ScChart2LabeledDataSequence",
        "com.sun.star.chart2.data.LabeledDataSequence")
SC_SIMPLE_SERVICE_INFO( ScChart2DataSequence, "ScChart2DataSequence",
        "com.sun.star.chart2.data.DataSequence")
#if USE_CHART2_EMPTYDATASEQUENCE
SC_SIMPLE_SERVICE_INFO( ScChart2EmptyDataSequence, "ScChart2EmptyDataSequence",
        "com.sun.star.chart2.data.DataSequence")
#endif

using namespace ::com::sun::star;
using namespace ::formula;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::std::auto_ptr;
using ::std::vector;
using ::std::list;
using ::std::distance;
using ::std::unary_function;
using ::std::hash_set;
using ::boost::shared_ptr;

namespace
{
const SfxItemPropertyMapEntry* lcl_GetDataProviderPropertyMap()
{
    static SfxItemPropertyMapEntry aDataProviderPropertyMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNONAME_INCLUDEHIDDENCELLS), 0,        &getBooleanCppuType(),                  0, 0 },
        {0,0,0,0,0,0}
    };
    return aDataProviderPropertyMap_Impl;
}

const SfxItemPropertyMapEntry* lcl_GetDataSequencePropertyMap()
{
    static SfxItemPropertyMapEntry aDataSequencePropertyMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNONAME_HIDDENVALUES), 0, &getCppuType((uno::Sequence<sal_Int32>*)0 ),                 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_ROLE), 0, &getCppuType((::com::sun::star::chart2::data::DataSequenceRole*)0),                  0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_INCLUDEHIDDENCELLS), 0,        &getBooleanCppuType(),                  0, 0 },
        {0,0,0,0,0,0}
    };
    return aDataSequencePropertyMap_Impl;
}

template< typename T >
::com::sun::star::uno::Sequence< T > lcl_VectorToSequence(
    const ::std::vector< T > & rCont )
{
    ::com::sun::star::uno::Sequence< T > aResult( rCont.size());
    ::std::copy( rCont.begin(), rCont.end(), aResult.getArray());
    return aResult;
}

::std::vector< ::rtl::OUString > lcl_getRangeRepresentationsFromDataSource(
    const uno::Reference< chart2::data::XDataSource > & xDataSource )
{
    ::std::vector< ::rtl::OUString > aResult;
    if( xDataSource.is())
    {
        uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > aSequences(
            xDataSource->getDataSequences());
        const sal_Int32 nCount( aSequences.getLength());
        for( sal_Int32 nIdx=0; nIdx<nCount; ++nIdx )
        {
            if( aSequences[nIdx].is() )
            {
                // first: label
                uno::Reference< chart2::data::XDataSequence > xSeq( aSequences[nIdx]->getLabel());
                if( xSeq.is())
                    aResult.push_back( xSeq->getSourceRangeRepresentation());
                // then: values
                xSeq.set( aSequences[nIdx]->getValues());
                if( xSeq.is())
                    aResult.push_back( xSeq->getSourceRangeRepresentation());
            }
        }
    }
    return aResult;
}

uno::Reference< chart2::data::XLabeledDataSequence > lcl_getCategoriesFromDataSource(
    const uno::Reference< chart2::data::XDataSource > & xDataSource )
{
    uno::Reference< chart2::data::XLabeledDataSequence > xResult;
    if( xDataSource.is())
    {
        uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > aSequences(
            xDataSource->getDataSequences());
        const sal_Int32 nCount( aSequences.getLength());
        for( sal_Int32 nIdx=0; nIdx<nCount; ++nIdx )
        {
            if( aSequences[nIdx].is() )
            {
                uno::Reference< beans::XPropertySet > xSeqProp( aSequences[nIdx]->getValues(), uno::UNO_QUERY );
                ::rtl::OUString aRole;
                if( xSeqProp.is() &&
                    (xSeqProp->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Role"))) >>= aRole) &&
                    aRole.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("categories")) )
                {
                    xResult.set( aSequences[nIdx] );
                    break;
                }
            }
        }
    }
    return xResult;
}

struct lcl_appendTableNumber : public ::std::unary_function< SCTAB, void >
{
    lcl_appendTableNumber( ::rtl::OUStringBuffer & rBuffer ) :
            m_rBuffer( rBuffer )
    {}
    void operator() ( SCTAB nTab )
    {
        // there is no append with SCTAB or sal_Int16
        m_rBuffer.append( static_cast< sal_Int32 >( nTab ));
        m_rBuffer.append( sal_Unicode( ' ' ));
    }
private:
    ::rtl::OUStringBuffer & m_rBuffer;
};

::rtl::OUString lcl_createTableNumberList( const ::std::list< SCTAB > & rTableList )
{
    ::rtl::OUStringBuffer aBuffer;
    ::std::for_each( rTableList.begin(), rTableList.end(), lcl_appendTableNumber( aBuffer ));
    // remove last trailing ' '
    if( aBuffer.getLength() > 0 )
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

uno::Reference< sheet::XSpreadsheetDocument > lcl_GetSpreadSheetDocument( ScDocument * pDoc )
{
    return uno::Reference< sheet::XSpreadsheetDocument >( lcl_GetXModel( pDoc ), uno::UNO_QUERY );
}

// ============================================================================

class Chart2PositionMap
{
public:
    Chart2PositionMap(SCCOL nColCount, SCROW nRowCount,
                      bool bColAdd, bool bRowAdd, Table& rCols);
    ~Chart2PositionMap();

    SCCOL getColCount() const { return static_cast<SCCOL>(maColHeaders.size()); }
    SCROW getRowCount() const { return static_cast<SCROW>(maRowHeaders.size()); }

    const FormulaToken* getColHeaderPosition(SCCOL nChartCol) const;
    const FormulaToken* getRowHeaderPosition(SCROW nChartRow) const;

    vector<ScSharedTokenRef>* getColRanges(SCCOL nCol) const;
    vector<ScSharedTokenRef>* getRowRanges(SCROW nRow) const;


private:
    sal_uInt32 getIndex(SCCOL nCol, SCROW nRow) const
    {
        return static_cast<sal_uInt32>(nCol*getRowCount() + nRow);
    }

private:
    vector<FormulaToken*> maRowHeaders;
    vector<FormulaToken*> maColHeaders;
    vector<FormulaToken*> maData;
};

Chart2PositionMap::Chart2PositionMap(SCCOL nColCount,  SCROW nRowCount,
                                     bool bColAdd, bool bRowAdd, Table& rCols)
{
    // bColAdd is true when the first column serves as a row header.  Likewise,
    // when bRowAdd is true the first row serves as a column header.

    maColHeaders.reserve(nColCount);
    maRowHeaders.reserve(nRowCount);
    maData.reserve(nColCount*nRowCount);

    Table* pCol = static_cast<Table*>(rCols.First());
    FormulaToken* pPos = static_cast<FormulaToken*>(pCol->First());

    if (bRowAdd)
        pPos = static_cast<FormulaToken*>(pCol->Next());

    if (bColAdd)
    {
        // 1st column as a row header.
        for (SCROW nRow = 0; nRow < nRowCount; ++nRow)
        {
            maRowHeaders.push_back(pPos);
            pPos = static_cast<FormulaToken*>(pCol->Next());
        }
        pCol = static_cast<Table*>(rCols.Next()); // move to the next column.
    }
    else
    {
        for (SCROW nRow = 0; nRow < nRowCount; ++nRow)
        {
            // Make a copy.
            maRowHeaders.push_back(pPos ? pPos->Clone() : NULL);
            pPos = static_cast<FormulaToken*>(pCol->Next());
        }
    }

    // Data in columns and in column headers.
    for (SCCOL nCol = 0; nCol < nColCount; ++nCol)
    {
        if (pCol)
        {
            pPos = static_cast<FormulaToken*>(pCol->First());
            if (bRowAdd)
            {
                // 1st row as a column header.
                maColHeaders.push_back(pPos);
                pPos = static_cast<FormulaToken*>(pCol->Next());
            }
            else
                // Duplicate the 1st cell as a column header.
                maColHeaders.push_back(pPos ? pPos->Clone() : NULL);

            for (SCROW nRow = 0; nRow < nRowCount; ++nRow)
            {
                maData.push_back(pPos);
                pPos = static_cast<FormulaToken*>(pCol->Next());
            }
        }
        else
        {
            // the entire column is empty.
            maColHeaders.push_back(NULL);
            for (SCROW nRow = 0; nRow < nRowCount; ++nRow)
                maData.push_back(NULL);
        }
        pCol = static_cast<Table*>(rCols.Next());
    }
}

namespace {

struct DeleteInstance : public unary_function<FormulaToken*, void>
{
    void operator() (FormulaToken* p) const
    {
        delete p;
    }
};

}

Chart2PositionMap::~Chart2PositionMap()
{
    for_each(maColHeaders.begin(), maColHeaders.end(), DeleteInstance());
    for_each(maRowHeaders.begin(), maRowHeaders.end(), DeleteInstance());
    for_each(maData.begin(), maData.end(), DeleteInstance());
}

const FormulaToken* Chart2PositionMap::getColHeaderPosition(SCCOL nCol) const
{
    if (nCol < getColCount())
        return maColHeaders[nCol];
    return NULL;
}
const FormulaToken* Chart2PositionMap::getRowHeaderPosition(SCROW nRow) const
{
    if (nRow < getRowCount())
        return maRowHeaders[nRow];
    return NULL;
}

vector<ScSharedTokenRef>* Chart2PositionMap::getColRanges(SCCOL nCol) const
{
    if (nCol >= getColCount())
        return NULL;

    auto_ptr< vector<ScSharedTokenRef> > pTokens(new vector<ScSharedTokenRef>);
    sal_uInt32 nStop = getIndex(nCol, getRowCount());
    for (sal_uInt32 i = getIndex(nCol, 0); i < nStop; ++i)
    {
        FormulaToken* p = maData[i];
        if (!p)
            continue;

        ScSharedTokenRef pCopy(static_cast<ScToken*>(p->Clone()));
        ScRefTokenHelper::join(*pTokens, pCopy);
    }
    return pTokens.release();
}

vector<ScSharedTokenRef>* Chart2PositionMap::getRowRanges(SCROW nRow) const
{
    SCROW nRowCount = getRowCount();
    if (nRow >= nRowCount)
        return NULL;

    auto_ptr< vector<ScSharedTokenRef> > pTokens(new vector<ScSharedTokenRef>);
    sal_uInt32 nStop = getIndex(getColCount(), nRow);
    for (sal_uInt32 i = getIndex(0, nRow); i < nStop; i += nRowCount)
    {
        FormulaToken* p = maData[i];
        if (!p)
            continue;

        ScSharedTokenRef p2(static_cast<ScToken*>(p->Clone()));
        ScRefTokenHelper::join(*pTokens, p2);
    }
    return pTokens.release();
}

// ----------------------------------------------------------------------------

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
    Chart2Positioner(ScDocument* pDoc, const vector<ScSharedTokenRef>& rRefTokens) :
        mpRefTokens(new vector<ScSharedTokenRef>(rRefTokens)),
        mpPositionMap(NULL),
        meGlue(GLUETYPE_NA),
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

    bool hasColHeaders() const { return mbColHeaders; }
    bool hasRowHeaders() const { return mbRowHeaders; }

    Chart2PositionMap* getPositionMap()
    {
        createPositionMap();
        return mpPositionMap.get();
    }

private:
    Chart2Positioner(); // disabled

    void invalidateGlue();
    void glueState();
    void createPositionMap();

private:
    shared_ptr< vector<ScSharedTokenRef> >  mpRefTokens;
    auto_ptr<Chart2PositionMap>             mpPositionMap;
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
    mpPositionMap.reset(NULL);
}

void Chart2Positioner::glueState()
{
    if (meGlue != GLUETYPE_NA)
        return;

    mbDummyUpperLeft = false;
    if (mpRefTokens->size() <= 1)
    {
        const ScSharedTokenRef& p = mpRefTokens->front();
        ScComplexRefData aData;
        if (ScRefTokenHelper::getDoubleRefDataFromToken(aData, p))
        {
            if (aData.Ref1.nTab == aData.Ref2.nTab)
                meGlue = GLUETYPE_NONE;
            else
                meGlue = GLUETYPE_COLS;
            mnStartCol = aData.Ref1.nCol;
            mnStartRow = aData.Ref1.nRow;
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
    ScRefTokenHelper::getDoubleRefDataFromToken(aData, mpRefTokens->front());
    mnStartCol = aData.Ref1.nCol;
    mnStartRow = aData.Ref1.nRow;

    SCCOL nMaxCols = 0, nEndCol = 0;
    SCROW nMaxRows = 0, nEndRow = 0;
    for (vector<ScSharedTokenRef>::const_iterator itr = mpRefTokens->begin(), itrEnd = mpRefTokens->end()
         ; itr != itrEnd; ++itr)
    {
        ScRefTokenHelper::getDoubleRefDataFromToken(aData, *itr);
        SCCOLROW n1 = aData.Ref1.nCol;
        SCCOLROW n2 = aData.Ref2.nCol;
        SCCOLROW nTmp = n2 - n1 + 1;
        if (n1 < mnStartCol)
            mnStartCol = static_cast<SCCOL>(n1);
        if (n2 > nEndCol)
            nEndCol = static_cast<SCCOL>(n2);
        if (nTmp > nMaxCols)
            nMaxCols = static_cast<SCCOL>(nTmp);

        n1 = aData.Ref1.nRow;
        n2 = aData.Ref2.nRow;
        nTmp = n2 - n1 + 1;

        if (n1 < mnStartRow)
            mnStartRow = static_cast<SCCOL>(n1);
        if (n2 > nEndRow)
            nEndRow = static_cast<SCCOL>(n2);
        if (nTmp > nMaxRows)
            nMaxRows = static_cast<SCCOL>(nTmp);
    }

    // total column size ?
    SCCOL nC = nEndCol - mnStartCol + 1;
    if (nC == 1)
    {
        meGlue = GLUETYPE_ROWS;
        return;
    }
    // total row size ?
    SCROW nR = nEndRow - mnStartRow + 1;
    if (nR == 1)
    {
        meGlue = GLUETYPE_COLS;
        return;
    }
    sal_uInt32 nCR = static_cast<sal_uInt32>(nC*nR);

    const sal_uInt8 nHole = 0;
    const sal_uInt8 nOccu = 1;
    const sal_uInt8 nFree = 2;
    const sal_uInt8 nGlue = 3;

    vector<sal_uInt8> aCellStates(nCR);
    for (vector<ScSharedTokenRef>::const_iterator itr = mpRefTokens->begin(), itrEnd = mpRefTokens->end();
          itr != itrEnd; ++itr)
    {
        ScRefTokenHelper::getDoubleRefDataFromToken(aData, *itr);
        SCCOL nCol1 = static_cast<SCCOL>(aData.Ref1.nCol) - mnStartCol;
        SCCOL nCol2 = static_cast<SCCOL>(aData.Ref2.nCol) - mnStartCol;
        SCROW nRow1 = static_cast<SCROW>(aData.Ref1.nRow) - mnStartRow;
        SCROW nRow2 = static_cast<SCROW>(aData.Ref2.nRow) - mnStartRow;
        for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
            for (SCROW nRow = nRow1; nRow <= nRow2; ++nRow)
            {
                size_t i = nCol*nR + nRow;
                aCellStates[i] = nOccu;
            }
    }
    bool bGlue = true;

    size_t i = 0;
    bool bGlueCols = false;
    for (SCCOL nCol = 0; bGlue && nCol < nC; ++nCol)
    {
        for (SCROW nRow = 0; bGlue && nRow < nR; ++nRow)
        {
            i = nCol*nR + nRow;
            if (aCellStates[i] == nOccu)
            {
                if (nRow > 0 && nRow > 0)
                    bGlue = false;
                else
                    nRow = nR;
            }
            else
                aCellStates[i] = nFree;
        }
        i = (nCol+1)*nR - 1; // index for the last cell in the column.
        if (bGlue && (aCellStates[i] == nFree))
        {
            aCellStates[i] = nGlue;
            bGlueCols = true;
        }
    }

    bool bGlueRows = false;
    for (SCROW nRow = 0; bGlue && nRow < nR; ++nRow)
    {
        i = nRow;
        for (SCCOL nCol = 0; bGlue && nCol < nC; ++nCol, i += nR)
        {
            if (aCellStates[i] == nOccu)
            {
                if (nCol > 0 && nRow > 0)
                    bGlue = false;
                else
                    nCol = nC;
            }
            else
                aCellStates[i] = nFree;
        }
        i = (nC-1)*nR + nRow; // index for the row position in the last column.
        if (bGlue && aCellStates[i] == nFree)
        {
            aCellStates[i] = nGlue;
            bGlueRows = true;
        }
    }

    i = 1;
    for (sal_uInt32 n = 1; bGlue && n < nCR; ++n, ++i)
        if (aCellStates[i] == nHole)
            bGlue = false;

    if (bGlue)
    {
        if (bGlueCols && bGlueRows)
            meGlue = GLUETYPE_BOTH;
        else if (bGlueRows)
            meGlue = GLUETYPE_ROWS;
        else
            meGlue = GLUETYPE_COLS;
        if (aCellStates.front() != nOccu)
            mbDummyUpperLeft = true;
    }
    else
        meGlue = GLUETYPE_NONE;
}

void Chart2Positioner::createPositionMap()
{
    if (meGlue == GLUETYPE_NA && mpPositionMap.get())
        mpPositionMap.reset(NULL);

    if (mpPositionMap.get())
        return;

    glueState();

    bool bNoGlue = (meGlue == GLUETYPE_NONE);
    auto_ptr<Table> pCols(new Table);
    auto_ptr<FormulaToken> pNewAddress;
    auto_ptr<Table> pNewRowTable(new Table);
    Table* pCol = NULL;
    SCROW nNoGlueRow = 0;
    for (vector<ScSharedTokenRef>::const_iterator itr = mpRefTokens->begin(), itrEnd = mpRefTokens->end();
          itr != itrEnd; ++itr)
    {
        const ScSharedTokenRef& pToken = *itr;

        bool bExternal = ScRefTokenHelper::isExternalRef(pToken);
        sal_uInt16 nFileId = bExternal ? pToken->GetIndex() : 0;
        String aTabName = bExternal ? pToken->GetString() : String();

        ScComplexRefData aData;
        ScRefTokenHelper::getDoubleRefDataFromToken(aData, *itr);
        const ScSingleRefData& s = aData.Ref1;
        const ScSingleRefData& e = aData.Ref2;
        SCCOL nCol1 = s.nCol, nCol2 = e.nCol;
        SCROW nRow1 = s.nRow, nRow2 = e.nRow;
        SCTAB nTab1 = s.nTab, nTab2 = e.nTab;

        for (SCTAB nTab = nTab1; nTab <= nTab2; ++nTab)
        {
            // What's this for ???
            sal_uInt32 nInsCol = (static_cast<sal_uInt32>(nTab) << 16) |
                (bNoGlue ? 0 : static_cast<sal_uInt32>(nCol1));
            for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol, ++nInsCol)
            {
                if (bNoGlue || meGlue == GLUETYPE_ROWS)
                {
                    pCol = static_cast<Table*>(pCols->Get(nInsCol));
                    if (!pCol)
                    {
                        pCol = pNewRowTable.get();
                        pCols->Insert(nInsCol, pNewRowTable.release());
                        pNewRowTable.reset(new Table);
                    }
                }
                else
                {
                    if (pCols->Insert(nInsCol, pNewRowTable.get()))
                    {
                        pCol = pNewRowTable.release();
                        pNewRowTable.reset(new Table);
                    }
                    else
                        pCol = static_cast<Table*>(pCols->Get(nInsCol));
                }

                sal_uInt32 nInsRow = static_cast<sal_uInt32>(bNoGlue ? nNoGlueRow : nRow1);
                for (SCROW nRow = nRow1; nRow <= nRow2; ++nRow, ++nInsRow)
                {
                    ScSingleRefData aCellData;
                    aCellData.InitFlags();
                    aCellData.SetFlag3D(true);
                    aCellData.SetColRel(false);
                    aCellData.SetRowRel(false);
                    aCellData.SetTabRel(false);
                    aCellData.nCol = nCol;
                    aCellData.nRow = nRow;
                    aCellData.nTab = nTab;

                    if (bExternal)
                        pNewAddress.reset(new ScExternalSingleRefToken(nFileId, aTabName, aCellData));
                    else
                        pNewAddress.reset(new ScSingleRefToken(aCellData));

                    if (pCol->Insert(nInsRow, pNewAddress.get()))
                        pNewAddress.release(); // To prevent the instance from being destroyed.
                }
            }
        }
        nNoGlueRow += nRow2 - nRow1 + 1;
    }
    pNewAddress.reset(NULL);
    pNewRowTable.reset(NULL);

    bool bColAdd = mbRowHeaders;
    bool bRowAdd = mbColHeaders;

    SCSIZE nColCount = static_cast<SCSIZE>(pCols->Count());
    SCSIZE nRowCount = 0;
    pCol = static_cast<Table*>(pCols->First());
    if (pCol)
    {
        if (mbDummyUpperLeft)
            pCol->Insert(0, NULL);        // Dummy fuer Beschriftung
        nRowCount = static_cast<SCSIZE>(pCol->Count());
    }
    else
        nRowCount = 0;

    if (nColCount > 0 && bColAdd)
        nColCount -= 1;
    if (nRowCount > 0 && bRowAdd)
        nRowCount -= 1;

    if (nColCount == 0 || nRowCount == 0)
    {
        ScComplexRefData aData;
        ScRefTokenHelper::getDoubleRefDataFromToken(aData, mpRefTokens->front());
        if (pCols->Count() > 0)
            pCol = static_cast<Table*>(pCols->First());
        else
        {
            pCol = new Table;
            pCols->Insert(0, pCol);
        }
        nColCount = 1;
        if (pCol->Count() > 0)
        {
            FormulaToken* pPos = static_cast<FormulaToken*>(pCol->First());
            if (pPos)
            {
                delete pPos;
                pCol->Replace(pCol->GetCurKey(), NULL);
            }
        }
        else
            pCol->Insert(0, NULL);

        nRowCount = 1;
        bColAdd = false;
        bRowAdd = false;
    }
    else
    {
        if (bNoGlue)
        {
            Table* pFirstCol = static_cast<Table*>(pCols->First());
            sal_uInt32 nCount = pFirstCol->Count();
            pFirstCol->First();
            for (sal_uInt32 n = 0; n < nCount; ++n, pFirstCol->Next())
            {
                sal_uInt32 nKey = pFirstCol->GetCurKey();
                pCols->First();
                for (pCol = static_cast<Table*>(pCols->Next()); pCol; pCol = static_cast<Table*>(pCols->Next()))
                    pCol->Insert(nKey, NULL);
            }
        }
    }
    mpPositionMap.reset(
        new Chart2PositionMap(
            static_cast<SCCOL>(nColCount), static_cast<SCROW>(nRowCount),
            bColAdd, bRowAdd, *pCols));

    // Destroy all column instances.
    for (pCol = static_cast<Table*>(pCols->First()); pCol; pCol = static_cast<Table*>(pCols->Next()))
        delete pCol;
}

// ============================================================================

/**
 * Function object to create a range string from a token list.
 */
class Tokens2RangeString : public unary_function<ScSharedTokenRef, void>
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

    Tokens2RangeString(const Tokens2RangeString& r) :
        mpRangeStr(r.mpRangeStr),
        mpDoc(r.mpDoc),
        meGrammar(r.meGrammar),
        mcRangeSep(r.mcRangeSep),
        mbFirst(r.mbFirst)
    {
    }

    void operator() (const ScSharedTokenRef& rToken)
    {
        ScCompiler aCompiler(mpDoc, ScAddress(0,0,0));
        aCompiler.SetGrammar(meGrammar);
        String aStr;
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
    Tokens2RangeString(); // disabled

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
class Tokens2RangeStringXML : public unary_function<ScSharedTokenRef, void>
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

    void operator() (const ScSharedTokenRef& rToken)
    {
        if (mbFirst)
            mbFirst = false;
        else
            mpRangeStr->append(mcRangeSep);

        ScSharedTokenRef aStart, aEnd;
        splitRangeToken(rToken, aStart, aEnd);
        ScCompiler aCompiler(mpDoc, ScAddress(0,0,0));
        aCompiler.SetGrammar(FormulaGrammar::GRAM_ENGLISH);
        {
            String aStr;
            aCompiler.CreateStringFromToken(aStr, aStart.get());
            mpRangeStr->append(aStr);
        }
        mpRangeStr->append(mcAddrSep);
        {
            String aStr;
            aCompiler.CreateStringFromToken(aStr, aEnd.get());
            mpRangeStr->append(aStr);
        }
    }

    void getString(OUString& rStr)
    {
        rStr = mpRangeStr->makeStringAndClear();
    }

private:
    Tokens2RangeStringXML(); // disabled

    void splitRangeToken(const ScSharedTokenRef& pToken, ScSharedTokenRef& rStart, ScSharedTokenRef& rEnd) const
    {
        ScComplexRefData aData;
        ScRefTokenHelper::getDoubleRefDataFromToken(aData, pToken);
        bool bExternal = ScRefTokenHelper::isExternalRef(pToken);
        sal_uInt16 nFileId = bExternal ? pToken->GetIndex() : 0;
        String aTabName = bExternal ? pToken->GetString() : String();

        // In saving to XML, we don't prepend address with '$'.
        setRelative(aData.Ref1);
        setRelative(aData.Ref2);

        // In XML, the end range must explicitly specify sheet name.
        aData.Ref2.SetFlag3D(true);

        if (bExternal)
            rStart.reset(new ScExternalSingleRefToken(nFileId, aTabName, aData.Ref1));
        else
            rStart.reset(new ScSingleRefToken(aData.Ref1));

        if (bExternal)
            rEnd.reset(new ScExternalSingleRefToken(nFileId, aTabName, aData.Ref2));
        else
            rEnd.reset(new ScSingleRefToken(aData.Ref2));
    }

    void setRelative(ScSingleRefData& rData) const
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

void lcl_convertTokensToString(OUString& rStr, const vector<ScSharedTokenRef>& rTokens, ScDocument* pDoc)
{
    const sal_Unicode cRangeSep = ScCompiler::GetNativeSymbol(ocSep).GetChar(0);
    FormulaGrammar::Grammar eGrammar = pDoc->GetGrammar();
    Tokens2RangeString func(pDoc, eGrammar, cRangeSep);
    func = for_each(rTokens.begin(), rTokens.end(), func);
    func.getString(rStr);
}

void lcl_convertTokenToString(OUString& rStr, const ScSharedTokenRef& rToken, ScDocument* pDoc,
                              FormulaGrammar::Grammar eGrammar)
{
    const sal_Unicode cRangeSep = ScCompiler::GetNativeSymbol(ocSep).GetChar(0);
    Tokens2RangeString func(pDoc, eGrammar, cRangeSep);
    func.operator() (rToken);
    func.getString(rStr);
}

} // anonymous namespace

// DataProvider ==============================================================

ScChart2DataProvider::ScChart2DataProvider( ScDocument* pDoc )
    : m_pDocument( pDoc)
    , m_aPropSet(lcl_GetDataProviderPropertyMap())
    , m_bIncludeHiddenCells( sal_True)
{
    if ( m_pDocument )
        m_pDocument->AddUnoObject( *this);
}

ScChart2DataProvider::~ScChart2DataProvider()
{
    if ( m_pDocument )
        m_pDocument->RemoveUnoObject( *this);
}


void ScChart2DataProvider::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint)
{
    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        m_pDocument = NULL;
    }
}

void lcl_SeperateOneColumnRange(ScRange aR, const ScAddress& rPos, ScRangeListRef& xRanges)
{
    if (aR.aStart == rPos)
    {
        aR.aStart.SetRow(aR.aStart.Row() + 1);
        xRanges->Join(aR);
    }
    else if (aR.aEnd == rPos)
    {
        aR.aStart.SetRow(aR.aStart.Row() - 1);
        xRanges->Join(aR);
    }
    else
    {
        xRanges->Join(ScRange(aR.aStart, ScAddress(rPos.Col(), rPos.Row() - 1, rPos.Tab())));
        xRanges->Join(ScRange(ScAddress(rPos.Col(), rPos.Row() + 1, rPos.Tab()), aR.aEnd ));
    }
}

void lcl_SeperateOneRowRange(ScRange aR, const ScAddress& rPos, ScRangeListRef& xRanges)
{
    if (aR.aStart == rPos)
    {
        aR.aStart.SetCol(aR.aStart.Col() + 1);
        xRanges->Join(aR);
    }
    else if (aR.aEnd == rPos)
    {
        aR.aStart.SetCol(aR.aStart.Col() - 1);
        xRanges->Join(aR);
    }
    else
    {
        xRanges->Join(ScRange(aR.aStart, ScAddress(rPos.Col() - 1, rPos.Row(), rPos.Tab())));
        xRanges->Join(ScRange(ScAddress(rPos.Col() + 1, rPos.Row(), rPos.Tab()), aR.aEnd ));
    }
}

::sal_Bool SAL_CALL ScChart2DataProvider::createDataSourcePossible( const uno::Sequence< beans::PropertyValue >& aArguments )
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if( ! m_pDocument )
        return false;

    rtl::OUString aRangeRepresentation;
    for(sal_Int32 i = 0; i < aArguments.getLength(); ++i)
    {
        rtl::OUString sName(aArguments[i].Name);
        if (aArguments[i].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("CellRangeRepresentation")))
        {
            aArguments[i].Value >>= aRangeRepresentation;
        }
    }

    vector<ScSharedTokenRef> aTokens;
    ScRefTokenHelper::compileRangeRepresentation(aTokens, aRangeRepresentation, m_pDocument);
    return !aTokens.empty();
}

namespace {

class RemoveHeaderFromRanges : public unary_function<ScSharedTokenRef, void>
{
public:
    RemoveHeaderFromRanges(const ScSharedTokenRef& rHeaderCell, bool bOrientCol) :
        mpTokens(new vector<ScSharedTokenRef>),
        mpHeaderCell(rHeaderCell),
        mbOrientCol(bOrientCol)
    {
    }

    RemoveHeaderFromRanges(const RemoveHeaderFromRanges& r) :
        mpTokens(r.mpTokens),
        mpHeaderCell(r.mpHeaderCell),
        mbOrientCol(r.mbOrientCol)
    {
    }

    void operator() (const ScSharedTokenRef& pRange)
    {
        if (!isContained(pRange))
        {
            // header cell is not part of this range.  Just add it to the
            // range list and move on.
            ScRefTokenHelper::join(*mpTokens, pRange);
            return;
        }

        // This range contains the header cell.

        ScComplexRefData aRange;
        ScRefTokenHelper::getDoubleRefDataFromToken(aRange, pRange);
        const ScSingleRefData& s = aRange.Ref1;
        const ScSingleRefData& e = aRange.Ref2;
        const ScSingleRefData& h = mpHeaderCell->GetSingleRef();

        if (equals(s, e))
            // This range *only* contains the header cell.  Skip it.
            return;

        if (s.nTab != e.nTab)
            // 3D range has no business being here....
            return;

        if (mbOrientCol)
        {
            // column major

            if (s.nCol == e.nCol)
            {
                // single column range.
                splitSingleColRange(pRange, s, e);
            }
            else
            {
                if (s.nCol == h.nCol)
                {
                    // header cell is in the first column.

                    {
                        ScSharedTokenRef pNew(static_cast<ScToken*>(pRange->Clone()));
                        ScComplexRefData& r = pNew->GetDoubleRef();
                        r.Ref2.InitAddress(s.nCol, e.nRow, s.nTab);
                        splitSingleColRange(pNew, r.Ref1, r.Ref2);
                    }

                    {
                        ScSharedTokenRef pNew(static_cast<ScToken*>(pRange->Clone()));
                        ScComplexRefData& r = pNew->GetDoubleRef();
                        r.Ref1.InitAddress(s.nCol + 1, s.nRow, s.nTab);
                        ScRefTokenHelper::join(*mpTokens, pNew);
                    }
                }
                else if (e.nCol == h.nCol)
                {
                    // header cell is in the last column.

                    {
                        ScSharedTokenRef pNew(static_cast<ScToken*>(pRange->Clone()));
                        ScComplexRefData& r = pNew->GetDoubleRef();
                        r.Ref1.InitAddress(e.nCol, s.nRow, s.nTab);
                        splitSingleColRange(pNew, r.Ref1, r.Ref2);
                    }

                    {
                        ScSharedTokenRef pNew(static_cast<ScToken*>(pRange->Clone()));
                        ScComplexRefData& r = pNew->GetDoubleRef();
                        r.Ref2.InitAddress(e.nCol - 1, e.nRow, e.nTab);
                        ScRefTokenHelper::join(*mpTokens, pNew);
                    }
                }
                else
                {
                    // header cell is somewhere between the first and last columns.

                    {
                        ScSharedTokenRef pNew(static_cast<ScToken*>(pRange->Clone()));
                        ScComplexRefData& r = pNew->GetDoubleRef();
                        r.Ref2.InitAddress(h.nCol - 1, e.nRow, h.nTab);
                        ScRefTokenHelper::join(*mpTokens, pNew);
                    }

                    {
                        ScSharedTokenRef pNew(static_cast<ScToken*>(pRange->Clone()));
                        ScComplexRefData& r = pNew->GetDoubleRef();
                        r.Ref1.InitAddress(h.nCol, s.nRow, s.nTab);
                        r.Ref2.InitAddress(h.nCol, e.nRow, e.nTab);
                        splitSingleColRange(pNew, r.Ref1, r.Ref2);
                    }

                    {
                        ScSharedTokenRef pNew(static_cast<ScToken*>(pRange->Clone()));
                        ScComplexRefData& r = pNew->GetDoubleRef();
                        r.Ref1.InitAddress(h.nCol + 1, s.nRow, h.nTab);
                        ScRefTokenHelper::join(*mpTokens, pNew);
                    }
                }
            }
        }
        else
        {
            // row major

            if (s.nRow == e.nRow)
            {
                // Single row range.
                splitSingleRowRange(pRange, s, e);
            }
            else
            {
                if (s.nRow == h.nRow)
                {
                    // header cell is in the first row.

                    {
                        ScSharedTokenRef pNew(static_cast<ScToken*>(pRange->Clone()));
                        ScComplexRefData& r = pNew->GetDoubleRef();
                        r.Ref2.InitAddress(e.nCol, s.nRow, s.nTab);
                        splitSingleRowRange(pNew, r.Ref1, r.Ref2);
                    }

                    {
                        ScSharedTokenRef pNew(static_cast<ScToken*>(pRange->Clone()));
                        ScComplexRefData& r = pNew->GetDoubleRef();
                        r.Ref1.InitAddress(s.nCol, s.nRow + 1, s.nTab);
                        ScRefTokenHelper::join(*mpTokens, pNew);
                    }
                }
                else if (e.nRow == h.nRow)
                {
                    // header cell is in the last row.

                    {
                        ScSharedTokenRef pNew(static_cast<ScToken*>(pRange->Clone()));
                        ScComplexRefData& r = pNew->GetDoubleRef();
                        r.Ref1.InitAddress(s.nCol, e.nRow, s.nTab);
                        splitSingleRowRange(pNew, r.Ref1, r.Ref2);
                    }

                    {
                        ScSharedTokenRef pNew(static_cast<ScToken*>(pRange->Clone()));
                        ScComplexRefData& r = pNew->GetDoubleRef();
                        r.Ref2.InitAddress(e.nCol, e.nRow - 1, e.nTab);
                        ScRefTokenHelper::join(*mpTokens, pNew);
                    }
                }
                else
                {
                    // header cell is somewhere between the 1st and last rows.

                    {
                        ScSharedTokenRef pNew(static_cast<ScToken*>(pRange->Clone()));
                        ScComplexRefData& r = pNew->GetDoubleRef();
                        r.Ref2.InitAddress(e.nCol, h.nRow - 1, h.nTab);
                        ScRefTokenHelper::join(*mpTokens, pNew);
                    }

                    {
                        ScSharedTokenRef pNew(static_cast<ScToken*>(pRange->Clone()));
                        ScComplexRefData& r = pNew->GetDoubleRef();
                        r.Ref1.InitAddress(s.nCol, h.nRow, s.nTab);
                        r.Ref2.InitAddress(e.nCol, h.nRow, e.nTab);
                        splitSingleRowRange(pNew, r.Ref1, r.Ref2);
                    }

                    {
                        ScSharedTokenRef pNew(static_cast<ScToken*>(pRange->Clone()));
                        ScComplexRefData& r = pNew->GetDoubleRef();
                        r.Ref1.InitAddress(s.nCol, h.nRow + 1, h.nTab);
                        ScRefTokenHelper::join(*mpTokens, pNew);
                    }
                }
            }
        }
    }

    void getNewTokens(vector<ScSharedTokenRef>& rTokens)
    {
        mpTokens->swap(rTokens);
    }

private:

    void splitSingleColRange(const ScSharedTokenRef& pRange, const ScSingleRefData& s, const ScSingleRefData& e)
    {
        const ScSingleRefData& h = mpHeaderCell->GetSingleRef();

        if (equals(s, h))
        {
            // header is at the top.

            ScSharedTokenRef pNew(static_cast<ScToken*>(pRange->Clone()));
            ScComplexRefData& r = pNew->GetDoubleRef();
            r.Ref1.nRow += 1;
            ScRefTokenHelper::join(*mpTokens, pNew);
        }
        else if (equals(e, h))
        {
            // header is at the bottom.

            ScSharedTokenRef pNew(static_cast<ScToken*>(pRange->Clone()));
            ScComplexRefData& r = pNew->GetDoubleRef();
            r.Ref2.nRow -= 1;
            ScRefTokenHelper::join(*mpTokens, pNew);
        }
        else
        {
            // header is somewhere in the middle.

            {
                ScSharedTokenRef pNew(static_cast<ScToken*>(pRange->Clone()));
                ScComplexRefData& r = pNew->GetDoubleRef();
                r.Ref2.InitAddress(h.nCol, h.nRow - 1, h.nTab);
                ScRefTokenHelper::join(*mpTokens, pNew);
            }

            {
                ScSharedTokenRef pNew(static_cast<ScToken*>(pRange->Clone()));
                ScComplexRefData& r = pNew->GetDoubleRef();
                r.Ref1.InitAddress(h.nCol, h.nRow + 1, h.nTab);
                ScRefTokenHelper::join(*mpTokens, pNew);
            }
        }
    }

    void splitSingleRowRange(const ScSharedTokenRef& pRange, const ScSingleRefData& s, const ScSingleRefData& e)
    {
        const ScSingleRefData& h = mpHeaderCell->GetSingleRef();

        if (equals(s, h))
        {
            // header is at the top.

            ScSharedTokenRef pNew(static_cast<ScToken*>(pRange->Clone()));
            ScComplexRefData& r = pNew->GetDoubleRef();
            r.Ref1.nCol += 1;
            ScRefTokenHelper::join(*mpTokens, pNew);
        }
        else if (equals(e, h))
        {
            // header is at the bottom.

            ScSharedTokenRef pNew(static_cast<ScToken*>(pRange->Clone()));
            ScComplexRefData& r = pNew->GetDoubleRef();
            r.Ref2.nCol -= 1;
            ScRefTokenHelper::join(*mpTokens, pNew);
        }
        else
        {
            // header is somewhere in the middle.

            {
                ScSharedTokenRef pNew(static_cast<ScToken*>(pRange->Clone()));
                ScComplexRefData& r = pNew->GetDoubleRef();
                r.Ref2.InitAddress(h.nCol - 1, h.nRow, h.nTab);
                ScRefTokenHelper::join(*mpTokens, pNew);
            }

            {
                ScSharedTokenRef pNew(static_cast<ScToken*>(pRange->Clone()));
                ScComplexRefData& r = pNew->GetDoubleRef();
                r.Ref1.InitAddress(h.nCol + 1, h.nRow, h.nTab);
                ScRefTokenHelper::join(*mpTokens, pNew);
            }
        }
    }

    /**
     * Compare two single ref data for equality, but only compare their
     * absolute cell addresses while ignoring flags and relative addresses.
     */
    bool equals(const ScSingleRefData& r1, const ScSingleRefData& r2) const
    {
        return (r1.nCol == r2.nCol) && (r1.nRow == r2.nRow) && (r1.nTab == r2.nTab);
    }

    bool isContained(const ScSharedTokenRef& pRange)
    {
        bool bExternal = ScRefTokenHelper::isExternalRef(mpHeaderCell);
        if (bExternal != ScRefTokenHelper::isExternalRef(pRange))
            // internal vs external.
            return false;

        if (bExternal)
        {
            if (pRange->GetIndex() != mpHeaderCell->GetIndex())
                // different external files.
                return false;

            if (pRange->GetString() != mpHeaderCell->GetString())
                // different table.
                return false;
        }

        ScComplexRefData aRange;
        ScRefTokenHelper::getDoubleRefDataFromToken(aRange, pRange);
        const ScSingleRefData& rCell = mpHeaderCell->GetSingleRef();

        bool bRowContained = (aRange.Ref1.nRow <= rCell.nRow) && (rCell.nRow <= aRange.Ref2.nRow);
        bool bColContained = (aRange.Ref1.nCol <= rCell.nCol) && (rCell.nCol <= aRange.Ref2.nCol);
        bool bTabContained = (aRange.Ref1.nTab <= rCell.nTab) && (rCell.nTab <= aRange.Ref2.nTab);

        return (bRowContained && bColContained && bTabContained);
    }

private:
    shared_ptr< vector<ScSharedTokenRef> > mpTokens;

    /**
     *  Stores header cell position.  Must be a single ref token i.e. either
     *  ScSingleRefToken or ScExternalSingleRefToken.
     */
    ScSharedTokenRef mpHeaderCell;

    bool mbOrientCol;
};

}

static void lcl_removeHeaderFromRanges(vector<ScSharedTokenRef>& rTokens, const ScSharedTokenRef& rHeaderCell, bool bOrientCol)
{
    RemoveHeaderFromRanges func(rHeaderCell, bOrientCol);
    func = for_each(rTokens.begin(), rTokens.end(), func);
    func.getNewTokens(rTokens);
}

uno::Reference< chart2::data::XDataSource> SAL_CALL
ScChart2DataProvider::createDataSource(
    const uno::Sequence< beans::PropertyValue >& aArguments )
    throw( lang::IllegalArgumentException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if ( ! m_pDocument )
        throw uno::RuntimeException();

    uno::Reference< chart2::data::XDataSource> xResult;
    bool bLabel = true;
    bool bOrientCol = true;
    ::rtl::OUString aRangeRepresentation;
    uno::Sequence< sal_Int32 > aSequenceMapping;
    for(sal_Int32 i = 0; i < aArguments.getLength(); ++i)
    {
        rtl::OUString sName(aArguments[i].Name);
        if (aArguments[i].Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DataRowSource")))
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
        else if (aArguments[i].Name == rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FirstCellAsLabel")))
        {
            bLabel = ::cppu::any2bool(aArguments[i].Value);
        }
        else if (aArguments[i].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("CellRangeRepresentation")))
        {
            aArguments[i].Value >>= aRangeRepresentation;
        }
        else if (aArguments[i].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("SequenceMapping")))
        {
            aArguments[i].Value >>= aSequenceMapping;
        }
    }

    vector<ScSharedTokenRef> aRefTokens;
    ScRefTokenHelper::compileRangeRepresentation(aRefTokens, aRangeRepresentation, m_pDocument);
    if (aRefTokens.empty())
        // Invalid range representation.  Bail out.
        throw lang::IllegalArgumentException();

    if (bLabel)
        addUpperLeftCornerIfMissing(aRefTokens);

    bool bColHeaders = (bOrientCol ? bLabel : false );
    bool bRowHeaders = (bOrientCol ? false : bLabel );

    Chart2Positioner aChPositioner(m_pDocument, aRefTokens);
    aChPositioner.setHeaders(bColHeaders, bRowHeaders);

    const Chart2PositionMap* pChartMap = aChPositioner.getPositionMap();
    if (!pChartMap)
        // No chart position map instance.  Bail out.
        return xResult;

    ScChart2DataSource* pDS = NULL;
    std::list < ScChart2LabeledDataSequence* > aSeqs;

    // Fill Categories

    ScChart2LabeledDataSequence* pHeader = NULL;
    if (bOrientCol ? aChPositioner.hasRowHeaders() : aChPositioner.hasColHeaders())
    {
        pHeader = new ScChart2LabeledDataSequence(m_pDocument);
        sal_Int32 nCount = static_cast< sal_Int32 >( bOrientCol ? pChartMap->getRowCount() : pChartMap->getColCount() );
        vector<ScSharedTokenRef> aRefTokens2;
        ScSharedTokenRef pLabelToken;
        for (sal_Int32 i = 0; i < nCount; ++i)
        {
            const FormulaToken* pPos = bOrientCol ?
                pChartMap->getRowHeaderPosition(static_cast<SCROW>(i)) :
                pChartMap->getColHeaderPosition(static_cast<SCCOL>(i));
            if (pPos)
            {
                ScSharedTokenRef p(static_cast<ScToken*>(pPos->Clone()));
                ScRefTokenHelper::join(aRefTokens2, p);
                if (!pLabelToken)
                {
                    pLabelToken = p;
                    StackVar eType = pLabelToken->GetType();
                    if (eType == svSingleRef || eType == svExternalSingleRef)
                    {
                        ScSingleRefData& r = pLabelToken->GetSingleRef();
                        if (bOrientCol)
                            r.nRow -= 1;
                        else
                            r.nCol -= 1;
                    }
                }
            }
        }
        if (pLabelToken)
        {
            if (bLabel)
            {
                auto_ptr< vector<ScSharedTokenRef> > pTokens(new vector<ScSharedTokenRef>);
                pTokens->push_back(pLabelToken);
                Reference < chart2::data::XDataSequence > xLabelSeq(new ScChart2DataSequence(m_pDocument, this, pTokens.release(), m_bIncludeHiddenCells));
                Reference< beans::XPropertySet > xLabelProps(xLabelSeq, uno::UNO_QUERY);
                if (xLabelProps.is())
                    xLabelProps->setPropertyValue(
                        OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_ROLE)),
                        uno::makeAny(OUString(RTL_CONSTASCII_USTRINGPARAM("label"))));
                pHeader->setLabel(xLabelSeq);
            }
            else
                ScRefTokenHelper::join(aRefTokens2, pLabelToken);
        }
        auto_ptr< vector<ScSharedTokenRef> > pTokens(new vector<ScSharedTokenRef>);
        pTokens->swap(aRefTokens2);
        uno::Reference< chart2::data::XDataSequence > xSeq( new ScChart2DataSequence( m_pDocument, this, pTokens.release(), m_bIncludeHiddenCells) );
        pHeader->setValues(xSeq);
    }
    if (pHeader)
        aSeqs.push_back(pHeader);

    // Fill Serieses with Labels

    sal_Int32 nCount = bOrientCol ? pChartMap->getColCount() : pChartMap->getRowCount();
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        ScChart2LabeledDataSequence* pLabeled = new ScChart2LabeledDataSequence(m_pDocument);
        uno::Reference < chart2::data::XDataSequence > xLabelSeq;
        auto_ptr< vector<ScSharedTokenRef> > pRanges(NULL);
        if (bOrientCol)
            pRanges.reset(pChartMap->getColRanges(static_cast<SCCOL>(i)));
        else
            pRanges.reset(pChartMap->getRowRanges(static_cast<SCROW>(i)));

        ScSharedTokenRef pHeaderCell;
        if (bOrientCol)
        {
            const FormulaToken* p = pChartMap->getColHeaderPosition(static_cast<SCCOL>(i));
            if (p)
                pHeaderCell.reset(static_cast<ScToken*>(p->Clone()));
        }
        else
        {
            const FormulaToken* p = pChartMap->getRowHeaderPosition(static_cast<SCROW>(i));
            if (p)
                pHeaderCell.reset(static_cast<ScToken*>(p->Clone()));
        }

        if (bLabel)
        {
            if (!pHeaderCell && pRanges.get() && !pRanges->empty())
            {
                const ScSharedTokenRef& p = pRanges->front();
                if (p && ScRefTokenHelper::isRef(p))
                {
                    // Take the first cell in the range as the header position.
                    ScSingleRefData aData = p->GetSingleRef();
                    bool bExternal = ScRefTokenHelper::isExternalRef(p);
                    if (bExternal)
                    {
                        sal_uInt16 nFileId = p->GetIndex();
                        const String& rTabName = p->GetString();
                        pHeaderCell.reset(new ScExternalSingleRefToken(nFileId, rTabName, aData));
                    }
                    else
                        pHeaderCell.reset(new ScSingleRefToken(aData));
                }
            }
            if (pHeaderCell)
            {
                auto_ptr< vector<ScSharedTokenRef> > pTokens(new vector<ScSharedTokenRef>);
                pTokens->reserve(1);
                pTokens->push_back(pHeaderCell);
                xLabelSeq.set(new ScChart2DataSequence(m_pDocument, this, pTokens.release(), m_bIncludeHiddenCells));
                uno::Reference< beans::XPropertySet > xLabelProps(xLabelSeq, uno::UNO_QUERY);
                if (xLabelProps.is())
                    xLabelProps->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_ROLE)), uno::makeAny(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("label"))));

                // remove Header from Ranges
                lcl_removeHeaderFromRanges(*pRanges, pHeaderCell, bOrientCol);
            }
        }
        else
        {
            if (pHeaderCell)
                ScRefTokenHelper::join(*pRanges, pHeaderCell);
        }

        // FIXME: if there are no labels the column or row name should be taken

        uno::Reference < chart2::data::XDataSequence > xSeq(new ScChart2DataSequence(m_pDocument, this, pRanges.release(), m_bIncludeHiddenCells));

        pLabeled->setValues(xSeq);
        pLabeled->setLabel(xLabelSeq);

        aSeqs.push_back(pLabeled);
    }

    pDS = new ScChart2DataSource(m_pDocument);
    std::list < ScChart2LabeledDataSequence* >::iterator aItr(aSeqs.begin());
    std::list < ScChart2LabeledDataSequence* >::iterator aEndItr(aSeqs.end());

    //reorder labeled sequences according to aSequenceMapping
    std::vector< ScChart2LabeledDataSequence* >  aSeqVector;
    while(aItr != aEndItr)
    {
        aSeqVector.push_back(*aItr);
        ++aItr;
    }

    std::map< sal_Int32, ScChart2LabeledDataSequence* > aSequenceMap;
    for( sal_Int32 nNewIndex = 0; nNewIndex < aSequenceMapping.getLength(); nNewIndex++ )
    {
        // note: assuming that the values in the sequence mapping are always non-negative
        std::vector< ScChart2LabeledDataSequence* >::size_type nOldIndex( static_cast< sal_uInt32 >( aSequenceMapping[nNewIndex] ));
        if( nOldIndex < aSeqVector.size() )
        {
            pDS->AddLabeledSequence( aSeqVector[nOldIndex] );
            aSeqVector[nOldIndex] = 0;
        }

    }

    std::vector< ScChart2LabeledDataSequence* >::iterator aVectorItr(aSeqVector.begin());
    std::vector< ScChart2LabeledDataSequence* >::iterator aVectorEndItr(aSeqVector.end());
    while(aVectorItr != aVectorEndItr)
    {
        if(*aVectorItr)
            pDS->AddLabeledSequence(*aVectorItr);
        ++aVectorItr;
    }

    xResult.set( pDS );
    return xResult;
}

namespace
{

bool lcl_HasCategories(
    const uno::Reference< chart2::data::XDataSource >& xDataSource,
    bool & rOutHasCategories )
{
    bool bResult = false;
    uno::Reference< chart2::data::XLabeledDataSequence > xCategories(
        lcl_getCategoriesFromDataSource( xDataSource ));
    if( xCategories.is())
    {
        uno::Reference< lang::XServiceInfo > xValues( xCategories->getValues(), uno::UNO_QUERY );
        if (xValues.is())
        {
            rOutHasCategories = xValues->getImplementationName().equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("ScChart2DataSequence"));
            bResult = true;
        }
    }
    return bResult;
}

bool lcl_HasFirstCellAsLabel(
    const uno::Reference< chart2::data::XDataSource >& xDataSource,
    bool & rOutHasFirstCellAsLabel )
{
    bool bResult = false;
    if( xDataSource.is())
    {
        uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > aSequences(
            xDataSource->getDataSequences());
        const sal_Int32 nCount( aSequences.getLength());
        if (nCount > 0 && aSequences[nCount - 1].is() )
        {
            uno::Reference< lang::XServiceInfo > xLabel( aSequences[nCount - 1]->getLabel(), uno::UNO_QUERY ); // take the last sequence, because the first has no label if it is also created
            if (xLabel.is())
            {
                rOutHasFirstCellAsLabel = xLabel->getImplementationName().equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("ScChart2DataSequence"));
                bResult = true;
            }
        }
    }
    return bResult;
}

} // anonymous namespace

bool ScChart2DataProvider::addUpperLeftCornerIfMissing(vector<ScSharedTokenRef>& rRefTokens)
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

    USHORT nFileId = 0;
    String aExtTabName;
    bool bExternal = false;

    vector<ScSharedTokenRef>::const_iterator itr = rRefTokens.begin(), itrEnd = rRefTokens.end();

    // Get the first ref token.
    ScSharedTokenRef pToken = *itr;
    switch (pToken->GetType())
    {
        case svSingleRef:
        {
            const ScSingleRefData& rData = pToken->GetSingleRef();
            nMinCol = rData.nCol;
            nMinRow = rData.nRow;
            nMaxCol = rData.nCol;
            nMaxRow = rData.nRow;
            nTab = rData.nTab;
        }
        break;
        case svDoubleRef:
        {
            const ScComplexRefData& rData = pToken->GetDoubleRef();
            nMinCol = min(rData.Ref1.nCol, rData.Ref2.nCol);
            nMinRow = min(rData.Ref1.nRow, rData.Ref2.nRow);
            nMaxCol = max(rData.Ref1.nCol, rData.Ref2.nCol);
            nMaxRow = max(rData.Ref1.nRow, rData.Ref2.nRow);
            nTab = rData.Ref1.nTab;
        }
        break;
        case svExternalSingleRef:
        {
            const ScSingleRefData& rData = pToken->GetSingleRef();
            nMinCol = rData.nCol;
            nMinRow = rData.nRow;
            nMaxCol = rData.nCol;
            nMaxRow = rData.nRow;
            nTab = rData.nTab;
            nFileId = pToken->GetIndex();
            aExtTabName = pToken->GetString();
            bExternal = true;
        }
        break;
        case svExternalDoubleRef:
        {
            const ScComplexRefData& rData = pToken->GetDoubleRef();
            nMinCol = min(rData.Ref1.nCol, rData.Ref2.nCol);
            nMinRow = min(rData.Ref1.nRow, rData.Ref2.nRow);
            nMaxCol = max(rData.Ref1.nCol, rData.Ref2.nCol);
            nMaxRow = max(rData.Ref1.nRow, rData.Ref2.nRow);
            nTab = rData.Ref1.nTab;
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
                const ScSingleRefData& rData = pToken->GetSingleRef();

                nMinCol = min(nMinCol, rData.nCol);
                nMinRow = min(nMinRow, rData.nRow);
                nMaxCol = max(nMaxCol, rData.nCol);
                nMaxRow = max(nMaxRow, rData.nRow);
                if (nTab != rData.nTab || bExternal)
                    return false;
            }
            break;
            case svDoubleRef:
            {
                const ScComplexRefData& rData = pToken->GetDoubleRef();

                nMinCol = min(nMinCol, rData.Ref1.nCol);
                nMinCol = min(nMinCol, rData.Ref2.nCol);
                nMinRow = min(nMinRow, rData.Ref1.nRow);
                nMinRow = min(nMinRow, rData.Ref2.nRow);

                nMaxCol = max(nMaxCol, rData.Ref1.nCol);
                nMaxCol = max(nMaxCol, rData.Ref2.nCol);
                nMaxRow = max(nMaxRow, rData.Ref1.nRow);
                nMaxRow = max(nMaxRow, rData.Ref2.nRow);

                if (nTab != rData.Ref1.nTab || bExternal)
                    return false;
            }
            break;
            case svExternalSingleRef:
            {
                if (!bExternal)
                    return false;

                if (nFileId != pToken->GetIndex() || aExtTabName != pToken->GetString())
                    return false;

                const ScSingleRefData& rData = pToken->GetSingleRef();

                nMinCol = min(nMinCol, rData.nCol);
                nMinRow = min(nMinRow, rData.nRow);
                nMaxCol = max(nMaxCol, rData.nCol);
                nMaxRow = max(nMaxRow, rData.nRow);
            }
            break;
            case svExternalDoubleRef:
            {
                if (!bExternal)
                    return false;

                if (nFileId != pToken->GetIndex() || aExtTabName != pToken->GetString())
                    return false;

                const ScComplexRefData& rData = pToken->GetDoubleRef();

                nMinCol = min(nMinCol, rData.Ref1.nCol);
                nMinCol = min(nMinCol, rData.Ref2.nCol);
                nMinRow = min(nMinRow, rData.Ref1.nRow);
                nMinRow = min(nMinRow, rData.Ref2.nRow);

                nMaxCol = max(nMaxCol, rData.Ref1.nCol);
                nMaxCol = max(nMaxCol, rData.Ref2.nCol);
                nMaxRow = max(nMaxRow, rData.Ref1.nRow);
                nMaxRow = max(nMaxRow, rData.Ref2.nRow);
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
    //
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
                const ScSingleRefData& rData = pToken->GetSingleRef();
                if (rData.nCol == nMinCol && rData.nRow == nMinRow)
                    // The corner cell is contained.
                    return false;

                if (rData.nCol == nMinCol+1 && rData.nRow == nMinRow)
                    bRight = true;

                if (rData.nCol == nMinCol && rData.nRow == nMinRow+1)
                    bBottom = true;

                if (rData.nCol == nMinCol+1 && rData.nRow == nMinRow+1)
                    bDiagonal = true;
            }
            break;
            case svDoubleRef:
            case svExternalDoubleRef:
            {
                const ScComplexRefData& rData = pToken->GetDoubleRef();
                const ScSingleRefData& r1 = rData.Ref1;
                const ScSingleRefData& r2 = rData.Ref2;
                if (r1.nCol <= nMinCol && nMinCol <= r2.nCol &&
                    r1.nRow <= nMinRow && nMinRow <= r2.nRow)
                    // The corner cell is contained.
                    return false;

                if (r1.nCol <= nMinCol+1 && nMinCol+1 <= r2.nCol &&
                    r1.nRow <= nMinRow && nMinRow <= r2.nRow)
                    bRight = true;

                if (r1.nCol <= nMinCol && nMinCol <= r2.nCol &&
                    r1.nRow <= nMinRow+1 && nMinRow+1 <= r2.nRow)
                    bBottom = true;

                if (r1.nCol <= nMinCol+1 && nMinCol+1 <= r2.nCol &&
                    r1.nRow <= nMinRow+1 && nMinRow+1 <= r2.nRow)
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

#if 0 // Do we really need to do this ???
    if (rRefTokens.size() == 2)
    {
        // Make a simple rectangular range if possible.
        ScRange aRightPart(ScAddress(nMinCol+1, nMinRow,   nTab),  ScAddress(nMaxCol, nMaxRow, nTab));
        ScRange aBottomPart(ScAddress(nMinCol,  nMinRow+1, nTab),  ScAddress(nMaxCol, nMaxRow, nTab));
        vector<ScRange> aRanges;
        aRanges.reserve(2);
        aRanges.push_back(aRightPart);
        aRanges.push_back(aBottomPart);
        if (lcl_isRangeContained(rRefTokens, aRanges))
        {
            // Consolidate them into a single rectangle.
            ScComplexRefData aData;
            aData.InitFlags();
            aData.Ref1.SetFlag3D(true);
            aData.Ref1.SetColRel(false);
            aData.Ref1.SetRowRel(false);
            aData.Ref1.SetTabRel(false);
            aData.Ref2.SetColRel(false);
            aData.Ref2.SetRowRel(false);
            aData.Ref2.SetTabRel(false);
            aData.Ref1.nCol = nMinCol;
            aData.Ref1.nRow = nMinRow;
            aData.Ref1.nTab = nTab;
            aData.Ref2.nCol = nMaxCol;
            aData.Ref2.nRow = nMaxRow;
            aData.Ref2.nTab = nTab;
            vector<ScSharedTokenRef> aNewTokens;
            aNewTokens.reserve(1);
            if (bExternal)
            {
                ScSharedTokenRef p(
                    new ScExternalDoubleRefToken(nFileId, aExtTabName, aData));
                aNewTokens.push_back(p);
            }
            else
            {
                ScSharedTokenRef p(new ScDoubleRefToken(aData));
                aNewTokens.push_back(p);
            }
            rRefTokens.swap(aNewTokens);
            return true;
        }
    }
#endif

    ScSingleRefData aData;
    aData.InitFlags();
    aData.SetFlag3D(true);
    aData.SetColRel(false);
    aData.SetRowRel(false);
    aData.SetTabRel(false);
    aData.nCol = nMinCol;
    aData.nRow = nMinRow;
    aData.nTab = nTab;

    if (bExternal)
    {
        ScSharedTokenRef pCorner(
            new ScExternalSingleRefToken(nFileId, aExtTabName, aData));
        ScRefTokenHelper::join(rRefTokens, pCorner);
    }
    else
    {
        ScSharedTokenRef pCorner(new ScSingleRefToken(aData));
        ScRefTokenHelper::join(rRefTokens, pCorner);
    }

    return true;
}

namespace {

/**
 * Function object to create a list of table numbers from a token list.
 */
class InsertTabNumber : public unary_function<ScSharedTokenRef, void>
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

    void operator() (const ScSharedTokenRef& pToken) const
    {
        if (!ScRefTokenHelper::isRef(pToken))
            return;

        const ScSingleRefData& r = pToken->GetSingleRef();
        mpTabNumList->push_back(r.nTab);
    }

    void getList(list<SCTAB>& rList)
    {
        mpTabNumList->swap(rList);
    }
private:
    shared_ptr< list<SCTAB> > mpTabNumList;
};

}

uno::Sequence< beans::PropertyValue > SAL_CALL ScChart2DataProvider::detectArguments(
    const uno::Reference< chart2::data::XDataSource >& xDataSource )
    throw (uno::RuntimeException)
{
    ::std::vector< beans::PropertyValue > aResult;
    bool bRowSourceDetected = false;
    bool bFirstCellAsLabel = false;
    bool bHasCategories = true;
    ::rtl::OUString sRangeRep;

    chart::ChartDataRowSource eRowSource = chart::ChartDataRowSource_COLUMNS;

    vector<ScSharedTokenRef> aTokens;

    // CellRangeRepresentation
    {
        ScUnoGuard aGuard;
        DBG_ASSERT( m_pDocument, "No Document -> no detectArguments" );
        if(!m_pDocument)
            return lcl_VectorToSequence( aResult );

        detectRangesFromDataSource(aTokens, eRowSource, bRowSourceDetected, xDataSource);
    }

    // TableNumberList
    {
        list<SCTAB> aTableNumList;
        InsertTabNumber func;
        func = for_each(aTokens.begin(), aTokens.end(), func);
        func.getList(aTableNumList);
        aResult.push_back(
            beans::PropertyValue( ::rtl::OUString::createFromAscii("TableNumberList"), -1,
                                  uno::makeAny( lcl_createTableNumberList( aTableNumList ) ),
                                  beans::PropertyState_DIRECT_VALUE ));
    }

    // DataRowSource (calculated before)
    if( bRowSourceDetected )
    {
        aResult.push_back(
            beans::PropertyValue( ::rtl::OUString::createFromAscii("DataRowSource"), -1,
                                  uno::makeAny( eRowSource ), beans::PropertyState_DIRECT_VALUE ));
    }

    // HasCategories
    if( bRowSourceDetected )
    {
        if( lcl_HasCategories( xDataSource, bHasCategories ))
        {
            aResult.push_back(
                beans::PropertyValue( ::rtl::OUString::createFromAscii("HasCategories"), -1,
                                      uno::makeAny( bHasCategories ), beans::PropertyState_DIRECT_VALUE ));
        }
    }

    // FirstCellAsLabel
    if( bRowSourceDetected )
    {
        lcl_HasFirstCellAsLabel( xDataSource, bFirstCellAsLabel );
        aResult.push_back(
            beans::PropertyValue( ::rtl::OUString::createFromAscii("FirstCellAsLabel"), -1,
                                  uno::makeAny( bFirstCellAsLabel ), beans::PropertyState_DIRECT_VALUE ));
    }

    // Add the left upper corner to the range if it is missing.
    if (bRowSourceDetected && bFirstCellAsLabel && bHasCategories)
        addUpperLeftCornerIfMissing(aTokens);

    // Get range string.
    lcl_convertTokensToString(sRangeRep, aTokens, m_pDocument);

    // add cell range property
    aResult.push_back(
        beans::PropertyValue( ::rtl::OUString::createFromAscii("CellRangeRepresentation"), -1,
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

            rtl::OUString aOldLabel;
            rtl::OUString aNewLabel;
            rtl::OUString aOldValues;
            rtl::OUString aNewValues;
            rtl::OUString aEmpty;

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

        if( bDifferentIndexes && aSequenceMappingVector.size() )
        {
            aResult.push_back(
                beans::PropertyValue( ::rtl::OUString::createFromAscii("SequenceMapping"), -1,
                    uno::makeAny( lcl_VectorToSequence(aSequenceMappingVector) )
                    , beans::PropertyState_DIRECT_VALUE ));
        }
    }

    return lcl_VectorToSequence( aResult );
}

::sal_Bool SAL_CALL ScChart2DataProvider::createDataSequenceByRangeRepresentationPossible( const ::rtl::OUString& aRangeRepresentation )
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if( ! m_pDocument )
        return false;

    vector<ScSharedTokenRef> aTokens;
    ScRefTokenHelper::compileRangeRepresentation(aTokens, aRangeRepresentation, m_pDocument);
    return !aTokens.empty();
}

uno::Reference< chart2::data::XDataSequence > SAL_CALL
    ScChart2DataProvider::createDataSequenceByRangeRepresentation(
    const ::rtl::OUString& aRangeRepresentation )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference< chart2::data::XDataSequence > xResult;

    DBG_ASSERT( m_pDocument, "No Document -> no createDataSequenceByRangeRepresentation" );
    if(!m_pDocument || (aRangeRepresentation.getLength() == 0))
        return xResult;

    vector<ScSharedTokenRef> aRefTokens;
    ScRefTokenHelper::compileRangeRepresentation(aRefTokens, aRangeRepresentation, m_pDocument);
    if (aRefTokens.empty())
        return xResult;

    // ScChart2DataSequence manages the life cycle of pRefTokens.
    vector<ScSharedTokenRef>* pRefTokens = new vector<ScSharedTokenRef>();
    pRefTokens->swap(aRefTokens);
    xResult.set(new ScChart2DataSequence(m_pDocument, this, pRefTokens, m_bIncludeHiddenCells));

    return xResult;
}

uno::Reference< sheet::XRangeSelection > SAL_CALL ScChart2DataProvider::getRangeSelection()
    throw (uno::RuntimeException)
{
    uno::Reference< sheet::XRangeSelection > xResult;

    uno::Reference< frame::XModel > xModel( lcl_GetXModel( m_pDocument ));
    if( xModel.is())
        xResult.set( xModel->getCurrentController(), uno::UNO_QUERY );

    return xResult;
}

/*uno::Reference< util::XNumberFormatsSupplier > SAL_CALL ScChart2DataProvider::getNumberFormatsSupplier()
    throw (uno::RuntimeException)
{
    return uno::Reference< util::XNumberFormatsSupplier >( lcl_GetXModel( m_pDocument ), uno::UNO_QUERY );
}*/

// XRangeXMLConversion ---------------------------------------------------

rtl::OUString SAL_CALL ScChart2DataProvider::convertRangeToXML( const rtl::OUString& sRangeRepresentation )
    throw ( uno::RuntimeException, lang::IllegalArgumentException )
{
    OUString aRet;
    if (!m_pDocument)
        return aRet;

    if (!sRangeRepresentation.getLength())
        // Empty data range is allowed.
        return aRet;

    vector<ScSharedTokenRef> aRefTokens;
    ScRefTokenHelper::compileRangeRepresentation(aRefTokens, sRangeRepresentation, m_pDocument);
    if (aRefTokens.empty())
        throw lang::IllegalArgumentException();

    Tokens2RangeStringXML converter(m_pDocument);
    converter = for_each(aRefTokens.begin(), aRefTokens.end(), converter);
    converter.getString(aRet);

    return aRet;
}

rtl::OUString SAL_CALL ScChart2DataProvider::convertRangeFromXML( const rtl::OUString& sXMLRange )
    throw ( uno::RuntimeException, lang::IllegalArgumentException )
{
    const sal_Unicode cSep = ' ';
    const sal_Unicode cQuote = '\'';

    if (!m_pDocument)
    {
        // #i74062# When loading flat XML, this is called before the referenced sheets are in the document,
        // so the conversion has to take place directly with the strings, without looking up the sheets.

        rtl::OUStringBuffer sRet;
        sal_Int32 nOffset = 0;
        while( nOffset >= 0 )
        {
            rtl::OUString sToken;
            ScRangeStringConverter::GetTokenByOffset( sToken, sXMLRange, nOffset, cSep, cQuote );
            if( nOffset >= 0 )
            {
                // convert one address (remove dots)

                String aUIString(sToken);

                sal_Int32 nIndex = ScRangeStringConverter::IndexOf( sToken, ':', 0, cQuote );
                if ( nIndex >= 0 && nIndex < aUIString.Len() - 1 &&
                        aUIString.GetChar((xub_StrLen)nIndex + 1) == (sal_Unicode) '.' )
                    aUIString.Erase( (xub_StrLen)nIndex + 1, 1 );

                if ( aUIString.GetChar(0) == (sal_Unicode) '.' )
                    aUIString.Erase( 0, 1 );

                if( sRet.getLength() )
                    sRet.append( (sal_Unicode) ';' );
                sRet.append( aUIString );
            }
        }

        return sRet.makeStringAndClear();
    }

    OUString aRet;
    ScRangeStringConverter::GetStringFromXMLRangeString(aRet, sXMLRange, m_pDocument);
    return aRet;
}

namespace {

class CollectRefTokens : public ::std::unary_function<ScSharedTokenRef, void>
{
public:
    CollectRefTokens() :
        mpRefTokens(new vector<ScSharedTokenRef>()),
        mnDataInRows(0),
        mnDataInCols(0),
        mbRowSourceAmbiguous(false)
    {
    }

    CollectRefTokens(const CollectRefTokens& r) :
        mpRefTokens(r.mpRefTokens),
        mnDataInRows(r.mnDataInRows),
        mnDataInCols(r.mnDataInCols),
        mbRowSourceAmbiguous(r.mbRowSourceAmbiguous)
    {
    }

    void operator() (const ScSharedTokenRef& rRefToken)
    {
        if (!mbRowSourceAmbiguous)
        {
            StackVar eVar = rRefToken->GetType();
            if (eVar == svDoubleRef || eVar == svExternalDoubleRef)
            {
                const ScComplexRefData& r = rRefToken->GetDoubleRef();
                mbRowSourceAmbiguous = r.Ref1.nTab != r.Ref2.nTab;
                if (!mbRowSourceAmbiguous)
                {
                    bool bColDiff = (r.Ref2.nCol - r.Ref1.nCol) != 0;
                    bool bRowDiff = (r.Ref2.nRow - r.Ref1.nRow) != 0;

                    if (bColDiff && !bRowDiff)
                        ++mnDataInRows;
                    else if (bRowDiff && !bColDiff)
                        ++mnDataInCols;
                    else if (bRowDiff && bColDiff)
                        mbRowSourceAmbiguous = true;

                    if (mnDataInRows > 0 && mnDataInCols > 0)
                        mbRowSourceAmbiguous = true;
                }
            }
        }

        mpRefTokens->push_back(rRefToken);
    }

    void appendTokens(vector<ScSharedTokenRef>& rTokens)
    {
        vector<ScSharedTokenRef> aNewTokens = rTokens;
        vector<ScSharedTokenRef>::const_iterator itr = mpRefTokens->begin(), itrEnd = mpRefTokens->end();
        for (; itr != itrEnd; ++itr)
            ScRefTokenHelper::join(aNewTokens, *itr);

        rTokens.swap(aNewTokens);
    }

    bool isRowSourceAmbiguous() const
    {
        return mbRowSourceAmbiguous;
    }

    sal_uInt32 getDataInRows() const
    {
        return mnDataInRows;
    }

    sal_uInt32 getDataInCols() const
    {
        return mnDataInCols;
    }

private:
    shared_ptr< vector<ScSharedTokenRef> > mpRefTokens;
    sal_uInt32 mnDataInRows;
    sal_uInt32 mnDataInCols;
    bool mbRowSourceAmbiguous;
};

}

// DataProvider XPropertySet -------------------------------------------------

uno::Reference< beans::XPropertySetInfo> SAL_CALL
ScChart2DataProvider::getPropertySetInfo() throw( uno::RuntimeException)
{
    ScUnoGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( m_aPropSet.getPropertyMap() );
    return aRef;
}


void SAL_CALL ScChart2DataProvider::setPropertyValue(
        const ::rtl::OUString& rPropertyName, const uno::Any& rValue)
            throw( beans::UnknownPropertyException,
                    beans::PropertyVetoException,
                    lang::IllegalArgumentException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    if ( rPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( SC_UNONAME_INCLUDEHIDDENCELLS)))
    {
        if ( !(rValue >>= m_bIncludeHiddenCells))
            throw lang::IllegalArgumentException();
    }
    else
        throw beans::UnknownPropertyException();
}


uno::Any SAL_CALL ScChart2DataProvider::getPropertyValue(
        const ::rtl::OUString& rPropertyName)
            throw( beans::UnknownPropertyException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    uno::Any aRet;
    if ( rPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( SC_UNONAME_INCLUDEHIDDENCELLS)))
        aRet <<= m_bIncludeHiddenCells;
    else
        throw beans::UnknownPropertyException();
    return aRet;
}


void SAL_CALL ScChart2DataProvider::addPropertyChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener>& /*xListener*/)
            throw( beans::UnknownPropertyException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_ENSURE( false, "Not yet implemented" );
}


void SAL_CALL ScChart2DataProvider::removePropertyChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener>& /*rListener*/)
            throw( beans::UnknownPropertyException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_ENSURE( false, "Not yet implemented" );
}


void SAL_CALL ScChart2DataProvider::addVetoableChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener>& /*rListener*/)
            throw( beans::UnknownPropertyException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_ENSURE( false, "Not yet implemented" );
}


void SAL_CALL ScChart2DataProvider::removeVetoableChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener>& /*rListener*/ )
            throw( beans::UnknownPropertyException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_ENSURE( false, "Not yet implemented" );
}

void ScChart2DataProvider::detectRangesFromDataSource(vector<ScSharedTokenRef>& rRefTokens,
                                                      chart::ChartDataRowSource& rRowSource,
                                                      bool& rRowSourceDetected,
                                                      const Reference<chart2::data::XDataSource>& xDataSource)
{
    if (!m_pDocument)
        return;

    sal_Int32 nDataInRows = 0;
    sal_Int32 nDataInCols = 0;
    bool bRowSourceAmbiguous = false;

    vector<OUString> aRangeReps = lcl_getRangeRepresentationsFromDataSource(xDataSource);
    for (vector<OUString>::const_iterator itr = aRangeReps.begin(), itrEnd = aRangeReps.end();
          itr != itrEnd; ++itr)
    {
        const OUString& rRangeRep = *itr;
        vector<ScSharedTokenRef> aTokens;
        ScRefTokenHelper::compileRangeRepresentation(aTokens, rRangeRep, m_pDocument);

        CollectRefTokens func;
        func = for_each(aTokens.begin(), aTokens.end(), func);
        func.appendTokens(rRefTokens);
        bRowSourceAmbiguous = bRowSourceAmbiguous || func.isRowSourceAmbiguous();
        if (!bRowSourceAmbiguous)
        {
            nDataInRows += func.getDataInRows();
            nDataInCols += func.getDataInCols();
        }
    }

    if (!bRowSourceAmbiguous)
    {
        rRowSourceDetected = true;
        rRowSource = ( nDataInRows > 0
                       ? chart::ChartDataRowSource_ROWS
                       : chart::ChartDataRowSource_COLUMNS );
    }
    else
    {
        // set DataRowSource to the better of the two ambiguities
        rRowSource = ( nDataInRows > nDataInCols
                       ? chart::ChartDataRowSource_ROWS
                       : chart::ChartDataRowSource_COLUMNS );
    }
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
    if ( m_pDocument )
        m_pDocument->RemoveUnoObject( *this);
}


void ScChart2DataSource::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint)
{
    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        m_pDocument = NULL;
    }
}


uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence> > SAL_CALL
ScChart2DataSource::getDataSequences() throw ( uno::RuntimeException)
{
    ScUnoGuard aGuard;

    LabeledList::const_iterator aItr(m_aLabeledSequences.begin());
    LabeledList::const_iterator aEndItr(m_aLabeledSequences.end());

    uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > aRet(m_aLabeledSequences.size());

    sal_Int32 i = 0;
    while (aItr != aEndItr)
    {
        aRet[i] = *aItr;
        ++i;
        ++aItr;
    }

    return aRet;

/*    typedef ::std::vector< uno::Reference< chart2::data::XLabeledDataSequence > > tVec;
    tVec aVec;
    bool bSeries = false;
    // split into columns - FIXME: different if GlueState() is used
    for ( ScRangePtr p = m_xRanges->First(); p; p = m_xRanges->Next())
    {
        for ( SCCOL nCol = p->aStart.Col(); nCol <= p->aEnd.Col(); ++nCol)
        {
            uno::Reference< chart2::data::XLabeledDataSequence > xLabeledSeq(
                new ScChart2LabeledDataSequence( m_pDocument));
            if( xLabeledSeq.is())
            {
                aVec.push_back( xLabeledSeq );
                if( bSeries )
                {
                    ScRangeListRef aColRanges = new ScRangeList;
                    // one single sheet selected assumed for now
                    aColRanges->Append( ScRange( nCol, p->aStart.Row(),
                                                 p->aStart.Tab(), nCol, p->aStart.Row(),
                                                 p->aStart.Tab()));
                    // TEST: add range two times, once as label, once as data
                    // TODO: create pure Numerical and Text sequences if possible
                    uno::Reference< chart2::data::XDataSequence > xLabel(
                        new ScChart2DataSequence( m_pDocument, aColRanges));

                    // set role
                    uno::Reference< beans::XPropertySet > xProp( xLabel, uno::UNO_QUERY );
                    if( xProp.is())
                        xProp->setPropertyValue(
                            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Role" )),
                            ::uno::makeAny( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "label" ))));

                    xLabeledSeq->setLabel( xLabel );
                }

                ScRangeListRef aColRanges = new ScRangeList;

                // one single sheet selected assumed for now
                aColRanges->Append( ScRange( nCol, p->aStart.Row() + 1,
                                             p->aStart.Tab(), nCol, p->aEnd.Row(),
                                             p->aStart.Tab()));
                uno::Reference< chart2::data::XDataSequence > xData(
                    new ScChart2DataSequence( m_pDocument, aColRanges));

                // set role
                uno::Reference< beans::XPropertySet > xProp( xData, uno::UNO_QUERY );
                if( xProp.is())
                    xProp->setPropertyValue(
                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Role" )),
                        ::uno::makeAny( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "values" ))));

                xLabeledSeq->setValues( xData );

                bSeries = true;
            }
        }
    }
    uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence> > aSequences(
            aVec.size());
    uno::Reference< chart2::data::XLabeledDataSequence> * pArr = aSequences.getArray();
    sal_Int32 j = 0;
    for ( tVec::const_iterator iSeq = aVec.begin(); iSeq != aVec.end();
            ++iSeq, ++j)
    {
        pArr[j] = *iSeq;
    }
    return aSequences;*/
}

void ScChart2DataSource::AddLabeledSequence(const uno::Reference < chart2::data::XLabeledDataSequence >& xNew)
{
    m_aLabeledSequences.push_back(xNew);
}

// LabeledDataSequence =======================================================

ScChart2LabeledDataSequence::ScChart2LabeledDataSequence(
    ScDocument* pDoc ) :
        m_pDocument( pDoc )
{
    if ( m_pDocument )
        m_pDocument->AddUnoObject( *this);
}

ScChart2LabeledDataSequence::~ScChart2LabeledDataSequence()
{
    if ( m_pDocument )
        m_pDocument->RemoveUnoObject( *this);
}

// SfxListener -----------------------------------------------------------

void ScChart2LabeledDataSequence::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint)
{
    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        ScUnoGuard aGuard;
        m_pDocument = NULL;
    }
}

// XLabeledDataSequence --------------------------------------------------

uno::Reference< chart2::data::XDataSequence > SAL_CALL ScChart2LabeledDataSequence::getValues()
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return m_aData;
}

void SAL_CALL ScChart2LabeledDataSequence::setValues(
    const uno::Reference< chart2::data::XDataSequence >& xSequence )
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    m_aData = xSequence;
}

uno::Reference< chart2::data::XDataSequence > SAL_CALL ScChart2LabeledDataSequence::getLabel()
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return m_aLabel;
}

void SAL_CALL ScChart2LabeledDataSequence::setLabel(
    const uno::Reference< chart2::data::XDataSequence >& xSequence )
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    m_aLabel = xSequence;
}

// XCloneable ================================================================

uno::Reference< util::XCloneable > SAL_CALL ScChart2LabeledDataSequence::createClone()
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference< util::XCloneable > xToClone(m_aData, uno::UNO_QUERY);
    if (xToClone.is())
    {
        ScChart2LabeledDataSequence* pRet = new ScChart2LabeledDataSequence(m_pDocument);
        uno::Reference< chart2::data::XDataSequence > xSequence(xToClone->createClone(), uno::UNO_QUERY);
        pRet->setValues(xSequence);
        xToClone.set(m_aLabel, uno::UNO_QUERY);
        if(xToClone.is())
        {
            xSequence.set(xToClone->createClone(), uno::UNO_QUERY);
            pRet->setLabel(xSequence);
        }
        return pRet;
    }
    return NULL;
}

// XModifyBroadcaster ========================================================

void SAL_CALL ScChart2LabeledDataSequence::addModifyListener( const uno::Reference< util::XModifyListener >& aListener )
    throw (uno::RuntimeException)
{
    // quick'n dirty: just add the listener to each DataSequence

    uno::Reference<util::XModifyBroadcaster> xDataBroadcaster( m_aData, uno::UNO_QUERY );
    if ( xDataBroadcaster.is() )
        xDataBroadcaster->addModifyListener( aListener );
    uno::Reference<util::XModifyBroadcaster> xLabelBroadcaster( m_aLabel, uno::UNO_QUERY );
    if ( xLabelBroadcaster.is() )
        xLabelBroadcaster->addModifyListener( aListener );
}

void SAL_CALL ScChart2LabeledDataSequence::removeModifyListener( const uno::Reference< util::XModifyListener >& aListener )
    throw (uno::RuntimeException)
{
    uno::Reference<util::XModifyBroadcaster> xDataBroadcaster( m_aData, uno::UNO_QUERY );
    if ( xDataBroadcaster.is() )
        xDataBroadcaster->removeModifyListener( aListener );
    uno::Reference<util::XModifyBroadcaster> xLabelBroadcaster( m_aLabel, uno::UNO_QUERY );
    if ( xLabelBroadcaster.is() )
        xLabelBroadcaster->removeModifyListener( aListener );
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
        vector<ScSharedTokenRef>* pTokens,
        bool bIncludeHiddenCells )
    : m_bIncludeHiddenCells( bIncludeHiddenCells)
    , m_nObjectId( 0 )
    , m_pDocument( pDoc)
    , m_pTokens(pTokens)
    , m_pRangeIndices(NULL)
    , m_pExtRefListener(NULL)
    , m_xDataProvider( xDP)
    , m_aPropSet(lcl_GetDataSequencePropertyMap())
    , m_pHiddenListener(NULL)
    , m_pValueListener( NULL )
    , m_bGotDataChangedHint(false)
    , m_bExtDataRebuildQueued(false)
{
    DBG_ASSERT(pTokens, "reference token list is null");

    if ( m_pDocument )
    {
        m_pDocument->AddUnoObject( *this);
        m_nObjectId = m_pDocument->GetNewUnoId();
    }
    // FIXME: real implementation of identifier and it's mapping to ranges.
    // Reuse ScChartListener?

    // BM: don't use names of named ranges but the UI range strings
//  String  aStr;
//  rRangeList->Format( aStr, SCR_ABS_3D, m_pDocument );
//    m_aIdentifier = ::rtl::OUString( aStr );

//      m_aIdentifier = ::rtl::OUString::createFromAscii( "ID_");
//      static sal_Int32 nID = 0;
//      m_aIdentifier += ::rtl::OUString::valueOf( ++nID);
}

ScChart2DataSequence::~ScChart2DataSequence()
{
    if ( m_pDocument )
    {
        m_pDocument->RemoveUnoObject( *this);
        if (m_pHiddenListener.get())
        {
            ScChartListenerCollection* pCLC = m_pDocument->GetChartListenerCollection();
            if (pCLC)
                pCLC->EndListeningHiddenRange(m_pHiddenListener.get());
        }
        StopListeningToAllExternalRefs();
    }

    delete m_pValueListener;
}

void ScChart2DataSequence::RefChanged()
{
    if( m_pValueListener && m_aValueListeners.Count() != 0 )
    {
        m_pValueListener->EndListeningAll();

        if( m_pDocument )
        {
            ScChartListenerCollection* pCLC = NULL;
            if (m_pHiddenListener.get())
            {
                pCLC = m_pDocument->GetChartListenerCollection();
                if (pCLC)
                    pCLC->EndListeningHiddenRange(m_pHiddenListener.get());
            }

            vector<ScSharedTokenRef>::const_iterator itr = m_pTokens->begin(), itrEnd = m_pTokens->end();
            for (; itr != itrEnd; ++itr)
            {
                ScRange aRange;
                if (!ScRefTokenHelper::getRangeFromToken(aRange, *itr))
                    continue;

                m_pDocument->StartListeningArea(aRange, m_pValueListener);
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

    if (!m_pTokens.get())
    {
        DBG_ERROR("m_pTokens == NULL!  Something is wrong.");
        return;
    }

    StopListeningToAllExternalRefs();

    ::std::list<sal_Int32> aHiddenValues;
    sal_Int32 nDataCount = 0;
    sal_Int32 nHiddenValueCount = 0;

    for (vector<ScSharedTokenRef>::const_iterator itr = m_pTokens->begin(), itrEnd = m_pTokens->end();
          itr != itrEnd; ++itr)
    {
        if (ScRefTokenHelper::isExternalRef(*itr))
        {
            nDataCount += FillCacheFromExternalRef(*itr);
        }
        else
        {
            ScRange aRange;
            if (!ScRefTokenHelper::getRangeFromToken(aRange, *itr))
                continue;

            for (SCTAB nTab = aRange.aStart.Tab(); nTab <= aRange.aEnd.Tab(); ++nTab)
            {
                for (SCCOL nCol = aRange.aStart.Col(); nCol <= aRange.aEnd.Col(); ++nCol)
                {
                    for (SCROW nRow = aRange.aStart.Row(); nRow <= aRange.aEnd.Row(); ++nRow)
                    {
                        bool bColHidden = (m_pDocument->GetColFlags(nCol, nTab) & CR_HIDDEN);
                        bool bRowHidden = (m_pDocument->GetRowFlags(nRow, nTab) & CR_HIDDEN);
                        if (bColHidden || bRowHidden)
                        {
                            // hidden cell
                            ++nHiddenValueCount;
                            aHiddenValues.push_back(nDataCount-1);

                            if( !m_bIncludeHiddenCells )
                                continue;
                        }

                        m_aDataArray.push_back(Item());
                        Item& rItem = m_aDataArray.back();
                        ++nDataCount;

                        ScAddress aAdr(nCol, nRow, nTab);
                        ScBaseCell* pCell = m_pDocument->GetCell(aAdr);
                        if (!pCell)
                            continue;

                        if (pCell->HasStringData())
                            rItem.maString = pCell->GetStringData();
                        else
                        {
                            String aStr;
                            m_pDocument->GetString(nCol, nRow, nTab, aStr);
                            rItem.maString = aStr;
                        }

                        switch (pCell->GetCellType())
                        {
                            case CELLTYPE_VALUE:
                                rItem.mfValue = static_cast< ScValueCell*>(pCell)->GetValue();
                                rItem.mbIsValue = true;
                            break;
                            case CELLTYPE_FORMULA:
                            {
                                ScFormulaCell* pFCell = static_cast<ScFormulaCell*>(pCell);
                                USHORT nErr = pFCell->GetErrCode();
                                if (nErr)
                                    break;

                                if (pFCell->HasValueData())
                                {
                                    rItem.mfValue = pFCell->GetValue();
                                    rItem.mbIsValue = true;
                                }
                            }
                            break;
#if DBG_UTIL
                            case CELLTYPE_DESTROYED:
#endif
                            case CELLTYPE_EDIT:
                            case CELLTYPE_NONE:
                            case CELLTYPE_NOTE:
                            case CELLTYPE_STRING:
                            case CELLTYPE_SYMBOLS:
                            default:
                                ; // do nothing
                        }
                    }
                }
            }
        }
    }

    // convert the hidden cell list to sequence.
    m_aHiddenValues.realloc(nHiddenValueCount);
    sal_Int32* pArr = m_aHiddenValues.getArray();
    ::std::list<sal_Int32>::const_iterator itr = aHiddenValues.begin(), itrEnd = aHiddenValues.end();
    for (;itr != itrEnd; ++itr, ++pArr)
        *pArr = *itr;
}

void ScChart2DataSequence::RebuildDataCache()
{
    if (!m_bExtDataRebuildQueued)
    {
        m_aDataArray.clear();
        m_pDocument->BroadcastUno(ScHint(SC_HINT_DATACHANGED, ScAddress(), NULL));
        m_bExtDataRebuildQueued = true;
        m_bGotDataChangedHint = true;
    }
}

sal_Int32 ScChart2DataSequence::FillCacheFromExternalRef(const ScSharedTokenRef& pToken)
{
    ScExternalRefManager* pRefMgr = m_pDocument->GetExternalRefManager();
    ScRange aRange;
    if (!ScRefTokenHelper::getRangeFromToken(aRange, pToken, true))
        return 0;

    sal_uInt16 nFileId = pToken->GetIndex();
    const String& rTabName = pToken->GetString();
    ScExternalRefCache::TokenArrayRef pArray = pRefMgr->getDoubleRefTokens(nFileId, rTabName, aRange, NULL);
    if (!pArray)
        // no external data exists for this range.
        return 0;

    // Start listening for this external document.
    ExternalRefListener* pExtRefListener = GetExtRefListener();
    pRefMgr->addLinkListener(nFileId, pExtRefListener);
    pExtRefListener->addFileId(nFileId);

    ScExternalRefCache::TableTypeRef pTable = pRefMgr->getCacheTable(nFileId, rTabName, false, NULL);
    sal_Int32 nDataCount = 0;
    for (FormulaToken* p = pArray->First(); p; p = pArray->Next())
    {
        // Cached external range is always represented as a single
        // matrix token, although that might change in the future when
        // we introduce a new token type to store multi-table range
        // data.

        if (p->GetType() != svMatrix)
        {
            DBG_ERROR("Cached array is not a matrix token.");
            continue;
        }

        const ScMatrix* pMat = static_cast<ScToken*>(p)->GetMatrix();
        SCSIZE nCSize, nRSize;
        pMat->GetDimensions(nCSize, nRSize);
        for (SCSIZE nC = 0; nC < nCSize; ++nC)
        {
            for (SCSIZE nR = 0; nR < nRSize; ++nR)
            {
                if (pMat->IsValue(nC, nR) || pMat->IsBoolean(nC, nR))
                {
                    m_aDataArray.push_back(Item());
                    Item& rItem = m_aDataArray.back();
                    ++nDataCount;

                    rItem.mbIsValue = true;
                    rItem.mfValue = pMat->GetDouble(nC, nR);

                    SvNumberFormatter* pFormatter = m_pDocument->GetFormatTable();
                    if (pFormatter)
                    {
                        String aStr;
                        const double fVal = rItem.mfValue;
                        Color* pColor = NULL;
                        sal_uInt32 nFmt = 0;
                        if (pTable)
                        {
                            // Get the correct format index from the cache.
                            SCCOL nCol = aRange.aStart.Col() + static_cast<SCCOL>(nC);
                            SCROW nRow = aRange.aStart.Row() + static_cast<SCROW>(nR);
                            pTable->getCell(nCol, nRow, &nFmt);
                        }
                        pFormatter->GetOutputString(fVal, nFmt, aStr, &pColor);
                        rItem.maString = aStr;
                    }
                }
                else if (pMat->IsString(nC, nR))
                {
                    m_aDataArray.push_back(Item());
                    Item& rItem = m_aDataArray.back();
                    ++nDataCount;

                    rItem.mbIsValue = false;
                    rItem.maString = pMat->GetString(nC, nR);
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

    sal_uInt32 nCount = rRanges.Count();
    for (sal_uInt32 i = 0; i < nCount; ++i)
    {
        ScSharedTokenRef pToken;
        ScRange* pRange = static_cast<ScRange*>(rRanges.GetObject(i));
        DBG_ASSERT(pRange, "range object is NULL.");

        ScRefTokenHelper::getTokenFromRange(pToken, *pRange);
        sal_uInt32 nOrigPos = (*m_pRangeIndices)[i];
        (*m_pTokens)[nOrigPos] = pToken;
    }

    RefChanged();

    // any change of the range address is broadcast to value (modify) listeners
    if ( m_aValueListeners.Count() )
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

    const hash_set<sal_uInt16>& rFileIds = m_pExtRefListener->getAllFileIds();
    hash_set<sal_uInt16>::const_iterator itr = rFileIds.begin(), itrEnd = rFileIds.end();
    ScExternalRefManager* pRefMgr = m_pDocument->GetExternalRefManager();
    for (; itr != itrEnd; ++itr)
        pRefMgr->removeLinkListener(*itr, m_pExtRefListener.get());

    m_pExtRefListener.reset(NULL);
}

void ScChart2DataSequence::CopyData(const ScChart2DataSequence& r)
{
    if (!m_pDocument)
    {
        DBG_ERROR("document instance is NULL!?");
        return;
    }

    list<Item> aDataArray(r.m_aDataArray);
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
        const hash_set<sal_uInt16>& rFileIds = r.m_pExtRefListener->getAllFileIds();
        hash_set<sal_uInt16>::const_iterator itr = rFileIds.begin(), itrEnd = rFileIds.end();
        for (; itr != itrEnd; ++itr)
        {
            pRefMgr->addLinkListener(*itr, m_pExtRefListener.get());
            m_pExtRefListener->addFileId(*itr);
        }
    }
}

void ScChart2DataSequence::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint)
{
    if ( rHint.ISA( SfxSimpleHint ) )
    {
        ULONG nId = static_cast<const SfxSimpleHint&>(rHint).GetId();
        if ( nId ==SFX_HINT_DYING )
        {
            m_pDocument = NULL;
        }
        else if ( nId == SFX_HINT_DATACHANGED )
        {
            // delayed broadcast as in ScCellRangesBase

            if ( m_bGotDataChangedHint && m_pDocument )
            {
                m_aDataArray.clear();
                m_aDataArray.clear();
                lang::EventObject aEvent;
                aEvent.Source.set((cppu::OWeakObject*)this);

                if( m_pDocument )
                {
                    for ( USHORT n=0; n<m_aValueListeners.Count(); n++ )
                        m_pDocument->AddUnoListenerCall( *m_aValueListeners[n], aEvent );
                }

                m_bGotDataChangedHint = false;
            }
        }
        else if ( nId == SC_HINT_CALCALL )
        {
            // broadcast from DoHardRecalc - set m_bGotDataChangedHint
            // (SFX_HINT_DATACHANGED follows separately)

            if ( m_aValueListeners.Count() )
                m_bGotDataChangedHint = true;
        }
    }
    else if ( rHint.ISA( ScUpdateRefHint ) )
    {
        // Create a range list from the token list, have the range list
        // updated, and bring the change back to the token list.

        ScRangeList aRanges;
        m_pRangeIndices.reset(new vector<sal_uInt32>());
        vector<ScSharedTokenRef>::const_iterator itrBeg = m_pTokens->begin(), itrEnd = m_pTokens->end();
        for (vector<ScSharedTokenRef>::const_iterator itr = itrBeg ;itr != itrEnd; ++itr)
        {
            if (!ScRefTokenHelper::isExternalRef(*itr))
            {
                ScRange aRange;
                ScRefTokenHelper::getRangeFromToken(aRange, *itr);
                aRanges.Append(aRange);
                sal_uInt32 nPos = distance(itrBeg, itr);
                m_pRangeIndices->push_back(nPos);
            }
        }

        DBG_ASSERT(m_pRangeIndices->size() == static_cast<size_t>(aRanges.Count()),
                   "range list and range index list have different sizes.");

        auto_ptr<ScRangeList> pUndoRanges;
        if ( m_pDocument->HasUnoRefUndo() )
            pUndoRanges.reset(new ScRangeList(aRanges));

        const ScUpdateRefHint& rRef = (const ScUpdateRefHint&)rHint;
        bool bChanged = aRanges.UpdateReference(
            rRef.GetMode(), m_pDocument, rRef.GetRange(), rRef.GetDx(), rRef.GetDy(), rRef.GetDz());

        if (bChanged)
        {
            DBG_ASSERT(m_pRangeIndices->size() == static_cast<size_t>(aRanges.Count()),
                       "range list and range index list have different sizes after the reference update.");

            // Bring the change back from the range list to the token list.
            UpdateTokensFromRanges(aRanges);

            if (pUndoRanges.get())
                m_pDocument->AddUnoRefChange(m_nObjectId, *pUndoRanges);
        }
    }
    else if ( rHint.ISA( ScUnoRefUndoHint ) )
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
                DBG_ERROR(" faulty range indices");
                break;
            }

            const ScRangeList& rRanges = rUndoHint.GetRanges();

            sal_uInt32 nCount = rRanges.Count();
            if (nCount != m_pRangeIndices->size())
            {
                DBG_ERROR("range count and range index count differ.");
                break;
            }

            UpdateTokensFromRanges(rRanges);
        }
        while (false);
    }
}


IMPL_LINK( ScChart2DataSequence, ValueListenerHdl, SfxHint*, pHint )
{
    if ( m_pDocument && pHint && pHint->ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint*)pHint)->GetId() & (SC_HINT_DATACHANGED | SC_HINT_DYING) )
    {
        //  This may be called several times for a single change, if several formulas
        //  in the range are notified. So only a flag is set that is checked when
        //  SFX_HINT_DATACHANGED is received.

        setDataChangedHint(true);
    }
    return 0;
}

// ----------------------------------------------------------------------------

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
            removeFileId(nFileId);
        break;
    }
}

void ScChart2DataSequence::ExternalRefListener::addFileId(sal_uInt16 nFileId)
{
    maFileIds.insert(nFileId);
}

void ScChart2DataSequence::ExternalRefListener::removeFileId(sal_uInt16 nFileId)
{
    maFileIds.erase(nFileId);
}

const hash_set<sal_uInt16>& ScChart2DataSequence::ExternalRefListener::getAllFileIds()
{
    return maFileIds;
}

// ----------------------------------------------------------------------------

uno::Sequence< uno::Any> SAL_CALL ScChart2DataSequence::getData()
            throw ( uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if ( !m_pDocument)
        throw uno::RuntimeException();

    BuildDataCache();
    sal_Int32 nCount = m_aDataArray.size();
    uno::Sequence<uno::Any> aSeq(nCount);
    uno::Any* pArr = aSeq.getArray();
    ::std::list<Item>::const_iterator itr = m_aDataArray.begin(), itrEnd = m_aDataArray.end();
    for (; itr != itrEnd; ++itr, ++pArr)
    {
        if (itr->mbIsValue)
            *pArr <<= itr->mfValue;
        else
            *pArr <<= itr->maString;
    }

    return aSeq;
}

// XNumericalDataSequence --------------------------------------------------

uno::Sequence< double > SAL_CALL ScChart2DataSequence::getNumericalData()
            throw ( uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if ( !m_pDocument)
        throw uno::RuntimeException();

    BuildDataCache();

    double fNAN;
    ::rtl::math::setNan(&fNAN);

    sal_Int32 nCount = m_aDataArray.size();
    uno::Sequence<double> aSeq(nCount);
    double* pArr = aSeq.getArray();
    ::std::list<Item>::const_iterator itr = m_aDataArray.begin(), itrEnd = m_aDataArray.end();
    for (; itr != itrEnd; ++itr, ++pArr)
        *pArr = itr->mbIsValue ? itr->mfValue : fNAN;

    return aSeq;
}

// XTextualDataSequence --------------------------------------------------

uno::Sequence< rtl::OUString > SAL_CALL ScChart2DataSequence::getTextualData(  ) throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if ( !m_pDocument)
        throw uno::RuntimeException();

    BuildDataCache();

    sal_Int32 nCount = m_aDataArray.size();
    uno::Sequence<rtl::OUString> aSeq(nCount);
    rtl::OUString* pArr = aSeq.getArray();
    ::std::list<Item>::const_iterator itr = m_aDataArray.begin(), itrEnd = m_aDataArray.end();
    for (; itr != itrEnd; ++itr, ++pArr)
        *pArr = itr->maString;

    return aSeq;
}

::rtl::OUString SAL_CALL ScChart2DataSequence::getSourceRangeRepresentation()
            throw ( uno::RuntimeException)
{
    ScUnoGuard aGuard;
    OUString aStr;
    DBG_ASSERT( m_pDocument, "No Document -> no SourceRangeRepresentation" );
    if (m_pDocument && m_pTokens.get())
        lcl_convertTokensToString(aStr, *m_pTokens, m_pDocument);

    return aStr;
}

namespace {

/**
 * This function object is used to accumulatively count the numbers of
 * columns and rows in all reference tokens.
 */
class AccumulateRangeSize : public unary_function<ScSharedTokenRef, void>
{
public:
    AccumulateRangeSize() :
        mnCols(0), mnRows(0) {}

    AccumulateRangeSize(const AccumulateRangeSize& r) :
        mnCols(r.mnCols), mnRows(r.mnRows) {}

    void operator() (const ScSharedTokenRef& pToken)
    {
        ScRange r;
        bool bExternal = ScRefTokenHelper::isExternalRef(pToken);
        ScRefTokenHelper::getRangeFromToken(r, pToken, bExternal);
        r.Justify();
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
class GenerateLabelStrings : public unary_function<ScSharedTokenRef, void>
{
public:
    GenerateLabelStrings(sal_Int32 nSize, chart2::data::LabelOrigin eOrigin, bool bColumn) :
        mpLabels(new Sequence<OUString>(nSize)),
        meOrigin(eOrigin),
        mnCount(0),
        mbColumn(bColumn) {}

    GenerateLabelStrings(const GenerateLabelStrings& r) :
        mpLabels(r.mpLabels),
        meOrigin(r.meOrigin),
        mnCount(r.mnCount),
        mbColumn(r.mbColumn) {}

    void operator() (const ScSharedTokenRef& pToken)
    {
        bool bExternal = ScRefTokenHelper::isExternalRef(pToken);
        ScRange aRange;
        ScRefTokenHelper::getRangeFromToken(aRange, pToken, bExternal);
        OUString* pArr = mpLabels->getArray();
        if (mbColumn)
        {
            for (SCCOL nCol = aRange.aStart.Col(); nCol <= aRange.aEnd.Col(); ++nCol)
            {
                if ( meOrigin != chart2::data::LabelOrigin_LONG_SIDE)
                {
                    String aString = ScGlobal::GetRscString(STR_COLUMN);
                    aString += ' ';
                    ScAddress aPos( nCol, 0, 0 );
                    String aColStr;
                    aPos.Format( aColStr, SCA_VALID_COL, NULL );
                    aString += aColStr;
                    pArr[mnCount] = aString;
                }
                else //only indices for categories
                    pArr[mnCount] = String::CreateFromInt32( mnCount+1 );
                ++mnCount;
            }
        }
        else
        {
            for (sal_Int32 nRow = aRange.aStart.Row(); nRow <= aRange.aEnd.Row(); ++nRow)
            {
                if (meOrigin != chart2::data::LabelOrigin_LONG_SIDE)
                {
                    String aString = ScGlobal::GetRscString(STR_ROW);
                    aString += ' ';
                    aString += String::CreateFromInt32( nRow+1 );
                    pArr[mnCount] = aString;
                }
                else //only indices for categories
                    pArr[mnCount] = String::CreateFromInt32( mnCount+1 );
                ++mnCount;
            }
        }
    }

    Sequence<OUString> getLabels() const { return *mpLabels; }

private:
    GenerateLabelStrings(); // disabled

    shared_ptr< Sequence<OUString> >    mpLabels;
    chart2::data::LabelOrigin           meOrigin;
    sal_Int32                           mnCount;
    bool                                mbColumn;
};

}

uno::Sequence< ::rtl::OUString > SAL_CALL ScChart2DataSequence::generateLabel(chart2::data::LabelOrigin eOrigin)
        throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if ( !m_pDocument)
        throw uno::RuntimeException();

    if (!m_pTokens.get())
        return Sequence<OUString>();

    // Determine the total size of all ranges.
    AccumulateRangeSize func;
    func = for_each(m_pTokens->begin(), m_pTokens->end(), func);
    SCCOL nCols = func.getCols();
    SCROW nRows = func.getRows();

    // Detemine whether this is column-major or row-major.
    bool bColumn = true;
    if ((eOrigin == chart2::data::LabelOrigin_SHORT_SIDE) ||
        (eOrigin == chart2::data::LabelOrigin_LONG_SIDE))
    {
        if (nRows > nCols)
        {
            if (eOrigin == chart2::data::LabelOrigin_SHORT_SIDE)
                bColumn = true;
            else
                bColumn = false;
        }
        else if (nCols > nRows)
        {
            if (eOrigin == chart2::data::LabelOrigin_SHORT_SIDE)
                bColumn = false;
            else
                bColumn = true;
        }
        else
            return Sequence<OUString>();
    }

    // Generate label strings based on the info so far.
    sal_Int32 nCount = bColumn ? nCols : nRows;
    GenerateLabelStrings genLabels(nCount, eOrigin, bColumn);
    genLabels = for_each(m_pTokens->begin(), m_pTokens->end(), genLabels);
    Sequence<OUString> aSeq = genLabels.getLabels();

    return aSeq;
}

::sal_Int32 SAL_CALL ScChart2DataSequence::getNumberFormatKeyByIndex( ::sal_Int32 nIndex )
    throw (lang::IndexOutOfBoundsException,
           uno::RuntimeException)
{
    // index -1 means a heuristic value for the entire sequence
    bool bGetSeriesFormat = (nIndex == -1);
    sal_Int32 nResult = 0;

    ScUnoGuard aGuard;
    if ( !m_pDocument || !m_pTokens.get())
        return nResult;

    sal_Int32 nCount = 0;
    bool bFound = false;
    ScRangePtr p;

    uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( lcl_GetSpreadSheetDocument( m_pDocument ));
    if (!xSpreadDoc.is())
        return nResult;

    uno::Reference<container::XIndexAccess> xIndex( xSpreadDoc->getSheets(), uno::UNO_QUERY );
    if (!xIndex.is())
        return nResult;

    ScRangeList aRanges;
    ScRefTokenHelper::getRangeListFromTokens(aRanges, *m_pTokens);
    uno::Reference< table::XCellRange > xSheet;
    for ( p = aRanges.First(); p && !bFound; p = aRanges.Next())
    {
        // TODO: use DocIter?
        table::CellAddress aStart, aEnd;
        ScUnoConversion::FillApiAddress( aStart, p->aStart );
        ScUnoConversion::FillApiAddress( aEnd, p->aEnd );
        for ( sal_Int16 nSheet = aStart.Sheet; nSheet <= aEnd.Sheet && !bFound; ++nSheet)
        {
            xSheet.set(xIndex->getByIndex(nSheet), uno::UNO_QUERY);
            for ( sal_Int32 nCol = aStart.Column; nCol <= aEnd.Column && !bFound; ++nCol)
            {
                for ( sal_Int32 nRow = aStart.Row; nRow <= aEnd.Row && !bFound; ++nRow)
                {
                    if( bGetSeriesFormat )
                    {
                        // TODO: use nicer heuristic
                        // return format of first non-empty cell
                        uno::Reference< text::XText > xText(
                            xSheet->getCellByPosition(nCol, nRow), uno::UNO_QUERY);
                        if (xText.is() && xText->getString().getLength())
                        {
                            uno::Reference< beans::XPropertySet > xProp(xText, uno::UNO_QUERY);
                            if( xProp.is())
                                xProp->getPropertyValue(
                                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NumberFormat"))) >>= nResult;
                            bFound = true;
                            break;
                        }
                    }
                    else if( nCount == nIndex )
                    {
                        uno::Reference< beans::XPropertySet > xProp(
                            xSheet->getCellByPosition(nCol, nRow), uno::UNO_QUERY);
                        if( xProp.is())
                            xProp->getPropertyValue(
                                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NumberFormat"))) >>= nResult;
                        bFound = true;
                        break;
                    }
                    ++nCount;
                }
            }
        }
    }

    return nResult;
}

// XCloneable ================================================================

uno::Reference< util::XCloneable > SAL_CALL ScChart2DataSequence::createClone()
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;

    auto_ptr< vector<ScSharedTokenRef> > pTokensNew;
    if (m_pTokens.get())
    {
        // Clone tokens.
        pTokensNew.reset(new vector<ScSharedTokenRef>);
        pTokensNew->reserve(m_pTokens->size());
        vector<ScSharedTokenRef>::const_iterator itr = m_pTokens->begin(), itrEnd = m_pTokens->end();
        for (; itr != itrEnd; ++itr)
        {
            ScSharedTokenRef p(static_cast<ScToken*>((*itr)->Clone()));
            pTokensNew->push_back(p);
        }
    }

    auto_ptr<ScChart2DataSequence> p(new ScChart2DataSequence(m_pDocument, m_xDataProvider, pTokensNew.release(), m_bIncludeHiddenCells));
    p->CopyData(*this);
    Reference< util::XCloneable > xClone(p.release());

    return xClone;
}

// XModifyBroadcaster ========================================================

void SAL_CALL ScChart2DataSequence::addModifyListener( const uno::Reference< util::XModifyListener >& aListener )
    throw (uno::RuntimeException)
{
    // like ScCellRangesBase::addModifyListener
    ScUnoGuard aGuard;
    if (!m_pTokens.get() || m_pTokens->empty())
        return;

    ScRangeList aRanges;
    ScRefTokenHelper::getRangeListFromTokens(aRanges, *m_pTokens);
    uno::Reference<util::XModifyListener> *pObj =
            new uno::Reference<util::XModifyListener>( aListener );
    m_aValueListeners.Insert( pObj, m_aValueListeners.Count() );

    if ( m_aValueListeners.Count() == 1 )
    {
        if (!m_pValueListener)
            m_pValueListener = new ScLinkListener( LINK( this, ScChart2DataSequence, ValueListenerHdl ) );

        if (!m_pHiddenListener.get())
            m_pHiddenListener.reset(new HiddenRangeListener(*this));

        if( m_pDocument )
        {
            ScChartListenerCollection* pCLC = m_pDocument->GetChartListenerCollection();
            vector<ScSharedTokenRef>::const_iterator itr = m_pTokens->begin(), itrEnd = m_pTokens->end();
            for (; itr != itrEnd; ++itr)
            {
                ScRange aRange;
                if (!ScRefTokenHelper::getRangeFromToken(aRange, *itr))
                    continue;

                m_pDocument->StartListeningArea( aRange, m_pValueListener );
                if (pCLC)
                    pCLC->StartListeningHiddenRange(aRange, m_pHiddenListener.get());
            }
        }

        acquire();  // don't lose this object (one ref for all listeners)
    }
}

void SAL_CALL ScChart2DataSequence::removeModifyListener( const uno::Reference< util::XModifyListener >& aListener )
    throw (uno::RuntimeException)
{
    // like ScCellRangesBase::removeModifyListener

    ScUnoGuard aGuard;
    if (!m_pTokens.get() || m_pTokens->empty())
        return;

    acquire();      // in case the listeners have the last ref - released below

    USHORT nCount = m_aValueListeners.Count();
    for ( USHORT n=nCount; n--; )
    {
        uno::Reference<util::XModifyListener> *pObj = m_aValueListeners[n];
        if ( *pObj == aListener )
        {
            m_aValueListeners.DeleteAndDestroy( n );

            if ( m_aValueListeners.Count() == 0 )
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

    release();      // might delete this object
}

// DataSequence XPropertySet -------------------------------------------------

uno::Reference< beans::XPropertySetInfo> SAL_CALL
ScChart2DataSequence::getPropertySetInfo() throw( uno::RuntimeException)
{
    ScUnoGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( m_aPropSet.getPropertyMap() );
    return aRef;
}


void SAL_CALL ScChart2DataSequence::setPropertyValue(
        const ::rtl::OUString& rPropertyName, const uno::Any& rValue)
            throw( beans::UnknownPropertyException,
                    beans::PropertyVetoException,
                    lang::IllegalArgumentException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    if ( rPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( SC_UNONAME_ROLE)))
    {
        if ( !(rValue >>= m_aRole))
            throw lang::IllegalArgumentException();
    }
    else if ( rPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( SC_UNONAME_INCLUDEHIDDENCELLS)))
    {
        sal_Bool bOldValue = m_bIncludeHiddenCells;
        if ( !(rValue >>= m_bIncludeHiddenCells))
            throw lang::IllegalArgumentException();
        if( bOldValue != m_bIncludeHiddenCells )
            m_aDataArray.clear();//data array is dirty now
    }
    else
        throw beans::UnknownPropertyException();
    // TODO: support optional properties
}


uno::Any SAL_CALL ScChart2DataSequence::getPropertyValue(
        const ::rtl::OUString& rPropertyName)
            throw( beans::UnknownPropertyException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    uno::Any aRet;
    if ( rPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( SC_UNONAME_ROLE)))
        aRet <<= m_aRole;
    else if ( rPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( SC_UNONAME_INCLUDEHIDDENCELLS)))
        aRet <<= m_bIncludeHiddenCells;
    else if ( rPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(SC_UNONAME_HIDDENVALUES)))
    {
        // This property is read-only thus cannot be set externally via
        // setPropertyValue(...).
        BuildDataCache();
        aRet <<= m_aHiddenValues;
    }
    else
        throw beans::UnknownPropertyException();
    // TODO: support optional properties
    return aRet;
}


void SAL_CALL ScChart2DataSequence::addPropertyChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener>& /*xListener*/)
            throw( beans::UnknownPropertyException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    // FIXME: real implementation
//     throw uno::RuntimeException();
    OSL_ENSURE( false, "Not yet implemented" );
}


void SAL_CALL ScChart2DataSequence::removePropertyChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener>& /*rListener*/)
            throw( beans::UnknownPropertyException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    // FIXME: real implementation
//     throw uno::RuntimeException();
    OSL_ENSURE( false, "Not yet implemented" );
}


void SAL_CALL ScChart2DataSequence::addVetoableChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener>& /*rListener*/)
            throw( beans::UnknownPropertyException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    // FIXME: real implementation
//     throw uno::RuntimeException();
    OSL_ENSURE( false, "Not yet implemented" );
}


void SAL_CALL ScChart2DataSequence::removeVetoableChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener>& /*rListener*/)
            throw( beans::UnknownPropertyException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    // FIXME: real implementation
//     throw uno::RuntimeException();
    OSL_ENSURE( false, "Not yet implemented" );
}

void ScChart2DataSequence::setDataChangedHint(bool b)
{
    m_bGotDataChangedHint = b;
}

// XUnoTunnel

// sal_Int64 SAL_CALL ScChart2DataSequence::getSomething(
//              const uno::Sequence<sal_Int8 >& rId ) throw(uno::RuntimeException)
// {
//  if ( rId.getLength() == 16 &&
//           0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
//                                  rId.getConstArray(), 16 ) )
//  {
//      return (sal_Int64)this;
//  }
//  return 0;
// }

// // static
// const uno::Sequence<sal_Int8>& ScChart2DataSequence::getUnoTunnelId()
// {
//  static uno::Sequence<sal_Int8> * pSeq = 0;
//  if( !pSeq )
//  {
//      osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
//      if( !pSeq )
//      {
//          static uno::Sequence< sal_Int8 > aSeq( 16 );
//          rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
//          pSeq = &aSeq;
//      }
//  }
//  return *pSeq;
// }

// // static
// ScChart2DataSequence* ScChart2DataSequence::getImplementation( const uno::Reference<uno::XInterface> xObj )
// {
//  ScChart2DataSequence* pRet = NULL;
//  uno::Reference<lang::XUnoTunnel> xUT( xObj, uno::UNO_QUERY );
//  if (xUT.is())
//      pRet = (ScChart2DataSequence*) xUT->getSomething( getUnoTunnelId() );
//  return pRet;
// }

#if USE_CHART2_EMPTYDATASEQUENCE
// DataSequence ==============================================================

ScChart2EmptyDataSequence::ScChart2EmptyDataSequence( ScDocument* pDoc,
        const uno::Reference < chart2::data::XDataProvider >& xDP,
        const ScRangeListRef& rRangeList,
        sal_Bool bColumn)
    : m_bIncludeHiddenCells( sal_True)
    , m_xRanges( rRangeList)
    , m_pDocument( pDoc)
    , m_xDataProvider( xDP)
    , m_aPropSet(lcl_GetDataSequencePropertyMap())
    , m_bColumn(bColumn)
{
    if ( m_pDocument )
        m_pDocument->AddUnoObject( *this);
    // FIXME: real implementation of identifier and it's mapping to ranges.
    // Reuse ScChartListener?

    // BM: don't use names of named ranges but the UI range strings
//  String  aStr;
//  rRangeList->Format( aStr, SCR_ABS_3D, m_pDocument );
//    m_aIdentifier = ::rtl::OUString( aStr );

//      m_aIdentifier = ::rtl::OUString::createFromAscii( "ID_");
//      static sal_Int32 nID = 0;
//      m_aIdentifier += ::rtl::OUString::valueOf( ++nID);
}


ScChart2EmptyDataSequence::~ScChart2EmptyDataSequence()
{
    if ( m_pDocument )
        m_pDocument->RemoveUnoObject( *this);
}


void ScChart2EmptyDataSequence::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint)
{
    if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        m_pDocument = NULL;
    }
}


uno::Sequence< uno::Any> SAL_CALL ScChart2EmptyDataSequence::getData()
            throw ( uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if ( !m_pDocument)
        throw uno::RuntimeException();
    return uno::Sequence< uno::Any>();
}

// XTextualDataSequence --------------------------------------------------

uno::Sequence< rtl::OUString > SAL_CALL ScChart2EmptyDataSequence::getTextualData(  ) throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if ( !m_pDocument)
        throw uno::RuntimeException();

    sal_Int32 nCount = 0;
    ScRangePtr p;

    DBG_ASSERT(m_xRanges->Count() == 1, "not handled count of ranges");

    for ( p = m_xRanges->First(); p; p = m_xRanges->Next())
    {
        p->Justify();
        // TODO: handle overlaping ranges?
        nCount += m_bColumn ? p->aEnd.Col() - p->aStart.Col() + 1 : p->aEnd.Row() - p->aStart.Row() + 1;
    }
    uno::Sequence< rtl::OUString > aSeq( nCount);
    rtl::OUString* pArr = aSeq.getArray();
    nCount = 0;
    for ( p = m_xRanges->First(); p; p = m_xRanges->Next())
    {
        if (m_bColumn)
        {
            for (SCCOL nCol = p->aStart.Col(); nCol <= p->aEnd.Col(); ++nCol)
            {
                String aString = ScGlobal::GetRscString(STR_COLUMN);
                aString += ' ';
                ScAddress aPos( nCol, 0, 0 );
                String aColStr;
                aPos.Format( aColStr, SCA_VALID_COL, NULL );
                aString += aColStr;
                pArr[nCount] = aString;
                ++nCount;
            }
        }
        else
        {
            for (sal_Int32 nRow = p->aStart.Row(); nRow <= p->aEnd.Row(); ++nRow)
            {
                String aString = ScGlobal::GetRscString(STR_ROW);
                aString += ' ';
                aString += String::CreateFromInt32( nRow+1 );
                pArr[nCount] = aString;
                ++nCount;
            }
        }
    }
    return aSeq;
}

::rtl::OUString SAL_CALL ScChart2EmptyDataSequence::getSourceRangeRepresentation()
            throw ( uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String  aStr;
    DBG_ASSERT( m_pDocument, "No Document -> no SourceRangeRepresentation" );
    if( m_pDocument )
        m_xRanges->Format( aStr, SCR_ABS_3D, m_pDocument, m_pDocument->GetAddressConvention() );
    return aStr;
}

uno::Sequence< ::rtl::OUString > SAL_CALL ScChart2EmptyDataSequence::generateLabel(chart2::data::LabelOrigin /*nOrigin*/)
        throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Sequence< ::rtl::OUString > aRet;
    return aRet;
}

::sal_Int32 SAL_CALL ScChart2EmptyDataSequence::getNumberFormatKeyByIndex( ::sal_Int32 /*nIndex*/ )
    throw (lang::IndexOutOfBoundsException,
           uno::RuntimeException)
{
    sal_Int32 nResult = 0;

    ScUnoGuard aGuard;
    if ( !m_pDocument)
        return nResult;

    return nResult;
}

// XCloneable ================================================================

uno::Reference< util::XCloneable > SAL_CALL ScChart2EmptyDataSequence::createClone()
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (m_xDataProvider.is())
    {
        // copy properties
        uno::Reference < util::XCloneable > xClone(new ScChart2EmptyDataSequence(m_pDocument, m_xDataProvider, new ScRangeList(*m_xRanges), m_bColumn));
        uno::Reference< beans::XPropertySet > xProp( xClone, uno::UNO_QUERY );
        if( xProp.is())
        {
            xProp->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_ROLE )),
                                     uno::makeAny( m_aRole ));
            xProp->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SC_UNONAME_INCLUDEHIDDENCELLS )),
                                     uno::makeAny( m_bIncludeHiddenCells ));
        }
        return xClone;
    }
    return uno::Reference< util::XCloneable >();
}

// XModifyBroadcaster ========================================================

void SAL_CALL ScChart2EmptyDataSequence::addModifyListener( const uno::Reference< util::XModifyListener >& /*aListener*/ )
    throw (uno::RuntimeException)
{
    // TODO: Implement
}

void SAL_CALL ScChart2EmptyDataSequence::removeModifyListener( const uno::Reference< util::XModifyListener >& /*aListener*/ )
    throw (uno::RuntimeException)
{
    // TODO: Implement
}

// DataSequence XPropertySet -------------------------------------------------

uno::Reference< beans::XPropertySetInfo> SAL_CALL
ScChart2EmptyDataSequence::getPropertySetInfo() throw( uno::RuntimeException)
{
    ScUnoGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( m_aPropSet.getPropertyMap() );
    return aRef;
}


void SAL_CALL ScChart2EmptyDataSequence::setPropertyValue(
        const ::rtl::OUString& rPropertyName, const uno::Any& rValue)
            throw( beans::UnknownPropertyException,
                    beans::PropertyVetoException,
                    lang::IllegalArgumentException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    if ( rPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( SC_UNONAME_ROLE)))
    {
        if ( !(rValue >>= m_aRole))
            throw lang::IllegalArgumentException();
    }
    else if ( rPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( SC_UNONAME_INCLUDEHIDDENCELLS)))
    {
        if ( !(rValue >>= m_bIncludeHiddenCells))
            throw lang::IllegalArgumentException();
    }
    else
        throw beans::UnknownPropertyException();
    // TODO: support optional properties
}


uno::Any SAL_CALL ScChart2EmptyDataSequence::getPropertyValue(
        const ::rtl::OUString& rPropertyName)
            throw( beans::UnknownPropertyException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    uno::Any aRet;
    if ( rPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( SC_UNONAME_ROLE)))
        aRet <<= m_aRole;
    else if ( rPropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( SC_UNONAME_INCLUDEHIDDENCELLS)))
        aRet <<= m_bIncludeHiddenCells;
    else
        throw beans::UnknownPropertyException();
    // TODO: support optional properties
    return aRet;
}


void SAL_CALL ScChart2EmptyDataSequence::addPropertyChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener>& /*xListener*/)
            throw( beans::UnknownPropertyException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    // FIXME: real implementation
//     throw uno::RuntimeException();
    OSL_ENSURE( false, "Not yet implemented" );
}


void SAL_CALL ScChart2EmptyDataSequence::removePropertyChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener>& /*rListener*/)
            throw( beans::UnknownPropertyException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    // FIXME: real implementation
//     throw uno::RuntimeException();
    OSL_ENSURE( false, "Not yet implemented" );
}


void SAL_CALL ScChart2EmptyDataSequence::addVetoableChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener>& /*rListener*/)
            throw( beans::UnknownPropertyException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    // FIXME: real implementation
//     throw uno::RuntimeException();
    OSL_ENSURE( false, "Not yet implemented" );
}


void SAL_CALL ScChart2EmptyDataSequence::removeVetoableChangeListener(
        const ::rtl::OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener>& /*rListener*/ )
            throw( beans::UnknownPropertyException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    // FIXME: real implementation
//     throw uno::RuntimeException();
    OSL_ENSURE( false, "Not yet implemented" );
}

// XUnoTunnel

// sal_Int64 SAL_CALL ScChart2EmptyDataSequence::getSomething(
//              const uno::Sequence<sal_Int8 >& rId ) throw(uno::RuntimeException)
// {
//  if ( rId.getLength() == 16 &&
//           0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
//                                  rId.getConstArray(), 16 ) )
//  {
//      return (sal_Int64)this;
//  }
//  return 0;
// }

// // static
// const uno::Sequence<sal_Int8>& ScChart2EmptyDataSequence::getUnoTunnelId()
// {
//  static uno::Sequence<sal_Int8> * pSeq = 0;
//  if( !pSeq )
//  {
//      osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
//      if( !pSeq )
//      {
//          static uno::Sequence< sal_Int8 > aSeq( 16 );
//          rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
//          pSeq = &aSeq;
//      }
//  }
//  return *pSeq;
// }

// // static
// ScChart2DataSequence* ScChart2EmptyDataSequence::getImplementation( const uno::Reference<uno::XInterface> xObj )
// {
//  ScChart2DataSequence* pRet = NULL;
//  uno::Reference<lang::XUnoTunnel> xUT( xObj, uno::UNO_QUERY );
//  if (xUT.is())
//      pRet = (ScChart2EmptyDataSequence*) xUT->getSomething( getUnoTunnelId() );
//  return pRet;
// }
#endif
