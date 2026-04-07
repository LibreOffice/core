/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Efficient string tokenization without memory allocation.
 * Classes: StringVector - Zero-copy string token management
 */

#include <config.h>

#include "StringVector.hpp"

#include <common/NumUtil.hpp>

#include <tuple>

bool StringVector::equals(std::size_t index, const StringVector& other, std::size_t otherIndex)
{
    if (index >= _tokens.size())
    {
        return false;
    }

    if (otherIndex >= other._tokens.size())
    {
        return false;
    }

    const StringToken& token = _tokens[index];
    const StringToken& otherToken = other._tokens[otherIndex];
    int ret = _string.compare(token._index, token._length, other._string, otherToken._index,
                              otherToken._length);
    return ret == 0;
}

bool StringVector::getUInt32(std::size_t index, const std::string& key, uint32_t& value) const
{
    if (index >= _tokens.size())
    {
        return false;
    }

    const StringToken& token = _tokens[index];

    size_t offset = key.size() + 1;
    if (token._length > offset &&
            _string.compare(token._index, key.size(), key, 0, key.size()) == 0 &&
            _string[token._index + key.size()] == '=')
    {
        bool res = false;
        std::tie(value, res) = NumUtil::u32FromString(
            std::string(&_string[token._index + offset], token._length - offset));
        return res;
    }

    return false;
}

bool StringVector::getNameIntegerPair(std::size_t index, std::string& name, int& value) const
{
    if (index >= _tokens.size())
    {
        return false;
    }

    const StringToken& token = _tokens[index];
    size_t mid = std::string::npos;
    for (size_t i = token._index; i < token._index + token._length; ++i)
    {
        if (_string[i] != '=')
        {
            continue;
        }

        mid = i;
        break;
    }
    if (mid == std::string::npos)
    {
        return false;
    }

    name = _string.substr(token._index, mid - token._index);
    size_t offset = mid + 1;
    bool res = false;
    std::tie(value, res) = NumUtil::i32FromString(
        std::string(&_string[offset], token._index + token._length - offset));
    return res;
}

StringVector StringVector::tokenizeAnyOf(std::string s, const std::string_view delimiters)
{
    // trim from the end so that we do not have to check this exact case
    // later
    std::size_t length = s.length();
    while (length > 0 && s[length - 1] == ' ')
        --length;

    if (length == 0 || delimiters.empty())
        return StringVector();

    std::size_t start = 0;

    std::vector<StringToken> tokens;
    tokens.reserve(16);

    while (start < length)
    {
        // ignore the leading whitespace
        while (start < length && s[start] == ' ')
            ++start;

        // anything left?
        if (start == length)
            break;

        std::size_t end = s.find_first_of(delimiters.data(), start, delimiters.size());
        if (end == std::string::npos)
            end = length;

        // trim the trailing whitespace
        std::size_t trimEnd = end;
        while (start < trimEnd && s[trimEnd - 1] == ' ')
            --trimEnd;

        // add only non-empty tokens
        if (start < trimEnd)
            tokens.emplace_back(start, trimEnd - start);

        start = end + 1;
    }

    return StringVector(std::move(s), std::move(tokens));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
