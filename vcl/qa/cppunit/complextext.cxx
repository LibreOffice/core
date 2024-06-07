/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_features.h>
#include <config_fonts.h>

#include <ostream>
#include <vector>
#include <tools/long.hxx>
#include <vcl/glyphitemcache.hxx>

#if HAVE_MORE_FONTS
// must be declared before inclusion of test/bootstrapfixture.hxx
static std::ostream& operator<<(std::ostream& rStream, const std::vector<sal_Int32>& rVec);
#endif
#include <test/bootstrapfixture.hxx>

#include <vcl/wrkwin.hxx>
#include <vcl/virdev.hxx>
// workaround MSVC2015 issue with std::unique_ptr
#include <sallayout.hxx>
#include <salgdi.hxx>


#include <ImplLayoutArgs.hxx>

#if HAVE_MORE_FONTS
static std::ostream& operator<<(std::ostream& rStream, const std::vector<sal_Int32>& rVec)
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
#if !defined _WIN32
    OUString maDataUrl = u"/vcl/qa/cppunit/data/"_ustr;

    OUString getFullUrl(std::u16string_view sFileName)
    {
        return m_directories.getURLFromSrc(maDataUrl) + sFileName;
    }

protected:
    bool addFont(OutputDevice* pOutDev, std::u16string_view sFileName,
                 std::u16string_view sFamilyName)
    {
        OutputDevice::ImplClearAllFontData(true);
        bool bAdded = pOutDev->AddTempDevFont(getFullUrl(sFileName), OUString(sFamilyName));
        OutputDevice::ImplRefreshAllFontData(true);
        return bAdded;
    }
#endif

public:
    VclComplexTextTest()
        : BootstrapFixture(true, false)
    {
    }
};

CPPUNIT_TEST_FIXTURE(VclComplexTextTest, testArabic)
{
#if HAVE_MORE_FONTS
    OUString aOneTwoThree(u"واحِدْ إثٍنين ثلاثةٌ"_ustr);

    vcl::Font aFont(u"DejaVu Sans"_ustr, u"Book"_ustr, Size(0, 2048));

    ScopedVclPtrInstance<VirtualDevice> pOutDev;
    pOutDev->SetFont( aFont );

    // absolute character widths AKA text array.
    tools::Long nRefTextWidth = 12595;
    std::vector<sal_Int32> aRefCharWidths = { 989, 1558, 2824, 2824, 3899,
        3899, 4550, 5119, 5689, 5689, 6307, 6925, 8484, 9135, 9705, 10927,
        10927, 11497, 12595, 12595 };
    KernArray aCharWidths;
    tools::Long nTextWidth
        = basegfx::fround<tools::Long>(pOutDev->GetTextArray(aOneTwoThree, &aCharWidths).nWidth);

    CPPUNIT_ASSERT_EQUAL(aRefCharWidths, aCharWidths.get_subunit_array());
    // this sporadically returns 75 or 74 on some of the windows tinderboxes eg. tb73
    CPPUNIT_ASSERT_EQUAL(nRefTextWidth, nTextWidth);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(nTextWidth), aCharWidths.back());

    // text advance width and line height
    CPPUNIT_ASSERT_EQUAL(nRefTextWidth, pOutDev->GetTextWidth(aOneTwoThree));
    CPPUNIT_ASSERT_EQUAL(tools::Long(2384), pOutDev->GetTextHeight());

    // exact bounding rectangle, not essentially the same as text width/height
    tools::Rectangle aBoundRect;
    pOutDev->GetTextBoundRect(aBoundRect, aOneTwoThree);
    CPPUNIT_ASSERT_EQUAL(tools::Long(145), aBoundRect.Left());
    CPPUNIT_ASSERT_EQUAL(tools::Long(212), aBoundRect.Top());
    CPPUNIT_ASSERT_EQUAL(tools::Long(12294), aBoundRect.GetWidth());
    CPPUNIT_ASSERT_EQUAL(tools::Long(2279), aBoundRect.getOpenHeight());

    // normal orientation
    tools::Rectangle aInput;
    tools::Rectangle aRect = pOutDev->GetTextRect( aInput, aOneTwoThree );

    // now rotate 270 degrees
    vcl::Font aRotated( aFont );
    aRotated.SetOrientation( 2700_deg10 );
    pOutDev->SetFont( aRotated );
    tools::Rectangle aRectRot = pOutDev->GetTextRect( aInput, aOneTwoThree );

    // Check that we did do the rotation...
    CPPUNIT_ASSERT_EQUAL( aRectRot.GetWidth(), aRect.GetHeight() );
    CPPUNIT_ASSERT_EQUAL( aRectRot.GetHeight(), aRect.GetWidth() );
