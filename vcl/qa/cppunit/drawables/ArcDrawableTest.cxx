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

#include <vcl/drawables/ArcDrawable.hxx>
#include <vcl/virdev.hxx>
#include <vcl/vclptr.hxx>

namespace
{
class ArcDrawableTest : public CppUnit::TestFixture
{
    void testArcDrawable();

    CPPUNIT_TEST_SUITE(ArcDrawableTest);
    CPPUNIT_TEST(testArcDrawable);
    CPPUNIT_TEST_SUITE_END();
};

void ArcDrawableTest::testArcDrawable()
{
    VclPtrInstance<VirtualDevice> pRenderContext;

    CPPUNIT_ASSERT(pRenderContext->Draw(
        vcl::ArcDrawable(tools::Rectangle(0, 0, 10, 10), Point(0, 0), Point(5, 5))));
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(ArcDrawableTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
