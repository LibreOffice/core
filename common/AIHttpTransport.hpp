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

#include <functional>
#include <string>

// Pluggable async HTTP-POST transport for the AI proxy (AIChatSession).
//
// The WSD server reaches AI providers with the COOL net stack (http::Session).
// The desktop apps don't have that stack - they use FakeSocket for the in-process
// document protocol and have no real outbound sockets through net/ - so they
// register a platform transport here (e.g. QNetworkAccessManager on Qt) instead.
namespace ai
{
/// Sentinel "status codes" passed to HttpDoneCallback. Real HTTP responses use
/// their actual code (>= 100).
constexpr int HttpNoResponse = 0;     ///< no response received (e.g. timed out)
constexpr int HttpConnectFailed = -1; ///< could not connect to the server

/// Delivers the outcome: the HTTP status code (or a sentinel above) and the
/// response body. May be invoked on any thread - the caller marshals as needed.
using HttpDoneCallback = std::function<void(int statusCode, std::string body)>;

/// Performs an asynchronous HTTP POST. The body is sent as application/json and
/// \c authHeader, when non-empty, becomes the Authorization header value.
using HttpPostFn = std::function<void(const std::string& url,
                                      const std::string& authHeader,
                                      std::string body,
                                      int timeoutSeconds,
                                      HttpDoneCallback onDone)>;

/// Register the platform transport (typically once at startup). Unset on the
/// WSD server, which uses http::Session directly.
void setHttpPostFn(HttpPostFn fn);

/// The registered transport, or an empty function if none was set.
const HttpPostFn& httpPostFn();
} // namespace ai

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
