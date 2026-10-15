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
 * host. The WOPI host receives the download from coolwsd with the WOPI proof headers, so it can
 * refuse the same URL when anyone else asks for it. The engine is given the downloaded file in
 * the jail and never the URL. A redirect within the WOPI host is followed by coolwsd, and a
 * redirect to any other host reaches the engine as the URL it points to. A URL on another
 * allowed WOPI host, which is not the host of the document, reaches the engine unchanged.
 */

#include <config.h>

#include <Unit.hpp>
#include <WopiTestServer.hpp>
#include <common/FileUtil.hpp>
#include <common/JsonUtil.hpp>
#include <common/Uri.hpp>
#include <test/testlog.hpp>

#include <Poco/JSON/Object.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/URI.h>
#include <Poco/Util/LayeredConfiguration.h>

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>

namespace
{
/// A link on the WOPI host that redirects to OncePath on the same host.
constexpr std::string_view RedirectPath = "/compare/redirect";

/// The document to compare against on the WOPI host. It has no access_token and no extension,
/// as a link that works once has neither. The server names the file in Content-Disposition.
constexpr std::string_view OncePath = "/compare/once";

/// A link on the WOPI host that redirects to AwayUrl.
constexpr std::string_view AwayPath = "/compare/away";

/// A document on a host that is not a WOPI host. Nothing ever connects to it.
constexpr std::string_view AwayUrl = "http://compare.invalid/baseline.odt";

/// A document on OtherHost. Nothing ever connects to it.
constexpr std::string_view OtherPath = "/compare/other";

/// A WOPI host that is allowed, but is not the host of the document, which is 127.0.0.1.
constexpr std::string_view OtherHost = "localhost";

/// The content the WOPI host serves for the document to compare against.
constexpr std::string_view CompareContent = "the document to compare against";
} // namespace

class UnitWOPICompareDocuments : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitCompare)
    _phase;

    /// Whether the GetFile request of the document carried the proof headers. They are there
    /// when coolwsd has a proof key.
    bool _getFileHadProof = false;

    /// Whether a request carries all of the WOPI proof headers. False when it carries none of them.
    bool hasProofHeaders(const Poco::Net::HTTPRequest& request)
    {
        std::size_t count = 0;
        for (const char* header : { "X-WOPI-Proof", "X-WOPI-ProofOld", "X-WOPI-TimeStamp" })
        {
            if (request.has(header) && !request.get(header).empty())
                ++count;
        }

        LOK_ASSERT_MESSAGE("The request carries only some of the proof headers",
                           count == 0 || count == 3);
        return count == 3;
    }

    /// The URL on the WOPI host of the given path.
    std::string wopiHostUrl(std::string_view path) const
    {
        return getWopiHostURI() + std::string(path) + "?testname=" + getTestname();
    }

    /// The URL on OtherHost of the given path.
    std::string otherHostUrl(std::string_view path) const
    {
        Poco::URI uri(wopiHostUrl(path));
        uri.setHost(std::string(OtherHost));
        return uri.toString();
    }

