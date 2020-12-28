/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/DirectoryHelper.hxx>

#include <sal/config.h>
#include <osl/file.hxx>

#include <memory>

namespace comphelper
{
typedef std::shared_ptr<osl::File> FileSharedPtr;

OUString DirectoryHelper::splitAtLastToken(const OUString& rSrc, sal_Unicode aToken,
                                           OUString& rRight)
{
    const sal_Int32 nIndex(rSrc.lastIndexOf(aToken));
    OUString aRetval;

    if (-1 == nIndex)
    {
        aRetval = rSrc;
        rRight.clear();
    }
    else if (nIndex > 0)
    {
        aRetval = rSrc.copy(0, nIndex);

        if (rSrc.getLength() > nIndex + 1)
        {
            rRight = rSrc.copy(nIndex + 1);
        }
    }

    return aRetval;
}

bool DirectoryHelper::fileExists(const OUString& rBaseURL)
{
    if (!rBaseURL.isEmpty())
    {
        FileSharedPtr aBaseFile = std::make_shared<osl::File>(rBaseURL);

        return (osl::File::E_None == aBaseFile->open(osl_File_OpenFlag_Read));
    }

    return false;
}

bool DirectoryHelper::dirExists(const OUString& rDirURL)
{
    if (!rDirURL.isEmpty())
    {
        osl::Directory aDirectory(rDirURL);

        return (osl::FileBase::E_None == aDirectory.open());
    }

    return false;
}

void DirectoryHelper::scanDirsAndFiles(const OUString& rDirURL, std::set<OUString>& rDirs,
                                       std::set<std::pair<OUString, OUString>>& rFiles)
{
    if (rDirURL.isEmpty())
        return;

    osl::Directory aDirectory(rDirURL);

    if (osl::FileBase::E_None != aDirectory.open())
        return;

    osl::DirectoryItem aDirectoryItem;

    while (osl::FileBase::E_None == aDirectory.getNextItem(aDirectoryItem))
    {
        osl::FileStatus aFileStatus(osl_FileStatus_Mask_Type | osl_FileStatus_Mask_FileURL
                                    | osl_FileStatus_Mask_FileName);

        if (osl::FileBase::E_None == aDirectoryItem.getFileStatus(aFileStatus))
        {
            if (aFileStatus.isDirectory())
            {
                const OUString aFileName(aFileStatus.getFileName());

                if (!aFileName.isEmpty())
                {
                    rDirs.insert(aFileName);
                }
            }
            else if (aFileStatus.isRegular())
            {
                OUString aFileName(aFileStatus.getFileName());
                OUString aExtension;
                aFileName = splitAtLastToken(aFileName, '.', aExtension);

                if (!aFileName.isEmpty())
                {
                    rFiles.insert(std::pair<OUString, OUString>(aFileName, aExtension));
                }
            }
        }
    }
}

bool DirectoryHelper::deleteDirRecursively(const OUString& rDirURL)
{
    std::set<OUString> aDirs;
    std::set<std::pair<OUString, OUString>> aFiles;
    bool bError(false);

    scanDirsAndFiles(rDirURL, aDirs, aFiles);

    for (const auto& dir : aDirs)
    {
        const OUString aNewDirURL(rDirURL + "/" + dir);

        bError |= deleteDirRecursively(aNewDirURL);
    }

    for (const auto& file : aFiles)
    {
        OUString aNewFileURL(rDirURL + "/" + file.first);

        if (!file.second.isEmpty())
        {
            aNewFileURL += "." + file.second;
        }
        bError |= (osl::FileBase::E_None != osl::File::remove(aNewFileURL));
    }

    bError |= (osl::FileBase::E_None != osl::Directory::remove(rDirURL));

    return bError;
}

// both exist, move content
bool DirectoryHelper::moveDirContent(const OUString& rSourceDirURL,
                                     std::u16string_view rTargetDirURL,
                                     const std::set<OUString>& rExcludeList)
{
    std::set<OUString> aDirs;
    std::set<std::pair<OUString, OUString>> aFiles;
    bool bError(false);

    scanDirsAndFiles(rSourceDirURL, aDirs, aFiles);

    for (const auto& dir : aDirs)
    {
        const bool bExcluded(!rExcludeList.empty() && rExcludeList.find(dir) != rExcludeList.end());

        if (!bExcluded)
        {
            const OUString aNewSourceDirURL(rSourceDirURL + "/" + dir);

            if (dirExists(aNewSourceDirURL))
            {
                const OUString aNewTargetDirURL(OUString::Concat(rTargetDirURL) + "/" + dir);

                if (dirExists(aNewTargetDirURL))
                {
                    deleteDirRecursively(aNewTargetDirURL);
                }

                bError |= (osl::FileBase::E_None
                           != osl::File::move(aNewSourceDirURL, aNewTargetDirURL));
            }
        }
    }

    for (const auto& file : aFiles)
    {
        OUString aSourceFileURL(rSourceDirURL + "/" + file.first);

        if (!file.second.isEmpty())
        {
            aSourceFileURL += "." + file.second;
        }

        if (fileExists(aSourceFileURL))
        {
            OUString aTargetFileURL(OUString::Concat(rTargetDirURL) + "/" + file.first);

            if (!file.second.isEmpty())
            {
                aTargetFileURL += "." + file.second;
            }

            if (fileExists(aTargetFileURL))
            {
                osl::File::remove(aTargetFileURL);
            }

            bError |= (osl::FileBase::E_None != osl::File::move(aSourceFileURL, aTargetFileURL));
        }
    }

    return bError;
}
}