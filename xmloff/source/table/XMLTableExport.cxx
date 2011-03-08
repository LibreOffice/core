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
#include "precompiled_xmloff.hxx"
#include "xmloff/dllapi.h"

#include "sal/config.h"
#include <osl/diagnose.h>

#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>

#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <com/sun/star/table/CellContentType.hpp>
#include <com/sun/star/table/XMergeableCell.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>

#include "xmloff/table/XMLTableExport.hxx"
#include "xmlnmspe.hxx"
#include <xmloff/xmlprmap.hxx>
#include <xmloff/xmlexppr.hxx>
#include <xmloff/xmlexp.hxx>
#include "table.hxx"

using ::rtl::OUString;
using namespace ::xmloff::token;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::style;
using namespace ::xmloff::token;

// --------------------------------------------------------------------

#define _MAP(name,prefix,token,type,context)  { name, sizeof(name)-1, prefix, token, type, context, SvtSaveOptions::ODFVER_010 }
#define CMAP(name,prefix,token,type,context) _MAP(name,prefix,token,type|XML_TYPE_PROP_TABLE_COLUMN,context)
#define RMAP(name,prefix,token,type,context) _MAP(name,prefix,token,type|XML_TYPE_PROP_TABLE_ROW,context)
#define MAP_END { 0L, 0, 0, XML_EMPTY, 0, 0, SvtSaveOptions::ODFVER_010 }

// --------------------------------------------------------------------

const XMLPropertyMapEntry* getColumnPropertiesMap()
{
    static const XMLPropertyMapEntry aXMLColumnProperties[] =
    {
        CMAP( "Width",          XML_NAMESPACE_STYLE,    XML_COLUMN_WIDTH,               XML_TYPE_MEASURE,   0 ),
        CMAP( "OptimalWidth",   XML_NAMESPACE_STYLE,    XML_USE_OPTIMAL_COLUMN_WIDTH,   XML_TYPE_BOOL, 0 ),
        MAP_END
    };

    return &aXMLColumnProperties[0];
}

// --------------------------------------------------------------------

const XMLPropertyMapEntry* getRowPropertiesMap()
{
    static const XMLPropertyMapEntry aXMLRowProperties[] =
    {
        RMAP( "Height",         XML_NAMESPACE_STYLE, XML_ROW_HEIGHT,                    XML_TYPE_MEASURE,   0 ),
        RMAP( "OptimalHeight",  XML_NAMESPACE_STYLE, XML_MIN_ROW_HEIGHT,                XML_TYPE_MEASURE,   0 ),
        RMAP( "OptimalWidth",   XML_NAMESPACE_STYLE, XML_USE_OPTIMAL_ROW_HEIGHT,        XML_TYPE_BOOL, 0 ),
        MAP_END
    };

    return &aXMLRowProperties[0];
}

// --------------------------------------------------------------------

class StringStatisticHelper : public std::map< OUString, sal_Int32 >
{
public:
    void add( const OUString& rStyleName );
    void clear() { std::map< OUString, sal_Int32 >::clear(); }

    sal_Int32 getModeString( /* out */ OUString& rModeString );
};

// --------------------------------------------------------------------

void StringStatisticHelper::add( const OUString& rStyleName )
{
    std::map< OUString, sal_Int32 >::iterator iter( find( rStyleName ) );
    if( iter == end() )
    {
        (*this)[rStyleName] = 1;
    }
    else
    {
        (*iter).second += 1;
    }
}

// --------------------------------------------------------------------

sal_Int32 StringStatisticHelper::getModeString( OUString& rStyleName )
{
    sal_Int32 nMax = 0;
    for( std::map< OUString, sal_Int32 >::iterator iter( begin() ); iter != end(); iter++ )
    {
        if( (*iter).second > nMax )
        {
            rStyleName = (*iter).first;
            nMax = (*iter).second;
        }
    }

    return nMax;
}

