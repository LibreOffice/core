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
#include <drawingml/fontworkhelpers.hxx>
#include <drawingml/textparagraph.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/relationship.hxx>
#include <oox/token/tokens.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <svtools/unitconv.hxx>
#include <sax/fastattribs.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <comphelper/processfactory.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/utils/gradienttools.hxx>

#include <numeric>
#include <string_view>

#include <com/sun/star/awt/CharSet.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/awt/Gradient2.hpp>
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
#include <com/sun/star/drawing/EnhancedCustomShapeParameterType.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegment.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegmentCommand.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/Hatch.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/TextFitToSizeType.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
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
#include <com/sun/star/drawing/RectanglePoint.hpp>

#include <comphelper/propertyvalue.hxx>
#include <comphelper/random.hxx>
#include <comphelper/seqstream.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/xmltools.hxx>
#include <o3tl/any.hxx>
#include <o3tl/safeint.hxx>
#include <o3tl/string_view.hxx>
#include <tools/stream.hxx>
#include <tools/UnitConversion.hxx>
#include <unotools/fontdefs.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/svapp.hxx>
#include <rtl/strbuf.hxx>
#include <filter/msfilter/escherex.hxx>
#include <filter/msfilter/util.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/svxenum.hxx>
#include <editeng/unonames.hxx>
#include <editeng/unoprnms.hxx>
#include <editeng/flditem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/unonrule.hxx>
#include <docmodel/uno/UnoComplexColor.hxx>
#include <svx/svdoashp.hxx>
#include <svx/svdomedia.hxx>
#include <svx/svdtrans.hxx>
#include <svx/unoshape.hxx>
#include <svx/EnhancedCustomShape2d.hxx>
#include <drawingml/presetgeometrynames.hxx>
#include <docmodel/uno/UnoGradientTools.hxx>

using namespace ::css;
using namespace ::css::beans;
using namespace ::css::drawing;
using namespace ::css::i18n;
using namespace ::css::style;
using namespace ::css::text;
using namespace ::css::uno;
using namespace ::css::container;
using namespace ::com::sun::star::drawing::EnhancedCustomShapeSegmentCommand;

using ::css::io::XOutputStream;
using ::sax_fastparser::FSHelperPtr;
using ::sax_fastparser::FastSerializerHelper;

namespace
{
const char* g_aPredefinedClrNames[] = {
    "dk1",
    "lt1",
    "dk2",
    "lt2",
    "accent1",
    "accent2",
    "accent3",
    "accent4",
    "accent5",
    "accent6",
    "hlink",
    "folHlink",
};
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

GraphicExportCache& GraphicExportCache::get()
{
    static GraphicExportCache staticGraphicExportCache;
    return staticGraphicExportCache;
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
void WriteGradientPath(const basegfx::BGradient& rBGradient, const FSHelperPtr& pFS, const bool bCircle)
{
    pFS->startElementNS(XML_a, XML_path, XML_path, bCircle ? "circle" : "rect");

    // Write the focus rectangle. Work with the focus point, and assume
    // that it extends 50% in all directions.  The below
    // left/top/right/bottom values are percentages, where 0 means the
    // edge of the tile rectangle and 100% means the center of it.
    rtl::Reference<sax_fastparser::FastAttributeList> pAttributeList(
        sax_fastparser::FastSerializerHelper::createAttrList());
    sal_Int32 nLeftPercent = rBGradient.GetXOffset();
    pAttributeList->add(XML_l, OString::number(nLeftPercent * PER_PERCENT));
    sal_Int32 nTopPercent = rBGradient.GetYOffset();
    pAttributeList->add(XML_t, OString::number(nTopPercent * PER_PERCENT));
    sal_Int32 nRightPercent = 100 - rBGradient.GetXOffset();
    pAttributeList->add(XML_r, OString::number(nRightPercent * PER_PERCENT));
    sal_Int32 nBottomPercent = 100 - rBGradient.GetYOffset();
    pAttributeList->add(XML_b, OString::number(nBottomPercent * PER_PERCENT));
    pFS->singleElementNS(XML_a, XML_fillToRect, pAttributeList);

    pFS->endElementNS(XML_a, XML_path);
}
}

// not thread safe
sal_Int32 DrawingML::mnDrawingMLCount = 0;
sal_Int32 DrawingML::mnVmlCount = 0;
sal_Int32 DrawingML::mnChartCount = 0;

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

void DrawingML::ResetMlCounters()
{
    mnDrawingMLCount = 0;
    mnVmlCount = 0;
    mnChartCount = 0;
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

        sColor = sBuf.toString();
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
        for (const auto& rProp : aGrabBag)
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
    basegfx::BGradient aTransparenceGradient;
    OUString sFillTransparenceGradientName;
    bool bNeedGradientFill(false);

    if (GetProperty(rXPropSet, "FillTransparenceGradientName")
        && (mAny >>= sFillTransparenceGradientName)
        && !sFillTransparenceGradientName.isEmpty()
        && GetProperty(rXPropSet, "FillTransparenceGradient"))
    {
        aTransparenceGradient = model::gradient::getFromAny(mAny);
        basegfx::BColor aSingleColor;
        bNeedGradientFill = !aTransparenceGradient.GetColorStops().isSingleColor(aSingleColor);

        // we no longer need to 'guess' if FillTransparenceGradient is used by
        // comparing it's 1st color to COL_BLACK after having tested that the
        // FillTransparenceGradientName is set
        if (!bNeedGradientFill)
        {
            // Our alpha is a gray color value.
            const sal_uInt8 nRed(aSingleColor.getRed() * 255.0);

            // drawingML alpha is a percentage on a 0..100000 scale.
            nAlpha = (255 - nRed) * oox::drawingml::MAX_PERCENT / 255;
        }
    }

    // write XML
    if (bNeedGradientFill)
    {
        // no longer create copy/PseudoColorGradient, use new API of
        // WriteGradientFill to express fix fill color
        mpFS->startElementNS(XML_a, XML_gradFill, XML_rotWithShape, "0");
        WriteGradientFill(nullptr, nFillColor, &aTransparenceGradient);
        mpFS->endElementNS( XML_a, XML_gradFill );
    }
    else if ( nFillColor != nOriginalColor )
    {
        // the user has set a different color for the shape
        if (!WriteSchemeColor(u"FillComplexColor"_ustr, rXPropSet))
        {
            WriteSolidFill(::Color(ColorTransparency, nFillColor & 0xffffff), nAlpha);
        }
    }
    // tdf#91332 LO doesn't export the actual theme.xml in XLSX.
    else if ( !sColorFillScheme.isEmpty() && GetDocumentType() != DOCUMENT_XLSX )
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

bool DrawingML::WriteSchemeColor(OUString const& rPropertyName, const uno::Reference<beans::XPropertySet>& xPropertySet)
{
    if (!xPropertySet->getPropertySetInfo()->hasPropertyByName(rPropertyName))
        return false;

    uno::Reference<util::XComplexColor> xComplexColor;
    xPropertySet->getPropertyValue(rPropertyName) >>= xComplexColor;
    if (!xComplexColor.is())
        return false;

    auto aComplexColor = model::color::getFromXComplexColor(xComplexColor);
    if (aComplexColor.getThemeColorType() == model::ThemeColorType::Unknown)
        return false;
    const char* pColorName = g_aPredefinedClrNames[sal_Int16(aComplexColor.getThemeColorType())];
    mpFS->startElementNS(XML_a, XML_solidFill);
    mpFS->startElementNS(XML_a, XML_schemeClr, XML_val, pColorName);
    for (auto const& rTransform : aComplexColor.getTransformations())
    {
        switch (rTransform.meType)
        {
            case model::TransformationType::LumMod:
                mpFS->singleElementNS(XML_a, XML_lumMod, XML_val, OString::number(rTransform.mnValue * 10));
                break;
            case model::TransformationType::LumOff:
                mpFS->singleElementNS(XML_a, XML_lumOff, XML_val, OString::number(rTransform.mnValue * 10));
                break;
            case model::TransformationType::Tint:
                mpFS->singleElementNS(XML_a, XML_tint, XML_val, OString::number(rTransform.mnValue * 10));
                break;
            case model::TransformationType::Shade:
                mpFS->singleElementNS(XML_a, XML_shade, XML_val, OString::number(rTransform.mnValue * 10));
                break;
            default:
                break;
        }
    }
    // Alpha is actually not contained in maTransformations although possible (as of Mar 2023).
    sal_Int16 nAPITransparency(0);
    if ((rPropertyName == u"FillComplexColor" && GetProperty(xPropertySet, "FillTransparence"))
        || (rPropertyName == u"LineComplexColor" && GetProperty(xPropertySet, "LineTransparence"))
        || (rPropertyName == u"CharComplexColor" && GetProperty(xPropertySet, "CharTransparence")))
    {
        mAny >>= nAPITransparency;
    }
    if (nAPITransparency != 0)
        mpFS->singleElementNS(XML_a, XML_alpha, XML_val,
                              OString::number(MAX_PERCENT - (PER_PERCENT * nAPITransparency)));

    mpFS->endElementNS(XML_a, XML_schemeClr);
    mpFS->endElementNS(XML_a, XML_solidFill);

    return true;
}

void DrawingML::WriteGradientStop(double fOffset, const basegfx::BColor& rColor, const basegfx::BColor& rAlpha)
{
    mpFS->startElementNS(XML_a, XML_gs, XML_pos, OString::number(basegfx::fround(fOffset * 100000)));
    WriteColor(
        ::Color(rColor),
        basegfx::fround((1.0 - rAlpha.luminance()) * oox::drawingml::MAX_PERCENT));
    mpFS->endElementNS( XML_a, XML_gs );
}

::Color DrawingML::ColorWithIntensity( sal_uInt32 nColor, sal_uInt32 nIntensity )
{
    return ::Color(ColorTransparency, ( ( ( nColor & 0xff ) * nIntensity ) / 100 )
        | ( ( ( ( ( nColor & 0xff00 ) >> 8 ) * nIntensity ) / 100 ) << 8 )
        | ( ( ( ( ( nColor & 0xff0000 ) >> 8 ) * nIntensity ) / 100 ) << 8 ));
}

void DrawingML::WriteGradientFill( const Reference< XPropertySet >& rXPropSet )
{
    if (!GetProperty(rXPropSet, "FillGradient"))
        return;

    // use BGradient constructor directly, it will take care of Gradient/Gradient2
    basegfx::BGradient aGradient = model::gradient::getFromAny(mAny);

    // get InteropGrabBag and search the relevant attributes
    basegfx::BGradient aOriginalGradient;
    Sequence< PropertyValue > aGradientStops;
    if ( GetProperty( rXPropSet, "InteropGrabBag" ) )
    {
        Sequence< PropertyValue > aGrabBag;
        mAny >>= aGrabBag;
        for (const auto& rProp : aGrabBag)
            if( rProp.Name == "GradFillDefinition" )
                rProp.Value >>= aGradientStops;
            else if( rProp.Name == "OriginalGradFill" )
                aOriginalGradient = model::gradient::getFromAny(rProp.Value);
    }

    // check if an ooxml gradient had been imported and if the user has modified it
    // Gradient grab-bag depends on theme grab-bag, which is implemented
    // only for DOCX.
    if (aOriginalGradient == aGradient && GetDocumentType() == DOCUMENT_DOCX)
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
        mpFS->startElementNS(XML_a, XML_gradFill, XML_rotWithShape, "0");

        basegfx::BGradient aTransparenceGradient;
        basegfx::BGradient* pTransparenceGradient(nullptr);
        double fTransparency(0.0);
        OUString sFillTransparenceGradientName;

        if (GetProperty(rXPropSet, "FillTransparenceGradientName")
            && (mAny >>= sFillTransparenceGradientName)
            && !sFillTransparenceGradientName.isEmpty()
            && GetProperty(rXPropSet, "FillTransparenceGradient"))
        {
            // TransparenceGradient is only used when name is not empty
            aTransparenceGradient = model::gradient::getFromAny(mAny);
            pTransparenceGradient = &aTransparenceGradient;
        }
        else if (GetProperty(rXPropSet, "FillTransparence"))
        {
            // no longer create PseudoTransparencyGradient, use new API of
            // WriteGradientFill to express fix transparency
            sal_Int32 nTransparency(0);
            mAny >>= nTransparency;
            // nTransparency is [0..100]%
            fTransparency = nTransparency * 0.01;
        }

        // tdf#155852 The gradient might wrongly have StepCount==0, as the draw:gradient-step-count
        // attribute in ODF does not belong to the gradient definition but is an attribute in
        // the graphic style of the shape.
        if (GetProperty(rXPropSet, "FillGradientStepCount"))
        {
            sal_Int16 nStepCount = 0;
            mAny >>= nStepCount;
            aGradient.SetSteps(nStepCount);
        }

        WriteGradientFill(&aGradient, 0, pTransparenceGradient, fTransparency);

        mpFS->endElementNS(XML_a, XML_gradFill);
    }
}

void DrawingML::WriteGrabBagGradientFill( const Sequence< PropertyValue >& aGradientStops, const basegfx::BGradient& rBGradient )
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
        for (const auto& rProp : aGradientStop)
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
        mpFS->startElementNS(XML_a, XML_gs, XML_pos, OString::number(nPos * 100000.0));
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

    switch (rBGradient.GetGradientStyle())
    {
        default:
        {
            const sal_Int16 nAngle(rBGradient.GetAngle());
            mpFS->singleElementNS(
                XML_a, XML_lin, XML_ang,
                OString::number(((3600 - static_cast<sal_Int32>(nAngle) + 900) * 6000) % 21600000));
            break;
        }
        case awt::GradientStyle_RADIAL:
        {
            WriteGradientPath(rBGradient, mpFS, true);
            break;
        }
    }
}

