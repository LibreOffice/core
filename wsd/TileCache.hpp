/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
 * Tile caching for rendered document tiles.
 * Classes: TileCache
 */

#pragma once

#include <common/Common.hpp>
#include <common/Log.hpp>
#include <common/ProcUtil.hpp>
#include <common/Rectangle.hpp>
#include <wsd/TileDesc.hpp>

#include <chrono>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

class ClientSession;

// The cache cares about only some properties.
struct TileDescCacheCompareEq final
{
    bool operator()(const TileDesc& l, const TileDesc& r) const
    {
        return l.getPart() == r.getPart() &&
               l.getWidth() == r.getWidth() &&
               l.getHeight() == r.getHeight() &&
               l.getTilePosX() == r.getTilePosX() &&
               l.getTilePosY() == r.getTilePosY() &&
               l.getTileWidth() == r.getTileWidth() &&
               l.getTileHeight() == r.getTileHeight() &&
               l.getCanonicalViewId() == r.getCanonicalViewId() &&
               l.getEditMode() == r.getEditMode();
    }
};

// The cache cares about only some properties.
struct TileDescCacheHasher final
{
    size_t operator()(const TileDesc& t) const
    {
        size_t hash = t.getPart();

        hash = (hash << 5) + hash + t.getEditMode();
        hash = (hash << 5) + hash + t.getWidth();
        hash = (hash << 5) + hash + t.getHeight();
        hash = (hash << 5) + hash + t.getTilePosX();
        hash = (hash << 5) + hash + t.getTilePosY();
        hash = (hash << 5) + hash + t.getTileWidth();
        hash = (hash << 5) + hash + t.getTileHeight();
        hash = (hash << 5) + hash + to_underlying(t.getCanonicalViewId());

        return hash;
    }
};

struct TileData
{
    TileData(TileWireId start, const char *data, const size_t size)
    {
        appendBlob(start, data, size);
    }

    // Add a frame or delta and - return the size change
    ssize_t appendBlob(TileWireId id, const char *data, const size_t dataSize)
    {
        size_t oldCacheSize = size();

        assert (dataSize >= 1); // kit provides us a 'Z' or a 'D' or a png
        if (isKeyframe(data, dataSize))
        {
            LOG_TRC("received key-frame - clearing tile");
            _wids.clear();
            _offsets.clear();
            _deltas.clear();
        }
        else
        {
            LOG_TRC("received delta of size " << dataSize << " - appending to existing " << _wids.size());
            // Issues #5532 and #5831 Replace assert with a log message
            // Remove assert and allow delta messages to be handled even if
            // there is no keyframe. Although it might make sense to skip
            // delta messages if there is no keyframe, that causes some
            // content, at least in Impress documents, to not render.
            if (!_wids.size())
                LOG_DBG("no underlying keyframe!");
        }

        size_t oldSize = size();

        // If we have an empty delta at the end - then just
        // bump the associated wid. There is no risk to sending
        // an empty delta twice.x
        if (dataSize == 1 && // just a 'D'
            _offsets.size() > 1 &&
            _offsets.back() == _deltas.size())
        {
            LOG_TRC("received empty delta - bumping wid from " << _wids.back() << " to " << id);
            _wids.back() = id;
        }
        else
        {
            _wids.push_back(id);
            _offsets.push_back(_deltas.size());
            if (dataSize > 1)
            {
                _deltas.resize(oldSize + dataSize - 1);
                std::memcpy(_deltas.data() + oldSize, data + 1, dataSize - 1);
            }
        }

        // FIXME: possible race - should store a seq. from the invalidation(s) ?
        _valid = true;

        return size() - oldCacheSize;
    }

    // At what point do we stop stacking deltas & render a keyframe ?
    bool tooLarge() const
    {
        // keyframe gets a free size pass
        if (_offsets.size() <= 1)
            return false;
        size_t deltaSize = size() - _offsets[1];
        return deltaSize > 128 * 1024; // deltas should be cumulatively small.
    }

    bool isPng() const { return (_deltas.size() > 1 &&
                                 _deltas[0] == (char)0x89); }

    static bool isKeyframe(const char *data, size_t dataSize)
    {
        // keyframe or png
        return dataSize > 0 && (data[0] == 'Z' || data[0] == (char)0x89);
    }

    bool isValid() const { return _valid; }
    void invalidate() { _valid = false; }

    std::vector<TileWireId> _wids;
    std::vector<size_t> _offsets; // offset of the start of data
    BlobData _deltas; // first item is a key-frame, followed by deltas at _offsets
    bool _valid; // not true - waiting for a new tile if in view.

    size_t size() const
    {
        return _deltas.size();
    }

    const BlobData &data() const
    {
        return _deltas;
    }

    /// if we send changes since this seq - do we need to first send the keyframe ?
    bool needsKeyframe(TileWireId since) const
    {
        return since < _wids[0];
    }

