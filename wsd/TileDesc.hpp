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
 * Tile descriptor for tile requests and metadata.
 * Classes: TileDesc, TileCombined
 */

#pragma once

#include <common/Protocol.hpp>
#include <common/Rectangle.hpp>
#include <common/StringVector.hpp>
#include <wsd/Exceptions.hpp>

#include <cassert>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>

#define TILE_WIRE_ID
using TileWireId = uint32_t;

namespace TileParse
{
    template <typename A> struct Comp
    {
        bool operator()(const A& av, const std::string_view arg) { return av.first < arg; }
        bool operator()(const std::string_view arg, const A& av) { return arg < av.first; }
        bool operator()(const A& av, const A& bv) { return av.first < bv.first; }
    };

#ifndef NDEBUG
    template <class T, size_t N> bool checkSorted(T const (&args)[N], int maxEnum)
    {
        bool sorted = std::is_sorted(std::begin(args), std::end(args), Comp<T>{});
        for (int i = 0; i < maxEnum; ++i)
        {
            auto range = std::equal_range(std::begin(args), std::end(args), args[i], Comp<T>{});
            assert(range.first != range.second &&                      // is found
                   std::distance(range.first, range.second) == 1 &&    // one match
                   std::distance(std::begin(args), range.first) == i); // match is in correct index
        }
        return sorted;
    }
#endif

    template <class T, size_t N> bool setArg(T (&args)[N], const std::string_view arg, int value)
    {
        auto range = std::equal_range(std::begin(args), std::end(args), arg, Comp<T>{});
        if (range.first == range.second)
            return false;
        range.first->second = value;
        return true;
    }
}

enum class CanonicalViewId : int
{
    Invalid = -1,
    None
};

inline std::ostream& operator<<(std::ostream& os, const CanonicalViewId e)
{
    os << to_underlying(e);
    return os;
}

/// Tile Descriptor
/// Represents a tile's coordinates and dimensions.
class TileDesc final
{
public:
    TileDesc(CanonicalViewId canonicalViewId, int part, int mode, int width, int height, int tilePosX, int tilePosY, int tileWidth,
             int tileHeight, int ver, int imgSize, int id)
        : _canonicalViewId(canonicalViewId)
        , _part(part)
        , _mode(mode)
        , _width(width)
        , _height(height)
        , _tilePosX(tilePosX)
        , _tilePosY(tilePosY)
        , _tileWidth(tileWidth)
        , _tileHeight(tileHeight)
        , _ver(ver)
        , _imgSize(imgSize)
        , _id(id)
        , _oldWireId(0)
        , _wireId(0)
    {
        if (_canonicalViewId <= CanonicalViewId::Invalid ||
            _part < 0 ||
            _mode < 0 ||
            _width <= 0 ||
            _height <= 0 ||
            _tilePosX < 0 ||
            _tilePosY < 0 ||
            _tileWidth <= 0 ||
            _tileHeight <= 0 ||
            _imgSize < 0)
        {
            throw BadArgumentException("Invalid tile descriptor.");
        }
    }

    CanonicalViewId getCanonicalViewId() const { return _canonicalViewId; }
    void setCanonicalViewId(CanonicalViewId canonicalViewId) { _canonicalViewId = canonicalViewId; }
    int getPart() const { return _part; }
    int getEditMode() const { return _mode; }
    int getWidth() const { return _width; }
    int getHeight() const { return _height; }
    int getTilePosX() const { return _tilePosX; }
    int getTilePosY() const { return _tilePosY; }
    int getTileWidth() const { return _tileWidth; }
    int getTileHeight() const { return _tileHeight; }
    int getVersion() const { return _ver; }
    void setVersion(const int ver) { _ver = ver; }
    int getImgSize() const { return _imgSize; }
    void setImgSize(const int imgSize) { _imgSize = imgSize; }
    bool isPreview() const { return _id >= 0; }
    void setId(TileWireId id) { _id = id; }
    void setOldWireId(TileWireId id) { _oldWireId = id; }
    void forceKeyframe() { setOldWireId(0); }
    TileWireId getOldWireId() const { return _oldWireId; }
    bool isForcedKeyFrame() const { return getOldWireId() == 0; }
    void setWireId(TileWireId id) { _wireId = id; }
    TileWireId getWireId() const { return _wireId; }

