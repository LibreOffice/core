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
#include "SchXMLImport.hxx"
#include "SchXMLTools.hxx"
#include "transporttypes.hxx"
#include "XMLStringBufferImportContext.hxx"
#include <rtl/math.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/nmspmap.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/chart2/XAnyDescriptionAccess.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XInternalDataProvider.hpp>
#include <com/sun/star/chart/ChartSeriesAddress.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/chart2/XAxis.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/AxisType.hpp>

#include <vector>
#include <algorithm>

using namespace com::sun::star;
using namespace ::xmloff::token;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;

namespace
{

const char aLabelPrefix[] = "label ";
const char aCategoriesRange[] = "categories";

typedef ::std::multimap< OUString, OUString >
    lcl_tOriginalRangeToInternalRangeMap;

struct lcl_ApplyCellToData : public ::std::unary_function< SchXMLCell, void >
{
    lcl_ApplyCellToData( Sequence< double > & rOutData ) :
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
    const OUString lcl_aLabelPrefix(aLabelPrefix);

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
                        rOutRangeMap.insert( lcl_tOriginalRangeToInternalRangeMap::value_type(
                                                 aRangeId, lcl_aCategoriesRange ));
                    }
                    else
                    {
                        OUString aColNumStr = OUString::number( nCol - nColOffset);
                        if( nRow == 0 && rTable.bHasHeaderRow )
                            rOutRangeMap.insert( lcl_tOriginalRangeToInternalRangeMap::value_type(
                                                     aRangeId, lcl_aLabelPrefix + aColNumStr ));
                        else
                            rOutRangeMap.insert( lcl_tOriginalRangeToInternalRangeMap::value_type(
                                                     aRangeId, aColNumStr ));
                    }
                }
                else // eDataRowSource == chart::ChartDataRowSource_ROWS
                {
                    if( nRow == 0 && rTable.bHasHeaderRow )
                    {
                        SAL_WARN_IF( static_cast< sal_Int32 >( nCol ) != nColOffset, "xmloff.chart", "nCol != nColOffset" );
                        rOutRangeMap.insert( lcl_tOriginalRangeToInternalRangeMap::value_type(
                                                 aRangeId, lcl_aCategoriesRange ));
                    }
                    else
                    {
                        OUString aRowNumStr = OUString::number( nRow - nRowOffset);
                        if( nCol == 0 && rTable.bHasHeaderColumn )
                            rOutRangeMap.insert( lcl_tOriginalRangeToInternalRangeMap::value_type(
                                                     aRangeId, lcl_aLabelPrefix + aRowNumStr ));
                        else
                            rOutRangeMap.insert( lcl_tOriginalRangeToInternalRangeMap::value_type(
                                                     aRangeId, aRowNumStr ));
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
    const OUString & rTableName )
{
    // both strings are non-empty and the table name is part of the range
    return ( !rRange.isEmpty() &&
             !rTableName.isEmpty() &&
             (rRange.indexOf( rTableName ) != -1 ));
}

template< typename T >
::std::vector< T > lcl_SequenceToVector( const uno::Sequence< T > & rSequence )
{
    ::std::vector< T > aResult( rSequence.getLength());
    ::std::copy( rSequence.getConstArray(), rSequence.getConstArray() + rSequence.getLength(),
                 aResult.begin());
    return aResult;
}

} // anonymous namespace

// class SchXMLTableContext
SchXMLTableContext::SchXMLTableContext( SchXMLImportHelper& rImpHelper,
                                        SvXMLImport& rImport,
                                        const OUString& rLName,
                                        SchXMLTable& aTable ) :
        SvXMLImportContext( rImport, XML_NAMESPACE_TABLE, rLName ),
        mrImportHelper( rImpHelper ),
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

