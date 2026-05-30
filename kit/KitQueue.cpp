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
 * Kit process callback queue management and optimization.
 * Classes: KitQueue, Callback - COKit callback handling and deduplication
 */

#include <config.h>

#include "KitQueue.hpp"

#include <common/JsonUtil.hpp>

#include <algorithm>
#include <climits>
#include <cstring>
#include <iostream>
#include <string>
#include <string_view>

/* static */ std::string KitQueue::Callback::toString(int view, int type,
                                                      const std::string& payload)
{
    std::ostringstream str;
    str << view << ' ' << kitCallbackTypeToString(type) << ' '
        << COOLProtocol::getAbbreviatedMessage(payload);
    return str.str();
}

namespace {
    bool textItem(const KitQueue::Payload &value, const std::string &firstToken, bool &removeText)
    {
        size_t offset = firstToken.size(); // in this case a session
        if (value.size() < offset + 3)
            return false;

        size_t remaining = value.size() - firstToken.size();

        if (!memcmp(value.data() + offset + 1, "textinput", std::min(remaining - 1, size_t(9))))
        {
            removeText = false;
            return true;
        }

        if (!memcmp(value.data() + offset + 1, "removetextcontext", std::min(remaining - 1, size_t(17))))
        {
            removeText = true;
            return true;
        }
        return false;
    }
}

void KitQueue::put(const Payload& value)
{
    if (value.empty())
        throw std::runtime_error("Cannot queue empty item.");

    const std::string firstToken = COOLProtocol::getFirstToken(value);

    bool removeText = false;

    if (firstToken == "tilecombine")
        pushTileCombineRequest(value);

    else if (firstToken == "tile")
        pushTileQueue(value);

    else if (firstToken == "callback")
        assert(false && "callbacks should not come from the client");

    else if (textItem(value, firstToken, removeText))
    {
        StringVector tokens = StringVector::tokenize(value.data(), value.size());

        std::string newMsg = !removeText ? combineTextInput(tokens)
            : combineRemoveText(tokens);

        if (!newMsg.empty())
            _queue.emplace_back(newMsg.data(), newMsg.data() + newMsg.size());
        else
            _queue.emplace_back(value);
    }
    else // not so special
        _queue.emplace_back(value);
}

std::vector<TileDesc>& KitQueue::ensureTileQueue(CanonicalViewId viewid)
{
    for (auto& queue : _tileQueues)
    {
        if (queue.first == viewid)
            return queue.second;
    }

    // Create and return a new one.
    return _tileQueues.emplace_back(viewid, std::vector<TileDesc>()).second;
}

namespace {

/// Read the viewId from the payload.
std::string extractViewId(const std::string& payload)
{
    Poco::JSON::Parser parser;
    const Poco::Dynamic::Var result = parser.parse(payload);
    const auto& json = result.extract<Poco::JSON::Object::Ptr>();
    return json->get("viewId").toString();
}

/// Extract rectangle from the invalidation callback payload
bool extractRectangle(const StringVector& tokens, int& x, int& y, int& w, int& h, int& part, int& mode)
{
    x = 0;
    y = 0;
    w = INT_MAX;
    h = INT_MAX;
    part = 0;
    mode = 0;

    if (tokens.size() < 2)
        return false;

    if (tokens.equals(0, "EMPTY,"))
    {
        part = std::atoi(tokens[1].c_str());
        if (tokens.size() > 2)
            mode = std::atoi(tokens[2].c_str());
        return true;
    }

    if (tokens.size() < 5)
        return false;

    x = std::atoi(tokens[0].c_str());
    y = std::atoi(tokens[1].c_str());
    w = std::atoi(tokens[2].c_str());
    h = std::atoi(tokens[3].c_str());
    part = std::atoi(tokens[4].c_str());

    if (tokens.size() == 6)
        mode = std::atoi(tokens[5].c_str());

    return true;
}

}

void KitQueue::putCallback(int view, int type, const std::string &payload)
{
    if (!elideDuplicateCallback(view, type, payload))
        _callbacks.emplace_back(view, type, payload);
}

