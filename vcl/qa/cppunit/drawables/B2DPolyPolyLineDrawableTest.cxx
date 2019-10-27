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

namespace
{
class B2DPolyPolyLineDrawableTest : public CppUnit::TestFixture
{
    void testCanApplyDashing();
    void testGenerateDotDashArray();
    void testDashDotArrayHasLength();

    CPPUNIT_TEST_SUITE(B2DPolyPolyLineDrawableTest);
    CPPUNIT_TEST(testGenerateDotDashArray);
    CPPUNIT_TEST(testCanApplyDashing);
    CPPUNIT_TEST(testDashDotArrayHasLength);
    CPPUNIT_TEST_SUITE_END();
};

void B2DPolyPolyLineDrawableTest::testCanApplyDashing()
{
    const basegfx::B2DPolyPolygon aTestPolyPolygon(
        { basegfx::B2DPoint(0, 0), basegfx::B2DPoint(1, 1) });
    LineInfo aTestLineInfo;
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

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(B2DPolyPolyLineDrawableTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
