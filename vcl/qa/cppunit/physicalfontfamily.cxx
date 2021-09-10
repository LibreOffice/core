/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <test/bootstrapfixture.hxx>
#include <cppunit/TestAssert.h>

#include <unotools/fontcfg.hxx>
#include <o3tl/sorted_vector.hxx>

#include <vcl/virdev.hxx>

#include <font/PhysicalFontFamily.hxx>

#include "fontmocks.hxx"

using namespace vcl::font;

class VclPhysicalFontFaceTest : public test::BootstrapFixture
{
public:
    VclPhysicalFontFaceTest()
        : BootstrapFixture(true, false)
    {
    }

    void testCreateFontFamily();

    CPPUNIT_TEST_SUITE(VclPhysicalFontFaceTest);
    CPPUNIT_TEST(testCreateFontFamily);
    CPPUNIT_TEST_SUITE_END();
};

void VclPhysicalFontFaceTest::testCreateFontFamily()
{
    PhysicalFontFamily aFamily("DejaVu Sans");

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Family name", OUString(""), aFamily.GetFamilyName());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Search name", OUString("DejaVu Sans"), aFamily.GetSearchName());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Alias names", OUString(""), aFamily.GetAliasNames());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Min quality", -1, aFamily.GetMinQuality());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Type faces", FontTypeFaces::NONE, aFamily.GetTypeFaces());

    o3tl::sorted_vector<int> aHeights;
    aFamily.GetFontHeights(aHeights);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number font heights", static_cast<unsigned long>(0),
                                 aHeights.size());

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Match family name", OUString(""), aFamily.GetMatchFamilyName());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Match type", ImplFontAttrs::None, aFamily.GetMatchType());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Match weight", WEIGHT_DONTKNOW, aFamily.GetMatchWeight());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Match width", WIDTH_DONTKNOW, aFamily.GetMatchWidth());
}

CPPUNIT_TEST_SUITE_REGISTRATION(VclPhysicalFontFaceTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