SvXMLImportContext *SchXMLTableContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& )
{
    SvXMLImportContext* pContext = 0;
    const SvXMLTokenMap& rTokenMap = mrImportHelper.GetTableElemTokenMap();

    switch( rTokenMap.Get( nPrefix, rLocalName ))
    {
        case XML_TOK_TABLE_HEADER_COLS:
            mrTable.bHasHeaderColumn = true;
            // fall through intended
        case XML_TOK_TABLE_COLUMNS:
            pContext = new SchXMLTableColumnsContext( GetImport(), rLocalName, mrTable );
            break;

        case XML_TOK_TABLE_COLUMN:
            pContext = new SchXMLTableColumnContext( GetImport(), rLocalName, mrTable );
            break;

        case XML_TOK_TABLE_HEADER_ROWS:
            mrTable.bHasHeaderRow = true;
            // fall through intended
        case XML_TOK_TABLE_ROWS:
            pContext = new SchXMLTableRowsContext( mrImportHelper, GetImport(), rLocalName, mrTable );
            break;

        case XML_TOK_TABLE_ROW:
            pContext = new SchXMLTableRowContext( mrImportHelper, GetImport(), rLocalName, mrTable );
            break;

        default:
            pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    }

    return pContext;
}

void SchXMLTableContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    // get table-name
    sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;

    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
        if ( nPrefix == XML_NAMESPACE_TABLE )
        {
            if ( IsXMLToken( aLocalName, XML_NAME ) )
            {
                mrTable.aTableNameOfFile = xAttrList->getValueByIndex( i );
            }
            else if ( IsXMLToken( aLocalName, XML_PROTECTED ) )
            {
                if ( IsXMLToken( xAttrList->getValueByIndex( i ), XML_TRUE ) )
                {
                    mrTable.bProtected = true;
                }
            }
        }
    }
}

void SchXMLTableContext::EndElement()
{
    if( mbHasColumnPermutation )
    {
        SAL_WARN_IF( mbHasRowPermutation, "xmloff.chart", "mbHasColumnPermutation is true" );
        ::std::vector< sal_Int32 > aPermutation( lcl_SequenceToVector( maColumnPermutation ));
        SAL_WARN_IF( aPermutation.empty(), "xmloff.chart", "aPermutation is NULL");
        if( aPermutation.empty())
            return;

        // permute the values of all rows according to aPermutation
        for( ::std::vector< ::std::vector< SchXMLCell > >::iterator aRowIt( mrTable.aData.begin());
             aRowIt != mrTable.aData.end(); ++aRowIt )
        {
            bool bModified = false;
            ::std::vector< SchXMLCell > aModifiedRow;
            const size_t nPermSize = aPermutation.size();
            SAL_WARN_IF( static_cast< sal_Int32 >( nPermSize ) - 1 != *(::std::max_element( aPermutation.begin(), aPermutation.end())), "xmloff.chart", "nPermSize - 1 != *(::std::max_element( aPermutation.begin(), aPermutation.end())");
            const size_t nRowSize = aRowIt->size();
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
                        aModifiedRow.reserve( aRowIt->size());
                        ::std::copy( aRowIt->begin(), aRowIt->end(), ::std::back_inserter( aModifiedRow ));
                        SAL_WARN_IF( aModifiedRow.empty(), "xmloff.chart", "aModifiedRow is NULL");
                    }
                    SAL_WARN_IF( nDestinationIndex >= aModifiedRow.size(), "xmloff.chart", "nDestinationIndex >= aModifiedRow.size()");
                    aModifiedRow[ nDestinationIndex ] = (*aRowIt)[ nSourceIndex ];
                    bModified = true;
                }
            }
            // copy back
            if( bModified )
                ::std::copy( aModifiedRow.begin(), aModifiedRow.end(), aRowIt->begin());
        }
    }
    else if( mbHasRowPermutation )
    {
        ::std::vector< sal_Int32 > aPermutation( lcl_SequenceToVector( maRowPermutation ));
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
                    aDestination.reserve( mrTable.aData.size());
                    ::std::copy( mrTable.aData.begin(), mrTable.aData.end(), ::std::back_inserter( aDestination ));
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
    mbHasRowPermutation = ( rPermutation.getLength() > 0 );

    if( mbHasRowPermutation && mbHasColumnPermutation )
    {
        mbHasColumnPermutation = false;
        maColumnPermutation.realloc( 0 );
    }
}

