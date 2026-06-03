/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <test/JsonTestTools.hxx>
#include <tools/JsonPath.hxx>

#include <basegfx/color/bcolor.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/range/b2drange.hxx>

#include <drawinglayer/primitive2d/Primitive2DContainer.hxx>
#include <drawinglayer/primitive2d/backgroundcolorprimitive2d.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonRGBAPrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolygonStrokePrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolygonHairlinePrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonStrokePrimitive2D.hxx>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>

#include <vcl/bitmap.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/primitive2d/textdecoratedprimitive2d.hxx>
#include <drawinglayer/primitive2d/textenumsprimitive2d.hxx>
#include <drawinglayer/attribute/fontattribute.hxx>

#include <com/sun/star/lang/Locale.hpp>
#include <tools/fontenum.hxx>
#include <drawinglayer/primitive2d/pointarrayprimitive2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/hiddengeometryprimitive2d.hxx>
#include <drawinglayer/primitive2d/exclusiveeditviewprimitive2d.hxx>
#include <drawinglayer/primitive2d/objectinfoprimitive2d.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/modifiedcolorprimitive2d.hxx>

#include <basegfx/color/bcolormodifier.hxx>
#include <drawinglayer/processor2d/Primitive2dJsonProcessor.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <drawinglayer/attribute/lineattribute.hxx>
#include <drawinglayer/attribute/strokeattribute.hxx>

#include <com/sun/star/drawing/LineCap.hpp>

#include <osl/file.hxx>
#include <tools/json_writer.hxx>
#include <vcl/graph.hxx>

#include <fstream>
#include <string_view>
#include <unordered_map>

using namespace drawinglayer::primitive2d;

