/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef VCL_ICONTHEMESECANNER_HXX_
#define VCL_ICONTHEMESECANNER_HXX_

#include <vcl/dllapi.h>
#include <tools/solar.h>
#include <tools/gen.hxx>

#include <rtl/ustring.hxx>

#include <memory>
#include <vector>

// forward declaration of unit test class. Required for friend relationship.
class IconThemeScannerTest;

namespace osl {
class Directory;
class DirectoryItem;
}

namespace vcl {

struct ThemeNameAndUrlToFile {
    rtl::OUString mThemeName;
    rtl::OUString mUrlToFile;
};

/** This class scans a folder for icon themes and provides the results.
 */
class SAL_DLLPUBLIC_EXPORT IconThemeScanner
{
public:

    /** Factory method to create the object.
     * Provide a path to search for IconThemes.
     */
    static std::unique_ptr<IconThemeScanner>
    Create(const rtl::OUString &path);

    /** This method will return the standard path where icon themes are located.
     */
    static rtl::OUString
    GetStandardIconThemePath();

    const std::vector<ThemeNameAndUrlToFile>&
    GetFoundIconThemes() const {return mFoundIconThemes;}

    /** Checks whether the theme with the provided name has been found in the
     * scanned directory.
     */
    bool
    IconThemeIsInstalled(const rtl::OUString&) const;

    /** Obtain the icon size by theme name.
     * @internal
     * It is not clear where this information belongs to. The sizes have been hard-coded. Maybe
     * there is a way to determine the sizes from the icon theme packages.
     */
    static Size
    SizeByThemeName(const rtl::OUString&);

private:
    IconThemeScanner();

    /** Scan a directory for icon themes.
     *
     * @return
     * This method will return true on success.
     * There are several cases when this method will fail:
     * - The directory does not exist
     * - There are no files which which match the pattern images_xxx.zip
     */
    bool
    ScanDirectoryForIconThemes(const rtl::OUString &path);

    /** Adds the provided icon theme by path.
     * The name of the icon theme is derived automatically from the file name
     */
    bool
    AddIconThemeByPath(const rtl::OUString &path);

    /** Scans the provided directory for icon themes.
     * The returned strings will contain the URLs to the icon themes.
     */
    static std::vector<rtl::OUString>
    ReadIconThemesFromPath(const rtl::OUString& dir);

    /** Check whether the provided path is valid */
    static bool
    IconThemeSearchPathIsValid(const rtl::OUString& dir);

    /** Check whether a single file is valid */
    static bool
    FileIsValidIconTheme(const rtl::OUString&);

    /** Determine the icon theme name from the filename
     * If the name has an underscore, the name is taken from the first underscore to the last dot,
     * e.g., images_oxygen.zip becomes oxygen
     * If the name does not have an underscore in it, the whole name until the last dot is returned,
     * e.g. default.zip becomes default
     */
    static rtl::OUString
    FileNameToIconTheme(const rtl::OUString&);

    std::vector<ThemeNameAndUrlToFile> mFoundIconThemes;

    friend class ::IconThemeScannerTest;
};

} // end namespace vcl

#endif /* VCL_ICONTHEMESECANNER_HXX_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
