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
public:
    VclComplexTextTest() : BootstrapFixture(true, false) {}

    /// Play with font measuring etc.
    void testArabic();
    void testTdf95650(); // Windows-only issue
    void testCaching();
    void testCachingSubstring();
    void testCaret();
    void testGdefCaret();

    CPPUNIT_TEST_SUITE(VclComplexTextTest);
    CPPUNIT_TEST(testArabic);
    CPPUNIT_TEST(testTdf95650);
    CPPUNIT_TEST(testCaching);
    CPPUNIT_TEST(testCachingSubstring);
    CPPUNIT_TEST(testCaret);
    CPPUNIT_TEST(testGdefCaret);
    CPPUNIT_TEST_SUITE_END();
};

void VclComplexTextTest::testArabic()
{
#if HAVE_MORE_FONTS
    OUString aOneTwoThree(u"واحِدْ إثٍنين ثلاثةٌ");

    vcl::Font aFont("DejaVu Sans", "Book", Size(0, 12));

    ScopedVclPtrInstance<VirtualDevice> pOutDev;
    pOutDev->SetFont( aFont );

    // absolute character widths AKA text array.
    std::vector<sal_Int32> aRefCharWidths {6,  9,  16, 16, 22, 22, 26, 29, 32, 32,
                                      36, 40, 49, 53, 56, 63, 63, 66, 72, 72};
    KernArray aCharWidths;
    tools::Long nTextWidth = basegfx::fround<tools::Long>(pOutDev->GetTextArray(aOneTwoThree, &aCharWidths));

    CPPUNIT_ASSERT_EQUAL(aRefCharWidths, aCharWidths.get_subunit_array());
    // this sporadically returns 75 or 74 on some of the windows tinderboxes eg. tb73
    CPPUNIT_ASSERT_EQUAL(tools::Long(72), nTextWidth);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(nTextWidth), aCharWidths.back());

    // text advance width and line height
    CPPUNIT_ASSERT_EQUAL(tools::Long(72), pOutDev->GetTextWidth(aOneTwoThree));
    CPPUNIT_ASSERT_EQUAL(tools::Long(14), pOutDev->GetTextHeight());

    // exact bounding rectangle, not essentially the same as text width/height
    tools::Rectangle aBoundRect;
    pOutDev->GetTextBoundRect(aBoundRect, aOneTwoThree);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0, aBoundRect.Left(), 1); // This sometimes equals to 1
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1, aBoundRect.Top(), 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(71, aBoundRect.GetWidth(), 2); // This sometimes equals to 70
    CPPUNIT_ASSERT_DOUBLES_EQUAL(15, aBoundRect.getOpenHeight(), 1);

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

void VclComplexTextTest::testTdf95650()
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
            const bool equal = (*g1)[i].isLayoutEquivalent((*g2)[i]);
            CPPUNIT_ASSERT_MESSAGE(messageLevel, equal);
        }
    }
}

