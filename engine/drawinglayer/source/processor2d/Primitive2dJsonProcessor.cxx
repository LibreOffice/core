/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <drawinglayer/processor2d/Primitive2dJsonProcessor.hxx>

#include <rtl/string.hxx>
#include <tools/json_writer.hxx>

#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/pointarrayprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/PolygonHairlinePrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <drawinglayer/primitive2d/objectinfoprimitive2d.hxx>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <drawinglayer/primitive2d/hiddengeometryprimitive2d.hxx>
#include <drawinglayer/primitive2d/textdecoratedprimitive2d.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/attribute/fontattribute.hxx>
#include <drawinglayer/primitive2d/PolygonStrokePrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonStrokePrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolygonStrokeArrowPrimitive2D.hxx>
#include <drawinglayer/attribute/lineattribute.hxx>
#include <drawinglayer/attribute/strokeattribute.hxx>
#include <drawinglayer/attribute/linestartendattribute.hxx>
#include <drawinglayer/primitive2d/fillgradientprimitive2d.hxx>
#include <drawinglayer/primitive2d/PolyPolygonGradientPrimitive2D.hxx>
#include <drawinglayer/primitive2d/fillhatchprimitive2d.hxx>
#include <drawinglayer/primitive2d/PolyPolygonHatchPrimitive2D.hxx>
#include <drawinglayer/attribute/fillgradientattribute.hxx>
#include <drawinglayer/attribute/fillhatchattribute.hxx>
#include <drawinglayer/attribute/fillgraphicattribute.hxx>
#include <drawinglayer/primitive2d/fillgraphicprimitive2d.hxx>
#include <drawinglayer/primitive2d/PolyPolygonGraphicPrimitive2D.hxx>
#include <drawinglayer/primitive2d/graphicprimitive2d.hxx>
#include <drawinglayer/primitive2d/PolyPolygonRGBAPrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonAlphaGradientPrimitive2D.hxx>
#include <drawinglayer/primitive2d/BitmapAlphaPrimitive2D.hxx>
#include <drawinglayer/primitive2d/patternfillprimitive2d.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/transparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/modifiedcolorprimitive2d.hxx>
#include <drawinglayer/primitive2d/shadowprimitive2d.hxx>
#include <drawinglayer/primitive2d/glowprimitive2d.hxx>
#include <drawinglayer/primitive2d/backgroundcolorprimitive2d.hxx>

#include <basegfx/utils/bgradient.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>

#include <vcl/graph.hxx>
#include <vcl/gfxlink.hxx>
#include <vcl/vectorgraphicdata.hxx>
#include <vcl/cvtgrf.hxx>
#include <comphelper/base64.hxx>
#include <tools/stream.hxx>

using namespace drawinglayer::primitive2d;

namespace
{
// TODO: Move to a tool class or maybe BColor itself and reuse elsewhere
OString colorToHex(const basegfx::BColor& rColor)
{
    return "#" + Color(rColor).AsRGBHexString().toUtf8();
}
} // end anonymous namespace