public:
    UnitWOPICompareDocuments()
        : WopiTestServer("UnitWOPICompareDocuments")
        , _phase(Phase::Load)
    {
        setHasKitHooks();
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);

        // Two WOPI hosts are allowed: the one that serves the document and OtherHost.
        config.setString("storage.wopi.alias_groups[@mode]", "groups");
        config.setString("storage.wopi.alias_groups.group[0].host", "127.0.0.1");
        config.setBool("storage.wopi.alias_groups.group[0].host[@allow]", true);
        config.setString("storage.wopi.alias_groups.group[1].host", std::string(OtherHost));
        config.setBool("storage.wopi.alias_groups.group[1].host[@allow]", true);
    }

    bool handleHttpRequest(const Poco::Net::HTTPRequest& request, std::istream& message,
                           const std::shared_ptr<StreamSocket>& socket) override
    {
        const Poco::URI uriReq(request.getURI());
        const std::string path = uriReq.getPath();
        if (path == OtherPath)
        {
            TST_LOG("The document on the other WOPI host was downloaded: " << request.getURI());
            failTest("downloaded-from-another-wopi-host");
            return false;
        }

        if (path != RedirectPath && path != OncePath && path != AwayPath)
            return WopiTestServer::handleHttpRequest(request, message, socket);

        TST_LOG("The WOPI host is asked for " << request.getURI());
        LOK_ASSERT_STATE(_phase, Phase::WaitCompare);

        // The access_token is never sent to the host of the link.
        LOK_ASSERT_MESSAGE("The download carries an Authorization header",
                           !request.has("Authorization"));

        // The download is signed whenever the GetFile request of the document is.
        LOK_ASSERT_EQUAL_MESSAGE("Whether the download carries the proof headers", _getFileHadProof,
                                 hasProofHeaders(request));

        if (path == OncePath)
        {
            http::Response httpResponse(http::StatusCode::OK);
            httpResponse.set("Content-Disposition", "attachment; filename=\"baseline.odt\"");
            httpResponse.setBody(std::string(CompareContent), "application/octet-stream");
            socket->sendAndShutdown(httpResponse);
            return true;
        }

        http::Response httpResponse(http::StatusCode::Found);
        // The redirect within the WOPI host is relative, as a server may send it.
        httpResponse.set("Location", path == RedirectPath
                                         ? std::string(OncePath) + "?testname=" + getTestname()
                                         : std::string(AwayUrl));
        // The redirect within the WOPI host has no Content-Length, so its body ends when the
        // connection closes, as a server may send it.
        if (path == AwayPath)
            httpResponse.setContentLength(0);
        socket->sendAndShutdown(httpResponse);
        return true;
    }

    std::unique_ptr<http::Response>
    assertGetFileRequest(const Poco::Net::HTTPRequest& request) override
    {
        _getFileHadProof = hasProofHeaders(request);
        TST_LOG("GetFile " << (_getFileHadProof ? "carries" : "carries no") << " proof headers");
        return nullptr;
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);
        TRANSITION_STATE(_phase, Phase::WaitCompare);

        for (const std::string_view path : { RedirectPath, AwayPath })
        {
            WSD_CMD("insertfile name=" + Uri::encode(wopiHostUrl(path)) +
                    " type=comparedocumentsurl");
        }

        WSD_CMD("insertfile name=" + Uri::encode(otherHostUrl(OtherPath)) +
                " type=comparedocumentsurl");

        return true;
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

/// Inside the kit process, where the document to compare against reaches the engine.
class UnitKitWOPICompareDocuments : public UnitKit
{
    /// Whether the engine was given the file downloaded from the WOPI host.
    bool _sawDownloaded = false;

    /// Whether the engine was given the URL on the host that is not a WOPI host.
    bool _sawAway = false;

    /// Whether the engine was given the URL on OtherHost.
    bool _sawOtherHost = false;

public:
    UnitKitWOPICompareDocuments()
        : UnitKit("UnitKitWOPICompareDocuments")
    {
    }

    void initialize() override
    {
        // Empty, so the socket poll thread is not started, which is unused by this test.
    }

    bool filterInsertCommand(const std::string& command, const std::string& arguments) override
    {
        if (isFinished())
            return true;

        TST_LOG("Read the command: " << command << ' ' << arguments);
        if (command != ".uno:CompareDocuments")
        {
            failTest("wrong-command");
            return true;
        }

        Poco::JSON::Object::Ptr object;
        std::string url;
        if (JsonUtil::parseJSON(arguments, object) && object->getObject("URL"))
            url = object->getObject("URL")->optValue<std::string>("value", std::string());

        static constexpr std::string_view FileScheme = "file://";
        const Poco::URI uri(url);
        if (url == AwayUrl)
        {
            _sawAway = true;
        }
        else if (uri.getHost() == OtherHost && uri.getPath() == OtherPath)
        {
            _sawOtherHost = true;
        }
        else if (url.starts_with(FileScheme))
        {
            // The file keeps the extension of the name the server gave it.
            if (!url.ends_with(".odt"))
            {
                TST_LOG("The file given to the engine has lost its extension: " << url);
                failTest("no-extension");
                return true;
            }

            std::string content;
            FileUtil::readFile(url.substr(FileScheme.size()), content);
            if (content != CompareContent)
            {
                TST_LOG("The file given to the engine holds [" << content << ']');
                failTest("wrong-content");
                return true;
            }

            _sawDownloaded = true;
        }
        else
        {
            TST_LOG("The engine was given an unexpected URL [" << url << ']');
            failTest("unexpected-url");
            return true;
        }

        if (_sawDownloaded && _sawAway && _sawOtherHost)
            passTest("compared-against-the-downloaded-file-and-the-other-hosts");

        return true;
    }
};

UnitBase* unit_create_wsd(void) { return new UnitWOPICompareDocuments(); }

UnitBase* unit_create_kit(void) { return new UnitKitWOPICompareDocuments(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
