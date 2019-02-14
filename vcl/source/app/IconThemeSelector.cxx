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
#include <config_mpl.h>

#include <algorithm>

namespace vcl {

/*static*/ const OUStringLiteral IconThemeSelector::FALLBACK_ICON_THEME_ID("tango");

namespace {

    class SameTheme
    {
    private:
        const OUString& m_rThemeId;
    public:
        explicit SameTheme(const OUString &rThemeId) : m_rThemeId(rThemeId) {}
        bool operator()(const vcl::IconThemeInfo &rInfo)
        {
            return m_rThemeId == rInfo.GetThemeId();
        }
    };

bool icon_theme_is_in_installed_themes(const OUString& theme,
        const std::vector<IconThemeInfo>& installedThemes)
{
    return std::any_of(installedThemes.begin(), installedThemes.end(),
                       SameTheme(theme));
}

} // end anonymous namespace

IconThemeSelector::IconThemeSelector()
    : mUseHighContrastTheme(false)
    , mPreferDarkIconTheme(false)
{
}

/*static*/ OUString
IconThemeSelector::GetIconThemeForDesktopEnvironment(const OUString& desktopEnvironment)
{
#ifdef _WIN32
    (void)desktopEnvironment;
    return OUString("colibre");
#else
    OUString r;
    if ( desktopEnvironment.equalsIgnoreAsciiCase("kde5") ||
         desktopEnvironment.equalsIgnoreAsciiCase("lxqt") ) {
        r = "breeze";
    }
    else if ( desktopEnvironment.equalsIgnoreAsciiCase("macosx") ) {
#if MPL_HAVE_SUBSET
        r = "tango";
#else
        r = "breeze";
#endif
    }
    else if ( desktopEnvironment.equalsIgnoreAsciiCase("gnome") ||
         desktopEnvironment.equalsIgnoreAsciiCase("mate") ||
         desktopEnvironment.equalsIgnoreAsciiCase("unity") ) {
        r = "elementary";
    } else
    {
        r = FALLBACK_ICON_THEME_ID;
    }
    return r;
#endif // _WIN32
}

OUString
IconThemeSelector::SelectIconThemeForDesktopEnvironment(
        const std::vector<IconThemeInfo>& installedThemes,
        const OUString& desktopEnvironment) const
{
    if (!mPreferredIconTheme.isEmpty()) {
        if (icon_theme_is_in_installed_themes(mPreferredIconTheme, installedThemes)) {
            return mPreferredIconTheme;
        }
        //if a dark variant is preferred, and we didn't have an exact match, then try our one and only dark theme
        if (mPreferDarkIconTheme && icon_theme_is_in_installed_themes("breeze_dark", installedThemes)) {
            return OUString("breeze_dark");
        }
    }

    OUString themeForDesktop = GetIconThemeForDesktopEnvironment(desktopEnvironment);
    if (icon_theme_is_in_installed_themes(themeForDesktop, installedThemes)) {
        return themeForDesktop;
    }

    return ReturnFallback(installedThemes);
}

OUString
IconThemeSelector::SelectIconTheme(
        const std::vector<IconThemeInfo>& installedThemes,
        const OUString& theme) const
{
    if (mUseHighContrastTheme) {
        if (icon_theme_is_in_installed_themes(IconThemeInfo::HIGH_CONTRAST_ID, installedThemes)) {
            return IconThemeInfo::HIGH_CONTRAST_ID;
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
IconThemeSelector::SetPreferredIconTheme(const OUString& theme, bool bDarkIconTheme)
{
    // lower case theme name, and (tdf#120175) replace - with _
    // see icon-themes/README
    mPreferredIconTheme = theme.toAsciiLowerCase().replace('-','_');
    mPreferDarkIconTheme = bDarkIconTheme;
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
    if (mPreferDarkIconTheme != other.mPreferDarkIconTheme) {
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
    return !(*this == other);
}

/*static*/ OUString
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