namespace drawinglayer
{
/// Write graphic data as base64 data URL, preferring native browser-supported formats.
void Primitive2dJsonProcessor::writeGraphicData(const Graphic& rGraphic)
{
    // TODO: Probably better to send images separately
    if (rGraphic.IsGfxLink())
    {
        GfxLink aLink = rGraphic.GetGfxLink();
        GfxLinkType eType = aLink.GetType();
        const char* pMime = nullptr;

        switch (eType)
        {
            case GfxLinkType::NativeJpg:
                pMime = "data:image/jpeg;base64,";
                break;
            case GfxLinkType::NativePng:
                pMime = "data:image/png;base64,";
                break;
            case GfxLinkType::NativeSvg:
                pMime = "data:image/svg+xml;base64,";
                break;
            case GfxLinkType::NativeGif:
                pMime = "data:image/gif;base64,";
                break;
            case GfxLinkType::NativeWebp:
                pMime = "data:image/webp;base64,";
                break;
            default:
                break;
        }

        if (pMime)
        {
            css::uno::Sequence<sal_Int8> aSequence(
                reinterpret_cast<const sal_Int8*>(aLink.GetData()), aLink.GetDataSize());
            OStringBuffer aBase64(pMime);
            comphelper::Base64::encode(aBase64, aSequence);
            mrWriter.put("data", aBase64);
            return;
        }
    }

    // Fallback: convert to PNG
    // Maybe we can be smarter what to do here, probably don't need to full resolution if the image is 40MP
    SvMemoryStream aStream;
    if (GraphicConverter::Export(aStream, rGraphic, ConvertDataFormat::PNG) == ERRCODE_NONE)
    {
        css::uno::Sequence<sal_Int8> aSequence(static_cast<const sal_Int8*>(aStream.GetData()),
                                               aStream.Tell());
        OStringBuffer aBase64("data:image/png;base64,");
        comphelper::Base64::encode(aBase64, aSequence);
        mrWriter.put("data", aBase64);
    }
}

/// Write bitmap data as base64, preferring native format if the Bitmap has one.
void Primitive2dJsonProcessor::writeBitmapData(const Bitmap& rBitmap)
{
    writeGraphicData(Graphic(rBitmap));
}

void Primitive2dJsonProcessor::writeGradient(
    const drawinglayer::attribute::FillGradientAttribute& rGradient, std::string_view sNodeName)
{
    auto aGradientNode = mrWriter.startNode(sNodeName);

    switch (rGradient.getStyle())
    {
        case css::awt::GradientStyle_LINEAR:
            mrWriter.put("style", "linear");
            break;
        case css::awt::GradientStyle_AXIAL:
            mrWriter.put("style", "axial");
            break;
        case css::awt::GradientStyle_RADIAL:
            mrWriter.put("style", "radial");
            break;
        case css::awt::GradientStyle_ELLIPTICAL:
            mrWriter.put("style", "elliptical");
            break;
        case css::awt::GradientStyle_SQUARE:
            mrWriter.put("style", "square");
            break;
        case css::awt::GradientStyle_RECT:
            mrWriter.put("style", "rect");
            break;
        default:
            mrWriter.put("style", "unknown");
            break;
    }

    mrWriter.put("angle", rGradient.getAngle());
    mrWriter.put("border", rGradient.getBorder());
    mrWriter.put("offsetX", rGradient.getOffsetX());
    mrWriter.put("offsetY", rGradient.getOffsetY());

    if (rGradient.getSteps() > 0)
        mrWriter.put("steps", sal_Int64(rGradient.getSteps()));

    // Color stops
    const basegfx::BColorStops& rStops = rGradient.getColorStops();
    if (!rStops.empty())
    {
        auto aStopsArray = mrWriter.startArray("colorStops");
        for (const auto& rStop : rStops)
        {
            auto aStopNode = mrWriter.startStruct();
            mrWriter.put("offset", rStop.getStopOffset());
            mrWriter.put("color", colorToHex(rStop.getStopColor()));
        }
    }
}

void Primitive2dJsonProcessor::writeRangeScaled(std::string_view sName,
                                                const basegfx::B2DRange& rRange)
{
    auto aArray = mrWriter.startArray(sName);
    mrWriter.putSimpleValue(OUString::number(rRange.getMinX() * mfScaleFactor));
    mrWriter.putSimpleValue(OUString::number(rRange.getMinY() * mfScaleFactor));
    mrWriter.putSimpleValue(OUString::number(rRange.getMaxX() * mfScaleFactor));
    mrWriter.putSimpleValue(OUString::number(rRange.getMaxY() * mfScaleFactor));
}

void Primitive2dJsonProcessor::writeHatchScaled(
    const drawinglayer::attribute::FillHatchAttribute& rHatch)
{
    auto aHatchNode = mrWriter.startNode("hatch");

    switch (rHatch.getStyle())
    {
        case drawinglayer::attribute::HatchStyle::Single:
            mrWriter.put("style", "single");
            break;
        case drawinglayer::attribute::HatchStyle::Double:
            mrWriter.put("style", "double");
            break;
        case drawinglayer::attribute::HatchStyle::Triple:
            mrWriter.put("style", "triple");
            break;
    }

    mrWriter.put("distance", rHatch.getDistance() * mfScaleFactor);
    mrWriter.put("angle", rHatch.getAngle());
    mrWriter.put("color", colorToHex(rHatch.getColor()));

    if (rHatch.isFillBackground())
        mrWriter.put("fillBackground", true);
}

void Primitive2dJsonProcessor::writeFillGraphicScaled(
    const drawinglayer::attribute::FillGraphicAttribute& rFillGraphic)
{
    auto aFillNode = mrWriter.startNode("fillGraphic");
    mrWriter.put("tiling", rFillGraphic.getTiling());
    mrWriter.put("offsetX", rFillGraphic.getOffsetX());
    mrWriter.put("offsetY", rFillGraphic.getOffsetY());

    const basegfx::B2DRange& rRange = rFillGraphic.getGraphicRange();
    writeRangeScaled("graphicRange", rRange);
    writeGraphicData(rFillGraphic.getGraphic());
}

void Primitive2dJsonProcessor::writeLineAttributeScaled(
    const drawinglayer::attribute::LineAttribute& rLineAttribute)
{
    auto aLineNode = mrWriter.startNode("line");
    mrWriter.put("color", colorToHex(rLineAttribute.getColor()));
    mrWriter.put("width", rLineAttribute.getWidth() * mfScaleFactor);

    switch (rLineAttribute.getLineJoin())
    {
        case basegfx::B2DLineJoin::NONE:
            mrWriter.put("linejoin", "none");
            break;
        case basegfx::B2DLineJoin::Bevel:
            mrWriter.put("linejoin", "bevel");
            break;
        case basegfx::B2DLineJoin::Miter:
            mrWriter.put("linejoin", "miter");
            break;
        case basegfx::B2DLineJoin::Round:
            mrWriter.put("linejoin", "round");
            break;
        default:
            mrWriter.put("linejoin", "unknown");
            break;
    }

    switch (rLineAttribute.getLineCap())
    {
        case css::drawing::LineCap::LineCap_BUTT:
            mrWriter.put("linecap", "butt");
            break;
        case css::drawing::LineCap::LineCap_ROUND:
            mrWriter.put("linecap", "round");
            break;
        case css::drawing::LineCap::LineCap_SQUARE:
            mrWriter.put("linecap", "square");
            break;
        default:
            mrWriter.put("linecap", "unknown");
            break;
    }
}

void Primitive2dJsonProcessor::writeStrokeAttributeScaled(
    const drawinglayer::attribute::StrokeAttribute& rStroke)
{
    if (rStroke.isDefault())
        return;

    auto aStrokeNode = mrWriter.startNode("stroke");
    mrWriter.put("fullDotDashLen", rStroke.getFullDotDashLen() * mfScaleFactor);

    const auto& rDashArray = rStroke.getDotDashArray();
    if (!rDashArray.empty())
    {
        auto aDashArrayNode = mrWriter.startArray("dotDashArray");
        for (double fValue : rDashArray)
            mrWriter.putSimpleValue(OUString::number(fValue * mfScaleFactor));
    }
}

void Primitive2dJsonProcessor::writeArrowAttributeScaled(
    std::string_view sName, const drawinglayer::attribute::LineStartEndAttribute& rArrow)
{
    if (!rArrow.isActive())
        return;

    auto aArrowNode = mrWriter.startNode(sName);
    mrWriter.put("width", rArrow.getWidth() * mfScaleFactor);
    mrWriter.put("centered", rArrow.isCentered());

    // Arrow polygon is in arbitrary coordinates.
    mrWriter.put("path",
                 basegfx::utils::exportToSvgD(rArrow.getB2DPolyPolygon(), true, true, false));

    // Reference point in path coordinates (the point placed at the line endpoint).
    // Non-centered: top-center of the bounding box.
    // Centered: center of bbox.
    const basegfx::B2DRange aRange(rArrow.getB2DPolyPolygon().getB2DRange());
    mrWriter.put("tipX", aRange.getCenter().getX());
    mrWriter.put("tipY",
                 rArrow.isCentered() ? aRange.getCenter().getY() : aRange.getMinimum().getY());
}

void Primitive2dJsonProcessor::writeTextPortionScaled(
    const TextSimplePortionPrimitive2D& rPrimitive)
{
    writeMatrixScaled("matrix", rPrimitive.getTextTransform());

    // Extract font size from the text transform matrix scale
    const basegfx::B2DHomMatrix& rMatrix = rPrimitive.getTextTransform();
    const double fFontScaleY = std::hypot(rMatrix.get(1, 0), rMatrix.get(1, 1)) * mfScaleFactor;
    mrWriter.put("fontSize", fFontScaleY);
    mrWriter.put("text", rPrimitive.getText());
    mrWriter.put("textPosition", rPrimitive.getTextPosition());
    mrWriter.put("textLength", rPrimitive.getTextLength());
    mrWriter.put("fontcolor", colorToHex(rPrimitive.getFontColor()));

    if (rPrimitive.getTextFillColor() != COL_TRANSPARENT)
        mrWriter.put("fillcolor", colorToHex(rPrimitive.getTextFillColor().getBColor()));

    if (rPrimitive.getLetterSpacing() != 0)
        mrWriter.put("letterSpacing",
                     static_cast<double>(rPrimitive.getLetterSpacing()) * mfScaleFactor);

    // Font attributes
    const drawinglayer::attribute::FontAttribute& rFontAttr = rPrimitive.getFontAttribute();
    mrWriter.put("familyname", rFontAttr.getFamilyName());

    if (!rFontAttr.getStyleName().isEmpty())
        mrWriter.put("stylename", rFontAttr.getStyleName());

    mrWriter.put("weight", rFontAttr.getWeight());

    if (rFontAttr.getItalic())
        mrWriter.put("italic", true);
    if (rFontAttr.getOutline())
        mrWriter.put("outline", true);
    if (rFontAttr.getRTL())
        mrWriter.put("rtl", true);
    if (rFontAttr.getMonospaced())
        mrWriter.put("monospaced", true);

    // Handle character advance widths
    if (!rPrimitive.getDXArray().empty())
    {
        auto aDxArray = mrWriter.startArray("dxarray");
        for (double fValue : rPrimitive.getDXArray())
            mrWriter.putSimpleValue(OUString::number(fValue * mfScaleFactor));
    }
}

// Write matrix as 6 elements. Scale all components (unit conversion).
void Primitive2dJsonProcessor::writeMatrixScaled(std::string_view sName,
                                                 const basegfx::B2DHomMatrix& rMatrix)
{
    auto aArray = mrWriter.startArray(sName);
    mrWriter.putSimpleValue(OUString::number(rMatrix.get(0, 0) * mfScaleFactor));
    mrWriter.putSimpleValue(OUString::number(rMatrix.get(1, 0) * mfScaleFactor));
    mrWriter.putSimpleValue(OUString::number(rMatrix.get(0, 1) * mfScaleFactor));
    mrWriter.putSimpleValue(OUString::number(rMatrix.get(1, 1) * mfScaleFactor));
    mrWriter.putSimpleValue(OUString::number(rMatrix.get(0, 2) * mfScaleFactor));
    mrWriter.putSimpleValue(OUString::number(rMatrix.get(1, 2) * mfScaleFactor));
}

// Write matrix with scale applied only to translation (e, f).
// Use for TransformPrimitive2D / ShadowPrimitive2D where the matrix maps
// child-space -> parent-space and both spaces are in the same units.
// The scale/rotation components (a, b, c, d) are dimensionless ratios.
// Only the translation needs converted.
void Primitive2dJsonProcessor::writeMatrixTranslationScaled(std::string_view sName,
                                                            const basegfx::B2DHomMatrix& rMatrix)
{
    auto aArray = mrWriter.startArray(sName);
    mrWriter.putSimpleValue(OUString::number(rMatrix.get(0, 0)));
    mrWriter.putSimpleValue(OUString::number(rMatrix.get(1, 0)));
    mrWriter.putSimpleValue(OUString::number(rMatrix.get(0, 1)));
    mrWriter.putSimpleValue(OUString::number(rMatrix.get(1, 1)));
    mrWriter.putSimpleValue(OUString::number(rMatrix.get(0, 2) * mfScaleFactor));
    mrWriter.putSimpleValue(OUString::number(rMatrix.get(1, 2) * mfScaleFactor));
}

basegfx::B2DPolyPolygon
Primitive2dJsonProcessor::scalePolyPolygon(const basegfx::B2DPolyPolygon& rPolyPolygon)
{
    if (mfScaleFactor == 1.0)
        return rPolyPolygon;

    basegfx::B2DHomMatrix aScaleMatrix;
    aScaleMatrix.scale(mfScaleFactor, mfScaleFactor);
    basegfx::B2DPolyPolygon aScaled(rPolyPolygon);
    aScaled.transform(aScaleMatrix);
    return aScaled;
}

void Primitive2dJsonProcessor::writePathScaled(const basegfx::B2DPolyPolygon& rPolyPolygon)
{
    const basegfx::B2DPolyPolygon aScaled = scalePolyPolygon(rPolyPolygon);
    mrWriter.put("path", basegfx::utils::exportToSvgD(aScaled, true, true, false));

    const basegfx::B2DRange aRange(aScaled.getB2DRange());
    {
        auto aBoundsArray = mrWriter.startArray("bounds");
        mrWriter.putSimpleValue(OUString::number(aRange.getMinX()));
        mrWriter.putSimpleValue(OUString::number(aRange.getMinY()));
        mrWriter.putSimpleValue(OUString::number(aRange.getMaxX()));
        mrWriter.putSimpleValue(OUString::number(aRange.getMaxY()));
    }
}

void Primitive2dJsonProcessor::decomposeAndWrite(const Primitive2DContainer& rPrimitive2DSequence)
{
    for (const auto& rPrimitive : rPrimitive2DSequence)
    {
        const BasePrimitive2D* pBasePrimitive = rPrimitive.get();
        if (!pBasePrimitive)
            continue;

        auto aNode = mrWriter.startStruct();
        processPrimitive(*pBasePrimitive);
    }
}

OString Primitive2dJsonProcessor::dumpAsJson(const Primitive2DContainer& rPrimitive2DSequence)
{
    tools::JsonWriter aWriter;
    Primitive2dJsonProcessor aProcessor(aWriter);
    {
        auto aArray = aWriter.startArray("primitives");
        aProcessor.decomposeAndWrite(rPrimitive2DSequence);
    }
    return aWriter.finishAndGetAsOString();
}

void Primitive2dJsonProcessor::processPrimitive(const BasePrimitive2D& rBasePrimitive)
{
    sal_uInt32 nId = rBasePrimitive.getPrimitive2DID();

    switch (nId)
    {
        case PRIMITIVE2D_ID_POLYGONHAIRLINEPRIMITIVE2D:
        {
            auto const& rPrimitive = static_cast<const PolygonHairlinePrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "polygonHairline");
            mrWriter.put("color", colorToHex(rPrimitive.getBColor()));
            const basegfx::B2DPolyPolygon aScaled
                = scalePolyPolygon(basegfx::B2DPolyPolygon(rPrimitive.getB2DPolygon()));
            mrWriter.put("path", basegfx::utils::exportToSvgD(aScaled, true, true, false));
        }
        break;

        case PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D:
        {
            const auto& rPrimitive
                = static_cast<const PolyPolygonColorPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "polyPolygonColor");
            mrWriter.put("color", colorToHex(rPrimitive.getBColor()));
            writePathScaled(rPrimitive.getB2DPolyPolygon());
        }
        break;

