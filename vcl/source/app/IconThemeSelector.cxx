/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/lok.hxx>

#include <IconThemeSelector.hxx>

#include <tools/color.hxx>
#include <vcl/IconThemeInfo.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <config_mpl.h>

#include <algorithm>

namespace vcl {

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
IconThemeSelector::GetIconThemeForDesktopEnvironment(const OUString& desktopEnvironment, bool bPreferDarkIconTheme)
{
    if (comphelper::LibreOfficeKit::isActive())
    {
        if (!bPreferDarkIconTheme)
            return "colibre";
        else
            return "colibre_dark";
    }

#ifdef _WIN32
    (void)desktopEnvironment;
    if (!bPreferDarkIconTheme)
        return "colibre";
    else
        return "colibre_dark";
#else
    OUString r;
    if ( desktopEnvironment.equalsIgnoreAsciiCase("plasma5") ||
         desktopEnvironment.equalsIgnoreAsciiCase("plasma6") ||
         desktopEnvironment.equalsIgnoreAsciiCase("lxqt") ) {
        if (!bPreferDarkIconTheme)
            r = "breeze";
        else
            r = "breeze_dark";
    }
    else if ( desktopEnvironment.equalsIgnoreAsciiCase("macosx") ) {
        if (!bPreferDarkIconTheme)
            r = "sukapura_svg";
        else
            r = "sukapura_dark_svg";
    }
    else if ( desktopEnvironment.equalsIgnoreAsciiCase("gnome") ||
         desktopEnvironment.equalsIgnoreAsciiCase("mate") ||
         desktopEnvironment.equalsIgnoreAsciiCase("unity") ) {
        if (!bPreferDarkIconTheme)
            r = "elementary";
        else
            r = "sifr_dark";
    } else
    {
        if (!bPreferDarkIconTheme)
            r = FALLBACK_LIGHT_ICON_THEME_ID;
        else
            r = FALLBACK_DARK_ICON_THEME_ID;
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
    }

    OUString themeForDesktop = GetIconThemeForDesktopEnvironment(desktopEnvironment, mPreferDarkIconTheme);
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
        const Color aCol(Application::GetSettings().GetStyleSettings().GetWindowColor());
        const OUString name(aCol.IsDark() ? IconThemeInfo::HIGH_CONTRAST_ID_DARK
                                          : IconThemeInfo::HIGH_CONTRAST_ID_BRIGHT);
        if (icon_theme_is_in_installed_themes(name, installedThemes)) {
            return name;
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

bool
IconThemeSelector::SetPreferredIconTheme(const OUString& theme, bool bDarkIconTheme)
{
    // lower case theme name, and (tdf#120175) replace - with _
    // see icon-themes/README
    OUString sIconTheme = theme.toAsciiLowerCase().replace('-','_');

    const bool bChanged = mPreferredIconTheme != sIconTheme || mPreferDarkIconTheme != bDarkIconTheme;
    if (bChanged)
    {
        mPreferredIconTheme = sIconTheme;
        mPreferDarkIconTheme = bDarkIconTheme;
    }
    return bChanged;
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
        return FALLBACK_LIGHT_ICON_THEME_ID;
    }
}

} /* namespace vcl */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
