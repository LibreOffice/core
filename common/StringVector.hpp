/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

/**
 * Stores an offset and a length into the single underlying string of StringVector.
 */
struct StringToken
{
    std::size_t _index;
    std::size_t _length;

    StringToken() = default;

    StringToken(std::size_t index, std::size_t length)
        : _index(index),
        _length(length)
    {
    }

    bool operator==(const StringToken& rhs) const
    {
        return _index == rhs._index && _length == rhs._length;
    }
};

/**
 * Safe wrapper around an std::vector<std::string>. Gives
 * you an empty string if you would read past the ends
 * of the vector.
 */
class StringVector
{
    /// All tokens are substrings of this string.
    std::string _string;
    std::vector<StringToken> _tokens;

public:
    explicit StringVector() = default;

    explicit StringVector(std::string string, std::vector<StringToken> tokens)
        : _string(std::move(string))
        , _tokens(std::move(tokens))
    {
    }

    /// Tokenize delimited values until we hit new-line or the end.
    static void tokenize(const char* data, const std::size_t size, const char delimiter,
                         std::vector<StringToken>& tokens)
    {
        if (size == 0 || data == nullptr || *data == '\0')
            return;

        tokens.reserve(16);

        const char* start = data;
        const char* end = data;
        for (std::size_t i = 0; i < size && data[i] != '\n'; ++i, ++end)
        {
            if (data[i] == delimiter)
            {
                if (start != end && *start != delimiter)
                    tokens.emplace_back(start - data, end - start);

                start = end;
            }
            else if (*start == delimiter)
                ++start;
        }

        if (start != end && *start != delimiter && *start != '\n')
            tokens.emplace_back(start - data, end - start);
    }

    // call func on each token until func returns true or we run out of tokens
    template <class UnaryFunction>
    static void tokenize_foreach(UnaryFunction&& func, const char* data, const std::size_t size, const char delimiter = ' ')
    {
        if (size == 0 || data == nullptr || *data == '\0')
            return;

        size_t index = 0;

        const char* start = data;
        const char* end = data;
        for (std::size_t i = 0; i < size && data[i] != '\n'; ++i, ++end)
        {
            if (data[i] == delimiter)
            {
                if (start != end && *start != delimiter)
                {
                    if (func(index++, std::string_view(start, end - start)))
                        return;
                }

                start = end;
            }
            else if (*start == delimiter)
                ++start;
        }

        if (start != end && *start != delimiter && *start != '\n')
            func(index, std::string_view(start, end - start));
    }

    /// Tokenize single-char delimited values until we hit new-line or the end.
    static StringVector tokenize(const char* data, const std::size_t size,
                                 const char delimiter = ' ')
    {
        if (size == 0 || data == nullptr || *data == '\0')
            return StringVector();

        std::vector<StringToken> tokens;
        tokenize(data, size, delimiter, tokens);
        return StringVector(std::string(data, size), std::move(tokens));
    }

    /// Tokenize single-char delimited values until we hit new-line or the end.
    static StringVector tokenize(std::string s, const char delimiter = ' ')
    {
        if (s.empty())
            return StringVector();

        std::vector<StringToken> tokens;
        tokenize(s.data(), s.size(), delimiter, tokens);
        return StringVector(std::move(s), std::move(tokens));
    }

    /// Tokenize by the delimiter string.
    static StringVector tokenize(std::string s, const std::string_view delimiter)
    {
        if (s.empty() || delimiter.empty())
            return StringVector();

        std::size_t start = 0;
        std::size_t end = s.find(delimiter, start);

        std::vector<StringToken> tokens;
        tokens.reserve(16);

        tokens.emplace_back(start, end - start);
        start = end + delimiter.size();

        while (end != std::string::npos)
        {
            end = s.find(delimiter, start);
            tokens.emplace_back(start, end - start);
            start = end + delimiter.size();
        }

        return StringVector(std::move(s), std::move(tokens));
    }

    /** Tokenize based on any of the characters in 'delimiters'.

        Ie. when there is '\n\r' in there, any of them means a delimiter.
        In addition, trim the values so there are no leading or trailing spaces.
    */
    static StringVector tokenizeAnyOf(std::string s, const std::string_view delimiters);

