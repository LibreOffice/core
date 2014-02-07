/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef VCL_ICONTHEMESELECTOR_HXX_
#define VCL_ICONTHEMESELECTOR_HXX_

#include <rtl/ustring.hxx>

#include <vcl/dllapi.h>

#include <vector>

// forward declaration of unit test class. Required for friend relationship.
class IconThemeSelectorTest;

namespace vcl {
struct ThemeNameAndUrlToFile;

/** This class helps to choose an icon theme from a list of installed themes.
 *
 * The following factors influence the selection:
 * -# When high contrast mode is enabled, the high contrast icon theme is selected (if it is installed).
 * -# When a preferred theme has been set (e.g., in the gnome desktop settings), that theme is selected.
 */
class SAL_DLLPUBLIC_EXPORT IconThemeSelector {
public:
    IconThemeSelector();

    /** Select an icon theme from the list of installed themes.
     *
     * If high contrast mode has been enabled, the highcontrast theme will be selected (if it is available).
     *
     * @pre
     * @p installedThemes must not be empty
     */
    rtl::OUString
    SelectIconTheme(
            const std::vector<ThemeNameAndUrlToFile>& installedThemes,
            const rtl::OUString& theme
            ) const;

    /** Select the standard icon theme for a desktop environment from a list of installed themes.
     *
     * The same logic as in SelectIconTheme() will apply.
     *
     * @pre
     * @p installedThemes must not be empty
     */
    rtl::OUString
    SelectIconThemeForDesktopEnvironment(
            const std::vector<ThemeNameAndUrlToFile>& installedThemes,
            const rtl::OUString& desktopEnvironment) const;

    void
    SetUseHighContrastTheme(bool);

    void
    SetPreferredIconTheme(const rtl::OUString& theme);

    bool
    operator==(const vcl::IconThemeSelector&) const;

    bool
    operator!=(const vcl::IconThemeSelector&) const;

private:
    /** Return the first element of the themes, or the fallback if the vector is empty */
    static rtl::OUString
    ReturnFallback(const std::vector<ThemeNameAndUrlToFile>& installedThemes);

    /** The name of the icon theme to use for high contrast mode */
    static const rtl::OUString
    HIGH_CONTRAST_ICON_THEME_NAME;

    /** The name of the icon theme which is used as fallback */
    static const rtl::OUString
    FALLBACK_ICON_THEME_NAME;


    static rtl::OUString
    GetIconThemeForDesktopEnvironment(const rtl::OUString& desktopEnvironment);

    rtl::OUString mPreferredIconTheme;
    bool mUseHighContrastTheme;

    friend class ::IconThemeSelectorTest;
};

} /* namespace vcl */

#endif /* VCL_ICONTHEMESELECTOR_HXX_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
