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

#include <config.h>

#include <WopiTestServer.hpp>
#include <common/Log.hpp>
#include <helpers.hpp>

#include <Poco/JSON/Parser.h>
#include <Poco/Net/HTTPRequest.h>

#include <chrono>

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
    const std::string path = "/co/collab?WOPISrc=" + wopiSrc;
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

/// Tests the /co/collab WebSocket protocol:
/// 1. editing_started is broadcast to other connected users
/// 2. editingActive flag is set in user_list for late joiners
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
        // Test 3: switch_to_collab broadcast
        T3_Connect1, T3_WaitAuth1, T3_Connect2, T3_WaitAuth2,
        T3_SendSwitch, T3_WaitSwitch,
        Done,
    } _phase;

    std::size_t _checkFileInfoCount;
    std::shared_ptr<http::WebSocketSession> _ws1;
    std::shared_ptr<http::WebSocketSession> _ws2;
    std::string _wopiSrc;

public:
    UnitCollabProtocol()
        : WopiTestServer("UnitCollabProtocol")
        , _phase(Phase::T1_Connect1)
        , _checkFileInfoCount(0)
    {
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& /*request*/,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
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
                    helpers::getTestServerURI() +
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
                    helpers::getTestServerURI() +
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

            // -- Test 3: switch_to_collab broadcast --

            case Phase::T3_Connect1:
            {
                PHASE_TRANSITION(T3_WaitAuth1);

                const Poco::URI wopiURL(
                    helpers::getTestServerURI() +
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

                PHASE_TRANSITION(T3_SendSwitch);
                break;
            }
            case Phase::T3_SendSwitch:
            {
                PHASE_TRANSITION(T3_WaitSwitch);

                _ws2->sendMessage("{\"type\":\"switch_to_collab\"}");
                break;
            }
            case Phase::T3_WaitSwitch:
            {
                auto msg = pollCollabMessage(_ws1, "switch_to_collab");
                if (!msg)
                    break;

                LOK_ASSERT_MESSAGE("Should have user object",
                                   msg->has("user"));

                TST_LOG("Test 3 passed: switch_to_collab broadcast");

                _ws1->asyncShutdown();
                _ws2->asyncShutdown();
                _ws1.reset();
                _ws2.reset();

                PHASE_TRANSITION(Done);
                passTest("All collab protocol tests passed");
                break;
            }
            case Phase::Done:
                break;
        }
    }
};

UnitBase* unit_create_wsd(void)
{
    return new UnitCollabProtocol();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
