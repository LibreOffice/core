/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_features.h>

#include <ostream>
#include <vector>
#if !defined(_WIN32) && HAVE_MORE_FONTS
static std::ostream& operator<<(std::ostream& rStream, const std::vector<long>& rVec);
#endif

#include <unotest/filters-test.hxx>
#include <test/bootstrapfixture.hxx>

#include <vcl/wrkwin.hxx>
// workaround MSVC2015 issue with std::unique_ptr
#include <sallayout.hxx>

#include <osl/file.hxx>
#include <osl/process.h>

#if !defined(_WIN32) && HAVE_MORE_FONTS
static std::ostream& operator<<(std::ostream& rStream, const std::vector<long>& rVec)
{
    rStream << "{ ";
    for (size_t i = 0; i < rVec.size() - 1; i++)
        rStream << rVec[i] << ", ";
    rStream << rVec.back();
    rStream << " }";
    return rStream;
}
#endif

class VclComplexTextTest : public test::BootstrapFixture
{
public:
    VclComplexTextTest() : BootstrapFixture(true, false) {}

#if HAVE_MORE_FONTS
    /// Play with font measuring etc.
    void testArabic();
#endif
#if defined(_WIN32)
    void testTdf95650(); // Windows-only issue
#endif

    CPPUNIT_TEST_SUITE(VclComplexTextTest);
#if HAVE_MORE_FONTS
    CPPUNIT_TEST(testArabic);
#endif
#if defined(_WIN32)
    CPPUNIT_TEST(testTdf95650);
#endif
    CPPUNIT_TEST_SUITE_END();
};

#if HAVE_MORE_FONTS
void VclComplexTextTest::testArabic()
{
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
    ScopedVclPtrInstance<WorkWindow> pWin(static_cast<vcl::Window *>(nullptr));
    CPPUNIT_ASSERT( pWin );

    vcl::Font aFont("DejaVu Sans", "Book", Size(0, 12));

    OutputDevice *pOutDev = pWin.get();
    pOutDev->SetFont( aFont );

    // absolute character widths AKA text array.
#if !defined(_WIN32)
    std::vector<long> aRefCharWidths {6,  9,  16, 16, 22, 22, 26, 29, 32, 32,
                                      36, 40, 49, 53, 56, 63, 63, 66, 72, 72};
    std::vector<long> aCharWidths(aOneTwoThree.getLength(), 0);
    long nTextWidth = pOutDev->GetTextArray(aOneTwoThree, aCharWidths.data());

    CPPUNIT_ASSERT_EQUAL(aRefCharWidths, aCharWidths);
    // this sporadically returns 75 or 74 on some of the windows tinderboxes eg. tb73
    CPPUNIT_ASSERT_EQUAL(72L, nTextWidth);
    CPPUNIT_ASSERT_EQUAL(nTextWidth, aCharWidths.back());
#endif

    // text advance width and line height
    CPPUNIT_ASSERT_EQUAL(72L, pOutDev->GetTextWidth(aOneTwoThree));
    CPPUNIT_ASSERT_EQUAL(14L, pOutDev->GetTextHeight());

    // exact bounding rectangle, not essentially the same as text width/height
#if defined(MACOSX) || defined(_WIN32)
    // FIXME: fails on some Linux tinderboxes, might be a FreeType issue.
    tools::Rectangle aBoundRect, aTestRect( 0, 1, 71, 15 );
    pOutDev->GetTextBoundRect(aBoundRect, aOneTwoThree);
#if defined(_WIN32)
    // if run on Win7 KVM QXL / Spice GUI, we "miss" the first pixel column?!
    if ( 1 == aBoundRect.Left() )
    {
        aTestRect.AdjustLeft(1);
    }
#endif
    CPPUNIT_ASSERT_EQUAL(aTestRect, aBoundRect);
#endif

    // normal orientation
    tools::Rectangle aInput;
    tools::Rectangle aRect = pOutDev->GetTextRect( aInput, aOneTwoThree );

    // now rotate 270 degress
    vcl::Font aRotated( aFont );
    aRotated.SetOrientation( 2700 );
    pOutDev->SetFont( aRotated );
    tools::Rectangle aRectRot = pOutDev->GetTextRect( aInput, aOneTwoThree );

    // Check that we did do the rotation ...
#if 0
    // FIXME: This seems to be wishful thinking, GetTextRect() does not take
    // rotation into account.
    fprintf( stderr, "%ld %ld %ld %ld\n",
             aRect.GetWidth(), aRect.GetHeight(),
             aRectRot.GetWidth(), aRectRot.GetHeight() );
    CPPUNIT_ASSERT( aRectRot.GetWidth() == aRect.GetHeight() );
    CPPUNIT_ASSERT( aRectRot.GetHeight() == aRect.GetWidth() );
#else
    (void)aRect; (void)aRectRot;
#endif
}
#endif

#if defined(_WIN32)
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
    ScopedVclPtrInstance<WorkWindow> pWin(static_cast<vcl::Window *>(nullptr));
    CPPUNIT_ASSERT(pWin);

    OutputDevice *pOutDev = pWin.get();
    // Check that the following executes without failing assertion
    pOutDev->ImplLayout(aTxt, 9, 1, Point(), 0, nullptr, SalLayoutFlags::BiDiRtl);
}
#endif

CPPUNIT_TEST_SUITE_REGISTRATION(VclComplexTextTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
