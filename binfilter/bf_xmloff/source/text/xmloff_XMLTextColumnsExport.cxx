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




#ifndef _COM_SUN_STAR_TEXT_XTEXTCOLUMNS_HPP_ 
#include <com/sun/star/text/XTextColumns.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_VERTICALALIGNMENT_HPP_ 
#include <com/sun/star/style/VerticalAlignment.hpp>
#endif


#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLUCONV_HXX 
#include "xmluconv.hxx"
#endif
#ifndef _XMLOFF_XMLEXP_HXX 
#include "xmlexp.hxx"
#endif

#ifndef _XMLOFF_XMLTEXTCOLUMNSEXPORT_HXX
#include "XMLTextColumnsExport.hxx"
#endif
namespace binfilter {

using namespace ::com::sun::star::style;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::rtl;
using namespace ::binfilter::xmloff::token;


XMLTextColumnsExport::XMLTextColumnsExport( SvXMLExport& rExp ) :
    rExport( rExp ),
    sSeparatorLineIsOn(RTL_CONSTASCII_USTRINGPARAM("SeparatorLineIsOn")),
    sSeparatorLineWidth(RTL_CONSTASCII_USTRINGPARAM("SeparatorLineWidth")),
    sSeparatorLineColor(RTL_CONSTASCII_USTRINGPARAM("SeparatorLineColor")),
    sSeparatorLineRelativeHeight(RTL_CONSTASCII_USTRINGPARAM("SeparatorLineRelativeHeight")),
    sSeparatorLineVerticalAlignment(RTL_CONSTASCII_USTRINGPARAM("SeparatorLineVerticalAlignment")),
    sIsAutomatic(RTL_CONSTASCII_USTRINGPARAM("IsAutomatic")),
    sAutomaticDistance(RTL_CONSTASCII_USTRINGPARAM("AutomaticDistance"))
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
    GetExport().GetMM100UnitConverter().convertNumber( sValue, nCount );
    GetExport().AddAttribute( XML_NAMESPACE_FO, XML_COLUMN_COUNT,
                              sValue.makeStringAndClear() );

    // handle 'automatic' columns
    Reference < XPropertySet > xPropSet( xColumns, UNO_QUERY );
    if( xPropSet.is() )
    {
        Any aAny = xPropSet->getPropertyValue( sIsAutomatic );
        if ( *(sal_Bool*)aAny.getValue() )
        {
            aAny = xPropSet->getPropertyValue( sAutomaticDistance );
            sal_Int32 nDistance = 0;
            aAny >>= nDistance;
            OUStringBuffer aBuffer;
            GetExport().GetMM100UnitConverter().convertMeasure( 
                aBuffer, nDistance );
            GetExport().AddAttribute( XML_NAMESPACE_FO, 
                                      XML_COLUMN_GAP,
                                      aBuffer.makeStringAndClear() );
        }
    }

    SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE, XML_COLUMNS,
                              sal_True, sal_True );

    if( xPropSet.is() )
    {
        Any aAny = xPropSet->getPropertyValue( sSeparatorLineIsOn );
        if( *(sal_Bool *)aAny.getValue() )
        {
            // style:width
            aAny = xPropSet->getPropertyValue( sSeparatorLineWidth );
            sal_Int32 nWidth;
            aAny >>= nWidth;
            GetExport().GetMM100UnitConverter().convertMeasure( sValue,
                                                                nWidth );
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_WIDTH,
                                      sValue.makeStringAndClear() );

            // style:color
            aAny = xPropSet->getPropertyValue( sSeparatorLineColor );
            sal_Int32 nColor;
            aAny >>= nColor;
            GetExport().GetMM100UnitConverter().convertColor( sValue,
                                                              nColor );
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_COLOR,
                                      sValue.makeStringAndClear() );

            // style:height
            aAny = xPropSet->getPropertyValue( sSeparatorLineRelativeHeight );
            sal_Int8 nHeight;
            aAny >>= nHeight;
            GetExport().GetMM100UnitConverter().convertPercent( sValue,
                                                                nHeight );
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_HEIGHT,
                                      sValue.makeStringAndClear() );

            // style:vertical-align
            aAny = xPropSet->getPropertyValue( sSeparatorLineVerticalAlignment );
            VerticalAlignment eVertAlign;
            aAny >>= eVertAlign;

            enum XMLTokenEnum eStr = XML_TOKEN_INVALID;
            switch( eVertAlign )
            {
//			case VerticalAlignment_TOP: eStr = XML_TOP;
            case VerticalAlignment_MIDDLE: eStr = XML_MIDDLE; break;
            case VerticalAlignment_BOTTOM: eStr = XML_BOTTOM; break;
            }

            if( eStr != XML_TOKEN_INVALID)
                GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                          XML_VERTICAL_ALIGN, eStr );

            // style:column-sep
            SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE,
                                      XML_COLUMN_SEP,
                                      sal_True, sal_True );
        }
    }
    
    while( nCount-- )
    {
        // style:rel-width
        GetExport().GetMM100UnitConverter().convertNumber( sValue,
                                                       pColumns->Width );
        sValue.append( (sal_Unicode)'*' );
        GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_REL_WIDTH,
                                  sValue.makeStringAndClear() );

        // fo:margin-left
        GetExport().GetMM100UnitConverter().convertMeasure( sValue,
                                                       pColumns->LeftMargin );
        GetExport().AddAttribute( XML_NAMESPACE_FO, XML_MARGIN_LEFT,
                                       sValue.makeStringAndClear() );

        // fo:margin-right
        GetExport().GetMM100UnitConverter().convertMeasure( sValue,
                                                       pColumns->RightMargin );
        GetExport().AddAttribute( XML_NAMESPACE_FO, XML_MARGIN_RIGHT,
                                    sValue.makeStringAndClear() );

        // style:column
        SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE, XML_COLUMN,
                                  sal_True, sal_True );
        pColumns++;
    }
}


}//end of namespace binfilter
