/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/IconThemeScanner.hxx>

#include <config_folders.h>
#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <salhelper/linkhelper.hxx>

#include <vcl/svapp.hxx>
#include <vcl/IconThemeInfo.hxx>

namespace vcl {

namespace {

// set the status of a file. Returns false if the status could not be determined.
bool set_file_status(osl::FileStatus& status, const OUString& file)
{
    osl::DirectoryItem dirItem;
    osl::FileBase::RC retvalGet = osl::DirectoryItem::get(file, dirItem);
    if (retvalGet != osl::FileBase::E_None) {
        SAL_WARN("vcl.app", "Could not determine status for file '" << file << "'.");
        return false;
    }
    osl::FileBase::RC retvalStatus = dirItem.getFileStatus(status);
    if (retvalStatus != osl::FileBase::E_None) {
        SAL_WARN("vcl.app", "Could not determine status for file '" << file << "'.");
        return false;
    }
    return true;
}

OUString convert_to_absolute_path(const OUString& path)
{
    salhelper::LinkResolver resolver(0);
    osl::FileBase::RC rc = resolver.fetchFileStatus(path);
    if (rc != osl::FileBase::E_None) {
        SAL_WARN("vcl.app", "Could not resolve path '" << path << "' to search for icon themes.");
        if (rc == osl::FileBase::E_MULTIHOP)
        {
            throw std::runtime_error("Provided a recursive symlink to a icon theme directory that could not be resolved.");
        }
    }
    return resolver.m_aStatus.getFileURL();
}

}

IconThemeScanner::IconThemeScanner()
{;}

bool
IconThemeScanner::ScanDirectoryForIconThemes(const OUString& path)
{
    osl::FileStatus fileStatus(osl_FileStatus_Mask_Type);
    bool couldSetFileStatus = set_file_status(fileStatus, path);
    if (!couldSetFileStatus) {
        return false;
    }

    if (!fileStatus.isDirectory()) {
        SAL_INFO("vcl.app", "Cannot search for icon themes in '"<< path << "'. It is not a directory.");
        return false;
    }

    std::vector<OUString> iconThemePaths = ReadIconThemesFromPath(path);
    if (iconThemePaths.empty()) {
        SAL_WARN("vcl.app", "Could not find any icon themes in the provided directory ('" <<path<<"'.");
        return false;
    }
    mFoundIconThemes.clear();
    for (std::vector<OUString>::iterator aI = iconThemePaths.begin(); aI != iconThemePaths.end(); ++aI)
    {
        AddIconThemeByPath(*aI);
    }
    return true;
}

bool
IconThemeScanner::AddIconThemeByPath(const OUString &url)
{
    if (!IconThemeInfo::UrlCanBeParsed(url)) {
        return false;
    }
    SAL_INFO("vcl.app", "Found a file that seems to be an icon theme: '" << url << "'" );
    IconThemeInfo newTheme(url);
    mFoundIconThemes.push_back(newTheme);
    SAL_INFO("vcl.app", "Adding the file as '" << newTheme.GetDisplayName() <<
            "' with id '" << newTheme.GetThemeId() << "'.");
    return true;
}

/*static*/ std::vector<OUString>
IconThemeScanner::ReadIconThemesFromPath(const OUString& dir)
{
    std::vector<OUString> found;
    SAL_INFO("vcl.app", "Scanning directory '" << dir << " for icon themes.");

    osl::Directory dirToScan(dir);
    osl::FileBase::RC retvalOpen = dirToScan.open();
    if (retvalOpen != osl::FileBase::E_None) {
        return found;
    }

    osl::DirectoryItem directoryItem;
    while (dirToScan.getNextItem(directoryItem) == osl::FileBase::E_None) {
        osl::FileStatus status(osl_FileStatus_Mask_Type | osl_FileStatus_Mask_FileURL | osl_FileStatus_Mask_FileName);
        osl::FileBase::RC retvalStatus = directoryItem.getFileStatus(status);
        if (retvalStatus != osl::FileBase::E_None) {
            continue;
        }

        OUString filename = convert_to_absolute_path(status.getFileURL());
        if (!FileIsValidIconTheme(filename)) {
            continue;
        }
        found.push_back(filename);
    }
    return found;
}

/*static*/ bool
IconThemeScanner::FileIsValidIconTheme(const OUString& filename)
{
    // check whether we can construct a IconThemeInfo from it
    if (!IconThemeInfo::UrlCanBeParsed(filename)) {
        SAL_INFO("vcl.app", "File '" << filename << "' does not seem to be an icon theme.");
        return false;
    }

    osl::FileStatus fileStatus(osl_FileStatus_Mask_Type);
    bool couldSetFileStatus = set_file_status(fileStatus, filename);
    if (!couldSetFileStatus) {
        return false;
    }

    if (!fileStatus.isRegular()) {
        return false;
    }
    return true;
}

bool
IconThemeScanner::IconThemeIsInstalled(const OUString& themeId) const
{
    return IconThemeInfo::IconThemeIsInVector(mFoundIconThemes, themeId);
}

/*static*/ std::shared_ptr<IconThemeScanner>
IconThemeScanner::Create(const OUString &path)
{
    std::shared_ptr<IconThemeScanner> retval(new IconThemeScanner);
    retval->ScanDirectoryForIconThemes(path);
    return retval;
}

/*static*/ OUString
IconThemeScanner::GetStandardIconThemePath()
{
    OUString url( "$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER "/config/" );
    rtl::Bootstrap::expandMacros(url);
    return url;
}

IconThemeScanner::~IconThemeScanner()
{;}

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
