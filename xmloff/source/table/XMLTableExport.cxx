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

#include <xmloff/table/XMLTableExport.hxx>

#include <xmloff/dllapi.h>

#include <sal/config.h>
#include <sal/log.hxx>
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
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <tools/diagnose_ex.h>

#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmlprmap.hxx>
#include <xmloff/xmlexppr.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmltypes.hxx>
#include <xmlsdtypes.hxx>
#include <xmloff/maptype.hxx>
#include <xmloff/prhdlfac.hxx>
#include <xmloff/txtprmap.hxx>
#include "table.hxx"

using namespace ::xmloff::token;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::style;

#define MAP_(name,prefix,token,type,context)  { name, sizeof(name)-1, prefix, token, type, context, SvtSaveOptions::ODFVER_010, false }
#define CMAP(name,prefix,token,type,context) MAP_(name,prefix,token,type|XML_TYPE_PROP_TABLE_COLUMN,context)
#define RMAP(name,prefix,token,type,context) MAP_(name,prefix,token,type|XML_TYPE_PROP_TABLE_ROW,context)
#define CELLMAP(name,prefix,token,type,context) MAP_(name,prefix,token,type|XML_TYPE_PROP_TABLE_CELL,context)
#define MAP_END { nullptr, 0, 0, XML_EMPTY, 0, 0, SvtSaveOptions::ODFVER_010, false }

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

const XMLPropertyMapEntry* getCellPropertiesMap()
{
    static const XMLPropertyMapEntry aXMLCellProperties[] =
    {
        CELLMAP( "RotateAngle",     XML_NAMESPACE_STYLE, XML_ROTATION_ANGLE,         XML_SD_TYPE_CELL_ROTATION_ANGLE,   0),
        MAP_END
    };

    return &aXMLCellProperties[0];
}

class StringStatisticHelper
{
private:
    std::map< OUString, sal_Int32 > mStats;

public:
    void add( const OUString& rStyleName );
    void clear() { mStats.clear(); }

    sal_Int32 getModeString( /* out */ OUString& rModeString );
};

void StringStatisticHelper::add( const OUString& rStyleName )
{
    std::map< OUString, sal_Int32 >::iterator iter( mStats.find( rStyleName ) );
    if( iter == mStats.end() )
    {
        mStats[rStyleName] = 1;
    }
    else
    {
        (*iter).second += 1;
    }
}

sal_Int32 StringStatisticHelper::getModeString( OUString& rStyleName )
{
    sal_Int32 nMax = 0;
    for( const auto& rStatsEntry : mStats )
    {
        if( rStatsEntry.second > nMax )
        {
            rStyleName = rStatsEntry.first;
            nMax = rStatsEntry.second;
        }
    }

    return nMax;
}

// class XMLTableExport

