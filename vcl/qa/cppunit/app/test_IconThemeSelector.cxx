/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
    void ThemeIsOverriddenByPreferredTheme();
    void NotInstalledThemeDoesNotOverride();
    void InstalledThemeIsFound();
    void FirstThemeIsReturnedIfRequestedThemeIsNotFound();
    void FallbackThemeIsReturnedForEmptyInput();
    void DifferentPreferredThemesAreInequal();
    void DifferentHighContrastModesAreInequal();
    static std::vector<vcl::IconThemeInfo> GetFakeInstalledThemes();

    // Adds code needed to register the test suite
    CPPUNIT_TEST_SUITE(IconThemeSelectorTest);

    CPPUNIT_TEST(ThemeIsOverriddenByPreferredTheme);
    CPPUNIT_TEST(NotInstalledThemeDoesNotOverride);
    CPPUNIT_TEST(InstalledThemeIsFound);
    CPPUNIT_TEST(FirstThemeIsReturnedIfRequestedThemeIsNotFound);
    CPPUNIT_TEST(FallbackThemeIsReturnedForEmptyInput);
    CPPUNIT_TEST(DifferentPreferredThemesAreInequal);
    CPPUNIT_TEST(DifferentHighContrastModesAreInequal);

    // End of test suite definition
    CPPUNIT_TEST_SUITE_END();
};

/*static*/ std::vector<vcl::IconThemeInfo>
IconThemeSelectorTest::GetFakeInstalledThemes()
{
    std::vector<vcl::IconThemeInfo> r;
    vcl::IconThemeInfo a;
    a.mThemeId = "colibre";
    r.push_back(a);
    a.mThemeId = "colibre_dark";
    r.push_back(a);
    return r;
}

void
IconThemeSelectorTest::ThemeIsOverriddenByPreferredTheme()
{
    vcl::IconThemeSelector s;
    OUString preferred(u"colibre_dark"_ustr);
    s.SetPreferredIconTheme(preferred, false);
    std::vector<vcl::IconThemeInfo> themes = GetFakeInstalledThemes();
    OUString selected = s.SelectIconThemeForDesktopEnvironment(themes);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("default theme is overridden by preferred theme", preferred, selected);
}

void
IconThemeSelectorTest::NotInstalledThemeDoesNotOverride()
{
    vcl::IconThemeSelector s;
    s.SetPreferredIconTheme(u"nonexistent"_ustr, false);
    std::vector<vcl::IconThemeInfo> themes = GetFakeInstalledThemes();
    OUString selected = s.SelectIconTheme(themes, u"colibre"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("'colibre' theme is not overridden by non-installed theme", u"colibre"_ustr, selected);
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
    OUString selected = s.SelectIconTheme(themes, u"nonexistent"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("first theme is returned as fallback", themes.front().GetThemeId(), selected);
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
    s1.SetPreferredIconTheme(u"colibre_dark"_ustr, false);
    s2.SetUseHighContrastTheme(true);
    bool equal = (s1 == s2);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Different preferred themes are detected as inequal", false, equal);
}

// Put the test suite in the registry
CPPUNIT_TEST_SUITE_REGISTRATION(IconThemeSelectorTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
