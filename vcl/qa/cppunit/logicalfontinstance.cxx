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

#include <vcl/virdev.hxx>
#include <vcl/font.hxx>

#include <font/LogicalFontInstance.hxx>

#include <memory>

class VclLogicalFontInstanceTest : public test::BootstrapFixture
{
public:
    VclLogicalFontInstanceTest()
        : BootstrapFixture(true, false)
    {
    }

    void testglyphboundrect();

    CPPUNIT_TEST_SUITE(VclLogicalFontInstanceTest);
    CPPUNIT_TEST(testglyphboundrect);

    CPPUNIT_TEST_SUITE_END();
};

void VclLogicalFontInstanceTest::testglyphboundrect()
{
    ScopedVclPtr<VirtualDevice> device = VclPtr<VirtualDevice>::Create(DeviceFormat::DEFAULT);
    device->SetOutputSizePixel(Size(1000, 1000));
    device->SetFont(vcl::Font("Liberation Sans", Size(0, 110)));

    const LogicalFontInstance* pFontInstance = device->GetFontInstance();

    tools::Rectangle aBoundRect;
    const auto LATIN_SMALL_LETTER_B = 0x0062;
    pFontInstance->GetGlyphBoundRect(pFontInstance->GetGlyphIndex(LATIN_SMALL_LETTER_B), aBoundRect,
                                     false);

    const tools::Long nExpectedX = 7;
    const tools::Long nExpectedY = -80;
    const tools::Long nExpectedWidth = 51;
    const tools::Long nExpectedHeight = 83;

    CPPUNIT_ASSERT_EQUAL_MESSAGE("x of glyph is wrong", nExpectedX, aBoundRect.getX());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("y of glyph is wrong", nExpectedY, aBoundRect.getY());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("height of glyph of wrong", nExpectedWidth, aBoundRect.GetWidth());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("width of glyph of wrong", nExpectedHeight,
                                 aBoundRect.GetHeight());
}

CPPUNIT_TEST_SUITE_REGISTRATION(VclLogicalFontInstanceTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