bool KitQueue::elideDuplicateCallback(int view, int type, const std::string &payload)
{
    const auto callbackType = static_cast<COKitCallbackType>(type);

    // Nothing to combine in this case:
    if (_callbacks.empty())
        return false;

    switch (callbackType)
    {
        case KIT_CALLBACK_INVALIDATE_TILES: // invalidation
        {
            StringVector tokens = StringVector::tokenize(payload);

            int msgX, msgY, msgW, msgH, msgPart, msgMode;
            if (!extractRectangle(tokens, msgX, msgY, msgW, msgH, msgPart, msgMode))
                return false;

            bool performedMerge = false;

            // we always travel the entire queue
            std::size_t i = 0;
            while (i < _callbacks.size())
            {
                auto& it = _callbacks[i];

                if (it._type != type || it._view != view)
                {
                    ++i;
                    continue;
                }
                StringVector queuedTokens = StringVector::tokenize(it._payload);

                int queuedX, queuedY, queuedW, queuedH, queuedPart, queuedMode;

                if (!extractRectangle(queuedTokens, queuedX, queuedY, queuedW, queuedH, queuedPart, queuedMode))
                {
                    ++i;
                    continue;
                }

                if (msgPart != queuedPart)
                {
                    ++i;
                    continue;
                }

                if (msgMode != queuedMode)
                {
                    ++i;
                    continue;
                }

                // the invalidation in the queue is fully covered by the payload,
                // just remove it
                if (msgX <= queuedX && queuedX + queuedW <= msgX + msgW && msgY <= queuedY
                    && queuedY + queuedH <= msgY + msgH)
                {
                    LOG_TRC("Removing smaller invalidation: "
                            << it._payload << " -> " << ' ' << msgX << ' ' << msgY << ' '
                            << msgW << ' ' << msgH << ' ' << msgPart << ' ' << msgMode);

                    // remove from the queue
                    _callbacks.erase(_callbacks.begin() + i);
                    continue;
                }

                // the invalidation just intersects, join those (if the result is
                // small)
                if (TileDesc::rectanglesIntersect(msgX, msgY, msgW, msgH, queuedX, queuedY, queuedW,
                                                  queuedH))
                {
                    int joinX = std::min(msgX, queuedX);
                    int joinY = std::min(msgY, queuedY);
                    int joinW = std::max(msgX + msgW, queuedX + queuedW) - joinX;
                    int joinH = std::max(msgY + msgH, queuedY + queuedH) - joinY;

                    const int reasonableSizeX = 4 * 3840; // 4x tile at 100% zoom
                    const int reasonableSizeY = 2 * 3840; // 2x tile at 100% zoom
                    if (joinW > reasonableSizeX || joinH > reasonableSizeY)
                    {
                        ++i;
                        continue;
                    }

                    LOG_TRC("Merging invalidations: "
                            << Callback::toString(view, type, payload) << " and "
                            << tokens[0] << ' ' << tokens[1] << ' ' << tokens[2] << ' '
                            << msgX << ' ' << msgY << ' ' << msgW << ' ' << msgH << ' '
                            << msgPart << ' ' << msgMode << " -> "
                            << tokens[0] << ' ' << tokens[1] << ' ' << tokens[2] << ' '
                            << joinX << ' ' << joinY << ' ' << joinW << ' ' << joinH << ' '
                            << msgPart << ' ' << msgMode);

                    msgX = joinX;
                    msgY = joinY;
                    msgW = joinW;
                    msgH = joinH;
                    performedMerge = true;

                    // remove from the queue
                    _callbacks.erase(_callbacks.begin() + i);
                    continue;
                }

                ++i;
            }

            if (performedMerge)
            {
                std::string newPayload =
                    std::to_string(msgX) + ", " + std::to_string(msgY) + ", " +
                    std::to_string(msgW) + ", " + std::to_string(msgH) + ", " +
                    tokens.cat(' ', 4); // part etc. ...

                LOG_TRC("Merge result: " << newPayload);

                _callbacks.emplace_back(view, type, newPayload);
                return true; // elide the original - use this instead
            }
        }
        break;

        case KIT_CALLBACK_STATE_CHANGED: // state changed
        {
            constexpr std::string_view unoPrefix(".uno:");
            if (!payload.starts_with(unoPrefix))
                return false;

            // Only elide .uno commands that have a value.
            const size_t equalPos = payload.find('=', unoPrefix.size());
            if (equalPos == std::string::npos)
                return false;

            const std::string_view unoCommand = std::string_view(payload).substr(0, equalPos);

            // This is needed because otherwise it creates some problems when
            // a save occurs while a cell is still edited in Calc.
            if (unoCommand == ".uno:ModifiedStatus")
                return false;

            // remove obsolete states of the same .uno: command
            const size_t unoCommandLen = unoCommand.size();
            for (size_t i = 0; i < _callbacks.size(); ++i)
            {
                const Callback& it = _callbacks[i];
                if (it._type != type || it._view != view)
                    continue;

                // Skip if the current callback payload doesn't start with '<unoCommand>='.
                if (it._payload.size() < unoCommandLen + 1 || it._payload[unoCommandLen] != '=' ||
                    !it._payload.starts_with(unoCommand))
                    continue;

                LOG_TRC("Remove obsolete uno command: " << it << " -> "
                        << Callback::toString(view, type, payload));
                _callbacks.erase(_callbacks.begin() + i);
                break;
            }
        }
        break;

        case KIT_CALLBACK_INVALIDATE_VISIBLE_CURSOR: // the cursor has moved
        case KIT_CALLBACK_CURSOR_VISIBLE: // the cursor visibility has changed
        case KIT_CALLBACK_STATUS_INDICATOR_SET_VALUE: // setting the indicator value
        case KIT_CALLBACK_DOCUMENT_SIZE_CHANGED: // setting the document size
        case KIT_CALLBACK_CELL_CURSOR: // the cell cursor has moved
        case KIT_CALLBACK_INVALIDATE_VIEW_CURSOR: // the view cursor has moved
        case KIT_CALLBACK_CELL_VIEW_CURSOR: // the view cell cursor has moved
        case KIT_CALLBACK_VIEW_CURSOR_VISIBLE: // the view cursor visibility has changed
        {
            const bool isViewCallback = (callbackType == KIT_CALLBACK_INVALIDATE_VIEW_CURSOR ||
                                         callbackType == KIT_CALLBACK_CELL_VIEW_CURSOR ||
                                         callbackType == KIT_CALLBACK_VIEW_CURSOR_VISIBLE);

            const std::string viewId
                = (isViewCallback ? extractViewId(payload) : std::string());

            for (std::size_t i = 0; i < _callbacks.size(); ++i)
            {
                const auto& it = _callbacks[i];

                if (it._type != type || it._view != view)
                    continue;

                if (!isViewCallback)
                {
                    LOG_TRC("Remove obsolete callback: " << it <<  " -> "
                            << Callback::toString(view, type, payload));
                    _callbacks.erase(_callbacks.begin() + i);
                    break;
                }
                else if (isViewCallback)
                {
                    // we additionally need to ensure that the payload is about
                    // the same viewid (otherwise we'd merge them all views into
                    // one)
                    const std::string queuedViewId = extractViewId(it._payload);
                    if (viewId == queuedViewId)
                    {
                        LOG_TRC("Remove obsolete view callback: " << it << " -> "
                                << Callback::toString(view, type, payload));
                        _callbacks.erase(_callbacks.begin() + i);
                        break;
                    }
                }
            }
        }
        break;

        default:
        break;

    } // switch

    // Append the new command to the callbacks list
    return false;
}

