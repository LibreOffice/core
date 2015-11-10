/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>

#include <osl/file.hxx>
#include <osl/process.h>

#include <vcl/metric.hxx>

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
    FontCharMapPtr pfcmap( new FontCharMap() ); // gets default map

    CPPUNIT_ASSERT( pfcmap->IsDefaultMap() );

    sal_uInt32 nStartBMPPlane = pfcmap->GetFirstChar();
    sal_uInt32 nStartSupBMPPlane = pfcmap->GetNextChar(0xD800);
    sal_uInt32 nEndBMPPlane = pfcmap->GetLastChar();

    CPPUNIT_ASSERT( nStartBMPPlane == 0x0020 );
    CPPUNIT_ASSERT( nStartSupBMPPlane == 0xE000 );
    CPPUNIT_ASSERT( nEndBMPPlane == 0xFFF0-1 );

    pfcmap = nullptr;
}

CPPUNIT_TEST_SUITE_REGISTRATION(VclFontCharMapTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
