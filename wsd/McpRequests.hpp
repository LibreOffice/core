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

#include <chrono>
#include <cstddef>
#include <fstream>
#include <istream>
#include <memory>
#include <string>

namespace Poco::Net
{
class HTTPRequest;
}

class RequestDetails;
class SocketDisposition;
class StreamSocket;

/// The body of an upload to the Model Context Protocol endpoint, written as it arrives straight to
/// the file the store keeps. Its size is counted against the store from the start, so uploads that
/// arrive at the same time together stay within its limit. A sink dropped before it is finished
/// removes its file and gives its room back.
class McpUploadSink
{
public:
    /// Start a sink for the upload the request describes, named by its filename parameter. The
    /// upload is given until now plus McpUploads::uploadTimeLimit of its length to arrive whole.
    /// Returns null when the request states no length, the store has no room, or the file cannot be
    /// made. storeFull is set to true when the store has no room, and to false otherwise.
    static std::shared_ptr<McpUploadSink>
    start(const Poco::Net::HTTPRequest& request, bool& storeFull,
          std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now());

    ~McpUploadSink();

    /// Append size bytes of the body. Returns false when the write fails or runs past the stated
    /// length.
    bool write(const char* data, std::size_t size);

    /// Enter the written document into the store and answer the client with its identifier, or with
    /// an error when the body was not written whole.
    void finish(const std::shared_ptr<StreamSocket>& socket);

    /// True when the time the upload was given to arrive whole has passed at now.
    bool isPastDeadline(std::chrono::steady_clock::time_point now) const
    {
        return now > _deadline;
    }

private:
    McpUploadSink(std::size_t size, std::chrono::steady_clock::time_point deadline)
        : _size(size)
        , _deadline(deadline)
    {
    }

    /// The stated length of the body, counted against the store.
    std::size_t _size;
    /// The time by which the whole body is to have arrived.
    std::chrono::steady_clock::time_point _deadline;
    /// The number of body bytes written so far.
    std::size_t _written = 0;
    /// Where the document is written.
    std::string _path;
    std::ofstream _stream;
    /// True once the store owns the file.
    bool _stored = false;
};

namespace McpRequests
{
/// True when the request is for the Model Context Protocol endpoint.
bool isMcpRequest(const RequestDetails& requestDetails);

/// Refuse a request for the Model Context Protocol endpoint that is not to be served, as soon as
/// its header is known. It is refused when the endpoint is off or has no key (404), when it comes
/// from a web page (403), when it lacks the key (401), and for a POST whose body has no stated
/// length (411) or a length over what the path accepts (413). Returns true when a refusal was sent.
bool refuseMcpRequest(const Poco::Net::HTTPRequest& request,
                      const std::shared_ptr<StreamSocket>& socket);

/// Refuse a request as refuseMcpRequest does, as soon as its header is in, when its address is for
/// the Model Context Protocol endpoint. The request itself is left unchanged. Returns true when a
/// refusal was sent.
bool refuseMcpRequestEarly(const Poco::Net::HTTPRequest& request,
                           const std::shared_ptr<StreamSocket>& socket);

/// True when the request is a POST to the upload address of the Model Context Protocol endpoint.
bool isMcpUpload(const Poco::Net::HTTPRequest& request);

/// Serve the Model Context Protocol endpoint at /cool/mcp. Applies the
/// endpoint's own gates (config switch, HTTP method, Origin, bearer key)
/// and then hands the body to the transport-agnostic core. A tool call
/// that loads a document keeps the request open and transfers the socket
/// through the disposition, in the same way a convert-to request does.
/// @return true if request has been handled synchronously and response sent, otherwise false
bool handleMcpRequest(const Poco::Net::HTTPRequest& request,
                      const RequestDetails& requestDetails, std::istream& message,
                      SocketDisposition& disposition,
                      const std::shared_ptr<StreamSocket>& socket);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
