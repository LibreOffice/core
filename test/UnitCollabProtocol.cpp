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

#include <config.h>

#include <WopiTestServer.hpp>
#include <common/JsonUtil.hpp>
#include <common/Log.hpp>
#include <helpers.hpp>
#include <wsd/CollabSocketHandler.hpp>
#include <wsd/ContentSecurityPolicy.hpp>

#include <Poco/AutoPtr.h>
#include <Poco/DOM/DOMParser.h>
#include <Poco/DOM/Document.h>
#include <Poco/DOM/Element.h>
#include <Poco/DOM/NodeList.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Util/LayeredConfiguration.h>

#include <chrono>
#include <map>
#include <mutex>
#include <vector>

/// Try to receive a JSON message matching the given type prefix.
/// Returns nullptr if no matching message is available yet.
static Poco::JSON::Object::Ptr
pollCollabMessage(const std::shared_ptr<http::WebSocketSession>& ws,
                  const std::string& type)
{
    const std::string prefix = "{\"type\":\"" + type + "\"";
    auto response = ws->waitForMessage(prefix, std::chrono::milliseconds(1),
                                       "collab");
    if (response.empty())
        return nullptr;

    Poco::JSON::Parser parser;
    return parser.parse(std::string(response.data(), response.size()))
               .extract<Poco::JSON::Object::Ptr>();
}

/// Start a collab WebSocket connection and send the access_token.
/// The caller must poll for user_list/authenticated in later phases.
static std::shared_ptr<http::WebSocketSession>
startCollabConnection(const std::shared_ptr<SocketPoll>& socketPoll,
                      const std::string& wopiSrc,
                      const std::string& accessToken)
{
    const std::string path = "/cool/collab/ws?WOPISrc=" + wopiSrc + "&protocol=1";
    auto ws = http::WebSocketSession::create(helpers::getTestServerURI());
    http::Request req(path);
    ws->asyncRequest(req, socketPoll);
    ws->sendMessage("access_token " + accessToken);
    return ws;
}

// Transition helper for plain enum class (STATE_ENUM's
// TRANSITION_STATE requires the name() function it generates).
#define PHASE_TRANSITION(STATE)                                \
    do {                                                       \
        TST_LOG("Phase -> " #STATE);                           \
        _phase = Phase::STATE;                                 \
        SocketPoll::wakeupWorld();                             \
    } while (false)

/// Tests the /cool/collab/ws WebSocket protocol:
/// 1. editing_started is broadcast to other connected users
/// 2. editingActive flag is set in user_list for late joiners
/// 3. switch_to_collab reaches the users who edit a local copy, and not a user who only views
class UnitCollabProtocol : public WopiTestServer
{
    // Plain enum class because STATE_ENUM supports at most 15 values.
    enum class Phase {
        // Test 1: editing_started broadcast
        T1_Connect1, T1_WaitAuth1, T1_Connect2, T1_WaitAuth2,
        T1_SendEditing, T1_WaitBroadcast,
        // Test 2: editingActive on late join
        T2_Connect1, T2_WaitAuth1, T2_StartEditing,
        T2_WaitDisconnect, T2_Connect2, T2_WaitAuth2,
        // Test 3: switch_to_collab goes to the editing users only
        T3_Connect1, T3_WaitAuth1, T3_Connect2, T3_WaitAuth2,
        T3_Connect3, T3_WaitAuth3, T3_SendEditing, T3_WaitEditing,
        T3_SendSwitch, T3_WaitSwitch, T3_WaitViewer,
        Done,
    } _phase;

    std::size_t _checkFileInfoCount;
    std::shared_ptr<http::WebSocketSession> _ws1;
    std::shared_ptr<http::WebSocketSession> _ws2;
    std::shared_ptr<http::WebSocketSession> _ws3;
    std::string _wopiSrc;

public:
    UnitCollabProtocol()
        : WopiTestServer("UnitCollabProtocol")
        , _phase(Phase::T1_Connect1)
        , _checkFileInfoCount(0)
    {
    }

    void configure(Poco::Util::LayeredConfiguration & config) override {
        WopiTestServer::configure(config);
        config.setBool("client_document_loading.enable", true);
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& request,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        // The WOPISrc carries an access_token too, but only the one sent in the first message
        // reaches the WOPI host.
        const Poco::URI::QueryParameters params = Poco::URI(request.getURI()).getQueryParameters();
        LOK_ASSERT_EQUAL(std::size_t(1),
                         std::size_t(std::count_if(params.begin(), params.end(),
                                                   [](const auto& param)
                                                   { return param.first == "access_token"; })));

        ++_checkFileInfoCount;
        fileInfo->set("UserId",
                      "user_" + std::to_string(_checkFileInfoCount));
        fileInfo->set("UserFriendlyName",
                      "User " + std::to_string(_checkFileInfoCount));
        fileInfo->set("UserCanWrite", "true");
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            // -- Test 1: editing_started broadcast --

            case Phase::T1_Connect1:
            {
                PHASE_TRANSITION(T1_WaitAuth1);

                const Poco::URI wopiURL(
                    getWopiHostURI() +
                    "/wopi/files/0?access_token=anything"
                    "&testname=UnitCollabProtocol");
                _wopiSrc = Uri::encode(wopiURL.toString());

                _ws1 = startCollabConnection(socketPoll(), _wopiSrc,
                                             "anything");
                break;
            }
            case Phase::T1_WaitAuth1:
            {
                auto userList = pollCollabMessage(_ws1, "user_list");
                if (!userList)
                    break;

                auto users = userList->getArray("users");
                LOK_ASSERT_EQUAL_MESSAGE(
                    "First user should see no others",
                    static_cast<std::size_t>(0), users->size());
                LOK_ASSERT_EQUAL_MESSAGE(
                    "editingActive should be false initially",
                    false,
                    userList->optValue<bool>("editingActive", true));

                PHASE_TRANSITION(T1_Connect2);
                break;
            }
            case Phase::T1_Connect2:
            {
                PHASE_TRANSITION(T1_WaitAuth2);

                _ws2 = startCollabConnection(socketPoll(), _wopiSrc,
                                             "anything");
                break;
            }
            case Phase::T1_WaitAuth2:
            {
                auto userList = pollCollabMessage(_ws2, "user_list");
                if (!userList)
                    break;

                auto users = userList->getArray("users");
                LOK_ASSERT_EQUAL_MESSAGE(
                    "Second user should see first user",
                    static_cast<std::size_t>(1), users->size());

                PHASE_TRANSITION(T1_SendEditing);
                break;
            }
            case Phase::T1_SendEditing:
            {
                PHASE_TRANSITION(T1_WaitBroadcast);

                _ws1->sendMessage("{\"type\":\"editing_started\"}");
                break;
            }
            case Phase::T1_WaitBroadcast:
            {
                auto msg = pollCollabMessage(_ws2, "editing_started");
                if (!msg)
                    break;

                LOK_ASSERT_MESSAGE("Should have user object",
                                   msg->has("user"));
                auto user = msg->getObject("user");
                LOK_ASSERT_EQUAL(std::string("user_1"),
                                 user->getValue<std::string>("id"));

                TST_LOG("Test 1 passed: editing_started broadcast");

                _ws1->asyncShutdown();
                _ws2->asyncShutdown();
                _ws1.reset();
                _ws2.reset();

                PHASE_TRANSITION(T2_Connect1);
                break;
            }

            // -- Test 2: editingActive on late join --

            case Phase::T2_Connect1:
            {
                PHASE_TRANSITION(T2_WaitAuth1);

                const Poco::URI wopiURL(
                    getWopiHostURI() +
                    "/wopi/files/1?access_token=anything"
                    "&testname=UnitCollabProtocol");
                _wopiSrc = Uri::encode(wopiURL.toString());

                _ws1 = startCollabConnection(socketPoll(), _wopiSrc,
                                             "anything");
                break;
            }
            case Phase::T2_WaitAuth1:
            {
                auto userList = pollCollabMessage(_ws1, "user_list");
                if (!userList)
                    break;

                PHASE_TRANSITION(T2_StartEditing);
                break;
            }
            case Phase::T2_StartEditing:
            {
                PHASE_TRANSITION(T2_WaitDisconnect);

                _ws1->sendMessage("{\"type\":\"editing_started\"}");
                _ws1->asyncShutdown();
                _ws1.reset();
                break;
            }
            case Phase::T2_WaitDisconnect:
            {
                PHASE_TRANSITION(T2_Connect2);
                break;
            }
            case Phase::T2_Connect2:
            {
                PHASE_TRANSITION(T2_WaitAuth2);

                _ws2 = startCollabConnection(socketPoll(), _wopiSrc,
                                             "anything");
                break;
            }
            case Phase::T2_WaitAuth2:
            {
                auto userList = pollCollabMessage(_ws2, "user_list");
                if (!userList)
                    break;

                LOK_ASSERT_EQUAL_MESSAGE(
                    "editingActive should be true for late joiner",
                    true,
                    userList->optValue<bool>("editingActive", false));

                TST_LOG("Test 2 passed: editingActive on late join");

                _ws2->asyncShutdown();
                _ws2.reset();

                PHASE_TRANSITION(T3_Connect1);
                break;
            }

            // -- Test 3: switch_to_collab goes to the editing users only --

            case Phase::T3_Connect1:
            {
                PHASE_TRANSITION(T3_WaitAuth1);

                const Poco::URI wopiURL(
                    getWopiHostURI() +
                    "/wopi/files/2?access_token=anything"
                    "&testname=UnitCollabProtocol");
                _wopiSrc = Uri::encode(wopiURL.toString());

                _ws1 = startCollabConnection(socketPoll(), _wopiSrc,
                                             "anything");
                break;
            }
            case Phase::T3_WaitAuth1:
            {
                auto userList = pollCollabMessage(_ws1, "user_list");
                if (!userList)
                    break;

                PHASE_TRANSITION(T3_Connect2);
                break;
            }
            case Phase::T3_Connect2:
            {
                PHASE_TRANSITION(T3_WaitAuth2);

                _ws2 = startCollabConnection(socketPoll(), _wopiSrc,
                                             "anything");
                break;
            }
            case Phase::T3_WaitAuth2:
            {
                auto userList = pollCollabMessage(_ws2, "user_list");
                if (!userList)
                    break;

                PHASE_TRANSITION(T3_Connect3);
                break;
            }
            case Phase::T3_Connect3:
            {
                PHASE_TRANSITION(T3_WaitAuth3);
                _ws3 = startCollabConnection(socketPoll(), _wopiSrc, "anything");
                break;
            }
            case Phase::T3_WaitAuth3:
            {
                auto userList = pollCollabMessage(_ws3, "user_list");
                if (!userList) {
                    break;
                }
                PHASE_TRANSITION(T3_SendEditing);
                break;
            }
            case Phase::T3_SendEditing:
            {
                PHASE_TRANSITION(T3_WaitEditing);
                _ws1->sendMessage("{\"type\":\"editing_started\"}");
                break;
            }
            case Phase::T3_WaitEditing:
            {
                auto msg = pollCollabMessage(_ws2, "editing_started");
                if (!msg) {
                    break;
                }
                PHASE_TRANSITION(T3_SendSwitch);
                break;
            }
            case Phase::T3_SendSwitch:
            {
                PHASE_TRANSITION(T3_WaitSwitch);

                _ws2->sendMessage("{\"type\":\"switch_to_collab\"}");
                _ws2->sendMessage("{\"type\":\"saved_and_switching\"}");
                break;
            }
            case Phase::T3_WaitSwitch:
            {
                auto msg = pollCollabMessage(_ws1, "switch_to_collab");
                if (!msg)
                    break;

                LOK_ASSERT_MESSAGE("Should have user object",
                                   msg->has("user"));

                PHASE_TRANSITION(T3_WaitViewer);
                break;
            }
            case Phase::T3_WaitViewer:
            {
                // The viewer's messages come in order, so the saved_and_switching sent after the
                // switch_to_collab comes first when the viewer did not get the switch_to_collab:
                auto const response = _ws3->waitForMessageAny(
                    { "{\"type\":\"switch_to_collab\"", "{\"type\":\"saved_and_switching\"" },
                    std::chrono::milliseconds(1), "collab");
                if (response.empty()) {
                    break;
                }
                LOK_ASSERT_MESSAGE(
                    "A user who only views should not get switch_to_collab",
                    std::string(response.data(), response.size())
                        .starts_with("{\"type\":\"saved_and_switching\""));
                TST_LOG("Test 3 passed: switch_to_collab goes to the editing users only");

                _ws1->asyncShutdown();
                _ws2->asyncShutdown();
                _ws3->asyncShutdown();
                _ws1.reset();
                _ws2.reset();
                _ws3.reset();

                PHASE_TRANSITION(Done);
                passTest("All collab protocol tests passed");
                break;
            }
            case Phase::Done:
                break;
        }
    }
};


