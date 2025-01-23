/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/UserResourceScanner.hxx>

#include <sal/config.h>
#include <sal/log.hxx>

#include <salhelper/linkhelper.hxx>
#include <unotools/pathoptions.hxx>
#include <o3tl/string_view.hxx>

#include <deque>

namespace vcl
{
namespace
{
OUString convertToAbsolutePath(const OUString& path)
{
    salhelper::LinkResolver resolver(0);
    osl::FileBase::RC rc = resolver.fetchFileStatus(path);
    if (rc != osl::FileBase::E_None)
    {
        SAL_WARN("vcl.app", "Could not resolve path '" << path << "' to search for icon themes.");
        if (rc == osl::FileBase::E_MULTIHOP)
        {
            throw std::runtime_error("Provided a recursive symlink to an icon theme directory that "
                                     "could not be resolved.");
        }
    }
    return resolver.m_aStatus.getFileURL();
}
}

namespace file
{
// read the status of a file. Returns false if the status could not be determined.
bool readFileStatus(osl::FileStatus& status, const OUString& file)
{
    osl::DirectoryItem dirItem;
    osl::FileBase::RC retvalGet = osl::DirectoryItem::get(file, dirItem);
    if (retvalGet != osl::FileBase::E_None)
    {
        SAL_WARN("vcl.app", "Could not determine status for file '" << file << "'.");
        return false;
    }
    osl::FileBase::RC retvalStatus = dirItem.getFileStatus(status);
    if (retvalStatus != osl::FileBase::E_None)
    {
        SAL_WARN("vcl.app", "Could not determine status for file '" << file << "'.");
        return false;
    }
    return true;
}

void splitPathString(std::u16string_view aPathString, std::deque<OUString>& rPaths)
{
    sal_Int32 nIndex = 0;
    do
    {
        rPaths.push_front(OUString(o3tl::getToken(aPathString, 0, ';', nIndex)));
    } while (nIndex >= 0);
}
}

UserResourceScanner::UserResourceScanner() = default;

void UserResourceScanner::addPaths(std::u16string_view aPathString)
{
    std::deque<OUString> aPaths;
    vcl::file::splitPathString(aPathString, aPaths);

    for (const auto& path : aPaths)
    {
        osl::FileStatus aFileStatus(osl_FileStatus_Mask_Type);

        if (!vcl::file::readFileStatus(aFileStatus, path))
            continue;

        if (!aFileStatus.isDirectory())
        {
            SAL_INFO("vcl.app", "Can not search for resource files in '"
                                    << path << "'. It is not a directory.");
            continue;
        }

        std::vector<OUString> aResourcePaths = readFilesFromPath(path);

        if (aResourcePaths.empty())
        {
            SAL_WARN("vcl.app",
                     "Could not find any file in the provided directory ('" << path << "'.");
            continue;
        }

        for (auto const& iconThemePath : aResourcePaths)
            addResource(iconThemePath);
    }
}

std::vector<OUString> UserResourceScanner::readFilesFromPath(const OUString& dir)
{
    std::vector<OUString> found;
    SAL_INFO("vcl", "Scanning directory '" << dir << " for potential resource files.");

    osl::Directory dirToScan(dir);
    osl::FileBase::RC retvalOpen = dirToScan.open();
    if (retvalOpen != osl::FileBase::E_None)
        return found;

    osl::DirectoryItem directoryItem;
    while (dirToScan.getNextItem(directoryItem) == osl::FileBase::E_None)
    {
        osl::FileStatus status(osl_FileStatus_Mask_Type | osl_FileStatus_Mask_FileURL
                               | osl_FileStatus_Mask_FileName);
        osl::FileBase::RC retvalStatus = directoryItem.getFileStatus(status);
        if (retvalStatus != osl::FileBase::E_None)
            continue;

        OUString filename = convertToAbsolutePath(status.getFileURL());
        if (isValidResource(filename))
            found.push_back(filename);
    }

    return found;
}

} // end namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
