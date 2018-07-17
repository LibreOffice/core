/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_ICONTHEMESCANNER_HXX
#define INCLUDED_VCL_ICONTHEMESCANNER_HXX

#include <vcl/dllapi.h>
#include <tools/solar.h>

#include <rtl/ustring.hxx>
#include <vcl/IconThemeInfo.hxx>

#include <memory>
#include <vector>

// forward declaration of unit test class. Required for friend relationship.
class IconThemeScannerTest;

namespace osl {
class Directory;
class DirectoryItem;
}

namespace vcl {

/** This class scans a folder for icon themes and provides the results.
 */
class VCL_DLLPUBLIC IconThemeScanner
{
public:
    ~IconThemeScanner();

    /** Factory method to create the object.
     * Provide a path to search for IconThemes.
     */
    static std::shared_ptr<IconThemeScanner> Create(const OUString &path);

    /** This method will return the standard path where icon themes are located.
     */
    static OUString
    GetStandardIconThemePath();

    const std::vector<IconThemeInfo>&
    GetFoundIconThemes() const {return mFoundIconThemes;}

    /** Get the IconThemeInfo for a theme.
     * If the theme id is not among the found themes, a std::runtime_error will be thrown.
     * Use IconThemeIsInstalled() to check whether it is available.
     */
    const IconThemeInfo& GetIconThemeInfo(const OUString& themeId);

    /** Checks whether the theme with the provided name has been found in the
     * scanned directory.
     */
    bool
    IconThemeIsInstalled(const OUString& themeId) const;

private:
    IconThemeScanner();

    /** Scan a directory for icon themes.
     *
     * @return
     * There are several cases when this method will fail:
     * - The directory does not exist
     * - There are no files which match the pattern images_xxx.zip
     */
    void ScanDirectoryForIconThemes(const OUString &path);

    /** Adds the provided icon theme by path.
     */
    bool
    AddIconThemeByPath(const OUString &path);

    /** Scans the provided directory for icon themes.
     * The returned strings will contain the URLs to the icon themes.
     */
    static std::vector<OUString>
    ReadIconThemesFromPath(const OUString& dir);

    /** Check whether a single file is valid */
    static bool
    FileIsValidIconTheme(const OUString&);

    std::vector<IconThemeInfo> mFoundIconThemes;

    friend class ::IconThemeScannerTest;
};

} // end namespace vcl

#endif // INCLUDED_VCL_ICONTHEMESCANNER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
