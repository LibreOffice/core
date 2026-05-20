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

#include <common/NumUtil.hpp>
#include <common/StringVector.hpp>
#include <common/Util.hpp>

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <string_view>
#include <vector>

#define KIT_USE_UNSTABLE_API
#include <COKit/COKitEnums.h>

namespace COOLProtocol
{
    // Protocol Version Number.
    // See protocol.txt.
    constexpr unsigned ProtocolMajorVersionNumber = 0;
    constexpr unsigned ProtocolMinorVersionNumber = 1;

    static constexpr const char* binaryMessageTypes[] {
        "tile:",
        "tilecombine:",
        "delta:",
        "rendersearchresult:",
        "slidelayer:",
        "zstdslidelayer:",
        "windowpaint:",
        "urp:"
    };

    static inline bool isMessageOfType(const char* message, const std::string& type, size_t length)
    {
        if (length < type.length() + 2)
            return false;
        for (size_t i = 0; i < type.length(); i++)
            if (message[i] != type[i])
                return false;
        return true;
    }

    static inline bool isBinaryMessage(const char *buffer, size_t length)
    {
        for (auto i : COOLProtocol::binaryMessageTypes)
        {
            if (isMessageOfType(buffer, i, length))
                return true;
        }

        return false;
    }

    inline std::string GetProtocolVersion()
    {
        return std::to_string(ProtocolMajorVersionNumber) + '.'
            + std::to_string(ProtocolMinorVersionNumber);
    }

    /// Build an "error: cmd=<cmd> kind=<kind> {errordetail=<message>}" frame
    std::string buildErrorFrame(std::string_view cmd, std::string_view kind,
                                std::string_view message = {});

    // Parse a string into a version tuple.
    // Negative numbers for error.
    std::tuple<int, int, std::string> ParseVersion(const std::string& version);

    inline bool stringToInteger(const std::string_view input, int& value)
    {
        bool res;
        std::tie(value, res) = NumUtil::i32FromString(input);
        return res;
    }

    inline bool stringToUInt32(const std::string_view input, uint32_t& value)
    {
        bool res;
        std::tie(value, res) = NumUtil::i32FromString(input);
        return res;
    }

    inline bool stringToUInt64(const std::string_view input, uint64_t& value)
    {
        bool res;
        std::tie(value, res) = NumUtil::u64FromString(input);
        return res;
    }

    inline bool parseNameValuePair(const std::string_view token, std::string& name,
                                   std::string& value, const char delim = '=')
    {
        const size_t mid = token.find_first_of(delim);
        if (mid != std::string::npos)
        {
            name = token.substr(0, mid);
            value = token.substr(mid + 1);
            return true;
        }

        return false;
    }

    bool getTokenInteger(std::string_view token, std::string_view name, int& value);
    bool getTokenUInt32(std::string_view token, std::string_view name, uint32_t& value);
    bool getTokenUInt64(std::string_view token, std::string_view name, uint64_t& value);
    bool getTokenString(std::string_view token, std::string_view name, std::string& value);
    bool getTokenKeyword(std::string_view token, std::string_view name,
                         const std::map<std::string, int>& map, int& value);

    bool getTokenKeyword(const StringVector& tokens, std::string_view name,
                         const std::map<std::string, int>& map, int& value);

    inline bool getTokenInteger(const StringVector& tokens, const std::string_view name,
                                int& value)
    {
        for (size_t i = 0; i < tokens.size(); i++)
        {
            if (getTokenInteger(tokens[i], name, value))
                return true;
        }
        return false;
    }

    /// Literal-string token names.
    template <std::size_t N>
    inline bool getTokenInteger(const std::string_view token, const char (&name)[N], int& value)
    {
        // N includes null termination.
        static_assert(N > 1, "Token name must be at least one character long.");
        if (token.size() > N && token[N - 1] == '=' && token.compare(0, N - 1, name) == 0)
        {
            bool success;
            std::tie(value, success) = NumUtil::i32FromString(token.substr(N));
            return success;
        }

        return false;
    }