// --------------------------------------------------------------------
// class XMLTableExport
// --------------------------------------------------------------------

XMLTableExport::XMLTableExport(SvXMLExport& rExp, const rtl::Reference< SvXMLExportPropertyMapper  >& xExportPropertyMapper, const rtl::Reference< XMLPropertyHandlerFactory >& xFactoryRef )
: mrExport( rExp )
, mbExportTables( false )
{
    Reference< XMultiServiceFactory > xFac( rExp.GetModel(), UNO_QUERY );
    if( xFac.is() ) try
    {
        Sequence< OUString > sSNS( xFac->getAvailableServiceNames() );
        sal_Int32 n = sSNS.getLength();
        const OUString* pSNS( sSNS.getConstArray() );
        while( --n > 0 )
        {
            if( (*pSNS++).equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.TableShape") ) )
            {
                mbExportTables = true;
                break;
            }
        }
    }
    catch( Exception& e )
    {
        (void)e;
    }

    mxCellExportPropertySetMapper = xExportPropertyMapper;
    mxCellExportPropertySetMapper->ChainExportMapper(XMLTextParagraphExport::CreateParaExtPropMapper(rExp));

    mxRowExportPropertySetMapper = new SvXMLExportPropertyMapper( new XMLPropertySetMapper( getRowPropertiesMap(), xFactoryRef.get() ) );
    mxColumnExportPropertySetMapper = new SvXMLExportPropertyMapper( new XMLPropertySetMapper( getColumnPropertiesMap(), xFactoryRef.get() ) );

    mrExport.GetAutoStylePool()->AddFamily(XML_STYLE_FAMILY_TABLE_COLUMN,
        OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_COLUMN_STYLES_NAME)),
        mxColumnExportPropertySetMapper.get(),
        OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_COLUMN_STYLES_PREFIX)));
    mrExport.GetAutoStylePool()->AddFamily(XML_STYLE_FAMILY_TABLE_ROW,
        OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_ROW_STYLES_NAME)),
        mxRowExportPropertySetMapper.get(),
        OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_ROW_STYLES_PREFIX)));
    mrExport.GetAutoStylePool()->AddFamily(XML_STYLE_FAMILY_TABLE_CELL,
        OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_CELL_STYLES_NAME)),
        mxCellExportPropertySetMapper.get(),
        OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_CELL_STYLES_PREFIX)));
}

// --------------------------------------------------------------------

XMLTableExport::~XMLTableExport ()
{
}

// --------------------------------------------------------------------

static bool has_states( const std::vector< XMLPropertyState >& xPropStates )
{
    if( !xPropStates.empty() )
    {
        std::vector< XMLPropertyState >::const_iterator aIter( xPropStates.begin() );
        std::vector< XMLPropertyState >::const_iterator aEnd( xPropStates.end() );
        while( aIter != aEnd )
        {
            if( aIter->mnIndex != -1 )
                return true;
            ++aIter;
        }
    }
    return false;
}