#endif
}

CPPUNIT_TEST_FIXTURE(VclComplexTextTest, testTdf95650)
{
    static constexpr OUStringLiteral aTxt =
        u"\u0131\u0302\u0504\u4E44\u3031\u3030\u3531\u2D30"
        "\u3037\u0706\u0908\u0B0A\u0D0C\u0F0E\u072E\u100A"
        "\u0D11\u1312\u0105\u020A\u0512\u1403\u030C\u1528"
        "\u2931\u632E\u7074\u0D20\u0E0A\u100A\uF00D\u0D20"
        "\u030A\u0C0B\u20E0\u0A0D";
    ScopedVclPtrInstance<VirtualDevice> pOutDev;
    // Check that the following executes without failing assertion
    pOutDev->ImplLayout(aTxt, 9, 1, Point(), 0, {}, {}, SalLayoutFlags::BiDiRtl);
}

static void checkCompareGlyphs( const SalLayoutGlyphs& aGlyphs1, const SalLayoutGlyphs& aGlyphs2,
    const std::string& message )
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE(message, aGlyphs1.IsValid(), aGlyphs2.IsValid());
    // And check it's the same.
    for( int level = 0; level < MAX_FALLBACK; ++level )
    {
        const std::string messageLevel( Concat2View(OString::Concat(std::string_view(message))
            + ", level: " + OString::number(level)) );
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
            const bool equal = (*g1)[i].isLayoutEquivalent((*g2)[i]);
            CPPUNIT_ASSERT_MESSAGE(messageLevel, equal);
        }
    }
}

static void testCachedGlyphs( const OUString& aText, const OUString& aFontName )
{
    const std::string message( OUString("Font: " + aFontName + ", text: '" + aText + "'").toUtf8() );
    ScopedVclPtrInstance<VirtualDevice> pOutputDevice;
    vcl::Font aFont( aFontName, Size(0, 12));
    pOutputDevice->SetFont( aFont );
    SalLayoutGlyphsCache::self()->clear();
    // Get the glyphs for the text.
    std::unique_ptr<SalLayout> pLayout1 = pOutputDevice->ImplLayout(
        aText, 0, aText.getLength(), Point(0, 0), 0, {}, {}, SalLayoutFlags::GlyphItemsOnly);
    SalLayoutGlyphs aGlyphs1 = pLayout1->GetGlyphs();
    // Reuse the cached glyphs to get glyphs again.
    std::unique_ptr<SalLayout> pLayout2 = pOutputDevice->ImplLayout(
        aText, 0, aText.getLength(), Point(0, 0), 0, {}, {}, SalLayoutFlags::GlyphItemsOnly, nullptr, &aGlyphs1);
    SalLayoutGlyphs aGlyphs2 = pLayout2->GetGlyphs();
    checkCompareGlyphs(aGlyphs1, aGlyphs2, message + " (reuse)");
    // Get cached glyphs from SalLayoutGlyphsCache.
    const SalLayoutGlyphs* aGlyphs3 = SalLayoutGlyphsCache::self()->GetLayoutGlyphs(
        pOutputDevice, aText, 0, aText.getLength(), 0);
    CPPUNIT_ASSERT_MESSAGE(message, aGlyphs3 != nullptr);
    checkCompareGlyphs(aGlyphs1, *aGlyphs3, message + " (cache)");
}

// Check that caching using SalLayoutGlyphs gives same results as without caching.
// This should preferably use fonts that come with LO.
CPPUNIT_TEST_FIXTURE(VclComplexTextTest, testCaching)
{
    // Just something basic, no font fallback.
    testCachedGlyphs( u"test"_ustr, u"Dejavu Sans"_ustr );
    // This font does not have latin characters, will need fallback.
    testCachedGlyphs( u"test"_ustr, u"Noto Kufi Arabic"_ustr );
    // see tdf#103492
    testCachedGlyphs( u"يوسف My name is"_ustr, u"Liberation Sans"_ustr);
}

