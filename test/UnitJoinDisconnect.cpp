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
 * Unit test for client join and disconnect scenarios.
 */

#include <config.h>

#include <lokassert.hpp>
#include <Unit.hpp>
#include <WopiTestServer.hpp>
#include <common/Log.hpp>
#include <helpers.hpp>
#include <wsd/ClientSession.hpp>

#include <Poco/Net/HTTPRequest.h>

#include <chrono>

/// This is to test that dropping connection is seen as leaving the document
class SecondJoinQuit : public WopiTestServer
{
    STATE_ENUM(Phase, LoadUser1, WaitUser1Loaded, User1Loaded, LoadUser2, WaitUser2Loaded, User2Loaded, DropUser2, ModifyDoc, ModifiedDoc, Done) _phase;

    const bool _earlyQuit;

    std::size_t _checkFileInfoCount;
    std::size_t _viewCount;

public:
    SecondJoinQuit(const std::string& name, bool earlyQuit)
        : WopiTestServer(name)
        , _phase(Phase::LoadUser1)
        , _earlyQuit(earlyQuit)
        , _checkFileInfoCount(0)
        , _viewCount(0)
    {
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& /*request*/,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        const bool firstView = _checkFileInfoCount++ == 0;

        TST_LOG("CheckFileInfo: " << (firstView ? "User#1" : "User#2"));

        fileInfo->set("UserCanWrite", "true");
    }

    bool onDocumentError(const std::string& message) override
    {
        TST_LOG("Testing " << name(_phase) << ": [" << message << ']');

        // The 'closedocument' command could race with the second load
        // such that the second load is rejected after we've transition to Done.
        if (_phase != Phase::Done)
            LOK_ASSERT_STATE(_phase, Phase::WaitUser2Loaded);

        LOK_ASSERT_EQUAL_MESSAGE("Expect only documentunloading errors",
                                 std::string("error: cmd=load kind=docunloading"), message);

        TRANSITION_STATE(_phase, Phase::Done);
        passTest("Loading failed, which is acceptable");

        return true;
    }

    bool onFilterSendWebSocketMessage(const std::string_view message, const WSOpCode /* code */,
                                      const bool /* flush */, int& /*unitReturn*/) override
    {
        TST_LOG("WS Message: [" << message << "] in phase: " << name(_phase));

        if (message.starts_with("viewinfo:"))
        {
            Poco::JSON::Parser parser0;
            Poco::JSON::Array::Ptr array =
                parser0.parse(std::string(message.substr(9))).extract<Poco::JSON::Array::Ptr>();
            auto const viewsActive = array->size();
            if (_phase == Phase::ModifiedDoc && viewsActive == 1)
            {
                TRANSITION_STATE(_phase, Phase::Done);
                WSD_CMD("closedocument");
            }
        }

        return false;
    }

    void onDocBrokerViewLoaded(const std::string&,
                               const std::shared_ptr<ClientSession>& session) override
    {
        TST_LOG("View #" << _viewCount + 1 << " [" << session->getName() << "] loaded");

        ++_viewCount;

        if (_viewCount == 1 && _phase == Phase::WaitUser1Loaded)
            TRANSITION_STATE(_phase, Phase::User1Loaded);

        if (_viewCount == 2 && _phase == Phase::WaitUser2Loaded)
            TRANSITION_STATE(_phase, Phase::User2Loaded);
    }

    void onDocBrokerDestroy(const std::string& docKey) override
    {
        TST_LOG("Destroyed dockey [" << docKey << "], phase: " << name(_phase));
        LOK_ASSERT_STATE(_phase, Phase::Done);

        passTest("View disconnection seen");
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::LoadUser1:
            {
                // Always transition before issuing commands.
                TRANSITION_STATE(_phase, Phase::WaitUser1Loaded);

                TST_LOG("Creating first connection");
                initWebsocket("/wopi/files/0?access_token=anything");

                TST_LOG("Loading first view");
                WSD_CMD_BY_CONNECTION_INDEX(0, "load url=" + getWopiSrc());
                break;
            }
            case Phase::User1Loaded:
            {
                TRANSITION_STATE(_phase, Phase::LoadUser2);
                break;
            }
            case Phase::LoadUser2:
            {
                if (!_earlyQuit)
                {
                    // normal case, user 2 loads then then quits after
                    // they have joined
                    TRANSITION_STATE(_phase, Phase::WaitUser2Loaded);
                }
                else
                {
                    // abnormal case, user 2 loses connection right
                    // after launching their load
                    TRANSITION_STATE(_phase, Phase::DropUser2);
                }

                TST_LOG("Creating second connection");
                addWebSocket();

                TST_LOG("Loading second view");
                WSD_CMD_BY_CONNECTION_INDEX(1, "load url=" + getWopiSrc());
                break;
            }
            case Phase::User2Loaded:
            {
                TRANSITION_STATE(_phase, Phase::DropUser2);
                break;
            }
            case Phase::DropUser2:
            {
                TRANSITION_STATE(_phase, Phase::ModifyDoc);

                TST_LOG("Disconnecting first view right after load start");
                deleteSocketAt(1);
                break;
            }
            case Phase::ModifyDoc:
            {
                TRANSITION_STATE(_phase, Phase::ModifiedDoc);

                // Modify the document.
                TST_LOG("Modifying");
                WSD_CMD_BY_CONNECTION_INDEX(0, "key type=input char=97 key=0");
                WSD_CMD_BY_CONNECTION_INDEX(0, "key type=up char=0 key=512");
                break;
            }
            case Phase::WaitUser1Loaded:
            case Phase::WaitUser2Loaded:
            case Phase::ModifiedDoc:
            case Phase::Done:
                break;
        }
    }
};

