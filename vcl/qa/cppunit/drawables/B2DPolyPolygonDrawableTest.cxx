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

#include <vcl/drawables/B2DPolyPolygonDrawable.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/virdev.hxx>
#include <vcl/vclptr.hxx>

namespace
{
class B2DPolyPolygonDrawableTest : public CppUnit::TestFixture
{
    void testB2DPolyPolygonDrawable();
    void testB2DPolyPolygonDrawable2();

    CPPUNIT_TEST_SUITE(B2DPolyPolygonDrawableTest);
    CPPUNIT_TEST(testB2DPolyPolygonDrawable);
    CPPUNIT_TEST(testB2DPolyPolygonDrawable2);
    CPPUNIT_TEST_SUITE_END();
};

void B2DPolyPolygonDrawableTest::testB2DPolyPolygonDrawable()
{
    basegfx::B2DPolygon aPolygon1{ { 0, 0 }, { 10, 10 }, { 20, 20 } };
    basegfx::B2DPolygon aPolygon2{ { 30, 30 }, { 40, 40 }, { 50, 50 } };
    basegfx::B2DPolyPolygon aPolyPolygon;

    aPolyPolygon.append(aPolygon1);
    aPolyPolygon.append(aPolygon2);

    VclPtrInstance<VirtualDevice> pRenderContext;

    CPPUNIT_ASSERT(pRenderContext->Draw(vcl::B2DPolyPolygonDrawable(aPolyPolygon)));
}

void B2DPolyPolygonDrawableTest::testB2DPolyPolygonDrawable2()
{
    tools::Polygon aPolygon(3);
    aPolygon.SetPoint(Point(1, 8), 0);
    aPolygon.SetPoint(Point(2, 7), 1);
    aPolygon.SetPoint(Point(3, 6), 2);

    tools::Polygon aPolygonWithControl(4);
    aPolygonWithControl.SetPoint(Point(8, 1), 0);
    aPolygonWithControl.SetPoint(Point(7, 2), 1);
    aPolygonWithControl.SetPoint(Point(6, 3), 2);
    aPolygonWithControl.SetPoint(Point(5, 4), 3);

    aPolygonWithControl.SetFlags(0, PolyFlags::Normal);
    aPolygonWithControl.SetFlags(1, PolyFlags::Control);
    aPolygonWithControl.SetFlags(2, PolyFlags::Smooth);
    aPolygonWithControl.SetFlags(3, PolyFlags::Symmetric);

    tools::PolyPolygon aPolyPolygon(2);
    aPolyPolygon.Insert(aPolygon);
    aPolyPolygon.Insert(aPolygonWithControl);

    VclPtrInstance<VirtualDevice> pRenderContext;

    CPPUNIT_ASSERT(
        pRenderContext->Draw(vcl::B2DPolyPolygonDrawable(aPolyPolygon.getB2DPolyPolygon())));
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(B2DPolyPolygonDrawableTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
