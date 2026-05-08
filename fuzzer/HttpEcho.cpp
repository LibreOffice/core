/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Simple HTTP echo server for fuzzing and testing.
 * Functions: HTTP request echo, socket handling
 */

#include <config.h>

#include <cstdlib>

#include <Socket.hpp>
#include <test/HttpTestServer.hpp>

#include <Poco/URI.h>

#include <chrono>
#include <memory>
#include <stdexcept>
#include <string>
#include <test/lokassert.hpp>

#if ENABLE_SSL
#include <Ssl.hpp>
#include <net/SslSocket.hpp>
#endif
#include <net/ServerSocket.hpp>
#include <net/HttpRequest.hpp>
#include <net/AsyncDNS.hpp>
#include <common/FileUtil.hpp>
#include <common/Util.hpp>
#include <fuzzer/Common.hpp>

class HttpRequestTests final
{
    std::string _localUri;
    SocketPoll _pollServerThread;
    std::shared_ptr<http::Session> _httpSession;
    SocketPoll _poller;
    bool _completed;

    class ServerSocketFactory final : public SocketFactory
    {
        std::shared_ptr<Socket> create(const int physicalFd, Socket::Type type) override
        {
            return StreamSocket::create<StreamSocket>("localhost", physicalFd, type, false,
                                                      HostType::LocalHost, std::make_shared<ServerRequestHandler>());
        }
    };

public:
    HttpRequestTests()
        : _pollServerThread("HttpServerPoll")
        , _poller("HttpSynReqPoll")
    {
        net::AsyncDNS::startAsyncDNS();
        _poller.runOnClientThread();

        std::map<std::string, std::string> logProperties;
        const auto log_level = std::getenv("LOG_LEVEL");
        if (log_level)
        {
            Log::initialize("fuz", log_level ? log_level : "error", isatty(fileno(stderr)), false,
                            logProperties, false, {});
        }

        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        std::shared_ptr<SocketFactory> factory = std::make_shared<ServerSocketFactory>();
        int port = 9990;
        std::shared_ptr<ServerSocket> socket;
        for (int i = 0; i < 40; ++i, ++port)
        {
            // Try listening on this port.
            socket = ServerSocket::create(ServerSocket::Type::Local, port, Socket::Type::IPv4,
                                           now, _pollServerThread, factory);
            if (socket)
                break;
        }

        if (!socket)
        {
            std::cerr << "Failed to create server socket on any port and gave up at port #" << port
                      << std::endl;
            exit(1);
        }

        _localUri = "http://127.0.0.1:" + std::to_string(port);
        _pollServerThread.startThread();
        _pollServerThread.insertNewSocket(socket);

        _httpSession = http::Session::create(localUri());
        if (!_httpSession)
            throw std::runtime_error("Failed to create http::Session to " + localUri());

        _httpSession->setTimeout(std::chrono::milliseconds(500));
        _httpSession->setFinishedHandler(
            [&](const std::shared_ptr<http::Session>&)
            {
                _completed = true;
                return true;
            });
    }

    ~HttpRequestTests()
    {
        _pollServerThread.stop();
        net::AsyncDNS::stopAsyncDNS();
    }

    const std::string& localUri() const { return _localUri; }
    std::shared_ptr<http::Session> session() const { return _httpSession; }
    SocketPoll& poller() { return _poller; };
    bool isCompleted() const { return _completed; };
    void resetCompleted()
    {
        _completed = false;
        _poller.removeSockets(); // We don't need stale sockets from prevous tests.
    };
};

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    static bool initialized = fuzzer::DoInitialization();
    (void)initialized;

    static HttpRequestTests test;

    http::Request httpRequest("/inject/" + HexUtil::bytesToHexString(data, size));

    test.resetCompleted();

    const std::shared_ptr<const http::Response> httpResponse =
        test.session()->syncRequest(httpRequest, test.poller());

    CHECK(httpResponse->done());
    CHECK(test.isCompleted()); // The onFinished callback must always be called.

    if (httpResponse->state() == http::Response::State::Complete)
    {
        CHECK(!httpResponse->statusLine().httpVersion().empty());
        // CHECK(!httpResponse->statusLine().reasonPhrase().empty());

        // CHECK(httpResponse->statusLine().statusCode() >= 100);
        // CHECK(httpResponse->statusLine().statusCode() < 600);
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
