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

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/table/XTableRows.hpp>
#include <com/sun/star/table/XMergeableCell.hpp>
#include <com/sun/star/table/XMergeableCellRange.hpp>
#include <com/sun/star/table/XTable.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <comphelper/sequence.hxx>
#include <comphelper/diagnose_ex.hxx>

#include <utility>
#include <xmloff/table/XMLTableImport.hxx>
#include <xmloff/maptype.hxx>
#include <xmloff/xmlprmap.hxx>
#include <xmloff/txtimp.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmlstyle.hxx>
#include <xmloff/prstylei.hxx>

#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmluconv.hxx>
#include "table.hxx"

#include <sal/log.hxx>

#include <memory>

using namespace ::xmloff::token;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;

namespace {

struct ColumnInfo
{
    OUString msStyleName;
    OUString msDefaultCellStyleName;
};

class XMLProxyContext : public SvXMLImportContext
{
public:
    XMLProxyContext( SvXMLImport& rImport, SvXMLImportContextRef xParent );

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;

private:
    SvXMLImportContextRef mxParent;
};

struct MergeInfo
{
    sal_Int32 mnStartColumn;
    sal_Int32 mnStartRow;
    sal_Int32 mnEndColumn;
    sal_Int32 mnEndRow;

    MergeInfo( sal_Int32 nStartColumn, sal_Int32 nStartRow, sal_Int32 nColumnSpan, sal_Int32 nRowSpan )
        : mnStartColumn( nStartColumn ), mnStartRow( nStartRow ), mnEndColumn( nStartColumn + nColumnSpan - 1 ), mnEndRow( nStartRow + nRowSpan - 1 ) {};
};

class XMLCellImportPropertyMapper : public SvXMLImportPropertyMapper
{
public:
    using SvXMLImportPropertyMapper::SvXMLImportPropertyMapper;

    bool handleSpecialItem(
        XMLPropertyState& rProperty,
        std::vector< XMLPropertyState >& rProperties,
        const OUString& rValue,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& /*rNamespaceMap*/) const override
    {
        assert(getPropertySetMapper()->GetEntryXMLName(rProperty.mnIndex) == GetXMLToken(XML_BACKGROUND_COLOR));
        (void)rProperty;

        auto nIndex = getPropertySetMapper()->GetEntryIndex(XML_NAMESPACE_DRAW, GetXMLToken(XML_FILL), 0);
        XMLPropertyState aFillProperty(nIndex);

        if (IsXMLToken(rValue, XML_TRANSPARENT))
        {
            getPropertySetMapper()->importXML(GetXMLToken(XML_NONE), aFillProperty, rUnitConverter);
            rProperties.push_back(aFillProperty);
        }
        else
        {
            getPropertySetMapper()->importXML(GetXMLToken(XML_SOLID), aFillProperty, rUnitConverter);
            rProperties.push_back(aFillProperty);

            nIndex = getPropertySetMapper()->GetEntryIndex(XML_NAMESPACE_DRAW, GetXMLToken(XML_FILL_COLOR), 0);
            XMLPropertyState aColorProperty(nIndex);
            getPropertySetMapper()->importXML(rValue, aColorProperty, rUnitConverter);
            rProperties.push_back(aColorProperty);
        }

        return false;
    }
};

}

class XMLTableImportContext : public SvXMLImportContext
{
public:
    XMLTableImportContext( const rtl::Reference< XMLTableImport >& xThis, Reference< XColumnRowRange > const & xColumnRowRange );

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;

    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

    void InitColumns();

    SvXMLImportContextRef ImportColumn( const Reference< XFastAttributeList >& xAttrList );
    SvXMLImportContext * ImportRow( const Reference< XFastAttributeList >& xAttrList );
    SvXMLImportContextRef ImportCell( sal_Int32 nElement, const Reference< XFastAttributeList >& xAttrList );

    OUString GetDefaultCellStyleName() const;

    css::uno::Reference< css::table::XTable > mxTable;
    Reference< XTableColumns > mxColumns;
    Reference< XTableRows > mxRows;

    std::vector< std::shared_ptr< ColumnInfo > > maColumnInfos;
    sal_Int32 mnCurrentRow;
    sal_Int32 mnCurrentColumn;

    // default cell style name for the current row
    OUString msDefaultCellStyleName;

    std::vector< std::shared_ptr< MergeInfo > > maMergeInfos;
};

