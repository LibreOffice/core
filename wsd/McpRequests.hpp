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

/*
 * The HTTP side of the Model Context Protocol endpoint at /cool/mcp: the checks a request passes as
 * soon as its header is in, and the handling of the request once its body is read.
 */

#pragma once

#include <istream>
#include <memory>

namespace Poco::Net
{
class HTTPRequest;
}

class RequestDetails;
class StreamSocket;

namespace McpRequests
{
/// True when the request is for the Model Context Protocol endpoint.
bool isMcpRequest(const RequestDetails& requestDetails);

/// Refuse a request for the Model Context Protocol endpoint that is not to be served, as soon as
/// its header is known. It is refused when the endpoint is off or has no key (404), when it comes
/// from a web page (403), when it lacks the key (401), and for a POST whose body has no stated
/// length (411). Returns true when a refusal was sent.
bool refuseMcpRequest(const Poco::Net::HTTPRequest& request,
                      const std::shared_ptr<StreamSocket>& socket);

/// Refuse a request as refuseMcpRequest does, as soon as its header is in, when its address is for
/// the Model Context Protocol endpoint. The request itself is left unchanged. Returns true when a
/// refusal was sent.
bool refuseMcpRequestEarly(const Poco::Net::HTTPRequest& request,
                           const std::shared_ptr<StreamSocket>& socket);

/// Serve the Model Context Protocol endpoint at /cool/mcp. Applies the
/// endpoint's own gates (config switch, HTTP method, Origin, bearer key)
/// and then hands the body to the transport-agnostic core.
/// @return true if request has been handled synchronously and response sent, otherwise false
bool handleMcpRequest(const Poco::Net::HTTPRequest& request, std::istream& message,
                      const std::shared_ptr<StreamSocket>& socket);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
