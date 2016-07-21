/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/IconThemeSelector.hxx>

#include <vcl/IconThemeScanner.hxx>
#include <vcl/IconThemeInfo.hxx>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

class IconThemeSelectorTest : public CppUnit::TestFixture
{
    void
    OxygenThemeIsReturnedForKde4Desktop();

    void
    TangoThemeIsReturnedForGtkDesktop();

    void
    ThemeIsOverriddenByPreferredTheme();

    void
    ThemeIsOverriddenByHighContrastMode();

    void
    NotInstalledThemeDoesNotOverride();

    void
    InstalledThemeIsFound();

    void
    FirstThemeIsReturnedIfRequestedThemeIsNotFound();

    void
    FallbackThemeIsReturnedForEmptyInput();

    void
    DifferentPreferredThemesAreInequal();

    void
    DifferentHighContrastModesAreInequal();

    static std::vector<vcl::IconThemeInfo>
    GetFakeInstalledThemes();

    // Adds code needed to register the test suite
    CPPUNIT_TEST_SUITE(IconThemeSelectorTest);

    CPPUNIT_TEST(OxygenThemeIsReturnedForKde4Desktop);
    CPPUNIT_TEST(TangoThemeIsReturnedForGtkDesktop);
    CPPUNIT_TEST(ThemeIsOverriddenByPreferredTheme);
    CPPUNIT_TEST(ThemeIsOverriddenByHighContrastMode);
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
    a.mThemeId = "tango";
    r.push_back(a);
    a.mThemeId = "oxygen";
    r.push_back(a);
    a.mThemeId = "hicontrast";
    r.push_back(a);
    return r;
}

void
IconThemeSelectorTest::OxygenThemeIsReturnedForKde4Desktop()
{
    std::vector<vcl::IconThemeInfo> themes = GetFakeInstalledThemes();
    vcl::IconThemeSelector s;
    OUString r = s.SelectIconThemeForDesktopEnvironment(themes, "kde4");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("'oxygen' theme is returned for kde4 desktop", OUString("oxygen"), r);
}

void
IconThemeSelectorTest::TangoThemeIsReturnedForGtkDesktop()
{
    std::vector<vcl::IconThemeInfo> themes = GetFakeInstalledThemes();
    vcl::IconThemeSelector s;
    OUString r = s.SelectIconThemeForDesktopEnvironment(themes, "gtk");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("'tango' theme is returned for kde4 desktop", OUString("tango"), r);
}

void
IconThemeSelectorTest::ThemeIsOverriddenByPreferredTheme()
{
    vcl::IconThemeSelector s;
    OUString preferred("oxygen");
    s.SetPreferredIconTheme(preferred);
    std::vector<vcl::IconThemeInfo> themes = GetFakeInstalledThemes();
    OUString selected = s.SelectIconThemeForDesktopEnvironment(themes, "gtk");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("'tango' theme is overridden by oxygen", preferred, selected);
}

void
IconThemeSelectorTest::ThemeIsOverriddenByHighContrastMode()
{
    vcl::IconThemeSelector s;
    s.SetUseHighContrastTheme(true);
    std::vector<vcl::IconThemeInfo> themes = GetFakeInstalledThemes();
    OUString selected = s.SelectIconTheme(themes, "tango");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("'tango' theme is overridden by high contrast mode",
            OUString("hicontrast"), selected);
    s.SetUseHighContrastTheme(false);
    selected = s.SelectIconTheme(themes, "tango");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("'tango' theme is no longer overridden by high contrast mode",
            OUString("tango"), selected);
}

void
IconThemeSelectorTest::NotInstalledThemeDoesNotOverride()
{
    vcl::IconThemeSelector s;
    s.SetPreferredIconTheme("oxygen_katze");
    std::vector<vcl::IconThemeInfo> themes = GetFakeInstalledThemes();
    OUString selected = s.SelectIconTheme(themes, "oxygen");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("'oxygen' theme is not overridden by 'oxygen_katze'", OUString("oxygen"), selected);
}

void
IconThemeSelectorTest::InstalledThemeIsFound()
{
    vcl::IconThemeSelector s;
    std::vector<vcl::IconThemeInfo> themes = GetFakeInstalledThemes();
    OUString selected = s.SelectIconTheme(themes, "oxygen");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("'oxygen' theme is found", OUString("oxygen"), selected);
}

void
IconThemeSelectorTest::FirstThemeIsReturnedIfRequestedThemeIsNotFound()
{
    vcl::IconThemeSelector s;
    std::vector<vcl::IconThemeInfo> themes = GetFakeInstalledThemes();
    OUString selected = s.SelectIconTheme(themes, "oxygen_katze");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("'oxygen' theme is found", themes.front().GetThemeId(), selected);
}

void
IconThemeSelectorTest::FallbackThemeIsReturnedForEmptyInput()
{
    vcl::IconThemeSelector s;
    OUString selected = s.SelectIconTheme(std::vector<vcl::IconThemeInfo>(), "oxygen");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("fallback is returned for empty input",
            OUString(vcl::IconThemeSelector::FALLBACK_ICON_THEME_ID), selected);
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
    s1.SetPreferredIconTheme("oxygen");
    s2.SetUseHighContrastTheme(true);
    bool equal = (s1 == s2);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Different preferred themes are detected as inequal", false, equal);
}

// Put the test suite in the registry
CPPUNIT_TEST_SUITE_REGISTRATION(IconThemeSelectorTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