static void testCachedGlyphs( const OUString& aText, const OUString& aFontName )
{
    const std::string message = OUString("Font: " + aFontName + ", text: '" + aText + "'").toUtf8().getStr();
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
void VclComplexTextTest::testCaching()
{
    // Just something basic, no font fallback.
    testCachedGlyphs( "test", "Dejavu Sans" );
    // This font does not have latin characters, will need fallback.
    testCachedGlyphs( "test", "Noto Kufi Arabic" );
}

static void testCachedGlyphsSubstring( const OUString& aText, const OUString& aFontName, bool rtl )
{
    const std::string prefix = OUString("Font: " + aFontName + ", text: '" + aText + "'").toUtf8().getStr();
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
void VclComplexTextTest::testCachingSubstring()
{
    // Just something basic.
    testCachedGlyphsSubstring( "test", "Dejavu Sans", false );
    // And complex arabic text, taken from tdf104649.docx .
    OUString text(u"فصل (پاره 2): درخواست حاجت از ديگران و برآوردن حاجت ديگران 90");
    testCachedGlyphsSubstring( text, "Dejavu Sans", true );
    // The text is RTL, but Writer will sometimes try to lay it out as LTR, for whatever reason
    // (tdf#149264)./ So make sure that gets handled properly too (SalLayoutGlyphsCache should
    // not use glyph subsets in that case).
    testCachedGlyphsSubstring( text, "Dejavu Sans", false );
}

void VclComplexTextTest::testCaret()
{
#if HAVE_MORE_FONTS
    // Test caret placement in fonts *without* ligature carets in GDEF table.

    vcl::Font aFont("DejaVu Sans", "Book", Size(0, 200));

    ScopedVclPtrInstance<VirtualDevice> pOutDev;
    pOutDev->SetFont( aFont );

    OUString aText;
    KernArray aCharWidths;
    std::vector<sal_Int32> aRefCharWidths;
    tools::Long nTextWidth, nTextWidth2;

    // A. RTL text
    aText = u"لا بلا";

    // 1) Regular DX array, the ligature width is given to the first components
    // and the next ones are all zero width.
    aRefCharWidths = { 114, 114, 178, 234, 353, 353 };
    nTextWidth = pOutDev->GetTextArray(aText, &aCharWidths, 0, -1, /*bCaret*/false);
    CPPUNIT_ASSERT_EQUAL(aRefCharWidths, aCharWidths.get_subunit_array());
    CPPUNIT_ASSERT_EQUAL(tools::Long(353), nTextWidth);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(nTextWidth), aCharWidths.back());

    // 2) Caret placement DX array, ligature width is distributed over its
    // components.
    aRefCharWidths = { 57, 114, 178, 234, 293, 353 };
    nTextWidth = pOutDev->GetTextArray(aText, &aCharWidths, 0, -1, /*bCaret*/true);
    CPPUNIT_ASSERT_EQUAL(aRefCharWidths, aCharWidths.get_subunit_array());
    CPPUNIT_ASSERT_EQUAL(tools::Long(353), nTextWidth);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(nTextWidth), aCharWidths.back());

    // 3) caret placement with combining marks, they should not add to ligature
    // component count.
    aText = u"لَاَ بلَاَ";
    aRefCharWidths = { 57, 57, 114, 114, 178, 234, 293, 293, 353, 353 };
    nTextWidth2 = pOutDev->GetTextArray(aText, &aCharWidths, 0, -1, /*bCaret*/true);
    CPPUNIT_ASSERT_EQUAL(aCharWidths[0], aCharWidths[1]);
    CPPUNIT_ASSERT_EQUAL(aCharWidths[2], aCharWidths[3]);
    CPPUNIT_ASSERT_EQUAL(aCharWidths[6], aCharWidths[7]);
    CPPUNIT_ASSERT_EQUAL(aCharWidths[8], aCharWidths[9]);
    CPPUNIT_ASSERT_EQUAL(aRefCharWidths, aCharWidths.get_subunit_array());
    CPPUNIT_ASSERT_EQUAL(tools::Long(353), nTextWidth2);
    CPPUNIT_ASSERT_EQUAL(nTextWidth, nTextWidth2);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(nTextWidth), aCharWidths.back());

    // B. LTR text
    aText = u"fi fl ffi ffl";

    // 1) Regular DX array, the ligature width is given to the first components
    // and the next ones are all zero width.
    aRefCharWidths = { 126, 126, 190, 316, 316, 380, 573, 573, 573, 637, 830, 830, 830 };
    nTextWidth = pOutDev->GetTextArray(aText, &aCharWidths, 0, -1, /*bCaret*/false);
    CPPUNIT_ASSERT_EQUAL(aRefCharWidths, aCharWidths.get_subunit_array());
    CPPUNIT_ASSERT_EQUAL(tools::Long(830), nTextWidth);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(nTextWidth), aCharWidths.back());

    // 2) Caret placement DX array, ligature width is distributed over its
    // components.
    aRefCharWidths = { 63, 126, 190, 253, 316, 380, 444, 508, 573, 637, 701, 765, 830 };
    nTextWidth = pOutDev->GetTextArray(aText, &aCharWidths, 0, -1, /*bCaret*/true);
    CPPUNIT_ASSERT_EQUAL(aRefCharWidths, aCharWidths.get_subunit_array());
    CPPUNIT_ASSERT_EQUAL(tools::Long(830), nTextWidth);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(nTextWidth), aCharWidths.back());
#endif
}

