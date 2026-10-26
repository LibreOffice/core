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

#include "PreviewCache.hpp"

#include <common/FileUtil.hpp>
#include <common/JsonUtil.hpp>
#include <common/Log.hpp>
#include <common/Uri.hpp>
#include <common/Util.hpp>
#include <wsd/COOLWSD.hpp>

#include <Poco/DigestEngine.h>
#include <Poco/File.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Path.h>
#include <Poco/SHA2Engine.h>

#include <sys/stat.h>

#include <algorithm>
#include <fstream>
#include <map>
#include <mutex>
#include <ostream>
#include <sstream>
#include <vector>

namespace
{
/// The largest index or picture file read back from the cache.
constexpr int MaxCachedFileSize = 64 * 1024 * 1024;

/// How often a lookup sweeps the entries past their time, so they leave the disk while no
/// store happens.
constexpr std::chrono::minutes SweepPeriod{ 15 };

/// A tag handed out for a cached entry.
struct Tag
{
    std::string docKey;
    std::chrono::steady_clock::time_point expiresAt;
    /// The position of every slide in the entry, by GUID. Empty until the first picture is
    /// read through the tag.
    std::map<std::string, std::size_t> slides;
};

std::mutex Mutex;
std::string CachePath;
std::size_t MaxSizeBytes = 0;
/// How long an entry is kept after its last use. Zero keeps entries until the size limit drops
/// them.
std::chrono::minutes Expiry{ 0 };
std::chrono::seconds TagTtl{ 0 };
std::chrono::steady_clock::time_point LastSweep;
std::map<std::string, Tag> Tags;

/// The directory of the entry of a document: the SHA-256 of its key, so that the name of the
/// document never becomes a path.
std::string entryPath(const std::string& docKey)
{
    Poco::SHA2Engine engine(Poco::SHA2Engine::SHA_256);
    engine.update(docKey);
    return Poco::Path(CachePath, Poco::DigestEngine::digestToHex(engine.digest())).toString();
}

std::string picturePath(const std::string& entry, std::size_t position)
{
    return Poco::Path(entry, std::to_string(position) + ".png").toString();
}

bool writeFile(const std::string& path, const std::string& data)
{
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    file.write(data.data(), static_cast<std::streamsize>(data.size()));
    file.close();
    return !file.fail();
}

void updateLastUsed(const std::string& entry)
{
    writeFile(Poco::Path(entry, "lastused").toString(),
              Util::getIso8601FracformatTime(std::chrono::system_clock::now()) + '\n');
}

/// The time an entry was last used. The start of time for an entry without a stamp, so it is
/// the oldest of all.
std::chrono::system_clock::time_point lastUsedOf(const std::string& entry)
{
    std::string stamp;
    std::ifstream file(Poco::Path(entry, "lastused").toString());
    std::getline(file, stamp);
    return stamp.empty() ? std::chrono::system_clock::time_point()
                         : Util::iso8601ToTimestamp(stamp, "PreviewCache: ");
}

/// True when an entry last used at the given time is past the time limit.
bool isPastExpiry(const std::chrono::system_clock::time_point lastUsed,
                  const std::chrono::system_clock::time_point now)
{
    return Expiry > std::chrono::minutes::zero() && now - lastUsed >= Expiry;
}

/// Parses the index.json of an entry. Null when it cannot be read.
Poco::JSON::Object::Ptr readIndex(const std::string& entry)
{
    std::string json;
    if (FileUtil::readFile(Poco::Path(entry, "index.json").toString(), json, MaxCachedFileSize) <=
        0)
        return nullptr;

    Poco::JSON::Object::Ptr index;
    if (!JsonUtil::parseJSON(json, index))
        return nullptr;
    return index;
}

/// Drops the expired tags. The caller holds the mutex.
void dropExpiredTags(std::chrono::steady_clock::time_point now)
{
    for (auto it = Tags.begin(); it != Tags.end();)
    {
        if (now >= it->second.expiresAt)
            it = Tags.erase(it);
        else
            ++it;
    }
}

/// The bytes of the files in the given directory.
std::size_t directorySize(const std::string& path)
{
    std::size_t size = 0;
    for (const std::string& name : FileUtil::getDirEntries(path))
        size += FileUtil::Stat(Poco::Path(path, name).toString()).size();
    return size;
}

/// Drops what a store left half-written and the entries past their time, then the entries used
/// least recently until the cache fits its size limit. The caller holds the mutex.
void sweep()
{
    struct Entry
    {
        std::string path;
        std::chrono::system_clock::time_point lastUsed;
        std::size_t size;
    };

    LastSweep = std::chrono::steady_clock::now();
    const auto now = std::chrono::system_clock::now();
    std::vector<Entry> entries;
    std::size_t total = 0;
    for (const std::string& name : FileUtil::getDirEntries(CachePath))
    {
        const std::string entry = Poco::Path(CachePath, name).toString();

        // A directory a store was writing into when coolwsd stopped is never finished.
        if (name.find('.') != std::string::npos)
        {
            LOG_INF("PreviewCache: removing the unfinished entry [" << name << ']');
            FileUtil::removeFile(entry, true);
            continue;
        }

        const auto lastUsed = lastUsedOf(entry);
        if (isPastExpiry(lastUsed, now))
        {
            LOG_INF("PreviewCache: removing the entry [" << name << "], unused for "
                                                         << Expiry.count() << " minutes");
            FileUtil::removeFile(entry, true);
            continue;
        }

        // An entry without a stamp sorts first, so it is the first to go.
        const std::size_t size = directorySize(entry);
        total += size;
        entries.push_back(Entry{ entry, lastUsed, size });
    }

    if (total <= MaxSizeBytes)
        return;

    std::sort(entries.begin(), entries.end(),
              [](const Entry& a, const Entry& b) { return a.lastUsed < b.lastUsed; });
    for (const Entry& entry : entries)
    {
        if (total <= MaxSizeBytes)
            break;

        LOG_INF("PreviewCache: removing the entry [" << entry.path << "] of " << entry.size
                                                     << " bytes, the cache holds " << total
                                                     << " bytes");
        FileUtil::removeFile(entry.path, true);
        total -= entry.size;
    }
}

/// Sweeps the cache when the last sweep is a period ago. The caller holds the mutex.
void sweepIfDue()
{
    if (std::chrono::steady_clock::now() - LastSweep >= SweepPeriod)
        sweep();
}
} // namespace

