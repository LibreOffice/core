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

#include <wsd/McpUploads.hpp>

#include <wsd/McpTools.hpp>

#include <common/FileUtil.hpp>
#include <common/Log.hpp>
#include <common/Util.hpp>

#include <algorithm>
#include <functional>
#include <istream>
#include <string>

McpUploads& McpUploads::instance()
{
    static McpUploads uploads;
    return uploads;
}

std::string McpUploads::store(const std::string& filename, const std::string& bytes,
                              Clock::time_point now)
{
    return storeWith(bytes.size(),
                     [&filename, &bytes]()
                     { return McpTools::writeIncomingDocument(filename, bytes); },
                     now);
}

std::string McpUploads::store(const std::string& filename, std::istream& source, std::size_t size,
                              Clock::time_point now)
{
    return storeWith(size,
                     [&filename, &source, size]()
                     { return McpTools::writeIncomingDocument(filename, source, size); },
                     now);
}

std::string McpUploads::storeWith(std::size_t size, const std::function<std::string()>& write,
                                  Clock::time_point now)
{
    // The size is counted against the store while the file is written. The lock is held only to
    // count the size and to enter the document, so a lookup goes ahead during the write.
    if (!reserve(size, now))
        return std::string();

    const std::string path = write();
    if (path.empty())
    {
        release(size);
        return std::string();
    }

    return adopt(path, size, now);
}

bool McpUploads::reserve(std::size_t size, Clock::time_point now)
{
    std::lock_guard<std::mutex> lock(_mutex);
    purgeExpired(now);
    if (!hasRoom(size))
    {
        LOG_WRN("MCP: the store is full, a document of " << size << " bytes is refused");
        return false;
    }
    _storedBytes += size;
    ++_pendingEntries;
    return true;
}

void McpUploads::release(std::size_t size)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _storedBytes -= std::min(size, _storedBytes);
    if (_pendingEntries > 0)
        --_pendingEntries;
}

std::string McpUploads::adopt(const std::string& path, std::size_t size, Clock::time_point now)
{
    const std::string id = Util::rng::getHexString(32);
    std::lock_guard<std::mutex> lock(_mutex);
    if (_pendingEntries > 0)
        --_pendingEntries;
    _entries[id] = Entry{ path, now + TimeToLive, size };
    LOG_TRC("MCP: stored upload [" << id << "] at [" << path << ']');
    return id;
}

std::string McpUploads::lookup(const std::string& id, Clock::time_point now)
{
    std::lock_guard<std::mutex> lock(_mutex);
    purgeExpired(now);

    const auto it = _entries.find(id);
    if (it == _entries.end())
        return std::string();
    return it->second.path;
}

bool McpUploads::hasRoomFor(std::size_t size, Clock::time_point now)
{
    std::lock_guard<std::mutex> lock(_mutex);
    purgeExpired(now);
    return hasRoom(size);
}

bool McpUploads::hasRoom(std::size_t size) const
{
    return size <= MaxStoredBytes && _storedBytes + size <= MaxStoredBytes &&
           _entries.size() + _pendingEntries < MaxStoredEntries;
}

void McpUploads::purgeExpiredEntries(Clock::time_point now)
{
    std::unique_lock<std::mutex> lock(_mutex, std::try_to_lock);
    if (lock.owns_lock())
        purgeExpired(now);
}

void McpUploads::purgeExpired(Clock::time_point now)
{
    for (auto it = _entries.begin(); it != _entries.end();)
    {
        if (it->second.expiry <= now)
        {
            FileUtil::removeFileAndEmptyParent(it->second.path);
            _storedBytes -= it->second.size;
            it = _entries.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
