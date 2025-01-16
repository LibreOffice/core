/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <sal/log.hxx>
#include <IconThemeScanner.hxx>

#include <salhelper/linkhelper.hxx>
#include <unotools/pathoptions.hxx>
#include <vcl/IconThemeInfo.hxx>
#include <o3tl/string_view.hxx>

namespace vcl
{

IconThemeScanner::IconThemeScanner() = default;

bool IconThemeScanner::addResource(const OUString& rURL)
{
    if (!IconThemeInfo::UrlCanBeParsed(rURL)) {
        return false;
    }
    SAL_INFO("vcl.app", "Found a file that seems to be an icon theme: '" << rURL << "'" );
    IconThemeInfo newTheme(rURL);
    mFoundIconThemes.push_back(newTheme);
    SAL_INFO("vcl.app", "Adding the file as '" << newTheme.GetDisplayName() <<
            "' with id '" << newTheme.GetThemeId() << "'.");
    return true;
}

bool IconThemeScanner::isValidResource(const OUString& filename)
{
    // check whether we can construct an IconThemeInfo from it
    if (!IconThemeInfo::UrlCanBeParsed(filename))
    {
        SAL_INFO("vcl.app", "File '" << filename << "' does not seem to be an icon theme.");
        return false;
    }

    osl::FileStatus fileStatus(osl_FileStatus_Mask_Type);
    if (!vcl::file::readFileStatus(fileStatus, filename))
        return false;

    if (!fileStatus.isRegular())
        return false;

    return true;
}

bool
IconThemeScanner::IconThemeIsInstalled(const OUString& themeId) const
{
    return IconThemeInfo::IconThemeIsInVector(mFoundIconThemes, themeId);
}

/*static*/ OUString
IconThemeScanner::GetStandardIconThemePath()
{
    SvtPathOptions aPathOptions;
    return aPathOptions.GetIconsetPath();
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

const vcl::IconThemeInfo&
IconThemeScanner::GetIconThemeInfo(const OUString& themeId)
{
    std::vector<IconThemeInfo>::iterator info = std::find_if(mFoundIconThemes.begin(), mFoundIconThemes.end(),
        SameTheme(themeId));
    if (info == mFoundIconThemes.end()) {
        SAL_WARN("vcl.app", "Requested information for icon theme with id '" << themeId
                << "' which does not exist.");
        throw std::runtime_error("Requested information on not-installed icon theme");
    }
    return *info;
}

} // end namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
