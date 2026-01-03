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
#include "wopi/CheckFileInfo.hpp"

#include <net/HttpRequest.hpp>
#include <net/Socket.hpp>

#include <Poco/JSON/Object.h>

#include <istream>
#include <memory>
#include <string>

/// Proxies file download/upload through the /co/collab endpoint.
/// Takes WOPISrc from URL and access_token from cookie.
class CollabFileProxy: public std::enable_shared_from_this<CollabFileProxy>
{
public:
    CollabFileProxy(std::string id, const RequestDetails& requestDetails,
                    const std::shared_ptr<StreamSocket>& socket,
                    const std::string& wopiSrc, const std::string& accessToken,
                    bool isUpload);

    void handleRequest(std::istream& message, const std::shared_ptr<TerminatingPoll>& poll,
                       SocketDisposition& disposition);

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

    /// Handle download/upload with pre-validated WOPI info from CollabBroker.
    /// This bypasses CheckFileInfo since the collab session already authenticated.
    void handleDirectRequest(std::istream& message,
                             Poco::JSON::Object::Ptr wopiInfo,
                             const std::shared_ptr<TerminatingPoll>& poll,
                             SocketDisposition& disposition);

private:
    inline void logPrefix(std::ostream& os) const { os << '#' << _logFD << ": "; }

    void checkFileInfo(const std::shared_ptr<TerminatingPoll>& poll, const Poco::URI& uri,
                       int redirectionLimit);
    void doDownload(const std::shared_ptr<TerminatingPoll>& poll, const Poco::URI& uri,
                    int redirectionLimit);
    void doUpload(const std::shared_ptr<TerminatingPoll>& poll, const Poco::URI& uri,
                  const std::string& body);

    const std::string _id;
    const RequestDetails _requestDetails;
    const std::weak_ptr<StreamSocket> _socket;
    const std::string _wopiSrc;
    const std::string _accessToken;
    const bool _isUpload;
    std::shared_ptr<http::Session> _httpSession;
    std::shared_ptr<CheckFileInfo> _checkFileInfo;
    std::string _uploadBody;
    int _logFD;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
