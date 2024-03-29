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
    ScopedVclPtr<VirtualDevice> device = VclPtr<VirtualDevice>::Create(DeviceFormat::WITHOUT_ALPHA);
    device->SetOutputSizePixel(Size(1000, 1000));
    vcl::Font font("Liberation Sans", Size(0, 110));
    device->SetFont(font);

    const LogicalFontInstance* pFontInstance = device->GetFontInstance();

    basegfx::B2DRectangle aBoundRect;
    const auto LATIN_SMALL_LETTER_B = 0x0062;
    pFontInstance->GetGlyphBoundRect(pFontInstance->GetGlyphIndex(LATIN_SMALL_LETTER_B), aBoundRect,
                                     false);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(7.1, aBoundRect.getMinX(), 0.05);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-79.7, aBoundRect.getMinY(), 0.05);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(49.5, aBoundRect.getWidth(), 0.05);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(80.8, aBoundRect.getHeight(), 0.05);

    font.SetOrientation(900_deg10);
    device->SetFont(font);

    pFontInstance = device->GetFontInstance();

    pFontInstance->GetGlyphBoundRect(pFontInstance->GetGlyphIndex(LATIN_SMALL_LETTER_B), aBoundRect,
                                     false);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(-79.7, aBoundRect.getMinX(), 0.05);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-56.6, aBoundRect.getMinY(), 0.05);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(80.8, aBoundRect.getWidth(), 0.05);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(49.5, aBoundRect.getHeight(), 0.05);

    font.SetOrientation(450_deg10);
    device->SetFont(font);

    pFontInstance = device->GetFontInstance();

    pFontInstance->GetGlyphBoundRect(pFontInstance->GetGlyphIndex(LATIN_SMALL_LETTER_B), aBoundRect,
                                     false);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(-51.3, aBoundRect.getMinX(), 0.05);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-96.4, aBoundRect.getMinY(), 0.05);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(92.1, aBoundRect.getWidth(), 0.05);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(92.1, aBoundRect.getHeight(), 0.05);
}

CPPUNIT_TEST_SUITE_REGISTRATION(VclLogicalFontInstanceTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
