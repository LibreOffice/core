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
 * Unit test for WOPI HTTP redirect handling.
 */

#include <config.h>

#include <WopiTestServer.hpp>
#include <common/Log.hpp>
#include <Unit.hpp>
#include <UnitHTTP.hpp>
#include <helpers.hpp>
#include <wsd/Storage.hpp>

#include <Poco/Net/HTTPRequest.h>

#include <regex>
#include <string>

class UnitWopiHttpRedirect : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoad, Redirected, GetFile, Redirected2, Done) _phase;

    const std::string params = "access_token=anything";

public:
    UnitWopiHttpRedirect()
        : WopiTestServer("UnitWopiHttpRedirect")
        , _phase(Phase::Load)
    {
    }

    virtual bool handleHttpRequest(const Poco::Net::HTTPRequest& request,
                                   std::istream& message,
                                   const std::shared_ptr<StreamSocket>& socket) override
    {
        Poco::URI uriReq(request.getURI());
        static const std::regex regInfo("/wopi/files/1");
        std::string redirectUri = "/wopi/files/0";
        static const std::regex regRedirected("/wopi/files/0");
        static const std::regex regContents("/wopi/files/0/contents");
        std::string redirectUri2 = "/wopi/files/2/contents";
        static const std::regex regContentsRedirected("/wopi/files/2/contents");

        TST_LOG("FakeWOPIHost: Request URI [" << uriReq.toString() << "]:\n");

        // CheckFileInfo - returns redirect response
        if (request.getMethod() == "GET" && std::regex_match(uriReq.getPath(), regInfo))
        {
            TST_LOG("FakeWOPIHost: Handling CheckFileInfo (1/2)");

            assertCheckFileInfoRequest(request);

            LOK_ASSERT_STATE(_phase, Phase::WaitLoad);
            TRANSITION_STATE(_phase, Phase::Redirected);

            http::Response httpResponse(http::StatusCode::Found);
            httpResponse.set("Location", helpers::getTestServerURI() + redirectUri + '?' + params);
            socket->sendAndShutdown(httpResponse);

            return true;
        }
        // CheckFileInfo - for redirected URI
        else if (request.getMethod() == "GET" && std::regex_match(uriReq.getPath(), regRedirected) && !std::regex_match(uriReq.getPath(), regContents))
        {
            TST_LOG("FakeWOPIHost: Handling CheckFileInfo: (2/2)");

            assertCheckFileInfoRequest(request);

            LOK_ASSERT_MESSAGE("Expected to be in Phase::Redirected or Phase::Done",
                               _phase == Phase::Redirected || _phase == Phase::Done);
            if (_phase == Phase::Redirected)
                TRANSITION_STATE(_phase, Phase::GetFile);

            Poco::JSON::Object::Ptr fileInfo = getDefaultCheckFileInfoPayload(uriReq);
            const std::string fileName(uriReq.getPath() == "/wopi/files/3" ? "he%llo.txt"
                                                                           : "hello.txt");
            fileInfo->set("BaseFileName", fileName);

            std::ostringstream jsonStream;
            fileInfo->stringify(jsonStream);

            http::Response httpResponse(http::StatusCode::OK);
            httpResponse.set("Last-Modified", Util::getHttpTime(getFileLastModifiedTime()));
            httpResponse.setBody(jsonStream.str(), "application/json; charset=utf-8");
            socket->sendAndShutdown(httpResponse);

            return true;
        }
        // GetFile - first try
        else if (request.getMethod() == "GET" && std::regex_match(uriReq.getPath(), regContents))
        {
            TST_LOG("FakeWOPIHost: Handling GetFile: " << uriReq.getPath());

            assertGetFileRequest(request);

            LOK_ASSERT_STATE(_phase, Phase::GetFile);
            TRANSITION_STATE(_phase, Phase::Redirected2);

            http::Response httpResponse(http::StatusCode::Found);
            httpResponse.set("Location", helpers::getTestServerURI() + redirectUri2 + '?' + params);
            socket->sendAndShutdown(httpResponse);

            return true;
        }
        // GetFile - redirected
        else if (request.getMethod() == "GET" && std::regex_match(uriReq.getPath(), regContentsRedirected))
        {
            TST_LOG("FakeWOPIHost: Handling GetFile: " << uriReq.getPath());

            assertGetFileRequest(request);

            LOK_ASSERT_STATE(_phase, Phase::Redirected2);
            TRANSITION_STATE(_phase, Phase::Done);

            http::Response httpResponse(http::StatusCode::OK);
            httpResponse.set("Last-Modified", Util::getHttpTime(getFileLastModifiedTime()));
            httpResponse.setBody(getFileContent(), "text/plain; charset=utf-8");
            socket->sendAndShutdown(httpResponse);

            exitTest(TestResult::Ok);

            return true;
        }

        return WopiTestServer::handleHttpRequest(request, message, socket);
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                TRANSITION_STATE(_phase, Phase::WaitLoad);

                initWebsocket("/wopi/files/1?" + params);

                WSD_CMD("load url=" + getWopiSrc());

                break;
            }
            case Phase::WaitLoad:
            case Phase::Redirected:
            case Phase::Redirected2:
            case Phase::GetFile:
            case Phase::Done:
            {
                break;
            }
        }
    }
};