namespace {

class XMLCellImportContext : public SvXMLImportContext
{
public:
    XMLCellImportContext( SvXMLImport& rImport,
                          const Reference< XMergeableCell >& xCell,
                          const OUString& sDefaultCellStyleName,
                          sal_Int32 nElement,
                          const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList );

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

    sal_Int32 getColumnSpan() const { return mnColSpan; }
    sal_Int32 getRowSpan() const { return mnRowSpan; }
    sal_Int32 getRepeated() const { return mnRepeated; }

    Reference< XMergeableCell > mxCell;
    Reference< XTextCursor >    mxCursor;
    Reference< XTextCursor >    mxOldCursor;
    bool                        mbListContextPushed;

    sal_Int32 mnColSpan, mnRowSpan, mnRepeated;
};

class XMLTableTemplateContext : public SvXMLStyleContext
{
public:
    XMLTableTemplateContext( SvXMLImport& rImport );

    // Create child element.
    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;

    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

protected:
    virtual void SetAttribute( sal_Int32 nElement,
                               const OUString& rValue ) override;
private:
    XMLTableTemplate maTableTemplate;
    OUString msTemplateStyleName;
};

}


XMLProxyContext::XMLProxyContext( SvXMLImport& rImport, SvXMLImportContextRef xParent )
: SvXMLImportContext( rImport )
, mxParent(std::move( xParent ))
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLProxyContext::createFastChildContext( sal_Int32 nElement, const Reference< XFastAttributeList >& xAttrList )
{
    if( mxParent.is() )
        return mxParent->createFastChildContext( nElement, xAttrList );
    return nullptr;
}


XMLTableImport::XMLTableImport( SvXMLImport& rImport, const rtl::Reference< XMLPropertySetMapper >& xCellPropertySetMapper, const rtl::Reference< XMLPropertyHandlerFactory >& xFactoryRef )
: mrImport( rImport )
, mbWriter( false )
{
    // check if called by Writer
    Reference<XMultiServiceFactory> xFac(rImport.GetModel(), UNO_QUERY);
    if (xFac.is()) try
    {
        Sequence<OUString> sSNS = xFac->getAvailableServiceNames();
        mbWriter = comphelper::findValue(sSNS, "com.sun.star.style.TableStyle") != -1;
    }
    catch(const Exception&)
    {
        SAL_WARN("xmloff.table", "Error while checking available service names");
    }

    if (mbWriter)
    {
        mxCellImportPropertySetMapper = XMLTextImportHelper::CreateTableCellExtPropMapper(rImport);
    }
    else
    {
        mxCellImportPropertySetMapper = new SvXMLImportPropertyMapper( xCellPropertySetMapper, rImport );
        mxCellImportPropertySetMapper->ChainImportMapper(XMLTextImportHelper::CreateParaExtPropMapper(rImport));
        mxCellImportPropertySetMapper->ChainImportMapper(new XMLCellImportPropertyMapper(new XMLPropertySetMapper(getCellPropertiesMap(), xFactoryRef, true), rImport));
    }

    rtl::Reference < XMLPropertySetMapper > xRowMapper( new XMLPropertySetMapper( getRowPropertiesMap(), xFactoryRef, false ) );
    mxRowImportPropertySetMapper = new SvXMLImportPropertyMapper( xRowMapper, rImport );

    rtl::Reference < XMLPropertySetMapper > xColMapper( new XMLPropertySetMapper( getColumnPropertiesMap(), xFactoryRef, false ) );
    mxColumnImportPropertySetMapper = new SvXMLImportPropertyMapper( xColMapper, rImport );
}

XMLTableImport::~XMLTableImport()
{
}

SvXMLImportContext* XMLTableImport::CreateTableContext( Reference< XColumnRowRange > const & xColumnRowRange )
{
    rtl::Reference< XMLTableImport > xThis( this );
    return new XMLTableImportContext( xThis, xColumnRowRange );
}

SvXMLStyleContext* XMLTableImport::CreateTableTemplateContext( sal_Int32 /*nElement*/, const Reference< XFastAttributeList >& /*xAttrList*/ )
{
    return new XMLTableTemplateContext( mrImport );
}

void XMLTableImport::addTableTemplate( const OUString& rsStyleName, XMLTableTemplate& xTableTemplate )
{
    auto xPtr = std::make_shared<XMLTableTemplate>();
    xPtr->swap( xTableTemplate );
    maTableTemplates.emplace_back(rsStyleName, xPtr);
}