/// A host answering 503 to CheckFileInfo is busy, not refusing us. We used to
/// fold every unsuccessful answer together and tell the user their document
/// had failed to load; a busy host now reads as one that did not answer.
class UnitCollabCheckFileInfoTransient : public WopiTestServer
{
    enum class Phase { Connect, WaitError, Done } _phase;

    /// CheckFileInfo requests answered with 503.
    std::size_t _refused;

    std::shared_ptr<http::WebSocketSession> _ws;
    std::string _wopiSrc;

public:
    UnitCollabCheckFileInfoTransient()
        : WopiTestServer("UnitCollabCheckFileInfoTransient")
        , _phase(Phase::Connect)
        , _refused(0)
    {
    }

    void configure(Poco::Util::LayeredConfiguration & config) override {
        WopiTestServer::configure(config);
        config.setBool("client_document_loading.enable", true);
    }

    std::unique_ptr<http::Response>
    assertCheckFileInfoRequest(const Poco::Net::HTTPRequest& /*request*/) override
    {
        ++_refused;
        TST_LOG("Refusing CheckFileInfo #" << _refused << " with 503");

        // Give it a body: the fake host shuts the socket after answering, and a
        // bodiless answer reaches us as a dropped connection rather than as the
        // status we meant to send.
        auto response =
            std::make_unique<http::Response>(http::StatusCode::ServiceUnavailable);
        response->setBody("busy", "text/plain");
        return response;
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Connect:
            {
                PHASE_TRANSITION(WaitError);

                const Poco::URI wopiURL(getWopiHostURI() +
                                        "/wopi/files/0?access_token=anything"
                                        "&testname=UnitCollabCheckFileInfoTransient");
                _wopiSrc = Uri::encode(wopiURL.toString());

                _ws = startCollabConnection(socketPoll(), _wopiSrc, "anything");
                break;
            }
            case Phase::WaitError:
            {
                const auto response = _ws->waitForMessage("error:", std::chrono::milliseconds(1),
                                                          "collab");
                if (response.empty())
                    break;

                const std::string message(response.data(), response.size());
                TST_LOG("Received: " << message);

                // "loadfailed" is what a refusal earns. A host that is merely
                // busy has not refused us and must not be reported as one.
                LOK_ASSERT_MESSAGE("Reported a busy host as a failure to load: " + message,
                                   message.find("kind=loadfailed") == std::string::npos);
                LOK_ASSERT_MESSAGE("Expected a busy host to read as no answer: " + message,
                                   message.find("kind=timeout") != std::string::npos);

                PHASE_TRANSITION(Done);
                break;
            }
            case Phase::Done:
            {
                LOK_ASSERT_MESSAGE("Expected CheckFileInfo to have been asked", _refused > 0);
                passTest("A busy host reads as no answer, not a failure to load");
                break;
            }
        }
    }
};

/// An upload by a collab client with the version that it got is refused once the document changed
/// in storage, one without a version is refused unless the client forces it:
class UnitCollabUploadTimestamp: public WopiTestServer {
public:
    UnitCollabUploadTimestamp():
        WopiTestServer("UnitCollabUploadTimestamp"), _phase(Phase::ConnectCollab) {}

    void configure(Poco::Util::LayeredConfiguration & config) override {
        WopiTestServer::configure(config);
        config.setBool("client_document_loading.enable", true);
    }

    void invokeWSDTest() override {
        switch (_phase) {
        case Phase::ConnectCollab: {
            PHASE_TRANSITION(WaitCollabAuth);

            Poco::URI const wopiURL(
                getWopiHostURI()
                + "/wopi/files/0?access_token=anything"
                  "&testname=UnitCollabUploadTimestamp");
            _collabWs = startCollabConnection(
                socketPoll(), Uri::encode(wopiURL.toString()), "anything");
            break;
        }
        case Phase::WaitCollabAuth: {
            if (!pollCollabMessage(_collabWs, "user_list")) {
                break;
            }

            PHASE_TRANSITION(WaitFetchURL);
            _collabWs->sendMessage(R"({"type":"fetch","stream":"contents","requestId":"f1"})");
            break;
        }
        case Phase::WaitFetchURL: {
            Poco::JSON::Object::Ptr const msg = pollCollabMessage(_collabWs, "fetch_url");
            if (!msg) {
                break;
            }

            _lastModifiedTime = msg->optValue<std::string>("lastModifiedTime", std::string());
            LOK_ASSERT_MESSAGE("Expected the version with the URL", !_lastModifiedTime.empty());

            std::shared_ptr<http::Response const> const fetched
                = http::Session::create(helpers::getTestServerURI())
                      ->syncRequest(http::Request(msg->getValue<std::string>("url")));
            LOK_ASSERT_EQUAL(http::StatusCode::OK, fetched->statusLine().statusCode());
            LOK_ASSERT_EQUAL(getFileContent(), fetched->getBody());
            LOK_ASSERT_EQUAL(std::string("nosniff"), fetched->get("X-Content-Type-Options"));
            LOK_ASSERT_EQUAL(std::string("sandbox"), fetched->get("Content-Security-Policy"));
            LOK_ASSERT_EQUAL(std::string("attachment"), fetched->get("Content-Disposition"));

            PHASE_TRANSITION(WaitUploadURL);
            _collabWs->sendMessage(
                R"({"type":"upload","stream":"contents","requestId":"r1","lastModifiedTime":")"
                + _lastModifiedTime + R"("})");
            break;
        }
        case Phase::WaitUploadURL: {
            Poco::JSON::Object::Ptr const msg = pollCollabMessage(_collabWs, "upload_url");
            if (!msg) {
                break;
            }

            http::Request request(msg->getValue<std::string>("url"), http::Request::VERB_POST);
            request.setBody("Uploaded by the collab client", "application/octet-stream");
            std::shared_ptr<http::Response const> const response
                = http::Session::create(helpers::getTestServerURI())->syncRequest(request);
            LOK_ASSERT_EQUAL(http::StatusCode::OK, response->statusLine().statusCode());
            LOK_ASSERT_EQUAL(std::string("Uploaded by the collab client"), getFileContent());
            Poco::JSON::Object::Ptr result;
            LOK_ASSERT(JsonUtil::parseJSON(response->getBody(), result));
            _lastModifiedTime = result->optValue<std::string>("LastModifiedTime", std::string());
            LOK_ASSERT_MESSAGE(
                "Expected the new version with the answer", !_lastModifiedTime.empty());

            // Someone else saves the document in storage:
            setFileContent("Saved by someone else");

            PHASE_TRANSITION(WaitStaleUploadURL);
            _collabWs->sendMessage(
                R"({"type":"upload","stream":"contents","requestId":"r2","lastModifiedTime":")"
                + _lastModifiedTime + R"("})");
            break;
        }
        case Phase::WaitStaleUploadURL: {
            Poco::JSON::Object::Ptr const msg = pollCollabMessage(_collabWs, "upload_url");
            if (!msg) {
                break;
            }

            PHASE_TRANSITION(WaitUnversionedRefused);
            http::Request request(msg->getValue<std::string>("url"), http::Request::VERB_POST);
            request.setBody("Uploaded again by the collab client", "application/octet-stream");
            std::shared_ptr<http::Response const> const response
                = http::Session::create(helpers::getTestServerURI())->syncRequest(request);
            LOK_ASSERT_EQUAL(http::StatusCode::Conflict, response->statusLine().statusCode());
            LOK_ASSERT_EQUAL(std::string("Saved by someone else"), getFileContent());

            _collabWs->sendMessage(R"({"type":"upload","stream":"contents","requestId":"r3"})");
            break;
        }
        case Phase::WaitUnversionedRefused: {
            Poco::JSON::Object::Ptr const msg = pollCollabMessage(_collabWs, "upload_error");
            if (!msg) {
                break;
            }
            LOK_ASSERT_EQUAL(std::string("r3"), msg->getValue<std::string>("requestId"));

            PHASE_TRANSITION(WaitForcedUploadURL);
            _collabWs->sendMessage(
                R"({"type":"upload","stream":"contents","requestId":"r4","force":true})");
            break;
        }
        case Phase::WaitForcedUploadURL: {
            Poco::JSON::Object::Ptr const msg = pollCollabMessage(_collabWs, "upload_url");
            if (!msg) {
                break;
            }

            PHASE_TRANSITION(Done);
            http::Request request(msg->getValue<std::string>("url"), http::Request::VERB_POST);
            request.setBody("Forced by the collab client", "application/octet-stream");
            std::shared_ptr<http::Response const> const response
                = http::Session::create(helpers::getTestServerURI())->syncRequest(request);
            LOK_ASSERT_EQUAL(http::StatusCode::OK, response->statusLine().statusCode());
            LOK_ASSERT_EQUAL(std::string("Forced by the collab client"), getFileContent());

            _collabWs->asyncShutdown();
            passTest("A stale collab upload was refused, and a forced one was not");
            break;
        }
        case Phase::Done:
            break;
        }
    }

private:
    enum class Phase {
        ConnectCollab, WaitCollabAuth, WaitFetchURL, WaitUploadURL, WaitStaleUploadURL,
        WaitUnversionedRefused, WaitForcedUploadURL, Done
    } _phase;

    std::shared_ptr<http::WebSocketSession> _collabWs;

    std::string _lastModifiedTime;
};

