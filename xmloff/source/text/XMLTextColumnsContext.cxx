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
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/text/TextColumn.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <sal/log.hxx>
#include <sax/tools/converter.hxx>
#include <xmloff/xmltkmap.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlement.hxx>
#include <XMLTextColumnsContext.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::beans;
using namespace ::xmloff::token;

namespace {

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

}

SvXMLEnumMapEntry<sal_Int8> const pXML_Sep_Style_Enum[] =
{
    { XML_NONE,          0 },
    { XML_SOLID,         1 },
    { XML_DOTTED,        2 },
    { XML_DASHED,        3 },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry<VerticalAlignment> const pXML_Sep_Align_Enum[] =
{
    { XML_TOP,           VerticalAlignment_TOP   },
    { XML_MIDDLE,        VerticalAlignment_MIDDLE },
    { XML_BOTTOM,        VerticalAlignment_BOTTOM },
    { XML_TOKEN_INVALID, VerticalAlignment(0) }
};

class XMLTextColumnContext_Impl: public SvXMLImportContext
{
    text::TextColumn aColumn;

public:

    XMLTextColumnContext_Impl( SvXMLImport& rImport, sal_Int32 nElement,
                               const uno::Reference<
                                       xml::sax::XFastAttributeList > & xAttrList );

    text::TextColumn& getTextColumn() { return aColumn; }
};


XMLTextColumnContext_Impl::XMLTextColumnContext_Impl(
                               SvXMLImport& rImport, sal_Int32 /*nElement*/,
                               const uno::Reference<
                                       xml::sax::XFastAttributeList > & xAttrList ) :
    SvXMLImportContext( rImport )
{
    aColumn.Width = 0;
    aColumn.LeftMargin = 0;
    aColumn.RightMargin = 0;

    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        const OUString sValue = aIter.toString();
        sal_Int32 nVal;
        switch( aIter.getToken() )
        {
        case XML_ELEMENT(STYLE, XML_REL_WIDTH):
            {
                sal_Int32 nPos = sValue.indexOf( '*' );
                if( nPos != -1 && nPos+1 == sValue.getLength() )
                {
                    if (::sax::Converter::convertNumber(
                                nVal,
                                std::u16string_view(sValue).substr(0, nPos),
                                0, USHRT_MAX))
                        aColumn.Width = nVal;
                }
            }
            break;
        case XML_ELEMENT(FO, XML_START_INDENT):
        case XML_ELEMENT(FO_COMPAT, XML_START_INDENT):
            if( GetImport().GetMM100UnitConverter().
                                convertMeasureToCore( nVal, sValue ) )
                aColumn.LeftMargin = nVal;
            break;
        case XML_ELEMENT(FO, XML_END_INDENT):
        case XML_ELEMENT(FO_COMPAT, XML_END_INDENT):
            if( GetImport().GetMM100UnitConverter().
                                convertMeasureToCore( nVal, sValue ) )
                aColumn.RightMargin = nVal;
            break;
        default:
            SAL_WARN("xmloff", "unknown attribute " << SvXMLImport::getPrefixAndNameFromToken(aIter.getToken()) << "=" << sValue);
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

    XMLTextColumnSepContext_Impl( SvXMLImport& rImport, sal_Int32 nElement,
                               const uno::Reference<
                                       xml::sax::XFastAttributeList > & xAttrList );

    sal_Int32 GetWidth() const { return nWidth; }
    sal_Int32 GetColor() const { return  nColor; }
    sal_Int8 GetHeight() const { return nHeight; }
    sal_Int8 GetStyle() const { return nStyle; }
    VerticalAlignment GetVertAlign() const { return eVertAlign; }
};


XMLTextColumnSepContext_Impl::XMLTextColumnSepContext_Impl(
                               SvXMLImport& rImport, sal_Int32 /*nElement*/,
                               const uno::Reference<
                                       xml::sax::XFastAttributeList > & xAttrList) :
    SvXMLImportContext( rImport ),
    nWidth( 2 ),
    nColor( 0 ),
    nHeight( 100 ),
    nStyle( 1 ),
    eVertAlign( VerticalAlignment_TOP )
{
    for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
    {
        const OUString sValue = aIter.toString();

        sal_Int32 nVal;
        switch( aIter.getToken() )
        {
        case XML_ELEMENT(STYLE,  XML_WIDTH):
            if( GetImport().GetMM100UnitConverter().
                                convertMeasureToCore( nVal, sValue ) )
                nWidth = nVal;
            break;
        case XML_ELEMENT(STYLE, XML_HEIGHT):
            if (::sax::Converter::convertPercent( nVal, sValue ) &&
                 nVal >=1 && nVal <= 100 )
                nHeight = static_cast<sal_Int8>(nVal);
            break;
        case XML_ELEMENT(STYLE, XML_COLOR):
            ::sax::Converter::convertColor( nColor, sValue );
            break;
        case XML_ELEMENT(STYLE, XML_VERTICAL_ALIGN):
            SvXMLUnitConverter::convertEnum( eVertAlign, sValue,
                                             pXML_Sep_Align_Enum );
            break;
        case XML_ELEMENT(STYLE, XML_STYLE):
            SvXMLUnitConverter::convertEnum( nStyle, sValue,
                                             pXML_Sep_Style_Enum );
            break;
        default:
            SAL_WARN("xmloff", "unknown attribute " << SvXMLImport::getPrefixAndNameFromToken(aIter.getToken()) << "=" << sValue);
        }
    }
}

constexpr OUStringLiteral gsSeparatorLineIsOn(u"SeparatorLineIsOn");
constexpr OUStringLiteral gsSeparatorLineWidth(u"SeparatorLineWidth");
constexpr OUStringLiteral gsSeparatorLineColor(u"SeparatorLineColor");
constexpr OUStringLiteral gsSeparatorLineRelativeHeight(u"SeparatorLineRelativeHeight");
constexpr OUStringLiteral gsSeparatorLineVerticalAlignment(u"SeparatorLineVerticalAlignment");
constexpr OUStringLiteral gsAutomaticDistance(u"AutomaticDistance");
constexpr OUStringLiteral gsSeparatorLineStyle(u"SeparatorLineStyle");

XMLTextColumnsContext::XMLTextColumnsContext(
                                SvXMLImport& rImport, sal_Int32 nElement,
                                const Reference< xml::sax::XFastAttributeList >& xAttrList,
                                const XMLPropertyState& rProp,
                                 ::std::vector< XMLPropertyState > &rProps )
:   XMLElementPropertyContext( rImport, nElement, rProp, rProps )
,   nCount( 0 )
,   bAutomatic( false )
,   nAutomaticDistance( 0 )
{
    sal_Int32 nVal;
    for (auto &aIter : sax_fastparser::castToFastAttributeList(xAttrList))
    {
        const OUString sValue = aIter.toString();
        switch(aIter.getToken())
        {
            case XML_ELEMENT(FO, XML_COLUMN_COUNT):
            case XML_ELEMENT(FO_COMPAT, XML_COLUMN_COUNT):
                if(::sax::Converter::convertNumber( nVal, sValue, 0, SHRT_MAX ))
                    nCount = static_cast<sal_Int16>(nVal);
                break;
            case XML_ELEMENT(FO, XML_COLUMN_GAP):
            case XML_ELEMENT(FO_COMPAT, XML_COLUMN_GAP):
            {
                bAutomatic = GetImport().GetMM100UnitConverter().
                    convertMeasureToCore( nAutomaticDistance, sValue );
                break;
            }
            default:
                SAL_WARN("xmloff", "unknown attribute " << SvXMLImport::getPrefixAndNameFromToken(aIter.getToken()) << "=" << sValue);
        }
    }
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLTextColumnsContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    if( nElement == XML_ELEMENT(STYLE, XML_COLUMN) )
    {
        const rtl::Reference<XMLTextColumnContext_Impl> xColumn{
            new XMLTextColumnContext_Impl( GetImport(), nElement, xAttrList )};

        // add new tabstop to array of tabstops
        if( !pColumns )
            pColumns = std::make_unique<XMLTextColumnsArray_Impl>();

        pColumns->push_back( xColumn );

        return xColumn.get();
    }
    else if( nElement == XML_ELEMENT(STYLE, XML_COLUMN_SEP) )
    {
        mxColumnSep.set(
            new XMLTextColumnSepContext_Impl( GetImport(), nElement, xAttrList ));

        return mxColumnSep.get();
    }
    SAL_WARN("xmloff", "unknown element " << SvXMLImport::getPrefixAndNameFromToken(nElement));
    return nullptr;
}

void XMLTextColumnsContext::endFastElement(sal_Int32 nElement )
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
             pColumns->size() == static_cast<sal_uInt16>(nCount) )
    {
        // if we have column descriptions, one per column, and we don't use
        // automatic width, then set the column widths

        sal_Int32 nRelWidth = 0;
        sal_uInt16 nColumnsWithWidth = 0;
        sal_Int16 i;

        for( i = 0; i < nCount; i++ )
        {
            const TextColumn& rColumn =
                (*pColumns)[static_cast<sal_uInt16>(i)]->getTextColumn();
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
                    (*pColumns)[static_cast<sal_uInt16>(i)]->getTextColumn();
                if( rColumn.Width == 0 )
                {
                    rColumn.Width = nColWidth;
                    nRelWidth += rColumn.Width;
                    if( 0 == --nColumnsWithWidth )
                        break;
                }
            }
        }

        Sequence< TextColumn > aColumns( static_cast<sal_Int32>(nCount) );
        TextColumn *pTextColumns = aColumns.getArray();
        for( i=0; i < nCount; i++ )
            *pTextColumns++ = (*pColumns)[static_cast<sal_uInt16>(i)]->getTextColumn();

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

        xPropSet->setPropertyValue( gsSeparatorLineIsOn, Any(bOn) );

        if( mxColumnSep.is() )
        {
            if( mxColumnSep->GetWidth() )
            {
                xPropSet->setPropertyValue( gsSeparatorLineWidth, Any(mxColumnSep->GetWidth()) );
            }
            if( mxColumnSep->GetHeight() )
            {
                xPropSet->setPropertyValue( gsSeparatorLineRelativeHeight,
                                            Any(mxColumnSep->GetHeight()) );
            }
            if ( mxColumnSep->GetStyle() )
            {
                xPropSet->setPropertyValue( gsSeparatorLineStyle, Any(mxColumnSep->GetStyle()) );
            }

            xPropSet->setPropertyValue( gsSeparatorLineColor, Any(mxColumnSep->GetColor()) );

            xPropSet->setPropertyValue( gsSeparatorLineVerticalAlignment, Any(mxColumnSep->GetVertAlign()) );
        }

        // handle 'automatic columns': column distance
        if( bAutomatic )
        {
            xPropSet->setPropertyValue( gsAutomaticDistance, Any(nAutomaticDistance) );
        }
    }

    aProp.maValue <<= xColumns;

    SetInsert( true );
    XMLElementPropertyContext::endFastElement(nElement);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
