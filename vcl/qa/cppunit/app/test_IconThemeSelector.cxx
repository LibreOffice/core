/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <IconThemeSelector.hxx>

#include <vcl/IconThemeInfo.hxx>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

class IconThemeSelectorTest : public CppUnit::TestFixture
{
#ifndef _WIN32 //default theme on Windows is Colibre independently from any desktop environment
    void BreezeIsReturnedForKde5Desktop();
    void ElementaryIsReturnedForGnomeDesktop();
    void ThemeIsOverriddenByPreferredTheme();
    void ThemeIsOverriddenByHighContrastMode();
    void NotInstalledThemeDoesNotOverride();
    void InstalledThemeIsFound();
    void FirstThemeIsReturnedIfRequestedThemeIsNotFound();
    void FallbackThemeIsReturnedForEmptyInput();
    void DifferentPreferredThemesAreInequal();
    void DifferentHighContrastModesAreInequal();
    static std::vector<vcl::IconThemeInfo> GetFakeInstalledThemes();
#endif

    // Adds code needed to register the test suite
    CPPUNIT_TEST_SUITE(IconThemeSelectorTest);

#ifndef _WIN32
    CPPUNIT_TEST(BreezeIsReturnedForKde5Desktop);
    CPPUNIT_TEST(ElementaryIsReturnedForGnomeDesktop);
    CPPUNIT_TEST(ThemeIsOverriddenByPreferredTheme);
    CPPUNIT_TEST(ThemeIsOverriddenByHighContrastMode);
    CPPUNIT_TEST(NotInstalledThemeDoesNotOverride);
    CPPUNIT_TEST(InstalledThemeIsFound);
    CPPUNIT_TEST(FirstThemeIsReturnedIfRequestedThemeIsNotFound);
    CPPUNIT_TEST(FallbackThemeIsReturnedForEmptyInput);
    CPPUNIT_TEST(DifferentPreferredThemesAreInequal);
    CPPUNIT_TEST(DifferentHighContrastModesAreInequal);
#endif

    // End of test suite definition
    CPPUNIT_TEST_SUITE_END();
};

#ifndef _WIN32

/*static*/ std::vector<vcl::IconThemeInfo>
IconThemeSelectorTest::GetFakeInstalledThemes()
{
    std::vector<vcl::IconThemeInfo> r;
    vcl::IconThemeInfo a;
    a.mThemeId = "breeze";
    r.push_back(a);
    a.mThemeId = "elementary";
    r.push_back(a);
    a.mThemeId = "colibre";
    r.push_back(a);
    a.mThemeId = "sifr";
    r.push_back(a);
    return r;
}

void
IconThemeSelectorTest::BreezeIsReturnedForKde5Desktop()
{
    std::vector<vcl::IconThemeInfo> themes = GetFakeInstalledThemes();
    vcl::IconThemeSelector s;
    OUString r = s.SelectIconThemeForDesktopEnvironment(themes, u"plasma5"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("'breeze' theme is returned for Plasma 5 desktop", u"breeze"_ustr, r);
}

void
IconThemeSelectorTest::ElementaryIsReturnedForGnomeDesktop()
{
    std::vector<vcl::IconThemeInfo> themes = GetFakeInstalledThemes();
    vcl::IconThemeSelector s;
    OUString r = s.SelectIconThemeForDesktopEnvironment(themes, u"gnome"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("'elementary' theme is returned for gnome desktop", u"elementary"_ustr, r);
}

void
IconThemeSelectorTest::ThemeIsOverriddenByPreferredTheme()
{
    vcl::IconThemeSelector s;
    OUString preferred(u"breeze"_ustr);
    s.SetPreferredIconTheme(preferred, false);
    std::vector<vcl::IconThemeInfo> themes = GetFakeInstalledThemes();
    OUString selected = s.SelectIconThemeForDesktopEnvironment(themes, u"gnome"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("'elementary' theme is overridden by breeze", preferred, selected);
}

void
IconThemeSelectorTest::ThemeIsOverriddenByHighContrastMode()
{
    vcl::IconThemeSelector s;
    s.SetUseHighContrastTheme(true);
    std::vector<vcl::IconThemeInfo> themes = GetFakeInstalledThemes();
    OUString selected = s.SelectIconTheme(themes, u"breeze"_ustr);
    bool sifr = selected.startsWith("sifr");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("'breeze' theme is overridden by high contrast mode", true, sifr);
    s.SetUseHighContrastTheme(false);
    selected = s.SelectIconTheme(themes, u"breeze"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("'breeze' theme is no longer overridden by high contrast mode",
            u"breeze"_ustr, selected);
}

void
IconThemeSelectorTest::NotInstalledThemeDoesNotOverride()
{
    vcl::IconThemeSelector s;
    s.SetPreferredIconTheme(u"breeze_foo"_ustr, false);
    std::vector<vcl::IconThemeInfo> themes = GetFakeInstalledThemes();
    OUString selected = s.SelectIconTheme(themes, u"colibre"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("'colibre' theme is not overridden by 'breeze_foo'", u"colibre"_ustr, selected);
}

void
IconThemeSelectorTest::InstalledThemeIsFound()
{
    vcl::IconThemeSelector s;
    std::vector<vcl::IconThemeInfo> themes = GetFakeInstalledThemes();
    OUString selected = s.SelectIconTheme(themes, u"colibre"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("'colibre' theme is found", u"colibre"_ustr, selected);
}

void
IconThemeSelectorTest::FirstThemeIsReturnedIfRequestedThemeIsNotFound()
{
    vcl::IconThemeSelector s;
    std::vector<vcl::IconThemeInfo> themes = GetFakeInstalledThemes();
    OUString selected = s.SelectIconTheme(themes, u"breeze_foo"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("'breeze' theme is found", themes.front().GetThemeId(), selected);
}

void
IconThemeSelectorTest::FallbackThemeIsReturnedForEmptyInput()
{
    vcl::IconThemeSelector s;
    OUString selected = s.SelectIconTheme(std::vector<vcl::IconThemeInfo>(), u"colibre"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("fallback is returned for empty input",
            vcl::IconThemeSelector::FALLBACK_LIGHT_ICON_THEME_ID, selected);
}

void
IconThemeSelectorTest::DifferentHighContrastModesAreInequal()
{
    vcl::IconThemeSelector s1;
    vcl::IconThemeSelector s2;
    s1.SetUseHighContrastTheme(true);
    s2.SetUseHighContrastTheme(false);
    bool equal = (s1 == s2);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Different high contrast modes are detected as inequal", false, equal);
}

void
IconThemeSelectorTest::DifferentPreferredThemesAreInequal()
{
    vcl::IconThemeSelector s1;
    vcl::IconThemeSelector s2;
    s1.SetPreferredIconTheme(u"breeze"_ustr, false);
    s2.SetUseHighContrastTheme(true);
    bool equal = (s1 == s2);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Different preferred themes are detected as inequal", false, equal);
}

#endif

// Put the test suite in the registry
CPPUNIT_TEST_SUITE_REGISTRATION(IconThemeSelectorTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
