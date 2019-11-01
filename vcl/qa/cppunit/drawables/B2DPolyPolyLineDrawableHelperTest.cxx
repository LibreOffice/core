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

#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawables/B2DPolyPolyLineDrawableHelper.hxx>
#include <vcl/virdev.hxx>

namespace
{
class B2DPolyPolyLineDrawableTest : public CppUnit::TestFixture
{
    void testCanApplyDashes();
    void testDashDotArrayHasLength();
    void testGenerateDotDashArray();
    void testApplyLineDashing();
    void testUseLineWidth();
    void testCreateFillPolyPolygon();
    void testFillPolyPolygon();
    void testDrawPolyPolyLine();
    void testDrawPolyLine();
    void testDrawPolyPolygonFallback();
    void testDrawPolyLineFallback();

    CPPUNIT_TEST_SUITE(B2DPolyPolyLineDrawableTest);
    CPPUNIT_TEST(testCanApplyDashes);
    CPPUNIT_TEST(testDashDotArrayHasLength);
    CPPUNIT_TEST(testGenerateDotDashArray);
    CPPUNIT_TEST(testApplyLineDashing);
    CPPUNIT_TEST(testUseLineWidth);
    CPPUNIT_TEST(testCreateFillPolyPolygon);
    CPPUNIT_TEST(testDrawPolyPolyLine);
    CPPUNIT_TEST(testDrawPolyLine);
    CPPUNIT_TEST(testFillPolyPolygon);
    CPPUNIT_TEST(testDrawPolyPolygonFallback);
    CPPUNIT_TEST(testDrawPolyLineFallback);
    CPPUNIT_TEST_SUITE_END();
};

void B2DPolyPolyLineDrawableTest::testCanApplyDashes()
{
    const basegfx::B2DPolyPolygon aTestPolyPolygon(
        { basegfx::B2DPoint(0, 0), basegfx::B2DPoint(1, 1) });
    LineInfo aTestLineInfo;
    aTestLineInfo.SetWidth(1);
    aTestLineInfo.SetStyle(LineStyle::Dash);

    CPPUNIT_ASSERT(
        vcl::B2DPolyPolyLineDrawableHelper::CanApplyDashes(aTestPolyPolygon, aTestLineInfo));

    aTestLineInfo.SetStyle(LineStyle::Solid);
    CPPUNIT_ASSERT(
        !vcl::B2DPolyPolyLineDrawableHelper::CanApplyDashes(aTestPolyPolygon, aTestLineInfo));

    aTestLineInfo.SetStyle(LineStyle::NONE);
    CPPUNIT_ASSERT(
        !vcl::B2DPolyPolyLineDrawableHelper::CanApplyDashes(aTestPolyPolygon, aTestLineInfo));
}

void B2DPolyPolyLineDrawableTest::testDashDotArrayHasLength()
{
    const double DASHCOUNT = 1;
    const double DOTCOUNT = 1;
    const double DASHLEN = 5;
    const double DOTLEN = 10;

    const basegfx::B2DPolyPolygon aTestPolyPolygon(
        { basegfx::B2DPoint(0, 0), basegfx::B2DPoint(1, 1) });
    LineInfo aTestLineInfo;
    aTestLineInfo.SetStyle(LineStyle::Dash);
    aTestLineInfo.SetDashCount(DASHCOUNT);
    aTestLineInfo.SetDotCount(DOTCOUNT);
    aTestLineInfo.SetDashLen(DASHLEN);
    aTestLineInfo.SetDotLen(DOTLEN);

    CPPUNIT_ASSERT(vcl::B2DPolyPolyLineDrawableHelper::DashDotArrayHasLength(
        vcl::B2DPolyPolyLineDrawableHelper::GenerateDotDashArray(aTestLineInfo)));
}

void B2DPolyPolyLineDrawableTest::testGenerateDotDashArray()
{
    const double DASHCOUNT = 1;
    const double DOTCOUNT = 1;
    const double DASHLEN = 5;
    const double DOTLEN = 10;
    const double DIST = 0;

    const basegfx::B2DPolyPolygon aTestPolyPolygon(
        { basegfx::B2DPoint(0, 0), basegfx::B2DPoint(1, 1) });
    LineInfo aTestLineInfo;
    aTestLineInfo.SetStyle(LineStyle::Dash);
    aTestLineInfo.SetDashCount(DASHCOUNT);
    aTestLineInfo.SetDotCount(DOTCOUNT);
    aTestLineInfo.SetDashLen(DASHLEN);
    aTestLineInfo.SetDotLen(DOTLEN);

    std::vector<double> aDashDotArray(
        vcl::B2DPolyPolyLineDrawableHelper::GenerateDotDashArray(aTestLineInfo));

    CPPUNIT_ASSERT_EQUAL(aDashDotArray[0], DASHLEN);
    CPPUNIT_ASSERT_EQUAL(aDashDotArray[1], DIST);
    CPPUNIT_ASSERT_EQUAL(aDashDotArray[2], DOTLEN);
    CPPUNIT_ASSERT_EQUAL(aDashDotArray[3], DIST);
}

void B2DPolyPolyLineDrawableTest::testApplyLineDashing()
{
    LineInfo aTestLineInfo;
    const basegfx::B2DPolyPolygon aTestPolyPolygon(
        { basegfx::B2DPoint(0, 0), basegfx::B2DPoint(1, 1) });

    basegfx::B2DPolyPolygon aResult
        = vcl::B2DPolyPolyLineDrawableHelper::ApplyLineDashing(aTestPolyPolygon, aTestLineInfo);

    CPPUNIT_ASSERT(aResult.count());
}

void B2DPolyPolyLineDrawableTest::testUseLineWidth()
{
    const sal_Int32 WIDTH = 5;

    LineInfo aTestLineInfo;
    const basegfx::B2DPolyPolygon aTestPolyPolygon(
        { basegfx::B2DPoint(0, 0), basegfx::B2DPoint(1, 1) });

    CPPUNIT_ASSERT(
        !vcl::B2DPolyPolyLineDrawableHelper::UseLineWidth(aTestPolyPolygon, aTestLineInfo));

    aTestLineInfo.SetWidth(WIDTH);
    CPPUNIT_ASSERT(
        vcl::B2DPolyPolyLineDrawableHelper::UseLineWidth(aTestPolyPolygon, aTestLineInfo));
}

void B2DPolyPolyLineDrawableTest::testCreateFillPolyPolygon()
{
    const sal_Int32 WIDTH = 5;

    LineInfo aTestLineInfo;
    basegfx::B2DPolyPolygon aTestPolyPolygon(
        { basegfx::B2DPoint(0, 0), basegfx::B2DPoint(5, 5), basegfx::B2DPoint(10, 10) });

    aTestLineInfo.SetWidth(WIDTH);

    basegfx::B2DPolyPolygon aResult = vcl::B2DPolyPolyLineDrawableHelper::CreateFillPolyPolygon(
        aTestPolyPolygon, aTestLineInfo);

    CPPUNIT_ASSERT(aResult.count());
}

void B2DPolyPolyLineDrawableTest::testFillPolyPolygon()
{
    LineInfo aTestLineInfo;
    aTestLineInfo.SetWidth(1);
    const basegfx::B2DPolyPolygon aTestFillPolyPolygon(
        { basegfx::B2DPoint(0, 0), basegfx::B2DPoint(1, 1), basegfx::B2DPoint(2, 2) });
    VclPtrInstance<VirtualDevice> pRenderContext;

    vcl::B2DPolyPolyLineDrawableHelper::FillPolyPolygon(pRenderContext, aTestFillPolyPolygon);
}

void B2DPolyPolyLineDrawableTest::testDrawPolyPolyLine()
{
    LineInfo aTestLineInfo;
    aTestLineInfo.SetWidth(1);
    const basegfx::B2DPolyPolygon aTestPolyPolygon(
        { basegfx::B2DPoint(0, 0), basegfx::B2DPoint(1, 1), basegfx::B2DPoint(2, 2) });
    VclPtrInstance<VirtualDevice> pRenderContext;

    vcl::B2DPolyPolyLineDrawableHelper::DrawPolyPolyLine(pRenderContext, aTestPolyPolygon);
}

void B2DPolyPolyLineDrawableTest::testDrawPolyLine()
{
    LineInfo aTestLineInfo;
    aTestLineInfo.SetWidth(1);
    const basegfx::B2DPolygon aTestPolygon(
        { basegfx::B2DPoint(0, 0), basegfx::B2DPoint(1, 1), basegfx::B2DPoint(2, 2) });
    VclPtrInstance<VirtualDevice> pRenderContext;

    vcl::B2DPolyPolyLineDrawableHelper::DrawPolyLine(pRenderContext, aTestPolygon);
}

void B2DPolyPolyLineDrawableTest::testDrawPolyPolygonFallback()
{
    LineInfo aTestLineInfo;
    aTestLineInfo.SetWidth(1);
    const basegfx::B2DPolyPolygon aTestPolyPolygon(
        { basegfx::B2DPoint(0, 0), basegfx::B2DPoint(1, 1), basegfx::B2DPoint(2, 2) });
    VclPtrInstance<VirtualDevice> pRenderContext;

    vcl::B2DPolyPolyLineDrawableHelper::DrawPolyPolygonFallback(pRenderContext, aTestPolyPolygon);
}

void B2DPolyPolyLineDrawableTest::testDrawPolyLineFallback()
{
    LineInfo aTestLineInfo;
    aTestLineInfo.SetWidth(1);
    const basegfx::B2DPolygon aTestPolygon(
        { basegfx::B2DPoint(0, 0), basegfx::B2DPoint(1, 1), basegfx::B2DPoint(2, 2) });
    VclPtrInstance<VirtualDevice> pRenderContext;

    vcl::B2DPolyPolyLineDrawableHelper::DrawPolyLineFallback(pRenderContext, aTestPolygon);
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(B2DPolyPolyLineDrawableTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