    bool operator==(const TileDesc& other) const
    {
        return _part == other._part &&
               _width == other._width &&
               _height == other._height &&
               _tilePosX == other._tilePosX &&
               _tilePosY == other._tilePosY &&
               _tileWidth == other._tileWidth &&
               _tileHeight == other._tileHeight &&
               _id == other._id &&
               _canonicalViewId == other._canonicalViewId &&
               _mode == other._mode;
    }

    bool operator!=(const TileDesc& other) const
    {
        return !(*this == other);
    }

    bool compareAsAtTilePos(const TileDesc& other, int tilePosX, int tilePosY) const
    {
        return std::tie(_canonicalViewId, _id,
                        _mode, _part,
                        _height, _width,
                        _tileHeight, _tileWidth,
                        _tilePosY, _tilePosX) <
               std::tie(other._canonicalViewId, other._id,
                        other._mode, other._part,
                        other._height, other._width,
                        other._tileHeight, other._tileWidth,
                        tilePosY, tilePosX);
    }

    // Sort tiles, so they are arranged as ttb rows with ltr cells within rows,
    // with previews at the end.
    bool operator<(const TileDesc& other) const
    {
        return compareAsAtTilePos(other, other._tilePosX, other._tilePosY);
    }

    // used to cache a hash of the key elements compared in ==
    [[nodiscard]] uint32_t equalityHash() const
    {
        uint32_t a = to_underlying(_canonicalViewId) << 17;
        uint32_t b = _tilePosX << 7;

        a ^= _part;
        b ^= _tilePosY;
        a ^= _mode << 30;
        b ^= _tileWidth << 20;
        a ^= _width << 19;

        return a ^ b;
    }

    /// Returns the tile's AABBox, i.e. tile-position + tile-extend
    [[nodiscard]] Util::Rectangle toAABBox() const
    {
        long x2 = getTilePosX();
        if (x2 + getTileWidth() <= std::numeric_limits<int>::max())
        {
            x2 += getTileWidth();
        }
        long y2 = getTilePosY();
        if (y2 + getTileHeight() <= std::numeric_limits<int>::max())
        {
            y2 += getTileHeight();
        }
        return Util::Rectangle::create(getTilePosX(), getTilePosY(), x2, y2);
    }

    /// Returns whether the given rectangle `a` intersects (partially contains) the given rectangle `b`.
    static bool rectanglesIntersect(int ax, int ay, int aw, int ah, int bx, int by, int bw, int bh)
    {
        const Util::Rectangle a = Util::Rectangle::create(ax, ay, ax+aw, ay+ah);
        const Util::Rectangle b = Util::Rectangle::create(bx, by, bx+bw, by+bh);
        return a.intersects(b);
    }

    /// Returns whether this Tile's AABBox intersects (partially contains) given rectangle.
    [[nodiscard]] bool intersectsWithRect(int x, int y, int w, int h) const
    {
        return toAABBox().intersects( Util::Rectangle::create(x, y, x+w, y+h) );
    }

    /// Returns whether this Tile's AABBox intersects (partially contains) given Tile's AABBox.
    [[nodiscard]] bool intersects(const TileDesc& other) const
    {
        return toAABBox().intersects(other.toAABBox());
    }

    /// Returns whether this Tile's AABBox intersects (partially contains) given AABBox.
    [[nodiscard]] bool intersects(const Util::Rectangle& otherAABBox) const
    {
        return toAABBox().intersects(otherAABBox);
    }

    /// Returns whether this Tile's AABBox is fully contained by the given AABBox.
    [[nodiscard]] bool isContained(const Util::Rectangle& otherAABBox) const
    {
        return otherAABBox.contains(toAABBox());
    }

    [[nodiscard]] bool isAdjacent(const TileDesc& other) const
    {
        if (other.getPart() != getPart() ||
            other.getEditMode() != getEditMode() ||
            other.getWidth() != getWidth() ||
            other.getHeight() != getHeight() ||
            other.getTileWidth() != getTileWidth() ||
            other.getTileHeight() != getTileHeight())
        {
            return false;
        }

        return intersects(other);
    }

