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
#include <tools/long.hxx>

#if HAVE_MORE_FONTS
// must be declared before inclusion of test/bootstrapfixture.hxx
static std::ostream& operator<<(std::ostream& rStream, const std::vector<tools::Long>& rVec);
#endif
#include <test/bootstrapfixture.hxx>

#include <vcl/wrkwin.hxx>
#include <vcl/virdev.hxx>
// workaround MSVC2015 issue with std::unique_ptr
#include <sallayout.hxx>
#include <salgdi.hxx>

#if HAVE_MORE_FONTS
static std::ostream& operator<<(std::ostream& rStream, const std::vector<tools::Long>& rVec)
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

    /// Play with font measuring etc.
    void testArabic();
    void testKashida();
    void testTdf95650(); // Windows-only issue
    void testCaching();

    CPPUNIT_TEST_SUITE(VclComplexTextTest);
    CPPUNIT_TEST(testArabic);
    CPPUNIT_TEST(testKashida);
    CPPUNIT_TEST(testTdf95650);
    CPPUNIT_TEST(testCaching);
    CPPUNIT_TEST_SUITE_END();
};

void VclComplexTextTest::testArabic()
{
#if HAVE_MORE_FONTS
    OUString aOneTwoThree(
        u"\u0648\u0627\u062d\u0650\u062f\u0652 \u0625\u062b\u064d\u0646\u064a\u0646"
        " \u062b\u0644\u0627\u062b\u0629\u064c" );
    ScopedVclPtrInstance<WorkWindow> pWin(static_cast<vcl::Window *>(nullptr));
    CPPUNIT_ASSERT( pWin );

    vcl::Font aFont("DejaVu Sans", "Book", Size(0, 12));

    OutputDevice *pOutDev = pWin.get();
    pOutDev->SetFont( aFont );

    // absolute character widths AKA text array.
    std::vector<tools::Long> aRefCharWidths {6,  9,  16, 16, 22, 22, 26, 29, 32, 32,
                                      36, 40, 49, 53, 56, 63, 63, 66, 72, 72};
    std::vector<tools::Long> aCharWidths(aOneTwoThree.getLength(), 0);
    tools::Long nTextWidth = pOutDev->GetTextArray(aOneTwoThree, aCharWidths.data());

    CPPUNIT_ASSERT_EQUAL(aRefCharWidths, aCharWidths);
    // this sporadically returns 75 or 74 on some of the windows tinderboxes eg. tb73
    CPPUNIT_ASSERT_EQUAL(tools::Long(72), nTextWidth);
    CPPUNIT_ASSERT_EQUAL(nTextWidth, aCharWidths.back());

    // text advance width and line height
    CPPUNIT_ASSERT_EQUAL(tools::Long(72), pOutDev->GetTextWidth(aOneTwoThree));
    CPPUNIT_ASSERT_EQUAL(tools::Long(14), pOutDev->GetTextHeight());

    // exact bounding rectangle, not essentially the same as text width/height
    tools::Rectangle aBoundRect;
    pOutDev->GetTextBoundRect(aBoundRect, aOneTwoThree);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0, aBoundRect.getX(), 1); // This sometimes equals to 1
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1, aBoundRect.getY(), 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(71, aBoundRect.getWidth(), 1); // This sometimes equals to 70
    CPPUNIT_ASSERT_DOUBLES_EQUAL(15, aBoundRect.getHeight(), 1);

#if 0
    // FIXME: This seems to be wishful thinking, GetTextRect() does not take
    // rotation into account.

    // normal orientation
    tools::Rectangle aInput;
    tools::Rectangle aRect = pOutDev->GetTextRect( aInput, aOneTwoThree );

    // now rotate 270 degrees
    vcl::Font aRotated( aFont );
    aRotated.SetOrientation( 2700 );
    pOutDev->SetFont( aRotated );
    tools::Rectangle aRectRot = pOutDev->GetTextRect( aInput, aOneTwoThree );

    // Check that we did do the rotation...
    fprintf( stderr, "%" SAL_PRIdINT64 " %" SAL_PRIdINT64 " %" SAL_PRIdINT64 " %" SAL_PRIdINT64 "\n",
             sal_Int64(aRect.GetWidth()), sal_Int64(aRect.GetHeight()),
             sal-Int64(aRectRot.GetWidth()), sal_Int64(aRectRot.GetHeight()) );
    CPPUNIT_ASSERT( aRectRot.GetWidth() == aRect.GetHeight() );
    CPPUNIT_ASSERT( aRectRot.GetHeight() == aRect.GetWidth() );
#endif
#endif
}

