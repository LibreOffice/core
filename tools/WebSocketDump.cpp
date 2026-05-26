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

/* A simple tool that accepts web-socket connections and dumps the contents */

#include <config.h>

#include <common/HexUtil.hpp>
#include <common/Log.hpp>
#include <common/Util.hpp>
#include <common/Protocol.hpp>
#include <net/ServerSocket.hpp>
#include <net/WebSocketHandler.hpp>
#if !MOBILEAPP
#include <net/HttpHelper.hpp>
#endif
#if ENABLE_SSL
#include <net/SslSocket.hpp>
#endif

#include <Poco/URI.h>
#include <Poco/MemoryStream.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Util/XMLConfiguration.h>

#include <unistd.h>

// Dumps incoming websocket messages and doesn't respond.
class DumpSocketHandler : public WebSocketHandler
{
public:
    DumpSocketHandler(const std::weak_ptr<StreamSocket>& socket,
                      const Poco::Net::HTTPRequest& request)
        : WebSocketHandler(socket.lock(), request, true)
    {
    }

private:
    /// Process incoming websocket messages
    void handleMessage(const std::vector<char> &data) override
    {
        std::cout << "WebSocket message data:\n";
        HexUtil::dumpHex(std::cout, data, "", "    ", false);
    }
};

/// Handles incoming connections and dispatches to the appropriate handler.
class ClientRequestDispatcher final : public SimpleSocketHandler
{
public:
    ClientRequestDispatcher()
    {
    }

private:

    /// Set the socket associated with this ResponseClient.
    void onConnect(const std::shared_ptr<StreamSocket>& socket) override
    {
        _socket = socket;
        LOG_TRC('#' << socket->getFD() << " Connected to ClientRequestDispatcher");
    }

    void onDisconnect() override
    {
        LOG_TRC("ClientRequestDispatcher disconnected");
        std::shared_ptr<StreamSocket> socket = _socket.lock();
        if (socket)
        {
            socket->asyncShutdown(); // Flag for shutdown for housekeeping in SocketPoll.
            socket->shutdownConnection(); // Immediately disconnect.
        }
    }

    /// Called after successful socket reads.
    void handleIncomingMessage(SocketDisposition &disposition) override
    {
        std::shared_ptr<StreamSocket> socket = _socket.lock();
        if (!socket)
        {
            LOG_ERR("Invalid socket while reading client message");
            return;
        }

        Buffer& in = socket->getInBuffer();
        LOG_TRC('#' << socket->getFD() << " handling incoming " << in.size() << " bytes");

        // Find the end of the header, if any.
        constexpr std::string_view marker("\r\n\r\n");
        auto itBody = std::search(in.begin(), in.end(),
                                  marker.begin(), marker.end());
        if (itBody == in.end())
        {
            LOG_DBG('#' << socket->getFD() << " doesn't have enough data yet");
            return;
        }

        // Skip the marker.
        itBody += marker.size();

        Poco::MemoryInputStream message(in.data(), in.size());
        Poco::Net::HTTPRequest request;
        try
        {
            request.read(message);

            LOG_INF('#' << socket->getFD() << ": Client HTTP Request: " << request);

            const std::streamsize contentLength = request.getContentLength();
            const auto offset = itBody - in.begin();
            const std::streamsize available = in.size() - offset;

            if (contentLength != Poco::Net::HTTPMessage::UNKNOWN_CONTENT_LENGTH && available < contentLength)
            {
                LOG_DBG("Not enough content yet: ContentLength: " << contentLength << ", available: " << available);
                return;
            }
        }
        catch (const std::exception& exc)
        {
            // Probably don't have enough data just yet.
            // TODO: timeout if we never get enough.
            return;
        }

        try
        {
            // Routing
            Poco::URI requestUri(request.getURI());
            std::vector<std::string> reqPathSegs;
            requestUri.getPathSegments(reqPathSegs);

            LOG_INF("Incoming websocket request: " << request.getURI());

            const std::string& requestURI = request.getURI();
            StringVector pathTokens(StringVector::tokenize(requestURI, '/'));
            if (request.find("Upgrade") != request.end()
                && Util::iequal(request["Upgrade"], "websocket"))
            {
                auto dumpHandler = std::make_shared<DumpSocketHandler>(_socket, request);
                socket->setHandler(dumpHandler);
                dumpHandler->sendTextMessage("version");
                dumpHandler->sendTextMessage("documents");
            }
            else
            {
                http::Response response(http::StatusCode::BadRequest);
                response.setContentLength(0);
                LOG_INF("DumpWebSockets bad request");
                socket->send(response);
                disposition.setClosed();
            }
        }
        catch (const std::exception& exc)
        {
            // Bad request.
            HttpHelper::sendErrorAndShutdown(http::StatusCode::BadRequest, socket);

            // NOTE: Check _wsState to choose between HTTP response or WebSocket (app-level) error.
            LOG_INF('#' << socket->getFD() << " Exception while processing incoming request: [" <<
                    COOLProtocol::getAbbreviatedMessage(in) << "]: " << exc.what());
        }

        // if we succeeded - remove the request from our input buffer
        // we expect one request per socket
        in.erase(in.begin(), itBody);
    }