void XMLTableImport::finishStyles()
{
    if( maTableTemplates.empty() )
        return;

    try
    {
        Reference< XStyleFamiliesSupplier > xFamiliesSupp( mrImport.GetModel(), UNO_QUERY_THROW );
        Reference< XNameAccess > xFamilies( xFamiliesSupp->getStyleFamilies() );

        const OUString aTableFamily(mbWriter ? u"TableStyles" : u"table");
        const OUString aCellFamily(mbWriter ? u"CellStyles" : u"cell");
        Reference< XNameContainer > xTableFamily( xFamilies->getByName( aTableFamily ), UNO_QUERY_THROW );
        Reference< XNameAccess > xCellFamily( xFamilies->getByName( aCellFamily ), UNO_QUERY_THROW );

        Reference< XSingleServiceFactory > xFactory( xTableFamily, UNO_QUERY );
        assert(xFactory.is() != mbWriter);
        Reference< XMultiServiceFactory > xMultiFactory( mrImport.GetModel(), UNO_QUERY_THROW );

        for( const auto& rTemplate : maTableTemplates ) try
        {
            const OUString sTemplateName( rTemplate.first );
            Reference< XNameReplace > xTemplate(xFactory ? xFactory->createInstance() :
                xMultiFactory->createInstance(u"com.sun.star.style.TableStyle"_ustr), UNO_QUERY_THROW);

            std::shared_ptr< XMLTableTemplate > xT( rTemplate.second );

            for( const auto& rStyle : *xT ) try
            {
                const OUString sPropName( rStyle.first );
                const OUString sStyleName( mrImport.GetStyleDisplayName(XmlStyleFamily::TABLE_CELL, rStyle.second) );
                xTemplate->replaceByName( sPropName, xCellFamily->getByName( sStyleName ) );
            }
            catch( Exception& )
            {
                TOOLS_WARN_EXCEPTION("xmloff.table", "");
            }

            if( xTemplate.is() )
            {
                if( xTableFamily->hasByName( sTemplateName ) )
                    xTableFamily->replaceByName( sTemplateName, Any( xTemplate ) );
                else
                    xTableFamily->insertByName( sTemplateName, Any( xTemplate ) );
            }

        }
        catch( Exception& )
        {
            TOOLS_WARN_EXCEPTION("xmloff.table", "");
        }
    }
    catch( Exception& )
    {
        TOOLS_WARN_EXCEPTION("xmloff.table", "");
    }
}


XMLTableImportContext::XMLTableImportContext( const rtl::Reference< XMLTableImport >& xImporter, Reference< XColumnRowRange > const & xColumnRowRange )
: SvXMLImportContext( xImporter->mrImport )
, mxTable( xColumnRowRange, UNO_QUERY )
, mxColumns( xColumnRowRange->getColumns() )
, mxRows( xColumnRowRange->getRows() )
, mnCurrentRow( -1 )
, mnCurrentColumn( -1 )
{
}

SvXMLImportContextRef XMLTableImportContext::ImportColumn( const Reference< XFastAttributeList >& xAttrList )
{
    if( mxColumns.is() && (mnCurrentRow == -1) ) try
    {
        auto xInfo = std::make_shared<ColumnInfo>();

        sal_Int32 nRepeated = 1;

        // read attributes for the table-column
        for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT(TABLE, XML_NUMBER_COLUMNS_REPEATED):
                    nRepeated = aIter.toInt32();
                    break;
                case XML_ELEMENT(TABLE, XML_STYLE_NAME):
                    xInfo->msStyleName = aIter.toString();
                    break;
                case XML_ELEMENT(TABLE, XML_DEFAULT_CELL_STYLE_NAME):
                    xInfo->msDefaultCellStyleName = aIter.toString();
                    break;
                case XML_ELEMENT(XML, XML_ID):
                    //FIXME: TODO
                    break;
            }
        }

        if( nRepeated <= 1 )
        {
            maColumnInfos.push_back( xInfo );
        }
        else
        {
            maColumnInfos.insert( maColumnInfos.end(), nRepeated, xInfo );
        }
    }
    catch( Exception& )
    {
        TOOLS_WARN_EXCEPTION("xmloff.table", "");
    }

    return nullptr;
}