void SchXMLTableContext::setColumnPermutation( const uno::Sequence< sal_Int32 > & rPermutation )
{
    maColumnPermutation = rPermutation;
    mbHasColumnPermutation = ( rPermutation.getLength() > 0 );

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
    const OUString& rLocalName,
    SchXMLTable& aTable ) :
        SvXMLImportContext( rImport, XML_NAMESPACE_TABLE, rLocalName ),
        mrTable( aTable )
{
}

SchXMLTableColumnsContext::~SchXMLTableColumnsContext()
{
}

SvXMLImportContext* SchXMLTableColumnsContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& )
{
    SvXMLImportContext* pContext = 0;

    if( nPrefix == XML_NAMESPACE_TABLE &&
        IsXMLToken( rLocalName, XML_TABLE_COLUMN ) )
    {
        pContext = new SchXMLTableColumnContext( GetImport(), rLocalName, mrTable );
    }
    else
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

// class SchXMLTableColumnContext
SchXMLTableColumnContext::SchXMLTableColumnContext(
    SvXMLImport& rImport,
    const OUString& rLocalName,
    SchXMLTable& aTable ) :
        SvXMLImportContext( rImport, XML_NAMESPACE_TABLE, rLocalName ),
        mrTable( aTable )
{
}

void SchXMLTableColumnContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    // get number-columns-repeated attribute
    sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;
    sal_Int32 nRepeated = 1;
    bool bHidden = false;

    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        if( nPrefix == XML_NAMESPACE_TABLE &&
            IsXMLToken( aLocalName, XML_NUMBER_COLUMNS_REPEATED ) )
        {
            OUString aValue = xAttrList->getValueByIndex( i );
            if( !aValue.isEmpty())
                nRepeated = aValue.toInt32();
        }
        else if( nPrefix == XML_NAMESPACE_TABLE &&
            IsXMLToken( aLocalName, XML_VISIBILITY ) )
        {
            OUString aVisibility = xAttrList->getValueByIndex( i );
            bHidden = aVisibility.equals( GetXMLToken( XML_COLLAPSE ) );
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
    SchXMLImportHelper& rImpHelper,
    SvXMLImport& rImport,
    const OUString& rLocalName,
    SchXMLTable& aTable ) :
        SvXMLImportContext( rImport, XML_NAMESPACE_TABLE, rLocalName ),
        mrImportHelper( rImpHelper ),
        mrTable( aTable )
{
}

SchXMLTableRowsContext::~SchXMLTableRowsContext()
{
}

SvXMLImportContext* SchXMLTableRowsContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& )
{
    SvXMLImportContext* pContext = 0;

    if( nPrefix == XML_NAMESPACE_TABLE &&
        IsXMLToken( rLocalName, XML_TABLE_ROW ) )
    {
        pContext = new SchXMLTableRowContext( mrImportHelper, GetImport(), rLocalName, mrTable );
    }
    else
    {
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    }

    return pContext;
}

// class SchXMLTableRowContext
SchXMLTableRowContext::SchXMLTableRowContext(
    SchXMLImportHelper& rImpHelper,
    SvXMLImport& rImport,
    const OUString& rLocalName,
    SchXMLTable& aTable ) :
        SvXMLImportContext( rImport, XML_NAMESPACE_TABLE, rLocalName ),
        mrImportHelper( rImpHelper ),
        mrTable( aTable )
{
    mrTable.nColumnIndex = -1;
    mrTable.nRowIndex++;

    std::vector< SchXMLCell > aNewRow;
    aNewRow.reserve( mrTable.nNumberOfColsEstimate );
    while( mrTable.aData.size() <= (unsigned long)mrTable.nRowIndex )
        mrTable.aData.push_back( aNewRow );
}

SchXMLTableRowContext::~SchXMLTableRowContext()
{
}

SvXMLImportContext* SchXMLTableRowContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& )
{
    SvXMLImportContext* pContext = 0;

    // <table:table-cell> element
    if( nPrefix == XML_NAMESPACE_TABLE &&
        IsXMLToken(rLocalName, XML_TABLE_CELL ) )
    {
        pContext = new SchXMLTableCellContext( mrImportHelper, GetImport(), rLocalName, mrTable );
    }
    else
    {
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    }

    return pContext;
}

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
                            sal_uInt16 nPrefix,
                            const OUString& rLocalName,
                            OUString& rRangeString );
    virtual ~SchXMLRangeSomewhereContext();

    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();
};

