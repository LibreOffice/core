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

#include <vcl/drawables/ChordDrawable.hxx>
#include <vcl/virdev.hxx>
#include <vcl/vclptr.hxx>

namespace
{
class ChordDrawableTest : public CppUnit::TestFixture
{
    void testChordDrawable();

    CPPUNIT_TEST_SUITE(ChordDrawableTest);
    CPPUNIT_TEST(testChordDrawable);
    CPPUNIT_TEST_SUITE_END();
};

void ChordDrawableTest::testChordDrawable()
{
    VclPtrInstance<VirtualDevice> pRenderContext;

    CPPUNIT_ASSERT(pRenderContext->Draw(
        vcl::ChordDrawable(tools::Rectangle(0, 0, 10, 10), Point(0, 0), Point(5, 5))));
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(ChordDrawableTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
