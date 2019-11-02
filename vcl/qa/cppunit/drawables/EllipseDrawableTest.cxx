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

#include <vcl/drawables/EllipseDrawable.hxx>
#include <vcl/virdev.hxx>
#include <vcl/vclptr.hxx>

namespace
{
class EllipseDrawableTest : public CppUnit::TestFixture
{
    void testEllipseDrawable();

    CPPUNIT_TEST_SUITE(EllipseDrawableTest);
    CPPUNIT_TEST(testEllipseDrawable);
    CPPUNIT_TEST_SUITE_END();
};

void EllipseDrawableTest::testEllipseDrawable()
{
    VclPtrInstance<VirtualDevice> pRenderContext;

    CPPUNIT_ASSERT(pRenderContext->Draw(vcl::EllipseDrawable(tools::Rectangle(0, 0, 10, 10))));
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(EllipseDrawableTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