XMLTableExport::XMLTableExport(SvXMLExport& rExp, const rtl::Reference< SvXMLExportPropertyMapper  >& xExportPropertyMapper, const rtl::Reference< XMLPropertyHandlerFactory >& xFactoryRef )
: mrExport( rExp )
, mbExportTables( false )
, mbWriter( false )
{
    Reference< XMultiServiceFactory > xFac( rExp.GetModel(), UNO_QUERY );
    if( xFac.is() ) try
    {
        Sequence< OUString > sSNS( xFac->getAvailableServiceNames() );
        sal_Int32 n = sSNS.getLength();
        const OUString* pSNS( sSNS.getConstArray() );
        while( --n > 0 )
        {
            if( *pSNS == "com.sun.star.drawing.TableShape" || *pSNS == "com.sun.star.style.TableStyle" )
            {
                mbExportTables = true;
                mbWriter = (*pSNS == "com.sun.star.style.TableStyle");
                break;
            }
            pSNS++;
        }
    }
    catch(const Exception&)
    {
    }

    if (mbWriter)
    {
        mxCellExportPropertySetMapper = new SvXMLExportPropertyMapper(new XMLTextPropertySetMapper(TextPropMap::CELL, true));
    }
    else
    {
        mxCellExportPropertySetMapper = xExportPropertyMapper;
        mxCellExportPropertySetMapper->ChainExportMapper(XMLTextParagraphExport::CreateParaExtPropMapper(rExp));
        mxCellExportPropertySetMapper->ChainExportMapper(new SvXMLExportPropertyMapper(new XMLPropertySetMapper(getCellPropertiesMap(), xFactoryRef.get(), true)));
    }

    mxRowExportPropertySetMapper = new SvXMLExportPropertyMapper( new XMLPropertySetMapper( getRowPropertiesMap(), xFactoryRef.get(), true ) );
    mxColumnExportPropertySetMapper = new SvXMLExportPropertyMapper( new XMLPropertySetMapper( getColumnPropertiesMap(), xFactoryRef.get(), true ) );

    mrExport.GetAutoStylePool()->AddFamily(XML_STYLE_FAMILY_TABLE_COLUMN,
        OUString(XML_STYLE_FAMILY_TABLE_COLUMN_STYLES_NAME),
        mxColumnExportPropertySetMapper.get(),
        OUString(XML_STYLE_FAMILY_TABLE_COLUMN_STYLES_PREFIX));
    mrExport.GetAutoStylePool()->AddFamily(XML_STYLE_FAMILY_TABLE_ROW,
        OUString(XML_STYLE_FAMILY_TABLE_ROW_STYLES_NAME),
        mxRowExportPropertySetMapper.get(),
        OUString(XML_STYLE_FAMILY_TABLE_ROW_STYLES_PREFIX));
    mrExport.GetAutoStylePool()->AddFamily(XML_STYLE_FAMILY_TABLE_CELL,
        OUString(XML_STYLE_FAMILY_TABLE_CELL_STYLES_NAME),
        mxCellExportPropertySetMapper.get(),
        OUString(XML_STYLE_FAMILY_TABLE_CELL_STYLES_PREFIX));
}

XMLTableExport::~XMLTableExport ()
{
}

