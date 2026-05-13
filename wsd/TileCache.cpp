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
 * Implementation of tile cache storage and retrieval.
 * Classes: TileCache
 */

#include <config.h>

#include "TileCache.hpp"

#include <common/Common.hpp>
#include <common/FileUtil.hpp>
#include <common/Protocol.hpp>
#include <common/StringVector.hpp>
#include <common/Unit.hpp>
#include <common/Util.hpp>
#include <wsd/ClientSession.hpp>

#include <algorithm>
#include <cassert>
#include <climits>
#include <cstddef>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace COOLProtocol;

TileCache::TileCache(std::string docURL, const std::chrono::system_clock::time_point& modifiedTime,
                     bool dontCache)
    : _docURL(std::move(docURL))
    , _owner()
    , _cacheSize(0)
    , _maxCacheSize(1024 * 1024)
    , _dontCache(dontCache)
{
#ifndef BUILDING_TESTS
    LOG_INF("TileCache ctor for uri [" << COOLWSD::anonymizeUrl(_docURL) <<
            "], modifiedTime=" << std::chrono::duration_cast<std::chrono::seconds>
							(modifiedTime.time_since_epoch()).count() << "], dontCache=" << _dontCache);
#endif
    (void)modifiedTime;
}

TileCache::~TileCache()
{
    _owner = ProcUtil::ThreadId();
#ifndef BUILDING_TESTS
    LOG_INF("~TileCache dtor for uri [" << COOLWSD::anonymizeUrl(_docURL) << "].");
#endif
}

void TileCache::clear()
{
    _cache.clear();
    _cacheSize = 0;
    for (std::map<std::string, Blob>& i : _streamCache)
        i.clear();

    LOG_INF("Completely cleared tile cache for: " << _docURL);
}

/// Tracks the rendering of a given tile
/// to avoid duplication and help clock
/// rendering latency.
struct TileCache::TileBeingRendered
{
    explicit TileBeingRendered(const TileDesc& tile,
                               const std::chrono::steady_clock::time_point now)
        : _startTime(now)
        , _tile(tile)
    {
    }

    const TileDesc& getTile() const { return _tile; }

    /// The version is used to ensure that if we have two in-progress
    /// renders sent to the kit, racing each other that the completion of
    /// the first does not remove the subscribers waiting for the second.
    int getVersion() const { return _tile.getVersion(); }
    void setVersion(int version) { _tile.setVersion(version); }

    std::chrono::steady_clock::time_point getStartTime() const { return _startTime; }
    /// Reset the start time. Used after re-issuing a stalled render so the
    /// entry is not flagged stale again on the very next sweep.
    void resetStartTime(std::chrono::steady_clock::time_point now) { _startTime = now; }
    std::chrono::milliseconds
    getElapsedTimeMs(const std::chrono::steady_clock::time_point now) const
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(now - _startTime);
    }

    bool isStale(const std::chrono::steady_clock::time_point now) const
    {
        return getElapsedTimeMs(now) > std::chrono::milliseconds(COMMAND_TIMEOUT_MS);
    }

    std::vector<std::weak_ptr<ClientSession>>& getSubscribers() { return _subscribers; }

    void dumpState(std::ostream& os);

private:
    std::vector<std::weak_ptr<ClientSession>> _subscribers;
    std::chrono::steady_clock::time_point _startTime;
    TileDesc _tile;
};

size_t TileCache::countTilesBeingRenderedForSession(const std::shared_ptr<ClientSession>& session,
                                                    const std::chrono::steady_clock::time_point now)
{
    size_t count = 0;
    for (const auto& it : _tilesBeingRendered)
    {
        if (it.second->isStale(now))
            continue;

        for (const auto& s : it.second->getSubscribers())
        {
            if (s.lock() == session)
                ++count;
        }
    }

    return count;
}

bool TileCache::hasTileBeingRendered(const TileDesc& tileDesc, const std::chrono::steady_clock::time_point *now) const
{
    const auto it = _tilesBeingRendered.find(tileDesc);
    if (it == _tilesBeingRendered.end())
        return false;

    /// did we stall ? if so re-issue.
    return !now ? true : !it->second->isStale(*now);
}

std::shared_ptr<TileCache::TileBeingRendered> TileCache::findTileBeingRendered(const TileDesc& tileDesc)
{
    ASSERT_CORRECT_THREAD_OWNER(_owner);

    const auto tile = _tilesBeingRendered.find(tileDesc);
    return tile != _tilesBeingRendered.end() ? tile->second : nullptr;
}

