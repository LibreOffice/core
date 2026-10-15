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
#include <common/JailUtil.hpp>
#include <wsd/COOLWSD.hpp>
#include <wsd/DocumentBroker.hpp>
#include <wsd/Process.hpp>

#include <Poco/Net/HTTPRequest.h>
#include <csignal>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <utility>

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

/// Two subforkits requested at once must both be forked by the forkit. Before, the first
/// subforkit went on through the rest of the requests that its parent had queued, and forked
/// a subforkit of its own, which nothing could reach.
class UnitSubForKitNested : public UnitWSD
{
    STATE_ENUM(Phase, Request, WaitSubForKits, Check, Done) _phase;

    std::atomic<int> _created;
    std::chrono::steady_clock::time_point _createdTime;

    /// The parent of a process, and its name, or 0 and empty when it is gone.
    static std::pair<pid_t, std::string> getParentAndName(pid_t pid)
    {
        std::ifstream stat("/proc/" + std::to_string(pid) + "/stat");
        std::string line;
        std::getline(stat, line);
        const auto open = line.find('(');
        const auto close = line.rfind(')');
        if (open == std::string::npos || close == std::string::npos)
            return { 0, std::string() };

        std::istringstream rest(line.substr(close + 2));
        char state = 0;
        pid_t ppid = 0;
        rest >> state >> ppid;
        return { ppid, line.substr(open + 1, close - open - 1) };
    }

public:
    UnitSubForKitNested()
        : UnitWSD("UnitSubForKitNested")
        , _phase(Phase::Request)
        , _created(0)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        UnitWSD::configure(config);

        // Discard the subforkits as soon as the test is done with them.
        config.setUInt("serverside_config.idle_timeout_secs", 0);
    }

    void newSubForKit(const std::shared_ptr<ForKitProcess>& subforkit,
                      const std::string& configId) override
    {
        const pid_t parent = getParentAndName(subforkit->getPid()).first;
        TST_LOG("New SubForKit " << subforkit->getPid() << " for [" << configId
                                 << "], forked by " << parent);
        LOK_ASSERT_EQUAL_MESSAGE("Expected the forkit to fork the subforkit",
                                 static_cast<int>(COOLWSD::ForKitProcId), parent);

        if (++_created == 2)
        {
            _createdTime = std::chrono::steady_clock::now();
            TRANSITION_STATE(_phase, Phase::Check);
        }
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Request:
                TRANSITION_STATE(_phase, Phase::WaitSubForKits);
                // From the prisoner poll, so that both requests reach the forkit together.
                TST_LOG("Requesting two subforkits at once");
                COOLWSD::runInPrisonerPollTestOnly(
                    []
                    {
                        COOLWSD::ensureSubForKit("nestedconfig_a");
                        COOLWSD::ensureSubForKit("nestedconfig_b");
                    });
                break;
            case Phase::Check:
            {
                // Leave time for any subforkit forked by a subforkit to start.
                if (std::chrono::steady_clock::now() - _createdTime < std::chrono::seconds(2))
                    break;

                TRANSITION_STATE(_phase, Phase::Done);
                for (const auto& entry : std::filesystem::directory_iterator("/proc"))
                {
                    const std::string name = entry.path().filename().string();
                    if (name.find_first_not_of("0123456789") != std::string::npos)
                        continue;

                    const auto [parent, comm] = getParentAndName(std::stoi(name));
                    if (comm.starts_with("subforkit_") &&
                        getParentAndName(parent).second.starts_with("subforkit_"))
                    {
                        failTest("Subforkit " + name + " was forked by subforkit " +
                                 std::to_string(parent));
                        return;
                    }
                }

                passTest("Both subforkits were forked by the forkit");
                break;
            }
            case Phase::WaitSubForKits:
            case Phase::Done:
                break;
        }
    }
};

/// When a subforkit is too slow to supply a spare kit, it is reset while documents of its
/// configuration are still open and loading. The shared settings of that configuration stay in
/// place, so the kits of the next subforkit get them.
///
/// The first subforkit is stopped as soon as it connects, so it never forks more than its first
/// kit, which document A gets. Documents B and C then find no spare kit and the subforkit is
/// reset, as on a host too busy to fork in time. Both clients create the settings directories
/// before the reset, and the settings of the second are served only after it, so the next
/// subforkit starts from what that install leaves behind.
class UnitSubForKitReset : public WopiTestServer
{
    using Base = WopiTestServer;

    STATE_ENUM(Phase, LoadFirst, WaitFirstLoaded, WaitReset, WaitNextLoaded, Done)
    _phase;

    const std::string _stamp;
    std::mutex _mutex;
    std::string _configId;
    pid_t _firstSubForKitPid;
    int _subForKitCount;
    int _settingsRequestsDuringReset;
    std::string _nextSubForKitMissing;

    std::string presetsPath() const
    {
        const std::string sharedPresets =
            Poco::Path(COOLWSD::ChildRoot, JailUtil::CHILDROOT_TMP_SHARED_PRESETS_PATH).toString();
        return Poco::Path(sharedPresets, Uri::encode(_configId)).toString();
    }

    bool firstSubForKitReaped() const
    {
        return _firstSubForKitPid > 0 && ::kill(_firstSubForKitPid, 0) != 0 && errno == ESRCH;
    }

    void loadDocument(int fileId)
    {
        initWebsocket("/wopi/files/" + std::to_string(fileId) + "?access_token=anything");
        WSD_CMD_BY_CONNECTION_INDEX(0, "load url=" + getWopiSrc());
    }

public:
    UnitSubForKitReset()
        : Base("UnitSubForKitReset")
        , _phase(Phase::LoadFirst)
        , _stamp("resetconfigid")
        , _firstSubForKitPid(0)
        , _subForKitCount(0)
        , _settingsRequestsDuringReset(0)
    {
    }

