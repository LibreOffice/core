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

#include "RequestDetails.hpp"

#include <net/HttpRequest.hpp>
#include <net/Socket.hpp>

#include <istream>
#include <memory>
#include <string>

/// Proxies file download/upload through the /co/collab endpoint
/// using token-based authentication.
class CollabFileProxy: public std::enable_shared_from_this<CollabFileProxy>
{
public:
    CollabFileProxy(std::string id, const RequestDetails& requestDetails,
                    const std::shared_ptr<StreamSocket>& socket,
                    const std::string& wopiSrc, const std::string& accessToken);

    /// Handle a fetch request with a pre-authorized URL (from WebSocket token)
    /// This bypasses CheckFileInfo since the token already validated access.
    void handleFetchRequest(const std::string& streamUrl,
                            const std::shared_ptr<TerminatingPoll>& poll,
                            SocketDisposition& disposition);

    /// Handle an upload request with a pre-authorized URL (from WebSocket token)
    /// This bypasses CheckFileInfo since the token already validated access.
    void handleUploadRequest(const std::string& targetUrl, std::istream& message,
                             const std::shared_ptr<TerminatingPoll>& poll,
                             SocketDisposition& disposition);

private:
    inline void logPrefix(std::ostream& os) const { os << '#' << _logFD << ": "; }

    void doDownload(const std::shared_ptr<TerminatingPoll>& poll, const Poco::URI& uri,
                    int redirectionLimit);
    void doUpload(const std::shared_ptr<TerminatingPoll>& poll, const Poco::URI& uri,
                  const std::string& body);

    const std::string _id;
    const RequestDetails _requestDetails;
    const std::weak_ptr<StreamSocket> _socket;
    const std::string _wopiSrc;
    const std::string _accessToken;
    std::shared_ptr<http::Session> _httpSession;
    std::string _uploadBody;
    int _logFD;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