static void testCachedGlyphsSubstring( const OUString& aText, const OUString& aFontName, bool rtl )
{
    const std::string prefix( OUString("Font: " + aFontName + ", text: '" + aText + "'").toUtf8() );
    ScopedVclPtrInstance<VirtualDevice> pOutputDevice;
    // BiDiStrong is needed, otherwise SalLayoutGlyphsImpl::cloneCharRange() will not do anything.
    vcl::text::ComplexTextLayoutFlags layoutFlags = vcl::text::ComplexTextLayoutFlags::BiDiStrong;
    if(rtl)
        layoutFlags |= vcl::text::ComplexTextLayoutFlags::BiDiRtl;
    pOutputDevice->SetLayoutMode( layoutFlags );
    vcl::Font aFont( aFontName, Size(0, 12));
    pOutputDevice->SetFont( aFont );
    SalLayoutGlyphsCache::self()->clear();
    std::shared_ptr<const vcl::text::TextLayoutCache> layoutCache = OutputDevice::CreateTextLayoutCache(aText);
    // Get the glyphs for the entire text once, to ensure the cache can built subsets from it.
    pOutputDevice->ImplLayout( aText, 0, aText.getLength(), Point(0, 0), 0, {}, {}, SalLayoutFlags::GlyphItemsOnly,
        layoutCache.get());
    // Now check for all subsets. Some of them possibly do not make sense in practice, but the code
    // should cope with them.
    for( sal_Int32 len = 1; len <= aText.getLength(); ++len )
        for( sal_Int32 pos = 0; pos < aText.getLength() - len; ++pos )
        {
            std::string message = prefix + " (" + std::to_string(pos) + "/" + std::to_string(len) + ")";
            std::unique_ptr<SalLayout> pLayout1 = pOutputDevice->ImplLayout(
                aText, pos, len, Point(0, 0), 0, {}, {}, SalLayoutFlags::GlyphItemsOnly, layoutCache.get());
            SalLayoutGlyphs aGlyphs1 = pLayout1->GetGlyphs();
            const SalLayoutGlyphs* aGlyphs2 = SalLayoutGlyphsCache::self()->GetLayoutGlyphs(
                pOutputDevice, aText, pos, len, 0, layoutCache.get());
            CPPUNIT_ASSERT_MESSAGE(message, aGlyphs2 != nullptr);
            checkCompareGlyphs(aGlyphs1, *aGlyphs2, message);
        }

}

// Check that SalLayoutGlyphsCache works properly when it builds a subset
// of glyphs using SalLayoutGlyphsImpl::cloneCharRange().
// This should preferably use fonts that come with LO.
CPPUNIT_TEST_FIXTURE(VclComplexTextTest, testCachingSubstring)
{
    // Just something basic.
    testCachedGlyphsSubstring( u"test"_ustr, u"Dejavu Sans"_ustr, false );
    // And complex arabic text, taken from tdf104649.docx .
    OUString text(u"فصل (پاره 2): درخواست حاجت از ديگران و برآوردن حاجت ديگران 90"_ustr);
    testCachedGlyphsSubstring( text, u"Dejavu Sans"_ustr, true );
    // The text is RTL, but Writer will sometimes try to lay it out as LTR, for whatever reason
    // (tdf#149264)./ So make sure that gets handled properly too (SalLayoutGlyphsCache should
    // not use glyph subsets in that case).
    testCachedGlyphsSubstring( text, u"Dejavu Sans"_ustr, false );
}

