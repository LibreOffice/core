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

#include <common/DraftStore.hpp>

#include <common/FileUtil.hpp>
#include <common/Log.hpp>

#include <Poco/Path.h>

#include <cerrno>
#include <fcntl.h>
#include <filesystem>

namespace drafts
{
namespace
{
std::string Directory;
std::string MarkedDirectory;

constexpr int MaxNumber = 100000;
constexpr int MaxCreateAttempts = 100;

std::string fileNameOf(const std::string& path) { return Poco::Path(path).getFileName(); }

std::string markerPath(const std::string& name) { return MarkedDirectory + '/' + name; }

bool isMarkedName(const std::string& name)
{
    return FileUtil::Stat(markerPath(name), /*link=*/true).exists();
}

bool isTaken(const std::string& name)
{
    return FileUtil::Stat(Directory + '/' + name, /*link=*/true).exists() || isMarkedName(name);
}

bool removeIfPresent(const std::string& path)
{
    return FileUtil::unlinkFile(path) == 0 || errno == ENOENT;
}

// The engine saves through a "lu<pid>...tmp" file next to the document.
bool isEngineTemporary(const std::string& name)
{
    return name.starts_with("lu") && name.ends_with(".tmp");
}

// A copyAtomic temporary is the final name plus 12 random characters, which no real extension
// is as long as.
bool isCopyTemporary(const std::string& name)
{
    const std::size_t dot = name.rfind('.');
    return dot != std::string::npos && name.size() - dot - 1 > 10;
}
} // namespace

bool initialize(const std::string& appDataDir)
{
    Directory = appDataDir + "/drafts";
    MarkedDirectory = Directory + "/.marked";
    try
    {
        FileUtil::createDirectories(MarkedDirectory);
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        LOG_ERR("Drafts: cannot create [" << MarkedDirectory << "]: " << e.what());
        return false;
    }
    return true;
}

const std::string& directory() { return Directory; }

std::string nextFreeName(const std::string& stem, const std::string& extension)
{
    std::string name = stem + '.' + extension;
    if (!FileUtil::isPlainFileName(name))
        return {};

    for (int number = 1; isTaken(name); ++number)
    {
        if (number > MaxNumber)
            return {};
        name = stem + " (" + std::to_string(number) + ")." + extension;
    }
    return Directory + '/' + name;
}

std::string createFromTemplate(const std::string& sourcePath, const std::string& stem,
                               const std::string& extension)
{
    for (int attempt = 0; attempt < MaxCreateAttempts; ++attempt)
    {
        const std::string path = nextFreeName(stem, extension);
        if (path.empty())
        {
            LOG_ERR("Drafts: no free name for [" << stem << '.' << extension << ']');
            return {};
        }

        // Claim the name first: copy() would replace a file made meanwhile.
        const int fd = FileUtil::openFileAsFD(path, O_CREAT | O_EXCL | O_WRONLY, 0666);
        if (fd < 0)
        {
            if (errno == EEXIST)
                continue;
            LOG_SYS("Drafts: cannot create [" << path << ']');
            return {};
        }
        FileUtil::closeFD(fd);

        if (FileUtil::copy(sourcePath, path, /*log=*/false, /*throw_on_error=*/false))
            return path;

        LOG_ERR("Drafts: cannot copy [" << sourcePath << "] to [" << path << ']');
        removeIfPresent(path);
        return {};
    }
    LOG_ERR("Drafts: [" << stem << '.' << extension << "] names kept appearing meanwhile");
    return {};
}

void markForDeletion(const std::string& path)
{
    const std::string name = fileNameOf(path);
    if (!FileUtil::isPlainFileName(name))
        return;
    const int fd = FileUtil::openFileAsFD(markerPath(name), O_CREAT | O_WRONLY, 0600);
    if (fd < 0)
    {
        LOG_SYS("Drafts: cannot mark [" << name << "] for deletion");
        return;
    }
    FileUtil::closeFD(fd);
}

void unmarkForDeletion(const std::string& path)
{
    const std::string name = fileNameOf(path);
    if (FileUtil::isPlainFileName(name) && !removeIfPresent(markerPath(name)))
        LOG_SYS("Drafts: cannot unmark [" << name << ']');
}

bool isMarkedForDeletion(const std::string& path)
{
    const std::string name = fileNameOf(path);
    return FileUtil::isPlainFileName(name) && isMarkedName(name);
}

bool discard(const std::string& path)
{
    markForDeletion(path);
    if (removeIfPresent(path))
        return true;
    LOG_DBG("Drafts: [" << path << "] stays until the next start");
    return false;
}

void deleteMarked()
{
    for (const std::string& name : FileUtil::getDirEntries(MarkedDirectory))
    {
        if (removeIfPresent(Directory + '/' + name))
            removeIfPresent(markerPath(name));
        else
            LOG_WRN_SYS("Drafts: cannot delete [" << name << "], trying again at the next start");
    }
}

std::vector<std::string> leftovers()
{
    std::vector<std::string> names;
    for (std::string& name : FileUtil::getDirEntries(Directory))
    {
        if (isEngineTemporary(name) || isCopyTemporary(name) || isMarkedName(name))
            continue;
        if (!FileUtil::Stat(Directory + '/' + name, /*link=*/true).isFile())
            continue;
        names.push_back(std::move(name));
    }
    std::sort(names.begin(), names.end());

    std::vector<std::string> paths;
    paths.reserve(names.size());
    for (const std::string& name : names)
        paths.push_back(Directory + '/' + name);
    return paths;
}

bool contains(const std::string& path)
{
    const std::string folder = FileUtil::hostPath(Directory) + '/';
    return FileUtil::hostPathOfTarget(path).starts_with(folder);
}
} // namespace drafts

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