static bool has_states( const std::vector< XMLPropertyState >& xPropStates )
{
    return std::any_of(xPropStates.cbegin(), xPropStates.cend(),
        [](const XMLPropertyState& rPropertyState) { return rPropertyState.mnIndex != -1; });
}

 void XMLTableExport::collectTableAutoStyles(const Reference < XColumnRowRange >& xColumnRowRange)
 {
    if( !mbExportTables )
         return;

    std::shared_ptr< XMLTableInfo > xTableInfo( new XMLTableInfo );
    maTableInfoMap[xColumnRowRange] = xTableInfo;

    try
    {
        Reference< XIndexAccess > xIndexAccessCols( xColumnRowRange->getColumns(), UNO_QUERY_THROW );
        const sal_Int32 nColumnCount = xIndexAccessCols->getCount();
        for( sal_Int32 nColumn = 0; nColumn < nColumnCount; ++nColumn ) try
        {
            Reference< XPropertySet > xPropSet( xIndexAccessCols->getByIndex(nColumn) , UNO_QUERY_THROW );
            std::vector< XMLPropertyState > aPropStates( mxColumnExportPropertySetMapper->Filter( xPropSet ) );

            if( has_states( aPropStates ) )
            {
                const OUString sStyleName( mrExport.GetAutoStylePool()->Add(XML_STYLE_FAMILY_TABLE_COLUMN, aPropStates) );
                Reference< XInterface > xKey( xPropSet, UNO_QUERY );
                xTableInfo->maColumnStyleMap[xKey] = sStyleName;
            }
        }
        catch(const Exception&)
        {
            OSL_FAIL("xmloff::XMLTableExport::collectTableAutoStyles(), exception during column style collection!");
        }

        Reference< XIndexAccess > xIndexAccessRows( xColumnRowRange->getRows(), UNO_QUERY_THROW );
        const sal_Int32 nRowCount = xIndexAccessRows->getCount();
        xTableInfo->maDefaultRowCellStyles.resize(nRowCount);

        StringStatisticHelper aStringStatistic;

        for( sal_Int32 nRow = 0; nRow < nRowCount; ++nRow )
        try
        {
            Reference< XPropertySet > xPropSet( xIndexAccessRows->getByIndex(nRow) , UNO_QUERY_THROW );
            std::vector< XMLPropertyState > aRowPropStates( mxRowExportPropertySetMapper->Filter( xPropSet ) );

            if( has_states( aRowPropStates ) )
            {
                const OUString sStyleName( mrExport.GetAutoStylePool()->Add(XML_STYLE_FAMILY_TABLE_ROW, aRowPropStates) );
                Reference< XInterface > xKey( xPropSet, UNO_QUERY );
                xTableInfo->maRowStyleMap[xKey] = sStyleName;
            }

            // get the current row
            Reference< XCellRange > xCellRange( xPropSet, UNO_QUERY_THROW );
            for ( sal_Int32 nColumn = 0; nColumn < nColumnCount; ++nColumn )
            {
                // get current cell, remarks row index is 0, because we get the range for each row separate
                Reference< XPropertySet > xCellSet( xCellRange->getCellByPosition(nColumn, 0), UNO_QUERY_THROW );

                // get style
                OUString sParentStyleName;
                Reference< XPropertySetInfo > xPropertySetInfo( xCellSet->getPropertySetInfo() );
                if( xPropertySetInfo.is() && xPropertySetInfo->hasPropertyByName("Style") )
                {
                    Reference< XStyle > xStyle( xCellSet->getPropertyValue("Style"), UNO_QUERY );
                    if( xStyle.is() )
                        sParentStyleName = xStyle->getName();
                }

                // create auto style, if needed
                OUString sStyleName;
                std::vector< XMLPropertyState > aCellPropStates( mxCellExportPropertySetMapper->Filter( xCellSet ) );
                if( has_states( aCellPropStates ) )
                    sStyleName = mrExport.GetAutoStylePool()->Add(XML_STYLE_FAMILY_TABLE_CELL, aCellPropStates);
                else
                    sStyleName = sParentStyleName;

                if( !sStyleName.isEmpty() )
                {
                    Reference< XInterface > xKey( xCellSet, UNO_QUERY );
                    xTableInfo->maCellStyleMap[xKey] = sStyleName;
                }

                // create auto style for text
                Reference< XText > xText(xCellSet, UNO_QUERY);
                if(xText.is() && !xText->getString().isEmpty())
                    GetExport().GetTextParagraphExport()->collectTextAutoStyles( xText );

                aStringStatistic.add( sStyleName );
            }

            OUString sDefaultCellStyle;
            if( aStringStatistic.getModeString( sDefaultCellStyle ) > 1 )
                xTableInfo->maDefaultRowCellStyles[nRow] = sDefaultCellStyle;

            aStringStatistic.clear();
        }
        catch(const Exception&)
        {
            OSL_FAIL("xmloff::XMLTableExport::collectTableAutoStyles(), exception during column style collection!");
        }
    }
    catch(const Exception&)
    {
        OSL_FAIL("xmloff::XMLTableExport::collectTableAutoStyles(), exception caught!");
    }
 }

 void XMLTableExport::exportTable( const Reference < XColumnRowRange >& xColumnRowRange )
 {
    if( !mbExportTables )
        return;

    try
    {
        std::shared_ptr< XMLTableInfo > xTableInfo( maTableInfoMap[xColumnRowRange] );

        // get row and column count
        Reference< XIndexAccess > xIndexAccess( xColumnRowRange->getRows(), UNO_QUERY_THROW );
        Reference< XIndexAccess > xIndexAccessCols( xColumnRowRange->getColumns(), UNO_QUERY_THROW );

        const sal_Int32 rowCount = xIndexAccess->getCount();
        const sal_Int32 columnCount = xIndexAccessCols->getCount();

        SvXMLElementExport tableElement( mrExport, XML_NAMESPACE_TABLE, XML_TABLE, true, true );

        // export table columns
        ExportTableColumns( xIndexAccessCols, xTableInfo );

        // start iterating rows and columns
        for ( sal_Int32 rowIndex = 0; rowIndex < rowCount; rowIndex++ )
        {
            // get the current row
            Reference< XCellRange > xCellRange( xIndexAccess->getByIndex(rowIndex), UNO_QUERY_THROW );

            OUString sDefaultCellStyle;

            // table:style-name
            if( xTableInfo.get() )
            {
                Reference< XInterface > xKey( xCellRange, UNO_QUERY );
                const OUString sStyleName( xTableInfo->maRowStyleMap[xKey] );
                if( !sStyleName.isEmpty() )
                    mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_STYLE_NAME, sStyleName );

                sDefaultCellStyle = xTableInfo->maDefaultRowCellStyles[rowIndex];
                if( !sDefaultCellStyle.isEmpty() )
                    mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DEFAULT_CELL_STYLE_NAME, sDefaultCellStyle );
            }

            // write row element
            SvXMLElementExport tableRowElement( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_ROW, true, true );

            for ( sal_Int32 columnIndex = 0; columnIndex < columnCount; columnIndex++ )
            {
                // get current cell, remarks row index is 0, because we get the range for each row separate
                Reference< XCell > xCell( xCellRange->getCellByPosition(columnIndex, 0), UNO_QUERY_THROW );

                // use XMergeableCell interface from offapi
                Reference< XMergeableCell > xMergeableCell( xCell, UNO_QUERY_THROW );

                // export cell
                ExportCell( xCell, xTableInfo, sDefaultCellStyle );
            }
        }
     }
     catch(const Exception&)
    {
         OSL_FAIL( "XMLTableExport::exportTable(), exception caught!" );
     }
 }

