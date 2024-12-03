/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <rtl/ustring.hxx>

#include <vcl/dllapi.h>

#include <vector>

// forward declaration of unit test class. Required for friend relationship.
class IconThemeSelectorTest;

namespace vcl {
class IconThemeInfo;

/** This class helps to choose an icon theme from a list of installed themes.
 *
 * The following factors influence the selection:
 * -# When high contrast mode is enabled, the high contrast icon theme is selected (if it is installed).
 * -# When a preferred theme has been set (e.g., in the gnome desktop settings), that theme is selected.
 */
class VCL_DLLPUBLIC IconThemeSelector {
public:
    IconThemeSelector();

    /** Select an icon theme from the list of installed themes.
     *
     * If high contrast mode has been enabled, the highcontrast theme will be selected (if it is available).
     *
     * @pre
     * @p installedThemes must not be empty
     */
    OUString
    SelectIconTheme(
            const std::vector<IconThemeInfo>& installedThemes,
            const OUString& theme
            ) const;

    /** Select the standard icon theme for a desktop environment from a list of installed themes.
     *
     * If a preferred theme has been set, this one will take precedence.
     *
     * The same logic as in SelectIconTheme() will apply.
     *
     * @pre
     * @p installedThemes must not be empty
     */
    OUString
    SelectIconThemeForDesktopEnvironment(
            const std::vector<IconThemeInfo>& installedThemes,
            const OUString& desktopEnvironment) const;

    void
    SetUseHighContrastTheme(bool);

    /** Returns true if the PreferredIconTheme was changed */
    bool
    SetPreferredIconTheme(const OUString&, bool bDarkIconTheme);

    bool
    operator==(const vcl::IconThemeSelector&) const = default;

    static OUString
    GetIconThemeForDesktopEnvironment(const OUString& desktopEnvironment, bool bPreferDarkIconTheme);

private:
    /** Return the first element of the themes, or the fallback if the vector is empty */
    static OUString
    ReturnFallback(const std::vector<IconThemeInfo>& installedThemes);

    /** The name of the icon themes which are used as fallbacks */
    static constexpr OUString FALLBACK_LIGHT_ICON_THEME_ID = u"colibre"_ustr;
    static constexpr OUString FALLBACK_DARK_ICON_THEME_ID = u"colibre_dark"_ustr;

    OUString mPreferredIconTheme;
    bool mUseHighContrastTheme;
    bool mPreferDarkIconTheme;

    friend class ::IconThemeSelectorTest;
};

} /* namespace vcl */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