/// An upload by a collab client reloads a plain-COOL session that has an older version of the
/// document loaded, with that document unloading at once:
class UnitCollabUploadConflict: public WopiTestServer {
public:
    UnitCollabUploadConflict():
        WopiTestServer("UnitCollabUploadConflict"), _phase(Phase::ConnectCollab) {}

    void configure(Poco::Util::LayeredConfiguration & config) override {
        WopiTestServer::configure(config);
        config.setBool("client_document_loading.enable", true);
    }

    bool onDocumentLoaded(std::string const & message) override {
        TST_LOG("Loaded: " << message);
        LOK_ASSERT_MESSAGE("Expected the document to load only once", _phase == Phase::WaitLoad);
        PHASE_TRANSITION(Upload);
        return true;
    }

    bool onFilterSendWebSocketMessage(
        std::string_view const message, WSOpCode const /*code*/, bool const /*flush*/,
        int & /*unitReturn*/) override {
        if (message.starts_with("close: documentconflict")) {
            TST_LOG("Received: " << message);
            // The unloading document sends its reason once more:
            if (_phase == Phase::WaitReload) {
                PHASE_TRANSITION(WaitUnload);
            } else {
                LOK_ASSERT_MESSAGE(
                    "Expected the reload only after the upload of the collab client",
                    _phase == Phase::WaitUnload);
            }
        }
        return false;
    }

    void onDocBrokerDestroy(std::string const & docKey) override {
        TST_LOG("Destroyed: " << docKey);
        LOK_ASSERT_MESSAGE(
            "Expected the document to unload right after the reload", _phase == Phase::WaitUnload);
        PHASE_TRANSITION(Done);
        _collabWs->asyncShutdown();
        passTest("The collab upload reloaded the older document, which unloaded");
    }

    void invokeWSDTest() override {
        switch (_phase) {
        case Phase::ConnectCollab: {
            PHASE_TRANSITION(WaitCollabAuth);

            Poco::URI const wopiURL(
                getWopiHostURI()
                + "/wopi/files/0?access_token=anything"
                  "&testname=UnitCollabUploadConflict");
            _collabWs = startCollabConnection(
                socketPoll(), Uri::encode(wopiURL.toString()), "anything");
            break;
        }
        case Phase::WaitCollabAuth: {
            if (!pollCollabMessage(_collabWs, "user_list")) {
                break;
            }

            PHASE_TRANSITION(WaitLoad);
            initWebsocket("/wopi/files/0?access_token=anything");
            WSD_CMD("load url=" + getWopiSrc());
            break;
        }
        case Phase::Upload: {
            PHASE_TRANSITION(WaitUploadURL);
            _collabWs->sendMessage(
                R"({"type":"upload","stream":"contents","requestId":"r1","force":true})");
            break;
        }
        case Phase::WaitUploadURL: {
            Poco::JSON::Object::Ptr const msg = pollCollabMessage(_collabWs, "upload_url");
            if (!msg) {
                break;
            }

            PHASE_TRANSITION(WaitReload);
            http::Request request(msg->getValue<std::string>("url"), http::Request::VERB_POST);
            request.setBody("Uploaded by the collab client", "application/octet-stream");
            std::shared_ptr<http::Response const> const response
                = http::Session::create(helpers::getTestServerURI())->syncRequest(request);
            LOK_ASSERT_EQUAL(http::StatusCode::OK, response->statusLine().statusCode());
            break;
        }
        case Phase::WaitLoad:
        case Phase::WaitReload:
        case Phase::WaitUnload:
        case Phase::Done:
            break;
        }
    }

private:
    enum class Phase {
        ConnectCollab, WaitCollabAuth, WaitLoad, Upload, WaitUploadURL, WaitReload, WaitUnload,
        Done
    } _phase;

    std::shared_ptr<http::WebSocketSession> _collabWs;
};

/// An upload by a collab client asks a plain-COOL user with unsaved changes what to do, and until
/// that user decides, another session cannot load the document and a save of that user does not
/// reach storage:
class UnitCollabUploadConflictModified: public WopiTestServer {
public:
    UnitCollabUploadConflictModified():
        WopiTestServer("UnitCollabUploadConflictModified"), _phase(Phase::ConnectCollab) {}

    void configure(Poco::Util::LayeredConfiguration & config) override {
        WopiTestServer::configure(config);
        config.setBool("client_document_loading.enable", true);
    }

    bool onDocumentLoaded(std::string const & message) override {
        TST_LOG("Loaded: " << message);
        LOK_ASSERT_MESSAGE("Expected the document to load only once", _phase == Phase::WaitLoad);
        PHASE_TRANSITION(WaitModified);
        WSD_CMD("key type=input char=97 key=0");
        WSD_CMD("key type=up char=0 key=512");
        return true;
    }

    bool onDocumentModified(std::string const & message) override {
        TST_LOG("Modified: " << message);
        if (_phase == Phase::WaitModified) {
            PHASE_TRANSITION(Upload);
        }
        return true;
    }

    bool onFilterSendWebSocketMessage(
        std::string_view const message, WSOpCode const /*code*/, bool const /*flush*/,
        int & /*unitReturn*/) override {
        if (message.starts_with("close: documentconflict")) {
            LOK_ASSERT_FAIL("Expected no reload of a document with unsaved changes");
        } else if (message.starts_with("error: cmd=storage kind=documentconflict")) {
            TST_LOG("Received: " << message);
            if (_phase == Phase::WaitConflict) {
                PHASE_TRANSITION(JoinOther);
            } else {
                LOK_ASSERT_MESSAGE(
                    "Expected the question again only after the save",
                    _phase == Phase::WaitAskedAgain);
                LOK_ASSERT_EQUAL_MESSAGE(
                    "Expected the save not to reach storage", _putFileCount, getCountPutFile());
                PHASE_TRANSITION(Done);
                passTest(
                    "The conflict kept another session out and the save of the user away from"
                    " storage");
            }
        } else if (message.starts_with("error: cmd=load kind=docunloading:collabconflict")) {
            TST_LOG("Received: " << message);
            LOK_ASSERT_MESSAGE(
                "Expected the other session to wait only while the user decides",
                _phase == Phase::WaitJoinRefused);
            PHASE_TRANSITION(Save);
        }
        return false;
    }

    void invokeWSDTest() override {
        switch (_phase) {
        case Phase::ConnectCollab: {
            PHASE_TRANSITION(WaitCollabAuth);

            Poco::URI const wopiURL(
                getWopiHostURI()
                + "/wopi/files/0?access_token=anything"
                  "&testname=UnitCollabUploadConflictModified");
            _collabWs = startCollabConnection(
                socketPoll(), Uri::encode(wopiURL.toString()), "anything");
            break;
        }
        case Phase::WaitCollabAuth: {
            if (!pollCollabMessage(_collabWs, "user_list")) {
                break;
            }

            PHASE_TRANSITION(WaitLoad);
            initWebsocket("/wopi/files/0?access_token=anything");
            WSD_CMD("load url=" + getWopiSrc());
            break;
        }
        case Phase::Upload: {
            PHASE_TRANSITION(WaitUploadURL);
            _collabWs->sendMessage(
                R"({"type":"upload","stream":"contents","requestId":"r1","force":true})");
            break;
        }
        case Phase::WaitUploadURL: {
            Poco::JSON::Object::Ptr const msg = pollCollabMessage(_collabWs, "upload_url");
            if (!msg) {
                break;
            }

            PHASE_TRANSITION(WaitConflict);
            http::Request request(msg->getValue<std::string>("url"), http::Request::VERB_POST);
            request.setBody("Uploaded by the collab client", "application/octet-stream");
            std::shared_ptr<http::Response const> const response
                = http::Session::create(helpers::getTestServerURI())->syncRequest(request);
            LOK_ASSERT_EQUAL(http::StatusCode::OK, response->statusLine().statusCode());
            break;
        }
        case Phase::JoinOther: {
            PHASE_TRANSITION(WaitJoinRefused);
            addWebSocket();
            WSD_CMD_BY_CONNECTION_INDEX(1, "load url=" + getWopiSrc());
            break;
        }
        case Phase::Save: {
            PHASE_TRANSITION(WaitAskedAgain);
            _putFileCount = getCountPutFile();
            WSD_CMD("save dontTerminateEdit=0 dontSaveIfUnmodified=0");
            break;
        }
        case Phase::WaitLoad:
        case Phase::WaitModified:
        case Phase::WaitConflict:
        case Phase::WaitJoinRefused:
        case Phase::WaitAskedAgain:
        case Phase::Done:
            break;
        }
    }

private:
    enum class Phase {
        ConnectCollab, WaitCollabAuth, WaitLoad, WaitModified, Upload, WaitUploadURL,
        WaitConflict, JoinOther, WaitJoinRefused, Save, WaitAskedAgain, Done
    } _phase;

    std::shared_ptr<http::WebSocketSession> _collabWs;

    std::size_t _putFileCount;
};

/// A collab client whose host allows no export and no writes gets neither the document bytes nor an
/// upload URL, and learns that it can only open the document through the server:
class UnitCollabNoCleanExport: public WopiTestServer {
public:
    UnitCollabNoCleanExport():
        WopiTestServer("UnitCollabNoCleanExport"), phase_(Phase::ConnectCollab) {}

    void configure(Poco::Util::LayeredConfiguration & config) override {
        WopiTestServer::configure(config);
        config.setBool("client_document_loading.enable", true);
    }

    void configCheckFileInfo(
        Poco::Net::HTTPRequest const &, Poco::JSON::Object::Ptr & fileInfo) override
    {
        fileInfo->set("UserCanWrite", false);
        fileInfo->set("DisableExport", true);
    }

    void invokeWSDTest() override {
        switch (phase_) {
        case Phase::ConnectCollab: {
            TRANSITION_STATE(phase_, Phase::WaitCollabAuth);
            Poco::URI const wopiURL(
                getWopiHostURI()
                + "/wopi/files/0?access_token=anything"
                  "&testname=UnitCollabNoCleanExport");
            collabWs_ = startCollabConnection(
                socketPoll(), Uri::encode(wopiURL.toString()), "anything");
            break;
        }
        case Phase::WaitCollabAuth: {
            if (!pollCollabMessage(collabWs_, "user_list")) {
                break;
            }
            TRANSITION_STATE(phase_, Phase::WaitContentsRefused);
            collabWs_->sendMessage(R"({"type":"fetch","stream":"contents","requestId":"f1"})");
            break;
        }
        case Phase::WaitContentsRefused: {
            Poco::JSON::Object::Ptr const msg = pollCollabMessage(collabWs_, "fetch_error");
            if (!msg) {
                break;
            }
            LOK_ASSERT_EQUAL(std::string("f1"), msg->getValue<std::string>("requestId"));
            LOK_ASSERT_EQUAL(
                std::string("Export not allowed"), msg->getValue<std::string>("error"));
            LOK_ASSERT_EQUAL(false, msg->getValue<bool>("cleanExport"));
            TRANSITION_STATE(phase_, Phase::WaitUploadRefused);
            collabWs_->sendMessage(R"({"type":"upload","stream":"contents","requestId":"u1"})");
            break;
        }
        case Phase::WaitUploadRefused: {
            Poco::JSON::Object::Ptr const msg = pollCollabMessage(collabWs_, "upload_error");
            if (!msg) {
                break;
            }
            LOK_ASSERT_EQUAL(std::string("u1"), msg->getValue<std::string>("requestId"));
            LOK_ASSERT_EQUAL(
                std::string("The host allows this user no writes"),
                msg->getValue<std::string>("error"));
            TRANSITION_STATE(phase_, Phase::WaitSharedSettingsUnavailable);
            collabWs_->sendMessage(
                R"({"type":"fetch","stream":"sharedSettings","requestId":"s1"})");
            break;
        }
        case Phase::WaitSharedSettingsUnavailable: {
            // The host names no settings. The answer says so, and does not blame the export rule.
            Poco::JSON::Object::Ptr const msg = pollCollabMessage(collabWs_, "fetch_error");
            if (!msg) {
                break;
            }
            LOK_ASSERT_EQUAL(std::string("s1"), msg->getValue<std::string>("requestId"));
            LOK_ASSERT_EQUAL(
                std::string("Stream not available"), msg->getValue<std::string>("error"));
            LOK_ASSERT(!msg->has("cleanExport"));
            TRANSITION_STATE(phase_, Phase::Done);
            collabWs_->asyncShutdown();
            passTest("A user without a clean export got no document bytes and no upload URL");
            break;
        }
        case Phase::Done:
            break;
        }
    }

private:
    STATE_ENUM(
        Phase, ConnectCollab, WaitCollabAuth, WaitContentsRefused, WaitUploadRefused,
        WaitSharedSettingsUnavailable, Done) phase_;

