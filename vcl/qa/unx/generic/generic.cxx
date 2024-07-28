/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <vcl/font.hxx>
#include <vcl/vclptr.hxx>
#include <vcl/wintypes.hxx>
#include <vcl/window.hxx>
#include <vcl/glyphitemcache.hxx>

#include <font/FontSelectPattern.hxx>
#include <unx/fontmanager.hxx>

using namespace ::com::sun::star;

namespace
{
/// Covers vcl/unx/generic/ fixes.
class Test : public UnoApiTest
{
public:
    Test()
        : UnoApiTest(u"/vcl/qa/unx/generic/data/"_ustr)
    {
    }
    static OUString GetFallbackFont(const vcl::Font& rFont)
    {
        Size aSize(0, 3840);
        float fExactHeight = 3840;
        bool bNonAntialias = false;
        vcl::font::FontSelectPattern aPattern(rFont, rFont.GetFamilyName(), aSize, fExactHeight,
                                              bNonAntialias);
        aPattern.maTargetName = rFont.GetFamilyName();
        psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
        OUString aMissingCodes;

        rMgr.Substitute(aPattern, aMissingCodes);

        return aPattern.maSearchName;
    }
};

CPPUNIT_TEST_FIXTURE(Test, testFontFallbackSerif)
{
    OUString sResolvedSerif = GetFallbackFont(vcl::Font("serif", Size(12, 12)));

    // Given a font select pattern with a font name we don't bundle and with a serif family:
    vcl::Font aFont;
    aFont.SetFamilyName("IBM Plex Serif");
    aFont.SetFamily(FAMILY_ROMAN);

    OUString sPlexFallback = GetFallbackFont(aFont);

    // Then make sure we get a serif fallback:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: Noto Serif (or DejaVu Serif)
    // - Actual  : Noto Kufi Arabic
    // i.e. we got a sans fallback for a serif pattern, which is clearly poor.
    CPPUNIT_ASSERT_EQUAL(sResolvedSerif, sPlexFallback);
}

CPPUNIT_TEST_FIXTURE(Test, testFontFallbackCaching)
{
    // Given a vcl-level text layout, created for the serif Verdana:
    ScopedVclPtrInstance<vcl::Window> pWindow(nullptr, WB_APP | WB_STDWORK);
    VclPtr<OutputDevice> pOutDev = pWindow->GetOutDev();
    vcl::Font aFont;
    aFont.SetFamilyName("Verdana");
    aFont.SetFamily(FAMILY_ROMAN);
    pOutDev->SetFont(aFont);
    OUString aText = u"1-1-2017"_ustr;
    sal_Int32 nIndex = 0;
    sal_Int32 nLength = aText.getLength();
    tools::Long nLogicWidth = 0;
    SalLayoutGlyphsCache::CachedGlyphsKey aKey1(pOutDev, aText, nIndex, nLength, nLogicWidth);

    // When creating a layout for the sans Verdana:
    aFont.SetFamily(FAMILY_SWISS);
    pOutDev->SetFont(aFont);
    SalLayoutGlyphsCache::CachedGlyphsKey aKey2(pOutDev, aText, nIndex, nLength, nLogicWidth);

    // Then make sure these two layouts don't match:
    // Without the accompanying fix in place, this test would have failed, the Noto Serif layout was
    // reused for the Noto Sans layout if those were selected as fallbacks.
    CPPUNIT_ASSERT(aKey1 != aKey2);
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
