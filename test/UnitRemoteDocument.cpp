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
 * Unit tests for subscriptions to remote documents: a document follows the
 * live state of another document through a headless client session.
 */

#include <config.h>

#include <common/JsonUtil.hpp>
#include <common/Protocol.hpp>
#include <common/Uri.hpp>
#include <helpers.hpp>
#include <lokassert.hpp>

#include <WopiTestServer.hpp>

#include <net/HttpRequest.hpp>
#include <wsd/ClientSession.hpp>
#include <wsd/DocumentBroker.hpp>
#include <wsd/RemoteDocumentBroker.hpp>

#include <Poco/Net/HTTPRequest.h>

#include <algorithm>
#include <future>
#include <map>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace
{
// Adds one remote link to CheckFileInfo, split the way the production
// code now expects a WOPI host to send it: the public part (WOPISrc and the
// last-modified time) in the top-level RemoteLinks, and this view's
// private access token in UserPrivateInfo.RemoteLinks. Calling this
// again names a second document, so a test can list several.
void setRemoteLink(Poco::JSON::Object::Ptr& fileInfo, const std::string& wopiSrc,
                   const std::string& accessToken,
                   const std::string& lastModifiedTime = std::string(),
                   const std::string& persistentLink = std::string())
{
    Poco::JSON::Array::Ptr remoteLinks = fileInfo->getArray("RemoteLinks");
    if (!remoteLinks)
        remoteLinks = new Poco::JSON::Array();
    Poco::JSON::Object::Ptr entry = new Poco::JSON::Object();
    entry->set("WOPISrc", wopiSrc);
    if (!lastModifiedTime.empty())
        entry->set("LastModifiedTime", lastModifiedTime);
    if (!persistentLink.empty())
        entry->set("PersistentLink", persistentLink);
    remoteLinks->add(entry);
    fileInfo->set("RemoteLinks", remoteLinks);

    Poco::JSON::Object::Ptr userPrivateInfo = fileInfo->getObject("UserPrivateInfo");
    if (!userPrivateInfo)
        userPrivateInfo = new Poco::JSON::Object();
    Poco::JSON::Array::Ptr tokens = userPrivateInfo->getArray("RemoteLinks");
    if (!tokens)
        tokens = new Poco::JSON::Array();
    Poco::JSON::Object::Ptr tokenEntry = new Poco::JSON::Object();
    tokenEntry->set("WOPISrc", wopiSrc);
    tokenEntry->set("AccessToken", accessToken);
    tokens->add(tokenEntry);
    userPrivateInfo->set("RemoteLinks", tokens);
    fileInfo->set("UserPrivateInfo", userPrivateInfo);
}
} // namespace

/// A document (file 1) subscribes to a remote document (file 2) named in its
/// CheckFileInfo RemoteLinks. Verifies that the subscriber receives
/// connected and modified events while another user edits the remote
/// document, and that unsubscribing closes the headless session.
class UnitRemoteDocument : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitConnected, LoadEditor, WaitEditorView, WaitModified,
               WaitHeadlessGone, Done)
    _phase;

    /// A second user editing the remote document (file 2).
    std::unique_ptr<UnitWebSocket> _editorWs;

    /// The subscription states seen in the remotelinks: messages.
    bool _sawAvailableState = false;
    bool _sawConnectedState = false;

    std::string remoteWopiSrc() const
    {
        return getWopiHostURI() + "/wopi/files/2";
    }

    std::string encodedRemoteWopiSrc() const { return Uri::encode(remoteWopiSrc()); }

public:
    UnitRemoteDocument()
        : WopiTestServer("UnitRemoteDocument")
        , _phase(Phase::Load)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);
        config.setBool("remote_links.enable", true);
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& request,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        // Only the subscribing document lists a remote link.
        if (Poco::URI(request.getURI()).getPath().ends_with("/1"))
        {
            setRemoteLink(fileInfo, remoteWopiSrc(), "remotetoken",
                          "2026-09-01T12:00:00.000000Z", "storage:quarter-3");
        }
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');

        if (_phase == Phase::WaitLoadStatus)
        {
            // The subscriber document is up; ask for the remote document.
            TRANSITION_STATE(_phase, Phase::WaitConnected);
            WSD_CMD("remotedocsubscribe wopisrc=" + encodedRemoteWopiSrc());
        }

        // The remote document loading through the headless session also
        // lands here; nothing to do for it.
        return true;
    }

    bool onFilterSendWebSocketMessage(const std::string_view message, const WSOpCode /*code*/,
                                      const bool /*flush*/, int& /*unitReturn*/) override
    {
        if (message.starts_with("remotelinks:"))
        {
            TST_LOG("Got: [" << message << ']');
            LOK_ASSERT_MESSAGE("The remote links JSON must not carry access tokens",
                               message.find("remotetoken") == std::string_view::npos);
            LOK_ASSERT_MESSAGE("The remote links JSON must carry the last modified time",
                               message.find("\"lastModifiedTime\":\"2026-09-01T12:00:00.000000Z\"") !=
                                   std::string_view::npos);
            LOK_ASSERT_MESSAGE("The remote links JSON must carry the persistent link",
                               message.find("\"persistentLink\":\"storage:quarter-3\"") !=
                                   std::string_view::npos);
            if (message.find("\"state\":\"available\"") != std::string_view::npos)
                _sawAvailableState = true;
            if (message.find("\"state\":\"connected\"") != std::string_view::npos)
                _sawConnectedState = true;
            return false;
        }

        if (!message.starts_with("remotedocevent:"))
            return false;

        TST_LOG("Got: [" << message << ']');

        if (message.find("event=connected") != std::string::npos)
        {
            LOK_ASSERT_STATE(_phase, Phase::WaitConnected);
            TRANSITION_STATE(_phase, Phase::LoadEditor);
        }
        else if (message.find("event=modified value=true") != std::string::npos)
        {
            LOK_ASSERT_STATE(_phase, Phase::WaitModified);
            TRANSITION_STATE(_phase, Phase::WaitHeadlessGone);
            WSD_CMD("remotedocunsubscribe wopisrc=" + encodedRemoteWopiSrc());
        }
        else if (message.find("event=error") != std::string::npos)
        {
            failTest("Unexpected remote document error: " + std::string(message));
        }

        return false;
    }

    bool onViewLoaded(const std::string& message) override
    {
        TST_LOG("onViewLoaded: [" << message << ']');

        if (_phase == Phase::WaitEditorView)
        {
            // The editor joined the remote document; modify it.
            TRANSITION_STATE(_phase, Phase::WaitModified);
            helpers::sendTextFrame(_editorWs->getWebSocket(), "key type=input char=97 key=0",
                                   getTestname());
            helpers::sendTextFrame(_editorWs->getWebSocket(), "key type=up char=0 key=512",
                                   getTestname());
        }

        return true;
    }

    void onDocBrokerRemoveSession(const std::string& docKey,
                                  const std::shared_ptr<ClientSession>& session) override
    {
        TST_LOG("onDocBrokerRemoveSession: [" << docKey << "], read-only: "
                                              << session->isReadOnly());

        // The headless session leaves the remote document after the
        // unsubscription, while the editor still holds it open.
        if (_phase == Phase::WaitHeadlessGone && docKey.ends_with("2") && session->isReadOnly())
        {
            LOK_ASSERT_MESSAGE("The clients saw the remote link as available",
                               _sawAvailableState);
            LOK_ASSERT_MESSAGE("The clients saw the remote link as connected",
                               _sawConnectedState);

            TRANSITION_STATE(_phase, Phase::Done);
            passTest("The remote document subscription connected, forwarded the modification "
                     "and disconnected cleanly");
        }
    }

    bool onDataLoss(const std::string& reason) override
    {
        // The editor's modification is intentionally never saved.
        TST_LOG("onDataLoss (expected): " << reason);
        return false;
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                TRANSITION_STATE(_phase, Phase::WaitLoadStatus);

                initWebsocket("/wopi/files/1?access_token=anything");
                WSD_CMD("load url=" + getWopiSrc());
                break;
            }
            case Phase::LoadEditor:
            {
                TRANSITION_STATE(_phase, Phase::WaitEditorView);

                const std::string editorWopiSrc =
                    Uri::encode(remoteWopiSrc() + "?access_token=anything");
                TST_LOG("Connecting an editor to the remote document: " << editorWopiSrc);
                _editorWs = std::make_unique<UnitWebSocket>(
                    socketPoll(), "/cool/" + editorWopiSrc + "/ws", getTestname());
                helpers::sendTextFrame(_editorWs->getWebSocket(), "load url=" + editorWopiSrc,
                                       getTestname());
                break;
            }
            case Phase::WaitLoadStatus:
            case Phase::WaitConnected:
            case Phase::WaitEditorView:
            case Phase::WaitModified:
            case Phase::WaitHeadlessGone:
            case Phase::Done:
            {
                break;
            }
        }
    }
};

/// Verifies the failure answers: subscribing a document to itself is refused
/// as a connection cycle, and subscribing to a document without a registered
/// access token is refused.
class UnitRemoteDocumentCycle : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitCycleError, WaitTokenError, Done) _phase;

    std::string ownWopiSrc() const { return getWopiHostURI() + "/wopi/files/1"; }