    // return false if the TileDesc cannot appear in the same TileCombine
    // because their fields differ for the shared tilecombine case
    [[nodiscard]] bool sameTileCombineParams(const TileDesc& other) const
    {
        if (other.getPart() != getPart() ||
            other.getEditMode() != getEditMode() ||
            other.getWidth() != getWidth() ||
            other.getHeight() != getHeight() ||
            other.getTileWidth() != getTileWidth() ||
            other.getTileHeight() != getTileHeight() ||
            other.getCanonicalViewId() != getCanonicalViewId())
        {
            return false;
        }
        return true;
    }

    /// Serialize this instance into a string.
    /// Optionally prepend a prefix.
    [[nodiscard]] std::string serialize(std::string_view prefix = std::string_view(),
                                        std::string_view suffix = std::string_view()) const
    {
        std::ostringstream oss;
        oss << prefix
            << " nviewid=" << _canonicalViewId
            << " part=" << _part
            << " width=" << _width
            << " height=" << _height
            << " tileposx=" << _tilePosX
            << " tileposy=" << _tilePosY
            << " tilewidth=" << _tileWidth
            << " tileheight=" << _tileHeight;
        if (_oldWireId != 0)
            oss << " oldwid=" << _oldWireId;
        if (_wireId != 0)
            oss << " wid=" << _wireId;

        // Anything after ver is optional.
        oss << " ver=" << _ver;

        if (_id >= 0)
        {
            oss << " id=" << _id;
        }

        if (_imgSize > 0)
        {
            oss << " imgsize=" << _imgSize;
        }

        if (_mode)
        {
            oss << " mode=" << _mode;
        }

        oss << suffix;
        return oss.str();
    }

    /// short name for a tile for debugging.
    [[nodiscard]] std::string debugName() const
    {
        std::ostringstream oss;
        oss << '(' << getCanonicalViewId() << ',' << getPart() << ',' << getEditMode() << ',' << getTilePosX() << ',' << getTilePosY() << ')';
        return oss.str();
    }

    /// Deserialize a TileDesc from a tokenized string.
    static TileDesc parse(const StringVector& tokens)
    {
        enum argenum { height, id, imgsize, mode, nviewid, part, tileheight, tileposx, tileposy, tilewidth, ver, width, maxEnum };

        struct TileDescParseResults
        {
            using arg_value = std::pair<const std::string_view, int>;

            arg_value args[maxEnum] = {
                { STRINGIFY(height), 0 },
                { STRINGIFY(id), -1 },          // Optional
                { STRINGIFY(imgsize), 0 },      // Optional
                { STRINGIFY(mode), 0 },         // Optional
                { STRINGIFY(nviewid), 0 },
                { STRINGIFY(part), 0 },
                { STRINGIFY(tileheight), 0 },
                { STRINGIFY(tileposx), 0 },
                { STRINGIFY(tileposy), 0 },
                { STRINGIFY(tilewidth), 0 },
                { STRINGIFY(ver), -1 },         // Optional
                { STRINGIFY(width), 0 }
            };

#ifndef NDEBUG
            TileDescParseResults()
            {
                static bool isSorted = TileParse::checkSorted(args, maxEnum);
                assert(isSorted);
            }
#endif

            bool set(const std::string_view arg, int value)
            {
                return TileParse::setArg(args, arg, value);
            }

            int operator[](argenum arg) const
            {
                return args[arg].second;
            }
        };

        // We don't expect undocumented fields and
        // assume all values to be int.
        TileDescParseResults pairs;

        TileWireId oldWireId = 0;
        TileWireId wireId = 0;
        for (std::size_t i = 0; i < tokens.size(); ++i)
        {
            if (tokens.getUInt32(i, "oldwid", oldWireId))
                ;
            else if (tokens.getUInt32(i, "wid", wireId))
                ;
            else
            {
                std::string name;
                int value = -1;
                if (tokens.getNameIntegerPair(i, name, value))
                    pairs.set(name, value);
            }
        }

        TileDesc result(CanonicalViewId(pairs[nviewid]), pairs[part], pairs[mode],
                        pairs[width], pairs[height],
                        pairs[tileposx], pairs[tileposy],
                        pairs[tilewidth], pairs[tileheight],
                        pairs[ver],
                        pairs[imgsize], pairs[id]);
        result.setOldWireId(oldWireId);
        result.setWireId(wireId);

        return result;
    }