CPPUNIT_TEST_FIXTURE(VclComplexTextTest, testCaret)
{
#if HAVE_MORE_FONTS
    // Test caret placement in fonts *without* ligature carets in GDEF table.

    // Set font size to its UPEM to decrease rounding issues
    vcl::Font aFont(u"DejaVu Sans"_ustr, u"Book"_ustr, Size(0, 2048));

    ScopedVclPtrInstance<VirtualDevice> pOutDev;
    pOutDev->SetFont( aFont );

    OUString aText;
    KernArray aCharWidths;
    std::vector<sal_Int32> aRefCharWidths;
    tools::Long nTextWidth, nTextWidth2, nRefTextWidth;

    // A. RTL text
    aText = u"لا بلا"_ustr;

    // 1) Regular DX array, the ligature width is given to the first components
    // and the next ones are all zero width.
    nRefTextWidth = 3611;
    aRefCharWidths = { 1168, 1168, 1819, 2389, 3611, 3611 };
    nTextWidth = basegfx::fround<tools::Long>(
        pOutDev->GetTextArray(aText, &aCharWidths, 0, -1, /*bCaret*/ false).nWidth);
    CPPUNIT_ASSERT_EQUAL(aRefCharWidths, aCharWidths.get_subunit_array());
    CPPUNIT_ASSERT_EQUAL(nRefTextWidth, nTextWidth);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(nTextWidth), aCharWidths.back());

    // 2) Caret placement DX array, ligature width is distributed over its
    // components.
    aRefCharWidths = { 584, 1168, 1819, 2389, 3000, 3611 };
    nTextWidth = basegfx::fround<tools::Long>(
        pOutDev->GetTextArray(aText, &aCharWidths, 0, -1, /*bCaret*/ true).nWidth);
    CPPUNIT_ASSERT_EQUAL(aRefCharWidths, aCharWidths.get_subunit_array());
    CPPUNIT_ASSERT_EQUAL(nRefTextWidth, nTextWidth);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(nTextWidth), aCharWidths.back());

    // 3) caret placement with combining marks, they should not add to ligature
    // component count.
    aText = u"لَاَ بلَاَ"_ustr;
    aRefCharWidths = { 584, 584, 1168, 1168, 1819, 2389, 3000, 3000, 3611, 3611 };
    nTextWidth2 = basegfx::fround<tools::Long>(
        pOutDev->GetTextArray(aText, &aCharWidths, 0, -1, /*bCaret*/ true).nWidth);
    CPPUNIT_ASSERT_EQUAL(aCharWidths[0], aCharWidths[1]);
    CPPUNIT_ASSERT_EQUAL(aCharWidths[2], aCharWidths[3]);
    CPPUNIT_ASSERT_EQUAL(aCharWidths[6], aCharWidths[7]);
    CPPUNIT_ASSERT_EQUAL(aCharWidths[8], aCharWidths[9]);
    CPPUNIT_ASSERT_EQUAL(aRefCharWidths, aCharWidths.get_subunit_array());
    CPPUNIT_ASSERT_EQUAL(nRefTextWidth, nTextWidth2);
    CPPUNIT_ASSERT_EQUAL(nTextWidth, nTextWidth2);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(nTextWidth), aCharWidths.back());

    // B. LTR text
    aText = u"fi fl ffi ffl"_ustr;

    // 1) Regular DX array, the ligature width is given to the first components
    // and the next ones are all zero width.
    nRefTextWidth = 8493;
    aRefCharWidths = { 1290, 1290, 1941, 3231, 3231, 3882, 5862, 5862, 5862, 6513, 8493, 8493, 8493 };
    nTextWidth = basegfx::fround<tools::Long>(
        pOutDev->GetTextArray(aText, &aCharWidths, 0, -1, /*bCaret*/ false).nWidth);
    CPPUNIT_ASSERT_EQUAL(aRefCharWidths, aCharWidths.get_subunit_array());
    CPPUNIT_ASSERT_EQUAL(nRefTextWidth, nTextWidth);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(nTextWidth), aCharWidths.back());

    // 2) Caret placement DX array, ligature width is distributed over its
    // components.
    aRefCharWidths = { 645, 1290, 1941, 2586, 3231, 3882, 4542, 5202, 5862, 6513, 7173, 7833, 8493 };
    nTextWidth = basegfx::fround<tools::Long>(
        pOutDev->GetTextArray(aText, &aCharWidths, 0, -1, /*bCaret*/ true).nWidth);
    CPPUNIT_ASSERT_EQUAL(aRefCharWidths, aCharWidths.get_subunit_array());
    CPPUNIT_ASSERT_EQUAL(nRefTextWidth, nTextWidth);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(nTextWidth), aCharWidths.back());
#endif
}