// classes for cells and their content
// class SchXMLTableCellContext
SchXMLTableCellContext::SchXMLTableCellContext(
    SchXMLImportHelper& rImpHelper,
    SvXMLImport& rImport,
    const OUString& rLocalName,
    SchXMLTable& aTable ) :
        SvXMLImportContext( rImport, XML_NAMESPACE_TABLE, rLocalName ),
        mrImportHelper( rImpHelper ),
        mrTable( aTable )
{
}

SchXMLTableCellContext::~SchXMLTableCellContext()
{
}

void SchXMLTableCellContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;
    OUString aValue;
    OUString aLocalName;
    OUString aCellContent;
    SchXMLCellType eValueType  = SCH_CELL_TYPE_UNKNOWN;
    const SvXMLTokenMap& rAttrTokenMap = mrImportHelper.GetCellAttrTokenMap();

    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ))
        {
            case XML_TOK_CELL_VAL_TYPE:
                aValue = xAttrList->getValueByIndex( i );
                if( IsXMLToken( aValue, XML_FLOAT ) )
                    eValueType = SCH_CELL_TYPE_FLOAT;
                else if( IsXMLToken( aValue, XML_STRING ) )
                    eValueType = SCH_CELL_TYPE_STRING;
                break;

            case XML_TOK_CELL_VALUE:
                aCellContent = xAttrList->getValueByIndex( i );
                break;
        }
    }

    mbReadText = sal_True;
    SchXMLCell aCell;
    aCell.eType = eValueType;

    if( eValueType == SCH_CELL_TYPE_FLOAT )
    {
        double fData;
        // the result may be false if a NaN is read, but that's ok
        ::sax::Converter::convertDouble( fData, aCellContent );

        aCell.fValue = fData;
        // dont read text from following <text:p> or <text:list> element
        mbReadText = sal_False;
    }

    mrTable.aData[ mrTable.nRowIndex ].push_back( aCell );
    mrTable.nColumnIndex++;
    if( mrTable.nMaxColumnIndex < mrTable.nColumnIndex )
        mrTable.nMaxColumnIndex = mrTable.nColumnIndex;
}

SvXMLImportContext* SchXMLTableCellContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& )
{
    SvXMLImportContext* pContext = 0;

    // <text:list> element
    if( nPrefix == XML_NAMESPACE_TEXT && IsXMLToken( rLocalName, XML_LIST ) && mbReadText )
    {
        SchXMLCell& rCell = mrTable.aData[ mrTable.nRowIndex ][ mrTable.nColumnIndex ];
        rCell.aComplexString = Sequence< OUString >();
        rCell.eType = SCH_CELL_TYPE_COMPLEX_STRING;
        pContext = new SchXMLTextListContext( GetImport(), rLocalName, rCell.aComplexString );
        mbReadText = sal_False;//don't apply text from <text:p>
    }
    // <text:p> element - read text (and range from text:id old version)
    else if( nPrefix == XML_NAMESPACE_TEXT && IsXMLToken( rLocalName, XML_P ) )
    {
        pContext = new SchXMLParagraphContext( GetImport(), rLocalName, maCellContent, &maRangeId );
    }
    // <draw:g> element - read range
    else if( nPrefix == XML_NAMESPACE_DRAW && IsXMLToken( rLocalName, XML_G ) )
    {
        //#i113950# previously the range was exported to attribute text:id, but that attribute does not allow arbitrary strings anymore
        //so we need to find an alternative to save that range info for copy/paste scenario ... -> use description at an empty group element for now
        pContext = new SchXMLRangeSomewhereContext( GetImport(), nPrefix, rLocalName, maRangeId );
    }
    else
    {
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    }

    return pContext;
}

