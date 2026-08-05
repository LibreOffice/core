/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * URI encoding and decoding utilities.
 * Classes: Uri - Functions: encode(), decode(), needsEncoding()
 */

#include <config.h>

#include "Uri.hpp"

#include <common/Log.hpp>
#include <common/Util.hpp>

#include <Poco/URI.h>



const std::string Uri::Reserved = ",/?:@&=+$#";

std::string Uri::encode(const std::string& uri, const std::string& reserved)
{
    std::string encoded;
    Poco::URI::encode(uri, reserved, encoded);
    return encoded;
}

std::string Uri::decode(const std::string& uri)
{
    std::string decoded;
    Poco::URI::decode(uri, decoded);
    return decoded;
}

std::string Uri::encodeAllPercent(const std::string_view path)
{
    std::string result;
    result.reserve(path.size());
    for (const char c : path)
    {
        if (c == '%')
            result += "%25";
        else
            result += c;
    }
    return result;
}

bool Uri::needsEncoding(const std::string& uri, const std::string& reserved)
{
    const std::string decoded = decode(uri);
    if (decoded != uri)
    {
        // We could decode it; must have been encoded already.
        return false;
    }

    // Identical when decoded, might need encoding.
    const std::string encoded = encode(uri, reserved);

    // If identical, then doesn't need encoding.
    return encoded != uri;
}

std::string Uri::getFilenameFromURL(const std::string& url)
{
    auto [base, filename, ext, params] = Util::splitUrl(url);
    return filename;
}

std::string Uri::getFilenameWithExtFromURL(const std::string& url)
{
    auto [base, filename, ext, params] = Util::splitUrl(url);
    // The url carries the name percent-encoded. The file name is the decoded
    // form, so a name with spaces comes back with real spaces rather than %20.
    if (ext.empty())
        return decode(filename);
    return decode(filename + ext);
}

bool Uri::hasReadonlyPermission(const std::string& url)
{
    //FIXME: Replace with our own implementation.
    for (const auto& param : Poco::URI(url).getQueryParameters())
    {
        if (!param.first.empty())
        {
            LOG_TRC("Query param: [" << param.first << "], value: [" << param.second << ']');
            if (param.first == "permission" && param.second == "readonly")
            {
                return true;
            }
        }
    }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