// Export the table columns

 void XMLTableExport::ExportTableColumns( const Reference < XIndexAccess >& xtableColumnsIndexAccess, const std::shared_ptr< XMLTableInfo >& rTableInfo )
 {
    const sal_Int32 nColumnCount = xtableColumnsIndexAccess->getCount();
    for( sal_Int32 nColumn = 0; nColumn < nColumnCount; ++nColumn )
    {
         Reference< XPropertySet > xColumnProperties( xtableColumnsIndexAccess->getByIndex(nColumn) , UNO_QUERY );
         if ( xColumnProperties.is() )
        {
            // table:style-name
            if( rTableInfo.get() )
            {
                Reference< XInterface > xKey( xColumnProperties, UNO_QUERY );
                const OUString sStyleName( rTableInfo->maColumnStyleMap[xKey] );
                if( !sStyleName.isEmpty() )
                    mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_STYLE_NAME, sStyleName );
            }

            // TODO: All columns first have to be checked if some ones
            // have identical properties. If yes, attr table:number-columns-repeated
            // has to be written.
            SvXMLElementExport tableColumnElement( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_COLUMN, true, true );
         }
     }
 }

// ODF export for a table cell.

 void XMLTableExport::ExportCell( const Reference < XCell >& xCell, const std::shared_ptr< XMLTableInfo >& rTableInfo, const OUString& rDefaultCellStyle )
 {
    bool bIsMerged = false;
    sal_Int32 nRowSpan = 0;
    sal_Int32 nColSpan = 0;

    try
    {
        if( rTableInfo.get() )
        {
            // table:style-name
            Reference< XInterface > xKey( xCell, UNO_QUERY );
            const OUString sStyleName( rTableInfo->maCellStyleMap[xKey] );
            if( !sStyleName.isEmpty() && (sStyleName != rDefaultCellStyle) )
                mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_STYLE_NAME, sStyleName );
        }

        Reference< XMergeableCell > xMerge( xCell, UNO_QUERY );
        if( xMerge.is() )
        {
            bIsMerged = xMerge->isMerged();
            nRowSpan = xMerge->getRowSpan();
            nColSpan = xMerge->getColumnSpan();
        }
        SAL_WARN_IF( (nRowSpan < 1) || (nColSpan < 1), "xmloff", "xmloff::XMLTableExport::ExportCell(), illegal row or col span < 1?" );
    }
    catch (const Exception&)
    {
        OSL_FAIL( "exception while exporting a table cell" );
    }

    // table:number-columns-repeated
    // todo

    // table:number-columns-spanned
    if( nColSpan > 1 )
        mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_NUMBER_COLUMNS_SPANNED, OUString::number( nColSpan ) );

    // table:number-rows-spanned
    if( nRowSpan > 1 )
        mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_NUMBER_ROWS_SPANNED, OUString::number( nRowSpan ) );

     // <table:table-cell> or <table:covered-table-cell>
    SvXMLElementExport tableCellElement( mrExport, XML_NAMESPACE_TABLE, bIsMerged ? XML_COVERED_TABLE_CELL : XML_TABLE_CELL, true, true );

    // export cells text content
    ImpExportText( xCell );
 }

// ODF export of the text contents of a table cell.
// Remarks: Up to now we only export text contents!
// TODO: Check against nested tables ....

 void XMLTableExport::ImpExportText( const Reference< XCell >& xCell )
 {
    Reference< XText > xText( xCell, UNO_QUERY );
    if( xText.is() && !xText->getString().isEmpty())
        mrExport.GetTextParagraphExport()->exportText( xText );
 }

