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
 * Unit test for multi-tenant functionality.
 */

#include <config.h>

#include <WopiTestServer.hpp>
#include <WOPIUploadConflictCommon.hpp>
#include <Unit.hpp>
#include <lokassert.hpp>
#include <testlog.hpp>
#include <common/FileUtil.hpp>
#include <wsd/COOLWSD.hpp>
#include <wsd/DocumentBroker.hpp>
#include <wsd/Process.hpp>

#include <Poco/Net/HTTPRequest.h>
#include <csignal>
#include <ctime>
#include <mutex>

/// This test simulates a crashing kit with modifications.
class UnitSubForKit : public WopiTestServer
{
    using Base = WopiTestServer;

    STATE_ENUM(Phase, Load, WaitCreateSubForKit, WaitLoadStatus, WaitDocClose, WaitKillSubForKit, Finish, Done) _phase;

    std::string _configId;

public:
    UnitSubForKit()
        : Base("UnitSubForKit")
        , _phase(Phase::Load)
        , _configId("someconfigid")
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        Base::configure(config);

        // Set to 0 to immediately discard any unused subforkits
        config.setUInt("serverside_config.idle_timeout_secs", 0);
    }

    void newSubForKit(const std::shared_ptr<ForKitProcess>& /*subforkit*/, const std::string& configId) override
    {
        TST_LOG("New SubForKit: " << configId);
        if (configId.find(_configId) == std::string::npos)
            failTest("unexpected subforkit configId");
        LOK_ASSERT_STATE(_phase, Phase::WaitCreateSubForKit);

        TRANSITION_STATE(_phase, Phase::WaitLoadStatus);
    }

    void killSubForKit(const std::string& configId) override
    {
        LOK_ASSERT_STATE(_phase, Phase::WaitKillSubForKit);
        TST_LOG("Killed SubForKit: " << configId);
        if (_configId == "someconfigid")
        {
            _configId = "someotherconfig";
            TST_LOG("reload with a different server config" << _configId);
            TRANSITION_STATE(_phase, Phase::Load);
        }
        else
            TRANSITION_STATE(_phase, Phase::Finish);
    }

    void onDocBrokerDestroy(const std::string& /*docKey*/) override
    {
        LOK_ASSERT_STATE(_phase, Phase::WaitDocClose);
        TRANSITION_STATE(_phase, Phase::WaitKillSubForKit);
        SocketPoll::wakeupWorld();
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);


        TRANSITION_STATE(_phase, Phase::WaitDocClose);
        WSD_CMD("closedocument");

        return true;
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& request,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        const Poco::URI uriReq(request.getURI());
        Poco::JSON::Object::Ptr sharedSettings = new Poco::JSON::Object();
        std::string uri = helpers::getTestServerURI() + "/wopi/settings/sharedconfig.json?testname=UnitSubForKit";
        sharedSettings->set("uri", Util::trim(uri));
        sharedSettings->set("stamp", _configId);
        fileInfo->set("SharedSettings", sharedSettings);
    }

    std::map<std::string, std::string>
        parallelizeCheckInfo(const Poco::Net::HTTPRequest& request,
                             std::istream& /*message*/,
                             const std::shared_ptr<StreamSocket>& /*socket*/) override
    {
        std::string uri = Uri::decode(request.getURI());
        TST_LOG("parallelizeCheckInfo requested: " << uri);
        return std::map<std::string, std::string>{
            {"wopiSrc", "/wopi/files/0"},
            {"accessToken", "anything"},
            {"noAuthHeader", ""},
            {"permission", ""},
            {"configid", _configId}
        };
    }

    // on loading this document, a new subforkit is needed, so that should
    // be created on demand, and then the document loaded via that subforkit
    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                // Always transition before issuing commands.
                TRANSITION_STATE(_phase, Phase::WaitCreateSubForKit);

                TST_LOG("Creating first connection");
                initWebsocket("/wopi/files/0?access_token=anything");

                TST_LOG("Loading view");
                WSD_CMD_BY_CONNECTION_INDEX(0, "load url=" + getWopiSrc());
                break;
            }
            case Phase::WaitCreateSubForKit:
            case Phase::WaitKillSubForKit:
            case Phase::WaitLoadStatus:
            case Phase::WaitDocClose:
            case Phase::Done:
            {
                // just wait for the results
                break;
            }
            case Phase::Finish:
            {
                TRANSITION_STATE(_phase, Phase::Done);
                passTest("Document loaded successfully");
            }
        }
    }
};

