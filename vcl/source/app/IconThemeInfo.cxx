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

// constants for theme ids and display names. (The theme id for high contrast is used
// outside of this class and hence made public in IconThemeInfo.)

namespace {

constexpr OUStringLiteral HELPIMG_FAKE_THEME(u"helpimg");

OUString
filename_from_url(std::u16string_view url)
{
    size_t slashPosition = url.rfind( '/' );
    if (slashPosition == std::u16string_view::npos) {
        return OUString();
    }
    OUString filename( url.substr( slashPosition+1 ) );
    return filename;
}

} // end anonymous namespace

namespace vcl {

const sal_Unicode ICON_THEME_PACKAGE_PREFIX[] = u"images_";

const sal_Unicode EXTENSION_FOR_ICON_PACKAGES[] = u".zip";

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
IconThemeInfo::SizeByThemeName(std::u16string_view themeName)
{
   if (themeName == u"galaxy") { //kept for compiler because of unused parameter 'themeName'
     return Size( 26, 26 );
   }
   else {
     return Size( 24, 24 );
   }
}

/*static*/ bool
IconThemeInfo::UrlCanBeParsed(std::u16string_view url)
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

    if (fname.indexOf(HELPIMG_FAKE_THEME) != -1 ) {
        return false;
    }

    return true;
}

/*static*/ OUString
IconThemeInfo::FileNameToThemeId(std::u16string_view filename)
{
    OUString r;
    size_t positionOfLastDot = filename.rfind(EXTENSION_FOR_ICON_PACKAGES);
    if (positionOfLastDot == std::u16string_view::npos) { // means index not found
        throw std::runtime_error("IconThemeInfo::FileNameToThemeId() called with invalid filename.");
    }
    size_t positionOfFirstUnderscore = filename.find(ICON_THEME_PACKAGE_PREFIX);
    if (positionOfFirstUnderscore == std::u16string_view::npos) { // means index not found. Use the whole name instead
        throw std::runtime_error("IconThemeInfo::FileNameToThemeId() called with invalid filename.");
    }
    positionOfFirstUnderscore += RTL_CONSTASCII_LENGTH(ICON_THEME_PACKAGE_PREFIX);
    r = filename.substr(positionOfFirstUnderscore, positionOfLastDot - positionOfFirstUnderscore);
    return r;
}

/*static*/ OUString
IconThemeInfo::ThemeIdToDisplayName(const OUString& themeId)
{
    if (themeId.isEmpty()) {
        throw std::runtime_error("IconThemeInfo::ThemeIdToDisplayName() called with invalid id.");
    }

    // Strip _svg and _dark filename "extensions"
    OUString aDisplayName = themeId;

    bool bIsSvg = aDisplayName.endsWith("_svg", &aDisplayName);
    bool bIsDark = aDisplayName.endsWith("_dark", &aDisplayName);
    if (!bIsSvg && bIsDark)
        bIsSvg = aDisplayName.endsWith("_svg", &aDisplayName);

    // make the first letter uppercase
    sal_Unicode firstLetter = aDisplayName[0];
    if (rtl::isAsciiLowerCase(firstLetter))
    {
        aDisplayName = OUStringChar(sal_Unicode(rtl::toAsciiUpperCase(firstLetter))) + aDisplayName.subView(1);
    }

    // replacing underscores with spaces of multi words pack name.
    aDisplayName = aDisplayName.replace('_', ' ');

    if (bIsSvg && bIsDark)
        aDisplayName += " (SVG + dark)";
    else if (bIsSvg)
        aDisplayName += " (SVG)";
    else if (bIsDark)
        aDisplayName += " (dark)";

    return aDisplayName;
}

namespace
{
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