void XMLTableExport::exportTableStyles()
{
    if( !mbExportTables )
         return;

    rtl::Reference<XMLStyleExport> aStEx;
    OUString sCellStyleName;
    if (mbWriter)
    {
        sCellStyleName = "CellStyles";
        aStEx.set(new XMLStyleExport(mrExport));
    }
    else
    {
        // write graphic family styles
        sCellStyleName = "cell";
        aStEx.set(new XMLStyleExport(mrExport, mrExport.GetAutoStylePool().get()));
    }

    aStEx->exportStyleFamily(sCellStyleName, OUString(XML_STYLE_FAMILY_TABLE_CELL_STYLES_NAME), mxCellExportPropertySetMapper.get(), true, XML_STYLE_FAMILY_TABLE_CELL);

    exportTableTemplates();
}

// Export the collected automatic styles

void XMLTableExport::exportAutoStyles()
{
    if( !mbExportTables )
         return;

    mrExport.GetAutoStylePool()->exportXML( XML_STYLE_FAMILY_TABLE_COLUMN );
    mrExport.GetAutoStylePool()->exportXML( XML_STYLE_FAMILY_TABLE_ROW );
    mrExport.GetAutoStylePool()->exportXML( XML_STYLE_FAMILY_TABLE_CELL );
}

const TableStyleElement* getTableStyleMap()
{
    static const struct TableStyleElement gTableStyleElements[] =
    {
        { XML_FIRST_ROW, OUString("first-row") },
        { XML_LAST_ROW, OUString("last-row") },
        { XML_FIRST_COLUMN, OUString("first-column") },
        { XML_LAST_COLUMN, OUString("last-column") },
        { XML_BODY, OUString("body") },
        { XML_EVEN_ROWS, OUString("even-rows") },
        { XML_ODD_ROWS, OUString("odd-rows") },
        { XML_EVEN_COLUMNS, OUString("even-columns") },
        { XML_ODD_COLUMNS, OUString("odd-columns") },
        { XML_BACKGROUND, OUString("background") },
        { XML_TOKEN_END, OUString() }
    };

    return &gTableStyleElements[0];
}

const TableStyleElement* getWriterSpecificTableStyleMap()
{
    static const struct TableStyleElement gWriterSpecificTableStyleElements[] =
    {
        { XML_FIRST_ROW_EVEN_COLUMN, OUString("first-row-even-column") },
        { XML_LAST_ROW_EVEN_COLUMN, OUString("last-row-even-column") },
        { XML_FIRST_ROW_END_COLUMN, OUString("first-row-end-column") },
        { XML_FIRST_ROW_START_COLUMN, OUString("first-row-start-column") },
        { XML_LAST_ROW_END_COLUMN, OUString("last-row-end-column") },
        { XML_LAST_ROW_START_COLUMN, OUString("last-row-start-column") },
        { XML_TOKEN_END, OUString() }
    };

    return &gWriterSpecificTableStyleElements[0];
}

static const TableStyleElement* getWriterSpecificTableStyleAttributes()
{
    static const struct TableStyleElement gWriterSpecifitTableStyleAttributes[] =
    {
        { XML_FIRST_ROW_END_COLUMN, OUString("FirstRowEndColumn") },
        { XML_FIRST_ROW_START_COLUMN, OUString("FirstRowStartColumn") },
        { XML_LAST_ROW_END_COLUMN, OUString("LastRowEndColumn") },
        { XML_LAST_ROW_START_COLUMN, OUString("LastRowStartColumn") },
        { XML_TOKEN_END, OUString() }
    };

    return &gWriterSpecifitTableStyleAttributes[0];
}