        case PRIMITIVE2D_ID_BITMAPPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const BitmapPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "bitmap");
            writeMatrixScaled("matrix", rPrimitive.getTransform());

            const Bitmap& rBitmap = rPrimitive.getBitmap();
            const Size aSizePixel(rBitmap.GetSizePixel());
            mrWriter.put("width", aSizePixel.getWidth());
            mrWriter.put("height", aSizePixel.getHeight());
            mrWriter.put("checksum", static_cast<sal_Int64>(rBitmap.GetChecksum()));
            writeBitmapData(rBitmap);
        }
        break;

        case PRIMITIVE2D_ID_POINTARRAYPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const PointArrayPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "pointArray");
            mrWriter.put("color", colorToHex(rPrimitive.getRGBColor()));
            {
                auto aPointsArray = mrWriter.startArray("points");
                for (const auto& rPoint : rPrimitive.getPositions())
                {
                    auto aPointNode = mrWriter.startStruct();
                    mrWriter.put("x", rPoint.getX() * mfScaleFactor);
                    mrWriter.put("y", rPoint.getY() * mfScaleFactor);
                }
            }
        }
        break;

        case PRIMITIVE2D_ID_TRANSFORMPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const TransformPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "transform");
            writeMatrixTranslationScaled("matrix", rPrimitive.getTransformation());
            {
                auto aChildArray = mrWriter.startArray("children");
                decomposeAndWrite(rPrimitive.getChildren());
            }
        }
        break;

        case PRIMITIVE2D_ID_MASKPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const MaskPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "mask");
            const basegfx::B2DPolyPolygon aScaled = scalePolyPolygon(rPrimitive.getMask());
            mrWriter.put("clip", basegfx::utils::exportToSvgD(aScaled, true, true, false));
            {
                auto aChildArray = mrWriter.startArray("children");
                decomposeAndWrite(rPrimitive.getChildren());
            }
        }
        break;

        case PRIMITIVE2D_ID_GROUPPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const GroupPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "group");
            {
                auto aChildArray = mrWriter.startArray("children");
                decomposeAndWrite(rPrimitive.getChildren());
            }
        }
        break;

        case PRIMITIVE2D_ID_OBJECTINFOPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const ObjectInfoPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "objectInfo");
            mrWriter.put("name", rPrimitive.getName());
            mrWriter.put("title", rPrimitive.getTitle());
            mrWriter.put("desc", rPrimitive.getDesc());
            {
                auto aChildArray = mrWriter.startArray("children");
                decomposeAndWrite(rPrimitive.getChildren());
            }
        }
        break;

        case PRIMITIVE2D_ID_HIDDENGEOMETRYPRIMITIVE2D:
        {
            // Hidden geometry is for hit-testing only, not visual - skip entirely
            mrWriter.put("type", "hiddenGeometry");
        }
        break;

        case PRIMITIVE2D_ID_TEXTSIMPLEPORTIONPRIMITIVE2D:
        {
            const auto& rPrimitive
                = static_cast<const TextSimplePortionPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "textSimplePortion");
            writeTextPortionScaled(rPrimitive);
        }
        break;

        case PRIMITIVE2D_ID_TEXTDECORATEDPORTIONPRIMITIVE2D:
        {
            const auto& rPrimitive
                = static_cast<const TextDecoratedPortionPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "textDecoratedPortion");
            writeTextPortionScaled(rPrimitive);

            // Decoration properties
            if (rPrimitive.getFontUnderline() != TEXT_LINE_NONE)
            {
                mrWriter.put("underline", sal_Int32(rPrimitive.getFontUnderline()));
                mrWriter.put("underlineColor", colorToHex(rPrimitive.getTextlineColor()));
                if (rPrimitive.getUnderlineAbove())
                    mrWriter.put("underlineAbove", true);
            }

            if (rPrimitive.getFontOverline() != TEXT_LINE_NONE)
            {
                mrWriter.put("overline", sal_Int32(rPrimitive.getFontOverline()));
                mrWriter.put("overlineColor", colorToHex(rPrimitive.getOverlineColor()));
            }

            if (rPrimitive.getTextStrikeout() != TEXT_STRIKEOUT_NONE)
                mrWriter.put("strikeout", sal_Int32(rPrimitive.getTextStrikeout()));

            if (rPrimitive.getTextEmphasisMark() != TEXT_FONT_EMPHASIS_MARK_NONE)
            {
                mrWriter.put("emphasisMark", sal_Int32(rPrimitive.getTextEmphasisMark()));
                if (rPrimitive.getEmphasisMarkAbove())
                    mrWriter.put("emphasisMarkAbove", true);
                if (rPrimitive.getEmphasisMarkBelow())
                    mrWriter.put("emphasisMarkBelow", true);
            }

            if (rPrimitive.getTextRelief() != TEXT_RELIEF_NONE)
                mrWriter.put("relief", sal_Int32(rPrimitive.getTextRelief()));

            if (rPrimitive.getShadow())
                mrWriter.put("shadow", true);

            if (rPrimitive.getWordLineMode())
                mrWriter.put("wordLineMode", true);
        }
        break;

        case PRIMITIVE2D_ID_POLYGONSTROKEPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const PolygonStrokePrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "polygonStroke");
            const basegfx::B2DPolyPolygon aScaled
                = scalePolyPolygon(basegfx::B2DPolyPolygon(rPrimitive.getB2DPolygon()));
            mrWriter.put("path", basegfx::utils::exportToSvgD(aScaled, true, true, false));
            writeLineAttributeScaled(rPrimitive.getLineAttribute());
            writeStrokeAttributeScaled(rPrimitive.getStrokeAttribute());
        }
        break;

        case PRIMITIVE2D_ID_POLYPOLYGONSTROKEPRIMITIVE2D:
        {
            const auto& rPrimitive
                = static_cast<const PolyPolygonStrokePrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "polyPolygonStroke");
            writePathScaled(rPrimitive.getB2DPolyPolygon());
            writeLineAttributeScaled(rPrimitive.getLineAttribute());
            writeStrokeAttributeScaled(rPrimitive.getStrokeAttribute());
        }
        break;

        case PRIMITIVE2D_ID_POLYGONSTROKEARROWPRIMITIVE2D:
        {
            const auto& rPrimitive
                = static_cast<const PolygonStrokeArrowPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "polygonStrokeArrow");
            const basegfx::B2DPolyPolygon aScaled
                = scalePolyPolygon(basegfx::B2DPolyPolygon(rPrimitive.getB2DPolygon()));
            mrWriter.put("path", basegfx::utils::exportToSvgD(aScaled, true, true, false));
            writeLineAttributeScaled(rPrimitive.getLineAttribute());
            writeStrokeAttributeScaled(rPrimitive.getStrokeAttribute());
            writeArrowAttributeScaled("start", rPrimitive.getStart());
            writeArrowAttributeScaled("end", rPrimitive.getEnd());
        }
        break;

        case PRIMITIVE2D_ID_FILLGRADIENTPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const FillGradientPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "fillGradient");
            writeRangeScaled("outputRange", rPrimitive.getOutputRange());
            writeRangeScaled("definitionRange", rPrimitive.getDefinitionRange());
            writeGradient(rPrimitive.getFillGradient());

            if (rPrimitive.hasAlphaGradient())
                writeGradient(rPrimitive.getAlphaGradient(), "alphaGradient");

            if (rPrimitive.hasTransparency())
                mrWriter.put("transparency", rPrimitive.getTransparency());
        }
        break;

        case PRIMITIVE2D_ID_POLYPOLYGONGRADIENTPRIMITIVE2D:
        {
            const auto& rPrimitive
                = static_cast<const PolyPolygonGradientPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "polyPolygonGradient");
            writePathScaled(rPrimitive.getB2DPolyPolygon());
            writeRangeScaled("definitionRange", rPrimitive.getDefinitionRange());
            writeGradient(rPrimitive.getFillGradient());

            if (rPrimitive.hasAlphaGradient())
                writeGradient(rPrimitive.getAlphaGradient(), "alphaGradient");

            if (rPrimitive.hasTransparency())
                mrWriter.put("transparency", rPrimitive.getTransparency());
        }
        break;

        case PRIMITIVE2D_ID_FILLHATCHPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const FillHatchPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "fillHatch");
            writeRangeScaled("outputRange", rPrimitive.getOutputRange());
            writeRangeScaled("definitionRange", rPrimitive.getDefinitionRange());
            mrWriter.put("backgroundColor", colorToHex(rPrimitive.getBColor()));
            writeHatchScaled(rPrimitive.getFillHatch());
        }
        break;

        case PRIMITIVE2D_ID_POLYPOLYGONHATCHPRIMITIVE2D:
        {
            const auto& rPrimitive
                = static_cast<const PolyPolygonHatchPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "polyPolygonHatch");
            writePathScaled(rPrimitive.getB2DPolyPolygon());
            writeRangeScaled("definitionRange", rPrimitive.getDefinitionRange());
            mrWriter.put("backgroundColor", colorToHex(rPrimitive.getBackgroundColor()));
            writeHatchScaled(rPrimitive.getFillHatch());
        }
        break;

        case PRIMITIVE2D_ID_FILLGRAPHICPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const FillGraphicPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "fillGraphic");
            writeMatrixScaled("matrix", rPrimitive.getTransformation());
            writeFillGraphicScaled(rPrimitive.getFillGraphic());

            if (rPrimitive.hasTransparency())
                mrWriter.put("transparency", rPrimitive.getTransparency());
        }
        break;

        case PRIMITIVE2D_ID_POLYPOLYGONGRAPHICPRIMITIVE2D:
        {
            const auto& rPrimitive
                = static_cast<const PolyPolygonGraphicPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "polyPolygonGraphic");
            writePathScaled(rPrimitive.getB2DPolyPolygon());
            writeRangeScaled("definitionRange", rPrimitive.getDefinitionRange());
            writeFillGraphicScaled(rPrimitive.getFillGraphic());

            if (rPrimitive.hasTransparency())
                mrWriter.put("transparency", rPrimitive.getTransparency());
        }
        break;

        case PRIMITIVE2D_ID_GRAPHICPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const GraphicPrimitive2D&>(rBasePrimitive);
            const Graphic& rGraphic = rPrimitive.getGraphicObject().GetGraphic();

            // Handle SVG images specially - send without conversion as native image data.
            if (auto pVectorData = rGraphic.getVectorGraphicData())
            {
                if (pVectorData->getType() == VectorGraphicDataType::Svg)
                {
                    mrWriter.put("type", "graphic");
                    writeMatrixScaled("matrix", rPrimitive.getTransform());
                    writeGraphicData(rGraphic);
                    break;
                }
            }

            // Non-SVG vector graphics should be decomposed into primitives.
            // The decomposition already applies the graphic's transform matrix, so
            // children are in the same coordinate space as all other primitives.
            if (rGraphic.GetType() == GraphicType::GdiMetafile || rGraphic.getVectorGraphicData())
            {
                mrWriter.put("type", "graphic");
                mrWriter.put("vector", true);

                Primitive2DContainer aContainer;
                rPrimitive.get2DDecomposition(aContainer, maViewInformation2D);
                if (!aContainer.empty())
                {
                    auto aChildArray = mrWriter.startArray("children");
                    decomposeAndWrite(aContainer);
                }
                break;
            }

            // Bitmap graphic: encode as image data
            mrWriter.put("type", "graphic");
            writeMatrixScaled("matrix", rPrimitive.getTransform());

            const GraphicAttr& rAttributes = rPrimitive.getGraphicAttr();
            if (rAttributes.IsCropped())
            {
                auto aCropNode = mrWriter.startNode("crop");
                mrWriter.put("left", double(rAttributes.GetLeftCrop()) * mfScaleFactor);
                mrWriter.put("top", double(rAttributes.GetTopCrop()) * mfScaleFactor);
                mrWriter.put("right", double(rAttributes.GetRightCrop()) * mfScaleFactor);
                mrWriter.put("bottom", double(rAttributes.GetBottomCrop()) * mfScaleFactor);
            }

            if (rAttributes.IsTransparent())
                mrWriter.put("alpha", sal_Int32(rAttributes.GetAlpha()));

            if (rAttributes.GetRotation() != 0_deg10)
                mrWriter.put("rotation", rAttributes.GetRotation().get());

            if (rAttributes.IsMirrored())
                mrWriter.put("mirror", sal_Int32(rAttributes.GetMirrorFlags()));

            if (rAttributes.IsSpecialDrawMode())
            {
                switch (rAttributes.GetDrawMode())
                {
                    case GraphicDrawMode::Greys:
                        mrWriter.put("drawMode", "greys");
                        break;
                    case GraphicDrawMode::Mono:
                        mrWriter.put("drawMode", "mono");
                        break;
                    case GraphicDrawMode::Watermark:
                        mrWriter.put("drawMode", "watermark");
                        break;
                    default:
                        break;
                }
            }

            const Size aSizePixel(rGraphic.GetSizePixel());
            mrWriter.put("width", sal_Int64(aSizePixel.getWidth()));
            mrWriter.put("height", sal_Int64(aSizePixel.getHeight()));
            writeGraphicData(rGraphic);
        }
        break;

        case PRIMITIVE2D_ID_POLYPOLYGONRGBAPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const PolyPolygonRGBAPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "polyPolygonRGBA");
            mrWriter.put("color", colorToHex(rPrimitive.getBColor()));
            writePathScaled(rPrimitive.getB2DPolyPolygon());

            if (rPrimitive.hasTransparency())
                mrWriter.put("transparency", rPrimitive.getTransparency());
        }
        break;

        case PRIMITIVE2D_ID_POLYPOLYGONALPHAGRADIENTPRIMITIVE2D:
        {
            const auto& rPrimitive
                = static_cast<const PolyPolygonAlphaGradientPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "polyPolygonAlphaGradient");
            mrWriter.put("color", colorToHex(rPrimitive.getBColor()));
            writePathScaled(rPrimitive.getB2DPolyPolygon());
            writeGradient(rPrimitive.getAlphaGradient(), "alphaGradient");
        }
        break;

        case PRIMITIVE2D_ID_BITMAPALPHAPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const BitmapAlphaPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "bitmapAlpha");
            writeMatrixScaled("matrix", rPrimitive.getTransform());

            if (rPrimitive.hasTransparency())
                mrWriter.put("transparency", rPrimitive.getTransparency());

            const Bitmap& rBitmap = rPrimitive.getBitmap();
            const Size aSizePixel(rBitmap.GetSizePixel());
            mrWriter.put("width", aSizePixel.getWidth());
            mrWriter.put("height", aSizePixel.getHeight());
            mrWriter.put("checksum", sal_Int64(rBitmap.GetChecksum()));
            writeBitmapData(rBitmap);
        }
        break;

        case PRIMITIVE2D_ID_PATTERNFILLPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const PatternFillPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "patternFill");
            writePathScaled(rPrimitive.getMask());

            writeRangeScaled("referenceRange", rPrimitive.getReferenceRange());

            const Primitive2DContainer& rChildren = rPrimitive.getChildren();
            if (!rChildren.empty())
            {
                double fSavedScale = mfScaleFactor;
                mfScaleFactor = 1.0;
                auto aChildArray = mrWriter.startArray("children");
                decomposeAndWrite(rChildren);
                mfScaleFactor = fSavedScale;
            }
        }
        break;

        case PRIMITIVE2D_ID_UNIFIEDTRANSPARENCEPRIMITIVE2D:
        {
            const auto& rPrimitive
                = static_cast<const UnifiedTransparencePrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "unifiedTransparence");
            mrWriter.put("transparence", rPrimitive.getTransparence());
            {
                auto aChildArray = mrWriter.startArray("children");
                decomposeAndWrite(rPrimitive.getChildren());
            }
        }
        break;

        case PRIMITIVE2D_ID_TRANSPARENCEPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const TransparencePrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "transparence");
            {
                auto aChildArray = mrWriter.startArray("children");
                decomposeAndWrite(rPrimitive.getChildren());
            }
            {
                auto aTranspArray = mrWriter.startArray("transparence");
                decomposeAndWrite(rPrimitive.getTransparence());
            }
        }
        break;

        case PRIMITIVE2D_ID_MODIFIEDCOLORPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const ModifiedColorPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "modifiedColor");
            const basegfx::BColorModifierSharedPtr& rModifier = rPrimitive.getColorModifier();
            mrWriter.put("modifier", rModifier->getModifierName());

            // Output modifier-specific parameters so the client can apply the modification
            if (rModifier->getBColorModifierType() == basegfx::BColorModifierType::BCMType_replace)
            {
                const auto& rReplace
                    = static_cast<const basegfx::BColorModifier_replace&>(*rModifier);
                mrWriter.put("color", colorToHex(rReplace.getBColor()));
            }

            {
                auto aChildArray = mrWriter.startArray("children");
                decomposeAndWrite(rPrimitive.getChildren());
            }
        }
        break;

        case PRIMITIVE2D_ID_SHADOWPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const ShadowPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "shadow");
            mrWriter.put("color", colorToHex(rPrimitive.getShadowColor()));
            mrWriter.put("blur", rPrimitive.getShadowBlur() * mfScaleFactor);
            writeMatrixTranslationScaled("matrix", rPrimitive.getShadowTransform());
            {
                auto aChildArray = mrWriter.startArray("children");
                decomposeAndWrite(rPrimitive.getChildren());
            }
        }
        break;

        case PRIMITIVE2D_ID_GLOWPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const GlowPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "glow");
            mrWriter.put("color", colorToHex(rPrimitive.getGlowColor().getBColor()));
            mrWriter.put("radius", rPrimitive.getGlowRadius() * mfScaleFactor);
            {
                auto aChildArray = mrWriter.startArray("children");
                decomposeAndWrite(rPrimitive.getChildren());
            }
        }
        break;

        case PRIMITIVE2D_ID_BACKGROUNDCOLORPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const BackgroundColorPrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "backgroundcolor");
            mrWriter.put("color", colorToHex(rPrimitive.getBColor()));
            if (rPrimitive.getTransparency() > 0.0)
                mrWriter.put("transparency", rPrimitive.getTransparency());
        }
        break;

        case PRIMITIVE2D_ID_FILLEDRECTANGLEPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const FilledRectanglePrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "filledRectangle");
            mrWriter.put("color", colorToHex(rPrimitive.getBColor()));
            const basegfx::B2DRange& rRange = rPrimitive.getB2DRange();
            auto aBoundsArray = mrWriter.startArray("bounds");
            mrWriter.putSimpleValue(OUString::number(rRange.getMinX() * mfScaleFactor));
            mrWriter.putSimpleValue(OUString::number(rRange.getMinY() * mfScaleFactor));
            mrWriter.putSimpleValue(OUString::number(rRange.getMaxX() * mfScaleFactor));
            mrWriter.putSimpleValue(OUString::number(rRange.getMaxY() * mfScaleFactor));
        }
        break;

        case PRIMITIVE2D_ID_LINERECTANGLEPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const LineRectanglePrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "lineRectangle");
            mrWriter.put("color", colorToHex(rPrimitive.getBColor()));
            const basegfx::B2DRange& rRange = rPrimitive.getB2DRange();
            auto aBoundsArray = mrWriter.startArray("bounds");
            mrWriter.putSimpleValue(OUString::number(rRange.getMinX() * mfScaleFactor));
            mrWriter.putSimpleValue(OUString::number(rRange.getMinY() * mfScaleFactor));
            mrWriter.putSimpleValue(OUString::number(rRange.getMaxX() * mfScaleFactor));
            mrWriter.putSimpleValue(OUString::number(rRange.getMaxY() * mfScaleFactor));
        }
        break;

        case PRIMITIVE2D_ID_SINGLELINEPRIMITIVE2D:
        {
            const auto& rPrimitive = static_cast<const SingleLinePrimitive2D&>(rBasePrimitive);
            mrWriter.put("type", "singleLine");
            mrWriter.put("color", colorToHex(rPrimitive.getBColor()));
            mrWriter.put("startX", rPrimitive.getStart().getX() * mfScaleFactor);
            mrWriter.put("startY", rPrimitive.getStart().getY() * mfScaleFactor);
            mrWriter.put("endX", rPrimitive.getEnd().getX() * mfScaleFactor);
            mrWriter.put("endY", rPrimitive.getEnd().getY() * mfScaleFactor);
        }
        break;

        default:
        {
            const char* pTypeName = nullptr;
            switch (nId)
            {
                // SVG gradient decomposition
                case PRIMITIVE2D_ID_SVGLINEARGRADIENTPRIMITIVE2D:
                    pTypeName = "svgLinearGradient";
                    break;
                case PRIMITIVE2D_ID_SVGRADIALGRADIENTPRIMITIVE2D:
                    pTypeName = "svgRadialGradient";
                    break;
                case PRIMITIVE2D_ID_SVGLINEARATOMPRIMITIVE2D:
                    pTypeName = "svgLinearAtom";
                    break;
                case PRIMITIVE2D_ID_SVGRADIALATOMPRIMITIVE2D:
                    pTypeName = "svgRadialAtom";
                    break;
                // Text hierarchy (structural text grouping)
                case PRIMITIVE2D_ID_TEXTHIERARCHYFIELDPRIMITIVE2D:
                    pTypeName = "textHierarchyField";
                    break;
                case PRIMITIVE2D_ID_TEXTHIERARCHYLINEPRIMITIVE2D:
                    pTypeName = "textHierarchyLine";
                    break;
                case PRIMITIVE2D_ID_TEXTHIERARCHYPARAGRAPHPRIMITIVE2D:
                    pTypeName = "textHierarchyParagraph";
                    break;
                case PRIMITIVE2D_ID_TEXTHIERARCHYBLOCKPRIMITIVE2D:
                    pTypeName = "textHierarchyBlock";
                    break;
                case PRIMITIVE2D_ID_TEXTHIERARCHYBULLETPRIMITIVE2D:
                    pTypeName = "textHierarchyBullet";
                    break;
                case PRIMITIVE2D_ID_TEXTHIERARCHYEMPHASISMARKPRIMITIVE2D:
                    pTypeName = "textHierarchyEmphasisMark";
                    break;
                // Other drawinglayer primitives
                case PRIMITIVE2D_ID_SOFTEDGEPRIMITIVE2D:
                    pTypeName = "softEdge";
                    break;
                case PRIMITIVE2D_ID_EXCLUSIVEEDITVIEWPRIMITIVE2D:
                    pTypeName = "exclusiveEditView";
                    break;
                default:
                    break;
            }

            if (pTypeName)
            {
                mrWriter.put("type", pTypeName);
            }
            else
            {
                // Encode module and local ID so we know the origin
                // Modules: 0=drawinglayer, 1=svx, 2=sd, 3=sw, 4=sc
                static constexpr auto aModuleNames
                    = std::to_array<std::string_view>({ "drawinglayer", "svx", "sd", "sw", "sc" });
                sal_uInt32 nModule = nId >> 16;
                sal_uInt32 nLocalId = nId & 0xFFFF;
                std::string_view aModule
                    = nModule < aModuleNames.size() ? aModuleNames[nModule] : "other";
                mrWriter.put("type", rtl::Concat2View(OString::Concat(aModule) + ":"
                                                      + OString::number(sal_Int32(nLocalId))));
            }

            Primitive2DContainer aContainer;
            rBasePrimitive.get2DDecomposition(aContainer, maViewInformation2D);
            if (!aContainer.empty())
            {
                auto aChildArray = mrWriter.startArray("children");
                decomposeAndWrite(aContainer);
            }
        }
        break;
    }
}

} // end namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