namespace
{
/// Tests a primitive used by vector rendering and writes one JSON file
/// to the workdir folder "VectorRenderingReference". Each test builds
/// only the primitive it cares about, so the fixture stays focused.
class VectorPrimitiveReferenceTest : public test::BootstrapFixture, public JsonTestTools
{
protected:
    /// Run the primitives through Primitive2dJsonProcessor with a
    /// local bitmap cache, write the result to workdir, and return
    /// the parsed JSON so the caller can assert on it. The cache is
    /// required because bitmap primitives are checksum-only on the
    /// wire.
    tools::JsonPath writeReference(std::u16string_view sName,
                                   Primitive2DContainer const& rPrimitives)
    {
        std::unordered_map<sal_Int64, Graphic> aBitmapCache;
        tools::JsonWriter aWriter;
        {
            drawinglayer::Primitive2dJsonProcessor aProcessor(aWriter);
            aProcessor.setBitmapCache(aBitmapCache);
            auto aArray = aWriter.startArray("primitives");
            aProcessor.decomposeAndWrite(rPrimitives);
        }
        OString aResult = aWriter.finishAndGetAsOString();
        CPPUNIT_ASSERT(!aResult.isEmpty());

        static constexpr OUString sFolder = u"/VectorRenderingReference/"_ustr;
        osl::Directory::createPath(m_directories.getURLFromWorkdir(sFolder));
        OUString aPath = m_directories.getPathFromWorkdir(sFolder) + sName + u".json";
        std::ofstream aOut(OUStringToOString(aPath, RTL_TEXTENCODING_UTF8).getStr());
        CPPUNIT_ASSERT_MESSAGE("cannot open reference file for writing", aOut.is_open());
        aOut.write(aResult.getStr(), aResult.getLength());
        CPPUNIT_ASSERT_MESSAGE("write to reference file failed", aOut.good());

        auto oJson
            = tools::JsonPath::parse(std::string_view(aResult.getStr(), aResult.getLength()));
        CPPUNIT_ASSERT_MESSAGE("JSON parse error", oJson.has_value());
        return *oJson;
    }
};

CPPUNIT_TEST_FIXTURE(VectorPrimitiveReferenceTest, testBackgroundColor)
{
    // A fully opaque red background. The engine writes a
    // transparency field only when it is greater than zero, so this
    // fixture has none.
    Primitive2DContainer aPrimitives;
    aPrimitives.append(new BackgroundColorPrimitive2D(basegfx::BColor(1.0, 0.0, 0.0), 0.0));

    auto aJson = writeReference(u"testBackgroundColor", aPrimitives);

    assertJsonPath(aJson, "/primitives/0/type", "backgroundcolor");
    assertJsonPath(aJson, "/primitives/0/color", "#ff0000");
    assertJsonPathMissing(aJson, "/primitives/0/transparency");
}

CPPUNIT_TEST_FIXTURE(VectorPrimitiveReferenceTest, testBackgroundColorTransparent)
{
    // A partly transparent cyan background. The renderer must read
    // the transparency value and turn it into globalAlpha.
    Primitive2DContainer aPrimitives;
    aPrimitives.append(new BackgroundColorPrimitive2D(basegfx::BColor(0.0, 1.0, 1.0), 0.25));

    auto aJson = writeReference(u"testBackgroundColorTransparent", aPrimitives);

    assertJsonPath(aJson, "/primitives/0/type", "backgroundcolor");
    assertJsonPath(aJson, "/primitives/0/color", "#00ffff");
    assertJsonPathDouble(aJson, "/primitives/0/transparency", 0.25, 1e-9);
}

CPPUNIT_TEST_FIXTURE(VectorPrimitiveReferenceTest, testPolyPolygonColor)
{
    // A green triangle. The polygon must be closed so the engine
    // writes a final "Z" command in the SVG path string.
    basegfx::B2DPolygon aTriangle;
    aTriangle.append(basegfx::B2DPoint(0.0, 0.0));
    aTriangle.append(basegfx::B2DPoint(100.0, 0.0));
    aTriangle.append(basegfx::B2DPoint(100.0, 100.0));
    aTriangle.setClosed(true);

    Primitive2DContainer aPrimitives;
    aPrimitives.append(new PolyPolygonColorPrimitive2D(basegfx::B2DPolyPolygon(aTriangle),
                                                       basegfx::BColor(0.0, 1.0, 0.0)));

    auto aJson = writeReference(u"testPolyPolygonColor", aPrimitives);

    assertJsonPath(aJson, "/primitives/0/type", "polyPolygonColor");
    assertJsonPath(aJson, "/primitives/0/color", "#00ff00");
    assertJsonPathExists(aJson, "/primitives/0/path");
}

CPPUNIT_TEST_FIXTURE(VectorPrimitiveReferenceTest, testPolyPolygonRGBAPrimitive)
{
    // polyPolygonRGBA fills a polygon with one colour and an optional
    // transparency in [0, 1]. The fixture uses 25 percent transparency
    // so the wire carries the value alongside the colour.
    basegfx::B2DPolygon aTriangle;
    aTriangle.append(basegfx::B2DPoint(0.0, 0.0));
    aTriangle.append(basegfx::B2DPoint(100.0, 0.0));
    aTriangle.append(basegfx::B2DPoint(100.0, 100.0));
    aTriangle.setClosed(true);

    Primitive2DContainer aPrimitives;
    aPrimitives.append(new PolyPolygonRGBAPrimitive2D(basegfx::B2DPolyPolygon(aTriangle),
                                                      basegfx::BColor(0.0, 0.0, 1.0), 0.25));

    auto aJson = writeReference(u"testPolyPolygonRGBAPrimitive", aPrimitives);

    assertJsonPath(aJson, "/primitives/0/type", "polyPolygonRGBA");
    assertJsonPath(aJson, "/primitives/0/color", "#0000ff");
    assertJsonPathDouble(aJson, "/primitives/0/transparency", 0.25, 1e-9);
    assertJsonPathExists(aJson, "/primitives/0/path");
}

CPPUNIT_TEST_FIXTURE(VectorPrimitiveReferenceTest, testPolygonStroke)
{
    // polygonStroke carries color, width, line join and line cap on the wire.
    basegfx::B2DPolygon aPolygon;
    aPolygon.append(basegfx::B2DPoint(0.0, 0.0));
    aPolygon.append(basegfx::B2DPoint(100.0, 0.0));

    drawinglayer::attribute::LineAttribute aLine(basegfx::BColor(0.0, 0.0, 0.0), 2.0,
                                                 basegfx::B2DLineJoin::Miter,
                                                 css::drawing::LineCap_BUTT);

    Primitive2DContainer aPrimitives;
    aPrimitives.append(new PolygonStrokePrimitive2D(aPolygon, aLine));

    auto aJson = writeReference(u"testPolygonStroke", aPrimitives);

    assertJsonPath(aJson, "/primitives/0/type", "polygonStroke");
    assertJsonPath(aJson, "/primitives/0/line/color", "#000000");
    assertJsonPathDouble(aJson, "/primitives/0/line/width", 2.0, 1e-9);
    assertJsonPath(aJson, "/primitives/0/line/linejoin", "miter");
    assertJsonPath(aJson, "/primitives/0/line/linecap", "butt");
    assertJsonPathExists(aJson, "/primitives/0/path");
}

CPPUNIT_TEST_FIXTURE(VectorPrimitiveReferenceTest, testGroup)
{
    // A group of two child primitives, a filled triangle followed
    // by a stroked horizontal line. Their fill and stroke must
    // render in that order. The group carries no drawing of its own.
    basegfx::B2DPolygon aTriangle;
    aTriangle.append(basegfx::B2DPoint(0.0, 0.0));
    aTriangle.append(basegfx::B2DPoint(100.0, 0.0));
    aTriangle.append(basegfx::B2DPoint(100.0, 100.0));
    aTriangle.setClosed(true);

    basegfx::B2DPolygon aLine;
    aLine.append(basegfx::B2DPoint(0.0, 50.0));
    aLine.append(basegfx::B2DPoint(100.0, 50.0));

    drawinglayer::attribute::LineAttribute aLineAttribute(basegfx::BColor(0.0, 0.0, 0.0), 1.0,
                                                          basegfx::B2DLineJoin::Miter,
                                                          css::drawing::LineCap_BUTT);

    Primitive2DContainer aChildren;
    aChildren.append(new PolyPolygonColorPrimitive2D(basegfx::B2DPolyPolygon(aTriangle),
                                                     basegfx::BColor(0.0, 1.0, 0.0)));
    aChildren.append(new PolygonStrokePrimitive2D(aLine, aLineAttribute));

    Primitive2DContainer aPrimitives;
    aPrimitives.append(new GroupPrimitive2D(std::move(aChildren)));

    auto aJson = writeReference(u"testGroup", aPrimitives);

    assertJsonPath(aJson, "/primitives/0/type", "group");
    CPPUNIT_ASSERT_EQUAL(size_t(2), aJson.getSize("/primitives/0/children").value_or(0));
    assertJsonPath(aJson, "/primitives/0/children/0/type", "polyPolygonColor");
    assertJsonPath(aJson, "/primitives/0/children/1/type", "polygonStroke");
}

CPPUNIT_TEST_FIXTURE(VectorPrimitiveReferenceTest, testGroupEmpty)
{
    // A group with no children. The engine still writes the type
    // and an empty children array.
    Primitive2DContainer aPrimitives;
    aPrimitives.append(new GroupPrimitive2D(Primitive2DContainer()));

    auto aJson = writeReference(u"testGroupEmpty", aPrimitives);

    assertJsonPath(aJson, "/primitives/0/type", "group");
    CPPUNIT_ASSERT_EQUAL(size_t(0), aJson.getSize("/primitives/0/children").value_or(0));
}

CPPUNIT_TEST_FIXTURE(VectorPrimitiveReferenceTest, testTransform)
{
    // A transform primitive wraps its children in a 2D affine
    // matrix. The fixture rotates a green triangle by thirty
    // degrees around the origin so the matrix entries are
    // non-trivial.
    basegfx::B2DPolygon aTriangle;
    aTriangle.append(basegfx::B2DPoint(0.0, 0.0));
    aTriangle.append(basegfx::B2DPoint(100.0, 0.0));
    aTriangle.append(basegfx::B2DPoint(100.0, 100.0));
    aTriangle.setClosed(true);

    Primitive2DContainer aChildren;
    aChildren.append(new PolyPolygonColorPrimitive2D(basegfx::B2DPolyPolygon(aTriangle),
                                                     basegfx::BColor(0.0, 1.0, 0.0)));

    basegfx::B2DHomMatrix aMatrix;
    aMatrix.rotate(basegfx::deg2rad(30.0));

    Primitive2DContainer aPrimitives;
    aPrimitives.append(
        new drawinglayer::primitive2d::TransformPrimitive2D(aMatrix, std::move(aChildren)));

    auto aJson = writeReference(u"testTransform", aPrimitives);

    assertJsonPath(aJson, "/primitives/0/type", "transform");
    CPPUNIT_ASSERT_EQUAL(size_t(6), aJson.getSize("/primitives/0/matrix").value_or(0));
    CPPUNIT_ASSERT_EQUAL(size_t(1), aJson.getSize("/primitives/0/children").value_or(0));
    assertJsonPath(aJson, "/primitives/0/children/0/type", "polyPolygonColor");
}

CPPUNIT_TEST_FIXTURE(VectorPrimitiveReferenceTest, testHiddenGeometry)
{
    // HiddenGeometryPrimitive2D wraps content that is rendered
    // only for hit-testing. The fixture wraps a filled triangle so
    // there are real children present. The wire output only carries
    // the type tag, since the JSON processor drops the children on
    // purpose.
    basegfx::B2DPolygon aTriangle;
    aTriangle.append(basegfx::B2DPoint(0.0, 0.0));
    aTriangle.append(basegfx::B2DPoint(100.0, 0.0));
    aTriangle.append(basegfx::B2DPoint(100.0, 100.0));
    aTriangle.setClosed(true);

    Primitive2DContainer aChildren;
    aChildren.append(new PolyPolygonColorPrimitive2D(basegfx::B2DPolyPolygon(aTriangle),
                                                     basegfx::BColor(1.0, 0.0, 0.0)));

    Primitive2DContainer aPrimitives;
    aPrimitives.append(new HiddenGeometryPrimitive2D(std::move(aChildren)));

    auto aJson = writeReference(u"testHiddenGeometry", aPrimitives);

    assertJsonPath(aJson, "/primitives/0/type", "hiddenGeometry");
}

CPPUNIT_TEST_FIXTURE(VectorPrimitiveReferenceTest, testExclusiveEditView)
{
    // ExclusiveEditViewPrimitive2D wraps content visible only in
    // edit mode. Same shape as hiddenGeometry - real children but
    // those get dropped on the wire.
    basegfx::B2DPolygon aTriangle;
    aTriangle.append(basegfx::B2DPoint(0.0, 0.0));
    aTriangle.append(basegfx::B2DPoint(100.0, 0.0));
    aTriangle.append(basegfx::B2DPoint(100.0, 100.0));
    aTriangle.setClosed(true);

    Primitive2DContainer aChildren;
    aChildren.append(new PolyPolygonColorPrimitive2D(basegfx::B2DPolyPolygon(aTriangle),
                                                     basegfx::BColor(0.0, 0.0, 1.0)));

    Primitive2DContainer aPrimitives;
    aPrimitives.append(new ExclusiveEditViewPrimitive2D(std::move(aChildren)));

    auto aJson = writeReference(u"testExclusiveEditView", aPrimitives);

    assertJsonPath(aJson, "/primitives/0/type", "exclusiveEditView");
}

CPPUNIT_TEST_FIXTURE(VectorPrimitiveReferenceTest, testObjectInfoPrimitive)
{
    // ObjectInfoPrimitive2D wraps a subtree with metadata:
    // a name, a title and a description. The wire format
    // carries the three strings alongside the children.
    basegfx::B2DPolygon aTriangle;
    aTriangle.append(basegfx::B2DPoint(0.0, 0.0));
    aTriangle.append(basegfx::B2DPoint(100.0, 0.0));
    aTriangle.append(basegfx::B2DPoint(100.0, 100.0));
    aTriangle.setClosed(true);

    Primitive2DContainer aChildren;
    aChildren.append(new PolyPolygonColorPrimitive2D(basegfx::B2DPolyPolygon(aTriangle),
                                                     basegfx::BColor(0.27, 0.45, 0.77)));

    Primitive2DContainer aPrimitives;
    aPrimitives.append(new ObjectInfoPrimitive2D(std::move(aChildren), u"Rectangle 1"_ustr,
                                                 u"My title"_ustr, u"My description"_ustr));

    auto aJson = writeReference(u"testObjectInfoPrimitive", aPrimitives);

    assertJsonPath(aJson, "/primitives/0/type", "objectInfo");
    assertJsonPath(aJson, "/primitives/0/name", "Rectangle 1");
    assertJsonPath(aJson, "/primitives/0/title", "My title");
    assertJsonPath(aJson, "/primitives/0/desc", "My description");
    CPPUNIT_ASSERT_EQUAL(size_t(1), aJson.getSize("/primitives/0/children").value_or(0));
    assertJsonPath(aJson, "/primitives/0/children/0/type", "polyPolygonColor");
}

CPPUNIT_TEST_FIXTURE(VectorPrimitiveReferenceTest, testUnifiedTransparence)
{
    // UnifiedTransparencePrimitive2D applies one alpha to the
    // wrapped subtree. The wire carries a transparence in [0, 1]
    // where 0 is fully opaque.
    basegfx::B2DPolygon aTriangle;
    aTriangle.append(basegfx::B2DPoint(0.0, 0.0));
    aTriangle.append(basegfx::B2DPoint(100.0, 0.0));
    aTriangle.append(basegfx::B2DPoint(100.0, 100.0));
    aTriangle.setClosed(true);

    Primitive2DContainer aChildren;
    aChildren.append(new PolyPolygonColorPrimitive2D(basegfx::B2DPolyPolygon(aTriangle),
                                                     basegfx::BColor(0.27, 0.45, 0.77)));

    Primitive2DContainer aPrimitives;
    aPrimitives.append(new UnifiedTransparencePrimitive2D(std::move(aChildren), 0.25));

    auto aJson = writeReference(u"testUnifiedTransparence", aPrimitives);

    assertJsonPath(aJson, "/primitives/0/type", "unifiedTransparence");
    assertJsonPathExists(aJson, "/primitives/0/transparence");
    CPPUNIT_ASSERT_EQUAL(size_t(1), aJson.getSize("/primitives/0/children").value_or(0));
    assertJsonPath(aJson, "/primitives/0/children/0/type", "polyPolygonColor");
}

CPPUNIT_TEST_FIXTURE(VectorPrimitiveReferenceTest, testModifiedColorGray)
{
    // ModifiedColorPrimitive2D with a gray modifier. The renderer
    // turns "gray" into the canvas filter "grayscale(1)".
    basegfx::B2DPolygon aTriangle;
    aTriangle.append(basegfx::B2DPoint(0.0, 0.0));
    aTriangle.append(basegfx::B2DPoint(100.0, 0.0));
    aTriangle.append(basegfx::B2DPoint(100.0, 100.0));
    aTriangle.setClosed(true);

    Primitive2DContainer aChildren;
    aChildren.append(new PolyPolygonColorPrimitive2D(basegfx::B2DPolyPolygon(aTriangle),
                                                     basegfx::BColor(1.0, 0.0, 0.0)));

    Primitive2DContainer aPrimitives;
    aPrimitives.append(new ModifiedColorPrimitive2D(
        std::move(aChildren), std::make_shared<basegfx::BColorModifier_gray>()));

    auto aJson = writeReference(u"testModifiedColorGray", aPrimitives);

    assertJsonPath(aJson, "/primitives/0/type", "modifiedColor");
    assertJsonPath(aJson, "/primitives/0/modifier", "gray");
    CPPUNIT_ASSERT_EQUAL(size_t(1), aJson.getSize("/primitives/0/children").value_or(0));
}

CPPUNIT_TEST_FIXTURE(VectorPrimitiveReferenceTest, testModifiedColorInvert)
{
    // ModifiedColorPrimitive2D with an invert modifier. The
    // renderer turns "invert" into the canvas filter "invert(1)".
    basegfx::B2DPolygon aTriangle;
    aTriangle.append(basegfx::B2DPoint(0.0, 0.0));
    aTriangle.append(basegfx::B2DPoint(100.0, 0.0));
    aTriangle.append(basegfx::B2DPoint(100.0, 100.0));
    aTriangle.setClosed(true);

    Primitive2DContainer aChildren;
    aChildren.append(new PolyPolygonColorPrimitive2D(basegfx::B2DPolyPolygon(aTriangle),
                                                     basegfx::BColor(0.0, 1.0, 0.0)));

    Primitive2DContainer aPrimitives;
    aPrimitives.append(new ModifiedColorPrimitive2D(
        std::move(aChildren), std::make_shared<basegfx::BColorModifier_invert>()));

    auto aJson = writeReference(u"testModifiedColorInvert", aPrimitives);

    assertJsonPath(aJson, "/primitives/0/type", "modifiedColor");
    assertJsonPath(aJson, "/primitives/0/modifier", "invert");
}

CPPUNIT_TEST_FIXTURE(VectorPrimitiveReferenceTest, testModifiedColorReplace)
{
    // ModifiedColorPrimitive2D with a replace modifier. The wire
    // carries an extra "color" field. The renderer leaves the
    // children rendered as-is for now, since colour substitution
    // is not implemented yet.
    basegfx::B2DPolygon aTriangle;
    aTriangle.append(basegfx::B2DPoint(0.0, 0.0));
    aTriangle.append(basegfx::B2DPoint(100.0, 0.0));
    aTriangle.append(basegfx::B2DPoint(100.0, 100.0));
    aTriangle.setClosed(true);

    Primitive2DContainer aChildren;
    aChildren.append(new PolyPolygonColorPrimitive2D(basegfx::B2DPolyPolygon(aTriangle),
                                                     basegfx::BColor(0.0, 0.0, 1.0)));

    Primitive2DContainer aPrimitives;
    aPrimitives.append(new ModifiedColorPrimitive2D(
        std::move(aChildren),
        std::make_shared<basegfx::BColorModifier_replace>(basegfx::BColor(1.0, 0.0, 0.0))));

    auto aJson = writeReference(u"testModifiedColorReplace", aPrimitives);

    assertJsonPath(aJson, "/primitives/0/type", "modifiedColor");
    assertJsonPath(aJson, "/primitives/0/modifier", "replace");
    assertJsonPath(aJson, "/primitives/0/color", "#ff0000");
}

CPPUNIT_TEST_FIXTURE(VectorPrimitiveReferenceTest, testFilledRectangle)
{
    // A blue axis-aligned filled rectangle. The wire carries the
    // colour and a four-value bounds array [minX, minY, maxX, maxY].
    basegfx::B2DRange aRange(10.0, 20.0, 110.0, 70.0);
    Primitive2DContainer aPrimitives;
    aPrimitives.append(new FilledRectanglePrimitive2D(aRange, basegfx::BColor(0.0, 0.0, 1.0)));

    auto aJson = writeReference(u"testFilledRectangle", aPrimitives);

    assertJsonPath(aJson, "/primitives/0/type", "filledRectangle");
    assertJsonPath(aJson, "/primitives/0/color", "#0000ff");
    CPPUNIT_ASSERT_EQUAL(size_t(4), aJson.getSize("/primitives/0/bounds").value_or(0));
}

CPPUNIT_TEST_FIXTURE(VectorPrimitiveReferenceTest, testLineRectangle)
{
    // A red axis-aligned outlined rectangle. Same wire shape as
    // filledRectangle (type, color, four-value bounds), but stroked.
    basegfx::B2DRange aRange(10.0, 20.0, 110.0, 70.0);
    Primitive2DContainer aPrimitives;
    aPrimitives.append(new LineRectanglePrimitive2D(aRange, basegfx::BColor(1.0, 0.0, 0.0)));

    auto aJson = writeReference(u"testLineRectangle", aPrimitives);

    assertJsonPath(aJson, "/primitives/0/type", "lineRectangle");
    assertJsonPath(aJson, "/primitives/0/color", "#ff0000");
    CPPUNIT_ASSERT_EQUAL(size_t(4), aJson.getSize("/primitives/0/bounds").value_or(0));
}

CPPUNIT_TEST_FIXTURE(VectorPrimitiveReferenceTest, testSingleLine)
{
    // A red line segment from (10, 20) to (110, 70). The wire
    // carries the colour and four endpoint coordinates.
    Primitive2DContainer aPrimitives;
    aPrimitives.append(new SingleLinePrimitive2D(basegfx::B2DPoint(10.0, 20.0),
                                                 basegfx::B2DPoint(110.0, 70.0),
                                                 basegfx::BColor(1.0, 0.0, 0.0)));

    auto aJson = writeReference(u"testSingleLine", aPrimitives);

    assertJsonPath(aJson, "/primitives/0/type", "singleLine");
    assertJsonPath(aJson, "/primitives/0/color", "#ff0000");
    assertJsonPathDouble(aJson, "/primitives/0/startX", 10.0, 1e-9);
    assertJsonPathDouble(aJson, "/primitives/0/startY", 20.0, 1e-9);
    assertJsonPathDouble(aJson, "/primitives/0/endX", 110.0, 1e-9);
    assertJsonPathDouble(aJson, "/primitives/0/endY", 70.0, 1e-9);
}

CPPUNIT_TEST_FIXTURE(VectorPrimitiveReferenceTest, testPolygonStrokeDashed)
{
    // A dot-dash stroke pattern reaches the wire as stroke.dotDashArray.
    basegfx::B2DPolygon aPolygon;
    aPolygon.append(basegfx::B2DPoint(0.0, 0.0));
    aPolygon.append(basegfx::B2DPoint(100.0, 0.0));

    drawinglayer::attribute::LineAttribute aLine(basegfx::BColor(0.0, 0.0, 0.0), 2.0,
                                                 basegfx::B2DLineJoin::Miter,
                                                 css::drawing::LineCap_BUTT);
    drawinglayer::attribute::StrokeAttribute aStroke(std::vector<double>{ 4.0, 2.0 }, 6.0);

    Primitive2DContainer aPrimitives;
    aPrimitives.append(new PolygonStrokePrimitive2D(aPolygon, aLine, aStroke));

    auto aJson = writeReference(u"testPolygonStrokeDashed", aPrimitives);

    assertJsonPath(aJson, "/primitives/0/type", "polygonStroke");
    assertJsonPathExists(aJson, "/primitives/0/stroke/dotDashArray");
    CPPUNIT_ASSERT_EQUAL(size_t(2), aJson.getSize("/primitives/0/stroke/dotDashArray").value_or(0));
}

CPPUNIT_TEST_FIXTURE(VectorPrimitiveReferenceTest, testTextSimplePortion)
{
    // A run of horizontal text "Hello" anchored at (50, 100) with a
    // font size of 20. The font is Liberation Sans, normal weight,
    // upright. The wire carries the text, the substring offsets,
    // the transform matrix, the font shape and the colour.
    basegfx::B2DHomMatrix aTextTransform;
    aTextTransform.scale(20.0, 20.0);
    aTextTransform.translate(50.0, 100.0);

    drawinglayer::attribute::FontAttribute aFontAttr(u"Liberation Sans"_ustr, u""_ustr,
                                                     WEIGHT_NORMAL);

    css::lang::Locale aLocale(u"en"_ustr, u"US"_ustr, u""_ustr);

    Primitive2DContainer aPrimitives;
    aPrimitives.append(new drawinglayer::primitive2d::TextSimplePortionPrimitive2D(
        aTextTransform, u"Hello"_ustr, 0, 5, std::vector<double>{}, std::vector<sal_Bool>{},
        aFontAttr, aLocale, basegfx::BColor(0.0, 0.0, 0.0)));

    auto aJson = writeReference(u"testTextSimplePortion", aPrimitives);

    assertJsonPath(aJson, "/primitives/0/type", "textSimplePortion");
    assertJsonPath(aJson, "/primitives/0/text", "Hello");
    assertJsonPath(aJson, "/primitives/0/textPosition", sal_Int64(0));
    assertJsonPath(aJson, "/primitives/0/textLength", sal_Int64(5));
    assertJsonPath(aJson, "/primitives/0/fontcolor", "#000000");
    assertJsonPath(aJson, "/primitives/0/familyname", "Liberation Sans");
    assertJsonPath(aJson, "/primitives/0/weight", sal_Int64(WEIGHT_NORMAL));
    assertJsonPathDouble(aJson, "/primitives/0/fontSize", 20.0, 1e-9);
    CPPUNIT_ASSERT_EQUAL(size_t(6), aJson.getSize("/primitives/0/matrix").value_or(0));
}

CPPUNIT_TEST_FIXTURE(VectorPrimitiveReferenceTest, testTextDecoratedPortion)
{
    // The same horizontal "Hello" as testTextSimplePortion, but
    // underlined and struck through. The wire adds the decoration
    // fields alongside the text portion ones.
    basegfx::B2DHomMatrix aTextTransform;
    aTextTransform.scale(20.0, 20.0);
    aTextTransform.translate(50.0, 100.0);

    drawinglayer::attribute::FontAttribute aFontAttr(u"Liberation Sans"_ustr, u""_ustr,
                                                     WEIGHT_NORMAL);

    css::lang::Locale aLocale(u"en"_ustr, u"US"_ustr, u""_ustr);

    Primitive2DContainer aPrimitives;
    aPrimitives.append(new drawinglayer::primitive2d::TextDecoratedPortionPrimitive2D(
        aTextTransform, u"Hello"_ustr, 0, 5, std::vector<double>{}, std::vector<sal_Bool>{},
        aFontAttr, aLocale, basegfx::BColor(0.0, 0.0, 0.0), COL_TRANSPARENT, 0, 100, 0, false,
        basegfx::BColor(0.0, 0.0, 0.0), basegfx::BColor(0.0, 0.0, 0.0),
        drawinglayer::primitive2d::TEXT_LINE_NONE, drawinglayer::primitive2d::TEXT_LINE_SINGLE,
        false, drawinglayer::primitive2d::TEXT_STRIKEOUT_SINGLE));

    auto aJson = writeReference(u"testTextDecoratedPortion", aPrimitives);

    assertJsonPath(aJson, "/primitives/0/type", "textDecoratedPortion");
    assertJsonPath(aJson, "/primitives/0/text", "Hello");
    assertJsonPath(aJson, "/primitives/0/underline",
                   sal_Int64(drawinglayer::primitive2d::TEXT_LINE_SINGLE));
    assertJsonPath(aJson, "/primitives/0/underlineColor", "#000000");
    assertJsonPath(aJson, "/primitives/0/strikeout",
                   sal_Int64(drawinglayer::primitive2d::TEXT_STRIKEOUT_SINGLE));
}

CPPUNIT_TEST_FIXTURE(VectorPrimitiveReferenceTest, testBitmap)
{
    // bitmap places a raster image through a 2D affine transform.
    // The matrix maps the unit square (0,0)-(1,1) to the image's
    // destination bounds. The wire carries the matrix, the source
    // pixel dimensions and a checksum. The image bytes live in the
    // processor's bitmap cache and never travel inline.
    Bitmap aBitmap(Size(4, 4), vcl::PixelFormat::N32_BPP);
    aBitmap.Erase(COL_GREEN);
    basegfx::B2DHomMatrix aTransform;
    aTransform.scale(40.0, 30.0);
    aTransform.translate(10.0, 20.0);

    Primitive2DContainer aPrimitives;
    aPrimitives.append(new drawinglayer::primitive2d::BitmapPrimitive2D(aBitmap, aTransform));

    auto aJson = writeReference(u"testBitmap", aPrimitives);

    assertJsonPath(aJson, "/primitives/0/type", "bitmap");
    CPPUNIT_ASSERT_EQUAL(size_t(6), aJson.getSize("/primitives/0/matrix").value_or(0));
    assertJsonPath(aJson, "/primitives/0/width", sal_Int64(4));
    assertJsonPath(aJson, "/primitives/0/height", sal_Int64(4));
    assertJsonPathExists(aJson, "/primitives/0/checksum");
}

CPPUNIT_TEST_FIXTURE(VectorPrimitiveReferenceTest, testPointArray)
{
    // pointArray carries a colour and a list of points.
    std::vector<basegfx::B2DPoint> aPoints{
        basegfx::B2DPoint(10.0, 10.0),
        basegfx::B2DPoint(20.0, 10.0),
        basegfx::B2DPoint(30.0, 10.0),
    };

    Primitive2DContainer aPrimitives;
    aPrimitives.append(
        new PointArrayPrimitive2D(std::move(aPoints), basegfx::BColor(1.0, 0.0, 0.0)));

    auto aJson = writeReference(u"testPointArray", aPrimitives);

    assertJsonPath(aJson, "/primitives/0/type", "pointArray");
    assertJsonPath(aJson, "/primitives/0/color", "#ff0000");
    CPPUNIT_ASSERT_EQUAL(size_t(3), aJson.getSize("/primitives/0/points").value_or(0));
    assertJsonPathDouble(aJson, "/primitives/0/points/0/x", 10.0, 1e-9);
    assertJsonPathDouble(aJson, "/primitives/0/points/0/y", 10.0, 1e-9);
    assertJsonPathDouble(aJson, "/primitives/0/points/2/x", 30.0, 1e-9);
}

CPPUNIT_TEST_FIXTURE(VectorPrimitiveReferenceTest, testMask)
{
    // mask carries a clip path and a child subtree on the wire.
    basegfx::B2DPolygon aClipRect;
    aClipRect.append(basegfx::B2DPoint(0.0, 0.0));
    aClipRect.append(basegfx::B2DPoint(50.0, 0.0));
    aClipRect.append(basegfx::B2DPoint(50.0, 50.0));
    aClipRect.append(basegfx::B2DPoint(0.0, 50.0));
    aClipRect.setClosed(true);

    basegfx::B2DPolygon aTriangle;
    aTriangle.append(basegfx::B2DPoint(0.0, 0.0));
    aTriangle.append(basegfx::B2DPoint(100.0, 0.0));
    aTriangle.append(basegfx::B2DPoint(100.0, 100.0));
    aTriangle.setClosed(true);

    Primitive2DContainer aChildren;
    aChildren.append(new PolyPolygonColorPrimitive2D(basegfx::B2DPolyPolygon(aTriangle),
                                                     basegfx::BColor(0.0, 1.0, 0.0)));

    Primitive2DContainer aPrimitives;
    aPrimitives.append(
        new MaskPrimitive2D(basegfx::B2DPolyPolygon(aClipRect), std::move(aChildren)));

    auto aJson = writeReference(u"testMask", aPrimitives);

    assertJsonPath(aJson, "/primitives/0/type", "mask");
    assertJsonPathExists(aJson, "/primitives/0/clip");
    CPPUNIT_ASSERT_EQUAL(size_t(1), aJson.getSize("/primitives/0/children").value_or(0));
    assertJsonPath(aJson, "/primitives/0/children/0/type", "polyPolygonColor");
}

CPPUNIT_TEST_FIXTURE(VectorPrimitiveReferenceTest, testPolyPolygonStroke)
{
    // polyPolygonStroke carries the same line and stroke attributes
    // as polygonStroke, but its path describes more than one
    // disjoint subpath. The fixture strokes two separate horizontal
    // lines with a single primitive.
    basegfx::B2DPolygon aFirst;
    aFirst.append(basegfx::B2DPoint(0.0, 0.0));
    aFirst.append(basegfx::B2DPoint(100.0, 0.0));

    basegfx::B2DPolygon aSecond;
    aSecond.append(basegfx::B2DPoint(0.0, 50.0));
    aSecond.append(basegfx::B2DPoint(100.0, 50.0));

    basegfx::B2DPolyPolygon aPolyPolygon;
    aPolyPolygon.append(aFirst);
    aPolyPolygon.append(aSecond);

    drawinglayer::attribute::LineAttribute aLine(basegfx::BColor(0.0, 0.0, 0.0), 2.0,
                                                 basegfx::B2DLineJoin::Miter,
                                                 css::drawing::LineCap_BUTT);

    Primitive2DContainer aPrimitives;
    aPrimitives.append(new PolyPolygonStrokePrimitive2D(aPolyPolygon, aLine));

    auto aJson = writeReference(u"testPolyPolygonStroke", aPrimitives);

    assertJsonPath(aJson, "/primitives/0/type", "polyPolygonStroke");
    assertJsonPath(aJson, "/primitives/0/line/color", "#000000");
    assertJsonPathDouble(aJson, "/primitives/0/line/width", 2.0, 1e-9);
    assertJsonPath(aJson, "/primitives/0/line/linejoin", "miter");
    assertJsonPath(aJson, "/primitives/0/line/linecap", "butt");
    assertJsonPathExists(aJson, "/primitives/0/path");
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