    void newSubForKit(const std::shared_ptr<ForKitProcess>& subforkit,
                      const std::string& configId) override
    {
        std::lock_guard<std::mutex> lock(_mutex);
        ++_subForKitCount;
        TST_LOG("New SubForKit #" << _subForKitCount << " [" << subforkit->getPid() << "]: "
                                  << configId);
        if (_subForKitCount == 1)
        {
            _configId = configId;
            _firstSubForKitPid = subforkit->getPid();
            // Its first kit is already forked. Stopped, it forks no spare kit after that one.
            ::kill(_firstSubForKitPid, SIGSTOP);
            return;
        }

        // The settings a kit of this subforkit needs, as they are when the subforkit starts.
        for (const char* entry : { "autotext", "wordbook", "template", "xcu/config.xcu" })
        {
            if (!FileUtil::Stat(Poco::Path(presetsPath(), entry).toString()).exists())
                _nextSubForKitMissing += std::string(_nextSubForKitMissing.empty() ? "" : ", ") +
                                         entry;
        }
    }

    void killSubForKit(const std::string& configId) override
    {
        TST_LOG("Killed SubForKit: " << configId);
        if (_phase == Phase::WaitReset)
            TRANSITION_STATE(_phase, Phase::WaitNextLoaded);
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');
        if (_phase == Phase::WaitFirstLoaded)
        {
            TRANSITION_STATE(_phase, Phase::WaitReset);
            loadDocument(1);
            loadDocument(2);
        }
        else if (_phase == Phase::WaitNextLoaded)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            TRANSITION_STATE(_phase, Phase::Done);
            if (_subForKitCount < 2)
                failTest("The document loaded without a new subforkit");
            else if (!_nextSubForKitMissing.empty())
                failTest("The next subforkit started without the shared settings: " +
                         _nextSubForKitMissing);
            else
                passTest("The next subforkit started with the shared settings");
        }

        return true;
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& /*request*/,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        Poco::JSON::Object::Ptr sharedSettings = new Poco::JSON::Object();
        std::string uri = helpers::getTestServerURI() +
                          "/wopi/settings/sharedconfig.json?testname=UnitSubForKitReset";
        sharedSettings->set("uri", Util::trim(uri));
        sharedSettings->set("stamp", _stamp);
        fileInfo->set("SharedSettings", sharedSettings);
    }

    std::map<std::string, std::string>
        parallelizeCheckInfo(const Poco::Net::HTTPRequest& request,
                             std::istream& /*message*/,
                             const std::shared_ptr<StreamSocket>& /*socket*/) override
    {
        const std::string uri = Uri::decode(request.getURI());
        std::string wopiSrc = "/wopi/files/0";
        for (const char* fileId : { "1", "2" })
        {
            if (uri.find(std::string("/wopi/files/") + fileId) != std::string::npos)
                wopiSrc = std::string("/wopi/files/") + fileId;
        }
        return std::map<std::string, std::string>{
            {"wopiSrc", wopiSrc},
            {"accessToken", "anything"},
            {"noAuthHeader", ""},
            {"permission", ""},
            {"configid", _stamp}
        };
    }

    /// The shared settings hold one xcu and nothing else, so an install creates no autotext,
    /// wordbook or template directory of its own.
    bool handleHttpGetRequest(const Poco::Net::HTTPRequest& request,
                              const std::shared_ptr<StreamSocket>& socket) override
    {
        const std::string path = Poco::URI(request.getURI()).getPath();
        if (path == "/wopi/settings/sharedconfig.json")
        {
            // Both clients have created the settings directories by the time the second of them
            // asks for the settings, so the reset removes directories they already made.
            if (_phase == Phase::WaitReset && ++_settingsRequestsDuringReset == 2)
            {
                TST_LOG("Holding the shared settings until the first subforkit is reaped");
                const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(20);
                while (!firstSubForKitReaped() && std::chrono::steady_clock::now() < deadline)
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }

            const std::string xcuUri =
                helpers::getTestServerURI() +
                "/wopi/settings/shared/config.xcu?testname=UnitSubForKitReset";
            const std::string json =
                "{\"kind\":\"shared\",\"xcu\":[{\"uri\":\"" + xcuUri + "\",\"stamp\":\"x1\"}]}";
            http::Response httpResponse(http::StatusCode::OK);
            httpResponse.setBody(json, "application/json; charset=utf-8");
            socket->sendAndShutdown(httpResponse);
            return true;
        }

        if (path == "/wopi/settings/shared/config.xcu")
        {
            const std::string xcu =
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                "<oor:items xmlns:oor=\"http://openoffice.org/2001/registry\" "
                "xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
                "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"></oor:items>";
            http::Response httpResponse(http::StatusCode::OK);
            httpResponse.setBody(xcu, "application/xml; charset=utf-8");
            socket->sendAndShutdown(httpResponse);
            return true;
        }

        return Base::handleHttpGetRequest(request, socket);
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::LoadFirst:
            {
                TRANSITION_STATE(_phase, Phase::WaitFirstLoaded);
                loadDocument(0);
                break;
            }
            case Phase::WaitFirstLoaded:
            case Phase::WaitReset:
            case Phase::WaitNextLoaded:
            case Phase::Done:
                break;
        }
    }
};

UnitBase** unit_create_wsd_multi(void)
{
    return new UnitBase*[5]{ new UnitSubForKit(), new UnitSubForKitJails(),
                             new UnitSubForKitNested(), new UnitSubForKitReset(), nullptr };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
