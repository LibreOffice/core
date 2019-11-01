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

#include <vcl/drawables/B2DPolygonDrawable.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/virdev.hxx>
#include <vcl/vclptr.hxx>

namespace
{
class B2DPolygonDrawableTest : public CppUnit::TestFixture
{
    void testB2DPolygonDrawable();

    CPPUNIT_TEST_SUITE(B2DPolygonDrawableTest);
    CPPUNIT_TEST(testB2DPolygonDrawable);
    CPPUNIT_TEST_SUITE_END();
};

void B2DPolygonDrawableTest::testB2DPolygonDrawable()
{
    basegfx::B2DPolygon aPolygon{ { 0, 0 }, { 10, 10 }, { 20, 20 } };
    VclPtrInstance<VirtualDevice> pRenderContext;

    CPPUNIT_ASSERT(pRenderContext->Draw(vcl::B2DPolygonDrawable(aPolygon)));
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(B2DPolygonDrawableTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
