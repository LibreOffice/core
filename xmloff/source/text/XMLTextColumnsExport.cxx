/*************************************************************************
 *
 *  $RCSfile: XMLTextColumnsExport.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mib $ $Date: 2000-10-30 12:47:14 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif


#ifndef _COM_SUN_STAR_TEXT_XTEXTCOLUMNS_HPP_
#include <com/sun/star/text/XTextColumns.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_TEXTCOLUMN_HPP_
#include <com/sun/star/text/TextColumn.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_VERTICALALIGNMENT_HPP_
#include <com/sun/star/style/VerticalAlignment.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif


#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
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

using namespace ::com::sun::star::style;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::rtl;


XMLTextColumnsExport::XMLTextColumnsExport( SvXMLExport& rExp ) :
    rExport( rExp ),
    sSeparatorLineIsOn(RTL_CONSTASCII_USTRINGPARAM("SeparatorLineIsOn")),
    sSeparatorLineWidth(RTL_CONSTASCII_USTRINGPARAM("SeparatorLineWidth")),
    sSeparatorLineColor(RTL_CONSTASCII_USTRINGPARAM("SeparatorLineColor")),
    sSeparatorLineRelativeHeight(RTL_CONSTASCII_USTRINGPARAM("SeparatorLineRelativeHeight")),
    sSeparatorLineVerticalAlignment(RTL_CONSTASCII_USTRINGPARAM("SeparatorLineVerticalAlignment"))
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
    GetExport().AddAttribute( XML_NAMESPACE_FO, sXML_column_count,
                              sValue.makeStringAndClear() );
    SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE, sXML_columns,
                              sal_True, sal_True );

    Reference < XPropertySet > xPropSet( xColumns, UNO_QUERY );
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
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, sXML_width,
                                      sValue.makeStringAndClear() );

            // style:color
            aAny = xPropSet->getPropertyValue( sSeparatorLineColor );
            sal_Int32 nColor;
            aAny >>= nColor;
            GetExport().GetMM100UnitConverter().convertColor( sValue,
                                                              nColor );
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, sXML_color,
                                      sValue.makeStringAndClear() );

            // style:height
            aAny = xPropSet->getPropertyValue( sSeparatorLineRelativeHeight );
            sal_Int8 nHeight;
            aAny >>= nHeight;
            GetExport().GetMM100UnitConverter().convertPercent( sValue,
                                                                nHeight );
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, sXML_height,
                                      sValue.makeStringAndClear() );

            // style:vertical-align
            aAny = xPropSet->getPropertyValue( sSeparatorLineVerticalAlignment );
            VerticalAlignment eVertAlign;
            aAny >>= eVertAlign;

            sal_Char *pStr = 0;
            switch( eVertAlign )
            {
//          case VerticalAlignment_TOP: pStr = sXML_top;
            case VerticalAlignment_MIDDLE: pStr = sXML_middle; break;
            case VerticalAlignment_BOTTOM: pStr = sXML_bottom; break;
            }

            if( pStr )
                GetExport().AddAttributeASCII( XML_NAMESPACE_STYLE,
                                               sXML_vertical_align, pStr );

            // style:column-sep
            SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE,
                                      sXML_column_sep,
                                      sal_True, sal_True );
        }
    }

    while( nCount-- )
    {
        // style:rel-width
        GetExport().GetMM100UnitConverter().convertNumber( sValue,
                                                       pColumns->Width );
        sValue.append( (sal_Unicode)'*' );
        GetExport().AddAttribute( XML_NAMESPACE_STYLE, sXML_rel_width,
                                  sValue.makeStringAndClear() );

        // fo:margin-left
        GetExport().GetMM100UnitConverter().convertMeasure( sValue,
                                                       pColumns->LeftMargin );
        GetExport().AddAttribute( XML_NAMESPACE_FO, sXML_margin_left,
                                       sValue.makeStringAndClear() );

        // fo:margin-right
        GetExport().GetMM100UnitConverter().convertMeasure( sValue,
                                                       pColumns->RightMargin );
        GetExport().AddAttribute( XML_NAMESPACE_FO, sXML_margin_right,
                                    sValue.makeStringAndClear() );

        // style:column
        SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE, sXML_column,
                                  sal_True, sal_True );
        pColumns++;
    }
}


