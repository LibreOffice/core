/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <unotest/filters-test.hxx>
#include <test/bootstrapfixture.hxx>

#include <vcl/wrkwin.hxx>

#include <osl/file.hxx>
#include <osl/process.h>

class VclComplexTextTest : public test::BootstrapFixture
{
public:
    VclComplexTextTest() : BootstrapFixture(true, false) {}

    /// Play with font measuring etc.
    void testArabic();
#if defined(WNT)
    void testTdf95650(); // Windows-only issue
#endif

    CPPUNIT_TEST_SUITE(VclComplexTextTest);
    CPPUNIT_TEST(testArabic);
#if defined(WNT)
    CPPUNIT_TEST(testTdf95650);
#endif
    CPPUNIT_TEST_SUITE_END();
};

void VclComplexTextTest::testArabic()
{
#if !defined (LINUX)
    return;
#else // only tested on Linux so far
    const unsigned char pOneTwoThreeUTF8[] = {
        0xd9, 0x88, 0xd8, 0xa7, 0xd8, 0xad, 0xd9, 0x90,
        0xd8, 0xaf, 0xd9, 0x92, 0x20, 0xd8, 0xa5, 0xd8,
        0xab, 0xd9, 0x8d, 0xd9, 0x86, 0xd9, 0x8a, 0xd9,
        0x86, 0x20, 0xd8, 0xab, 0xd9, 0x84, 0xd8, 0xa7,
        0xd8, 0xab, 0xd8, 0xa9, 0xd9, 0x8c, 0x00
    };
    OUString aOneTwoThree( reinterpret_cast<char const *>(pOneTwoThreeUTF8),
                           SAL_N_ELEMENTS( pOneTwoThreeUTF8 ) - 1,
                           RTL_TEXTENCODING_UTF8 );
    VclPtr<vcl::Window> pWin = VclPtr<WorkWindow>::Create( static_cast<vcl::Window *>(nullptr) );
    CPPUNIT_ASSERT( pWin );

    OutputDevice *pOutDev = static_cast< OutputDevice * >( pWin.get() );

    vcl::Font aFont = OutputDevice::GetDefaultFont(
                        DefaultFontType::CTL_SPREADSHEET,
                        LANGUAGE_ARABIC_SAUDI_ARABIA,
                        GetDefaultFontFlags::OnlyOne );
    pOutDev->SetFont( aFont );

    // normal orientation
    Rectangle aInput;
    Rectangle aRect = pOutDev->GetTextRect( aInput, aOneTwoThree );

    // now rotate 270 degress
    vcl::Font aRotated( aFont );
    aRotated.SetOrientation( 2700 );
    pOutDev->SetFont( aRotated );
    Rectangle aRectRot = pOutDev->GetTextRect( aInput, aOneTwoThree );

    // Check that we did do the rotation ...
#if 0
    fprintf( stderr, "%ld %ld %ld %ld\n",
             aRect.GetWidth(), aRect.GetHeight(),
             aRectRot.GetWidth(), aRectRot.GetHeight() );
    CPPUNIT_ASSERT( aRectRot.GetWidth() == aRect.GetHeight() );
    CPPUNIT_ASSERT( aRectRot.GetHeight() == aRect.GetWidth() );
#else
    (void)aRect; (void)aRectRot;
#endif
#endif
}

#if defined(WNT)
void VclComplexTextTest::testTdf95650()
{
    const sal_Unicode pTxt[] = {
        0x0131, 0x0302, 0x0504, 0x4E44, 0x3031, 0x3030, 0x3531, 0x2D30,
        0x3037, 0x0706, 0x0908, 0x0B0A, 0x0D0C, 0x0F0E, 0x072E, 0x100A,
        0x0D11, 0x1312, 0x0105, 0x020A, 0x0512, 0x1403, 0x030C, 0x1528,
        0x2931, 0x632E, 0x7074, 0x0D20, 0x0E0A, 0x100A, 0xF00D, 0x0D20,
        0x030A, 0x0C0B, 0x20E0, 0x0A0D
    };
    OUString aTxt(pTxt, SAL_N_ELEMENTS(pTxt) - 1);
    VclPtr<vcl::Window> pWin = VclPtr<WorkWindow>::Create(static_cast<vcl::Window *>(nullptr));
    CPPUNIT_ASSERT(pWin);

    OutputDevice *pOutDev = static_cast< OutputDevice * >(pWin.get());
    // Check that the following executes without failing assertion
    pOutDev->ImplLayout(aTxt, 9, 1, Point(), 0, 0, SalLayoutFlags::BiDiRtl, nullptr);
}
#endif

CPPUNIT_TEST_SUITE_REGISTRATION(VclComplexTextTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
