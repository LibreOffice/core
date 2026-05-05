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

#include <config.h>

#include <cassert>
#include <cstdint>
#include <sstream>

#include <common/FileUtil.hpp>
#include <common/JsonUtil.hpp>
#include <common/RecentFiles.hpp>

#include <Poco/Path.h>
#include <Poco/URI.h>

RecentFiles::RecentFiles()
    : _initialised(false)
{
}

void RecentFiles::load(const std::string& fileName, int maxFiles)
{
    _initialised = true;

    _fileName = fileName;
    _maxFiles = maxFiles;

    std::ifstream stream;

    FileUtil::openFileToIFStream(fileName, stream);
    if (stream.is_open())
    {
        int n = 0;
        while (!stream.eof() && !stream.bad() && n++ < _maxFiles * 2)
        {
            Entry entry;
            std::getline(stream, entry.uri);
            if (stream.bad() || entry.uri == "")
                break;
            std::string line;
            std::getline(stream, line);
            if (stream.bad() || line == "")
                break;
            uint64_t count;
            std::istringstream(line) >> count;
            entry.timestamp = std::chrono::time_point<std::chrono::system_clock>(std::chrono::system_clock::duration(count));
            _mostRecentlyUsed.push_back(entry);
        }
    }
}

void RecentFiles::add(const std::string& uri)
{
    assert(_initialised);

    // Add entry for the file first in the list, removing old entry for it if it exist.
    for (auto it = _mostRecentlyUsed.begin(); it != _mostRecentlyUsed.end(); it++)
        if (it->uri == uri)
        {
            _mostRecentlyUsed.erase(it);
            break;
        }
    _mostRecentlyUsed.insert(_mostRecentlyUsed.begin(),
                { uri, std::chrono::time_point<std::chrono::system_clock>(std::chrono::system_clock::now()) });

    // Save the list.
    std::ofstream stream;
    FileUtil::openFileToOFStream(_fileName, stream);
    if (!stream.is_open() || stream.bad())
    {
        LOG_ERR("Could not open '" << _fileName << "' for writing");
        return;
    }
    for (const auto& i : _mostRecentlyUsed)
        stream << i.uri << std::endl << i.timestamp.time_since_epoch().count() << std::endl;
}

std::string RecentFiles::serialise()
{
    return serialiseFiltered({ });
}

std::string RecentFiles::serialiseFiltered(std::set<std::string> dropTheseURIs)
{
    std::string result;

    result = "[ ";
    int n = 0;
    for (int i = 0; i < _mostRecentlyUsed.size(); i++)
    {
        if (dropTheseURIs.contains(_mostRecentlyUsed[i].uri))
            continue;

        Poco::URI uri(_mostRecentlyUsed[i].uri);

        std::vector<std::string> segments;
        uri.getPathSegments(segments);

        // Verify that the file still exists
        std::string path = uri.getPath();
#ifdef _WIN32
        // Handle the URIs for drive letter paths and for UNC paths
        if (path.length() > 4 && path[0] == '/' && path[2] == ':' && path[3] == '/')
            path = path.substr(1);
        else if (uri.getHost() != "")
            path = "//" + uri.getHost() + path;
#endif
        std::ifstream stream;
        FileUtil::openFileToIFStream(path, stream);
        if (!stream.is_open())
            continue;

        if (n > 0)
            result += ", ";
        result += "{ "
            "\"uri\": \"" + _mostRecentlyUsed[i].uri + "\", "
            "\"name\": \"" + JsonUtil::escapeJSONValue(segments.empty() ? uri.getPathEtc() : segments.back()) + "\", "
            "\"timestamp\": \"" + std::format("{:%FT%TZ}", _mostRecentlyUsed[i].timestamp) + "\""
            " }";
        n++;
        if (n == _maxFiles)
            break;
    }
    result += " ]";

    return result;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
