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

#include <common/Protocol.hpp>
#include <common/Uri.hpp>
#include <helpers.hpp>
#include <lokassert.hpp>

#include <WopiTestServer.hpp>

#include <net/HttpRequest.hpp>
#include <wsd/ClientSession.hpp>
#include <wsd/RemoteDocumentBroker.hpp>

#include <Poco/Net/HTTPRequest.h>

#include <set>
#include <string>
#include <thread>

namespace
{
// Fills CheckFileInfo with one related document, split the way the production
// code now expects a WOPI host to send it: the public part (WOPISrc and the
// last-modified time) in the top-level RelatedDocuments, and this view's
// private access token in UserPrivateInfo.RelatedDocuments.
void setRelatedDocument(Poco::JSON::Object::Ptr& fileInfo, const std::string& wopiSrc,
                        const std::string& accessToken,
                        const std::string& lastModifiedTime = std::string())
{
    Poco::JSON::Array::Ptr relatedDocuments = new Poco::JSON::Array();
    Poco::JSON::Object::Ptr entry = new Poco::JSON::Object();
    entry->set("WOPISrc", wopiSrc);
    if (!lastModifiedTime.empty())
        entry->set("LastModifiedTime", lastModifiedTime);
    relatedDocuments->add(entry);
    fileInfo->set("RelatedDocuments", relatedDocuments);

    Poco::JSON::Array::Ptr tokens = new Poco::JSON::Array();
    Poco::JSON::Object::Ptr tokenEntry = new Poco::JSON::Object();
    tokenEntry->set("WOPISrc", wopiSrc);
    tokenEntry->set("AccessToken", accessToken);
    tokens->add(tokenEntry);

    Poco::JSON::Object::Ptr userPrivateInfo = fileInfo->getObject("UserPrivateInfo");
    if (!userPrivateInfo)
        userPrivateInfo = new Poco::JSON::Object();
    userPrivateInfo->set("RelatedDocuments", tokens);
    fileInfo->set("UserPrivateInfo", userPrivateInfo);
}
} // namespace

