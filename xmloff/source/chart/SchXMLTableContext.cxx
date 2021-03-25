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

#include <sax/tools/converter.hxx>

#include "SchXMLTableContext.hxx"
#include "SchXMLParagraphContext.hxx"
#include "SchXMLTextListContext.hxx"
#include <SchXMLImport.hxx>
#include "SchXMLTools.hxx"
#include "transporttypes.hxx"
#include <XMLStringBufferImportContext.hxx>
#include <o3tl/safeint.hxx>
#include <rtl/math.hxx>
#include <sal/log.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/namespacemap.hxx>
#include <comphelper/sequence.hxx>
#include <com/sun/star/chart2/XAnyDescriptionAccess.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XInternalDataProvider.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>

#include <vector>
#include <algorithm>
#include <iterator>
#include <string_view>

using namespace com::sun::star;
using namespace ::xmloff::token;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;

namespace
{

constexpr OUStringLiteral aCategoriesRange = u"categories";

typedef ::std::multimap< OUString, OUString >
    lcl_tOriginalRangeToInternalRangeMap;

struct lcl_ApplyCellToData
{
    explicit lcl_ApplyCellToData( Sequence< double > & rOutData ) :
            m_rData( rOutData ),
            m_nIndex( 0 ),
            m_nSize( rOutData.getLength()),
            m_fNaN( 0.0 )
    {
        ::rtl::math::setNan( &m_fNaN );
    }

    void operator() ( const SchXMLCell & rCell )
    {
        if( m_nIndex < m_nSize )
        {
            if( rCell.eType == SCH_CELL_TYPE_FLOAT )
                m_rData[m_nIndex] = rCell.fValue;
            else
                m_rData[m_nIndex] = m_fNaN;
        }
        ++m_nIndex;
    }

    sal_Int32 getCurrentIndex() const
    {
        return m_nIndex;
    }

private:
    Sequence< double > & m_rData;
    sal_Int32 m_nIndex;
    sal_Int32 m_nSize;
    double m_fNaN;
};

void lcl_fillRangeMapping(
    const SchXMLTable & rTable,
    lcl_tOriginalRangeToInternalRangeMap & rOutRangeMap,
    chart::ChartDataRowSource eDataRowSource )
{
    sal_Int32 nRowOffset = ( rTable.bHasHeaderRow ? 1 : 0 );
    sal_Int32 nColOffset = ( rTable.bHasHeaderColumn ? 1 : 0 );

    const OUString lcl_aCategoriesRange(aCategoriesRange);
    static const OUStringLiteral lcl_aLabelPrefix(u"label ");

    // Fill range mapping
    const size_t nTableRowCount( rTable.aData.size());
    for( size_t nRow = 0; nRow < nTableRowCount; ++nRow )
    {
        const ::std::vector< SchXMLCell > & rRow( rTable.aData[nRow] );
        const size_t nTableColCount( rRow.size());
        for( size_t nCol = 0; nCol < nTableColCount; ++nCol )
        {
            const OUString aRangeId( rRow[nCol].aRangeId );
            if( !aRangeId.isEmpty())
            {
                if( eDataRowSource == chart::ChartDataRowSource_COLUMNS )
                {
                    if( nCol == 0 && rTable.bHasHeaderColumn )
                    {
                        SAL_WARN_IF( static_cast< sal_Int32 >( nRow ) != nRowOffset, "xmloff.chart", "nRow != nRowOffset" );
                        rOutRangeMap.emplace(aRangeId, lcl_aCategoriesRange);
                    }
                    else
                    {
                        OUString aColNumStr = OUString::number( nCol - nColOffset);
                        if( nRow == 0 && rTable.bHasHeaderRow )
                            rOutRangeMap.emplace( aRangeId, lcl_aLabelPrefix + aColNumStr );
                        else
                            rOutRangeMap.emplace( aRangeId, aColNumStr );
                    }
                }
                else // eDataRowSource == chart::ChartDataRowSource_ROWS
                {
                    if( nRow == 0 && rTable.bHasHeaderRow )
                    {
                        SAL_WARN_IF( static_cast< sal_Int32 >( nCol ) != nColOffset, "xmloff.chart", "nCol != nColOffset" );
                        rOutRangeMap.emplace( aRangeId, lcl_aCategoriesRange );
                    }
                    else
                    {
                        OUString aRowNumStr = OUString::number( nRow - nRowOffset);
                        if( nCol == 0 && rTable.bHasHeaderColumn )
                            rOutRangeMap.emplace( aRangeId, lcl_aLabelPrefix + aRowNumStr );
                        else
                            rOutRangeMap.emplace( aRangeId, aRowNumStr );
                    }
                }
            }
        }
    }
}

Reference< chart2::data::XDataSequence >
    lcl_reassignDataSequence(
        const Reference< chart2::data::XDataSequence > & xSequence,
        const Reference< chart2::data::XDataProvider > & xDataProvider,
        lcl_tOriginalRangeToInternalRangeMap & rRangeMap,
        const OUString & rRange )
{
    Reference< chart2::data::XDataSequence > xResult( xSequence );
    lcl_tOriginalRangeToInternalRangeMap::iterator aIt( rRangeMap.find( rRange ));
    if( aIt != rRangeMap.end())
    {
        // set sequence with correct data
        xResult.set( xDataProvider->createDataSequenceByRangeRepresentation( aIt->second ));
        // remove translation, because it was used
        rRangeMap.erase( aIt );
    }

    return xResult;
}

bool lcl_mapContainsRange(
    lcl_tOriginalRangeToInternalRangeMap & rRangeMap,
    const OUString & rRange )
{
    lcl_tOriginalRangeToInternalRangeMap::iterator aIt( rRangeMap.find( rRange ));
    return ( aIt != rRangeMap.end());
}

bool lcl_tableOfRangeMatches(
    const OUString & rRange,
    std::u16string_view rTableName )
{
    // both strings are non-empty and the table name is part of the range
    return ( !rRange.isEmpty() &&
             !rTableName.empty() &&
             (rRange.indexOf( rTableName ) != -1 ));
}

} // anonymous namespace

