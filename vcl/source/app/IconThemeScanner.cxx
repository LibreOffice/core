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

#include <vcl/svapp.hxx>
#include <vcl/IconThemeInfo.hxx>

namespace vcl {

namespace {

bool
search_path_is_valid(const rtl::OUString& dir)
{
    osl::DirectoryItem dirItem;
    osl::FileBase::RC retvalGet = osl::DirectoryItem::get(dir, dirItem);
    if (retvalGet != osl::FileBase::RC::E_None) {
        return false;
    }
    osl::FileStatus fileStatus(osl_FileStatus_Mask_Type);
    osl::FileBase::RC retvalStatus = dirItem.getFileStatus(fileStatus);
    if (retvalStatus != osl::FileBase::RC::E_None) {
        return false;
    }

    if (!fileStatus.isDirectory()) {
        return false;
    }
    return true;
}

}

IconThemeScanner::IconThemeScanner()
{;}

bool
IconThemeScanner::ScanDirectoryForIconThemes(const rtl::OUString& path)
{
    bool pathIsValid = search_path_is_valid(path);
    if (!pathIsValid) {
        return false;
    }
    std::vector<rtl::OUString> iconThemePaths = ReadIconThemesFromPath(path);
    if (iconThemePaths.empty()) {
        return false;
    }
    mFoundIconThemes.clear();
    for (const rtl::OUString& pathToTheme : iconThemePaths) {
        AddIconThemeByPath(pathToTheme);
    }
    return true;
}

bool
IconThemeScanner::AddIconThemeByPath(const rtl::OUString &url)
{
    if (!IconThemeInfo::UrlCanBeParsed(url)) {
        return false;
    }
    IconThemeInfo newTheme{url};
    mFoundIconThemes.push_back(newTheme);
    return true;
}

/*static*/ std::vector<rtl::OUString>
IconThemeScanner::ReadIconThemesFromPath(const rtl::OUString& dir)
{
    std::vector<rtl::OUString> found;

    osl::Directory dirToScan(dir);
    osl::FileBase::RC retvalOpen = dirToScan.open();
    if (retvalOpen != osl::FileBase::RC::E_None) {
        return found;
    }

    osl::DirectoryItem directoryItem;
    while (dirToScan.getNextItem(directoryItem) == osl::FileBase::RC::E_None) {
        osl::FileStatus status(osl_FileStatus_Mask_Type | osl_FileStatus_Mask_FileURL | osl_FileStatus_Mask_FileName);
        osl::FileBase::RC retvalStatus = directoryItem.getFileStatus(status);
        if (retvalStatus != osl::FileBase::RC::E_None) {
            continue;
        }
        if (!status.isRegular()) {
            continue;
        }
        if (!FileIsValidIconTheme(status.getFileURL())) {
            continue;
        }
        rtl::OUString entry;
        entry = status.getFileURL();
        found.push_back(entry);
    }
    return found;
}

/*static*/ bool
IconThemeScanner::FileIsValidIconTheme(const rtl::OUString& filename)
{
    // check whether we can construct a IconThemeInfo from it
    if (!IconThemeInfo::UrlCanBeParsed(filename)) {
        return false;
    }

    // check whether the file is a regular file
    osl::DirectoryItem dirItem;
    osl::FileBase::RC retvalGet = osl::DirectoryItem::get(filename, dirItem);
    if (retvalGet != osl::FileBase::RC::E_None) {
        return false;
    }
    osl::FileStatus fileStatus(osl_FileStatus_Mask_Type);
    osl::FileBase::RC retvalStatus = dirItem.getFileStatus(fileStatus);
    if (retvalStatus != osl::FileBase::RC::E_None) {
        return false;
    }
    if (!fileStatus.isRegular()) {
        return false;
    }
    // TODO: We could add checks more checks here, e.g., whether the file is actually a zip file,
    //       or check whether the required files are contained in the zip file.
    return true;
}

bool
IconThemeScanner::IconThemeIsInstalled(const rtl::OUString& themeId) const
{
    return IconThemeInfo::IconThemeIsInVector(mFoundIconThemes, themeId);
}

/*static*/ std::unique_ptr<IconThemeScanner>
IconThemeScanner::Create(const rtl::OUString &path)
{
    std::unique_ptr<IconThemeScanner> retval(new IconThemeScanner{});
    retval->ScanDirectoryForIconThemes(path);
    return retval;
}

/*static*/ rtl::OUString
IconThemeScanner::GetStandardIconThemePath()
{
    rtl::OUString url( "$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER "/config/" );
    rtl::Bootstrap::expandMacros(url);
    return url;
}

IconThemeScanner::~IconThemeScanner()
{;}

const vcl::IconThemeInfo&
IconThemeScanner::GetIconThemeInfo(const rtl::OUString& themeId)
{
    auto info = std::find_if(mFoundIconThemes.begin(), mFoundIconThemes.end(),
            [&](const IconThemeInfo& v) {
                    return v.GetThemeId() == themeId;
                    });
    if (info == mFoundIconThemes.end()) {
        throw std::runtime_error("Requested information on not-installed icon theme");
    }
    return *info;
}


} // end namespace vcl
