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
 * Simple HTTP/WebSocket client for testing and benchmarking purposes.
 * Application: command-line test client using Poco networking
 */

#include <config.h>

#include <common/NumUtil.hpp>
#include <common/Util.hpp>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <thread>
#include <memory>
#include <assert.h>

#include <Poco/Net/HTMLForm.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/FilePartSource.h>
#include <Poco/Net/SSLManager.h>
#include <Poco/Net/WebSocket.h>
#include <Poco/Net/AcceptCertificateHandler.h>
#include <Poco/StreamCopier.h>
#include <Poco/URI.h>
#include <Poco/Util/Application.h>
#include <Poco/Util/HelpFormatter.h>
#include <Poco/Util/Option.h>
#include <Poco/Util/OptionSet.h>
#include <Poco/Runnable.h>

using Poco::Runnable;
using Poco::Net::HTTPRequest;
using Poco::Net::HTTPResponse;
using Poco::Net::WebSocket;

const char *HostName = "127.0.0.1";
constexpr int HttpPortNumber = 9191;
constexpr int SslPortNumber = 9193;

static bool EnableHttps = false;
bool EnableExperimental = false;

struct Session
{
private:
    std::string _session_name;
    Poco::Net::HTTPClientSession *_session;

public:
    Session(const char *session_name, bool https = false)
        : _session_name(session_name)
    {
#if ENABLE_SSL
        if (https)
            _session = new Poco::Net::HTTPSClientSession(HostName, SslPortNumber);
        else
            _session = new Poco::Net::HTTPClientSession(HostName, HttpPortNumber);
#else
        (void)https;
        _session = new Poco::Net::HTTPClientSession(HostName, HttpPortNumber);
#endif
    }
    ~Session()
    {
        delete _session;
    }

    void sendPing(int i)
    {
        Poco::Net::HTTPRequest request(
            Poco::Net::HTTPRequest::HTTP_POST,
            "/ping/" + _session_name + '/' + std::to_string(i));
        try {
            Poco::Net::HTMLForm form;
            form.setEncoding(Poco::Net::HTMLForm::ENCODING_MULTIPART);
            form.prepareSubmit(request);
            form.write(_session->sendRequest(request));
        }
        catch (const Poco::Exception &e)
        {
            std::cerr << "Failed to write data: " << e.name() <<
                  ' ' << e.message() << '\n';
            throw;
        }
    }

    std::string getResponseString()
    {
        Poco::Net::HTTPResponse response;
        std::istream& responseStream = _session->receiveResponse(response);
        return std::string(std::istreambuf_iterator<char>(responseStream), {});
    }

    int getResponseInt()
    {
        int number = 42;

        try {
//            std::cerr << "try to get response\n";
            const std::string result = getResponseString();
            number = NumUtil::stoi(result);
        }
        catch (const Poco::Exception &e)
        {
            std::cerr << "Exception converting: " << e.name() <<
                  ' ' << e.message() << '\n';
            throw;
        }
        return number;
    }

    std::shared_ptr<WebSocket> getWebSocket()
    {
        _session->setTimeout(Poco::Timespan(10, 0));
        HTTPRequest request(HTTPRequest::HTTP_GET, "/ws");
        HTTPResponse response;
        return std::make_shared<WebSocket>(*_session, request, response);
    }
};

struct ThreadWorker : public Runnable
{
private:
    const char *_domain;
public:
    ThreadWorker(const char *domain = nullptr)
        : _domain(domain)
    {
    }
    virtual void run()
    {
        for (int i = 0; i < 100; ++i)
        {
            Session ping(_domain ? _domain : "init", EnableHttps);
            ping.sendPing(i);
#ifndef NDEBUG
            int back =
#endif
                ping.getResponseInt();
            assert(back == i + 1);
        }
    }
};

