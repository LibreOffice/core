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

#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/text/TextColumn.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <o3tl/make_unique.hxx>
#include <sax/tools/converter.hxx>
#include <xmloff/xmltkmap.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmltoken.hxx>
#include "XMLTextColumnsContext.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::beans;
using namespace ::xmloff::token;

enum SvXMLTokenMapAttrs
{
    XML_TOK_COLUMN_WIDTH,
    XML_TOK_COLUMN_MARGIN_LEFT,
    XML_TOK_COLUMN_MARGIN_RIGHT
};

enum SvXMLSepTokenMapAttrs
{
    XML_TOK_COLUMN_SEP_WIDTH,
    XML_TOK_COLUMN_SEP_HEIGHT,
    XML_TOK_COLUMN_SEP_COLOR,
    XML_TOK_COLUMN_SEP_ALIGN,
    XML_TOK_COLUMN_SEP_STYLE
};

static const SvXMLTokenMapEntry aColAttrTokenMap[] =
{
    { XML_NAMESPACE_STYLE,  XML_REL_WIDTH,      XML_TOK_COLUMN_WIDTH },
    { XML_NAMESPACE_FO,     XML_START_INDENT,   XML_TOK_COLUMN_MARGIN_LEFT },
    { XML_NAMESPACE_FO,     XML_END_INDENT,     XML_TOK_COLUMN_MARGIN_RIGHT },
    XML_TOKEN_MAP_END
};

static const SvXMLTokenMapEntry aColSepAttrTokenMap[] =
{
    { XML_NAMESPACE_STYLE,  XML_WIDTH,          XML_TOK_COLUMN_SEP_WIDTH },
    { XML_NAMESPACE_STYLE,  XML_COLOR,          XML_TOK_COLUMN_SEP_COLOR },
    { XML_NAMESPACE_STYLE,  XML_HEIGHT,         XML_TOK_COLUMN_SEP_HEIGHT },
    { XML_NAMESPACE_STYLE,  XML_VERTICAL_ALIGN, XML_TOK_COLUMN_SEP_ALIGN },
    { XML_NAMESPACE_STYLE,  XML_STYLE,          XML_TOK_COLUMN_SEP_STYLE },
    XML_TOKEN_MAP_END
};

static SvXMLEnumMapEntry const pXML_Sep_Style_Enum[] =
{
    { XML_NONE,          0 },
    { XML_SOLID,         1 },
    { XML_DOTTED,        2 },
    { XML_DASHED,        3 },
    { XML_TOKEN_INVALID, 0 }
};

static SvXMLEnumMapEntry const pXML_Sep_Align_Enum[] =
{
    { XML_TOP,          VerticalAlignment_TOP   },
    { XML_MIDDLE,       VerticalAlignment_MIDDLE },
    { XML_BOTTOM,       VerticalAlignment_BOTTOM },
    { XML_TOKEN_INVALID, 0 }
};

class XMLTextColumnContext_Impl: public SvXMLImportContext
{
    text::TextColumn aColumn;

public:

    XMLTextColumnContext_Impl( SvXMLImport& rImport, sal_uInt16 nPrfx,
                               const OUString& rLName,
                               const uno::Reference<
                                       xml::sax::XAttributeList > & xAttrList,
                               const SvXMLTokenMap& rTokenMap );

    text::TextColumn& getTextColumn() { return aColumn; }
};


XMLTextColumnContext_Impl::XMLTextColumnContext_Impl(
                               SvXMLImport& rImport, sal_uInt16 nPrfx,
                               const OUString& rLName,
                               const uno::Reference<
                                       xml::sax::XAttributeList > & xAttrList,
                               const SvXMLTokenMap& rTokenMap ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    aColumn.Width = 0;
    aColumn.LeftMargin = 0;
    aColumn.RightMargin = 0;

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        const OUString& rValue = xAttrList->getValueByIndex( i );

        sal_Int32 nVal;
        switch( rTokenMap.Get( nPrefix, aLocalName ) )
        {
        case XML_TOK_COLUMN_WIDTH:
            {
                sal_Int32 nPos = rValue.indexOf( (sal_Unicode)'*' );
                if( nPos != -1 && nPos+1 == rValue.getLength() )
                {
                    OUString sTmp( rValue.copy( 0, nPos ) );
                    if (::sax::Converter::convertNumber(
                                nVal, sTmp, 0, USHRT_MAX))
                    aColumn.Width = nVal;
                }
            }
            break;
        case XML_TOK_COLUMN_MARGIN_LEFT:
            if( GetImport().GetMM100UnitConverter().
                                convertMeasureToCore( nVal, rValue ) )
                aColumn.LeftMargin = nVal;
            break;
        case XML_TOK_COLUMN_MARGIN_RIGHT:

            if( GetImport().GetMM100UnitConverter().
                                convertMeasureToCore( nVal, rValue ) )
                aColumn.RightMargin = nVal;
            break;
        default:
            break;
        }
    }
}

