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
 * Unit test for user preset functionality.
 */

#include <config.h>

#include <WopiTestServer.hpp>
#include <Unit.hpp>
#include <lokassert.hpp>
#include <testlog.hpp>
#include <common/FileUtil.hpp>
#include <wsd/DocumentBroker.hpp>
#include <wsd/Process.hpp>

#include <Poco/Net/HTTPRequest.h>
#include <csignal>
#include <ctime>

using namespace std::literals;

/// This test ensures that a document which has presets, but whose load is
/// canceled before the presets are installed, gracefully handles the case that
/// the document broker poll no longer exists when the response from async dns
/// arrives and the preset download attempt cannot be attached to the dead
/// poll.
class UnitEarlyDocDeath : public WopiTestServer
{
    using Base = WopiTestServer;

    STATE_ENUM(Phase, Load, WaitDocPresetsInstallStart, DocPresetsInstallStart, WaitDocClose, ResumeDNS, Finish, Done) _phase;

    std::mutex _dns_mutex;
    std::condition_variable _dns_cv;

public:
    UnitEarlyDocDeath()
        : Base("UnitEarlyDocDeath")
        , _phase(Phase::Load)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        Base::configure(config);

        // Set to 0 to immediately discard any unused subforkits
        config.setUInt("serverside_config.idle_timeout_secs", 0);
    }

    // replace the preset asset uri so dns requests for them can
    // be identified when they are queried so we can delay their
    // resolution until the document load is abandoned.
    void filterRegisterPresetAsset(std::string& uri) override
    {
        TST_LOG("filterRegisterPresetAsset before: " << uri);
        uri = Util::replace(uri, "localhost", "presetasset");
        TST_LOG("filterRegisterPresetAsset after: " << uri);
    }

    // delay the resolution of these queries so we can cancel
    // the document load and let the dns complete when the
    // document has cancelled to test we don't crash under
    // this circumstance
    void filterResolveDNS(std::string& query) override
    {
        if (query == "presetasset")
        {
            query = "localhost";
            // Not waiting for Done would lead to a hang on slow machines.
            if (_phase != Phase::Finish && _phase != Phase::Done)
            {
                TST_LOG(
                    "delaying dns resolution of preset host until document broker is destroyed");
                std::unique_lock<std::mutex> lock(_dns_mutex);
                _dns_cv.wait(lock, [this]() { return _phase == Phase::ResumeDNS; });
                TST_LOG("dns resumed after doc broker destruction");
                TRANSITION_STATE(_phase, Phase::Finish);
            }
        }
    }

    // as soon as presets install starts, then cancel the load of the
    // document.
    void onDocBrokerPresetsInstallStart() override
    {
        TST_LOG("onDocBrokerPresetsInstallStart");
        LOK_ASSERT_STATE(_phase, Phase::WaitDocPresetsInstallStart);
        TRANSITION_STATE(_phase, Phase::DocPresetsInstallStart);
        SocketPoll::wakeupWorld();
    }

    // presets install is delayed until document should be destroyed
    void onDocBrokerPresetsInstallEnd(bool /*success*/) override
    {
        failTest("Document should be destroyed before presets are installed");
    }

    void onDocBrokerDestroy(const std::string& /*docKey*/) override
    {
        LOK_ASSERT_STATE(_phase, Phase::WaitDocClose);
        TRANSITION_STATE(_phase, Phase::ResumeDNS);
        TST_LOG("resume dns resolution after doc broker was destroyed");
        _dns_cv.notify_one();
        SocketPoll::wakeupWorld();
    }

    // document shouldn't get loaded until presets are installed,
    // and we delay preset installation via stalled dns lookup
    // and cancel the load, so the doc should never get loaded
    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');
        failTest("Document should not get loaded.");
        return true;
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& request,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        const Poco::URI uriReq(request.getURI());
        Poco::JSON::Object::Ptr userSettings = new Poco::JSON::Object();
        std::string uri = helpers::getTestServerURI() + "/wopi/settings/userconfig.json?testname=UnitEarlyDocDeath";
        userSettings->set("uri", Util::trim(uri));
        userSettings->set("stamp", "something");
        fileInfo->set("UserSettings", userSettings);
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
            {"configid", ""}
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
                TRANSITION_STATE(_phase, Phase::WaitDocPresetsInstallStart);

                TST_LOG("Creating first connection");
                initWebsocket("/wopi/files/0?access_token=anything");

                TST_LOG("Loading view");
                WSD_CMD_BY_CONNECTION_INDEX(0, "load url=" + getWopiSrc());
                break;
            }
            case Phase::DocPresetsInstallStart:
                TRANSITION_STATE(_phase, Phase::WaitDocClose);
                TST_LOG("Close document just after preset install starts");
                WSD_CMD_BY_CONNECTION_INDEX(0, "closedocument");
                break;
            case Phase::WaitDocPresetsInstallStart:
            case Phase::WaitDocClose:
            case Phase::ResumeDNS:
            case Phase::Done:
            {
                // just wait for the results
                break;
            }
            case Phase::Finish:
            {
                TRANSITION_STATE(_phase, Phase::Done);
                passTest("Document load successfully abandoned");
            }
        }
    }
};

UnitBase** unit_create_wsd_multi(void)
{
    return new UnitBase*[2]{ new UnitEarlyDocDeath(), nullptr };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
