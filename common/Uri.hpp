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

#include <string>
#include <string_view>
#include <typeinfo>
#include <utility>
#include <map>

#include <Poco/URI.h>

/// Represents a URI/URL with helpers to manipulate it.
/// Intended to encapsulate, and then replace, Poco::URI.
class Uri
{
    /// Default reserved characters.
    static const std::string Reserved;

public:
    template <typename T>
    explicit Uri(T&& uri)
        : _uri(std::forward<T>(uri))
    {
    }

    /// Returns the URI as a string.
    const std::string& uri() const { return _uri; }

    /// URI-encode the given URI with the given reserved characters (that need encoding).
    static std::string encode(const std::string& uri, const std::string& reserved = Reserved);

    /// URI-encode and return the URI with the given reserved characters (that need encoding).
    std::string encoded(const std::string& reserved = Reserved) const
    {
        return encode(_uri, reserved);
    }

    /// URI-decode the given URI.
    static std::string decode(const std::string& uri);

    /// URI-decode and return the URI.
    std::string decoded() const { return decode(_uri); }

    /// Encode every '%' as '%25' unconditionally.
    /// For local file paths where '%' is always a literal character, not URI encoding.
    static std::string encodeAllPercent(std::string_view path);

    /// Checks whether or not the given string is encoded.
    /// That is, a string that is identical when encoded will return false.
    /// Similarly, a string that is already encoded will return false.
    /// Optionally takes a string of reserved characters to escape while encoding.
    /// Return true if the URI needs encoding. Used for warning about unencoded URIs.
    static bool needsEncoding(const std::string& uri, const std::string& reserved = Reserved);

    /// Extract and return the filename (without extension) given a url or path.
    static std::string getFilenameFromURL(const std::string& url);

    /// Extract and return the filename (with extension) given a url or path,
    /// percent-decoded.
    static std::string getFilenameWithExtFromURL(const std::string& url);

    static bool hasReadonlyPermission(const std::string& url);

private:
    std::string _uri;
};

using AdditionalFilePocoUris = std::map<std::string, Poco::URI>;
using AdditionalFilePaths = std::map<std::string, std::string>;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
