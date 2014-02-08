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

#include <algorithm>

namespace vcl {

/*static*/ const rtl::OUString
IconThemeSelector::HIGH_CONTRAST_ICON_THEME_ID("hicontrast");

/*static*/ const rtl::OUString
IconThemeSelector::FALLBACK_ICON_THEME_ID("tango");


namespace {

bool icon_theme_is_in_installed_themes(const rtl::OUString& theme,
        const std::vector<IconThemeInfo>& installedThemes)
{
    return std::find_if(installedThemes.begin(), installedThemes.end(),
               [&](const IconThemeInfo& v) {
                   return v.GetThemeId() == theme;
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
        r = FALLBACK_ICON_THEME_ID;
    }
    return r;
}

rtl::OUString
IconThemeSelector::SelectIconThemeForDesktopEnvironment(
        const std::vector<IconThemeInfo>& installedThemes,
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
        const std::vector<IconThemeInfo>& installedThemes,
        const rtl::OUString& theme) const
{
    if (mUseHighContrastTheme) {
        if (icon_theme_is_in_installed_themes(HIGH_CONTRAST_ICON_THEME_ID, installedThemes)) {
            return HIGH_CONTRAST_ICON_THEME_ID;
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
IconThemeSelector::ReturnFallback(const std::vector<IconThemeInfo>& installedThemes)
{
    if (!installedThemes.empty()) {
        return installedThemes.front().GetThemeId();
    }
    else {
        return FALLBACK_ICON_THEME_ID;
    }
}

} /* namespace vcl */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
