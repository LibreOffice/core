/*************************************************************************
 *
 *  $RCSfile: GradientStyle.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 08:18:45 $
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
#include <com/sun/star/awt/Gradient.hpp>
#endif

#ifndef _XMLOFF_ATTRLIST_HXX
#include"attrlist.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _XMLOFF_XMLTKMAP_HXX
#include "xmltkmap.hxx"
#endif

#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif



using namespace ::com::sun::star;
using namespace ::rtl;
using namespace ::xmloff::token;

enum SvXMLTokenMapAttrs
{
    XML_TOK_GRADIENT_NAME,
    XML_TOK_GRADIENT_DISPLAY_NAME,
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
    { XML_NAMESPACE_DRAW, XML_NAME, XML_TOK_GRADIENT_NAME },
    { XML_NAMESPACE_DRAW, XML_DISPLAY_NAME, XML_TOK_GRADIENT_DISPLAY_NAME },
    { XML_NAMESPACE_DRAW, XML_STYLE, XML_TOK_GRADIENT_STYLE },
    { XML_NAMESPACE_DRAW, XML_CX, XML_TOK_GRADIENT_CX },
    { XML_NAMESPACE_DRAW, XML_CY, XML_TOK_GRADIENT_CY },
    { XML_NAMESPACE_DRAW, XML_START_COLOR, XML_TOK_GRADIENT_STARTCOLOR },
    { XML_NAMESPACE_DRAW, XML_END_COLOR, XML_TOK_GRADIENT_ENDCOLOR },
    { XML_NAMESPACE_DRAW, XML_START_INTENSITY, XML_TOK_GRADIENT_STARTINT },
    { XML_NAMESPACE_DRAW, XML_END_INTENSITY, XML_TOK_GRADIENT_ENDINT },
    { XML_NAMESPACE_DRAW, XML_GRADIENT_ANGLE, XML_TOK_GRADIENT_ANGLE },
    { XML_NAMESPACE_DRAW, XML_GRADIENT_BORDER, XML_TOK_GRADIENT_BORDER },
    XML_TOKEN_MAP_END
};

SvXMLEnumMapEntry __READONLY_DATA pXML_GradientStyle_Enum[] =
{
    { XML_GRADIENTSTYLE_LINEAR,         awt::GradientStyle_LINEAR },
    { XML_GRADIENTSTYLE_AXIAL,          awt::GradientStyle_AXIAL },
    { XML_GRADIENTSTYLE_RADIAL,         awt::GradientStyle_RADIAL },
    { XML_GRADIENTSTYLE_ELLIPSOID,      awt::GradientStyle_ELLIPTICAL },
    { XML_GRADIENTSTYLE_SQUARE,         awt::GradientStyle_SQUARE },
    { XML_GRADIENTSTYLE_RECTANGULAR,    awt::GradientStyle_RECT },
    { XML_TOKEN_INVALID, 0 }
};

//-------------------------------------------------------------
// Import
//-------------------------------------------------------------
XMLGradientStyleImport::XMLGradientStyleImport(
    SvXMLImport& rImp )
    : rImport(rImp)
{
}

XMLGradientStyleImport::~XMLGradientStyleImport()
{
}

sal_Bool XMLGradientStyleImport::importXML(
    const uno::Reference< xml::sax::XAttributeList >& xAttrList,
    uno::Any& rValue,
    OUString& rStrName )
{
    sal_Bool bRet           = sal_False;
    sal_Bool bHasName       = sal_False;
    sal_Bool bHasStyle      = sal_False;
    sal_Bool bHasStartColor = sal_False;
    sal_Bool bHasEndColor   = sal_False;
    OUString aDisplayName;

    awt::Gradient aGradient;
    aGradient.XOffset = 0;
    aGradient.YOffset = 0;
    aGradient.StartIntensity = 100;
    aGradient.EndIntensity = 100;
    aGradient.Angle = 0;
    aGradient.Border = 0;

    SvXMLTokenMap aTokenMap( aGradientAttrTokenMap );
    SvXMLNamespaceMap& rNamespaceMap = rImport.GetNamespaceMap();
    SvXMLUnitConverter& rUnitConverter = rImport.GetMM100UnitConverter();

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rFullAttrName = xAttrList->getNameByIndex( i );
        OUString aStrAttrName;
        sal_uInt16 nPrefix = rNamespaceMap.GetKeyByAttrName( rFullAttrName, &aStrAttrName );
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
        case XML_TOK_GRADIENT_DISPLAY_NAME:
            {
                aDisplayName = rStrValue;
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
            aGradient.XOffset = static_cast< sal_Int16 >( nTmpValue );
            break;
        case XML_TOK_GRADIENT_CY:
            rUnitConverter.convertPercent( nTmpValue, rStrValue );
            aGradient.YOffset = static_cast< sal_Int16 >( nTmpValue );
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
            aGradient.StartIntensity = static_cast< sal_Int16 >( nTmpValue );
            break;
        case XML_TOK_GRADIENT_ENDINT:
            rUnitConverter.convertPercent( nTmpValue, rStrValue );
            aGradient.EndIntensity = static_cast< sal_Int16 >( nTmpValue );
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
            aGradient.Border = static_cast< sal_Int16 >( nTmpValue );
            break;

        default:
            DBG_WARNING( "Unknown token at import gradient style" )
            ;
        }
    }

    rValue <<= aGradient;

    if( aDisplayName.getLength() )
    {
        rImport.AddStyleDisplayName( XML_STYLE_FAMILY_SD_GRADIENT_ID, rStrName,
                                     aDisplayName );
        rStrName = aDisplayName;
    }

    bRet = bHasName && bHasStyle && bHasStartColor && bHasEndColor;

    return bRet;
}


//-------------------------------------------------------------
// Export
//-------------------------------------------------------------

#ifndef SVX_LIGHT

XMLGradientStyleExport::XMLGradientStyleExport(
    SvXMLExport& rExp )
    : rExport(rExp)
{
}

XMLGradientStyleExport::~XMLGradientStyleExport()
{
}

sal_Bool XMLGradientStyleExport::exportXML(
    const OUString& rStrName,
    const uno::Any& rValue )
{
    sal_Bool bRet = sal_False;
    awt::Gradient aGradient;

    if( rStrName.getLength() )
    {
        if( rValue >>= aGradient )
        {
            OUString aStrValue;
            OUStringBuffer aOut;

            SvXMLUnitConverter& rUnitConverter =
                rExport.GetMM100UnitConverter();

            // Style
            if( !rUnitConverter.convertEnum( aOut, aGradient.Style, pXML_GradientStyle_Enum ) )
            {
                bRet = sal_False;
            }
            else
            {
                // Name
                sal_Bool bEncoded = sal_False;
                OUString aStrName( rStrName );
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME,
                                      rExport.EncodeStyleName( aStrName,
                                                                &bEncoded ) );
                if( bEncoded )
                    rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_DISPLAY_NAME,
                                            aStrName );


                aStrValue = aOut.makeStringAndClear();
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_STYLE, aStrValue );

                // Center x/y
                if( aGradient.Style != awt::GradientStyle_LINEAR &&
                    aGradient.Style != awt::GradientStyle_AXIAL   )
                {
                    rUnitConverter.convertPercent( aOut, aGradient.XOffset );
                    aStrValue = aOut.makeStringAndClear();
                    rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_CX, aStrValue );

                    rUnitConverter.convertPercent( aOut, aGradient.YOffset );
                    aStrValue = aOut.makeStringAndClear();
                    rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_CY, aStrValue );
                }

                Color aColor;

                // Color start
                aColor.SetColor( aGradient.StartColor );
                rUnitConverter.convertColor( aOut, aColor );
                aStrValue = aOut.makeStringAndClear();
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_START_COLOR, aStrValue );

                // Color end
                aColor.SetColor( aGradient.EndColor );
                rUnitConverter.convertColor( aOut, aColor );
                aStrValue = aOut.makeStringAndClear();
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_END_COLOR, aStrValue );

                // Intensity start
                rUnitConverter.convertPercent( aOut, aGradient.StartIntensity );
                aStrValue = aOut.makeStringAndClear();
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_START_INTENSITY, aStrValue );

                // Intensity end
                rUnitConverter.convertPercent( aOut, aGradient.EndIntensity );
                aStrValue = aOut.makeStringAndClear();
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_END_INTENSITY, aStrValue );

                // Angle
                if( aGradient.Style != awt::GradientStyle_RADIAL )
                {
                    rUnitConverter.convertNumber( aOut, sal_Int32( aGradient.Angle ) );
                    aStrValue = aOut.makeStringAndClear();
                    rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_GRADIENT_ANGLE, aStrValue );
                }

                // Border
                rUnitConverter.convertPercent( aOut, aGradient.Border );
                aStrValue = aOut.makeStringAndClear();
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_GRADIENT_BORDER, aStrValue );

                // Do Write
                SvXMLElementExport aElem( rExport, XML_NAMESPACE_DRAW, XML_GRADIENT,
                                      sal_True, sal_False );
            }
        }
    }

    return bRet;
}

#endif // #ifndef SVX_LIGHT
