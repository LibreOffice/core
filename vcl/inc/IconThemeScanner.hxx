/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/dllapi.h>

#include <rtl/ustring.hxx>
#include <vcl/IconThemeInfo.hxx>
#include <vcl/UserResourceScanner.hxx>

#include <memory>
#include <vector>

// forward declaration of unit test class. Required for friend relationship.
class IconThemeScannerTest;

namespace vcl
{
/** This class scans a folder for icon themes and provides the results.
 */
class VCL_DLLPUBLIC IconThemeScanner : public UserResourceScanner
{
public:
    IconThemeScanner();

    /** This method will return the standard path where icon themes are located.
     */
    static OUString GetStandardIconThemePath();

    const std::vector<IconThemeInfo>& GetFoundIconThemes() const { return mFoundIconThemes; }

    /** Get the IconThemeInfo for a theme.
     * If the theme id is not among the found themes, a std::runtime_error will be thrown.
     * Use IconThemeIsInstalled() to check whether it is available.
     */
    const IconThemeInfo& GetIconThemeInfo(const OUString& themeId);

    /** Checks whether the theme with the provided name has been found in the
     * scanned directory.
     */
    bool IconThemeIsInstalled(const OUString& themeId) const;

private:
    /** Adds the provided icon theme by path. */
    bool addResource(const OUString& path) override;

    /** Check whether a single file is valid */
    bool isValidResource(const OUString& rFilename) override;

    std::vector<IconThemeInfo> mFoundIconThemes;

    friend class ::IconThemeScannerTest;
};

} // end namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