KitQueue::Payload KitQueue::pop()
{
    if (_queue.empty())
        return Payload();

    Payload front = _queue.front();

    LOG_TRC("KitQueue(" << _queue.size() << ") - pop " <<
            COOLProtocol::getAbbreviatedMessage(front));

    _queue.erase(_queue.begin());

    return front;
}

TileCombined KitQueue::popTileQueue(TilePrioritizer::Priority &priority)
{
    assert(!isTileQueueEmpty());

    std::vector<TileDesc>* tileQueue(nullptr);

    // canonical viewIds in order of least inactive to highest
    auto viewIdsByPrio = _prio.getViewIdsByInactivity();

    // find which queue has tiles for the least inactive canonical viewId
    for (const auto& viewIdEntry : viewIdsByPrio)
    {
        auto found = std::find_if(_tileQueues.begin(), _tileQueues.end(),
                                  [viewIdEntry](const auto& queue) {
                                    return viewIdEntry.first == queue.first && !queue.second.empty();
                                  });
        if (found != _tileQueues.end())
        {
            tileQueue = &found->second;
            break;
        }
    }

    // This shouldn't happen, but if it does, pick something to pop from
    if (!tileQueue)
    {
        LOG_ERR("No existing session for any tiles in queue.");
        for (auto& queue : _tileQueues)
        {
            if (queue.second.empty())
                continue;
            tileQueue = &queue.second;
            break;
        }
    }

    assert(tileQueue);

    return popTileQueue(*tileQueue, priority);
}

