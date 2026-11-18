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

/*
 * Unit test for handling initial document load failures.
 */

#include <config.h>

#include <lokassert.hpp>
#include <Unit.hpp>
#include <WopiTestServer.hpp>
#include <common/Log.hpp>
#include <helpers.hpp>
#include <wsd/ClientSession.hpp>

#include <Poco/Net/HTTPRequest.h>

#include <chrono>

using namespace std::literals;

/// This is to test that: if a file initially fails to load for some transient
/// reason, f.e. timeout, then a subsequent load where there is no such failure
/// should succeed.
class InitialLoadFail : public WopiTestServer
{
    STATE_ENUM(Phase, LoadAttempt, WaitLoad, LoadSuccess, Done) _phase;

    std::size_t _getFileCount;

public:
    InitialLoadFail()
        : WopiTestServer("Initial Load Fail")
        , _phase(Phase::LoadAttempt)
        , _getFileCount(0)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        UnitWSD::configure(config);

        config.setInt("net.connection_timeout_secs", 3);
    }

    bool handleHttpRequest(const Poco::Net::HTTPRequest& request,
                           std::istream& message,
                           const std::shared_ptr<StreamSocket>& socket) override
    {
        return WopiTestServer::handleHttpRequest(request, message, socket);
    }

    std::map<std::string, std::string>
        parallelizeCheckInfo(const Poco::Net::HTTPRequest& request,
                             std::istream& /*message*/,
                             const std::shared_ptr<StreamSocket>& /*socket*/) override
    {
        std::string uri = Uri::decode(request.getURI());
        TST_LOG("parallelizeCheckInfo requested: " << uri);
        return std::map<std::string, std::string>{
            {"wopiSrc", "/wopi/files/0"},
            {"accessToken", "anything"},
            {"noAuthHeader", ""},
            {"permission", ""},
            {"configid", ""}
        };
    }

    bool handleGetFileRequest(const Poco::Net::HTTPRequest& request,
                              const std::shared_ptr<StreamSocket>& socket) override
    {
        if (_getFileCount++ == 0)
        {
            TST_LOG("handleGetFileRequest: force timeout for 1st load\n");
            std::this_thread::sleep_for(5000ms);
            return true;
        }
        return WopiTestServer::handleGetFileRequest(request, socket);
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& /*request*/,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        fileInfo->set("UserCanWrite", "true");
    }

    bool onFilterSendWebSocketMessage(const std::string_view message, const WSOpCode /* code */,
                                      const bool /* flush */, int& /*unitReturn*/) override
    {
        TST_LOG("onFilterSendWebSocketMessage:" << message);

        if (message.starts_with("session "))
        {
            TST_LOG("PASS, session seen: " << message);
            TRANSITION_STATE(_phase, Phase::LoadSuccess);
        }

        if (message.starts_with("error: cmd=storage"))
        {
            TST_LOG("Load failed, explicit retry: " << message);
            TRANSITION_STATE(_phase, Phase::LoadAttempt);
        }

        return false;
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::LoadAttempt:
            {
                // Always transition before issuing commands.
                TRANSITION_STATE(_phase, Phase::WaitLoad);

                TST_LOG("Attempt first load which should timeout once");

                initWebsocket("/wopi/files/0?access_token=anything");
                WSD_CMD_BY_CONNECTION_INDEX(0, "load url=" + getWopiSrc());

                break;
            }
            case Phase::LoadSuccess:
            {
                TRANSITION_STATE(_phase, Phase::Done);
                passTest("Load Succeeded");
                break;
            }
            case Phase::WaitLoad:
            case Phase::Done:
                break;
        }
    }
};

/// This is to test that: if a file fails to load due to an Unauthorized response,
/// then we don't leak sockets
class UnauthorizedSocketLeak : public WopiTestServer
{
    STATE_ENUM(Phase, LoadAttempt, WaitUnauthorized, WaitSocketDtor, Done) _phase;

    std::weak_ptr<StreamSocket> _ensureNotLeaked;

public:
    UnauthorizedSocketLeak()
        : WopiTestServer("Unauthorized Load Leak")
        , _phase(Phase::LoadAttempt)
    {
    }

    // enables parallel checkFileInfo connection
    std::map<std::string, std::string>
        parallelizeCheckInfo(const Poco::Net::HTTPRequest& request,
                             std::istream& /*message*/,
                             const std::shared_ptr<StreamSocket>& socket) override
    {
        // We want to test that this socket dtor will get called when this
        // session is Unauthorized. Without the fix the test times out and
        // fails.
        _ensureNotLeaked = socket;

        std::string uri = Uri::decode(request.getURI());
        TST_LOG("parallelizeCheckInfo requested: " << uri);
        return std::map<std::string, std::string>{
            {"wopiSrc", "/wopi/files/0"},
            {"accessToken", "anything"},
            {"noAuthHeader", ""},
            {"permission", ""},
            {"configid", ""}
        };
    }

    bool handleCheckFileInfoRequest(const Poco::Net::HTTPRequest& /*request*/,
                                    const std::shared_ptr<StreamSocket>& socket) override
    {
        std::unique_ptr<http::Response> httpResponse =
            std::make_unique<http::Response>(http::StatusCode::Unauthorized);
        socket->sendAndShutdown(*httpResponse);
        TRANSITION_STATE(_phase, Phase::WaitUnauthorized);
        return true;
    }

    bool onFilterSendWebSocketMessage(const std::string_view message, const WSOpCode /* code */,
                                      const bool /* flush */, int& /*unitReturn*/) override
    {
        TST_LOG("onFilterSendWebSocketMessage:" << message);

        if (message.starts_with("error: cmd=internal kind=unauthorized"))
        {
            TST_LOG("Expected Unauthorized load failed: " << message);
            TRANSITION_STATE(_phase, Phase::WaitSocketDtor);
        }

        return false;
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::LoadAttempt:
            {
                // Always transition before issuing commands.
                TRANSITION_STATE(_phase, Phase::WaitUnauthorized);

                TST_LOG("Attempt load which should fail with Unauthorized");

                initWebsocket("/wopi/files/0?access_token=anything");
                WSD_CMD_BY_CONNECTION_INDEX(0, "load url=" + getWopiSrc());

                break;
            }
            case Phase::WaitSocketDtor:
            {
                if (_ensureNotLeaked.expired())
                {
                    TRANSITION_STATE(_phase, Phase::Done);
                    passTest("Socket not leaked");
                }
                break;
            }
            case Phase::WaitUnauthorized:
            case Phase::Done:
                break;
        }
    }
};

UnitBase** unit_create_wsd_multi(void)
{
    return new UnitBase* [3]
    {
        new InitialLoadFail(),
        new UnauthorizedSocketLeak(),
        nullptr
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