public:
    UnitRemoteDocumentCycle()
        : WopiTestServer("UnitRemoteDocumentCycle")
        , _phase(Phase::Load)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);
        config.setBool("remote_links.enable", true);
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& request,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        if (Poco::URI(request.getURI()).getPath().ends_with("/1"))
        {
            // The document lists itself as a remote link.
            setRemoteLink(fileInfo, ownWopiSrc(), "remotetoken");
        }
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);

        TRANSITION_STATE(_phase, Phase::WaitCycleError);
        WSD_CMD("remotedocsubscribe wopisrc=" + Uri::encode(ownWopiSrc()));
        return true;
    }

    bool onFilterSendWebSocketMessage(const std::string_view message, const WSOpCode /*code*/,
                                      const bool /*flush*/, int& /*unitReturn*/) override
    {
        if (!message.starts_with("remotedocevent:"))
            return false;

        TST_LOG("Got: [" << message << ']');

        if (_phase == Phase::WaitCycleError)
        {
            LOK_ASSERT_MESSAGE("Expected a cycle rejection",
                               message.find("event=error kind=cycledetected") !=
                                   std::string::npos);

            // A document the server holds no access token for.
            TRANSITION_STATE(_phase, Phase::WaitTokenError);
            WSD_CMD("remotedocsubscribe wopisrc=" +
                    Uri::encode(getWopiHostURI() + "/wopi/files/3"));
        }
        else if (_phase == Phase::WaitTokenError)
        {
            LOK_ASSERT_MESSAGE("Expected a missing-token rejection",
                               message.find("event=error kind=notoken") != std::string::npos);

            TRANSITION_STATE(_phase, Phase::Done);
            passTest("Cycle and missing-token subscriptions were both refused");
        }

        return false;
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                TRANSITION_STATE(_phase, Phase::WaitLoadStatus);

                initWebsocket("/wopi/files/1?access_token=anything");
                WSD_CMD("load url=" + getWopiSrc());
                break;
            }
            case Phase::WaitLoadStatus:
            case Phase::WaitCycleError:
            case Phase::WaitTokenError:
            case Phase::Done:
            {
                break;
            }
        }
    }
};

/// Registers a remote link over POST /cool/links: the request
/// is authorized by the view's own one-time token, a wrong token is refused,
/// a body over the size a registration takes is refused before it is read,
/// the token is accepted only once, and the registered access token then
/// serves that view's subscription.
class UnitLinkPost : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitToken, Posting, WaitConnected, Done) _phase;

    /// The one-time token the view was handed for the POST.
    std::string _oneTimeToken;
    bool _documentLoaded = false;

    std::thread _postThread;

    std::string documentWopiSrc() const
    {
        return getWopiHostURI() + "/wopi/files/1";
    }

    std::string remoteWopiSrc() const
    {
        return getWopiHostURI() + "/wopi/files/2";
    }

    /// POSTs the registration authorized by the given one-time token, naming the given access
    /// token, and returns the response status.
    unsigned postLink(const std::string& oneTimeToken,
                      const std::string& accessToken = "remotetoken")
    {
        http::Request request("/cool/links?WOPISrc=" + Uri::encode(documentWopiSrc()),
                              http::Request::VERB_POST);
        request.setBody("{\"Nonce\":\"" + oneTimeToken +
                            "\",\"Link\":{\"WOPISrc\":\"" + remoteWopiSrc() +
                            "\",\"AccessToken\":\"" + accessToken + "\""
                            ",\"LastModifiedTime\":\"2026-09-01T12:00:00.000000Z\"}}",
                        "application/json");

        auto session = http::Session::create(helpers::getTestServerURI());
        session->setTimeout(std::chrono::seconds(10));
        const std::shared_ptr<const http::Response> response = session->syncRequest(request);
        return response ? static_cast<unsigned>(response->statusLine().statusCode()) : 0;
    }

    /// Runs the POST sequence once the view has a token and the document is up.
    void maybeStartPost()
    {
        if (_phase != Phase::WaitToken || _oneTimeToken.empty() || !_documentLoaded ||
            _postThread.joinable())
            return;

        TRANSITION_STATE(_phase, Phase::Posting);
        _postThread = std::thread(
            [this, oneTimeToken = _oneTimeToken]
            {
                // A token no view holds is refused.
                LOK_ASSERT_EQUAL(static_cast<unsigned>(http::StatusCode::Unauthorized),
                                 postLink("wrongtoken"));

                // A body too large to name one document and one token is refused on its
                // length alone, before the token in it is read.
                LOK_ASSERT_EQUAL(static_cast<unsigned>(http::StatusCode::PayloadTooLarge),
                                 postLink(oneTimeToken, std::string(64 * 1024, 'x')));

                // The view's own one-time token authorizes the registration, so the refused
                // request above did not spend it.
                LOK_ASSERT_EQUAL(static_cast<unsigned>(http::StatusCode::OK),
                                 postLink(oneTimeToken));

                // The same token is not accepted a second time.
                LOK_ASSERT_EQUAL(static_cast<unsigned>(http::StatusCode::Unauthorized),
                                 postLink(oneTimeToken));

                // The registered token serves this view's subscription.
                TRANSITION_STATE(_phase, Phase::WaitConnected);
                WSD_CMD("remotedocsubscribe wopisrc=" + Uri::encode(remoteWopiSrc()));
            });
    }

public:
    UnitLinkPost()
        : WopiTestServer("UnitLinkPost")
        , _phase(Phase::Load)
    {
    }

    ~UnitLinkPost()
    {
        if (_postThread.joinable())
            _postThread.join();
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);
        config.setBool("remote_links.enable", true);
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');

        _documentLoaded = true;
        maybeStartPost();
        return true;
    }

    bool onFilterSendWebSocketMessage(const std::string_view message, const WSOpCode /*code*/,
                                      const bool /*flush*/, int& /*unitReturn*/) override
    {
        if (message.starts_with("linktoken:"))
        {
            TST_LOG("Got: [" << message << ']');
            // Keep the first token; a fresh one arrives after the accepted POST.
            if (_oneTimeToken.empty())
            {
                _oneTimeToken = std::string(message.substr(message.find(' ') + 1));
                maybeStartPost();
            }

            return false;
        }

        if (message.starts_with("remotelinks:"))
        {
            TST_LOG("Got: [" << message << ']');
            LOK_ASSERT_MESSAGE(
                "The POST-registered last modified time must reach the remote links JSON",
                message.find("\"lastModifiedTime\":\"2026-09-01T12:00:00.000000Z\"") !=
                    std::string_view::npos);
            return false;
        }

        if (!message.starts_with("remotedocevent:"))
            return false;

        TST_LOG("Got: [" << message << ']');

        if (message.find("event=connected") != std::string::npos)
        {
            if (_phase == Phase::WaitConnected)
            {
                TRANSITION_STATE(_phase, Phase::Done);
                passTest("The view's one-time token registered a remote link");
            }
        }
        else if (message.find("event=error") != std::string::npos)
        {
            failTest("Unexpected remote document error: " + std::string(message));
        }

        return false;
    }

    void invokeWSDTest() override
    {
        if (_phase == Phase::Load)
        {
            TRANSITION_STATE(_phase, Phase::WaitToken);

            initWebsocket("/wopi/files/1?access_token=firsttoken");
            WSD_CMD("load url=" + getWopiSrc());
        }
    }
};

/// Drops a remote link over the remotelinkremove command: a view drops it for every view
class UnitLinkDelete : public WopiTestServer
{
    /// The persistent link the registered remote link is bound to.
    static constexpr auto SourceLink = "storage:quarter-3";

    STATE_ENUM(Phase, Load, WaitToken, Registering, WaitListed, WaitSyntaxError, WaitNotFound,
               WaitDropped, Done)
    _phase;

    /// The latest one-time token the view was handed.
    std::string _oneTimeToken;
    bool _documentLoaded = false;
    /// Whether the list the views are sent has named the remote document.
    bool _listed = false;

    /// Guards the phase against the thread the POST runs on.
    std::mutex _mutex;
    std::thread _postThread;

    std::string documentWopiSrc() const { return getWopiHostURI() + "/wopi/files/1"; }

    std::string remoteWopiSrc() const { return getWopiHostURI() + "/wopi/files/2"; }

    unsigned postLink(const std::string& oneTimeToken)
    {
        http::Request request("/cool/links?WOPISrc=" + Uri::encode(documentWopiSrc()),
                              http::Request::VERB_POST);
        request.setBody("{\"Nonce\":\"" + oneTimeToken + "\",\"Link\":{\"WOPISrc\":\"" +
                            remoteWopiSrc() + "\",\"AccessToken\":\"remotetoken\"," +
                            "\"PersistentLink\":\"" + std::string(SourceLink) + "\"}}",
                        "application/json");

        auto session = http::Session::create(helpers::getTestServerURI());
        session->setTimeout(std::chrono::seconds(10));
        const std::shared_ptr<const http::Response> response = session->syncRequest(request);
        return response ? static_cast<unsigned>(response->statusLine().statusCode()) : 0;
    }

    /// Registers the document once the view has a token and the document is up.
    void maybeStartPost()
    {
        std::lock_guard<std::mutex> lock(_mutex);

        if (_phase != Phase::WaitToken || _oneTimeToken.empty() || !_documentLoaded ||
            _postThread.joinable())
            return;

        TRANSITION_STATE(_phase, Phase::Registering);
        _postThread = std::thread(
            [this, oneTimeToken = _oneTimeToken]
            {
                LOK_ASSERT_EQUAL(static_cast<unsigned>(http::StatusCode::OK),
                                 postLink(oneTimeToken));
                TRANSITION_STATE(_phase, Phase::WaitListed);
                // The list may already have arrived while the POST was in flight, so
                // nothing else would start the drop.
                maybeStartRemove();
            });
    }

    /// Drops the document again, once the accepted POST has put it on the list.
    void maybeStartRemove()
    {
        std::lock_guard<std::mutex> lock(_mutex);

        if (_phase != Phase::WaitListed || !_listed)
            return;

        // A command that names no persistent link is refused.
        TRANSITION_STATE(_phase, Phase::WaitSyntaxError);
        WSD_CMD("remotelinkremove");
    }

public:
    UnitLinkDelete()
        : WopiTestServer("UnitLinkDelete")
        , _phase(Phase::Load)
    {
    }

    ~UnitLinkDelete()
    {
        if (_postThread.joinable())
            _postThread.join();
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);
        config.setBool("remote_links.enable", true);
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');