    /// Deserialize a TileDesc from a string format.
    static TileDesc parse(std::string_view message)
    {
        return parse(StringVector::tokenize(message.data(), message.size()));
    }

private:
    CanonicalViewId _canonicalViewId;
    int _part;
    int _mode; ///< Used in Impress for EditMode::(Page|MasterPage), 0 = default
    int _width;
    int _height;
    int _tilePosX;
    int _tilePosY;
    int _tileWidth;
    int _tileHeight;
    int _ver; ///< Versioning support.
    int _imgSize; ///< Used for responses.
    int _id;
    TileWireId _oldWireId;
    TileWireId _wireId;
};

/// One or more tile header.
/// Used to request the rendering of multiple
/// tiles as well as the header of the response.
class TileCombined
{
private:
    TileCombined(CanonicalViewId canonicalViewId, int part, int mode, int width, int height,
                 const std::string& tilePositionsX, const std::string& tilePositionsY,
                 int tileWidth, int tileHeight, const std::string& vers,
                 const std::string& imgSizes,
                 const std::string& oldWireIds,
                 const std::string& wireIds) :
        _canonicalViewId(canonicalViewId),
        _part(part),
        _mode(mode),
        _width(width),
        _height(height),
        _tileWidth(tileWidth),
        _tileHeight(tileHeight),
        _hasWids(false),
        _hasOldWids(false),
        _isCombined(true),
        _hasImgSizes(false)
    {
        if (_part < 0 ||
            _mode < 0 ||
            _width <= 0 ||
            _height <= 0 ||
            _tileWidth <= 0 ||
            _tileHeight <= 0)
        {
            throw BadArgumentException(
                "Invalid tilecombine descriptor. Elements: " + std::to_string(_part) + ' ' +
                std::to_string(_mode) + ' ' + std::to_string(_width) + ' ' +
                std::to_string(_height) + ' ' + std::to_string(_tileWidth) + ' ' +
                std::to_string(_tileHeight));
        }

        StringVector positionXtokens(StringVector::tokenize(tilePositionsX, ','));
        StringVector positionYtokens(StringVector::tokenize(tilePositionsY, ','));
        StringVector imgSizeTokens(StringVector::tokenize(imgSizes, ','));
        StringVector verTokens(StringVector::tokenize(vers, ','));
        StringVector oldWireIdTokens(StringVector::tokenize(oldWireIds, ','));
        StringVector wireIdTokens(StringVector::tokenize(wireIds, ','));

        const std::size_t numberOfPositions = positionXtokens.size();

        // check that the comma-separated strings have the same number of elements
        if (numberOfPositions != positionYtokens.size() ||
            (!imgSizes.empty() && numberOfPositions != imgSizeTokens.size()) ||
            (!vers.empty() && numberOfPositions != verTokens.size()) ||
            (!oldWireIds.empty() && numberOfPositions != oldWireIdTokens.size()) ||
            (!wireIds.empty() && numberOfPositions != wireIdTokens.size()))
        {
            throw BadArgumentException("Invalid tilecombine descriptor. Unequal number of tiles in parameters.");
        }

        for (std::size_t i = 0; i < numberOfPositions; ++i)
        {
            int x = 0;
            if (!COOLProtocol::stringToInteger(positionXtokens[i], x))
            {
                throw BadArgumentException("Invalid 'tileposx' in tilecombine descriptor.");
            }

            int y = 0;
            if (!COOLProtocol::stringToInteger(positionYtokens[i], y))
            {
                throw BadArgumentException("Invalid 'tileposy' in tilecombine descriptor.");
            }

            int imgSize = 0;
            if (imgSizeTokens.empty() || COOLProtocol::stringToInteger(imgSizeTokens[i], imgSize))
                _hasImgSizes = _hasImgSizes || (imgSize != 0);
            else
            {
                throw BadArgumentException("Invalid 'imgsize' in tilecombine descriptor.");
            }

            int ver = -1;
            if (!verTokens.empty() && !verTokens[i].empty() && !COOLProtocol::stringToInteger(verTokens[i], ver))
            {
                throw BadArgumentException("Invalid 'ver' in tilecombine descriptor.");
            }

            TileWireId oldWireId = 0;
            if (oldWireIdTokens.empty() || COOLProtocol::stringToUInt32(oldWireIdTokens[i], oldWireId))
                _hasOldWids = _hasOldWids || (oldWireId != 0);
            else
            {
                throw BadArgumentException("Invalid tilecombine descriptor. oldWireIdToken: " + oldWireIdTokens[i]);
            }

            TileWireId wireId = 0;
            if (wireIdTokens.empty() || COOLProtocol::stringToUInt32(wireIdTokens[i], wireId))
                _hasWids = _hasWids || (wireId != 0);
            else
            {
                throw BadArgumentException("Invalid tilecombine descriptor. wireIdToken: " + wireIdTokens[i]);
            }

            _tiles.emplace_back(_canonicalViewId, _part, _mode, _width, _height, x, y, _tileWidth, _tileHeight, ver, imgSize, -1);
            _tiles.back().setOldWireId(oldWireId);
            _tiles.back().setWireId(wireId);
            _aabbox.extend(_tiles.back().toAABBox());
        }
    }
protected:
    TileCombined() :
        _canonicalViewId(CanonicalViewId::Invalid),
        _part(-1),
        _mode(-1),
        _width(-1),
        _height(-1),
        _tileWidth(-1),
        _tileHeight(-1),
        _hasWids(false),
        _hasOldWids(false),
        _isCombined(false),
        _hasImgSizes(false)
    {
    }

public:
    CanonicalViewId getCanonicalViewId() const { return _canonicalViewId; }
    int getPart() const { return _part; }
    int getEditMode() const { return _mode; }
    int getWidth() const { return _width; }
    int getHeight() const { return _height; }
    int getTileWidth() const { return _tileWidth; }
    int getTileHeight() const { return _tileHeight; }
    /// Returns the combined-tile's AABBox, i.e. min-position + max-extend
    const Util::Rectangle& toAABBox() const { return _aabbox; }
    bool getCombined() const { return _isCombined; }