void XMLTableImportContext::InitColumns()
{
    if( !mxColumns.is() )
        return;

    try
    {
        const sal_Int32 nCount1 = mxColumns->getCount();
        const sal_Int32 nCount2 = sal::static_int_cast< sal_Int32 >( maColumnInfos.size() );
        if( nCount1 < nCount2 )
            mxColumns->insertByIndex( nCount1, nCount2 - nCount1 );

        SvXMLStylesContext * pAutoStyles = GetImport().GetShapeImport()->GetAutoStylesContext();

        for( sal_Int32 nCol = 0; nCol < nCount2; nCol++ )
        {
            std::shared_ptr< ColumnInfo > xInfo( maColumnInfos[nCol] );

            if( pAutoStyles && !xInfo->msStyleName.isEmpty() )
            {
                const XMLPropStyleContext* pStyle =
                    dynamic_cast< const XMLPropStyleContext* >(
                        pAutoStyles->FindStyleChildContext(XmlStyleFamily::TABLE_COLUMN, xInfo->msStyleName) );

                if( pStyle )
                {
                    Reference< XPropertySet > xColProps( mxColumns->getByIndex(nCol), UNO_QUERY_THROW );
                    const_cast< XMLPropStyleContext* >( pStyle )->FillPropertySet( xColProps );
                }
            }

        }
    }
    catch( Exception& )
    {
        TOOLS_WARN_EXCEPTION("xmloff.table", "");
    }
}

SvXMLImportContext * XMLTableImportContext::ImportRow( const Reference< XFastAttributeList >& xAttrList )
{
    if( mxRows.is() )
    {
        mnCurrentRow++;
        if( mnCurrentRow == 0 )
            InitColumns();      // first init columns

        mnCurrentColumn = -1;

        const sal_Int32 nRowCount = mxRows->getCount();
        if( ( nRowCount - 1) < mnCurrentRow )
        {
            const sal_Int32 nCount = mnCurrentRow - nRowCount + 1;
            mxRows->insertByIndex( nRowCount, nCount );
        }

        Reference< XPropertySet > xRowSet( mxRows->getByIndex(mnCurrentRow), UNO_QUERY );

        OUString sStyleName;

        // read attributes for the table-row
        for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
        {
            switch(aIter.getToken())
            {
                case XML_ELEMENT(TABLE, XML_STYLE_NAME):
                    sStyleName = aIter.toString();
                    break;
                case XML_ELEMENT(TABLE, XML_DEFAULT_CELL_STYLE_NAME):
                    msDefaultCellStyleName = aIter.toString();
                    break;
                case XML_ELEMENT(XML, XML_ID):
                    //FIXME: TODO
                    break;
            }
        }

        if( !sStyleName.isEmpty() )
        {
            SvXMLStylesContext * pAutoStyles = GetImport().GetShapeImport()->GetAutoStylesContext();
            if( pAutoStyles )
            {
                const XMLPropStyleContext* pStyle =
                    dynamic_cast< const XMLPropStyleContext* >(
                        pAutoStyles->FindStyleChildContext(XmlStyleFamily::TABLE_ROW, sStyleName) );

                if( pStyle )
                {
                    const_cast< XMLPropStyleContext* >( pStyle )->FillPropertySet( xRowSet );
                }
            }
        }
    }

    return new XMLProxyContext( GetImport(), SvXMLImportContextRef(this) );
}