void PreviewCache::initialize(const std::string& path, const std::size_t maxSizeBytes,
                              const std::chrono::minutes expiry, const std::chrono::seconds tagTtl)
{
    std::lock_guard<std::mutex> lock(Mutex);

    if (path.empty())
    {
        LOG_INF("PreviewCache: switched off or no cache path, slide previews are not cached");
        return;
    }

    try
    {
        Poco::File(path).createDirectories();
    }
    catch (const std::exception& exc)
    {
        LOG_WRN("PreviewCache: cannot create [" << path << "], slide previews are not cached: "
                                                << exc.what());
        return;
    }

    // The pictures carry the content of the slides, so no other user of the machine reads
    // them.
    if (::chmod(path.c_str(), S_IRWXU) != 0)
        LOG_WRN("PreviewCache: cannot limit the access to [" << path << "] to coolwsd");

    CachePath = path;
    MaxSizeBytes = maxSizeBytes;
    Expiry = expiry;
    TagTtl = tagTtl;
    LOG_INF("PreviewCache: slide previews are cached in [" << CachePath << "], up to "
                                                           << MaxSizeBytes << " bytes, for "
                                                           << Expiry.count()
                                                           << " minutes after their last use");
    sweep();
}

bool PreviewCache::isEnabled()
{
    std::lock_guard<std::mutex> lock(Mutex);
    return !CachePath.empty();
}