// class SchXMLTableContext
SchXMLTableContext::SchXMLTableContext( SvXMLImport& rImport,
                                        SchXMLTable& aTable ) :
        SvXMLImportContext( rImport ),
        mrTable( aTable ),
        mbHasRowPermutation( false ),
        mbHasColumnPermutation( false )
{
    mrTable.nColumnIndex = -1;
    mrTable.nMaxColumnIndex = -1;
    mrTable.nRowIndex = -1;
    mrTable.aData.clear();
}

SchXMLTableContext::~SchXMLTableContext()
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SchXMLTableContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >&  )
{
    SvXMLImportContext* pContext = nullptr;

    switch(nElement)
    {
        case XML_ELEMENT(TABLE, XML_TABLE_HEADER_COLUMNS):
            mrTable.bHasHeaderColumn = true;
            [[fallthrough]];
        case XML_ELEMENT(TABLE, XML_TABLE_COLUMNS):
            pContext = new SchXMLTableColumnsContext( GetImport(), mrTable );
            break;

        case XML_ELEMENT(TABLE, XML_TABLE_COLUMN):
            pContext = new SchXMLTableColumnContext( GetImport(), mrTable );
            break;

        case XML_ELEMENT(TABLE, XML_TABLE_HEADER_ROWS):
            mrTable.bHasHeaderRow = true;
            [[fallthrough]];
        case XML_ELEMENT(TABLE, XML_TABLE_ROWS):
            pContext = new SchXMLTableRowsContext( GetImport(), mrTable );
            break;

        case XML_ELEMENT(TABLE, XML_TABLE_ROW):
            pContext = new SchXMLTableRowContext( GetImport(), mrTable );
            break;
        default:
            XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);
    }

    return pContext;
}

void SchXMLTableContext::startFastElement (sal_Int32 /*nElement*/,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList)
{
    // get table-name

    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        switch(aIter.getToken())
        {
            case XML_ELEMENT(TABLE, XML_NAME):
                mrTable.aTableNameOfFile = aIter.toString();
                break;
            case XML_ELEMENT(TABLE, XML_PROTECTED):
                if ( IsXMLToken( aIter, XML_TRUE ) )
                {
                    mrTable.bProtected = true;
                }
                break;
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }
}

