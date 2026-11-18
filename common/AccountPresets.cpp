/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config.h>

#include <common/AccountPresets.hpp>
#include <common/FileUtil.hpp>
#include <common/Log.hpp>
#include <common/Uri.hpp>
#include <common/Util.hpp>

#include <Poco/URI.h>

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <map>

namespace filesystem = std::filesystem;

namespace
{
constexpr const char* stampFileName = "stamp";
// The parts of a file that is not finished yet.
constexpr const char* partialFileName = "partial";
constexpr const char* templateGroup = "template";
constexpr const char* storedLists[] = { "shared", "user" };

// The file name a settings uri gives, when a file of that name can be stored: the name of a
// template or document file.
std::optional<std::string> storedNameOfUri(const std::string& uri)
{
    std::optional<std::string> name = AccountPresets::fileNameOfUri(uri);
    if (name && UserTemplates::documentTypeOfFile(*name).empty())
        name.reset();
    return name;
}

// The entries of listed whose file can be stored, by file name. The first entry of a file name
// stands for that name.
std::map<std::string, const AccountPresets::Listed*>
listedByStoredName(const std::vector<AccountPresets::Listed>& listed)
{
    std::map<std::string, const AccountPresets::Listed*> listedByName;
    for (const AccountPresets::Listed& entry : listed)
    {
        if (const std::optional<std::string> name = storedNameOfUri(entry.uri))
            listedByName.emplace(*name, &entry);
    }
    return listedByName;
}

std::string readStamp(const filesystem::path& entryFolder)
{
    std::ifstream in(entryFolder / stampFileName, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

// How many entries the folder of a group holds.
std::size_t entryCount(const filesystem::path& groupFolder)
{
    std::error_code error;
    std::size_t count = 0;
    for (auto item = filesystem::directory_iterator(groupFolder, error);
         !error && item != filesystem::directory_iterator(); item.increment(error))
        ++count;
    return count;
}

// Writes bytes to the file at path. mode is trunc for a new file, or app to add to the file.
bool writeFile(const filesystem::path& path, const std::string& bytes, std::ios::openmode mode)
{
    std::ofstream out(path, std::ios::binary | mode);
    out.write(bytes.data(), bytes.size());
    out.close();
    return !out.fail();
}
} // namespace

AccountPresets::AccountPresets(filesystem::path accountFolder)
    : _presetsFolder(std::move(accountFolder) / "presets")
{
}

std::optional<std::string> AccountPresets::fileNameOfUri(const std::string& uri)
{
    std::string name;
    try
    {
        const Poco::URI parsed(uri);
        for (const auto& [key, value] : parsed.getQueryParameters())
        {
            if (key == "file_name")
                name = value;
        }

        if (name.empty())
        {
            // The text of the uri still has its escapes, so an escaped separator stays inside
            // the segment and the decoded name then has a separator in it.
            const std::size_t authority = uri.find("://");
            const std::size_t pathStart =
                uri.find('/', authority == std::string::npos ? 0 : authority + 3);
            if (pathStart != std::string::npos)
            {
                const std::string path =
                    uri.substr(pathStart, uri.find_first_of("?#", pathStart) - pathStart);
                name = Uri::decode(path.substr(path.find_last_of('/') + 1));
            }
        }
    }
    catch (const std::exception& exception)
    {
        LOG_WRN("AccountPresets: cannot read the file name of a settings uri: "
                << exception.what());
        return std::nullopt;
    }

    if (!FileUtil::isPlainFileName(name))
        return std::nullopt;
    return name;
}

std::optional<filesystem::path> AccountPresets::groupFolder(const std::string& list,
                                                            const std::string& group) const
{
    if (group != templateGroup ||
        std::find(std::begin(storedLists), std::end(storedLists), list) == std::end(storedLists))
        return std::nullopt;
    return _presetsFolder / list / group;
}

void AccountPresets::removeUnlisted(const std::string& list, const std::string& group,
                                    const std::vector<Listed>& listed)
{
    const std::optional<filesystem::path> folder = groupFolder(list, group);
    if (!folder)
        return;

    const std::map<std::string, const Listed*> listedByName = listedByStoredName(listed);

    std::error_code error;
    std::vector<filesystem::path> unlisted;
    for (const filesystem::directory_entry& item : filesystem::directory_iterator(*folder, error))
    {
        if (!listedByName.contains(item.path().filename().string<char>()))
            unlisted.push_back(item.path());
    }
    for (const filesystem::path& path : unlisted)
        filesystem::remove_all(path, error);
}

std::vector<std::string> AccountPresets::outdatedUris(const std::string& list,
                                                      const std::string& group,
                                                      const std::vector<Listed>& listed) const
{
    std::vector<std::string> outdated;
    const std::optional<filesystem::path> folder = groupFolder(list, group);
    if (!folder)
        return outdated;

    // A file that has no entry yet is asked for while the group has a free place.
    const std::size_t stored = entryCount(*folder);
    std::size_t freePlaces = stored < MaxFilesPerGroup ? MaxFilesPerGroup - stored : 0;

    std::error_code error;
    for (const auto& [name, entry] : listedByStoredName(listed))
    {
        if (entry->stamp.size() > MaxStampBytes)
            continue;

        const filesystem::path entryFolder = *folder / name;
        if (!filesystem::exists(entryFolder, error))
        {
            if (freePlaces == 0)
                continue;
            --freePlaces;
            outdated.push_back(entry->uri);
        }
        else if (!filesystem::is_regular_file(entryFolder / name, error) ||
                 readStamp(entryFolder) != entry->stamp)
            outdated.push_back(entry->uri);
    }
    return outdated;
}

bool AccountPresets::storeFile(const std::string& list, const std::string& group,
                               const std::string& uri, const std::string& stamp,
                               const std::string& bytes)
{
    return storeFilePart(list, group, uri, 0, bytes) && finishFile(list, group, uri, stamp);
}

bool AccountPresets::storeFilePart(const std::string& list, const std::string& group,
                                   const std::string& uri, std::size_t offset,
                                   const std::string& bytes)
{
    const std::optional<filesystem::path> folder = groupFolder(list, group);
    const std::optional<std::string> name = storedNameOfUri(uri);
    if (!folder || !name)
        return false;
    if (offset > MaxFileBytes || offset + bytes.size() > MaxFileBytes)
        return false;

    const filesystem::path entryFolder = *folder / *name;
    const filesystem::path partial = entryFolder / partialFileName;
    std::error_code error;
    if (offset != 0)
    {
        const std::uintmax_t stored = filesystem::file_size(partial, error);
        if (error || stored != offset)
            return false;
    }
    else if (!filesystem::exists(entryFolder, error) && entryCount(*folder) >= MaxFilesPerGroup)
        return false;

    filesystem::create_directories(entryFolder, error);
    if (error)
    {
        LOG_ERR("AccountPresets: could not create '" << entryFolder.string<char>()
                                                     << "': " << error.message());
        return false;
    }

    if (!writeFile(partial, bytes, offset == 0 ? std::ios::trunc : std::ios::app))
    {
        LOG_ERR("AccountPresets: could not write '" << partial.string<char>() << '\'');
        filesystem::remove(partial, error);
        return false;
    }
    return true;
}

bool AccountPresets::finishFile(const std::string& list, const std::string& group,
                                const std::string& uri, const std::string& stamp)
{
    const std::optional<filesystem::path> folder = groupFolder(list, group);
    const std::optional<std::string> name = storedNameOfUri(uri);
    if (!folder || !name)
        return false;

    const filesystem::path entryFolder = *folder / *name;
    const filesystem::path partial = entryFolder / partialFileName;
    std::error_code error;
    if (!filesystem::is_regular_file(partial, error))
        return false;
    if (stamp.size() > MaxStampBytes)
    {
        filesystem::remove(partial, error);
        return false;
    }

    // The file gets its name only when all its bytes are on disk.
    const filesystem::path target = entryFolder / *name;
    filesystem::rename(partial, target, error);
    if (error)
    {
        LOG_ERR("AccountPresets: could not rename '" << partial.string<char>()
                                                     << "': " << error.message());
        filesystem::remove(partial, error);
        return false;
    }

    if (!writeFile(entryFolder / stampFileName, stamp, std::ios::trunc))
    {
        LOG_ERR("AccountPresets: could not write the stamp of '" << target.string<char>() << '\'');
        return false;
    }

    const std::string oldPreview = UserTemplates::previewIn(entryFolder);
    if (!oldPreview.empty())
        filesystem::remove(oldPreview, error);
    UserTemplates::extractPreview(target, entryFolder);
    return true;
}

std::vector<UserTemplates::Entry> AccountPresets::templates() const
{
    std::vector<UserTemplates::Entry> entries;
    for (const char* list : storedLists)
    {
        const std::size_t first = entries.size();
        std::error_code error;
        for (const filesystem::directory_entry& item :
             filesystem::directory_iterator(_presetsFolder / list / templateGroup, error))
        {
            const std::string name = item.path().filename().string<char>();
            const filesystem::path path = item.path() / name;
            const std::string type = UserTemplates::documentTypeOfFile(name);
            if (type.empty() || item.is_symlink(error) || filesystem::is_symlink(path, error) ||
                !filesystem::is_regular_file(path, error))
                continue;

            entries.push_back({ std::string(list) + '/' + name, path.stem().string<char>(), type,
                                path.string<char>(), UserTemplates::previewIn(item.path()) });
        }

        std::sort(entries.begin() + first, entries.end(),
                  [](const UserTemplates::Entry& a, const UserTemplates::Entry& b)
                  { return Util::toLower(a.name) < Util::toLower(b.name); });
    }
    return entries;
}

void AccountPresets::removeAll()
{
    std::error_code error;
    filesystem::remove_all(_presetsFolder, error);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