Poco::JSON::Object::Ptr PreviewCache::lookup(const std::string& docKey,
                                             const std::string& lastModifiedTime,
                                             const std::size_t size)
{
    std::lock_guard<std::mutex> lock(Mutex);
    if (CachePath.empty() || lastModifiedTime.empty())
        return nullptr;

    sweepIfDue();

    const std::string entry = entryPath(docKey);
    if (isPastExpiry(lastUsedOf(entry), std::chrono::system_clock::now()) &&
        FileUtil::Stat(entry).exists())
    {
        LOG_INF("PreviewCache: removing the entry of [" << docKey << "], unused for "
                                                        << Expiry.count() << " minutes");
        FileUtil::removeFile(entry, true);
        return nullptr;
    }

    Poco::JSON::Object::Ptr index = readIndex(entry);
    if (!index)
        return nullptr;

    Poco::JSON::Object::Ptr cache = index->getObject("cache");
    if (!cache ||
        cache->optValue<std::string>("lastModifiedTime", std::string()) != lastModifiedTime ||
        cache->optValue<Poco::UInt64>("size", 0) != size)
    {
        LOG_DBG("PreviewCache: the entry of [" << docKey << "] is outdated");
        return nullptr;
    }

    index->remove("cache");
    updateLastUsed(entry);
    LOG_DBG("PreviewCache: answering [" << docKey << "] from the cache");
    return index;
}

void PreviewCache::store(const std::string& docKey, const std::string& lastModifiedTime,
                         const std::size_t size, const Poco::JSON::Object::Ptr& index,
                         const std::vector<std::string>& pictures)
{
    std::lock_guard<std::mutex> lock(Mutex);
    if (CachePath.empty() || lastModifiedTime.empty() || !index)
        return;

    const std::string entry = entryPath(docKey);

    // The new entry is written beside the old one and moved in place once complete, so a
    // reader finds either the old entry or the new one. The dot keeps it apart from finished
    // entries.
    const std::string staging = entry + ".new-" + Util::rng::getHexString(8);
    try
    {
        Poco::File(staging).createDirectories();

        for (std::size_t i = 0; i < pictures.size(); ++i)
        {
            if (!writeFile(picturePath(staging, i), pictures[i]))
                throw std::runtime_error("cannot write a picture");
        }

        Poco::JSON::Object::Ptr cache = new Poco::JSON::Object();
        cache->set("lastModifiedTime", lastModifiedTime);
        cache->set("size", static_cast<Poco::UInt64>(size));

        // The stamp goes into a copy, so the index of the caller stays as it was.
        Poco::JSON::Object stored(*index);
        stored.set("cache", cache);
        std::ostringstream json;
        stored.stringify(json);
        if (!writeFile(Poco::Path(staging, "index.json").toString(), json.str()))
            throw std::runtime_error("cannot write the index");

        updateLastUsed(staging);

        if (FileUtil::Stat(entry).exists())
            FileUtil::removeFile(entry, true);
        Poco::File(staging).renameTo(entry);
    }
    catch (const std::exception& exc)
    {
        LOG_WRN("PreviewCache: cannot store the previews of [" << docKey << "]: " << exc.what());
        FileUtil::removeFile(staging, true);
        return;
    }

    // A tag reads the entry it names, so the positions it learned from the old one are dropped.
    for (auto& it : Tags)
    {
        if (it.second.docKey == docKey)
            it.second.slides.clear();
    }

    LOG_INF("PreviewCache: stored " << pictures.size() << " slide previews of [" << docKey
                                    << ']');
    sweep();
}

void PreviewCache::registerTag(const std::string& tag, const std::string& docKey)
{
    registerTag(tag, docKey, std::chrono::steady_clock::now() + TagTtl);
}

void PreviewCache::registerTag(const std::string& tag, const std::string& docKey,
                               const std::chrono::steady_clock::time_point expiresAt)
{
    std::lock_guard<std::mutex> lock(Mutex);
    if (CachePath.empty())
        return;

    dropExpiredTags(std::chrono::steady_clock::now());
    Tag& entry = Tags[tag];
    entry.docKey = docKey;
    entry.expiresAt = expiresAt;
    entry.slides.clear();
}