class UnitWopiHttpRedirectLoop : public WopiTestServer
{
    STATE_ENUM(Phase, Load, Redirected) _phase;

    const std::string params = "access_token=anything";

public:
    UnitWopiHttpRedirectLoop()
        : WopiTestServer("UnitWopiHttpRedirectLoop")
        , _phase(Phase::Load)
    {
    }

    virtual bool handleHttpRequest(const Poco::Net::HTTPRequest& request,
                                   std::istream& /*message*/,
                                   const std::shared_ptr<StreamSocket>& socket) override
    {
        Poco::URI uriReq(request.getURI());
        static const std::regex regInfo("/wopi/files/[0-9]+");
        static unsigned redirectionCount = 0;

        TST_LOG("FakeWOPIHost: Request URI [" << uriReq.toString() << "]:\n");

        // CheckFileInfo - always returns redirect response
        if (request.getMethod() == "GET" && std::regex_match(uriReq.getPath(), regInfo))
        {
            TST_LOG("FakeWOPIHost: Handling CheckFileInfo");

            assertCheckFileInfoRequest(request);

            std::string expectedMessage = "It is expected to stop requesting after " +
                                           std::to_string(HTTP_REDIRECTION_LIMIT) + " redirections";
            LOK_ASSERT_MESSAGE(expectedMessage, redirectionCount <= HTTP_REDIRECTION_LIMIT);

            LOK_ASSERT_MESSAGE("Expected to be in Phase::Load or Phase::Redirected",
                               _phase == Phase::Load || _phase == Phase::Redirected);

            if (redirectionCount == HTTP_REDIRECTION_LIMIT)
            {
                exitTest(TestResult::Ok);
                return true;
            }

            TRANSITION_STATE(_phase, Phase::Redirected);

            http::Response httpResponse(http::StatusCode::Found);
            std::string location = getWopiHostURI() + "/wopi/files/" +
                                         std::to_string(redirectionCount) + '?' + params;
            httpResponse.set("Location", std::move(location));
            socket->sendAndShutdown(httpResponse);

            redirectionCount++;

            return true;
        }

        return false;
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                initWebsocket("/wopi/files/0?" + params);

                WSD_CMD("load url=" + getWopiSrc());

                break;
            }
            case Phase::Redirected:
            {
                break;
            }
        }
    }
};

UnitBase** unit_create_wsd_multi(void)
{
    return new UnitBase* [3]
    { new UnitWopiHttpRedirect(), new UnitWopiHttpRedirectLoop(), nullptr };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
