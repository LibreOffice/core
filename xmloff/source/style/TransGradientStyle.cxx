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


#include "TransGradientStyle.hxx"

#include <com/sun/star/awt/Gradient.hpp>

#include <sax/tools/converter.hxx>

#include <xmloff/attrlist.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmluconv.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <tools/color.hxx>
#include <tools/debug.hxx>
#include <xmloff/xmltkmap.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlimp.hxx>


using namespace ::com::sun::star;

using namespace ::xmloff::token;

enum SvXMLTokenMapAttrs
{
    XML_TOK_GRADIENT_NAME,
    XML_TOK_GRADIENT_DISPLAY_NAME,
    XML_TOK_GRADIENT_STYLE,
    XML_TOK_GRADIENT_CX,
    XML_TOK_GRADIENT_CY,
    XML_TOK_GRADIENT_START,
    XML_TOK_GRADIENT_END,
    XML_TOK_GRADIENT_ANGLE,
    XML_TOK_GRADIENT_BORDER,
    XML_TOK_TABSTOP_END=XML_TOK_UNKNOWN
};


SvXMLEnumMapEntry const pXML_GradientStyle_Enum[] =
{
    { XML_GRADIENTSTYLE_LINEAR,         awt::GradientStyle_LINEAR },
    { XML_GRADIENTSTYLE_AXIAL,          awt::GradientStyle_AXIAL },
    { XML_GRADIENTSTYLE_RADIAL,         awt::GradientStyle_RADIAL },
    { XML_GRADIENTSTYLE_ELLIPSOID,      awt::GradientStyle_ELLIPTICAL },
    { XML_GRADIENTSTYLE_SQUARE,         awt::GradientStyle_SQUARE },
    { XML_GRADIENTSTYLE_RECTANGULAR,    awt::GradientStyle_RECT },
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
    OUString aDisplayName;

    awt::Gradient aGradient;
    aGradient.XOffset = 0;
    aGradient.YOffset = 0;
    aGradient.StartIntensity = 100;
    aGradient.EndIntensity = 100;
    aGradient.Angle = 0;
    aGradient.Border = 0;

    {
        static const SvXMLTokenMapEntry aTrGradientAttrTokenMap[] =
{
    { XML_NAMESPACE_DRAW, XML_NAME, XML_TOK_GRADIENT_NAME },
    { XML_NAMESPACE_DRAW, XML_DISPLAY_NAME, XML_TOK_GRADIENT_DISPLAY_NAME },
    { XML_NAMESPACE_DRAW, XML_STYLE, XML_TOK_GRADIENT_STYLE },
    { XML_NAMESPACE_DRAW, XML_CX, XML_TOK_GRADIENT_CX },
    { XML_NAMESPACE_DRAW, XML_CY, XML_TOK_GRADIENT_CY },
    { XML_NAMESPACE_DRAW, XML_START, XML_TOK_GRADIENT_START },
    { XML_NAMESPACE_DRAW, XML_END, XML_TOK_GRADIENT_END },
    { XML_NAMESPACE_DRAW, XML_GRADIENT_ANGLE, XML_TOK_GRADIENT_ANGLE },
    { XML_NAMESPACE_DRAW, XML_GRADIENT_BORDER, XML_TOK_GRADIENT_BORDER },
    XML_TOKEN_MAP_END
};

    SvXMLTokenMap aTokenMap( aTrGradientAttrTokenMap );
    SvXMLNamespaceMap& rNamespaceMap = rImport.GetNamespaceMap();

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
                if( SvXMLUnitConverter::convertEnum( eValue, rStrValue, pXML_GradientStyle_Enum ) )
                {
                    aGradient.Style = (awt::GradientStyle) eValue;
                    bHasStyle = sal_True;
                }
            }
            break;
        case XML_TOK_GRADIENT_CX:
            ::sax::Converter::convertPercent( nTmpValue, rStrValue );
            aGradient.XOffset = sal::static_int_cast< sal_Int16 >(nTmpValue);
            break;
        case XML_TOK_GRADIENT_CY:
            ::sax::Converter::convertPercent( nTmpValue, rStrValue );
            aGradient.YOffset = sal::static_int_cast< sal_Int16 >(nTmpValue);
            break;
        case XML_TOK_GRADIENT_START:
            {
                sal_Int32 aStartTransparency;
                ::sax::Converter::convertPercent( aStartTransparency, rStrValue );

                sal_uInt8 n = sal::static_int_cast< sal_uInt8 >(
                    ( (100 - aStartTransparency) * 255 ) / 100 );

                Color aColor( n, n, n );
                aGradient.StartColor = (sal_Int32)( aColor.GetColor() );
            }
            break;
        case XML_TOK_GRADIENT_END:
            {
                sal_Int32 aEndTransparency;
                ::sax::Converter::convertPercent( aEndTransparency, rStrValue );

                sal_uInt8 n = sal::static_int_cast< sal_uInt8 >(
                    ( (100 - aEndTransparency) * 255 ) / 100 );

                Color aColor( n, n, n );
                aGradient.EndColor = (sal_Int32)( aColor.GetColor() );
            }
            break;
        case XML_TOK_GRADIENT_ANGLE:
            {
                sal_Int32 nValue;
                ::sax::Converter::convertNumber( nValue, rStrValue, 0, 3600 );
                aGradient.Angle = sal_Int16( nValue );
            }
            break;
        case XML_TOK_GRADIENT_BORDER:
            ::sax::Converter::convertPercent( nTmpValue, rStrValue );
            aGradient.Border = sal::static_int_cast< sal_Int16 >(nTmpValue);
            break;

        default:
            DBG_WARNING( "Unknown token at import transparency gradient style" )
            ;
        }
    }

    rValue <<= aGradient;

    if( !aDisplayName.isEmpty() )
    {
        rImport.AddStyleDisplayName( XML_STYLE_FAMILY_SD_GRADIENT_ID, rStrName,
                                     aDisplayName );
        rStrName = aDisplayName;
    }

    bRet = bHasName && bHasStyle;

    }

    return bRet;
}


