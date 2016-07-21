/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_ICONTHEMEINFO_HXX
#define INCLUDED_VCL_ICONTHEMEINFO_HXX

#include <vcl/dllapi.h>
#include <tools/gen.hxx>
#include <rtl/ustring.hxx>

#include <vector>

// forward declaration of unit test classes. Required for friend relationship.
class IconThemeInfoTest;
class IconThemeSelectorTest;
class IconThemeScannerTest;

namespace vcl {

/** This class provides information about an icon theme.
 */
class VCL_DLLPUBLIC IconThemeInfo {

public:

    /** The name of the icon theme to use for high contrast mode */
    static const OUStringLiteral HIGH_CONTRAST_ID;

    /** Construct an IconThemeInfo from the URL to a file.
     * This method will throw a std::runtime_error if the URL cannot be properly parsed.
     * Check the URL with UrlCanBeParsed() first.
     */
    IconThemeInfo(const OUString& urlToFile);

    const OUString& GetDisplayName() const {return mDisplayName;}

    const OUString& GetThemeId() const { return mThemeId; }

    const OUString& GetUrlToFile() const { return mUrlToFile; }

    /** Obtain the icon size by theme name.
     * @internal
     * It is not clear where this information belongs to. The sizes were hard-coded before they moved here.
     * Maybe there is a way to determine the sizes from the icon theme packages.
     */
    static Size SizeByThemeName(const OUString&);

    /** Check whether a IconThemeInfo can be constructed from an URL */
    static bool UrlCanBeParsed(const OUString& url);

    /** Find an icon theme by its id in a vector.
     * Throws a runtime_error if the theme is not contained in the vector
     */
    static const vcl::IconThemeInfo&
    FindIconThemeById(const std::vector<vcl::IconThemeInfo>& themes, const OUString& themeId);

    /** Check whether a theme with a specified id is in a vector of IconThemeInfo */
    static bool
    IconThemeIsInVector(const std::vector<vcl::IconThemeInfo>& themes, const OUString& themeId);

private:
    /** private constructor for testing purposes only */
    IconThemeInfo();

    /** Determine the icon theme name from the filename
     * If the name has an underscore, the name is taken from the first underscore to the last dot,
     * e.g., images_oxygen.zip becomes oxygen
     * If the name does not have an underscore in it, the whole name until the last dot is returned,
     * e.g. default.zip becomes default
     */
    static OUString FileNameToThemeId(const OUString&);

    /** Creates the display name for the given id of a file.
     * Currently, we only uppercase the id.
     */
    static OUString ThemeIdToDisplayName(const OUString&);

    /** The name which is presented to the user */
    OUString mDisplayName;
    /** The theme id. This id is used in ... to determine the file name */
    OUString mThemeId;
    /** The url to the icon theme package */
    OUString mUrlToFile;

    friend class ::IconThemeInfoTest;
    friend class ::IconThemeScannerTest;
    friend class ::IconThemeSelectorTest;
};

} // namespace vcl


#endif // INCLUDED_VCL_ICONTHEMEINFO_HXX
