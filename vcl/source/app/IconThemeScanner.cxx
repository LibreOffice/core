/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <vcl/IconThemeScanner.hxx>

#include <config_folders.h>
#include <vcl/svapp.hxx>
#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>

namespace vcl {

namespace {
// Extension for files which contain symbol styles
rtl::OUString EXTENSION_FOR_ICON_PACKAGES(".zip");
}

IconThemeScanner::IconThemeScanner()
{;}

bool
IconThemeScanner::ScanDirectoryForIconThemes(const rtl::OUString& path)
{
    bool pathIsValid = IconThemeSearchPathIsValid(path);
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
IconThemeScanner::AddIconThemeByPath(const rtl::OUString &path)
{
    sal_Int32 slashPosition = path.lastIndexOf( '/' );
    if (slashPosition < 0) {
        return false;
    }

    rtl::OUString basename  = path.copy( slashPosition+1 );

    ThemeNameAndUrlToFile newTheme;
    newTheme.mThemeName = FileNameToIconTheme(basename);
    newTheme.mUrlToFile = path;
    mFoundIconThemes.push_back(newTheme);
    return true;
}

/*static*/ bool
IconThemeScanner::IconThemeSearchPathIsValid(const rtl::OUString& dir)
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
    sal_Int32 pos = filename.lastIndexOf(sal_Unicode('.'));
    if (pos < 0) { // -1 means index not found
        return false;
    }
    rtl::OUString suffix = filename.copy(pos);
    if (!suffix.equalsIgnoreAsciiCase(EXTENSION_FOR_ICON_PACKAGES)) {
        return false;
    }

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

/*static*/ rtl::OUString
IconThemeScanner::FileNameToIconTheme(const rtl::OUString& fname)
{
    rtl::OUString r;
    sal_Int32 positionOfLastDot = fname.lastIndexOf(sal_Unicode('.'));
    if (positionOfLastDot < 0) { // -1 means index not found
        return r;
    }
    sal_Int32 positionOfFirstUnderscore = fname.indexOf(sal_Unicode('_'));
    if (positionOfFirstUnderscore < 0) { // -1 means index not found. Use the whole name instead
        positionOfFirstUnderscore = 0;
    }
    else {
        positionOfFirstUnderscore += 1; // we do not want to capture the underscore
    }
    r = fname.copy(positionOfFirstUnderscore, positionOfLastDot - positionOfFirstUnderscore);
    return r;
}

bool
IconThemeScanner::IconThemeIsInstalled(const rtl::OUString& value) const
{
    return std::find_if(mFoundIconThemes.begin(), mFoundIconThemes.end(),
               [&](const ThemeNameAndUrlToFile& v) {
                   return v.mThemeName == value;
               }
           ) != mFoundIconThemes.end();
}

/*static*/ Size
IconThemeScanner::SizeByThemeName(const rtl::OUString& themeName)
{
    if (themeName == "tango") {
        return Size( 24, 24 );
    }
    else if (themeName == "crystal") {
        return Size( 22, 22 );
    }
    else if (themeName == "oxygen") {
        return Size( 22, 22 );
    }
    else {
        return Size( 26, 26 );
    }
}

/*static*/ std::unique_ptr<IconThemeScanner>
IconThemeScanner::Create(const rtl::OUString &path)
{
    std::unique_ptr<IconThemeScanner> retval(new IconThemeScanner());
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

} // end namespace vcl
