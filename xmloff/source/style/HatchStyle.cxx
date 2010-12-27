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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"
#include "xmloff/HatchStyle.hxx"
#include <com/sun/star/drawing/Hatch.hpp>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmluconv.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlimp.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <tools/debug.hxx>
#include <xmloff/xmltkmap.hxx>

using namespace ::com::sun::star;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::xmloff::token;

enum SvXMLTokenMapAttrs
{
    XML_TOK_HATCH_NAME,
    XML_TOK_HATCH_DISPLAY_NAME,
    XML_TOK_HATCH_STYLE,
    XML_TOK_HATCH_COLOR,
    XML_TOK_HATCH_DISTANCE,
    XML_TOK_HATCH_ROTATION,
    XML_TOK_TABSTOP_END=XML_TOK_UNKNOWN
};


SvXMLEnumMapEntry __READONLY_DATA pXML_HatchStyle_Enum[] =
{
    { XML_HATCHSTYLE_SINGLE,    drawing::HatchStyle_SINGLE },
    { XML_HATCHSTYLE_DOUBLE,    drawing::HatchStyle_DOUBLE },
    { XML_HATCHSTYLE_TRIPLE,    drawing::HatchStyle_TRIPLE },
    { XML_TOKEN_INVALID, 0 }
};


//-------------------------------------------------------------
// Import
//-------------------------------------------------------------

XMLHatchStyleImport::XMLHatchStyleImport( SvXMLImport& rImp )
    : rImport(rImp)
{
}

XMLHatchStyleImport::~XMLHatchStyleImport()
{
}

sal_Bool XMLHatchStyleImport::importXML(
    const uno::Reference< xml::sax::XAttributeList >& xAttrList,
    uno::Any& rValue,
    OUString& rStrName )
{
    sal_Bool bRet = sal_False;

    sal_Bool bHasName  = sal_False;
    sal_Bool bHasStyle = sal_False;
    sal_Bool bHasColor = sal_False;
    sal_Bool bHasDist  = sal_False;
    OUString aDisplayName;

    drawing::Hatch aHatch;
    aHatch.Style = drawing::HatchStyle_SINGLE;
    aHatch.Color = 0;
    aHatch.Distance = 0;
    aHatch.Angle = 0;

    {
        static __FAR_DATA SvXMLTokenMapEntry aHatchAttrTokenMap[] =
{
    { XML_NAMESPACE_DRAW, XML_NAME, XML_TOK_HATCH_NAME },
    { XML_NAMESPACE_DRAW, XML_DISPLAY_NAME, XML_TOK_HATCH_DISPLAY_NAME },
    { XML_NAMESPACE_DRAW, XML_STYLE, XML_TOK_HATCH_STYLE },
    { XML_NAMESPACE_DRAW, XML_COLOR, XML_TOK_HATCH_COLOR },
    { XML_NAMESPACE_DRAW, XML_HATCH_DISTANCE, XML_TOK_HATCH_DISTANCE },
    { XML_NAMESPACE_DRAW, XML_ROTATION, XML_TOK_HATCH_ROTATION },
    XML_TOKEN_MAP_END
};

    SvXMLTokenMap aTokenMap( aHatchAttrTokenMap );
    SvXMLNamespaceMap rNamespaceMap = rImport.GetNamespaceMap();
    SvXMLUnitConverter& rUnitConverter = rImport.GetMM100UnitConverter();

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rFullAttrName = xAttrList->getNameByIndex( i );
        OUString aStrAttrName;
        sal_uInt16 nPrefix = rNamespaceMap.GetKeyByAttrName( rFullAttrName, &aStrAttrName );
        const OUString& rStrValue = xAttrList->getValueByIndex( i );

        switch( aTokenMap.Get( nPrefix, aStrAttrName ) )
        {
            case XML_TOK_HATCH_NAME:
                {
                    rStrName = rStrValue;
                    bHasName = sal_True;
                }
                break;
            case XML_TOK_HATCH_DISPLAY_NAME:
                aDisplayName = rStrValue;
                break;
            case XML_TOK_HATCH_STYLE:
                {
                    sal_uInt16 eValue;
                    bHasStyle = rUnitConverter.convertEnum( eValue, rStrValue, pXML_HatchStyle_Enum );
                    if( bHasStyle )
                        aHatch.Style = (drawing::HatchStyle) eValue;
                }
                break;
            case XML_TOK_HATCH_COLOR:
                {
                    Color aColor;
                    bHasColor = rUnitConverter.convertColor( aColor, rStrValue );
                    if( bHasColor )
                        aHatch.Color = (sal_Int32)( aColor.GetColor() );
                }
                break;
            case XML_TOK_HATCH_DISTANCE:
                bHasDist = rUnitConverter.convertMeasure( (sal_Int32&)aHatch.Distance, rStrValue );
                break;
            case XML_TOK_HATCH_ROTATION:
                {
                    sal_Int32 nValue;
                    rUnitConverter.convertNumber( nValue, rStrValue, 0, 3600 );
                    aHatch.Angle = sal_Int16( nValue );
                }
                break;

            default:
                DBG_WARNING( "Unknown token at import hatch style" )
                ;
        }
    }

    rValue <<= aHatch;

    if( aDisplayName.getLength() )
    {
        rImport.AddStyleDisplayName( XML_STYLE_FAMILY_SD_HATCH_ID, rStrName,
                                     aDisplayName );
        rStrName = aDisplayName;
    }

    bRet = bHasName && bHasStyle && bHasColor && bHasDist;

    }

    return bRet;
}


//-------------------------------------------------------------
// Export
//-------------------------------------------------------------

#ifndef SVX_LIGHT

XMLHatchStyleExport::XMLHatchStyleExport( SvXMLExport& rExp )
    : rExport(rExp)
{
}

XMLHatchStyleExport::~XMLHatchStyleExport()
{
}

sal_Bool XMLHatchStyleExport::exportXML(
    const OUString& rStrName,
    const uno::Any& rValue )
{
    sal_Bool bRet = sal_False;
    drawing::Hatch aHatch;

    if( rStrName.getLength() )
    {
        if( rValue >>= aHatch )
        {
            OUString aStrValue;
            OUStringBuffer aOut;

            SvXMLUnitConverter& rUnitConverter =
                rExport.GetMM100UnitConverter();

            // Style
            if( !rUnitConverter.convertEnum( aOut, aHatch.Style, pXML_HatchStyle_Enum ) )
            {
                bRet = sal_False;
            }
            else
            {
                // Name
                sal_Bool bEncoded = sal_False;
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME,
                                      rExport.EncodeStyleName( rStrName,
                                                                &bEncoded ) );
                if( bEncoded )
                    rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_DISPLAY_NAME,
                                            rStrName );

                aStrValue = aOut.makeStringAndClear();
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_STYLE, aStrValue );

                // Color
                rUnitConverter.convertColor( aOut, Color( aHatch.Color ) );
                aStrValue = aOut.makeStringAndClear();
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_COLOR, aStrValue );

                // Distance
                rUnitConverter.convertMeasure( aOut, aHatch.Distance );
                aStrValue = aOut.makeStringAndClear();
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_HATCH_DISTANCE, aStrValue );

                // Angle
                rUnitConverter.convertNumber( aOut, sal_Int32( aHatch.Angle ) );
                aStrValue = aOut.makeStringAndClear();
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_ROTATION, aStrValue );

                // Do Write
                SvXMLElementExport rElem( rExport, XML_NAMESPACE_DRAW, XML_HATCH,
                                          sal_True, sal_False );
            }
        }
    }

    return bRet;
}

#endif // #ifndef SVX_LIGHT
