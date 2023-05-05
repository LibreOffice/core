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

#include <com/sun/star/awt/Gradient2.hpp>

#include <comphelper/documentconstants.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <sax/tools/converter.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmlement.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltkmap.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>

using namespace ::com::sun::star;
using namespace ::xmloff::token;

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

void XMLGradientStyleImport::importXML(
    const uno::Reference< xml::sax::XFastAttributeList >& xAttrList,
    uno::Any& rValue,
    OUString& rStrName )
{
    OUString aDisplayName;

    awt::Gradient2 aGradient;
    aGradient.Style = css::awt::GradientStyle_LINEAR;
    aGradient.StartColor = 0;
    aGradient.EndColor = 0;
    aGradient.Angle = 0;
    aGradient.Border = 0;
    aGradient.XOffset = 0;
    aGradient.YOffset = 0;
    aGradient.StartIntensity = 100;
    aGradient.EndIntensity = 100;
    aGradient.StepCount = 0;

    for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
    {
        sal_Int32 nTmpValue(0);

        switch( aIter.getToken() )
        {
        case XML_ELEMENT(DRAW, XML_NAME):
            rStrName = aIter.toString();
            break;
        case XML_ELEMENT(DRAW, XML_DISPLAY_NAME):
            aDisplayName = aIter.toString();
            break;
        case XML_ELEMENT(DRAW, XML_STYLE):
            SvXMLUnitConverter::convertEnum( aGradient.Style, aIter.toView(), pXML_GradientStyle_Enum );
            break;
        case XML_ELEMENT(DRAW, XML_CX):
            ::sax::Converter::convertPercent( nTmpValue, aIter.toView() );
            aGradient.XOffset = static_cast< sal_Int16 >( nTmpValue );
            break;
        case XML_ELEMENT(DRAW, XML_CY):
            ::sax::Converter::convertPercent( nTmpValue, aIter.toView() );
            aGradient.YOffset = static_cast< sal_Int16 >( nTmpValue );
            break;
        case XML_ELEMENT(DRAW, XML_START_COLOR):
            ::sax::Converter::convertColor(aGradient.StartColor, aIter.toView());
            break;
        case XML_ELEMENT(DRAW, XML_END_COLOR):
            ::sax::Converter::convertColor(aGradient.EndColor, aIter.toView());
            break;
        case XML_ELEMENT(DRAW, XML_START_INTENSITY):
            ::sax::Converter::convertPercent( nTmpValue, aIter.toView() );
            aGradient.StartIntensity = static_cast< sal_Int16 >( nTmpValue );
            break;
        case XML_ELEMENT(DRAW, XML_END_INTENSITY):
            ::sax::Converter::convertPercent( nTmpValue, aIter.toView() );
            aGradient.EndIntensity = static_cast< sal_Int16 >( nTmpValue );
            break;
        case XML_ELEMENT(DRAW, XML_GRADIENT_ANGLE):
            {
                auto const cmp12(rImport.GetODFVersion().compareTo(ODFVER_012_TEXT));
                bool const bSuccess =
                    ::sax::Converter::convertAngle(aGradient.Angle, aIter.toView(),
                        // tdf#89475 try to detect borked OOo angles
                        (cmp12 < 0) || (cmp12 == 0
                            && (rImport.isGeneratorVersionOlderThan(SvXMLImport::AOO_4x, SvXMLImport::LO_7x)
                                // also for AOO 4.x, assume there won't ever be a 4.2
                                || rImport.getGeneratorVersion() == SvXMLImport::AOO_4x)));
                SAL_INFO_IF(!bSuccess, "xmloff.style", "failed to import draw:angle");
            }
            break;
        case XML_ELEMENT(DRAW, XML_BORDER):
            ::sax::Converter::convertPercent( nTmpValue, aIter.toView() );
            aGradient.Border = static_cast< sal_Int16 >( nTmpValue );
            break;

        default:
            XMLOFF_WARN_UNKNOWN("xmloff.style", aIter);
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

XMLGradientStopContext::XMLGradientStopContext(
    SvXMLImport& rImport, sal_Int32 nElement,
    const uno::Reference< xml::sax::XFastAttributeList >& xAttrList,
    std::vector<awt::ColorStop>& rColorStopVec)
:   SvXMLImportContext(rImport)
{
    if(nElement != XML_ELEMENT(LO_EXT, xmloff::token::XML_GRADIENT_STOP))
        return;

    double fOffset = -1.0;
    OUString sColorType;
    OUString sColorValue;
    // First collect all attributes
    for (auto &aIter : sax_fastparser::castToFastAttributeList(xAttrList))
    {
        switch(aIter.getToken())
        {
        case XML_ELEMENT(SVG, xmloff::token::XML_OFFSET): // needed??
        case XML_ELEMENT(SVG_COMPAT, xmloff::token::XML_OFFSET):
            if (!::sax::Converter::convertDouble(fOffset, aIter.toView()))
                return;
            break;
        case XML_ELEMENT(LO_EXT, xmloff::token::XML_COLOR_VALUE):
            sColorValue = aIter.toString();
            if (sColorValue.isEmpty())
                return;
            break;
        case XML_ELEMENT(LO_EXT, xmloff::token::XML_COLOR_TYPE):
            sColorType = aIter.toString();
            if (sColorType.isEmpty())
                return;
            break;
        default:
            XMLOFF_WARN_UNKNOWN("xmloff.style", aIter);
        }
    }

    // As of LO 7.6.0 only "rgb" is implemented.
    if (sColorType != u"rgb")
        return;

    // Type "rgb" requires kind color-value="#rrggbb".
    ::Color aColor;
    if (!::sax::Converter::convertColor(aColor, sColorValue))
        return;

    // All attribute values OK. Generate ColorStop.
    css::rendering::RGBColor aRGBColor;
    aRGBColor.Red = aColor.GetRed() / 255.0;
    aRGBColor.Green = aColor.GetGreen() / 255.0;
    aRGBColor.Blue = aColor.GetBlue() / 255.0;

    awt::ColorStop aColorStop;
    aColorStop.StopOffset = fOffset;
    aColorStop.StopColor = aRGBColor;
    rColorStopVec.push_back(aColorStop);
}

XMLGradientStopContext::~XMLGradientStopContext()
{
}

// Export

XMLGradientStyleExport::XMLGradientStyleExport(
    SvXMLExport& rExp )
    : rExport(rExp)
{
}

void XMLGradientStyleExport::exportXML(
    const OUString& rStrName,
    const uno::Any& rValue )
{
    awt::Gradient2 aGradient;

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

    // ctor writes start tag. End-tag is written by destructor at block end.
    SvXMLElementExport aElem( rExport, XML_NAMESPACE_DRAW, XML_GRADIENT,
                          true, false );

    // Write child elements <loext:gradient-stop>
    // Do not export in standard ODF 1.3 or older.
    if ((rExport.getSaneDefaultVersion() & SvtSaveOptions::ODFSVER_EXTENDED) == 0)
        return;
    sal_Int32 nCount = aGradient.ColorStops.getLength();
    if (nCount == 0)
        return;

    double fPreviousOffset = 0.0;
    for (auto& aCandidate : aGradient.ColorStops)
    {
        // Attribute svg:offset. Make sure offsets are increasing.
        double fOffset = std::clamp<double>(aCandidate.StopOffset, 0.0, 1.0);
        if (fOffset < fPreviousOffset)
            fOffset = fPreviousOffset;
        rExport.AddAttribute(XML_NAMESPACE_SVG, XML_OFFSET, OUString::number(fOffset));
        fPreviousOffset = fOffset;

        // As of LO 7.6.0 only color-type="rgb" is implemented.
        rExport.AddAttribute(XML_NAMESPACE_LO_EXT, XML_COLOR_TYPE, u"rgb");

        // Attribute loext:color-value, data type color, that is #rrggbb.
        rendering::RGBColor aDecimalColor = aCandidate.StopColor;
        ::Color aToolsColor(std::clamp<sal_uInt8>(std::round(aDecimalColor.Red * 255.0), 0, 255),
                            std::clamp<sal_uInt8>(std::round(aDecimalColor.Green * 255.0), 0, 255),
                            std::clamp<sal_uInt8>(std::round(aDecimalColor.Blue * 255.0), 0, 255));
        rExport.AddAttribute(XML_NAMESPACE_LO_EXT, XML_COLOR_VALUE,
                             rtl::OUStringChar('#') + aToolsColor.AsRGBHexString());

        // write gradient stop element
        SvXMLElementExport aStopElement(rExport, XML_NAMESPACE_LO_EXT, XML_GRADIENT_STOP, true, true);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