void SchXMLTableContext::endFastElement(sal_Int32 )
{
    if( mbHasColumnPermutation )
    {
        SAL_WARN_IF( mbHasRowPermutation, "xmloff.chart", "mbHasColumnPermutation is true" );
        auto aPermutation( comphelper::sequenceToContainer<std::vector< sal_Int32 >>( maColumnPermutation ));
        SAL_WARN_IF( aPermutation.empty(), "xmloff.chart", "aPermutation is NULL");
        if( aPermutation.empty())
            return;

        // permute the values of all rows according to aPermutation
        for( auto& rRow : mrTable.aData )
        {
            bool bModified = false;
            ::std::vector< SchXMLCell > aModifiedRow;
            const size_t nPermSize = aPermutation.size();
            SAL_WARN_IF( static_cast< sal_Int32 >( nPermSize ) - 1 != *(::std::max_element( aPermutation.begin(), aPermutation.end())), "xmloff.chart", "nPermSize - 1 != *(::std::max_element( aPermutation.begin(), aPermutation.end())");
            const size_t nRowSize = rRow.size();
            const size_t nDestSize = ::std::min( nPermSize, nRowSize );
            for( size_t nDestinationIndex = 0; nDestinationIndex < nDestSize; ++nDestinationIndex )
            {
                const size_t nSourceIndex = static_cast< size_t >( aPermutation[ nDestinationIndex ] );
                if( nSourceIndex != nDestinationIndex &&
                    nSourceIndex < nRowSize )
                {
                    // copy original on first real permutation
                    if( !bModified )
                    {
                        SAL_WARN_IF( !aModifiedRow.empty(), "xmloff.chart", "aModifiedRow is NOT NULL");
                        aModifiedRow.insert( aModifiedRow.end(), rRow.begin(), rRow.end() );
                        SAL_WARN_IF( aModifiedRow.empty(), "xmloff.chart", "aModifiedRow is NULL");
                    }
                    SAL_WARN_IF( nDestinationIndex >= aModifiedRow.size(), "xmloff.chart", "nDestinationIndex >= aModifiedRow.size()");
                    aModifiedRow[ nDestinationIndex ] = rRow[ nSourceIndex ];
                    bModified = true;
                }
            }
            // copy back
            if( bModified )
                ::std::copy( aModifiedRow.begin(), aModifiedRow.end(), rRow.begin());
        }
    }
    else if( mbHasRowPermutation )
    {
        auto aPermutation( comphelper::sequenceToContainer<std::vector< sal_Int32 >>( maRowPermutation ));
        SAL_WARN_IF( aPermutation.empty(), "xmloff.chart", "aPermutation is NULL");
        if( aPermutation.empty())
            return;

        bool bModified = false;
        const size_t nPermSize = aPermutation.size();
        SAL_WARN_IF( static_cast< sal_Int32 >( nPermSize ) - 1 != *(::std::max_element( aPermutation.begin(), aPermutation.end())), "xmloff.chart", "nPermSize - 1 != *(::std::max_element( aPermutation.begin(), aPermutation.end())");
        const size_t nTableRowCount = mrTable.aData.size();
        const size_t nDestSize = ::std::min( nPermSize, nTableRowCount );
        ::std::vector< ::std::vector< SchXMLCell > > aDestination;
        for( size_t nDestinationIndex = 0; nDestinationIndex < nDestSize; ++nDestinationIndex )
        {
            const size_t nSourceIndex = static_cast< size_t >( aPermutation[ nDestinationIndex ] );
            if( nSourceIndex != nDestinationIndex &&
                nSourceIndex < nTableRowCount )
            {
                // copy original on first real permutation
                if( !bModified )
                {
                    SAL_WARN_IF( !aDestination.empty(), "xmloff.chart", "aDestination is NOT NULL");
                    aDestination.insert( aDestination.end(), mrTable.aData.begin(), mrTable.aData.end());
                    SAL_WARN_IF( aDestination.empty(), "xmloff.chart", "aDestination is NULL");
                }
                SAL_WARN_IF( nDestinationIndex >= aDestination.size(), "xmloff.chart", "nDestinationIndex >= aDestination.size()");
                aDestination[ nDestinationIndex ] = mrTable.aData[ nSourceIndex ];
                bModified = true;
            }
        }
        if( bModified )
        {
            // copy back
            ::std::copy( aDestination.begin(), aDestination.end(), mrTable.aData.begin());
        }
    }
}

void SchXMLTableContext::setRowPermutation( const uno::Sequence< sal_Int32 > & rPermutation )
{
    maRowPermutation = rPermutation;
    mbHasRowPermutation = rPermutation.hasElements();

    if( mbHasRowPermutation && mbHasColumnPermutation )
    {
        mbHasColumnPermutation = false;
        maColumnPermutation.realloc( 0 );
    }
}

void SchXMLTableContext::setColumnPermutation( const uno::Sequence< sal_Int32 > & rPermutation )
{
    maColumnPermutation = rPermutation;
    mbHasColumnPermutation = rPermutation.hasElements();

    if( mbHasColumnPermutation && mbHasRowPermutation )
    {
        mbHasRowPermutation = false;
        maRowPermutation.realloc( 0 );
    }
}

// classes for columns
// class SchXMLTableColumnsContext
SchXMLTableColumnsContext::SchXMLTableColumnsContext(
    SvXMLImport& rImport,
    SchXMLTable& aTable ) :
        SvXMLImportContext( rImport ),
        mrTable( aTable )
{
}

SchXMLTableColumnsContext::~SchXMLTableColumnsContext()
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SchXMLTableColumnsContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >&  )
{
    SvXMLImportContext* pContext = nullptr;

    if( nElement == XML_ELEMENT(TABLE, XML_TABLE_COLUMN) )
        pContext = new SchXMLTableColumnContext( GetImport(), mrTable );
    else
        XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);

    return pContext;
}

// class SchXMLTableColumnContext
SchXMLTableColumnContext::SchXMLTableColumnContext(
    SvXMLImport& rImport,
    SchXMLTable& aTable ) :
        SvXMLImportContext( rImport ),
        mrTable( aTable )
{
}