    /// Extracts a name and value from token. Returns true if value is a non-negative integer.
    template <std::size_t N>
    inline bool getNonNegTokenInteger(const std::string_view token, const char (&name)[N],
                                      int& value)
    {
        return getTokenInteger(token, name, value) && value >= 0;
    }

    inline bool getTokenString(const StringVector& tokens,
                               const std::string_view name,
                               std::string& value)
    {
        for (const auto& token : tokens)
        {
            if (getTokenString(tokens.getParam(token), name, value))
            {
                return true;
            }
        }

        return false;
    }

    bool getTokenStringFromMessage(std::string_view message, std::string_view name,
                                   std::string& value);

    inline
    std::vector<int> tokenizeInts(const char* data, const size_t size, const char delimiter = ',')
    {
        std::vector<int> tokens;
        if (size == 0 || data == nullptr)
            return tokens;

        const char* start = data;
        const char* end = data;
        for (size_t i = 0; i < size && data[i] != '\n'; ++i, ++end)
        {
            if (data[i] == delimiter)
            {
                if (start != end && *start != delimiter)
                    tokens.emplace_back(std::atoi(start));

                start = end;
            }
            else if (*start == delimiter)
                ++start;
        }

        if (start != end && *start != delimiter && *start != '\n')
            tokens.emplace_back(std::atoi(start));

        return tokens;
    }

    inline std::vector<int> tokenizeInts(const std::string_view str, const char delimiter = ',')
    {
        return tokenizeInts(str.data(), str.size(), delimiter);
    }

    inline bool getTokenIntegerFromMessage(const std::string& message, const std::string_view name, int& value)
    {
        return getTokenInteger(StringVector::tokenize(message), name, value);
    }

    /// Returns the first token of a message.
    inline
    std::string getFirstToken(const char *message, const int length, const char delim = ' ')
    {
        return Util::getDelimitedInitialSubstring(message, length, delim);
    }

    template <typename T>
    std::string getFirstToken(const T& message, const char delim = ' ')
    {
        return getFirstToken(message.data(), message.size(), delim);
    }

    inline
    bool matchPrefix(const std::string_view prefix, const std::string_view message)
    {
        return (message.size() >= prefix.size() &&
                message.compare(0, prefix.size(), prefix) == 0);
    }

    inline
    bool matchPrefix(const std::string_view prefix, const std::vector<char>& message)
    {
        return (message.size() >= prefix.size() &&
                prefix.compare(0, prefix.size(), message.data(), prefix.size()) == 0);
    }

    inline
    bool matchPrefix(const std::string_view prefix, const std::string_view message, const bool ignoreWhitespace)
    {
        if (ignoreWhitespace)
        {
            const size_t posPre = prefix.find_first_not_of(' ');
            const size_t posMsg = message.find_first_not_of(' ');

            return matchPrefix(posPre == std::string::npos ? prefix : prefix.substr(posPre),
                               posMsg == std::string::npos ? message : message.substr(posMsg));
        }
        else
        {
            return matchPrefix(prefix, message);
        }
    }

    /// Returns true if the token is a user-interaction token.
    /// Currently this excludes commands sent automatically.
    /// Notice that this doesn't guarantee editing activity,
    /// rather just user interaction with the UI.
    inline
    bool tokenIndicatesUserInteraction(const std::string_view token)
    {
        // Exclude tokens that include these keywords, such as statusindicator.

        // FIXME: This is wrong. That the token happens to contain (or not) a certain substring is
        // no guarantee that it "indicates user interaction". It might be like that at the moment,
        // but that is coincidental. We should check what the actual whole token is, at least, not
        // look for a substring.

        return (token.find("tile") == std::string::npos &&
                token.find("status") == std::string::npos &&
                token.find("state") == std::string::npos &&
                token != "userinactive");
    }