    const std::vector<TileDesc>& getTiles() const { return _tiles; }

    // for DocumentBroker::handleTileCombinedRequest
    std::vector<TileDesc>& getTiles() { return _tiles; }
    void setHasOldWireId() { _hasOldWids = true; }

    void setCanonicalViewId(CanonicalViewId viewId)
    {
        for (auto& tile : _tiles)
            tile.setCanonicalViewId(viewId);

        _canonicalViewId = viewId;
    }

    [[nodiscard]] bool hasDuplicates() const
    {
        if (_tiles.size() < 2)
            return false;
        for (size_t i = 0; i < _tiles.size() - 1; ++i)
        {
            const auto &a = _tiles[i];
            assert(a.getPart() == _part);
            assert(a.getEditMode() == _mode);
            assert(a.getWidth() == _width);
            assert(a.getHeight() == _height);
            assert(a.getTileWidth() == _tileWidth);
            assert(a.getTileHeight() == _tileHeight);
            for (size_t j = i + 1; j < _tiles.size(); ++j)
            {
                const auto &b = _tiles[j];
                if (a.getTilePosX() == b.getTilePosX() &&
                    a.getTilePosY() == b.getTilePosY())
                    return true;
            }
        }
        return false;
    }

    /// Serialize this instance into a string.
    /// Optionally prepend a prefix.
    [[nodiscard]] std::string serialize(std::string_view prefix = std::string_view(),
                                        std::string_view suffix = std::string_view()) const
    {
        std::ostringstream oss;
        int num = 0;
        oss << prefix
            << " nviewid=" << _canonicalViewId
            << " part=" << _part
            << " width=" << _width
            << " height=" << _height
            << " tileposx=";

        num = 0;
        for (const auto& tile : _tiles)
            oss << (num++ ? "," : "") << tile.getTilePosX();

        oss << " tileposy=";
        num = 0;
        for (const auto& tile : _tiles)
            oss << (num++ ? "," : "") << tile.getTilePosY();

        if (_hasImgSizes)
        {
            oss << " imgsize=";
            num = 0;
            for (const auto& tile : _tiles)
                oss << (num++ ? "," : "") << tile.getImgSize();
        }

        oss << " tilewidth=" << _tileWidth
            << " tileheight=" << _tileHeight;

        oss << " ver=";
        num = 0;
        for (const auto& tile : _tiles)
            oss << (num++ ? "," : "") << tile.getVersion();

        if (_hasOldWids)
        {
            oss << " oldwid=";
            num = 0;
            for (const auto& tile : _tiles)
                oss << (num++ ? "," : "") << tile.getOldWireId();
        }

        if (_hasWids)
        {
            oss << " wid=";
            num = 0;
            for (const auto& tile : _tiles)
                oss << (num++ ? "," : "") << tile.getWireId();
        }

        if (_mode)
            oss << " mode=" << _mode;

        oss << suffix;
        return oss.str();
    }