// --------------------------------------------------------------------

 void XMLTableExport::collectTableAutoStyles(const Reference < XColumnRowRange >& xColumnRowRange)
 {
     if( !mbExportTables )
         return;

    boost::shared_ptr< XMLTableInfo > pTableInfo( new XMLTableInfo() );
    maTableInfoMap[xColumnRowRange] = pTableInfo;

    try
    {
        Reference< XIndexAccess > xIndexAccessCols( xColumnRowRange->getColumns(), UNO_QUERY_THROW );
        const sal_Int32 nColumnCount = xIndexAccessCols->getCount();
         for( sal_Int32 nColumn = 0; nColumn < nColumnCount; ++nColumn ) try
        {
             Reference< XPropertySet > xPropSet( xIndexAccessCols->getByIndex(nColumn) , UNO_QUERY_THROW );
            std::vector< XMLPropertyState > xPropStates( mxColumnExportPropertySetMapper->Filter( xPropSet ) );

            if( has_states( xPropStates ) )
            {
                const OUString sStyleName( mrExport.GetAutoStylePool()->Add(XML_STYLE_FAMILY_TABLE_COLUMN, xPropStates) );
                Reference< XInterface > xKey( xPropSet, UNO_QUERY );
                pTableInfo->maColumnStyleMap[xKey] = sStyleName;
            }
        }
        catch( Exception& )
        {
            OSL_FAIL("xmloff::XMLTableExport::collectTableAutoStyles(), exception during column style collection!");
        }

        Reference< XIndexAccess > xIndexAccessRows( xColumnRowRange->getRows(), UNO_QUERY_THROW );
        const sal_Int32 nRowCount = xIndexAccessRows->getCount();
        pTableInfo->maDefaultRowCellStyles.resize(nRowCount);

        StringStatisticHelper aStringStatistic;

         for( sal_Int32 nRow = 0; nRow < nRowCount; ++nRow ) try
        {
             Reference< XPropertySet > xPropSet( xIndexAccessRows->getByIndex(nRow) , UNO_QUERY_THROW );
            std::vector< XMLPropertyState > xRowPropStates( mxRowExportPropertySetMapper->Filter( xPropSet ) );

            if( has_states( xRowPropStates ) )
            {
                const OUString sStyleName( mrExport.GetAutoStylePool()->Add(XML_STYLE_FAMILY_TABLE_ROW, xRowPropStates) );
                Reference< XInterface > xKey( xPropSet, UNO_QUERY );
                pTableInfo->maRowStyleMap[xKey] = sStyleName;
            }

            // get the current row
            Reference< XCellRange > xCellRange( xPropSet, UNO_QUERY_THROW );
            for ( sal_Int32 nColumn = 0; nColumn < nColumnCount; ++nColumn )
            {
                // get current cell, remarks row index is 0, because we get the range for each row seperate
                Reference< XPropertySet > xCellSet( xCellRange->getCellByPosition(nColumn, 0), UNO_QUERY_THROW );

                // get style
                OUString sParentStyleName;
                Reference< XPropertySetInfo > xPropertySetInfo( xCellSet->getPropertySetInfo() );
                if( xPropertySetInfo.is() && xPropertySetInfo->hasPropertyByName( OUString(RTL_CONSTASCII_USTRINGPARAM("Style"))) )
                {
                    Reference< XStyle > xStyle( xCellSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("Style"))), UNO_QUERY );
                    if( xStyle.is() )
                        sParentStyleName = xStyle->getName();
                }

                // create auto style, if needed
                OUString sStyleName;
                std::vector< XMLPropertyState > xCellPropStates( mxCellExportPropertySetMapper->Filter( xCellSet ) );
                if( has_states( xCellPropStates ) )
                    sStyleName = mrExport.GetAutoStylePool()->Add(XML_STYLE_FAMILY_TABLE_CELL, xCellPropStates);
                else
                    sStyleName = sParentStyleName;

                if( sStyleName.getLength() )
                {
                    Reference< XInterface > xKey( xCellSet, UNO_QUERY );
                    pTableInfo->maCellStyleMap[xKey] = sStyleName;
                }

                // create auto style for text
                Reference< XText > xText(xCellSet, UNO_QUERY);
                if(xText.is() && xText->getString().getLength())
                    GetExport().GetTextParagraphExport()->collectTextAutoStyles( xText );

                aStringStatistic.add( sStyleName );
            }

            OUString sDefaultCellStyle;
            if( aStringStatistic.getModeString( sDefaultCellStyle ) > 1 )
                pTableInfo->maDefaultRowCellStyles[nRow] = sDefaultCellStyle;

            aStringStatistic.clear();
        }
        catch( Exception& )
        {
            OSL_FAIL("xmloff::XMLTableExport::collectTableAutoStyles(), exception during column style collection!");
        }
    }
    catch( Exception& )
    {
        OSL_FAIL("xmloff::XMLTableExport::collectTableAutoStyles(), exception caught!");
    }
 }

 // --------------------------------------------------------------------

 void XMLTableExport::exportTable( const Reference < XColumnRowRange >& xColumnRowRange )
 {
     if( !mbExportTables )
         return;

     try
    {
        boost::shared_ptr< XMLTableInfo > pTableInfo( maTableInfoMap[xColumnRowRange] );

        // get row and column count
        Reference< XIndexAccess > xIndexAccess( xColumnRowRange->getRows(), UNO_QUERY_THROW );
        Reference< XIndexAccess > xIndexAccessCols( xColumnRowRange->getColumns(), UNO_QUERY_THROW );

        const sal_Int32 rowCount = xIndexAccess->getCount();
        const sal_Int32 columnCount = xIndexAccessCols->getCount();

        SvXMLElementExport tableElement( mrExport, XML_NAMESPACE_TABLE, XML_TABLE, sal_True, sal_True );

        // export table columns
        ExportTableColumns( xIndexAccessCols, pTableInfo );

        // start iterating rows and columns
        for ( sal_Int32 rowIndex = 0; rowIndex < rowCount; rowIndex++ )
        {
            // get the current row
            Reference< XCellRange > xCellRange( xIndexAccess->getByIndex(rowIndex), UNO_QUERY_THROW );

            OUString sDefaultCellStyle;

            // table:style-name
            if( pTableInfo.get() )
            {
                Reference< XInterface > xKey( xCellRange, UNO_QUERY );
                const OUString sStyleName( pTableInfo->maRowStyleMap[xKey] );
                if( sStyleName.getLength() )
                    mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_STYLE_NAME, sStyleName );

                sDefaultCellStyle = pTableInfo->maDefaultRowCellStyles[rowIndex];
                if( sDefaultCellStyle.getLength() )
                    mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DEFAULT_CELL_STYLE_NAME, sDefaultCellStyle );
            }

            // write row element
            SvXMLElementExport tableRowElement( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_ROW, sal_True, sal_True );

            for ( sal_Int32 columnIndex = 0; columnIndex < columnCount; columnIndex++ )
            {
                // get current cell, remarks row index is 0, because we get the range for each row seperate
                Reference< XCell > xCell( xCellRange->getCellByPosition(columnIndex, 0), UNO_QUERY_THROW );

                // use XMergeableCell interface from offapi
                Reference< XMergeableCell > xMergeableCell( xCell, UNO_QUERY_THROW );

                // export cell
                ExportCell( xCell, pTableInfo, sDefaultCellStyle );
            }
        }
     }
     catch( Exception )
    {
         OSL_FAIL( "XMLTableExport::exportTable(), exception cought!" );
     }
 }