    std::shared_ptr<http::WebSocketSession> collabWs_;
};

// The avatar of a user who joins reaches the other users as an address under /cool/collab/avatar
// that carries the WOPISrc of the document, and that address answers with the avatar:
class UnitCollabAvatar: public WopiTestServer {
public:
    UnitCollabAvatar(): WopiTestServer("UnitCollabAvatar"), phase_(Phase::ConnectFirst) {}

    void configure(Poco::Util::LayeredConfiguration & config) override {
        WopiTestServer::configure(config);
        config.setBool("client_document_loading.enable", true);
    }

    void configCheckFileInfo(
        Poco::Net::HTTPRequest const &, Poco::JSON::Object::Ptr & fileInfo) override
    {
        Poco::JSON::Object::Ptr extra = new Poco::JSON::Object();
        extra->set("avatar", getWopiHostURI() + "/wopi/files/0/contents?testname=UnitCollabAvatar");
        fileInfo->set("UserExtraInfo", extra);
    }

    void invokeWSDTest() override {
        switch (phase_) {
        case Phase::ConnectFirst:
            TRANSITION_STATE(phase_, Phase::WaitFirstAuth);
            first_ = startCollabConnection(socketPoll(), wopiSrc(), "anything");
            break;
        case Phase::WaitFirstAuth:
            if (!pollCollabMessage(first_, "user_list")) {
                break;
            }
            TRANSITION_STATE(phase_, Phase::WaitJoined);
            second_ = startCollabConnection(socketPoll(), wopiSrc(), "anything");
            break;
        case Phase::WaitJoined: {
            Poco::JSON::Object::Ptr const msg = pollCollabMessage(first_, "user_joined");
            if (!msg) {
                break;
            }
            std::string const avatar
                = msg->getObject("user")->getValue<std::string>("avatar");
            LOK_ASSERT_MESSAGE(
                "The avatar is at " + avatar,
                avatar.starts_with("/cool/collab/avatar?")
                    && avatar.ends_with(
                        "&WOPISrc=" + Uri::encode(getWopiHostURI() + "/wopi/files/0")));
            std::shared_ptr<http::Response const> const response
                = http::get(helpers::getTestServerURI(), avatar + "&testname=UnitCollabAvatar");
            LOK_ASSERT_EQUAL(http::StatusCode::OK, response->statusLine().statusCode());
            LOK_ASSERT_EQUAL(getFileContent(), response->getBody());
            TRANSITION_STATE(phase_, Phase::Done);
            first_->asyncShutdown();
            second_->asyncShutdown();
            passTest("The avatar of a user who joined loaded from its address");
            break;
        }
        case Phase::Done:
            break;
        }
    }

private:
    std::string wopiSrc() const {
        return Uri::encode(
            getWopiHostURI() + "/wopi/files/0?access_token=anything&testname=UnitCollabAvatar");
    }

    STATE_ENUM(Phase, ConnectFirst, WaitFirstAuth, WaitJoined, Done) phase_;

    std::shared_ptr<http::WebSocketSession> first_;
    std::shared_ptr<http::WebSocketSession> second_;
};

// Two connections of one user are told apart by their connection IDs, also when one of them
// leaves:
class UnitCollabConnections: public WopiTestServer {
public:
    UnitCollabConnections(): WopiTestServer("UnitCollabConnections"), phase_(Phase::ConnectFirst) {}

    void configure(Poco::Util::LayeredConfiguration & config) override {
        WopiTestServer::configure(config);
        config.setBool("client_document_loading.enable", true);
    }

    void invokeWSDTest() override {
        switch (phase_) {
        case Phase::ConnectFirst:
            TRANSITION_STATE(phase_, Phase::WaitFirstAuth);
            first_ = startCollabConnection(socketPoll(), wopiSrc(), "anything");
            break;
        case Phase::WaitFirstAuth:
            if (!pollCollabMessage(first_, "user_list")) {
                break;
            }
            TRANSITION_STATE(phase_, Phase::WaitSecondAuth);
            second_ = startCollabConnection(socketPoll(), wopiSrc(), "anything");
            break;
        case Phase::WaitSecondAuth: {
            Poco::JSON::Object::Ptr const msg = pollCollabMessage(second_, "user_list");
            if (!msg) {
                break;
            }
            Poco::JSON::Array::Ptr const users = msg->getArray("users");
            LOK_ASSERT_EQUAL(std::size_t(1), users->size());
            firstConnection_ = users->getObject(0)->getValue<std::string>("connection");
            TRANSITION_STATE(phase_, Phase::WaitJoined);
            break;
        }
        case Phase::WaitJoined: {
            Poco::JSON::Object::Ptr const msg = pollCollabMessage(first_, "user_joined");
            if (!msg) {
                break;
            }
            Poco::JSON::Object::Ptr const user = msg->getObject("user");
            LOK_ASSERT_EQUAL(std::string("tuser"), user->getValue<std::string>("id"));
            secondConnection_ = user->getValue<std::string>("connection");
            LOK_ASSERT_MESSAGE(
                "The connections are " + firstConnection_ + " and " + secondConnection_,
                !secondConnection_.empty() && secondConnection_ != firstConnection_);
            TRANSITION_STATE(phase_, Phase::WaitLeft);
            second_->sendMessage(R"({"type":"bye"})");
            second_->asyncShutdown();
            break;
        }
        case Phase::WaitLeft: {
            Poco::JSON::Object::Ptr const msg = pollCollabMessage(first_, "user_left");
            if (!msg) {
                break;
            }
            LOK_ASSERT_EQUAL(
                secondConnection_,
                msg->getObject("user")->getValue<std::string>("connection"));
            TRANSITION_STATE(phase_, Phase::Done);
            first_->asyncShutdown();
            passTest("The connection that left was the second one");
            break;
        }
        case Phase::Done:
            break;
        }
    }

private:
    // The WOPISrc, followed by the test name for the request to coolwsd:
    std::string wopiSrc() const {
        return Uri::encode(getWopiHostURI() + "/wopi/files/2?testname=UnitCollabConnections")
            + "&testname=UnitCollabConnections";
    }

    STATE_ENUM(
        Phase, ConnectFirst, WaitFirstAuth, WaitSecondAuth, WaitJoined, WaitLeft, Done) phase_;

    std::shared_ptr<http::WebSocketSession> first_;
    std::shared_ptr<http::WebSocketSession> second_;
    std::string firstConnection_;
    std::string secondConnection_;
};

// A new access token of the same user replaces the one of the connection for the fetches that
// follow, and one of another user does not:
class UnitCollabTokenRenewal: public WopiTestServer {
public:
    UnitCollabTokenRenewal(): WopiTestServer("UnitCollabTokenRenewal"), phase_(Phase::Connect) {}

    void configure(Poco::Util::LayeredConfiguration & config) override {
        WopiTestServer::configure(config);
        config.setBool("client_document_loading.enable", true);
    }

    void configCheckFileInfo(
        Poco::Net::HTTPRequest const & request, Poco::JSON::Object::Ptr & fileInfo) override
    {
        if (accessToken(request) == "other") {
            fileInfo->set("UserId", "someone-else");
        }
    }

    std::unique_ptr<http::Response>
    assertGetFileRequest(Poco::Net::HTTPRequest const & request) override
    {
        getFileToken_ = accessToken(request);
        return nullptr;
    }

    void invokeWSDTest() override {
        switch (phase_) {
        case Phase::Connect:
            TRANSITION_STATE(phase_, Phase::WaitAuth);
            // The WOPISrc, followed by the test name for the request to coolwsd:
            collabWs_ = startCollabConnection(
                socketPoll(),
                Uri::encode(getWopiHostURI() + "/wopi/files/0?testname=UnitCollabTokenRenewal")
                    + "&testname=UnitCollabTokenRenewal",
                "first");
            break;
        case Phase::WaitAuth:
            if (!pollCollabMessage(collabWs_, "user_list")) {
                break;
            }
            TRANSITION_STATE(phase_, Phase::WaitOtherRefused);
            collabWs_->sendMessage(R"({"type":"access_token","token":"other"})");
            break;
        case Phase::WaitOtherRefused: {
            Poco::JSON::Object::Ptr const msg = pollCollabMessage(collabWs_, "access_token");
            if (!msg) {
                break;
            }
            LOK_ASSERT_EQUAL(false, msg->getValue<bool>("accepted"));
            TRANSITION_STATE(phase_, Phase::WaitRenewedAccepted);
            // The token "re=newed", URI-encoded as the page sends it:
            collabWs_->sendMessage(R"({"type":"access_token","token":"re%3Dnewed"})");
            break;
        }
        case Phase::WaitRenewedAccepted: {
            Poco::JSON::Object::Ptr const msg = pollCollabMessage(collabWs_, "access_token");
            if (!msg) {
                break;
            }
            LOK_ASSERT_EQUAL(true, msg->getValue<bool>("accepted"));
            TRANSITION_STATE(phase_, Phase::WaitFetchUrl);
            collabWs_->sendMessage(R"({"type":"fetch","stream":"contents","requestId":"f1"})");
            break;
        }
        case Phase::WaitFetchUrl: {
            Poco::JSON::Object::Ptr const msg = pollCollabMessage(collabWs_, "fetch_url");
            if (!msg) {
                break;
            }
            std::shared_ptr<http::Response const> const response = http::get(
                helpers::getTestServerURI(),
                msg->getValue<std::string>("url") + "&testname=UnitCollabTokenRenewal");
            LOK_ASSERT_EQUAL(http::StatusCode::OK, response->statusLine().statusCode());
            LOK_ASSERT_EQUAL(std::string("re=newed"), getFileToken_);
            TRANSITION_STATE(phase_, Phase::Done);
            collabWs_->asyncShutdown();
            passTest("The fetch went with the renewed token");
            break;
        }
        case Phase::Done:
            break;
        }
    }

private:
    static std::string accessToken(Poco::Net::HTTPRequest const & request) {
        for (auto const & [name, value]: Poco::URI(request.getURI()).getQueryParameters()) {
            if (name == "access_token") {
                return value;
            }
        }
        return "";
    }