SvXMLImportContextRef XMLTableImportContext::ImportCell( sal_Int32 nElement, const Reference< XFastAttributeList >& xAttrList )
{
    mnCurrentColumn++;
    if( mxColumns.is() ) try
    {
        if( mxColumns->getCount() <= mnCurrentColumn )
            mxColumns->insertByIndex( mxColumns->getCount(), mnCurrentColumn - mxColumns->getCount() + 1 );

        Reference< XMergeableCell > xCell( mxTable->getCellByPosition( mnCurrentColumn, mnCurrentRow ), UNO_QUERY_THROW );
        XMLCellImportContext* pCellContext = new XMLCellImportContext( GetImport(), xCell, GetDefaultCellStyleName(), nElement, xAttrList );

        const sal_Int32 nColumnSpan = pCellContext->getColumnSpan();
        const sal_Int32 nRowSpan = pCellContext->getRowSpan();
        if( (nColumnSpan > 1) || (nRowSpan > 1) )
            maMergeInfos.push_back( std::make_shared< MergeInfo >( mnCurrentColumn, mnCurrentRow, nColumnSpan, nRowSpan ) );

        const sal_Int32 nRepeated = pCellContext->getRepeated();
        if( nRepeated > 1 )
        {
            OSL_FAIL("xmloff::XMLTableImportContext::ImportCell(), import of repeated Cells not implemented (TODO)");
            mnCurrentColumn  += nRepeated - 1;
        }

        return pCellContext;
    }
    catch( Exception& )
    {
        TOOLS_WARN_EXCEPTION("xmloff.table", "");
    }

    return nullptr;
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLTableImportContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    switch (nElement)
    {
        case XML_ELEMENT(TABLE, XML_TABLE_CELL):
        case XML_ELEMENT(TABLE, XML_COVERED_TABLE_CELL):
            return ImportCell( nElement, xAttrList );
        case XML_ELEMENT(TABLE, XML_TABLE_COLUMN):
            return ImportColumn( xAttrList );
        case XML_ELEMENT(TABLE, XML_TABLE_ROW):
            return ImportRow( xAttrList );
        case XML_ELEMENT(TABLE, XML_TABLE_COLUMNS):
        case XML_ELEMENT(TABLE, XML_TABLE_ROWS):
        {
            SvXMLImportContextRef xThis( this );
            return new XMLProxyContext( GetImport(), xThis );
        }
    }
    SAL_WARN("xmloff", "unknown element");
    return nullptr;
}

void XMLTableImportContext::endFastElement(sal_Int32 )
{
    for( const std::shared_ptr< MergeInfo >& xInfo : maMergeInfos )
    {
        if( xInfo ) try
        {
            Reference< XCellRange > xRange( mxTable->getCellRangeByPosition( xInfo->mnStartColumn, xInfo->mnStartRow, xInfo->mnEndColumn, xInfo->mnEndRow ) );
            Reference< XMergeableCellRange > xCursor( mxTable->createCursorByRange( xRange ), UNO_QUERY_THROW );
            xCursor->merge();
        }
        catch( Exception& )
        {
            TOOLS_WARN_EXCEPTION("xmloff.table", "");
        }
    }
}

OUString XMLTableImportContext::GetDefaultCellStyleName() const
{
    OUString sStyleName( msDefaultCellStyleName );

    // if there is still no style name, try default style name from column
    if( (sStyleName.isEmpty()) && (mnCurrentColumn < sal::static_int_cast<sal_Int32>(maColumnInfos.size())) )
        sStyleName = maColumnInfos[mnCurrentColumn]->msDefaultCellStyleName;

    return sStyleName;
}

// XMLCellImportContext

XMLCellImportContext::XMLCellImportContext( SvXMLImport& rImport,
    const Reference< XMergeableCell >& xCell,
    const OUString& sDefaultCellStyleName,
    sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
: SvXMLImportContext( rImport )
, mxCell( xCell )
, mbListContextPushed( false )
, mnColSpan( 1 )
, mnRowSpan( 1 )
, mnRepeated( 1 )
{
    OUString sStyleName;

    // read attributes for the table-cell
    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        switch (aIter.getToken())
        {
            case XML_ELEMENT(TABLE, XML_NUMBER_COLUMNS_REPEATED):
                mnRepeated = aIter.toInt32();
                break;
            case XML_ELEMENT(TABLE, XML_NUMBER_COLUMNS_SPANNED):
                mnColSpan = aIter.toInt32();
                break;
            case XML_ELEMENT(TABLE, XML_NUMBER_ROWS_SPANNED):
                mnRowSpan = aIter.toInt32();
                break;
            case XML_ELEMENT(TABLE, XML_STYLE_NAME):
                sStyleName = aIter.toString();
                break;
            case XML_ELEMENT(XML, XML_ID):
//FIXME: TODO
                break;
//FIXME: RDFa (table:table-cell)
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }

    // if there is no style name at the cell, try default style name from row
    if( sStyleName.isEmpty() )
        sStyleName = sDefaultCellStyleName;

    if( sStyleName.isEmpty() )
        return;

    SvXMLStylesContext * pAutoStyles = GetImport().GetShapeImport()->GetAutoStylesContext();
    if( !pAutoStyles )
        return;

    const XMLPropStyleContext* pStyle =
        dynamic_cast< const XMLPropStyleContext* >(
            pAutoStyles->FindStyleChildContext(XmlStyleFamily::TABLE_CELL, sStyleName) );

    if( pStyle )
    {
        Reference< XPropertySet > xCellSet( mxCell, UNO_QUERY );
        if( xCellSet.is() )
            const_cast< XMLPropStyleContext* >( pStyle )->FillPropertySet( xCellSet );
    }
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLCellImportContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    // create text cursor on demand
    if( !mxCursor.is() )
    {
        Reference< XText > xText( mxCell, UNO_QUERY );
        if( xText.is() )
        {
            rtl::Reference < XMLTextImportHelper > xTxtImport( GetImport().GetTextImport() );
            mxOldCursor = xTxtImport->GetCursor();
            mxCursor = xText->createTextCursor();
            if( mxCursor.is() )
                xTxtImport->SetCursor( mxCursor );

            // remember old list item and block (#91964#) and reset them
            // for the text frame
            xTxtImport->PushListContext();
            mbListContextPushed = true;
        }
    }

    SvXMLImportContext * pContext = nullptr;

    // if we have a text cursor, lets  try to import some text
    if( mxCursor.is() )
    {
        pContext = GetImport().GetTextImport()->CreateTextChildContext( GetImport(), nElement, xAttrList );
    }

    if (!pContext)
        XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);
    return pContext;
}

