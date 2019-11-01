/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_features.h>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <sal/log.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

#include <vcl/virdev.hxx>

#include <drawables/B2DPolyLineDrawableHelper.hxx>

#include <cassert>

namespace
{
class B2DPolyLineDrawableTest : public CppUnit::TestFixture
{
    void testCreateFilledPolygon();
    void testDrawPolyPolygonOutline();
    void testFillPolyPolygon();
    void testDrawB2DPolyLine();

    CPPUNIT_TEST_SUITE(B2DPolyLineDrawableTest);
    CPPUNIT_TEST(testCreateFilledPolygon);
    CPPUNIT_TEST(testDrawPolyPolygonOutline);
    CPPUNIT_TEST(testFillPolyPolygon);
    CPPUNIT_TEST(testDrawB2DPolyLine);
    CPPUNIT_TEST_SUITE_END();
};

void B2DPolyLineDrawableTest::testCreateFilledPolygon()
{
    const sal_uInt32 NUMPOLYGONS = 1;
    const sal_uInt32 NUMPOINTS = 6;

    const basegfx::B2DPolygon aTestPolygon({ basegfx::B2DPoint(0, 0), basegfx::B2DPoint(0, 10) });
    LineInfo aTestLineInfo;
    aTestLineInfo.SetWidth(1);

    basegfx::B2DPolyPolygon aTestFilledPolygon(
        vcl::B2DPolyLineDrawableHelper::CreateFilledPolygon(aTestPolygon, aTestLineInfo, 0));

    CPPUNIT_ASSERT_EQUAL(NUMPOLYGONS, aTestFilledPolygon.count());
    CPPUNIT_ASSERT_EQUAL(NUMPOINTS, aTestFilledPolygon.getB2DPolygon(0).count());

    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(-1, 0),
                         aTestFilledPolygon.getB2DPolygon(0).getB2DPoint(0));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(0, 0),
                         aTestFilledPolygon.getB2DPolygon(0).getB2DPoint(1));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(1, 0),
                         aTestFilledPolygon.getB2DPolygon(0).getB2DPoint(2));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(1, 10),
                         aTestFilledPolygon.getB2DPolygon(0).getB2DPoint(3));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(0, 10),
                         aTestFilledPolygon.getB2DPolygon(0).getB2DPoint(4));
    CPPUNIT_ASSERT_EQUAL(basegfx::B2DPoint(-1, 10),
                         aTestFilledPolygon.getB2DPolygon(0).getB2DPoint(5));
}

void B2DPolyLineDrawableTest::testDrawPolyPolygonOutline()
{
    const basegfx::B2DPolyPolygon aTestPolyPolygon(
        { basegfx::B2DPoint(0, 0), basegfx::B2DPoint(0, 10) });
    VclPtrInstance<VirtualDevice> pRenderContext;

    vcl::B2DPolyLineDrawableHelper::DrawPolyPolygonOutline(pRenderContext, aTestPolyPolygon);
}

void B2DPolyLineDrawableTest::testFillPolyPolygon()
{
    const basegfx::B2DPolyPolygon aTestPolyPolygon(
        { basegfx::B2DPoint(0, 0), basegfx::B2DPoint(0, 10) });

    LineInfo aTestLineInfo;
    aTestLineInfo.SetWidth(1);

    VclPtrInstance<VirtualDevice> pRenderContext;

    vcl::B2DPolyLineDrawableHelper::FillPolyPolygon(pRenderContext, aTestPolyPolygon,
                                                    aTestLineInfo);
}

void B2DPolyLineDrawableTest::testDrawB2DPolyLine()
{
    const basegfx::B2DPolygon aTestPolygon({ basegfx::B2DPoint(0, 0), basegfx::B2DPoint(0, 10) });

    LineInfo aTestLineInfo;
    aTestLineInfo.SetWidth(1);

    VclPtrInstance<VirtualDevice> pRenderContext;

    vcl::B2DPolyLineDrawableHelper::DrawB2DPolyLine(pRenderContext, aTestPolygon, aTestLineInfo,
                                                    15.0);
}
} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(B2DPolyLineDrawableTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
