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

#include <config_features.h>

#include <config_folders.h>
#include <rtl/bootstrap.hxx>
#include <sal/log.hxx>
#include <oox/core/xmlfilterbase.hxx>
#include <oox/export/drawingml.hxx>
#include <oox/export/utils.hxx>
#include <oox/helper/propertyset.hxx>
#include <oox/drawingml/color.hxx>
#include <drawingml/fillproperties.hxx>
#include <drawingml/textparagraph.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/relationship.hxx>
#include <oox/token/tokens.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <svtools/unitconv.hxx>
#include <sax/fastattribs.hxx>
#include <tools/diagnose_ex.h>
#include <comphelper/processfactory.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/range/b2drange.hxx>

#include <numeric>
#include <string_view>

#include <com/sun/star/awt/CharSet.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/drawing/BitmapMode.hpp>
#include <com/sun/star/drawing/ColorMode.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeAdjustmentValue.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegment.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegmentCommand.hpp>
#include <com/sun/star/drawing/Hatch.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/TextFitToSizeType.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/i18n/BreakIterator.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/style/CaseMap.hpp>
#include <com/sun/star/xml/dom/XNodeList.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/xml/sax/XSAXSerializable.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>

#include <comphelper/random.hxx>
#include <comphelper/seqstream.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/xmltools.hxx>
#include <o3tl/any.hxx>
#include <o3tl/safeint.hxx>
#include <o3tl/unit_conversion.hxx>
#include <tools/stream.hxx>
#include <unotools/fontdefs.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/graph.hxx>
#include <vcl/svapp.hxx>
#include <rtl/strbuf.hxx>
#include <filter/msfilter/escherex.hxx>
#include <filter/msfilter/util.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/svxenum.hxx>
#include <editeng/unonames.hxx>
#include <editeng/unoprnms.hxx>
#include <editeng/flditem.hxx>
#include <svx/svdoashp.hxx>
#include <svx/svdomedia.hxx>
#include <svx/unoapi.hxx>
#include <svx/unoshape.hxx>
#include <svx/EnhancedCustomShape2d.hxx>
#include <drawingml/presetgeometrynames.hxx>

using namespace ::css;
using namespace ::css::beans;
using namespace ::css::drawing;
using namespace ::css::i18n;
using namespace ::css::style;
using namespace ::css::text;
using namespace ::css::uno;
using namespace ::css::container;

using ::css::io::XOutputStream;
using ::sax_fastparser::FSHelperPtr;
using ::sax_fastparser::FastSerializerHelper;

namespace
{
/// Extracts start or end alpha information from a transparency gradient.
sal_Int32 GetAlphaFromTransparenceGradient(const awt::Gradient& rGradient, bool bStart)
{
    // Our alpha is a gray color value.
    sal_uInt8 nRed = ::Color(ColorTransparency, bStart ? rGradient.StartColor : rGradient.EndColor).GetRed();
    // drawingML alpha is a percentage on a 0..100000 scale.
    return (255 - nRed) * oox::drawingml::MAX_PERCENT / 255;
}
}

