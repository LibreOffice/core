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

#include <unordered_map>

class Bitmap;
class Graphic;

namespace basegfx
{
class B2DHomMatrix;
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
class LineStartEndAttribute;
}

namespace drawinglayer
{
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

    /// Write a Graphic as base64 data URL to a JsonWriter.
    static void writeGraphicBase64(tools::JsonWriter& rWriter, const Graphic& rGraphic);

    /// Set an external bitmap cache. Graphics encountered during
    /// processing are stored here keyed by their checksum.
    void setBitmapCache(std::unordered_map<sal_Int64, Graphic>& rCache);

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

    void writeRangeScaled(std::string_view sName, const basegfx::B2DRange& rRange);
    void writeHatchScaled(const drawinglayer::attribute::FillHatchAttribute& rHatch);
    void writeFillGraphicScaled(const drawinglayer::attribute::FillGraphicAttribute& rFillGraphic);
    void writeLineAttributeScaled(const drawinglayer::attribute::LineAttribute& rLineAttribute);
    void writeStrokeAttributeScaled(const drawinglayer::attribute::StrokeAttribute& rStroke);
    void writeArrowAttributeScaled(std::string_view sName,
                                   const drawinglayer::attribute::LineStartEndAttribute& rArrow);
    void
    writeTextPortionScaled(const drawinglayer::primitive2d::TextSimplePortionPrimitive2D& rPrim);

    void writeSvgGradientCommon(const drawinglayer::primitive2d::SvgGradientHelper& rGradient);
    void writeGraphicData(const Graphic& rGraphic);
    void writeBitmapData(const Bitmap& rBitmap);
    void writeGradient(const drawinglayer::attribute::FillGradientAttribute& rGradient,
                       std::string_view sNodeName = "gradient");

    tools::JsonWriter& mrWriter;
    std::unordered_map<sal_Int64, Graphic>* mpBitmapCache = nullptr;
    double mfScaleFactor = 1.0;
    drawinglayer::geometry::ViewInformation2D maViewInformation2D;
};

} // namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