void SchXMLTableColumnContext::startFastElement (sal_Int32 /*nElement*/,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList)
{
    // get number-columns-repeated attribute
    sal_Int32 nRepeated = 1;
    bool bHidden = false;

    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        switch(aIter.getToken())
        {
            case XML_ELEMENT(TABLE, XML_NUMBER_COLUMNS_REPEATED):
            {
                if( !aIter.isEmpty())
                    nRepeated = aIter.toInt32();
                break;
            }
            case XML_ELEMENT(TABLE, XML_VISIBILITY):
            {
                OUString aVisibility = aIter.toString();
                bHidden = aVisibility == GetXMLToken( XML_COLLAPSE );
                break;
            }
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }

    sal_Int32 nOldCount = mrTable.nNumberOfColsEstimate;
    sal_Int32 nNewCount = nOldCount + nRepeated;
    mrTable.nNumberOfColsEstimate = nNewCount;

    if( bHidden )
    {
        //i91578 display of hidden values (copy paste scenario; use hidden flag during migration to locale table upon paste )
        sal_Int32 nColOffset = ( mrTable.bHasHeaderColumn ? 1 : 0 );
        for( sal_Int32 nN = nOldCount; nN<nNewCount; nN++ )
        {
            sal_Int32 nHiddenColumnIndex = nN-nColOffset;
            if( nHiddenColumnIndex>=0 )
                mrTable.aHiddenColumns.push_back(nHiddenColumnIndex);
        }
    }
}

SchXMLTableColumnContext::~SchXMLTableColumnContext()
{
}

// classes for rows
// class SchXMLTableRowsContext
SchXMLTableRowsContext::SchXMLTableRowsContext(
    SvXMLImport& rImport,
    SchXMLTable& aTable ) :
        SvXMLImportContext( rImport ),
        mrTable( aTable )
{
}

SchXMLTableRowsContext::~SchXMLTableRowsContext()
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SchXMLTableRowsContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >&  )
{
    SvXMLImportContext* pContext = nullptr;

    if( nElement == XML_ELEMENT(TABLE, XML_TABLE_ROW) )
        pContext = new SchXMLTableRowContext( GetImport(), mrTable );
    else
        XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);

    return pContext;
}

// class SchXMLTableRowContext
SchXMLTableRowContext::SchXMLTableRowContext(
    SvXMLImport& rImport,
    SchXMLTable& aTable ) :
        SvXMLImportContext( rImport ),
        mrTable( aTable )
{
    mrTable.nColumnIndex = -1;
    mrTable.nRowIndex++;

    std::vector< SchXMLCell > aNewRow;
    aNewRow.reserve( mrTable.nNumberOfColsEstimate );
    while( mrTable.aData.size() <= o3tl::make_unsigned(mrTable.nRowIndex) )
        mrTable.aData.push_back( aNewRow );
}

SchXMLTableRowContext::~SchXMLTableRowContext()
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SchXMLTableRowContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >&  )
{
    SvXMLImportContext* pContext = nullptr;

    // <table:table-cell> element
    if( nElement == XML_ELEMENT(TABLE, XML_TABLE_CELL) )
    {
        pContext = new SchXMLTableCellContext( GetImport(), mrTable );
    }
    else
    {
        XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);
        assert(false);
    }

    return pContext;
}

namespace {

class SchXMLRangeSomewhereContext : public SvXMLImportContext
{
//#i113950# previously the range was exported to attribute text:id,
//but that attribute does not allow arbitrary strings anymore
//so we need to find an alternative to save that range info for copy/paste scenario ...
//-> use description at an empty group element for now

private:
    OUString& mrRangeString;
    OUStringBuffer maRangeStringBuffer;

public:
    SchXMLRangeSomewhereContext( SvXMLImport& rImport,
                            OUString& rRangeString );

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;
    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;
};

}

// classes for cells and their content
// class SchXMLTableCellContext
SchXMLTableCellContext::SchXMLTableCellContext(
    SvXMLImport& rImport,
    SchXMLTable& aTable)
    : SvXMLImportContext(rImport)
    , mrTable(aTable)
    , mbReadText(false)
{
}

SchXMLTableCellContext::~SchXMLTableCellContext()
{
}