namespace oox::drawingml {

URLTransformer::~URLTransformer()
{
}

OUString URLTransformer::getTransformedString(const OUString& rString) const
{
    return rString;
}

bool URLTransformer::isExternalURL(const OUString& rURL) const
{
    bool bExternal = true;
    if (rURL.startsWith("#"))
        bExternal = false;
    return bExternal;
}

static css::uno::Any getLineDash( const css::uno::Reference<css::frame::XModel>& xModel, const OUString& rDashName )
    {
        css::uno::Reference<css::lang::XMultiServiceFactory> xFact(xModel, css::uno::UNO_QUERY);
        css::uno::Reference<css::container::XNameAccess> xNameAccess(
            xFact->createInstance("com.sun.star.drawing.DashTable"),
            css::uno::UNO_QUERY );
        if(xNameAccess.is())
        {
            if (!xNameAccess->hasByName(rDashName))
                return css::uno::Any();

            return xNameAccess->getByName(rDashName);
        }

        return css::uno::Any();
    }

namespace
{
void WriteGradientPath(const awt::Gradient& rGradient, const FSHelperPtr& pFS, const bool bCircle)
{
    pFS->startElementNS(XML_a, XML_path, XML_path, bCircle ? "circle" : "rect");

    // Write the focus rectangle. Work with the focus point, and assume
    // that it extends 50% in all directions.  The below
    // left/top/right/bottom values are percentages, where 0 means the
    // edge of the tile rectangle and 100% means the center of it.
    rtl::Reference<sax_fastparser::FastAttributeList> pAttributeList(
        sax_fastparser::FastSerializerHelper::createAttrList());
    sal_Int32 nLeftPercent = rGradient.XOffset;
    pAttributeList->add(XML_l, OString::number(nLeftPercent * PER_PERCENT));
    sal_Int32 nTopPercent = rGradient.YOffset;
    pAttributeList->add(XML_t, OString::number(nTopPercent * PER_PERCENT));
    sal_Int32 nRightPercent = 100 - rGradient.XOffset;
    pAttributeList->add(XML_r, OString::number(nRightPercent * PER_PERCENT));
    sal_Int32 nBottomPercent = 100 - rGradient.YOffset;
    pAttributeList->add(XML_b, OString::number(nBottomPercent * PER_PERCENT));
    pFS->singleElementNS(XML_a, XML_fillToRect, pAttributeList);

    pFS->endElementNS(XML_a, XML_path);
}
}

// not thread safe
int DrawingML::mnImageCounter = 1;
int DrawingML::mnWdpImageCounter = 1;
std::map<OUString, OUString> DrawingML::maWdpCache;
sal_Int32 DrawingML::mnDrawingMLCount = 0;
sal_Int32 DrawingML::mnVmlCount = 0;

sal_Int16 DrawingML::GetScriptType(const OUString& rStr)
{
    if (rStr.getLength() > 0)
    {
        static Reference<css::i18n::XBreakIterator> xBreakIterator =
            css::i18n::BreakIterator::create(comphelper::getProcessComponentContext());

        sal_Int16 nScriptType = xBreakIterator->getScriptType(rStr, 0);

        if (nScriptType == css::i18n::ScriptType::WEAK)
        {
            sal_Int32 nPos = xBreakIterator->nextScript(rStr, 0, nScriptType);
            if (nPos < rStr.getLength())
                nScriptType = xBreakIterator->getScriptType(rStr, nPos);

        }

        if (nScriptType != css::i18n::ScriptType::WEAK)
            return nScriptType;
    }

    return css::i18n::ScriptType::LATIN;
}

void DrawingML::ResetCounters()
{
    mnImageCounter = 1;
    mnWdpImageCounter = 1;
    maWdpCache.clear();
}

void DrawingML::ResetMlCounters()
{
    mnDrawingMLCount = 0;
    mnVmlCount = 0;
}

bool DrawingML::GetProperty( const Reference< XPropertySet >& rXPropertySet, const OUString& aName )
{
    try
    {
        mAny = rXPropertySet->getPropertyValue(aName);
        if (mAny.hasValue())
            return true;
    }
    catch( const Exception& )
    {
        /* printf ("exception when trying to get value of property: %s\n", aName.toUtf8()); */
    }
    return false;
}

bool DrawingML::GetPropertyAndState( const Reference< XPropertySet >& rXPropertySet, const Reference< XPropertyState >& rXPropertyState, const OUString& aName, PropertyState& eState )
{
    try
    {
        mAny = rXPropertySet->getPropertyValue(aName);
        if (mAny.hasValue())
        {
            eState = rXPropertyState->getPropertyState(aName);
            return true;
        }
    }
    catch( const Exception& )
    {
        /* printf ("exception when trying to get value of property: %s\n", aName.toUtf8()); */
    }
    return false;
}

namespace
{
/// Gets hexa value of color on string format.
OString getColorStr(const ::Color nColor)
{
    // Transparency is a separate element.
    OString sColor = OString::number(sal_uInt32(nColor) & 0x00FFFFFF, 16);
    if (sColor.getLength() < 6)
    {
        OStringBuffer sBuf("0");
        int remains = 5 - sColor.getLength();

        while (remains > 0)
        {
            sBuf.append("0");
            remains--;
        }

        sBuf.append(sColor);

        sColor = sBuf.getStr();
    }
    return sColor;
}
}

void DrawingML::WriteColor( ::Color nColor, sal_Int32 nAlpha )
{
    const auto sColor = getColorStr(nColor);
    if( nAlpha < MAX_PERCENT )
    {
        mpFS->startElementNS(XML_a, XML_srgbClr, XML_val, sColor);
        mpFS->singleElementNS(XML_a, XML_alpha, XML_val, OString::number(nAlpha));
        mpFS->endElementNS( XML_a, XML_srgbClr );

    }
    else
    {
        mpFS->singleElementNS(XML_a, XML_srgbClr, XML_val, sColor);
    }
}

void DrawingML::WriteColor( const OUString& sColorSchemeName, const Sequence< PropertyValue >& aTransformations, sal_Int32 nAlpha )
{
    // prevent writing a tag with empty val attribute
    if( sColorSchemeName.isEmpty() )
        return;

    if( aTransformations.hasElements() )
    {
        mpFS->startElementNS(XML_a, XML_schemeClr, XML_val, sColorSchemeName);
        WriteColorTransformations( aTransformations, nAlpha );
        mpFS->endElementNS( XML_a, XML_schemeClr );
    }
    else if(nAlpha < MAX_PERCENT)
    {
        mpFS->startElementNS(XML_a, XML_schemeClr, XML_val, sColorSchemeName);
        mpFS->singleElementNS(XML_a, XML_alpha, XML_val, OString::number(nAlpha));
        mpFS->endElementNS( XML_a, XML_schemeClr );
    }
    else
    {
        mpFS->singleElementNS(XML_a, XML_schemeClr, XML_val, sColorSchemeName);
    }
}

void DrawingML::WriteColor( const ::Color nColor, const Sequence< PropertyValue >& aTransformations, sal_Int32 nAlpha )
{
    const auto sColor = getColorStr(nColor);
    if( aTransformations.hasElements() )
    {
        mpFS->startElementNS(XML_a, XML_srgbClr, XML_val, sColor);
        WriteColorTransformations(aTransformations, nAlpha);
        mpFS->endElementNS(XML_a, XML_srgbClr);
    }
    else if(nAlpha < MAX_PERCENT)
    {
        mpFS->startElementNS(XML_a, XML_srgbClr, XML_val, sColor);
        mpFS->singleElementNS(XML_a, XML_alpha, XML_val, OString::number(nAlpha));
        mpFS->endElementNS(XML_a, XML_srgbClr);
    }
    else
    {
        mpFS->singleElementNS(XML_a, XML_srgbClr, XML_val, sColor);
    }
}

void DrawingML::WriteColorTransformations( const Sequence< PropertyValue >& aTransformations, sal_Int32 nAlpha )
{
    for( const auto& rTransformation : aTransformations )
    {
        sal_Int32 nToken = Color::getColorTransformationToken( rTransformation.Name );
        if( nToken != XML_TOKEN_INVALID && rTransformation.Value.hasValue() )
        {
            if(nToken == XML_alpha && nAlpha < MAX_PERCENT)
            {
                mpFS->singleElementNS(XML_a, nToken, XML_val, OString::number(nAlpha));
            }
            else
            {
                sal_Int32 nValue = rTransformation.Value.get<sal_Int32>();
                mpFS->singleElementNS(XML_a, nToken, XML_val, OString::number(nValue));
            }
        }
    }
}

void DrawingML::WriteSolidFill( ::Color nColor, sal_Int32 nAlpha )
{
    mpFS->startElementNS(XML_a, XML_solidFill);
    WriteColor( nColor, nAlpha );
    mpFS->endElementNS( XML_a, XML_solidFill );
}

void DrawingML::WriteSolidFill( const OUString& sSchemeName, const Sequence< PropertyValue >& aTransformations, sal_Int32 nAlpha )
{
    mpFS->startElementNS(XML_a, XML_solidFill);
    WriteColor( sSchemeName, aTransformations, nAlpha );
    mpFS->endElementNS( XML_a, XML_solidFill );
}

void DrawingML::WriteSolidFill( const ::Color nColor, const Sequence< PropertyValue >& aTransformations, sal_Int32 nAlpha )
{
    mpFS->startElementNS(XML_a, XML_solidFill);
    WriteColor(nColor, aTransformations, nAlpha);
    mpFS->endElementNS(XML_a, XML_solidFill);
}

void DrawingML::WriteSolidFill( const Reference< XPropertySet >& rXPropSet )
{
    // get fill color
    if ( !GetProperty( rXPropSet, "FillColor" ) )
        return;
    sal_uInt32 nFillColor = mAny.get<sal_uInt32>();

    // get InteropGrabBag and search the relevant attributes
    OUString sColorFillScheme;
    sal_uInt32 nOriginalColor = 0;
    Sequence< PropertyValue > aStyleProperties, aTransformations;
    if ( GetProperty( rXPropSet, "InteropGrabBag" ) )
    {
        Sequence< PropertyValue > aGrabBag;
        mAny >>= aGrabBag;
        for( const auto& rProp : std::as_const(aGrabBag) )
        {
            if( rProp.Name == "SpPrSolidFillSchemeClr" )
                rProp.Value >>= sColorFillScheme;
            else if( rProp.Name == "OriginalSolidFillClr" )
                rProp.Value >>= nOriginalColor;
            else if( rProp.Name == "StyleFillRef" )
                rProp.Value >>= aStyleProperties;
            else if( rProp.Name == "SpPrSolidFillSchemeClrTransformations" )
                rProp.Value >>= aTransformations;
        }
    }

    sal_Int32 nAlpha = MAX_PERCENT;
    if( GetProperty( rXPropSet, "FillTransparence" ) )
    {
        sal_Int32 nTransparency = 0;
        mAny >>= nTransparency;
        // Calculate alpha value (see oox/source/drawingml/color.cxx : getTransparency())
        nAlpha = (MAX_PERCENT - ( PER_PERCENT * nTransparency ) );
    }

    // OOXML has no separate transparence gradient but uses transparency in the gradient stops.
    // So we merge transparency and color and use gradient fill in such case.
    awt::Gradient aTransparenceGradient;
    bool bNeedGradientFill(false);
    if (GetProperty(rXPropSet, "FillTransparenceGradient"))
    {
        mAny >>= aTransparenceGradient;
        if (aTransparenceGradient.StartColor != aTransparenceGradient.EndColor)
            bNeedGradientFill = true;
        else if (aTransparenceGradient.StartColor != 0)
            nAlpha = GetAlphaFromTransparenceGradient(aTransparenceGradient, true);
    }

    // write XML
    if (bNeedGradientFill)
    {
        awt::Gradient aPseudoColorGradient;
        aPseudoColorGradient.XOffset = aTransparenceGradient.XOffset;
        aPseudoColorGradient.YOffset = aTransparenceGradient.YOffset;
        aPseudoColorGradient.StartIntensity = 100;
        aPseudoColorGradient.EndIntensity = 100;
        aPseudoColorGradient.Angle = aTransparenceGradient.Angle;
        aPseudoColorGradient.Border = aTransparenceGradient.Border;
        aPseudoColorGradient.Style = aTransparenceGradient.Style;
        aPseudoColorGradient.StartColor = nFillColor;
        aPseudoColorGradient.EndColor = nFillColor;
        aPseudoColorGradient.StepCount = aTransparenceGradient.StepCount;
        mpFS->startElementNS(XML_a, XML_gradFill, XML_rotWithShape, "0");
        WriteGradientFill(aPseudoColorGradient, aTransparenceGradient);
        mpFS->endElementNS( XML_a, XML_gradFill );
    }
    else if ( nFillColor != nOriginalColor )
    {
        // the user has set a different color for the shape
        WriteSolidFill( ::Color(ColorTransparency, nFillColor & 0xffffff), nAlpha );
    }
    else if ( !sColorFillScheme.isEmpty() )
    {
        // the shape had a scheme color and the user didn't change it
        WriteSolidFill( sColorFillScheme, aTransformations, nAlpha );
    }
    else
    {
        // the shape had a custom color and the user didn't change it
        // tdf#124013
        WriteSolidFill( ::Color(ColorTransparency, nFillColor & 0xffffff), nAlpha );
    }
}

void DrawingML::WriteGradientStop(sal_uInt16 nStop, ::Color nColor, sal_Int32 nAlpha)
{
    mpFS->startElementNS(XML_a, XML_gs, XML_pos, OString::number(nStop * 1000));
    WriteColor(nColor, nAlpha);
    mpFS->endElementNS( XML_a, XML_gs );
}

::Color DrawingML::ColorWithIntensity( sal_uInt32 nColor, sal_uInt32 nIntensity )
{
    return ::Color(ColorTransparency, ( ( ( nColor & 0xff ) * nIntensity ) / 100 )
        | ( ( ( ( ( nColor & 0xff00 ) >> 8 ) * nIntensity ) / 100 ) << 8 )
        | ( ( ( ( ( nColor & 0xff0000 ) >> 8 ) * nIntensity ) / 100 ) << 8 ));
}

bool DrawingML::EqualGradients( awt::Gradient aGradient1, awt::Gradient aGradient2 )
{
    return aGradient1.Style == aGradient2.Style &&
            aGradient1.StartColor == aGradient2.StartColor &&
            aGradient1.EndColor == aGradient2.EndColor &&
            aGradient1.Angle == aGradient2.Angle &&
            aGradient1.Border == aGradient2.Border &&
            aGradient1.XOffset == aGradient2.XOffset &&
            aGradient1.YOffset == aGradient2.YOffset &&
            aGradient1.StartIntensity == aGradient2.StartIntensity &&
            aGradient1.EndIntensity == aGradient2.EndIntensity &&
            aGradient1.StepCount == aGradient2.StepCount;
}

void DrawingML::WriteGradientFill( const Reference< XPropertySet >& rXPropSet )
{
    awt::Gradient aGradient;
    if (!GetProperty(rXPropSet, "FillGradient"))
        return;

    aGradient = *o3tl::doAccess<awt::Gradient>(mAny);

    // get InteropGrabBag and search the relevant attributes
    awt::Gradient aOriginalGradient;
    Sequence< PropertyValue > aGradientStops;
    if ( GetProperty( rXPropSet, "InteropGrabBag" ) )
    {
        Sequence< PropertyValue > aGrabBag;
        mAny >>= aGrabBag;
        for( const auto& rProp : std::as_const(aGrabBag) )
            if( rProp.Name == "GradFillDefinition" )
                rProp.Value >>= aGradientStops;
            else if( rProp.Name == "OriginalGradFill" )
                rProp.Value >>= aOriginalGradient;
    }

    // check if an ooxml gradient had been imported and if the user has modified it
    // Gradient grab-bag depends on theme grab-bag, which is implemented
    // only for DOCX.
    if( EqualGradients( aOriginalGradient, aGradient ) && GetDocumentType() == DOCUMENT_DOCX)
    {
        // If we have no gradient stops that means original gradient were defined by a theme.
        if( aGradientStops.hasElements() )
        {
            mpFS->startElementNS(XML_a, XML_gradFill, XML_rotWithShape, "0");
            WriteGrabBagGradientFill(aGradientStops, aGradient);
            mpFS->endElementNS( XML_a, XML_gradFill );
        }
    }
    else
    {
        awt::Gradient aTransparenceGradient;
        mpFS->startElementNS(XML_a, XML_gradFill, XML_rotWithShape, "0");
        OUString sFillTransparenceGradientName;
        if (GetProperty(rXPropSet, "FillTransparenceGradientName")
            && (mAny >>= sFillTransparenceGradientName)
            && !sFillTransparenceGradientName.isEmpty())
        {
            if (GetProperty(rXPropSet, "FillTransparenceGradient"))
                aTransparenceGradient = *o3tl::doAccess<awt::Gradient>(mAny);
        }
        else if (GetProperty(rXPropSet, "FillTransparence"))
        {
            // currently only StartColor and EndColor are evaluated in WriteGradientFill()
            sal_Int32 nTransparency = 0;
            mAny >>= nTransparency;
            // convert percent to gray color
            nTransparency = nTransparency * 255/100;
            const sal_Int32 aGrayColor = static_cast<sal_Int32>( nTransparency | nTransparency << 8 | nTransparency << 16 );
            aTransparenceGradient.StartColor = aGrayColor;
            aTransparenceGradient.EndColor = aGrayColor;
        }
        WriteGradientFill(aGradient, aTransparenceGradient);
        mpFS->endElementNS(XML_a, XML_gradFill);
    }
}

void DrawingML::WriteGrabBagGradientFill( const Sequence< PropertyValue >& aGradientStops, awt::Gradient rGradient )
{
    // write back the original gradient
    mpFS->startElementNS(XML_a, XML_gsLst);

    // get original stops and write them
    for( const auto& rGradientStop : aGradientStops )
    {
        Sequence< PropertyValue > aGradientStop;
        rGradientStop.Value >>= aGradientStop;

        // get values
        OUString sSchemeClr;
        double nPos = 0;
        sal_Int16 nTransparency = 0;
        ::Color nRgbClr;
        Sequence< PropertyValue > aTransformations;
        for( const auto& rProp : std::as_const(aGradientStop) )
        {
            if( rProp.Name == "SchemeClr" )
                rProp.Value >>= sSchemeClr;
            else if( rProp.Name == "RgbClr" )
                rProp.Value >>= nRgbClr;
            else if( rProp.Name == "Pos" )
                rProp.Value >>= nPos;
            else if( rProp.Name == "Transparency" )
                rProp.Value >>= nTransparency;
            else if( rProp.Name == "Transformations" )
                rProp.Value >>= aTransformations;
        }
        // write stop
        mpFS->startElementNS(XML_a, XML_gs, XML_pos, OString::number(nPos * 100000.0).getStr());
        if( sSchemeClr.isEmpty() )
        {
            // Calculate alpha value (see oox/source/drawingml/color.cxx : getTransparency())
            sal_Int32 nAlpha = MAX_PERCENT - ( PER_PERCENT * nTransparency );
            WriteColor( nRgbClr, nAlpha );
        }
        else
        {
            WriteColor( sSchemeClr, aTransformations );
        }
        mpFS->endElementNS( XML_a, XML_gs );
    }
    mpFS->endElementNS( XML_a, XML_gsLst );

    switch (rGradient.Style)
    {
        default:
            mpFS->singleElementNS(
                XML_a, XML_lin, XML_ang,
                OString::number(((3600 - rGradient.Angle + 900) * 6000) % 21600000));
            break;
        case awt::GradientStyle_RADIAL:
            WriteGradientPath(rGradient, mpFS, true);
            break;
    }
}

void DrawingML::WriteGradientFill(awt::Gradient rGradient, awt::Gradient rTransparenceGradient,
                                  const uno::Reference<beans::XPropertySet>& rXPropSet)
{
    sal_Int32 nStartAlpha;
    sal_Int32 nEndAlpha;
    if( rXPropSet.is() && GetProperty(rXPropSet, "FillTransparence") )
    {
        sal_Int32 nTransparency = 0;
        mAny >>= nTransparency;
        nStartAlpha = nEndAlpha = (MAX_PERCENT - (PER_PERCENT * nTransparency));
    }
    else
    {
        nStartAlpha = GetAlphaFromTransparenceGradient(rTransparenceGradient, true);
        nEndAlpha = GetAlphaFromTransparenceGradient(rTransparenceGradient, false);
    }
    switch( rGradient.Style )
    {
        default:
        case awt::GradientStyle_LINEAR:
        {
            mpFS->startElementNS(XML_a, XML_gsLst);
            WriteGradientStop(rGradient.Border, ColorWithIntensity(rGradient.StartColor, rGradient.StartIntensity),
                              nStartAlpha);
            WriteGradientStop(100, ColorWithIntensity(rGradient.EndColor, rGradient.EndIntensity),
                              nEndAlpha);
            mpFS->endElementNS( XML_a, XML_gsLst );
            mpFS->singleElementNS(
                XML_a, XML_lin, XML_ang,
                OString::number(((3600 - rGradient.Angle + 900) * 6000) % 21600000));
            break;
        }

        case awt::GradientStyle_AXIAL:
        {
            mpFS->startElementNS(XML_a, XML_gsLst);
            WriteGradientStop(0, ColorWithIntensity(rGradient.EndColor, rGradient.EndIntensity),
                              nEndAlpha);
            if (rGradient.Border > 0 && rGradient.Border < 100)
            {
                WriteGradientStop(rGradient.Border/2,
                                  ColorWithIntensity(rGradient.EndColor, rGradient.EndIntensity),
                                  nEndAlpha);
            }
            WriteGradientStop(50, ColorWithIntensity(rGradient.StartColor, rGradient.StartIntensity),
                              nStartAlpha);
            if (rGradient.Border > 0 && rGradient.Border < 100)
            {
                WriteGradientStop(100 - rGradient.Border/2,
                                  ColorWithIntensity(rGradient.EndColor, rGradient.EndIntensity),
                                  nEndAlpha);
            }
            WriteGradientStop(100, ColorWithIntensity(rGradient.EndColor, rGradient.EndIntensity),
                              nEndAlpha);
            mpFS->endElementNS(XML_a, XML_gsLst);
            mpFS->singleElementNS(
                XML_a, XML_lin, XML_ang,
                OString::number(((3600 - rGradient.Angle + 900) * 6000) % 21600000));
            break;
        }

        case awt::GradientStyle_RADIAL:
        case awt::GradientStyle_ELLIPTICAL:
        case awt::GradientStyle_RECT:
        case awt::GradientStyle_SQUARE:
        {
            mpFS->startElementNS(XML_a, XML_gsLst);
            WriteGradientStop(0, ColorWithIntensity(rGradient.EndColor, rGradient.EndIntensity),
                              nEndAlpha);
            if (rGradient.Border > 0 && rGradient.Border < 100)
            {
                // Map border to an additional gradient stop, which has the
                // same color as the final stop.
                WriteGradientStop(100 - rGradient.Border,
                                  ColorWithIntensity(rGradient.StartColor, rGradient.StartIntensity),
                                  nStartAlpha);
            }
            WriteGradientStop(100,
                              ColorWithIntensity(rGradient.StartColor, rGradient.StartIntensity),
                              nStartAlpha);
            mpFS->endElementNS(XML_a, XML_gsLst);

            WriteGradientPath(rGradient, mpFS, rGradient.Style == awt::GradientStyle_RADIAL || rGradient.Style == awt::GradientStyle_ELLIPTICAL);
            break;
        }
    }
}

void DrawingML::WriteLineArrow( const Reference< XPropertySet >& rXPropSet, bool bLineStart )
{
    ESCHER_LineEnd eLineEnd;
    sal_Int32 nArrowLength;
    sal_Int32 nArrowWidth;

    if ( !EscherPropertyContainer::GetLineArrow( bLineStart, rXPropSet, eLineEnd, nArrowLength, nArrowWidth ) )
        return;

    const char* len;
    const char* type;
    const char* width;

    switch( nArrowLength )
    {
        case ESCHER_LineShortArrow:
            len = "sm";
            break;
        default:
        case ESCHER_LineMediumLenArrow:
            len = "med";
            break;
        case ESCHER_LineLongArrow:
            len = "lg";
            break;
    }

    switch( eLineEnd )
    {
        default:
        case ESCHER_LineNoEnd:
            type = "none";
            break;
        case ESCHER_LineArrowEnd:
            type = "triangle";
            break;
        case ESCHER_LineArrowStealthEnd:
            type = "stealth";
            break;
        case ESCHER_LineArrowDiamondEnd:
            type = "diamond";
            break;
        case ESCHER_LineArrowOvalEnd:
            type = "oval";
            break;
        case ESCHER_LineArrowOpenEnd:
            type = "arrow";
            break;
    }

    switch( nArrowWidth )
    {
        case ESCHER_LineNarrowArrow:
            width = "sm";
            break;
        default:
        case ESCHER_LineMediumWidthArrow:
            width = "med";
            break;
        case ESCHER_LineWideArrow:
            width = "lg";
            break;
    }

    mpFS->singleElementNS( XML_a, bLineStart ? XML_headEnd : XML_tailEnd,
                           XML_len, len,
                           XML_type, type,
                           XML_w, width );
}

void DrawingML::WriteOutline( const Reference<XPropertySet>& rXPropSet, Reference< frame::XModel > const & xModel )
{
    drawing::LineStyle aLineStyle( drawing::LineStyle_NONE );
    if (GetProperty(rXPropSet, "LineStyle"))
        mAny >>= aLineStyle;

    const LineCap aLineCap = GetProperty(rXPropSet, "LineCap") ? mAny.get<drawing::LineCap>() : LineCap_BUTT;

    sal_uInt32 nLineWidth = 0;
    sal_uInt32 nEmuLineWidth = 0;
    ::Color nColor;
    sal_Int32 nColorAlpha = MAX_PERCENT;
    bool bColorSet = false;
    const char* cap = nullptr;
    drawing::LineDash aLineDash;
    bool bDashSet = false;
    bool bNoFill = false;


    // get InteropGrabBag and search the relevant attributes
    OUString sColorFillScheme;
    ::Color aResolvedColorFillScheme;

    ::Color nOriginalColor;
    ::Color nStyleColor;
    sal_uInt32 nStyleLineWidth = 0;

    Sequence<PropertyValue> aStyleProperties;
    Sequence<PropertyValue> aTransformations;

    drawing::LineStyle aStyleLineStyle(drawing::LineStyle_NONE);
    drawing::LineJoint aStyleLineJoint(drawing::LineJoint_NONE);

    if (GetProperty(rXPropSet, "InteropGrabBag"))
    {
        Sequence<PropertyValue> aGrabBag;
        mAny >>= aGrabBag;

        for (const auto& rProp : std::as_const(aGrabBag))
        {
            if( rProp.Name == "SpPrLnSolidFillSchemeClr" )
                rProp.Value >>= sColorFillScheme;
            if( rProp.Name == "SpPrLnSolidFillResolvedSchemeClr" )
                rProp.Value >>= aResolvedColorFillScheme;
            else if( rProp.Name == "OriginalLnSolidFillClr" )
                rProp.Value >>= nOriginalColor;
            else if( rProp.Name == "StyleLnRef" )
                rProp.Value >>= aStyleProperties;
            else if( rProp.Name == "SpPrLnSolidFillSchemeClrTransformations" )
                rProp.Value >>= aTransformations;
            else if( rProp.Name == "EmuLineWidth" )
                rProp.Value >>= nEmuLineWidth;
        }
        for (const auto& rStyleProp : std::as_const(aStyleProperties))
        {
            if( rStyleProp.Name == "Color" )
                rStyleProp.Value >>= nStyleColor;
            else if( rStyleProp.Name == "LineStyle" )
                rStyleProp.Value >>= aStyleLineStyle;
            else if( rStyleProp.Name == "LineJoint" )
                rStyleProp.Value >>= aStyleLineJoint;
            else if( rStyleProp.Name == "LineWidth" )
                rStyleProp.Value >>= nStyleLineWidth;
        }
    }

    if (GetProperty(rXPropSet, "LineWidth"))
        mAny >>= nLineWidth;

    switch (aLineStyle)
    {
        case drawing::LineStyle_NONE:
            bNoFill = true;
            break;
        case drawing::LineStyle_DASH:
            if (GetProperty(rXPropSet, "LineDash"))
            {
                aLineDash = mAny.get<drawing::LineDash>();
                //this query is good for shapes, but in the case of charts it returns 0 values
                if (aLineDash.Dots == 0 && aLineDash.DotLen == 0 && aLineDash.Dashes == 0 && aLineDash.DashLen == 0 && aLineDash.Distance == 0) {
                    OUString aLineDashName;
                    if (GetProperty(rXPropSet, "LineDashName"))
                        mAny >>= aLineDashName;
                    if (!aLineDashName.isEmpty() && xModel) {
                        css::uno::Any aAny = getLineDash(xModel, aLineDashName);
                        aAny >>= aLineDash;
                    }
                }
            }
            else
            {
                //export the linestyle of chart wall (plot area) and chart page
                OUString aLineDashName;
                if (GetProperty(rXPropSet, "LineDashName"))
                    mAny >>= aLineDashName;
                if (!aLineDashName.isEmpty() && xModel) {
                    css::uno::Any aAny = getLineDash(xModel, aLineDashName);
                    aAny >>= aLineDash;
                }
            }
            bDashSet = true;
            if (aLineDash.Style == DashStyle_ROUND || aLineDash.Style == DashStyle_ROUNDRELATIVE)
            {
                cap = "rnd";
            }

            SAL_INFO("oox.shape", "dash dots: " << aLineDash.Dots << " dashes: " << aLineDash.Dashes
                    << " dotlen: " << aLineDash.DotLen << " dashlen: " << aLineDash.DashLen << " distance: " <<  aLineDash.Distance);

            [[fallthrough]];
        case drawing::LineStyle_SOLID:
        default:
            if (GetProperty(rXPropSet, "LineColor"))
            {
                nColor = ::Color(ColorTransparency, mAny.get<sal_uInt32>() & 0xffffff);
                bColorSet = true;
            }
            if (GetProperty(rXPropSet, "LineTransparence"))
            {
                nColorAlpha = MAX_PERCENT - (mAny.get<sal_Int16>() * PER_PERCENT);
            }
            if (aLineCap == LineCap_ROUND)
                cap = "rnd";
            else if (aLineCap == LineCap_SQUARE)
                 cap = "sq";
            break;
    }

    // if the line-width was not modified after importing then the original EMU value will be exported to avoid unexpected conversion (rounding) error
    if (nEmuLineWidth == 0 || static_cast<sal_uInt32>(oox::drawingml::convertEmuToHmm(nEmuLineWidth)) != nLineWidth)
        nEmuLineWidth = oox::drawingml::convertHmmToEmu(nLineWidth);
    mpFS->startElementNS( XML_a, XML_ln,
                          XML_cap, cap,
                          XML_w, sax_fastparser::UseIf(OString::number(nEmuLineWidth),
                              nLineWidth == 0 || (nLineWidth > 1 && nStyleLineWidth != nLineWidth)) );

    if( bColorSet )
    {
        if( nColor != nOriginalColor )
        {
            // the user has set a different color for the line
            WriteSolidFill( nColor, nColorAlpha );
        }
        else if( !sColorFillScheme.isEmpty() )
        {
            // the line had a scheme color and the user didn't change it
            WriteSolidFill( aResolvedColorFillScheme, aTransformations );
        }
        else
        {
            WriteSolidFill( nColor, nColorAlpha );
        }
    }

    if( bDashSet && aStyleLineStyle != drawing::LineStyle_DASH )
    {
        // Try to detect if it might come from ms preset line style import.
        // MS Office styles are always relative, both binary and OOXML.
        // "dot" is always the first dash and "dash" the second one. All OOXML presets linestyles
        // start with the longer one. Definitions are in OOXML part 1, 20.1.10.49
        // The tests are strict, for to not catch styles from standard.sod (as of Aug 2019).
        bool bIsConverted = false;

        bool bIsRelative(aLineDash.Style == DashStyle_RECTRELATIVE || aLineDash.Style == DashStyle_ROUNDRELATIVE);
        if ( bIsRelative && aLineDash.Dots == 1)
        {   // The length were tweaked on import in case of prstDash. Revert it here.
            sal_uInt32 nDotLen = aLineDash.DotLen;
            sal_uInt32 nDashLen = aLineDash.DashLen;
            sal_uInt32 nDistance = aLineDash.Distance;
            if (aLineCap != LineCap_BUTT && nDistance >= 99)
            {
                nDistance -= 99;
                nDotLen += 99;
                if (nDashLen > 0)
                    nDashLen += 99;
            }
            // LO uses length 0 for 100%, if the attribute is missing in ODF.
            // Other applications might write 100%. Make is unique for the conditions.
            if (nDotLen == 0)
                nDotLen = 100;
            if (nDashLen == 0 && aLineDash.Dashes > 0)
                nDashLen = 100;
            bIsConverted = true;
            if (nDotLen == 100 && aLineDash.Dashes == 0 && nDashLen == 0 && nDistance == 300)
            {
                mpFS->singleElementNS(XML_a, XML_prstDash, XML_val, "dot");
            }
            else if (nDotLen == 400 && aLineDash.Dashes == 0 && nDashLen == 0 && nDistance == 300)
            {
                mpFS->singleElementNS(XML_a, XML_prstDash, XML_val, "dash");
            }
            else if (nDotLen == 400 && aLineDash.Dashes == 1 && nDashLen == 100 && nDistance == 300)
            {
                mpFS->singleElementNS(XML_a, XML_prstDash, XML_val, "dashDot");
            }
            else if (nDotLen == 800 && aLineDash.Dashes == 0 && nDashLen == 0 && nDistance == 300)
            {
                mpFS->singleElementNS(XML_a, XML_prstDash, XML_val, "lgDash");
            }
            else if (nDotLen == 800 && aLineDash.Dashes == 1 && nDashLen == 100 && nDistance == 300)
            {
                mpFS->singleElementNS(XML_a, XML_prstDash, XML_val, "lgDashDot");
            }
            else if (nDotLen == 800 && aLineDash.Dashes == 2 && nDashLen == 100 && nDistance == 300)
            {
                mpFS->singleElementNS(XML_a, XML_prstDash, XML_val, "lgDashDotDot");
            }
            else if (nDotLen == 100 && aLineDash.Dashes == 0 && nDashLen == 0 && nDistance == 100)
            {
                mpFS->singleElementNS(XML_a, XML_prstDash, XML_val, "sysDot");
            }
            else if (nDotLen == 300 && aLineDash.Dashes == 0 && nDashLen == 0 && nDistance == 100)
            {
                mpFS->singleElementNS(XML_a, XML_prstDash, XML_val, "sysDash");
            }
            else if (nDotLen == 300 && aLineDash.Dashes == 1 && nDashLen == 100 && nDistance == 100)
            {
                mpFS->singleElementNS(XML_a, XML_prstDash, XML_val, "sysDashDot");
            }
            else if (nDotLen == 300 && aLineDash.Dashes == 2 && nDashLen == 100 && nDistance == 100)
            {
                mpFS->singleElementNS(XML_a, XML_prstDash, XML_val, "sysDashDotDot");
            }
            else
                bIsConverted = false;
        }
        // Do not map our own line styles to OOXML prstDash values, because custDash gives better results.
        if (!bIsConverted)
        {
            mpFS->startElementNS(XML_a, XML_custDash);
            // In case of hairline we would need the current pixel size. Instead use a reasonable
            // ersatz for it. The value is the same as SMALLEST_DASH_WIDTH in xattr.cxx.
            // (And it makes sure fLineWidth is not zero in below division.)
            double fLineWidth = nLineWidth > 0 ? nLineWidth : 26.95;
            int i;
            double fSp = bIsRelative ? aLineDash.Distance : aLineDash.Distance * 100.0 / fLineWidth;
            // LO uses line width, in case Distance is zero. MS Office would use a space of zero length.
            // So set 100% explicitly.
            if (aLineDash.Distance <= 0)
                    fSp = 100.0;
            // In case of custDash, round caps are included in dash length in MS Office. Square caps are added
            // to dash length, same as in ODF. Change the length values accordingly.
            if (aLineCap == LineCap_ROUND && fSp > 99.0)
                fSp -= 99.0;

            if (aLineDash.Dots > 0)
            {
                double fD = bIsRelative ? aLineDash.DotLen : aLineDash.DotLen * 100.0 / fLineWidth;
                // LO sets length to 0, if attribute is missing in ODF. Then a relative length of 100% is intended.
                if (aLineDash.DotLen == 0)
                    fD = 100.0;
                // Tweak dash length, see above.
                if (aLineCap == LineCap_ROUND && fSp > 99.0)
                    fD += 99.0;

                for( i = 0; i < aLineDash.Dots; i ++ )
                {
                    mpFS->singleElementNS( XML_a, XML_ds,
                                           XML_d , write1000thOfAPercent(fD),
                                           XML_sp, write1000thOfAPercent(fSp) );
                }
            }
            if ( aLineDash.Dashes > 0 )
            {
                double fD = bIsRelative ? aLineDash.DashLen : aLineDash.DashLen * 100.0 / fLineWidth;
                // LO sets length to 0, if attribute is missing in ODF. Then a relative length of 100% is intended.
                if (aLineDash.DashLen == 0)
                    fD = 100.0;
                // Tweak dash length, see above.
                if (aLineCap == LineCap_ROUND && fSp > 99.0)
                    fD += 99.0;

                for( i = 0; i < aLineDash.Dashes; i ++ )
                {
                    mpFS->singleElementNS( XML_a , XML_ds,
                                           XML_d , write1000thOfAPercent(fD),
                                           XML_sp, write1000thOfAPercent(fSp) );
                }
            }

            SAL_WARN_IF(nLineWidth <= 0,
                        "oox.shape", "while writing outline - custom dash - line width was < 0  : " << nLineWidth);
            SAL_WARN_IF(aLineDash.Dashes < 0,
                        "oox.shape", "while writing outline - custom dash - number of dashes was < 0  : " << aLineDash.Dashes);
            SAL_WARN_IF(aLineDash.Dashes > 0 && aLineDash.DashLen <= 0,
                        "oox.shape", "while writing outline - custom dash - dash length was < 0  : " << aLineDash.DashLen);
            SAL_WARN_IF(aLineDash.Dots < 0,
                        "oox.shape", "while writing outline - custom dash - number of dots was < 0  : " << aLineDash.Dots);
            SAL_WARN_IF(aLineDash.Dots > 0 && aLineDash.DotLen <= 0,
                        "oox.shape", "while writing outline - custom dash - dot length was < 0  : " << aLineDash.DotLen);
            SAL_WARN_IF(aLineDash.Distance <= 0,
                        "oox.shape", "while writing outline - custom dash - distance was < 0  : " << aLineDash.Distance);

            mpFS->endElementNS( XML_a, XML_custDash );
        }
    }

    if (!bNoFill && nLineWidth > 1 && GetProperty(rXPropSet, "LineJoint"))
    {
        LineJoint eLineJoint = mAny.get<LineJoint>();

        if( aStyleLineJoint == LineJoint_NONE || aStyleLineJoint != eLineJoint )
        {
            // style-defined line joint does not exist, or is different from the shape's joint
            switch( eLineJoint )
            {
                case LineJoint_NONE:
                case LineJoint_BEVEL:
                    mpFS->singleElementNS(XML_a, XML_bevel);
                    break;
                default:
                case LineJoint_MIDDLE:
                case LineJoint_MITER:
                    mpFS->singleElementNS(XML_a, XML_miter);
                    break;
                case LineJoint_ROUND:
                    mpFS->singleElementNS(XML_a, XML_round);
                    break;
            }
        }
    }

    if( !bNoFill )
    {
        WriteLineArrow( rXPropSet, true );
        WriteLineArrow( rXPropSet, false );
    }
    else
    {
        mpFS->singleElementNS(XML_a, XML_noFill);
    }

    mpFS->endElementNS( XML_a, XML_ln );
}

const char* DrawingML::GetComponentDir() const
{
    switch ( meDocumentType )
    {
        case DOCUMENT_DOCX: return "word";
        case DOCUMENT_PPTX: return "ppt";
        case DOCUMENT_XLSX: return "xl";
    }

    return "unknown";
}

const char* DrawingML::GetRelationCompPrefix() const
{
    switch ( meDocumentType )
    {
        case DOCUMENT_DOCX: return "";
        case DOCUMENT_PPTX:
        case DOCUMENT_XLSX: return "../";
    }

    return "unknown";
}

OUString DrawingML::WriteImage( const Graphic& rGraphic , bool bRelPathToMedia, OUString* pFileName )
{
    GfxLink aLink = rGraphic.GetGfxLink ();
    OUString sMediaType;
    const char* pExtension = "";
    OUString sRelId;

    SvMemoryStream aStream;
    const void* aData = aLink.GetData();
    std::size_t nDataSize = aLink.GetDataSize();

    switch ( aLink.GetType() )
    {
        case GfxLinkType::NativeGif:
            sMediaType = "image/gif";
            pExtension = ".gif";
            break;

        // #i15508# added BMP type for better exports
        // export not yet active, so adding for reference (not checked)
        case GfxLinkType::NativeBmp:
            sMediaType = "image/bmp";
            pExtension = ".bmp";
            break;

        case GfxLinkType::NativeJpg:
            sMediaType = "image/jpeg";
            pExtension = ".jpeg";
            break;
        case GfxLinkType::NativePng:
            sMediaType = "image/png";
            pExtension = ".png";
            break;
        case GfxLinkType::NativeTif:
            sMediaType = "image/tiff";
            pExtension = ".tif";
            break;
        case GfxLinkType::NativeWmf:
            sMediaType = "image/x-wmf";
            pExtension = ".wmf";
            break;
        case GfxLinkType::NativeMet:
            sMediaType = "image/x-met";
            pExtension = ".met";
            break;
        case GfxLinkType::NativePct:
            sMediaType = "image/x-pict";
            pExtension = ".pct";
            break;
        case GfxLinkType::NativeMov:
            sMediaType = "application/movie";
            pExtension = ".MOV";
            break;
        default:
        {
            GraphicType aType = rGraphic.GetType();
            if ( aType == GraphicType::Bitmap || aType == GraphicType::GdiMetafile)
            {
                if ( aType == GraphicType::Bitmap )
                {
                    (void)GraphicConverter::Export( aStream, rGraphic, ConvertDataFormat::PNG );
                    sMediaType = "image/png";
                    pExtension = ".png";
                }
                else
                {
                    (void)GraphicConverter::Export( aStream, rGraphic, ConvertDataFormat::EMF );
                    sMediaType = "image/x-emf";
                    pExtension = ".emf";
                }
            }
            else
            {
                SAL_WARN("oox.shape", "unhandled graphic type " << static_cast<int>(aType) );
                /*Earlier, even in case of unhandled graphic types we were
                  proceeding to write the image, which would eventually
                  write an empty image with a zero size, and return a valid
                  relationID, which is incorrect.
                  */
                return sRelId;
            }

            aData = aStream.GetData();
            nDataSize = aStream.GetEndOfData();
            break;
        }
    }

    Reference< XOutputStream > xOutStream = mpFB->openFragmentStream( OUStringBuffer()
                                                                      .appendAscii( GetComponentDir() )
                                                                      .append( "/media/image" +
                                                                        OUString::number(mnImageCounter) )
                                                                      .appendAscii( pExtension )
                                                                      .makeStringAndClear(),
                                                                      sMediaType );
    xOutStream->writeBytes( Sequence< sal_Int8 >( static_cast<const sal_Int8*>(aData), nDataSize ) );
    xOutStream->closeOutput();

    const OString sRelPathToMedia = "media/image";
    OString sRelationCompPrefix;
    if ( bRelPathToMedia )
        sRelationCompPrefix = "../";
    else
        sRelationCompPrefix = GetRelationCompPrefix();
    OUString sPath = OUStringBuffer()
                     .appendAscii( sRelationCompPrefix.getStr() )
                     .appendAscii( sRelPathToMedia.getStr() )
                     .append( static_cast<sal_Int32>(mnImageCounter ++) )
                     .appendAscii( pExtension )
                     .makeStringAndClear();
    sRelId = mpFB->addRelation( mpFS->getOutputStream(),
                                oox::getRelationship(Relationship::IMAGE),
                                sPath );

    if (pFileName)
        *pFileName = sPath;
    return sRelId;
}

void DrawingML::WriteMediaNonVisualProperties(const css::uno::Reference<css::drawing::XShape>& xShape)
{
    SdrMediaObj* pMediaObj = dynamic_cast<SdrMediaObj*>(GetSdrObjectFromXShape(xShape));
    if (!pMediaObj)
        return;

    // extension
    OUString aExtension;
    const OUString& rURL(pMediaObj->getURL());
    int nLastDot = rURL.lastIndexOf('.');
    if (nLastDot >= 0)
        aExtension = rURL.copy(nLastDot);

    bool bEmbed = rURL.startsWith("vnd.sun.star.Package:");
    Relationship eMediaType = Relationship::VIDEO;

    // mime type
#if HAVE_FEATURE_AVMEDIA
    OUString aMimeType(pMediaObj->getMediaProperties().getMimeType());
#else
    OUString aMimeType("none");
#endif
    if (aMimeType == "application/vnd.sun.star.media")
    {
        // try to set something better
        // TODO fix the importer to actually set the mimetype on import
        if (aExtension.equalsIgnoreAsciiCase(".avi"))
            aMimeType = "video/x-msvideo";
        else if (aExtension.equalsIgnoreAsciiCase(".flv"))
            aMimeType = "video/x-flv";
        else if (aExtension.equalsIgnoreAsciiCase(".mp4"))
            aMimeType = "video/mp4";
        else if (aExtension.equalsIgnoreAsciiCase(".mov"))
            aMimeType = "video/quicktime";
        else if (aExtension.equalsIgnoreAsciiCase(".ogv"))
            aMimeType = "video/ogg";
        else if (aExtension.equalsIgnoreAsciiCase(".wmv"))
            aMimeType = "video/x-ms-wmv";
        else if (aExtension.equalsIgnoreAsciiCase(".wav"))
        {
            aMimeType = "audio/x-wav";
            eMediaType = Relationship::AUDIO;
        }
        else if (aExtension.equalsIgnoreAsciiCase(".m4a"))
        {
            aMimeType = "audio/mp4";
            eMediaType = Relationship::AUDIO;
        }
    }

    OUString aVideoFileRelId;
    OUString aMediaRelId;

    if (bEmbed)
    {
        // copy the video stream
        Reference<XOutputStream> xOutStream = mpFB->openFragmentStream(OUStringBuffer()
                                                                       .appendAscii(GetComponentDir())
                                                                       .append("/media/media" +
                                                                            OUString::number(mnImageCounter) +
                                                                            aExtension)
                                                                       .makeStringAndClear(),
                                                                       aMimeType);

        uno::Reference<io::XInputStream> xInputStream(pMediaObj->GetInputStream());
        comphelper::OStorageHelper::CopyInputToOutput(xInputStream, xOutStream);

        xOutStream->closeOutput();

        // create the relation
        OUString aPath = OUStringBuffer().appendAscii(GetRelationCompPrefix())
                                         .append("media/media" + OUString::number(mnImageCounter++) + aExtension)
                                         .makeStringAndClear();
        aVideoFileRelId = mpFB->addRelation(mpFS->getOutputStream(), oox::getRelationship(eMediaType), aPath);
        aMediaRelId = mpFB->addRelation(mpFS->getOutputStream(), oox::getRelationship(Relationship::MEDIA), aPath);
    }
    else
    {
        aVideoFileRelId = mpFB->addRelation(mpFS->getOutputStream(), oox::getRelationship(eMediaType), rURL);
        aMediaRelId = mpFB->addRelation(mpFS->getOutputStream(), oox::getRelationship(Relationship::MEDIA), rURL);
    }

    GetFS()->startElementNS(XML_p, XML_nvPr);

    GetFS()->singleElementNS(XML_a, eMediaType == Relationship::VIDEO ? XML_videoFile : XML_audioFile,
                    FSNS(XML_r, XML_link), aVideoFileRelId);

    GetFS()->startElementNS(XML_p, XML_extLst);
    // media extensions; google this ID for details
    GetFS()->startElementNS(XML_p, XML_ext, XML_uri, "{DAA4B4D4-6D71-4841-9C94-3DE7FCFB9230}");

    GetFS()->singleElementNS(XML_p14, XML_media,
            bEmbed? FSNS(XML_r, XML_embed): FSNS(XML_r, XML_link), aMediaRelId);

    GetFS()->endElementNS(XML_p, XML_ext);
    GetFS()->endElementNS(XML_p, XML_extLst);

    GetFS()->endElementNS(XML_p, XML_nvPr);
}

void DrawingML::WriteImageBrightnessContrastTransparence(uno::Reference<beans::XPropertySet> const & rXPropSet)
{
    sal_Int16 nBright = 0;
    sal_Int32 nContrast = 0;
    sal_Int32 nTransparence = 0;

    if (GetProperty(rXPropSet, "AdjustLuminance"))
        nBright = mAny.get<sal_Int16>();
    if (GetProperty(rXPropSet, "AdjustContrast"))
        nContrast = mAny.get<sal_Int32>();
    // Used for shapes with picture fill
    if (GetProperty(rXPropSet, "FillTransparence"))
        nTransparence = mAny.get<sal_Int32>();
    // Used for pictures
    if (nTransparence == 0 && GetProperty(rXPropSet, "Transparency"))
        nTransparence = static_cast<sal_Int32>(mAny.get<sal_Int16>());

    if (GetProperty(rXPropSet, "GraphicColorMode"))
    {
        drawing::ColorMode aColorMode;
        mAny >>= aColorMode;
        if (aColorMode == drawing::ColorMode_GREYS)
            mpFS->singleElementNS(XML_a, XML_grayscl);
        else if (aColorMode == drawing::ColorMode_MONO)
            //black/white has a 0,5 threshold in LibreOffice
            mpFS->singleElementNS(XML_a, XML_biLevel, XML_thresh, OString::number(50000));
        else if (aColorMode == drawing::ColorMode_WATERMARK)
        {
            //map watermark with mso washout
            nBright = 70;
            nContrast = -70;
        }
    }


    if (nBright || nContrast)
    {
        mpFS->singleElementNS(XML_a, XML_lum,
                   XML_bright, sax_fastparser::UseIf(OString::number(nBright * 1000), nBright != 0),
                   XML_contrast, sax_fastparser::UseIf(OString::number(nContrast * 1000), nContrast != 0));
    }

    if (nTransparence)
    {
        sal_Int32 nAlphaMod = (100 - nTransparence ) * PER_PERCENT;
        mpFS->singleElementNS(XML_a, XML_alphaModFix, XML_amt, OString::number(nAlphaMod));
    }
}

OUString DrawingML::WriteXGraphicBlip(uno::Reference<beans::XPropertySet> const & rXPropSet,
                                      uno::Reference<graphic::XGraphic> const & rxGraphic,
                                      bool bRelPathToMedia)
{
    OUString sRelId;
    OUString sFileName;

    if (!rxGraphic.is())
        return sRelId;

    Graphic aGraphic(rxGraphic);
    if (mpTextExport)
    {
        BitmapChecksum nChecksum = aGraphic.GetChecksum();
        sRelId = mpTextExport->FindRelId(nChecksum);
        sFileName = mpTextExport->FindFileName(nChecksum);
    }
    if (sRelId.isEmpty())
    {
        sRelId = WriteImage(aGraphic, bRelPathToMedia, &sFileName);
        if (mpTextExport)
        {
            BitmapChecksum nChecksum = aGraphic.GetChecksum();
            mpTextExport->CacheRelId(nChecksum, sRelId, sFileName);
        }
    }
    else
    {
        // Include the same relation again. This makes it possible to
        // reuse an image across different headers.
        sRelId = mpFB->addRelation( mpFS->getOutputStream(),
                                    oox::getRelationship(Relationship::IMAGE),
                                    sFileName );
    }

    mpFS->startElementNS(XML_a, XML_blip, FSNS(XML_r, XML_embed), sRelId);

    WriteImageBrightnessContrastTransparence(rXPropSet);

    WriteArtisticEffect(rXPropSet);

    mpFS->endElementNS(XML_a, XML_blip);

    return sRelId;
}

void DrawingML::WriteXGraphicBlipMode(uno::Reference<beans::XPropertySet> const & rXPropSet,
                                      uno::Reference<graphic::XGraphic> const & rxGraphic)
{
    BitmapMode eBitmapMode(BitmapMode_NO_REPEAT);
    if (GetProperty(rXPropSet, "FillBitmapMode"))
        mAny >>= eBitmapMode;

    SAL_INFO("oox.shape", "fill bitmap mode: " << int(eBitmapMode));

    switch (eBitmapMode)
    {
    case BitmapMode_REPEAT:
        mpFS->singleElementNS(XML_a, XML_tile);
        break;
    case BitmapMode_STRETCH:
        WriteXGraphicStretch(rXPropSet, rxGraphic);
        break;
    default:
        break;
    }
}

void DrawingML::WriteBlipOrNormalFill( const Reference< XPropertySet >& xPropSet, const OUString& rURLPropName )
{
    // check for blip and otherwise fall back to normal fill
    // we always store normal fill properties but OOXML
    // uses a choice between our fill props and BlipFill
    if (GetProperty ( xPropSet, rURLPropName ))
        WriteBlipFill( xPropSet, rURLPropName );
    else
        WriteFill(xPropSet);
}

void DrawingML::WriteBlipFill( const Reference< XPropertySet >& rXPropSet, const OUString& sURLPropName )
{
    WriteBlipFill( rXPropSet, sURLPropName, XML_a );
}

void DrawingML::WriteBlipFill( const Reference< XPropertySet >& rXPropSet, const OUString& sURLPropName, sal_Int32 nXmlNamespace )
{
    if ( !GetProperty( rXPropSet, sURLPropName ) )
        return;

    uno::Reference<graphic::XGraphic> xGraphic;
    if (mAny.has<uno::Reference<awt::XBitmap>>())
    {
        uno::Reference<awt::XBitmap> xBitmap = mAny.get<uno::Reference<awt::XBitmap>>();
        if (xBitmap.is())
            xGraphic.set(xBitmap, uno::UNO_QUERY);
    }
    else if (mAny.has<uno::Reference<graphic::XGraphic>>())
    {
        xGraphic = mAny.get<uno::Reference<graphic::XGraphic>>();
    }

    if (xGraphic.is())
    {
        bool bWriteMode = false;
        if (sURLPropName == "FillBitmap" || sURLPropName == "BackGraphic")
            bWriteMode = true;
        WriteXGraphicBlipFill(rXPropSet, xGraphic, nXmlNamespace, bWriteMode);
    }
}

void DrawingML::WriteXGraphicBlipFill(uno::Reference<beans::XPropertySet> const & rXPropSet,
                                      uno::Reference<graphic::XGraphic> const & rxGraphic,
                                      sal_Int32 nXmlNamespace, bool bWriteMode, bool bRelPathToMedia)
{
    if (!rxGraphic.is() )
        return;

    mpFS->startElementNS(nXmlNamespace , XML_blipFill, XML_rotWithShape, "0");

    WriteXGraphicBlip(rXPropSet, rxGraphic, bRelPathToMedia);

    if (GetDocumentType() != DOCUMENT_DOCX)
    {
        // Write the crop rectangle of Impress as a source rectangle.
        WriteSrcRectXGraphic(rXPropSet, rxGraphic);
    }

    if (bWriteMode)
    {
        WriteXGraphicBlipMode(rXPropSet, rxGraphic);
    }
    else if(GetProperty(rXPropSet, "FillBitmapStretch"))
    {
            bool bStretch = mAny.get<bool>();

            if (bStretch)
            {
                WriteXGraphicStretch(rXPropSet, rxGraphic);
            }
    }
    mpFS->endElementNS(nXmlNamespace, XML_blipFill);
}

void DrawingML::WritePattFill( const Reference< XPropertySet >& rXPropSet )
{
    if ( GetProperty( rXPropSet, "FillHatch" ) )
    {
        drawing::Hatch aHatch;
        mAny >>= aHatch;
        WritePattFill(rXPropSet, aHatch);
    }
}

void DrawingML::WritePattFill(const Reference<XPropertySet>& rXPropSet, const css::drawing::Hatch& rHatch)
{
        mpFS->startElementNS(XML_a, XML_pattFill, XML_prst, GetHatchPattern(rHatch));

        mpFS->startElementNS(XML_a, XML_fgClr);
        WriteColor(::Color(ColorTransparency, rHatch.Color));
        mpFS->endElementNS( XML_a , XML_fgClr );

        ::Color nColor = COL_WHITE;
        sal_Int32 nAlpha  = 0;

        if ( GetProperty( rXPropSet, "FillBackground" ) )
        {
            bool isBackgroundFilled = false;
            mAny >>= isBackgroundFilled;
            if( isBackgroundFilled )
            {
                nAlpha = MAX_PERCENT;

                if( GetProperty( rXPropSet, "FillColor" ) )
                {
                    mAny >>= nColor;
                }
            }
        }

        mpFS->startElementNS(XML_a, XML_bgClr);
        WriteColor(nColor, nAlpha);
        mpFS->endElementNS( XML_a , XML_bgClr );

        mpFS->endElementNS( XML_a , XML_pattFill );
}

void DrawingML::WriteGraphicCropProperties(uno::Reference<beans::XPropertySet> const & rXPropSet,
                                           Size const & rOriginalSize,
                                           MapMode const & rMapMode)
{
    if (!GetProperty(rXPropSet, "GraphicCrop"))
        return;

    css::text::GraphicCrop aGraphicCropStruct;
    mAny >>= aGraphicCropStruct;

    if(GetProperty(rXPropSet, "CustomShapeGeometry"))
    {
    // tdf#134210 GraphicCrop property is handled in import filter because of LibreOffice has not core
    // feature. We cropped the bitmap physically and MSO shouldn't crop bitmap one more time. When we
    // have core feature for graphic cropping in custom shapes, we should uncomment the code anymore.

        mpFS->singleElementNS( XML_a, XML_srcRect);
    }
    else
    {
        Size aOriginalSize(rOriginalSize);

        // GraphicCrop is in mm100, so in case the original size is in pixels, convert it over.
        if (rMapMode.GetMapUnit() == MapUnit::MapPixel)
            aOriginalSize = Application::GetDefaultDevice()->PixelToLogic(aOriginalSize, MapMode(MapUnit::Map100thMM));

        if ( (0 != aGraphicCropStruct.Left) || (0 != aGraphicCropStruct.Top) || (0 != aGraphicCropStruct.Right) || (0 != aGraphicCropStruct.Bottom) )
        {
            mpFS->singleElementNS( XML_a, XML_srcRect,
                XML_l, OString::number(rtl::math::round(aGraphicCropStruct.Left * 100000.0 / aOriginalSize.Width())),
                XML_t, OString::number(rtl::math::round(aGraphicCropStruct.Top * 100000.0 / aOriginalSize.Height())),
                XML_r, OString::number(rtl::math::round(aGraphicCropStruct.Right * 100000.0 / aOriginalSize.Width())),
                XML_b, OString::number(rtl::math::round(aGraphicCropStruct.Bottom * 100000.0 / aOriginalSize.Height())) );
        }
    }
}

void DrawingML::WriteSrcRectXGraphic(uno::Reference<beans::XPropertySet> const & rxPropertySet,
                                     uno::Reference<graphic::XGraphic> const & rxGraphic)
{
    Graphic aGraphic(rxGraphic);
    Size aOriginalSize = aGraphic.GetPrefSize();
    const MapMode& rMapMode = aGraphic.GetPrefMapMode();
    WriteGraphicCropProperties(rxPropertySet, aOriginalSize, rMapMode);
}

void DrawingML::WriteXGraphicStretch(uno::Reference<beans::XPropertySet> const & rXPropSet,
                                     uno::Reference<graphic::XGraphic> const & rxGraphic)
{
    if (GetDocumentType() != DOCUMENT_DOCX)
    {
        // Limiting the area used for stretching is not supported in Impress.
        mpFS->singleElementNS(XML_a, XML_stretch);
        return;
    }

    mpFS->startElementNS(XML_a, XML_stretch);

    bool bCrop = false;
    if (GetProperty(rXPropSet, "GraphicCrop"))
    {
        css::text::GraphicCrop aGraphicCropStruct;
        mAny >>= aGraphicCropStruct;

        if ((0 != aGraphicCropStruct.Left)
         || (0 != aGraphicCropStruct.Top)
         || (0 != aGraphicCropStruct.Right)
         || (0 != aGraphicCropStruct.Bottom))
        {
            Graphic aGraphic(rxGraphic);
            Size aOriginalSize(aGraphic.GetPrefSize());
            mpFS->singleElementNS(XML_a, XML_fillRect,
                XML_l, OString::number(((aGraphicCropStruct.Left)   * 100000) / aOriginalSize.Width()),
                XML_t, OString::number(((aGraphicCropStruct.Top)    * 100000) / aOriginalSize.Height()),
                XML_r, OString::number(((aGraphicCropStruct.Right)  * 100000) / aOriginalSize.Width()),
                XML_b, OString::number(((aGraphicCropStruct.Bottom) * 100000) / aOriginalSize.Height()));
            bCrop = true;
        }
    }

    if (!bCrop)
    {
        mpFS->singleElementNS(XML_a, XML_fillRect);
    }

    mpFS->endElementNS(XML_a, XML_stretch);
}

namespace
{
bool IsTopGroupObj(const uno::Reference<drawing::XShape>& xShape)
{
    SdrObject* pObject = GetSdrObjectFromXShape(xShape);
    if (!pObject)
        return false;

    if (pObject->getParentSdrObjectFromSdrObject())
        return false;

    return pObject->IsGroupObject();
}
}

void DrawingML::WriteTransformation(const Reference< XShape >& xShape, const tools::Rectangle& rRect,
        sal_Int32 nXmlNamespace, bool bFlipH, bool bFlipV, sal_Int32 nRotation, bool bIsGroupShape)
{

    mpFS->startElementNS( nXmlNamespace, XML_xfrm,
                          XML_flipH, sax_fastparser::UseIf("1", bFlipH),
                          XML_flipV, sax_fastparser::UseIf("1", bFlipV),
                          XML_rot, sax_fastparser::UseIf(OString::number(nRotation), nRotation % 21600000 != 0));

    sal_Int32 nLeft = rRect.Left();
    sal_Int32 nChildLeft = nLeft;
    sal_Int32 nTop = rRect.Top();
    sal_Int32 nChildTop = nTop;
    if (GetDocumentType() == DOCUMENT_DOCX && !m_xParent.is())
    {
        nLeft = 0;
        nTop = 0;
    }

    mpFS->singleElementNS(XML_a, XML_off,
        XML_x, OString::number(oox::drawingml::convertHmmToEmu(nLeft)),
        XML_y, OString::number(oox::drawingml::convertHmmToEmu(nTop)));
    mpFS->singleElementNS(XML_a, XML_ext,
        XML_cx, OString::number(oox::drawingml::convertHmmToEmu(rRect.GetWidth())),
        XML_cy, OString::number(oox::drawingml::convertHmmToEmu(rRect.GetHeight())));

    if (bIsGroupShape && (GetDocumentType() != DOCUMENT_DOCX || IsTopGroupObj(xShape)))
    {
        mpFS->singleElementNS(XML_a, XML_chOff,
            XML_x, OString::number(oox::drawingml::convertHmmToEmu(nChildLeft)),
            XML_y, OString::number(oox::drawingml::convertHmmToEmu(nChildTop)));
        mpFS->singleElementNS(XML_a, XML_chExt,
            XML_cx, OString::number(oox::drawingml::convertHmmToEmu(rRect.GetWidth())),
            XML_cy, OString::number(oox::drawingml::convertHmmToEmu(rRect.GetHeight())));
    }

    mpFS->endElementNS( nXmlNamespace, XML_xfrm );
}

void DrawingML::WriteShapeTransformation( const Reference< XShape >& rXShape, sal_Int32 nXmlNamespace, bool bFlipH, bool bFlipV, bool bSuppressRotation, bool bSuppressFlipping, bool bFlippedBeforeRotation )
{
    SAL_INFO("oox.shape",  "write shape transformation");

    Degree100 nRotation;
    Degree100 nCameraRotation;
    awt::Point aPos = rXShape->getPosition();
    awt::Size aSize = rXShape->getSize();

    bool bFlipHWrite = bFlipH && !bSuppressFlipping;
    bool bFlipVWrite = bFlipV && !bSuppressFlipping;
    bFlipH = bFlipH && !bFlippedBeforeRotation;
    bFlipV = bFlipV && !bFlippedBeforeRotation;

    if (GetDocumentType() == DOCUMENT_DOCX && m_xParent.is())
    {
        awt::Point aParentPos = m_xParent->getPosition();
        aPos.X -= aParentPos.X;
        aPos.Y -= aParentPos.Y;
    }

    if ( aSize.Width < 0 )
        aSize.Width = 1000;
    if ( aSize.Height < 0 )
        aSize.Height = 1000;
    if (!bSuppressRotation)
    {
        SdrObject* pShape = GetSdrObjectFromXShape( rXShape );
        nRotation = pShape ? pShape->GetRotateAngle() : 0_deg100;
        if ( GetDocumentType() != DOCUMENT_DOCX )
        {
            int faccos=bFlipV ? -1 : 1;
            int facsin=bFlipH ? -1 : 1;
            aPos.X-=(1-faccos*cos(nRotation.get()*F_PI18000))*aSize.Width/2-facsin*sin(nRotation.get()*F_PI18000)*aSize.Height/2;
            aPos.Y-=(1-faccos*cos(nRotation.get()*F_PI18000))*aSize.Height/2+facsin*sin(nRotation.get()*F_PI18000)*aSize.Width/2;
        }
        else  if (m_xParent.is() && nRotation != 0_deg100)
        {
            // Position for rotated shapes inside group is not set by DocxSdrExport.
            basegfx::B2DRange aRect(-aSize.Width / 2.0, -aSize.Height / 2.0, aSize.Width / 2.0,
                                    aSize.Height / 2.0);
            basegfx::B2DHomMatrix aRotateMatrix =
                basegfx::utils::createRotateB2DHomMatrix(toRadians(nRotation));
            aRect.transform(aRotateMatrix);
            aPos.X += -aSize.Width / 2.0 - aRect.getMinX();
            aPos.Y += -aSize.Height / 2.0 - aRect.getMinY();
        }

        // The RotateAngle property's value is independent from any flipping, and that's exactly what we need here.
        uno::Reference<beans::XPropertySet> xPropertySet(rXShape, uno::UNO_QUERY);
        uno::Reference<beans::XPropertySetInfo> xPropertySetInfo = xPropertySet->getPropertySetInfo();
        if (xPropertySetInfo->hasPropertyByName("RotateAngle"))
        {
            sal_Int32 nTmp;
            if (xPropertySet->getPropertyValue("RotateAngle") >>= nTmp)
                nRotation = Degree100(nTmp);
        }
        // tdf#133037: restore original rotate angle before output
        if (nRotation && xPropertySetInfo->hasPropertyByName(UNO_NAME_MISC_OBJ_INTEROPGRABBAG))
        {
            uno::Sequence<beans::PropertyValue> aGrabBagProps;
            xPropertySet->getPropertyValue(UNO_NAME_MISC_OBJ_INTEROPGRABBAG) >>= aGrabBagProps;
            auto p3DEffectProps = std::find_if(std::cbegin(aGrabBagProps), std::cend(aGrabBagProps),
                [](const PropertyValue& rProp) { return rProp.Name == "3DEffectProperties"; });
            if (p3DEffectProps != std::cend(aGrabBagProps))
            {
                uno::Sequence<beans::PropertyValue> a3DEffectProps;
                p3DEffectProps->Value >>= a3DEffectProps;
                auto pCameraProps = std::find_if(std::cbegin(a3DEffectProps), std::cend(a3DEffectProps),
                    [](const PropertyValue& rProp) { return rProp.Name == "Camera"; });
                if (pCameraProps != std::cend(a3DEffectProps))
                {
                    uno::Sequence<beans::PropertyValue> aCameraProps;
                    pCameraProps->Value >>= aCameraProps;
                    auto pZRotationProp = std::find_if(std::cbegin(aCameraProps), std::cend(aCameraProps),
                        [](const PropertyValue& rProp) { return rProp.Name == "rotRev"; });
                    if (pZRotationProp != std::cend(aCameraProps))
                    {
                        sal_Int32 nTmp = 0;
                        pZRotationProp->Value >>= nTmp;
                        nCameraRotation = NormAngle36000(Degree100(nTmp / -600));
                    }
                }
            }
        }
    }

    // OOXML flips shapes before rotating them.
    if(bFlipH != bFlipV)
        nRotation = Degree100(nRotation.get() * -1 + 36000);

    WriteTransformation(rXShape, tools::Rectangle(Point(aPos.X, aPos.Y), Size(aSize.Width, aSize.Height)), nXmlNamespace,
            bFlipHWrite, bFlipVWrite, ExportRotateClockwisify(nRotation + nCameraRotation), IsGroupShape( rXShape ));
}

static OUString lcl_GetTarget(const css::uno::Reference<css::frame::XModel>& xModel, OUString& rURL)
{
    Reference<drawing::XDrawPagesSupplier> xDPS(xModel, uno::UNO_QUERY_THROW);
    Reference<drawing::XDrawPages> xDrawPages(xDPS->getDrawPages(), uno::UNO_SET_THROW);
    sal_uInt32 nPageCount = xDrawPages->getCount();
    OUString sTarget;

    for (sal_uInt32 i = 0; i < nPageCount; ++i)
    {
        Reference<XDrawPage> xDrawPage;
        xDrawPages->getByIndex(i) >>= xDrawPage;
        Reference<container::XNamed> xNamed(xDrawPage, UNO_QUERY);
        if (!xNamed)
            continue;
        OUString sSlideName = "#" + xNamed->getName();
        if (rURL == sSlideName)
        {
            sTarget = "slide" + OUString::number(i + 1) + ".xml";
            break;
        }
    }

    return sTarget;
}

void DrawingML::WriteRunProperties( const Reference< XPropertySet >& rRun, bool bIsField, sal_Int32 nElement,
                                    bool bCheckDirect,bool& rbOverridingCharHeight, sal_Int32& rnCharHeight,
                                    sal_Int16 nScriptType, const Reference< XPropertySet >& rXShapePropSet)
{
    Reference< XPropertySet > rXPropSet = rRun;
    Reference< XPropertyState > rXPropState( rRun, UNO_QUERY );
    OUString usLanguage;
    PropertyState eState;
    bool bComplex = ( nScriptType ==  css::i18n::ScriptType::COMPLEX );
    const char* bold = "0";
    const char* italic = nullptr;
    const char* underline = nullptr;
    const char* strikeout = nullptr;
    const char* cap = nullptr;
    sal_Int32 nSize = 1800;
    sal_Int32 nCharEscapement = 0;
    sal_Int32 nCharKerning = 0;

    if ( nElement == XML_endParaRPr && rbOverridingCharHeight )
    {
        nSize = rnCharHeight;
    }
    else if (GetProperty(rXPropSet, "CharHeight"))
    {
        nSize = static_cast<sal_Int32>(100*(*o3tl::doAccess<float>(mAny)));
        if ( nElement == XML_rPr )
        {
            rbOverridingCharHeight = true;
            rnCharHeight = nSize;
        }
    }

    if (GetProperty(rXPropSet, "CharKerning"))
        nCharKerning = static_cast<sal_Int32>(*o3tl::doAccess<sal_Int16>(mAny));
    /**  While setting values in propertymap,
    *    CharKerning converted using GetTextSpacingPoint
    *    i.e set @ https://opengrok.libreoffice.org/xref/core/oox/source/drawingml/textcharacterproperties.cxx#129
    *    therefore to get original value CharKerning need to be convert.
    *    https://opengrok.libreoffice.org/xref/core/oox/source/drawingml/drawingmltypes.cxx#95
    **/
    nCharKerning = ((nCharKerning * 720)-360) / 254;

    if ((bComplex && GetProperty(rXPropSet, "CharWeightComplex"))
        || GetProperty(rXPropSet, "CharWeight"))
    {
        if ( *o3tl::doAccess<float>(mAny) >= awt::FontWeight::SEMIBOLD )
            bold = "1";
    }

    if ((bComplex && GetProperty(rXPropSet, "CharPostureComplex"))
        || GetProperty(rXPropSet, "CharPosture"))
        switch ( *o3tl::doAccess<awt::FontSlant>(mAny) )
        {
            case awt::FontSlant_OBLIQUE :
            case awt::FontSlant_ITALIC :
                italic = "1";
                break;
            default:
                break;
        }

    if ((bCheckDirect && GetPropertyAndState(rXPropSet, rXPropState, "CharUnderline", eState)
         && eState == beans::PropertyState_DIRECT_VALUE)
        || GetProperty(rXPropSet, "CharUnderline"))
    {
        switch ( *o3tl::doAccess<sal_Int16>(mAny) )
        {
            case awt::FontUnderline::SINGLE :
                underline = "sng";
                break;
            case awt::FontUnderline::DOUBLE :
                underline = "dbl";
                break;
            case awt::FontUnderline::DOTTED :
                underline = "dotted";
                break;
            case awt::FontUnderline::DASH :
                underline = "dash";
                break;
            case awt::FontUnderline::LONGDASH :
                underline = "dashLong";
                break;
            case awt::FontUnderline::DASHDOT :
                underline = "dotDash";
                break;
            case awt::FontUnderline::DASHDOTDOT :
                underline = "dotDotDash";
                break;
            case awt::FontUnderline::WAVE :
                underline = "wavy";
                break;
            case awt::FontUnderline::DOUBLEWAVE :
                underline = "wavyDbl";
                break;
            case awt::FontUnderline::BOLD :
                underline = "heavy";
                break;
            case awt::FontUnderline::BOLDDOTTED :
                underline = "dottedHeavy";
                break;
            case awt::FontUnderline::BOLDDASH :
                underline = "dashHeavy";
                break;
            case awt::FontUnderline::BOLDLONGDASH :
                underline = "dashLongHeavy";
                break;
            case awt::FontUnderline::BOLDDASHDOT :
                underline = "dotDashHeavy";
                break;
            case awt::FontUnderline::BOLDDASHDOTDOT :
                underline = "dotDotDashHeavy";
                break;
            case awt::FontUnderline::BOLDWAVE :
                underline = "wavyHeavy";
                break;
        }
    }

    if ((bCheckDirect && GetPropertyAndState(rXPropSet, rXPropState, "CharStrikeout", eState)
         && eState == beans::PropertyState_DIRECT_VALUE)
        || GetProperty(rXPropSet, "CharStrikeout"))
    {
        switch ( *o3tl::doAccess<sal_Int16>(mAny) )
        {
            case awt::FontStrikeout::NONE :
               strikeout = "noStrike";
               break;
            case awt::FontStrikeout::SINGLE :
            // LibO supports further values of character
            // strikeout, OOXML standard (20.1.10.78,
            // ST_TextStrikeType) however specifies only
            // 3 - single, double and none. Approximate
            // the remaining ones by single strike (better
            // some strike than none at all).
            // TODO: figure out how to do this better
            case awt::FontStrikeout::BOLD :
            case awt::FontStrikeout::SLASH :
            case awt::FontStrikeout::X :
               strikeout = "sngStrike";
               break;
            case awt::FontStrikeout::DOUBLE :
               strikeout = "dblStrike";
               break;
        }
    }

    bool bLang = false;
    switch(nScriptType)
    {
        case css::i18n::ScriptType::ASIAN:
            bLang = GetProperty(rXPropSet, "CharLocaleAsian"); break;
        case css::i18n::ScriptType::COMPLEX:
            bLang = GetProperty(rXPropSet, "CharLocaleComplex"); break;
        default:
            bLang = GetProperty(rXPropSet, "CharLocale"); break;
    }

    if (bLang)
    {
        css::lang::Locale aLocale;
        mAny >>= aLocale;
        LanguageTag aLanguageTag( aLocale);
        if (!aLanguageTag.isSystemLocale())
            usLanguage = aLanguageTag.getBcp47MS();
    }

    if (GetPropertyAndState(rXPropSet, rXPropState, "CharEscapement", eState)
        && eState == beans::PropertyState_DIRECT_VALUE)
        mAny >>= nCharEscapement;

    if (nCharEscapement
        && (GetPropertyAndState(rXPropSet, rXPropState, "CharEscapementHeight", eState)
            && eState == beans::PropertyState_DIRECT_VALUE))
    {
        sal_uInt32 nCharEscapementHeight = 0;
        mAny >>= nCharEscapementHeight;
        nSize = (nSize * nCharEscapementHeight) / 100;
        // MSO uses default ~58% size
        nSize = (nSize / 0.58);
    }

    if (GetProperty(rXPropSet, "CharCaseMap"))
    {
        switch ( *o3tl::doAccess<sal_Int16>(mAny) )
        {
            case CaseMap::UPPERCASE :
                cap = "all";
                break;
            case CaseMap::SMALLCAPS :
                cap = "small";
                break;
        }
    }

    mpFS->startElementNS( XML_a, nElement,
                          XML_b, bold,
                          XML_i, italic,
                          XML_lang, sax_fastparser::UseIf(usLanguage, !usLanguage.isEmpty()),
                          XML_sz, OString::number(nSize),
            // For Condensed character spacing spc value is negative.
                          XML_spc, sax_fastparser::UseIf(OString::number(nCharKerning), nCharKerning != 0),
                          XML_strike, strikeout,
                          XML_u, underline,
                          XML_baseline, sax_fastparser::UseIf(OString::number(nCharEscapement*1000), nCharEscapement != 0),
                          XML_cap, cap );

    // Fontwork-shapes in LO have text outline and fill from shape stroke and shape fill
    // PowerPoint has this as run properties
    if (IsFontworkShape(rXShapePropSet))
    {
        WriteOutline(rXShapePropSet);
        WriteBlipOrNormalFill(rXShapePropSet, "Graphic");
        WriteShapeEffects(rXShapePropSet);
    }
    else
    {
        // mso doesn't like text color to be placed after typeface
        if ((bCheckDirect && GetPropertyAndState(rXPropSet, rXPropState, "CharColor", eState)
            && eState == beans::PropertyState_DIRECT_VALUE)
            || GetProperty(rXPropSet, "CharColor"))
        {
            ::Color color( ColorTransparency, *o3tl::doAccess<sal_uInt32>(mAny) );
            SAL_INFO("oox.shape", "run color: " << sal_uInt32(color) << " auto: " << sal_uInt32(COL_AUTO));

            // WriteSolidFill() handles MAX_PERCENT as "no transparency".
            sal_Int32 nTransparency = MAX_PERCENT;
            if (rXPropSet->getPropertySetInfo()->hasPropertyByName("CharTransparence"))
            {
                rXPropSet->getPropertyValue("CharTransparence") >>= nTransparency;
                // UNO scale is 0..100, OOXML scale is 0..100000; also UNO tracks transparency, OOXML
                // tracks opacity.
                nTransparency = MAX_PERCENT - (nTransparency * PER_PERCENT);
            }

            // tdf#104219 In LibreOffice and MS Office, there are two types of colors:
            // Automatic and Fixed. OOXML is setting automatic color, by not providing color.
            if( color != COL_AUTO )
            {
                color.SetAlpha(255);
                // TODO: special handle embossed/engraved
                WriteSolidFill(color, nTransparency);
            }
        }
    }

    // tdf#128096, exporting XML_highlight to docx already works fine,
    // so make sure this code is only run when exporting to pptx, just in case
    if (GetDocumentType() == DOCUMENT_PPTX)
    {
        if (GetProperty(rXPropSet, "CharBackColor"))
        {
            ::Color color(ColorTransparency, *o3tl::doAccess<sal_uInt32>(mAny));
            if( color != COL_AUTO )
            {
                mpFS->startElementNS(XML_a, XML_highlight);
                WriteColor( color );
                mpFS->endElementNS( XML_a, XML_highlight );
            }
        }
    }

    if (underline
        && ((bCheckDirect
             && GetPropertyAndState(rXPropSet, rXPropState, "CharUnderlineColor", eState)
             && eState == beans::PropertyState_DIRECT_VALUE)
            || GetProperty(rXPropSet, "CharUnderlineColor")))
    {
        ::Color color(ColorTransparency, *o3tl::doAccess<sal_uInt32>(mAny));
        // if color is automatic, then we shouldn't write information about color but to take color from character
        if( color != COL_AUTO )
        {
            mpFS->startElementNS(XML_a, XML_uFill);
            WriteSolidFill( color );
            mpFS->endElementNS( XML_a, XML_uFill );
        }
        else
        {
            mpFS->singleElementNS(XML_a, XML_uFillTx);
        }
    }

    if (GetProperty(rXPropSet, "CharFontName"))
    {
        const char* const pitch = nullptr;
        const char* const charset = nullptr;
        OUString usTypeface;

        mAny >>= usTypeface;
        OUString aSubstName( GetSubsFontName( usTypeface, SubsFontFlags::ONLYONE | SubsFontFlags::MS ) );
        if (!aSubstName.isEmpty())
            usTypeface = aSubstName;

        mpFS->singleElementNS( XML_a, XML_latin,
                               XML_typeface, usTypeface,
                               XML_pitchFamily, pitch,
                               XML_charset, charset );
    }

    if ((bComplex
         && (GetPropertyAndState(rXPropSet, rXPropState, "CharFontNameComplex", eState)
             && eState == beans::PropertyState_DIRECT_VALUE))
        || (!bComplex
            && (GetPropertyAndState(rXPropSet, rXPropState, "CharFontNameAsian", eState)
                && eState == beans::PropertyState_DIRECT_VALUE)))
    {
        const char* const pitch = nullptr;
        const char* const charset = nullptr;
        OUString usTypeface;

        mAny >>= usTypeface;
        OUString aSubstName( GetSubsFontName( usTypeface, SubsFontFlags::ONLYONE | SubsFontFlags::MS ) );
        if (!aSubstName.isEmpty())
            usTypeface = aSubstName;

        mpFS->singleElementNS( XML_a, bComplex ? XML_cs : XML_ea,
                               XML_typeface, usTypeface,
                               XML_pitchFamily, pitch,
                               XML_charset, charset );
    }

    if( bIsField )
    {
        Reference< XTextField > rXTextField;
        if (GetProperty(rXPropSet, "TextField"))
            mAny >>= rXTextField;
        if( rXTextField.is() )
            rXPropSet.set( rXTextField, UNO_QUERY );
    }

    // field properties starts here
    if (GetProperty(rXPropSet, "URL"))
    {
        OUString sURL;

        mAny >>= sURL;
        if (!sURL.isEmpty())
        {
            if (!sURL.match("#action?jump="))
            {
                bool bExtURL = URLTransformer().isExternalURL(sURL);
                sURL = bExtURL ? sURL : lcl_GetTarget(GetFB()->getModel(), sURL);

                OUString sRelId
                    = mpFB->addRelation(mpFS->getOutputStream(),
                                        bExtURL ? oox::getRelationship(Relationship::HYPERLINK)
                                                : oox::getRelationship(Relationship::SLIDE),
                                        sURL, bExtURL);

                if (bExtURL)
                    mpFS->singleElementNS(XML_a, XML_hlinkClick, FSNS(XML_r, XML_id), sRelId);
                else
                    mpFS->singleElementNS(XML_a, XML_hlinkClick, FSNS(XML_r, XML_id), sRelId,
                                          XML_action, "ppaction://hlinksldjump");
            }
            else
            {
                sal_Int32 nIndex = sURL.indexOf('=');
                OUString aDestination(sURL.copy(nIndex + 1));
                mpFS->singleElementNS(XML_a, XML_hlinkClick, FSNS(XML_r, XML_id), "", XML_action,
                                      "ppaction://hlinkshowjump?jump=" + aDestination);
            }
        }
    }
    mpFS->endElementNS( XML_a, nElement );
}

OUString DrawingML::GetFieldValue( const css::uno::Reference< css::text::XTextRange >& rRun, bool& bIsURLField )
{
    Reference< XPropertySet > rXPropSet( rRun, UNO_QUERY );
    OUString aFieldType, aFieldValue;

    if (GetProperty(rXPropSet, "TextPortionType"))
    {
        aFieldType = *o3tl::doAccess<OUString>(mAny);
        SAL_INFO("oox.shape", "field type: " << aFieldType);
    }

    if( aFieldType == "TextField" )
    {
        Reference< XTextField > rXTextField;
        if (GetProperty(rXPropSet, "TextField"))
            mAny >>= rXTextField;
        if( rXTextField.is() )
        {
            rXPropSet.set( rXTextField, UNO_QUERY );
            if( rXPropSet.is() )
            {
                OUString aFieldKind( rXTextField->getPresentation( true ) );
                SAL_INFO("oox.shape", "field kind: " << aFieldKind);
                if( aFieldKind == "Page" )
                {
                    aFieldValue = "slidenum";
                }
                else if( aFieldKind == "Pages" )
                {
                    aFieldValue = "slidecount";
                }
                else if( aFieldKind == "PageName" )
                {
                    aFieldValue = "slidename";
                }
                else if( aFieldKind == "URL" )
                {
                    bIsURLField = true;
                    if (GetProperty(rXPropSet, "Representation"))
                        mAny >>= aFieldValue;

                }
                else if(aFieldKind == "Date")
                {
                    sal_Int32 nNumFmt = -1;
                    rXPropSet->getPropertyValue(UNO_TC_PROP_NUMFORMAT) >>= nNumFmt;
                    switch(static_cast<SvxDateFormat>(nNumFmt))
                    {
                        case SvxDateFormat::StdSmall:
                        case SvxDateFormat::A: aFieldValue = "datetime"; // 13/02/96
                                              break;
                        case SvxDateFormat::B: aFieldValue = "datetime1"; // 13/02/1996
                                              break;
                        case SvxDateFormat::StdBig:
                        case SvxDateFormat::D: aFieldValue = "datetime3"; // 13 February 1996
                                              break;
                        default: break;
                    }
                }
                else if(aFieldKind == "ExtTime")
                {
                    sal_Int32 nNumFmt = -1;
                    rXPropSet->getPropertyValue(UNO_TC_PROP_NUMFORMAT) >>= nNumFmt;
                    switch(static_cast<SvxTimeFormat>(nNumFmt))
                    {
                        case SvxTimeFormat::Standard:
                        case SvxTimeFormat::HH24_MM_SS:
                            aFieldValue = "datetime11"; // 13:49:38
                            break;
                        case SvxTimeFormat::HH24_MM:
                            aFieldValue = "datetime10"; // 13:49
                            break;
                        case SvxTimeFormat::HH12_MM:
                            aFieldValue = "datetime12"; // 01:49 PM
                            break;
                        case SvxTimeFormat::HH12_MM_SS:
                            aFieldValue = "datetime13"; // 01:49:38 PM
                            break;
                        default: break;
                    }
                }
                else if(aFieldKind == "ExtFile")
                {
                    sal_Int32 nNumFmt = -1;
                    rXPropSet->getPropertyValue(UNO_TC_PROP_FILE_FORMAT) >>= nNumFmt;
                    switch(nNumFmt)
                    {
                        case 0: aFieldValue = "file"; // Path/File name
                                break;
                        case 1: aFieldValue = "file1"; // Path
                                break;
                        case 2: aFieldValue = "file2"; // File name without extension
                                break;
                        case 3: aFieldValue = "file3"; // File name with extension
                    }
                }
                else if(aFieldKind == "Author")
                {
                    aFieldValue = "author";
                }
            }
        }
    }
    return aFieldValue;
}

void DrawingML::WriteRun( const Reference< XTextRange >& rRun,
                          bool& rbOverridingCharHeight, sal_Int32& rnCharHeight,
                          const css::uno::Reference< css::beans::XPropertySet >& rXShapePropSet)
{
    Reference< XPropertySet > rXPropSet( rRun, UNO_QUERY );
    sal_Int16 nLevel = -1;
    if (GetProperty(rXPropSet, "NumberingLevel"))
        mAny >>= nLevel;

    bool bNumberingIsNumber = true;
    if (GetProperty(rXPropSet, "NumberingIsNumber"))
        mAny >>= bNumberingIsNumber;

    bool bIsURLField = false;
    OUString sFieldValue = GetFieldValue( rRun, bIsURLField );
    bool bWriteField  = !( sFieldValue.isEmpty() || bIsURLField );

    OUString sText = rRun->getString();

    //if there is no text following the bullet, add a space after the bullet
    if (nLevel !=-1 && bNumberingIsNumber && sText.isEmpty() )
         sText=" ";

    if ( bIsURLField )
        sText = sFieldValue;

    if( sText.isEmpty())
    {
        Reference< XPropertySet > xPropSet( rRun, UNO_QUERY );

        try
        {
            if( !xPropSet.is() || !( xPropSet->getPropertyValue( "PlaceholderText" ) >>= sText ) )
                return;
            if( sText.isEmpty() )
                return;
        }
        catch (const Exception &)
        {
            return;
        }
    }

    if (sText == "\n")
    {
        mpFS->singleElementNS(XML_a, XML_br);
    }
    else
    {
        if( bWriteField )
        {
            OString sUUID(comphelper::xml::generateGUIDString());
            mpFS->startElementNS( XML_a, XML_fld,
                                  XML_id, sUUID.getStr(),
                                  XML_type, sFieldValue );
        }
        else
        {
            mpFS->startElementNS(XML_a, XML_r);
        }

        Reference< XPropertySet > xPropSet( rRun, uno::UNO_QUERY );

        WriteRunProperties( xPropSet, bIsURLField, XML_rPr, true, rbOverridingCharHeight, rnCharHeight, GetScriptType(sText), rXShapePropSet);
        mpFS->startElementNS(XML_a, XML_t);
        mpFS->writeEscaped( sText );
        mpFS->endElementNS( XML_a, XML_t );

        if( bWriteField )
            mpFS->endElementNS( XML_a, XML_fld );
        else
            mpFS->endElementNS( XML_a, XML_r );
    }
}

static OUString GetAutoNumType(SvxNumType nNumberingType, bool bSDot, bool bPBehind, bool bPBoth)
{
    OUString sPrefixSuffix;

    if (bPBoth)
        sPrefixSuffix = "ParenBoth";
    else if (bPBehind)
        sPrefixSuffix = "ParenR";
    else if (bSDot)
        sPrefixSuffix = "Period";

    switch( nNumberingType )
    {
        case SVX_NUM_CHARS_UPPER_LETTER_N :
        case SVX_NUM_CHARS_UPPER_LETTER :
            return "alphaUc" + sPrefixSuffix;

        case SVX_NUM_CHARS_LOWER_LETTER_N :
        case SVX_NUM_CHARS_LOWER_LETTER :
            return "alphaLc" + sPrefixSuffix;

        case SVX_NUM_ROMAN_UPPER :
            return "romanUc" + sPrefixSuffix;

        case SVX_NUM_ROMAN_LOWER :
            return "romanLc" + sPrefixSuffix;

        case SVX_NUM_ARABIC :
        {
            if (sPrefixSuffix.isEmpty())
                return "arabicPlain";
            else
                return "arabic" + sPrefixSuffix;
        }
        default:
            break;
    }

    return OUString();
}

void DrawingML::WriteParagraphNumbering(const Reference< XPropertySet >& rXPropSet, float fFirstCharHeight, sal_Int16 nLevel )
{
    if (nLevel < 0 || !GetProperty(rXPropSet, "NumberingRules"))
    {
        if (GetDocumentType() == DOCUMENT_PPTX)
        {
            mpFS->singleElementNS(XML_a, XML_buNone);
        }
        return;
    }

    Reference< XIndexAccess > rXIndexAccess;

    if (!(mAny >>= rXIndexAccess) || nLevel >= rXIndexAccess->getCount())
        return;

    SAL_INFO("oox.shape", "numbering rules");

    Sequence<PropertyValue> aPropertySequence;
    rXIndexAccess->getByIndex(nLevel) >>= aPropertySequence;

    if (!aPropertySequence.hasElements())
        return;

    SvxNumType nNumberingType = SVX_NUM_NUMBER_NONE;
    bool bSDot = false;
    bool bPBehind = false;
    bool bPBoth = false;
    sal_Unicode aBulletChar = 0x2022; // a bullet
    awt::FontDescriptor aFontDesc;
    bool bHasFontDesc = false;
    uno::Reference<graphic::XGraphic> xGraphic;
    sal_Int16 nBulletRelSize = 0;
    sal_Int16 nStartWith = 1;
    ::Color nBulletColor;
    bool bHasBulletColor = false;
    awt::Size aGraphicSize;

    for ( const PropertyValue& rPropValue : std::as_const(aPropertySequence) )
    {
        OUString aPropName( rPropValue.Name );
        SAL_INFO("oox.shape", "pro name: " << aPropName);
        if ( aPropName == "NumberingType" )
        {
            nNumberingType = static_cast<SvxNumType>(*o3tl::doAccess<sal_Int16>(rPropValue.Value));
        }
        else if ( aPropName == "Prefix" )
        {
            if( *o3tl::doAccess<OUString>(rPropValue.Value) == ")")
                bPBoth = true;
        }
        else if ( aPropName == "Suffix" )
        {
            auto s = o3tl::doAccess<OUString>(rPropValue.Value);
            if( *s == ".")
                bSDot = true;
            else if( *s == ")")
                bPBehind = true;
        }
        else if(aPropName == "BulletColor")
        {
            nBulletColor = ::Color(ColorTransparency, *o3tl::doAccess<sal_uInt32>(rPropValue.Value));
            bHasBulletColor = true;
        }
        else if ( aPropName == "BulletChar" )
        {
            aBulletChar = (*o3tl::doAccess<OUString>(rPropValue.Value))[ 0 ];
        }
        else if ( aPropName == "BulletFont" )
        {
            aFontDesc = *o3tl::doAccess<awt::FontDescriptor>(rPropValue.Value);
            bHasFontDesc = true;

            // Our numbullet dialog has set the wrong textencoding for our "StarSymbol" font,
            // instead of a Unicode encoding the encoding RTL_TEXTENCODING_SYMBOL was used.
            // Because there might exist a lot of damaged documents I added this two lines
            // which fixes the bullet problem for the export.
            if ( aFontDesc.Name.equalsIgnoreAsciiCase("StarSymbol") )
                aFontDesc.CharSet = RTL_TEXTENCODING_MS_1252;

        }
        else if ( aPropName == "BulletRelSize" )
        {
            nBulletRelSize = *o3tl::doAccess<sal_Int16>(rPropValue.Value);
        }
        else if ( aPropName == "StartWith" )
        {
            nStartWith = *o3tl::doAccess<sal_Int16>(rPropValue.Value);
        }
        else if (aPropName == "GraphicBitmap")
        {
            auto xBitmap = rPropValue.Value.get<uno::Reference<awt::XBitmap>>();
            xGraphic.set(xBitmap, uno::UNO_QUERY);
        }
        else if ( aPropName == "GraphicSize" )
        {
            aGraphicSize = *o3tl::doAccess<awt::Size>(rPropValue.Value);
            SAL_INFO("oox.shape", "graphic size: " << aGraphicSize.Width << "x" << aGraphicSize.Height);
        }
    }

    if (nNumberingType == SVX_NUM_NUMBER_NONE)
        return;

    Graphic aGraphic(xGraphic);
    if (xGraphic.is() && aGraphic.GetType() != GraphicType::NONE)
    {
        tools::Long nFirstCharHeightMm = TransformMetric(fFirstCharHeight * 100.f, FieldUnit::POINT, FieldUnit::MM);
        float fBulletSizeRel = aGraphicSize.Height / static_cast<float>(nFirstCharHeightMm) / OOX_BULLET_LIST_SCALE_FACTOR;

        OUString sRelationId;

        if (fBulletSizeRel < 1.0f)
        {
            // Add padding to get the bullet point centered in PPT
            Size aDestSize(64, 64);
            float fBulletSizeRelX = fBulletSizeRel / aGraphicSize.Height * aGraphicSize.Width;
            tools::Long nPaddingX = std::max<tools::Long>(0, std::lround((aDestSize.Width() - fBulletSizeRelX * aDestSize.Width()) / 2.f));
            tools::Long nPaddingY = std::lround((aDestSize.Height() - fBulletSizeRel * aDestSize.Height()) / 2.f);
            tools::Rectangle aDestRect(nPaddingX, nPaddingY, aDestSize.Width() - nPaddingX, aDestSize.Height() - nPaddingY);

            AlphaMask aMask(aDestSize);
            aMask.Erase(255);
            BitmapEx aSourceBitmap(aGraphic.GetBitmapEx());
            aSourceBitmap.Scale(aDestRect.GetSize());
            tools::Rectangle aSourceRect(Point(0, 0), aDestRect.GetSize());
            BitmapEx aDestBitmap(Bitmap(aDestSize, vcl::PixelFormat::N24_BPP), aMask);
            aDestBitmap.CopyPixel(aDestRect, aSourceRect, &aSourceBitmap);
            Graphic aDestGraphic(aDestBitmap);
            sRelationId = WriteImage(aDestGraphic);
            fBulletSizeRel = 1.0f;
        }
        else
        {
            sRelationId = WriteImage(aGraphic);
        }

        mpFS->singleElementNS( XML_a, XML_buSzPct,
                               XML_val, OString::number(std::min<sal_Int32>(std::lround(100000.f * fBulletSizeRel), 400000)));
        mpFS->startElementNS(XML_a, XML_buBlip);
        mpFS->singleElementNS(XML_a, XML_blip, FSNS(XML_r, XML_embed), sRelationId);
        mpFS->endElementNS( XML_a, XML_buBlip );
    }
    else
    {
        if(bHasBulletColor)
        {
               if (nBulletColor == COL_AUTO )
               {
                   nBulletColor = ::Color(ColorTransparency, mbIsBackgroundDark ? 0xffffff : 0x000000);
               }
               mpFS->startElementNS(XML_a, XML_buClr);
               WriteColor( nBulletColor );
               mpFS->endElementNS( XML_a, XML_buClr );
        }

        if( nBulletRelSize && nBulletRelSize != 100 )
            mpFS->singleElementNS( XML_a, XML_buSzPct,
                                   XML_val, OString::number(std::clamp<sal_Int32>(1000*nBulletRelSize, 25000, 400000)));
        if( bHasFontDesc )
        {
            if ( SVX_NUM_CHAR_SPECIAL == nNumberingType )
                aBulletChar = SubstituteBullet( aBulletChar, aFontDesc );
            mpFS->singleElementNS( XML_a, XML_buFont,
                                   XML_typeface, aFontDesc.Name,
                                   XML_charset, sax_fastparser::UseIf("2", aFontDesc.CharSet == awt::CharSet::SYMBOL));
        }

        OUString aAutoNumType = GetAutoNumType( nNumberingType, bSDot, bPBehind, bPBoth );

        if (!aAutoNumType.isEmpty())
        {
            mpFS->singleElementNS(XML_a, XML_buAutoNum,
                                  XML_type, aAutoNumType,
                                  XML_startAt, sax_fastparser::UseIf(OString::number(nStartWith), nStartWith > 1));
        }
        else
        {
            mpFS->singleElementNS(XML_a, XML_buChar, XML_char, OUString(aBulletChar));
        }
    }
}

void DrawingML::WriteParagraphTabStops(const Reference<XPropertySet>& rXPropSet)
{
    css::uno::Sequence<css::style::TabStop> aTabStops;
    if (GetProperty(rXPropSet, "ParaTabStops"))
        aTabStops = *o3tl::doAccess<css::uno::Sequence<css::style::TabStop>>(mAny);

    if (aTabStops.getLength() > 0)
        mpFS->startElementNS(XML_a, XML_tabLst);

    for (const css::style::TabStop& rTabStop : std::as_const(aTabStops))
    {
        OString sPosition = OString::number(GetPointFromCoordinate(rTabStop.Position));
        OString sAlignment;
        switch (rTabStop.Alignment)
        {
            case css::style::TabAlign_DECIMAL:
                sAlignment = "dec";
                break;
            case css::style::TabAlign_RIGHT:
                sAlignment = "r";
                break;
            case css::style::TabAlign_CENTER:
                sAlignment = "ctr";
                break;
            case css::style::TabAlign_LEFT:
            default:
                sAlignment = "l";
        }
        mpFS->singleElementNS(XML_a, XML_tab, XML_algn, sAlignment, XML_pos, sPosition);
    }
    if (aTabStops.getLength() > 0)
        mpFS->endElementNS(XML_a, XML_tabLst);
}

bool DrawingML::IsGroupShape( const Reference< XShape >& rXShape )
{
    bool bRet = false;
    if ( rXShape.is() )
    {
        uno::Reference<lang::XServiceInfo> xServiceInfo(rXShape, uno::UNO_QUERY_THROW);
        bRet = xServiceInfo->supportsService("com.sun.star.drawing.GroupShape");
    }
    return bRet;
}

bool DrawingML::IsDiagram(const Reference<XShape>& rXShape)
{
    uno::Reference<beans::XPropertySet> xPropSet(rXShape, uno::UNO_QUERY);
    if (!xPropSet.is())
        return false;

    // if the shape doesn't have the InteropGrabBag property, it's not a diagram
    uno::Reference<beans::XPropertySetInfo> xPropSetInfo = xPropSet->getPropertySetInfo();
    OUString aName = UNO_NAME_MISC_OBJ_INTEROPGRABBAG;
    if (!xPropSetInfo->hasPropertyByName(aName))
        return false;

    uno::Sequence<beans::PropertyValue> propList;
    xPropSet->getPropertyValue(aName) >>= propList;
    return std::any_of(std::cbegin(propList), std::cend(propList),
        [](const beans::PropertyValue& rProp) {
            // if we find any of the diagram components, it's a diagram
            OUString propName = rProp.Name;
            return propName == "OOXData" || propName == "OOXLayout" || propName == "OOXStyle"
                || propName == "OOXColor" || propName == "OOXDrawing";
        });
}

sal_Int32 DrawingML::getBulletMarginIndentation (const Reference< XPropertySet >& rXPropSet,sal_Int16 nLevel, std::u16string_view propName)
{
    if (nLevel < 0 || !GetProperty(rXPropSet, "NumberingRules"))
        return 0;

    Reference< XIndexAccess > rXIndexAccess;

    if (!(mAny >>= rXIndexAccess) || nLevel >= rXIndexAccess->getCount())
        return 0;

    SAL_INFO("oox.shape", "numbering rules");

    Sequence<PropertyValue> aPropertySequence;
    rXIndexAccess->getByIndex(nLevel) >>= aPropertySequence;

    if (!aPropertySequence.hasElements())
        return 0;

    for ( const PropertyValue& rPropValue : std::as_const(aPropertySequence) )
    {
        OUString aPropName( rPropValue.Name );
        SAL_INFO("oox.shape", "pro name: " << aPropName);
        if ( aPropName == propName )
            return *o3tl::doAccess<sal_Int32>(rPropValue.Value);
    }

    return 0;
}

const char* DrawingML::GetAlignment( style::ParagraphAdjust nAlignment )
{
    const char* sAlignment = nullptr;

    switch( nAlignment )
    {
        case style::ParagraphAdjust_CENTER:
            sAlignment = "ctr";
            break;
        case style::ParagraphAdjust_RIGHT:
            sAlignment = "r";
            break;
        case style::ParagraphAdjust_BLOCK:
            sAlignment = "just";
            break;
        default:
            ;
    }

    return sAlignment;
}

void DrawingML::WriteLinespacing( const LineSpacing& rSpacing )
{
    if( rSpacing.Mode == LineSpacingMode::PROP )
    {
        mpFS->singleElementNS( XML_a, XML_spcPct,
                               XML_val, OString::number(static_cast<sal_Int32>(rSpacing.Height)*1000));
    }
    else
    {
        mpFS->singleElementNS( XML_a, XML_spcPts,
                               XML_val, OString::number(std::lround(rSpacing.Height / 25.4 * 72)));
    }
}

void DrawingML::WriteParagraphProperties( const Reference< XTextContent >& rParagraph, float fFirstCharHeight)
{
    Reference< XPropertySet > rXPropSet( rParagraph, UNO_QUERY );
    Reference< XPropertyState > rXPropState( rParagraph, UNO_QUERY );
    PropertyState eState;

    if( !rXPropSet.is() || !rXPropState.is() )
        return;

    sal_Int16 nLevel = -1;
    if (GetProperty(rXPropSet, "NumberingLevel"))
        mAny >>= nLevel;

    sal_Int16 nTmp = sal_Int16(style::ParagraphAdjust_LEFT);
    if (GetProperty(rXPropSet, "ParaAdjust"))
        mAny >>= nTmp;
    style::ParagraphAdjust nAlignment = static_cast<style::ParagraphAdjust>(nTmp);

    bool bHasLinespacing = false;
    LineSpacing aLineSpacing;
    if (GetPropertyAndState(rXPropSet, rXPropState, "ParaLineSpacing", eState)
        && eState == beans::PropertyState_DIRECT_VALUE)
        bHasLinespacing = ( mAny >>= aLineSpacing );

    bool bRtl = false;
    if (GetProperty(rXPropSet, "WritingMode"))
    {
        sal_Int16 nWritingMode;
        if( ( mAny >>= nWritingMode ) && nWritingMode == text::WritingMode2::RL_TB )
        {
            bRtl = true;
        }
    }

    sal_Int32 nParaLeftMargin = 0;
    sal_Int32 nParaFirstLineIndent = 0;

    if (GetProperty(rXPropSet, "ParaLeftMargin"))
        mAny >>= nParaLeftMargin;
    if (GetProperty(rXPropSet, "ParaFirstLineIndent"))
        mAny >>= nParaFirstLineIndent;

    sal_Int32 nParaTopMargin = 0;
    sal_Int32 nParaBottomMargin = 0;

    if (GetProperty(rXPropSet, "ParaTopMargin"))
        mAny >>= nParaTopMargin;
    if (GetProperty(rXPropSet, "ParaBottomMargin"))
        mAny >>= nParaBottomMargin;

    sal_Int32 nLeftMargin =  getBulletMarginIndentation ( rXPropSet, nLevel,u"LeftMargin");
    sal_Int32 nLineIndentation = getBulletMarginIndentation ( rXPropSet, nLevel,u"FirstLineOffset");

    if( !(nLevel != -1
        || nAlignment != style::ParagraphAdjust_LEFT
        || bHasLinespacing) )
        return;

    if (nParaLeftMargin) // For Paragraph
        mpFS->startElementNS( XML_a, XML_pPr,
                           XML_lvl, sax_fastparser::UseIf(OString::number(nLevel), nLevel > 0),
                           XML_marL, sax_fastparser::UseIf(OString::number(oox::drawingml::convertHmmToEmu(nParaLeftMargin)), nParaLeftMargin > 0),
                           XML_indent, sax_fastparser::UseIf(OString::number(oox::drawingml::convertHmmToEmu(nParaFirstLineIndent)), nParaFirstLineIndent != 0),
                           XML_algn, GetAlignment( nAlignment ),
                           XML_rtl, sax_fastparser::UseIf(ToPsz10(bRtl), bRtl));
    else
        mpFS->startElementNS( XML_a, XML_pPr,
                           XML_lvl, sax_fastparser::UseIf(OString::number(nLevel), nLevel > 0),
                           XML_marL, sax_fastparser::UseIf(OString::number(oox::drawingml::convertHmmToEmu(nLeftMargin)), nLeftMargin > 0),
                           XML_indent, sax_fastparser::UseIf(OString::number(oox::drawingml::convertHmmToEmu(nLineIndentation)), nLineIndentation != 0),
                           XML_algn, GetAlignment( nAlignment ),
                           XML_rtl, sax_fastparser::UseIf(ToPsz10(bRtl), bRtl));


    if( bHasLinespacing )
    {
        mpFS->startElementNS(XML_a, XML_lnSpc);
        WriteLinespacing( aLineSpacing );
        mpFS->endElementNS( XML_a, XML_lnSpc );
    }

    if( nParaTopMargin != 0 )
    {
        mpFS->startElementNS(XML_a, XML_spcBef);
        {
            mpFS->singleElementNS( XML_a, XML_spcPts,
                                   XML_val, OString::number(std::lround(nParaTopMargin / 25.4 * 72)));
        }
        mpFS->endElementNS( XML_a, XML_spcBef );
    }

    if( nParaBottomMargin != 0 )
    {
        mpFS->startElementNS(XML_a, XML_spcAft);
        {
            mpFS->singleElementNS( XML_a, XML_spcPts,
                                   XML_val, OString::number(std::lround(nParaBottomMargin / 25.4 * 72)));
        }
        mpFS->endElementNS( XML_a, XML_spcAft );
    }

    WriteParagraphNumbering( rXPropSet, fFirstCharHeight, nLevel );

    WriteParagraphTabStops( rXPropSet );

    mpFS->endElementNS( XML_a, XML_pPr );
}

void DrawingML::WriteParagraph( const Reference< XTextContent >& rParagraph,
                                bool& rbOverridingCharHeight, sal_Int32& rnCharHeight,
                                const css::uno::Reference< css::beans::XPropertySet >& rXShapePropSet)
{
    Reference< XEnumerationAccess > access( rParagraph, UNO_QUERY );
    if( !access.is() )
        return;

    Reference< XEnumeration > enumeration( access->createEnumeration() );
    if( !enumeration.is() )
        return;

    mpFS->startElementNS(XML_a, XML_p);

    bool bPropertiesWritten = false;
    while( enumeration->hasMoreElements() )
    {
        Reference< XTextRange > run;
        Any any ( enumeration->nextElement() );

        if (any >>= run)
        {
            if( !bPropertiesWritten )
            {
                float fFirstCharHeight = rnCharHeight / 1000.;
                Reference< XPropertySet > xFirstRunPropSet (run, UNO_QUERY);
                Reference< XPropertySetInfo > xFirstRunPropSetInfo = xFirstRunPropSet->getPropertySetInfo();
                if( xFirstRunPropSetInfo->hasPropertyByName("CharHeight") )
                    fFirstCharHeight = xFirstRunPropSet->getPropertyValue("CharHeight").get<float>();
                WriteParagraphProperties( rParagraph, fFirstCharHeight );
                bPropertiesWritten = true;
            }
            WriteRun( run, rbOverridingCharHeight, rnCharHeight, rXShapePropSet);
        }
    }
    Reference< XPropertySet > rXPropSet( rParagraph, UNO_QUERY );
    sal_Int16 nDummy = -1;
    WriteRunProperties(rXPropSet, false, XML_endParaRPr, false, rbOverridingCharHeight,
                       rnCharHeight, nDummy, rXShapePropSet);

    mpFS->endElementNS( XML_a, XML_p );
}

bool DrawingML::IsFontworkShape(const css::uno::Reference<css::beans::XPropertySet>& rXShapePropSet)
{
    bool bResult(false);
    if (rXShapePropSet.is())
    {
        Sequence<PropertyValue> aCustomShapeGeometryProps;
        if (GetProperty(rXShapePropSet, "CustomShapeGeometry"))
        {
            mAny >>= aCustomShapeGeometryProps;
            uno::Sequence<beans::PropertyValue> aTextPathSeq;
            for (const auto& rProp : std::as_const(aCustomShapeGeometryProps))
            {
                if (rProp.Name == "TextPath")
                {
                    rProp.Value >>= aTextPathSeq;
                    for (const auto& rTextPathItem : std::as_const(aTextPathSeq))
                    {
                        if (rTextPathItem.Name == "TextPath")
                        {
                            rTextPathItem.Value >>= bResult;
                            break;
                        }
                    }
                    break;
                }
            }
        }
    }
    return bResult;
}

void DrawingML::WriteText(const Reference<XInterface>& rXIface, bool bBodyPr, bool bText,
                           sal_Int32 nXmlNamespace)
{
    // ToDo: Fontwork in DOCX
    Reference< XText > xXText( rXIface, UNO_QUERY );
    if( !xXText.is() )
        return;

    Reference< XPropertySet > rXPropSet( rXIface, UNO_QUERY );

    sal_Int32 nTextPreRotateAngle = 0;
    double nTextRotateAngle = 0;

#define DEFLRINS 254
#define DEFTBINS 127
    sal_Int32 nLeft, nRight, nTop, nBottom;
    nLeft = nRight = DEFLRINS;
    nTop = nBottom = DEFTBINS;

    // top inset looks a bit different compared to ppt export
    // check if something related doesn't work as expected
    if (GetProperty(rXPropSet, "TextLeftDistance"))
        mAny >>= nLeft;
    if (GetProperty(rXPropSet, "TextRightDistance"))
        mAny >>= nRight;
    if (GetProperty(rXPropSet, "TextUpperDistance"))
        mAny >>= nTop;
    if (GetProperty(rXPropSet, "TextLowerDistance"))
        mAny >>= nBottom;

    TextVerticalAdjust eVerticalAlignment( TextVerticalAdjust_TOP );
    const char* sVerticalAlignment = nullptr;
    if (GetProperty(rXPropSet, "TextVerticalAdjust"))
        mAny >>= eVerticalAlignment;
    sVerticalAlignment = GetTextVerticalAdjust(eVerticalAlignment);

    const char* sWritingMode = nullptr;
    bool bVertical = false;
    if (GetProperty(rXPropSet, "TextWritingMode"))
    {
        WritingMode eMode;

        if( ( mAny >>= eMode ) && eMode == WritingMode_TB_RL )
        {
            sWritingMode = "eaVert";
            bVertical = true;
        }
    }

    bool bIsFontworkShape(IsFontworkShape(rXPropSet));
    Sequence<drawing::EnhancedCustomShapeAdjustmentValue> aAdjustmentSeq;
    uno::Sequence<beans::PropertyValue> aTextPathSeq;
    bool bScaleX(false);
    OUString sShapeType("non-primitive");
    // ToDo move to InteropGrabBag
    OUString sMSWordPresetTextWarp;

    if (GetProperty(rXPropSet, "CustomShapeGeometry"))
    {
        Sequence< PropertyValue > aProps;
        if ( mAny >>= aProps )
        {
            for ( const auto& rProp : std::as_const(aProps) )
            {
                if ( rProp.Name == "TextPreRotateAngle" && ( rProp.Value >>= nTextPreRotateAngle ) )
                {
                    if ( nTextPreRotateAngle == -90 )
                    {
                        sWritingMode = "vert";
                        bVertical = true;
                    }
                    else if ( nTextPreRotateAngle == -270 )
                    {
                        sWritingMode = "vert270";
                        bVertical = true;
                    }
                }
                else if (rProp.Name == "AdjustmentValues")
                    rProp.Value >>= aAdjustmentSeq;
                else if( rProp.Name == "TextRotateAngle" )
                    rProp.Value >>= nTextRotateAngle;
                else if (rProp.Name == "Type")
                    rProp.Value >>= sShapeType;
                else if (rProp.Name == "TextPath")
                {
                    rProp.Value >>= aTextPathSeq;
                    for (const auto& rTextPathItem : std::as_const(aTextPathSeq))
                    {
                        if (rTextPathItem.Name == "ScaleX")
                            rTextPathItem.Value >>= bScaleX;
                    }
                }
                else if (rProp.Name == "PresetTextWarp")
                    rProp.Value >>= sMSWordPresetTextWarp;
            }
        }
    }
    else
    {
        if (mpTextExport)
        {
            uno::Reference<drawing::XShape> xShape(rXIface, uno::UNO_QUERY);
            if (xShape)
            {
                auto xTextFrame = mpTextExport->GetUnoTextFrame(xShape);
                if (xTextFrame)
                {
                    uno::Reference<beans::XPropertySet> xPropSet(xTextFrame, uno::UNO_QUERY);
                    auto aAny = xPropSet->getPropertyValue("WritingMode");
                    sal_Int16 nWritingMode;
                    if (aAny >>= nWritingMode)
                    {
                        switch (nWritingMode)
                        {
                        case WritingMode2::TB_RL:
                            sWritingMode = "vert";
                            bVertical = true;
                            break;
                        case WritingMode2::BT_LR:
                            sWritingMode = "vert270";
                            bVertical = true;
                            break;
                        default:
                            break;
                        }
                    }
                }
            }
        }
    }
    OUString sPresetWarp(PresetGeometryTypeNames::GetMsoName(sShapeType));
    // ODF may have user defined TextPath, use "textPlain" as ersatz.
    if (sPresetWarp.isEmpty())
        sPresetWarp = bIsFontworkShape ? std::u16string_view(u"textPlain") : std::u16string_view(u"textNoShape");

    bool bFromWordArt = !bScaleX
                        && ( sPresetWarp == "textArchDown" || sPresetWarp == "textArchUp"
                            || sPresetWarp == "textButton" || sPresetWarp == "textCircle");

    TextHorizontalAdjust eHorizontalAlignment( TextHorizontalAdjust_CENTER );
    bool bHorizontalCenter = false;
    if (GetProperty(rXPropSet, "TextHorizontalAdjust"))
        mAny >>= eHorizontalAlignment;
    if( eHorizontalAlignment == TextHorizontalAdjust_CENTER )
        bHorizontalCenter = true;
    else if( bVertical && eHorizontalAlignment == TextHorizontalAdjust_LEFT )
        sVerticalAlignment = "b";

    bool bHasWrap = false;
    bool bWrap = false;
    // Only custom shapes obey the TextWordWrap option, normal text always wraps.
    if (dynamic_cast<SvxCustomShape*>(rXIface.get()) && GetProperty(rXPropSet, "TextWordWrap"))
    {
        mAny >>= bWrap;
        bHasWrap = true;
    }

    if (bBodyPr)
    {
        const char* pWrap = (bHasWrap && !bWrap) || bIsFontworkShape ? "none" : nullptr;
        if (GetDocumentType() == DOCUMENT_DOCX)
        {
            // In case of DOCX, if we want to have the same effect as
            // TextShape's automatic word wrapping, then we need to set
            // wrapping to square.
            uno::Reference<lang::XServiceInfo> xServiceInfo(rXIface, uno::UNO_QUERY);
            if (xServiceInfo.is() && xServiceInfo->supportsService("com.sun.star.drawing.TextShape"))
                pWrap = "square";
        }

        std::optional<OUString> sHorzOverflow;
        std::optional<OUString> sVertOverflow;
        sal_Int32 nShapeRotateAngle = rXPropSet->getPropertyValue("RotateAngle").get<sal_Int32>() / 300;
        sal_Int16 nCols = 0;
        sal_Int32 nColSpacing = -1;
        if (GetProperty(rXPropSet, "TextColumns"))
        {
            if (css::uno::Reference<css::text::XTextColumns> xCols{ mAny, css::uno::UNO_QUERY })
            {
                nCols = xCols->getColumnCount();
                if (css::uno::Reference<css::beans::XPropertySet> xProps{ mAny,
                                                                          css::uno::UNO_QUERY })
                {
                    if (GetProperty(xProps, "AutomaticDistance"))
                        mAny >>= nColSpacing;
                }
            }
        }

        std::optional<OString> isUpright;
        if (GetProperty(rXPropSet, "InteropGrabBag"))
        {
            if (rXPropSet->getPropertySetInfo()->hasPropertyByName("InteropGrabBag"))
            {
                bool bUpright = false;
                sal_Int32 nOldShapeRotation = 0;
                sal_Int32 nOldTextRotation = 0;
                uno::Sequence<beans::PropertyValue> aGrabBag;
                rXPropSet->getPropertyValue("InteropGrabBag") >>= aGrabBag;
                for (const auto& aProp : std::as_const(aGrabBag))
                {
                    if (aProp.Name == "Upright")
                    {
                        aProp.Value >>= bUpright;
                        isUpright = OString(bUpright ? "1" : "0");
                    }
                    else if (aProp.Name == "horzOverflow")
                    {
                        OUString sValue;
                        aProp.Value >>= sValue;
                        sHorzOverflow = sValue;
                    }
                    else if (aProp.Name == "vertOverflow")
                    {
                        OUString sValue;
                        aProp.Value >>= sValue;
                        sVertOverflow = sValue;
                    }
                }
                if (bUpright)
                {
                    for (auto& aProp : aGrabBag)
                    {
                        if (aProp.Name == "nShapeRotationAtImport")
                            aProp.Value >>= nOldShapeRotation;
                        else if (aProp.Name == "nTextRotationAtImport")
                            aProp.Value >>= nOldTextRotation;
                    }
                    // So our shape with the textbox in it was not rotated.
                    // Keep upright and make the preRotateAngle 0, it is an attribute
                    // of textBodyPr and must be 0 when upright is true, otherwise
                    // bad rotation happens in MSO.
                    if (nShapeRotateAngle == nOldShapeRotation && nShapeRotateAngle == nOldTextRotation)
                        nTextPreRotateAngle = 0;
                    // So we rotated the shape, in this case lose upright and do
                    // as LO normally does.
                    else
                        isUpright.reset();
                }
            }
        }

        mpFS->startElementNS( (nXmlNamespace ? nXmlNamespace : XML_a), XML_bodyPr,
                               XML_numCol, sax_fastparser::UseIf(OString::number(nCols), nCols > 0),
                               XML_spcCol, sax_fastparser::UseIf(OString::number(oox::drawingml::convertHmmToEmu(nColSpacing)), nCols > 0 && nColSpacing >= 0),
                               XML_wrap, pWrap,
                               XML_horzOverflow, sHorzOverflow,
                               XML_vertOverflow, sVertOverflow,
                               XML_fromWordArt, sax_fastparser::UseIf("1", bFromWordArt),
                               XML_lIns, sax_fastparser::UseIf(OString::number(oox::drawingml::convertHmmToEmu(nLeft)), nLeft != DEFLRINS),
                               XML_rIns, sax_fastparser::UseIf(OString::number(oox::drawingml::convertHmmToEmu(nRight)), nRight != DEFLRINS),
                               XML_tIns, sax_fastparser::UseIf(OString::number(oox::drawingml::convertHmmToEmu(nTop)), nTop != DEFTBINS),
                               XML_bIns, sax_fastparser::UseIf(OString::number(oox::drawingml::convertHmmToEmu(nBottom)), nBottom != DEFTBINS),
                               XML_anchor, sVerticalAlignment,
                               XML_anchorCtr, sax_fastparser::UseIf("1", bHorizontalCenter),
                               XML_vert, sWritingMode,
                               XML_upright, isUpright,
                               XML_rot, sax_fastparser::UseIf(oox::drawingml::calcRotationValue((nTextPreRotateAngle + nTextRotateAngle) * 100), (nTextPreRotateAngle + nTextRotateAngle) != 0));
        if (bIsFontworkShape)
        {
            if (aAdjustmentSeq.hasElements())
            {
                mpFS->startElementNS(XML_a, XML_prstTxWarp, XML_prst, sPresetWarp);
                mpFS->startElementNS(XML_a, XML_avLst);
                bool bHasTwoHandles(
                    sPresetWarp == "textArchDownPour" || sPresetWarp == "textArchUpPour"
                    || sPresetWarp == "textButtonPour" || sPresetWarp == "textCirclePour"
                    || sPresetWarp == "textDoubleWave1" || sPresetWarp == "textWave1"
                    || sPresetWarp == "textWave2" || sPresetWarp == "textWave4");
                for (sal_Int32 i = 0, nElems = aAdjustmentSeq.getLength(); i < nElems; ++i )
                {
                    OString sName = "adj" + (bHasTwoHandles ? OString::number(i + 1) : OString());
                    double fValue(0.0);
                    if (aAdjustmentSeq[i].Value.getValueTypeClass() == TypeClass_DOUBLE)
                        aAdjustmentSeq[i].Value >>= fValue;
                    else
                    {
                        sal_Int32 nNumber(0);
                        aAdjustmentSeq[i].Value >>= nNumber;
                        fValue = static_cast<double>(nNumber);
                    }
                    // Convert from binary coordinate system with viewBox "0 0 21600 21600" and simple degree
                    // to DrawingML with coordinate range 0..100000 and angle in 1/60000 degree.
                    // Reverse to conversion in lcl_createPresetShape in drawingml/shape.cxx on import.
                    if (sPresetWarp == "textArchDown" || sPresetWarp == "textArchUp"
                        || sPresetWarp == "textButton" || sPresetWarp == "textCircle"
                        || ((i == 0)
                            && (sPresetWarp == "textArchDownPour" || sPresetWarp == "textArchUpPour"
                                || sPresetWarp == "textButtonPour" || sPresetWarp == "textCirclePour")))
                    {
                        fValue *= 60000.0;
                        if (fValue < 0)
                            fValue += 21600000;
                    }
                    else if ((i == 1)
                             && (sPresetWarp == "textDoubleWave1" || sPresetWarp == "textWave1"
                            || sPresetWarp == "textWave2" || sPresetWarp == "textWave4"))
                    {
                        fValue = fValue / 0.216 - 50000.0;
                    }
                    else if ((i == 1)
                             && (sPresetWarp == "textArchDownPour"
                                 || sPresetWarp == "textArchUpPour"
                                 || sPresetWarp == "textButtonPour"
                                 || sPresetWarp == "textCirclePour"))
                    {
                        fValue /= 0.108;
                    }
                    else
                    {
                        fValue /= 0.216;
                    }
                    OString sFmla = "val " + OString::number(std::lround(fValue));
                    mpFS->singleElementNS(XML_a, XML_gd, XML_name, sName, XML_fmla, sFmla);
                    // There exists faulty Favorite shapes with one handle but two adjustment values.
                    if (!bHasTwoHandles)
                        break;
                }
                mpFS->endElementNS(XML_a, XML_avLst);
                mpFS->endElementNS(XML_a, XML_prstTxWarp);
            }
            else
            {
                mpFS->singleElementNS(XML_a, XML_prstTxWarp, XML_prst, sPresetWarp);
            }
        }
        else if (GetDocumentType() == DOCUMENT_DOCX)
        {
            // interim solution for fdo#80897, roundtrip DOCX > LO > DOCX
            if (!sMSWordPresetTextWarp.isEmpty())
                mpFS->singleElementNS(XML_a, XML_prstTxWarp, XML_prst, sMSWordPresetTextWarp);
        }

        if (GetDocumentType() == DOCUMENT_DOCX || GetDocumentType() == DOCUMENT_XLSX)
        {
            // tdf#112312: only custom shapes obey the TextAutoGrowHeight option
            bool bTextAutoGrowHeight = false;
            uno::Reference<drawing::XShape> xShape(rXIface, uno::UNO_QUERY);
            auto pSdrObjCustomShape = xShape.is() ? dynamic_cast<SdrObjCustomShape*>(GetSdrObjectFromXShape(xShape)) : nullptr;
            if (pSdrObjCustomShape && GetProperty(rXPropSet, "TextAutoGrowHeight"))
            {
                mAny >>= bTextAutoGrowHeight;
            }
            mpFS->singleElementNS(XML_a, (bTextAutoGrowHeight ? XML_spAutoFit : XML_noAutofit));
        }
        if (GetDocumentType() == DOCUMENT_PPTX)
        {
            TextFitToSizeType eFit = TextFitToSizeType_NONE;
            if (GetProperty(rXPropSet, "TextFitToSize"))
                mAny >>= eFit;

            if (eFit == TextFitToSizeType_AUTOFIT)
            {
                const sal_Int32 MAX_SCALE_VAL = 100000;
                sal_Int32 nFontScale = MAX_SCALE_VAL;
                SvxShapeText* pTextShape = dynamic_cast<SvxShapeText*>(rXIface.get());
                if (pTextShape)
                {
                    SdrTextObj* pTextObject = dynamic_cast<SdrTextObj*>(pTextShape->GetSdrObject());
                    if (pTextObject)
                        nFontScale = pTextObject->GetFontScaleY() * 1000;
                }

                mpFS->singleElementNS(XML_a, XML_normAutofit, XML_fontScale,
                    sax_fastparser::UseIf(OString::number(nFontScale), nFontScale < MAX_SCALE_VAL && nFontScale > 0));
            }
            else
            {
                // tdf#127030: Only custom shapes obey the TextAutoGrowHeight option.
                bool bTextAutoGrowHeight = false;
                if (dynamic_cast<SvxCustomShape*>(rXIface.get()) && GetProperty(rXPropSet, "TextAutoGrowHeight"))
                    mAny >>= bTextAutoGrowHeight;
                mpFS->singleElementNS(XML_a, (bTextAutoGrowHeight ? XML_spAutoFit : XML_noAutofit));
            }
        }

        WriteShape3DEffects( rXPropSet );

        mpFS->endElementNS((nXmlNamespace ? nXmlNamespace : XML_a), XML_bodyPr);
    }

    Reference< XEnumerationAccess > access( xXText, UNO_QUERY );
    if( !access.is() || !bText )
        return;

    Reference< XEnumeration > enumeration( access->createEnumeration() );
    if( !enumeration.is() )
        return;

    uno::Reference<drawing::XShape> xShape(rXIface, uno::UNO_QUERY);
    SdrObject* pSdrObject = xShape.is() ? GetSdrObjectFromXShape(xShape) : nullptr;
    const SdrTextObj* pTxtObj = dynamic_cast<SdrTextObj*>( pSdrObject );
    if (pTxtObj && mpTextExport)
    {
        const OutlinerParaObject* pParaObj = nullptr;
        bool bOwnParaObj = false;

        /*
        #i13885#
        When the object is actively being edited, that text is not set into
        the objects normal text object, but lives in a separate object.
        */
        if (pTxtObj->IsTextEditActive())
        {
            pParaObj = pTxtObj->CreateEditOutlinerParaObject().release();
            bOwnParaObj = true;
        }
        else
            pParaObj = pTxtObj->GetOutlinerParaObject();

        if (pParaObj)
        {
            // this is reached only in case some text is attached to the shape
            mpTextExport->WriteOutliner(*pParaObj);
            if (bOwnParaObj)
                delete pParaObj;
        }
        return;
    }

    bool bOverridingCharHeight = false;
    sal_Int32 nCharHeight = -1;

    while( enumeration->hasMoreElements() )
    {
        Reference< XTextContent > paragraph;
        Any any ( enumeration->nextElement() );

        if( any >>= paragraph)
            WriteParagraph( paragraph, bOverridingCharHeight, nCharHeight, rXPropSet );
    }
}

void DrawingML::WritePresetShape( const char* pShape , std::vector< std::pair<sal_Int32,sal_Int32>> & rAvList )
{
    mpFS->startElementNS(XML_a, XML_prstGeom, XML_prst, pShape);
    if ( !rAvList.empty() )
    {

        mpFS->startElementNS(XML_a, XML_avLst);
        for (auto const& elem : rAvList)
        {
            OString sName = "adj" + ( ( elem.first > 0 ) ? OString::number(elem.first) : OString() );
            OString sFmla = "val " + OString::number( elem.second );

            mpFS->singleElementNS(XML_a, XML_gd, XML_name, sName, XML_fmla, sFmla);
        }
        mpFS->endElementNS( XML_a, XML_avLst );
    }
    else
        mpFS->singleElementNS(XML_a, XML_avLst);

    mpFS->endElementNS(  XML_a, XML_prstGeom );
}

void DrawingML::WritePresetShape( const char* pShape )
{
    mpFS->startElementNS(XML_a, XML_prstGeom, XML_prst, pShape);
    mpFS->singleElementNS(XML_a, XML_avLst);
    mpFS->endElementNS(  XML_a, XML_prstGeom );
}

static std::map< OString, std::vector<OString> > lcl_getAdjNames()
{
    std::map< OString, std::vector<OString> > aRet;

    OUString aPath("$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER "/filter/oox-drawingml-adj-names");
    rtl::Bootstrap::expandMacros(aPath);
    SvFileStream aStream(aPath, StreamMode::READ);
    if (aStream.GetError() != ERRCODE_NONE)
        SAL_WARN("oox.shape", "failed to open oox-drawingml-adj-names");
    OString aLine;
    bool bNotDone = aStream.ReadLine(aLine);
    while (bNotDone)
    {
        sal_Int32 nIndex = 0;
        // Each line is in a "key\tvalue" format: read the key, the rest is the value.
        OString aKey = aLine.getToken(0, '\t', nIndex);
        OString aValue = aLine.copy(nIndex);
        aRet[aKey].push_back(aValue);
        bNotDone = aStream.ReadLine(aLine);
    }
    return aRet;
}

void DrawingML::WritePresetShape( const char* pShape, MSO_SPT eShapeType, bool bPredefinedHandlesUsed, const PropertyValue& rProp )
{
    static std::map< OString, std::vector<OString> > aAdjMap = lcl_getAdjNames();
    // If there are predefined adj names for this shape type, look them up now.
    std::vector<OString> aAdjustments;
    if (aAdjMap.find(OString(pShape)) != aAdjMap.end())
        aAdjustments = aAdjMap[OString(pShape)];

    mpFS->startElementNS(XML_a, XML_prstGeom, XML_prst, pShape);
    mpFS->startElementNS(XML_a, XML_avLst);

    Sequence< drawing::EnhancedCustomShapeAdjustmentValue > aAdjustmentSeq;
    if ( ( rProp.Value >>= aAdjustmentSeq )
         && eShapeType != mso_sptActionButtonForwardNext  // we have adjustments values for these type of shape, but MSO doesn't like them
         && eShapeType != mso_sptActionButtonBackPrevious // so they are now disabled
         && pShape != std::string_view("rect") //some shape types are commented out in pCustomShapeTypeTranslationTable[] & are being defaulted to rect & rect does not have adjustment values/name.
        )
    {
        SAL_INFO("oox.shape", "adj seq len: " << aAdjustmentSeq.getLength());
        sal_Int32 nAdjustmentsWhichNeedsToBeConverted = 0;
        if ( bPredefinedHandlesUsed )
            EscherPropertyContainer::LookForPolarHandles( eShapeType, nAdjustmentsWhichNeedsToBeConverted );

        sal_Int32 nValue, nLength = aAdjustmentSeq.getLength();
        // aAdjustments will give info about the number of adj values for a particular geometry. For example for hexagon aAdjustments.size() will be 2 and for circular arrow it will be 5 as per lcl_getAdjNames.
        // Sometimes there are more values than needed, so we ignore the excessive ones.
        if (aAdjustments.size() <= o3tl::make_unsigned(nLength))
        {
            for (sal_Int32 i = 0; i < static_cast<sal_Int32>(aAdjustments.size()); i++)
            {
                if( EscherPropertyContainer::GetAdjustmentValue( aAdjustmentSeq[ i ], i, nAdjustmentsWhichNeedsToBeConverted, nValue ) )
                {
                    // If the document model doesn't have an adjustment name (e.g. shape was created from VML), then take it from the predefined list.
                    OString aAdjName = aAdjustmentSeq[i].Name.isEmpty()
                                           ? aAdjustments[i]
                                           : aAdjustmentSeq[i].Name.toUtf8();

                    mpFS->singleElementNS( XML_a, XML_gd,
                                       XML_name, aAdjName,
                                       XML_fmla, "val " + OString::number(nValue));
                }
            }
        }
    }

    mpFS->endElementNS( XML_a, XML_avLst );
    mpFS->endElementNS(  XML_a, XML_prstGeom );
}

bool DrawingML::WriteCustomGeometry(
    const Reference< XShape >& rXShape,
    const SdrObjCustomShape& rSdrObjCustomShape)
{
    uno::Reference< beans::XPropertySet > aXPropSet;
    uno::Any aAny( rXShape->queryInterface(cppu::UnoType<beans::XPropertySet>::get()));

    if ( ! (aAny >>= aXPropSet) )
        return false;

    try
    {
        aAny = aXPropSet->getPropertyValue( "CustomShapeGeometry" );
        if ( !aAny.hasValue() )
            return false;
    }
    catch( const ::uno::Exception& )
    {
        return false;
    }


    auto pGeometrySeq = o3tl::tryAccess<uno::Sequence<beans::PropertyValue>>(aAny);

    if ( pGeometrySeq )
    {
        for( const beans::PropertyValue& rProp : *pGeometrySeq )
        {
            if ( rProp.Name == "Path" )
            {
                uno::Sequence<beans::PropertyValue> aPathProp;
                rProp.Value >>= aPathProp;

                uno::Sequence<drawing::EnhancedCustomShapeParameterPair> aPairs;
                uno::Sequence<drawing::EnhancedCustomShapeSegment> aSegments;
                uno::Sequence<awt::Size> aPathSize;
                for (const beans::PropertyValue& rPathProp : std::as_const(aPathProp))
                {
                    if (rPathProp.Name == "Coordinates")
                        rPathProp.Value >>= aPairs;
                    else if (rPathProp.Name == "Segments")
                        rPathProp.Value >>= aSegments;
                    else if (rPathProp.Name == "SubViewSize")
                        rPathProp.Value >>= aPathSize;
                }

                if ( !aPairs.hasElements() )
                    return false;

                if ( !aSegments.hasElements() )
                {
                    aSegments = uno::Sequence<drawing::EnhancedCustomShapeSegment>
                    {
                        { drawing::EnhancedCustomShapeSegmentCommand::MOVETO, 1 },
                        { drawing::EnhancedCustomShapeSegmentCommand::LINETO,
                          static_cast<sal_Int16>(std::min( aPairs.getLength() - 1, sal_Int32(32767) )) },
                        { drawing::EnhancedCustomShapeSegmentCommand::CLOSESUBPATH, 0 },
                        { drawing::EnhancedCustomShapeSegmentCommand::ENDSUBPATH, 0 }
                    };
                };

                int nExpectedPairCount = std::accumulate(std::cbegin(aSegments), std::cend(aSegments), 0,
                    [](const int nSum, const drawing::EnhancedCustomShapeSegment& rSegment) { return nSum + rSegment.Count; });

                if ( nExpectedPairCount > aPairs.getLength() )
                {
                    SAL_WARN("oox.shape", "Segments need " << nExpectedPairCount << " coordinates, but Coordinates have only " << aPairs.getLength() << " pairs.");
                    return false;
                }

                mpFS->startElementNS(XML_a, XML_custGeom);
                mpFS->singleElementNS(XML_a, XML_avLst);
                mpFS->singleElementNS(XML_a, XML_gdLst);
                mpFS->singleElementNS(XML_a, XML_ahLst);
                mpFS->singleElementNS(XML_a, XML_rect, XML_l, "l", XML_t, "t",
                                      XML_r, "r", XML_b, "b");
                mpFS->startElementNS(XML_a, XML_pathLst);

                if ( aPathSize.hasElements() )
                {
                    mpFS->startElementNS( XML_a, XML_path,
                          XML_w, OString::number(aPathSize[0].Width),
                          XML_h, OString::number(aPathSize[0].Height) );
                }
                else
                {
                    sal_Int32 nXMin(0);
                    aPairs[0].First.Value >>= nXMin;
                    sal_Int32 nXMax = nXMin;
                    sal_Int32 nYMin(0);
                    aPairs[0].Second.Value >>= nYMin;
                    sal_Int32 nYMax = nYMin;

                    for ( const auto& rPair : std::as_const(aPairs) )
                    {
                        sal_Int32 nX = GetCustomGeometryPointValue(rPair.First, rSdrObjCustomShape);
                        sal_Int32 nY = GetCustomGeometryPointValue(rPair.Second, rSdrObjCustomShape);
                        if (nX < nXMin)
                            nXMin = nX;
                        if (nY < nYMin)
                            nYMin = nY;
                        if (nX > nXMax)
                            nXMax = nX;
                        if (nY > nYMax)
                            nYMax = nY;
                    }
                    mpFS->startElementNS( XML_a, XML_path,
                          XML_w, OString::number(nXMax - nXMin),
                          XML_h, OString::number(nYMax - nYMin) );
                }


                int nPairIndex = 0;
                bool bOK = true;
                for (const auto& rSegment : std::as_const(aSegments))
                {
                    if ( rSegment.Command == drawing::EnhancedCustomShapeSegmentCommand::CLOSESUBPATH )
                    {
                        mpFS->singleElementNS(XML_a, XML_close);
                    }
                    for (int k = 0; k < rSegment.Count && bOK; ++k)
                    {
                        switch( rSegment.Command )
                        {
                            case drawing::EnhancedCustomShapeSegmentCommand::MOVETO :
                            {
                                if (nPairIndex >= aPairs.getLength())
                                    bOK = false;
                                else
                                {
                                    mpFS->startElementNS(XML_a, XML_moveTo);
                                    WriteCustomGeometryPoint(aPairs[nPairIndex], rSdrObjCustomShape);
                                    mpFS->endElementNS( XML_a, XML_moveTo );
                                    nPairIndex++;
                                }
                                break;
                            }
                            case drawing::EnhancedCustomShapeSegmentCommand::LINETO :
                            {
                                if (nPairIndex >= aPairs.getLength())
                                    bOK = false;
                                else
                                {
                                    mpFS->startElementNS(XML_a, XML_lnTo);
                                    WriteCustomGeometryPoint(aPairs[nPairIndex], rSdrObjCustomShape);
                                    mpFS->endElementNS( XML_a, XML_lnTo );
                                    nPairIndex++;
                                }
                                break;
                            }
                            case drawing::EnhancedCustomShapeSegmentCommand::CURVETO :
                            {
                                if (nPairIndex + 2 >= aPairs.getLength())
                                    bOK = false;
                                else
                                {
                                    mpFS->startElementNS(XML_a, XML_cubicBezTo);
                                    for( sal_uInt8 l = 0; l <= 2; ++l )
                                    {
                                        WriteCustomGeometryPoint(aPairs[nPairIndex+l], rSdrObjCustomShape);
                                    }
                                    mpFS->endElementNS( XML_a, XML_cubicBezTo );
                                    nPairIndex += 3;
                                }
                                break;
                            }
                            case drawing::EnhancedCustomShapeSegmentCommand::ANGLEELLIPSETO :
                            case drawing::EnhancedCustomShapeSegmentCommand::ANGLEELLIPSE :
                            {
                                nPairIndex += 3;
                                break;
                            }
                            case drawing::EnhancedCustomShapeSegmentCommand::ARCTO :
                            case drawing::EnhancedCustomShapeSegmentCommand::ARC :
                            case drawing::EnhancedCustomShapeSegmentCommand::CLOCKWISEARCTO :
                            case drawing::EnhancedCustomShapeSegmentCommand::CLOCKWISEARC :
                            {
                                nPairIndex += 4;
                                break;
                            }
                            case drawing::EnhancedCustomShapeSegmentCommand::ELLIPTICALQUADRANTX :
                            case drawing::EnhancedCustomShapeSegmentCommand::ELLIPTICALQUADRANTY :
                            {
                                nPairIndex++;
                                break;
                            }
                            case drawing::EnhancedCustomShapeSegmentCommand::QUADRATICCURVETO :
                            {
                                if (nPairIndex + 1 >= aPairs.getLength())
                                    bOK = false;
                                else
                                {
                                    mpFS->startElementNS(XML_a, XML_quadBezTo);
                                    for( sal_uInt8 l = 0; l < 2; ++l )
                                    {
                                        WriteCustomGeometryPoint(aPairs[nPairIndex+l], rSdrObjCustomShape);
                                    }
                                    mpFS->endElementNS( XML_a, XML_quadBezTo );
                                    nPairIndex += 2;
                                }
                                break;
                            }
                            case drawing::EnhancedCustomShapeSegmentCommand::ARCANGLETO :
                            {
                                if (nPairIndex + 1 >= aPairs.getLength())
                                    bOK = false;
                                else
                                {
                                    const EnhancedCustomShape2d aCustoShape2d(
                                        const_cast<SdrObjCustomShape&>(rSdrObjCustomShape));
                                    double fWR = 0.0;
                                    aCustoShape2d.GetParameter(fWR, aPairs[nPairIndex].First, false,
                                                               false);
                                    double fHR = 0.0;
                                    aCustoShape2d.GetParameter(fHR, aPairs[nPairIndex].Second,
                                                               false, false);
                                    double fStartAngle = 0.0;
                                    aCustoShape2d.GetParameter(
                                        fStartAngle, aPairs[nPairIndex + 1].First, false, false);
                                    sal_Int32 nStartAng(std::lround(fStartAngle * 60000));
                                    double fSwingAng = 0.0;
                                    aCustoShape2d.GetParameter(
                                        fSwingAng, aPairs[nPairIndex + 1].Second, false, false);
                                    sal_Int32 nSwingAng(std::lround(fSwingAng * 60000));
                                    mpFS->singleElement(FSNS(XML_a, XML_arcTo),
                                                        XML_wR, OString::number(fWR),
                                                        XML_hR, OString::number(fHR),
                                                        XML_stAng, OString::number(nStartAng),
                                                        XML_swAng, OString::number(nSwingAng));
                                    nPairIndex += 2;
                                }
                                break;
                            }
                            default:
                                // do nothing
                                break;
                        }
                    }
                    if (!bOK)
                        break;
                }
                mpFS->endElementNS( XML_a, XML_path );
                mpFS->endElementNS( XML_a, XML_pathLst );
                mpFS->endElementNS( XML_a, XML_custGeom );
                return bOK;
            }
        }
    }
    return false;
}

void DrawingML::WriteCustomGeometryPoint(
    const drawing::EnhancedCustomShapeParameterPair& rParamPair,
    const SdrObjCustomShape& rSdrObjCustomShape)
{
    sal_Int32 nX = GetCustomGeometryPointValue(rParamPair.First, rSdrObjCustomShape);
    sal_Int32 nY = GetCustomGeometryPointValue(rParamPair.Second, rSdrObjCustomShape);

    mpFS->singleElementNS(XML_a, XML_pt, XML_x, OString::number(nX), XML_y, OString::number(nY));
}

sal_Int32 DrawingML::GetCustomGeometryPointValue(
    const css::drawing::EnhancedCustomShapeParameter& rParam,
    const SdrObjCustomShape& rSdrObjCustomShape)
{
    const EnhancedCustomShape2d aCustoShape2d(const_cast< SdrObjCustomShape& >(rSdrObjCustomShape));
    double fValue = 0.0;
    aCustoShape2d.GetParameter(fValue, rParam, false, false);
    sal_Int32 nValue(std::lround(fValue));

    return nValue;
}

void DrawingML::WritePolyPolygon(const css::uno::Reference<css::drawing::XShape>& rXShape,
                                 const tools::PolyPolygon& rPolyPolygon, const bool bClosed)
{
    // In case of Writer, the parent element is <wps:spPr>, and there the
    // <a:custGeom> element is not optional.
    if (rPolyPolygon.Count() < 1 && GetDocumentType() != DOCUMENT_DOCX)
        return;

    mpFS->startElementNS(XML_a, XML_custGeom);
    mpFS->singleElementNS(XML_a, XML_avLst);
    mpFS->singleElementNS(XML_a, XML_gdLst);
    mpFS->singleElementNS(XML_a, XML_ahLst);
    mpFS->singleElementNS(XML_a, XML_rect, XML_l, "0", XML_t, "0", XML_r, "r", XML_b, "b");

    mpFS->startElementNS( XML_a, XML_pathLst );

    const tools::Rectangle aRect( rPolyPolygon.GetBoundRect() );

    // tdf#101122
    std::optional<OString> sFill;
    if (HasEnhancedCustomShapeSegmentCommand(rXShape, css::drawing::EnhancedCustomShapeSegmentCommand::NOFILL))
        sFill = "none"; // for possible values see ST_PathFillMode in OOXML standard

    // Put all polygons of rPolyPolygon in the same path element
    // to subtract the overlapped areas.
    mpFS->startElementNS( XML_a, XML_path,
            XML_fill, sFill,
            XML_w, OString::number(aRect.GetWidth()),
            XML_h, OString::number(aRect.GetHeight()) );

    for( sal_uInt16 i = 0; i < rPolyPolygon.Count(); i ++ )
    {

        const tools::Polygon& rPoly = rPolyPolygon[ i ];

        if( rPoly.GetSize() > 0 )
        {
            mpFS->startElementNS(XML_a, XML_moveTo);

            mpFS->singleElementNS( XML_a, XML_pt,
                                   XML_x, OString::number(rPoly[0].X() - aRect.Left()),
                                   XML_y, OString::number(rPoly[0].Y() - aRect.Top()) );

            mpFS->endElementNS( XML_a, XML_moveTo );
        }

        for( sal_uInt16 j = 1; j < rPoly.GetSize(); j ++ )
        {
            PolyFlags flags = rPoly.GetFlags(j);
            if( flags == PolyFlags::Control )
            {
                // a:cubicBezTo can only contain 3 a:pt elements, so we need to make sure of this
                if( j+2 < rPoly.GetSize() && rPoly.GetFlags(j+1) == PolyFlags::Control && rPoly.GetFlags(j+2) != PolyFlags::Control )
                {

                    mpFS->startElementNS(XML_a, XML_cubicBezTo);
                    for( sal_uInt8 k = 0; k <= 2; ++k )
                    {
                        mpFS->singleElementNS(XML_a, XML_pt,
                                              XML_x, OString::number(rPoly[j+k].X() - aRect.Left()),
                                              XML_y, OString::number(rPoly[j+k].Y() - aRect.Top()));

                    }
                    mpFS->endElementNS( XML_a, XML_cubicBezTo );
                    j += 2;
                }
            }
            else if( flags == PolyFlags::Normal )
            {
                mpFS->startElementNS(XML_a, XML_lnTo);
                mpFS->singleElementNS( XML_a, XML_pt,
                                       XML_x, OString::number(rPoly[j].X() - aRect.Left()),
                                       XML_y, OString::number(rPoly[j].Y() - aRect.Top()) );
                mpFS->endElementNS( XML_a, XML_lnTo );
            }
        }
    }
    if (bClosed)
        mpFS->singleElementNS( XML_a, XML_close);
    mpFS->endElementNS( XML_a, XML_path );

    mpFS->endElementNS( XML_a, XML_pathLst );

    mpFS->endElementNS( XML_a, XML_custGeom );
}

void DrawingML::WriteConnectorConnections( EscherConnectorListEntry& rConnectorEntry, sal_Int32 nStartID, sal_Int32 nEndID )
{
    if( nStartID != -1 )
    {
        mpFS->singleElementNS( XML_a, XML_stCxn,
                               XML_id, OString::number(nStartID),
                               XML_idx, OString::number(rConnectorEntry.GetConnectorRule(true)) );
    }
    if( nEndID != -1 )
    {
        mpFS->singleElementNS( XML_a, XML_endCxn,
                               XML_id, OString::number(nEndID),
                               XML_idx, OString::number(rConnectorEntry.GetConnectorRule(false)) );
    }
}

sal_Unicode DrawingML::SubstituteBullet( sal_Unicode cBulletId, css::awt::FontDescriptor& rFontDesc )
{
    if ( IsStarSymbol(rFontDesc.Name) )
    {
        rtl_TextEncoding eCharSet = rFontDesc.CharSet;
        cBulletId = msfilter::util::bestFitOpenSymbolToMSFont(cBulletId, eCharSet, rFontDesc.Name);
        rFontDesc.CharSet = eCharSet;
    }

    return cBulletId;
}

sax_fastparser::FSHelperPtr DrawingML::CreateOutputStream (
    const OUString& sFullStream,
    std::u16string_view sRelativeStream,
    const Reference< XOutputStream >& xParentRelation,
    const char* sContentType,
    const char* sRelationshipType,
    OUString* pRelationshipId )
{
    OUString sRelationshipId;
    if (xParentRelation.is())
        sRelationshipId = GetFB()->addRelation( xParentRelation, OUString::createFromAscii( sRelationshipType), sRelativeStream );
    else
        sRelationshipId = GetFB()->addRelation( OUString::createFromAscii( sRelationshipType ), sRelativeStream );

    if( pRelationshipId )
        *pRelationshipId = sRelationshipId;

    sax_fastparser::FSHelperPtr p = GetFB()->openFragmentStreamWithSerializer( sFullStream, OUString::createFromAscii( sContentType ) );

    return p;
}

void DrawingML::WriteFill( const Reference< XPropertySet >& xPropSet )
{
    if ( !GetProperty( xPropSet, "FillStyle" ) )
        return;
    FillStyle aFillStyle( FillStyle_NONE );
    xPropSet->getPropertyValue( "FillStyle" ) >>= aFillStyle;

    // map full transparent background to no fill
    if ( aFillStyle == FillStyle_SOLID && GetProperty( xPropSet, "FillTransparence" ) )
    {
        sal_Int16 nVal = 0;
        xPropSet->getPropertyValue( "FillTransparence" ) >>= nVal;
        if ( nVal == 100 )
            aFillStyle = FillStyle_NONE;
    }
    if (aFillStyle == FillStyle_SOLID && GetProperty( xPropSet, "FillTransparenceGradient"))
    {
        awt::Gradient aTransparenceGradient;
        mAny >>= aTransparenceGradient;
        if (aTransparenceGradient.StartColor == 0xffffff && aTransparenceGradient.EndColor == 0xffffff)
            aFillStyle = FillStyle_NONE;
    }

    switch( aFillStyle )
    {
    case FillStyle_SOLID :
        WriteSolidFill( xPropSet );
        break;
    case FillStyle_GRADIENT :
        WriteGradientFill( xPropSet );
        break;
    case FillStyle_BITMAP :
        WriteBlipFill( xPropSet, "FillBitmap" );
        break;
    case FillStyle_HATCH :
        WritePattFill( xPropSet );
        break;
    case FillStyle_NONE:
        mpFS->singleElementNS(XML_a, XML_noFill);
        break;
    default:
        ;
    }
}

void DrawingML::WriteStyleProperties( sal_Int32 nTokenId, const Sequence< PropertyValue >& aProperties )
{
    if( aProperties.hasElements() )
    {
        OUString sSchemeClr;
        sal_uInt32 nIdx = 0;
        Sequence< PropertyValue > aTransformations;
        for( const auto& rProp : aProperties)
        {
            if( rProp.Name == "SchemeClr" )
                rProp.Value >>= sSchemeClr;
            else if( rProp.Name == "Idx" )
                rProp.Value >>= nIdx;
            else if( rProp.Name == "Transformations" )
                rProp.Value >>= aTransformations;
        }
        mpFS->startElementNS(XML_a, nTokenId, XML_idx, OString::number(nIdx));
        WriteColor(sSchemeClr, aTransformations);
        mpFS->endElementNS( XML_a, nTokenId );
    }
    else
    {
        // write mock <a:*Ref> tag
        mpFS->singleElementNS(XML_a, nTokenId, XML_idx, OString::number(0));
    }
}

void DrawingML::WriteShapeStyle( const Reference< XPropertySet >& xPropSet )
{
    // check existence of the grab bag
    if ( !GetProperty( xPropSet, "InteropGrabBag" ) )
        return;

    // extract the relevant properties from the grab bag
    Sequence< PropertyValue > aGrabBag;
    Sequence< PropertyValue > aFillRefProperties, aLnRefProperties, aEffectRefProperties;
    mAny >>= aGrabBag;
    for( const auto& rProp : std::as_const(aGrabBag))
    {
        if( rProp.Name == "StyleFillRef" )
            rProp.Value >>= aFillRefProperties;
        else if( rProp.Name == "StyleLnRef" )
            rProp.Value >>= aLnRefProperties;
        else if( rProp.Name == "StyleEffectRef" )
            rProp.Value >>= aEffectRefProperties;
    }

    WriteStyleProperties( XML_lnRef, aLnRefProperties );
    WriteStyleProperties( XML_fillRef, aFillRefProperties );
    WriteStyleProperties( XML_effectRef, aEffectRefProperties );

    // write mock <a:fontRef>
    mpFS->singleElementNS(XML_a, XML_fontRef, XML_idx, "minor");
}

void DrawingML::WriteShapeEffect( std::u16string_view sName, const Sequence< PropertyValue >& aEffectProps )
{
    if( !aEffectProps.hasElements() )
        return;

    // assign the proper tag and enable bContainsColor if necessary
    sal_Int32 nEffectToken = 0;
    bool bContainsColor = false;
    if( sName == u"outerShdw" )
    {
        nEffectToken = FSNS( XML_a, XML_outerShdw );
        bContainsColor = true;
    }
    else if( sName == u"innerShdw" )
    {
        nEffectToken = FSNS( XML_a, XML_innerShdw );
        bContainsColor = true;
    }
    else if( sName == u"glow" )
    {
        nEffectToken = FSNS( XML_a, XML_glow );
        bContainsColor = true;
    }
    else if( sName == u"softEdge" )
        nEffectToken = FSNS( XML_a, XML_softEdge );
    else if( sName == u"reflection" )
        nEffectToken = FSNS( XML_a, XML_reflection );
    else if( sName == u"blur" )
        nEffectToken = FSNS( XML_a, XML_blur );

    OUString sSchemeClr;
    ::Color nRgbClr;
    sal_Int32 nAlpha = MAX_PERCENT;
    Sequence< PropertyValue > aTransformations;
    rtl::Reference<sax_fastparser::FastAttributeList> aOuterShdwAttrList = FastSerializerHelper::createAttrList();
    for( const auto& rEffectProp : aEffectProps )
    {
        if( rEffectProp.Name == "Attribs" )
        {
            // read tag attributes
            uno::Sequence< beans::PropertyValue > aOuterShdwProps;
            rEffectProp.Value >>= aOuterShdwProps;
            for( const auto& rOuterShdwProp : std::as_const(aOuterShdwProps) )
            {
                if( rOuterShdwProp.Name == "algn" )
                {
                    OUString sVal;
                    rOuterShdwProp.Value >>= sVal;
                    aOuterShdwAttrList->add( XML_algn, OUStringToOString( sVal, RTL_TEXTENCODING_UTF8 ) );
                }
                else if( rOuterShdwProp.Name == "blurRad" )
                {
                    sal_Int64 nVal = 0;
                    rOuterShdwProp.Value >>= nVal;
                    aOuterShdwAttrList->add( XML_blurRad, OString::number( nVal ).getStr() );
                }
                else if( rOuterShdwProp.Name == "dir" )
                {
                    sal_Int32 nVal = 0;
                    rOuterShdwProp.Value >>= nVal;
                    aOuterShdwAttrList->add( XML_dir, OString::number( nVal ).getStr() );
                }
                else if( rOuterShdwProp.Name == "dist" )
                {
                    sal_Int32 nVal = 0;
                    rOuterShdwProp.Value >>= nVal;
                    aOuterShdwAttrList->add( XML_dist, OString::number( nVal ).getStr() );
                }
                else if( rOuterShdwProp.Name == "kx" )
                {
                    sal_Int32 nVal = 0;
                    rOuterShdwProp.Value >>= nVal;
                    aOuterShdwAttrList->add( XML_kx, OString::number( nVal ).getStr() );
                }
                else if( rOuterShdwProp.Name == "ky" )
                {
                    sal_Int32 nVal = 0;
                    rOuterShdwProp.Value >>= nVal;
                    aOuterShdwAttrList->add( XML_ky, OString::number( nVal ).getStr() );
                }
                else if( rOuterShdwProp.Name == "rotWithShape" )
                {
                    sal_Int32 nVal = 0;
                    rOuterShdwProp.Value >>= nVal;
                    aOuterShdwAttrList->add( XML_rotWithShape, OString::number( nVal ).getStr() );
                }
                else if( rOuterShdwProp.Name == "sx" )
                {
                    sal_Int32 nVal = 0;
                    rOuterShdwProp.Value >>= nVal;
                    aOuterShdwAttrList->add( XML_sx, OString::number( nVal ).getStr() );
                }
                else if( rOuterShdwProp.Name == "sy" )
                {
                    sal_Int32 nVal = 0;
                    rOuterShdwProp.Value >>= nVal;
                    aOuterShdwAttrList->add( XML_sy, OString::number( nVal ).getStr() );
                }
                else if( rOuterShdwProp.Name == "rad" )
                {
                    sal_Int64 nVal = 0;
                    rOuterShdwProp.Value >>= nVal;
                    aOuterShdwAttrList->add( XML_rad, OString::number( nVal ).getStr() );
                }
                else if( rOuterShdwProp.Name == "endA" )
                {
                    sal_Int32 nVal = 0;
                    rOuterShdwProp.Value >>= nVal;
                    aOuterShdwAttrList->add( XML_endA, OString::number( nVal ).getStr() );
                }
                else if( rOuterShdwProp.Name == "endPos" )
                {
                    sal_Int32 nVal = 0;
                    rOuterShdwProp.Value >>= nVal;
                    aOuterShdwAttrList->add( XML_endPos, OString::number( nVal ).getStr() );
                }
                else if( rOuterShdwProp.Name == "fadeDir" )
                {
                    sal_Int32 nVal = 0;
                    rOuterShdwProp.Value >>= nVal;
                    aOuterShdwAttrList->add( XML_fadeDir, OString::number( nVal ).getStr() );
                }
                else if( rOuterShdwProp.Name == "stA" )
                {
                    sal_Int32 nVal = 0;
                    rOuterShdwProp.Value >>= nVal;
                    aOuterShdwAttrList->add( XML_stA, OString::number( nVal ).getStr() );
                }
                else if( rOuterShdwProp.Name == "stPos" )
                {
                    sal_Int32 nVal = 0;
                    rOuterShdwProp.Value >>= nVal;
                    aOuterShdwAttrList->add( XML_stPos, OString::number( nVal ).getStr() );
                }
                else if( rOuterShdwProp.Name == "grow" )
                {
                    sal_Int32 nVal = 0;
                    rOuterShdwProp.Value >>= nVal;
                    aOuterShdwAttrList->add( XML_grow, OString::number( nVal ).getStr() );
                }
            }
        }
        else if(rEffectProp.Name == "RgbClr")
        {
            rEffectProp.Value >>= nRgbClr;
        }
        else if(rEffectProp.Name == "RgbClrTransparency")
        {
            sal_Int32 nTransparency;
            if (rEffectProp.Value >>= nTransparency)
                // Calculate alpha value (see oox/source/drawingml/color.cxx : getTransparency())
                nAlpha = MAX_PERCENT - ( PER_PERCENT * nTransparency );
        }
        else if(rEffectProp.Name == "SchemeClr")
        {
            rEffectProp.Value >>= sSchemeClr;
        }
        else if(rEffectProp.Name == "SchemeClrTransformations")
        {
            rEffectProp.Value >>= aTransformations;
        }
    }

    if( nEffectToken <= 0 )
        return;

    mpFS->startElement( nEffectToken, aOuterShdwAttrList );

    if( bContainsColor )
    {
        if( sSchemeClr.isEmpty() )
            WriteColor( nRgbClr, nAlpha );
        else
            WriteColor( sSchemeClr, aTransformations );
    }

    mpFS->endElement( nEffectToken );
}

static sal_Int32 lcl_CalculateDist(const double dX, const double dY)
{
    return static_cast< sal_Int32 >(sqrt(dX*dX + dY*dY) * 360);
}

static sal_Int32 lcl_CalculateDir(const double dX, const double dY)
{
    return (static_cast< sal_Int32 >(basegfx::rad2deg(atan2(dY,dX)) * 60000) + 21600000) % 21600000;
}

void DrawingML::WriteShapeEffects( const Reference< XPropertySet >& rXPropSet )
{
    Sequence< PropertyValue > aGrabBag, aEffects, aOuterShdwProps;
    bool bHasInteropGrabBag = rXPropSet->getPropertySetInfo()->hasPropertyByName("InteropGrabBag");
    if (bHasInteropGrabBag && GetProperty(rXPropSet, "InteropGrabBag"))
    {
        mAny >>= aGrabBag;
        auto pProp = std::find_if(std::cbegin(aGrabBag), std::cend(aGrabBag),
            [](const PropertyValue& rProp) { return rProp.Name == "EffectProperties"; });
        if (pProp != std::cend(aGrabBag))
        {
            pProp->Value >>= aEffects;
            auto pEffect = std::find_if(std::cbegin(aEffects), std::cend(aEffects),
                [](const PropertyValue& rEffect) { return rEffect.Name == "outerShdw"; });
            if (pEffect != std::cend(aEffects))
                pEffect->Value >>= aOuterShdwProps;
        }
    }

    // tdf#132201: the order of effects is important. Effects order (CT_EffectList in ECMA-376):
    // blur -> fillOverlay -> glow -> innerShdw -> outerShdw -> prstShdw -> reflection -> softEdge

    if( !aEffects.hasElements() )
    {
        bool bHasShadow = false;
        if( GetProperty( rXPropSet, "Shadow" ) )
            mAny >>= bHasShadow;
        bool bHasEffects = bHasShadow;
        if (!bHasEffects && GetProperty(rXPropSet, "GlowEffectRadius"))
        {
            sal_Int32 rad = 0;
            mAny >>= rad;
            bHasEffects = rad > 0;
        }
        if (!bHasEffects && GetProperty(rXPropSet, "SoftEdgeRadius"))
        {
            sal_Int32 rad = 0;
            mAny >>= rad;
            bHasEffects = rad > 0;
        }

        if (bHasEffects)
        {
            mpFS->startElementNS(XML_a, XML_effectLst);
            WriteGlowEffect(rXPropSet);
            if( bHasShadow )
            {
                Sequence< PropertyValue > aShadowGrabBag( 3 );
                Sequence< PropertyValue > aShadowAttribsGrabBag( 4 );

                double dX = +0.0, dY = +0.0;
                sal_Int32 nBlur =0;
                rXPropSet->getPropertyValue( "ShadowXDistance" ) >>= dX;
                rXPropSet->getPropertyValue( "ShadowYDistance" ) >>= dY;
                rXPropSet->getPropertyValue( "ShadowBlur" ) >>= nBlur;

                aShadowAttribsGrabBag[0].Name = "dist";
                aShadowAttribsGrabBag[0].Value <<= lcl_CalculateDist(dX, dY);
                aShadowAttribsGrabBag[1].Name = "dir";
                aShadowAttribsGrabBag[1].Value <<= lcl_CalculateDir(dX, dY);
                aShadowAttribsGrabBag[2].Name = "blurRad";
                aShadowAttribsGrabBag[2].Value <<=  oox::drawingml::convertHmmToEmu(nBlur);
                aShadowAttribsGrabBag[3].Name = "rotWithShape";
                aShadowAttribsGrabBag[3].Value <<= false; //ooxml default is 'true', so must write it

                aShadowGrabBag[0].Name = "Attribs";
                aShadowGrabBag[0].Value <<= aShadowAttribsGrabBag;
                aShadowGrabBag[1].Name = "RgbClr";
                aShadowGrabBag[1].Value = rXPropSet->getPropertyValue( "ShadowColor" );
                aShadowGrabBag[2].Name = "RgbClrTransparency";
                aShadowGrabBag[2].Value = rXPropSet->getPropertyValue( "ShadowTransparence" );

                WriteShapeEffect( u"outerShdw", aShadowGrabBag );
            }
            WriteSoftEdgeEffect(rXPropSet);
            mpFS->endElementNS(XML_a, XML_effectLst);
        }
    }
    else
    {
        for( auto& rOuterShdwProp : aOuterShdwProps )
        {
            if( rOuterShdwProp.Name == "Attribs" )
            {
                Sequence< PropertyValue > aAttribsProps;
                rOuterShdwProp.Value >>= aAttribsProps;

                double dX = +0.0, dY = +0.0;
                sal_Int32 nBlur =0;
                rXPropSet->getPropertyValue( "ShadowXDistance" ) >>= dX;
                rXPropSet->getPropertyValue( "ShadowYDistance" ) >>= dY;
                rXPropSet->getPropertyValue( "ShadowBlur" ) >>= nBlur;


                for( auto& rAttribsProp : aAttribsProps )
                {
                    if( rAttribsProp.Name == "dist" )
                    {
                        rAttribsProp.Value <<= lcl_CalculateDist(dX, dY);
                    }
                    else if( rAttribsProp.Name == "dir" )
                    {
                        rAttribsProp.Value <<= lcl_CalculateDir(dX, dY);
                    }
                    else if( rAttribsProp.Name == "blurRad" )
                    {
                        rAttribsProp.Value <<= oox::drawingml::convertHmmToEmu(nBlur);
                    }
                }

                rOuterShdwProp.Value <<= aAttribsProps;
            }
            else if( rOuterShdwProp.Name == "RgbClr" )
            {
                rOuterShdwProp.Value = rXPropSet->getPropertyValue( "ShadowColor" );
            }
            else if( rOuterShdwProp.Name == "RgbClrTransparency" )
            {
                rOuterShdwProp.Value = rXPropSet->getPropertyValue( "ShadowTransparence" );
            }
        }

        mpFS->startElementNS(XML_a, XML_effectLst);
        bool bGlowWritten = false;
        for( const auto& rEffect : std::as_const(aEffects) )
        {
            if (!bGlowWritten
                && (rEffect.Name == "innerShdw" || rEffect.Name == "outerShdw"
                    || rEffect.Name == "prstShdw" || rEffect.Name == "reflection"
                    || rEffect.Name == "softEdge"))
            {
                WriteGlowEffect(rXPropSet);
                bGlowWritten = true;
            }

            if( rEffect.Name == "outerShdw" )
            {
                WriteShapeEffect( rEffect.Name, aOuterShdwProps );
            }
            else
            {
                Sequence< PropertyValue > aEffectProps;
                rEffect.Value >>= aEffectProps;
                WriteShapeEffect( rEffect.Name, aEffectProps );
            }
        }
        if (!bGlowWritten)
            WriteGlowEffect(rXPropSet);
        WriteSoftEdgeEffect(rXPropSet); // the last

        mpFS->endElementNS(XML_a, XML_effectLst);
    }
}

void DrawingML::WriteGlowEffect(const Reference< XPropertySet >& rXPropSet)
{
    if (!rXPropSet->getPropertySetInfo()->hasPropertyByName("GlowEffectRadius"))
    {
        return;
    }

    sal_Int32 nRad = 0;
    rXPropSet->getPropertyValue("GlowEffectRadius") >>= nRad;
    if (!nRad)
        return;

    Sequence< PropertyValue > aGlowAttribs(1);
    aGlowAttribs[0].Name = "rad";
    aGlowAttribs[0].Value <<= oox::drawingml::convertHmmToEmu(nRad);
    Sequence< PropertyValue > aGlowProps(3);
    aGlowProps[0].Name = "Attribs";
    aGlowProps[0].Value <<= aGlowAttribs;
    aGlowProps[1].Name = "RgbClr";
    aGlowProps[1].Value = rXPropSet->getPropertyValue("GlowEffectColor");
    aGlowProps[2].Name = "RgbClrTransparency";
    aGlowProps[2].Value = rXPropSet->getPropertyValue("GlowEffectTransparency");
    // TODO other stuff like saturation or luminance

    WriteShapeEffect(u"glow", aGlowProps);
}

void DrawingML::WriteSoftEdgeEffect(const css::uno::Reference<css::beans::XPropertySet>& rXPropSet)
{
    if (!rXPropSet->getPropertySetInfo()->hasPropertyByName("SoftEdgeRadius"))
    {
        return;
    }

    sal_Int32 nRad = 0;
    rXPropSet->getPropertyValue("SoftEdgeRadius") >>= nRad;
    if (!nRad)
        return;

    css::uno::Sequence<css::beans::PropertyValue> aAttribs(1);
    aAttribs[0].Name = "rad";
    aAttribs[0].Value <<= oox::drawingml::convertHmmToEmu(nRad);
    css::uno::Sequence<css::beans::PropertyValue> aProps(1);
    aProps[0].Name = "Attribs";
    aProps[0].Value <<= aAttribs;

    WriteShapeEffect(u"softEdge", aProps);
}

bool DrawingML::HasEnhancedCustomShapeSegmentCommand(
    const css::uno::Reference<css::drawing::XShape>& rXShape, const sal_Int16 nCommand)
{
    try
    {
        uno::Reference<beans::XPropertySet> xPropSet(rXShape, uno::UNO_QUERY_THROW);
        if (!GetProperty(xPropSet, "CustomShapeGeometry"))
            return false;
        Sequence<PropertyValue> aCustomShapeGeometryProps;
        mAny >>= aCustomShapeGeometryProps;
        for (const beans::PropertyValue& rGeomProp : std::as_const(aCustomShapeGeometryProps))
        {
            if (rGeomProp.Name == "Path")
            {
                uno::Sequence<beans::PropertyValue> aPathProps;
                rGeomProp.Value >>= aPathProps;
                for (const beans::PropertyValue& rPathProp : std::as_const(aPathProps))
                {
                    if (rPathProp.Name == "Segments")
                    {
                        uno::Sequence<drawing::EnhancedCustomShapeSegment> aSegments;
                        rPathProp.Value >>= aSegments;
                        for (const auto& rSegment : std::as_const(aSegments))
                        {
                            if (rSegment.Command == nCommand)
                                return true;
                        }
                    }
                }
            }
        }
    }
    catch (const ::uno::Exception&)
    {
    }
    return false;
}

void DrawingML::WriteShape3DEffects( const Reference< XPropertySet >& xPropSet )
{
    // check existence of the grab bag
    if( !GetProperty( xPropSet, "InteropGrabBag" ) )
        return;

    // extract the relevant properties from the grab bag
    Sequence< PropertyValue > aGrabBag, aEffectProps, aLightRigProps, aShape3DProps;
    mAny >>= aGrabBag;
    auto pShapeProp = std::find_if(std::cbegin(aGrabBag), std::cend(aGrabBag),
        [](const PropertyValue& rProp) { return rProp.Name == "3DEffectProperties"; });
    if (pShapeProp != std::cend(aGrabBag))
    {
        Sequence< PropertyValue > a3DEffectProps;
        pShapeProp->Value >>= a3DEffectProps;
        for( const auto& r3DEffectProp : std::as_const(a3DEffectProps) )
        {
            if( r3DEffectProp.Name == "Camera" )
                r3DEffectProp.Value >>= aEffectProps;
            else if( r3DEffectProp.Name == "LightRig" )
                r3DEffectProp.Value >>= aLightRigProps;
            else if( r3DEffectProp.Name == "Shape3D" )
                r3DEffectProp.Value >>= aShape3DProps;
        }
    }

    auto pTextProp = std::find_if(std::cbegin(aGrabBag), std::cend(aGrabBag),
        [](const PropertyValue& rProp) { return rProp.Name == "Text3DEffectProperties"; });

    if (pTextProp != std::cend(aGrabBag))
    {
        Sequence< PropertyValue > a3DEffectProps;
        pTextProp->Value >>= a3DEffectProps;
        for( const auto& r3DEffectProp : std::as_const(a3DEffectProps) )
        {
            if( r3DEffectProp.Name == "Camera" )
                r3DEffectProp.Value >>= aEffectProps;
            else if( r3DEffectProp.Name == "LightRig" )
                r3DEffectProp.Value >>= aLightRigProps;
            else if( r3DEffectProp.Name == "Shape3D" )
                r3DEffectProp.Value >>= aShape3DProps;
        }
    }

    if( !aEffectProps.hasElements() && !aLightRigProps.hasElements() && !aShape3DProps.hasElements() )
        return;

    bool bCameraRotationPresent = false;
    rtl::Reference<sax_fastparser::FastAttributeList> aCameraAttrList = FastSerializerHelper::createAttrList();
    rtl::Reference<sax_fastparser::FastAttributeList> aCameraRotationAttrList = FastSerializerHelper::createAttrList();
    for( const auto& rEffectProp : std::as_const(aEffectProps) )
    {
        if( rEffectProp.Name == "prst" )
        {
            OUString sVal;
            rEffectProp.Value >>= sVal;
            aCameraAttrList->add(XML_prst, OUStringToOString(sVal, RTL_TEXTENCODING_UTF8));
        }
        else if( rEffectProp.Name == "fov" )
        {
            float fVal = 0;
            rEffectProp.Value >>= fVal;
            aCameraAttrList->add( XML_fov, OString::number( fVal * 60000 ).getStr() );
        }
        else if( rEffectProp.Name == "zoom" )
        {
            float fVal = 1;
            rEffectProp.Value >>= fVal;
            aCameraAttrList->add( XML_zoom, OString::number( fVal * 100000 ).getStr() );
        }
        else if( rEffectProp.Name == "rotLat" ||
                rEffectProp.Name == "rotLon" ||
                rEffectProp.Name == "rotRev" )
        {
            sal_Int32 nVal = 0, nToken = XML_none;
            rEffectProp.Value >>= nVal;
            if( rEffectProp.Name == "rotLat" )
                nToken = XML_lat;
            else if( rEffectProp.Name == "rotLon" )
                nToken = XML_lon;
            else if( rEffectProp.Name == "rotRev" )
                nToken = XML_rev;
            aCameraRotationAttrList->add( nToken, OString::number( nVal ).getStr() );
            bCameraRotationPresent = true;
        }
    }

    bool bLightRigRotationPresent = false;
    rtl::Reference<sax_fastparser::FastAttributeList> aLightRigAttrList = FastSerializerHelper::createAttrList();
    rtl::Reference<sax_fastparser::FastAttributeList> aLightRigRotationAttrList = FastSerializerHelper::createAttrList();
    for( const auto& rLightRigProp : std::as_const(aLightRigProps) )
    {
        if( rLightRigProp.Name == "rig" || rLightRigProp.Name == "dir" )
        {
            OUString sVal;
            sal_Int32 nToken = XML_none;
            rLightRigProp.Value >>= sVal;
            if( rLightRigProp.Name == "rig" )
                nToken = XML_rig;
            else if( rLightRigProp.Name == "dir" )
                nToken = XML_dir;
            aLightRigAttrList->add(nToken, OUStringToOString(sVal, RTL_TEXTENCODING_UTF8));
        }
        else if( rLightRigProp.Name == "rotLat" ||
                rLightRigProp.Name == "rotLon" ||
                rLightRigProp.Name == "rotRev" )
        {
            sal_Int32 nVal = 0, nToken = XML_none;
            rLightRigProp.Value >>= nVal;
            if( rLightRigProp.Name == "rotLat" )
                nToken = XML_lat;
            else if( rLightRigProp.Name == "rotLon" )
                nToken = XML_lon;
            else if( rLightRigProp.Name == "rotRev" )
                nToken = XML_rev;
            aLightRigRotationAttrList->add( nToken, OString::number( nVal ).getStr() );
            bLightRigRotationPresent = true;
        }
    }

    mpFS->startElementNS(XML_a, XML_scene3d);

    if( aEffectProps.hasElements() )
    {
        mpFS->startElementNS( XML_a, XML_camera, aCameraAttrList );
        if( bCameraRotationPresent )
        {
            mpFS->singleElementNS( XML_a, XML_rot, aCameraRotationAttrList );
        }
        mpFS->endElementNS( XML_a, XML_camera );
    }
    else
    {
        // a:camera with Word default values - Word won't open the document if this is not present
        mpFS->singleElementNS(XML_a, XML_camera, XML_prst, "orthographicFront");
    }

    if( aEffectProps.hasElements() )
    {
        mpFS->startElementNS( XML_a, XML_lightRig, aLightRigAttrList );
        if( bLightRigRotationPresent )
        {
            mpFS->singleElementNS( XML_a, XML_rot, aLightRigRotationAttrList );
        }
        mpFS->endElementNS( XML_a, XML_lightRig );
    }
    else
    {
        // a:lightRig with Word default values - Word won't open the document if this is not present
        mpFS->singleElementNS(XML_a, XML_lightRig, XML_rig, "threePt", XML_dir, "t");
    }

    mpFS->endElementNS( XML_a, XML_scene3d );

    if( !aShape3DProps.hasElements() )
        return;

    bool bBevelTPresent = false, bBevelBPresent = false;
    Sequence< PropertyValue > aExtrusionColorProps, aContourColorProps;
    rtl::Reference<sax_fastparser::FastAttributeList> aBevelTAttrList = FastSerializerHelper::createAttrList();
    rtl::Reference<sax_fastparser::FastAttributeList> aBevelBAttrList = FastSerializerHelper::createAttrList();
    rtl::Reference<sax_fastparser::FastAttributeList> aShape3DAttrList = FastSerializerHelper::createAttrList();
    for( const auto& rShape3DProp : std::as_const(aShape3DProps) )
    {
        if( rShape3DProp.Name == "extrusionH" || rShape3DProp.Name == "contourW" || rShape3DProp.Name == "z" )
        {
            sal_Int32 nVal = 0, nToken = XML_none;
            rShape3DProp.Value >>= nVal;
            if( rShape3DProp.Name == "extrusionH" )
                nToken = XML_extrusionH;
            else if( rShape3DProp.Name == "contourW" )
                nToken = XML_contourW;
            else if( rShape3DProp.Name == "z" )
                nToken = XML_z;
            aShape3DAttrList->add( nToken, OString::number( nVal ).getStr() );
        }
        else if( rShape3DProp.Name == "prstMaterial" )
        {
            OUString sVal;
            rShape3DProp.Value >>= sVal;
            aShape3DAttrList->add(XML_prstMaterial, OUStringToOString(sVal, RTL_TEXTENCODING_UTF8));
        }
        else if( rShape3DProp.Name == "extrusionClr" )
        {
            rShape3DProp.Value >>= aExtrusionColorProps;
        }
        else if( rShape3DProp.Name == "contourClr" )
        {
            rShape3DProp.Value >>= aContourColorProps;
        }
        else if( rShape3DProp.Name == "bevelT" || rShape3DProp.Name == "bevelB" )
        {
            Sequence< PropertyValue > aBevelProps;
            rShape3DProp.Value >>= aBevelProps;
            if ( !aBevelProps.hasElements() )
                continue;

            rtl::Reference<sax_fastparser::FastAttributeList> aBevelAttrList;
            if( rShape3DProp.Name == "bevelT" )
            {
                bBevelTPresent = true;
                aBevelAttrList = aBevelTAttrList;
            }
            else
            {
                bBevelBPresent = true;
                aBevelAttrList = aBevelBAttrList;
            }
            for( const auto& rBevelProp : std::as_const(aBevelProps) )
            {
                if( rBevelProp.Name == "w" || rBevelProp.Name == "h" )
                {
                    sal_Int32 nVal = 0, nToken = XML_none;
                    rBevelProp.Value >>= nVal;
                    if( rBevelProp.Name == "w" )
                        nToken = XML_w;
                    else if( rBevelProp.Name == "h" )
                        nToken = XML_h;
                    aBevelAttrList->add( nToken, OString::number( nVal ).getStr() );
                }
                else  if( rBevelProp.Name == "prst" )
                {
                    OUString sVal;
                    rBevelProp.Value >>= sVal;
                    aBevelAttrList->add(XML_prst, OUStringToOString(sVal, RTL_TEXTENCODING_UTF8));
                }
            }

        }
    }

    mpFS->startElementNS( XML_a, XML_sp3d, aShape3DAttrList );
    if( bBevelTPresent )
    {
        mpFS->singleElementNS( XML_a, XML_bevelT, aBevelTAttrList );
    }
    if( bBevelBPresent )
    {
        mpFS->singleElementNS( XML_a, XML_bevelB, aBevelBAttrList );
    }
    if( aExtrusionColorProps.hasElements() )
    {
        OUString sSchemeClr;
        ::Color nColor;
        sal_Int32 nTransparency(0);
        Sequence< PropertyValue > aColorTransformations;
        for( const auto& rExtrusionColorProp : std::as_const(aExtrusionColorProps) )
        {
            if( rExtrusionColorProp.Name == "schemeClr" )
                rExtrusionColorProp.Value >>= sSchemeClr;
            else if( rExtrusionColorProp.Name == "schemeClrTransformations" )
                rExtrusionColorProp.Value >>= aColorTransformations;
            else if( rExtrusionColorProp.Name == "rgbClr" )
                rExtrusionColorProp.Value >>= nColor;
            else if( rExtrusionColorProp.Name == "rgbClrTransparency" )
                rExtrusionColorProp.Value >>= nTransparency;
        }
        mpFS->startElementNS(XML_a, XML_extrusionClr);

        if( sSchemeClr.isEmpty() )
            WriteColor( nColor, MAX_PERCENT - ( PER_PERCENT * nTransparency ) );
        else
            WriteColor( sSchemeClr, aColorTransformations );

        mpFS->endElementNS( XML_a, XML_extrusionClr );
    }
    if( aContourColorProps.hasElements() )
    {
        OUString sSchemeClr;
        ::Color nColor;
        sal_Int32 nTransparency(0);
        Sequence< PropertyValue > aColorTransformations;
        for( const auto& rContourColorProp : std::as_const(aContourColorProps) )
        {
            if( rContourColorProp.Name == "schemeClr" )
                rContourColorProp.Value >>= sSchemeClr;
            else if( rContourColorProp.Name == "schemeClrTransformations" )
                rContourColorProp.Value >>= aColorTransformations;
            else if( rContourColorProp.Name == "rgbClr" )
                rContourColorProp.Value >>= nColor;
            else if( rContourColorProp.Name == "rgbClrTransparency" )
                rContourColorProp.Value >>= nTransparency;
        }
        mpFS->startElementNS(XML_a, XML_contourClr);

        if( sSchemeClr.isEmpty() )
            WriteColor( nColor, MAX_PERCENT - ( PER_PERCENT * nTransparency ) );
        else
            WriteColor( sSchemeClr, aContourColorProps );

        mpFS->endElementNS( XML_a, XML_contourClr );
    }
    mpFS->endElementNS( XML_a, XML_sp3d );
}

void DrawingML::WriteArtisticEffect( const Reference< XPropertySet >& rXPropSet )
{
    if( !GetProperty( rXPropSet, "InteropGrabBag" ) )
        return;

    PropertyValue aEffect;
    Sequence< PropertyValue > aGrabBag;
    mAny >>= aGrabBag;
    auto pProp = std::find_if(std::cbegin(aGrabBag), std::cend(aGrabBag),
        [](const PropertyValue& rProp) { return rProp.Name == "ArtisticEffectProperties"; });
    if (pProp != std::cend(aGrabBag))
        pProp->Value >>= aEffect;
    sal_Int32 nEffectToken = ArtisticEffectProperties::getEffectToken( aEffect.Name );
    if( nEffectToken == XML_none )
        return;

    Sequence< PropertyValue > aAttrs;
    aEffect.Value >>= aAttrs;
    rtl::Reference<sax_fastparser::FastAttributeList> aAttrList = FastSerializerHelper::createAttrList();
    OString sRelId;
    for( const auto& rAttr : std::as_const(aAttrs) )
    {
        sal_Int32 nToken = ArtisticEffectProperties::getEffectToken( rAttr.Name );
        if( nToken != XML_none )
        {
            sal_Int32 nVal = 0;
            rAttr.Value >>= nVal;
            aAttrList->add( nToken, OString::number( nVal ).getStr() );
        }
        else if( rAttr.Name == "OriginalGraphic" )
        {
            Sequence< PropertyValue > aGraphic;
            rAttr.Value >>= aGraphic;
            Sequence< sal_Int8 > aGraphicData;
            OUString sGraphicId;
            for( const auto& rProp : std::as_const(aGraphic) )
            {
                if( rProp.Name == "Id" )
                    rProp.Value >>= sGraphicId;
                else if( rProp.Name == "Data" )
                    rProp.Value >>= aGraphicData;
            }
            sRelId = WriteWdpPicture( sGraphicId, aGraphicData );
        }
    }

    mpFS->startElementNS(XML_a, XML_extLst);
    mpFS->startElementNS(XML_a, XML_ext, XML_uri, "{BEBA8EAE-BF5A-486C-A8C5-ECC9F3942E4B}");
    mpFS->startElementNS( XML_a14, XML_imgProps,
                          FSNS(XML_xmlns, XML_a14), mpFB->getNamespaceURL(OOX_NS(a14)) );
    mpFS->startElementNS(XML_a14, XML_imgLayer, FSNS(XML_r, XML_embed), sRelId);
    mpFS->startElementNS(XML_a14, XML_imgEffect);

    mpFS->singleElementNS( XML_a14, nEffectToken, aAttrList );

    mpFS->endElementNS( XML_a14, XML_imgEffect );
    mpFS->endElementNS( XML_a14, XML_imgLayer );
    mpFS->endElementNS( XML_a14, XML_imgProps );
    mpFS->endElementNS( XML_a, XML_ext );
    mpFS->endElementNS( XML_a, XML_extLst );
}

OString DrawingML::WriteWdpPicture( const OUString& rFileId, const Sequence< sal_Int8 >& rPictureData )
{
    std::map<OUString, OUString>::iterator aCachedItem = maWdpCache.find( rFileId );
    if( aCachedItem != maWdpCache.end() )
        return OUStringToOString( aCachedItem->second, RTL_TEXTENCODING_UTF8 );

    OUString sFileName = "media/hdphoto" + OUString::number( mnWdpImageCounter++ ) + ".wdp";
    Reference< XOutputStream > xOutStream = mpFB->openFragmentStream( OUStringBuffer()
                                                                      .appendAscii( GetComponentDir() )
                                                                      .append( "/" + sFileName )
                                                                      .makeStringAndClear(),
                                                                      "image/vnd.ms-photo" );
    OUString sId;
    xOutStream->writeBytes( rPictureData );
    xOutStream->closeOutput();

    sId = mpFB->addRelation( mpFS->getOutputStream(),
                             oox::getRelationship(Relationship::HDPHOTO),
                             OUStringBuffer()
                             .appendAscii( GetRelationCompPrefix() )
                             .append( sFileName )
                             .makeStringAndClear() );

    maWdpCache[rFileId] = sId;
    return OUStringToOString( sId, RTL_TEXTENCODING_UTF8 );
}

void DrawingML::WriteDiagram(const css::uno::Reference<css::drawing::XShape>& rXShape, int nDiagramId)
{
    uno::Reference<beans::XPropertySet> xPropSet(rXShape, uno::UNO_QUERY);

    uno::Reference<xml::dom::XDocument> dataDom;
    uno::Reference<xml::dom::XDocument> layoutDom;
    uno::Reference<xml::dom::XDocument> styleDom;
    uno::Reference<xml::dom::XDocument> colorDom;
    uno::Reference<xml::dom::XDocument> drawingDom;
    uno::Sequence<uno::Sequence<uno::Any>> xDataRelSeq;
    uno::Sequence<uno::Any> diagramDrawing;

    // retrieve the doms from the GrabBag
    uno::Sequence<beans::PropertyValue> propList;
    xPropSet->getPropertyValue(UNO_NAME_MISC_OBJ_INTEROPGRABBAG) >>= propList;
    for (const auto& rProp : std::as_const(propList))
    {
        OUString propName = rProp.Name;
        if (propName == "OOXData")
            rProp.Value >>= dataDom;
        else if (propName == "OOXLayout")
            rProp.Value >>= layoutDom;
        else if (propName == "OOXStyle")
            rProp.Value >>= styleDom;
        else if (propName == "OOXColor")
            rProp.Value >>= colorDom;
        else if (propName == "OOXDrawing")
        {
            rProp.Value >>= diagramDrawing;
            diagramDrawing[0]
                >>= drawingDom; // if there is OOXDrawing property then set drawingDom here only.
        }
        else if (propName == "OOXDiagramDataRels")
            rProp.Value >>= xDataRelSeq;
    }

    // check that we have the 4 mandatory XDocuments
    // if not, there was an error importing and we won't output anything
    if (!dataDom.is() || !layoutDom.is() || !styleDom.is() || !colorDom.is())
        return;

    // generate a unique id
    rtl::Reference<sax_fastparser::FastAttributeList> pDocPrAttrList
        = sax_fastparser::FastSerializerHelper::createAttrList();
    pDocPrAttrList->add(XML_id, OString::number(nDiagramId).getStr());
    OUString sName = "Diagram" + OUString::number(nDiagramId);
    pDocPrAttrList->add(XML_name, OUStringToOString(sName, RTL_TEXTENCODING_UTF8));

    if (GetDocumentType() == DOCUMENT_DOCX)
    {
        mpFS->singleElementNS(XML_wp, XML_docPr, pDocPrAttrList);
        mpFS->singleElementNS(XML_wp, XML_cNvGraphicFramePr);

        mpFS->startElementNS(XML_a, XML_graphic, FSNS(XML_xmlns, XML_a),
                             mpFB->getNamespaceURL(OOX_NS(dml)));
    }
    else
    {
        mpFS->startElementNS(XML_p, XML_nvGraphicFramePr);

        mpFS->singleElementNS(XML_p, XML_cNvPr, pDocPrAttrList);
        mpFS->singleElementNS(XML_p, XML_cNvGraphicFramePr);

        mpFS->startElementNS(XML_p, XML_nvPr);
        mpFS->startElementNS(XML_p, XML_extLst);
        // change tracking extension - required in PPTX
        mpFS->startElementNS(XML_p, XML_ext, XML_uri, "{D42A27DB-BD31-4B8C-83A1-F6EECF244321}");
        mpFS->singleElementNS(XML_p14, XML_modId,
            FSNS(XML_xmlns, XML_p14), mpFB->getNamespaceURL(OOX_NS(p14)),
            XML_val,
            OString::number(comphelper::rng::uniform_uint_distribution(1, SAL_MAX_UINT32)));
        mpFS->endElementNS(XML_p, XML_ext);
        mpFS->endElementNS(XML_p, XML_extLst);
        mpFS->endElementNS(XML_p, XML_nvPr);

        mpFS->endElementNS(XML_p, XML_nvGraphicFramePr);

        // store size and position of background shape instead of group shape
        // as some shapes may be outside
        css::uno::Reference<css::drawing::XShapes> xShapes(rXShape, uno::UNO_QUERY);
        if (xShapes.is() && xShapes->hasElements())
        {
            css::uno::Reference<css::drawing::XShape> xShapeBg(xShapes->getByIndex(0),
                                                               uno::UNO_QUERY);
            awt::Point aPos = xShapeBg->getPosition();
            awt::Size aSize = xShapeBg->getSize();
            WriteTransformation(
                xShapeBg, tools::Rectangle(Point(aPos.X, aPos.Y), Size(aSize.Width, aSize.Height)),
                XML_p, false, false, 0, false);
        }

        mpFS->startElementNS(XML_a, XML_graphic);
    }

    mpFS->startElementNS(XML_a, XML_graphicData, XML_uri,
                         "http://schemas.openxmlformats.org/drawingml/2006/diagram");

    OUString sRelationCompPrefix = OUString::createFromAscii(GetRelationCompPrefix());

    // add data relation
    OUString dataFileName = "diagrams/data" + OUString::number(nDiagramId) + ".xml";
    OString dataRelId = OUStringToOString(
        mpFB->addRelation(mpFS->getOutputStream(), oox::getRelationship(Relationship::DIAGRAMDATA),
                          OUString(sRelationCompPrefix + dataFileName)),
        RTL_TEXTENCODING_UTF8);

    // add layout relation
    OUString layoutFileName = "diagrams/layout" + OUString::number(nDiagramId) + ".xml";
    OString layoutRelId
        = OUStringToOString(mpFB->addRelation(mpFS->getOutputStream(),
                                              oox::getRelationship(Relationship::DIAGRAMLAYOUT),
                                              OUString(sRelationCompPrefix + layoutFileName)),
                            RTL_TEXTENCODING_UTF8);

    // add style relation
    OUString styleFileName = "diagrams/quickStyle" + OUString::number(nDiagramId) + ".xml";
    OString styleRelId
        = OUStringToOString(mpFB->addRelation(mpFS->getOutputStream(),
                                              oox::getRelationship(Relationship::DIAGRAMQUICKSTYLE),
                                              OUString(sRelationCompPrefix + styleFileName)),
                            RTL_TEXTENCODING_UTF8);

    // add color relation
    OUString colorFileName = "diagrams/colors" + OUString::number(nDiagramId) + ".xml";
    OString colorRelId
        = OUStringToOString(mpFB->addRelation(mpFS->getOutputStream(),
                                              oox::getRelationship(Relationship::DIAGRAMCOLORS),
                                              OUString(sRelationCompPrefix + colorFileName)),
                            RTL_TEXTENCODING_UTF8);

    OUString drawingFileName;
    if (drawingDom.is())
    {
        // add drawing relation
        drawingFileName = "diagrams/drawing" + OUString::number(nDiagramId) + ".xml";
        OUString drawingRelId = mpFB->addRelation(
            mpFS->getOutputStream(), oox::getRelationship(Relationship::DIAGRAMDRAWING),
            OUString(sRelationCompPrefix + drawingFileName));

        // the data dom contains a reference to the drawing relation. We need to update it with the new generated
        // relation value before writing the dom to a file

        // Get the dsp:damaModelExt node from the dom
        uno::Reference<xml::dom::XNodeList> nodeList = dataDom->getElementsByTagNameNS(
            "http://schemas.microsoft.com/office/drawing/2008/diagram", "dataModelExt");

        // There must be one element only so get it
        uno::Reference<xml::dom::XNode> node = nodeList->item(0);

        // Get the list of attributes of the node
        uno::Reference<xml::dom::XNamedNodeMap> nodeMap = node->getAttributes();

        // Get the node with the relId attribute and set its new value
        uno::Reference<xml::dom::XNode> relIdNode = nodeMap->getNamedItem("relId");
        relIdNode->setNodeValue(drawingRelId);
    }

    mpFS->singleElementNS(XML_dgm, XML_relIds,
        FSNS(XML_xmlns, XML_dgm), mpFB->getNamespaceURL(OOX_NS(dmlDiagram)),
        FSNS(XML_xmlns, XML_r), mpFB->getNamespaceURL(OOX_NS(officeRel)),
        FSNS(XML_r, XML_dm), dataRelId, FSNS(XML_r, XML_lo), layoutRelId,
        FSNS(XML_r, XML_qs), styleRelId, FSNS(XML_r, XML_cs), colorRelId);

    mpFS->endElementNS(XML_a, XML_graphicData);
    mpFS->endElementNS(XML_a, XML_graphic);

    uno::Reference<xml::sax::XSAXSerializable> serializer;
    uno::Reference<xml::sax::XWriter> writer
        = xml::sax::Writer::create(comphelper::getProcessComponentContext());

    OUString sDir = OUString::createFromAscii(GetComponentDir());

    // write data file
    serializer.set(dataDom, uno::UNO_QUERY);
    uno::Reference<io::XOutputStream> xDataOutputStream = mpFB->openFragmentStream(
        sDir + "/" + dataFileName,
        "application/vnd.openxmlformats-officedocument.drawingml.diagramData+xml");
    writer->setOutputStream(xDataOutputStream);
    serializer->serialize(uno::Reference<xml::sax::XDocumentHandler>(writer, uno::UNO_QUERY_THROW),
                          uno::Sequence<beans::StringPair>());

    // write the associated Images and rels for data file
    writeDiagramRels(xDataRelSeq, xDataOutputStream, u"OOXDiagramDataRels", nDiagramId);

    // write layout file
    serializer.set(layoutDom, uno::UNO_QUERY);
    writer->setOutputStream(mpFB->openFragmentStream(
        sDir + "/" + layoutFileName,
        "application/vnd.openxmlformats-officedocument.drawingml.diagramLayout+xml"));
    serializer->serialize(uno::Reference<xml::sax::XDocumentHandler>(writer, uno::UNO_QUERY_THROW),
                          uno::Sequence<beans::StringPair>());

    // write style file
    serializer.set(styleDom, uno::UNO_QUERY);
    writer->setOutputStream(mpFB->openFragmentStream(
        sDir + "/" + styleFileName,
        "application/vnd.openxmlformats-officedocument.drawingml.diagramStyle+xml"));
    serializer->serialize(uno::Reference<xml::sax::XDocumentHandler>(writer, uno::UNO_QUERY_THROW),
                          uno::Sequence<beans::StringPair>());

    // write color file
    serializer.set(colorDom, uno::UNO_QUERY);
    writer->setOutputStream(mpFB->openFragmentStream(
        sDir + "/" + colorFileName,
        "application/vnd.openxmlformats-officedocument.drawingml.diagramColors+xml"));
    serializer->serialize(uno::Reference<xml::sax::XDocumentHandler>(writer, uno::UNO_QUERY_THROW),
                          uno::Sequence<beans::StringPair>());

    // write drawing file
    if (!drawingDom.is())
        return;

    serializer.set(drawingDom, uno::UNO_QUERY);
    uno::Reference<io::XOutputStream> xDrawingOutputStream = mpFB->openFragmentStream(
        sDir + "/" + drawingFileName, "application/vnd.ms-office.drawingml.diagramDrawing+xml");
    writer->setOutputStream(xDrawingOutputStream);
    serializer->serialize(
        uno::Reference<xml::sax::XDocumentHandler>(writer, uno::UNO_QUERY_THROW),
        uno::Sequence<beans::StringPair>());

    // write the associated Images and rels for drawing file
    uno::Sequence<uno::Sequence<uno::Any>> xDrawingRelSeq;
    diagramDrawing[1] >>= xDrawingRelSeq;
    writeDiagramRels(xDrawingRelSeq, xDrawingOutputStream, u"OOXDiagramDrawingRels", nDiagramId);
}

void DrawingML::writeDiagramRels(const uno::Sequence<uno::Sequence<uno::Any>>& xRelSeq,
                                 const uno::Reference<io::XOutputStream>& xOutStream,
                                 std::u16string_view sGrabBagProperyName, int nDiagramId)
{
    // add image relationships of OOXData, OOXDiagram
    OUString sType(oox::getRelationship(Relationship::IMAGE));
    uno::Reference<xml::sax::XWriter> xWriter
        = xml::sax::Writer::create(comphelper::getProcessComponentContext());
    xWriter->setOutputStream(xOutStream);

    // retrieve the relationships from Sequence
    for (sal_Int32 j = 0; j < xRelSeq.getLength(); j++)
    {
        // diagramDataRelTuple[0] => RID,
        // diagramDataRelTuple[1] => xInputStream
        // diagramDataRelTuple[2] => extension
        uno::Sequence<uno::Any> diagramDataRelTuple = xRelSeq[j];

        OUString sRelId;
        OUString sExtension;
        diagramDataRelTuple[0] >>= sRelId;
        diagramDataRelTuple[2] >>= sExtension;
        OUString sContentType;
        if (sExtension.equalsIgnoreAsciiCase(".WMF"))
            sContentType = "image/x-wmf";
        else
            sContentType = OUString::Concat("image/") + sExtension.subView(1);
        sRelId = sRelId.copy(3);

        StreamDataSequence dataSeq;
        diagramDataRelTuple[1] >>= dataSeq;
        uno::Reference<io::XInputStream> dataImagebin(
            new ::comphelper::SequenceInputStream(dataSeq));

        //nDiagramId is used to make the name unique irrespective of the number of smart arts.
        OUString sFragment = OUString::Concat("media/") + sGrabBagProperyName
                             + OUString::number(nDiagramId) + "_"
                             + OUString::number(j) + sExtension;

        PropertySet aProps(xOutStream);
        aProps.setAnyProperty(PROP_RelId, uno::makeAny(sRelId.toInt32()));

        mpFB->addRelation(xOutStream, sType, OUString("../" + sFragment));

        OUString sDir = OUString::createFromAscii(GetComponentDir());
        uno::Reference<io::XOutputStream> xBinOutStream
            = mpFB->openFragmentStream(sDir + "/" + sFragment, sContentType);

        try
        {
            comphelper::OStorageHelper::CopyInputToOutput(dataImagebin, xBinOutStream);
        }
        catch (const uno::Exception&)
        {
            TOOLS_WARN_EXCEPTION("oox.drawingml", "DrawingML::writeDiagramRels Failed to copy grabbaged Image");
        }
        dataImagebin->closeInput();
    }
}

void DrawingML::WriteFromTo(const uno::Reference<css::drawing::XShape>& rXShape, const awt::Size& aPageSize,
                            const FSHelperPtr& pDrawing)
{
    awt::Point aTopLeft = rXShape->getPosition();
    awt::Size aSize = rXShape->getSize();

    SdrObject* pObj = SdrObject::getSdrObjectFromXShape(rXShape);
    if (pObj)
    {
        Degree100 nRotation = pObj->GetRotateAngle();
        if (nRotation)
        {
            sal_Int16 nHalfWidth = aSize.Width / 2;
            sal_Int16 nHalfHeight = aSize.Height / 2;
            // aTopLeft needs correction for rotated customshapes
            if (pObj->GetObjIdentifier() == OBJ_CUSTOMSHAPE)
            {
                const tools::Rectangle& aSnapRect(pObj->GetSnapRect()); // bounding box of the rotated shape
                aTopLeft.X = aSnapRect.getX() + (aSnapRect.GetWidth() / 2) - nHalfWidth;
                aTopLeft.Y = aSnapRect.getY() + (aSnapRect.GetHeight() / 2) - nHalfHeight;
            }

            // MSO changes the anchor positions at these angles and that does an extra 90 degrees
            // rotation on our shapes, so we output it in such position that MSO
            // can draw this shape correctly.
            if ((nRotation >= 4500_deg100 && nRotation < 13500_deg100) || (nRotation >= 22500_deg100 && nRotation < 31500_deg100))
            {
                aTopLeft.X = aTopLeft.X - nHalfHeight + nHalfWidth;
                aTopLeft.Y = aTopLeft.Y - nHalfWidth + nHalfHeight;

                std::swap(aSize.Width, aSize.Height);
            }
        }
    }

    tools::Rectangle aLocation(aTopLeft.X, aTopLeft.Y, aTopLeft.X + aSize.Width, aTopLeft.Y + aSize.Height);
    double nXpos = static_cast<double>(aLocation.TopLeft().getX()) / static_cast<double>(aPageSize.Width);
    double nYpos = static_cast<double>(aLocation.TopLeft().getY()) / static_cast<double>(aPageSize.Height);

    pDrawing->startElement(FSNS(XML_cdr, XML_from));
    pDrawing->startElement(FSNS(XML_cdr, XML_x));
    pDrawing->write(nXpos);
    pDrawing->endElement(FSNS(XML_cdr, XML_x));
    pDrawing->startElement(FSNS(XML_cdr, XML_y));
    pDrawing->write(nYpos);
    pDrawing->endElement(FSNS(XML_cdr, XML_y));
    pDrawing->endElement(FSNS(XML_cdr, XML_from));

    nXpos = static_cast<double>(aLocation.BottomRight().getX()) / static_cast<double>(aPageSize.Width);
    nYpos = static_cast<double>(aLocation.BottomRight().getY()) / static_cast<double>(aPageSize.Height);

    pDrawing->startElement(FSNS(XML_cdr, XML_to));
    pDrawing->startElement(FSNS(XML_cdr, XML_x));
    pDrawing->write(nXpos);
    pDrawing->endElement(FSNS(XML_cdr, XML_x));
    pDrawing->startElement(FSNS(XML_cdr, XML_y));
    pDrawing->write(nYpos);
    pDrawing->endElement(FSNS(XML_cdr, XML_y));
    pDrawing->endElement(FSNS(XML_cdr, XML_to));
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