CPPUNIT_TEST_FIXTURE(VclComplexTextTest, testGdefCaret)
{
#if HAVE_MORE_FONTS
    // Test caret placement in fonts *with* ligature carets in GDEF table.

    ScopedVclPtrInstance<VirtualDevice> pOutDev;

    vcl::Font aFont;
    OUString aText;
    KernArray aCharWidths;
    std::vector<sal_Int32> aRefCharWidths;
    tools::Long nTextWidth, nTextWidth2, nRefTextWidth;

    // A. RTL text
    // Set font size to its UPEM to decrease rounding issues
    aFont = vcl::Font(u"Noto Sans Arabic"_ustr, u"Regular"_ustr, Size(0, 1000));
    pOutDev->SetFont(aFont);

    aText = u"لا بلا"_ustr;

    // 1) Regular DX array, the ligature width is given to the first components
    // and the next ones are all zero width.
    nRefTextWidth = 1710;
    aRefCharWidths= { 582, 582, 842, 1111, 1710, 1710 };
    nTextWidth = basegfx::fround<tools::Long>(
        pOutDev->GetTextArray(aText, &aCharWidths, 0, -1, /*bCaret*/ false).nWidth);
    CPPUNIT_ASSERT_EQUAL(aRefCharWidths, aCharWidths.get_subunit_array());
    CPPUNIT_ASSERT_EQUAL(nRefTextWidth, nTextWidth);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(nTextWidth), aCharWidths.back());

    // 2) Caret placement DX array, ligature width is distributed over its
    // components.
    aRefCharWidths = { 291, 582, 842, 1111, 1410, 1710 };
    nTextWidth = basegfx::fround<tools::Long>(
        pOutDev->GetTextArray(aText, &aCharWidths, 0, -1, /*bCaret*/ true).nWidth);
    CPPUNIT_ASSERT_EQUAL(aRefCharWidths, aCharWidths.get_subunit_array());
    CPPUNIT_ASSERT_EQUAL(nRefTextWidth, nTextWidth);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(nTextWidth), aCharWidths.back());

    // 3) caret placement with combining marks, they should not add to ligature
    // component count.
    aText = u"لَاَ بلَاَ"_ustr;
    aRefCharWidths = { 291, 291, 582, 582, 842, 1111, 1410, 1410, 1710, 1710 };
    nTextWidth2 = basegfx::fround<tools::Long>(
        pOutDev->GetTextArray(aText, &aCharWidths, 0, -1, /*bCaret*/ true).nWidth);
    CPPUNIT_ASSERT_EQUAL(aCharWidths[0], aCharWidths[1]);
    CPPUNIT_ASSERT_EQUAL(aCharWidths[2], aCharWidths[3]);
    CPPUNIT_ASSERT_EQUAL(aCharWidths[6], aCharWidths[7]);
    CPPUNIT_ASSERT_EQUAL(aCharWidths[8], aCharWidths[9]);
    CPPUNIT_ASSERT_EQUAL(aRefCharWidths, aCharWidths.get_subunit_array());
    CPPUNIT_ASSERT_EQUAL(nRefTextWidth, nTextWidth2);
    CPPUNIT_ASSERT_EQUAL(nTextWidth, nTextWidth2);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(nTextWidth), aCharWidths.back());

    // B. LTR text
    // Set font size to its UPEM to decrease rounding issues
    aFont = vcl::Font(u"Amiri"_ustr, u"Regular"_ustr, Size(0, 1000));
    pOutDev->SetFont(aFont);

    aText = u"fi ffi fl ffl fb ffb"_ustr;

    // 1) Regular DX array, the ligature width is given to the first components
    // and the next ones are all zero width.
    nRefTextWidth = 5996;
    aRefCharWidths = { 519, 519, 811, 1606, 1606, 1606, 1898, 2439, 2439, 2731,
                       3544, 3544, 3544, 3836, 4634, 4634, 4926, 5996, 5996, 5996 };
    nTextWidth = basegfx::fround<tools::Long>(
        pOutDev->GetTextArray(aText, &aCharWidths, 0, -1, /*bCaret*/ false).nWidth);
    CPPUNIT_ASSERT_EQUAL(aRefCharWidths, aCharWidths.get_subunit_array());
    CPPUNIT_ASSERT_EQUAL(nRefTextWidth, nTextWidth);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(nTextWidth), aCharWidths.back());

    // 2) Caret placement DX array, ligature width is distributed over its
    // components.
    aRefCharWidths = { 269, 519, 811, 1080, 1348, 1606, 1898, 2171, 2439, 2731,
                       3004, 3278, 3544, 3836, 4138, 4634, 4926, 5199, 5494, 5996 };
    nTextWidth = basegfx::fround<tools::Long>(
        pOutDev->GetTextArray(aText, &aCharWidths, 0, -1, /*bCaret*/ true).nWidth);
    CPPUNIT_ASSERT_EQUAL(aRefCharWidths, aCharWidths.get_subunit_array());
    CPPUNIT_ASSERT_EQUAL(nRefTextWidth, nTextWidth);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(nTextWidth), aCharWidths.back());
#endif
}