void XMLCellImportContext::endFastElement(sal_Int32 )
{
    if(mxCursor.is())
    {
        // delete addition newline
        mxCursor->gotoEnd( false );
        mxCursor->goLeft( 1, true );
        mxCursor->setString( u""_ustr );

        // reset cursor
        GetImport().GetTextImport()->ResetCursor();
    }

    if(mxOldCursor.is())
        GetImport().GetTextImport()->SetCursor( mxOldCursor );

    // reinstall old list item (if necessary) #91964#
    if (mbListContextPushed) {
        GetImport().GetTextImport()->PopListContext();
    }
}


XMLTableTemplateContext::XMLTableTemplateContext( SvXMLImport& rImport )
: SvXMLStyleContext( rImport, XmlStyleFamily::TABLE_TEMPLATE_ID, false )
{
}

void XMLTableTemplateContext::SetAttribute( sal_Int32 nElement,
                               const OUString& rValue )
{
    if( nElement == XML_ELEMENT(TEXT, XML_STYLE_NAME)
        // Writer specific: according to oasis odf 1.2 prefix should be "table" and element name should be "name"
        || nElement == XML_ELEMENT(TABLE, XML_NAME) )
    {
        msTemplateStyleName = rValue;
    }
}

void XMLTableTemplateContext::endFastElement(sal_Int32 )
{
    rtl::Reference< XMLTableImport > xTableImport( GetImport().GetShapeImport()->GetShapeTableImport() );
    if( xTableImport.is() )
        xTableImport->addTableTemplate( msTemplateStyleName, maTableTemplate );
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLTableTemplateContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    if( IsTokenInNamespace(nElement, XML_NAMESPACE_TABLE) )
    {
        const TableStyleElement* pElements = getTableStyleMap();
        sal_Int32 nLocalName = nElement & TOKEN_MASK;
        while( (pElements->meElement != XML_TOKEN_END) && pElements->meElement != nLocalName)
            pElements++;

        if( pElements->meElement != XML_TOKEN_END )
        {
            for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
            {
                switch (aIter.getToken())
                {
                    case XML_ELEMENT(TEXT, XML_STYLE_NAME):
                    case XML_ELEMENT(TABLE, XML_STYLE_NAME):
                        maTableTemplate[pElements->msStyleName] = aIter.toString();
                        break;
                    default:
                        XMLOFF_WARN_UNKNOWN("xmloff", aIter);
                }
            }
        }
    } else if (IsTokenInNamespace(nElement, XML_NAMESPACE_LO_EXT)) // Writer specific cell styles
    {
        const TableStyleElement* pElements = getWriterSpecificTableStyleMap();
        sal_Int32 nLocalName = nElement & TOKEN_MASK;
        while( (pElements->meElement != XML_TOKEN_END) && pElements->meElement != nLocalName)
            pElements++;

        if (pElements->meElement != XML_TOKEN_END)
        {
            for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
            {
                switch (aIter.getToken())
                {
                    case XML_ELEMENT(TEXT, XML_STYLE_NAME):
                    case XML_ELEMENT(TABLE, XML_STYLE_NAME):
                        maTableTemplate[pElements->msStyleName] = aIter.toString();
                        break;
                    default:
                        XMLOFF_WARN_UNKNOWN("xmloff", aIter);
                }
            }
        }
    }

    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