namespace {

    // If these can appear in the same tile combine
    bool allowCombine(const TileDesc& a, const TileDesc& b)
    {
        if (a.isPreview() || b.isPreview())
            return false;
        return a.sameTileCombineParams(b);
    }

    // If candidate can appear in the same tile combine as prioTile and is
    // positioned at the same Y pos as tilePosY
    bool allowCombineIfRow(const TileDesc& prioTile,
                           int tilePosY,
                           const TileDesc& candidate,
                           std::string_view check)
    {
        bool res = tilePosY == candidate.getTilePosY() &&
                   allowCombine(prioTile, candidate);
        LOG_TRC("Combining candidate: " << candidate.serialize() <<
                " x-grid=" << candidate.getTilePosX() / candidate.getTileWidth() <<
                " y-grid-" << candidate.getTilePosY() / candidate.getTileHeight() <<
                (res ? " accepted" : " rejected") << ' ' << check);
        return res;
    }

    void sortedInsert(std::vector<TileDesc>& tileQueue, const TileDesc& tile)
    {
        const auto it = std::lower_bound(tileQueue.begin(), tileQueue.end(), tile);
        const bool duplicate = it != tileQueue.end() && tile == *it;
        if (duplicate)
        {
            // We discard the earlier dup in favour of this new one
            LOG_TRC("Remove duplicate tile request: " << it->serialize() <<
                    " -> " << tile.serialize());
            *it = tile;
            return;
        }
        tileQueue.insert(it, tile);
    }

    constexpr int maxCombinedGridSpan = 16;

    std::pair<int, int> combineHoriAround(std::vector<TileDesc>& tileQueue,
                                          std::vector<TileDesc>& destTiles, int tilePos)
    {
        const TileDesc& prioTile = tileQueue[tilePos];
        const int tileWidth = prioTile.getTileWidth();

        const int prioGridPos = prioTile.getTilePosX() / tileWidth;

        int leftGridX = prioGridPos;
        int rightGridX = prioGridPos;

        int leftTile = tilePos - 1;
        int rightTile = tilePos + 1;

        int tilePosY = prioTile.getTilePosY();

        // Starting at the (guaranteed to exist) priority tile check in the
        // sorted tileQueue to its left and right along the same row for
        // candidate tiles that can be combined into the same request.
        // But limit the request at maxCombinedGridSpan wide.
        while (true)
        {
            const bool canCombineLeft = leftTile > -1 &&
                allowCombineIfRow(prioTile, tilePosY, tileQueue[leftTile], "same row, left");
            const bool canCombineRight = static_cast<unsigned>(rightTile) < tileQueue.size() &&
                allowCombineIfRow(prioTile, tilePosY, tileQueue[rightTile], "same row, right");
            if (!canCombineLeft && !canCombineRight)
                break;

            int leftGridDistance = INT_MAX;
            int leftProposedGridX = leftGridX;
            if (canCombineLeft)
            {
                leftProposedGridX = tileQueue[leftTile].getTilePosX() / tileWidth;
                leftGridDistance = prioGridPos - leftProposedGridX;
            }

            int rightGridDistance = INT_MAX;
            int rightProposedGridX = rightGridX;
            if (canCombineRight)
            {
                rightProposedGridX = tileQueue[rightTile].getTilePosX() / tileWidth;
                rightGridDistance = rightProposedGridX - prioGridPos;
            }

            int combineCandidate;
            if (leftGridDistance < rightGridDistance)
            {
                if (rightGridX - leftProposedGridX >= maxCombinedGridSpan)
                    break;

                leftGridX = leftProposedGridX;
                combineCandidate = leftTile--;
            }
            else
            {
                if (rightProposedGridX - leftGridX >= maxCombinedGridSpan)
                    break;

                rightGridX = rightProposedGridX;
                combineCandidate = rightTile++;
            }

            sortedInsert(destTiles, tileQueue[combineCandidate]);
        }

        // erase from source all the dest inserted tiles in one fell swoop
        tileQueue.erase(tileQueue.begin() + (leftTile + 1), tileQueue.begin() + rightTile);

        return std::make_pair(leftGridX, rightGridX);
    }

