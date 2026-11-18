/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
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
#include <span>
#include <string>
#include <string_view>

/// Proxies file download/upload through the /cool/collab endpoint
/// using token-based authentication.
class CollabFileProxy: public std::enable_shared_from_this<CollabFileProxy>
{
public:
    CollabFileProxy(std::string id, const RequestDetails& requestDetails,
                    const std::shared_ptr<StreamSocket>& socket,
                    const std::string& wopiSrc, const std::string& accessToken);

    /// Handle a fetch request with a pre-authorized URL (from WebSocket token)
    /// This bypasses CheckFileInfo since the token already validated access.
    /// The body goes to the client with the value of each of secretFields left empty.
    void handleFetchRequest(const std::string& streamUrl,
                            const std::shared_ptr<TerminatingPoll>& poll,
                            SocketDisposition& disposition,
                            std::span<const std::string_view> secretFields = {});

    /// Handle an upload request with a pre-authorized URL (from WebSocket token)
    /// This bypasses CheckFileInfo since the token already validated access.
    void handleUploadRequest(const std::string& targetUrl, std::string const & lastModifiedTime,
                             std::istream& message,
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
    /// The fields of the downloaded body that hold a stored secret. The lists have static
    /// storage duration.
    std::span<const std::string_view> _secretFields;
    std::string _uploadBody;
    std::string _uploadLastModifiedTime;
    int _logFD;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
