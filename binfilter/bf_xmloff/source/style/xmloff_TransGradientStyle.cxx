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

#ifndef _XMLOFF_TRANSGRADIENTSTYLE_HXX
#include "TransGradientStyle.hxx"
#endif

#ifndef _COM_SUN_STAR_AWT_GRADIENT_HPP_
#include <com/sun/star/awt/Gradient.hpp>
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


#include "rtl/ustring.hxx"

#ifndef _TOOLS_DEBUG_HXX 
#include <tools/debug.hxx>
#endif


#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif
namespace binfilter {


using namespace ::com::sun::star;
using namespace ::rtl;
using namespace ::binfilter::xmloff::token;

enum SvXMLTokenMapAttrs
{
    XML_TOK_GRADIENT_NAME,
    XML_TOK_GRADIENT_STYLE,
    XML_TOK_GRADIENT_CX,
    XML_TOK_GRADIENT_CY,
    XML_TOK_GRADIENT_START,
    XML_TOK_GRADIENT_END,
    XML_TOK_GRADIENT_ANGLE,
    XML_TOK_GRADIENT_BORDER,
    XML_TOK_TABSTOP_END=XML_TOK_UNKNOWN
};

static __FAR_DATA SvXMLTokenMapEntry aTrGradientAttrTokenMap[] =
{
    { XML_NAMESPACE_DRAW, XML_NAME, XML_TOK_GRADIENT_NAME },
    { XML_NAMESPACE_DRAW, XML_STYLE, XML_TOK_GRADIENT_STYLE },
    { XML_NAMESPACE_DRAW, XML_CX, XML_TOK_GRADIENT_CX },
    { XML_NAMESPACE_DRAW, XML_CY, XML_TOK_GRADIENT_CY },
    { XML_NAMESPACE_DRAW, XML_START, XML_TOK_GRADIENT_START },
    { XML_NAMESPACE_DRAW, XML_END, XML_TOK_GRADIENT_END },
    { XML_NAMESPACE_DRAW, XML_GRADIENT_ANGLE, XML_TOK_GRADIENT_ANGLE },
    { XML_NAMESPACE_DRAW, XML_GRADIENT_BORDER, XML_TOK_GRADIENT_BORDER },
    XML_TOKEN_MAP_END 
};

SvXMLEnumMapEntry __READONLY_DATA pXML_GradientStyle_Enum[] =
{
    { XML_GRADIENTSTYLE_LINEAR,		    awt::GradientStyle_LINEAR },
    { XML_GRADIENTSTYLE_AXIAL,			awt::GradientStyle_AXIAL },
    { XML_GRADIENTSTYLE_RADIAL,		    awt::GradientStyle_RADIAL },
    { XML_GRADIENTSTYLE_ELLIPSOID,		awt::GradientStyle_ELLIPTICAL },
    { XML_GRADIENTSTYLE_SQUARE,		    awt::GradientStyle_SQUARE },
    { XML_GRADIENTSTYLE_RECTANGULAR,	awt::GradientStyle_RECT },
    { XML_TOKEN_INVALID,                0 }
};


//-------------------------------------------------------------
// Import
//-------------------------------------------------------------

XMLTransGradientStyleImport::XMLTransGradientStyleImport( SvXMLImport& rImp )
    : rImport(rImp)
{
}

XMLTransGradientStyleImport::~XMLTransGradientStyleImport()
{
}

sal_Bool XMLTransGradientStyleImport::importXML( 
    const uno::Reference< xml::sax::XAttributeList >& xAttrList, 
    uno::Any& rValue, 
    OUString& rStrName )
{
    sal_Bool bRet           = sal_False;
    sal_Bool bHasName       = sal_False;
    sal_Bool bHasStyle      = sal_False;

    awt::Gradient aGradient;
    aGradient.XOffset = 0;
    aGradient.YOffset = 0;
    aGradient.StartIntensity = 100;
    aGradient.EndIntensity = 100;
    aGradient.Angle = 0;
    aGradient.Border = 0;

    SvXMLTokenMap aTokenMap( aTrGradientAttrTokenMap );
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
        case XML_TOK_GRADIENT_START:
            {
                sal_Int32 aStartTransparency;
                rUnitConverter.convertPercent( aStartTransparency, rStrValue );

                aStartTransparency = ( aStartTransparency * 255 ) / 100;
                
                Color aColor(aStartTransparency, aStartTransparency, aStartTransparency );
                aGradient.StartColor = (sal_Int32)( aColor.GetColor() );
            }
            break;
        case XML_TOK_GRADIENT_END:
            {
                sal_Int32 aEndTransparency;
                rUnitConverter.convertPercent( aEndTransparency, rStrValue );

                aEndTransparency = ( aEndTransparency * 255 ) / 100;

                Color aColor( aEndTransparency, aEndTransparency, aEndTransparency );
                aGradient.EndColor = (sal_Int32)( aColor.GetColor() );
            }
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
            DBG_WARNING( "Unknown token at import transparency gradient style" )
            ;
        }
    }

    rValue <<= aGradient;

    bRet = bHasName && bHasStyle;

    return bRet;
}


//-------------------------------------------------------------
// Export
//-------------------------------------------------------------

#ifndef SVX_LIGHT

XMLTransGradientStyleExport::XMLTransGradientStyleExport( SvXMLExport& rExp )
    : rExport(rExp)
{
}

XMLTransGradientStyleExport::~XMLTransGradientStyleExport()
{
}


sal_Bool XMLTransGradientStyleExport::exportXML( 
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
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME, rStrName );
                
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

                // Transparency start
                aColor.SetColor( aGradient.StartColor );
                sal_Int32 aStartValue = (sal_Int32)(((aColor.GetRed() + 1) * 100) / 255);
                rUnitConverter.convertPercent( aOut, aStartValue );
                aStrValue = aOut.makeStringAndClear();
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_START, aStrValue );
                
                // Transparency end
                aColor.SetColor( aGradient.EndColor );
                sal_Int32 aEndValue = (sal_Int32)(((aColor.GetRed() + 1) * 100) / 255);
                rUnitConverter.convertPercent( aOut, aEndValue );
                aStrValue = aOut.makeStringAndClear();
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_END, aStrValue );
                
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
                SvXMLElementExport rElem( rExport, 
                                          XML_NAMESPACE_DRAW, XML_TRANSPARENCY,
                                          sal_True, sal_False );
            }
        }
    }

    return bRet;
}

#endif // #ifndef SVX_LIGHT
}//end of namespace binfilter
