/*************************************************************************
 *
 *  $RCSfile: GradientStyle.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: cl $ $Date: 2000-10-10 11:04:20 $
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

#ifndef _XMLOFF_GRADIENTSTYLE_HXX
#include "GradientStyle.hxx"
#endif

#ifndef _COM_SUN_STAR_AWT_GRADIENT_HPP_
#include<com/sun/star/awt/Gradient.hpp>
#endif

#ifndef _XMLOFF_ATTRLIST_HXX
#include"attrlist.hxx"
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
    XML_TOK_GRADIENT_NAME,
    XML_TOK_GRADIENT_STYLE,
    XML_TOK_GRADIENT_CX,
    XML_TOK_GRADIENT_CY,
    XML_TOK_GRADIENT_STARTCOLOR,
    XML_TOK_GRADIENT_ENDCOLOR,
    XML_TOK_GRADIENT_STARTINT,
    XML_TOK_GRADIENT_ENDINT,
    XML_TOK_GRADIENT_ANGLE,
    XML_TOK_GRADIENT_BORDER,
    XML_TOK_TABSTOP_END=XML_TOK_UNKNOWN
};

static __FAR_DATA SvXMLTokenMapEntry aGradientAttrTokenMap[] =
{
    { XML_NAMESPACE_DRAW, sXML_name, XML_TOK_GRADIENT_NAME },
    { XML_NAMESPACE_DRAW, sXML_style, XML_TOK_GRADIENT_STYLE },
    { XML_NAMESPACE_DRAW, sXML_cx, XML_TOK_GRADIENT_CX },
    { XML_NAMESPACE_DRAW, sXML_cy, XML_TOK_GRADIENT_CY },
    { XML_NAMESPACE_DRAW, sXML_start_color, XML_TOK_GRADIENT_STARTCOLOR },
    { XML_NAMESPACE_DRAW, sXML_end_color, XML_TOK_GRADIENT_ENDCOLOR },
    { XML_NAMESPACE_DRAW, sXML_start_intensity, XML_TOK_GRADIENT_STARTINT },
    { XML_NAMESPACE_DRAW, sXML_end_intensity, XML_TOK_GRADIENT_ENDINT },
    { XML_NAMESPACE_DRAW, sXML_gradient_angle, XML_TOK_GRADIENT_ANGLE },
    { XML_NAMESPACE_DRAW, sXML_gradient_border, XML_TOK_GRADIENT_BORDER },
    XML_TOKEN_MAP_END
};

SvXMLEnumMapEntry __READONLY_DATA pXML_GradientStyle_Enum[] =
{
    { sXML_gradientstyle_linear,        awt::GradientStyle_LINEAR },
    { sXML_gradientstyle_axial,         awt::GradientStyle_AXIAL },
    { sXML_gradientstyle_radial,        awt::GradientStyle_RADIAL },
    { sXML_gradientstyle_ellipsoid,     awt::GradientStyle_ELLIPTICAL },
    { sXML_gradientstyle_square,        awt::GradientStyle_SQUARE },
    { sXML_gradientstyle_rectangular,   awt::GradientStyle_RECT },
    { 0, 0 }
};

XMLGradientStyle::XMLGradientStyle( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > * _pHandler,
                                        const SvXMLNamespaceMap& _rNamespaceMap, const SvXMLUnitConverter& _rUnitConverter )
: mpHandler      ( _pHandler ),
  mrNamespaceMap ( _rNamespaceMap ),
  mrUnitConverter( _rUnitConverter ),
  mpAttrList     ( NULL )
{
}

XMLGradientStyle::~XMLGradientStyle()
{
}

void XMLGradientStyle::AddAttribute( sal_uInt16 nPrefix, const sal_Char *pName, const OUString& rStrValue )
{
    const OUString aStrName( OUString::createFromAscii( pName ) );
    const OUString aStrCDATA( OUString::createFromAscii( sXML_CDATA ) );

    mpAttrList->AddAttribute( mrNamespaceMap.GetQNameByKey( nPrefix, aStrName ), aStrCDATA, rStrValue );
}

sal_Bool XMLGradientStyle::exportXML( const OUString& rStrName, const ::com::sun::star::uno::Any& rValue )
{
    return ImpExportXML( *mpHandler, mrNamespaceMap, mrUnitConverter, rStrName, rValue );
}

sal_Bool XMLGradientStyle::importXML( const uno::Reference< xml::sax::XAttributeList >& xAttrList, uno::Any& rValue, OUString& rStrName )
{
    return ImpImportXML( mrUnitConverter, xAttrList, rValue, rStrName );
}

sal_Bool XMLGradientStyle::ImpExportXML( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > & rHandler,
                                           const SvXMLNamespaceMap& rNamespaceMap, const SvXMLUnitConverter& rUnitConverter,
                                           const OUString& rStrName, const uno::Any& rValue )
{
    sal_Bool bRet = sal_False;
    awt::Gradient aGradient;

    if( rStrName.getLength() )
    {
        if( rValue >>= aGradient )
        {
            mpAttrList = new SvXMLAttributeList();  // Do NOT delete me !!
            ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > xAttrList( mpAttrList );

            OUString aStrValue;
            OUStringBuffer aOut;

            // Name
            OUString aStrName( rStrName );
            AddAttribute( XML_NAMESPACE_DRAW, sXML_name, aStrName );

            // Style
            if( !rUnitConverter.convertEnum( aOut, aGradient.Style, pXML_GradientStyle_Enum ) )
                return sal_False;
            aStrValue = aOut.makeStringAndClear();
            AddAttribute( XML_NAMESPACE_DRAW, sXML_style, aStrValue );

            // Center x/y
            if( aGradient.Style != awt::GradientStyle_LINEAR &&
                aGradient.Style != awt::GradientStyle_AXIAL   )
            {
                rUnitConverter.convertPercent( aOut, aGradient.XOffset );
                aStrValue = aOut.makeStringAndClear();
                AddAttribute( XML_NAMESPACE_DRAW, sXML_cx, aStrValue );

                rUnitConverter.convertPercent( aOut, aGradient.YOffset );
                aStrValue = aOut.makeStringAndClear();
                AddAttribute( XML_NAMESPACE_DRAW, sXML_cy, aStrValue );
            }

            Color aColor;

            // Color start
            aColor.SetColor( aGradient.StartColor );
            rUnitConverter.convertColor( aOut, aColor );
            aStrValue = aOut.makeStringAndClear();
            AddAttribute( XML_NAMESPACE_DRAW, sXML_start_color, aStrValue );

            // Color end
            aColor.SetColor( aGradient.EndColor );
            rUnitConverter.convertColor( aOut, aColor );
            aStrValue = aOut.makeStringAndClear();
            AddAttribute( XML_NAMESPACE_DRAW, sXML_end_color, aStrValue );

            // Intensity start
            rUnitConverter.convertPercent( aOut, aGradient.StartIntensity );
            aStrValue = aOut.makeStringAndClear();
            AddAttribute( XML_NAMESPACE_DRAW, sXML_start_intensity, aStrValue );

            // Intensity end
            rUnitConverter.convertPercent( aOut, aGradient.EndIntensity );
            aStrValue = aOut.makeStringAndClear();
            AddAttribute( XML_NAMESPACE_DRAW, sXML_end_intensity, aStrValue );

            // Angle
            if( aGradient.Style != awt::GradientStyle_RADIAL )
            {
                rUnitConverter.convertNumber( aOut, sal_Int32( aGradient.Angle ) );
                aStrValue = aOut.makeStringAndClear();
                AddAttribute( XML_NAMESPACE_DRAW, sXML_gradient_angle, aStrValue );
            }

            // Border
            rUnitConverter.convertPercent( aOut, aGradient.Border );
            aStrValue = aOut.makeStringAndClear();
            AddAttribute( XML_NAMESPACE_DRAW, sXML_gradient_border, aStrValue );

            // Do Write
            OUString aStrTmp( RTL_CONSTASCII_USTRINGPARAM( sXML_gradient ) );
            rHandler->startElement( rNamespaceMap.GetQNameByKey( XML_NAMESPACE_DRAW, aStrTmp ),
                                    xAttrList );
            rHandler->endElement( OUString::createFromAscii( sXML_gradient ) );
        }
    }

    return bRet;
}

sal_Bool XMLGradientStyle::ImpImportXML( const SvXMLUnitConverter& rUnitConverter,
                                           const uno::Reference< xml::sax::XAttributeList >& xAttrList,
                                           uno::Any& rValue, OUString& rStrName )
{
    sal_Bool bRet           = sal_False;
    sal_Bool bHasName       = sal_False;
    sal_Bool bHasStyle      = sal_False;
    sal_Bool bHasStartColor = sal_False;
    sal_Bool bHasEndColor   = sal_False;

    awt::Gradient aGradient;
    aGradient.XOffset = 0;
    aGradient.YOffset = 0;
    aGradient.StartIntensity = 100;
    aGradient.EndIntensity = 100;
    aGradient.Angle = 0;
    aGradient.Border = 0;

    SvXMLTokenMap aTokenMap( aGradientAttrTokenMap );

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
        case XML_TOK_GRADIENT_NAME:
            {
                rStrName = rStrValue;
                bHasName = sal_True;
            }
            break;
        case XML_TOK_GRADIENT_STYLE:
            {
                sal_uInt16 eValue;
                if( rUnitConverter.convertEnum( eValue, rStrValue, pXML_GradientStyle_Enum ) )
                {
                    aGradient.Style = (awt::GradientStyle) eValue;
                    bHasStyle = sal_True;
                }
            }
            break;
        case XML_TOK_GRADIENT_CX:
            rUnitConverter.convertPercent( nTmpValue, rStrValue );
            aGradient.XOffset = nTmpValue;
            break;
        case XML_TOK_GRADIENT_CY:
            rUnitConverter.convertPercent( nTmpValue, rStrValue );
            aGradient.YOffset = nTmpValue;
            break;
        case XML_TOK_GRADIENT_STARTCOLOR:
            {
                Color aColor;
                if( bHasStartColor = rUnitConverter.convertColor( aColor, rStrValue ) )
                    aGradient.StartColor = (sal_Int32)( aColor.GetColor() );
            }
            break;
        case XML_TOK_GRADIENT_ENDCOLOR:
            {
                Color aColor;
                if( bHasStartColor = rUnitConverter.convertColor( aColor, rStrValue ) )
                    aGradient.EndColor = (sal_Int32)( aColor.GetColor() );
            }
            break;
        case XML_TOK_GRADIENT_STARTINT:
            rUnitConverter.convertPercent( nTmpValue, rStrValue );
            aGradient.StartIntensity = nTmpValue;
            break;
        case XML_TOK_GRADIENT_ENDINT:
            rUnitConverter.convertPercent( nTmpValue, rStrValue );
            aGradient.EndIntensity = nTmpValue;
            break;
        case XML_TOK_GRADIENT_ANGLE:
            {
                sal_Int32 nValue;
                rUnitConverter.convertNumber( nValue, rStrValue, 0, 360 );
                aGradient.Angle = sal_Int16( nValue );
            }
            break;
        case XML_TOK_GRADIENT_BORDER:
            rUnitConverter.convertPercent( nTmpValue, rStrValue );
            aGradient.Border = nTmpValue;
            break;

        default:
            DBG_WARNING( "Unknown token at import gradient style" )
            ;
        }
    }

    rValue <<= aGradient;

    bRet = bHasName && bHasStyle && bHasStartColor && bHasEndColor;

    return bRet;
}