// --------------------------------------------------------------------
// Export the table columns
// --------------------------------------------------------------------

 void XMLTableExport::ExportTableColumns( const Reference < XIndexAccess >& xtableColumnsIndexAccess, const boost::shared_ptr< XMLTableInfo >& pTableInfo )
 {
    const sal_Int32 nColumnCount = xtableColumnsIndexAccess->getCount();
     for( sal_Int32 nColumn = 0; nColumn < nColumnCount; ++nColumn )
    {
         Reference< XPropertySet > xColumnProperties( xtableColumnsIndexAccess->getByIndex(nColumn) , UNO_QUERY );
         if ( xColumnProperties.is() )
        {
            // table:style-name
            if( pTableInfo.get() )
            {
                Reference< XInterface > xKey( xColumnProperties, UNO_QUERY );
                const OUString sStyleName( pTableInfo->maColumnStyleMap[xKey] );
                if( sStyleName.getLength() )
                    mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_STYLE_NAME, sStyleName );
            }

             // TODO: All columns first have to be checked if some ones
             // have identical properties. If yes, attr table:number-columns-repeated
             // has to be written.
             SvXMLElementExport tableColumnElement( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_COLUMN, sal_True, sal_True );
         }
     }
 }

// --------------------------------------------------------------------
// ODF export for a table cell.
// --------------------------------------------------------------------

 void XMLTableExport::ExportCell( const Reference < XCell >& xCell, const boost::shared_ptr< XMLTableInfo >& pTableInfo, const OUString& rDefaultCellStyle )
 {
    bool bIsMerged = false;
    sal_Int32 nRowSpan = 0;
    sal_Int32 nColSpan = 0;

     try
    {
        if( pTableInfo.get() )
        {
            // table:style-name
            Reference< XInterface > xKey( xCell, UNO_QUERY );
            const OUString sStyleName( pTableInfo->maCellStyleMap[xKey] );
            if( sStyleName.getLength() && (sStyleName != rDefaultCellStyle) )
                mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_STYLE_NAME, sStyleName );
        }

        Reference< XMergeableCell > xMerge( xCell, UNO_QUERY );
        if( xMerge.is() )
        {
            bIsMerged = xMerge->isMerged();
            nRowSpan = xMerge->getRowSpan();
            nColSpan = xMerge->getColumnSpan();
        }
        DBG_ASSERT( (nRowSpan >= 1) && (nColSpan >= 1), "xmloff::XMLTableExport::ExportCell(), illegal row or col span < 1?" );
    }
    catch ( Exception )
    {
        OSL_FAIL( "exception while exporting a table cell" );
    }

    // table:number-columns-repeated
    // todo

    // table:number-columns-spanned
    if( nColSpan > 1 )
        mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_NUMBER_COLUMNS_SPANNED, OUString::valueOf( nColSpan ) );

    // table:number-rows-spanned
    if( nRowSpan > 1 )
        mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_NUMBER_ROWS_SPANNED, OUString::valueOf( nRowSpan ) );

     // <table:table-cell> or <table:covered-table-cell>
    SvXMLElementExport tableCellElement( mrExport, XML_NAMESPACE_TABLE, bIsMerged ? XML_COVERED_TABLE_CELL : XML_TABLE_CELL, sal_True, sal_True );

    // export cells text content
    ImpExportText( xCell );
 }