        _documentLoaded = true;
        maybeStartPost();
        return true;
    }

    bool onFilterSendWebSocketMessage(const std::string_view message, const WSOpCode /*code*/,
                                      const bool /*flush*/, int& /*unitReturn*/) override
    {
        if (message.starts_with("linktoken:"))
        {
            TST_LOG("Got: [" << message << ']');
            _oneTimeToken = std::string(message.substr(message.find(' ') + 1));
            maybeStartPost();
            return false;
        }

        if (message.starts_with("error: cmd=remotelinkremove"))
        {
            TST_LOG("Got: [" << message << ']');
            std::lock_guard<std::mutex> lock(_mutex);

            if (_phase == Phase::WaitSyntaxError)
            {
                LOK_ASSERT_MESSAGE("Expected a syntax rejection",
                                   message.find("kind=syntax") != std::string_view::npos);

                // A persistent link no remote link is bound to leaves the list as it is.
                TRANSITION_STATE(_phase, Phase::WaitNotFound);
                WSD_CMD("remotelinkremove source=" + Uri::encode("storage:other"));
            }
            else if (_phase == Phase::WaitNotFound)
            {
                LOK_ASSERT_MESSAGE("Expected a not-found rejection",
                                   message.find("kind=notfound") != std::string_view::npos);

                TRANSITION_STATE(_phase, Phase::WaitDropped);
                WSD_CMD("remotelinkremove source=" + Uri::encode(SourceLink));
            }
            else
            {
                LOK_ASSERT_FAIL("Unexpected remotelinkremove error");
            }

            return false;
        }

        if (message.starts_with("remotelinks:"))
        {
            TST_LOG("Got: [" << message << ']');
            const bool names = message.find(remoteWopiSrc()) != std::string_view::npos;

            if (names)
            {
                _listed = true;
                maybeStartRemove();
                return false;
            }

            // The list that follows the accepted drop names the document no more.
            if (_phase == Phase::WaitDropped && _listed)
            {
                TRANSITION_STATE(_phase, Phase::Done);
                passTest("The dropped remote link left the list");
            }

            return false;
        }

        return false;
    }

    void invokeWSDTest() override
    {
        if (_phase == Phase::Load)
        {
            TRANSITION_STATE(_phase, Phase::WaitToken);

            initWebsocket("/wopi/files/1?access_token=firsttoken");
            WSD_CMD("load url=" + getWopiSrc());
        }
    }
};

/// Two documents subscribe to each other at the same moment. Exactly one
/// link survives and the other is refused as a cycle, so the pair cannot
/// keep each other loaded forever.
class UnitRemoteDocumentMutual : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitFirstLoad, LoadSecond, WaitSecondLoad, WaitOutcome, Done) _phase;

    /// The user of the second document.
    std::unique_ptr<UnitWebSocket> _secondWs;

    int _connectedCount = 0;
    int _cycleCount = 0;

    std::string fileWopiSrc(int fileId) const
    {
        return getWopiHostURI() + "/wopi/files/" + std::to_string(fileId);
    }

public:
    UnitRemoteDocumentMutual()
        : WopiTestServer("UnitRemoteDocumentMutual")
        , _phase(Phase::Load)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);
        config.setBool("remote_links.enable", true);
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& request,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        // Each document lists the other as a remote link.
        const std::string path = Poco::URI(request.getURI()).getPath();
        const int other = path.ends_with("/1") ? 2 : path.ends_with("/2") ? 1 : 0;
        if (other)
        {
            setRemoteLink(fileInfo, fileWopiSrc(other), "remotetoken");
        }
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');

        if (_phase == Phase::WaitFirstLoad)
        {
            TRANSITION_STATE(_phase, Phase::LoadSecond);
        }
        else if (_phase == Phase::WaitSecondLoad)
        {
            // Both documents subscribe to each other back to back, before
            // either connection chain can have traveled.
            TRANSITION_STATE(_phase, Phase::WaitOutcome);
            WSD_CMD("remotedocsubscribe wopisrc=" + Uri::encode(fileWopiSrc(2)));
            helpers::sendTextFrame(_secondWs->getWebSocket(),
                                   "remotedocsubscribe wopisrc=" + Uri::encode(fileWopiSrc(1)),
                                   getTestname());
        }

        return true;
    }

    bool onFilterSendWebSocketMessage(const std::string_view message, const WSOpCode /*code*/,
                                      const bool /*flush*/, int& /*unitReturn*/) override
    {
        if (!message.starts_with("remotedocevent:") || _phase != Phase::WaitOutcome)
            return false;

        TST_LOG("Got: [" << message << ']');

        if (message.find("event=connected") != std::string::npos)
        {
            ++_connectedCount;
            LOK_ASSERT_MESSAGE("Both mutual subscriptions connected, the documents keep each "
                               "other loaded",
                               _connectedCount < 2);
        }
        else if (message.find("event=error") != std::string::npos)
        {
            LOK_ASSERT_MESSAGE("Expected only cycle rejections",
                               message.find("kind=cycledetected") != std::string::npos);
            ++_cycleCount;
        }

        // The event echo reaches every view of a document, including the
        // headless one of the surviving link, so the rejection may be seen
        // more than once.
        if (_connectedCount == 1 && _cycleCount > 0)
        {
            TRANSITION_STATE(_phase, Phase::Done);
            passTest("One of two mutual subscriptions connected, the other was refused");
        }

        return false;
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                TRANSITION_STATE(_phase, Phase::WaitFirstLoad);

                initWebsocket("/wopi/files/1?access_token=anything");
                WSD_CMD("load url=" + getWopiSrc());
                break;
            }
            case Phase::LoadSecond:
            {
                TRANSITION_STATE(_phase, Phase::WaitSecondLoad);

                const std::string secondWopiSrc =
                    Uri::encode(fileWopiSrc(2) + "?access_token=anything");
                TST_LOG("Connecting the second document: " << secondWopiSrc);
                _secondWs = std::make_unique<UnitWebSocket>(
                    socketPoll(), "/cool/" + secondWopiSrc + "/ws", getTestname());
                helpers::sendTextFrame(_secondWs->getWebSocket(), "load url=" + secondWopiSrc,
                                       getTestname());
                break;
            }
            default:
            {
                break;
            }
        }
    }
};

/// A read-only client command sent to a subscribed remote document travels
/// over the headless session and its reply comes back to the originating
/// view, stamped with the remote's WOPISrc; a modifying command is refused.
class UnitRemoteDocumentCommand : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitConnected, WaitResult, Done) _phase;

    std::string remoteWopiSrc() const
    {
        return getWopiHostURI() + "/wopi/files/2";
    }

    std::string encodedRemoteWopiSrc() const { return Uri::encode(remoteWopiSrc()); }

public:
    UnitRemoteDocumentCommand()
        : WopiTestServer("UnitRemoteDocumentCommand")
        , _phase(Phase::Load)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);
        config.setBool("remote_links.enable", true);
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& request,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        if (Poco::URI(request.getURI()).getPath().ends_with("/1"))
        {
            setRemoteLink(fileInfo, remoteWopiSrc(), "remotetoken");
        }
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');

        if (_phase == Phase::WaitLoadStatus)
        {
            TRANSITION_STATE(_phase, Phase::WaitConnected);
            WSD_CMD("remotedocsubscribe wopisrc=" + encodedRemoteWopiSrc());
        }

        return true;
    }

    bool onFilterSendWebSocketMessage(const std::string_view message, const WSOpCode /*code*/,
                                      const bool /*flush*/, int& /*unitReturn*/) override
    {
        if (message.starts_with("remotedocevent:"))
        {
            if (message.find("event=connected") != std::string_view::npos &&
                _phase == Phase::WaitConnected)
            {
                TRANSITION_STATE(_phase, Phase::WaitResult);

                // A modifying command must be refused by the read-only filter
                // and never reach the remote.
                WSD_CMD("remotedoccommand wopisrc=" + encodedRemoteWopiSrc() +
                        " key type=input char=97 key=0");

                // A read-only command round-trips to the remote and back.
                WSD_CMD("remotedoccommand wopisrc=" + encodedRemoteWopiSrc() +
                        " getslidesections");
            }
            else if (message.find("event=modified value=true") != std::string_view::npos)
            {
                failTest("A read-only command modified the remote document");
            }
            else if (message.find("event=error") != std::string_view::npos)
            {
                failTest("Unexpected remote document error: " + std::string(message));
            }

            return false;
        }

        // The remote streams several frames once a command channel is open;
        // the test passes on the getslidesections reply and ignores the rest.
        if (message.starts_with("remotedoccommandresult:") && _phase == Phase::WaitResult &&
            message.find("slidesections:") != std::string_view::npos)
        {
            TST_LOG("Got: [" << message << ']');
            LOK_ASSERT_MESSAGE("The reply names the remote by its WOPISrc",
                               message.find(encodedRemoteWopiSrc()) != std::string_view::npos);

            TRANSITION_STATE(_phase, Phase::Done);
            passTest("A read-only command round-tripped to the remote and back");
        }

        return false;
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                TRANSITION_STATE(_phase, Phase::WaitLoadStatus);

                initWebsocket("/wopi/files/1?access_token=anything");
                WSD_CMD("load url=" + getWopiSrc());
                break;
            }
            default:
            {
                break;
            }
        }
    }
};

/// A document subscribes to a remote link whose file is gone from storage.
/// The storage answers the remote load with 404, so the subscriber is told the
/// source is missing rather than merely disconnected. A source that cannot be
/// loaded then holds no connection of the process: with room for one remote
/// document at a time, a second source still connects after the first failed.
class UnitRemoteDocumentMissing : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitMissing, WaitReadableSource, Done) _phase;

    std::string remoteWopiSrc() const { return getWopiHostURI() + "/wopi/files/2"; }

    std::string encodedRemoteWopiSrc() const { return Uri::encode(remoteWopiSrc()); }

    /// A second source, whose file the storage serves normally.
    std::string readableWopiSrc() const { return getWopiHostURI() + "/wopi/files/3"; }

    std::string encodedReadableWopiSrc() const { return Uri::encode(readableWopiSrc()); }