    /// Deserialize a TileDesc from a tokenized string.
    static TileCombined parse(const StringVector& tokens)
    {
        enum argenum { height, mode, nviewid, part, tileheight, tilewidth, width, maxEnum };

        struct TileCombinedParseResults
        {
            using arg_value = std::pair<const std::string_view, int>;

            arg_value args[maxEnum] = {
                { STRINGIFY(height), 0 },
                { STRINGIFY(mode), 0 },
                { STRINGIFY(nviewid), 0 },
                { STRINGIFY(part), 0 },
                { STRINGIFY(tileheight), 0 },
                { STRINGIFY(tilewidth), 0 },
                { STRINGIFY(width), 0 }
            };

#ifndef NDEBUG
            TileCombinedParseResults()
            {
                static bool isSorted = TileParse::checkSorted(args, maxEnum);
                assert(isSorted);
            }
#endif

            bool set(const std::string_view arg, int value)
            {
                return TileParse::setArg(args, arg, value);
            }

            int operator[](argenum arg) const
            {
                return args[arg].second;
            }
        };

        // We don't expect undocumented fields and
        // assume all values to be int.
        TileCombinedParseResults pairs;

        std::string tilePositionsX;
        std::string tilePositionsY;
        std::string imgSizes;
        std::string versions;
        std::string oldwireIds;
        std::string wireIds;

        for (const auto& token : tokens)
        {
            std::string name;
            std::string value;
            if (COOLProtocol::parseNameValuePair(tokens.getParam(token), name, value))
            {
                if (name == "tileposx")
                {
                    tilePositionsX = std::move(value);
                }
                else if (name == "tileposy")
                {
                    tilePositionsY = std::move(value);
                }
                else if (name == "imgsize")
                {
                    imgSizes = std::move(value);
                }
                else if (name == "ver")
                {
                    versions = std::move(value);
                }
                else if (name == "oldwid")
                {
                    oldwireIds = std::move(value);
                }
                else if (name == "wid")
                {
                    wireIds = std::move(value);
                }
                else
                {
                    int v = 0;
                    if (COOLProtocol::stringToInteger(value, v))
                    {
                        pairs.set(name, v);
                    }
                }
            }
        }

        return TileCombined(CanonicalViewId(pairs[nviewid]),
                            pairs[part], pairs[mode],
                            pairs[width], pairs[height],
                            tilePositionsX, tilePositionsY,
                            pairs[tilewidth], pairs[tileheight],
                            versions, imgSizes, oldwireIds, wireIds);
    }

    /// Deserialize a TileDesc from a string format.
    static TileCombined parse(std::string_view message)
    {
        return parse(StringVector::tokenize(message.data(), message.size()));
    }

