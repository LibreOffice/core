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

#include <common/Util.hpp>
#include <common/Log.hpp>

#include <cassert>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include <Poco/Dynamic/Var.h>
#include <Poco/JSON/JSONException.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>

namespace JsonUtil
{

// Parse the json string and fill the Poco::JSON object
// Returns true if parsing successful otherwise false
inline bool parseJSON(const std::string_view json, Poco::JSON::Object::Ptr& object)
{
    const std::size_t index = json.find_first_of('{');
    if (index != std::string::npos)
    {
        const std::string stringJSON(json.substr(index));
        Poco::JSON::Parser parser;
        try
        {
            const Poco::Dynamic::Var result = parser.parse(stringJSON);
            object = result.extract<Poco::JSON::Object::Ptr>();
            return true;
        }
        catch (const Poco::JSON::JSONException& exception)
        {
            LOG_WRN("parseJSON: failed to parse '" << stringJSON << "': '" << exception.what() << "'");
        }
    }

    return false;
}

inline
int getLevenshteinDist(const std::string& string1, const std::string& string2)
{
    std::vector<std::vector<int>> matrix(string1.size() + 1, std::vector<int>(string2.size() + 1));

    for (std::size_t i = 0; i < string1.size() + 1; i++)
    {
        for (std::size_t j = 0; j < string2.size() + 1; j++)
        {
            if (i == 0)
            {
                matrix[i][j] = j;
            }
            else if (j == 0)
            {
                matrix[i][j] = i;
            }
            else if (string1[i - 1] == string2[j - 1])
            {
                matrix[i][j] = matrix[i - 1][j - 1];
            }
            else
            {
                matrix[i][j] = 1 + std::min(std::min(matrix[i][j - 1], matrix[i - 1][j]),
                                            matrix[i - 1][j - 1]);
            }
        }
    }

    return matrix[string1.size()][string2.size()];
}

/// Converts the given @valueVar to the type T, if possible.
/// @key is used for logging only.
template <typename T> T getJSONValue(const std::string& key, const Poco::Dynamic::Var & valueVar)
{
    try
    {
        return valueVar.convert<T>();
    }
    catch (const Poco::Exception& exc)
    {
        LOG_ERR("getJSONValue for ["
                << key << "]: " << exc.displayText()
                << (exc.nested() ? " (" + exc.nested()->displayText() + ')' : ""));
    }

    return T();
}

/// Gets value for @key directly from the given JSON in @object.
template <typename T> T getJSONValue(const Poco::JSON::Object::Ptr& object, const std::string& key)
{
    return getJSONValue<T>(key, object->get(key));
}

/// Function that searches `object` for `key` and warns if there are minor mis-spellings involved.
/// Upon successful search, fills `value` with value found in object.
/// Removes the entry from the JSON object if @bRemove == true.
template <typename T>
bool findJSONValue(const Poco::JSON::Object::Ptr& object, const std::string& key, T& value)
{
    // Try exact match first.
    const Poco::Dynamic::Var var = object->get(key);
    if (!var.isEmpty())
    {
        value = getJSONValue<T>(key, var);

        LOG_TRC("Found JSON property [" << key << "] => [" << value << ']');
        return true;
    }

    std::vector<std::string> propertyNames;
    object->getNames(propertyNames);

    // Check each property name against given key
    // and warn for mis-spells with tolerance of 2.
    const std::string keyLower = Util::toLower(key);
    for (const std::string& userInput : propertyNames)
    {
        if (key != userInput)
        {
            const std::string userInputLower = Util::toLower(userInput);

            // Mis-spelling tolerance.
            const int levDist = getLevenshteinDist(keyLower, userInputLower);
            if (levDist > 2)
                continue; // Not even close, keep searching.

            // We found something with some differences--warn and return.
            LOG_ERR("Incorrect JSON property [" << userInput << "]. Did you mean [" << key <<
                    "] ? (Levenshtein distance: " << levDist << ')');

            // Fail without exact match.
            return false;
        }

        value = getJSONValue<T>(object, userInput);

        LOG_TRC("Found JSON property [" << userInput << "] => [" << value << ']');
        return true;
    }

    LOG_INF("Missing JSON property [" << key << "] will default to [" << value << ']');
    return false;
}

static char getEscapementChar(char ch)
{
    switch (ch)
    {
        case '\b':
            return 'b';
        case '\t':
            return 't';
        case '\n':
            return 'n';
        case '\f':
            return 'f';
        case '\r':
            return 'r';
        default:
            return ch;
    }
}

inline void writeEscapedSequence(uint32_t ch, std::string& buf)
{
    switch (ch)
    {
        case '\b':
        case '\t':
        case '\n':
        case '\f':
        case '\r':
        case '"':
        case '/':
        case '\\':
            buf.push_back('\\');
            buf.push_back(getEscapementChar(ch));
            break;
        // Special processing of U+2028 and U+2029, which are valid JSON, but invalid JavaScript
        // Write them in escaped '\u2028' or '\u2029' form
        case 0x2028:
        case 0x2029:
            buf.push_back('\\');
            buf.push_back('u');
            buf.push_back('2');
            buf.push_back('0');
            buf.push_back('2');
            buf.push_back(ch == 0x2028 ? '8' : '9');
            break;
        default:
            assert(!"Unexpected character passed to writeEscapedSequence");
    }
}

inline std::string escapeJSONValue(const std::string_view val)
{
    std::string buf;
    buf.reserve(val.size() + 64); // some small initial extra space for escaping
    for (size_t i = 0; i < val.size(); ++i)
    {
        const char ch = val[i];
        switch(ch)
        {
            case '\b':
            case '\t':
            case '\n':
            case '\f':
            case '\r':
            case '"':
            case '/':
            case '\\':
                writeEscapedSequence(ch, buf);
                break;
            case '\xE2': // Special processing of U+2028 and U+2029
                if (i + 2 < val.size() && val[i + 1] == '\x80'
                    && (val[i + 2] == '\xA8' || val[i + 2] == '\xA9'))
                {
                    writeEscapedSequence(val[i + 2] == '\xA8' ? 0x2028 : 0x2029, buf);
                    i += 2;
                    break;
                }
                [[fallthrough]];
            default:
                buf.push_back(ch);
                break;
        }
    }

    return buf;
}

/// Extract all json entries into a map.
inline std::map<std::string, std::string> jsonToMap(const std::string& jsonString)
{
    std::map<std::string, std::string> map;
    if (jsonString.empty())
        return map;

    Poco::JSON::Parser parser;
    const Poco::Dynamic::Var result = parser.parse(jsonString);
    const auto& json = result.extract<Poco::JSON::Object::Ptr>();

    std::vector<std::string> names;
    json->getNames(names);

    for (const auto& name : names)
    {
        map[name] = json->get(name).toString();
    }

    return map;
}

template <typename T>
Poco::JSON::Object::Ptr makePropertyValue(const std::string& type, const T& val)
{
    Poco::JSON::Object::Ptr obj = new Poco::JSON::Object();
    obj->set("type", type);
    obj->set("value", val);
    return obj;
}

inline std::string jsonToString(const Poco::JSON::Object::Ptr& json)
{
    std::ostringstream jsonStream;
    json->stringify(jsonStream);
    return jsonStream.str();
}

} // end namespace JsonUtil

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