void TileCache::forgetTileBeingRendered(const TileDesc &descForKitReply,
                                        const std::shared_ptr<TileCache::TileBeingRendered>& tileBeingRendered)
{
    ASSERT_CORRECT_THREAD_OWNER(_owner);
    assert(tileBeingRendered);
    assert(hasTileBeingRendered(tileBeingRendered->getTile()));

    if (tileBeingRendered->getVersion() <= descForKitReply.getVersion())
    {
        LOG_TRC("Removing all subscribers for " << tileBeingRendered->getTile().serialize());
        _tilesBeingRendered.erase(tileBeingRendered->getTile());
    }
    else
        LOG_TRC("Racing renderings for tile " << tileBeingRendered->getTile().serialize() <<
                " waiting for ver " << tileBeingRendered->getVersion() << " but have " << descForKitReply.getVersion());
}

int TileCache::getTileBeingRenderedVersion(const TileDesc& tile)
{
    std::shared_ptr<TileBeingRendered> tileBeingRendered = findTileBeingRendered(tile);
    return tileBeingRendered ? tileBeingRendered->getVersion() : 0;
}

std::vector<TileDesc>
TileCache::takeStaleRendersForReissue(std::chrono::steady_clock::time_point now)
{
    ASSERT_CORRECT_THREAD_OWNER(_owner);

    std::vector<TileDesc> reissue;
    for (auto it = _tilesBeingRendered.begin(); it != _tilesBeingRendered.end(); )
    {
        const auto& tbr = it->second;
        if (!tbr->isStale(now))
        {
            ++it;
            continue;
        }

        // Compact the subscriber list, discarding any sessions that
        // have gone away while the kit was being slow.
        auto& subs = tbr->getSubscribers();
        subs.erase(std::remove_if(subs.begin(), subs.end(),
                                  [](const std::weak_ptr<ClientSession>& w)
                                  { return w.expired(); }),
                   subs.end());

        if (subs.empty())
        {
            LOG_DBG("Dropping stale render with no live subscribers: "
                    << it->first.serialize());
            it = _tilesBeingRendered.erase(it);
            continue;
        }

        LOG_WRN("Re-issuing stalled tile render after "
                << tbr->getElapsedTimeMs(now).count() << "ms for "
                << subs.size() << " subscribers: " << it->first.serialize());
        reissue.push_back(it->first);
        tbr->resetStartTime(now);
        ++it;
    }
    return reissue;
}

Tile TileCache::lookupTile(const TileDesc& tile)
{
    if (_dontCache)
        return Tile();

    Tile ret = findTile(tile);

    UNITWSD_CALL(lookupTile(tile.getPart(), tile.getEditMode(), tile.getWidth(), tile.getHeight(),
                            tile.getTilePosX(), tile.getTilePosY(), tile.getTileWidth(),
                            tile.getTileHeight(), ret));

    return ret;
}

void TileCache::saveTileAndNotify(const TileDesc& desc, const char *data, const size_t size)
{
    ASSERT_CORRECT_THREAD_OWNER(_owner);

    std::shared_ptr<TileBeingRendered> tileBeingRendered = findTileBeingRendered(desc);

    // Save to in-memory cache.

    // Ignore if we can't save the tile, things will work anyway, but slower.
    // An error indication is supposed to be sent to all users in that case.
    Tile tile = saveDataToCache(desc, data, size);
    if (!_dontCache)
        LOG_TRC("Saved cache tile: " << cacheFileName(desc) << " of size " << size << " bytes");
    else
        LOG_TRC("Got (non-cached) tile: " << cacheFileName(desc));

    // Notify subscribers, if any.
    if (tileBeingRendered)
    {
        const size_t subscriberCount = tileBeingRendered->getSubscribers().size();

        // sendTile also does enqueueSendMessage underneath ...
        if (tile && subscriberCount > 0)
        {
            for (size_t i = 0; i < subscriberCount; ++i)
            {
                auto& subscriber = tileBeingRendered->getSubscribers()[i];
                std::shared_ptr<ClientSession> session = subscriber.lock();
                if (session)
                {
                    session->sendTileNow(desc, tile);
                    if (auto db = session->getDocumentBroker())
                        db->recordFirstTileSent();
                }
            }
        }
        else if (subscriberCount == 0)
            LOG_DBG("No subscribers for: " << cacheFileName(desc));
        // else zero sized

        // Remove subscriptions.
        LOG_DBG("STATISTICS: tile "
                << desc.getVersion() << " internal roundtrip "
                << tileBeingRendered->getElapsedTimeMs(std::chrono::steady_clock::now()));
        forgetTileBeingRendered(desc, tileBeingRendered);
    }
    else
        LOG_DBG("No subscribers for: " << cacheFileName(desc));
}