    int getPollEvents(std::chrono::steady_clock::time_point /* now */,
                      int64_t & /* timeoutMaxMicroS */) override
    {
        return POLLIN;
    }

    void performWrites(std::size_t /*capacity*/) override {}


    // The socket that owns us (we can't own it).
    std::weak_ptr<StreamSocket> _socket;
};

class DumpSocketFactory final : public SocketFactory
{
private:
    bool _isSSL = false;

public:
    DumpSocketFactory(bool isSSL) : _isSSL(isSSL) {}

    std::shared_ptr<Socket> create(const int physicalFd, Socket::Type type) override
    {
#if ENABLE_SSL
        if (_isSSL)
            return StreamSocket::create<SslStreamSocket>(
                std::string(), physicalFd, type, false, HostType::Other,
                std::make_shared<ClientRequestDispatcher>());
#else
        (void)_isSSL;
#endif
        return StreamSocket::create<StreamSocket>(
            std::string(), physicalFd, type, false, HostType::Other,
            std::make_shared<ClientRequestDispatcher>());
    }
};

namespace Util
{
    void alertAllUsers(const std::string& cmd, const std::string& kind)
    {
        std::cout << "error: cmd=" << cmd << " kind=" << kind << std::endl;
    }
}

class CoolConfig final: public Poco::Util::XMLConfiguration
{
public:
    CoolConfig()
        {}
};

// coverity[root_function] : don't warn about uncaught exceptions
int main (int argc, char **argv)
{
    (void) argc; (void) argv;

    std::shared_ptr<SocketPoll> DumpSocketPoll = std::make_shared<SocketPoll>("websocket");

    if (!UnitWSD::init(UnitWSD::UnitType::Wsd, ""))
    {
        throw std::runtime_error("Failed to load wsd unit test library");
    }

    Log::initialize("WebSocketDump", "trace", true, false,
                    std::map<std::string, std::string>(), false,
                    std::map<std::string, std::string>());

    CoolConfig config;
    config.load("coolwsd.xml");

    // read the port & ssl support
    int port = 9042;
    bool isSSL = false;
    std::string monitorAddress = config.getString("monitors.monitor");
    if (!monitorAddress.empty())
    {
        Poco::URI monitorURI(monitorAddress);
        port = monitorURI.getPort();
        isSSL = (monitorURI.getScheme() == "wss");
    }

#if ENABLE_SSL
    // hard coded but easy for now.
    const std::string ssl_cert_file_path = "etc/cert.pem";
    const std::string ssl_key_file_path = "etc/key.pem";
    const std::string ssl_ca_file_path = "etc/ca-chain.cert.pem";
    const std::string ssl_cipher_list = "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH";

    // Initialize the non-blocking socket SSL.
    if (isSSL)
        ssl::Manager::initializeServerContext(ssl_cert_file_path, ssl_key_file_path,
                                              ssl_ca_file_path, ssl_cipher_list,
                                              ssl::CertificateVerification::Disabled);
#endif

    std::shared_ptr<SocketPoll> acceptPoll = std::make_shared<SocketPoll>("accept");

    // Setup listening socket with a factory for connected sockets.
    auto serverSocket = std::make_shared<ServerSocket>(
        Socket::Type::All,
        std::chrono::steady_clock::now(),
        *DumpSocketPoll,
        std::make_shared<DumpSocketFactory>(isSSL));

    if (!serverSocket->bind(ServerSocket::Type::Public, port))
    {
        fprintf(stderr, "Failed to bind websocket to port %d\n", port);
        return -1;
    }

    if (!serverSocket->listen())
    {
        fprintf(stderr, "Failed to listen on websocket, port %d\n", port);
        return -1;
    }

    acceptPoll->startThread();
    acceptPoll->insertNewSocket(serverSocket);

    while (true)
    {
        DumpSocketPoll->poll(std::chrono::seconds(100));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