struct Client : public Poco::Util::Application
{
    void testPing()
    {
        std::cerr << "testPing\n";
        Session first("init", EnableHttps);
        Session second("init", EnableHttps);

        int count = 42, back;
        first.sendPing(count);
        second.sendPing(count + 1);

        back = first.getResponseInt();
        std::cerr << "testPing: " << back << '\n';
        assert (back == count + 1);

        back = second.getResponseInt();
        std::cerr << "testPing: " << back << '\n';
        assert (back == count + 2);
    }

    void testLadder()
    {
        std::cerr << "testLadder\n";
        ThreadWorker ladder;
        std::thread thread([&ladder]{ladder.run();});
        thread.join();
    }

    void testParallel()
    {
        std::cerr << "testParallel\n";
        const int num = 10;
        std::thread snakes[num];
        ThreadWorker ladders[num];

        for (size_t i = 0; i < num; i++)
            snakes[i] = std::thread([&ladders, i]{ladders[i].run();});

        for (int i = 0; i < num; i++)
            snakes[i].join();
    }

    void testWebsocketPingPong()
    {
        std::cerr << "testWebsocketPingPong\n";
        Session session("ws", EnableHttps);
        std::shared_ptr<WebSocket> ws = session.getWebSocket();

        std::string send = "hello there";
        ws->sendFrame(send.data(), send.length(),
                      WebSocket::SendFlags::FRAME_TEXT);

        for (size_t i = 0; i < 10; i++)
        {
            ws->sendFrame(&i, sizeof(i), WebSocket::SendFlags::FRAME_BINARY);
            size_t back[5];
            int flags = 0;
#ifndef NDEBUG
            int recvd =
#endif
                ws->receiveFrame((void *)back, sizeof(back), flags);
            assert(recvd == sizeof(size_t));
            assert(back[0] == i + 1);
        }
    }

    void testWebsocketEcho()
    {
        std::cerr << "testwebsocketEcho\n";
        Session session("ws", EnableHttps);
        std::shared_ptr<WebSocket> ws = session.getWebSocket();

        std::vector<char> res;

        std::srand(std::time(nullptr));

        std::vector<char> data;
        for (size_t i = 1; i < (1 << 14); ++i)
        {
            data.push_back((char)(Util::rng::getNext() / (UINT_MAX/256)));
            ws->sendFrame(data.data(), data.size(), WebSocket::SendFlags::FRAME_BINARY);

            res.resize(i);
            int flags;
#ifndef NDEBUG
            int recvd =
#endif
                ws->receiveFrame(res.data(), res.size(), flags);
            assert(recvd == static_cast<int>(i));

            if (i == sizeof(size_t))
            {
                assert(*reinterpret_cast<const size_t*>(res.data()) ==
                       *reinterpret_cast<const size_t*>(data.data()) + 1);
            }
            else
            {
                assert(res == data);
            }
        }
    }

public:
    // coverity[root_function] : don't warn about uncaught exceptions
    int main(const std::vector<std::string>& args) override
    {
        EnableHttps = (args.size() > 0 && args[0] == "ssl");
        std::cerr << "Starting " << (EnableHttps ? "HTTPS" : "HTTP") << " client." << std::endl;

#if ENABLE_SSL
        if (EnableHttps)
        {
            Poco::Net::initializeSSL();
            // Just accept the certificate anyway for testing purposes
            Poco::SharedPtr<Poco::Net::InvalidCertificateHandler> invalidCertHandler = new Poco::Net::AcceptCertificateHandler(false);

            Poco::Net::Context::Params sslParams;
            Poco::Net::Context::Ptr sslContext = new Poco::Net::Context(Poco::Net::Context::CLIENT_USE, sslParams);
            Poco::Net::SSLManager::instance().initializeClient(nullptr, std::move(invalidCertHandler), std::move(sslContext));
        }
#endif

        testWebsocketPingPong();
        testWebsocketEcho();

        testPing();
        testLadder();
        testParallel();

        return 0;
    }
};

// coverity[root_function] : don't warn about uncaught exceptions
POCO_APP_MAIN(Client)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
