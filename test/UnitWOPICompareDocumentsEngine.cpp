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
 * Integration test for comparing the open document against another one at a URL on the WOPI
 * host, through to the result in the engine. The URL has no extension and the server names no
 * file, so the engine works out the type of the downloaded file from its content. A comparison
 * that finds differences opens the dialog to manage the changes.
 */

#include <config.h>

#include <Unit.hpp>
#include <WopiTestServer.hpp>
#include <common/Uri.hpp>
#include <helpers.hpp>
#include <test/testlog.hpp>

#include <Poco/Net/HTTPRequest.h>
#include <Poco/URI.h>

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace
{
/// The document to compare against on the WOPI host, without an extension.
constexpr std::string_view ComparePath = "/compare/once";
} // namespace

class UnitWOPICompareDocumentsEngine : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitDownload, WaitChangesDialog)
    _phase;

    /// The file in the test data that the WOPI host serves to compare against.
    const std::string _baseline;

public:
    UnitWOPICompareDocumentsEngine(const std::string& name, const std::string& document,
                                   std::string baseline)
        : WopiTestServer(name, document)
        , _phase(Phase::Load)
        , _baseline(std::move(baseline))
    {
    }

    bool handleHttpRequest(const Poco::Net::HTTPRequest& request, std::istream& message,
                           const std::shared_ptr<StreamSocket>& socket) override
    {
        const Poco::URI uriReq(request.getURI());
        if (uriReq.getPath() != ComparePath)
            return WopiTestServer::handleHttpRequest(request, message, socket);

        TST_LOG("The WOPI host serves [" << _baseline << "] to compare against");
        LOK_ASSERT_STATE(_phase, Phase::WaitDownload);
        TRANSITION_STATE(_phase, Phase::WaitChangesDialog);

        const std::vector<char> data = helpers::readDataFromFile(_baseline);
        LOK_ASSERT_MESSAGE("The document to compare against is missing", !data.empty());

        http::Response httpResponse(http::StatusCode::OK);
        httpResponse.setBody(std::string(data.begin(), data.end()), "application/octet-stream");
        socket->sendAndShutdown(httpResponse);
        return true;
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);
        TRANSITION_STATE(_phase, Phase::WaitDownload);

        const std::string url =
            getWopiHostURI() + std::string(ComparePath) + "?testname=" + getTestname();
        WSD_CMD("insertfile name=" + Uri::encode(url) + " type=comparedocumentsurl");
        return true;
    }

    bool onFilterSendWebSocketMessage(const std::string_view message, const WSOpCode /* code */,
                                      const bool /* flush */, int& /* unitReturn */) override
    {
        if (!isFinished() && message.starts_with("jsdialog:") &&
            message.find("\"id\": \"AcceptRejectChangesDialog\"") != std::string_view::npos)
        {
            LOK_ASSERT_STATE(_phase, Phase::WaitChangesDialog);
            passTest("compared-and-found-changes");
        }

        return false;
    }

    void invokeWSDTest() override
    {
        if (_phase != Phase::Load)
            return;

        TRANSITION_STATE(_phase, Phase::WaitLoadStatus);
        initWebsocket("/wopi/files/0?access_token=anything");
        WSD_CMD("load url=" + getWopiSrc());
    }
};

UnitBase** unit_create_wsd_multi(void)
{
    return new UnitBase*[3]{ new UnitWOPICompareDocumentsEngine(
                                 "UnitWOPICompareDocumentsEngineDocx", "new.docx", "old.docx"),
                             new UnitWOPICompareDocumentsEngine("UnitWOPICompareDocumentsEngineOdt",
                                                                "hello.odt", "empty.odt"),
                             nullptr };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