bool TileCache::getTextStream(StreamType type, const std::string& fileName, std::string& content)
{
    Blob textStream = lookupCachedStream(type, fileName);
    if (!textStream)
    {
        // This is not an error because the first time
        // we lookup a file, it won't be in the cache.
        LOG_INF("Cache miss, could not find text stream: " << fileName);
        return false;
    }

    std::vector<char> buffer = *textStream;

    if (!buffer.empty() && buffer.back() == '\n')
        buffer.pop_back();

    content = std::string(buffer.data(), buffer.size());
    LOG_INF("Read '" << COOLProtocol::getAbbreviatedMessage(content.c_str(), content.size()) <<
            "' from " << fileName);

    return true;
}

void TileCache::saveTextStream(StreamType type, const std::string& fileName,
                               const std::vector<char>& data)
{
    LOG_INF("Saving '" << COOLProtocol::getAbbreviatedMessage(data.data(), data.size()) << "' to " << fileName
                       << " of size " << data.size() << " bytes");

    saveDataToStreamCache(type, fileName, data.data(), data.size());
}

void TileCache::saveStream(StreamType type, const std::string& name, const char *data, std::size_t size)
{
    // can fonts be invalidated?
    saveDataToStreamCache(type, name, data, size);
}

Blob TileCache::lookupCachedStream(StreamType type, const std::string& name)
{
    auto it = _streamCache[type].find(name);
    if (it != _streamCache[type].end())
    {
        LOG_TRC("Found stream cache tile: " << name << " of size " << it->second->size() << " bytes");
        return it->second;
    }

    return Blob();
}

bool TileCache::invalidateTiles(int part, int mode, int x, int y, int width, int height, CanonicalViewId canonicalViewId)
{
    LOG_TRC("Removing invalidated tiles: part: " << part << ", mode: " << mode <<
            ", x: " << x << ", y: " << y <<
            ", width: " << width <<
            ", height: " << height <<
            ", viewid: " << canonicalViewId);

    ASSERT_CORRECT_THREAD_OWNER(_owner);

    if (_cache.empty())
    {
        LOG_TRC("Removing invalidated tiles: cache was empty");
        return false;
    }

    for (auto it = _cache.begin(); it != _cache.end();)
    {
        if (intersectsTile(it->first, part, mode, x, y, width, height, canonicalViewId))
        {
            // FIXME: only want to keep as invalid keyframes in the view area(s)
            it->second->invalidate();
            ++it;
        }
#if 0
        {
            LOG_TRC("Removing tile: " << it->first.serialize());
            _cacheSize -= itemCacheSize(it->second);
            it = _cache.erase(it);
        }
#endif
        else
        {
            ++it;
        }
    }

    return true;
}

bool TileCache::invalidateTiles(const std::string& tiles, CanonicalViewId canonicalViewId)
{
    int part = 0, mode = 0;
    TileWireId wireId = 0;
    const Util::Rectangle invalidateRect = TileCache::parseInvalidateMsg(tiles, part, mode, wireId);

    return invalidateTiles(part, mode, invalidateRect.getLeft(), invalidateRect.getTop(),
                    invalidateRect.getWidth(), invalidateRect.getHeight(), canonicalViewId);
}