    bool appendChangesSince(std::vector<char>& output, TileWireId since) const
    {
        size_t i;
        for (i = 0; since != 0 && i < _wids.size() && _wids[i] <= since; ++i);

        if (i >= _wids.size())
        {
            // We don't throttle delta sending - yet the code thinks we do still.
            // We just send all the deltas we have on top of the keyframe.
            // LOG_WRN("odd outcome - requested for a later id " << since <<
            //        " than the last known: " << ((_wids.size() > 0) ? _wids.back() : -1));
            return false;
        }
        else
        {
            size_t offset = _offsets[i];
            if (i != _offsets.size() - 1)
                LOG_TRC("appending from " << i << " to " << (_offsets.size() - 1) <<
                        " from wid: " << _wids[i] << " to wid: " << since <<
                        " from offset: " << offset << " to " << _deltas.size());

            assert(offset <= _deltas.size());
            if (offset < _deltas.size())
            {
                const size_t extra = _deltas.size() - offset;
                const size_t dest = output.size();
                output.resize(dest + extra);
                std::memcpy(output.data() + dest, _deltas.data() + offset, extra);
            }

            return true;
        }
    }

    void dumpState(std::ostream& os)
    {
        if (_wids.size() < 2)
            os << "keyframe";
        else {
            os << "deltas: ";
            for (size_t i = 0; i < _wids.size(); ++i)
            {
                os << i << ": " << _wids[i] << " -> " << _offsets[i] << ' ';
            }
            os << (tooLarge() ? "too-large " : "");
        }
    }
};
using Tile = std::shared_ptr<TileData>;

/// Handles the caching of tiles of one document.
class TileCache
{
    struct TileBeingRendered;

    std::shared_ptr<TileBeingRendered> findTileBeingRendered(const TileDesc& tile);

public:
    /// When the docURL is a non-file:// url, the timestamp has to be provided by the caller.
    /// For file:// url's, it's ignored.
    /// When it is missing for non-file:// url, it is assumed the document must be read, and no cached value used.
    TileCache(std::string docURL, const std::chrono::system_clock::time_point& modifiedTime,
              bool dontCache = false);
    ~TileCache();

    /// Completely clear the cache contents.
    void clear();

    TileCache(const TileCache&) = delete;
    TileCache& operator=(const TileCache&) = delete;

    /// Subscribes if no subscription exists and returns true.
    /// Otherwise returns false to signify a subscription already exists.
    bool subscribeToTileRendering(const TileDesc& tile,
                                  const std::shared_ptr<ClientSession>& subscriber,
                                  std::chrono::steady_clock::time_point now);

    /// Cancels all tile requests by the given subscriber.
    std::string cancelTiles(const std::shared_ptr<ClientSession>& subscriber);

    /// Find the tile with this description
    Tile lookupTile(const TileDesc& tile);

    void saveTileAndNotify(const TileDesc& tile, const char* data, size_t size);

    enum StreamType : std::uint8_t
    {
        Style,
        CmdValues,
        Last
    };

    /// Get the content of a cache file.
    /// @param content Valid only when the call returns true.
    /// @return true when the file actually exists
    bool getTextStream(StreamType type, const std::string& fileName, std::string& content);

    // Save some text into a file in the cache directory
    void saveTextStream(StreamType type, const std::string& fileName, const std::vector<char>& data);

    // Saves a font / style / etc rendering
    void saveStream(StreamType type, const std::string& name, const char* data, size_t size);

    /// Return the data if we have it, or nothing.
    Blob lookupCachedStream(StreamType type, const std::string& name);

    /// The tiles parameter is an invalidatetiles: message as sent by the child process
    /// returns true if cache wasn't empty
    bool invalidateTiles(const std::string& tiles, CanonicalViewId canonicalViewId);

    /// Parse invalidateTiles message to rectangle and associated attributes of the invalidated area
    static Util::Rectangle parseInvalidateMsg(const std::string& tiles, int &part, int &mode, TileWireId &wid);

    /// Forget the tile being rendered if it is the latest version we expect.
    void forgetTileBeingRendered(const TileDesc& descForKitReply,
                                 const std::shared_ptr<TileCache::TileBeingRendered>& tileBeingRendered);

    size_t countTilesBeingRenderedForSession(const std::shared_ptr<ClientSession>& session,
                                             std::chrono::steady_clock::time_point now);
    bool hasTileBeingRendered(const TileDesc& tileDesc, const std::chrono::steady_clock::time_point *now = nullptr) const;

    int getTileBeingRenderedVersion(const TileDesc& tileDesc);

    /// Sweep for renderings older than COMMAND_TIMEOUT_MS - typically
    /// caused by a slow or stuck kit. As a side effect:
    ///  - entries whose subscribers have all gone are removed,
    ///  - the start time of returned entries is reset to @p now so the
    ///    very next sweep does not see them as stale again.
    /// Returns the tile descriptors that should be re-issued to the kit.
    std::vector<TileDesc>
    takeStaleRendersForReissue(std::chrono::steady_clock::time_point now);

