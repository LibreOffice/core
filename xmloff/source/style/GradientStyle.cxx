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

#include <xmloff/GradientStyle.hxx>

#include <com/sun/star/awt/Gradient.hpp>

#include <sax/tools/converter.hxx>
#include <comphelper/documentconstants.hxx>

#include <xmloff/namespacemap.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <xmloff/xmltkmap.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlement.hxx>

using namespace ::com::sun::star;

using namespace ::xmloff::token;

namespace {

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

}

SvXMLEnumMapEntry<awt::GradientStyle> const pXML_GradientStyle_Enum[] =
{
    { XML_LINEAR,                       awt::GradientStyle_LINEAR },
    { XML_GRADIENTSTYLE_AXIAL,          awt::GradientStyle_AXIAL },
    { XML_GRADIENTSTYLE_RADIAL,         awt::GradientStyle_RADIAL },
    { XML_GRADIENTSTYLE_ELLIPSOID,      awt::GradientStyle_ELLIPTICAL },
    { XML_GRADIENTSTYLE_SQUARE,         awt::GradientStyle_SQUARE },
    { XML_GRADIENTSTYLE_RECTANGULAR,    awt::GradientStyle_RECT },
    { XML_TOKEN_INVALID, awt::GradientStyle(0) }
};

// Import
XMLGradientStyleImport::XMLGradientStyleImport(
    SvXMLImport& rImp )
    : rImport(rImp)
{
}

XMLGradientStyleImport::~XMLGradientStyleImport()
{
}

void XMLGradientStyleImport::importXML(
    const uno::Reference< xml::sax::XFastAttributeList >& xAttrList,
    uno::Any& rValue,
    OUString& rStrName )
{
    OUString aDisplayName;

    awt::Gradient aGradient;
    aGradient.XOffset = 0;
    aGradient.YOffset = 0;
    aGradient.StartIntensity = 100;
    aGradient.EndIntensity = 100;
    aGradient.Angle = 0;
    aGradient.Border = 0;

    for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
    {
        const OUString aStrValue = aIter.toString();

        sal_Int32 nTmpValue;

        switch( aIter.getToken() )
        {
        case XML_ELEMENT(DRAW, XML_NAME):
            rStrName = aStrValue;
            break;
        case XML_ELEMENT(DRAW, XML_DISPLAY_NAME):
            aDisplayName = aStrValue;
            break;
        case XML_ELEMENT(DRAW, XML_STYLE):
            SvXMLUnitConverter::convertEnum( aGradient.Style, aStrValue, pXML_GradientStyle_Enum );
            break;
        case XML_ELEMENT(DRAW, XML_CX):
            ::sax::Converter::convertPercent( nTmpValue, aStrValue );
            aGradient.XOffset = static_cast< sal_Int16 >( nTmpValue );
            break;
        case XML_ELEMENT(DRAW, XML_CY):
            ::sax::Converter::convertPercent( nTmpValue, aStrValue );
            aGradient.YOffset = static_cast< sal_Int16 >( nTmpValue );
            break;
        case XML_ELEMENT(DRAW, XML_START_COLOR):
            ::sax::Converter::convertColor(aGradient.StartColor, aStrValue);
            break;
        case XML_ELEMENT(DRAW, XML_END_COLOR):
            ::sax::Converter::convertColor(aGradient.EndColor, aStrValue);
            break;
        case XML_ELEMENT(DRAW, XML_START_INTENSITY):
            ::sax::Converter::convertPercent( nTmpValue, aStrValue );
            aGradient.StartIntensity = static_cast< sal_Int16 >( nTmpValue );
            break;
        case XML_ELEMENT(DRAW, XML_END_INTENSITY):
            ::sax::Converter::convertPercent( nTmpValue, aStrValue );
            aGradient.EndIntensity = static_cast< sal_Int16 >( nTmpValue );
            break;
        case XML_ELEMENT(DRAW, XML_GRADIENT_ANGLE):
            {
                auto const cmp12(rImport.GetODFVersion().compareTo(ODFVER_012_TEXT));
                bool const bSuccess =
                    ::sax::Converter::convertAngle(aGradient.Angle, aStrValue,
                        // tdf#89475 try to detect borked OOo angles
                        (cmp12 < 0) || (cmp12 == 0
                            && (rImport.isGeneratorVersionOlderThan(SvXMLImport::AOO_4x, SvXMLImport::LO_7x)
                                // also for AOO 4.x, assume there won't ever be a 4.2
                                || rImport.getGeneratorVersion() == SvXMLImport::AOO_4x)));
                SAL_INFO_IF(!bSuccess, "xmloff.style", "failed to import draw:angle");
            }
            break;
        case XML_ELEMENT(DRAW, XML_BORDER):
            ::sax::Converter::convertPercent( nTmpValue, aStrValue );
            aGradient.Border = static_cast< sal_Int16 >( nTmpValue );
            break;

        default:
            SAL_WARN("xmloff.style", "unknown attribute " << SvXMLImport::getPrefixAndNameFromToken(aIter.getToken()) << "=" << aStrValue);
        }
    }

    rValue <<= aGradient;

    if( !aDisplayName.isEmpty() )
    {
        rImport.AddStyleDisplayName( XmlStyleFamily::SD_GRADIENT_ID, rStrName,
                                     aDisplayName );
        rStrName = aDisplayName;
    }
}

// Export

XMLGradientStyleExport::XMLGradientStyleExport(
    SvXMLExport& rExp )
    : rExport(rExp)
{
}

XMLGradientStyleExport::~XMLGradientStyleExport()
{
}

void XMLGradientStyleExport::exportXML(
    const OUString& rStrName,
    const uno::Any& rValue )
{
    awt::Gradient aGradient;

    if( rStrName.isEmpty() )
        return;

    if( !(rValue >>= aGradient) )
        return;

    OUString aStrValue;
    OUStringBuffer aOut;

    // Style
    if( !SvXMLUnitConverter::convertEnum( aOut, aGradient.Style, pXML_GradientStyle_Enum ) )
        return;

    // Name
    bool bEncoded = false;
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

    // Color start
    ::sax::Converter::convertColor(aOut, aGradient.StartColor);
    aStrValue = aOut.makeStringAndClear();
    rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_START_COLOR, aStrValue );

    // Color end
    ::sax::Converter::convertColor(aOut, aGradient.EndColor);
    aStrValue = aOut.makeStringAndClear();
    rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_END_COLOR, aStrValue );

    // Intensity start
    ::sax::Converter::convertPercent(aOut, aGradient.StartIntensity);
    aStrValue = aOut.makeStringAndClear();
    rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_START_INTENSITY, aStrValue );

    // Intensity end
    ::sax::Converter::convertPercent(aOut, aGradient.EndIntensity);
    aStrValue = aOut.makeStringAndClear();
    rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_END_INTENSITY, aStrValue );

    // Angle
    if( aGradient.Style != awt::GradientStyle_RADIAL )
    {
        ::sax::Converter::convertAngle(aOut, aGradient.Angle, rExport.getSaneDefaultVersion());
        aStrValue = aOut.makeStringAndClear();
        rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_GRADIENT_ANGLE, aStrValue );
    }

    // Border
    ::sax::Converter::convertPercent( aOut, aGradient.Border );
    aStrValue = aOut.makeStringAndClear();
    rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_BORDER, aStrValue );

    // Do Write
    SvXMLElementExport aElem( rExport, XML_NAMESPACE_DRAW, XML_GRADIENT,
                          true, false );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