    STATE_ENUM(
        Phase, Connect, WaitAuth, WaitOtherRefused, WaitRenewedAccepted, WaitFetchUrl, Done)
        phase_;

    std::shared_ptr<http::WebSocketSession> collabWs_;
    std::string getFileToken_;
};

// The fetch address answers only GET, and the upload address only POST, and a request with
// another method leaves the one-shot token for the right one:
class UnitCollabMethods: public WopiTestServer {
public:
    UnitCollabMethods(): WopiTestServer("UnitCollabMethods"), phase_(Phase::Connect) {}

    void configure(Poco::Util::LayeredConfiguration & config) override {
        WopiTestServer::configure(config);
        config.setBool("client_document_loading.enable", true);
    }

    void invokeWSDTest() override {
        switch (phase_) {
        case Phase::Connect:
            TRANSITION_STATE(phase_, Phase::WaitAuth);
            // The WOPISrc, followed by the test name for the request to coolwsd:
            collabWs_ = startCollabConnection(
                socketPoll(),
                Uri::encode(getWopiHostURI() + "/wopi/files/0?testname=UnitCollabMethods")
                    + "&testname=UnitCollabMethods",
                "anything");
            break;
        case Phase::WaitAuth:
            if (!pollCollabMessage(collabWs_, "user_list")) {
                break;
            }
            TRANSITION_STATE(phase_, Phase::WaitFetchUrl);
            collabWs_->sendMessage(R"({"type":"fetch","stream":"contents","requestId":"f1"})");
            break;
        case Phase::WaitFetchUrl: {
            Poco::JSON::Object::Ptr const msg = pollCollabMessage(collabWs_, "fetch_url");
            if (!msg) {
                break;
            }
            std::string const url
                = msg->getValue<std::string>("url") + "&testname=UnitCollabMethods";
            http::Request post(url, http::Request::VERB_POST);
            post.setBody("Not a download", "application/octet-stream");
            std::shared_ptr<http::Response const> const refused
                = http::Session::create(helpers::getTestServerURI())->syncRequest(post);
            LOK_ASSERT_EQUAL(
                http::StatusCode::MethodNotAllowed, refused->statusLine().statusCode());
            std::shared_ptr<http::Response const> const response
                = http::get(helpers::getTestServerURI(), url);
            LOK_ASSERT_EQUAL(http::StatusCode::OK, response->statusLine().statusCode());
            LOK_ASSERT_EQUAL(getFileContent(), response->getBody());
            TRANSITION_STATE(phase_, Phase::WaitUploadUrl);
            collabWs_->sendMessage(
                R"({"type":"upload","stream":"contents","requestId":"u1","force":true})");
            break;
        }
        case Phase::WaitUploadUrl: {
            Poco::JSON::Object::Ptr const msg = pollCollabMessage(collabWs_, "upload_url");
            if (!msg) {
                break;
            }
            std::string const url
                = msg->getValue<std::string>("url") + "&testname=UnitCollabMethods";
            std::shared_ptr<http::Response const> const refused
                = http::get(helpers::getTestServerURI(), url);
            LOK_ASSERT_EQUAL(
                http::StatusCode::MethodNotAllowed, refused->statusLine().statusCode());
            LOK_ASSERT_EQUAL(std::string("Hello, world"), getFileContent());
            http::Request post(url, http::Request::VERB_POST);
            post.setBody("Uploaded by the collab client", "application/octet-stream");
            std::shared_ptr<http::Response const> const response
                = http::Session::create(helpers::getTestServerURI())->syncRequest(post);
            LOK_ASSERT_EQUAL(http::StatusCode::OK, response->statusLine().statusCode());
            LOK_ASSERT_EQUAL(std::string("Uploaded by the collab client"), getFileContent());
            TRANSITION_STATE(phase_, Phase::Done);
            collabWs_->asyncShutdown();
            passTest("Each address took only its own method");
            break;
        }
        case Phase::Done:
            break;
        }
    }

private:
    STATE_ENUM(Phase, Connect, WaitAuth, WaitFetchUrl, WaitUploadUrl, Done) phase_;

    std::shared_ptr<http::WebSocketSession> collabWs_;
};

// The fetch and upload addresses carry the WOPISrc of the document, without its query:
class UnitCollabRouting: public WopiTestServer {
public:
    UnitCollabRouting(): WopiTestServer("UnitCollabRouting"), phase_(Phase::Connect) {}

    void configure(Poco::Util::LayeredConfiguration & config) override {
        WopiTestServer::configure(config);
        config.setBool("client_document_loading.enable", true);
    }

    void invokeWSDTest() override {
        switch (phase_) {
        case Phase::Connect:
            TRANSITION_STATE(phase_, Phase::WaitAuth);
            collabWs_ = startCollabConnection(
                socketPoll(),
                Uri::encode(
                    getWopiHostURI()
                    + "/wopi/files/0?access_token=anything&testname=UnitCollabRouting"),
                "anything");
            break;
        case Phase::WaitAuth:
            if (!pollCollabMessage(collabWs_, "user_list")) {
                break;
            }
            TRANSITION_STATE(phase_, Phase::WaitFetchUrl);
            collabWs_->sendMessage(R"({"type":"fetch","stream":"contents","requestId":"f1"})");
            break;
        case Phase::WaitFetchUrl: {
            Poco::JSON::Object::Ptr const msg = pollCollabMessage(collabWs_, "fetch_url");
            if (!msg) {
                break;
            }
            std::string const url = msg->getValue<std::string>("url");
            LOK_ASSERT_MESSAGE(
                "The fetch address is " + url,
                url.starts_with("/cool/collab/fetch?")
                    && url.ends_with(
                        "&WOPISrc=" + Uri::encode(getWopiHostURI() + "/wopi/files/0")));
            TRANSITION_STATE(phase_, Phase::WaitUploadUrl);
            collabWs_->sendMessage(
                R"({"type":"upload","stream":"contents","requestId":"u1","force":true})");
            break;
        }
        case Phase::WaitUploadUrl: {
            Poco::JSON::Object::Ptr const msg = pollCollabMessage(collabWs_, "upload_url");
            if (!msg) {
                break;
            }
            std::string const url = msg->getValue<std::string>("url");
            LOK_ASSERT_MESSAGE(
                "The upload address is " + url,
                url.starts_with("/cool/collab/put?")
                    && url.ends_with(
                        "&WOPISrc=" + Uri::encode(getWopiHostURI() + "/wopi/files/0")));
            TRANSITION_STATE(phase_, Phase::Done);
            collabWs_->asyncShutdown();
            passTest("The fetch and upload addresses carry the WOPISrc of the document");
            break;
        }
        case Phase::Done:
            break;
        }
    }

private:
    STATE_ENUM(Phase, Connect, WaitAuth, WaitFetchUrl, WaitUploadUrl, Done) phase_;

    std::shared_ptr<http::WebSocketSession> collabWs_;
};

// A fetch of the document from a FileUrl that the host redirects delivers the document:
class UnitCollabFetchRedirect: public WopiTestServer {
public:
    UnitCollabFetchRedirect():
        WopiTestServer("UnitCollabFetchRedirect"), phase_(Phase::Connect) {}

    void configure(Poco::Util::LayeredConfiguration & config) override {
        WopiTestServer::configure(config);
        config.setBool("client_document_loading.enable", true);
    }

    void configCheckFileInfo(
        Poco::Net::HTTPRequest const &, Poco::JSON::Object::Ptr & fileInfo) override
    {
        fileInfo->set(
            "FileUrl", getWopiHostURI() + "/redirected-file?testname=UnitCollabFetchRedirect");
    }

    bool handleHttpRequest(
        Poco::Net::HTTPRequest const & request, std::istream & message,
        std::shared_ptr<StreamSocket> const & socket) override
    {
        if (request.getMethod() == "GET"
            && Poco::URI(request.getURI()).getPath() == "/redirected-file")
        {
            http::Response response(http::StatusCode::Found);
            response.set(
                "Location",
                getWopiHostURI() + "/wopi/files/0/contents?testname=UnitCollabFetchRedirect");
            socket->sendAndShutdown(response);
            return true;
        }
        return WopiTestServer::handleHttpRequest(request, message, socket);
    }

    void invokeWSDTest() override {
        switch (phase_) {
        case Phase::Connect:
            TRANSITION_STATE(phase_, Phase::WaitAuth);
            // The WOPISrc, followed by the test name for the request to coolwsd:
            collabWs_ = startCollabConnection(
                socketPoll(),
                Uri::encode(
                    getWopiHostURI() + "/wopi/files/0?testname=UnitCollabFetchRedirect")
                    + "&testname=UnitCollabFetchRedirect",
                "anything");
            break;
        case Phase::WaitAuth:
            if (!pollCollabMessage(collabWs_, "user_list")) {
                break;
            }
            TRANSITION_STATE(phase_, Phase::WaitFetchUrl);
            collabWs_->sendMessage(R"({"type":"fetch","stream":"contents","requestId":"f1"})");
            break;
        case Phase::WaitFetchUrl: {
            Poco::JSON::Object::Ptr const msg = pollCollabMessage(collabWs_, "fetch_url");
            if (!msg) {
                break;
            }
            std::shared_ptr<http::Response const> const response = http::get(
                helpers::getTestServerURI(),
                msg->getValue<std::string>("url") + "&testname=UnitCollabFetchRedirect");
            LOK_ASSERT_EQUAL(http::StatusCode::OK, response->statusLine().statusCode());
            LOK_ASSERT_EQUAL(getFileContent(), response->getBody());
            TRANSITION_STATE(phase_, Phase::Done);
            collabWs_->asyncShutdown();
            passTest("The redirected document arrived");
            break;
        }
        case Phase::Done:
            break;
        }
    }

private:
    STATE_ENUM(Phase, Connect, WaitAuth, WaitFetchUrl, Done) phase_;

    std::shared_ptr<http::WebSocketSession> collabWs_;
};

// A fetch of the document that the host answers with a temporary error once delivers the document:
class UnitCollabFetchRetry: public WopiTestServer {
public:
    UnitCollabFetchRetry():
        WopiTestServer("UnitCollabFetchRetry"), phase_(Phase::Connect), getFiles_(0) {}

    void configure(Poco::Util::LayeredConfiguration & config) override {
        WopiTestServer::configure(config);
        config.setBool("client_document_loading.enable", true);
    }

    std::unique_ptr<http::Response>
    assertGetFileRequest(Poco::Net::HTTPRequest const &) override
    {
        ++getFiles_;
        if (getFiles_ > 1) {
            return nullptr;
        }
        // A body keeps the answer from reaching coolwsd as a dropped connection:
        auto response = std::make_unique<http::Response>(http::StatusCode::ServiceUnavailable);
        response->setBody("busy", "text/plain");
        return response;
    }

