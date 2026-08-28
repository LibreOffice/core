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

#include <Poco/Net/HTTPRequest.h>

#include <set>
#include <string>
#include <thread>

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
            Poco::JSON::Array::Ptr relatedDocuments = new Poco::JSON::Array();
            Poco::JSON::Object::Ptr entry = new Poco::JSON::Object();
            entry->set("WOPISrc", remoteWopiSrc());
            entry->set("AccessToken", "remotetoken");
            relatedDocuments->add(entry);
            fileInfo->set("RelatedDocuments", relatedDocuments);
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
            Poco::JSON::Array::Ptr relatedDocuments = new Poco::JSON::Array();
            Poco::JSON::Object::Ptr entry = new Poco::JSON::Object();
            entry->set("WOPISrc", ownWopiSrc());
            entry->set("AccessToken", "remotetoken");
            relatedDocuments->add(entry);
            fileInfo->set("RelatedDocuments", relatedDocuments);
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

/// One document holds two links to the same remote document. Both links
/// receive events, dropping one link by tag keeps the other alive, and
/// dropping the rest disconnects the headless session.
class UnitRemoteDocumentTags : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitConnectedOne, WaitConnectedTwo, LoadEditor,
               WaitEditorView, WaitBothModified, WaitUnsubscribedOne, WaitSecondLinkEvent,
               WaitUnsubscribedAll, WaitHeadlessGone, Done)
    _phase;

    /// A user editing the remote document (file 2).
    std::unique_ptr<UnitWebSocket> _editorWs;

    /// The tags that received the modified event.
    std::set<std::string> _modifiedTags;

    std::string remoteWopiSrc() const
    {
        return helpers::getTestServerURI() + "/wopi/files/2";
    }

    std::string encodedRemoteWopiSrc() const { return Uri::encode(remoteWopiSrc()); }

public:
    UnitRemoteDocumentTags()
        : WopiTestServer("UnitRemoteDocumentTags")
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
            Poco::JSON::Array::Ptr relatedDocuments = new Poco::JSON::Array();
            Poco::JSON::Object::Ptr entry = new Poco::JSON::Object();
            entry->set("WOPISrc", remoteWopiSrc());
            entry->set("AccessToken", "remotetoken");
            relatedDocuments->add(entry);
            fileInfo->set("RelatedDocuments", relatedDocuments);
        }
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');

        if (_phase == Phase::WaitLoadStatus)
        {
            TRANSITION_STATE(_phase, Phase::WaitConnectedOne);
            WSD_CMD("remotedocsubscribe wopisrc=" + encodedRemoteWopiSrc());
        }

        return true;
    }

    bool onFilterSendWebSocketMessage(const std::string_view message, const WSOpCode /*code*/,
                                      const bool /*flush*/, int& /*unitReturn*/) override
    {
        if (!message.starts_with("remotedocevent:"))
            return false;

        TST_LOG("Got: [" << message << ']');

        if (message.find("event=error") != std::string::npos)
        {
            failTest("Unexpected remote document error: " + std::string(message));
        }
        else if (message.find("event=connected") != std::string::npos)
        {
            if (_phase == Phase::WaitConnectedOne)
            {
                LOK_ASSERT_MESSAGE("Expected the first link to connect",
                                   message.find("tag=1 ") != std::string::npos);

                // A second link from the same document to the same remote document.
                TRANSITION_STATE(_phase, Phase::WaitConnectedTwo);
                WSD_CMD("remotedocsubscribe wopisrc=" + encodedRemoteWopiSrc());
            }
            else if (_phase == Phase::WaitConnectedTwo)
            {
                LOK_ASSERT_MESSAGE("Expected the second link to connect",
                                   message.find("tag=2 ") != std::string::npos);
                TRANSITION_STATE(_phase, Phase::LoadEditor);
            }
        }
        else if (message.find("event=modified value=true") != std::string::npos)
        {
            if (_phase == Phase::WaitBothModified)
            {
                std::string tag;
                COOLProtocol::getTokenStringFromMessage(message, "tag", tag);
                _modifiedTags.insert(tag);

                // Both links carry the modification.
                if (_modifiedTags.size() == 2)
                {
                    TRANSITION_STATE(_phase, Phase::WaitUnsubscribedOne);
                    WSD_CMD("remotedocunsubscribe wopisrc=" + encodedRemoteWopiSrc() + " tag=1");
                }
            }
        }
        else if (message.find("event=unsubscribed") != std::string::npos)
        {
            if (_phase == Phase::WaitUnsubscribedOne)
            {
                LOK_ASSERT_MESSAGE("Expected only the first link to be dropped",
                                   message.find("tag=1 ") != std::string::npos);

                // Saving the remote document turns its modified state off,
                // which the remaining link must still deliver.
                TRANSITION_STATE(_phase, Phase::WaitSecondLinkEvent);
                helpers::sendTextFrame(_editorWs->getWebSocket(),
                                       "save dontTerminateEdit=1 dontSaveIfUnmodified=0",
                                       getTestname());
            }
            else if (_phase == Phase::WaitUnsubscribedAll)
            {
                LOK_ASSERT_MESSAGE("Expected the second link to be dropped",
                                   message.find("tag=2 ") != std::string::npos);
                TRANSITION_STATE(_phase, Phase::WaitHeadlessGone);
            }
        }
        else if (message.find("event=modified value=false") != std::string::npos)
        {
            if (_phase == Phase::WaitSecondLinkEvent)
            {
                LOK_ASSERT_MESSAGE("Expected the surviving link to deliver the event",
                                   message.find("tag=2 ") != std::string::npos);

                // Dropping the remaining link ends the whole subscription.
                TRANSITION_STATE(_phase, Phase::WaitUnsubscribedAll);
                WSD_CMD("remotedocunsubscribe wopisrc=" + encodedRemoteWopiSrc());
            }
        }

        return false;
    }

    bool onViewLoaded(const std::string& message) override
    {
        TST_LOG("onViewLoaded: [" << message << ']');

        if (_phase == Phase::WaitEditorView)
        {
            TRANSITION_STATE(_phase, Phase::WaitBothModified);
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

        if (_phase == Phase::WaitHeadlessGone && docKey.ends_with("2") && session->isReadOnly())
        {
            TRANSITION_STATE(_phase, Phase::Done);
            passTest("Two links to one remote document lived and died independently");
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
            default:
            {
                break;
            }
        }
    }
};

