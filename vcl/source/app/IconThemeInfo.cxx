/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/IconThemeInfo.hxx>
#include <rtl/character.hxx>

#include <stdexcept>
#include <algorithm>

// constants for theme ids and display names. Only the theme id for hicontrast is used
// outside of this class and hence made public.

const OUStringLiteral vcl::IconThemeInfo::HIGH_CONTRAST_ID("hicontrast");

namespace {

static const OUStringLiteral HIGH_CONTRAST_DISPLAY_NAME("High Contrast");
static const OUStringLiteral TANGO_TESTING_ID("tango_testing");
static const OUStringLiteral TANGO_TESTING_DISPLAY_NAME("Tango Testing");

OUString
filename_from_url(const OUString& url)
{
    sal_Int32 slashPosition = url.lastIndexOf( '/' );
    if (slashPosition < 0) {
        return OUString("");
    }
    OUString filename = url.copy( slashPosition+1 );
    return filename;
}

} // end anonymous namespace

namespace vcl {

static const char ICON_THEME_PACKAGE_PREFIX[] = "images_";

static const char EXTENSION_FOR_ICON_PACKAGES[] = ".zip";

IconThemeInfo::IconThemeInfo()
{
}

IconThemeInfo::IconThemeInfo(const OUString& urlToFile)
: mUrlToFile(urlToFile)
{
    OUString filename = filename_from_url(urlToFile);
    if (filename.isEmpty()) {
        throw std::runtime_error("invalid URL passed to IconThemeInfo()");
    }

    mThemeId = FileNameToThemeId(filename);
    mDisplayName = ThemeIdToDisplayName(mThemeId);

}

/*static*/ Size
IconThemeInfo::SizeByThemeName(const OUString& themeName)
{
    if (themeName == "tango") {
        return Size( 24, 24 );
    }
    else if (themeName == "crystal") {
        return Size( 22, 22 );
    }
    else if (themeName == "galaxy") {
        return Size( 22, 22 );
    }
    else {
        return Size( 26, 26 );
    }
}

/*static*/ bool
IconThemeInfo::UrlCanBeParsed(const OUString& url)
{
    OUString fname = filename_from_url(url);
    if (fname.isEmpty()) {
        return false;
    }

    if (!fname.startsWithIgnoreAsciiCase(ICON_THEME_PACKAGE_PREFIX)) {
        return false;
    }

    if (!fname.endsWithIgnoreAsciiCase(EXTENSION_FOR_ICON_PACKAGES)) {
        return false;
    }

    return true;
}

/*static*/ OUString
IconThemeInfo::FileNameToThemeId(const OUString& filename)
{
    OUString r;
    sal_Int32 positionOfLastDot = filename.lastIndexOf(EXTENSION_FOR_ICON_PACKAGES);
    if (positionOfLastDot < 0) { // -1 means index not found
        throw std::runtime_error("IconThemeInfo::FileNameToThemeId() called with invalid filename.");
    }
    sal_Int32 positionOfFirstUnderscore = filename.indexOf(ICON_THEME_PACKAGE_PREFIX);
    if (positionOfFirstUnderscore < 0) { // -1 means index not found. Use the whole name instead
        throw std::runtime_error("IconThemeInfo::FileNameToThemeId() called with invalid filename.");
    }
    positionOfFirstUnderscore += RTL_CONSTASCII_LENGTH(ICON_THEME_PACKAGE_PREFIX);
    r = filename.copy(positionOfFirstUnderscore, positionOfLastDot - positionOfFirstUnderscore);
    return r;
}

/*static*/ OUString
IconThemeInfo::ThemeIdToDisplayName(const OUString& themeId)
{
    if (themeId.isEmpty()) {
        throw std::runtime_error("IconThemeInfo::ThemeIdToDisplayName() called with invalid id.");
    }

    // special cases
    if (themeId.equalsIgnoreAsciiCase(HIGH_CONTRAST_ID)) {
        return HIGH_CONTRAST_DISPLAY_NAME;
    }
    else if (themeId.equalsIgnoreAsciiCase(TANGO_TESTING_ID)) {
        return TANGO_TESTING_DISPLAY_NAME;
    }

    // make the first letter uppercase
    OUString r;
    sal_Unicode firstLetter = themeId[0];
    if (rtl::isAsciiLowerCase(firstLetter)) {
        r = OUString(sal_Unicode(rtl::toAsciiUpperCase(firstLetter)));
        r += themeId.copy(1);
    }
    else {
        r = themeId;
    }

    return r;
}

namespace
{
    class SameTheme :
        public std::unary_function<const vcl::IconThemeInfo &, bool>
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
}

/*static*/ const vcl::IconThemeInfo&
IconThemeInfo::FindIconThemeById(const std::vector<vcl::IconThemeInfo>& themes, const OUString& themeId)
{
    std::vector<vcl::IconThemeInfo>::const_iterator it = std::find_if(themes.begin(), themes.end(),
               SameTheme(themeId));
    if (it == themes.end())
    {
        throw std::runtime_error("Could not find theme id in theme vector.");
    }
    return *it;
}

/*static*/ bool
IconThemeInfo::IconThemeIsInVector(const std::vector<vcl::IconThemeInfo>& themes, const OUString& themeId)
{
    return std::any_of(themes.begin(), themes.end(), SameTheme(themeId));
}

} // end namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