    /// Unlike std::vector, gives an empty string if index is unexpected.
    std::string operator[](std::size_t index) const
    {
        if (index >= _tokens.size())
        {
            return std::string();
        }

        const StringToken& token = _tokens[index];
        return _string.substr(token._index, token._length);
    }

    std::size_t size() const { return _tokens.size(); }

    bool empty() const { return _tokens.empty(); }

    std::vector<StringToken>::const_iterator begin() const { return _tokens.begin(); }

    std::vector<StringToken>::iterator begin() { return _tokens.begin(); }

    std::vector<StringToken>::const_iterator end() const { return _tokens.end(); }

    std::vector<StringToken>::iterator end() { return _tokens.end(); }

    std::vector<StringToken>::iterator erase(std::vector<StringToken>::const_iterator it)
    {
        return _tokens.erase(it);
    }

    void push_back(const std::string_view string)
    {
        _tokens.emplace_back(_string.size(), string.size());
        _string += string;
    }

    /// Gets the underlying string of a single token.
    std::string getParam(const StringToken& token) const
    {
        assert(token._index < _string.size() && "Index is out of range");
        return _string.substr(token._index, token._length);
    }

    /// Concats tokens starting from firstOffset, using separator as separator.
    /// An optional lastOffset can be used to decide the last entry, inclusive.
    template <typename T>
    std::string cat(const T& separator, std::size_t firstOffset,
                    std::size_t lastOffset = std::string::npos) const
    {
        if (firstOffset >= _tokens.size() || firstOffset > lastOffset)
        {
            return std::string();
        }

        assert(!_tokens.empty() && "Unexpected empty tokens");

        std::string ret;
        ret.reserve(_string.size() * 2);

        std::size_t i = firstOffset;
        const std::size_t end = std::min<std::size_t>(lastOffset, _tokens.size() - 1);
        ret = getParam(_tokens[i]);
        for (++i; i <= end; ++i)
        {
            // Avoid temporary strings, append separately.
            ret += separator;
            assert(i < _tokens.size() && "Index is out of range");
            ret += getParam(_tokens[i]);
        }

        return ret;
    }

    /// Returns a copy of the original string starting at token 'startOffset'
    /// until the 'lastOffset', the given, inclusive. Otherwise, to the end.
    std::string substrFromToken(std::size_t firstOffset,
                                std::size_t lastOffset = std::string::npos) const
    {
        if (firstOffset >= _tokens.size() || firstOffset > lastOffset)
            return std::string();

        const std::size_t end = lastOffset < _tokens.size()
                                    ? _tokens[lastOffset]._index + _tokens[lastOffset]._length -
                                          _tokens[firstOffset]._index
                                    : std::string::npos;
        return _string.substr(_tokens[firstOffset]._index, end);
    }

    /// Compares the nth token with string.
    template <typename T> bool equals(std::size_t index, const T& string) const
    {
        if (index >= _tokens.size())
        {
            return false;
        }

        const StringToken& token = _tokens[index];
        return std::string_view(_string.data() + token._index, token._length) == string;
    }

    /// Compares the nth token with string.
    bool equals(std::size_t index, const std::string_view string) const
    {
        if (index >= _tokens.size())
        {
            return false;
        }

        const StringToken& token = _tokens[index];
        return std::string_view(_string.data() + token._index, token._length) == string;
    }

    // Checks if the token text at index starts with the given string
    template <std::size_t N>
    bool startsWith(std::size_t index, const char (&string)[N]) const
    {
        if (index >= _tokens.size())
        {
            return false;
        }

        const StringToken& token = _tokens[index];
        constexpr auto len = N - 1; // we don't want to compare the '\0'
        return token._length >= len && _string.compare(token._index, len, string) == 0;
    }

    // Checks if the token text starts with the given string
    template <std::size_t N>
    bool startsWith(const StringToken& token, const char (&string)[N]) const
    {
        if (token._index >= _string.size())
        {
            return false;
        }

        constexpr auto len = N - 1; // we don't want to compare the '\0'
        return token._length >= len && _string.compare(token._index, len, string) == 0;
    }

    /// Compares the nth token with the mth token from another StringVector.
    bool equals(std::size_t index, const StringVector& other, std::size_t otherIndex);

    bool getUInt32(std::size_t index, const std::string& key, uint32_t& value) const;
    bool getUInt64(std::size_t index, const std::string& key, uint64_t& value) const;
    bool getNameIntegerPair(std::size_t index, std::string& name, int& value) const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
