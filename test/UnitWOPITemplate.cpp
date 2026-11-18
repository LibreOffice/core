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
 * Unit test for WOPI template functionality.
 */

#include <config.h>

#include <Poco/Net/HTTPRequest.h>

#include <WopiTestServer.hpp>
#include <common/Log.hpp>
#include <Unit.hpp>
#include <UnitHTTP.hpp>
#include <helpers.hpp>
#include <net/HttpHelper.hpp>

class UnitWOPITemplate : public WopiTestServer
{
    STATE_ENUM(Phase, LoadTemplate, SaveDoc, CloseDoc, Polling) _phase;

    bool _savedTemplate;

    std::string _templateFile;

public:
    UnitWOPITemplate(std::string templateFile)
        : WopiTestServer("UnitWOPITemplate_" + templateFile)
        , _phase(Phase::LoadTemplate)
        , _savedTemplate(false)
        , _templateFile(templateFile)
    {
    }

    virtual bool handleHttpRequest(const Poco::Net::HTTPRequest& request,
                                   std::istream& /*message*/,
                                   const std::shared_ptr<StreamSocket>& socket) override
    {
        const Poco::URI uriReq(request.getURI());
        TST_LOG("FakeWOPIHost: " << request.getMethod() << " request: " << uriReq.toString());

        // CheckFileInfo
        if (request.getMethod() == "GET" && uriReq.getPath() == "/wopi/files/10")
        {
            TST_LOG("FakeWOPIHost: Handling CheckFileInfo: " << uriReq.getPath());

            Poco::JSON::Object::Ptr fileInfo = getDefaultCheckFileInfoPayload(uriReq);
            fileInfo->set("BaseFileName", "test.odt");
            fileInfo->set("TemplateSource", helpers::getTestServerURI() + "/" + _templateFile);

            std::ostringstream jsonStream;
            fileInfo->stringify(jsonStream);

            http::Response httpResponse(http::StatusCode::OK);
            httpResponse.set("Last-Modified", Util::getHttpTime(getFileLastModifiedTime()));
            httpResponse.setBody(jsonStream.str(), "application/json; charset=utf-8");
            socket->sendAndShutdown(httpResponse);

            return true;
        }
        else if ((request.getMethod() == "OPTIONS" || request.getMethod() == "HEAD" ||
                  request.getMethod() == "PROPFIND") &&
                 uriReq.getPath() == "/" + _templateFile)
        {
            TST_LOG("FakeWOPIHost: Handling " << request.getMethod() << " on " << uriReq.getPath());

            http::Response httpResponse(http::StatusCode::OK);
            httpResponse.set("Allow", "GET");
            socket->sendAndShutdown(httpResponse);

            return true;
        }
        // Get the template
        else if (request.getMethod() == "GET" && uriReq.getPath() == "/" + _templateFile)
        {
            TST_LOG("FakeWOPIHost: Handling template GetFile: " << uriReq.getPath());

            http::Response response(http::StatusCode::OK);
            HttpHelper::sendFileAndShutdown(socket, TDOC "/" + _templateFile, response);

            return true;
        }
        // Save template
        else if (request.getMethod() == "POST" && uriReq.getPath() == "/wopi/files/10/contents")
        {
            TST_LOG("FakeWOPIHost: Handling PutFile: " << uriReq.getPath());

            if (!_savedTemplate)
            {
                // First, we expect to get a PutFile right after loading.
                LOK_ASSERT_EQUAL(static_cast<int>(Phase::SaveDoc), static_cast<int>(_phase));
                _savedTemplate = true;
                TST_LOG("SaveDoc => CloseDoc");
                TRANSITION_STATE(_phase, Phase::CloseDoc);
            }
            else
            {
                // This is the save at shutting down.
                LOK_ASSERT_EQUAL(static_cast<int>(Phase::Polling), static_cast<int>(_phase));
                exitTest(TestResult::Ok);
            }

            const std::streamsize size = request.getContentLength();
            LOK_ASSERT(size > 0);

            const std::string body = "{\"LastModifiedTime\": \"" +
                                     Util::getIso8601FracformatTime(getFileLastModifiedTime()) + "\" }";
            http::Response httpResponse(http::StatusCode::OK);
            httpResponse.setBody(body, "application/json; charset=utf-8");
            socket->sendAndShutdown(httpResponse);

            return true;
        }

        TST_LOG("FakeWOPIHost: unknown request "
                << request.getMethod() << " request: " << uriReq.toString() << ". Defaulting.");
        return false;
    }


    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::LoadTemplate:
            {
                TST_LOG("LoadTemplate => SaveDoc");
                TRANSITION_STATE(_phase, Phase::SaveDoc);

                initWebsocket("/wopi/files/10?access_token=anything");
                WSD_CMD("load url=" + getWopiSrc());

                break;
            }
            case Phase::CloseDoc:
            {
                TST_LOG("CloseDoc => Polling");
                TRANSITION_STATE(_phase, Phase::Polling);
                WSD_CMD("closedocument");
                break;
            }
            case Phase::Polling:
            {
                exitTest(TestResult::Ok);
                break;
            }
            case Phase::SaveDoc:
            {
                break;
            }
        }
    }
};

UnitBase** unit_create_wsd_multi(void)
{
    return new UnitBase*[3]{ new UnitWOPITemplate("test.ott"), new UnitWOPITemplate("test.dotx"),
                             nullptr };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
