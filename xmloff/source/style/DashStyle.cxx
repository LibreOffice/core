/*************************************************************************
 *
 *  $RCSfile: DashStyle.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: cl $ $Date: 2000-11-28 14:25:18 $
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

#ifndef _COM_SUN_STAR_DRAWING_DASHSTYLE_HPP_
#include <com/sun/star/drawing/DashStyle.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_LINEDASH_HPP_
#include <com/sun/star/drawing/LineDash.hpp>
#endif

#ifndef _XMLOFF_DASHSTYLE_HXX
#include "DashStyle.hxx"
#endif

#ifndef _XMLOFF_ATTRLIST_HXX
#include "attrlist.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include"xmluconv.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include"xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif

#ifndef _RTL_USTRBUF_HXX_
#include<rtl/ustrbuf.hxx>
#endif

#ifndef _RTL_USTRING_
#include<rtl/ustring>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _XMLOFF_XMLTKMAP_HXX
#include "xmltkmap.hxx"
#endif

using namespace ::com::sun::star;
using namespace ::rtl;

enum SvXMLTokenMapAttrs
{
    XML_TOK_DASH_NAME,
    XML_TOK_DASH_STYLE,
    XML_TOK_DASH_DOTS1,
    XML_TOK_DASH_DOTS1LEN,
    XML_TOK_DASH_DOTS2,
    XML_TOK_DASH_DOTS2LEN,
    XML_TOK_DASH_DISTANCE,
    XML_TOK_DASH_END=XML_TOK_UNKNOWN
};

static __FAR_DATA SvXMLTokenMapEntry aDashStyleAttrTokenMap[] =
{
    { XML_NAMESPACE_DRAW, sXML_name,            XML_TOK_DASH_NAME },
    { XML_NAMESPACE_DRAW, sXML_style,           XML_TOK_DASH_STYLE },
    { XML_NAMESPACE_DRAW, sXML_dots1,           XML_TOK_DASH_DOTS1 },
    { XML_NAMESPACE_DRAW, sXML_dots1_length,    XML_TOK_DASH_DOTS1LEN },
    { XML_NAMESPACE_DRAW, sXML_dots2,           XML_TOK_DASH_DOTS2 },
    { XML_NAMESPACE_DRAW, sXML_dots2_length,    XML_TOK_DASH_DOTS2LEN },
    { XML_NAMESPACE_DRAW, sXML_distance,        XML_TOK_DASH_DISTANCE },
    XML_TOKEN_MAP_END
};

SvXMLEnumMapEntry __READONLY_DATA pXML_DashStyle_Enum[] =
{
    { sXML_rect,        drawing::DashStyle_RECT },
    { sXML_round,       drawing::DashStyle_ROUND },
    { sXML_rect,        drawing::DashStyle_RECTRELATIVE },
    { sXML_round,       drawing::DashStyle_ROUNDRELATIVE },
    { 0, 0 }
};

XMLDashStyle::XMLDashStyle( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > * _pHandler,
                                        const SvXMLNamespaceMap& _rNamespaceMap, const SvXMLUnitConverter& _rUnitConverter )
: mpHandler      ( _pHandler ),
  mrNamespaceMap ( _rNamespaceMap ),
  mrUnitConverter( _rUnitConverter ),
  mpAttrList     ( NULL )
{
}

XMLDashStyle::~XMLDashStyle()
{
}

void XMLDashStyle::AddAttribute( sal_uInt16 nPrefix, const sal_Char *pName, const OUString& rStrValue )
{
    const OUString aStrName( OUString::createFromAscii( pName ) );
    const OUString aStrCDATA( OUString::createFromAscii( sXML_CDATA ) );

    mpAttrList->AddAttribute( mrNamespaceMap.GetQNameByKey( nPrefix, aStrName ), aStrCDATA, rStrValue );
}

sal_Bool XMLDashStyle::exportXML( const OUString& rStrName, const ::com::sun::star::uno::Any& rValue )
{
    return ImpExportXML( *mpHandler, mrNamespaceMap, mrUnitConverter, rStrName, rValue );
}

sal_Bool XMLDashStyle::importXML( const uno::Reference< xml::sax::XAttributeList >& xAttrList, uno::Any& rValue, OUString& rStrName )
{
    return ImpImportXML( mrUnitConverter, xAttrList, rValue, rStrName );
}

sal_Bool XMLDashStyle::ImpExportXML( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > & rHandler,
                                           const SvXMLNamespaceMap& rNamespaceMap, const SvXMLUnitConverter& rUnitConverter,
                                           const OUString& rStrName, const uno::Any& rValue )
{
    sal_Bool bRet = sal_False;

    drawing::LineDash aLineDash;

    if( rStrName.getLength() )
    {
        if( rValue >>= aLineDash )
        {
            sal_Bool bIsRel = aLineDash.Style == drawing::DashStyle_RECTRELATIVE || aLineDash.Style == drawing::DashStyle_ROUNDRELATIVE;

            mpAttrList = new SvXMLAttributeList();
            uno::Reference< xml::sax::XAttributeList > xAttrList( mpAttrList );

            OUString aStrValue;
            OUStringBuffer aOut;

            // Name
            AddAttribute( XML_NAMESPACE_DRAW, sXML_name, rStrName );

            // Style
            rUnitConverter.convertEnum( aOut, aLineDash.Style, pXML_DashStyle_Enum );
            aStrValue = aOut.makeStringAndClear();
            AddAttribute( XML_NAMESPACE_DRAW, sXML_style, aStrValue );


            // dots
            if( aLineDash.Dots )
            {
                AddAttribute( XML_NAMESPACE_DRAW, sXML_dots1, OUString::valueOf( (sal_Int32)aLineDash.Dots ) );

                if( aLineDash.DotLen )
                {
                    // dashes length
                    if( bIsRel )
                    {
                        rUnitConverter.convertPercent( aOut, aLineDash.DotLen );
                    }
                    else
                    {
                        rUnitConverter.convertMeasure( aOut, aLineDash.DotLen );
                    }
                    aStrValue = aOut.makeStringAndClear();
                    AddAttribute( XML_NAMESPACE_DRAW, sXML_dots1_length, aStrValue );
                }
            }

            // dashes
            if( aLineDash.Dashes )
            {
                AddAttribute( XML_NAMESPACE_DRAW, sXML_dots2, OUString::valueOf( (sal_Int32)aLineDash.Dashes ) );

                if( aLineDash.DashLen )
                {
                    // dashes length
                    if( bIsRel )
                    {
                        rUnitConverter.convertPercent( aOut, aLineDash.DashLen );
                    }
                    else
                    {
                        rUnitConverter.convertMeasure( aOut, aLineDash.DashLen );
                    }
                    aStrValue = aOut.makeStringAndClear();
                    AddAttribute( XML_NAMESPACE_DRAW, sXML_dots2_length, aStrValue );
                }
            }

            // distance
            if( bIsRel )
            {
                rUnitConverter.convertPercent( aOut, aLineDash.Distance );
            }
            else
            {
                rUnitConverter.convertMeasure( aOut, aLineDash.Distance );
            }
            aStrValue = aOut.makeStringAndClear();
            AddAttribute( XML_NAMESPACE_DRAW, sXML_distance, aStrValue );


            // Do Write
            OUString sElem = rNamespaceMap.GetQNameByKey( XML_NAMESPACE_DRAW,
                                           OUString::createFromAscii(sXML_stroke_dash) );

            rHandler->startElement( sElem, xAttrList );
            rHandler->endElement( sElem );
        }
    }
    return bRet;
}

sal_Bool XMLDashStyle::ImpImportXML( const SvXMLUnitConverter& rUnitConverter,
                                           const uno::Reference< xml::sax::XAttributeList >& xAttrList,
                                           uno::Any& rValue, OUString& rStrName )
{
    drawing::LineDash aLineDash;
    aLineDash.Style = drawing::DashStyle_RECT;
    aLineDash.Dots = 0;
    aLineDash.DotLen = 0;
    aLineDash.Dashes = 0;
    aLineDash.DashLen = 0;
    aLineDash.Distance = 20;

    sal_Bool bIsRel = sal_False;

    SvXMLTokenMap aTokenMap( aDashStyleAttrTokenMap );

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rFullAttrName = xAttrList->getNameByIndex( i );
        OUString aStrAttrName;
        sal_uInt16 nPrefix = mrNamespaceMap.GetKeyByAttrName( rFullAttrName, &aStrAttrName );
        const OUString& rStrValue = xAttrList->getValueByIndex( i );

        sal_Int32 nTmpValue;

        switch( aTokenMap.Get( nPrefix, aStrAttrName ) )
        {
        case XML_TOK_DASH_NAME:
            {
                rStrName = rStrValue;
            }
            break;
        case XML_TOK_DASH_STYLE:
            {
                sal_uInt16 eValue;
                if( rUnitConverter.convertEnum( eValue, rStrValue, pXML_DashStyle_Enum ) )
                {
                    aLineDash.Style = (drawing::DashStyle) eValue;
                }
            }
            break;
        case XML_TOK_DASH_DOTS1:
            aLineDash.Dots = (sal_Int16)rStrValue.toInt32();
            break;

        case XML_TOK_DASH_DOTS1LEN:
            {
                if( rStrValue.indexOf( sal_Unicode('%') ) != -1 ) // its a percentage
                {
                    bIsRel = sal_True;
                    rUnitConverter.convertPercent( aLineDash.DotLen, rStrValue );
                }
                else
                {
                    rUnitConverter.convertMeasure( aLineDash.DotLen, rStrValue );
                }
            }
            break;

        case XML_TOK_DASH_DOTS2:
            aLineDash.Dashes = (sal_Int16)rStrValue.toInt32();
            break;

        case XML_TOK_DASH_DOTS2LEN:
            {
                if( rStrValue.indexOf( sal_Unicode('%') ) != -1 ) // its a percentage
                {
                    bIsRel = sal_True;
                    rUnitConverter.convertPercent( aLineDash.DashLen, rStrValue );
                }
                else
                {
                    rUnitConverter.convertMeasure( aLineDash.DashLen, rStrValue );
                }
            }
            break;

        case XML_TOK_DASH_DISTANCE:
            {
                if( rStrValue.indexOf( sal_Unicode('%') ) != -1 ) // its a percentage
                {
                    bIsRel = sal_True;
                    rUnitConverter.convertPercent( aLineDash.Distance, rStrValue );
                }
                else
                {
                    rUnitConverter.convertMeasure( aLineDash.Distance, rStrValue );
                }
            }
            break;
        default:
            DBG_WARNING( "Unknown token at import gradient style" )
        }
    }

    if( bIsRel )
        aLineDash.Style = aLineDash.Style == drawing::DashStyle_RECT ? drawing::DashStyle_RECTRELATIVE : drawing::DashStyle_ROUNDRELATIVE;

    rValue <<= aLineDash;

    return sal_True;
}
