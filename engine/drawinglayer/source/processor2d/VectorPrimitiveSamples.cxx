/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <drawinglayer/processor2d/VectorPrimitiveSamples.hxx>

#include <basegfx/color/bcolor.hxx>
#include <basegfx/color/bcolormodifier.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/range/b2drange.hxx>
#include <com/sun/star/drawing/LineCap.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <drawinglayer/attribute/fontattribute.hxx>
#include <drawinglayer/attribute/lineattribute.hxx>
#include <drawinglayer/attribute/linestartendattribute.hxx>
#include <drawinglayer/attribute/strokeattribute.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/primitive2d/BitmapAlphaPrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonRGBAPrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonStrokePrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolygonHairlinePrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolygonStrokeArrowPrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolygonStrokePrimitive2D.hxx>
#include <drawinglayer/primitive2d/Primitive2DContainer.hxx>
#include <drawinglayer/primitive2d/ReflectionPrimitive2D.hxx>
#include <drawinglayer/primitive2d/backgroundcolorprimitive2d.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/exclusiveeditviewprimitive2d.hxx>
#include <drawinglayer/primitive2d/graphicprimitive2d.hxx>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <drawinglayer/primitive2d/hiddengeometryprimitive2d.hxx>
#include <drawinglayer/primitive2d/invertprimitive2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <drawinglayer/primitive2d/modifiedcolorprimitive2d.hxx>
#include <drawinglayer/primitive2d/objectinfoprimitive2d.hxx>
#include <drawinglayer/primitive2d/pointarrayprimitive2d.hxx>
#include <drawinglayer/primitive2d/svggradientprimitive2d.hxx>
#include <drawinglayer/primitive2d/textdecoratedprimitive2d.hxx>
#include <drawinglayer/primitive2d/textenumsprimitive2d.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/wrongspellprimitive2d.hxx>
#include <drawinglayer/processor2d/Primitive2dJsonProcessor.hxx>
#include <tools/color.hxx>
#include <tools/fontenum.hxx>
#include <tools/json_writer.hxx>
#include <tools/stream.hxx>
#include <vcl/BinaryDataContainer.hxx>
#include <vcl/GraphicAttributes.hxx>
#include <vcl/GraphicObject.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/graph.hxx>
#include <vcl/salctype.hxx>
#include <vcl/vectorgraphicdata.hxx>
#include <vcl/virdev.hxx>

#include <memory>
#include <unordered_map>
#include <vector>

using namespace drawinglayer::primitive2d;