void SchXMLTableCellContext::startFastElement (sal_Int32 /*nElement*/,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList)
{
    OUString aCellContent;
    SchXMLCellType eValueType  = SCH_CELL_TYPE_UNKNOWN;

    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        switch(aIter.getToken())
        {
            case XML_ELEMENT(OFFICE, XML_VALUE_TYPE):
                if( IsXMLToken( aIter, XML_FLOAT ) )
                    eValueType = SCH_CELL_TYPE_FLOAT;
                else if( IsXMLToken( aIter, XML_STRING ) )
                    eValueType = SCH_CELL_TYPE_STRING;
                break;

            case XML_ELEMENT(OFFICE, XML_VALUE):
                aCellContent = aIter.toString();
                break;

            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }

    mbReadText = true;
    SchXMLCell aCell;
    aCell.eType = eValueType;

    if( eValueType == SCH_CELL_TYPE_FLOAT )
    {
        double fData;
        // the result may be false if a NaN is read, but that's ok
        ::sax::Converter::convertDouble( fData, aCellContent );

        aCell.fValue = fData;
        // don't read text from following <text:p> or <text:list> element
        mbReadText = false;
    }

    mrTable.aData[ mrTable.nRowIndex ].push_back( aCell );
    mrTable.nColumnIndex++;
    if( mrTable.nMaxColumnIndex < mrTable.nColumnIndex )
        mrTable.nMaxColumnIndex = mrTable.nColumnIndex;
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SchXMLTableCellContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >&  )
{
    SvXMLImportContext* pContext = nullptr;

    // <text:list> element
    if( nElement == XML_ELEMENT(TEXT, XML_LIST ) && mbReadText )
    {
        SchXMLCell& rCell = mrTable.aData[ mrTable.nRowIndex ][ mrTable.nColumnIndex ];
        rCell.aComplexString = Sequence< OUString >();
        rCell.eType = SCH_CELL_TYPE_COMPLEX_STRING;
        pContext = new SchXMLTextListContext( GetImport(), rCell.aComplexString );
        mbReadText = false;//don't apply text from <text:p>
    }
    // <text:p> element - read text (and range from text:id old version)
    else if( nElement == XML_ELEMENT(TEXT, XML_P) ||
            nElement == XML_ELEMENT(LO_EXT, XML_P) )
    {
        pContext = new SchXMLParagraphContext( GetImport(), maCellContent, &maRangeId );
    }
    // <draw:g> element - read range
    else if( nElement == XML_ELEMENT(DRAW, XML_G) )
    {
        //#i113950# previously the range was exported to attribute text:id, but that attribute does not allow arbitrary strings anymore
        //so we need to find an alternative to save that range info for copy/paste scenario ... -> use description at an empty group element for now
        pContext = new SchXMLRangeSomewhereContext( GetImport(), maRangeId );
    }
    else
        XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);

    return pContext;
}

void SchXMLTableCellContext::endFastElement(sal_Int32 )
{
    if( mbReadText && !maCellContent.isEmpty() ) //apply text from <text:p> element
        mrTable.aData[ mrTable.nRowIndex ][ mrTable.nColumnIndex ].aString = maCellContent;
    if( !maRangeId.isEmpty())
        mrTable.aData[ mrTable.nRowIndex ][ mrTable.nColumnIndex ].aRangeId = maRangeId;
}

static void lcl_ApplyCellToComplexLabel( const SchXMLCell& rCell, Sequence< uno::Any >& rComplexLabel )
{
    if( rCell.eType == SCH_CELL_TYPE_STRING )
    {
        rComplexLabel.realloc(1);
        rComplexLabel[0] <<= rCell.aString;
    }
    else if( rCell.aComplexString.hasElements() && rCell.eType == SCH_CELL_TYPE_COMPLEX_STRING )
    {
        sal_Int32 nCount = rCell.aComplexString.getLength();
        rComplexLabel.realloc( nCount );
        for( sal_Int32 nN=0; nN<nCount; nN++)
            rComplexLabel[nN] <<= (rCell.aComplexString)[nN];
    }
    else if( rCell.eType == SCH_CELL_TYPE_FLOAT )
    {
        rComplexLabel.realloc(1);
        rComplexLabel[0] <<= rCell.fValue;
    }
}

