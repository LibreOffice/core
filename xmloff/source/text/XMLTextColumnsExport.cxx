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

#include <o3tl/any.hxx>
#include <rtl/ustrbuf.hxx>


#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/text/TextColumn.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <sax/tools/converter.hxx>

#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlexp.hxx>
#include <XMLTextColumnsExport.hxx>

using namespace ::com::sun::star::style;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::xmloff::token;


const OUStringLiteral gsSeparatorLineIsOn(u"SeparatorLineIsOn");
const OUStringLiteral gsSeparatorLineWidth(u"SeparatorLineWidth");
const OUStringLiteral gsSeparatorLineColor(u"SeparatorLineColor");
const OUStringLiteral gsSeparatorLineRelativeHeight(u"SeparatorLineRelativeHeight");
const OUStringLiteral gsSeparatorLineVerticalAlignment(u"SeparatorLineVerticalAlignment");
const OUStringLiteral gsIsAutomatic(u"IsAutomatic");
const OUStringLiteral gsAutomaticDistance(u"AutomaticDistance");
const OUStringLiteral gsSeparatorLineStyle(u"SeparatorLineStyle");

XMLTextColumnsExport::XMLTextColumnsExport( SvXMLExport& rExp ) :
    rExport( rExp )
{
}

void XMLTextColumnsExport::exportXML( const Any& rAny )
{
    Reference < XTextColumns > xColumns;
    rAny >>= xColumns;

    const Sequence < TextColumn > aColumns = xColumns->getColumns();
    sal_Int32 nCount = aColumns.getLength();

    OUStringBuffer sValue;
    GetExport().AddAttribute( XML_NAMESPACE_FO, XML_COLUMN_COUNT,
                              OUString::number(nCount ? nCount : 1) );

    // handle 'automatic' columns
    Reference < XPropertySet > xPropSet( xColumns, UNO_QUERY );
    if( xPropSet.is() )
    {
        Any aAny = xPropSet->getPropertyValue( gsIsAutomatic );
        if ( *o3tl::doAccess<bool>(aAny) )
        {
            aAny = xPropSet->getPropertyValue( gsAutomaticDistance );
            sal_Int32 nDistance = 0;
            aAny >>= nDistance;
            OUStringBuffer aBuffer;
            GetExport().GetMM100UnitConverter().convertMeasureToXML(
                aBuffer, nDistance );
            GetExport().AddAttribute( XML_NAMESPACE_FO,
                                      XML_COLUMN_GAP,
                                      aBuffer.makeStringAndClear() );
        }
    }

    SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE, XML_COLUMNS,
                              true, true );

    if( xPropSet.is() )
    {
        Any aAny = xPropSet->getPropertyValue( gsSeparatorLineIsOn );
        if( *o3tl::doAccess<bool>(aAny) )
        {
            // style:width
            aAny = xPropSet->getPropertyValue( gsSeparatorLineWidth );
            sal_Int32 nWidth = 0;
            aAny >>= nWidth;
            GetExport().GetMM100UnitConverter().convertMeasureToXML( sValue,
                                                                nWidth );
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_WIDTH,
                                      sValue.makeStringAndClear() );

            // style:color
            aAny = xPropSet->getPropertyValue( gsSeparatorLineColor );
            sal_Int32 nColor = 0;
            aAny >>= nColor;
            ::sax::Converter::convertColor( sValue, nColor );
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_COLOR,
                                      sValue.makeStringAndClear() );

            // style:height
            aAny = xPropSet->getPropertyValue( gsSeparatorLineRelativeHeight );
            sal_Int8 nHeight = 0;
            aAny >>= nHeight;
            ::sax::Converter::convertPercent( sValue, nHeight );
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_HEIGHT,
                                      sValue.makeStringAndClear() );

            // style::style
            aAny = xPropSet->getPropertyValue( gsSeparatorLineStyle );
            sal_Int8 nStyle = 0;
            aAny >>= nStyle;

            enum XMLTokenEnum eStr = XML_TOKEN_INVALID;
            switch ( nStyle )
            {
                case 0:  eStr = XML_NONE; break;
                case 1:  eStr = XML_SOLID; break;
                case 2:  eStr = XML_DOTTED; break;
                case 3:  eStr = XML_DASHED; break;
                default:
                         break;
            }
            if ( eStr != XML_TOKEN_INVALID )
                GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_STYLE, eStr );

            // style:vertical-align
            aAny = xPropSet->getPropertyValue( gsSeparatorLineVerticalAlignment );
            VerticalAlignment eVertAlign;
            aAny >>= eVertAlign;

            eStr = XML_TOKEN_INVALID;
            switch( eVertAlign )
            {
//          case VerticalAlignment_TOP: eStr = XML_TOP;
            case VerticalAlignment_MIDDLE: eStr = XML_MIDDLE; break;
            case VerticalAlignment_BOTTOM: eStr = XML_BOTTOM; break;
            default:
                break;
            }

            if( eStr != XML_TOKEN_INVALID)
                GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                          XML_VERTICAL_ALIGN, eStr );

            // style:column-sep
            SvXMLElementExport aElement( GetExport(), XML_NAMESPACE_STYLE,
                                      XML_COLUMN_SEP,
                                      true, true );
        }
    }

    for (const auto& rColumn : aColumns)
    {
        // style:rel-width
        GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_REL_WIDTH,
                                  OUString::number(rColumn.Width) + "*" );

        // fo:margin-left
        GetExport().GetMM100UnitConverter().convertMeasureToXML( sValue,
                                                       rColumn.LeftMargin );
        GetExport().AddAttribute( XML_NAMESPACE_FO, XML_START_INDENT,
                                       sValue.makeStringAndClear() );

        // fo:margin-right
        GetExport().GetMM100UnitConverter().convertMeasureToXML( sValue,
                                                       rColumn.RightMargin );
        GetExport().AddAttribute( XML_NAMESPACE_FO, XML_END_INDENT,
                                    sValue.makeStringAndClear() );

        // style:column
        SvXMLElementExport aElement( GetExport(), XML_NAMESPACE_STYLE, XML_COLUMN,
                                  true, true );
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
