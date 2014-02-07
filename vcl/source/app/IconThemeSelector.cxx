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

#include <algorithm>

namespace vcl {

/*static*/ const rtl::OUString
IconThemeSelector::HIGH_CONTRAST_ICON_THEME_NAME("hicontrast");

/*static*/ const rtl::OUString
IconThemeSelector::FALLBACK_ICON_THEME_NAME("tango");


namespace {

bool icon_theme_is_in_installed_themes(const rtl::OUString& theme,
        const std::vector<ThemeNameAndUrlToFile>& installedThemes)
{
    return std::find_if(installedThemes.begin(), installedThemes.end(),
               [&](const ThemeNameAndUrlToFile& v) {
                   return v.mThemeName == theme;
               }
           ) != installedThemes.end();
}

} // end anonymous namespace

IconThemeSelector::IconThemeSelector()
: mUseHighContrastTheme(false)
{
}

/*static*/ rtl::OUString
IconThemeSelector::GetIconThemeForDesktopEnvironment(const rtl::OUString& desktopEnvironment)
{
    rtl::OUString r;
    if ( desktopEnvironment.equalsIgnoreAsciiCase("tde") ||
         desktopEnvironment.equalsIgnoreAsciiCase("kde") ) {
        r = "crystal";
    }
    else if ( desktopEnvironment.equalsIgnoreAsciiCase("kde4") ) {
        r = "oxygen";
    }
    else {
        r = FALLBACK_ICON_THEME_NAME;
    }
    return r;
}

rtl::OUString
IconThemeSelector::SelectIconThemeForDesktopEnvironment(
        const std::vector<ThemeNameAndUrlToFile>& installedThemes,
        const rtl::OUString& desktopEnvironment) const
{
    if (!mPreferredIconTheme.isEmpty()) {
        if (icon_theme_is_in_installed_themes(mPreferredIconTheme, installedThemes)) {
            return mPreferredIconTheme;
        }
    }

    rtl::OUString themeForDesktop = GetIconThemeForDesktopEnvironment(desktopEnvironment);
    if (icon_theme_is_in_installed_themes(themeForDesktop, installedThemes)) {
        return themeForDesktop;
    }

    return ReturnFallback(installedThemes);
}

rtl::OUString
IconThemeSelector::SelectIconTheme(
        const std::vector<ThemeNameAndUrlToFile>& installedThemes,
        const rtl::OUString& theme) const
{
    if (mUseHighContrastTheme) {
        if (icon_theme_is_in_installed_themes(HIGH_CONTRAST_ICON_THEME_NAME, installedThemes)) {
            return HIGH_CONTRAST_ICON_THEME_NAME;
        }
    }

    if (icon_theme_is_in_installed_themes(theme, installedThemes)) {
        return theme;
    }

    return ReturnFallback(installedThemes);
}

void
IconThemeSelector::SetUseHighContrastTheme(bool v)
{
    mUseHighContrastTheme = v;
}

void
IconThemeSelector::SetPreferredIconTheme(const rtl::OUString& theme)
{
    mPreferredIconTheme = theme;
}

bool
IconThemeSelector::operator==(const vcl::IconThemeSelector& other) const
{
    if (this == &other) {
        return true;
    }
    if (mPreferredIconTheme != other.mPreferredIconTheme) {
        return false;
    }
    if (mUseHighContrastTheme != other.mUseHighContrastTheme) {
        return false;
    }
    return true;
}

bool
IconThemeSelector::operator!=(const vcl::IconThemeSelector& other) const
{
    return !((*this) == other);
}

/*static*/ rtl::OUString
IconThemeSelector::ReturnFallback(const std::vector<ThemeNameAndUrlToFile>& installedThemes)
{
    if (!installedThemes.empty()) {
        return installedThemes.front().mThemeName;
    }
    else {
        return FALLBACK_ICON_THEME_NAME;
    }
}

} /* namespace vcl */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
