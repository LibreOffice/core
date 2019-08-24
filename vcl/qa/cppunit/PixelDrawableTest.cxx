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

#include <vcl/drawables/PixelDrawable.hxx>
#include <vcl/virdev.hxx>
#include <vcl/vclptr.hxx>

namespace
{
class PixelDrawableTest : public CppUnit::TestFixture
{
    void testPixelDrawable();

    CPPUNIT_TEST_SUITE(PixelDrawableTest);
    CPPUNIT_TEST(testPixelDrawable);
    CPPUNIT_TEST_SUITE_END();
};

void PixelDrawableTest::testPixelDrawable()
{
    VclPtrInstance<VirtualDevice> pRenderContext;
    pRenderContext->SetOutputSizePixel(Size(20, 20));

    CPPUNIT_ASSERT(pRenderContext->Draw(vcl::PixelDrawable(Point(10, 10))));
    CPPUNIT_ASSERT(pRenderContext->Draw(vcl::PixelDrawable(Point(10, 10), COL_GREEN)));
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(PixelDrawableTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