Util::Rectangle TileCache::parseInvalidateMsg(const std::string& tiles, int &part, int &mode, TileWireId &wireId)
{
    StringVector tokens = StringVector::tokenize(tiles);

    assert(!tokens.empty() && tokens.equals(0, "invalidatetiles:"));

    mode = 0;
    part = 0;
    wireId = 0;
    if (tokens.size() == 2 && tokens.equals(1, "EMPTY"))
    {
        part = -1;
        return Util::Rectangle(0, 0, INT_MAX, INT_MAX);
    }
    else if (tokens.size() == 3 && tokens.equals(1, "EMPTY,"))
    {
        part = -1;
        if (!tokens.getUInt32(2, "wid", wireId))
            assert(false && "missing wid");
        return Util::Rectangle(0, 0, INT_MAX, INT_MAX);
    }
    else if (tokens.size() == 4 && tokens.equals(1, "EMPTY,"))
    {
        if (stringToInteger(tokens[2], part))
        {
            if (!tokens.getUInt32(3, "wid", wireId))
                assert(false && "missing wid");
            return Util::Rectangle(0, 0, INT_MAX, INT_MAX);
        }
    }
    else if (tokens.size() == 5 && tokens.equals(1, "EMPTY,"))
    {
        if (stringToInteger(tokens[2], part))
        {
            if (stringToInteger(tokens[3], mode))
            {
                if (!tokens.getUInt32(4, "wid", wireId))
                    assert(false && "missing wid");
                return Util::Rectangle(0, 0, INT_MAX, INT_MAX);
            }
        }
    }
    else
    {
        int x = 0;
        int y = 0;
        int width = 0;
        int height = 0;
        if (tokens.size() == 7 &&
            getTokenInteger(tokens[1], "part", part) &&
            getNonNegTokenInteger(tokens[2], "x", x) &&
            getNonNegTokenInteger(tokens[3], "y", y) &&
            getNonNegTokenInteger(tokens[4], "width", width) &&
            getNonNegTokenInteger(tokens[5], "height", height) &&
            tokens.getUInt32(6, "wid", wireId))
        {
            return Util::Rectangle(x, y, width, height);
        }
        else if (tokens.size() == 8 &&
            getTokenInteger(tokens[1], "part", part) &&
            getTokenInteger(tokens[2], "mode", mode) &&
            getNonNegTokenInteger(tokens[3], "x", x) &&
            getNonNegTokenInteger(tokens[4], "y", y) &&
            getNonNegTokenInteger(tokens[5], "width", width) &&
            getNonNegTokenInteger(tokens[6], "height", height) &&
            tokens.getUInt32(7, "wid", wireId))
        {
            return Util::Rectangle(x, y, width, height);
        }
    }

    LOG_ERR("Unexpected invalidatetiles request [" << tiles << "].");
    assert(false && "Unexpected invalidatetiles request");
    part = -1;
    return Util::Rectangle(0, 0, 0, 0);
}

std::string TileCache::cacheFileName(const TileDesc& tile)
{
    std::ostringstream oss;
    oss << tile.getCanonicalViewId() << '_' << tile.getPart() << '_' << tile.getEditMode() << '_'
        << tile.getWidth() << 'x' << tile.getHeight() << '.'
        << tile.getTilePosX() << ',' << tile.getTilePosY() << '.'
        << tile.getTileWidth() << 'x' << tile.getTileHeight() << ".png";
    return oss.str();
}

bool TileCache::parseCacheFileName(const std::string& fileName, int& part, int& mode, int& width, int& height,
                                   int& tilePosX, int& tilePosY, int& tileWidth, int& tileHeight,
                                   int& nviewid)
{
    return std::sscanf(fileName.c_str(), "%d_%d_%d_%dx%d.%d,%d.%dx%d.png", &nviewid, &part, &mode,
                       &width, &height, &tilePosX, &tilePosY, &tileWidth, &tileHeight)
           == 8;
}

bool TileCache::intersectsTile(const TileDesc &tileDesc, int part, int mode, int x, int y, int width, int height, CanonicalViewId canonicalViewId)
{
    if (part != -1 && tileDesc.getPart() != part)
        return false;

    if (mode != tileDesc.getEditMode())
        return false;

    if (canonicalViewId != tileDesc.getCanonicalViewId())
        return false;

    const int left = std::max(x, tileDesc.getTilePosX());
    const int right = std::min(x + width, tileDesc.getTilePosX() + tileDesc.getTileWidth());
    const int top = std::max(y, tileDesc.getTilePosY());
    const int bottom = std::min(y + height, tileDesc.getTilePosY() + tileDesc.getTileHeight());

    return left <= right && top <= bottom;
}