    // To stand in for a tile otherwise the same as prioTile, but at an
    // alternative position
    struct SpeculativeTileDesc
    {
        const TileDesc& _prioTile;
        const int _tilePosX;
        const int _tilePosY;

        SpeculativeTileDesc(const TileDesc& prioTile, int leftGridX, int vertDirection)
            : _prioTile(prioTile)
            , _tilePosX(leftGridX * prioTile.getTileWidth())
            , _tilePosY(prioTile.getTilePosY() + (prioTile.getTileHeight() * vertDirection))
        {
        }
    };

    bool operator<(const TileDesc& candidate, const SpeculativeTileDesc& other)
    {
        return candidate.compareAsAtTilePos(other._prioTile,
                                            other._tilePosX,
                                            other._tilePosY);
    }

    void combineVerticalTiles(std::vector<TileDesc>& tileQueue,
                              std::vector<TileDesc>& destTiles,
                              const TileDesc& prioTile,
                              int leftGridX, int rightGridX,
                              int vertDirection)
    {
        SpeculativeTileDesc anchorTile(prioTile, leftGridX, vertDirection);

        auto it = std::lower_bound(tileQueue.begin(), tileQueue.end(), anchorTile);
        while (it != tileQueue.end())
        {
            if (!allowCombineIfRow(prioTile, anchorTile._tilePosY, *it,
                                   vertDirection > 0 ? "row below" : "row above"))
            {
                break;
            }

            const int gridX = it->getTilePosX() / it->getTileWidth();
            if (gridX >= leftGridX && gridX <= rightGridX)
            {
                sortedInsert(destTiles, *it);
                it = tileQueue.erase(it);
            }
            else
                ++it;
        }
    }
}