void VclComplexTextTest::testGdefCaret()
{
#if HAVE_MORE_FONTS
    // Test caret placement in fonts *with* ligature carets in GDEF table.

    ScopedVclPtrInstance<VirtualDevice> pOutDev;

    vcl::Font aFont;
    OUString aText;
    KernArray aCharWidths;
    std::vector<sal_Int32> aRefCharWidths;
    tools::Long nTextWidth, nTextWidth2;

    // A. RTL text
    aFont = vcl::Font("Noto Naskh Arabic", "Regular", Size(0, 200));
    pOutDev->SetFont(aFont);

    aText = u"لا بلا";

    // 1) Regular DX array, the ligature width is given to the first components
    // and the next ones are all zero width.
    aRefCharWidths= { 104, 104, 148, 203, 325, 325 };
    nTextWidth = pOutDev->GetTextArray(aText, &aCharWidths, 0, -1, /*bCaret*/false);
    CPPUNIT_ASSERT_EQUAL(aRefCharWidths, aCharWidths.get_subunit_array());
    CPPUNIT_ASSERT_EQUAL(tools::Long(325), nTextWidth);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(nTextWidth), aCharWidths.back());

    // 2) Caret placement DX array, ligature width is distributed over its
    // components.
    aRefCharWidths = { 53, 104, 148, 203, 265, 325 };
    nTextWidth = pOutDev->GetTextArray(aText, &aCharWidths, 0, -1, /*bCaret*/true);
    CPPUNIT_ASSERT_EQUAL(aRefCharWidths, aCharWidths.get_subunit_array());
    CPPUNIT_ASSERT_EQUAL(tools::Long(325), nTextWidth);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(nTextWidth), aCharWidths.back());

    // 3) caret placement with combining marks, they should not add to ligature
    // component count.
    aText = u"لَاَ بلَاَ";
    aRefCharWidths = { 53, 53, 104, 104, 148, 203, 265, 265, 325, 325 };
    nTextWidth2 = pOutDev->GetTextArray(aText, &aCharWidths, 0, -1, /*bCaret*/true);
    CPPUNIT_ASSERT_EQUAL(aCharWidths[0], aCharWidths[1]);
    CPPUNIT_ASSERT_EQUAL(aCharWidths[2], aCharWidths[3]);
    CPPUNIT_ASSERT_EQUAL(aCharWidths[6], aCharWidths[7]);
    CPPUNIT_ASSERT_EQUAL(aCharWidths[8], aCharWidths[9]);
    CPPUNIT_ASSERT_EQUAL(aRefCharWidths, aCharWidths.get_subunit_array());
    CPPUNIT_ASSERT_EQUAL(tools::Long(325), nTextWidth2);
    CPPUNIT_ASSERT_EQUAL(nTextWidth, nTextWidth2);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(nTextWidth), aCharWidths.back());

    // B. LTR text
    aFont = vcl::Font("Amiri", "Regular", Size(0, 200));
    pOutDev->SetFont(aFont);

    aText = u"fi ffi fl ffl fb ffb";

    // 1) Regular DX array, the ligature width is given to the first components
    // and the next ones are all zero width.
    aRefCharWidths = { 104, 104, 162, 321, 321, 321, 379, 487, 487, 545, 708,
                       708, 708, 766, 926, 926, 984, 1198, 1198, 1198 };
    nTextWidth = pOutDev->GetTextArray(aText, &aCharWidths, 0, -1, /*bCaret*/false);
    CPPUNIT_ASSERT_EQUAL(aRefCharWidths, aCharWidths.get_subunit_array());
    CPPUNIT_ASSERT_EQUAL(tools::Long(1198), nTextWidth);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(nTextWidth), aCharWidths.back());

    // 2) Caret placement DX array, ligature width is distributed over its
    // components.
    aRefCharWidths = { 53, 104, 162, 215, 269, 321, 379, 433, 487, 545, 599,
                       654, 708, 766, 826, 926, 984, 1038, 1097, 1198 };
    nTextWidth = pOutDev->GetTextArray(aText, &aCharWidths, 0, -1, /*bCaret*/true);
    CPPUNIT_ASSERT_EQUAL(aRefCharWidths, aCharWidths.get_subunit_array());
    CPPUNIT_ASSERT_EQUAL(tools::Long(1198), nTextWidth);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(nTextWidth), aCharWidths.back());
#endif
}

CPPUNIT_TEST_SUITE_REGISTRATION(VclComplexTextTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