    void invokeWSDTest() override {
        switch (phase_) {
        case Phase::Connect:
            TRANSITION_STATE(phase_, Phase::WaitAuth);
            // The WOPISrc, followed by the test name for the request to coolwsd:
            collabWs_ = startCollabConnection(
                socketPoll(),
                Uri::encode(getWopiHostURI() + "/wopi/files/0?testname=UnitCollabFetchRetry")
                    + "&testname=UnitCollabFetchRetry",
                "anything");
            break;
        case Phase::WaitAuth:
            if (!pollCollabMessage(collabWs_, "user_list")) {
                break;
            }
            TRANSITION_STATE(phase_, Phase::WaitFetchUrl);
            collabWs_->sendMessage(R"({"type":"fetch","stream":"contents","requestId":"f1"})");
            break;
        case Phase::WaitFetchUrl: {
            Poco::JSON::Object::Ptr const msg = pollCollabMessage(collabWs_, "fetch_url");
            if (!msg) {
                break;
            }
            std::shared_ptr<http::Response const> const response = http::get(
                helpers::getTestServerURI(),
                msg->getValue<std::string>("url") + "&testname=UnitCollabFetchRetry");
            LOK_ASSERT_EQUAL(http::StatusCode::OK, response->statusLine().statusCode());
            LOK_ASSERT_EQUAL(getFileContent(), response->getBody());
            LOK_ASSERT_EQUAL(2, getFiles_);
            TRANSITION_STATE(phase_, Phase::Done);
            collabWs_->asyncShutdown();
            passTest("The document arrived on the second try");
            break;
        }
        case Phase::Done:
            break;
        }
    }

private:
    STATE_ENUM(Phase, Connect, WaitAuth, WaitFetchUrl, Done) phase_;

    // The GetFile requests that the host got:
    int getFiles_;

    std::shared_ptr<http::WebSocketSession> collabWs_;
};

// A fetch of the document that the host always answers with a temporary error answers 502 after
// three tries:
class UnitCollabFetchRetryLimit: public WopiTestServer {
public:
    UnitCollabFetchRetryLimit():
        WopiTestServer("UnitCollabFetchRetryLimit"), phase_(Phase::Connect), getFiles_(0) {}

    void configure(Poco::Util::LayeredConfiguration & config) override {
        WopiTestServer::configure(config);
        config.setBool("client_document_loading.enable", true);
    }

    std::unique_ptr<http::Response>
    assertGetFileRequest(Poco::Net::HTTPRequest const &) override
    {
        ++getFiles_;
        // A body keeps the answer from reaching coolwsd as a dropped connection:
        auto response = std::make_unique<http::Response>(http::StatusCode::ServiceUnavailable);
        response->setBody("busy", "text/plain");
        return response;
    }

    void invokeWSDTest() override {
        switch (phase_) {
        case Phase::Connect:
            TRANSITION_STATE(phase_, Phase::WaitAuth);
            // The WOPISrc, followed by the test name for the request to coolwsd:
            collabWs_ = startCollabConnection(
                socketPoll(),
                Uri::encode(getWopiHostURI() + "/wopi/files/0?testname=UnitCollabFetchRetryLimit")
                    + "&testname=UnitCollabFetchRetryLimit",
                "anything");
            break;
        case Phase::WaitAuth:
            if (!pollCollabMessage(collabWs_, "user_list")) {
                break;
            }
            TRANSITION_STATE(phase_, Phase::WaitFetchUrl);
            collabWs_->sendMessage(R"({"type":"fetch","stream":"contents","requestId":"f1"})");
            break;
        case Phase::WaitFetchUrl: {
            Poco::JSON::Object::Ptr const msg = pollCollabMessage(collabWs_, "fetch_url");
            if (!msg) {
                break;
            }
            std::shared_ptr<http::Response const> const response = http::get(
                helpers::getTestServerURI(),
                msg->getValue<std::string>("url") + "&testname=UnitCollabFetchRetryLimit");
            LOK_ASSERT_EQUAL(http::StatusCode::BadGateway, response->statusLine().statusCode());
            LOK_ASSERT_EQUAL(3, getFiles_);
            TRANSITION_STATE(phase_, Phase::Done);
            collabWs_->asyncShutdown();
            passTest("The fetch gave up after three tries");
            break;
        }
        case Phase::Done:
            break;
        }
    }

private:
    STATE_ENUM(Phase, Connect, WaitAuth, WaitFetchUrl, Done) phase_;

    // The GetFile requests that the host got:
    int getFiles_;

    std::shared_ptr<http::WebSocketSession> collabWs_;
};

// A fetch of the document from a FileUrl whose host cannot be reached answers 502:
class UnitCollabFetchNoConnection: public WopiTestServer {
public:
    UnitCollabFetchNoConnection():
        WopiTestServer("UnitCollabFetchNoConnection"), phase_(Phase::Connect) {}

    void configure(Poco::Util::LayeredConfiguration & config) override {
        WopiTestServer::configure(config);
        config.setBool("client_document_loading.enable", true);
    }

    void configCheckFileInfo(
        Poco::Net::HTTPRequest const &, Poco::JSON::Object::Ptr & fileInfo) override
    {
        fileInfo->set("FileUrl", "http://nonexistent.invalid/file");
    }

    void invokeWSDTest() override {
        switch (phase_) {
        case Phase::Connect:
            TRANSITION_STATE(phase_, Phase::WaitAuth);
            // The WOPISrc, followed by the test name for the request to coolwsd:
            collabWs_ = startCollabConnection(
                socketPoll(),
                Uri::encode(
                    getWopiHostURI() + "/wopi/files/0?testname=UnitCollabFetchNoConnection")
                    + "&testname=UnitCollabFetchNoConnection",
                "anything");
            break;
        case Phase::WaitAuth:
            if (!pollCollabMessage(collabWs_, "user_list")) {
                break;
            }
            TRANSITION_STATE(phase_, Phase::WaitFetchUrl);
            collabWs_->sendMessage(R"({"type":"fetch","stream":"contents","requestId":"f1"})");
            break;
        case Phase::WaitFetchUrl: {
            Poco::JSON::Object::Ptr const msg = pollCollabMessage(collabWs_, "fetch_url");
            if (!msg) {
                break;
            }
            std::shared_ptr<http::Response const> const response = http::get(
                helpers::getTestServerURI(),
                msg->getValue<std::string>("url") + "&testname=UnitCollabFetchNoConnection");
            LOK_ASSERT_EQUAL(http::StatusCode::BadGateway, response->statusLine().statusCode());
            TRANSITION_STATE(phase_, Phase::Done);
            collabWs_->asyncShutdown();
            passTest("The fetch from an unreachable host was answered with 502");
            break;
        }
        case Phase::Done:
            break;
        }
    }

private:
    STATE_ENUM(Phase, Connect, WaitAuth, WaitFetchUrl, Done) phase_;

    std::shared_ptr<http::WebSocketSession> collabWs_;
};

// The server speaks its own protocol with a newer collab client, and ignores a query parameter
// and a message field that it does not know:
class UnitCollabProtocolNumber: public WopiTestServer {
public:
    UnitCollabProtocolNumber():
        WopiTestServer("UnitCollabProtocolNumber"), phase_(Phase::Connect) {}

    void configure(Poco::Util::LayeredConfiguration & config) override {
        WopiTestServer::configure(config);
        config.setBool("client_document_loading.enable", true);
    }

    void invokeWSDTest() override {
        switch (phase_) {
        case Phase::Connect: {
            TRANSITION_STATE(phase_, Phase::WaitAuth);
            Poco::URI const wopiURL(
                getWopiHostURI()
                + "/wopi/files/0?access_token=anything"
                  "&testname=UnitCollabProtocolNumber");
            ws_ = http::WebSocketSession::create(helpers::getTestServerURI());
            http::Request request(
                "/cool/collab/ws?WOPISrc=" + Uri::encode(wopiURL.toString()) + "&protocol="
                + std::to_string(CollabSocketHandler::Protocol + 1) + "&unknown=x");
            LOK_ASSERT(ws_->asyncRequest(request, socketPoll()));
            ws_->sendMessage("access_token anything");
            break;
        }
        case Phase::WaitAuth: {
            Poco::JSON::Object::Ptr authenticated;
            ws_->poll(
                [&](std::vector<char> const & message) {
                    Poco::JSON::Object::Ptr object;
                    if (JsonUtil::parseJSON(std::string(message.data(), message.size()), object)
                        && object->optValue<std::string>("type", "") == "authenticated")
                    {
                        authenticated = object;
                        return true;
                    }
                    return false;
                },
                std::chrono::milliseconds(1), "collab");
            if (!authenticated) {
                break;
            }
            LOK_ASSERT_EQUAL(
                CollabSocketHandler::Protocol, authenticated->getValue<int>("protocol"));
            TRANSITION_STATE(phase_, Phase::WaitFetchWithUnknownField);
            ws_->sendMessage(
                R"({"type":"fetch","stream":"contents","requestId":"f1","unknown":true})");
            break;
        }
        case Phase::WaitFetchWithUnknownField: {
            Poco::JSON::Object::Ptr const msg = pollCollabMessage(ws_, "fetch_url");
            if (!msg) {
                break;
            }
            LOK_ASSERT_EQUAL(std::string("f1"), msg->getValue<std::string>("requestId"));
            TRANSITION_STATE(phase_, Phase::Done);
            ws_->asyncShutdown();
            passTest("The server spoke its own protocol with a newer collab client");
            break;
        }
        case Phase::Done:
            break;
        }
    }

private:
    STATE_ENUM(Phase, Connect, WaitAuth, WaitFetchWithUnknownField, Done) phase_;

    std::shared_ptr<http::WebSocketSession> ws_;
};

// A server with client document loading keeps the paths of its editor page and the directives of
// that page's content security policy that "The editor frame" in wsd/protocol.txt lists:
class UnitCollabEditorFrame: public WopiTestServer {
public:
    UnitCollabEditorFrame(): WopiTestServer("UnitCollabEditorFrame") {}

    void configure(Poco::Util::LayeredConfiguration & config) override {
        WopiTestServer::configure(config);
        config.setBool("client_document_loading.enable", true);
    }

