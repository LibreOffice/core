/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>
#include <drawinglayer/drawinglayerdllapi.h>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/range/b2drange.hxx>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <tools/color.hxx>

#include <functional>
#include <optional>
#include <set>
#include <unordered_map>
#include <vector>

#include <vcl/BinaryDataContainer.hxx>

class Bitmap;
class Graphic;
class GraphicAttr;

namespace basegfx
{
class B2DHomMatrix;
class BColorModifier;
}
namespace tools
{
class JsonWriter;
}
namespace drawinglayer::primitive2d
{
class BasePrimitive2D;
class TextSimplePortionPrimitive2D;
class Primitive2DContainer;
class SvgGradientHelper;
}

namespace drawinglayer::attribute
{
class FillGradientAttribute;
class FillHatchAttribute;
class FillGraphicAttribute;
class LineAttribute;
class StrokeAttribute;
}

namespace drawinglayer
{
/** The font attributes of text portions (family, cut, size, color, resolved face), each kept once
    under an id. An id stays valid for the life of the table, so content written at different times
    can name it.
 */
class DRAWINGLAYER_DLLPUBLIC TextStyleTable
{
public:
    /// The id of the style with these fields. A style that is not in the table yet is added.
    sal_Int32 idOf(const OString& rFields);
    /// The fields of the style with the id.
    const OString& fieldsOf(sal_Int32 nId) const { return maFields[nId]; }

private:
    std::unordered_map<OString, sal_Int32> maIds;
    std::vector<OString> maFields;
};

/** Primitive processor that converts primitives into a JSON resolution independent representation. */
class DRAWINGLAYER_DLLPUBLIC Primitive2dJsonProcessor
{
public:
    explicit Primitive2dJsonProcessor(tools::JsonWriter& rWriter)
        : mrWriter(rWriter)
    {
    }

    /// Set a scale factor applied to all coordinates for unit conversion.
    void setScaleFactor(double fScale) { mfScaleFactor = fScale; }
    double getScaleFactor() const { return mfScaleFactor; }

    /// Set ViewInformation2D (needed for field resolution)
    void setViewInformation2D(const drawinglayer::geometry::ViewInformation2D& rViewInfo)
    {
        maViewInformation2D = rViewInfo;
    }

    void
    decomposeAndWrite(const drawinglayer::primitive2d::Primitive2DContainer& rPrimitive2DSequence);

    static OString
    dumpAsJson(const drawinglayer::primitive2d::Primitive2DContainer& rPrimitive2DSequence);

    /// Write a Graphic as base64 data URL to a JsonWriter. A graphic that cannot be encoded gets
    /// an error attribute naming the reason instead of data.
    static void writeGraphicBase64(tools::JsonWriter& rWriter, const Graphic& rGraphic);

    /// Set an external bitmap cache. Graphics encountered during
    /// processing are stored here keyed by their checksum.
    void setBitmapCache(std::unordered_map<sal_Int64, Graphic>& rCache);

    /// What a font key resolved to: the id of the face, and whether it falls
    /// short of the weight or slant asked for.
    struct ResolvedFace
    {
        sal_uInt64 mnFontId = 0;
        bool mbSyntheticBold = false;
        bool mbSyntheticItalic = false;
    };

    /// External caches: the font files text portions use, keyed by a
    /// content hash of the bytes, and what each font key resolved to, so a
    /// face is read, hashed and measured once.
    void setFontCache(std::unordered_map<sal_uInt64, BinaryDataContainer>& rCache,
                      std::unordered_map<OUString, ResolvedFace>& rFaceByKey);

    /// Writes the type and fields of a primitive defined outside drawinglayer, or returns false for
    /// a primitive it does not know. Its decomposition is written as children either way.
    using ExternalPrimitiveWriter
        = std::function<bool(const drawinglayer::primitive2d::BasePrimitive2D&, tools::JsonWriter&,
                             double fScaleFactor)>;

    void setExternalPrimitiveWriter(ExternalPrimitiveWriter aWriter)
    {
        maExternalPrimitiveWriter = std::move(aWriter);
    }