public:
    UnitRemoteDocumentMissing()
        : WopiTestServer("UnitRemoteDocumentMissing")
        , _phase(Phase::Load)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);
        config.setBool("remote_links.enable", true);
        // Room for one remote document at a time, so the second source connects
        // only if the first one stopped holding its slot.
        config.setInt("remote_links.max_remote_docs", 1);
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& request,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        // The subscribing document lists both the missing file and the readable
        // one as remote links.
        if (Poco::URI(request.getURI()).getPath().ends_with("/1"))
        {
            setRemoteLink(fileInfo, remoteWopiSrc(), "remotetoken");
            setRemoteLink(fileInfo, readableWopiSrc(), "remotetoken");
        }
    }

    std::unique_ptr<http::Response>
    assertCheckFileInfoRequest(const Poco::Net::HTTPRequest& request) override
    {
        // The remote link's file is gone, so the storage cannot find it.
        // The subscribing document itself still loads normally.
        if (Poco::URI(request.getURI()).getPath().ends_with("/2"))
            return std::make_unique<http::Response>(http::StatusCode::NotFound);

        return nullptr;
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');

        if (_phase == Phase::WaitLoadStatus)
        {
            // The subscriber document is up; ask for the missing remote link.
            TRANSITION_STATE(_phase, Phase::WaitMissing);
            WSD_CMD("remotedocsubscribe wopisrc=" + encodedRemoteWopiSrc());
        }

        return true;
    }

    bool onFilterSendWebSocketMessage(const std::string_view message, const WSOpCode /*code*/,
                                      const bool /*flush*/, int& /*unitReturn*/) override
    {
        if (!message.starts_with("remotelinks:"))
            return false;

        TST_LOG("Got: [" << message << ']');

        // The subscribe fails to read the source, so the entry ends up missing.
        if (_phase == Phase::WaitMissing &&
            message.find("\"state\":\"missing\"") != std::string_view::npos)
        {
            TST_LOG("The source that is gone was reported missing, asking for the readable one");
            TRANSITION_STATE(_phase, Phase::WaitReadableSource);
            WSD_CMD("remotedocsubscribe wopisrc=" + encodedReadableWopiSrc());
            return false;
        }

        // The source that is gone reports missing and never connects, so a
        // connected state here is the readable source's own.
        if (_phase == Phase::WaitReadableSource &&
            message.find("\"state\":\"connected\"") != std::string_view::npos)
        {
            TRANSITION_STATE(_phase, Phase::Done);
            passTest("A source that could not be loaded stops holding a connection of the "
                     "process, so the next source connects");
        }

        return false;
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                TRANSITION_STATE(_phase, Phase::WaitLoadStatus);

                initWebsocket("/wopi/files/1?access_token=anything");
                WSD_CMD("load url=" + getWopiSrc());
                break;
            }
            case Phase::WaitLoadStatus:
            case Phase::WaitMissing:
            case Phase::WaitReadableSource:
            case Phase::Done:
            {
                break;
            }
        }
    }
};

/// A view subscribes to sources whose files are gone from storage, and each report comes back
/// missing. A record in that state holds no connection: subscribing to the same source again
/// reads the storage afresh, and with as many missing sources as the per-view link limit, a
/// readable source still connects.
class UnitRemoteDocumentRetry : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitMissing, WaitRetryMissing, WaitAllMissing,
               WaitReadableSource, Done)
    _phase;

    /// CheckFileInfo requests answered with 404, counted per source file id.
    std::map<int, int> _goneRequests;

    std::string fileWopiSrc(int id) const
    {
        return getWopiHostURI() + "/wopi/files/" + std::to_string(id);
    }

    std::string encodedFileWopiSrc(int id) const { return Uri::encode(fileWopiSrc(id)); }

    /// How many sources the given list message reports in the given state.
    static std::size_t countStates(const std::string_view message, const std::string_view state)
    {
        const std::string needle = "\"state\":\"" + std::string(state) + '"';
        std::size_t count = 0;
        for (std::size_t pos = message.find(needle); pos != std::string_view::npos;
             pos = message.find(needle, pos + needle.size()))
            ++count;
        return count;
    }

public:
    UnitRemoteDocumentRetry()
        : WopiTestServer("UnitRemoteDocumentRetry")
        , _phase(Phase::Load)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);
        config.setBool("remote_links.enable", true);
        // Room for several remote documents at once, so the per-view link limit is the one
        // limit exercised here.
        config.setInt("remote_links.max_remote_docs", 16);
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& request,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        // The subscribing document lists four sources whose files are gone, as many as the
        // per-view link limit, and one readable source.
        if (Poco::URI(request.getURI()).getPath().ends_with("/1"))
        {
            for (int id = 2; id <= 6; ++id)
                setRemoteLink(fileInfo, fileWopiSrc(id), "remotetoken");
        }
    }

    std::unique_ptr<http::Response>
    assertCheckFileInfoRequest(const Poco::Net::HTTPRequest& request) override
    {
        // The files of the sources 2 to 5 are gone, so the storage cannot find them. The
        // subscribing document itself and the source 6 load normally.
        const std::string path = Poco::URI(request.getURI()).getPath();
        for (int id = 2; id <= 5; ++id)
        {
            if (path.ends_with("/" + std::to_string(id)))
            {
                ++_goneRequests[id];
                return std::make_unique<http::Response>(http::StatusCode::NotFound);
            }
        }

        return nullptr;
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');

        if (_phase == Phase::WaitLoadStatus)
        {
            // The subscriber document is up; ask for the first source that is gone.
            TRANSITION_STATE(_phase, Phase::WaitMissing);
            WSD_CMD("remotedocsubscribe wopisrc=" + encodedFileWopiSrc(2));
        }

        return true;
    }

    bool onFilterSendWebSocketMessage(const std::string_view message, const WSOpCode /*code*/,
                                      const bool /*flush*/, int& /*unitReturn*/) override
    {
        if (!message.starts_with("remotelinks:"))
            return false;

        TST_LOG("Got: [" << message << ']');

        // The first read of the source failed, so its entry reports missing. Subscribing to
        // the same source again opens a fresh attempt.
        if (_phase == Phase::WaitMissing && countStates(message, "missing") == 1)
        {
            TST_LOG("The source that is gone was reported missing, subscribing to it again");
            TRANSITION_STATE(_phase, Phase::WaitRetryMissing);
            WSD_CMD("remotedocsubscribe wopisrc=" + encodedFileWopiSrc(2));
            return false;
        }

        // The second subscribe read the storage again and came back missing again.
        if (_phase == Phase::WaitRetryMissing && countStates(message, "missing") == 1)
        {
            if (_goneRequests[2] < 2)
            {
                failTest("The second subscribe to a missing source must read the storage "
                         "again, but it was read " +
                         std::to_string(_goneRequests[2]) + " times");
                return false;
            }

            TST_LOG("The retried source was read again, filling the link limit with gone sources");
            TRANSITION_STATE(_phase, Phase::WaitAllMissing);
            for (int id = 3; id <= 5; ++id)
                WSD_CMD("remotedocsubscribe wopisrc=" + encodedFileWopiSrc(id));
            return false;
        }

        // All four sources that are gone report missing, so together they would fill the
        // per-view link limit if they counted. The readable source is asked for next.
        if (_phase == Phase::WaitAllMissing && countStates(message, "missing") == 4)
        {
            TST_LOG("All the gone sources report missing, asking for the readable one");
            TRANSITION_STATE(_phase, Phase::WaitReadableSource);
            WSD_CMD("remotedocsubscribe wopisrc=" + encodedFileWopiSrc(6));
            return false;
        }

        if (_phase == Phase::WaitReadableSource &&
            message.find("\"state\":\"connected\"") != std::string_view::npos)
        {
            TRANSITION_STATE(_phase, Phase::Done);
            passTest("A missing source can be subscribed to again, and missing sources do not "
                     "count toward the per-view link limit");
        }

        return false;
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                TRANSITION_STATE(_phase, Phase::WaitLoadStatus);

                initWebsocket("/wopi/files/1?access_token=anything");
                WSD_CMD("load url=" + getWopiSrc());
                break;
            }
            case Phase::WaitLoadStatus:
            case Phase::WaitMissing:
            case Phase::WaitRetryMissing:
            case Phase::WaitAllMissing:
            case Phase::WaitReadableSource:
            case Phase::Done:
            {
                break;
            }
        }
    }
};

/// Two views of one document hold different access to the same related source:
/// only the view whose UserPrivateInfo carries the token can subscribe. The
/// other view, which sees the source but holds no token, is refused, so one
/// view's access is never borrowed by another.
class UnitRemoteDocumentIsolation : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitFirstLoad, LoadSecond, WaitSecondView, WaitOutcome, Done) _phase;

    /// The second view of the same document, holding no token for the source.
    std::unique_ptr<UnitWebSocket> _secondWs;

    bool _sawConnected = false;
    bool _sawNoToken = false;

    std::string remoteWopiSrc() const
    {
        return getWopiHostURI() + "/wopi/files/2";
    }

    std::string encodedRemoteWopiSrc() const { return Uri::encode(remoteWopiSrc()); }

