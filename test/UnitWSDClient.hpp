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

/*
 * Base class for WSD client unit tests.
 */

#include <helpers.hpp>
#include <lokassert.hpp>
#include <testlog.hpp>
#include <Unit.hpp>
#include <common/Uri.hpp>

#include <Poco/URI.h>

#include <memory>
#include <string>
#include <vector>

/// Send a command message to WSD from a UnitWSDClient instance on the given connection.
#define WSD_CMD_BY_CONNECTION_INDEX(INDEX, MSG)                                                    \
    do                                                                                             \
    {                                                                                              \
        TST_LOG("Sending from #" << INDEX << ": " << MSG);                                         \
        sendCommand(INDEX, MSG);                                                                   \
    } while (false)

/// Send a command message to WSD from a UnitWSDClient instance on the primary connection.
#define WSD_CMD(MSG) WSD_CMD_BY_CONNECTION_INDEX(0, MSG)

/// A WebSocketSession wrapper to help with testing.
class UnitWebSocket final
{
    std::shared_ptr<http::WebSocketSession> _httpSocket;

public:
    /// Get a websocket connected for a given URL.
    UnitWebSocket(const std::shared_ptr<SocketPoll>& socketPoll, const std::string& documentURL,
                  const std::string& testname = "UnitWebSocket ")
    {
        Poco::URI uri(helpers::getTestServerURI());
        _httpSocket = helpers::connectLOKit(socketPoll, uri, documentURL, testname);
    }

    /// Destroy the WS.
    /// Here, we can't do IO as we don't own the socket (SocketPoll does).
    /// In fact, we can't destroy it (it's referenced by SocketPoll).
    /// Instead, we can only flag for shutting down.
    ~UnitWebSocket() { _httpSocket->asyncShutdown(); }

    const std::shared_ptr<http::WebSocketSession>& getWebSocket() { return _httpSocket; }
};

/// A WSD unit-test base class with support
/// to manage client connections.
/// This cannot be in UnitWSD or UnitBase because
/// we use test code that isn't available in COOLWSD.
class UnitWSDClient : public UnitWSD
{
public:
    explicit UnitWSDClient(const std::string& name)
        : UnitWSD(name)
    {
    }

protected:
    const std::string& getWopiSrc() const { return _wopiSrc; }

    const std::unique_ptr<UnitWebSocket>& getWs() const { return _wsList.at(0); }

    const std::unique_ptr<UnitWebSocket>& getWsAt(int index) { return _wsList.at(index); }

    void deleteSocketAt(int index)
    {
        // Don't remove from the container, because the
        // indexes are how the test refers to them.
        std::unique_ptr<UnitWebSocket>& socket = _wsList.at(index);
        socket.reset();
    }

    /// Where the WOPI URLs of this test point. By default that is coolwsd
    /// itself, which hands /wopi/ paths to the test's handleHttpRequest().
    virtual std::string getWopiHostURI() const { return helpers::getTestServerURI(); }

    std::string initWebsocket(const std::string& wopiName)
    {
        const Poco::URI wopiURL(getWopiHostURI() + wopiName +
                                "&testname=" + getTestname());

        _wopiSrc = Uri::encode(wopiURL.toString());

        // This is just a client connection that is used from the tests.
        TST_LOG("Connecting test client to COOL (#" << (_wsList.size() + 1)
                                                    << " connection): /cool/" << _wopiSrc << "/ws");

        // Insert at the front.
        const auto& _ws = _wsList.emplace(
            _wsList.begin(), std::make_unique<UnitWebSocket>(
                                 socketPoll(), "/cool/" + _wopiSrc + "/ws", getTestname()));

        assert((*_ws).get());

        return _wopiSrc;
    }

    void addWebSocket()
    {
        // This is just a client connection that is used from the tests.
        TST_LOG("Connecting test client to COOL (#" << (_wsList.size() + 1)
                                                    << " connection): /cool/" << _wopiSrc << "/ws");

        // Insert at the back.
        const auto& _ws = _wsList.emplace(
            _wsList.end(), std::make_unique<UnitWebSocket>(
                               socketPoll(), "/cool/" + _wopiSrc + "/ws", getTestname()));

        assert((*_ws).get());
    }

    void endTest(const std::string& reason) override
    {
        TST_LOG("Ending test by disconnecting " << _wsList.size() << " connection(s): " << reason);
        _wsList.clear();
        UnitWSD::endTest(reason);
    }

    /// Send a command to WSD.
    void sendCommand(int index, const std::string& msg)
    {
        LOK_ASSERT_SILENT(index >= 0 && static_cast<std::size_t>(index) < _wsList.size());
        helpers::sendTextFrame(getWsAt(index)->getWebSocket(), msg, getTestname());
        SocketPoll::wakeupWorld();
    }

    /// Connect to a local test document (not a fake wopi URL), without loading it.
    /// Returns the document URL to use for loading.
    std::string connectToLocalDocument(const std::string& docFilename)
    {
        std::string documentPath, documentURL;
        helpers::getDocumentPathAndURL(docFilename, documentPath, documentURL, getTestname());

        TST_LOG("Connecting to local document [" << docFilename << "] with URL: " << documentURL);
        _wsList.emplace_back(
            std::make_unique<UnitWebSocket>(socketPoll(), documentURL, getTestname()));

        return documentURL;
    }

    /// Connect and load a local test document (not a fake wopi document).
    void connectAndLoadLocalDocument(const std::string& docFilename)
    {
        const std::string documentURL = connectToLocalDocument(docFilename);

        TST_LOG("Loading local document [" << docFilename << "] with URL: " << documentURL);
        WSD_CMD("load url=" + documentURL);
    }

private:
    /// The WOPISrc URL.
    std::string _wopiSrc;

    /// Websockets to communicate.
    std::vector<std::unique_ptr<UnitWebSocket>> _wsList;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