class XMLTextColumnSepContext_Impl: public SvXMLImportContext
{
    sal_Int32 nWidth;
    sal_Int32 nColor;
    sal_Int8 nHeight;
    sal_Int8 nStyle;
    VerticalAlignment eVertAlign;

public:

    XMLTextColumnSepContext_Impl( SvXMLImport& rImport, sal_uInt16 nPrfx,
                               const OUString& rLName,
                               const uno::Reference<
                                       xml::sax::XAttributeList > & xAttrList,
                               const SvXMLTokenMap& rTokenMap );

    sal_Int32 GetWidth() const { return nWidth; }
    sal_Int32 GetColor() const { return  nColor; }
    sal_Int8 GetHeight() const { return nHeight; }
    sal_Int8 GetStyle() const { return nStyle; }
    VerticalAlignment GetVertAlign() const { return eVertAlign; }
};


XMLTextColumnSepContext_Impl::XMLTextColumnSepContext_Impl(
                               SvXMLImport& rImport, sal_uInt16 nPrfx,
                               const OUString& rLName,
                               const uno::Reference<
                                       xml::sax::XAttributeList > & xAttrList,
                               const SvXMLTokenMap& rTokenMap ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    nWidth( 2 ),
    nColor( 0 ),
    nHeight( 100 ),
    nStyle( 1 ),
    eVertAlign( VerticalAlignment_TOP )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        const OUString& rValue = xAttrList->getValueByIndex( i );

        sal_Int32 nVal;
        switch( rTokenMap.Get( nPrefix, aLocalName ) )
        {
        case XML_TOK_COLUMN_SEP_WIDTH:
            if( GetImport().GetMM100UnitConverter().
                                convertMeasureToCore( nVal, rValue ) )
                nWidth = nVal;
            break;
        case XML_TOK_COLUMN_SEP_HEIGHT:
            if (::sax::Converter::convertPercent( nVal, rValue ) &&
                 nVal >=1 && nVal <= 100 )
                nHeight = (sal_Int8)nVal;
            break;
        case XML_TOK_COLUMN_SEP_COLOR:
            {
                ::sax::Converter::convertColor( nColor, rValue );
            }
            break;
        case XML_TOK_COLUMN_SEP_ALIGN:
            {
                sal_uInt16 nAlign;
                if( SvXMLUnitConverter::convertEnum( nAlign, rValue,
                                                       pXML_Sep_Align_Enum ) )
                    eVertAlign = (VerticalAlignment)nAlign;
            }
            break;
        case XML_TOK_COLUMN_SEP_STYLE:
            {
                sal_uInt16 nStyleVal;
                if( SvXMLUnitConverter::convertEnum( nStyleVal, rValue,
                                                       pXML_Sep_Style_Enum ) )
                    nStyle = (sal_Int8)nStyleVal;
            }
            break;
        }
    }
}

XMLTextColumnsContext::XMLTextColumnsContext(
                                SvXMLImport& rImport, sal_uInt16 nPrfx,
                                const OUString& rLName,
                                const Reference< xml::sax::XAttributeList >&
                                    xAttrList,
                                const XMLPropertyState& rProp,
                                 ::std::vector< XMLPropertyState > &rProps )
:   XMLElementPropertyContext( rImport, nPrfx, rLName, rProp, rProps )
,   sSeparatorLineIsOn("SeparatorLineIsOn")
,   sSeparatorLineWidth("SeparatorLineWidth")
,   sSeparatorLineColor("SeparatorLineColor")
,   sSeparatorLineRelativeHeight("SeparatorLineRelativeHeight")
,   sSeparatorLineVerticalAlignment("SeparatorLineVerticalAlignment")
,   sAutomaticDistance("AutomaticDistance")
,   sSeparatorLineStyle("SeparatorLineStyle")
,   pColumnAttrTokenMap( new SvXMLTokenMap(aColAttrTokenMap) )
,   pColumnSepAttrTokenMap( new SvXMLTokenMap(aColSepAttrTokenMap) )
,   nCount( 0 )
,   bAutomatic( false )
,   nAutomaticDistance( 0 )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    sal_Int32 nVal;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        const OUString& rValue = xAttrList->getValueByIndex( i );
        if( XML_NAMESPACE_FO == nPrefix )
        {
            if( IsXMLToken( aLocalName, XML_COLUMN_COUNT ) &&
                ::sax::Converter::convertNumber( nVal, rValue, 0, SHRT_MAX ))
            {
                nCount = (sal_Int16)nVal;
            }
            else if( IsXMLToken( aLocalName, XML_COLUMN_GAP ) )
            {
                bAutomatic = GetImport().GetMM100UnitConverter().
                    convertMeasureToCore( nAutomaticDistance, rValue );
            }
        }
    }
}