/// A document (file 1) subscribes to a remote document (file 2) named in its
/// CheckFileInfo RelatedDocuments. Verifies that the subscriber receives
/// connected and modified events while another user edits the remote
/// document, and that unsubscribing closes the headless session.
class UnitRemoteDocument : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitConnected, LoadEditor, WaitEditorView, WaitModified,
               WaitHeadlessGone, Done)
    _phase;

    /// A second user editing the remote document (file 2).
    std::unique_ptr<UnitWebSocket> _editorWs;

    /// The subscription states seen in the relateddocuments: messages.
    bool _sawAvailableState = false;
    bool _sawConnectedState = false;

    std::string remoteWopiSrc() const
    {
        return helpers::getTestServerURI() + "/wopi/files/2";
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
        config.setBool("remote_documents.enable", true);
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& request,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        // Only the subscribing document lists a related document.
        if (Poco::URI(request.getURI()).getPath().ends_with("/1"))
        {
            setRelatedDocument(fileInfo, remoteWopiSrc(), "remotetoken",
                               "2026-09-01T12:00:00.000000Z");
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
        if (message.starts_with("relateddocuments:"))
        {
            TST_LOG("Got: [" << message << ']');
            LOK_ASSERT_MESSAGE("The related documents JSON must not carry access tokens",
                               message.find("remotetoken") == std::string_view::npos);
            LOK_ASSERT_MESSAGE("The related documents JSON must carry the last modified time",
                               message.find("\"lastModifiedTime\":\"2026-09-01T12:00:00.000000Z\"") !=
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
            LOK_ASSERT_MESSAGE("The clients saw the related document as available",
                               _sawAvailableState);
            LOK_ASSERT_MESSAGE("The clients saw the related document as connected",
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

    std::string ownWopiSrc() const { return helpers::getTestServerURI() + "/wopi/files/1"; }

public:
    UnitRemoteDocumentCycle()
        : WopiTestServer("UnitRemoteDocumentCycle")
        , _phase(Phase::Load)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);
        config.setBool("remote_documents.enable", true);
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& request,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        if (Poco::URI(request.getURI()).getPath().ends_with("/1"))
        {
            // The document lists itself as a related document.
            setRelatedDocument(fileInfo, ownWopiSrc(), "remotetoken");
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
                    Uri::encode(helpers::getTestServerURI() + "/wopi/files/3"));
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

/// Registers a related document over POST /cool/relateddocument: the request
/// is authorized by the view's own one-time token, a wrong token is refused,
/// a body over the size a registration takes is refused before it is read,
/// the token is accepted only once, and the registered access token then
/// serves that view's subscription.
class UnitRelatedDocumentPost : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitToken, Posting, WaitConnected, Done) _phase;

    /// The one-time token the view was handed for the POST.
    std::string _oneTimeToken;
    bool _documentLoaded = false;

    std::thread _postThread;

    std::string documentWopiSrc() const
    {
        return helpers::getTestServerURI() + "/wopi/files/1";
    }

    std::string remoteWopiSrc() const
    {
        return helpers::getTestServerURI() + "/wopi/files/2";
    }

    /// POSTs the registration authorized by the given one-time token, naming the given access
    /// token, and returns the response status.
    unsigned postRelatedDocument(const std::string& oneTimeToken,
                                 const std::string& accessToken = "remotetoken")
    {
        http::Request request("/cool/relateddocument?WOPISrc=" + Uri::encode(documentWopiSrc()),
                              http::Request::VERB_POST);
        request.setBody("{\"Nonce\":\"" + oneTimeToken +
                            "\",\"RelatedDocument\":{\"WOPISrc\":\"" + remoteWopiSrc() +
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
                                 postRelatedDocument("wrongtoken"));

                // A body too large to name one document and one token is refused on its
                // length alone, before the token in it is read.
                LOK_ASSERT_EQUAL(static_cast<unsigned>(http::StatusCode::PayloadTooLarge),
                                 postRelatedDocument(oneTimeToken, std::string(64 * 1024, 'x')));

                // The view's own one-time token authorizes the registration, so the refused
                // request above did not spend it.
                LOK_ASSERT_EQUAL(static_cast<unsigned>(http::StatusCode::OK),
                                 postRelatedDocument(oneTimeToken));

                // The same token is not accepted a second time.
                LOK_ASSERT_EQUAL(static_cast<unsigned>(http::StatusCode::Unauthorized),
                                 postRelatedDocument(oneTimeToken));

                // The registered token serves this view's subscription.
                TRANSITION_STATE(_phase, Phase::WaitConnected);
                WSD_CMD("remotedocsubscribe wopisrc=" + Uri::encode(remoteWopiSrc()));
            });
    }

public:
    UnitRelatedDocumentPost()
        : WopiTestServer("UnitRelatedDocumentPost")
        , _phase(Phase::Load)
    {
    }

    ~UnitRelatedDocumentPost()
    {
        if (_postThread.joinable())
            _postThread.join();
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);
        config.setBool("remote_documents.enable", true);
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
        if (message.starts_with("relateddocumenttoken:"))
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

        if (message.starts_with("relateddocuments:"))
        {
            TST_LOG("Got: [" << message << ']');
            LOK_ASSERT_MESSAGE(
                "The POST-registered last modified time must reach the related documents JSON",
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
                passTest("The view's one-time token registered a related document");
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
        return helpers::getTestServerURI() + "/wopi/files/" + std::to_string(fileId);
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
        config.setBool("remote_documents.enable", true);
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& request,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        // Each document lists the other as a related document.
        const std::string path = Poco::URI(request.getURI()).getPath();
        const int other = path.ends_with("/1") ? 2 : path.ends_with("/2") ? 1 : 0;
        if (other)
        {
            setRelatedDocument(fileInfo, fileWopiSrc(other), "remotetoken");
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
        return helpers::getTestServerURI() + "/wopi/files/2";
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
        config.setBool("remote_documents.enable", true);
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& request,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        if (Poco::URI(request.getURI()).getPath().ends_with("/1"))
        {
            setRelatedDocument(fileInfo, remoteWopiSrc(), "remotetoken");
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

/// A document subscribes to a related document whose file is gone from storage.
/// The storage answers the remote load with 404, so the subscriber is told the
/// source is missing rather than merely disconnected.
class UnitRemoteDocumentMissing : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitMissing, Done) _phase;

    std::string remoteWopiSrc() const { return helpers::getTestServerURI() + "/wopi/files/2"; }

    std::string encodedRemoteWopiSrc() const { return Uri::encode(remoteWopiSrc()); }

public:
    UnitRemoteDocumentMissing()
        : WopiTestServer("UnitRemoteDocumentMissing")
        , _phase(Phase::Load)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);
        config.setBool("remote_documents.enable", true);
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& request,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        // The subscribing document lists the missing file as a related document.
        if (Poco::URI(request.getURI()).getPath().ends_with("/1"))
        {
            setRelatedDocument(fileInfo, remoteWopiSrc(), "remotetoken");
        }
    }

    std::unique_ptr<http::Response>
    assertCheckFileInfoRequest(const Poco::Net::HTTPRequest& request) override
    {
        // The related document's file is gone, so the storage cannot find it.
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
            // The subscriber document is up; ask for the missing related document.
            TRANSITION_STATE(_phase, Phase::WaitMissing);
            WSD_CMD("remotedocsubscribe wopisrc=" + encodedRemoteWopiSrc());
        }

        return true;
    }

    bool onFilterSendWebSocketMessage(const std::string_view message, const WSOpCode /*code*/,
                                      const bool /*flush*/, int& /*unitReturn*/) override
    {
        if (!message.starts_with("relateddocuments:"))
            return false;

        TST_LOG("Got: [" << message << ']');

        // The subscribe fails to read the source, so the entry ends up missing.
        if (_phase == Phase::WaitMissing &&
            message.find("\"state\":\"missing\"") != std::string_view::npos)
        {
            TRANSITION_STATE(_phase, Phase::Done);
            passTest("The subscriber was told the source is missing when its file is gone");
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
        return helpers::getTestServerURI() + "/wopi/files/2";
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
        config.setBool("remote_documents.enable", true);
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
            setRelatedDocument(fileInfo, remoteWopiSrc(), "remotetoken");
        }
        else
        {
            Poco::JSON::Array::Ptr relatedDocuments = new Poco::JSON::Array();
            Poco::JSON::Object::Ptr entry = new Poco::JSON::Object();
            entry->set("WOPISrc", remoteWopiSrc());
            relatedDocuments->add(entry);
            fileInfo->set("RelatedDocuments", relatedDocuments);
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
                    Uri::encode(helpers::getTestServerURI() + "/wopi/files/1?access_token=secondtoken");
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
/// user saves the remote document to storage, the subscriber's related
/// documents list picks up the source's new last-modified time and the client
/// is told the source is newer.
class UnitRemoteDocumentSaved : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitConnected, LoadEditor, WaitEditorView, WaitSaved,
               Done)
    _phase;

    /// A second user editing the remote document (file 2).
    std::unique_ptr<UnitWebSocket> _editorWs;

    /// The last-modified time last seen in a related documents list.
    std::string _lastSeenModifiedTime;
    /// The time seen just before the editor saved the remote document.
    std::string _modifiedTimeBeforeSave;
    bool _sawSavedEvent = false;
    bool _sawUpdatedTime = false;

    std::string remoteWopiSrc() const { return helpers::getTestServerURI() + "/wopi/files/2"; }

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
            passTest("The remote save updated the related document's last modified time and "
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
        config.setBool("remote_documents.enable", true);
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& request,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        if (Poco::URI(request.getURI()).getPath().ends_with("/1"))
        {
            setRelatedDocument(fileInfo, remoteWopiSrc(), "remotetoken",
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
        if (message.starts_with("relateddocuments:"))
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
/// names the documents already on the chain it was made for. Verifies that
/// such a connection naming no chain is refused, and that the same connection
/// naming one loads the document.
class UnitRemoteDocumentNoChain : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, Connecting, Done) _phase;

    /// Carries the two connections below, so that waiting for their answers
    /// leaves coolwsd's main thread free to fork the kit one of them loads in.
    std::thread _connectThread;

    std::string fileWopiSrc(int fileId) const
    {
        return helpers::getTestServerURI() + "/wopi/files/" + std::to_string(fileId);
    }

    /// Loads the given document over a connection carrying the secret of a
    /// headless one, with the given options after the url. Returns the answer
    /// to the load: a status: for a document that loaded, an error: for a load
    /// that was refused, and an empty string when neither arrived.
    std::string loadOverHeadlessConnection(const std::string& wopiSrc,
                                           const std::string& loadOptions)
    {
        const std::string encodedWopiSrc = Uri::encode(wopiSrc + "?access_token=anything");

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

        helpers::sendTextFrame(session, "load url=" + encodedWopiSrc + " readonly=1" + loadOptions,
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
        config.setBool("remote_documents.enable", true);
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
                passTest("A headless connection naming no chain is refused, and one naming a "
                         "chain loads");
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

UnitBase** unit_create_wsd_multi(void)
{
    return new UnitBase* [] { new UnitRemoteDocument(), new UnitRemoteDocumentCycle(),
                              new UnitRelatedDocumentPost(), new UnitRemoteDocumentMutual(),
                              new UnitRemoteDocumentCommand(), new UnitRemoteDocumentMissing(),
                              new UnitRemoteDocumentIsolation(), new UnitRemoteDocumentSaved(),
                              new UnitRemoteDocumentNoChain(), nullptr };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