    /// Returns true if the token is a likely document modifying command.
    /// This is never 100% accurate, but it is needed to filter out tokens
    /// that certainly do not modify the document, such as 'load' and 'save'
    /// commands. Some commands are certainly modifying, e.g. 'key', others
    /// can only potentially be modifying, e.g. 'mouse' while dragging.
    /// Note: this is only used when we don't have the modified flag from
    /// Core so we flag the document as user-modified more accurately.
    inline bool tokenIndicatesDocumentModification(const StringVector& tokens)
    {
        // These keywords are chosen to cover the largest set of
        // commands that may potentially modify the document.
        // We need to assume modification rather than not.
        if (tokens.equals(0, "key") || tokens.equals(0, "outlinestate") ||
            tokens.equals(0, "paste") || tokens.equals(0, "insertfile") ||
            tokens.equals(0, "textinput") || tokens.equals(0, "windowkey") ||
            tokens.equals(0, "windowmouse") || tokens.equals(0, "windowgesture"))
        {
            return true;
        }

        if (tokens.size() > 1 && tokens.equals(0, "uno"))
        {
            // By default, all uno commands are modifying, unless we are certain they don't.
            return !tokens.equals(1, ".uno:SidebarHide") && !tokens.equals(1, ".uno:SidebarShow") &&
                   !tokens.equals(1, ".uno:Copy") && !tokens.equals(1, ".uno:Save") &&
                   !tokens.startsWith(1, ".uno:ToolbarMode") && // ToolbarMode?Mode...
                   !tokens.equals(1, ".uno:InvertBackground") &&
                   !tokens.equals(1, ".uno:ChangeTheme");
        }

        return false;
    }

    /// Returns the first line of a message.
    inline
    std::string getFirstLine(const char *message, const int length)
    {
        return Util::getDelimitedInitialSubstring(message, length, '\n');
    }

    /// Returns the first line of any data which payload char*.
    template <typename T>
    std::string getFirstLine(const T& message)
    {
        return getFirstLine(message.data(), message.size());
    }

    constexpr int maxNonAbbreviatedMsgLen = 500;

    inline bool shouldEllipse(const char* message, const size_t length, const size_t spanLen)
    {
        // If first line is less than the length (ignoring possible final newline), add ellipsis.
        if (spanLen == length)
            return false;
        if (spanLen < length - 1)
            return true;
        return message[length - 1] != '\n';
    }

    /// Given a well-formed utf-8 string 'message' of messageLen bytes and a
    /// desire to truncate to approximately abbrevLen bytes return the shortest
    /// string greater of equal to abbrevLen that does not split a utf-8
    /// sequence.
    inline std::string truncateUtf8(const char* message, size_t messageLen, size_t abbrevLen)
    {
        std::string ret(message, abbrevLen);
        for (size_t i = abbrevLen; i < messageLen; ++i)
        {
            const char unit = message[i];
            const bool continuation = (static_cast<uint8_t>(unit) & 0xC0) == 0x80;
            if (!continuation) // likely
                break;
            ret.push_back(unit);
        }
        return ret;
    }

    /// Returns an abbreviation of the message (the first line, indicating truncation). We assume
    /// that it adheres to the COOL protocol, i.e. that there is always a first (or only) line that
    /// is in printable UTF-8. I.e. no encoding of binary bytes is done. The format of the result is
    /// not guaranteed to be stable. It is to be used for logging purposes only, not for decoding
    /// protocol frames.
    inline
    std::string getAbbreviatedMessage(const char *message, const int length)
    {
        if (message == nullptr || length <= 0)
        {
            return std::string();
        }

        const size_t spanLen = Util::getDelimiterPosition(message,
            std::min(length, maxNonAbbreviatedMsgLen), '\n');

        // If first line is less than the length (minus newline), add ellipsis.
        if (shouldEllipse(message, length, spanLen))
            return truncateUtf8(message, length, spanLen) + "...";

        return std::string(message, spanLen);
    }

    inline std::string getAbbreviatedMessage(const std::string_view message)
    {
        const size_t spanLen = Util::getDelimiterPosition(message.data(),
            std::min<size_t>(message.size(), maxNonAbbreviatedMsgLen), '\n');

        // If first line is less than the length (minus newline), add ellipsis.
        if (shouldEllipse(message.data(), message.size(), spanLen))
            return truncateUtf8(message.data(), message.size(), spanLen) + "...";

        return std::string(message.substr(0, spanLen));
    }

    template <typename T>
    std::string getAbbreviatedMessage(const T& message)
    {
        return getAbbreviatedMessage(message.data(), message.size());
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
