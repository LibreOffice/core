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

#if !ENABLE_WOPI
#error This file should be excluded from builds without the WOPI storage backend
#endif // !ENABLE_WOPI

#include <RequestDetails.hpp>
#include <wopi/CheckFileInfo.hpp>
#include <Storage.hpp>

#include <istream>
#include <optional>
#include <string>
#include <utility>

/// Responsible for HTTP-serving a Wopi document, after authenticating.
class WopiProxy
{
public:
    WopiProxy(std::string id, const RequestDetails& requestDetails,
              const std::shared_ptr<StreamSocket>& socket)
        : _id(std::move(id))
        , _requestDetails(requestDetails)
        , _socket(socket)
        , _logFD(socket->getFD())
    {
    }

    void handleRequest(std::istream & message, const std::shared_ptr<TerminatingPoll>& poll,
                       SocketDisposition& disposition);

private:
    void logPrefix(std::ostream& os) const { os << '#' << _logFD << ": "; }

    void checkFileInfo(const std::shared_ptr<TerminatingPoll>& poll, const Poco::URI& uri,
                       std::optional<std::string> const & postBody, int redirectionLimit);
    void transfer(const std::shared_ptr<TerminatingPoll>& poll, const std::string& url,
                  std::optional<std::string> const & postBody,
                  const Poco::URI& uriPublic, int redirectionLimit);

    const std::string _id;
    const RequestDetails _requestDetails;
    const std::weak_ptr<StreamSocket> _socket;
    std::shared_ptr<http::Session> _httpSession;
    std::shared_ptr<CheckFileInfo> _checkFileInfo;
    const int _logFD;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
