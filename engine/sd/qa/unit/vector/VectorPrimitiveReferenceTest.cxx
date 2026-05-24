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

#include <drawinglayer/primitive2d/Primitive2DContainer.hxx>
#include <drawinglayer/primitive2d/backgroundcolorprimitive2d.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/processor2d/Primitive2dJsonProcessor.hxx>

#include <osl/file.hxx>

#include <fstream>
#include <string_view>

using namespace drawinglayer::primitive2d;

namespace
{
/// Tests a primitive used by vector rendering and writes one JSON file
/// to the workdir folder "VectorRenderingReference". Each test builds
/// only the primitive it cares about, so the fixture stays focused.
class VectorPrimitiveReferenceTest : public test::BootstrapFixture, public JsonTestTools
{
protected:
    /// Run the primitives through Primitive2dJsonProcessor::dumpAsJson,
    /// write the result to workdir, and return the parsed JSON so the
    /// caller can assert on it.
    tools::JsonPath writeReference(std::u16string_view sName,
                                   Primitive2DContainer const& rPrimitives)
    {
        OString aResult = drawinglayer::Primitive2dJsonProcessor::dumpAsJson(rPrimitives);
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

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