//-------------------------------------------------------------
// Export
//-------------------------------------------------------------

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

    if( !rStrName.isEmpty() )
    {
        if( rValue >>= aGradient )
        {
            OUString aStrValue;
            OUStringBuffer aOut;

            // Style
            if( !SvXMLUnitConverter::convertEnum( aOut, aGradient.Style, pXML_GradientStyle_Enum ) )
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

                // Center x/y
                if( aGradient.Style != awt::GradientStyle_LINEAR &&
                    aGradient.Style != awt::GradientStyle_AXIAL   )
                {
                    ::sax::Converter::convertPercent(aOut, aGradient.XOffset);
                    aStrValue = aOut.makeStringAndClear();
                    rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_CX, aStrValue );

                    ::sax::Converter::convertPercent(aOut, aGradient.YOffset);
                    aStrValue = aOut.makeStringAndClear();
                    rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_CY, aStrValue );
                }


                Color aColor;

                // Transparency start
                aColor.SetColor( aGradient.StartColor );
                sal_Int32 aStartValue = 100 - (sal_Int32)(((aColor.GetRed() + 1) * 100) / 255);
                ::sax::Converter::convertPercent( aOut, aStartValue );
                aStrValue = aOut.makeStringAndClear();
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_START, aStrValue );

                // Transparency end
                aColor.SetColor( aGradient.EndColor );
                sal_Int32 aEndValue = 100 - (sal_Int32)(((aColor.GetRed() + 1) * 100) / 255);
                ::sax::Converter::convertPercent( aOut, aEndValue );
                aStrValue = aOut.makeStringAndClear();
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_END, aStrValue );

                // Angle
                if( aGradient.Style != awt::GradientStyle_RADIAL )
                {
                    ::sax::Converter::convertNumber(
                            aOut, sal_Int32(aGradient.Angle));
                    aStrValue = aOut.makeStringAndClear();
                    rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_GRADIENT_ANGLE, aStrValue );
                }

                // Border
                ::sax::Converter::convertPercent( aOut, aGradient.Border );
                aStrValue = aOut.makeStringAndClear();
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_GRADIENT_BORDER, aStrValue );

                // Do Write
                SvXMLElementExport rElem( rExport,
                                          XML_NAMESPACE_DRAW, XML_OPACITY,
                                          sal_True, sal_False );
            }
        }
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