    /// Write text portions compact. A portion names its font attributes in the table by id as
    /// "style". Text that is not rotated, sheared or stretched is placed by "x" and "y" in whole
    /// units. A window into the text that shows all of it is left out.
    void setTextStyles(TextStyleTable* pTable) { mpTextStyles = pTable; }
    /// The ids of the styles that the portions written so far name.
    const std::set<sal_Int32>& getUsedTextStyles() const { return maUsedTextStyles; }

private:
    void processPrimitive(const drawinglayer::primitive2d::BasePrimitive2D& rPrimitive);

    /// Write matrix with all 6 components scaled
    void writeMatrixScaled(std::string_view sName, const basegfx::B2DHomMatrix& rMatrix);

    /// Write matrix with scale applied only to translation (e, f)
    void writeMatrixTranslationScaled(std::string_view sName, const basegfx::B2DHomMatrix& rMatrix);

    /// Scale a polypolygon by the scale factor
    basegfx::B2DPolyPolygon scalePolyPolygon(const basegfx::B2DPolyPolygon& rPolyPolygon);

    /// Write path and bounds, scaled
    void writePathScaled(const basegfx::B2DPolyPolygon& rPolyPolygon);

    void writeRange(std::string_view sName, const basegfx::B2DRange& rRange, double fScale);
    /// Write the range the primitive covers as "bounds", scaled. An empty range is left out.
    void writeBoundsScaled(const drawinglayer::primitive2d::BasePrimitive2D& rPrimitive);
    void writeHatchScaled(const drawinglayer::attribute::FillHatchAttribute& rHatch);
    void writeFillGraphic(const drawinglayer::attribute::FillGraphicAttribute& rFillGraphic);
    void writeLineAttributeScaled(const drawinglayer::attribute::LineAttribute& rLineAttribute);
    void writeStrokeAttributeScaled(const drawinglayer::attribute::StrokeAttribute& rStroke);
    /// Write the modifier's name and its parameters into the open node.
    void writeColorModifier(const basegfx::BColorModifier& rModifier);
    /// Write a graphic's draw mode and adjustments as color modifiers, in order. None if unchanged.
    void writeGraphicColorModifiers(const GraphicAttr& rAttributes);
    void
    writeTextPortionScaled(const drawinglayer::primitive2d::TextSimplePortionPrimitive2D& rPrim);
    /// Write the font attributes of a portion: its size, colors, family, cut and face.
    void writeTextStyleFields(tools::JsonWriter& rWriter,
                              const drawinglayer::primitive2d::TextSimplePortionPrimitive2D& rPrim,
                              double fFontSize);

    void writeSvgGradientCommon(const drawinglayer::primitive2d::SvgGradientHelper& rGradient);
    void writeGraphicData(const Graphic& rGraphic);
    void writeBitmapData(const Bitmap& rBitmap);
    void writeGradient(const drawinglayer::attribute::FillGradientAttribute& rGradient,
                       std::string_view sNodeName = "gradient");

    tools::JsonWriter& mrWriter;
    std::unordered_map<sal_Int64, Graphic>* mpBitmapCache = nullptr;
    std::unordered_map<sal_uInt64, BinaryDataContainer>* mpFontCache = nullptr;
    std::unordered_map<OUString, ResolvedFace>* mpFaceByKey = nullptr;
    double mfScaleFactor = 1.0;
    drawinglayer::geometry::ViewInformation2D maViewInformation2D;
    /// The fill right under the text of the text block being written, and the share of what
    /// lies behind that fill that shows through. The color is empty outside a text block, and
    /// inside one that sits on whatever the page shows there.
    std::optional<Color> moTextSurface;
    double mfTextSurfaceTransparency = 0.0;
    ExternalPrimitiveWriter maExternalPrimitiveWriter;
    TextStyleTable* mpTextStyles = nullptr;
    std::set<sal_Int32> maUsedTextStyles;
};

} // namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
