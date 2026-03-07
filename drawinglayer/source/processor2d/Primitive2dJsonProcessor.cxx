/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
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

#include <basegfx/utils/bgradient.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>

#include <vcl/graph.hxx>
#include <vcl/gfxlink.hxx>
#include <vcl/vectorgraphicdata.hxx>
#include <vcl/cvtgrf.hxx>
#include <comphelper/base64.hxx>
#include <tools/stream.hxx>

using namespace drawinglayer::primitive2d;

namespace drawinglayer
{
namespace
{
// TODO: Move to a tool class or maybe BColor itself and reuse elsewhere
OString colorToHex(const basegfx::BColor& rColor)
{
    return "#" + Color(rColor).AsRGBHexString().toUtf8();
}

/// Write graphic data as base64 data URL, preferring native browser-supported formats.
void writeGraphicData(tools::JsonWriter& rWriter, const Graphic& rGraphic)
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
            rWriter.put("data", aBase64);
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
        rWriter.put("data", aBase64);
    }
}

/// Write bitmap data as base64, preferring native format if the Bitmap has one.
void writeBitmapData(tools::JsonWriter& rWriter, const Bitmap& rBitmap)
{
    writeGraphicData(rWriter, Graphic(rBitmap));
}

} // end anonymous namespace

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
            writeBitmapData(mrWriter, rBitmap);
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

        default:
        {
            const char* pTypeName = nullptr;
            switch (nId)
            {
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