    /// Set the high watermark for tilecache size
    void setMaxCacheSize(size_t cacheSize);

    /// Get the current memory use.
    size_t getMemorySize() const { return _cacheSize; }

    // Debugging bits ...
    void dumpState(std::ostream& os);
    void setThreadOwner(const ProcUtil::ThreadId id) { _owner = id; }
    void assertCacheSize();

#ifdef BUILDING_TESTS
    /// Test-only: register a tile as being rendered with the given start
    /// time, bypassing the need for a real ClientSession subscriber.
    /// Used to simulate scenarios where the kit has stalled or hung.
    /// If subscriber is non-null, it is added to the rendering's
    /// subscriber list (held as a weak_ptr) so takeStaleRendersForReissue
    /// sees a live subscriber.
    void injectTileBeingRenderedForTest(
        const TileDesc& tile,
        std::chrono::steady_clock::time_point startTime,
        const std::shared_ptr<ClientSession>& subscriber = {});
#endif

private:
    void ensureCacheSize();
    static size_t itemCacheSize(const Tile &tile);

    /// Removes the invalid tiles from the cache
    /// returns true if cache wasn't empty
    bool invalidateTiles(int part, int mode, int x, int y, int width, int height, CanonicalViewId canonicalViewId);

    /// Lookup tile in our cache.
    Tile findTile(const TileDesc &desc);

    static std::string cacheFileName(const TileDesc& tileDesc);
    static bool parseCacheFileName(const std::string& fileName, int& part, int& mode,
                                   int& width, int& height, int& tilePosX, int& tilePosY,
                                   int& tileWidth, int& tileHeight, int& nviewid);

    /// Extract location from fileName, and check if it intersects with [x, y, width, height].
    static bool intersectsTile(const TileDesc &tileDesc, int part, int mode, int x, int y,
                               int width, int height, CanonicalViewId canonicalViewId);

    Tile saveDataToCache(const TileDesc& desc, const char* data, size_t size);
    void saveDataToStreamCache(StreamType type, const std::string& fileName, const char* data,
                               size_t size);

    // old-style file-name to data grab-bag.
    std::map<std::string, Blob> _streamCache[static_cast<int>(StreamType::Last)];

    // FIXME: should we have a tile-desc to WID map instead and a simpler lookup ?
    std::unordered_map<TileDesc, Tile,
                       TileDescCacheHasher,
                       TileDescCacheCompareEq> _cache;
    // FIXME: TileBeingRendered contains TileDesc too ...
    std::unordered_map<TileDesc, std::shared_ptr<TileBeingRendered>,
                       TileDescCacheHasher,
                       TileDescCacheCompareEq> _tilesBeingRendered;

    const std::string _docURL;

    ProcUtil::ThreadId _owner;

    /// Approximate size of tilecache in bytes
    size_t _cacheSize;

    /// Maximum (high watermark) size of the tilecache in bytes
    size_t _maxCacheSize;

    const bool _dontCache;
};

/// Tracks view-port area tiles to track which we last
/// sent to avoid re-sending an existing delta causing grief
class ClientDeltaTracker final
{
    // FIXME: could be a simple 2d TileWireId array for better packing.
    std::unordered_set<TileDesc,
                       TileDescCacheHasher,
                       TileDescCacheCompareEq> _cache;

public:
    // FIXME: only need to store this for the current viewports
    void updateViewPort( /* ... */ )
    {
        // copy the set to another while filtering I guess.
    }

    /// return wire-id of last tile sent - or 0 if not present
    /// update last-tile sent wire-id to curSeq if found.
    TileWireId updateTileSeq(const TileDesc &desc)
    {
        auto it = _cache.find(desc);
        if (it == _cache.end())
        {
            _cache.insert(desc);
            return 0;
        }
        const TileWireId curSeq = desc.getWireId();
        TileWireId last = it->getWireId();
        // id is not included in the hash.
        auto descPointer = const_cast<TileDesc *>(&(*it));
        descPointer->setWireId(curSeq);
        return last;
    }

    void resetTileSeq(const TileDesc &desc)
    {
        auto it = _cache.find(desc);
        if (it == _cache.end())
            return;
        // id is not included in the hash.
        auto descPointer = const_cast<TileDesc *>(&(*it));
        descPointer->setWireId(0);
    }
};

inline std::ostream& operator<< (std::ostream& os, const Tile& tile)
{
    if (!tile)
        os << "nullptr";
    else
        os << "keyframe id " << tile->_wids[0] <<
            " size: " << tile->_deltas.size() <<
            " deltas: " << (tile->_wids.size() - 1);
    return os;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