/// Registers a related document over POST /cool/relateddocument: the access
/// token of any live session authorizes the request, a stranger's token does
/// not, and the registered token then serves a subscription.
class UnitRelatedDocumentPost : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, LoadSecond, WaitSecondView, Posting, WaitConnected,
               Done)
    _phase;

    /// A second user of the same document, holding its own access token.
    std::unique_ptr<UnitWebSocket> _secondWs;

    std::thread _postThread;

    std::string documentWopiSrc() const
    {
        return helpers::getTestServerURI() + "/wopi/files/1";
    }

    std::string remoteWopiSrc() const
    {
        return helpers::getTestServerURI() + "/wopi/files/2";
    }

    /// POSTs the registration authorized by the given token and returns the
    /// response status.
    unsigned postRelatedDocument(const std::string& accessToken)
    {
        http::Request request("/cool/relateddocument?WOPISrc=" +
                                  Uri::encode(documentWopiSrc()),
                              http::Request::VERB_POST);
        request.setBody("{\"AccessToken\":\"" + accessToken +
                            "\",\"RelatedDocument\":{\"WOPISrc\":\"" + remoteWopiSrc() +
                            "\",\"AccessToken\":\"remotetoken\"}}",
                        "application/json");

        auto session = http::Session::create(helpers::getTestServerURI());
        session->setTimeout(std::chrono::seconds(10));
        const std::shared_ptr<const http::Response> response = session->syncRequest(request);
        return response ? static_cast<unsigned>(response->statusLine().statusCode()) : 0;
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

        if (_phase == Phase::WaitLoadStatus)
            TRANSITION_STATE(_phase, Phase::LoadSecond);

        return true;
    }

    bool onViewLoaded(const std::string& message) override
    {
        TST_LOG("onViewLoaded: [" << message << ']');

        if (_phase == Phase::WaitSecondView)
        {
            TRANSITION_STATE(_phase, Phase::Posting);
            _postThread = std::thread(
                [this]
                {
                    // A token no session holds is refused.
                    LOK_ASSERT_EQUAL(static_cast<unsigned>(http::StatusCode::Unauthorized),
                                     postRelatedDocument("strangertoken"));

                    // The second user's token authorizes the registration.
                    LOK_ASSERT_EQUAL(static_cast<unsigned>(http::StatusCode::OK),
                                     postRelatedDocument("secondtoken"));

                    // The registered token serves the subscription.
                    TRANSITION_STATE(_phase, Phase::WaitConnected);
                    WSD_CMD("remotedocsubscribe wopisrc=" + Uri::encode(remoteWopiSrc()));
                });
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
        {
            // The event echo reaches every view, so it may be seen again
            // after the first delivery.
            if (_phase == Phase::WaitConnected)
            {
                TRANSITION_STATE(_phase, Phase::Done);
                passTest("A live collaborator's token registered a related document");
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
        switch (_phase)
        {
            case Phase::Load:
            {
                TRANSITION_STATE(_phase, Phase::WaitLoadStatus);

                initWebsocket("/wopi/files/1?access_token=firsttoken");
                WSD_CMD("load url=" + getWopiSrc());
                break;
            }
            case Phase::LoadSecond:
            {
                TRANSITION_STATE(_phase, Phase::WaitSecondView);

                const std::string secondWopiSrc =
                    Uri::encode(documentWopiSrc() + "?access_token=secondtoken");
                TST_LOG("Connecting a second user: " << secondWopiSrc);
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
            Poco::JSON::Array::Ptr relatedDocuments = new Poco::JSON::Array();
            Poco::JSON::Object::Ptr entry = new Poco::JSON::Object();
            entry->set("WOPISrc", fileWopiSrc(other));
            entry->set("AccessToken", "remotetoken");
            relatedDocuments->add(entry);
            fileInfo->set("RelatedDocuments", relatedDocuments);
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
            Poco::JSON::Array::Ptr relatedDocuments = new Poco::JSON::Array();
            Poco::JSON::Object::Ptr entry = new Poco::JSON::Object();
            entry->set("WOPISrc", remoteWopiSrc());
            entry->set("AccessToken", "remotetoken");
            relatedDocuments->add(entry);
            fileInfo->set("RelatedDocuments", relatedDocuments);
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

UnitBase** unit_create_wsd_multi(void)
{
    return new UnitBase* [] { new UnitRemoteDocument(), new UnitRemoteDocumentCycle(),
                              new UnitRemoteDocumentTags(), new UnitRelatedDocumentPost(),
                              new UnitRemoteDocumentMutual(), new UnitRemoteDocumentCommand(),
                              nullptr };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