public:
    UnitRemoteDocumentIsolation()
        : WopiTestServer("UnitRemoteDocumentIsolation")
        , _phase(Phase::Load)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);
        config.setBool("remote_links.enable", true);
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& request,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        if (!Poco::URI(request.getURI()).getPath().ends_with("/1"))
            return;

        // Both views see the source, but only the first view is given a token
        // for it, in its own UserPrivateInfo.
        if (request.getURI().find("access_token=firsttoken") != std::string::npos)
        {
            setRemoteLink(fileInfo, remoteWopiSrc(), "remotetoken");
        }
        else
        {
            Poco::JSON::Array::Ptr remoteLinks = new Poco::JSON::Array();
            Poco::JSON::Object::Ptr entry = new Poco::JSON::Object();
            entry->set("WOPISrc", remoteWopiSrc());
            remoteLinks->add(entry);
            fileInfo->set("RemoteLinks", remoteLinks);
        }
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');

        if (_phase == Phase::WaitFirstLoad)
            TRANSITION_STATE(_phase, Phase::LoadSecond);

        return true;
    }

    bool onViewLoaded(const std::string& message) override
    {
        TST_LOG("onViewLoaded: [" << message << ']');

        if (_phase == Phase::WaitSecondView)
        {
            TRANSITION_STATE(_phase, Phase::WaitOutcome);

            // The first view holds the token and connects.
            WSD_CMD("remotedocsubscribe wopisrc=" + encodedRemoteWopiSrc());

            // The second view holds no token for the source and is refused.
            helpers::sendTextFrame(_secondWs->getWebSocket(),
                                   "remotedocsubscribe wopisrc=" + encodedRemoteWopiSrc(),
                                   getTestname());
        }

        return true;
    }

    bool onFilterSendWebSocketMessage(const std::string_view message, const WSOpCode /*code*/,
                                      const bool /*flush*/, int& /*unitReturn*/) override
    {
        if (!message.starts_with("remotedocevent:"))
            return false;

        TST_LOG("Got: [" << message << ']');

        if (message.find("event=connected") != std::string::npos)
            _sawConnected = true;
        else if (message.find("event=error") != std::string::npos &&
                 message.find("kind=notoken") != std::string::npos)
            _sawNoToken = true;

        if (_phase == Phase::WaitOutcome && _sawConnected && _sawNoToken)
        {
            TRANSITION_STATE(_phase, Phase::Done);
            passTest("Only the view holding the token subscribed; the other was refused");
        }

        return false;
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                TRANSITION_STATE(_phase, Phase::WaitFirstLoad);

                initWebsocket("/wopi/files/1?access_token=firsttoken");
                WSD_CMD("load url=" + getWopiSrc());
                break;
            }
            case Phase::LoadSecond:
            {
                TRANSITION_STATE(_phase, Phase::WaitSecondView);

                const std::string secondWopiSrc =
                    Uri::encode(getWopiHostURI() + "/wopi/files/1?access_token=secondtoken");
                TST_LOG("Connecting a second view: " << secondWopiSrc);
                _secondWs = std::make_unique<UnitWebSocket>(
                    socketPoll(), "/cool/" + secondWopiSrc + "/ws", getTestname());
                helpers::sendTextFrame(_secondWs->getWebSocket(), "load url=" + secondWopiSrc,
                                       getTestname());
                break;
            }
            default:
            {
                break;
            }
        }
    }
};

/// A document (file 1) subscribes to a remote document (file 2). When another
/// user saves the remote document to storage, the subscriber's remote
/// links list picks up the source's new last-modified time and the client
/// is told the source is newer.
class UnitRemoteDocumentSaved : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitConnected, LoadEditor, WaitEditorView, WaitSaved,
               Done)
    _phase;

    /// A second user editing the remote document (file 2).
    std::unique_ptr<UnitWebSocket> _editorWs;

    /// The last-modified time last seen in a remote links list.
    std::string _lastSeenModifiedTime;
    /// The time seen just before the editor saved the remote document.
    std::string _modifiedTimeBeforeSave;
    bool _sawSavedEvent = false;
    bool _sawUpdatedTime = false;

    std::string remoteWopiSrc() const { return getWopiHostURI() + "/wopi/files/2"; }

    std::string encodedRemoteWopiSrc() const { return Uri::encode(remoteWopiSrc()); }

    static std::string modifiedTimeOf(std::string_view message)
    {
        static constexpr std::string_view key = "\"lastModifiedTime\":\"";
        const std::size_t start = message.find(key);
        if (start == std::string_view::npos)
            return std::string();
        const std::size_t from = start + key.size();
        const std::size_t end = message.find('"', from);
        if (end == std::string_view::npos)
            return std::string();
        return std::string(message.substr(from, end - from));
    }

    void finishWhenPropagated()
    {
        if (_phase == Phase::WaitSaved && _sawSavedEvent && _sawUpdatedTime)
        {
            TRANSITION_STATE(_phase, Phase::Done);
            passTest("The remote save updated the remote link's last modified time and "
                     "notified the client");
        }
    }

public:
    UnitRemoteDocumentSaved()
        : WopiTestServer("UnitRemoteDocumentSaved")
        , _phase(Phase::Load)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);
        config.setBool("remote_links.enable", true);
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& request,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        if (Poco::URI(request.getURI()).getPath().ends_with("/1"))
        {
            setRemoteLink(fileInfo, remoteWopiSrc(), "remotetoken",
                               "2026-09-01T12:00:00.000000Z");
        }
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');

        if (_phase == Phase::WaitLoadStatus)
        {
            TRANSITION_STATE(_phase, Phase::WaitConnected);
            WSD_CMD("remotedocsubscribe wopisrc=" + encodedRemoteWopiSrc());
        }

        return true;
    }

    bool onFilterSendWebSocketMessage(const std::string_view message, const WSOpCode /*code*/,
                                      const bool /*flush*/, int& /*unitReturn*/) override
    {
        if (message.starts_with("remotelinks:"))
        {
            TST_LOG("Got: [" << message << ']');
            const std::string modifiedTime = modifiedTimeOf(message);
            if (!modifiedTime.empty())
                _lastSeenModifiedTime = modifiedTime;

            if (_phase == Phase::WaitSaved && !_modifiedTimeBeforeSave.empty() &&
                !modifiedTime.empty() && modifiedTime != _modifiedTimeBeforeSave)
            {
                _sawUpdatedTime = true;
                finishWhenPropagated();
            }
            return false;
        }

        if (!message.starts_with("remotedocevent:"))
            return false;

        TST_LOG("Got: [" << message << ']');

        if (message.find("event=connected") != std::string::npos)
        {
            if (_phase == Phase::WaitConnected)
                TRANSITION_STATE(_phase, Phase::LoadEditor);
        }
        else if (message.find("event=saved") != std::string::npos)
        {
            _sawSavedEvent = true;
            finishWhenPropagated();
        }
        else if (message.find("event=error") != std::string::npos)
        {
            failTest("Unexpected remote document error: " + std::string(message));
        }

        return false;
    }

    bool onViewLoaded(const std::string& message) override
    {
        TST_LOG("onViewLoaded: [" << message << ']');

        if (_phase == Phase::WaitEditorView)
        {
            TRANSITION_STATE(_phase, Phase::WaitSaved);
            _modifiedTimeBeforeSave = _lastSeenModifiedTime;

            // Change the remote document and upload it to storage. The new
            // last-modified time must reach the subscriber.
            helpers::sendTextFrame(_editorWs->getWebSocket(), "key type=input char=97 key=0",
                                   getTestname());
            helpers::sendTextFrame(_editorWs->getWebSocket(), "key type=up char=0 key=512",
                                   getTestname());
            helpers::sendTextFrame(_editorWs->getWebSocket(),
                                   "save dontTerminateEdit=0 dontSaveIfUnmodified=0",
                                   getTestname());
        }

        return true;
    }

    bool onDataLoss(const std::string& reason) override
    {
        TST_LOG("onDataLoss: " << reason);
        return false;
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                TRANSITION_STATE(_phase, Phase::WaitLoadStatus);

                initWebsocket("/wopi/files/1?access_token=anything");
                WSD_CMD("load url=" + getWopiSrc());
                break;
            }
            case Phase::LoadEditor:
            {
                TRANSITION_STATE(_phase, Phase::WaitEditorView);

                const std::string editorWopiSrc =
                    Uri::encode(remoteWopiSrc() + "?access_token=anything");
                TST_LOG("Connecting an editor to the remote document: " << editorWopiSrc);
                _editorWs = std::make_unique<UnitWebSocket>(
                    socketPoll(), "/cool/" + editorWopiSrc + "/ws", getTestname());
                helpers::sendTextFrame(_editorWs->getWebSocket(), "load url=" + editorWopiSrc,
                                       getTestname());
                break;
            }
            case Phase::WaitLoadStatus:
            case Phase::WaitConnected:
            case Phase::WaitEditorView:
            case Phase::WaitSaved:
            case Phase::Done:
            {
                break;
            }
        }
    }
};

/// A headless connection carries the secret of the server that made it, and
/// names the documents already on the chain it was made for. Verifies that a
/// request carrying a secret which is not this server's is refused outright,
/// that a connection carrying the secret and naming no chain is refused, and
/// that the same connection naming one loads the document.
class UnitRemoteDocumentNoChain : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, Connecting, Done) _phase;

    /// Carries the two connections below, so that waiting for their answers
    /// leaves coolwsd's main thread free to fork the kit one of them loads in.
    std::thread _connectThread;

    std::string fileWopiSrc(int fileId) const
    {
        return getWopiHostURI() + "/wopi/files/" + std::to_string(fileId);
    }

    /// The status a request carrying the given secret is answered with. The
    /// request itself is one every server answers, so the status is the answer
    /// to the secret alone.
    unsigned statusWithSecret(const std::string& secret)
    {
        http::Request request("/hosting/discovery");
        request.add(std::string(RemoteDocumentBroker::ChainSecretHeader), secret);

        auto session = http::Session::create(helpers::getTestServerURI());
        session->setTimeout(std::chrono::seconds(10));
        const std::shared_ptr<const http::Response> response = session->syncRequest(request);
        return response ? static_cast<unsigned>(response->statusLine().statusCode()) : 0;
    }

    /// Loads the given document over a connection carrying the secret of a
    /// headless one, with the given options after the url. Returns the answer
    /// to the load: a status: for a document that loaded, an error: for a load
    /// that was refused, and an empty string when neither arrived.
    std::string loadOverHeadlessConnection(const std::string& wopiSrc,
                                           const std::string& loadOptions)
    {
        const std::string encodedWopiSrc =
            Uri::encode(wopiSrc + "?access_token=anything&permission=readonly");

        const std::shared_ptr<http::WebSocketSession> session =
            http::WebSocketSession::create(helpers::getTestServerURI());
        if (!session)
            return std::string();

        // The secret this server holds is the one it gives every headless
        // connection of its own, so the connection is taken for one of them.
        http::Request request("/cool/" + encodedWopiSrc + "/ws");
        request.add(std::string(RemoteDocumentBroker::ChainSecretHeader),
                    RemoteDocumentBroker::getChainSecret());
        session->asyncRequest(request, socketPoll());

        helpers::sendTextFrame(session, "load url=" + encodedWopiSrc + loadOptions,
                               getTestname());

        const std::string answer =
            helpers::getResponseStringAny(session, { "status:", "error:" }, getTestname());
        session->asyncShutdown();
        return answer;
    }

