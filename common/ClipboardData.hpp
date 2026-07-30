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

// The clipboardData struct

#pragma once

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

struct ClipboardData
{
    std::vector<std::string> _mimeTypes;
    std::vector<std::string> _content;
    ClipboardData()
    {
    }

    /// Determines if inStream is a list of mimetype-length-bytes tuples, as expected.
    static bool isOwnFormat(std::istream& inStream)
    {
        if (inStream.eof())
        {
            return false;
        }

        std::string mime, hexLen;
        std::getline(inStream, mime, '\n');
        if (mime.empty())
        {
            return false;
        }

        std::getline(inStream, hexLen, '\n');
        if (hexLen.empty())
        {
            return false;
        }

        uint64_t len = strtoll(hexLen.c_str(), nullptr, 16);
        if (len == 0)
        {
            return false;
        }

        return true;
    }

    bool read(std::istream& inStream)
    {
        while (!inStream.eof() && !inStream.fail())
        {
            std::string mime, hexLen, newline;
            std::getline(inStream, mime, '\n');
            std::getline(inStream, hexLen, '\n');
            if (mime.length() && hexLen.length() && !inStream.fail())
            {
                uint64_t len = strtoll( hexLen.c_str(), nullptr, 16 );
                std::string content(len, ' ');
                inStream.read(content.data(), len);
                if (inStream.fail())
                    return false;
                std::getline(inStream, newline, '\n');
                if (mime.length() > 0)
                {
                    _mimeTypes.push_back(std::move(mime));
                    _content.push_back(std::move(content));
                }
            }
        }
        return true;
    }

    std::size_t size() const
    {
        assert(_mimeTypes.size() == _content.size());
        return _mimeTypes.size();
    }

    void dumpState(std::ostream& os)
    {
        os << "Clipboard with " << size() << " entries:\n";
        for (size_t i = 0; i < size(); ++i)
            os << "\t[" << i << "] - size " << _content[i].size() <<
                " type: '" << _mimeTypes[i] << "'\n";
    }

    bool findType(const std::string &mime, std::string &value)
    {
        for (size_t i = 0; i < _mimeTypes.size(); ++i)
        {
            if (_mimeTypes[i] == mime)
            {
                value = _content[i];
                return true;
            }
        }
        value.clear();
        return false;
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
