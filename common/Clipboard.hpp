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

// Clipboard abstraction.

#pragma once

#include <common/ClipboardData.hpp>
#include <common/Common.hpp>
#include <common/ContainerUtil.hpp>
#include <common/FileUtil.hpp>
#include <common/JailUtil.hpp>
#include <common/Log.hpp>
#include <common/Protocol.hpp>
#include <common/Util.hpp>
#include <wsd/COOLWSD.hpp>

#include <cstdlib>
#include <mutex>
#include <string>
#include <vector>

/// Used to store expired view's clipboards
class ClipboardCache
{
    /// Check and expire clipboard-cache entries every 1/10th the lifetime of each.
    static constexpr std::chrono::seconds ExpiryCheckPeriod{ CLIPBOARD_EXPIRY_MINUTES * 60 / 10 };

    std::mutex _mutex;
    struct Entry {
        std::chrono::steady_clock::time_point _inserted;
        std::shared_ptr<FileUtil::OwnedFile> _cacheFile;  // cached clipboard

        bool hasExpired(const std::chrono::steady_clock::time_point now) const
        {
            return (now - _inserted) >= std::chrono::minutes(CLIPBOARD_EXPIRY_MINUTES);
        }
    };
    // clipboard key -> data
    Util::UnorderedStringMap<Entry> _cache;
    std::string _cacheDir;
    std::chrono::steady_clock::time_point _nextExpiryTime;
    int _cacheFileId;
public:
    ClipboardCache()
        : _cacheDir(FileUtil::createRandomTmpDir(
              COOLWSD::ChildRoot + JailUtil::CHILDROOT_TMP_INCOMING_PATH + "/clipboards"))
        , _nextExpiryTime(std::chrono::steady_clock::now() + ExpiryCheckPeriod)
        , _cacheFileId(0)
    {
    }

    ~ClipboardCache()
    {
        FileUtil::removeFile(_cacheDir, true);
    }

    void dumpState(std::ostream& os) const
    {
        os << "Saved clipboards: " << _cache.size() << '\n';
        size_t totalSize = 0;
        auto now = std::chrono::steady_clock::now();
        for (const auto &it : _cache)
        {
            const std::string& cacheFile = it.second._cacheFile->_file;

            size_t size = FileUtil::Stat(cacheFile).size();
            os << "  size: " << size << " bytes, lifetime: " <<
                std::chrono::duration_cast<std::chrono::seconds>(
                    now - it.second._inserted).count() << " seconds\n";
            os << "  file: " << cacheFile << "\n";
            totalSize += size;
        }

        os << "Saved clipboard total size: " << totalSize << " bytes (disk)\n";
    }

    std::string nextClipFileName()
    {
        return _cacheDir + '/' + std::to_string(_cacheFileId++);
    }

    std::string insertClipboard(const std::string key[2],
                         const std::string& clipFile)
    {
        Entry ent;
        ent._inserted = std::chrono::steady_clock::now();

        std::string cacheFile = nextClipFileName();
        if (::rename(clipFile.c_str(), cacheFile.c_str()) < 0)
        {
            LOG_SYS("Failed to rename [" << clipFile << "] to [" << cacheFile << ']');
            return clipFile;
        }

        ent._cacheFile = std::make_shared<FileUtil::OwnedFile>(cacheFile);
        LOG_TRC("Insert cached clipboard: " << key[0] << " and " << key[1] << ", clip file of: " << cacheFile);
        std::lock_guard<std::mutex> lock(_mutex);
        _cache[key[0]] = _cache[key[1]] = std::move(ent);
        return cacheFile;
    }

    std::shared_ptr<FileUtil::OwnedFile> getClipboard(const std::string &key)
    {
        LOG_TRC("Looking up cached clipboard with key [" << key << ']');

        std::lock_guard<std::mutex> lock(_mutex);
        const auto it = _cache.find(key);
        if (it == _cache.end())
        {
            LOG_TRC("Clipboard key [" << key << "] is not present");
            return nullptr;
        }
        else if (it->second.hasExpired(std::chrono::steady_clock::now()))
        {
            LOG_TRC("Clipboard item with key [" << key << "] is expired");
            return nullptr;
        }

        return it->second._cacheFile;
    }

    void checkexpiry(std::chrono::steady_clock::time_point now)
    {
        if (now < _nextExpiryTime)
        {
            return;
        }

        std::unique_lock<std::mutex> lock(_mutex, std::defer_lock);
        if (!lock.try_lock())
        {
            LOG_DBG("Clipboard cache expiry check was due, but the lock is taken.");
            return;
        }

        LOG_TRC("check expiry of cached clipboards");
        for (auto it = _cache.begin(); it != _cache.end();)
        {
            if (it->second.hasExpired(now))
            {
                LOG_TRC("Expiring cached clipboard entry: " << it->first);
                it = _cache.erase(it);
            }
            else
                ++it;
        }

        _nextExpiryTime = now + ExpiryCheckPeriod;
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
