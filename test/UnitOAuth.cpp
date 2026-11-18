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
 * Unit test for OAuth authentication functionality.
 */

#include <config.h>

#include <HttpRequest.hpp>
#include <WopiTestServer.hpp>
#include <common/Log.hpp>
#include <Unit.hpp>
#include <UnitHTTP.hpp>
#include <helpers.hpp>
#include <common/Util.hpp>

#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/OAuth20Credentials.h>
#include <Poco/Util/LayeredConfiguration.h>

using Poco::Net::OAuth20Credentials;

class UnitOAuth : public WopiTestServer
{
    STATE_ENUM(Phase,
               Start, // Initial phase
               LoadToken, // loading the document with Bearer token
               ModifyAccessToken, // Modify the access-token after loading
               LoadHeader, // load the document with Basic auth
               LoadingHeader, // loading the document with Basic auth
               Done // just wait for the results
    )
    _phase;

    inline static const std::string access_token_ttl_value = "123456";
    std::string _credential;
    bool _checkFileInfoCalled;
    bool _getFileCalled;

public:
    UnitOAuth()
        : WopiTestServer("UnitOAuth")
        , _phase(Phase::Start)
        , _checkFileInfoCalled(false)
        , _getFileCalled(false)
    {
    }

    /// The actual assert of the authentication.
    void assertRequest(const Poco::Net::HTTPRequest& request)
    {
        TST_LOG("URI: " << request.getURI());

        LOK_ASSERT(request.getURI().find("access_token_ttl") != std::string::npos);
        for (const auto& param : Poco::URI(request.getURI()).getQueryParameters())
        {
            if (param.first == "access_token_ttl")
            {
                LOK_ASSERT_EQUAL_STR(access_token_ttl_value, param.second);
                break;
            }
        }

        // check that the request contains the Authorization: header
        try
        {
            if (_phase == Phase::LoadToken || _phase == Phase::ModifyAccessToken)
            {
                OAuth20Credentials creds(request);
                LOK_ASSERT_EQUAL(_credential, creds.getBearerToken());
            }
            else if (_phase == Phase::LoadingHeader)
            {
                OAuth20Credentials creds(request, "Basic");
                LOK_ASSERT_EQUAL(_credential, creds.getBearerToken());
            }
            else
            {
                LOK_ASSERT_FAIL("Unexpected phase: " << name(_phase));
            }
        }
        catch (const std::exception& ex)
        {
            // fail as fast as possible
            failTest(std::string("Exception: ") + ex.what());
        }
    }

    virtual std::unique_ptr<http::Response>
    assertCheckFileInfoRequest(const Poco::Net::HTTPRequest& request) override
    {
        _checkFileInfoCalled = true;
        assertRequest(request);

        return nullptr; // Success.
    }

    std::unique_ptr<http::Response>
    assertGetFileRequest(const Poco::Net::HTTPRequest& request) override
    {
        _getFileCalled = true;
        assertRequest(request);

        return nullptr; // Success.
    }

    std::unique_ptr<http::Response>
    assertPutFileRequest(const Poco::Net::HTTPRequest& request) override
    {
        LOK_ASSERT_STATE(_phase, Phase::ModifyAccessToken);
        assertRequest(request);

        TRANSITION_STATE(_phase, Phase::LoadHeader);
        return nullptr;
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("Doc (" << name(_phase) << "): [" << message << ']');

        LOK_ASSERT_EQUAL_MESSAGE("CheckFileInfo was not invoked", true, _checkFileInfoCalled);
        _checkFileInfoCalled = false;
        LOK_ASSERT_EQUAL_MESSAGE("GetFile was not invoked", true, _getFileCalled);
        _getFileCalled = false;

        switch (_phase)
        {
            case Phase::LoadToken:
                TRANSITION_STATE(_phase, Phase::ModifyAccessToken);
                _credential = "abcdefg123456_newtoken";
                WSD_CMD("resetaccesstoken " + _credential);
                WSD_CMD("save dontTerminateEdit=0 dontSaveIfUnmodified=0");

                // Close the document after loading.
                WSD_CMD("closedocument");
                break;
            case Phase::ModifyAccessToken:
                // Close the document after loading.
                WSD_CMD("closedocument");
                break;
            case Phase::LoadHeader:
                // Close the document after loading.
                WSD_CMD("closedocument");
                TRANSITION_STATE(_phase, Phase::LoadingHeader);
                _credential = "basic==";
                initWebsocket("/wopi/files/1?access_header=Authorization: Basic " + _credential +
                              "&access_token_ttl=" + access_token_ttl_value);
                WSD_CMD("load url=" + getWopiSrc());
                break;
            case Phase::LoadingHeader:
                // Close the document after loading.
                WSD_CMD("closedocument");
                passTest("Finished all cases successfully");
                break;
            default:
                LOK_ASSERT_FAIL("Unexpected phase: " << name(_phase));
        }

        return true;
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Start:
            {
                TRANSITION_STATE(_phase, Phase::LoadToken);
                _credential = "s3hn3ct0k3v";
                initWebsocket("/wopi/files/0?access_token=" + _credential +
                              "&access_token_ttl=" + access_token_ttl_value);

                WSD_CMD("load url=" + getWopiSrc());
            }
            break;
            case Phase::LoadToken:
            case Phase::ModifyAccessToken:
                break;
            case Phase::LoadHeader:
            {
                TRANSITION_STATE(_phase, Phase::LoadingHeader);
                _credential = "basic==";
                initWebsocket("/wopi/files/1?access_header=Authorization: Basic " + _credential +
                              "&access_token_ttl=" + access_token_ttl_value);
                WSD_CMD("load url=" + getWopiSrc());
            }
            break;
            case Phase::LoadingHeader:
            case Phase::Done:
            {
                // just wait for the results
                break;
            }
        }
    }
};

UnitBase* unit_create_wsd(void) { return new UnitOAuth(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
