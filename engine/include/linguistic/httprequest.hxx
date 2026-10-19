/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <linguistic/lngdllapi.h>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <boost/property_tree/ptree_fwd.hpp>

#include <chrono>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

// The HTTP round trip the remote linguistic services share: the remote
// sentence checkers and the translation providers build a Request, post()
// or get() carry it out, and the Response says what came back.
namespace linguistic::http
{
struct Request
{
    OUString aURL;
    /// The body, already encoded the way aContentType says; empty for a GET.
    OString aBody;
    /// Sent as the Content-Type header; with none, a POST of form fields is
    /// marked as such by the transfer itself.
    OString aContentType;
    std::vector<OString> aHeaders;
    /// "user:password" for HTTP Basic authentication, empty for none.
    OString aUserPassword;
    bool bVerifySSL = true;
    /// How long the whole round trip may take, and the connection alone.
    std::chrono::seconds aTimeout{ 10 };
    std::chrono::seconds aConnectTimeout{ 3 };
};

struct Response
{
    /// True when the transfer went through and the server answered with a
    /// status below 400.
    bool bOk = false;
    /// The HTTP status; 0 when the server did not answer at all.
    long nStatus = 0;
    /// What the server sent, the body of an error answer included.
    std::string aBody;
    /// Describes a failure: the transfer's own words, or "HTTP nnn". Empty
    /// when bOk.
    OString aError;
};

LNG_DLLPUBLIC Response post(const Request& rRequest);
LNG_DLLPUBLIC Response get(const Request& rRequest);

/// Keeps RFC 3986's unreserved characters and percent-encodes the UTF-8
/// bytes of everything else, for a form field or a query parameter.
LNG_DLLPUBLIC OString percentEncode(std::u16string_view rText);
/// The same for text that is UTF-8 already.
LNG_DLLPUBLIC OString percentEncode(std::string_view rUtf8);

/// Parses a JSON document; nothing when it is not one.
LNG_DLLPUBLIC std::optional<boost::property_tree::ptree> parseJson(const std::string& rJson);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