namespace
{
/// The closed triangle (0, 0), (100, 0), (100, 100).
basegfx::B2DPolyPolygon triangle()
{
    basegfx::B2DPolygon aTriangle;
    aTriangle.append(basegfx::B2DPoint(0.0, 0.0));
    aTriangle.append(basegfx::B2DPoint(100.0, 0.0));
    aTriangle.append(basegfx::B2DPoint(100.0, 100.0));
    aTriangle.setClosed(true);
    return basegfx::B2DPolyPolygon(aTriangle);
}

/// The triangle filled in rColor, as the only child of a container.
Primitive2DContainer filledTriangle(const basegfx::BColor& rColor)
{
    Primitive2DContainer aChildren;
    aChildren.append(new PolyPolygonColorPrimitive2D(triangle(), rColor));
    return aChildren;
}

/// The open line from rStart to rEnd.
basegfx::B2DPolygon line(const basegfx::B2DPoint& rStart, const basegfx::B2DPoint& rEnd)
{
    basegfx::B2DPolygon aLine;
    aLine.append(rStart);
    aLine.append(rEnd);
    return aLine;
}

/// The horizontal line from (0, 0) to (100, 0).
basegfx::B2DPolygon horizontalLine()
{
    return line(basegfx::B2DPoint(0.0, 0.0), basegfx::B2DPoint(100.0, 0.0));
}

/// A black line 2 wide with mitered joins and butt caps.
drawinglayer::attribute::LineAttribute blackLine()
{
    return drawinglayer::attribute::LineAttribute(basegfx::BColor(0.0, 0.0, 0.0), 2.0,
                                                  basegfx::B2DLineJoin::Miter,
                                                  css::drawing::LineCap_BUTT);
}

/// Text 20 high, its baseline starting at (50, 100).
basegfx::B2DHomMatrix textTransform()
{
    basegfx::B2DHomMatrix aTextTransform;
    aTextTransform.scale(20.0, 20.0);
    aTextTransform.translate(50.0, 100.0);
    return aTextTransform;
}

drawinglayer::attribute::FontAttribute liberationSans()
{
    return drawinglayer::attribute::FontAttribute(u"Liberation Sans"_ustr, u""_ustr, WEIGHT_NORMAL);
}

css::lang::Locale englishLocale() { return css::lang::Locale(u"en"_ustr, u"US"_ustr, u""_ustr); }

/// The graphic of a 10 by 10 raster image, all black.
GraphicObject rasterGraphic()
{
    return GraphicObject(Graphic(Bitmap(Size(10, 10), vcl::PixelFormat::N24_BPP)));
}

/// The graphic of a 10 by 10 SVG image, a red square.
GraphicObject svgGraphic()
{
    static constexpr OString sSvg
        = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"10\" height=\"10\">"
          "<rect width=\"10\" height=\"10\" fill=\"red\"/></svg>"_ostr;
    SvMemoryStream aStream(const_cast<char*>(sSvg.getStr()), sSvg.getLength(), StreamMode::READ);
    auto pVectorData = std::make_shared<VectorGraphicData>(
        BinaryDataContainer(aStream, sSvg.getLength()), VectorGraphicDataType::Svg);
    return GraphicObject(Graphic(pVectorData));
}

/// rGraphic scaled to 100 by 100, with the attributes rAttribute.
Primitive2DContainer graphicPrimitive(const GraphicObject& rGraphic,
                                      const GraphicAttr& rAttribute = GraphicAttr())
{
    Primitive2DContainer aPrimitives;
    aPrimitives.append(new GraphicPrimitive2D(basegfx::utils::createScaleB2DHomMatrix(100.0, 100.0),
                                              rGraphic, rAttribute));
    return aPrimitives;
}

Primitive2DContainer backgroundColor()
{
    // Fully opaque, so the transparency is left out.
    Primitive2DContainer aPrimitives;
    aPrimitives.append(new BackgroundColorPrimitive2D(basegfx::BColor(1.0, 0.0, 0.0), 0.0));
    return aPrimitives;
}

Primitive2DContainer backgroundColorTransparent()
{
    Primitive2DContainer aPrimitives;
    aPrimitives.append(new BackgroundColorPrimitive2D(basegfx::BColor(0.0, 1.0, 1.0), 0.25));
    return aPrimitives;
}

Primitive2DContainer polyPolygonColor() { return filledTriangle(basegfx::BColor(0.0, 1.0, 0.0)); }

Primitive2DContainer polyPolygonColorWithHole()
{
    // Both rings go in the same direction, so the hole shows only with an even-odd fill.
    basegfx::B2DPolyPolygon aPolyPolygon(
        basegfx::utils::createPolygonFromRect(basegfx::B2DRange(0.0, 0.0, 100.0, 100.0)));
    aPolyPolygon.append(
        basegfx::utils::createPolygonFromRect(basegfx::B2DRange(25.0, 25.0, 75.0, 75.0)));

    Primitive2DContainer aPrimitives;
    aPrimitives.append(
        new PolyPolygonColorPrimitive2D(aPolyPolygon, basegfx::BColor(1.0, 0.0, 0.0)));
    return aPrimitives;
}

Primitive2DContainer polyPolygonRGBA()
{
    Primitive2DContainer aPrimitives;
    aPrimitives.append(
        new PolyPolygonRGBAPrimitive2D(triangle(), basegfx::BColor(0.0, 0.0, 1.0), 0.25));
    return aPrimitives;
}

Primitive2DContainer polygonStroke()
{
    Primitive2DContainer aPrimitives;
    aPrimitives.append(new PolygonStrokePrimitive2D(horizontalLine(), blackLine()));
    return aPrimitives;
}

Primitive2DContainer polygonStrokeNoJoin()
{
    // Three edges with round caps, and no join between them.
    basegfx::B2DPolygon aPolygon;
    aPolygon.append(basegfx::B2DPoint(0.0, 0.0));
    aPolygon.append(basegfx::B2DPoint(100.0, 0.0));
    aPolygon.append(basegfx::B2DPoint(100.0, 100.0));
    aPolygon.append(basegfx::B2DPoint(200.0, 100.0));

    drawinglayer::attribute::LineAttribute aLine(basegfx::BColor(0.0, 0.0, 0.0), 20.0,
                                                 basegfx::B2DLineJoin::NONE,
                                                 css::drawing::LineCap_ROUND);

    Primitive2DContainer aPrimitives;
    aPrimitives.append(new PolygonStrokePrimitive2D(aPolygon, aLine));
    return aPrimitives;
}

Primitive2DContainer polygonStrokeDashed()
{
    drawinglayer::attribute::StrokeAttribute aStroke(std::vector<double>{ 4.0, 2.0 }, 6.0);

    Primitive2DContainer aPrimitives;
    aPrimitives.append(new PolygonStrokePrimitive2D(horizontalLine(), blackLine(), aStroke));
    return aPrimitives;
}

Primitive2DContainer polyPolygonStroke()
{
    // Two separate horizontal lines, 50 apart.
    basegfx::B2DPolyPolygon aPolyPolygon(horizontalLine());
    aPolyPolygon.append(line(basegfx::B2DPoint(0.0, 50.0), basegfx::B2DPoint(100.0, 50.0)));

    Primitive2DContainer aPrimitives;
    aPrimitives.append(new PolyPolygonStrokePrimitive2D(aPolyPolygon, blackLine()));
    return aPrimitives;
}

Primitive2DContainer svgLinearGradient()
{
    // A repeating gradient from opaque red to half transparent blue, across the rectangle from
    // (100, 100) to (300, 200).
    const basegfx::B2DPolyPolygon aPolyPolygon(
        basegfx::utils::createPolygonFromRect(basegfx::B2DRange(100.0, 100.0, 300.0, 200.0)));
    SvgGradientEntryVector aEntries{
        SvgGradientEntry(0.0, basegfx::BColor(1.0, 0.0, 0.0), 1.0),
        SvgGradientEntry(1.0, basegfx::BColor(0.0, 0.0, 1.0), 0.5),
    };

    Primitive2DContainer aPrimitives;
    aPrimitives.append(new SvgLinearGradientPrimitive2D(
        basegfx::B2DHomMatrix(), aPolyPolygon, std::move(aEntries), basegfx::B2DPoint(0.0, 0.0),
        basegfx::B2DPoint(1.0, 0.0), false, SpreadMethod::Repeat));
    return aPrimitives;
}

Primitive2DContainer polygonStrokeArrow()
{
    // A blue line 1000 long, 20 wide, with a head 100 wide at its end. The point of the head is
    // at the top middle of its polygon.
    basegfx::B2DPolygon aPolygon;
    aPolygon.append(basegfx::B2DPoint(0.0, 0.0));
    aPolygon.append(basegfx::B2DPoint(1000.0, 0.0));

    drawinglayer::attribute::LineAttribute aLine(basegfx::BColor(0.0, 0.0, 1.0), 20.0,
                                                 basegfx::B2DLineJoin::Miter,
                                                 css::drawing::LineCap_BUTT);

    basegfx::B2DPolygon aHead;
    aHead.append(basegfx::B2DPoint(50.0, 0.0));
    aHead.append(basegfx::B2DPoint(100.0, 100.0));
    aHead.append(basegfx::B2DPoint(0.0, 100.0));
    aHead.setClosed(true);
    const drawinglayer::attribute::LineStartEndAttribute aEnd(100.0, basegfx::B2DPolyPolygon(aHead),
                                                              false);

    Primitive2DContainer aPrimitives;
    aPrimitives.append(new PolygonStrokeArrowPrimitive2D(
        aPolygon, aLine, drawinglayer::attribute::LineStartEndAttribute(), aEnd));
    return aPrimitives;
}

Primitive2DContainer group()
{
    // A green triangle, then a black line across it.
    Primitive2DContainer aChildren = filledTriangle(basegfx::BColor(0.0, 1.0, 0.0));
    aChildren.append(new PolygonStrokePrimitive2D(
        line(basegfx::B2DPoint(0.0, 50.0), basegfx::B2DPoint(100.0, 50.0)),
        drawinglayer::attribute::LineAttribute(basegfx::BColor(0.0, 0.0, 0.0), 1.0,
                                               basegfx::B2DLineJoin::Miter,
                                               css::drawing::LineCap_BUTT)));

    Primitive2DContainer aPrimitives;
    aPrimitives.append(new GroupPrimitive2D(std::move(aChildren)));
    return aPrimitives;
}

Primitive2DContainer groupEmpty()
{
    Primitive2DContainer aPrimitives;
    aPrimitives.append(new GroupPrimitive2D(Primitive2DContainer()));
    return aPrimitives;
}

Primitive2DContainer transform()
{
    // A turn by thirty degrees, so every entry of the matrix is set.
    basegfx::B2DHomMatrix aMatrix;
    aMatrix.rotate(basegfx::deg2rad(30.0));

    Primitive2DContainer aPrimitives;
    aPrimitives.append(
        new TransformPrimitive2D(aMatrix, filledTriangle(basegfx::BColor(0.0, 1.0, 0.0))));
    return aPrimitives;
}

Primitive2DContainer hiddenGeometry()
{
    Primitive2DContainer aPrimitives;
    aPrimitives.append(
        new HiddenGeometryPrimitive2D(filledTriangle(basegfx::BColor(1.0, 0.0, 0.0))));
    return aPrimitives;
}

Primitive2DContainer exclusiveEditView()
{
    Primitive2DContainer aPrimitives;
    aPrimitives.append(
        new ExclusiveEditViewPrimitive2D(filledTriangle(basegfx::BColor(0.0, 0.0, 1.0))));
    return aPrimitives;
}

Primitive2DContainer objectInfo()
{
    Primitive2DContainer aPrimitives;
    aPrimitives.append(new ObjectInfoPrimitive2D(filledTriangle(basegfx::BColor(0.27, 0.45, 0.77)),
                                                 u"Rectangle 1"_ustr, u"My title"_ustr,
                                                 u"My description"_ustr));
    return aPrimitives;
}

Primitive2DContainer unifiedTransparence()
{
    Primitive2DContainer aPrimitives;
    aPrimitives.append(new UnifiedTransparencePrimitive2D(
        filledTriangle(basegfx::BColor(0.27, 0.45, 0.77)), 0.25));
    return aPrimitives;
}

Primitive2DContainer modifiedColorGray()
{
    Primitive2DContainer aPrimitives;
    aPrimitives.append(
        new ModifiedColorPrimitive2D(filledTriangle(basegfx::BColor(1.0, 0.0, 0.0)),
                                     std::make_shared<basegfx::BColorModifier_gray>()));
    return aPrimitives;
}

Primitive2DContainer modifiedColorInvert()
{
    Primitive2DContainer aPrimitives;
    aPrimitives.append(
        new ModifiedColorPrimitive2D(filledTriangle(basegfx::BColor(0.0, 1.0, 0.0)),
                                     std::make_shared<basegfx::BColorModifier_invert>()));
    return aPrimitives;
}

Primitive2DContainer modifiedColorReplace()
{
    // A blue triangle that every pixel of turns red.
    Primitive2DContainer aPrimitives;
    aPrimitives.append(new ModifiedColorPrimitive2D(
        filledTriangle(basegfx::BColor(0.0, 0.0, 1.0)),
        std::make_shared<basegfx::BColorModifier_replace>(basegfx::BColor(1.0, 0.0, 0.0))));
    return aPrimitives;
}

Primitive2DContainer modifiedColorParameters()
{
    // A red triangle under each modifier that takes parameters, in this order: interpolate,
    // saturate, hue rotate, matrix, black and white, gamma, and luminance to alpha. The luminance
    // and contrast modifier comes with the adjusted graphic.
    const basegfx::BColorModifierSharedPtr aModifiers[] = {
        std::make_shared<basegfx::BColorModifier_interpolate>(basegfx::BColor(0.0, 0.0, 1.0), 0.25),
        std::make_shared<basegfx::BColorModifier_saturate>(0.5),
        std::make_shared<basegfx::BColorModifier_hueRotate>(M_PI / 2.0),
        std::make_shared<basegfx::BColorModifier_matrix>(
            std::vector<double>{ 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0,
                                 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0 }),
        std::make_shared<basegfx::BColorModifier_black_and_white>(0.25),
        std::make_shared<basegfx::BColorModifier_gamma>(2.0),
        std::make_shared<basegfx::BColorModifier_luminance_to_alpha>(),
    };

    Primitive2DContainer aPrimitives;
    for (const auto& rModifier : aModifiers)
        aPrimitives.append(new ModifiedColorPrimitive2D(
            filledTriangle(basegfx::BColor(1.0, 0.0, 0.0)), rModifier));
    return aPrimitives;
}

Primitive2DContainer filledRectangle()
{
    Primitive2DContainer aPrimitives;
    aPrimitives.append(new FilledRectanglePrimitive2D(basegfx::B2DRange(10.0, 20.0, 110.0, 70.0),
                                                      basegfx::BColor(0.0, 0.0, 1.0)));
    return aPrimitives;
}

Primitive2DContainer lineRectangle()
{
    Primitive2DContainer aPrimitives;
    aPrimitives.append(new LineRectanglePrimitive2D(basegfx::B2DRange(10.0, 20.0, 110.0, 70.0),
                                                    basegfx::BColor(1.0, 0.0, 0.0)));
    return aPrimitives;
}

Primitive2DContainer singleLine()
{
    Primitive2DContainer aPrimitives;
    aPrimitives.append(new SingleLinePrimitive2D(basegfx::B2DPoint(10.0, 20.0),
                                                 basegfx::B2DPoint(110.0, 70.0),
                                                 basegfx::BColor(1.0, 0.0, 0.0)));
    return aPrimitives;
}

Primitive2DContainer wrongSpell()
{
    // The mark under a misspelled word in text 200 high, from x 0 to 2 in text units.
    Primitive2DContainer aPrimitives;
    aPrimitives.append(new WrongSpellPrimitive2D(
        basegfx::utils::createScaleTranslateB2DHomMatrix(200.0, 200.0, 100.0, 500.0), 0.0, 2.0,
        basegfx::BColor(1.0, 0.0, 0.0)));
    return aPrimitives;
}

Primitive2DContainer textSimplePortion()
{
    // "Hello" in black, with no advances of its own.
    Primitive2DContainer aPrimitives;
    aPrimitives.append(new TextSimplePortionPrimitive2D(
        textTransform(), u"Hello"_ustr, 0, 5, std::vector<double>{}, {}, liberationSans(),
        englishLocale(), basegfx::BColor(0.0, 0.0, 0.0)));
    return aPrimitives;
}

Primitive2DContainer textHighlight()
{
    // "Hello" over a yellow highlight, its letters 10 apart.
    Primitive2DContainer aPrimitives;
    aPrimitives.append(new TextSimplePortionPrimitive2D(
        textTransform(), u"Hello"_ustr, 0, 5, std::vector<double>{ 10, 20, 30, 40, 50 }, {},
        liberationSans(), englishLocale(), basegfx::BColor(0.0, 0.0, 0.0), COL_YELLOW));
    return aPrimitives;
}

Primitive2DContainer textNoHighlight()
{
    Primitive2DContainer aPrimitives;
    aPrimitives.append(new TextSimplePortionPrimitive2D(
        basegfx::utils::createScaleB2DHomMatrix(20.0, 20.0), u"Hello"_ustr, 0, 5,
        std::vector<double>{}, {}, liberationSans(), englishLocale(),
        basegfx::BColor(0.0, 0.0, 0.0)));
    return aPrimitives;
}

Primitive2DContainer textDecoratedPortion()
{
    // "Hello" underlined and struck through, with no advances of its own.
    Primitive2DContainer aPrimitives;
    aPrimitives.append(new TextDecoratedPortionPrimitive2D(
        textTransform(), u"Hello"_ustr, 0, 5, std::vector<double>{}, {}, liberationSans(),
        englishLocale(), basegfx::BColor(0.0, 0.0, 0.0), COL_TRANSPARENT, 0, 100, 0, false,
        basegfx::BColor(0.0, 0.0, 0.0), basegfx::BColor(0.0, 0.0, 0.0), TEXT_LINE_NONE,
        TEXT_LINE_SINGLE, false, TEXT_STRIKEOUT_SINGLE));
    return aPrimitives;
}

Primitive2DContainer textDecoratedPortionDecorations()
{
    // "Hello" with a red underline and struck through, its letters 10 apart.
    Primitive2DContainer aPrimitives;
    aPrimitives.append(new TextDecoratedPortionPrimitive2D(
        textTransform(), u"Hello"_ustr, 0, 5, std::vector<double>{ 10, 20, 30, 40, 50 }, {},
        liberationSans(), englishLocale(), basegfx::BColor(0.0, 0.0, 0.0), COL_TRANSPARENT, 0, 100,
        0, false, basegfx::BColor(0.0, 0.0, 0.0), basegfx::BColor(1.0, 0.0, 0.0), TEXT_LINE_NONE,
        TEXT_LINE_SINGLE, false, TEXT_STRIKEOUT_SINGLE));
    return aPrimitives;
}

Primitive2DContainer textDecoratedPortionDecorationsInTextColor()
{
    // "Hello" in the automatic color, with a blue overline and a double underline in the
    // automatic color.
    basegfx::BColor aAutomatic(0.0, 0.0, 0.0);
    aAutomatic.setAutomatic(true);

    Primitive2DContainer aPrimitives;
    aPrimitives.append(new TextDecoratedPortionPrimitive2D(
        textTransform(), u"Hello"_ustr, 0, 5, std::vector<double>{ 10, 20, 30, 40, 50 }, {},
        liberationSans(), englishLocale(), aAutomatic, COL_TRANSPARENT, 0, 100, 0, false,
        basegfx::BColor(0.0, 0.0, 1.0), aAutomatic, TEXT_LINE_SINGLE, TEXT_LINE_DOUBLE));
    return aPrimitives;
}

Primitive2DContainer bitmap()
{
    // A green image 4 by 4 pixels, drawn 40 by 30 at (10, 20).
    Bitmap aBitmap(Size(4, 4), vcl::PixelFormat::N32_BPP);
    aBitmap.Erase(COL_GREEN);

    Primitive2DContainer aPrimitives;
    aPrimitives.append(new BitmapPrimitive2D(
        aBitmap, basegfx::utils::createScaleTranslateB2DHomMatrix(40.0, 30.0, 10.0, 20.0)));
    return aPrimitives;
}

Primitive2DContainer bitmapAlpha()
{
    // The image of the bitmap sample, a quarter transparent.
    Bitmap aBitmap(Size(4, 4), vcl::PixelFormat::N32_BPP);
    aBitmap.Erase(COL_GREEN);

    Primitive2DContainer aPrimitives;
    aPrimitives.append(new BitmapAlphaPrimitive2D(
        aBitmap, basegfx::utils::createScaleTranslateB2DHomMatrix(40.0, 30.0, 10.0, 20.0), 0.25));
    return aPrimitives;
}

Primitive2DContainer graphic()
{
    // A raster image 8 by 6 pixels, drawn 80 by 60 at (15, 25).
    GraphicObject aGraphicObject{ Graphic(Bitmap(Size(8, 6), vcl::PixelFormat::N24_BPP)) };

    Primitive2DContainer aPrimitives;
    aPrimitives.append(new GraphicPrimitive2D(
        basegfx::utils::createScaleTranslateB2DHomMatrix(80.0, 60.0, 15.0, 25.0), aGraphicObject));
    return aPrimitives;
}

/// The crop of the cropped samples, in 1/100 mm from the left, top, right and bottom.
GraphicAttr croppedAttribute()
{
    GraphicAttr aAttribute;
    aAttribute.SetCrop(100, 200, 50, 25);
    return aAttribute;
}

Primitive2DContainer graphicCrop()
{
    // The image is 1000 by 800 in 1/100 mm, the same size as its frame.
    Graphic aGraphic{ Bitmap(Size(10, 10), vcl::PixelFormat::N24_BPP) };
    aGraphic.SetPrefSize(Size(1000, 800));
    aGraphic.SetPrefMapMode(MapMode(MapUnit::Map100thMM));

    Primitive2DContainer aPrimitives;
    aPrimitives.append(
        new GraphicPrimitive2D(basegfx::utils::createScaleB2DHomMatrix(1000.0, 800.0),
                               GraphicObject(aGraphic), croppedAttribute()));
    return aPrimitives;
}

Primitive2DContainer graphicCropRotated()
{
    // The same cropped image upright, then turned by 45 degrees.
    const GraphicObject aGraphicObject = rasterGraphic();

    basegfx::B2DHomMatrix aRotated = basegfx::utils::createScaleB2DHomMatrix(1000.0, 800.0);
    aRotated.rotate(M_PI / 4.0);

    Primitive2DContainer aPrimitives;
    aPrimitives.append(
        new GraphicPrimitive2D(basegfx::utils::createScaleB2DHomMatrix(1000.0, 800.0),
                               aGraphicObject, croppedAttribute()));
    aPrimitives.append(new GraphicPrimitive2D(aRotated, aGraphicObject, croppedAttribute()));
    return aPrimitives;
}

Primitive2DContainer graphicRotation()
{
    // 45 degrees clockwise.
    GraphicAttr aAttribute;
    aAttribute.SetRotation(Degree10(450));

    Primitive2DContainer aPrimitives;
    aPrimitives.append(new GraphicPrimitive2D(
        basegfx::utils::createScaleB2DHomMatrix(1000.0, 1000.0), rasterGraphic(), aAttribute));
    return aPrimitives;
}

Primitive2DContainer graphicAlpha()
{
    // About half opaque.
    GraphicAttr aAttribute;
    aAttribute.SetAlpha(128);
    return graphicPrimitive(rasterGraphic(), aAttribute);
}

Primitive2DContainer graphicMirror()
{
    GraphicAttr aAttribute;
    aAttribute.SetMirrorFlags(BmpMirrorFlags::Horizontal | BmpMirrorFlags::Vertical);
    return graphicPrimitive(rasterGraphic(), aAttribute);
}

Primitive2DContainer graphicDrawMode()
{
    GraphicAttr aAttribute;
    aAttribute.SetDrawMode(GraphicDrawMode::Greys);
    return graphicPrimitive(rasterGraphic(), aAttribute);
}

Primitive2DContainer graphicWatermark()
{
    GraphicAttr aAttribute;
    aAttribute.SetDrawMode(GraphicDrawMode::Watermark);
    return graphicPrimitive(rasterGraphic(), aAttribute);
}

Primitive2DContainer graphicAdjusted()
{
    // Every picture adjustment, on top of the mono draw mode.
    GraphicAttr aAttribute;
    aAttribute.SetDrawMode(GraphicDrawMode::Mono);
    aAttribute.SetLuminance(20);
    aAttribute.SetContrast(-10);
    aAttribute.SetChannelR(5);
    aAttribute.SetChannelG(-15);
    aAttribute.SetChannelB(25);
    aAttribute.SetGamma(2.0);
    aAttribute.SetInvert(true);
    return graphicPrimitive(rasterGraphic(), aAttribute);
}

Primitive2DContainer graphicUnadjusted() { return graphicPrimitive(rasterGraphic()); }

Primitive2DContainer graphicSvg() { return graphicPrimitive(svgGraphic()); }

Primitive2DContainer graphicSvgAdjusted()
{
    GraphicAttr aAttribute;
    aAttribute.SetDrawMode(GraphicDrawMode::Greys);
    aAttribute.SetInvert(true);
    return graphicPrimitive(svgGraphic(), aAttribute);
}

Primitive2DContainer graphicSvgCropRotateMirrorAlpha()
{
    GraphicAttr aAttribute;
    aAttribute.SetCrop(10, 20, 30, 40);
    aAttribute.SetRotation(Degree10(450));
    aAttribute.SetMirrorFlags(BmpMirrorFlags::Horizontal);
    aAttribute.SetAlpha(128);
    return graphicPrimitive(svgGraphic(), aAttribute);
}

Primitive2DContainer graphicEmf()
{
    // The EMF of a metafile that draws one pixel. An export that fails leaves the sample empty.
    GDIMetaFile aMetaFile;
    {
        ScopedVclPtrInstance<VirtualDevice> pVirtualDevice;
        pVirtualDevice->SetConnectMetaFile(&aMetaFile);
        pVirtualDevice->SetOutputSizePixel(Size(10, 10));
        pVirtualDevice->DrawPixel(Point(4, 4));
    }

    SvMemoryStream aEmfStream;
    if (GraphicConverter::Export(aEmfStream, Graphic(aMetaFile), ConvertDataFormat::EMF)
        != ERRCODE_NONE)
        return Primitive2DContainer();
    const sal_uInt64 nEmfSize = aEmfStream.Tell();
    aEmfStream.Seek(0);

    auto pVectorData = std::make_shared<VectorGraphicData>(
        BinaryDataContainer(aEmfStream, nEmfSize), VectorGraphicDataType::Emf);
    return graphicPrimitive(GraphicObject(Graphic(pVectorData)));
}

Primitive2DContainer pointArray()
{
    std::vector<basegfx::B2DPoint> aPoints{
        basegfx::B2DPoint(10.0, 10.0),
        basegfx::B2DPoint(20.0, 10.0),
        basegfx::B2DPoint(30.0, 10.0),
    };

    Primitive2DContainer aPrimitives;
    aPrimitives.append(
        new PointArrayPrimitive2D(std::move(aPoints), basegfx::BColor(1.0, 0.0, 0.0)));
    return aPrimitives;
}

Primitive2DContainer mask()
{
    // The green triangle, clipped to the square from (0, 0) to (50, 50).
    Primitive2DContainer aPrimitives;
    aPrimitives.append(
        new MaskPrimitive2D(basegfx::B2DPolyPolygon(basegfx::utils::createPolygonFromRect(
                                basegfx::B2DRange(0.0, 0.0, 50.0, 50.0))),
                            filledTriangle(basegfx::BColor(0.0, 1.0, 0.0))));
    return aPrimitives;
}

/// A blue rectangle 100 by 50 at the origin, as the only child of a container.
Primitive2DContainer blueRectangle()
{
    Primitive2DContainer aChildren;
    aChildren.append(new PolyPolygonColorPrimitive2D(
        basegfx::B2DPolyPolygon(
            basegfx::utils::createPolygonFromRect(basegfx::B2DRange(0.0, 0.0, 100.0, 50.0))),
        basegfx::BColor(0.0, 0.0, 1.0)));
    return aChildren;
}

Primitive2DContainer reflection()
{
    // The reflection starts 10 below the content, blurred by 4, and fades from half transparent
    // at its start to clear at 0.9 of its height.
    const drawinglayer::attribute::ReflectionAttribute aReflection(10, 4, 0.5, 0.0, 1.0, 0.9);
    Primitive2DContainer aPrimitives;
    aPrimitives.append(new ReflectionPrimitive2D(aReflection, blueRectangle()));
    return aPrimitives;
}

Primitive2DContainer reflectionInvisible()
{
    // The default reflection is fully transparent.
    Primitive2DContainer aPrimitives;
    aPrimitives.append(
        new ReflectionPrimitive2D(drawinglayer::attribute::ReflectionAttribute(), blueRectangle()));
    return aPrimitives;
}

Primitive2DContainer invert()
{
    // A white rectangle from (10, 20) to (110, 70), painted with XOR.
    Primitive2DContainer aChildren;
    aChildren.append(new PolyPolygonColorPrimitive2D(
        basegfx::B2DPolyPolygon(
            basegfx::utils::createPolygonFromRect(basegfx::B2DRange(10.0, 20.0, 110.0, 70.0))),
        basegfx::BColor(1.0, 1.0, 1.0)));

    Primitive2DContainer aPrimitives;
    aPrimitives.append(new InvertPrimitive2D(std::move(aChildren)));
    return aPrimitives;
}

using SampleBuilder = Primitive2DContainer (*)();

/// Every sample, by its name.
const std::unordered_map<std::string_view, SampleBuilder>& samples()
{
    static const std::unordered_map<std::string_view, SampleBuilder> aSamples{
        { "backgroundColor", backgroundColor },
        { "backgroundColorTransparent", backgroundColorTransparent },
        { "polyPolygonColor", polyPolygonColor },
        { "polyPolygonColorWithHole", polyPolygonColorWithHole },
        { "polyPolygonRGBA", polyPolygonRGBA },
        { "polygonStroke", polygonStroke },
        { "polygonStrokeNoJoin", polygonStrokeNoJoin },
        { "polygonStrokeDashed", polygonStrokeDashed },
        { "polyPolygonStroke", polyPolygonStroke },
        { "svgLinearGradient", svgLinearGradient },
        { "polygonStrokeArrow", polygonStrokeArrow },
        { "group", group },
        { "groupEmpty", groupEmpty },
        { "transform", transform },
        { "hiddenGeometry", hiddenGeometry },
        { "exclusiveEditView", exclusiveEditView },
        { "objectInfo", objectInfo },
        { "unifiedTransparence", unifiedTransparence },
        { "modifiedColorGray", modifiedColorGray },
        { "modifiedColorInvert", modifiedColorInvert },
        { "modifiedColorReplace", modifiedColorReplace },
        { "modifiedColorParameters", modifiedColorParameters },
        { "filledRectangle", filledRectangle },
        { "lineRectangle", lineRectangle },
        { "singleLine", singleLine },
        { "wrongSpell", wrongSpell },
        { "textSimplePortion", textSimplePortion },
        { "textHighlight", textHighlight },
        { "textNoHighlight", textNoHighlight },
        { "textDecoratedPortion", textDecoratedPortion },
        { "textDecoratedPortionDecorations", textDecoratedPortionDecorations },
        { "textDecoratedPortionDecorationsInTextColor",
          textDecoratedPortionDecorationsInTextColor },
        { "bitmap", bitmap },
        { "bitmapAlpha", bitmapAlpha },
        { "graphic", graphic },
        { "graphicCrop", graphicCrop },
        { "graphicCropRotated", graphicCropRotated },
        { "graphicRotation", graphicRotation },
        { "graphicAlpha", graphicAlpha },
        { "graphicMirror", graphicMirror },
        { "graphicDrawMode", graphicDrawMode },
        { "graphicWatermark", graphicWatermark },
        { "graphicAdjusted", graphicAdjusted },
        { "graphicUnadjusted", graphicUnadjusted },
        { "graphicSvg", graphicSvg },
        { "graphicSvgAdjusted", graphicSvgAdjusted },
        { "graphicSvgCropRotateMirrorAlpha", graphicSvgCropRotateMirrorAlpha },
        { "graphicEmf", graphicEmf },
        { "pointArray", pointArray },
        { "mask", mask },
        { "reflection", reflection },
        { "reflectionInvisible", reflectionInvisible },
        { "invert", invert },
    };
    return aSamples;
}
}

namespace drawinglayer
{
bool writeVectorPrimitiveSample(tools::JsonWriter& rWriter, std::string_view rName)
{
    const auto aIterator = samples().find(rName);
    if (aIterator == samples().end())
        return false;

    const Primitive2DContainer aPrimitives = aIterator->second();

    // Images travel by checksum, so the processor needs a cache to put them in.
    std::unordered_map<sal_Int64, Graphic> aBitmapCache;
    Primitive2dJsonProcessor aProcessor(rWriter);
    aProcessor.setBitmapCache(aBitmapCache);
    // A document view that edits shows the content kept for such a view alone.
    geometry::ViewInformation2D aViewInformation;
    aViewInformation.setEditViewActive(true);
    aProcessor.setViewInformation2D(aViewInformation);
    auto aArray = rWriter.startArray("primitives");
    aProcessor.decomposeAndWrite(aPrimitives);
    return true;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