SvXMLImportContext *XMLTextColumnsContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = nullptr;

    if( XML_NAMESPACE_STYLE == nPrefix &&
        IsXMLToken( rLocalName, XML_COLUMN ) )
    {
        const rtl::Reference<XMLTextColumnContext_Impl> xColumn{
            new XMLTextColumnContext_Impl( GetImport(), nPrefix, rLocalName,
                                           xAttrList, *pColumnAttrTokenMap )};

        // add new tabstop to array of tabstops
        if( !pColumns )
            pColumns = o3tl::make_unique<XMLTextColumnsArray_Impl>();

        pColumns->push_back( xColumn );

        pContext = xColumn.get();
    }
    else if( XML_NAMESPACE_STYLE == nPrefix &&
             IsXMLToken( rLocalName, XML_COLUMN_SEP ) )
    {
        mxColumnSep.set(
            new XMLTextColumnSepContext_Impl( GetImport(), nPrefix, rLocalName,
                                           xAttrList, *pColumnSepAttrTokenMap ));

        pContext = mxColumnSep.get();
    }
    else
    {
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    }

    return pContext;
}

void XMLTextColumnsContext::EndElement( )
{
    Reference<XMultiServiceFactory> xFactory(GetImport().GetModel(),UNO_QUERY);
    if( !xFactory.is() )
        return;

    Reference<XInterface> xIfc = xFactory->createInstance("com.sun.star.text.TextColumns");
    if( !xIfc.is() )
        return;

    Reference< XTextColumns > xColumns( xIfc, UNO_QUERY );
    if ( 0 == nCount )
    {
        // zero columns = no columns -> 1 column
        xColumns->setColumnCount( 1 );
    }
    else if( !bAutomatic && pColumns &&
             pColumns->size() == (sal_uInt16)nCount )
    {
        // if we have column descriptions, one per column, and we don't use
        // automatic width, then set the column widths

        sal_Int32 nRelWidth = 0;
        sal_uInt16 nColumnsWithWidth = 0;
        sal_Int16 i;

        for( i = 0; i < nCount; i++ )
        {
            const TextColumn& rColumn =
                (*pColumns)[(sal_uInt16)i]->getTextColumn();
            if( rColumn.Width > 0 )
            {
                nRelWidth += rColumn.Width;
                nColumnsWithWidth++;
            }
        }
        if( nColumnsWithWidth < nCount )
        {
            sal_Int32 nColWidth = 0==nRelWidth
                                        ? USHRT_MAX / nCount
                                        : nRelWidth / nColumnsWithWidth;

            for( i=0; i < nCount; i++ )
            {
                TextColumn& rColumn =
                    (*pColumns)[(sal_uInt16)i]->getTextColumn();
                if( rColumn.Width == 0 )
                {
                    rColumn.Width = nColWidth;
                    nRelWidth += rColumn.Width;
                    if( 0 == --nColumnsWithWidth )
                        break;
                }
            }
        }

        Sequence< TextColumn > aColumns( (sal_Int32)nCount );
        TextColumn *pTextColumns = aColumns.getArray();
        for( i=0; i < nCount; i++ )
            *pTextColumns++ = (*pColumns)[(sal_uInt16)i]->getTextColumn();

        xColumns->setColumns( aColumns );
    }
    else
    {
        // only set column count (and let the columns be distributed
        // automatically)

        xColumns->setColumnCount( nCount );
    }

    Reference < XPropertySet > xPropSet( xColumns, UNO_QUERY );
    if( xPropSet.is() )
    {
        bool bOn = mxColumnSep != nullptr;

        xPropSet->setPropertyValue( sSeparatorLineIsOn, Any(bOn) );

        if( mxColumnSep.is() )
        {
            if( mxColumnSep->GetWidth() )
            {
                xPropSet->setPropertyValue( sSeparatorLineWidth, Any(mxColumnSep->GetWidth()) );
            }
            if( mxColumnSep->GetHeight() )
            {
                xPropSet->setPropertyValue( sSeparatorLineRelativeHeight,
                                            Any(mxColumnSep->GetHeight()) );
            }
            if ( mxColumnSep->GetStyle() )
            {
                xPropSet->setPropertyValue( sSeparatorLineStyle, Any(mxColumnSep->GetStyle()) );
            }

            xPropSet->setPropertyValue( sSeparatorLineColor, Any(mxColumnSep->GetColor()) );

            xPropSet->setPropertyValue( sSeparatorLineVerticalAlignment, Any(mxColumnSep->GetVertAlign()) );
        }

        // handle 'automatic columns': column distance
        if( bAutomatic )
        {
            xPropSet->setPropertyValue( sAutomaticDistance, Any(nAutomaticDistance) );
        }
    }

    aProp.maValue <<= xColumns;

    SetInsert( true );
    XMLElementPropertyContext::EndElement();

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