CPPUNIT_TEST_FIXTURE(VclComplexTextTest, testTdf152048)
{
#if HAVE_MORE_FONTS
    OUString aText(u"می‌شود"_ustr);

    vcl::Font aFont(u"Noto Naskh Arabic"_ustr, u"Regular"_ustr, Size(0, 2048));

    ScopedVclPtrInstance<VirtualDevice> pOutDev;
    pOutDev->SetFont(aFont);

    // get an compare the default text array
    std::vector<sal_Int32> aRefCharWidths{ 934, 2341, 2341, 3689, 4647, 5495 };
    tools::Long nRefTextWidth(5495);

    KernArray aCharWidths;
    tools::Long nTextWidth
        = basegfx::fround<tools::Long>(pOutDev->GetTextArray(aText, &aCharWidths).nWidth);

    CPPUNIT_ASSERT_EQUAL(aRefCharWidths, aCharWidths.get_subunit_array());
    CPPUNIT_ASSERT_EQUAL(nRefTextWidth, nTextWidth);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(nTextWidth), aCharWidths.back());

    // Simulate Kashida insertion using Kashida array and extending text array
    // to have room for Kashida.
    std::vector<sal_Bool> aKashidaArray{ false, false, false, true, false, false };
    auto nKashida = 4000;

    aCharWidths.set(3, aCharWidths[3] + nKashida);
    aCharWidths.set(4, aCharWidths[4] + nKashida);
    aCharWidths.set(5, aCharWidths[5] + nKashida);
    auto pLayout = pOutDev->ImplLayout(aText, 0, -1, Point(0, 0), 0, aCharWidths, aKashidaArray);

    // Without the fix this fails with:
    // - Expected: 393
    // - Actual  : 511
    CPPUNIT_ASSERT_EQUAL(double(nRefTextWidth + nKashida), pLayout->GetTextWidth());
#endif
}

CPPUNIT_TEST_FIXTURE(VclComplexTextTest, testTdf152048_2)
{
#if HAVE_MORE_FONTS
    vcl::Font aFont(u"Noto Naskh Arabic"_ustr, u"Regular"_ustr, Size(0, 72));

    ScopedVclPtrInstance<VirtualDevice> pOutDev;
    pOutDev->SetFont(aFont);

    // get an compare the default text array
    KernArray aCharWidths;
    auto nTextWidth
        = basegfx::fround<tools::Long>(pOutDev->GetTextArray(u"ع a ع"_ustr, &aCharWidths).nWidth);

    // Text width should always be equal to the width of the last glyph in the
    // kern array.
    // Without the fix this fails with:
    // - Expected: 158
    // - Actual  : 118
    CPPUNIT_ASSERT_EQUAL(aCharWidths.back(), sal_Int32(nTextWidth));
#endif
}

CPPUNIT_TEST_FIXTURE(VclComplexTextTest, testTdf153440)
{
#if HAVE_MORE_FONTS
    vcl::Font aFont(u"Noto Naskh Arabic"_ustr, u"Regular"_ustr, Size(0, 72));

    ScopedVclPtrInstance<VirtualDevice> pOutDev;
    pOutDev->SetFont(aFont);

#if !defined _WIN32 // TODO: Fails on jenkins but passes locally
    // Add an emoji font so that we are sure a font will be found for the
    // emoji. The font is subset and supports only 🌿.
    bool bAdded = addFont(pOutDev, u"tdf153440.ttf", u"Noto Emoji");
    CPPUNIT_ASSERT_EQUAL(true, bAdded);
#endif

    for (auto& aString : { u"ع 🌿 ع", u"a 🌿 a" })
    {
        OUString aText(aString);
        bool bRTL = aText.startsWith(u"ع");

        auto pLayout = pOutDev->ImplLayout(aText, 0, -1, Point(0, 0), 0, {}, {});

        int nStart = 0;
        basegfx::B2DPoint aPos;
        const GlyphItem* pGlyphItem;
        while (pLayout->GetNextGlyph(&pGlyphItem, aPos, nStart))
        {
            // Assert glyph ID is not 0, if it is 0 then font fallback didn’t
            // happen.
            CPPUNIT_ASSERT(pGlyphItem->glyphId());

            // Assert that we are indeed doing RTL layout for RTL text since
            // the bug does not happen for LTR text.
            CPPUNIT_ASSERT_EQUAL(bRTL, pGlyphItem->IsRTLGlyph());
        }
    }
#endif
}

