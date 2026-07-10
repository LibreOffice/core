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
 * A short-lived store of documents uploaded to the Model Context Protocol endpoint. A client with
 * shell access can send the bytes of a document once and then name it by identifier in one or more
 * tool calls. Each entry carries the time it stops being valid. An entry whose time has passed is
 * unknown to a lookup, and it is removed from disk the next time the store is used.
 */

#pragma once

#include <config.h>

#include <chrono>
#include <cstddef>
#include <functional>
#include <istream>
#include <map>
#include <mutex>
#include <string>

class McpUploads
{
public:
    using Clock = std::chrono::steady_clock;

    /// How long an uploaded document stays available before it is removed.
    static constexpr std::chrono::seconds TimeToLive{ 900 };

    /// The most bytes the store holds at one time, over every stored document.
    static constexpr std::size_t MaxStoredBytes = std::size_t(2) * 1024 * 1024 * 1024;

    /// The most documents the store holds at one time. The documents being written count. The
    /// number allows about 18 new documents a second over the 15 minutes each one stays.
    static constexpr std::size_t MaxStoredEntries = 16384;

    /// The time every upload is given to arrive whole, on top of the time its size needs.
    static constexpr std::chrono::seconds MinimumUploadTime{ 60 };

    /// The slowest rate, in bytes a second, at which an upload still arrives in the time it is
    /// given.
    static constexpr std::size_t MinimumUploadBytesPerSecond = 64 * 1024;

    /// The time an upload of size bytes is given to arrive whole: MinimumUploadTime and the time
    /// size bytes take at MinimumUploadBytesPerSecond.
    static constexpr std::chrono::seconds uploadTimeLimit(std::size_t size)
    {
        return MinimumUploadTime + std::chrono::seconds(size / MinimumUploadBytesPerSecond);
    }

    /// The one store for the process.
    static McpUploads& instance();

    /// Store the given bytes as an uploaded document named after filename and return the identifier
    /// a later tool call uses to name it. Returns an empty string when the document cannot be
    /// stored, which includes when it would take the store past MaxStoredBytes. now is the current
    /// time, taken as an argument so a test can control it.
    std::string store(const std::string& filename, const std::string& bytes,
                      Clock::time_point now = Clock::now());

    /// Store the next size bytes of source in the same way. Returns an empty string when source
    /// ends before size bytes, as well as in the cases above.
    std::string store(const std::string& filename, std::istream& source, std::size_t size,
                      Clock::time_point now = Clock::now());

    /// True when a document of size bytes fits in the store as it is now, after the documents whose
    /// time has passed are removed, both in bytes and in the number of documents. now is the
    /// current time, taken as an argument so a test can control it.
    bool hasRoomFor(std::size_t size, Clock::time_point now = Clock::now());

    /// Count size bytes and one document against the store for a document about to be written, so
    /// the documents written at the same time together stay within MaxStoredBytes and
    /// MaxStoredEntries. Returns false when the store has no room. now is the current time, taken
    /// as an argument so a test can control it.
    bool reserve(std::size_t size, Clock::time_point now = Clock::now());

    /// Give back the size bytes and the document that reserve counted for a document that was not
    /// stored.
    void release(std::size_t size);

    /// Enter the document written at path, whose size bytes and place reserve already counted, and
    /// return the identifier used to name it later. The store owns the file from then on.
    std::string adopt(const std::string& path, std::size_t size,
                      Clock::time_point now = Clock::now());

    /// Return the stored path for id, or an empty string when id is unknown or its time has passed.
    /// Entries whose time has passed are removed as they are found. now is the current time, taken
    /// as an argument so a test can control it.
    std::string lookup(const std::string& id, Clock::time_point now = Clock::now());

    /// Remove every document whose time has passed. Does nothing when another call holds the store,
    /// since the next call removes them as well. now is the current time, taken as an argument so a
    /// test can control it.
    void purgeExpiredEntries(Clock::time_point now = Clock::now());

private:
    struct Entry
    {
        std::string path;
        Clock::time_point expiry;
        /// The size of the stored document in bytes.
        std::size_t size = 0;
    };

    /// Count size bytes against the store, write the document with write, which returns its path or
    /// an empty string on failure, and enter it under a new identifier. Returns the identifier, or
    /// an empty string when the store has no room or the write fails.
    std::string storeWith(std::size_t size, const std::function<std::string()>& write,
                          Clock::time_point now);

    /// Remove every entry whose time has passed, deleting each one's file and the directory that
    /// held it. The caller holds the lock.
    void purgeExpired(Clock::time_point now);

    /// True when a document of size bytes fits next to the stored documents and the documents being
    /// written. The caller holds the lock.
    bool hasRoom(std::size_t size) const;

    std::mutex _mutex;
    std::map<std::string, Entry> _entries;
    /// The sum of the sizes of the stored documents and of the documents being written.
    std::size_t _storedBytes = 0;
    /// The number of documents being written: those reserve counted that are not yet adopted or
    /// released.
    std::size_t _pendingEntries = 0;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