void SchXMLTableCellContext::EndElement()
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
        rComplexLabel[0] = uno::makeAny( rCell.aString );
    }
    else if( rCell.aComplexString.getLength() && rCell.eType == SCH_CELL_TYPE_COMPLEX_STRING )
    {
        sal_Int32 nCount = rCell.aComplexString.getLength();
        rComplexLabel.realloc( nCount );
        for( sal_Int32 nN=0; nN<nCount; nN++)
            rComplexLabel[nN] = uno::makeAny((rCell.aComplexString)[nN]);
    }
    else if( rCell.eType == SCH_CELL_TYPE_FLOAT )
    {
        rComplexLabel.realloc(1);
        rComplexLabel[0] = uno::makeAny( rCell.fValue );
    }
}

namespace {

void transposeTable(SchXMLTable& rTable)
{
    std::vector<std::vector<SchXMLCell> > aNewData;
    sal_Int32 nRows = rTable.aData.size();
    aNewData.resize(rTable.nMaxColumnIndex+1);
    for(sal_Int32 i = 0; i < nRows; ++i)
    {
        sal_Int32 nCols = rTable.aData[i].size();
        for(sal_Int32 j = 0; j < nCols; ++j)
        {
            SchXMLCell& rCell = rTable.aData[i][j];
            aNewData[j].push_back(rCell);
        }
    }
    bool bHasRowHeader = rTable.bHasHeaderRow;
    bool bHasColHeader = rTable.bHasHeaderColumn;
    rTable.bHasHeaderColumn = bHasRowHeader;
    rTable.bHasHeaderRow = bHasColHeader;
    rTable.nMaxColumnIndex = nRows;
    rTable.aData = aNewData;
}

}

void SchXMLTableHelper::applyTableToInternalDataProvider(
    SchXMLTable rTable,
    uno::Reference< chart2::XChartDocument > xChartDoc, chart::ChartDataRowSource eDataRowSource )
{
    // apply all data read from the local table to the internal data provider
    if( !xChartDoc.is() || !xChartDoc->hasInternalDataProvider() )
        return;
    Reference< chart2::data::XDataProvider >  xDataProv( xChartDoc->getDataProvider() );
    if( !xDataProv.is() )
        return;

    if(eDataRowSource == chart::ChartDataRowSource_ROWS)
        transposeTable(rTable);

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

    if( rTable.aData.begin() != rTable.aData.end())
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
            xProps->setPropertyValue( "DisableDataTableDialog", uno::makeAny( sal_True ) );
            xProps->setPropertyValue( "DisableComplexChartTypes", uno::makeAny( sal_True ) );
        }
        catch ( uno::Exception& )
        {
        }
    }
}