/// Once an idle subforkit is dropped, the jails of all its kits are removed, including those of
/// its spare kits.
class UnitSubForKitJails : public WopiTestServer
{
    using Base = WopiTestServer;

    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitDocClose, WaitKillSubForKit, WaitJailsRemoved,
               Done)
    _phase;

    const std::string _configId;
    std::mutex _jailPathsMutex;
    std::vector<std::string> _jailPaths;
    std::chrono::steady_clock::time_point _killTime;

public:
    UnitSubForKitJails()
        : Base("UnitSubForKitJails")
        , _phase(Phase::Load)
        , _configId("jailsconfigid")
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        Base::configure(config);

        // Set to 0 to immediately discard any unused subforkits
        config.setUInt("serverside_config.idle_timeout_secs", 0);
    }

    void newChild(const std::shared_ptr<ChildProcess>& child) override
    {
        if (child->getConfigId().find(_configId) == std::string::npos)
            return;

        const std::string jailPath = COOLWSD::ChildRoot + child->getJailId();
        TST_LOG("New kit of the subforkit, jail [" << jailPath << ']');
        std::lock_guard<std::mutex> lock(_jailPathsMutex);
        _jailPaths.push_back(jailPath);
    }

    void killSubForKit(const std::string& configId) override
    {
        LOK_ASSERT_STATE(_phase, Phase::WaitKillSubForKit);
        TST_LOG("Killed SubForKit: " << configId);
        _killTime = std::chrono::steady_clock::now();
        TRANSITION_STATE(_phase, Phase::WaitJailsRemoved);
    }

    void onDocBrokerDestroy(const std::string& /*docKey*/) override
    {
        LOK_ASSERT_STATE(_phase, Phase::WaitDocClose);
        TRANSITION_STATE(_phase, Phase::WaitKillSubForKit);
        SocketPoll::wakeupWorld();
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);

        TRANSITION_STATE(_phase, Phase::WaitDocClose);
        WSD_CMD("closedocument");

        return true;
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& /*request*/,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        Poco::JSON::Object::Ptr sharedSettings = new Poco::JSON::Object();
        std::string uri = helpers::getTestServerURI() +
                          "/wopi/settings/sharedconfig.json?testname=UnitSubForKitJails";
        sharedSettings->set("uri", Util::trim(uri));
        sharedSettings->set("stamp", _configId);
        fileInfo->set("SharedSettings", sharedSettings);
    }

    std::map<std::string, std::string>
        parallelizeCheckInfo(const Poco::Net::HTTPRequest& /*request*/,
                             std::istream& /*message*/,
                             const std::shared_ptr<StreamSocket>& /*socket*/) override
    {
        return std::map<std::string, std::string>{
            {"wopiSrc", "/wopi/files/0"},
            {"accessToken", "anything"},
            {"noAuthHeader", ""},
            {"permission", ""},
            {"configid", _configId}
        };
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                TRANSITION_STATE(_phase, Phase::WaitLoadStatus);

                initWebsocket("/wopi/files/0?access_token=anything");
                WSD_CMD_BY_CONNECTION_INDEX(0, "load url=" + getWopiSrc());
                break;
            }
            case Phase::WaitJailsRemoved:
            {
                std::vector<std::string> remaining;
                {
                    std::lock_guard<std::mutex> lock(_jailPathsMutex);
                    // The kit of the document and at least one spare kit.
                    LOK_ASSERT_MESSAGE("The subforkit had no spare kit", _jailPaths.size() >= 2);
                    for (const std::string& jailPath : _jailPaths)
                    {
                        if (FileUtil::Stat(jailPath).exists())
                            remaining.push_back(jailPath);
                    }
                }

                if (remaining.empty())
                {
                    TRANSITION_STATE(_phase, Phase::Done);
                    passTest("The jails of the dropped subforkit were removed");
                }
                else if (std::chrono::steady_clock::now() - _killTime > std::chrono::seconds(20))
                {
                    failTest("Jail left behind after the subforkit was dropped: " +
                             remaining.front());
                }
                break;
            }
            case Phase::WaitLoadStatus:
            case Phase::WaitDocClose:
            case Phase::WaitKillSubForKit:
            case Phase::Done:
                break;
        }
    }
};

UnitBase** unit_create_wsd_multi(void)
{
    return new UnitBase*[3]{
        new UnitSubForKit(), new UnitSubForKitJails(), nullptr
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
