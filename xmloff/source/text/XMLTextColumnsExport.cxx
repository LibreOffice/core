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
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlexp.hxx>
#include "XMLTextColumnsExport.hxx"

using namespace ::com::sun::star::style;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::xmloff::token;


XMLTextColumnsExport::XMLTextColumnsExport( SvXMLExport& rExp ) :
    rExport( rExp ),
    sSeparatorLineIsOn("SeparatorLineIsOn"),
    sSeparatorLineWidth("SeparatorLineWidth"),
    sSeparatorLineColor("SeparatorLineColor"),
    sSeparatorLineRelativeHeight("SeparatorLineRelativeHeight"),
    sSeparatorLineVerticalAlignment("SeparatorLineVerticalAlignment"),
    sIsAutomatic("IsAutomatic"),
    sAutomaticDistance("AutomaticDistance"),
    sSeparatorLineStyle("SeparatorLineStyle")
{
}

void XMLTextColumnsExport::exportXML( const Any& rAny )
{
    Reference < XTextColumns > xColumns;
    rAny >>= xColumns;

    Sequence < TextColumn > aColumns = xColumns->getColumns();
    const TextColumn *pColumns = aColumns.getArray();
    sal_Int32 nCount = aColumns.getLength();

    OUStringBuffer sValue;
    GetExport().AddAttribute( XML_NAMESPACE_FO, XML_COLUMN_COUNT,
                              OUString::number(nCount ? nCount : 1) );

    // handle 'automatic' columns
    Reference < XPropertySet > xPropSet( xColumns, UNO_QUERY );
    if( xPropSet.is() )
    {
        Any aAny = xPropSet->getPropertyValue( sIsAutomatic );
        if ( *o3tl::doAccess<bool>(aAny) )
        {
            aAny = xPropSet->getPropertyValue( sAutomaticDistance );
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
        Any aAny = xPropSet->getPropertyValue( sSeparatorLineIsOn );
        if( *o3tl::doAccess<bool>(aAny) )
        {
            // style:width
            aAny = xPropSet->getPropertyValue( sSeparatorLineWidth );
            sal_Int32 nWidth = 0;
            aAny >>= nWidth;
            GetExport().GetMM100UnitConverter().convertMeasureToXML( sValue,
                                                                nWidth );
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_WIDTH,
                                      sValue.makeStringAndClear() );

            // style:color
            aAny = xPropSet->getPropertyValue( sSeparatorLineColor );
            sal_Int32 nColor = 0;
            aAny >>= nColor;
            ::sax::Converter::convertColor( sValue, nColor );
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_COLOR,
                                      sValue.makeStringAndClear() );

            // style:height
            aAny = xPropSet->getPropertyValue( sSeparatorLineRelativeHeight );
            sal_Int8 nHeight = 0;
            aAny >>= nHeight;
            ::sax::Converter::convertPercent( sValue, nHeight );
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_HEIGHT,
                                      sValue.makeStringAndClear() );

            // style::style
            aAny = xPropSet->getPropertyValue( sSeparatorLineStyle );
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
            aAny = xPropSet->getPropertyValue( sSeparatorLineVerticalAlignment );
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

    while( nCount-- )
    {
        // style:rel-width
        GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_REL_WIDTH,
                                  OUString::number(pColumns->Width) + "*" );

        // fo:margin-left
        GetExport().GetMM100UnitConverter().convertMeasureToXML( sValue,
                                                       pColumns->LeftMargin );
        GetExport().AddAttribute( XML_NAMESPACE_FO, XML_START_INDENT,
                                       sValue.makeStringAndClear() );

        // fo:margin-right
        GetExport().GetMM100UnitConverter().convertMeasureToXML( sValue,
                                                       pColumns->RightMargin );
        GetExport().AddAttribute( XML_NAMESPACE_FO, XML_END_INDENT,
                                    sValue.makeStringAndClear() );

        // style:column
        SvXMLElementExport aElement( GetExport(), XML_NAMESPACE_STYLE, XML_COLUMN,
                                  true, true );
        pColumns++;
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