void SchXMLTableHelper::applyTableToInternalDataProvider(
    const SchXMLTable& rTable,
    const uno::Reference< chart2::XChartDocument >& xChartDoc )
{
    // apply all data read from the local table to the internal data provider
    if( !xChartDoc.is() || !xChartDoc->hasInternalDataProvider() )
        return;
    Reference< chart2::data::XDataProvider >  xDataProv( xChartDoc->getDataProvider() );
    if( !xDataProv.is() )
        return;

    //prepare the read local table data
    sal_Int32 nNumRows( static_cast< sal_Int32 >( rTable.aData.size()));
    sal_Int32 nRowOffset = 0;
    if( rTable.bHasHeaderRow )
    {
        --nNumRows;
        nRowOffset = 1;
    }
    sal_Int32 nNumColumns( rTable.nMaxColumnIndex + 1 );
    sal_Int32 nColOffset = 0;
    if( rTable.bHasHeaderColumn )
    {
        --nNumColumns;
        nColOffset = 1;
    }

    Sequence< Sequence< double > > aDataInRows( nNumRows );
    Sequence< Sequence< uno::Any > > aComplexRowDescriptions( nNumRows );
    Sequence< Sequence< uno::Any > > aComplexColumnDescriptions( nNumColumns );
    for( sal_Int32 i=0; i<nNumRows; ++i )
        aDataInRows[i].realloc( nNumColumns );

    if( !rTable.aData.empty() )
    {
        //apply column labels
        if( rTable.bHasHeaderRow )
        {
            const ::std::vector< SchXMLCell >& rFirstRow = rTable.aData.front();
            const sal_Int32 nColumnLabelsSize = aComplexColumnDescriptions.getLength();
            const sal_Int32 nMax = ::std::min< sal_Int32 >( nColumnLabelsSize, static_cast< sal_Int32 >( rFirstRow.size()) - nColOffset );
            SAL_WARN_IF( nMax != nColumnLabelsSize, "xmloff.chart", "nMax != nColumnLabelsSize");
            for( sal_Int32 i=0; i<nMax; ++i )
                lcl_ApplyCellToComplexLabel( rFirstRow[i+nColOffset], aComplexColumnDescriptions[i] );
        }

        std::vector< ::std::vector< SchXMLCell > >::const_iterator aRowIter( rTable.aData.begin() + nRowOffset );
        std::vector< ::std::vector< SchXMLCell > >::const_iterator aEnd( rTable.aData.end() );
        for( sal_Int32 nRow = 0; aRowIter != aEnd && nRow < nNumRows; ++aRowIter, ++nRow )
        {
            const ::std::vector< SchXMLCell >& rRow = *aRowIter;
            if( !rRow.empty() )
            {
                // row label
                if( rTable.bHasHeaderColumn )
                    lcl_ApplyCellToComplexLabel( rRow.front(), aComplexRowDescriptions[nRow] );

                // values
                Sequence< double >& rTargetRow = aDataInRows[nRow];
                lcl_ApplyCellToData aApplyCellToData = ::std::for_each( rRow.begin() + nColOffset, rRow.end(), lcl_ApplyCellToData( rTargetRow ) );
                double fNaN = 0.0;
                ::rtl::math::setNan( &fNaN );
                for( sal_Int32 nCurrentIndex = aApplyCellToData.getCurrentIndex(); nCurrentIndex<nNumColumns; nCurrentIndex++ )
                    rTargetRow[nCurrentIndex] = fNaN;//#i110615#
            }
        }
    }

    //apply the collected data to the chart
    Reference< chart2::XAnyDescriptionAccess > xDataAccess( xDataProv, uno::UNO_QUERY );
    if( !xDataAccess.is() )
        return;

    xDataAccess->setData( aDataInRows );
    if( rTable.bHasHeaderColumn )
        xDataAccess->setAnyRowDescriptions( aComplexRowDescriptions );
    if( rTable.bHasHeaderRow )
        xDataAccess->setAnyColumnDescriptions( aComplexColumnDescriptions );

    if ( rTable.bProtected )
    {
        try
        {
            Reference< beans::XPropertySet > xProps( xChartDoc, uno::UNO_QUERY_THROW );
            xProps->setPropertyValue( "DisableDataTableDialog", uno::makeAny( true ) );
            xProps->setPropertyValue( "DisableComplexChartTypes", uno::makeAny( true ) );
        }
        catch ( uno::Exception& )
        {
        }
    }
}