void SchXMLTableHelper::switchRangesFromOuterToInternalIfNecessary(
    const SchXMLTable& rTable,
    const tSchXMLLSequencesPerIndex & rLSequencesPerIndex,
    uno::Reference< chart2::XChartDocument > xChartDoc,
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

    // translate ranges (using the map created before)
    for( tSchXMLLSequencesPerIndex::const_iterator aLSeqIt( rLSequencesPerIndex.begin());
         aLSeqIt != rLSequencesPerIndex.end(); ++aLSeqIt )
    {
        if( aLSeqIt->second.is())
        {
            // values/error bars/categories
            if( aLSeqIt->first.second == SCH_XML_PART_VALUES ||
                aLSeqIt->first.second == SCH_XML_PART_ERROR_BARS )
            {
                Reference< chart2::data::XDataSequence > xSeq( aLSeqIt->second->getValues());

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
                        aLSeqIt->second->setValues( xNewSeq );
                    }
                }
                else
                {
                    if( !lcl_tableOfRangeMatches( aRange, rTable.aTableNameOfFile ))
                    {
                        if( aLSeqIt->first.first == SCH_XML_CATEGORIES_INDEX )
                        {
                            Reference< beans::XPropertySet > xOldSequenceProp( aLSeqIt->second->getValues(), uno::UNO_QUERY );
                            Reference< chart2::data::XDataSequence > xNewSequence(
                                xDataProv->createDataSequenceByRangeRepresentation(
                                    OUString("categories")));
                            SchXMLTools::copyProperties(
                                xOldSequenceProp, Reference< beans::XPropertySet >( xNewSequence, uno::UNO_QUERY ));
                            aLSeqIt->second->setValues( xNewSequence );
                        }
                        else
                        {
                            Reference< beans::XPropertySet > xOldSequenceProp( aLSeqIt->second->getValues(), uno::UNO_QUERY );
                            OUString aRep( OUString::number( aLSeqIt->first.first ));
                            Reference< chart2::data::XDataSequence > xNewSequence(
                                xDataProv->createDataSequenceByRangeRepresentation( aRep ));
                            SchXMLTools::copyProperties(
                                xOldSequenceProp, Reference< beans::XPropertySet >( xNewSequence, uno::UNO_QUERY ));
                            aLSeqIt->second->setValues( xNewSequence );
                        }
                    }
                }
            }
            else // labels
            {
                SAL_WARN_IF( aLSeqIt->first.second != SCH_XML_PART_LABEL, "xmloff.chart", "aLSeqIt->first.second != SCH_XML_PART_LABEL" );
                // labels
                Reference< chart2::data::XDataSequence > xSeq( aLSeqIt->second->getLabel());
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
                        aLSeqIt->second->setLabel( xNewSeq );
                    }
                }
                else if( ! lcl_tableOfRangeMatches( aRange, rTable.aTableNameOfFile ))
                {
                    OUString aRep("label ");
                    aRep += OUString::number( aLSeqIt->first.first );

                    Reference< chart2::data::XDataSequence > xNewSeq(
                        xDataProv->createDataSequenceByRangeRepresentation( aRep ));
                    SchXMLTools::copyProperties( Reference< beans::XPropertySet >( xSeq, uno::UNO_QUERY ),
                                        Reference< beans::XPropertySet >( xNewSeq, uno::UNO_QUERY ));
                    aLSeqIt->second->setLabel( xNewSeq );
                }
            }
        }
    }

    //i91578 display of hidden values (copy paste scenario; use hidden flag during migration to locale table upon paste )
    //remove series that consist only of hidden columns
    Reference< chart2::XInternalDataProvider > xInternalDataProvider( xDataProv, uno::UNO_QUERY );
    if( xInternalDataProvider.is() && !rTable.aHiddenColumns.empty() )
    {
        try
        {
            Reference< chart2::XCoordinateSystemContainer > xCooSysCnt( xChartDoc->getFirstDiagram(), uno::UNO_QUERY_THROW );
            Sequence< Reference< chart2::XCoordinateSystem > > aCooSysSeq( xCooSysCnt->getCoordinateSystems() );
            for( sal_Int32 nC=0; nC<aCooSysSeq.getLength(); ++nC )
            {
                Reference< chart2::XChartTypeContainer > xCooSysContainer( aCooSysSeq[nC], uno::UNO_QUERY_THROW );
                Sequence< Reference< chart2::XChartType > > aChartTypeSeq( xCooSysContainer->getChartTypes());
                for( sal_Int32 nT=0; nT<aChartTypeSeq.getLength(); ++nT )
                {
                    Reference< chart2::XDataSeriesContainer > xSeriesContainer( aChartTypeSeq[nT], uno::UNO_QUERY );
                    if(!xSeriesContainer.is())
                        continue;
                    Sequence< Reference< chart2::XDataSeries > > aSeriesSeq( xSeriesContainer->getDataSeries() );
                    std::vector< Reference< chart2::XDataSeries > > aRemainingSeries;

                    for( sal_Int32 nS = 0; nS < aSeriesSeq.getLength(); nS++ )
                    {
                        Reference< chart2::data::XDataSource > xDataSource( aSeriesSeq[nS], uno::UNO_QUERY );
                        if( xDataSource.is() )
                        {
                            bool bHasUnhiddenColumns = false;
                            OUString aRange;
                            uno::Sequence< Reference< chart2::data::XLabeledDataSequence > > aSequences( xDataSource->getDataSequences() );
                            for( sal_Int32 nN=0; nN< aSequences.getLength(); ++nN )
                            {
                                Reference< chart2::data::XLabeledDataSequence > xLabeledSequence( aSequences[nN] );
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
                                        sal_Int32 nSearchIndex = 0;
                                        OUString aSecondToken = aRange.getToken( 1, ' ', nSearchIndex );
                                        if( ::std::find( rTable.aHiddenColumns.begin(), rTable.aHiddenColumns.end(), aSecondToken.toInt32() ) == rTable.aHiddenColumns.end() )
                                            bHasUnhiddenColumns = true;
                                    }
                                }
                            }
                            if( bHasUnhiddenColumns )
                                aRemainingSeries.push_back( aSeriesSeq[nS] );
                        }
                    }

                    if( static_cast<sal_Int32>(aRemainingSeries.size()) != aSeriesSeq.getLength() )
                    {
                        //remove the series that have only hidden data
                        Sequence< Reference< chart2::XDataSeries > > aRemainingSeriesSeq( aRemainingSeries.size());
                        ::std::copy( aRemainingSeries.begin(), aRemainingSeries.end(), aRemainingSeriesSeq.getArray());
                        xSeriesContainer->setDataSeries( aRemainingSeriesSeq );

                        //remove unused sequences
                        Reference< chart2::data::XDataSource > xDataSource( xChartDoc, uno::UNO_QUERY );
                        if( xDataSource.is() )
                        {
                            //first detect which collumns are really used
                            std::map< sal_Int32, bool > aUsageMap;
                            OUString aRange;
                            Sequence< Reference< chart2::data::XLabeledDataSequence > > aUsedSequences( xDataSource->getDataSequences() );
                            for( sal_Int32 nN=0; nN< aUsedSequences.getLength(); ++nN )
                            {
                                Reference< chart2::data::XLabeledDataSequence > xLabeledSequence( aUsedSequences[nN] );
                                if(!xLabeledSequence.is())
                                    continue;
                                Reference< chart2::data::XDataSequence > xValues( xLabeledSequence->getValues() );
                                if( xValues.is() )
                                {
                                    aRange = xValues->getSourceRangeRepresentation();
                                    sal_Int32 nIndex = aRange.toInt32();
                                    if( nIndex!=0 || !aRange.equals(lcl_aCategoriesRange) )
                                        aUsageMap[nIndex] = true;
                                }
                                Reference< chart2::data::XDataSequence > xLabel( xLabeledSequence->getLabel() );
                                if( xLabel.is() )
                                {
                                    aRange = xLabel->getSourceRangeRepresentation();
                                    sal_Int32 nSearchIndex = 0;
                                    OUString aSecondToken = aRange.getToken( 1, ' ', nSearchIndex );
                                    if( !aSecondToken.isEmpty() )
                                        aUsageMap[aSecondToken.toInt32()] = true;
                                }
                            }

                            ::std::vector< sal_Int32 > aSequenceIndexesToDelete;
                            for( ::std::vector< sal_Int32 >::const_iterator aIt(
                                     rTable.aHiddenColumns.begin()); aIt != rTable.aHiddenColumns.end(); ++aIt )
                            {
                                sal_Int32 nSequenceIndex = *aIt;
                                if( aUsageMap.find(nSequenceIndex) != aUsageMap.end() )
                                    continue;
                                aSequenceIndexesToDelete.push_back(nSequenceIndex);
                            }

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
}

SchXMLRangeSomewhereContext::SchXMLRangeSomewhereContext( SvXMLImport& rImport,
                                                sal_uInt16 nPrefix,
                                                const OUString& rLocalName,
                                                OUString& rRangeString ) :
        SvXMLImportContext( rImport, nPrefix, rLocalName ),
        mrRangeString( rRangeString )
{
}

SchXMLRangeSomewhereContext::~SchXMLRangeSomewhereContext()
{
}

SvXMLImportContext* SchXMLRangeSomewhereContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& )
{
    if( XML_NAMESPACE_SVG == nPrefix && IsXMLToken( rLocalName, XML_DESC ) )
    {
        return new XMLStringBufferImportContext(
            GetImport(), nPrefix, rLocalName, maRangeStringBuffer );
    }
    return new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
}

void SchXMLRangeSomewhereContext::EndElement()
{
    mrRangeString = maRangeStringBuffer.makeStringAndClear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