CPPUNIT_TEST_FIXTURE(VclComplexTextTest, testMixedCJKLatinScript_glyph_advancements)
{
#if HAVE_MORE_FONTS
#if !defined _WIN32
    OUString aTestScript(u"根据10.1(37BA) Eng"_ustr);

    ScopedVclPtrInstance<VirtualDevice> pOutDev;
    // note you can only run this once and it was designed for tdf#107718
    bool bAdded = addFont(pOutDev, u"tdf107718.otf", u"Source Han Sans");
    CPPUNIT_ASSERT_EQUAL(true, bAdded);

    vcl::Font aFont(u"Source Han Sans"_ustr, u"Regular"_ustr, Size(0, 72));
    pOutDev->SetFont( aFont );

    vcl::Font aFallbackFont(u"DejaVu Sans"_ustr, u"Book"_ustr, Size(0, 72));
    pOutDev->ForceFallbackFont(aFallbackFont);

    // absolute character widths AKA text array.
    tools::Long nRefTextWidth = 704;
    std::vector<sal_Int32> aRefCharWidths = { 72, 144, 190, 236, 259, 305, 333, 379, 425, 474, 523, 551, 567, 612, 658, 704 };
    KernArray aCharWidths;
    tools::Long nTextWidth
        = basegfx::fround<tools::Long>(pOutDev->GetTextArray(aTestScript, &aCharWidths).nWidth);

    CPPUNIT_ASSERT_EQUAL(aRefCharWidths, aCharWidths.get_subunit_array());
    CPPUNIT_ASSERT_EQUAL(nRefTextWidth, nTextWidth);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(nTextWidth), aCharWidths.back());

    // text advance width and line height
    CPPUNIT_ASSERT_EQUAL(nRefTextWidth, pOutDev->GetTextWidth(aTestScript));
    CPPUNIT_ASSERT_EQUAL(tools::Long(105), pOutDev->GetTextHeight());
#endif
#endif
}

CPPUNIT_TEST_FIXTURE(VclComplexTextTest, testTdf107718)
{
#if HAVE_MORE_FONTS
#if !defined _WIN32 // TODO: Fails on jenkins but passes locally
    vcl::Font aFont(u"Source Han Sans"_ustr, u"Regular"_ustr, Size(0, 72));

    ScopedVclPtrInstance<VirtualDevice> pOutDev;

    OUString aText(u"\u4E16\u1109\u1168\u11BC\u302E"_ustr);
    for (bool bVertical : { false, true })
    {
        aFont.SetVertical(bVertical);
        pOutDev->SetFont(aFont);

        auto pLayout = pOutDev->ImplLayout(aText, 0, -1, Point(0, 0), 0, {}, {});

        int nStart = 0;
        basegfx::B2DPoint aPos;
        const GlyphItem* pGlyphItem;
        while (pLayout->GetNextGlyph(&pGlyphItem, aPos, nStart))
        {
            // Check that we found a font for all characters, a zero glyph ID
            // means no font was found so the rest of the test would be
            // meaningless.
            CPPUNIT_ASSERT(pGlyphItem->glyphId());

            // Assert that we are indeed doing vertical layout for vertical
            // font since the bug does not happen for horizontal text.
            CPPUNIT_ASSERT_EQUAL(bVertical, pGlyphItem->IsVertical());

            // For the second glyph, assert that it is a composition of characters 1 to 4
            // Without the fix this fails with:
            // - Expected: 4
            // - Actual  : 1
            if (nStart == 2)
            {
                CPPUNIT_ASSERT_EQUAL(1, pGlyphItem->charPos());
                CPPUNIT_ASSERT_EQUAL(4, pGlyphItem->charCount());
            }
        }

        // Assert there are only three glyphs
        // Without the fix this fails with:
        // - Expected: 3
        // - Actual  : 5
        CPPUNIT_ASSERT_EQUAL(3, nStart);
    }
#endif
#endif
}

