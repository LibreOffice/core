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
};

CPPUNIT_TEST_FIXTURE(Test, testFontFallbackSerif)
{
    // Given a font select pattern with a font name we don't bundle and with a serif family:
    vcl::Font aFont;
    aFont.SetFamilyName("IBM Plex Serif");
    aFont.SetFamily(FAMILY_ROMAN);
    Size aSize(0, 3840);
    float fExactHeight = 3840;
    bool bNonAntialias = false;
    vcl::font::FontSelectPattern aPattern(aFont, aFont.GetFamilyName(), aSize, fExactHeight,
                                          bNonAntialias);
    aPattern.maTargetName = aFont.GetFamilyName();
    psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
    OUString aMissingCodes;

    // When substituting that font:
    rMgr.Substitute(aPattern, aMissingCodes);

    // Then make sure we get a serif fallback:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: Noto Serif (or DejaVu Serif)
    // - Actual  : Noto Kufi Arabic
    // i.e. we got a sans fallback for a serif pattern, which is clearly poor.
    CPPUNIT_ASSERT(aPattern.maSearchName.endsWith(u"Serif"));
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
