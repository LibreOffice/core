/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>

#include <vcl/fontcharmap.hxx>

class VclFontCharMapTest : public test::BootstrapFixture
{
public:
    VclFontCharMapTest() : BootstrapFixture(true, false) {}

    void testDefaultFontCharMap();

    CPPUNIT_TEST_SUITE(VclFontCharMapTest);
    CPPUNIT_TEST(testDefaultFontCharMap);
    CPPUNIT_TEST_SUITE_END();
};

void VclFontCharMapTest::testDefaultFontCharMap()
{
    FontCharMapRef xfcmap( new FontCharMap() ); // gets default map

    CPPUNIT_ASSERT( xfcmap->IsDefaultMap() );

    sal_uInt32 nStartBMPPlane = xfcmap->GetFirstChar();
    sal_uInt32 nStartSupBMPPlane = xfcmap->GetNextChar(0xD800);
    sal_uInt32 nEndBMPPlane = xfcmap->GetLastChar();

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(0x0020), nStartBMPPlane);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(0xE000), nStartSupBMPPlane);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(0xFFF0-1), nEndBMPPlane);
}

CPPUNIT_TEST_SUITE_REGISTRATION(VclFontCharMapTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