    static TileCombined create(const std::vector<TileDesc>& tiles)
    {
        assert(!tiles.empty());

        std::ostringstream xs;
        std::ostringstream ys;
        std::ostringstream vers;
        std::ostringstream oldhs;
        std::ostringstream hs;

        for (const auto& tile : tiles)
        {
            xs << tile.getTilePosX() << ',';
            ys << tile.getTilePosY() << ',';
            vers << tile.getVersion() << ',';
            oldhs << tile.getOldWireId() << ',';
            hs << tile.getWireId() << ',';
        }

        vers.seekp(-1, std::ios_base::cur); // Remove last comma.
        return TileCombined(tiles[0].getCanonicalViewId(), tiles[0].getPart(), tiles[0].getEditMode(),
                            tiles[0].getWidth(), tiles[0].getHeight(),
                            xs.str(), ys.str(), tiles[0].getTileWidth(), tiles[0].getTileHeight(),
                            vers.str(), "", oldhs.str(), hs.str());
    }

    /// Split a heterogeneous set of tiles into homogeneous tilecombines.
    /// create() takes its part, size and canonicalViewId from tiles[0] and
    /// only concatenates positions, so every member must already share those
    /// params or two tiles from different views collide on one position. Bucket
    /// by sameTileCombineParams so each returned combine is internally uniform.
    static std::vector<TileCombined> createGroups(const std::vector<TileDesc>& tilesNeedsRendering)
    {
        std::vector<TileCombined> combines;
        if (tilesNeedsRendering.empty())
            return combines;

        std::vector<std::vector<TileDesc>> groupsNeedsRendering(1);
        auto it = tilesNeedsRendering.begin();
        // start off with one group bucket
        groupsNeedsRendering[0].push_back(*it++);
        while (it != tilesNeedsRendering.end())
        {
            bool inserted = false;
            // check if tile should go into an existing group bucket
            for (size_t i = 0; i < groupsNeedsRendering.size(); ++i)
            {
                if (it->sameTileCombineParams(groupsNeedsRendering[i][0]))
                {
                    groupsNeedsRendering[i].push_back(*it);
                    inserted = true;
                    break;
                }
            }
            // if not, add another and put it there
            if (!inserted)
            {
                groupsNeedsRendering.emplace_back();
                groupsNeedsRendering.back().push_back(*it);
            }
            ++it;
        }

        combines.reserve(groupsNeedsRendering.size());
        for (const auto& group : groupsNeedsRendering)
            combines.push_back(create(group));
        return combines;
    }

    void initFrom(const TileDesc &desc)
    {
        _part = desc.getPart();
        _mode = desc.getEditMode();
        _width = desc.getWidth();
        _height = desc.getHeight();
        _tileWidth = desc.getTileWidth();
        _tileHeight = desc.getTileHeight();
        _canonicalViewId = desc.getCanonicalViewId();
        _tiles.push_back(desc);
        _isCombined = false;
        _hasWids = desc.getWireId() != 0;
        _hasOldWids = desc.getOldWireId() != 0;
        _hasImgSizes = desc.getImgSize() != 0;
    }

    /// To support legacy / under-used renderTile
    explicit TileCombined(const TileDesc &desc)
    {
        initFrom(desc);
    }

protected:
    std::vector<TileDesc> _tiles;
    Util::Rectangle _aabbox;
    CanonicalViewId _canonicalViewId;
    int _part;
    int _mode;
    int _width;
    int _height;
    int _tileWidth;
    int _tileHeight;
    bool _hasWids : 1;
    bool _hasOldWids : 1;
    bool _isCombined : 1;
    bool _hasImgSizes : 1;
};

class TileCombinedBuilder final : public TileCombined
{
public:
    TileCombinedBuilder() = default;

    void pushRendered(const TileDesc &desc, TileWireId wireId, size_t imgSize)
    {
        // uninitialized
        if (_part < 0 && _mode < 0 && _width <= 0)
            initFrom(desc);
        else
            _tiles.push_back(desc);

        _tiles.back().setWireId(wireId);
        _hasWids = true;

        _tiles.back().setImgSize(imgSize);
        _hasImgSizes = true;

        _isCombined = _tiles.size() > 1;
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