// --------------------------------------------------------------------
// ODF export of the text contents of a table cell.
// Remarks: Up to now we only export text contents!
// TODO: Check against nested tables ....
// --------------------------------------------------------------------

 void XMLTableExport::ImpExportText( const Reference< XCell >& xCell )
 {
    Reference< XText > xText( xCell, UNO_QUERY );
    if( xText.is() && xText->getString().getLength())
        mrExport.GetTextParagraphExport()->exportText( xText );
 }

// --------------------------------------------------------------------

void XMLTableExport::exportTableStyles()
{
     if( !mbExportTables )
         return;

    XMLStyleExport aStEx(mrExport, OUString(), mrExport.GetAutoStylePool().get());

    // write graphic family styles
    aStEx.exportStyleFamily("cell", OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_TABLE_CELL_STYLES_NAME)), mxCellExportPropertySetMapper.get(), TRUE, XML_STYLE_FAMILY_TABLE_CELL);

    exportTableTemplates();
}

// --------------------------------------------------------------------
// Export the collected automatic styles
// --------------------------------------------------------------------

void XMLTableExport::exportAutoStyles()
{
     if( !mbExportTables )
         return;

    mrExport.GetAutoStylePool()->exportXML( XML_STYLE_FAMILY_TABLE_COLUMN, mrExport.GetDocHandler(), mrExport.GetMM100UnitConverter(), mrExport.GetNamespaceMap() );
    mrExport.GetAutoStylePool()->exportXML( XML_STYLE_FAMILY_TABLE_ROW, mrExport.GetDocHandler(), mrExport.GetMM100UnitConverter(), mrExport.GetNamespaceMap() );
    mrExport.GetAutoStylePool()->exportXML( XML_STYLE_FAMILY_TABLE_CELL, mrExport.GetDocHandler(), mrExport.GetMM100UnitConverter(), mrExport.GetNamespaceMap() );
}