void XMLTableExport::exportTableTemplates()
{
    if( !mbExportTables )
         return;

    try
    {
        Reference< XStyleFamiliesSupplier > xFamiliesSupp( mrExport.GetModel(), UNO_QUERY_THROW );
        Reference< XNameAccess > xFamilies( xFamiliesSupp->getStyleFamilies() );
        OUString sFamilyName;
        if (mbWriter)
            sFamilyName = "TableStyles";
        else
            sFamilyName = "table";

        Reference< XIndexAccess > xTableFamily( xFamilies->getByName( sFamilyName ), UNO_QUERY_THROW );

        for( sal_Int32 nIndex = 0; nIndex < xTableFamily->getCount(); nIndex++ ) try
        {
            SvtSaveOptions::ODFSaneDefaultVersion eVersion = mrExport.getSaneDefaultVersion();

            Reference< XStyle > xTableStyle( xTableFamily->getByIndex( nIndex ), UNO_QUERY_THROW );
            if( !xTableStyle->isInUse() )
                continue;

            const TableStyleElement* pElements;
            if (mbWriter)
            {
                mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_NAME, xTableStyle->getName());
                Reference<XPropertySet> xTableStylePropSet(xTableStyle.get(), UNO_QUERY_THROW);
                pElements = getWriterSpecificTableStyleAttributes();
                while(pElements->meElement != XML_TOKEN_END)
                {
                    try
                    {
                        OUString sVal;
                        xTableStylePropSet->getPropertyValue(pElements->msStyleName) >>= sVal;
                        mrExport.AddAttribute(XML_NAMESPACE_TABLE, pElements->meElement, sVal);
                    }
                    catch(const Exception&)
                    {
                        css::uno::Any ex( cppu::getCaughtException() );
                        SAL_WARN("xmloff", "XMLTableExport::exportTableTemplates(), export Writer specific attributes, exception caught! " << exceptionToString(ex));
                    }
                    pElements++;
                }
            }
            else
            {
                // checks if any of the extended version of ODF are set
                if ((eVersion & SvtSaveOptions::ODFSVER_EXTENDED) != 0)
                {
                    // tdf#106780 historically this wrong attribute was used
                    // for the name; write it if extended because LO < 5.3 can
                    // read only text:style-name, not the correct table:name
                    mrExport.AddAttribute(XML_NAMESPACE_TEXT, XML_STYLE_NAME, GetExport().EncodeStyleName( xTableStyle->getName() ) );
                }
                mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_NAME, GetExport().EncodeStyleName(xTableStyle->getName()));
            }

            SvXMLElementExport tableTemplate( mrExport, XML_NAMESPACE_TABLE, XML_TABLE_TEMPLATE, true, true );

            Reference< XNameAccess > xStyleNames( xTableStyle, UNO_QUERY_THROW );
            pElements = getTableStyleMap();
            while( pElements->meElement != XML_TOKEN_END )
            {
                try
                {
                    Reference< XStyle > xStyle( xStyleNames->getByName( pElements->msStyleName ), UNO_QUERY );
                    if( xStyle.is() )
                    {
                        mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_STYLE_NAME, GetExport().EncodeStyleName( xStyle->getName() ) );
                        SvXMLElementExport element( mrExport, XML_NAMESPACE_TABLE, pElements->meElement, true, true );
                    }
                }
                catch(const Exception&)
                {
                    OSL_FAIL("xmloff::XMLTableExport::exportTableTemplates(), exception caught!");
                }

                pElements++;
            }

            if (mbWriter && ((eVersion & SvtSaveOptions::ODFSVER_EXTENDED) != 0))
            {
                pElements = getWriterSpecificTableStyleMap();
                while(pElements->meElement != XML_TOKEN_END)
                {
                    try
                    {
                        Reference<XStyle> xStyle(xStyleNames->getByName(pElements->msStyleName), UNO_QUERY);
                        if(xStyle.is())
                        {
                            mrExport.AddAttribute(XML_NAMESPACE_TABLE, XML_STYLE_NAME, GetExport().EncodeStyleName(xStyle->getName()));
                            SvXMLElementExport element(mrExport, XML_NAMESPACE_LO_EXT, pElements->meElement, true, true);
                        }
                    }
                    catch(const Exception&)
                    {
                        css::uno::Any ex( cppu::getCaughtException() );
                        SAL_WARN("xmloff", "XMLTableExport::exportTableTemplates(), export Writer specific styles, exception caught!" << exceptionToString(ex));
                    }
                    pElements++;
                }
            }
        }
        catch(const Exception&)
        {
            css::uno::Any ex( cppu::getCaughtException() );
            SAL_WARN("xmloff", "XMLTableExport::exportTableDesigns(), exception caught while exporting a table design! " << exceptionToString(ex));
        }
    }
    catch(const Exception&)
    {
        css::uno::Any ex( cppu::getCaughtException() );
        SAL_WARN("xmloff", "XMLTableExport::exportTableDesigns(), exception caught! " << exceptionToString(ex));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
