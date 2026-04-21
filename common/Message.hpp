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

#include <common/Log.hpp>
#include <common/Protocol.hpp>
#include <common/StringVector.hpp>
#include <common/Util.hpp>

#include <atomic>
#include <cstring>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

/// The payload type used to send/receive data.
class Message
{
public:

    enum class Type : std::uint8_t { Text, JSON, Binary };
    enum class Dir : std::uint8_t { In, Out };

    /// Construct a text message.
    /// message must include the full first-line.
    Message(const std::string_view message, const enum Dir dir)
        : Message(message.data(), message.size(), dir)
    {
    }

    /// Construct a message from a character array with type.
    /// Note: p must include the full first-line.
    Message(const char* p,
            const size_t len,
            const enum Dir dir) :
        _forwardToken(getForwardToken(p, len)),
        _data(copyDataAfterOffset(p, len, _forwardToken.size())),
        _tokens(StringVector::tokenize(_data.data(), _data.size())),
        _id(makeId(dir)),
        _type(detectType()),
        _hash(0)
    {
        LOG_TRC("Message " << abbr());
    }

    size_t size() const { return _data.size(); }
    const std::vector<char>& data() const { return _data; }

    const StringVector& tokens() const { return _tokens; }
    const std::string& forwardToken() const { return _forwardToken; }
    std::string firstToken() const { return _tokens[0]; }
    bool firstTokenMatches(const std::string_view target) const
    {
        return _tokens.equals(0, target);
    }
    std::string operator[](size_t index) const { return _tokens[index]; }

    /// Allow a message to annotate a hash of its content for use later
    uint32_t getHash() const { return _hash; }
    void setHash(uint32_t hash) { _hash = hash; }

    /// Returns true iff the subarray exists in the raw message.
    bool contains(const std::string_view msg) const
    {
        return std::string_view(_data.data(), _data.size()).find(msg) != std::string::npos;
    }

    const std::string& firstLine()
    {
        assignFirstLineIfEmpty();
        return _firstLine;
    }

    bool getTokenInteger(const std::string_view name, int& value)
    {
        return COOLProtocol::getTokenInteger(_tokens, name, value);
    }

    /// Return the abbreviated message for logging purposes.
    std::string abbr() const {
        return _id + ' ' + COOLProtocol::getAbbreviatedMessage(_data.data(), _data.size());
    }
    const std::string& id() const { return _id; }

    /// Returns the json part of the message, if any.
    std::string jsonString() const
    {
        if (_tokens.size() > 1 && _tokens[1].size() >= 1 &&
            (_tokens[1][0] == '{' || _tokens[1][0] == '['))
        {
            const size_t firstTokenSize = _tokens[0].size();
            return std::string(_data.data() + firstTokenSize, _data.size() - firstTokenSize);
        }

        return std::string();
    }

    /// Append more data to the message.
    void append(const char* p, const size_t len)
    {
        const size_t curSize = _data.size();
        _data.resize(curSize + len);
        std::memcpy(_data.data() + curSize, p, len);
    }

    /// Returns true if and only if the payload is considered Binary.
    bool isBinary() const { return _type == Type::Binary; }

    /// Allows some in-line re-writing of the message
    void rewriteDataBody(const std::function<bool (std::vector<char> &)>& func)
    {
        // Make sure _firstLine is assigned before we change _data
        assignFirstLineIfEmpty();
        if (func(_data))
        {
            // Check - just the body.
            assert(_firstLine == COOLProtocol::getFirstLine(_data.data(), _data.size()));
            assert(_type == detectType());
        }
    }

private:

    /// Constructs a unique ID.
    static std::string makeId(const enum Dir dir)
    {
        static std::atomic<unsigned> Counter;
        return (dir == Dir::In ? 'i' : 'o') + std::to_string(++Counter);
    }

    void assignFirstLineIfEmpty()
    {
        if(_firstLine.empty())
        {
            _firstLine = COOLProtocol::getFirstLine(_data.data(), _data.size());
        }
    }

    Type detectType() const
    {
        for (auto i : COOLProtocol::binaryMessageTypes)
            if (_tokens.equals(0, i))
            {
                return Type::Binary;
            }

        if (!_data.empty() && (_data.back() == '}' || _data.back() == ']'))
        {
            return Type::JSON;
        }

        // All others are plain text.
        return Type::Text;
    }

    std::string getForwardToken(const char* buffer, int length)
    {
        std::string forward = COOLProtocol::getFirstToken(buffer, length);
        if (forward.find('-') != std::string::npos)
           return forward;
        return std::string();
    }

    std::vector<char> copyDataAfterOffset(const char *p, size_t len, size_t fromOffset)
    {
        if (!p || fromOffset >= len)
            return std::vector<char>();

        size_t i;
        for (i = fromOffset; i < len; ++i)
        {
            if (p[i] != ' ')
                break;
        }
        if (i < len)
            return std::vector<char>(p + i, p + len);
        else
            return std::vector<char>();
    }

private:
    const std::string _forwardToken;
    std::vector<char> _data;
    const StringVector _tokens;
    const std::string _id;
    std::string _firstLine;
    const Type _type;
    uint32_t _hash;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