// FIXME: to be further simplified when we centralize tile messages.
bool TileCache::subscribeToTileRendering(const TileDesc& tile,
                                         const std::shared_ptr<ClientSession>& subscriber,
                                         const std::chrono::steady_clock::time_point now)
{
    ASSERT_CORRECT_THREAD_OWNER(_owner);

    std::shared_ptr<TileBeingRendered> tileBeingRendered = findTileBeingRendered(tile);

    if (tileBeingRendered)
    {
        if (tileBeingRendered->isStale(now))
            LOG_DBG("Painting stalled; need to re-issue on tile " << tile.debugName());

        for (const auto &s : tileBeingRendered->getSubscribers())
        {
            if (s.lock().get() == subscriber.get())
            {
                LOG_TRC("Redundant request to subscribe on tile " << tile.debugName());
                // the version stops us unsubscribing when we get there.
                tileBeingRendered->setVersion(tile.getVersion());
                return false;
            }
        }

        LOG_DBG("Subscribing " << subscriber->getName() << " to tile " << tile.debugName() << " which has " <<
                tileBeingRendered->getSubscribers().size() << " subscribers already.");
        tileBeingRendered->getSubscribers().push_back(subscriber);
    }
    else
    {
        LOG_DBG("Subscribing " << subscriber->getName() << " to tile " << tile.debugName() <<
                " ver=" << tile.getVersion() << " which has no subscribers " << tile.serialize());

        assert(_tilesBeingRendered.find(tile) == _tilesBeingRendered.end());

        tileBeingRendered = std::make_shared<TileBeingRendered>(tile, now);
        tileBeingRendered->getSubscribers().push_back(subscriber);
        _tilesBeingRendered[tile] = std::move(tileBeingRendered);
    }
    return true;
}

Tile TileCache::findTile(const TileDesc &desc)
{
    const auto it = _cache.find(desc);
    if (it != _cache.end())
    {
        LOG_TRC("Found cache tile: " << desc.serialize() << " of size " << it->second);
        return it->second;
    }

    return Tile();
}

Tile TileCache::saveDataToCache(const TileDesc &desc, const char *data, const size_t size)
{
    if (_dontCache)
        return std::make_shared<TileData>(desc.getWireId(), data, size);

    ensureCacheSize();

    Tile tile = _cache[desc];
    if (!tile)
    {
        if (!TileData::isKeyframe(data, size))
        {
            // canceltiles removes all subscribers - which allows the
            // TileCache re-balancing to remove the in-process delta's
            // underlying keyframe.
            LOG_TRC("rare race between canceltiles and delta rendering - "
                    "discarding delta for " << desc.serialize());
            _cache.erase(desc);
            return Tile();
        }
        else
        {
            LOG_TRC("new tile for " << desc.serialize() << " of size " << size);
            tile = std::make_shared<TileData>(desc.getWireId(), data, size);
            _cache[desc] = tile;
            _cacheSize += itemCacheSize(tile);
        }
    }
    else
    {
        LOG_TRC("append blob to " << desc.serialize() << " of size " << size);
        _cacheSize += tile->appendBlob(desc.getWireId(), data, size);
    }

    return tile;
}

size_t TileCache::itemCacheSize(const Tile &tile)
{
    return sizeof(Tile) + sizeof(TileDesc) + tile->size();
}

void TileCache::assertCacheSize()
{
    if constexpr (Util::isDebugEnabled())
    {
#if !defined NDEBUG
        size_t recalcSize = 0;
        for (const auto& it : _cache)
        {
            recalcSize += itemCacheSize(it.second);
        }
        assert(recalcSize == _cacheSize);
#endif
    }
}