public:
    UnitRemoteDocumentNoChain()
        : WopiTestServer("UnitRemoteDocumentNoChain")
        , _phase(Phase::Load)
    {
    }

    ~UnitRemoteDocumentNoChain()
    {
        if (_connectThread.joinable())
            _connectThread.join();
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);
        config.setBool("remote_links.enable", true);
    }

    bool onDataLoss(const std::string& reason) override
    {
        // The documents here are read for their load answer alone and none of
        // them is ever saved, so a document leaving with nothing uploaded is
        // what this test does rather than a loss it reports.
        TST_LOG("onDataLoss (expected): " << reason);
        return false;
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');

        // The source document loading over the second connection below also
        // lands here; only the first client drives the connections.
        if (_phase != Phase::WaitLoadStatus)
            return true;

        TRANSITION_STATE(_phase, Phase::Connecting);

        // An assertion failure throws, which would leave this thread through
        // no catch of its own, so each answer is reported instead.
        _connectThread = std::thread(
            [this]
            {
                static constexpr std::string_view Refusal = "error: cmd=load kind=syntax";

                // A secret that is not this server's
                const unsigned wrongSecret = statusWithSecret("notthesecretofthisserver");
                if (wrongSecret != static_cast<unsigned>(http::StatusCode::Forbidden))
                {
                    failTest("A request carrying a secret that is not this server's must be "
                             "refused with " +
                             std::to_string(static_cast<unsigned>(http::StatusCode::Forbidden)) +
                             ", got " + std::to_string(wrongSecret));
                    return;
                }

                // The same request carrying this server's own secret is answered.
                const unsigned rightSecret =
                    statusWithSecret(RemoteDocumentBroker::getChainSecret());
                if (rightSecret != static_cast<unsigned>(http::StatusCode::OK))
                {
                    failTest("A request carrying this server's secret must be answered with " +
                             std::to_string(static_cast<unsigned>(http::StatusCode::OK)) +
                             ", got " + std::to_string(rightSecret));
                    return;
                }

                // The chain is the whole record the source has of what reads
                // it, and the only thing a link of its own back to the other
                // end is refused by, so a connection naming none is refused.
                // Each connection reads a document of its own, so that the
                // refused one leaves no document unloading in the way of the
                // load below.
                const std::string refused =
                    loadOverHeadlessConnection(fileWopiSrc(2), std::string());
                if (refused != Refusal)
                {
                    failTest("A headless connection naming no chain must be answered with [" +
                             std::string(Refusal) + "], got [" + refused + ']');
                    return;
                }

                // The same connection naming a chain loads the document.
                const std::string loaded = loadOverHeadlessConnection(
                    fileWopiSrc(3), " remotechain=" + Uri::encode(fileWopiSrc(4)));
                if (!loaded.starts_with("status:"))
                {
                    failTest("A headless connection naming a chain must load the document, got [" +
                             loaded + ']');
                    return;
                }

                TRANSITION_STATE(_phase, Phase::Done);
                passTest("A wrong secret is refused, a headless connection naming no chain is "
                         "refused, and one naming a chain loads");
            });

        return true;
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                TRANSITION_STATE(_phase, Phase::WaitLoadStatus);

                initWebsocket("/wopi/files/1?access_token=anything");
                WSD_CMD("load url=" + getWopiSrc());
                break;
            }
            case Phase::WaitLoadStatus:
            case Phase::Connecting:
            case Phase::Done:
            {
                break;
            }
        }
    }
};

/// A POST /cool/links whose Link object carries a PersistentLink binds the registered document to a
/// source the open document's pages name: the name stops being reported as missing, and the
/// views are sent the registered link, under the name the integrator gave it, standing for
/// that source.
class UnitLinkPostPersistentLink : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitToken, WaitMissing, Posting, WaitBound, Done) _phase;

    /// The source document the pages name, as the user knows it.
    static constexpr auto SourceName = "Q3 #1 100%.odp";

    /// The one-time token the view was handed for the POST.
    std::string _oneTimeToken;
    bool _documentLoaded = false;

    std::mutex _brokerMutex;
    std::weak_ptr<DocumentBroker> _docBroker;

    std::thread _postThread;

    std::string documentWopiSrc() const
    {
        return getWopiHostURI() + "/wopi/files/1";
    }

    std::string remoteWopiSrc() const
    {
        return getWopiHostURI() + "/wopi/files/2";
    }

    /// POSTs the registration, naming the source the pages store, and returns the status.
    unsigned postLink(const std::string& oneTimeToken)
    {
        http::Request request("/cool/links?WOPISrc=" + Uri::encode(documentWopiSrc()),
                              http::Request::VERB_POST);
        request.setBody("{\"Nonce\":\"" + oneTimeToken + "\",\"Link\":{\"WOPISrc\":\"" +
                            remoteWopiSrc() +
                            "\",\"AccessToken\":\"remotetoken\","
                            "\"BaseFileName\":\"Quarter.odp\",\"PersistentLink\":\"" +
                            std::string(SourceName) + "\"}}",
                        "application/json");

        auto session = http::Session::create(helpers::getTestServerURI());
        session->setTimeout(std::chrono::seconds(10));
        const std::shared_ptr<const http::Response> response = session->syncRequest(request);
        return response ? static_cast<unsigned>(response->statusLine().statusCode()) : 0;
    }

    /// Names the source on the document's own thread, the way the kit's slide link list does,
    /// once the view has its token and the document is up.
    void maybeNameSource()
    {
        if (_phase != Phase::WaitToken || _oneTimeToken.empty() || !_documentLoaded)
            return;

        std::shared_ptr<DocumentBroker> docBroker;
        {
            std::lock_guard<std::mutex> lock(_brokerMutex);
            docBroker = _docBroker.lock();
        }
        LOK_ASSERT_MESSAGE("The document has no broker to name a source on", docBroker);

        TRANSITION_STATE(_phase, Phase::WaitMissing);
        docBroker->addCallback(
            [docBroker]
            { docBroker->setRemoteDocumentNamedSources({ std::string(SourceName) }); });
    }

public:
    UnitLinkPostPersistentLink()
        : WopiTestServer("UnitLinkPostPersistentLink")
        , _phase(Phase::Load)
    {
    }

    ~UnitLinkPostPersistentLink()
    {
        if (_postThread.joinable())
            _postThread.join();
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);
        config.setBool("remote_links.enable", true);
    }

    void onDocBrokerAddSession(const std::string&,
                               const std::shared_ptr<ClientSession>& session) override
    {
        std::lock_guard<std::mutex> lock(_brokerMutex);
        _docBroker = session->getDocumentBroker();
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');

        _documentLoaded = true;
        maybeNameSource();
        return true;
    }

    bool onFilterSendWebSocketMessage(const std::string_view message, const WSOpCode /*code*/,
                                      const bool /*flush*/, int& /*unitReturn*/) override
    {
        if (message.starts_with("linktoken:"))
        {
            TST_LOG("Got: [" << message << ']');
            // Keep the first token; a fresh one arrives after the accepted POST.
            if (_oneTimeToken.empty())
            {
                _oneTimeToken = std::string(message.substr(message.find(' ') + 1));
                maybeNameSource();
            }

            return false;
        }

        if (!message.starts_with("remotelinks:"))
            return false;

        TST_LOG("Got: [" << message << ']');

        const std::string linkField = "\"persistentLink\":\"" + std::string(SourceName) + '"';
        const bool missing = message.find("\"state\":\"missing\"") != std::string_view::npos;

        if (_phase == Phase::WaitMissing)
        {
            if (!missing)
                return false;

            // A source no remote link stands for is reported under its name, with no address.
            LOK_ASSERT_MESSAGE("A missing source must carry its name as the persistent link",
                               message.find(linkField) != std::string_view::npos);
            LOK_ASSERT_MESSAGE("A missing source has no address",
                               message.find("\"wopiSrc\":\"\"") != std::string_view::npos);

            TRANSITION_STATE(_phase, Phase::Posting);
            _postThread = std::thread(
                [this, oneTimeToken = _oneTimeToken]
                {
                    TRANSITION_STATE(_phase, Phase::WaitBound);
                    LOK_ASSERT_EQUAL(static_cast<unsigned>(http::StatusCode::OK),
                                     postLink(oneTimeToken));
                });
            return false;
        }

        if (_phase == Phase::WaitBound)
        {
            // The registered link is recorded before this view's token is, so the list may
            // arrive once more before the view holds access to it.
            if (message.find("\"state\":\"available\"") == std::string_view::npos)
                return false;

            LOK_ASSERT_MESSAGE("The registered link must stand for the named source",
                               message.find(linkField) != std::string_view::npos);
            LOK_ASSERT_MESSAGE("The registered link keeps the name the integrator gave it",
                               message.find("\"name\":\"Quarter.odp\"") != std::string_view::npos);
            LOK_ASSERT_MESSAGE("A source a remote link stands for is no longer missing",
                               !missing);

            TRANSITION_STATE(_phase, Phase::Done);
            passTest("A POST naming a PersistentLink bound the registered link to it");
        }

        return false;
    }

    void invokeWSDTest() override
    {
        if (_phase == Phase::Load)
        {
            TRANSITION_STATE(_phase, Phase::WaitToken);

            initWebsocket("/wopi/files/1?access_token=anything");
            WSD_CMD("load url=" + getWopiSrc());
        }
    }
};