void VclComplexTextTest::testKashida()
{
#if HAVE_MORE_FONTS
    // Cache the glyph list of some Arabic text.
    ScopedVclPtrInstance<VirtualDevice> pOutputDevice;
    auto aText
        = OUString(u"ﻊﻨﺻﺭ ﺎﻠﻓﻮﺴﻓﻭﺭ ﻊﻨﺻﺭ ﻒﻟﺰﻳ ﺺﻠﺑ. ﺖﺘﻛﻮﻧ ﺎﻟﺩﻭﺭﺓ ﺎﻟﺭﺎﺒﻋﺓ ﻢﻧ ١٥ ﻊﻨﺻﺭﺍ.");
    std::unique_ptr<SalLayout> pLayout = pOutputDevice->ImplLayout(
        aText, 0, aText.getLength(), Point(0, 0), 0, nullptr, SalLayoutFlags::GlyphItemsOnly);
    SalLayoutGlyphs aGlyphs = pLayout->GetGlyphs();
    CPPUNIT_ASSERT(aGlyphs.IsValid());
    CPPUNIT_ASSERT(aGlyphs.Impl(0) != nullptr);

    // Now lay it out using the cached glyph list.
    ImplLayoutArgs aLayoutArgs(aText, 0, aText.getLength(), SalLayoutFlags::NONE,
                               pOutputDevice->GetFont().GetLanguageTag(), nullptr);
    pLayout = pOutputDevice->GetGraphics()->GetTextLayout(0);
    CPPUNIT_ASSERT(pLayout->LayoutText(aLayoutArgs, aGlyphs.Impl(0)));

    // Without the accompanying fix in place, this test would have failed with 'assertion failed'.
    // The kashida justification flag was lost when going via the glyph cache.
    CPPUNIT_ASSERT(aLayoutArgs.mnFlags & SalLayoutFlags::KashidaJustification);
#endif
}

void VclComplexTextTest::testTdf95650()
{
    static constexpr OUStringLiteral aTxt =
        u"\u0131\u0302\u0504\u4E44\u3031\u3030\u3531\u2D30"
        "\u3037\u0706\u0908\u0B0A\u0D0C\u0F0E\u072E\u100A"
        "\u0D11\u1312\u0105\u020A\u0512\u1403\u030C\u1528"
        "\u2931\u632E\u7074\u0D20\u0E0A\u100A\uF00D\u0D20"
        "\u030A\u0C0B\u20E0\u0A0D";
    ScopedVclPtrInstance<WorkWindow> pWin(static_cast<vcl::Window *>(nullptr));
    CPPUNIT_ASSERT(pWin);

    OutputDevice *pOutDev = pWin.get();
    // Check that the following executes without failing assertion
    pOutDev->ImplLayout(aTxt, 9, 1, Point(), 0, nullptr, SalLayoutFlags::BiDiRtl);
}

static void testCachedGlyphs( const OUString& aText, const OUString& aFontName = OUString())
{
    const std::string message = OUString("Font: " + aFontName + ", text: '" + aText + "'").toUtf8().getStr();
    ScopedVclPtrInstance<VirtualDevice> pOutputDevice;
    if(!aFontName.isEmpty())
    {
        vcl::Font aFont( aFontName, Size(0, 12));
        pOutputDevice->SetFont( aFont );
    }
    // Get the glyphs for the text.
    std::unique_ptr<SalLayout> pLayout1 = pOutputDevice->ImplLayout(
        aText, 0, aText.getLength(), Point(0, 0), 0, nullptr, SalLayoutFlags::GlyphItemsOnly);
    SalLayoutGlyphs aGlyphs1 = pLayout1->GetGlyphs();
    CPPUNIT_ASSERT_MESSAGE(message, aGlyphs1.IsValid());
    CPPUNIT_ASSERT_MESSAGE(message, aGlyphs1.Impl(0) != nullptr);
    // Reuse the cached glyphs to get glyphs again.
    std::unique_ptr<SalLayout> pLayout2 = pOutputDevice->ImplLayout(
        aText, 0, aText.getLength(), Point(0, 0), 0, nullptr, SalLayoutFlags::GlyphItemsOnly, nullptr, &aGlyphs1);
    SalLayoutGlyphs aGlyphs2 = pLayout2->GetGlyphs();
    CPPUNIT_ASSERT_MESSAGE(message, aGlyphs2.IsValid());
    // And check it's the same.
    for( int level = 0; level < MAX_FALLBACK; ++level )
    {
        const std::string messageLevel = OString(message.c_str()
            + OString::Concat(", level: ") + OString::number(level)).getStr();
        if( aGlyphs1.Impl(level) == nullptr)
        {
            CPPUNIT_ASSERT_MESSAGE(messageLevel, aGlyphs2.Impl(level) == nullptr);
            continue;
        }
        const SalLayoutGlyphsImpl* g1 = aGlyphs1.Impl(level);
        const SalLayoutGlyphsImpl* g2 = aGlyphs2.Impl(level);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(messageLevel, g1->GetFont().get(), g2->GetFont().get());
        CPPUNIT_ASSERT_EQUAL_MESSAGE(messageLevel, g1->size(), g2->size());
        for( size_t i = 0; i < g1->size(); ++i )
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE(messageLevel, (*g1)[i].glyphId(), (*g2)[i].glyphId());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(messageLevel, (*g1)[i].IsRTLGlyph(), (*g2)[i].IsRTLGlyph());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(messageLevel, (*g1)[i].IsVertical(), (*g2)[i].IsVertical());
        }
    }
}

// Check that caching using SalLayoutGlyphs gives same results as without caching.
// This should preferably use fonts that come with LO.
void VclComplexTextTest::testCaching()
{
    // Just something basic, no font fallback.
    testCachedGlyphs( "test", "Dejavu Sans" );
    // This font does not have latin characters, will need fallback.
    testCachedGlyphs( "test", "KacstBook" );
}

CPPUNIT_TEST_SUITE_REGISTRATION(VclComplexTextTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
