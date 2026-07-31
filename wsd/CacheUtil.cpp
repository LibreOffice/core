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

/*
 * Implementation of cache utilities.
 * Functions: getCacheDir(), clearCacheDir()
 */

#include <config.h>

#include "CacheUtil.hpp"

#include <common/Common.hpp>
#include <common/ConfigUtil.hpp>
#include <common/FileUtil.hpp>
#include <common/Log.hpp>
#include <common/StringVector.hpp>
#include <common/Uri.hpp>
#include <common/Util.hpp>

#include <Poco/Path.h>
#include <Poco/URI.h>

#include <chrono>
#include <fstream>
#include <mutex>
#include <set>

static std::mutex CacheMutex;
static std::string CachePath;
static std::chrono::minutes MaxAgeMins;

void Cache::initialize(const std::string& path)
{
    if (!CachePath.empty())
        return;

    LOG_INF("Initializing Cache at [" << path << "]");

    // Make sure the cache directories exists, or we throw if we can't create it.
    Poco::File(path).createDirectories();

    // We are initialized at this point.
    CachePath = path;

    MaxAgeMins = ConfigUtil::getConfigValue<std::chrono::minutes>("cache_files.expiry_min", 3000);

    clearOutdatedConfigs();
}

std::string Cache::getConfigId(const std::string& uri)
{
    const Poco::URI settingsUri(uri);
    std::string sourcePrefix;
    sourcePrefix.reserve(256);
    sourcePrefix.append(settingsUri.getScheme());
    sourcePrefix.push_back('_');
    sourcePrefix.append(settingsUri.getAuthority());
    sourcePrefix.append(settingsUri.getPath());
    return sourcePrefix;
}

void Cache::cacheConfigFile(const std::string& configId, const std::string& uri,
                            const std::string& stamp, const std::string& filename)
{
    if (CachePath.empty())
        return;

    try
    {
        std::unique_lock<std::mutex> lock(CacheMutex);

        Poco::Path rootPath(CachePath, Uri::encode(configId));
        rootPath.makeDirectory();

        Poco::Path cachePath(rootPath, Uri::encode(uri));
        cachePath.makeDirectory();

        Poco::File(cachePath).createDirectories();

        std::string cacheFileDir = cachePath.toString();
        std::string cacheFile = Poco::Path(cacheFileDir, "contents").toString();

        FileUtil::copyFileTo(filename, cacheFile);

        std::ofstream cacheStamp(Poco::Path(cacheFileDir, "stamp").toString());
        cacheStamp << stamp << '\n';
        cacheStamp.close();

        updateLastUsed(rootPath.toString());

        clearOutdatedConfigs();
    }
    catch (const std::exception& exc)
    {
        LOG_ERR("Error while caching: " << filename << " of: " << exc.what());
    }
}

void Cache::updateLastUsed(const std::string& path)
{
    std::string cacheLastUsed = Poco::Path(path, "lastused").toString();
    std::ofstream of(cacheLastUsed);
    of << std::chrono::system_clock::now() << '\n';
    of.close();
}

bool Cache::supplyConfigFile(const std::string& cacheDir, const std::string& stamp,
                             const std::string& dest)
{
    std::string cacheContentsFile = Poco::Path(cacheDir, "contents").toString();
    if (!FileUtil::Stat(cacheContentsFile).exists())
    {
        LOG_DBG("cacheFile: " << cacheContentsFile << " cache miss.");
        return false;
    }

    std::string cacheStampFile = Poco::Path(cacheDir, "stamp").toString();
    std::string cacheStamp;
    std::ifstream ifs(cacheStampFile);
    std::getline(ifs, cacheStamp);
    if (cacheStamp.empty() || stamp != cacheStamp)
    {
        LOG_DBG("cacheFile: " << cacheStampFile << " with obsolete stamp: " << cacheStamp << " vs: " << stamp);
        return false;
    }
    if (!FileUtil::copy(cacheContentsFile, dest, false, false))
    {
        LOG_WRN("cacheFile: " << cacheContentsFile << " couldn't be copied");
        return false;
    }
    LOG_DBG("cacheFile: " << cacheContentsFile << " supplied");
    return true;
}

void Cache::supplyConfigFiles(const std::string& configId, std::vector<CacheQuery>& queries)
{
    if (CachePath.empty())
        return;

    std::unique_lock<std::mutex> lock(CacheMutex);

    Poco::Path rootPath(CachePath, Uri::encode(configId));
    rootPath.makeDirectory();
    Poco::File(rootPath).createDirectories();

    std::set<std::string> cacheHits;

    auto it = queries.begin();
    while (it != queries.end())
    {
        std::string cachename = Uri::encode(it->_uri);

        Poco::Path cacheDir(rootPath, cachename);

        if (supplyConfigFile(cacheDir.toString(), it->_stamp, it->_dest))
        {
            it = queries.erase(it);
            cacheHits.insert(std::move(cachename));
        }
        else
            ++it;
    }

    auto names = FileUtil::getDirEntries(rootPath.toString());
    for (const auto& name : names)
    {
        if (name == "lastused")
            continue;
        if (cacheHits.find(name) == cacheHits.end())
        {
            LOG_INF("cacheFile: " "removing stale cache file: " << name);
            Poco::Path cacheDir(rootPath, name);
            FileUtil::removeFile(cacheDir.toString(), true);
        }
    }

    if (!cacheHits.empty())
        updateLastUsed(rootPath.toString());

    clearOutdatedConfigs();
}

void Cache::clearOutdatedConfigs()
{
    if (CachePath.empty())
        return;

    auto now = std::chrono::system_clock::now();

    auto names = FileUtil::getDirEntries(CachePath);
    for (const auto& name : names)
    {
        Poco::Path rootPath(CachePath, name);
        rootPath.makeDirectory();

        std::string timeStampFile = Poco::Path(rootPath, "lastused").toString();

        std::string timeStamp;
        std::ifstream ifs(timeStampFile);
        std::getline(ifs, timeStamp);

        if (!timeStamp.empty())
        {
            auto t = Util::iso8601ToTimestamp(timeStamp, "cacheFile: ");
            auto age = now - t;
            if (age < MaxAgeMins)
                continue;
        }

        LOG_INF("cacheFile: " "removing stale cache dir: " << rootPath.toString());
        FileUtil::removeFile(rootPath.toString(), true);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