CPPUNIT_TEST_FIXTURE(VclComplexTextTest, testTdf107612)
{
#if HAVE_MORE_FONTS
    vcl::Font aFont(u"DejaVu Sans"_ustr, u"Book"_ustr, Size(0, 72));

    ScopedVclPtrInstance<VirtualDevice> pOutDev;
    pOutDev->SetFont(aFont);

    auto pLayout = pOutDev->ImplLayout(u"a\u202F\u1823"_ustr, 0, -1, Point(0, 0), 0, {}, {});

    // If font fallback happened, then the returned layout must be a
    // MultiSalLayout instance.
    auto pMultiLayout = dynamic_cast<MultiSalLayout*>(pLayout.get());
    CPPUNIT_ASSERT(pMultiLayout);

    auto pFallbackRuns = pMultiLayout->GetFallbackRuns();
    CPPUNIT_ASSERT(!pFallbackRuns->IsEmpty());

    bool bRTL;
    int nCharPos = -1;
    std::vector<sal_Int32> aFallbacks;
    while (pFallbackRuns->GetNextPos(&nCharPos, &bRTL))
        aFallbacks.push_back(nCharPos);

    // Assert that U+202F is included in the fallback run.
    // Without the fix this fails with:
    // - Expected: { 1, 2 }
    // - Actual  : { 2 }
    std::vector<sal_Int32> aExpctedFallbacks = { 1, 2 };
    CPPUNIT_ASSERT_EQUAL(aExpctedFallbacks, aExpctedFallbacks);
#endif
}

CPPUNIT_TEST_FIXTURE(VclComplexTextTest, testPartialKoreanJamoComposition)
{
    OUString aStr = u"은"_ustr;
    vcl::Font aFont("DejaVu Sans", "Book", Size(0, 2048));

    ScopedVclPtrInstance<VirtualDevice> pOutDev;
    pOutDev->SetFont(aFont);

    // Absolute character widths for the complete array.
    KernArray aCompleteWidths;
    auto nCompleteWidth = pOutDev->GetTextArray(aStr, &aCompleteWidths).nWidth;

    CPPUNIT_ASSERT_EQUAL(size_t{ 3 }, aCompleteWidths.size());

    // Accumulate partial widths
    double nPartialWidth = 0.0;

    sal_Int32 nPrevWidth = 0;
    for (sal_Int32 i = 0; i < 3; ++i)
    {
        KernArray aFragmentWidths;
        auto nFragmentWidth
            = pOutDev
                  ->GetPartialTextArray(aStr, &aFragmentWidths, /*nIndex*/ 0, /*nLen*/ 3,
                                        /*nPartIndex*/ i, /*nPartLen*/ 1)
                  .nWidth;
        nPartialWidth += nFragmentWidth;

        CPPUNIT_ASSERT_EQUAL(size_t{ 1 }, aFragmentWidths.size());
        CPPUNIT_ASSERT_EQUAL(aCompleteWidths[i] - nPrevWidth, aFragmentWidths[0]);
        nPrevWidth = aCompleteWidths[i];
    }

    CPPUNIT_ASSERT_DOUBLES_EQUAL(nCompleteWidth, nPartialWidth, /*delta*/ 0.01);
}

CPPUNIT_TEST_FIXTURE(VclComplexTextTest, testPartialArabicComposition)
{
    OUString aStr = u"سُكُونْ"_ustr;
    vcl::Font aFont("DejaVu Sans", "Book", Size(0, 2048));

    ScopedVclPtrInstance<VirtualDevice> pOutDev;
    pOutDev->SetFont(aFont);

    // Absolute character widths for the complete array.
    KernArray aCompleteWidths;
    auto nCompleteWidth = pOutDev->GetTextArray(aStr, &aCompleteWidths).nWidth;

    CPPUNIT_ASSERT_EQUAL(size_t{ 7 }, aCompleteWidths.size());

    // Accumulate partial widths
    double nPartialWidth = 0.0;

    sal_Int32 nPrevWidth = 0;
    for (sal_Int32 i = 0; i < 7; ++i)
    {
        KernArray aFragmentWidths;
        auto nFragmentWidth
            = pOutDev
                  ->GetPartialTextArray(aStr, &aFragmentWidths, /*nIndex*/ 0, /*nLen*/ 7,
                                        /*nPartIndex*/ i, /*nPartLen*/ 1)
                  .nWidth;
        nPartialWidth += nFragmentWidth;

        CPPUNIT_ASSERT_EQUAL(size_t{ 1 }, aFragmentWidths.size());
        CPPUNIT_ASSERT_EQUAL(aCompleteWidths[i] - nPrevWidth, aFragmentWidths[0]);
        nPrevWidth = aCompleteWidths[i];
    }

    CPPUNIT_ASSERT_DOUBLES_EQUAL(nCompleteWidth, nPartialWidth, /*delta*/ 0.01);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
