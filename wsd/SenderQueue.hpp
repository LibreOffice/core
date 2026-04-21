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
 * Message queue for sending data to clients.
 * Classes: SenderQueue
 */

#pragma once

#include <common/Log.hpp>
#include <common/Protocol.hpp>
#include <common/SigUtil.hpp>
#include <wsd/TileDesc.hpp>

#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>

#include <deque>
#include <mutex>

/// A queue of data to send to certain Session's WS.
template <typename Item>
class SenderQueue final
{
public:
    SenderQueue() = default;

    /// Enqueues an item, if not a duplicate and not shutting down.
    /// Returns true if enqueued.
    bool enqueue(const Item& item)
    {
        if (!SigUtil::getTerminationFlag())
        {
            std::unique_lock<std::mutex> lock(_mutex);
            if (deduplicate(item))
            {
                _queue.push_back(item);
                return true;
            }
        }

        return false;
    }

    /// Dequeue an item if we have one - @returns true if we do, else false.
    bool dequeue(Item& item)
    {
        // This check is always thread-safe.
        if (SigUtil::getTerminationFlag())
        {
            LOG_DBG("SenderQueue: TerminationFlag is set, will not dequeue");
            return false;
        }

        std::unique_lock<std::mutex> lock(_mutex);

        if (!_queue.empty())
        {
            item = _queue.front();
            _queue.pop_front();
            return true;
        }

        return false;
    }

    size_t size() const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _queue.size();
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _queue.empty();
    }

    void dumpState(std::ostream& os)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        size_t queueSize = _queue.size();
        size_t totalSize = 0;

        os << "\t\tqueue items: " << queueSize << '\n';

        std::size_t repeats = 0;
        std::string lastStr;
        for (const Item &item : _queue)
        {
            std::string itemStr = COOLProtocol::getAbbreviatedMessage(
                item->data().data(), item->size());
            if (lastStr == itemStr && !item->isBinary())
                repeats++;
            else if (repeats > 0)
            {
                os << "\t\t\t<repeats " << repeats << " times>\n";
                repeats = 0;
            }
            if (repeats == 0)
            {
                os << "\t\t\ttype: " << (item->isBinary() ? "binary" : "text");
                os << ": " << item->id() << " - " << itemStr << '\n';
            }
            lastStr = std::move(itemStr);
            totalSize += item->size();
        }
        if (repeats > 0)
            os << "\t\t\t<repeats " << repeats << " times>\n";
        os << "\t\tqueue size: " << totalSize << " bytes\n";
    }

private:
    /// Deduplicate messages based on the new one.
    /// Returns true if the new message should be
    /// enqueued, otherwise false.
    bool deduplicate(const Item& item)
    {
        // Deduplicate messages based on the incoming one.
        std::string command = item->firstToken();
        if (command == "tile:")
        {
            // Remove previous identical tile, if any, and use most recent (incoming).
            const TileDesc newTile = TileDesc::parse(item->firstLine());
            uint32_t newTilePosHash = newTile.equalityHash();
            // store a hash of position for this tile.
            item->setHash(newTilePosHash);

            const auto& pos = std::find_if(_queue.begin(), _queue.end(),
                                           [&newTile, newTilePosHash](const queue_item_t& cur)
                {
                    if (!cur->firstTokenMatches("tile:"))
                        return false;
                    if (newTilePosHash != cur->getHash()) // eliminate N^2 parsing
                        return false;
                    if (newTile != TileDesc::parse(cur->firstLine()))
                    {
                        LOG_TRC("Ununusal - tile " << newTile.serialize() << " has quality "
                                " hash collision with " << cur->firstLine() << " of " << newTilePosHash);
                        return false;
                    }
                    return true;
                });

            if (pos != _queue.end())
                _queue.erase(pos);
        }
        else if (command == "invalidatecursor:" ||
                 command == "setpart:")
        {
            // Remove previous identical entries of this command,
            // if any, and use most recent (incoming).
            const auto& pos = std::find_if(_queue.begin(), _queue.end(),
                [&command](const queue_item_t& cur)
                {
                    return cur->firstTokenMatches(command);
                });

            if (pos != _queue.end())
                _queue.erase(pos);
        }
        else if (command == "progress:")
        {
            // find other progress commands with similar content
            static constexpr std::string_view setvalueTag = R"("id":"setvalue")";
            if (item->contains(setvalueTag))
            {
                const auto& pos = std::find_if(_queue.begin(), _queue.end(),
                                               [&command](const queue_item_t& cur)
                {
                    return cur->firstTokenMatches(command) &&
                           cur->contains(setvalueTag);
                });

                if (pos != _queue.end())
                    _queue.erase(pos);
            }
        }
        else if (command == "invalidateviewcursor:")
        {
            // Remove previous cursor invalidation for same view,
            // if any, and use most recent (incoming).
            const std::string newMsg = item->jsonString();
            Poco::JSON::Parser newParser;
            const Poco::Dynamic::Var newResult = newParser.parse(newMsg);
            const auto& newJson = newResult.extract<Poco::JSON::Object::Ptr>();
            std::string viewId = newJson->get("viewId").toString();
            const auto& pos = std::find_if(_queue.begin(), _queue.end(),
                [command=std::move(command),
                 viewId=std::move(viewId)](const queue_item_t& cur)
                {
                    if (cur->firstTokenMatches(command))
                    {
                        const std::string msg = cur->jsonString();
                        Poco::JSON::Parser parser;
                        const Poco::Dynamic::Var result = parser.parse(msg);
                        const auto& json = result.extract<Poco::JSON::Object::Ptr>();
                        return viewId == json->get("viewId").toString();
                    }

                    return false;
                });

            if (pos != _queue.end())
                _queue.erase(pos);
        }

        return true;
    }

private:
    mutable std::mutex _mutex;
    std::deque<Item> _queue;
    using queue_item_t = typename std::deque<Item>::value_type;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
