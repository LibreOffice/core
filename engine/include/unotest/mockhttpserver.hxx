/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <unotest/detail/unotestdllapi.hxx>

namespace test
{
/// A small HTTP server on 127.0.0.1 for a unit test, on a thread of its own:
/// a handler of the test's answers each request, and every request is kept
/// for the test to look at. Stands in for a remote service, such as a
/// sentence checker or a translator, so a test needs no network and no key.
class OOO_DLLPUBLIC_UNOTEST MockHttpServer
{
public:
    struct Request
    {
        /// "GET" or "POST".
        std::string aMethod;
        /// The path with its query, e.g. "/v2/translate".
        std::string aPath;
        /// The header lines, each ending in CR LF.
        std::string aHeaders;
        std::string aBody;
    };

    struct Response
    {
        int nStatus = 200;
        std::string aStatusText = "OK";
        /// Sent as the Content-Type header; none when empty.
        std::string aContentType = "application/json";
        std::string aBody;
    };

    using Handler = std::function<Response(const Request&)>;

    MockHttpServer(sal_uInt16 nPort, Handler aHandler);
    ~MockHttpServer();

    MockHttpServer(const MockHttpServer&) = delete;
    MockHttpServer& operator=(const MockHttpServer&) = delete;

    /// Binds the port and starts answering. False when the port is taken.
    bool start();
    /// Stops answering; nothing to do when not started.
    void stop();

    /// "http://127.0.0.1:<port>" with rPath appended.
    OUString getUrl(std::u16string_view rPath = std::u16string_view()) const;

    /// The requests answered so far, in order.
    std::vector<Request> getRequests() const;

private:
    class Thread;
    std::unique_ptr<Thread> m_pThread;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