void DrawingML::WriteGradientFill(
    const basegfx::BGradient* pColorGradient, sal_Int32 nFixColor,
    const basegfx::BGradient* pTransparenceGradient, double fFixTransparence)
{
    basegfx::BColorStops aColorStops;
    basegfx::BColorStops aAlphaStops;
    basegfx::BColor aSingleColor(::Color(ColorTransparency, nFixColor).getBColor());
    basegfx::BColor aSingleAlpha(fFixTransparence);
    const basegfx::BGradient* pGradient(pColorGradient);

    if (nullptr != pColorGradient)
    {
        // extract and correct/process ColorStops
        basegfx::utils::prepareColorStops(*pColorGradient, aColorStops, aSingleColor);

        // tdf#155827 Convert 'axial' to 'linear' before synchronize and for each gradient separate.
        if (aColorStops.size() > 0 && awt::GradientStyle_AXIAL == pColorGradient->GetGradientStyle())
            aColorStops.doApplyAxial();
    }
    if (nullptr != pTransparenceGradient)
    {
        // remember basic Gradient definition to use
        // So we can get the gradient geometry in any case from pGradient.
        if (nullptr == pGradient)
        {
            pGradient = pTransparenceGradient;
        }

        // extract and correct/process AlphaStops
        basegfx::utils::prepareColorStops(*pTransparenceGradient, aAlphaStops, aSingleAlpha);
        if (aAlphaStops.size() > 0
            && awt::GradientStyle_AXIAL == pTransparenceGradient->GetGradientStyle())
        {
            aAlphaStops.doApplyAxial();
        }
    }

    if (nullptr == pGradient)
    {
        // an error - see comment in header - is to give neither pColorGradient
        // nor pTransparenceGradient
        assert(false && "pColorGradient or pTransparenceGradient should be set");
        return;
    }

    // Apply steps if used. That increases the number of stops and thus needs to be done before
    // synchronize.
    if (pGradient->GetSteps())
    {
        aColorStops.doApplySteps(pGradient->GetSteps());
        // transparency gradients are always automatic, so do not have steps.
    }

    // synchronize ColorStops and AlphaStops as preparation to export
    // so also gradients 'coupled' indirectly using the 'FillTransparenceGradient'
    // method (at import time) will be exported again.
    basegfx::utils::synchronizeColorStops(aColorStops, aAlphaStops, aSingleColor, aSingleAlpha);

    if (aColorStops.size() != aAlphaStops.size())
    {
        // this is an error - synchronizeColorStops above *has* to create that
        // state, see description there (!)
        assert(false && "oox::WriteGradientFill: non-synchronized gradients (!)");
        return;
    }

    bool bLinearOrAxial(awt::GradientStyle_LINEAR == pGradient->GetGradientStyle()
                        || awt::GradientStyle_AXIAL == pGradient->GetGradientStyle());
    if (!bLinearOrAxial)
    {
        // case awt::GradientStyle_RADIAL:
        // case awt::GradientStyle_ELLIPTICAL:
        // case awt::GradientStyle_RECT:
        // case awt::GradientStyle_SQUARE:
        // all these types need the gradients to be mirrored
        aColorStops.reverseColorStops();
        aAlphaStops.reverseColorStops();
    }

    // If there were one stop, prepareColorStops() method would have cleared aColorStops, same for
    // aAlphaStops. In case of empty stops vectors synchronizeColorStops() method creates two stops
    // for each. So at this point we have at least two stops and can fulfill the requirement of
    // <gsLst> element to have at least two child elements.

    // export GradientStops (with alpha)
    mpFS->startElementNS(XML_a, XML_gsLst);

    basegfx::BColorStops::const_iterator aCurrColor(aColorStops.begin());
    basegfx::BColorStops::const_iterator aCurrAlpha(aAlphaStops.begin());

    while (aCurrColor != aColorStops.end() && aCurrAlpha != aAlphaStops.end())
    {
        WriteGradientStop(
            aCurrColor->getStopOffset(),
            aCurrColor->getStopColor(),
            aCurrAlpha->getStopColor());
        aCurrColor++;
        aCurrAlpha++;
    }

    mpFS->endElementNS( XML_a, XML_gsLst );

    if (bLinearOrAxial)
    {
        // CT_LinearShadeProperties, cases where gradient rotation has to be exported
        // 'scaled' does not exist in LO, so only 'ang'.
        const sal_Int16 nAngle(pGradient->GetAngle());
        mpFS->singleElementNS(
            XML_a, XML_lin, XML_ang,
            OString::number(((3600 - static_cast<sal_Int32>(nAngle) + 900) * 6000) % 21600000));
    }
    else
    {
        // CT_PathShadeProperties, cases where gradient path has to be exported
        // Concentric fill is not yet implemented, therefore no type 'shape', only 'circle' or 'rect'
        const bool bCircle(pGradient->GetGradientStyle() == awt::GradientStyle_RADIAL ||
            pGradient->GetGradientStyle() == awt::GradientStyle_ELLIPTICAL);
        WriteGradientPath(*pGradient, mpFS, bCircle);
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

        for (const auto& rProp : aGrabBag)
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
        for (const auto& rStyleProp : aStyleProperties)
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
                              nLineWidth == 0 || GetDocumentType() == DOCUMENT_XLSX    // tdf#119565 LO doesn't export the actual theme.xml in XLSX.
                                  || (nLineWidth > 1 && nStyleLineWidth != nLineWidth)));

    if( bColorSet )
    {
        if( nColor != nOriginalColor )
        {
            // the user has set a different color for the line
            if (!WriteSchemeColor(u"LineComplexColor"_ustr, rXPropSet))
                WriteSolidFill(nColor, nColorAlpha);
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

        // tdf#119565 LO doesn't export the actual theme.xml in XLSX.
        if (aStyleLineJoint == LineJoint_NONE || GetDocumentType() == DOCUMENT_XLSX
            || aStyleLineJoint != eLineJoint)
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

OUString DrawingML::GetComponentDir() const
{
    return OUString(getComponentDir(meDocumentType));
}

OUString DrawingML::GetRelationCompPrefix() const
{
    return OUString(getRelationCompPrefix(meDocumentType));
}

void GraphicExport::writeSvgExtension(OUString const& rSvgRelId)
{
    if (rSvgRelId.isEmpty())
        return;

    mpFS->startElementNS(XML_a, XML_extLst);
    mpFS->startElementNS(XML_a, XML_ext, XML_uri, "{96DAC541-7B7A-43D3-8B79-37D633B846F1}");
    mpFS->singleElementNS(XML_asvg, XML_svgBlip,
            FSNS(XML_xmlns, XML_asvg), mpFilterBase->getNamespaceURL(OOX_NS(asvg)),
            FSNS(XML_r, XML_embed), rSvgRelId);
    mpFS->endElementNS(XML_a, XML_ext);
    mpFS->endElementNS( XML_a, XML_extLst);
}

void GraphicExport::writeBlip(Graphic const& rGraphic, std::vector<model::BlipEffect> const& rEffects, bool bRelPathToMedia)
{
    OUString sRelId = writeToStorage(rGraphic, bRelPathToMedia);

    mpFS->startElementNS(XML_a, XML_blip, FSNS(XML_r, XML_embed), sRelId);

    auto const& rVectorGraphicDataPtr = rGraphic.getVectorGraphicData();

    if (rVectorGraphicDataPtr && rVectorGraphicDataPtr->getType() == VectorGraphicDataType::Svg)
    {
        OUString sSvgRelId = writeToStorage(rGraphic, bRelPathToMedia, TypeHint::SVG);
        writeSvgExtension(sSvgRelId);
    }

    for (auto const& rEffect : rEffects)
    {
        switch (rEffect.meType)
        {
            case model::BlipEffectType::AlphaBiLevel:
            {
                mpFS->singleElementNS(XML_a, XML_alphaBiLevel, XML_thresh, OString::number(rEffect.mnThreshold));
            }
            break;
            case model::BlipEffectType::AlphaCeiling:
            {
                mpFS->singleElementNS(XML_a, XML_alphaCeiling);
            }
            break;
            case model::BlipEffectType::AlphaFloor:
            {
                mpFS->singleElementNS(XML_a, XML_alphaFloor);
            }
            break;
            case model::BlipEffectType::AlphaInverse:
            {
                mpFS->singleElementNS(XML_a, XML_alphaInv);
                // TODO: export rEffect.maColor1
            }
            break;
            case model::BlipEffectType::AlphaModulate:
            {
                mpFS->singleElementNS(XML_a, XML_alphaMod);
                // TODO
            }
            break;
            case model::BlipEffectType::AlphaModulateFixed:
            {
                mpFS->singleElementNS(XML_a, XML_alphaModFix, XML_amt, OString::number(rEffect.mnAmount));
            }
            break;
            case model::BlipEffectType::AlphaReplace:
            {
                mpFS->singleElementNS(XML_a, XML_alphaRepl, XML_a, OString::number(rEffect.mnAlpha));
            }
            break;
            case model::BlipEffectType::BiLevel:
            {
                mpFS->singleElementNS(XML_a, XML_biLevel, XML_thresh, OString::number(rEffect.mnThreshold));
            }
            break;
            case model::BlipEffectType::Blur:
            {
                mpFS->singleElementNS(XML_a, XML_blur,
                    XML_rad, OString::number(rEffect.mnRadius),
                    XML_grow, rEffect.mbGrow ? "1" : "0");
            }
            break;
            case model::BlipEffectType::ColorChange:
            {
                mpFS->startElementNS(XML_a, XML_clrChange, XML_useA, rEffect.mbUseAlpha ? "1" : "0");
                mpFS->endElementNS(XML_a, XML_clrChange);
            }
            break;
            case model::BlipEffectType::ColorReplace:
            {
                mpFS->startElementNS(XML_a, XML_clrRepl);
                mpFS->endElementNS(XML_a, XML_clrRepl);
            }
            break;
            case model::BlipEffectType::DuoTone:
            {
                mpFS->startElementNS(XML_a, XML_duotone);
                mpFS->endElementNS(XML_a, XML_duotone);
            }
            break;
            case model::BlipEffectType::FillOverlay:
            {
                mpFS->singleElementNS(XML_a, XML_fillOverlay);
            }
            break;
            case model::BlipEffectType::Grayscale:
            {
                mpFS->singleElementNS(XML_a, XML_grayscl);
            }
            break;
            case model::BlipEffectType::HSL:
            {
                mpFS->singleElementNS(XML_a, XML_hsl,
                    XML_hue, OString::number(rEffect.mnHue),
                    XML_sat, OString::number(rEffect.mnSaturation),
                    XML_lum, OString::number(rEffect.mnLuminance));
            }
            break;
            case model::BlipEffectType::Luminance:
            {
                mpFS->singleElementNS(XML_a, XML_lum,
                    XML_bright, OString::number(rEffect.mnBrightness),
                    XML_contrast, OString::number(rEffect.mnContrast));
            }
            break;
            case model::BlipEffectType::Tint:
            {
                mpFS->singleElementNS(XML_a, XML_tint,
                    XML_hue, OString::number(rEffect.mnHue),
                    XML_amt, OString::number(rEffect.mnAmount));
            }
            break;

            default:
                break;
        }
    }

    mpFS->endElementNS(XML_a, XML_blip);
}

OUString GraphicExport::writeNewEntryToStorage(const Graphic& rGraphic, bool bRelPathToMedia)
{
    GfxLink const& rLink = rGraphic.GetGfxLink();

    OUString sMediaType;
    OUString aExtension;

    SvMemoryStream aStream;
    const void* aData = rLink.GetData();
    std::size_t nDataSize = rLink.GetDataSize();

    switch (rLink.GetType())
    {
        case GfxLinkType::NativeGif:
            sMediaType = u"image/gif"_ustr;
            aExtension = u"gif"_ustr;
            break;

        // #i15508# added BMP type for better exports
        // export not yet active, so adding for reference (not checked)
        case GfxLinkType::NativeBmp:
            sMediaType = u"image/bmp"_ustr;
            aExtension = u"bmp"_ustr;
            break;

        case GfxLinkType::NativeJpg:
            sMediaType = u"image/jpeg"_ustr;
            aExtension = u"jpeg"_ustr;
            break;
        case GfxLinkType::NativePng:
            sMediaType = u"image/png"_ustr;
            aExtension = u"png"_ustr;
            break;
        case GfxLinkType::NativeTif:
            sMediaType = u"image/tiff"_ustr;
            aExtension = u"tif"_ustr;
            break;
        case GfxLinkType::NativeWmf:
            sMediaType = u"image/x-wmf"_ustr;
            aExtension = u"wmf"_ustr;
            break;
        case GfxLinkType::NativeMet:
            sMediaType = u"image/x-met"_ustr;
            aExtension = u"met"_ustr;
            break;
        case GfxLinkType::NativePct:
            sMediaType = u"image/x-pict"_ustr;
            aExtension = u"pct"_ustr;
            break;
        case GfxLinkType::NativeMov:
            sMediaType = u"application/movie"_ustr;
            aExtension = u"MOV"_ustr;
            break;
        default:
        {
            GraphicType aType = rGraphic.GetType();
            if (aType == GraphicType::Bitmap || aType == GraphicType::GdiMetafile)
            {
                if (aType == GraphicType::Bitmap)
                {
                    (void)GraphicConverter::Export(aStream, rGraphic, ConvertDataFormat::PNG);
                    sMediaType = u"image/png"_ustr;
                    aExtension = u"png"_ustr;
                }
                else
                {
                    (void)GraphicConverter::Export(aStream, rGraphic, ConvertDataFormat::EMF);
                    sMediaType = u"image/x-emf"_ustr;
                    aExtension = u"emf"_ustr;
                }
            }
            else
            {
                SAL_WARN("oox.shape", "unhandled graphic type " << static_cast<int>(aType));

                /*Earlier, even in case of unhandled graphic types we were
                  proceeding to write the image, which would eventually
                  write an empty image with a zero size, and return a valid
                  relationID, which is incorrect.
                  */
                return OUString();
            }

            aData = aStream.GetData();
            nDataSize = aStream.GetEndOfData();
        }
        break;
    }

    GraphicExportCache& rGraphicExportCache = GraphicExportCache::get();
    auto sImageCountString = OUString::number(rGraphicExportCache.nextImageCount());

    OUString sComponentDir(getComponentDir(meDocumentType));

    OUString sImagePath = sComponentDir + u"/media/image"_ustr + sImageCountString + u"."_ustr + aExtension;

    Reference<XOutputStream> xOutStream = mpFilterBase->openFragmentStream(sImagePath, sMediaType);
    xOutStream->writeBytes(Sequence<sal_Int8>(static_cast<const sal_Int8*>(aData), nDataSize));
    xOutStream->closeOutput();

    OUString sRelationCompPrefix;
    if (bRelPathToMedia)
        sRelationCompPrefix = u"../"_ustr;
    else
        sRelationCompPrefix = getRelationCompPrefix(meDocumentType);

    OUString sPath = sRelationCompPrefix + u"media/image"_ustr + sImageCountString + u"."_ustr + aExtension;

    rGraphicExportCache.addExportGraphics(rGraphic.GetChecksum(), sPath);

    return sPath;
}

namespace
{
BitmapChecksum makeChecksumUniqueForSVG(BitmapChecksum const& rChecksum)
{
    // need to modify the checksum so we know it's for SVG - just invert it
    return ~rChecksum;
}

} // end anonymous namespace

OUString GraphicExport::writeNewSvgEntryToStorage(const Graphic& rGraphic, bool bRelPathToMedia)
{
    OUString sMediaType = u"image/svg"_ustr;
    OUString aExtension = u"svg"_ustr;

    GfxLink const& rLink = rGraphic.GetGfxLink();
    if (rLink.GetType() != GfxLinkType::NativeSvg)
        return OUString();

    const void* aData = rLink.GetData();
    std::size_t nDataSize = rLink.GetDataSize();

    GraphicExportCache& rGraphicExportCache = GraphicExportCache::get();
    auto sImageCountString = OUString::number(rGraphicExportCache.nextImageCount());

    OUString sComponentDir(getComponentDir(meDocumentType));

    OUString sImagePath = sComponentDir + u"/media/image"_ustr + sImageCountString + u"."_ustr + aExtension;

    Reference<XOutputStream> xOutStream = mpFilterBase->openFragmentStream(sImagePath, sMediaType);
    xOutStream->writeBytes(Sequence<sal_Int8>(static_cast<const sal_Int8*>(aData), nDataSize));
    xOutStream->closeOutput();

    OUString sRelationCompPrefix;
    if (bRelPathToMedia)
        sRelationCompPrefix = u"../"_ustr;
    else
        sRelationCompPrefix = getRelationCompPrefix(meDocumentType);

    OUString sPath = sRelationCompPrefix + u"media/image"_ustr + sImageCountString + u"."_ustr + aExtension;

    rGraphicExportCache.addExportGraphics(makeChecksumUniqueForSVG(rGraphic.GetChecksum()), sPath);

    return sPath;
}

OUString GraphicExport::writeToStorage(const Graphic& rGraphic, bool bRelPathToMedia, TypeHint eHint)
{
    OUString sPath;

    auto aChecksum = rGraphic.GetChecksum();
    if (eHint == TypeHint::SVG)
        aChecksum = makeChecksumUniqueForSVG(aChecksum);

    GraphicExportCache& rGraphicExportCache = GraphicExportCache::get();
    sPath = rGraphicExportCache.findExportGraphics(aChecksum);

    if (sPath.isEmpty())
    {
        if (eHint == TypeHint::SVG)
            sPath = writeNewSvgEntryToStorage(rGraphic, bRelPathToMedia);
        else
            sPath = writeNewEntryToStorage(rGraphic, bRelPathToMedia);

        if (sPath.isEmpty())
            return OUString(); // couldn't store
    }

    OUString sRelId = mpFilterBase->addRelation(mpFS->getOutputStream(), oox::getRelationship(Relationship::IMAGE), sPath);

    return sRelId;
}

std::shared_ptr<GraphicExport> DrawingML::createGraphicExport()
{
    return std::make_shared<GraphicExport>(mpFS, mpFB, meDocumentType);
}

OUString DrawingML::writeGraphicToStorage(const Graphic& rGraphic , bool bRelPathToMedia, GraphicExport::TypeHint eHint)
{
    GraphicExport aExporter(mpFS, mpFB, meDocumentType);
    return aExporter.writeToStorage(rGraphic, bRelPathToMedia, eHint);
}

void DrawingML::WriteMediaNonVisualProperties(const css::uno::Reference<css::drawing::XShape>& xShape)
{
    SdrMediaObj* pMediaObj = dynamic_cast<SdrMediaObj*>(SdrObject::getSdrObjectFromXShape(xShape));
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
    if (aMimeType.startsWith("audio/"))
    {
        eMediaType = Relationship::AUDIO;
    }
    else
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
        else if (aExtension.equalsIgnoreAsciiCase(".mp3"))
        {
            aMimeType = "audio/mp3";
            eMediaType = Relationship::AUDIO;
        }
    }

    OUString aVideoFileRelId;
    OUString aMediaRelId;

    if (bEmbed)
    {
        sal_Int32  nImageCount = GraphicExportCache::get().nextImageCount();

        OUString sFileName = GetComponentDir() + u"/media/media"_ustr + OUString::number(nImageCount) + aExtension;

        // copy the video stream
        Reference<XOutputStream> xOutStream = mpFB->openFragmentStream(sFileName, aMimeType);

        uno::Reference<io::XInputStream> xInputStream(pMediaObj->GetInputStream());
        comphelper::OStorageHelper::CopyInputToOutput(xInputStream, xOutStream);

        xOutStream->closeOutput();

        // create the relation
        OUString aPath = GetRelationCompPrefix() + u"media/media"_ustr + OUString::number(nImageCount) + aExtension;

        aVideoFileRelId = mpFB->addRelation(mpFS->getOutputStream(), oox::getRelationship(eMediaType), aPath);
        aMediaRelId = mpFB->addRelation(mpFS->getOutputStream(), oox::getRelationship(Relationship::MEDIA), aPath);
    }
    else
    {
        aVideoFileRelId = mpFB->addRelation(mpFS->getOutputStream(), oox::getRelationship(eMediaType), rURL, true);
        aMediaRelId = mpFB->addRelation(mpFS->getOutputStream(), oox::getRelationship(Relationship::MEDIA), rURL, true);
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

void DrawingML::WriteXGraphicBlip(uno::Reference<beans::XPropertySet> const & rXPropSet,
                                      uno::Reference<graphic::XGraphic> const & rxGraphic,
                                      bool bRelPathToMedia)
{
    OUString sRelId;

    if (!rxGraphic.is())
        return;

    Graphic aGraphic(rxGraphic);

    sRelId = writeGraphicToStorage(aGraphic, bRelPathToMedia);

    mpFS->startElementNS(XML_a, XML_blip, FSNS(XML_r, XML_embed), sRelId);

    auto pVectorGraphicDataPtr = aGraphic.getVectorGraphicData();

    if (pVectorGraphicDataPtr && pVectorGraphicDataPtr->getType() == VectorGraphicDataType::Svg)
    {
        GraphicExport aExporter(mpFS, mpFB, meDocumentType);
        OUString sSvgRelId =  aExporter.writeToStorage(aGraphic, bRelPathToMedia, GraphicExport::TypeHint::SVG);
        if (!sSvgRelId.isEmpty())
            aExporter.writeSvgExtension(sSvgRelId);
    }

    WriteImageBrightnessContrastTransparence(rXPropSet);

    WriteArtisticEffect(rXPropSet);

    mpFS->endElementNS(XML_a, XML_blip);
}

void DrawingML::WriteXGraphicBlipMode(uno::Reference<beans::XPropertySet> const & rXPropSet,
                                      uno::Reference<graphic::XGraphic> const & rxGraphic,
                                      css::awt::Size const& rSize)
{
    BitmapMode eBitmapMode(BitmapMode_NO_REPEAT);
    if (GetProperty(rXPropSet, "FillBitmapMode"))
        mAny >>= eBitmapMode;

    SAL_INFO("oox.shape", "fill bitmap mode: " << int(eBitmapMode));

    switch (eBitmapMode)
    {
    case BitmapMode_REPEAT:
        WriteXGraphicTile(rXPropSet, rxGraphic, rSize);
        break;
    case BitmapMode_STRETCH:
        WriteXGraphicStretch(rXPropSet, rxGraphic);
        break;
    case BitmapMode_NO_REPEAT:
        WriteXGraphicCustomPosition(rXPropSet, rxGraphic, rSize);
        break;
    default:
        break;
    }
}

void DrawingML::WriteBlipOrNormalFill(const Reference<XPropertySet>& xPropSet,
                                      const OUString& rURLPropName, const awt::Size& rSize)
{
    // check for blip and otherwise fall back to normal fill
    // we always store normal fill properties but OOXML
    // uses a choice between our fill props and BlipFill
    if (GetProperty ( xPropSet, rURLPropName ))
        WriteBlipFill( xPropSet, rURLPropName );
    else
        WriteFill(xPropSet, rSize);
}

void DrawingML::WriteBlipFill(const Reference<XPropertySet>& rXPropSet,
                              const OUString& sURLPropName, const awt::Size& rSize)
{
    WriteBlipFill( rXPropSet, rSize, sURLPropName, XML_a );
}

void DrawingML::WriteBlipFill(const Reference<XPropertySet>& rXPropSet, const awt::Size& rSize,
                              const OUString& sURLPropName, sal_Int32 nXmlNamespace)
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
        WriteXGraphicBlipFill(rXPropSet, xGraphic, nXmlNamespace, bWriteMode, false, rSize);
    }
}

void DrawingML::WriteXGraphicBlipFill(uno::Reference<beans::XPropertySet> const & rXPropSet,
                                      uno::Reference<graphic::XGraphic> const & rxGraphic,
                                      sal_Int32 nXmlNamespace, bool bWriteMode,
                                      bool bRelPathToMedia, css::awt::Size const& rSize)
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
        WriteXGraphicBlipMode(rXPropSet, rxGraphic, rSize);
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

        sal_Int32 nAlpha = MAX_PERCENT;
        if (GetProperty(rXPropSet, "FillTransparence"))
        {
            sal_Int32 nTransparency = 0;
            mAny >>= nTransparency;
            nAlpha = (MAX_PERCENT - (PER_PERCENT * nTransparency));
        }

        mpFS->startElementNS(XML_a, XML_fgClr);
        WriteColor(::Color(ColorTransparency, rHatch.Color), nAlpha);
        mpFS->endElementNS( XML_a , XML_fgClr );

        ::Color nColor = COL_WHITE;

        if ( GetProperty( rXPropSet, "FillBackground" ) )
        {
            bool isBackgroundFilled = false;
            mAny >>= isBackgroundFilled;
            if( isBackgroundFilled )
            {
                if( GetProperty( rXPropSet, "FillColor" ) )
                {
                    mAny >>= nColor;
                }
            }
            else
                nAlpha = 0;
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

static OUString lclConvertRectanglePointToToken(RectanglePoint eRectanglePoint)
{
    OUString sAlignment;
    switch (eRectanglePoint)
    {
        case RectanglePoint_LEFT_TOP:
            sAlignment = "tl";
            break;
        case RectanglePoint_MIDDLE_TOP:
            sAlignment = "t";
            break;
        case RectanglePoint_RIGHT_TOP:
            sAlignment = "tr";
            break;
        case RectanglePoint_LEFT_MIDDLE:
            sAlignment = "l";
            break;
        case RectanglePoint_MIDDLE_MIDDLE:
            sAlignment = "ctr";
            break;
        case RectanglePoint_RIGHT_MIDDLE:
            sAlignment = "r";
            break;
        case RectanglePoint_LEFT_BOTTOM:
            sAlignment = "bl";
            break;
        case RectanglePoint_MIDDLE_BOTTOM:
            sAlignment = "b";
            break;
        case RectanglePoint_RIGHT_BOTTOM:
            sAlignment = "br";
            break;
        default:
            break;
    }
    return sAlignment;
}

void DrawingML::WriteXGraphicTile(uno::Reference<beans::XPropertySet> const& rXPropSet,
                                  uno::Reference<graphic::XGraphic> const& rxGraphic,
                                  css::awt::Size const& rSize)
{
    Graphic aGraphic(rxGraphic);
    Size aOriginalSize(aGraphic.GetPrefSize());
    const MapMode& rMapMode = aGraphic.GetPrefMapMode();
    // if the original size is in pixel, convert it to mm100
    if (rMapMode.GetMapUnit() == MapUnit::MapPixel)
        aOriginalSize = Application::GetDefaultDevice()->PixelToLogic(aOriginalSize,
                                                                      MapMode(MapUnit::Map100thMM));
    sal_Int32 nSizeX = 0;
    sal_Int32 nOffsetX = 0;
    if (GetProperty(rXPropSet, "FillBitmapSizeX"))
    {
        mAny >>= nSizeX;
        if (GetProperty(rXPropSet, "FillBitmapPositionOffsetX"))
        {
            sal_Int32 nX = (nSizeX != 0) ? nSizeX : aOriginalSize.Width();
            if (nX < 0 && rSize.Width > 0)
                nX = rSize.Width * std::abs(nX) / 100;
            nOffsetX = (*o3tl::doAccess<sal_Int32>(mAny)) * nX * 3.6;
        }

        // convert the X size of bitmap to a percentage
        if (nSizeX > 0)
            nSizeX = double(nSizeX) / aOriginalSize.Width() * 100000;
        else if (nSizeX < 0)
            nSizeX *= 1000;
        else
            nSizeX = 100000;
    }

    sal_Int32 nSizeY = 0;
    sal_Int32 nOffsetY = 0;
    if (GetProperty(rXPropSet, "FillBitmapSizeY"))
    {
        mAny >>= nSizeY;
        if (GetProperty(rXPropSet, "FillBitmapPositionOffsetY"))
        {
            sal_Int32 nY = (nSizeY != 0) ? nSizeY : aOriginalSize.Height();
            if (nY < 0 && rSize.Height > 0)
                nY = rSize.Height * std::abs(nY) / 100;
            nOffsetY = (*o3tl::doAccess<sal_Int32>(mAny)) * nY * 3.6;
        }

        // convert the Y size of bitmap to a percentage
        if (nSizeY > 0)
            nSizeY = double(nSizeY) / aOriginalSize.Height() * 100000;
        else if (nSizeY < 0)
            nSizeY *= 1000;
        else
            nSizeY = 100000;
    }

    // if the "Scale" setting is checked in the images settings dialog.
    if (nSizeX < 0 && nSizeY < 0)
    {
        if (rSize.Width != 0 && rSize.Height != 0)
        {
            nSizeX = rSize.Width / double(aOriginalSize.Width()) * std::abs(nSizeX);
            nSizeY = rSize.Height / double(aOriginalSize.Height()) * std::abs(nSizeY);
        }
        else
        {
            nSizeX = std::abs(nSizeX);
            nSizeY = std::abs(nSizeY);
        }
    }

    OUString sRectanglePoint;
    if (GetProperty(rXPropSet, "FillBitmapRectanglePoint"))
        sRectanglePoint = lclConvertRectanglePointToToken(*o3tl::doAccess<RectanglePoint>(mAny));

    mpFS->singleElementNS(XML_a, XML_tile, XML_tx, OUString::number(nOffsetX), XML_ty,
                          OUString::number(nOffsetY), XML_sx, OUString::number(nSizeX), XML_sy,
                          OUString::number(nSizeY), XML_algn, sRectanglePoint);
}

void DrawingML::WriteXGraphicCustomPosition(uno::Reference<beans::XPropertySet> const& rXPropSet,
                                            uno::Reference<graphic::XGraphic> const& rxGraphic,
                                            css::awt::Size const& rSize)
{
    Graphic aGraphic(rxGraphic);
    Size aOriginalSize(aGraphic.GetPrefSize());
    const MapMode& rMapMode = aGraphic.GetPrefMapMode();
    // if the original size is in pixel, convert it to mm100
    if (rMapMode.GetMapUnit() == MapUnit::MapPixel)
        aOriginalSize = Application::GetDefaultDevice()->PixelToLogic(aOriginalSize,
                                                                      MapMode(MapUnit::Map100thMM));
    double nSizeX = 0;
    if (GetProperty(rXPropSet, "FillBitmapSizeX"))
    {
        mAny >>= nSizeX;
        if (nSizeX <= 0)
        {
            if (nSizeX == 0)
                nSizeX = aOriginalSize.Width();
            else
                nSizeX /= 100; // percentage
        }
    }

    double nSizeY = 0;
    if (GetProperty(rXPropSet, "FillBitmapSizeY"))
    {
        mAny >>= nSizeY;
        if (nSizeY <= 0)
        {
            if (nSizeY == 0)
                nSizeY = aOriginalSize.Height();
            else
                nSizeY /= 100; // percentage
        }
    }

    if (nSizeX < 0 && nSizeY < 0 && rSize.Width != 0 && rSize.Height != 0)
    {
        nSizeX = rSize.Width * std::abs(nSizeX);
        nSizeY = rSize.Height * std::abs(nSizeY);
    }

    sal_Int32 nL = 0, nT = 0, nR = 0, nB = 0;
    if (GetProperty(rXPropSet, "FillBitmapRectanglePoint") && rSize.Width != 0 && rSize.Height != 0)
    {
        sal_Int32 nWidth = (1 - (nSizeX / rSize.Width)) * 100000;
        sal_Int32 nHeight = (1 - (nSizeY / rSize.Height)) * 100000;

        switch (*o3tl::doAccess<RectanglePoint>(mAny))
        {
            case RectanglePoint_LEFT_TOP:      nR = nWidth;          nB = nHeight;          break;
            case RectanglePoint_RIGHT_TOP:     nL = nWidth;          nB = nHeight;          break;
            case RectanglePoint_LEFT_BOTTOM:   nR = nWidth;          nT = nHeight;          break;
            case RectanglePoint_RIGHT_BOTTOM:  nL = nWidth;          nT = nHeight;          break;
            case RectanglePoint_LEFT_MIDDLE:   nR = nWidth;          nT = nB = nHeight / 2; break;
            case RectanglePoint_RIGHT_MIDDLE:  nL = nWidth;          nT = nB = nHeight / 2; break;
            case RectanglePoint_MIDDLE_TOP:    nB = nHeight;         nL = nR = nWidth / 2;  break;
            case RectanglePoint_MIDDLE_BOTTOM: nT = nHeight;         nL = nR = nWidth / 2;  break;
            case RectanglePoint_MIDDLE_MIDDLE: nL = nR = nWidth / 2; nT = nB = nHeight / 2; break;
            default: break;
        }
    }

    mpFS->startElementNS(XML_a, XML_stretch);

    mpFS->singleElementNS(XML_a, XML_fillRect, XML_l,
                          sax_fastparser::UseIf(OString::number(nL), nL != 0), XML_t,
                          sax_fastparser::UseIf(OString::number(nT), nT != 0), XML_r,
                          sax_fastparser::UseIf(OString::number(nR), nR != 0), XML_b,
                          sax_fastparser::UseIf(OString::number(nB), nB != 0));

    mpFS->endElementNS(XML_a, XML_stretch);
}

namespace
{
bool IsTopGroupObj(const uno::Reference<drawing::XShape>& xShape)
{
    SdrObject* pObject = SdrObject::getSdrObjectFromXShape(xShape);
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
    sal_Int32 nTop = rRect.Top();
    if (GetDocumentType() == DOCUMENT_DOCX && !m_xParent.is())
    {
        nLeft = 0;
        nTop = 0;
    }
    sal_Int32 nChildLeft = nLeft;
    sal_Int32 nChildTop = nTop;

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
        SdrObject* pShape = SdrObject::getSdrObjectFromXShape(rXShape);
        nRotation = pShape ? pShape->GetRotateAngle() : 0_deg100;
        if ( GetDocumentType() != DOCUMENT_DOCX )
        {
            int faccos=bFlipV ? -1 : 1;
            int facsin=bFlipH ? -1 : 1;
            aPos.X-=(1-faccos*cos(toRadians(nRotation)))*aSize.Width/2-facsin*sin(toRadians(nRotation))*aSize.Height/2;
            aPos.Y-=(1-faccos*cos(toRadians(nRotation)))*aSize.Height/2+facsin*sin(toRadians(nRotation))*aSize.Width/2;
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

        // As long as the support of MS Office 3D-features is rudimentary, we restore the original
        // values from InteropGrabBag. This affects images and custom shapes.
        if (xPropertySetInfo->hasPropertyByName(UNO_NAME_MISC_OBJ_INTEROPGRABBAG))
        {
            uno::Sequence<beans::PropertyValue> aGrabBagProps;
            xPropertySet->getPropertyValue(UNO_NAME_MISC_OBJ_INTEROPGRABBAG) >>= aGrabBagProps;
            auto p3DEffectProps = std::find_if(
                std::cbegin(aGrabBagProps), std::cend(aGrabBagProps),
                [](const PropertyValue& rProp) { return rProp.Name == "3DEffectProperties"; });
            if (p3DEffectProps != std::cend(aGrabBagProps))
            {
                uno::Sequence<beans::PropertyValue> a3DEffectProps;
                p3DEffectProps->Value >>= a3DEffectProps;
                // We have imported a scene3d.
                if (rXShape->getShapeType() == "com.sun.star.drawing.CustomShape")
                {
                    auto pMSORotation
                        = std::find_if(std::cbegin(aGrabBagProps), std::cend(aGrabBagProps),
                                       [](const PropertyValue& rProp) {
                                           return rProp.Name == "mso-rotation-angle";
                                       });
                    sal_Int32 nMSORotation = 0;
                    if (pMSORotation != std::cend(aGrabBagProps))
                        pMSORotation->Value >>= nMSORotation;
                    WriteTransformation(
                        rXShape,
                        tools::Rectangle(Point(aPos.X, aPos.Y), Size(aSize.Width, aSize.Height)),
                        nXmlNamespace, bFlipHWrite, bFlipVWrite, nMSORotation);
                    return;
                }
                if (rXShape->getShapeType() == "com.sun.star.drawing.GraphicObjectShape")
                {
                    // tdf#133037: restore original rotate angle of image before output
                    auto pCameraProps = std::find_if(
                        std::cbegin(a3DEffectProps), std::cend(a3DEffectProps),
                        [](const PropertyValue& rProp) { return rProp.Name == "Camera"; });
                    if (pCameraProps != std::cend(a3DEffectProps))
                    {
                        uno::Sequence<beans::PropertyValue> aCameraProps;
                        pCameraProps->Value >>= aCameraProps;
                        auto pZRotationProp = std::find_if(
                            std::cbegin(aCameraProps), std::cend(aCameraProps),
                            [](const PropertyValue& rProp) { return rProp.Name == "rotRev"; });
                        if (pZRotationProp != std::cend(aCameraProps))
                        {
                            sal_Int32 nTmp = 0;
                            pZRotationProp->Value >>= nTmp;
                            nCameraRotation = NormAngle36000(Degree100(nTmp / -600));
                            // FixMe tdf#160327. Vertical flip will be false.
                        }
                    }
                }
            }
        }
    } // end if (!bSuppressRotation)

    // OOXML flips shapes before rotating them.
    if(bFlipH != bFlipV)
        nRotation = 36000_deg100 - nRotation;

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
    if (sTarget.isEmpty())
    {
        sal_Int32 nSplit = rURL.lastIndexOf(' ');
        if (nSplit > -1)
            sTarget = OUString::Concat("slide") + rURL.subView(nSplit + 1) + ".xml";
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
    sal_Int32 nCharEscapementHeight = 0;

    if ( nElement == XML_endParaRPr && rbOverridingCharHeight )
    {
        nSize = rnCharHeight;
    }
    else if (GetProperty(rXPropSet, "CharHeight"))
    {
        nSize = static_cast<sal_Int32>(100*(*o3tl::doAccess<float>(mAny)));
        if ( nElement == XML_rPr || nElement == XML_defRPr )
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

    if (GetPropertyAndState(rXPropSet, rXPropState, "CharEscapementHeight", eState)
        && eState == beans::PropertyState_DIRECT_VALUE)
        mAny >>= nCharEscapementHeight;

    if (DFLT_ESC_AUTO_SUPER == nCharEscapement)
    {
        // Raised by the differences between the ascenders (ascent = baseline to top of highest letter).
        // The ascent is generally about 80% of the total font height.
        // That is why DFLT_ESC_PROP (58) leads to 33% (DFLT_ESC_SUPER)
        nCharEscapement = .8 * (100 - nCharEscapementHeight);
    }
    else if (DFLT_ESC_AUTO_SUB == nCharEscapement)
    {
        // Lowered by the differences between the descenders (descent = baseline to bottom of lowest letter).
        // The descent is generally about 20% of the total font height.
        // That is why DFLT_ESC_PROP (58) leads to 8% (DFLT_ESC_SUB)
        nCharEscapement = .2 * -(100 - nCharEscapementHeight);
    }

    if (nCharEscapement && nCharEscapementHeight)
    {
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

            bool bContoured = false;
            if (GetProperty(rXPropSet, "CharContoured"))
                bContoured = *o3tl::doAccess<bool>(mAny);

            // tdf#127696 If the CharContoured is true, then the text color is white and the outline color is the CharColor.
            if (bContoured)
            {
                mpFS->startElementNS(XML_a, XML_ln);
                if (color == COL_AUTO)
                {
                    mbIsBackgroundDark ? WriteSolidFill(COL_WHITE) : WriteSolidFill(COL_BLACK);
                }
                else
                {
                    color.SetAlpha(255);
                    if (!WriteSchemeColor(u"CharComplexColor"_ustr, rXPropSet))
                        WriteSolidFill(color, nTransparency);
                }
                mpFS->endElementNS(XML_a, XML_ln);

                WriteSolidFill(COL_WHITE);
            }
            // tdf#104219 In LibreOffice and MS Office, there are two types of colors:
            // Automatic and Fixed. OOXML is setting automatic color, by not providing color.
            else if( color != COL_AUTO )
            {
                color.SetAlpha(255);
                // TODO: special handle embossed/engraved
                if (!WriteSchemeColor(u"CharComplexColor"_ustr, rXPropSet))
                {
                    WriteSolidFill(color, nTransparency);
                }
            }
            else if (GetDocumentType() == DOCUMENT_PPTX)
            {
                // Resolve COL_AUTO for PPTX since MS Powerpoint doesn't have automatic colors.
                bool bIsTextBackgroundDark = mbIsBackgroundDark;
                if (rXShapePropSet.is() && GetProperty(rXShapePropSet, "FillStyle")
                    && mAny.get<FillStyle>() != FillStyle_NONE
                    && GetProperty(rXShapePropSet, "FillColor"))
                {
                    ::Color aShapeFillColor(ColorTransparency, mAny.get<sal_uInt32>());
                    bIsTextBackgroundDark = aShapeFillColor.IsDark();
                }

                if (bIsTextBackgroundDark)
                    WriteSolidFill(COL_WHITE);
                else
                    WriteSolidFill(COL_BLACK);
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
                std::u16string_view aDestination(sURL.subView(nIndex + 1));
                mpFS->singleElementNS(XML_a, XML_hlinkClick, FSNS(XML_r, XML_id), "", XML_action,
                                      OUString::Concat("ppaction://hlinkshowjump?jump=") + aDestination);
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
                    aFieldValue = GetDatetimeTypeFromDate(static_cast<SvxDateFormat>(nNumFmt));
                }
                else if(aFieldKind == "ExtTime")
                {
                    sal_Int32 nNumFmt = -1;
                    rXPropSet->getPropertyValue(UNO_TC_PROP_NUMFORMAT) >>= nNumFmt;
                    aFieldValue = GetDatetimeTypeFromTime(static_cast<SvxTimeFormat>(nNumFmt));
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

OUString DrawingML::GetDatetimeTypeFromDate(SvxDateFormat eDate)
{
    return GetDatetimeTypeFromDateTime(eDate, SvxTimeFormat::AppDefault);
}

OUString DrawingML::GetDatetimeTypeFromTime(SvxTimeFormat eTime)
{
    return GetDatetimeTypeFromDateTime(SvxDateFormat::AppDefault, eTime);
}

OUString DrawingML::GetDatetimeTypeFromDateTime(SvxDateFormat eDate, SvxTimeFormat eTime)
{
    OUString aDateField;
    switch (eDate)
    {
        case SvxDateFormat::StdSmall:
        case SvxDateFormat::A:
            aDateField = "datetime";
            break;
        case SvxDateFormat::B:
            aDateField = "datetime1"; // 13/02/1996
            break;
        case SvxDateFormat::C:
            aDateField = "datetime5";
            break;
        case SvxDateFormat::D:
            aDateField = "datetime3"; // 13 February 1996
            break;
        case SvxDateFormat::StdBig:
        case SvxDateFormat::E:
        case SvxDateFormat::F:
            aDateField = "datetime2";
            break;
        default:
            break;
    }

    OUString aTimeField;
    switch (eTime)
    {
        case SvxTimeFormat::Standard:
        case SvxTimeFormat::HH24_MM_SS:
        case SvxTimeFormat::HH24_MM_SS_00:
            aTimeField = "datetime11"; // 13:49:38
            break;
        case SvxTimeFormat::HH24_MM:
            aTimeField = "datetime10"; // 13:49
            break;
        case SvxTimeFormat::HH12_MM:
        case SvxTimeFormat::HH12_MM_AMPM:
            aTimeField = "datetime12"; // 01:49 PM
            break;
        case SvxTimeFormat::HH12_MM_SS:
        case SvxTimeFormat::HH12_MM_SS_AMPM:
        case SvxTimeFormat::HH12_MM_SS_00:
        case SvxTimeFormat::HH12_MM_SS_00_AMPM:
            aTimeField = "datetime13"; // 01:49:38 PM
            break;
        default:
            break;
    }

    if (!aDateField.isEmpty() && aTimeField.isEmpty())
        return aDateField;
    else if (!aTimeField.isEmpty() && aDateField.isEmpty())
        return aTimeField;
    else if (!aDateField.isEmpty() && !aTimeField.isEmpty())
    {
        if (aTimeField == "datetime11" || aTimeField == "datetime13")
            // only datetime format that has Date and HH:MM:SS
            return "datetime9"; // dd/mm/yyyy H:MM:SS
        else
            // only datetime format that has Date and HH:MM
            return "datetime8"; // dd/mm/yyyy H:MM
    }
    else
        return "";
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

    float nFontSize = -1;
    if (GetProperty(rXPropSet, "CharHeight"))
        mAny >>= nFontSize;

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
        // Empty run? Do not forget to write the font size in case of pptx:
        if ((GetDocumentType() == DOCUMENT_PPTX) && (nFontSize != -1))
        {
            mpFS->startElementNS(XML_a, XML_br);
            mpFS->singleElementNS(XML_a, XML_rPr, XML_sz,
                                  OString::number(nFontSize * 100));
            mpFS->endElementNS(XML_a, XML_br);
        }
        else
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
        return;

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

    for (const PropertyValue& rPropValue : aPropertySequence)
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
            aDestBitmap.CopyPixel(aDestRect, aSourceRect, aSourceBitmap);
            Graphic aDestGraphic(aDestBitmap);
            sRelationId = writeGraphicToStorage(aDestGraphic);
            fBulletSizeRel = 1.0f;
        }
        else
        {
            sRelationId = writeGraphicToStorage(aGraphic);
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

    for (const css::style::TabStop& rTabStop : aTabStops)
    {
        OString sPosition = OString::number(GetPointFromCoordinate(rTabStop.Position));
        OString sAlignment;
        switch (rTabStop.Alignment)
        {
            case css::style::TabAlign_DECIMAL:
                sAlignment = "dec"_ostr;
                break;
            case css::style::TabAlign_RIGHT:
                sAlignment = "r"_ostr;
                break;
            case css::style::TabAlign_CENTER:
                sAlignment = "ctr"_ostr;
                break;
            case css::style::TabAlign_LEFT:
            default:
                sAlignment = "l"_ostr;
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

    for (const PropertyValue& rPropValue : aPropertySequence)
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

void DrawingML::WriteLinespacing(const LineSpacing& rSpacing, float fFirstCharHeight)
{
    if( rSpacing.Mode == LineSpacingMode::PROP )
    {
        mpFS->singleElementNS( XML_a, XML_spcPct,
                               XML_val, OString::number(static_cast<sal_Int32>(rSpacing.Height)*1000));
    }
    else if (rSpacing.Mode == LineSpacingMode::MINIMUM
             && fFirstCharHeight > static_cast<float>(rSpacing.Height) * 0.001 * 72.0 / 2.54)
    {
        // 100% proportional line spacing = single line spacing
        mpFS->singleElementNS(XML_a, XML_spcPct, XML_val,
                              OString::number(static_cast<sal_Int32>(100000)));
    }
    else
    {
        mpFS->singleElementNS( XML_a, XML_spcPts,
                               XML_val, OString::number(std::lround(rSpacing.Height / 25.4 * 72)));
    }
}

bool DrawingML::WriteParagraphProperties(const Reference<XTextContent>& rParagraph, float fFirstCharHeight, sal_Int32 nElement)
{
    Reference< XPropertySet > rXPropSet( rParagraph, UNO_QUERY );
    Reference< XPropertyState > rXPropState( rParagraph, UNO_QUERY );
    PropertyState eState;

    if( !rXPropSet.is() || !rXPropState.is() )
        return false;

    sal_Int16 nLevel = -1;
    if (GetProperty(rXPropSet, "NumberingLevel"))
        mAny >>= nLevel;

    bool bWriteNumbering = true;
    bool bForceZeroIndent = false;
    if (mbPlaceholder)
    {
        Reference< text::XTextRange > xParaText(rParagraph, UNO_QUERY);
        if (xParaText)
        {
            bool bNumberingOnThisLevel = false;
            if (nLevel > -1)
            {
                Reference< XIndexAccess > xNumberingRules(rXPropSet->getPropertyValue("NumberingRules"), UNO_QUERY);
                const PropertyValues& rNumRuleOfLevel = xNumberingRules->getByIndex(nLevel).get<PropertyValues>();
                for (const PropertyValue& rRule : rNumRuleOfLevel)
                    if (rRule.Name == "NumberingType" && rRule.Value.hasValue())
                        bNumberingOnThisLevel = rRule.Value.get<sal_uInt16>() != style::NumberingType::NUMBER_NONE;
            }

            const bool bIsNumberingVisible = rXPropSet->getPropertyValue("NumberingIsNumber").get<bool>();
            const bool bIsLineEmpty = !xParaText->getString().getLength();

            bWriteNumbering = !bIsLineEmpty && bIsNumberingVisible && (nLevel != -1);
            bForceZeroIndent = (!bIsNumberingVisible || bIsLineEmpty || !bNumberingOnThisLevel);
        }

    }

    sal_Int16 nTmp = sal_Int16(style::ParagraphAdjust_LEFT);
    if (GetProperty(rXPropSet, "ParaAdjust"))
        mAny >>= nTmp;
    style::ParagraphAdjust nAlignment = static_cast<style::ParagraphAdjust>(nTmp);

    bool bHasLinespacing = false;
    LineSpacing aLineSpacing;
    if (GetPropertyAndState(rXPropSet, rXPropState, "ParaLineSpacing", eState)
        && (mAny >>= aLineSpacing)
        && (eState == beans::PropertyState_DIRECT_VALUE ||
            // only export if it differs from the default 100% line spacing
            aLineSpacing.Mode != LineSpacingMode::PROP || aLineSpacing.Height != 100))
        bHasLinespacing = true;

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

    if (bWriteNumbering && !bForceZeroIndent)
    {
        if (!(nLevel != -1
            || nAlignment != style::ParagraphAdjust_LEFT
            || bHasLinespacing))
            return false;
    }

    sal_Int32 nParaDefaultTabSize = 0;
    if (GetProperty(rXPropSet, "ParaTabStopDefaultDistance"))
        mAny >>= nParaDefaultTabSize;

    if (nParaLeftMargin) // For Paragraph
        mpFS->startElementNS( XML_a, nElement,
                           XML_lvl, sax_fastparser::UseIf(OString::number(nLevel), nLevel > 0),
                           XML_marL, sax_fastparser::UseIf(OString::number(oox::drawingml::convertHmmToEmu(nParaLeftMargin)), nParaLeftMargin > 0),
                           XML_indent, sax_fastparser::UseIf(OString::number((bForceZeroIndent && nParaFirstLineIndent == 0) ? 0 : oox::drawingml::convertHmmToEmu(nParaFirstLineIndent)), (bForceZeroIndent || nParaFirstLineIndent != 0)),
                           XML_algn, GetAlignment( nAlignment ),
                           XML_defTabSz, sax_fastparser::UseIf(OString::number(oox::drawingml::convertHmmToEmu(nParaDefaultTabSize)), nParaDefaultTabSize > 0),
                           XML_rtl, sax_fastparser::UseIf(ToPsz10(bRtl), bRtl));
    else
        mpFS->startElementNS( XML_a, nElement,
                           XML_lvl, sax_fastparser::UseIf(OString::number(nLevel), nLevel > 0),
                           XML_marL, sax_fastparser::UseIf(OString::number(oox::drawingml::convertHmmToEmu(nLeftMargin)), nLeftMargin > 0),
                           XML_indent, sax_fastparser::UseIf(OString::number(!bForceZeroIndent ? oox::drawingml::convertHmmToEmu(nLineIndentation) : 0), (bForceZeroIndent || ( nLineIndentation != 0))),
                           XML_algn, GetAlignment( nAlignment ),
                           XML_defTabSz, sax_fastparser::UseIf(OString::number(oox::drawingml::convertHmmToEmu(nParaDefaultTabSize)), nParaDefaultTabSize > 0),
                           XML_rtl, sax_fastparser::UseIf(ToPsz10(bRtl), bRtl));


    if( bHasLinespacing )
    {
        mpFS->startElementNS(XML_a, XML_lnSpc);
        WriteLinespacing(aLineSpacing, fFirstCharHeight);
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

    if (!bWriteNumbering)
        mpFS->singleElementNS(XML_a, XML_buNone);
    else
        WriteParagraphNumbering( rXPropSet, fFirstCharHeight, nLevel );

    WriteParagraphTabStops( rXPropSet );

    // do not end element for lstStyles since, defRPr should be stacked inside it
    if( nElement != XML_lvl1pPr )
        mpFS->endElementNS( XML_a, nElement );

    return true;
}

void DrawingML::WriteLstStyles(const css::uno::Reference<css::text::XTextContent>& rParagraph,
                               bool& rbOverridingCharHeight, sal_Int32& rnCharHeight,
                               const css::uno::Reference<css::beans::XPropertySet>& rXShapePropSet)
{
    Reference<XEnumerationAccess> xAccess(rParagraph, UNO_QUERY);
    if (!xAccess.is())
        return;

    Reference<XEnumeration> xEnumeration(xAccess->createEnumeration());
    if (!xEnumeration.is())
        return;


    Reference<XTextRange> rRun;

    if (!xEnumeration->hasMoreElements())
        return;

    Any aAny(xEnumeration->nextElement());
    if (aAny >>= rRun)
    {
        float fFirstCharHeight = rnCharHeight / 1000.;
        Reference<XPropertySet> xFirstRunPropSet(rRun, UNO_QUERY);
        Reference<XPropertySetInfo> xFirstRunPropSetInfo
            = xFirstRunPropSet->getPropertySetInfo();

        if (xFirstRunPropSetInfo->hasPropertyByName("CharHeight"))
            fFirstCharHeight = xFirstRunPropSet->getPropertyValue("CharHeight").get<float>();

        mpFS->startElementNS(XML_a, XML_lstStyle);
        if( !WriteParagraphProperties(rParagraph, fFirstCharHeight, XML_lvl1pPr) )
            mpFS->startElementNS(XML_a, XML_lvl1pPr);
        WriteRunProperties(xFirstRunPropSet, false, XML_defRPr, true, rbOverridingCharHeight,
                           rnCharHeight, GetScriptType(rRun->getString()), rXShapePropSet);
        mpFS->endElementNS(XML_a, XML_lvl1pPr);
        mpFS->endElementNS(XML_a, XML_lstStyle);
    }
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
                {
                    fFirstCharHeight = xFirstRunPropSet->getPropertyValue("CharHeight").get<float>();
                    rnCharHeight = 100 * fFirstCharHeight;
                    rbOverridingCharHeight = true;
                }
                WriteParagraphProperties(rParagraph, fFirstCharHeight, XML_pPr);
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
            for (const auto& rProp : aCustomShapeGeometryProps)
            {
                if (rProp.Name == "TextPath")
                {
                    rProp.Value >>= aTextPathSeq;
                    for (const auto& rTextPathItem : aTextPathSeq)
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
                          sal_Int32 nXmlNamespace, bool bWritePropertiesAsLstStyles)
{
    // ToDo: Fontwork in DOCX
    uno::Reference<XText> xXText(rXIface, UNO_QUERY);
    if( !xXText.is() )
        return;

    uno::Reference<drawing::XShape> xShape(rXIface, UNO_QUERY);
    uno::Reference<XPropertySet> rXPropSet(rXIface, UNO_QUERY);

    constexpr const sal_Int32 constDefaultLeftRightInset = 254;
    constexpr const sal_Int32 constDefaultTopBottomInset = 127;
    sal_Int32 nLeft = constDefaultLeftRightInset;
    sal_Int32 nRight = constDefaultLeftRightInset;
    sal_Int32 nTop = constDefaultTopBottomInset;
    sal_Int32 nBottom = constDefaultTopBottomInset;

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

    // Transform the text distance values so they are compatible with OOXML insets
    if (xShape.is())
    {
        sal_Int32 nTextHeight = xShape->getSize().Height; // Hmm, default

        // CustomShape can have text area different from shape rectangle
        auto* pCustomShape
            = dynamic_cast<SdrObjCustomShape*>(SdrObject::getSdrObjectFromXShape(xShape));
        if (pCustomShape)
        {
            const EnhancedCustomShape2d aCustomShape2d(*pCustomShape);
            nTextHeight = aCustomShape2d.GetTextRect().getOpenHeight();
            if (DOCUMENT_DOCX == meDocumentType)
                nTextHeight = convertTwipToMm100(nTextHeight);
        }

        if (nTop + nBottom >= nTextHeight)
        {
            // Effective bottom would be above effective top of text area. LO normalizes the
            // effective text area in such case implicitly for rendering. MS needs indents so that
            // the result is the normalized effective text area.
            std::swap(nTop, nBottom);
            nTop = nTextHeight - nTop;
            nBottom = nTextHeight - nBottom;
        }
    }

    std::optional<OString> sWritingMode;
    if (GetProperty(rXPropSet, "TextWritingMode"))
    {
        WritingMode eMode;
        if( ( mAny >>= eMode ) && eMode == WritingMode_TB_RL )
            sWritingMode = "eaVert";
    }
    if (GetProperty(rXPropSet, "WritingMode"))
    {
        sal_Int16 nWritingMode;
        if (mAny >>= nWritingMode)
        {
            if (nWritingMode == text::WritingMode2::TB_RL)
                sWritingMode = "eaVert";
            else if (nWritingMode == text::WritingMode2::BT_LR)
                sWritingMode = "vert270";
            else if (nWritingMode == text::WritingMode2::TB_RL90)
                sWritingMode = "vert";
            else if (nWritingMode == text::WritingMode2::TB_LR)
                sWritingMode = "mongolianVert";
            else if (nWritingMode == text::WritingMode2::STACKED)
                sWritingMode = "wordArtVert";
        }
    }

    // read values from CustomShapeGeometry
    Sequence<drawing::EnhancedCustomShapeAdjustmentValue> aAdjustmentSeq;
    uno::Sequence<beans::PropertyValue> aTextPathSeq;
    bool bScaleX(false);
    OUString sShapeType("non-primitive");
    OUString sMSWordPresetTextWarp;
    sal_Int32 nTextPreRotateAngle = 0; // degree
    std::optional<Degree100> nTextRotateAngleDeg100; // text area rotation

    if (GetProperty(rXPropSet, "CustomShapeGeometry"))
    {
        Sequence< PropertyValue > aProps;
        if ( mAny >>= aProps )
        {
            for (const auto& rProp : aProps)
            {
                if (rProp.Name == "TextPreRotateAngle")
                    rProp.Value >>= nTextPreRotateAngle;
                else if (rProp.Name == "AdjustmentValues")
                    rProp.Value >>= aAdjustmentSeq;
                else if (rProp.Name == "TextRotateAngle")
                {
                    double fTextRotateAngle = 0; // degree
                    rProp.Value >>= fTextRotateAngle;
                    nTextRotateAngleDeg100 = Degree100(std::lround(fTextRotateAngle * 100.0));
                }
                else if (rProp.Name == "Type")
                    rProp.Value >>= sShapeType;
                else if (rProp.Name == "TextPath")
                {
                    rProp.Value >>= aTextPathSeq;
                    for (const auto& rTextPathItem : aTextPathSeq)
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
                            sWritingMode = "eaVert";
                            break;
                        case WritingMode2::BT_LR:
                            sWritingMode = "vert270";
                            break;
                        case WritingMode2::TB_RL90:
                            sWritingMode = "vert";
                            break;
                        case WritingMode2::TB_LR:
                            sWritingMode = "mongolianVert";
                            break;
                        default:
                            break;
                        }
                    }
                }
            }
        }
    }

    // read InteropGrabBag if any
    std::optional<OUString> sHorzOverflow;
    std::optional<OUString> sVertOverflow;
    bool bUpright = false;
    std::optional<OString> isUpright;
    if (rXPropSet->getPropertySetInfo()->hasPropertyByName("InteropGrabBag"))
    {
        uno::Sequence<beans::PropertyValue> aGrabBag;
        rXPropSet->getPropertyValue("InteropGrabBag") >>= aGrabBag;
        for (const auto& aProp : aGrabBag)
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
    }

    bool bIsFontworkShape(IsFontworkShape(rXPropSet));
    OUString sPresetWarp(PresetGeometryTypeNames::GetMsoName(sShapeType));
    // ODF may have user defined TextPath, use "textPlain" as ersatz.
    if (sPresetWarp.isEmpty())
        sPresetWarp = bIsFontworkShape ? std::u16string_view(u"textPlain") : std::u16string_view(u"textNoShape");

    bool bFromWordArt = !bScaleX
                        && ( sPresetWarp == "textArchDown" || sPresetWarp == "textArchUp"
                            || sPresetWarp == "textButton" || sPresetWarp == "textCircle");

    // Fontwork shapes in LO ignore insets in rendering, Word interprets them.
    if (GetDocumentType() == DOCUMENT_DOCX && bIsFontworkShape)
    {
        nLeft = 0;
        nRight = 0;
        nTop = 0;
        nBottom = 0;
    }

    if (bUpright)
    {
        Degree100 nShapeRotateAngleDeg100(0_deg100);
        if (GetProperty(rXPropSet, "RotateAngle"))
            nShapeRotateAngleDeg100 = Degree100(mAny.get<sal_Int32>());
        // Depending on shape rotation, the import has made 90deg changes to properties
        // "TextPreRotateAngle" and "TextRotateAngle". Revert it.
        bool bWasAngleChanged
            = (nShapeRotateAngleDeg100 > 4500_deg100 && nShapeRotateAngleDeg100 <= 13500_deg100)
              || (nShapeRotateAngleDeg100 > 22500_deg100
                  && nShapeRotateAngleDeg100 <= 31500_deg100);
        if (bWasAngleChanged)
        {
            nTextRotateAngleDeg100 = nTextRotateAngleDeg100.value_or(0_deg100) + 9000_deg100;
            nTextPreRotateAngle -= 90;
        }
        // If text is no longer upright, user has changed something. Do not write 'upright' then.
        // This try to detect the case assumes, that the text area rotation was 0 in the original
        // MS Office document. That is likely because MS Office has no UI to set it and the
        // predefined SmartArt shapes, which use it, do not use 'upright'.
        Degree100 nAngleSum = nShapeRotateAngleDeg100 + nTextRotateAngleDeg100.value_or(0_deg100);
        if (abs(NormAngle18000(nAngleSum)) < 100_deg100) // consider inaccuracy from rounding
        {
            nTextRotateAngleDeg100.reset(); // 'upright' does not overrule text area rotation.
        }
        else
        {
            // User changes. Keep current angles.
            isUpright.reset();
            if (bWasAngleChanged)
            {
                nTextPreRotateAngle += 90;
                nTextRotateAngleDeg100 = nTextRotateAngleDeg100.value_or(0_deg100) - 9000_deg100;
            }
        }
    }

    // ToDo: Unsure about this. Need to investigate shapes from diagram import, especially diagrams
    // with vertical text directions.
    if (nTextPreRotateAngle != 0 && !sWritingMode)
    {
        if (nTextPreRotateAngle == -90 || nTextPreRotateAngle == 270)
            sWritingMode = "vert";
        else if (nTextPreRotateAngle == -270 || nTextPreRotateAngle == 90)
            sWritingMode = "vert270";
        else if (nTextPreRotateAngle == -180 || nTextPreRotateAngle == 180)
        {
#if defined __GNUC__ && !defined __clang__ && __GNUC__ == 12
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
            nTextRotateAngleDeg100
                = NormAngle18000(nTextRotateAngleDeg100.value_or(0_deg100) + 18000_deg100);
#if defined __GNUC__ && !defined __clang__ && __GNUC__ == 12
#pragma GCC diagnostic pop
#endif
            // ToDo: Examine insets. They might need rotation too. Check diagrams (SmartArt).
        }
        else
            SAL_WARN("oox", "unsuitable value for TextPreRotateAngle:" << nTextPreRotateAngle);
    }
    else if (nTextPreRotateAngle != 0 && sWritingMode && sWritingMode.value() == "eaVert")
    {
        // ToDo: eaVert plus 270deg clockwise rotation has to be written with vert="horz"
        // plus attribute 'normalEastAsianFlow="1"' on the <wps:wsp> element.
    }
    // else nothing to do

    // Our WritingMode introduces text pre rotation which includes padding, MSO vert does not include
    // padding. Therefore set padding so, that is looks the same in MSO as in LO.
    if (sWritingMode)
    {
        if (sWritingMode.value() == "vert" || sWritingMode.value() == "eaVert")
        {
            sal_Int32 nHelp = nLeft;
            nLeft = nBottom;
            nBottom = nRight;
            nRight = nTop;
            nTop = nHelp;
        }
        else if (sWritingMode.value() == "vert270")
        {
            sal_Int32 nHelp = nLeft;
            nLeft = nTop;
            nTop = nRight;
            nRight = nBottom;
            nBottom = nHelp;
        }
        else if (sWritingMode.value() == "mongolianVert")
        {
            // ToDo: Examine padding
        }
    }


    std::optional<OString> sTextRotateAngleMSUnit;
    if (nTextRotateAngleDeg100.has_value())
#if defined __GNUC__ && !defined __clang__ && __GNUC__ == 12
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
        sTextRotateAngleMSUnit
            = oox::drawingml::calcRotationValue(nTextRotateAngleDeg100.value().get());
#if defined __GNUC__ && !defined __clang__ && __GNUC__ == 12
#pragma GCC diagnostic pop
#endif

    // Prepare attributes 'anchor' and 'anchorCtr'
    // LibreOffice has 12 value sets, MS Office only 6. We map them so, that it reverses the
    // 6 mappings from import, and we assign the others approximately.
    TextVerticalAdjust eVerticalAlignment(TextVerticalAdjust_TOP);
    if (GetProperty(rXPropSet, "TextVerticalAdjust"))
        mAny >>= eVerticalAlignment;
    TextHorizontalAdjust eHorizontalAlignment(TextHorizontalAdjust_CENTER);
    if (GetProperty(rXPropSet, "TextHorizontalAdjust"))
        mAny >>= eHorizontalAlignment;

    const char* sAnchor = nullptr;
    bool bAnchorCtr = false;
    if (sWritingMode.has_value()
        && (sWritingMode.value() == "eaVert" || sWritingMode.value() == "mongolianVert"))
    {
        bAnchorCtr = eVerticalAlignment == TextVerticalAdjust_CENTER
                     || eVerticalAlignment == TextVerticalAdjust_BOTTOM
                     || eVerticalAlignment == TextVerticalAdjust_BLOCK;
        switch (eHorizontalAlignment)
        {
            case TextHorizontalAdjust_CENTER:
                sAnchor = "ctr";
                break;
            case TextHorizontalAdjust_LEFT:
                sAnchor = sWritingMode.value() == "eaVert" ? "b" : "t";
                break;
            case TextHorizontalAdjust_RIGHT:
            default: // TextHorizontalAdjust_BLOCK, should not happen
                sAnchor = sWritingMode.value() == "eaVert" ? "t" : "b";
                break;
        }
    }
    else
    {
        bAnchorCtr = eHorizontalAlignment == TextHorizontalAdjust_CENTER
                     || eHorizontalAlignment == TextHorizontalAdjust_RIGHT;
        sAnchor = GetTextVerticalAdjust(eVerticalAlignment);
    }

    bool bHasWrap = false;
    bool bWrap = false;
    // Only custom shapes obey the TextWordWrap option, normal text always wraps.
    if (dynamic_cast<SvxCustomShape*>(rXIface.get()) && GetProperty(rXPropSet, "TextWordWrap"))
    {
        mAny >>= bWrap;
        bHasWrap = true;
    }

    // tdf#134401: If AUTOGROWWIDTH and AUTOGROWHEIGHT are set, then export it as TextWordWrap
    if (SvxShapeText* pShpTxt = dynamic_cast<SvxShapeText*>(rXIface.get()))
    {
        const sdr::properties::BaseProperties& rProperties
            = pShpTxt->GetSdrObject()->GetProperties();

        const SdrOnOffItem& rSdrTextFitWidth = rProperties.GetItem(SDRATTR_TEXT_AUTOGROWWIDTH);
        const SdrOnOffItem& rSdrTextFitHeight = rProperties.GetItem(SDRATTR_TEXT_AUTOGROWHEIGHT);

        if (rSdrTextFitWidth.GetValue() == true && rSdrTextFitHeight.GetValue() == true)
        {
            bHasWrap = true;
            bWrap = false;
        }
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
            if ((xServiceInfo.is() && xServiceInfo->supportsService("com.sun.star.drawing.TextShape"))
                || bIsFontworkShape)
                pWrap = "square";
        }

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

        if (!sVertOverflow && GetProperty(rXPropSet, "TextClipVerticalOverflow") && mAny.get<bool>())
        {
            sVertOverflow = "clip";
        }

        // tdf#151134 When writing placeholder shapes, inset must be explicitly specified
        bool bRequireInset = GetProperty(rXPropSet, "IsPresentationObject") && rXPropSet->getPropertyValue("IsPresentationObject").get<bool>();

        mpFS->startElementNS( (nXmlNamespace ? nXmlNamespace : XML_a), XML_bodyPr,
                               XML_numCol, sax_fastparser::UseIf(OString::number(nCols), nCols > 0),
                               XML_spcCol, sax_fastparser::UseIf(OString::number(oox::drawingml::convertHmmToEmu(nColSpacing)), nCols > 0 && nColSpacing >= 0),
                               XML_wrap, pWrap,
                               XML_horzOverflow, sHorzOverflow,
                               XML_vertOverflow, sVertOverflow,
                               XML_fromWordArt, sax_fastparser::UseIf("1", bFromWordArt),
                               XML_lIns, sax_fastparser::UseIf(OString::number(oox::drawingml::convertHmmToEmu(nLeft)),
                                                               bRequireInset || nLeft != constDefaultLeftRightInset),
                               XML_rIns, sax_fastparser::UseIf(OString::number(oox::drawingml::convertHmmToEmu(nRight)),
                                                               bRequireInset || nRight != constDefaultLeftRightInset),
                               XML_tIns, sax_fastparser::UseIf(OString::number(oox::drawingml::convertHmmToEmu(nTop)),
                                                               bRequireInset || nTop != constDefaultTopBottomInset),
                               XML_bIns, sax_fastparser::UseIf(OString::number(oox::drawingml::convertHmmToEmu(nBottom)),
                                                               bRequireInset || nBottom != constDefaultTopBottomInset),
                               XML_anchor, sAnchor,
                               XML_anchorCtr, sax_fastparser::UseIf("1", bAnchorCtr),
                               XML_vert, sWritingMode,
                               XML_upright, isUpright,
                               XML_rot, sTextRotateAngleMSUnit);

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
            auto pSdrObjCustomShape = xShape.is() ? dynamic_cast<SdrObjCustomShape*>(SdrObject::getSdrObjectFromXShape(xShape)) : nullptr;
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
                sal_Int32 nSpacingReduction = 0;
                SvxShapeText* pTextShape = dynamic_cast<SvxShapeText*>(rXIface.get());
                if (pTextShape)
                {
                    SdrTextObj* pTextObject = DynCastSdrTextObj(pTextShape->GetSdrObject());
                    if (pTextObject)
                    {
                        nFontScale = sal_Int32(pTextObject->GetFontScale() * 1000.0);
                        nSpacingReduction = sal_Int32((100.0 - pTextObject->GetSpacingScale()) * 1000.0);
                    }
                }

                bool bExportFontScale = false;
                if (nFontScale < MAX_SCALE_VAL && nFontScale > 0)
                    bExportFontScale = true;

                bool bExportSpaceReduction = false;
                if (nSpacingReduction < MAX_SCALE_VAL && nSpacingReduction > 0)
                    bExportSpaceReduction = true;

                mpFS->singleElementNS(XML_a, XML_normAutofit,
                    XML_fontScale, sax_fastparser::UseIf(OString::number(nFontScale), bExportFontScale),
                    XML_lnSpcReduction, sax_fastparser::UseIf(OString::number(nSpacingReduction), bExportSpaceReduction));
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

        Write3DEffects( rXPropSet, /*bIsText=*/true );

        mpFS->endElementNS((nXmlNamespace ? nXmlNamespace : XML_a), XML_bodyPr);
    }

    Reference< XEnumerationAccess > access( xXText, UNO_QUERY );
    if( !access.is() || !bText )
        return;

    Reference< XEnumeration > enumeration( access->createEnumeration() );
    if( !enumeration.is() )
        return;

    SdrObject* pSdrObject = xShape.is() ? SdrObject::getSdrObjectFromXShape(xShape) : nullptr;
    const SdrTextObj* pTxtObj = DynCastSdrTextObj( pSdrObject );
    if (pTxtObj && mpTextExport)
    {
        std::vector<beans::PropertyValue> aOldCharFillPropVec;
        if (bIsFontworkShape)
        {
            // Users may have set the character fill properties for more convenient editing.
            // Save the properties before changing them for Fontwork export.
            FontworkHelpers::collectCharColorProps(xXText, aOldCharFillPropVec);
            // Word has properties for abc-transform in the run properties of the text of the shape.
            // Writer has the Fontwork properties as shape properties. Create the character fill
            // properties needed for export from the shape fill properties
            // and apply them to all runs.
            std::vector<beans::PropertyValue> aExportCharFillPropVec;
            FontworkHelpers::createCharFillPropsFromShape(rXPropSet, aExportCharFillPropVec);
            FontworkHelpers::applyPropsToRuns(aExportCharFillPropVec, xXText);
            // Import has converted some items from CharInteropGrabBag to fill and line
            // properties of the shape. For export we convert them back because users might have
            // changed them. And we create them in case we come from an odt document.
            std::vector<beans::PropertyValue> aUpdatePropVec;
            FontworkHelpers::createCharInteropGrabBagUpdatesFromShapeProps(rXPropSet, aUpdatePropVec);
            FontworkHelpers::applyUpdatesToCharInteropGrabBag(aUpdatePropVec, xXText);
        }

        std::optional<OutlinerParaObject> pParaObj;

        /*
        #i13885#
        When the object is actively being edited, that text is not set into
        the objects normal text object, but lives in a separate object.
        */
        if (pTxtObj->IsTextEditActive())
        {
            pParaObj = pTxtObj->CreateEditOutlinerParaObject();
        }
        else if (pTxtObj->GetOutlinerParaObject())
            pParaObj = *pTxtObj->GetOutlinerParaObject();

        if (pParaObj)
        {
            // this is reached only in case some text is attached to the shape
            mpTextExport->WriteOutliner(*pParaObj);
        }

        if (bIsFontworkShape)
            FontworkHelpers::applyPropsToRuns(aOldCharFillPropVec, xXText);
        return;
    }

    bool bOverridingCharHeight = false;
    sal_Int32 nCharHeight = -1;
    bool bFirstParagraph = true;

    // tdf#144092 For shapes without text: Export run properties (into
    // endParaRPr) from the shape's propset instead of the paragraph's.
    if(xXText->getString().isEmpty() && enumeration->hasMoreElements())
    {
        Any aAny (enumeration->nextElement());
        Reference<XTextContent> xParagraph;
        if( aAny >>= xParagraph )
        {
            mpFS->startElementNS(XML_a, XML_p);
            WriteParagraphProperties(xParagraph, nCharHeight, XML_pPr);
            sal_Int16 nDummy = -1;
            WriteRunProperties(rXPropSet, false, XML_endParaRPr, false,
                               bOverridingCharHeight, nCharHeight, nDummy, rXPropSet);
            mpFS->endElementNS(XML_a, XML_p);
        }
        return;
    }

    while( enumeration->hasMoreElements() )
    {
        Reference< XTextContent > paragraph;
        Any any ( enumeration->nextElement() );

        if( any >>= paragraph)
        {
            if (bFirstParagraph && bWritePropertiesAsLstStyles)
                WriteLstStyles(paragraph, bOverridingCharHeight, nCharHeight, rXPropSet);

            WriteParagraph(paragraph, bOverridingCharHeight, nCharHeight, rXPropSet);
            bFirstParagraph = false;
        }
    }
}

void DrawingML::WritePresetShape( const OString& pShape , std::vector< std::pair<sal_Int32,sal_Int32>> & rAvList )
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

void DrawingML::WritePresetShape( const OString& pShape )
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
    OStringBuffer aLine;
    bool bNotDone = aStream.ReadLine(aLine);
    while (bNotDone)
    {
        sal_Int32 nIndex = 0;
        // Each line is in a "key\tvalue" format: read the key, the rest is the value.
        OString aKey( o3tl::getToken(aLine, 0, '\t', nIndex) );
        OString aValue( std::string_view(aLine).substr(nIndex) );
        aRet[aKey].push_back(aValue);
        bNotDone = aStream.ReadLine(aLine);
    }
    return aRet;
}

void DrawingML::WritePresetShape( const OString& pShape, MSO_SPT eShapeType, bool bPredefinedHandlesUsed, const PropertyValue& rProp )
{
    static std::map< OString, std::vector<OString> > aAdjMap = lcl_getAdjNames();
    // If there are predefined adj names for this shape type, look them up now.
    std::vector<OString> aAdjustments;
    auto it = aAdjMap.find(pShape);
    if (it != aAdjMap.end())
        aAdjustments = it->second;

    mpFS->startElementNS(XML_a, XML_prstGeom, XML_prst, pShape);
    mpFS->startElementNS(XML_a, XML_avLst);

    Sequence< drawing::EnhancedCustomShapeAdjustmentValue > aAdjustmentSeq;
    if ( ( rProp.Value >>= aAdjustmentSeq )
         && eShapeType != mso_sptActionButtonForwardNext  // we have adjustments values for these type of shape, but MSO doesn't like them
         && eShapeType != mso_sptActionButtonBackPrevious // so they are now disabled
         && pShape != "rect" //some shape types are commented out in pCustomShapeTypeTranslationTable[] & are being defaulted to rect & rect does not have adjustment values/name.
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

namespace // helpers for DrawingML::WriteCustomGeometry
{
sal_Int32
FindNextCommandEndSubpath(const sal_Int32 nStart,
                          const uno::Sequence<drawing::EnhancedCustomShapeSegment>& rSegments)
{
    sal_Int32 i = nStart < 0 ? 0 : nStart;
    while (i < rSegments.getLength() && rSegments[i].Command != ENDSUBPATH)
        i++;
    return i;
}

bool HasCommandInSubPath(const sal_Int16 nCommand, const sal_Int32 nFirst, const sal_Int32 nLast,
                         const uno::Sequence<drawing::EnhancedCustomShapeSegment>& rSegments)
{
    for (sal_Int32 i = nFirst < 0 ? 0 : nFirst; i <= nLast && i < rSegments.getLength(); i++)
    {
        if (rSegments[i].Command == nCommand)
            return true;
    }
    return false;
}

// Ellipse is given by radii fwR and fhR and center (fCx|fCy). The ray from center through point RayP
// intersects the ellipse in point S and this point S has angle fAngleDeg in degrees.
void getEllipsePointAndAngleFromRayPoint(double& rfAngleDeg, double& rfSx, double& rfSy,
                                         const double fWR, const double fHR, const double fCx,
                                         const double fCy, const double fRayPx, const double fRayPy)
{
    if (basegfx::fTools::equalZero(fWR) || basegfx::fTools::equalZero(fHR))
    {
        rfSx = fCx; // needed for getting new 'current point'
        rfSy = fCy;
    }
    else
    {
        // center ellipse at origin, stretch in y-direction to circle, flip to Math orientation
        // and get angle
        double fCircleMathAngle = atan2(-fWR / fHR * (fRayPy - fCy), fRayPx - fCx);
        // use angle for intersection point on circle and stretch back to ellipse
        double fPointMathEllipse_x = fWR * cos(fCircleMathAngle);
        double fPointMathEllipse_y = fHR * sin(fCircleMathAngle);
        // get angle of intersection point on ellipse
        double fEllipseMathAngle = atan2(fPointMathEllipse_y, fPointMathEllipse_x);
        // convert from Math to View orientation and shift ellipse back from origin
        rfAngleDeg = -basegfx::rad2deg(fEllipseMathAngle);
        rfSx = fPointMathEllipse_x + fCx;
        rfSy = -fPointMathEllipse_y + fCy;
    }
}

void getEllipsePointFromViewAngle(double& rfSx, double& rfSy, const double fWR, const double fHR,
                                  const double fCx, const double fCy, const double fViewAngleDeg)
{
    if (basegfx::fTools::equalZero(fWR) || basegfx::fTools::equalZero(fHR))
    {
        rfSx = fCx; // needed for getting new 'current point'
        rfSy = fCy;
    }
    else
    {
        double fX = cos(basegfx::deg2rad(fViewAngleDeg)) / fWR;
        double fY = sin(basegfx::deg2rad(fViewAngleDeg)) / fHR;
        double fRadius = 1.0 / std::hypot(fX, fY);
        rfSx = fCx + fRadius * cos(basegfx::deg2rad(fViewAngleDeg));
        rfSy = fCy + fRadius * sin(basegfx::deg2rad(fViewAngleDeg));
    }
}

sal_Int32 GetCustomGeometryPointValue(const css::drawing::EnhancedCustomShapeParameter& rParam,
                                      const EnhancedCustomShape2d& rCustomShape2d,
                                      const bool bReplaceGeoWidth, const bool bReplaceGeoHeight)
{
    double fValue = 0.0;
    rCustomShape2d.GetParameter(fValue, rParam, bReplaceGeoWidth, bReplaceGeoHeight);
    sal_Int32 nValue(std::lround(fValue));

    return nValue;
}

struct TextAreaRect
{
    OString left;
    OString top;
    OString right;
    OString bottom;
};

struct Guide
{
    OString sName;
    OString sFormula;
};

void prepareTextArea(const EnhancedCustomShape2d& rEnhancedCustomShape2d,
                     std::vector<Guide>& rGuideList, TextAreaRect& rTextAreaRect)
{
    tools::Rectangle aTextAreaLO(rEnhancedCustomShape2d.GetTextRect());
    tools::Rectangle aLogicRectLO(rEnhancedCustomShape2d.GetLogicRect());
    if (aTextAreaLO == aLogicRectLO)
    {
        rTextAreaRect.left = "l"_ostr;
        rTextAreaRect.top = "t"_ostr;
        rTextAreaRect.right = "r"_ostr;
        rTextAreaRect.bottom = "b"_ostr;
        return;
    }
    // Flip aTextAreaLO if shape is flipped
    if (rEnhancedCustomShape2d.IsFlipHorz())
        aTextAreaLO.Move((aLogicRectLO.Center().X() - aTextAreaLO.Center().X()) * 2, 0);
    if (rEnhancedCustomShape2d.IsFlipVert())
        aTextAreaLO.Move(0, (aLogicRectLO.Center().Y() - aTextAreaLO.Center().Y()) * 2);

    Guide aGuide;
    // horizontal
    const sal_Int32 nWidth = aLogicRectLO.Right() - aLogicRectLO.Left();
    const OString sWidth = OString::number(oox::drawingml::convertHmmToEmu(nWidth));

    // left
    aGuide.sName = "textAreaLeft"_ostr;
    sal_Int32 nHelp = aTextAreaLO.Left() - aLogicRectLO.Left();
    const OString sLeft = OString::number(oox::drawingml::convertHmmToEmu(nHelp));
    aGuide.sFormula = "*/ " + sLeft + " w " + sWidth;
    rTextAreaRect.left = aGuide.sName;
    rGuideList.push_back(aGuide);

    // right
    aGuide.sName = "textAreaRight"_ostr;
    nHelp = aTextAreaLO.Right() - aLogicRectLO.Left();
    const OString sRight = OString::number(oox::drawingml::convertHmmToEmu(nHelp));
    aGuide.sFormula = "*/ " + sRight + " w " + sWidth;
    rTextAreaRect.right = aGuide.sName;
    rGuideList.push_back(aGuide);

    // vertical
    const sal_Int32 nHeight = aLogicRectLO.Bottom() - aLogicRectLO.Top();
    const OString sHeight = OString::number(oox::drawingml::convertHmmToEmu(nHeight));

    // top
    aGuide.sName = "textAreaTop"_ostr;
    nHelp = aTextAreaLO.Top() - aLogicRectLO.Top();
    const OString sTop = OString::number(oox::drawingml::convertHmmToEmu(nHelp));
    aGuide.sFormula = "*/ " + sTop + " h " + sHeight;
    rTextAreaRect.top = aGuide.sName;
    rGuideList.push_back(aGuide);

    // bottom
    aGuide.sName = "textAreaBottom"_ostr;
    nHelp = aTextAreaLO.Bottom() - aLogicRectLO.Top();
    const OString sBottom = OString::number(oox::drawingml::convertHmmToEmu(nHelp));
    aGuide.sFormula = "*/ " + sBottom + " h " + sHeight;
    rTextAreaRect.bottom = aGuide.sName;
    rGuideList.push_back(aGuide);

    return;
}
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
    if (!pGeometrySeq)
        return false;

    auto pPathProp = std::find_if(std::cbegin(*pGeometrySeq), std::cend(*pGeometrySeq),
                                  [](const PropertyValue& rProp) { return rProp.Name == "Path"; });
    if (pPathProp == std::cend(*pGeometrySeq))
        return false;

    uno::Sequence<beans::PropertyValue> aPathProp;
    pPathProp->Value >>= aPathProp;

    uno::Sequence<drawing::EnhancedCustomShapeParameterPair> aPairs;
    uno::Sequence<drawing::EnhancedCustomShapeSegment> aSegments;
    uno::Sequence<awt::Size> aPathSize;
    bool bReplaceGeoWidth = false;
    bool bReplaceGeoHeight = false;
    for (const beans::PropertyValue& rPathProp : aPathProp)
    {
        if (rPathProp.Name == "Coordinates")
            rPathProp.Value >>= aPairs;
        else if (rPathProp.Name == "Segments")
            rPathProp.Value >>= aSegments;
        else if (rPathProp.Name == "SubViewSize")
            rPathProp.Value >>= aPathSize;
        else if (rPathProp.Name == "StretchX")
            bReplaceGeoWidth = true;
        else if (rPathProp.Name == "StretchY")
            bReplaceGeoHeight = true;
    }

    if ( !aPairs.hasElements() )
        return false;

    if ( !aSegments.hasElements() )
    {
        aSegments = uno::Sequence<drawing::EnhancedCustomShapeSegment>
            {
                { MOVETO, 1 },
                { LINETO,
                  static_cast<sal_Int16>(std::min( aPairs.getLength() - 1, sal_Int32(32767) )) },
                { CLOSESUBPATH, 0 },
                { ENDSUBPATH, 0 }
            };
    };

    int nExpectedPairCount = std::accumulate(std::cbegin(aSegments), std::cend(aSegments), 0,
        [](const int nSum, const drawing::EnhancedCustomShapeSegment& rSegment) { return nSum + rSegment.Count; });

    if ( nExpectedPairCount > aPairs.getLength() )
    {
        SAL_WARN("oox.shape", "Segments need " << nExpectedPairCount << " coordinates, but Coordinates have only " << aPairs.getLength() << " pairs.");
        return false;
    }

    // A EnhancedCustomShape2d caches the equation results. Therefore we use only one of it for the
    // entire method.
    const EnhancedCustomShape2d aCustomShape2d(const_cast<SdrObjCustomShape&>(rSdrObjCustomShape));

    TextAreaRect aTextAreaRect;
    std::vector<Guide> aGuideList; // for now only for <a:rect>
    prepareTextArea(aCustomShape2d, aGuideList, aTextAreaRect);
    mpFS->startElementNS(XML_a, XML_custGeom);
    mpFS->singleElementNS(XML_a, XML_avLst);
    if (aGuideList.empty())
    {
        mpFS->singleElementNS(XML_a, XML_gdLst);
    }
    else
    {
        mpFS->startElementNS(XML_a, XML_gdLst);
        for (auto const& elem : aGuideList)
        {
            mpFS->singleElementNS(XML_a, XML_gd, XML_name, elem.sName, XML_fmla, elem.sFormula);
        }
        mpFS->endElementNS(XML_a, XML_gdLst);
    }
    mpFS->singleElementNS(XML_a, XML_ahLst);
    mpFS->singleElementNS(XML_a, XML_rect, XML_l, aTextAreaRect.left, XML_t, aTextAreaRect.top,
                          XML_r, aTextAreaRect.right, XML_b, aTextAreaRect.bottom);
    mpFS->startElementNS(XML_a, XML_pathLst);

    // Prepare width and height for <a:path>
    bool bUseGlobalViewBox(false);

    // nViewBoxWidth must be integer otherwise ReplaceGeoWidth in aCustomShape2d.GetParameter() is not
    // triggered; same for height.
    sal_Int32 nViewBoxWidth(0);
    sal_Int32 nViewBoxHeight(0);
    if (!aPathSize.hasElements())
    {
        bUseGlobalViewBox = true;
        // If draw:viewBox is missing in draw:enhancedGeometry, then import sets
        // viewBox="0 0 21600 21600". Missing ViewBox can only occur, if user has manipulated
        // current file via macro. Author of macro has to fix it.
        auto pProp = std::find_if(
            std::cbegin(*pGeometrySeq), std::cend(*pGeometrySeq),
            [](const beans::PropertyValue& rGeomProp) { return rGeomProp.Name == "ViewBox"; });
        if (pProp != std::cend(*pGeometrySeq))
        {
            css::awt::Rectangle aViewBox;
            if (pProp->Value >>= aViewBox)
            {
                nViewBoxWidth = aViewBox.Width;
                nViewBoxHeight = aViewBox.Height;
                css::drawing::EnhancedCustomShapeParameter aECSP;
                aECSP.Type = css::drawing::EnhancedCustomShapeParameterType::NORMAL;
                aECSP.Value <<= nViewBoxWidth;
                double fRetValue;
                aCustomShape2d.GetParameter(fRetValue, aECSP, true, false);
                nViewBoxWidth = basegfx::fround(fRetValue);
                aECSP.Value <<= nViewBoxHeight;
                aCustomShape2d.GetParameter(fRetValue, aECSP, false, true);
                nViewBoxHeight = basegfx::fround(fRetValue);
            }
        }
        // Import from oox or documents, which are imported from oox and saved to strict ODF, might
        // have no subViewSize but viewBox="0 0 0 0". We need to generate width and height in those
        // cases. Even if that is fixed, we need the substitute for old documents.
        if ((nViewBoxWidth == 0 && nViewBoxHeight == 0) || pProp == std::cend(*pGeometrySeq))
        {
            // Generate a substitute based on point coordinates
            sal_Int32 nXMin(0);
            aPairs[0].First.Value >>= nXMin;
            sal_Int32 nXMax = nXMin;
            sal_Int32 nYMin(0);
            aPairs[0].Second.Value >>= nYMin;
            sal_Int32 nYMax = nYMin;

            for (const auto& rPair : aPairs)
            {
                sal_Int32 nX = GetCustomGeometryPointValue(rPair.First, aCustomShape2d,
                                                           bReplaceGeoWidth, false);
                sal_Int32 nY = GetCustomGeometryPointValue(rPair.Second, aCustomShape2d, false,
                                                           bReplaceGeoHeight);
                if (nX < nXMin)
                    nXMin = nX;
                if (nY < nYMin)
                    nYMin = nY;
                if (nX > nXMax)
                    nXMax = nX;
                if (nY > nYMax)
                    nYMax = nY;
            }
            nViewBoxWidth = std::max(nXMax, nXMax - nXMin);
            nViewBoxHeight = std::max(nYMax, nYMax - nYMin);
        }
        // ToDo: Other values of left,top than 0,0 are not considered yet. Such would require a
        // shift of the resulting path coordinates.
    }

    // Iterate over subpaths
    sal_Int32 nPairIndex = 0; // index over "Coordinates"
    sal_Int32 nPathSizeIndex = 0; // index over "SubViewSize"
    sal_Int32 nSubpathStartIndex(0); // index over "Segments"
    sal_Int32 nSubPathIndex(0); // serial number of current subpath
    do
    {
        bool bOK(true); // catch faulty paths were commands do not correspond to points
        // get index of next command ENDSUBPATH; if such doesn't exist use index behind last segment
        sal_Int32 nNextNcommandIndex = FindNextCommandEndSubpath(nSubpathStartIndex, aSegments);

        // Prepare attributes for a:path start element
        // NOFILL or one of the LIGHTEN commands
        std::optional<OString> sFill;
        if (HasCommandInSubPath(NOFILL, nSubpathStartIndex, nNextNcommandIndex - 1, aSegments))
            sFill = "none";
        else if (HasCommandInSubPath(DARKEN, nSubpathStartIndex, nNextNcommandIndex - 1, aSegments))
            sFill = "darken";
        else if (HasCommandInSubPath(DARKENLESS, nSubpathStartIndex, nNextNcommandIndex - 1,
                                     aSegments))
            sFill = "darkenLess";
        else if (HasCommandInSubPath(LIGHTEN, nSubpathStartIndex, nNextNcommandIndex - 1,
                                     aSegments))
            sFill = "lighten";
        else if (HasCommandInSubPath(LIGHTENLESS, nSubpathStartIndex, nNextNcommandIndex - 1,
                                     aSegments))
            sFill = "lightenLess";
        else
        {
            // shading info might be in object type, e.g. "Octagon Bevel".
            sal_Int32 nLuminanceChange(aCustomShape2d.GetLuminanceChange(nSubPathIndex));
            if (nLuminanceChange <= -40)
                sFill = "darken";
            else if (nLuminanceChange <= -10)
                sFill = "darkenLess";
            else if (nLuminanceChange >= 40)
                sFill = "lighten";
            else if (nLuminanceChange >= 10)
                sFill = "lightenLess";
        }
        // NOSTROKE
        std::optional<OString> sStroke;
        if (HasCommandInSubPath(NOSTROKE, nSubpathStartIndex, nNextNcommandIndex - 1, aSegments))
            sStroke = "0";

        // Write a:path start element
        mpFS->startElementNS(
            XML_a, XML_path, XML_fill, sFill, XML_stroke, sStroke, XML_w,
            OString::number(bUseGlobalViewBox ? nViewBoxWidth : aPathSize[nPathSizeIndex].Width),
            XML_h,
            OString::number(bUseGlobalViewBox ? nViewBoxHeight : aPathSize[nPathSizeIndex].Height));

        // Arcs drawn by commands ELLIPTICALQUADRANTX and ELLIPTICALQUADRANTY depend on the position
        // of the target point in regard to the current point. Therefore we need to track the
        // current point. A current point is not defined in the beginning.
        double fCurrentX(0.0);
        double fCurrentY(0.0);
        bool bCurrentValid(false);
        // Actually write the subpath
        for (sal_Int32 nSegmentIndex = nSubpathStartIndex; nSegmentIndex < nNextNcommandIndex;
             ++nSegmentIndex)
        {
            const auto& rSegment(aSegments[nSegmentIndex]);
            if (rSegment.Command == CLOSESUBPATH)
            {
                mpFS->singleElementNS(XML_a, XML_close); // command Z has no parameter
                // ODF 1.4 specifies, that the start of the subpath becomes the current point.
                // But that is not implemented yet. Currently LO keeps the last current point.
            }
            for (sal_Int32 k = 0; k < rSegment.Count && bOK; ++k)
            {
                bOK = WriteCustomGeometrySegment(rSegment.Command, k, aPairs, nPairIndex, fCurrentX,
                                                 fCurrentY, bCurrentValid, aCustomShape2d,
                                                 bReplaceGeoWidth, bReplaceGeoHeight);
            }
        } // end loop over all commands of subpath
        // finish this subpath in any case
        mpFS->endElementNS(XML_a, XML_path);

        if (!bOK)
            break; // exit loop if not enough values in aPairs

        // step forward to next subpath
        nSubpathStartIndex = nNextNcommandIndex + 1;
        nPathSizeIndex++;
        nSubPathIndex++;
    } while (nSubpathStartIndex < aSegments.getLength());

    mpFS->endElementNS(XML_a, XML_pathLst);
    mpFS->endElementNS(XML_a, XML_custGeom);
    return true; // We have written custGeom even if path is poorly structured.
}

bool DrawingML::WriteCustomGeometrySegment(
    const sal_Int16 eCommand, const sal_Int32 nCount,
    const uno::Sequence<css::drawing::EnhancedCustomShapeParameterPair>& rPairs,
    sal_Int32& rnPairIndex, double& rfCurrentX, double& rfCurrentY, bool& rbCurrentValid,
    const EnhancedCustomShape2d& rCustomShape2d, const bool bReplaceGeoWidth,
    const bool bReplaceGeoHeight)
{
    switch (eCommand)
    {
        case MOVETO:
        {
            if (rnPairIndex >= rPairs.getLength())
                return false;

            mpFS->startElementNS(XML_a, XML_moveTo);
            WriteCustomGeometryPoint(rPairs[rnPairIndex], rCustomShape2d, bReplaceGeoWidth,
                                     bReplaceGeoHeight);
            mpFS->endElementNS(XML_a, XML_moveTo);
            rCustomShape2d.GetParameter(rfCurrentX, rPairs[rnPairIndex].First, bReplaceGeoWidth,
                                        false);
            rCustomShape2d.GetParameter(rfCurrentY, rPairs[rnPairIndex].Second, false,
                                        bReplaceGeoHeight);
            rbCurrentValid = true;
            rnPairIndex++;
            break;
        }
        case LINETO:
        {
            if (rnPairIndex >= rPairs.getLength())
                return false;
            // LINETO without valid current point is a faulty path. LO is tolerant and makes a
            // moveTo instead. Do the same on export. MS OFFICE requires a current point for lnTo,
            // otherwise it shows nothing of the shape.
            if (rbCurrentValid)
            {
                mpFS->startElementNS(XML_a, XML_lnTo);
                WriteCustomGeometryPoint(rPairs[rnPairIndex], rCustomShape2d, bReplaceGeoWidth,
                                         bReplaceGeoHeight);
                mpFS->endElementNS(XML_a, XML_lnTo);
            }
            else
            {
                mpFS->startElementNS(XML_a, XML_moveTo);
                WriteCustomGeometryPoint(rPairs[rnPairIndex], rCustomShape2d, bReplaceGeoWidth,
                                         bReplaceGeoHeight);
                mpFS->endElementNS(XML_a, XML_moveTo);
            }
            rCustomShape2d.GetParameter(rfCurrentX, rPairs[rnPairIndex].First, bReplaceGeoWidth,
                                        false);
            rCustomShape2d.GetParameter(rfCurrentY, rPairs[rnPairIndex].Second, false,
                                        bReplaceGeoHeight);
            rbCurrentValid = true;
            rnPairIndex++;
            break;
        }
        case CURVETO:
        {
            if (rnPairIndex + 2 >= rPairs.getLength())
                return false;

            mpFS->startElementNS(XML_a, XML_cubicBezTo);
            for (sal_uInt8 i = 0; i <= 2; ++i)
            {
                WriteCustomGeometryPoint(rPairs[rnPairIndex + i], rCustomShape2d, bReplaceGeoWidth,
                                         bReplaceGeoHeight);
            }
            mpFS->endElementNS(XML_a, XML_cubicBezTo);
            rCustomShape2d.GetParameter(rfCurrentX, rPairs[rnPairIndex + 2].First, bReplaceGeoWidth,
                                        false);
            rCustomShape2d.GetParameter(rfCurrentY, rPairs[rnPairIndex + 2].Second, false,
                                        bReplaceGeoHeight);
            rbCurrentValid = true;
            rnPairIndex += 3;
            break;
        }
        case ANGLEELLIPSETO:
        case ANGLEELLIPSE:
        {
            if (rnPairIndex + 2 >= rPairs.getLength())
                return false;

            // Read parameters
            double fCx = 0.0;
            rCustomShape2d.GetParameter(fCx, rPairs[rnPairIndex].First, bReplaceGeoWidth, false);
            double fCy = 0.0;
            rCustomShape2d.GetParameter(fCy, rPairs[rnPairIndex].Second, false, bReplaceGeoHeight);
            double fWR = 0.0;
            rCustomShape2d.GetParameter(fWR, rPairs[rnPairIndex + 1].First, false, false);
            double fHR = 0.0;
            rCustomShape2d.GetParameter(fHR, rPairs[rnPairIndex + 1].Second, false, false);
            double fStartAngle = 0.0;
            rCustomShape2d.GetParameter(fStartAngle, rPairs[rnPairIndex + 2].First, false, false);
            double fEndAngle = 0.0;
            rCustomShape2d.GetParameter(fEndAngle, rPairs[rnPairIndex + 2].Second, false, false);

            // Prepare start and swing angle
            sal_Int32 nStartAng(std::lround(fStartAngle * 60000));
            sal_Int32 nSwingAng = 0;
            if (basegfx::fTools::equalZero(fStartAngle)
                && basegfx::fTools::equalZero(fEndAngle - 360.0))
                nSwingAng = 360 * 60000; // special case full circle
            else
            {
                nSwingAng = std::lround((fEndAngle - fStartAngle) * 60000);
                if (nSwingAng < 0)
                    nSwingAng += 360 * 60000;
            }

            // calculate start point on ellipse
            double fSx = 0.0;
            double fSy = 0.0;
            getEllipsePointFromViewAngle(fSx, fSy, fWR, fHR, fCx, fCy, fStartAngle);

            // write markup for going to start point
            // lnTo requires a valid current point
            if (eCommand == ANGLEELLIPSETO && rbCurrentValid)
            {
                mpFS->startElementNS(XML_a, XML_lnTo);
                mpFS->singleElementNS(XML_a, XML_pt, XML_x, OString::number(std::lround(fSx)),
                                      XML_y, OString::number(std::lround(fSy)));
                mpFS->endElementNS(XML_a, XML_lnTo);
            }
            else
            {
                mpFS->startElementNS(XML_a, XML_moveTo);
                mpFS->singleElementNS(XML_a, XML_pt, XML_x, OString::number(std::lround(fSx)),
                                      XML_y, OString::number(std::lround(fSy)));
                mpFS->endElementNS(XML_a, XML_moveTo);
            }
            // write markup for arcTo
            if (!basegfx::fTools::equalZero(fWR) && !basegfx::fTools::equalZero(fHR))
                mpFS->singleElement(
                    FSNS(XML_a, XML_arcTo), XML_wR, OString::number(std::lround(fWR)), XML_hR,
                    OString::number(std::lround(fHR)), XML_stAng, OString::number(nStartAng),
                    XML_swAng, OString::number(nSwingAng));

            getEllipsePointFromViewAngle(rfCurrentX, rfCurrentY, fWR, fHR, fCx, fCy, fEndAngle);
            rbCurrentValid = true;
            rnPairIndex += 3;
            break;
        }
        case ARCTO:
        case ARC:
        case CLOCKWISEARCTO:
        case CLOCKWISEARC:
        {
            if (rnPairIndex + 3 >= rPairs.getLength())
                return false;

            // read parameters
            double fX1 = 0.0;
            rCustomShape2d.GetParameter(fX1, rPairs[rnPairIndex].First, bReplaceGeoWidth, false);
            double fY1 = 0.0;
            rCustomShape2d.GetParameter(fY1, rPairs[rnPairIndex].Second, false, bReplaceGeoHeight);
            double fX2 = 0.0;
            rCustomShape2d.GetParameter(fX2, rPairs[rnPairIndex + 1].First, bReplaceGeoWidth,
                                        false);
            double fY2 = 0.0;
            rCustomShape2d.GetParameter(fY2, rPairs[rnPairIndex + 1].Second, false,
                                        bReplaceGeoHeight);
            double fX3 = 0.0;
            rCustomShape2d.GetParameter(fX3, rPairs[rnPairIndex + 2].First, bReplaceGeoWidth,
                                        false);
            double fY3 = 0.0;
            rCustomShape2d.GetParameter(fY3, rPairs[rnPairIndex + 2].Second, false,
                                        bReplaceGeoHeight);
            double fX4 = 0.0;
            rCustomShape2d.GetParameter(fX4, rPairs[rnPairIndex + 3].First, bReplaceGeoWidth,
                                        false);
            double fY4 = 0.0;
            rCustomShape2d.GetParameter(fY4, rPairs[rnPairIndex + 3].Second, false,
                                        bReplaceGeoHeight);
            // calculate ellipse parameter
            const double fWR = (std::max(fX1, fX2) - std::min(fX1, fX2)) / 2.0;
            const double fHR = (std::max(fY1, fY2) - std::min(fY1, fY2)) / 2.0;
            const double fCx = (fX1 + fX2) / 2.0;
            const double fCy = (fY1 + fY2) / 2.0;
            // calculate start angle
            double fStartAngle = 0.0;
            double fPx = 0.0;
            double fPy = 0.0;
            getEllipsePointAndAngleFromRayPoint(fStartAngle, fPx, fPy, fWR, fHR, fCx, fCy, fX3,
                                                fY3);
            // markup for going to start point
            // lnTo requires a valid current point.
            if ((eCommand == ARCTO || eCommand == CLOCKWISEARCTO) && rbCurrentValid)
            {
                mpFS->startElementNS(XML_a, XML_lnTo);
                mpFS->singleElementNS(XML_a, XML_pt, XML_x, OString::number(std::lround(fPx)),
                                      XML_y, OString::number(std::lround(fPy)));
                mpFS->endElementNS(XML_a, XML_lnTo);
            }
            else
            {
                mpFS->startElementNS(XML_a, XML_moveTo);
                mpFS->singleElementNS(XML_a, XML_pt, XML_x, OString::number(std::lround(fPx)),
                                      XML_y, OString::number(std::lround(fPy)));
                mpFS->endElementNS(XML_a, XML_moveTo);
            }
            // calculate swing angle
            double fEndAngle = 0.0;
            getEllipsePointAndAngleFromRayPoint(fEndAngle, fPx, fPy, fWR, fHR, fCx, fCy, fX4, fY4);
            double fSwingAngle(fEndAngle - fStartAngle);
            const bool bIsClockwise(eCommand == CLOCKWISEARCTO || eCommand == CLOCKWISEARC);
            if (bIsClockwise && fSwingAngle < 0)
                fSwingAngle += 360.0;
            else if (!bIsClockwise && fSwingAngle > 0)
                fSwingAngle -= 360.0;
            // markup for arcTo
            // ToDo: write markup for case zero width or height of ellipse
            const sal_Int32 nStartAng(std::lround(fStartAngle * 60000));
            const sal_Int32 nSwingAng(std::lround(fSwingAngle * 60000));
            mpFS->singleElement(FSNS(XML_a, XML_arcTo), XML_wR, OString::number(std::lround(fWR)),
                                XML_hR, OString::number(std::lround(fHR)), XML_stAng,
                                OString::number(nStartAng), XML_swAng, OString::number(nSwingAng));
            rfCurrentX = fPx;
            rfCurrentY = fPy;
            rbCurrentValid = true;
            rnPairIndex += 4;
            break;
        }
        case ELLIPTICALQUADRANTX:
        case ELLIPTICALQUADRANTY:
        {
            if (rnPairIndex >= rPairs.getLength())
                return false;

            // read parameters
            double fX = 0.0;
            rCustomShape2d.GetParameter(fX, rPairs[rnPairIndex].First, bReplaceGeoWidth, false);
            double fY = 0.0;
            rCustomShape2d.GetParameter(fY, rPairs[rnPairIndex].Second, false, bReplaceGeoHeight);

            // Prepare parameters for arcTo
            if (rbCurrentValid)
            {
                double fWR = std::abs(rfCurrentX - fX);
                double fHR = std::abs(rfCurrentY - fY);
                double fStartAngle(0.0);
                double fSwingAngle(0.0);
                // The starting direction of the arc toggles between X and Y
                if ((eCommand == ELLIPTICALQUADRANTX && !(nCount % 2))
                    || (eCommand == ELLIPTICALQUADRANTY && (nCount % 2)))
                {
                    // arc starts horizontal
                    fStartAngle = fY < rfCurrentY ? 90.0 : 270.0;
                    const bool bClockwise = (fX < rfCurrentX && fY < rfCurrentY)
                                            || (fX > rfCurrentX && fY > rfCurrentY);
                    fSwingAngle = bClockwise ? 90.0 : -90.0;
                }
                else
                {
                    // arc starts vertical
                    fStartAngle = fX < rfCurrentX ? 0.0 : 180.0;
                    const bool bClockwise = (fX < rfCurrentX && fY > rfCurrentY)
                                            || (fX > rfCurrentX && fY < rfCurrentY);
                    fSwingAngle = bClockwise ? 90.0 : -90.0;
                }
                sal_Int32 nStartAng(std::lround(fStartAngle * 60000));
                sal_Int32 nSwingAng(std::lround(fSwingAngle * 60000));
                mpFS->singleElement(
                    FSNS(XML_a, XML_arcTo), XML_wR, OString::number(std::lround(fWR)), XML_hR,
                    OString::number(std::lround(fHR)), XML_stAng, OString::number(nStartAng),
                    XML_swAng, OString::number(nSwingAng));
            }
            else
            {
                // faulty path, but we continue with the target point
                mpFS->startElementNS(XML_a, XML_moveTo);
                WriteCustomGeometryPoint(rPairs[rnPairIndex], rCustomShape2d, bReplaceGeoWidth,
                                         bReplaceGeoHeight);
                mpFS->endElementNS(XML_a, XML_moveTo);
            }
            rfCurrentX = fX;
            rfCurrentY = fY;
            rbCurrentValid = true;
            rnPairIndex++;
            break;
        }
        case QUADRATICCURVETO:
        {
            if (rnPairIndex + 1 >= rPairs.getLength())
                return false;

            mpFS->startElementNS(XML_a, XML_quadBezTo);
            for (sal_uInt8 i = 0; i < 2; ++i)
            {
                WriteCustomGeometryPoint(rPairs[rnPairIndex + i], rCustomShape2d, bReplaceGeoWidth,
                                         bReplaceGeoHeight);
            }
            mpFS->endElementNS(XML_a, XML_quadBezTo);
            rCustomShape2d.GetParameter(rfCurrentX, rPairs[rnPairIndex + 1].First, bReplaceGeoWidth,
                                        false);
            rCustomShape2d.GetParameter(rfCurrentY, rPairs[rnPairIndex + 1].Second, false,
                                        bReplaceGeoHeight);
            rbCurrentValid = true;
            rnPairIndex += 2;
            break;
        }
        case ARCANGLETO:
        {
            if (rnPairIndex + 1 >= rPairs.getLength())
                return false;

            double fWR = 0.0;
            rCustomShape2d.GetParameter(fWR, rPairs[rnPairIndex].First, false, false);
            double fHR = 0.0;
            rCustomShape2d.GetParameter(fHR, rPairs[rnPairIndex].Second, false, false);
            double fStartAngle = 0.0;
            rCustomShape2d.GetParameter(fStartAngle, rPairs[rnPairIndex + 1].First, false, false);
            sal_Int32 nStartAng(std::lround(fStartAngle * 60000));
            double fSwingAng = 0.0;
            rCustomShape2d.GetParameter(fSwingAng, rPairs[rnPairIndex + 1].Second, false, false);
            sal_Int32 nSwingAng(std::lround(fSwingAng * 60000));
            mpFS->singleElement(FSNS(XML_a, XML_arcTo), XML_wR, OString::number(fWR), XML_hR,
                                OString::number(fHR), XML_stAng, OString::number(nStartAng),
                                XML_swAng, OString::number(nSwingAng));
            double fPx = 0.0;
            double fPy = 0.0;
            getEllipsePointFromViewAngle(fPx, fPy, fWR, fHR, 0.0, 0.0, fStartAngle);
            double fCx = rfCurrentX - fPx;
            double fCy = rfCurrentY - fPy;
            getEllipsePointFromViewAngle(rfCurrentX, rfCurrentY, fWR, fHR, fCx, fCy,
                                         fStartAngle + fSwingAng);
            rbCurrentValid = true;
            rnPairIndex += 2;
            break;
        }
        default:
            // do nothing
            break;
    }
    return true;
}

void DrawingML::WriteCustomGeometryPoint(
    const drawing::EnhancedCustomShapeParameterPair& rParamPair,
    const EnhancedCustomShape2d& rCustomShape2d, const bool bReplaceGeoWidth,
    const bool bReplaceGeoHeight)
{
    sal_Int32 nX
        = GetCustomGeometryPointValue(rParamPair.First, rCustomShape2d, bReplaceGeoWidth, false);
    sal_Int32 nY
        = GetCustomGeometryPointValue(rParamPair.Second, rCustomShape2d, false, bReplaceGeoHeight);

    mpFS->singleElementNS(XML_a, XML_pt, XML_x, OString::number(nX), XML_y, OString::number(nY));
}

void DrawingML::WriteEmptyCustomGeometry()
{
    // This method is used for export to docx in case WriteCustomGeometry fails.
    mpFS->startElementNS(XML_a, XML_custGeom);
    mpFS->singleElementNS(XML_a, XML_avLst);
    mpFS->singleElementNS(XML_a, XML_gdLst);
    mpFS->singleElementNS(XML_a, XML_ahLst);
    mpFS->singleElementNS(XML_a, XML_rect, XML_l, "0", XML_t, "0", XML_r, "r", XML_b, "b");
    mpFS->singleElementNS(XML_a, XML_pathLst);
    mpFS->endElementNS(XML_a, XML_custGeom);
}

// version for SdrPathObj
void DrawingML::WritePolyPolygon(const css::uno::Reference<css::drawing::XShape>& rXShape,
                                 const bool bClosed)
{
    tools::PolyPolygon aPolyPolygon = EscherPropertyContainer::GetPolyPolygon(rXShape);
    // In case of Writer, the parent element is <wps:spPr>, and there the
    // <a:custGeom> element is not optional.
    if (aPolyPolygon.Count() < 1 && GetDocumentType() != DOCUMENT_DOCX)
        return;

    mpFS->startElementNS(XML_a, XML_custGeom);
    mpFS->singleElementNS(XML_a, XML_avLst);
    mpFS->singleElementNS(XML_a, XML_gdLst);
    mpFS->singleElementNS(XML_a, XML_ahLst);
    mpFS->singleElementNS(XML_a, XML_rect, XML_l, "0", XML_t, "0", XML_r, "r", XML_b, "b");

    mpFS->startElementNS(XML_a, XML_pathLst);

    awt::Size aSize = rXShape->getSize();
    awt::Point aPos = rXShape->getPosition();
    Reference<XPropertySet> xPropertySet(rXShape, UNO_QUERY);
    uno::Reference<XPropertySetInfo> xPropertySetInfo = xPropertySet->getPropertySetInfo();
    if (xPropertySetInfo->hasPropertyByName("AnchorPosition"))
    {
        awt::Point aAnchorPosition;
        xPropertySet->getPropertyValue("AnchorPosition") >>= aAnchorPosition;
        aPos.X += aAnchorPosition.X;
        aPos.Y += aAnchorPosition.Y;
    }

    // Only closed SdrPathObj can be filled
    std::optional<OString> sFill;
    if (!bClosed)
        sFill = "none"; // for possible values see ST_PathFillMode in OOXML standard

    // Put all polygons of rPolyPolygon in the same path element
    // to subtract the overlapped areas.
    mpFS->startElementNS(XML_a, XML_path, XML_fill, sFill, XML_w, OString::number(aSize.Width),
                         XML_h, OString::number(aSize.Height));

    for (sal_uInt16 i = 0; i < aPolyPolygon.Count(); i++)
    {
        const tools::Polygon& aPoly = aPolyPolygon[i];

        if (aPoly.GetSize() > 0)
        {
            mpFS->startElementNS(XML_a, XML_moveTo);

            mpFS->singleElementNS(XML_a, XML_pt, XML_x, OString::number(aPoly[0].X() - aPos.X),
                                  XML_y, OString::number(aPoly[0].Y() - aPos.Y));

            mpFS->endElementNS(XML_a, XML_moveTo);
        }

        for (sal_uInt16 j = 1; j < aPoly.GetSize(); j++)
        {
            PolyFlags flags = aPoly.GetFlags(j);
            if (flags == PolyFlags::Control)
            {
                // a:cubicBezTo can only contain 3 a:pt elements, so we need to make sure of this
                if (j + 2 < aPoly.GetSize() && aPoly.GetFlags(j + 1) == PolyFlags::Control
                    && aPoly.GetFlags(j + 2) != PolyFlags::Control)
                {
                    mpFS->startElementNS(XML_a, XML_cubicBezTo);
                    for (sal_uInt8 k = 0; k <= 2; ++k)
                    {
                        mpFS->singleElementNS(XML_a, XML_pt, XML_x,
                                              OString::number(aPoly[j + k].X() - aPos.X), XML_y,
                                              OString::number(aPoly[j + k].Y() - aPos.Y));
                    }
                    mpFS->endElementNS(XML_a, XML_cubicBezTo);
                    j += 2;
                }
            }
            else if (flags == PolyFlags::Normal)
            {
                mpFS->startElementNS(XML_a, XML_lnTo);
                mpFS->singleElementNS(XML_a, XML_pt, XML_x, OString::number(aPoly[j].X() - aPos.X),
                                      XML_y, OString::number(aPoly[j].Y() - aPos.Y));
                mpFS->endElementNS(XML_a, XML_lnTo);
            }
        }
    }
    if (bClosed)
        mpFS->singleElementNS(XML_a, XML_close);
    mpFS->endElementNS(XML_a, XML_path);

    mpFS->endElementNS(XML_a, XML_pathLst);

    mpFS->endElementNS(XML_a, XML_custGeom);
}

void DrawingML::WriteConnectorConnections( sal_Int32 nStartGlueId, sal_Int32 nEndGlueId, sal_Int32 nStartID, sal_Int32 nEndID )
{
    if( nStartID != -1 )
    {
        mpFS->singleElementNS( XML_a, XML_stCxn,
                               XML_id, OString::number(nStartID),
                               XML_idx, OString::number(nStartGlueId) );
    }
    if( nEndID != -1 )
    {
        mpFS->singleElementNS( XML_a, XML_endCxn,
                               XML_id, OString::number(nEndID),
                               XML_idx, OString::number(nEndGlueId) );
    }
}

sal_Unicode DrawingML::SubstituteBullet( sal_Unicode cBulletId, css::awt::FontDescriptor& rFontDesc )
{
    if ( IsOpenSymbol(rFontDesc.Name) )
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
    const OUString& sContentType,
    const OUString& sRelationshipType,
    OUString* pRelationshipId )
{
    OUString sRelationshipId;
    if (xParentRelation.is())
        sRelationshipId = GetFB()->addRelation( xParentRelation, sRelationshipType, sRelativeStream );
    else
        sRelationshipId = GetFB()->addRelation( sRelationshipType, sRelativeStream );

    if( pRelationshipId )
        *pRelationshipId = sRelationshipId;

    sax_fastparser::FSHelperPtr p = GetFB()->openFragmentStreamWithSerializer( sFullStream, sContentType );

    return p;
}

void DrawingML::WriteFill(const Reference<XPropertySet>& xPropSet, const awt::Size& rSize)
{
    if ( !GetProperty( xPropSet, "FillStyle" ) )
        return;
    FillStyle aFillStyle( FillStyle_NONE );
    xPropSet->getPropertyValue( "FillStyle" ) >>= aFillStyle;

    // map full transparent background to no fill
    if (aFillStyle == FillStyle_SOLID)
    {
        OUString sFillTransparenceGradientName;

        if (GetProperty(xPropSet, "FillTransparenceGradientName")
            && (mAny >>= sFillTransparenceGradientName)
            && !sFillTransparenceGradientName.isEmpty()
            && GetProperty(xPropSet, "FillTransparenceGradient"))
        {
            // check if a fully transparent TransparenceGradient is used
            // use BGradient constructor & tooling here now
            const basegfx::BGradient aTransparenceGradient = model::gradient::getFromAny(mAny);
            basegfx::BColor aSingleColor;
            const bool bSingleColor(aTransparenceGradient.GetColorStops().isSingleColor(aSingleColor));
            const bool bCompletelyTransparent(bSingleColor && basegfx::fTools::equal(aSingleColor.luminance(), 1.0));

            if (bCompletelyTransparent)
            {
                aFillStyle = FillStyle_NONE;
            }
        }
        else if ( GetProperty( xPropSet, "FillTransparence" ) )
        {
            // check if a fully transparent FillTransparence is used
            sal_Int16 nVal = 0;
            xPropSet->getPropertyValue( "FillTransparence" ) >>= nVal;
            if ( nVal == 100 )
                aFillStyle = FillStyle_NONE;
        }
    }

    bool bUseBackground(false);
    if (GetProperty(xPropSet, "FillUseSlideBackground"))
        xPropSet->getPropertyValue("FillUseSlideBackground") >>= bUseBackground;

    switch( aFillStyle )
    {
    case FillStyle_SOLID :
        WriteSolidFill( xPropSet );
        break;
    case FillStyle_GRADIENT :
        WriteGradientFill( xPropSet );
        break;
    case FillStyle_BITMAP :
        WriteBlipFill( xPropSet, "FillBitmap", rSize );
        break;
    case FillStyle_HATCH :
        WritePattFill( xPropSet );
        break;
    case FillStyle_NONE:
        if (!bUseBackground) // attribute `useBgFill` will be written at parent p:sp shape
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
    for (const auto& rProp : aGrabBag)
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
            for (const auto& rOuterShdwProp : aOuterShdwProps)
            {
                if( rOuterShdwProp.Name == "algn" )
                {
                    OUString sVal;
                    rOuterShdwProp.Value >>= sVal;
                    aOuterShdwAttrList->add( XML_algn, sVal );
                }
                else if( rOuterShdwProp.Name == "blurRad" )
                {
                    sal_Int64 nVal = 0;
                    rOuterShdwProp.Value >>= nVal;
                    aOuterShdwAttrList->add( XML_blurRad, OString::number( nVal ) );
                }
                else if( rOuterShdwProp.Name == "dir" )
                {
                    sal_Int32 nVal = 0;
                    rOuterShdwProp.Value >>= nVal;
                    aOuterShdwAttrList->add( XML_dir, OString::number( nVal ) );
                }
                else if( rOuterShdwProp.Name == "dist" )
                {
                    sal_Int32 nVal = 0;
                    rOuterShdwProp.Value >>= nVal;
                    aOuterShdwAttrList->add( XML_dist, OString::number( nVal ) );
                }
                else if( rOuterShdwProp.Name == "kx" )
                {
                    sal_Int32 nVal = 0;
                    rOuterShdwProp.Value >>= nVal;
                    aOuterShdwAttrList->add( XML_kx, OString::number( nVal ) );
                }
                else if( rOuterShdwProp.Name == "ky" )
                {
                    sal_Int32 nVal = 0;
                    rOuterShdwProp.Value >>= nVal;
                    aOuterShdwAttrList->add( XML_ky, OString::number( nVal ) );
                }
                else if( rOuterShdwProp.Name == "rotWithShape" )
                {
                    sal_Int32 nVal = 0;
                    rOuterShdwProp.Value >>= nVal;
                    aOuterShdwAttrList->add( XML_rotWithShape, OString::number( nVal ) );
                }
                else if( rOuterShdwProp.Name == "sx" )
                {
                    sal_Int32 nVal = 0;
                    rOuterShdwProp.Value >>= nVal;
                    aOuterShdwAttrList->add( XML_sx, OString::number( nVal ) );
                }
                else if( rOuterShdwProp.Name == "sy" )
                {
                    sal_Int32 nVal = 0;
                    rOuterShdwProp.Value >>= nVal;
                    aOuterShdwAttrList->add( XML_sy, OString::number( nVal ) );
                }
                else if( rOuterShdwProp.Name == "rad" )
                {
                    sal_Int64 nVal = 0;
                    rOuterShdwProp.Value >>= nVal;
                    aOuterShdwAttrList->add( XML_rad, OString::number( nVal ) );
                }
                else if( rOuterShdwProp.Name == "endA" )
                {
                    sal_Int32 nVal = 0;
                    rOuterShdwProp.Value >>= nVal;
                    aOuterShdwAttrList->add( XML_endA, OString::number( nVal ) );
                }
                else if( rOuterShdwProp.Name == "endPos" )
                {
                    sal_Int32 nVal = 0;
                    rOuterShdwProp.Value >>= nVal;
                    aOuterShdwAttrList->add( XML_endPos, OString::number( nVal ) );
                }
                else if( rOuterShdwProp.Name == "fadeDir" )
                {
                    sal_Int32 nVal = 0;
                    rOuterShdwProp.Value >>= nVal;
                    aOuterShdwAttrList->add( XML_fadeDir, OString::number( nVal ) );
                }
                else if( rOuterShdwProp.Name == "stA" )
                {
                    sal_Int32 nVal = 0;
                    rOuterShdwProp.Value >>= nVal;
                    aOuterShdwAttrList->add( XML_stA, OString::number( nVal ) );
                }
                else if( rOuterShdwProp.Name == "stPos" )
                {
                    sal_Int32 nVal = 0;
                    rOuterShdwProp.Value >>= nVal;
                    aOuterShdwAttrList->add( XML_stPos, OString::number( nVal ) );
                }
                else if( rOuterShdwProp.Name == "grow" )
                {
                    sal_Int32 nVal = 0;
                    rOuterShdwProp.Value >>= nVal;
                    aOuterShdwAttrList->add( XML_grow, OString::number( nVal ) );
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
    return static_cast< sal_Int32 >(std::hypot(dX, dY) * 360);
}

static sal_Int32 lcl_CalculateDir(const double dX, const double dY)
{
    return (static_cast< sal_Int32 >(basegfx::rad2deg<60000>(atan2(dY,dX))) + 21600000) % 21600000;
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
                double dX = +0.0, dY = +0.0;
                sal_Int32 nBlur =0;
                rXPropSet->getPropertyValue( "ShadowXDistance" ) >>= dX;
                rXPropSet->getPropertyValue( "ShadowYDistance" ) >>= dY;
                rXPropSet->getPropertyValue( "ShadowBlur" ) >>= nBlur;

                Sequence< PropertyValue > aShadowAttribsGrabBag{
                    comphelper::makePropertyValue("dist", lcl_CalculateDist(dX, dY)),
                    comphelper::makePropertyValue("dir", lcl_CalculateDir(dX, dY)),
                    comphelper::makePropertyValue("blurRad", oox::drawingml::convertHmmToEmu(nBlur)),
                    comphelper::makePropertyValue("rotWithShape", false) //ooxml default is 'true', so must write it
                };

                Sequence< PropertyValue > aShadowGrabBag{
                    comphelper::makePropertyValue("Attribs", aShadowAttribsGrabBag),
                    comphelper::makePropertyValue("RgbClr", rXPropSet->getPropertyValue( "ShadowColor" )),
                    comphelper::makePropertyValue("RgbClrTransparency", rXPropSet->getPropertyValue( "ShadowTransparence" ))
                };

                WriteShapeEffect( u"outerShdw", aShadowGrabBag );
            }
            WriteSoftEdgeEffect(rXPropSet);
            mpFS->endElementNS(XML_a, XML_effectLst);
        }
    }
    else
    {
        for( auto& rOuterShdwProp : asNonConstRange(aOuterShdwProps) )
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


                for( auto& rAttribsProp : asNonConstRange(aAttribsProps) )
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
        for (const auto& rEffect : aEffects)
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

    Sequence< PropertyValue > aGlowAttribs{ comphelper::makePropertyValue(
        "rad", oox::drawingml::convertHmmToEmu(nRad)) };
    Sequence< PropertyValue > aGlowProps{
        comphelper::makePropertyValue("Attribs", aGlowAttribs),
        comphelper::makePropertyValue("RgbClr", rXPropSet->getPropertyValue("GlowEffectColor")),
        comphelper::makePropertyValue("RgbClrTransparency", rXPropSet->getPropertyValue("GlowEffectTransparency"))
    };
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

    css::uno::Sequence<css::beans::PropertyValue> aAttribs{ comphelper::makePropertyValue(
        "rad", oox::drawingml::convertHmmToEmu(nRad)) };
    css::uno::Sequence<css::beans::PropertyValue> aProps{ comphelper::makePropertyValue("Attribs",
                                                                                        aAttribs) };

    WriteShapeEffect(u"softEdge", aProps);
}

void DrawingML::Write3DEffects( const Reference< XPropertySet >& xPropSet, bool bIsText )
{
    // check existence of the grab bag
    if( !GetProperty( xPropSet, "InteropGrabBag" ) )
        return;

    // extract the relevant properties from the grab bag
    Sequence< PropertyValue > aGrabBag, aEffectProps, aLightRigProps, aShape3DProps;
    mAny >>= aGrabBag;

    auto pShapeProp = std::find_if( std::cbegin(aGrabBag), std::cend(aGrabBag),
        [bIsText](const PropertyValue& rProp)
        { return rProp.Name == (bIsText ? u"Text3DEffectProperties" : u"3DEffectProperties"); });
    if (pShapeProp != std::cend(aGrabBag))
    {
        Sequence< PropertyValue > a3DEffectProps;
        pShapeProp->Value >>= a3DEffectProps;
        for (const auto& r3DEffectProp : a3DEffectProps)
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
    for (const auto& rEffectProp : aEffectProps)
    {
        if( rEffectProp.Name == "prst" )
        {
            OUString sVal;
            rEffectProp.Value >>= sVal;
            aCameraAttrList->add(XML_prst, sVal);
        }
        else if( rEffectProp.Name == "fov" )
        {
            float fVal = 0;
            rEffectProp.Value >>= fVal;
            aCameraAttrList->add( XML_fov, OString::number( fVal * 60000 ) );
        }
        else if( rEffectProp.Name == "zoom" )
        {
            float fVal = 1;
            rEffectProp.Value >>= fVal;
            aCameraAttrList->add( XML_zoom, OString::number( fVal * 100000 ) );
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
            aCameraRotationAttrList->add( nToken, OString::number( nVal ) );
            bCameraRotationPresent = true;
        }
    }

    bool bLightRigRotationPresent = false;
    rtl::Reference<sax_fastparser::FastAttributeList> aLightRigAttrList = FastSerializerHelper::createAttrList();
    rtl::Reference<sax_fastparser::FastAttributeList> aLightRigRotationAttrList = FastSerializerHelper::createAttrList();
    for (const auto& rLightRigProp : aLightRigProps)
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
            aLightRigAttrList->add(nToken, sVal);
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
            aLightRigRotationAttrList->add( nToken, OString::number( nVal ) );
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
    for (const auto& rShape3DProp : aShape3DProps)
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
            aShape3DAttrList->add( nToken, OString::number( nVal ) );
        }
        else if( rShape3DProp.Name == "prstMaterial" )
        {
            OUString sVal;
            rShape3DProp.Value >>= sVal;
            aShape3DAttrList->add(XML_prstMaterial, sVal);
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
            for (const auto& rBevelProp : aBevelProps)
            {
                if( rBevelProp.Name == "w" || rBevelProp.Name == "h" )
                {
                    sal_Int32 nVal = 0, nToken = XML_none;
                    rBevelProp.Value >>= nVal;
                    if( rBevelProp.Name == "w" )
                        nToken = XML_w;
                    else if( rBevelProp.Name == "h" )
                        nToken = XML_h;
                    aBevelAttrList->add( nToken, OString::number( nVal ) );
                }
                else  if( rBevelProp.Name == "prst" )
                {
                    OUString sVal;
                    rBevelProp.Value >>= sVal;
                    aBevelAttrList->add(XML_prst, sVal);
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
        for (const auto& rExtrusionColorProp : aExtrusionColorProps)
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
        for (const auto& rContourColorProp : aContourColorProps)
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
    for (const auto& rAttr : aAttrs)
    {
        sal_Int32 nToken = ArtisticEffectProperties::getEffectToken( rAttr.Name );
        if( nToken != XML_none )
        {
            sal_Int32 nVal = 0;
            rAttr.Value >>= nVal;
            aAttrList->add( nToken, OString::number( nVal ) );
        }
        else if( rAttr.Name == "OriginalGraphic" )
        {
            Sequence< PropertyValue > aGraphic;
            rAttr.Value >>= aGraphic;
            Sequence< sal_Int8 > aGraphicData;
            OUString sGraphicId;
            for (const auto& rProp : aGraphic)
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
    auto& rGraphicExportCache = GraphicExportCache::get();

    OUString aId = rGraphicExportCache.findWdpID(rFileId);
    if (!aId.isEmpty())
        return OUStringToOString(aId, RTL_TEXTENCODING_UTF8);

    sal_Int32 nWdpImageCount = rGraphicExportCache.nextWdpImageCount();
    OUString sFileName = u"media/hdphoto"_ustr + OUString::number(nWdpImageCount) + u".wdp"_ustr;
    OUString sFragment = GetComponentDir() + u"/"_ustr + sFileName;
    Reference< XOutputStream > xOutStream = mpFB->openFragmentStream(sFragment, "image/vnd.ms-photo");
    xOutStream->writeBytes( rPictureData );
    xOutStream->closeOutput();

    aId = mpFB->addRelation(mpFS->getOutputStream(),
                            oox::getRelationship(Relationship::HDPHOTO),
                            Concat2View(GetRelationCompPrefix() + sFileName));

    rGraphicExportCache.addToWdpCache(rFileId, aId);

    return OUStringToOString(aId, RTL_TEXTENCODING_UTF8);
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
    for (const auto& rProp : propList)
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
    pDocPrAttrList->add(XML_id, OString::number(nDiagramId));
    OString sName = "Diagram" + OString::number(nDiagramId);
    pDocPrAttrList->add(XML_name, sName);

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

    OUString sRelationCompPrefix = GetRelationCompPrefix();

    // add data relation
    OUString dataFileName = "diagrams/data" + OUString::number(nDiagramId) + ".xml";
    OUString dataRelId =
        mpFB->addRelation(mpFS->getOutputStream(), oox::getRelationship(Relationship::DIAGRAMDATA),
                          Concat2View(sRelationCompPrefix + dataFileName));

    // add layout relation
    OUString layoutFileName = "diagrams/layout" + OUString::number(nDiagramId) + ".xml";
    OUString layoutRelId = mpFB->addRelation(mpFS->getOutputStream(),
                                              oox::getRelationship(Relationship::DIAGRAMLAYOUT),
                                              Concat2View(sRelationCompPrefix + layoutFileName));

    // add style relation
    OUString styleFileName = "diagrams/quickStyle" + OUString::number(nDiagramId) + ".xml";
    OUString styleRelId = mpFB->addRelation(mpFS->getOutputStream(),
                                              oox::getRelationship(Relationship::DIAGRAMQUICKSTYLE),
                                              Concat2View(sRelationCompPrefix + styleFileName));

    // add color relation
    OUString colorFileName = "diagrams/colors" + OUString::number(nDiagramId) + ".xml";
    OUString colorRelId = mpFB->addRelation(mpFS->getOutputStream(),
                                              oox::getRelationship(Relationship::DIAGRAMCOLORS),
                                              Concat2View(sRelationCompPrefix + colorFileName));

    OUString drawingFileName;
    if (drawingDom.is())
    {
        // add drawing relation
        drawingFileName = "diagrams/drawing" + OUString::number(nDiagramId) + ".xml";
        OUString drawingRelId = mpFB->addRelation(
            mpFS->getOutputStream(), oox::getRelationship(Relationship::DIAGRAMDRAWING),
            Concat2View(sRelationCompPrefix + drawingFileName));

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

    OUString sDir = GetComponentDir();

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
        aProps.setAnyProperty(PROP_RelId, uno::Any(sRelId.toInt32()));

        mpFB->addRelation(xOutStream, sType, Concat2View("../" + sFragment));

        OUString sDir = GetComponentDir();
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
            if (pObj->GetObjIdentifier() == SdrObjKind::CustomShape)
            {
                // Center of bounding box of the rotated shape
                const auto aSnapRectCenter(pObj->GetSnapRect().Center());
                aTopLeft.X = aSnapRectCenter.X() - nHalfWidth;
                aTopLeft.Y = aSnapRectCenter.Y() - nHalfHeight;
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