TileCombined KitQueue::popTileQueue(std::vector<TileDesc>& tileQueue, TilePrioritizer::Priority &priority)
{
    assert(!tileQueue.empty());

    LOG_TRC("KitQueue depth: " << tileQueue.size());

    // vector of tiles we will render
    std::vector<TileDesc> tiles;

    // We are handling a tile; first try to find one that is at the cursor's
    // position, otherwise handle the one that is at the front
    int prioritized = 0;
    priority = _prio.getTilePriority(tileQueue[0]);
    for (std::size_t i = 1; i < tileQueue.size(); ++i)
    {
        const auto& tile = tileQueue[i];

        const TilePrioritizer::Priority p = _prio.getTilePriority(tile);
        if (p > priority)
        {
            priority = p;
            prioritized = static_cast<int>(i);
        }
    }

    // Among equal priority tiles advance past the last row we serviced, only
    // falling back to that front tile once we run off the end, so one busy row
    // cannot hold the front of a shared queue.
    for (std::size_t i = 0; i < tileQueue.size(); ++i)
    {
        if (_prio.getTilePriority(tileQueue[i]) == priority &&
            tileQueue[i].getTilePosY() > _lastServicedPosY)
        {
            prioritized = static_cast<int>(i);
            break;
        }
    }
    _lastServicedPosY = tileQueue[prioritized].getTilePosY();

    const TileDesc msg = tileQueue[prioritized];

    LOG_TRC("Priority tile: " << msg.serialize() <<
            " x-grid=" << msg.getTilePosX() / msg.getTileWidth() <<
            " y-grid=" << msg.getTilePosY() / msg.getTileHeight());

    // and add it to the render list
    tiles.emplace_back(msg);

    if (msg.isPreview())
    {
        // Short circuit previews, which don't combine
        tileQueue.erase(tileQueue.begin() + prioritized);
    }
    else
    {
        // Combine as many horizontal tiles as sensible with this tile, return the populated range,
        auto [ leftGridX, rightGridX ] = combineHoriAround(tileQueue, tiles, prioritized);

        int gridSpan = rightGridX - leftGridX;
        // Distribute available max sensible window width to collect potential
        // additional candidates in neighbour rows
        if (gridSpan < maxCombinedGridSpan)
        {
            int availSpans = maxCombinedGridSpan - gridSpan;
            int leftExpand = std::min(leftGridX, availSpans / 2);
            int rightExpand = availSpans - leftExpand;
            leftGridX -= leftExpand;
            rightGridX += rightExpand;
        }

        // Combine tiles vertically adjacent to this range
        combineVerticalTiles(tileQueue, tiles, msg, leftGridX, rightGridX, -1);
        combineVerticalTiles(tileQueue, tiles, msg, leftGridX, rightGridX, +1);
    }

    LOG_TRC("Combined " << tiles.size() << " tiles, leaving " << tileQueue.size() << " in queue.");

    if (tiles.size() == 1)
    {
        LOG_TRC("KitQueue res: " << tiles[0].serialize());
        return TileCombined(tiles[0]);
    }

    TileCombined combined = TileCombined::create(tiles);
    assert(!combined.hasDuplicates());
    LOG_TRC("KitQueue res: " << combined.serialize());
    return combined;
}

std::string KitQueue::combineTextInput(const StringVector& tokens)
{
    std::string id;
    std::string text;
    if (!COOLProtocol::getTokenString(tokens, "id", id) ||
        !COOLProtocol::getTokenString(tokens, "text", text))
        return std::string();

    int i = _queue.size() - 1;
    while (i >= 0)
    {
        auto& it = _queue[i];

        const std::string queuedMessage(it.data(), it.size());
        StringVector queuedTokens = StringVector::tokenize(it.data(), it.size());

        // If any messages of these types are present before the current ("textinput") message,
        // no combination is possible.
        if (queuedTokens.size() == 1 ||
            (queuedTokens.equals(0, tokens, 0) &&
             (queuedTokens.equals(1, "key") ||
              queuedTokens.equals(1, "mouse") ||
              queuedTokens.equals(1, "removetextcontext") ||
              queuedTokens.equals(1, "windowkey"))))
            return std::string();

        std::string queuedId;
        std::string queuedText;
        if (queuedTokens.equals(0, tokens, 0) &&
            queuedTokens.equals(1, "textinput") &&
            COOLProtocol::getTokenString(queuedTokens, "id", queuedId) &&
            queuedId == id &&
            COOLProtocol::getTokenString(queuedTokens, "text", queuedText))
        {
            // Remove the queued textinput message and combine it with the current one
            _queue.erase(_queue.begin() + i);

            std::string newMsg;
            newMsg.reserve(it.size() * 2);
            newMsg.append(queuedTokens[0]);
            newMsg.append(" textinput id=");
            newMsg.append(id);
            newMsg.append(" text=");
            newMsg.append(queuedText);
            newMsg.append(text);

            LOG_TRC("Combined [" << queuedMessage << "] with current message to [" << newMsg
                    << ']');

            return newMsg;
        }

        --i;
    }

    return std::string();
}

