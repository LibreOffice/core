/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <test/bootstrapfixture.hxx>

#include <drawinglayer/tools/SvgWriter.hxx>

#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>

using namespace drawinglayer;

class SvgExportTest : public test::BootstrapFixture
{
public:
    SvgExportTest()
        : BootstrapFixture(true, false)
    {
    }

    CPPUNIT_TEST_SUITE(SvgExportTest);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();

    void test()
    {
        SvFileStream aStream("~/test.svg", StreamMode::WRITE | StreamMode::TRUNC);

        primitive2d::Primitive2DContainer aPrimitives;

        basegfx::BColor aBColor(1.0, 0.0, 0.0);

        auto aPolygon
            = basegfx::utils::createPolygonFromRect(basegfx::B2DRectangle(10.0, 10.0, 50.0, 60.0));
        basegfx::B2DPolyPolygon aPolyPolygon(aPolygon);

        auto pStrokePrimitive(
            new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(aPolyPolygon, aBColor));

        aPrimitives.push_back(drawinglayer::primitive2d::Primitive2DReference(pStrokePrimitive));

        svg::SvgWriter aWriter(aStream, svg::SvgVersion::v1_1);
        aWriter.write(aPrimitives);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SvgExportTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