void TileCache::ensureCacheSize()
{
    assertCacheSize();

    if (_cacheSize < _maxCacheSize || _cache.size() < 2)
        return;

    LOG_TRC("Cleaning tile cache of size " << _cacheSize << " vs. " << _maxCacheSize <<
            " with " << _cache.size() << " entries");

    struct WidSize {
        TileWireId _wid;
        size_t     _size;
        WidSize(TileWireId w, size_t s) : _wid(w), _size(s) {}
    };
    std::vector<WidSize> wids;
    wids.reserve(_cache.size());
    for (const auto& it : _cache)
        wids.emplace_back(it.first.getWireId(), itemCacheSize(it.second));

    std::sort(wids.begin(), wids.end(),
              [](const WidSize &a, const WidSize &b) { return a._wid < b._wid; });

    // FIXME: should we just take a wid 25% into the list ?
    TileWireId maxToRemove = wids.front()._wid;

    // do we have (the very rare) WID wrap-around
    if (wids.back()._wid - wids.front()._wid > 256 * 256 * 256)
    {
        maxToRemove = wids.back()._wid;
        LOG_TRC("Rare wid wrap-around detected, clear tile cache");
    }
    else
    {
        // calculate which wid to start at.
        size_t total = 0;
        for (const auto &it : wids)
        {
            total += it._size;
            maxToRemove = it._wid;
            if (total > _maxCacheSize / 4)
                break;
        }
    }
    LOG_TRC("cleaning up to wid " << maxToRemove << " between " <<
            wids.front()._wid << " and " << wids.back()._wid);

    for (auto it = _cache.begin(); it != _cache.end();)
    {
        if (it->first.getWireId() <= maxToRemove)
        {
            auto rit = _tilesBeingRendered.find(it->first);
            if (rit != _tilesBeingRendered.end())
            {
                // avoid getting a delta instead of a keyframe at the bottom.
                LOG_TRC("skip cleaning tile we are waiting on: " << it->first.serialize() <<
                        " which has " << rit->second->getSubscribers().size() << " waiting");
                ++it;
            }
            else
            {
                LOG_TRC("cleaned out tile: " << it->first.serialize());
                _cacheSize -= itemCacheSize(it->second);
                it = _cache.erase(it);
            }
        }
        else
        {
            ++it;
        }
    }

    LOG_TRC("Cache is now of size " << _cacheSize << " and " <<
            _cache.size() << " entries after cleaning");

    assertCacheSize();
}

void TileCache::setMaxCacheSize(size_t cacheSize)
{
    _maxCacheSize = cacheSize;
    ensureCacheSize();
}

#ifdef BUILDING_TESTS
void TileCache::injectTileBeingRenderedForTest(
    const TileDesc& tile,
    std::chrono::steady_clock::time_point startTime,
    const std::shared_ptr<ClientSession>& subscriber)
{
    auto tileBeingRendered = std::make_shared<TileBeingRendered>(tile, startTime);
    if (subscriber)
        tileBeingRendered->getSubscribers().push_back(subscriber);
    _tilesBeingRendered[tile] = std::move(tileBeingRendered);
}
#endif

void TileCache::saveDataToStreamCache(StreamType type, const std::string &fileName, const char *data, const size_t size)
{
    if (_dontCache)
        return;

    Blob blob = std::make_shared<BlobData>(size);
    std::memcpy(blob->data(), data, size);
    _streamCache[type][fileName] = std::move(blob);
}

void TileCache::TileBeingRendered::dumpState(std::ostream& os)
{
    os << "    " << _tile.serialize() << ' ' << std::setw(4)
       << getElapsedTimeMs(std::chrono::steady_clock::now()) << _subscribers.size()
       << " subscribers\n";
    for (const auto& it : _subscribers)
    {
        std::shared_ptr<ClientSession> session = it.lock();
        if (session)
        {
            os << "      " << session->getId() << ' ' << session->getUserId() << ' '
               << session->getName() << '\n';
        }
    }
}

void TileCache::dumpState(std::ostream& os)
{
    os << "\n  TileCache:";
    os << "\n    num: " << _cache.size() << ", size: " << _cacheSize << " (max: " << _maxCacheSize
       << ") bytes\n";
    size_t totalSize = 0;
    size_t totalCapacity = 0;
    for (const auto& it : _cache)
    {
        totalSize += it.second->size();
        totalCapacity += it.second->data().capacity();
        os << "    " << std::setw(4) << it.first.getWireId() << '\t' << std::setw(6)
           << it.second->size() << " bytes" << "\t'" << it.first.serialize() << " ";
        it.second->dumpState(os);
        os << '\n';
    }

    int type = 0;
    for (const auto& i : _streamCache)
    {
        size_t num = 0;
        size_t size = 0;
        size_t capacity = 0;
        for (const auto& it : i)
        {
            num++;
            size += it.second->size();
            capacity += it.second->capacity();
        }

        totalSize += size;
        totalCapacity += capacity;
        os << "    stream cache: " << type++ << ", num: " << num << ", size: " << size << " ("
           << capacity << ") bytes\n";
        for (const auto& it : i)
        {
            os << "    " << it.first << '\t' << std::setw(6) << it.second->size() << " ("
               << std::setw(6) << it.second->capacity() << ") bytes\n";
        }
    }

    os << "    total size: " << totalSize << ", total capacity: " << totalCapacity << " bytes\n";
    os << "    tiles being rendered " << _tilesBeingRendered.size() << '\n';
    for (const auto& it : _tilesBeingRendered)
        it.second->dumpState(os);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