/// What the fake link access endpoint was asked in one request.
struct LinkAccessRequest
{
    /// The path the request went to.
    std::string path;
    /// The access_token query parameter, empty when the URL carried none.
    std::string accessToken;
    /// The Authorization header, empty when the request carried none.
    std::string authorization;
    /// Whether the request carried the WOPI proof header.
    bool hasProofHeader = false;
    /// The PersistentLink of the body.
    std::string persistentLink;
};

/// A fake WOPI host that lists no remote links and answers link access requests instead,
/// recording what it was asked. The document (file 1) names one source, which the storage
/// may resolve to a second document (file 2).
class LinkAccessTestServer : public WopiTestServer
{
protected:
    /// The source document the pages name, as the user knows it.
    static constexpr auto SourceName = "Q3 #1 100%.odp";
    static constexpr auto LinkAccessSuffix = "/linkaccess";

    std::mutex _requestsMutex;
    std::vector<LinkAccessRequest> _requests;

    std::mutex _brokerMutex;
    std::weak_ptr<DocumentBroker> _docBroker;

    explicit LinkAccessTestServer(const std::string& name)
        : WopiTestServer(name)
    {
    }

    std::string remoteWopiSrc() const
    {
        return getWopiHostURI() + "/wopi/files/2";
    }

    /// The 200 answer naming file 2 for the source, readable with the given token.
    std::string linkBody(const std::string& accessToken) const
    {
        return "{\"WOPISrc\":\"" + remoteWopiSrc() + "\",\"AccessToken\":\"" + accessToken +
               "\",\"BaseFileName\":\"Quarter.odp\","
               "\"LastModifiedTime\":\"2026-09-01T12:00:00.000000Z\",\"PersistentLink\":\"" +
               std::string(SourceName) + "\"}";
    }

    /// The answer to one request: the status and the body.
    virtual std::pair<http::StatusCode, std::string>
    answerLinkAccess(const LinkAccessRequest& request) = 0;

    std::vector<LinkAccessRequest> requests()
    {
        std::lock_guard<std::mutex> lock(_requestsMutex);
        return _requests;
    }

    std::size_t requestCount()
    {
        std::lock_guard<std::mutex> lock(_requestsMutex);
        return _requests.size();
    }

    std::shared_ptr<DocumentBroker> docBroker()
    {
        std::lock_guard<std::mutex> lock(_brokerMutex);
        return _docBroker.lock();
    }

    /// Names the sources on the document's own thread, the way the kit's slide link list does.
    void nameSources(std::vector<std::string> names)
    {
        const std::shared_ptr<DocumentBroker> broker = docBroker();
        LOK_ASSERT_MESSAGE("The document has no broker to name a source on", broker);
        broker->addCallback([broker, names = std::move(names)]() mutable
                            { broker->setRemoteDocumentNamedSources(std::move(names)); });
    }

    /// The document's state dump, taken on its own thread.
    std::string dumpBrokerState()
    {
        const std::shared_ptr<DocumentBroker> broker = docBroker();
        LOK_ASSERT_MESSAGE("The document has no broker to dump", broker);

        auto promise = std::make_shared<std::promise<std::string>>();
        std::future<std::string> future = promise->get_future();
        broker->addCallback(
            [broker, promise]
            {
                std::ostringstream oss;
                broker->dumpState(oss);
                promise->set_value(oss.str());
            });
        LOK_ASSERT_MESSAGE("The document did not dump its state in time",
                           future.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
        return future.get();
    }

    /// Waits until the state dump reports the given text, or gives up after a while.
    bool waitForBrokerState(const std::string& text)
    {
        for (int i = 0; i < 100; ++i)
        {
            if (dumpBrokerState().find(text) != std::string::npos)
                return true;
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        return false;
    }

    /// Waits until the endpoint has seen at least the given number of requests.
    bool waitForRequests(std::size_t count)
    {
        for (int i = 0; i < 100; ++i)
        {
            if (requestCount() >= count)
                return true;
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        return false;
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);
        config.setBool("remote_links.enable", true);
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& request,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        // The storage knows no links between its files: it lists none, and answers for a
        // source the document stores instead.
        if (Poco::URI(request.getURI()).getPath().ends_with("/1"))
            fileInfo->set("SupportsLinkAccess", true);
    }

    void onDocBrokerAddSession(const std::string&,
                               const std::shared_ptr<ClientSession>& session) override
    {
        std::lock_guard<std::mutex> lock(_brokerMutex);
        _docBroker = session->getDocumentBroker();
    }

    bool handleHttpPostRequest(const Poco::Net::HTTPRequest& request, std::istream& message,
                               const std::shared_ptr<StreamSocket>& socket) override
    {
        const Poco::URI uriReq(request.getURI());
        if (!uriReq.getPath().ends_with(LinkAccessSuffix))
            return WopiTestServer::handleHttpPostRequest(request, message, socket);

        LinkAccessRequest record;
        record.path = uriReq.getPath();
        for (const auto& param : uriReq.getQueryParameters())
        {
            if (param.first == "access_token")
                record.accessToken = param.second;
        }
        record.authorization = request.get("Authorization", std::string());
        record.hasProofHeader = request.has("X-WOPI-Proof");

        const std::string body(std::istreambuf_iterator<char>(message), {});
        Poco::JSON::Object::Ptr object;
        if (JsonUtil::parseJSON(body, object))
            JsonUtil::findJSONValue(object, "PersistentLink", record.persistentLink);

        TST_LOG("Link access request for [" << record.persistentLink << "] with token ["
                                            << record.accessToken << ']');
        {
            std::lock_guard<std::mutex> lock(_requestsMutex);
            _requests.push_back(record);
        }

        const std::pair<http::StatusCode, std::string> answer = answerLinkAccess(record);
        http::Response response(answer.first);
        if (!answer.second.empty())
            response.setBody(answer.second, "application/json");
        socket->sendAndShutdown(response);
        return true;
    }

    /// Checks one request for what every link access request must be: sent next to the file,
    /// authorized like any WOPI request of the view, for the named source.
    void assertRequest(const LinkAccessRequest& request, const std::string& accessToken)
    {
        LOK_ASSERT_EQUAL(std::string("/wopi/files/1") + LinkAccessSuffix, request.path);
        LOK_ASSERT_EQUAL(accessToken, request.accessToken);
        LOK_ASSERT_EQUAL("Bearer " + accessToken, request.authorization);
        LOK_ASSERT_EQUAL(std::string(SourceName), request.persistentLink);
    }
};

/// A storage that lists no remote links resolves a source the document names: the server
/// asks the storage with the view's own token, records the answer as a remote link standing
/// for that source, and the view reads the source through it.
class UnitLinkAccessResolve : public LinkAccessTestServer
{
    STATE_ENUM(Phase, Load, WaitLoad, WaitResolved, WaitConnected, Done) _phase;

    std::pair<http::StatusCode, std::string>
    answerLinkAccess(const LinkAccessRequest&) override
    {
        return { http::StatusCode::OK, linkBody("remotetoken") };
    }

public:
    UnitLinkAccessResolve()
        : LinkAccessTestServer("UnitLinkAccessResolve")
        , _phase(Phase::Load)
    {
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');

        if (_phase == Phase::WaitLoad)
        {
            TRANSITION_STATE(_phase, Phase::WaitResolved);
            nameSources({ std::string(SourceName) });
        }

        return true;
    }

    bool onFilterSendWebSocketMessage(const std::string_view message, const WSOpCode /*code*/,
                                      const bool /*flush*/, int& /*unitReturn*/) override
    {
        if (message.starts_with("remotelinks:"))
        {
            TST_LOG("Got: [" << message << ']');

            if (_phase != Phase::WaitResolved ||
                message.find("\"state\":\"available\"") == std::string_view::npos)
                return false;

            const std::string linkField =
                "\"persistentLink\":\"" + std::string(SourceName) + '"';
            LOK_ASSERT_MESSAGE("The resolved link must stand for the named source",
                               message.find(linkField) != std::string_view::npos);
            LOK_ASSERT_MESSAGE("The resolved link keeps the name the storage gave it",
                               message.find("\"name\":\"Quarter.odp\"") != std::string_view::npos);
            LOK_ASSERT_MESSAGE("A source a remote link stands for is no longer missing",
                               message.find("\"state\":\"missing\"") == std::string_view::npos);

            const std::vector<LinkAccessRequest> asked = requests();
            LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), asked.size());
            assertRequest(asked[0], "anything");

            // The token the storage answered with serves this view's subscription.
            TRANSITION_STATE(_phase, Phase::WaitConnected);
            WSD_CMD("remotedocsubscribe wopisrc=" + Uri::encode(remoteWopiSrc()));
            return false;
        }

        if (!message.starts_with("remotedocevent:"))
            return false;

        TST_LOG("Got: [" << message << ']');

        if (message.find("event=connected") != std::string::npos)
        {
            if (_phase == Phase::WaitConnected)
            {
                TRANSITION_STATE(_phase, Phase::Done);
                passTest("The storage resolved the source the document names");
            }
        }
        else if (message.find("event=error") != std::string::npos)
        {
            failTest("Unexpected remote document error: " + std::string(message));
        }

        return false;
    }

    void invokeWSDTest() override
    {
        if (_phase == Phase::Load)
        {
            TRANSITION_STATE(_phase, Phase::WaitLoad);

            initWebsocket("/wopi/files/1?access_token=anything");
            WSD_CMD("load url=" + getWopiSrc());
        }
    }
};