class SecondJoinQuitNormal : public SecondJoinQuit
{
public:
    SecondJoinQuitNormal()
        : SecondJoinQuit("SecondJoinQuitNormal", false)
    {
    }
};

/* In this case, we are currently failing.

   We have one user connected, the 2nd user join, and immediately drop
   connection after "load" is dispatched. While in the normal case we wait
   until the 2nd user join has completed, and then close the connection.

   In both cases we receive:

     ToClient-007: Send: [viewinfo: [{"id":4,...},
                                     {"id":5,...}]]| common/Session.cpp:62

   so two users are seen as joined, but in the 2nd case we don't get a
   follow up indicating that the 2nd user has dropped.
*/
class SecondJoinQuitEarly : public SecondJoinQuit
{
public:
    SecondJoinQuitEarly()
        : SecondJoinQuit("SecondJoinQuitEarly", true)
    {
        setTimeout(std::chrono::seconds(60));
    }
};

/// This is to test that when a second view join while the first
/// is quitting, we do load the document successfully.
class SecondJoinWhileFirstQuit : public WopiTestServer
{
    STATE_ENUM(Phase, LoadUser1, WaitUser1Loaded, User1Loaded, WaitUser2Loaded, User2Loaded, Done)
    _phase;

    std::size_t _checkFileInfoCount;
    std::size_t _viewCount;
    std::size_t _viewsActive;
    std::size_t _sessionCount;

public:
    SecondJoinWhileFirstQuit()
        : WopiTestServer("SecondJoinWhileFirstQuit")
        , _phase(Phase::LoadUser1)
        , _checkFileInfoCount(0)
        , _viewCount(0)
        , _viewsActive(0)
        , _sessionCount(0)
    {
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& /*request*/,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        const bool firstView = _checkFileInfoCount++ == 0;

        TST_LOG("CheckFileInfo: " << (firstView ? "User#1" : "User#2"));

        fileInfo->set("UserCanWrite", "true");
    }

    bool onDocumentError(const std::string& message) override
    {
        TST_LOG("Testing " << name(_phase) << ": [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitUser2Loaded);

        LOK_ASSERT_EQUAL_MESSAGE("Expect only documentunloading errors",
                                 std::string("error: cmd=load kind=docunloading"), message);

        TRANSITION_STATE(_phase, Phase::Done);
        passTest("Loading failed, which is acceptable");

        return true;
    }

    bool onFilterSendWebSocketMessage(const std::string_view message, const WSOpCode /* code */,
                                      const bool /* flush */, int& /*unitReturn*/) override
    {
        TST_LOG("WS Message: [" << message << "] in phase: " << name(_phase));

        if (message.starts_with("viewinfo:"))
        {
            Poco::JSON::Parser parser0;
            Poco::JSON::Array::Ptr array =
                parser0.parse(std::string(message.substr(9))).extract<Poco::JSON::Array::Ptr>();
            _viewsActive = array->size();
            TST_LOG("Views active: " << _viewsActive);
        }

        return false;
    }

    void onDocBrokerViewLoaded(const std::string&,
                               const std::shared_ptr<ClientSession>& session) override
    {
        TST_LOG("View #" << _viewCount + 1 << " [" << session->getName() << "] loaded");

        ++_viewCount;

        if (_viewCount == 1 && _phase == Phase::WaitUser1Loaded)
        {
            TRANSITION_STATE(_phase, Phase::User1Loaded);
        }
        else if (_viewCount == 2 && _phase == Phase::WaitUser2Loaded)
        {
            TRANSITION_STATE(_phase, Phase::Done);
            passTest("Second view loaded");
        }
    }

    void onDocBrokerDestroy(const std::string& docKey) override
    {
        TST_LOG("Destroyed dockey [" << docKey << "], phase: " << name(_phase));
        LOK_ASSERT_STATE(_phase, Phase::Done);
    }

    /// Called when a new client session is added to a DocumentBroker.
    void onDocBrokerAddSession(const std::string&,
                               const std::shared_ptr<ClientSession>& session) override
    {
        ++_sessionCount;
        TST_LOG("New Session [" << session->getName() << "] added. Have " << _sessionCount
                                << " sessions");
        if (_sessionCount == 2)
        {
            TST_LOG("Disconnecting the first session");
            deleteSocketAt(0);
        }
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::LoadUser1:
            {
                // Always transition before issuing commands.
                TRANSITION_STATE(_phase, Phase::WaitUser1Loaded);

                TST_LOG("Creating first connection");
                initWebsocket("/wopi/files/0?access_token=anything");

                TST_LOG("Loading first view");
                WSD_CMD_BY_CONNECTION_INDEX(0, "load url=" + getWopiSrc());
                break;
            }
            case Phase::User1Loaded:
            {
                TRANSITION_STATE(_phase, Phase::WaitUser2Loaded);

                TST_LOG("Creating second connection");
                addWebSocket();

                TST_LOG("Loading second view");
                WSD_CMD_BY_CONNECTION_INDEX(1, "load url=" + getWopiSrc());
                break;
            }
            case Phase::User2Loaded:
            {
                break;
            }
            case Phase::WaitUser1Loaded:
            case Phase::WaitUser2Loaded:
            case Phase::Done:
                break;
        }
    }
};

UnitBase** unit_create_wsd_multi(void)
{
    return new UnitBase*[4]{ new SecondJoinQuitNormal(), new SecondJoinQuitEarly(),
                             new SecondJoinWhileFirstQuit(), nullptr };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