    void invokeWSDTest() override {
        if (done_) {
            return;
        }
        done_ = true;

        // The editor page of the discovery is at <service-root>/browser/.../cool.html:
        std::shared_ptr<http::Response const> const discovery
            = http::get(
                helpers::getTestServerURI(), "/hosting/discovery?testname=UnitCollabEditorFrame");
        LOK_ASSERT_EQUAL(http::StatusCode::OK, discovery->statusLine().statusCode());
        Poco::XML::DOMParser parser;
        Poco::AutoPtr<Poco::XML::Document> const xml = parser.parseString(discovery->getBody());
        Poco::AutoPtr<Poco::XML::NodeList> const actions = xml->getElementsByTagName("action");
        std::string editorUrl;
        for (unsigned long i = 0; i != actions->length(); ++i) {
            auto const action = static_cast<Poco::XML::Element *>(actions->item(i));
            if (action->getAttribute("name") == "edit") {
                editorUrl = action->getAttribute("urlsrc");
                break;
            }
        }
        std::string const editorPath = Poco::URI(editorUrl).getPath();
        LOK_ASSERT_MESSAGE(
            "The editor page is at " + editorPath, editorPath.ends_with("/cool.html"));
        std::size_t const browser = editorPath.find("/browser/");
        LOK_ASSERT_MESSAGE("The editor page is at " + editorPath, browser != std::string::npos);
        std::string const serviceRoot = editorPath.substr(0, browser);

        // The capabilities are at <service-root>/hosting/capabilities:
        std::shared_ptr<http::Response const> const capabilities
            = http::get(
                helpers::getTestServerURI(),
                serviceRoot + "/hosting/capabilities?testname=UnitCollabEditorFrame");
        LOK_ASSERT_EQUAL(http::StatusCode::OK, capabilities->statusLine().statusCode());
        LOK_ASSERT_EQUAL(
            true,
            Poco::JSON::Parser()
                .parse(capabilities->getBody())
                .extract<Poco::JSON::Object::Ptr>()
                ->getValue<bool>("hasClientDocumentLoadingSupport"));

        // The editor page, posted to as an integrator does, answers with the directives:
        http::Request request(
            editorPath + "?WOPISrc="
                + Uri::encode(getWopiHostURI() + "/wopi/files/0?testname=UnitCollabEditorFrame")
                + "&testname=UnitCollabEditorFrame",
            http::Request::VERB_POST);
        request.setBody(
            "access_token=anything&access_token_ttl=0", "application/x-www-form-urlencoded");
        std::shared_ptr<http::Response const> const editor
            = http::Session::create(helpers::getTestServerURI())->syncRequest(request);
        LOK_ASSERT_EQUAL(http::StatusCode::OK, editor->statusLine().statusCode());
        ContentSecurityPolicy const csp(
            editor->header().get("Content-Security-Policy", std::string()));
        std::string const connect = csp.getDirective("connect-src");
        LOK_ASSERT_MESSAGE("connect-src is" + connect, connect.find("'self'") != std::string::npos);
        std::string const script = csp.getDirective("script-src");
        LOK_ASSERT_MESSAGE("script-src is" + script, script.find("'self'") != std::string::npos);
        std::string const style = csp.getDirective("style-src");
        LOK_ASSERT_MESSAGE("style-src is" + style, style.find("'self'") != std::string::npos);
        std::string const font = csp.getDirective("font-src");
        LOK_ASSERT_MESSAGE("font-src is" + font, font.find("'self'") != std::string::npos);
        std::string const image = csp.getDirective("img-src");
        LOK_ASSERT_MESSAGE("img-src is" + image, image.find("'self'") != std::string::npos);
        LOK_ASSERT_MESSAGE("img-src is" + image, image.find("data:") != std::string::npos);
        std::string const form = csp.getDirective("form-action");
        LOK_ASSERT_MESSAGE(
            "form-action is" + form, form.empty() || form.find("'self'") != std::string::npos);

        passTest("The editor page keeps what an editor frame depends on");
    }

private:
    bool done_ = false;
};

/// A collab client gets the shared and the user settings list of the host. The host gets the
/// request for each list with the uri that CheckFileInfo gave: the shared list has an access
/// token of its own, and the user list has none.
class UnitCollabSettingsStreams : public WopiTestServer
{
public:
    UnitCollabSettingsStreams()
        : WopiTestServer("UnitCollabSettingsStreams")
        , _phase(Phase::ConnectCollab)
    {
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& /*request*/,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        Poco::JSON::Object::Ptr sharedSettings = new Poco::JSON::Object();
        sharedSettings->set("uri", settingsUri("sharedconfig.json", "access_token=settingstoken&"));
        sharedSettings->set("stamp", "sharedstamp");
        fileInfo->set("SharedSettings", sharedSettings);

        Poco::JSON::Object::Ptr userSettings = new Poco::JSON::Object();
        userSettings->set("uri", settingsUri("userconfig.json", std::string()));
        userSettings->set("stamp", "userstamp");
        fileInfo->set("UserSettings", userSettings);
    }

    bool handleHttpGetRequest(const Poco::Net::HTTPRequest& request,
                              const std::shared_ptr<StreamSocket>& socket) override
    {
        const Poco::URI uriReq(request.getURI());
        const std::string path = uriReq.getPath();
        const bool shared = path == "/wopi/settings/sharedconfig.json";
        if (!shared && path != "/wopi/settings/userconfig.json")
            return WopiTestServer::handleHttpGetRequest(request, socket);

        std::vector<std::string> tokens;
        for (const auto& param : uriReq.getQueryParameters())
        {
            if (param.first == "access_token")
                tokens.push_back(param.second);
        }
        if (shared)
        {
            LOK_ASSERT_EQUAL(std::size_t(1), tokens.size());
            LOK_ASSERT_EQUAL(std::string("settingstoken"), tokens[0]);
        }
        else
        {
            LOK_ASSERT_EQUAL(std::size_t(0), tokens.size());
            LOK_ASSERT(!request.has("Authorization"));
        }

        http::Response httpResponse(http::StatusCode::OK);
        httpResponse.setBody(shared ? SharedList : UserList, "application/json; charset=utf-8");
        socket->sendAndShutdown(httpResponse);
        return true;
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::ConnectCollab:
            {
                TRANSITION_STATE(_phase, Phase::WaitCollabAuth);
                const Poco::URI wopiURL(getWopiHostURI() +
                                        "/wopi/files/0?access_token=anything"
                                        "&testname=UnitCollabSettingsStreams");
                _collabWs = startCollabConnection(socketPoll(), Uri::encode(wopiURL.toString()),
                                                  "anything");
                break;
            }
            case Phase::WaitCollabAuth:
            {
                if (!pollCollabMessage(_collabWs, "user_list"))
                    break;
                TRANSITION_STATE(_phase, Phase::WaitSharedSettings);
                _collabWs->sendMessage(
                    R"({"type":"fetch","stream":"sharedSettings","requestId":"s1"})");
                break;
            }
            case Phase::WaitSharedSettings:
            {
                const Poco::JSON::Object::Ptr msg = pollCollabMessage(_collabWs, "fetch_url");
                if (!msg)
                    break;
                LOK_ASSERT_EQUAL(std::string("s1"), msg->getValue<std::string>("requestId"));
                LOK_ASSERT_EQUAL(SharedList, download(msg->getValue<std::string>("url")));
                TRANSITION_STATE(_phase, Phase::WaitUserSettings);
                _collabWs->sendMessage(
                    R"({"type":"fetch","stream":"userSettings","requestId":"u1"})");
                break;
            }
            case Phase::WaitUserSettings:
            {
                const Poco::JSON::Object::Ptr msg = pollCollabMessage(_collabWs, "fetch_url");
                if (!msg)
                    break;
                LOK_ASSERT_EQUAL(std::string("u1"), msg->getValue<std::string>("requestId"));
                LOK_ASSERT_EQUAL(UserList, download(msg->getValue<std::string>("url")));
                TRANSITION_STATE(_phase, Phase::Done);
                _collabWs->asyncShutdown();
                passTest("The collab client got both settings lists");
                break;
            }
            case Phase::Done:
                break;
        }
    }

private:
    static inline const std::string SharedList = R"({"kind":"shared","template":[]})";
    static inline const std::string UserList = R"({"kind":"user","template":[]})";

    /// The uri of a settings list. Its query starts with token, which is empty or one parameter
    /// with a trailing ampersand.
    static std::string settingsUri(const std::string& name, const std::string& token)
    {
        return helpers::getTestServerURI() + "/wopi/settings/" + name + '?' + token +
               "testname=UnitCollabSettingsStreams";
    }

    /// The body that the one-time URL of a fetch_url answer gives.
    std::string download(const std::string& url)
    {
        http::Request request(url);
        const std::shared_ptr<const http::Response> response =
            http::Session::create(helpers::getTestServerURI())->syncRequest(request);
        LOK_ASSERT_EQUAL(http::StatusCode::OK, response->statusLine().statusCode());
        return response->getBody();
    }

    STATE_ENUM(Phase, ConnectCollab, WaitCollabAuth, WaitSharedSettings, WaitUserSettings, Done)
    _phase;

    std::shared_ptr<http::WebSocketSession> _collabWs;
};

/// A collab client gets a file that a settings list of the host names, and no other file. The
/// client names the list, the group and the entry.
class UnitCollabSettingsFile : public WopiTestServer
{
public:
    UnitCollabSettingsFile()
        : WopiTestServer("UnitCollabSettingsFile")
        , _phase(Phase::ConnectCollab)
        , _step(0)
        , _sent(0)
    {
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& /*request*/,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        Poco::JSON::Object::Ptr sharedSettings = new Poco::JSON::Object();
        sharedSettings->set("uri", settingsUri("sharedconfig.json"));
        sharedSettings->set("stamp", "sharedstamp");
        fileInfo->set("SharedSettings", sharedSettings);

        Poco::JSON::Object::Ptr userSettings = new Poco::JSON::Object();
        userSettings->set("uri", settingsUri("userconfig.json"));
        userSettings->set("stamp", "userstamp");
        fileInfo->set("UserSettings", userSettings);
    }