void SchXMLTableHelper::switchRangesFromOuterToInternalIfNecessary(
    const SchXMLTable& rTable,
    const tSchXMLLSequencesPerIndex & rLSequencesPerIndex,
    const uno::Reference< chart2::XChartDocument >& xChartDoc,
    chart::ChartDataRowSource eDataRowSource )
{
    if( ! (xChartDoc.is() && xChartDoc->hasInternalDataProvider()))
        return;

    // If the range-strings are valid (starting with "local-table") they should
    // be interpreted like given, otherwise (when the ranges refer to Calc- or
    // Writer-ranges, but the container is not available like when pasting a
    // chart from Calc to Impress) the range is ignored, and every object gets
    // one table column in the order of appearance, which is: 1. categories,
    // 2. data series: 2.a) domains, 2.b) values (main-role, usually y-values)

    Reference< chart2::data::XDataProvider >  xDataProv( xChartDoc->getDataProvider());

    // create a mapping from original ranges to new ranges
    lcl_tOriginalRangeToInternalRangeMap aRangeMap;

    lcl_fillRangeMapping( rTable, aRangeMap, eDataRowSource );

    const OUString lcl_aCategoriesRange(aCategoriesRange);

    bool bCategoriesApplied = false;
    // translate ranges (using the map created before)
    for( const auto& rLSeq : rLSequencesPerIndex )
    {
        if( rLSeq.second.is())
        {
            // values/error bars/categories
            if( rLSeq.first.second == SCH_XML_PART_VALUES ||
                rLSeq.first.second == SCH_XML_PART_ERROR_BARS )
            {
                Reference< chart2::data::XDataSequence > xSeq( rLSeq.second->getValues());

                OUString aRange;
                if( xSeq.is() &&
                    SchXMLTools::getXMLRangePropertyFromDataSequence( xSeq, aRange, /* bClearProp = */ true ) &&
                    lcl_mapContainsRange( aRangeMap, aRange ))
                {
                    Reference< chart2::data::XDataSequence > xNewSeq(
                        lcl_reassignDataSequence( xSeq, xDataProv, aRangeMap, aRange ));
                    if( xNewSeq != xSeq )
                    {
                        SchXMLTools::copyProperties( Reference< beans::XPropertySet >( xSeq, uno::UNO_QUERY ),
                                            Reference< beans::XPropertySet >( xNewSeq, uno::UNO_QUERY ));
                        rLSeq.second->setValues( xNewSeq );
                    }
                }
                else
                {
                    if( lcl_tableOfRangeMatches( aRange, rTable.aTableNameOfFile ))
                    {
                        if( rLSeq.first.first == SCH_XML_CATEGORIES_INDEX )
                            bCategoriesApplied = true;
                    }
                    else
                    {
                        if( rLSeq.first.first == SCH_XML_CATEGORIES_INDEX )
                        {
                            Reference< beans::XPropertySet > xOldSequenceProp( rLSeq.second->getValues(), uno::UNO_QUERY );
                            Reference< chart2::data::XDataSequence > xNewSequence(
                                xDataProv->createDataSequenceByRangeRepresentation("categories"));
                            SchXMLTools::copyProperties(
                                xOldSequenceProp, Reference< beans::XPropertySet >( xNewSequence, uno::UNO_QUERY ));
                            rLSeq.second->setValues( xNewSequence );
                            bCategoriesApplied = true;
                        }
                        else
                        {
                            Reference< beans::XPropertySet > xOldSequenceProp( rLSeq.second->getValues(), uno::UNO_QUERY );
                            OUString aRep( OUString::number( rLSeq.first.first ));
                            Reference< chart2::data::XDataSequence > xNewSequence(
                                xDataProv->createDataSequenceByRangeRepresentation( aRep ));
                            SchXMLTools::copyProperties(
                                xOldSequenceProp, Reference< beans::XPropertySet >( xNewSequence, uno::UNO_QUERY ));
                            rLSeq.second->setValues( xNewSequence );
                        }
                    }
                }
            }
            else // labels
            {
                SAL_WARN_IF( rLSeq.first.second != SCH_XML_PART_LABEL, "xmloff.chart", "rLSeq.first.second != SCH_XML_PART_LABEL" );
                // labels
                Reference< chart2::data::XDataSequence > xSeq( rLSeq.second->getLabel());
                OUString aRange;
                if( xSeq.is() &&
                    SchXMLTools::getXMLRangePropertyFromDataSequence( xSeq, aRange, /* bClearProp = */ true ) &&
                    lcl_mapContainsRange( aRangeMap, aRange ))
                {
                    Reference< chart2::data::XDataSequence > xNewSeq(
                        lcl_reassignDataSequence( xSeq, xDataProv, aRangeMap, aRange ));
                    if( xNewSeq != xSeq )
                    {
                        SchXMLTools::copyProperties( Reference< beans::XPropertySet >( xSeq, uno::UNO_QUERY ),
                                            Reference< beans::XPropertySet >( xNewSeq, uno::UNO_QUERY ));
                        rLSeq.second->setLabel( xNewSeq );
                    }
                }
                else if( ! lcl_tableOfRangeMatches( aRange, rTable.aTableNameOfFile ))
                {
                    OUString aRep = "label " + OUString::number( rLSeq.first.first );

                    Reference< chart2::data::XDataSequence > xNewSeq(
                        xDataProv->createDataSequenceByRangeRepresentation( aRep ));
                    SchXMLTools::copyProperties( Reference< beans::XPropertySet >( xSeq, uno::UNO_QUERY ),
                                        Reference< beans::XPropertySet >( xNewSeq, uno::UNO_QUERY ));
                    rLSeq.second->setLabel( xNewSeq );
                }
            }
        }
    }

    // there exist files with own data without a categories element but with row
    // descriptions.  The row descriptions were used as categories even without
    // the categories element
    if( ! bCategoriesApplied )
    {
        SchXMLTools::CreateCategories(
            xDataProv, xChartDoc, "categories",
            0 /* nCooSysIndex */, 0 /* nDimension */ );
    }

    //i91578 display of hidden values (copy paste scenario; use hidden flag during migration to locale table upon paste )
    //remove series that consist only of hidden columns
    Reference< chart2::XInternalDataProvider > xInternalDataProvider( xDataProv, uno::UNO_QUERY );
    if( !xInternalDataProvider.is() || rTable.aHiddenColumns.empty() )
        return;

    try
    {
        Reference< chart2::XCoordinateSystemContainer > xCooSysCnt( xChartDoc->getFirstDiagram(), uno::UNO_QUERY_THROW );
        const Sequence< Reference< chart2::XCoordinateSystem > > aCooSysSeq( xCooSysCnt->getCoordinateSystems() );
        for( const auto& rCooSys : aCooSysSeq )
        {
            Reference< chart2::XChartTypeContainer > xCooSysContainer( rCooSys, uno::UNO_QUERY_THROW );
            const Sequence< Reference< chart2::XChartType > > aChartTypeSeq( xCooSysContainer->getChartTypes());
            for( const auto& rChartType : aChartTypeSeq )
            {
                Reference< chart2::XDataSeriesContainer > xSeriesContainer( rChartType, uno::UNO_QUERY );
                if(!xSeriesContainer.is())
                    continue;
                const Sequence< Reference< chart2::XDataSeries > > aSeriesSeq( xSeriesContainer->getDataSeries() );
                std::vector< Reference< chart2::XDataSeries > > aRemainingSeries;

                for( const auto& rSeries : aSeriesSeq )
                {
                    Reference< chart2::data::XDataSource > xDataSource( rSeries, uno::UNO_QUERY );
                    if( xDataSource.is() )
                    {
                        bool bHasUnhiddenColumns = false;
                        OUString aRange;
                        const uno::Sequence< Reference< chart2::data::XLabeledDataSequence > > aSequences( xDataSource->getDataSequences() );
                        for( const auto& xLabeledSequence : aSequences )
                        {
                            if(!xLabeledSequence.is())
                                continue;
                            Reference< chart2::data::XDataSequence > xValues( xLabeledSequence->getValues() );
                            if( xValues.is() )
                            {
                                aRange = xValues->getSourceRangeRepresentation();
                                if( ::std::find( rTable.aHiddenColumns.begin(), rTable.aHiddenColumns.end(), aRange.toInt32() ) == rTable.aHiddenColumns.end() )
                                    bHasUnhiddenColumns = true;
                            }
                            if( !bHasUnhiddenColumns )
                            {
                                Reference< chart2::data::XDataSequence > xLabel( xLabeledSequence->getLabel() );
                                if( xLabel.is() )
                                {
                                    aRange = xLabel->getSourceRangeRepresentation();
                                    const sal_Int32 nId {aRange.getToken(1, ' ').toInt32()};
                                    if( ::std::find( rTable.aHiddenColumns.begin(), rTable.aHiddenColumns.end(), nId ) == rTable.aHiddenColumns.end() )
                                        bHasUnhiddenColumns = true;
                                }
                            }
                        }
                        if( bHasUnhiddenColumns )
                            aRemainingSeries.push_back( rSeries );
                    }
                }

                if( static_cast<sal_Int32>(aRemainingSeries.size()) != aSeriesSeq.getLength() )
                {
                    //remove the series that have only hidden data
                    xSeriesContainer->setDataSeries( comphelper::containerToSequence(aRemainingSeries) );

                    //remove unused sequences
                    Reference< chart2::data::XDataSource > xDataSource( xChartDoc, uno::UNO_QUERY );
                    if( xDataSource.is() )
                    {
                        //first detect which columns are really used
                        std::map< sal_Int32, bool > aUsageMap;
                        OUString aRange;
                        const Sequence< Reference< chart2::data::XLabeledDataSequence > > aUsedSequences( xDataSource->getDataSequences() );
                        for( const auto& xLabeledSequence : aUsedSequences )
                        {
                            if(!xLabeledSequence.is())
                                continue;
                            Reference< chart2::data::XDataSequence > xValues( xLabeledSequence->getValues() );
                            if( xValues.is() )
                            {
                                aRange = xValues->getSourceRangeRepresentation();
                                sal_Int32 nIndex = aRange.toInt32();
                                if( nIndex!=0 || aRange != lcl_aCategoriesRange )
                                    aUsageMap[nIndex] = true;
                            }
                            Reference< chart2::data::XDataSequence > xLabel( xLabeledSequence->getLabel() );
                            if( xLabel.is() )
                            {
                                aRange = xLabel->getSourceRangeRepresentation();
                                OUString aSecondToken = aRange.getToken(1, ' ');
                                if( !aSecondToken.isEmpty() )
                                    aUsageMap[aSecondToken.toInt32()] = true;
                            }
                        }

                        ::std::vector< sal_Int32 > aSequenceIndexesToDelete;
                        std::copy_if(rTable.aHiddenColumns.begin(), rTable.aHiddenColumns.end(),
                            std::back_inserter(aSequenceIndexesToDelete),
                            [&aUsageMap](sal_Int32 nSequenceIndex) { return aUsageMap.find(nSequenceIndex) == aUsageMap.end(); });

                        // delete unnecessary sequences of the internal data
                        // iterate using greatest index first, so that deletion does not
                        // shift other sequences that will be deleted later
                        ::std::sort( aSequenceIndexesToDelete.begin(), aSequenceIndexesToDelete.end());
                        for( ::std::vector< sal_Int32 >::reverse_iterator aIt(
                                 aSequenceIndexesToDelete.rbegin()); aIt != aSequenceIndexesToDelete.rend(); ++aIt )
                        {
                            if( *aIt != -1 )
                                xInternalDataProvider->deleteSequence( *aIt );
                        }
                    }
                }
            }
        }
    }
    catch( const uno::Exception & )
    {
    }
}

SchXMLRangeSomewhereContext::SchXMLRangeSomewhereContext( SvXMLImport& rImport,
                                                OUString& rRangeString ) :
        SvXMLImportContext( rImport ),
        mrRangeString( rRangeString )
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SchXMLRangeSomewhereContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >&  )
{
    if( nElement == XML_ELEMENT(SVG, XML_DESC) )
    {
        return new XMLStringBufferImportContext( GetImport(), maRangeStringBuffer );
    }
    else
        XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);
    return nullptr;
}

void SchXMLRangeSomewhereContext::endFastElement(sal_Int32 )
{
    mrRangeString = maRangeStringBuffer.makeStringAndClear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