PreviewCache::Lookup PreviewCache::readPicture(const std::string& tag, const std::string& guid,
                                               std::string& png)
{
    std::lock_guard<std::mutex> lock(Mutex);
    if (CachePath.empty())
        return Lookup::UnknownTag;

    const auto now = std::chrono::steady_clock::now();
    const auto it = Tags.find(tag);
    if (it == Tags.end())
        return Lookup::UnknownTag;

    if (now >= it->second.expiresAt)
    {
        Tags.erase(it);
        return Lookup::UnknownTag;
    }

    it->second.expiresAt = now + TagTtl;

    const std::string entry = entryPath(it->second.docKey);
    if (it->second.slides.empty())
    {
        Poco::JSON::Object::Ptr index = readIndex(entry);
        if (!index)
            return Lookup::Missing;

        if (Poco::JSON::Array::Ptr slides = index->getArray("slides"))
        {
            for (std::size_t i = 0; i < slides->size(); ++i)
            {
                if (Poco::JSON::Object::Ptr slide = slides->getObject(i))
                    it->second.slides.emplace(
                        slide->optValue<std::string>("part", std::string()), i);
            }
        }
    }

    // The picture is named by the position of the slide, which comes from the index, so the
    // GUID of the request never becomes a path.
    const auto itSlide = it->second.slides.find(guid);
    if (itSlide == it->second.slides.end())
        return Lookup::UnknownSlide;

    png.clear();
    if (FileUtil::readFile(picturePath(entry, itSlide->second), png, MaxCachedFileSize) <= 0)
        return Lookup::Missing;

    return Lookup::Found;
}

long PreviewCache::remainingTtlSeconds(const std::string& tag)
{
    std::lock_guard<std::mutex> lock(Mutex);
    const auto it = Tags.find(tag);
    const auto now = std::chrono::steady_clock::now();
    if (it == Tags.end() || now >= it->second.expiresAt)
        return 0;
    return std::chrono::duration_cast<std::chrono::seconds>(it->second.expiresAt - now).count();
}

std::string PreviewCache::buildContentUrl(const std::string& tag, const Poco::URI& wopiSrc,
                                          const std::string& guid)
{
    // The tag is hexadecimal, so it goes into the path as it is.
    std::string url = "/cool/preview/" + tag + "/content?WOPISrc=" +
                      Uri::encode(wopiSrc.toString()) +
                      "&ServerId=" + Util::getProcessIdentifier() + "&slide=" + Uri::encode(guid);
    if (!COOLWSD::RouteToken.empty())
        url += "&RouteToken=" + COOLWSD::RouteToken;
    return url;
}

std::string PreviewCache::buildAnswer(const Poco::JSON::Object::Ptr& index,
                                      const std::string& tag, const long ttlSecs,
                                      const Poco::URI& wopiSrc)
{
    // The index is shared with the cache, so the answer is built on a copy of it.
    std::ostringstream copy;
    index->stringify(copy);
    Poco::JSON::Object::Ptr answer;
    if (!JsonUtil::parseJSON(copy.str(), answer))
        return std::string();

    const auto expiresAt = std::chrono::system_clock::now() + std::chrono::seconds(ttlSecs);
    answer->set("version", 1);
    answer->set("tag", tag);
    answer->set("expiresAt", Util::getIso8601FracformatTime(expiresAt));
    answer->set("ttlSecs", ttlSecs);

    if (Poco::JSON::Array::Ptr slides = answer->getArray("slides"))
    {
        for (std::size_t i = 0; i < slides->size(); ++i)
        {
            Poco::JSON::Object::Ptr slide = slides->getObject(i);
            if (!slide)
                continue;

            if (Poco::JSON::Object::Ptr preview = slide->getObject("preview"))
                preview->set("url",
                             buildContentUrl(tag, wopiSrc,
                                             slide->optValue<std::string>("part", std::string())));
        }
    }

    std::ostringstream oss;
    answer->stringify(oss);
    return oss.str();
}

void PreviewCache::dumpState(std::ostream& os)
{
    std::lock_guard<std::mutex> lock(Mutex);
    os << "\nPreviewCache:";
    os << "\n  path: " << (CachePath.empty() ? std::string("none, disabled") : CachePath);
    os << "\n  size limit: " << MaxSizeBytes;
    os << " bytes\n  expiry: " << Expiry.count();
    os << " minutes after the last use\n  tag ttl: " << TagTtl.count();
    os << "s\n  live tags: " << Tags.size();
    os << "\n  entries: " << (CachePath.empty() ? 0 : FileUtil::getDirEntries(CachePath).size());
    os << '\n';
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