    bool handleHttpGetRequest(const Poco::Net::HTTPRequest& request,
                              const std::shared_ptr<StreamSocket>& socket) override
    {
        const std::string path = Poco::URI(request.getURI()).getPath();
        if (!path.starts_with("/wopi/settings/"))
            return WopiTestServer::handleHttpGetRequest(request, socket);

        // The settings uris have no access token, and the host gets them as they are.
        for (const auto& param : Poco::URI(request.getURI()).getQueryParameters())
            LOK_ASSERT(param.first != "access_token");
        LOK_ASSERT(!request.has("Authorization"));

        const int count = countRequest(path);

        const std::string json = "application/json; charset=utf-8";
        const std::string binary = "application/octet-stream";
        std::string body;
        std::string contentType = binary;
        if (path == "/wopi/settings/sharedconfig.json")
        {
            body = R"({"template":[{"uri":")" + settingsUri("shared/a.otp") +
                   R"(","stamp":"1"},{"uri":")" + settingsUri("shared/viewsetting.json") +
                   R"(","stamp":"1"}],"xcu":[{"uri":")" + settingsUri("shared/c.xcu") +
                   R"(","stamp":"1"}]})";
            contentType = json;
        }
        else if (path == "/wopi/settings/userconfig.json" && count > 1)
        {
            body = R"({"template":[{"uri":")" + settingsUri("user/b.otp") + R"(","stamp":"1"}]})";
            contentType = json;
        }
        else if (path == "/wopi/settings/shared/a.otp")
            body = "shared template bytes";
        else if (path == "/wopi/settings/shared/viewsetting.json")
            body = R"({"zoteroAPIKey":"stored value","darkTheme":"true"})";
        else if (path == "/wopi/settings/shared/c.xcu")
            body = "shared xcu bytes";
        else if (path == "/wopi/settings/user/b.otp")
            body = "user template bytes";

        if (body.empty())
        {
            // The first request for the user list fails, and the other paths do not exist.
            http::Response httpResponse(path == "/wopi/settings/userconfig.json"
                                            ? http::StatusCode::InternalServerError
                                            : http::StatusCode::NotFound);
            socket->sendAndShutdown(httpResponse);
            return true;
        }

        http::Response httpResponse(http::StatusCode::OK);
        httpResponse.setBody(body, contentType);
        socket->sendAndShutdown(httpResponse);
        return true;
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::ConnectCollab:
            {
                TRANSITION_STATE(_phase, Phase::WaitCollabAuth);
                const Poco::URI wopiURL(getWopiHostURI() +
                                        "/wopi/files/0?access_token=anything"
                                        "&testname=UnitCollabSettingsFile");
                _collabWs = startCollabConnection(socketPoll(), Uri::encode(wopiURL.toString()),
                                                  "anything");
                break;
            }
            case Phase::WaitCollabAuth:
            {
                if (!pollCollabMessage(_collabWs, "user_list"))
                    break;
                TRANSITION_STATE(_phase, Phase::Steps);
                // The first two requests leave together, so the second one arrives while the
                // shared list downloads.
                sendStep();
                sendStep();
                break;
            }
            case Phase::Steps:
            {
                // Matches fetch_url and fetch_error.
                const auto response = _collabWs->waitForMessage(
                    R"({"type":"fetch_)", std::chrono::milliseconds(1), "collab");
                if (response.empty())
                    break;
                Poco::JSON::Object::Ptr msg;
                LOK_ASSERT(
                    JsonUtil::parseJSON(std::string(response.data(), response.size()), msg));

                const Step& step = steps()[_step];
                TST_LOG("Step " << _step << " (" << step.list << ", " << step.file << ") got "
                                << std::string(response.data(), response.size()));
                LOK_ASSERT_EQUAL(std::string("r") + std::to_string(_step),
                                 msg->getValue<std::string>("requestId"));
                LOK_ASSERT_EQUAL(std::string("settingsFile"),
                                 msg->getValue<std::string>("stream"));
                if (step.error.empty())
                {
                    LOK_ASSERT_EQUAL(std::string("fetch_url"),
                                     msg->getValue<std::string>("type"));
                    http::Request request(msg->getValue<std::string>("url"));
                    const std::shared_ptr<const http::Response> httpResponse =
                        http::Session::create(helpers::getTestServerURI())->syncRequest(request);
                    LOK_ASSERT_EQUAL(http::StatusCode::OK,
                                     httpResponse->statusLine().statusCode());
                    if (step.file.ends_with(".json"))
                        assertSameJson(step.body, httpResponse->getBody());
                    else
                        LOK_ASSERT_EQUAL(step.body, httpResponse->getBody());
                }
                else
                {
                    LOK_ASSERT_EQUAL(std::string("fetch_error"),
                                     msg->getValue<std::string>("type"));
                    LOK_ASSERT_EQUAL(step.error, msg->getValue<std::string>("error"));
                }

                if (++_step < steps().size())
                {
                    if (_sent == _step)
                        sendStep();
                    break;
                }

                // The host got no request for a file that no list names, and coolwsd asked
                // for the shared list once.
                LOK_ASSERT_EQUAL(0, requestCount("/wopi/settings/shared/secret.otp"));
                LOK_ASSERT_EQUAL(0, requestCount("/wopi/settings/shared/c.xcu"));
                LOK_ASSERT_EQUAL(0, requestCount("/wopi/settings/shared/othertoken/a.otp"));
                LOK_ASSERT_EQUAL(2, requestCount("/wopi/settings/shared/a.otp"));
                LOK_ASSERT_EQUAL(1, requestCount("/wopi/settings/user/b.otp"));
                LOK_ASSERT_EQUAL(1, requestCount("/wopi/settings/sharedconfig.json"));
                LOK_ASSERT_EQUAL(2, requestCount("/wopi/settings/userconfig.json"));

                TRANSITION_STATE(_phase, Phase::Done);
                _collabWs->asyncShutdown();
                passTest("The collab client got the listed files and no other");
                break;
            }
            case Phase::Done:
                break;
        }
    }

private:
    /// One settingsFile fetch, and the answer it must get: the body, or the error.
    struct Step
    {
        std::string list;
        std::string group;
        std::string file;
        std::string error;
        std::string body;
    };

    static const std::vector<Step>& steps()
    {
        static const std::vector<Step> all = {
            { "shared", "template", "shared/a.otp", "", "shared template bytes" },
            // The same file under another path, as when each copy of a list carries its own token.
            { "shared", "template", "shared/othertoken/a.otp", "", "shared template bytes" },
            // In another group of the list.
            { "shared", "template", "shared/c.xcu", "Not in settings list", "" },
            // In no list.
            { "shared", "template", "shared/secret.otp", "Not in settings list", "" },
            { "other", "template", "shared/a.otp", "Unknown list", "" },
            // A group of the list that this stream does not give.
            { "shared", "xcu", "shared/c.xcu", "Unknown group", "" },
            // A settings file comes without the values that the server keeps for itself.
            { "shared", "template", "shared/viewsetting.json", "",
              R"({"zoteroAPIKey":"","zoteroAPIKeyStored":true,"darkTheme":"true"})" },
            // The host fails the first request for the user list, and answers the second.
            { "user", "template", "user/b.otp", "Settings list not available", "" },
            { "user", "template", "user/b.otp", "", "user template bytes" },
        };
        return all;
    }

    /// Asserts that the JSON object in actual has the members of the one in expected, with the
    /// same values, and no other member.
    void assertSameJson(const std::string& expected, const std::string& actual)
    {
        Poco::JSON::Object::Ptr expectedObject;
        Poco::JSON::Object::Ptr actualObject;
        LOK_ASSERT(JsonUtil::parseJSON(expected, expectedObject));
        LOK_ASSERT(JsonUtil::parseJSON(actual, actualObject));
        LOK_ASSERT_EQUAL(expectedObject->size(), actualObject->size());
        for (const std::string& name : expectedObject->getNames())
        {
            LOK_ASSERT(actualObject->has(name));
            LOK_ASSERT_EQUAL(expectedObject->get(name).toString(),
                             actualObject->get(name).toString());
        }
    }

    void sendStep()
    {
        const Step& step = steps()[_sent];
        Poco::JSON::Object::Ptr message = new Poco::JSON::Object();
        message->set("type", "fetch");
        message->set("stream", "settingsFile");
        message->set("list", step.list);
        message->set("group", step.group);
        message->set("uri", settingsUri(step.file));
        message->set("requestId", "r" + std::to_string(_sent));
        _collabWs->sendMessage(JsonUtil::jsonToString(message));
        ++_sent;
    }

    static std::string settingsUri(const std::string& name)
    {
        return helpers::getTestServerURI() + "/wopi/settings/" + name +
               "?testname=UnitCollabSettingsFile";
    }

    int countRequest(const std::string& path)
    {
        std::lock_guard<std::mutex> lock(_requestsMutex);
        return ++_requests[path];
    }

    int requestCount(const std::string& path)
    {
        std::lock_guard<std::mutex> lock(_requestsMutex);
        return _requests[path];
    }

    STATE_ENUM(Phase, ConnectCollab, WaitCollabAuth, Steps, Done) _phase;

    /// The index of the step that waits for its answer.
    std::size_t _step;
    /// How many steps went out.
    std::size_t _sent;
    std::shared_ptr<http::WebSocketSession> _collabWs;
    std::mutex _requestsMutex;
    /// How many requests the host got, by path.
    std::map<std::string, int> _requests;
};

/// A settings list on a host whose name does not resolve gives an error.
class UnitCollabSettingsListUnreachable : public WopiTestServer
{
public:
    UnitCollabSettingsListUnreachable()
        : WopiTestServer("UnitCollabSettingsListUnreachable")
        , _phase(Phase::ConnectCollab)
    {
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& /*request*/,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        Poco::JSON::Object::Ptr sharedSettings = new Poco::JSON::Object();
        sharedSettings->set("uri",
                            "http://settings-list-host.invalid/wopi/settings/sharedconfig.json");
        sharedSettings->set("stamp", "sharedstamp");
        fileInfo->set("SharedSettings", sharedSettings);
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::ConnectCollab:
            {
                TRANSITION_STATE(_phase, Phase::WaitCollabAuth);
                const Poco::URI wopiURL(getWopiHostURI() +
                                        "/wopi/files/0?access_token=anything"
                                        "&testname=UnitCollabSettingsListUnreachable");
                _collabWs = startCollabConnection(socketPoll(), Uri::encode(wopiURL.toString()),
                                                  "anything");
                break;
            }
            case Phase::WaitCollabAuth:
            {
                if (!pollCollabMessage(_collabWs, "user_list"))
                    break;
                TRANSITION_STATE(_phase, Phase::WaitAnswer);
                Poco::JSON::Object::Ptr message = new Poco::JSON::Object();
                message->set("type", "fetch");
                message->set("stream", "settingsFile");
                message->set("list", "shared");
                message->set("group", "template");
                message->set("uri", "http://settings-list-host.invalid/wopi/settings/shared/a.otp");
                message->set("requestId", "r0");
                _collabWs->sendMessage(JsonUtil::jsonToString(message));
                break;
            }
            case Phase::WaitAnswer:
            {
                const auto response = _collabWs->waitForMessage(
                    R"({"type":"fetch_)", std::chrono::milliseconds(1), "collab");
                if (response.empty())
                    break;
                Poco::JSON::Object::Ptr msg;
                LOK_ASSERT(
                    JsonUtil::parseJSON(std::string(response.data(), response.size()), msg));
                LOK_ASSERT_EQUAL(std::string("r0"), msg->getValue<std::string>("requestId"));
                LOK_ASSERT_EQUAL(std::string("fetch_error"), msg->getValue<std::string>("type"));
                LOK_ASSERT_EQUAL(std::string("Settings list not available"),
                                 msg->getValue<std::string>("error"));

                TRANSITION_STATE(_phase, Phase::Done);
                _collabWs->asyncShutdown();
                passTest("The request for the list that cannot be reached got an error");
                break;
            }
            case Phase::Done:
                break;
        }
    }

private:
    STATE_ENUM(Phase, ConnectCollab, WaitCollabAuth, WaitAnswer, Done) _phase;

    std::shared_ptr<http::WebSocketSession> _collabWs;
};

UnitBase** unit_create_wsd_multi(void)
{
    return new UnitBase* [21] { new UnitCollabProtocol(),
                                new UnitCollabCheckFileInfoTransient(),
                                new UnitCollabUploadTimestamp(), new UnitCollabUploadConflict(),
                                new UnitCollabUploadConflictModified(),
                                new UnitCollabNoCleanExport(), new UnitCollabSettingsStreams(),
                                new UnitCollabSettingsFile(),
                                new UnitCollabSettingsListUnreachable(),
                                new UnitCollabAvatar(),
                                new UnitCollabConnections(), new UnitCollabTokenRenewal(),
                                new UnitCollabMethods(), new UnitCollabRouting(),
                                new UnitCollabFetchRedirect(), new UnitCollabFetchRetry(),
                                new UnitCollabFetchRetryLimit(), new UnitCollabFetchNoConnection(),
                                new UnitCollabProtocolNumber(), new UnitCollabEditorFrame(),
                                nullptr };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
