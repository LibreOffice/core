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

#pragma once

#include <Poco/JSON/Object.h>
#include <Poco/URI.h>

#include <chrono>
#include <cstddef>
#include <iosfwd>
#include <string>
#include <vector>

/// The slide previews of presentations, kept on disk so that they outlive the document they
/// were rendered from.
///
/// The cache directory belongs to coolwsd alone: it is outside the child roots, so no kit
/// sees it. One entry per document holds index.json, which is the preview index without the
/// parts that belong to one answer (the tag, its expiry and the picture URLs), the storage's
/// LastModifiedTime and Size the entry was made for, and one picture per slide, named by the
/// position of the slide. An entry is valid while the storage reports the same
/// LastModifiedTime and Size.
///
/// The tags handed out for cached entries live in memory, each for a ttl counted from its
/// last use. A tag stands for a CheckFileInfo that passed, so reading a picture by its tag
/// needs no access token.
///
/// Every function is safe to call from any thread.
class PreviewCache
{
public:
    /// How a picture asked for by tag was found.
    enum class Lookup
    {
        Found, ///< The picture is in the cache.
        UnknownTag, ///< No live tag of the cache has this name.
        UnknownSlide, ///< The tag is live, but its document has no slide with this GUID.
        Missing ///< The tag is live, but its entry is gone from the disk.
    };

    /// Makes the given directory the cache, with access for coolwsd alone, holding at most the
    /// given number of bytes, and each entry for the given time after its last use. When an
    /// entry is stored and the cache grows over the size, the entries used least recently are
    /// dropped. An entry past its time is dropped when it is looked up or when the cache is next
    /// swept; a zero time keeps entries until the size limit drops them. An empty path leaves
    /// the cache off.
    static void initialize(const std::string& path, std::size_t maxSizeBytes,
                           std::chrono::minutes expiry, std::chrono::seconds tagTtl);

    static bool isEnabled();

    /// The cached index of the document with the given key, when the cache holds one made for
    /// the given LastModifiedTime and Size. Null otherwise.
    static Poco::JSON::Object::Ptr lookup(const std::string& docKey,
                                          const std::string& lastModifiedTime,
                                          std::size_t size);

    /// Replaces the entry of the document with the given key. The index has the shape
    /// lookup returns, and the pictures are in the order of its slides, one per slide.
    static void store(const std::string& docKey, const std::string& lastModifiedTime,
                      std::size_t size, const Poco::JSON::Object::Ptr& index,
                      const std::vector<std::string>& pictures);

    /// Makes the given tag a name for the entry of the document with the given key, for one
    /// ttl from now, renewed on every use.
    static void registerTag(const std::string& tag, const std::string& docKey);

    /// Makes the given tag a name for the entry of the document with the given key until the
    /// given time, renewed by one ttl on every use. A tag that already had a life elsewhere
    /// keeps the expiry it had there.
    static void registerTag(const std::string& tag, const std::string& docKey,
                            std::chrono::steady_clock::time_point expiresAt);

    /// Reads the picture of one slide of the entry the given tag names.
    static Lookup readPicture(const std::string& tag, const std::string& guid, std::string& png);

    /// The seconds before the given tag expires, zero when it is not live.
    static long remainingTtlSeconds(const std::string& tag);

    /// The answer to an index request: the index with the version, the tag, its expiry and
    /// the URL of every picture added. The document URL names the document without any query.
    static std::string buildAnswer(const Poco::JSON::Object::Ptr& index, const std::string& tag,
                                   long ttlSecs, const Poco::URI& wopiSrc);

    /// The URL a picture is read from: a path from the service root.
    static std::string buildContentUrl(const std::string& tag, const Poco::URI& wopiSrc,
                                       const std::string& guid);

    static void dumpState(std::ostream& os);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
