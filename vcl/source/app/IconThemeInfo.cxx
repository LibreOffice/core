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

using rtl::OUString;

namespace {

rtl::OUString
filename_from_url(const rtl::OUString& url)
{
    sal_Int32 slashPosition = url.lastIndexOf( '/' );
    if (slashPosition < 0) {
        return rtl::OUString("");
    }
    rtl::OUString filename = url.copy( slashPosition+1 );
    return filename;
}

}

namespace vcl {
const rtl::OUString
IconThemeInfo::ICON_THEME_PACKAGE_PREFIX("images_");

const rtl::OUString
IconThemeInfo::EXTENSION_FOR_ICON_PACKAGES(".zip");

IconThemeInfo::IconThemeInfo()
{;}

IconThemeInfo::IconThemeInfo(const rtl::OUString& urlToFile)
: mUrlToFile(urlToFile)
{
    rtl::OUString filename = filename_from_url(urlToFile);
    if (filename.isEmpty()) {
        throw std::runtime_error("invalid URL passed to IconThemeInfo()");
    }

    mThemeId = FileNameToThemeId(filename);
    mDisplayName = ThemeIdToDisplayName(mThemeId);

}

/*static*/ Size
IconThemeInfo::SizeByThemeName(const rtl::OUString& themeName)
{
    if (themeName == "tango") {
        return Size( 24, 24 );
    }
    else if (themeName == "crystal") {
        return Size( 22, 22 );
    }
    else if (themeName == "default") {
        return Size( 22, 22 );
    }
    else {
        return Size( 26, 26 );
    }
}

/*static*/ bool
IconThemeInfo::UrlCanBeParsed(const rtl::OUString& url)
{
    rtl::OUString fname = filename_from_url(url);
    if (fname.isEmpty()) {
        return false;
    }

    if (fname == "default.zip") {
        return true;
    }

    if (!fname.startsWithIgnoreAsciiCase(ICON_THEME_PACKAGE_PREFIX)) {
        return false;
    }

    if (!fname.endsWithIgnoreAsciiCase(EXTENSION_FOR_ICON_PACKAGES, nullptr)) {
        return false;
    }

    return true;
}

/*static*/ rtl::OUString
IconThemeInfo::FileNameToThemeId(const rtl::OUString& filename)
{
    if (filename == "default.zip") {
        return OUString("default");
    }
    rtl::OUString r;
    sal_Int32 positionOfLastDot = filename.lastIndexOf(EXTENSION_FOR_ICON_PACKAGES);
    if (positionOfLastDot < 0) { // -1 means index not found
        throw std::runtime_error("IconThemeInfo::FileNameToThemeId() called with invalid filename.");
    }
    sal_Int32 positionOfFirstUnderscore = filename.indexOf(ICON_THEME_PACKAGE_PREFIX);
    if (positionOfFirstUnderscore < 0) { // -1 means index not found. Use the whole name instead
        throw std::runtime_error("IconThemeInfo::FileNameToThemeId() called with invalid filename.");
    }
    positionOfFirstUnderscore += ICON_THEME_PACKAGE_PREFIX.getLength();
    r = filename.copy(positionOfFirstUnderscore, positionOfLastDot - positionOfFirstUnderscore);
    return r;
}

/*static*/ rtl::OUString
IconThemeInfo::ThemeIdToDisplayName(const rtl::OUString& themeId)
{
    if (themeId.isEmpty()) {
        throw std::runtime_error("IconThemeInfo::ThemeIdToDisplayName() called with invalid id.");
    }

    if (themeId == "default") {
        return rtl::OUString("Galaxy");
    }

    // make the first letter uppercase
    rtl::OUString r;
    sal_Unicode firstLetter = themeId[0];
    if (rtl::isAsciiLowerCase(firstLetter)) {
        r = rtl::OUString(rtl::toAsciiUpperCase(firstLetter));
        r += themeId.copy(1);
    }
    else {
        r = themeId;
    }
    return r;
}

/*static*/ const vcl::IconThemeInfo&
IconThemeInfo::FindIconThemeById(const std::vector<vcl::IconThemeInfo>& themes, const rtl::OUString& themeId)
{
    auto it = std::find_if(themes.begin(), themes.end(),
               [&](const IconThemeInfo& v) {
                   return v.GetThemeId() == themeId;
               });
    if (it == themes.end()) {
        throw std::runtime_error("Could not find theme id in theme vector.");
    }
    return *it;
}

/*static*/ bool
IconThemeInfo::IconThemeIsInVector(const std::vector<vcl::IconThemeInfo>& themes, const rtl::OUString& themeId)
{
    auto it = std::find_if(themes.begin(), themes.end(),
               [&](const IconThemeInfo& v) {
                   return v.GetThemeId() == themeId;
               });
    if (it != themes.end()) {
        return true;
    }
    else {
        return false;
    }
}

} // end namespace vcl