// --------------------------------------------------------------------

const TableStyleElement* getTableStyleMap()
{
    static struct TableStyleElement gTableStyleElements[] =
    {
        { XML_FIRST_ROW, OUString( RTL_CONSTASCII_USTRINGPARAM( "first-row" ) ) },
        { XML_LAST_ROW, OUString( RTL_CONSTASCII_USTRINGPARAM( "last-row" ) ) },
        { XML_FIRST_COLUMN, OUString( RTL_CONSTASCII_USTRINGPARAM( "first-column" ) ) },
        { XML_LAST_COLUMN, OUString( RTL_CONSTASCII_USTRINGPARAM( "last-column" ) ) },
        { XML_EVEN_ROWS, OUString( RTL_CONSTASCII_USTRINGPARAM( "even-rows" ) ) },
        { XML_ODD_ROWS, OUString( RTL_CONSTASCII_USTRINGPARAM( "odd-rows" ) ) },
        { XML_EVEN_COLUMNS, OUString( RTL_CONSTASCII_USTRINGPARAM( "even-columns" ) ) },
        { XML_ODD_COLUMNS, OUString( RTL_CONSTASCII_USTRINGPARAM( "odd-columns" ) ) },
        { XML_BODY, OUString( RTL_CONSTASCII_USTRINGPARAM( "body" ) ) },
        { XML_TOKEN_END, OUString() }
    };

    return &gTableStyleElements[0];
}

// --------------------------------------------------------------------

void XMLTableExport::exportTableTemplates()
{
     if( !mbExportTables )
         return;

    try
    {
        Reference< XStyleFamiliesSupplier > xFamiliesSupp( mrExport.GetModel(), UNO_QUERY_THROW );
        Reference< XNameAccess > xFamilies( xFamiliesSupp->getStyleFamilies() );
        const OUString sFamilyName( RTL_CONSTASCII_USTRINGPARAM("table" ) );
        Reference< XIndexAccess > xTableFamily( xFamilies->getByName( sFamilyName ), UNO_QUERY_THROW );

        for( sal_Int32 nIndex = 0; nIndex < xTableFamily->getCount(); nIndex++ ) try
        {
            Reference< XStyle > xTableStyle( xTableFamily->getByIndex( nIndex ), UNO_QUERY_THROW );
            if( !xTableStyle->isInUse() )
                continue;

            Reference< XNameAccess > xStyleNames( xTableStyle, UNO_QUERY_THROW );

            mrExport.AddAttribute(XML_NAMESPACE_TEXT, XML_STYLE_NAME, GetExport().EncodeStyleName( xTableStyle->getName() ) );
             SvXMLElementExport tableTemplate( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_TEMPLATE, sal_True, sal_True );

            const TableStyleElement* pElements = getTableStyleMap();
            while( pElements->meElement != XML_TOKEN_END )
            {
                try
                {
                    Reference< XStyle > xStyle( xStyleNames->getByName( pElements->msStyleName ), UNO_QUERY );
                    if( xStyle.is() )
                    {
                        mrExport.AddAttribute(XML_NAMESPACE_TEXT, XML_STYLE_NAME, GetExport().EncodeStyleName( xStyle->getName() ) );
                         SvXMLElementExport element( mrExport, XML_NAMESPACE_TABLE, pElements->meElement, sal_True, sal_True );
                    }
                }
                catch( Exception& )
                {
                    OSL_FAIL("xmloff::XMLTableExport::exportTableTemplates(), exception caught!");
                }

                pElements++;
            }
        }
        catch( Exception& )
        {
            OSL_FAIL("xmloff::XMLTableExport::exportTableDesigns(), exception caught while exporting a table design!");
        }
    }
    catch( Exception& )
    {
        OSL_FAIL("xmloff::XMLTableExport::exportTableDesigns(), exception caught!");
    }
}

// --------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