void KitQueue::pushTileCombineRequest(const Payload &value)
{
    assert(COOLProtocol::getFirstToken(value) == "tilecombine");

    // Breakup tilecombine and deduplicate (we are re-combining
    // the tiles inside popTileQueue() again)
    const std::string_view msg(value.data(), value.size());
    const TileCombined tileCombined = TileCombined::parse(msg);

    std::vector<TileDesc>& tileQueue = ensureTileQueue(tileCombined.getCanonicalViewId());
    const std::vector<TileDesc>& tiles = tileCombined.getTiles();
    tileQueue.reserve(tileQueue.size() + tiles.size());
    for (const auto& tile : tiles)
        sortedInsert(tileQueue, tile);
}

void KitQueue::pushTileQueue(const Payload &value)
{
    const std::string_view msg(value.data(), value.size());
    const TileDesc desc = TileDesc::parse(msg);
    std::vector<TileDesc>& tileQueue = ensureTileQueue(desc.getCanonicalViewId());
    sortedInsert(tileQueue, desc);
}

size_t KitQueue::getTileQueueSize() const
{
    size_t queuedTiles(0);

    for (const auto& queue : _tileQueues)
        queuedTiles += queue.second.size();

    return queuedTiles;
}

std::string KitQueue::combineRemoveText(const StringVector& tokens)
{
    std::string id;
    int before = 0;
    int after = 0;
    if (!COOLProtocol::getTokenString(tokens, "id", id) ||
        !COOLProtocol::getTokenInteger(tokens, "before", before) ||
        !COOLProtocol::getTokenInteger(tokens, "after", after))
        return std::string();

    int i = _queue.size() - 1;
    while (i >= 0)
    {
        auto& it = _queue[i];

        const std::string queuedMessage(it.data(), it.size());
        StringVector queuedTokens = StringVector::tokenize(it.data(), it.size());

        // If any messages of these types are present before the current (removetextcontext)
        // message, no combination is possible.
        if (queuedTokens.size() == 1 ||
            (queuedTokens.equals(0, tokens, 0) &&
             (queuedTokens.equals(1, "key") ||
              queuedTokens.equals(1, "mouse") ||
              queuedTokens.equals(1, "textinput") ||
              queuedTokens.equals(1, "windowkey"))))
            return std::string();

        std::string queuedId;
        int queuedBefore = 0;
        int queuedAfter = 0;
        if (queuedTokens.equals(0, tokens, 0) &&
            queuedTokens.equals(1, "removetextcontext") &&
            COOLProtocol::getTokenStringFromMessage(queuedMessage, "id", queuedId) &&
            queuedId == id &&
            COOLProtocol::getTokenIntegerFromMessage(queuedMessage, "before", queuedBefore) &&
            COOLProtocol::getTokenIntegerFromMessage(queuedMessage, "after", queuedAfter))
        {
            // Remove the queued removetextcontext message and combine it with the current one
            _queue.erase(_queue.begin() + i);

            std::string newMsg = queuedTokens[0] + " removetextcontext id=" + id +
                " before=" + std::to_string(queuedBefore + before) +
                " after=" + std::to_string(queuedAfter + after);

            LOG_TRC("Combined [" << queuedMessage << "] with current message to [" << newMsg << "]");

            return newMsg;
        }

        --i;
    }

    return std::string();
}

void KitQueue::dumpState(std::ostream& oss)
{
    oss << "\tIncoming Queue size: " << _queue.size() << "\n";
    size_t i = 0;
    for (const Payload &it : _queue)
        oss << "\t\t" << i++ << ": " << COOLProtocol::getFirstLine(it) << "\n";

    oss << "\tTile Queues count: " << _tileQueues.size() << "\n";
    for (const auto& queue : _tileQueues)
    {
        CanonicalViewId viewId = queue.first;
        const std::vector<TileDesc>& tileQueue = queue.second;
        oss << "\t\tTile Queue size: " << tileQueue.size() << " viewId: " << viewId << "\n";
        i = 0;
        for (const TileDesc &it : tileQueue)
            oss << "\t\t\t" << i++ << ": " << it.serialize() << "\n";
    }

    oss << "\tCallbacks size: " << _callbacks.size() << "\n";
    i = 0;
    for (const auto &it : _callbacks)
        oss << "\t\t" << i++ << ": " << it << "\n";
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