/// The storage knows no document for the source the document names: the source stays
/// missing, the question is asked once when the view finds the source, and editing asks
/// nothing more. Only the user asks again.
class UnitLinkAccessNotFound : public LinkAccessTestServer
{
    STATE_ENUM(Phase, Load, WaitLoad, Checking, Done) _phase;

    /// A second source the document comes to name.
    static constexpr auto OtherSourceName = "Q4 outlook.odp";

    /// The two sources as a client spells them in a command.
    static constexpr auto EncodedSourceName = "Q3%20%231%20100%25.odp";
    static constexpr auto EncodedOtherSourceName = "Q4%20outlook.odp";

    std::thread _checkThread;

    std::pair<http::StatusCode, std::string>
    answerLinkAccess(const LinkAccessRequest&) override
    {
        return { http::StatusCode::NotFound, std::string() };
    }

    /// How many requests named the given source.
    std::size_t countRequestsFor(const std::string& source)
    {
        const std::vector<LinkAccessRequest> asked = requests();
        return std::count_if(asked.begin(), asked.end(),
                             [&source](const LinkAccessRequest& request)
                             { return request.persistentLink == source; });
    }

public:
    UnitLinkAccessNotFound()
        : LinkAccessTestServer("UnitLinkAccessNotFound")
        , _phase(Phase::Load)
    {
    }

    ~UnitLinkAccessNotFound()
    {
        if (_checkThread.joinable())
            _checkThread.join();
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');

        if (_phase != Phase::WaitLoad)
            return true;

        TRANSITION_STATE(_phase, Phase::Checking);
        _checkThread = std::thread(
            [this]
            {
                nameSources({ std::string(SourceName) });

                // The storage is asked once and says it knows no such document.
                LOK_ASSERT_MESSAGE("The endpoint must be asked for the source",
                                   waitForRequests(1));
                LOK_ASSERT_MESSAGE("The answer must be recorded as not found",
                                   waitForBrokerState("access: notfound"));
                const std::vector<LinkAccessRequest> asked = requests();
                LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), asked.size());
                assertRequest(asked[0], "anything");

                // The same list again asks nothing more.
                nameSources({ std::string(SourceName) });
                LOK_ASSERT_MESSAGE("The endpoint must not be asked again for the same list",
                                   waitForBrokerState("sources the document names: 1") &&
                                       requestCount() == 1);

                // Editing names a source of its own. Neither it nor the source the storage
                // knew nothing of is asked for: the view asked when it found its sources.
                nameSources({ std::string(SourceName), std::string(OtherSourceName) });
                LOK_ASSERT_MESSAGE("A changed list must ask nothing",
                                   waitForBrokerState("sources the document names: 2") &&
                                       requestCount() == 1);

                // The user has the source the storage knew nothing of looked for again, and
                // that one alone is asked for.
                WSD_CMD(std::string("remotelinkresolve source=") + EncodedSourceName);
                LOK_ASSERT_MESSAGE("The user's request must ask the endpoint once more",
                                   waitForRequests(2));
                LOK_ASSERT_EQUAL(static_cast<std::size_t>(2), countRequestsFor(SourceName));
                LOK_ASSERT_EQUAL(static_cast<std::size_t>(0), countRequestsFor(OtherSourceName));

                // A source the document does not name is asked for by nobody, which the
                // request for one it does name, sent behind it, shows.
                WSD_CMD(std::string("remotelinkresolve source=Nothing.odp"));
                WSD_CMD(std::string("remotelinkresolve source=") + EncodedOtherSourceName);
                LOK_ASSERT_MESSAGE("The source the document names must be asked for",
                                   waitForRequests(3));
                LOK_ASSERT_MESSAGE("The answers must be recorded as not found",
                                   waitForBrokerState(std::string("persistent link ") +
                                                      OtherSourceName + " access: notfound"));
                LOK_ASSERT_EQUAL(static_cast<std::size_t>(3), requestCount());
                LOK_ASSERT_EQUAL(static_cast<std::size_t>(0), countRequestsFor("Nothing.odp"));

                TRANSITION_STATE(_phase, Phase::Done);
                passTest("A source the storage knows nothing of is asked for when the user asks");
            });

        return true;
    }

    bool onFilterSendWebSocketMessage(const std::string_view message, const WSOpCode /*code*/,
                                      const bool /*flush*/, int& /*unitReturn*/) override
    {
        if (!message.starts_with("remotelinks:"))
            return false;

        TST_LOG("Got: [" << message << ']');

        // Nothing stands for the source, so it is reported missing whenever it is reported.
        LOK_ASSERT_MESSAGE("A source the storage knows nothing of stays missing",
                           message.find("\"state\":\"missing\"") != std::string_view::npos);
        LOK_ASSERT_MESSAGE("Nothing stands for the source, so the list holds no address",
                           message.find("\"state\":\"available\"") == std::string_view::npos);
        return false;
    }

    void invokeWSDTest() override
    {
        if (_phase == Phase::Load)
        {
            TRANSITION_STATE(_phase, Phase::WaitLoad);

            initWebsocket("/wopi/files/1?access_token=anything");
            WSD_CMD("load url=" + getWopiSrc());
        }
    }
};

/// Two views of one document each resolve the source the document names on their own: each
/// request carries that view's own token, and the answer's token serves that view alone.
class UnitLinkAccessPerView : public LinkAccessTestServer
{
    STATE_ENUM(Phase, Load, WaitFirstLoad, WaitFirstResolved, WaitSecondResolved,
               WaitConnected, Done)
    _phase;

    /// The second view of the same document.
    std::unique_ptr<UnitWebSocket> _secondWs;

    std::pair<http::StatusCode, std::string>
    answerLinkAccess(const LinkAccessRequest& request) override
    {
        // Each view is given a token of its own for the source.
        return { http::StatusCode::OK, linkBody("remote-" + request.accessToken) };
    }

public:
    UnitLinkAccessPerView()
        : LinkAccessTestServer("UnitLinkAccessPerView")
        , _phase(Phase::Load)
    {
    }

    std::unique_ptr<http::Response>
    assertCheckFileInfoRequest(const Poco::Net::HTTPRequest& request) override
    {
        // The second view reads the source with the token the storage issued to it.
        if (Poco::URI(request.getURI()).getPath().ends_with("/2"))
        {
            LOK_ASSERT_MESSAGE("The source must be read with the second view's own token",
                               request.getURI().find("access_token=remote-secondtoken") !=
                                   std::string::npos);
        }

        return nullptr;
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');

        if (_phase == Phase::WaitFirstLoad)
        {
            TRANSITION_STATE(_phase, Phase::WaitFirstResolved);
            nameSources({ std::string(SourceName) });
        }

        return true;
    }

    bool onFilterSendWebSocketMessage(const std::string_view message, const WSOpCode /*code*/,
                                      const bool /*flush*/, int& /*unitReturn*/) override
    {
        if (message.starts_with("remotelinks:"))
        {
            TST_LOG("Got: [" << message << ']');

            if (message.find("\"state\":\"available\"") == std::string_view::npos)
                return false;

            if (_phase == Phase::WaitFirstResolved)
            {
                // The first view reads the source. The second view joins a document whose
                // sources are already named, and holds no token for this one.
                const std::vector<LinkAccessRequest> asked = requests();
                LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), asked.size());
                assertRequest(asked[0], "firsttoken");

                TRANSITION_STATE(_phase, Phase::WaitSecondResolved);
                const std::string secondWopiSrc = Uri::encode(
                    getWopiHostURI() + "/wopi/files/1?access_token=secondtoken");
                TST_LOG("Connecting a second view: " << secondWopiSrc);
                _secondWs = std::make_unique<UnitWebSocket>(
                    socketPoll(), "/cool/" + secondWopiSrc + "/ws", getTestname());
                helpers::sendTextFrame(_secondWs->getWebSocket(), "load url=" + secondWopiSrc,
                                       getTestname());
                return false;
            }

            if (_phase == Phase::WaitSecondResolved && requestCount() == 2)
            {
                // The first view's list did not change, so this is the second view's, once
                // its own request was answered.
                const std::vector<LinkAccessRequest> asked = requests();
                assertRequest(asked[1], "secondtoken");

                TRANSITION_STATE(_phase, Phase::WaitConnected);
                helpers::sendTextFrame(_secondWs->getWebSocket(),
                                       "remotedocsubscribe wopisrc=" +
                                           Uri::encode(remoteWopiSrc()),
                                       getTestname());
            }

            return false;
        }

        if (!message.starts_with("remotedocevent:"))
            return false;

        TST_LOG("Got: [" << message << ']');

        if (message.find("event=connected") != std::string::npos)
        {
            if (_phase == Phase::WaitConnected)
            {
                TRANSITION_STATE(_phase, Phase::Done);
                passTest("Each view resolved the source with its own token");
            }
        }
        else if (message.find("event=error") != std::string::npos)
        {
            failTest("Unexpected remote document error: " + std::string(message));
        }

        return false;
    }

    void invokeWSDTest() override
    {
        if (_phase == Phase::Load)
        {
            TRANSITION_STATE(_phase, Phase::WaitFirstLoad);

            initWebsocket("/wopi/files/1?access_token=firsttoken");
            WSD_CMD("load url=" + getWopiSrc());
        }
    }
};

UnitBase** unit_create_wsd_multi(void)
{
    return new UnitBase* [] { new UnitRemoteDocument(), new UnitRemoteDocumentCycle(),
                              new UnitLinkPost(), new UnitLinkDelete(),
                              new UnitRemoteDocumentMutual(),
                              new UnitRemoteDocumentCommand(), new UnitRemoteDocumentMissing(),
                              new UnitRemoteDocumentRetry(), new UnitRemoteDocumentIsolation(),
                              new UnitRemoteDocumentSaved(),
                              new UnitRemoteDocumentNoChain(), new UnitLinkPostPersistentLink(),
                              new UnitLinkAccessResolve(), new UnitLinkAccessNotFound(),
                              new UnitLinkAccessPerView(), nullptr };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
