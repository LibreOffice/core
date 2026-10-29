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
#include <kit/ChildSession.hpp>
#include <common/Unit.hpp>
#include <lokassert.hpp>

#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <Poco/Net/HTTPRequest.h>

#include <sstream>
#include <string>

/// The thumbnail sizes a host lists in CheckFileInfo reach the kit with the load command.
/// The host side lists the sizes; the kit side, below, checks what arrives and ends the test.
class UnitWOPIThumbnailSizes : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitKit) _phase;

public:
    UnitWOPIThumbnailSizes()
        : WopiTestServer("UnitWOPIThumbnailSizes")
        , _phase(Phase::Load)
    {
        setHasKitHooks();
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& /*request*/,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        // The entry that is not a size is dropped before the kit sees the list.
        Poco::JSON::Array::Ptr sizes = new Poco::JSON::Array();
        sizes->add("1000x1000");
        sizes->add("large");
        sizes->add("256x256");
        fileInfo->set("ThumbnailSizes", sizes);
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                TRANSITION_STATE(_phase, Phase::WaitKit);

                TST_LOG("Load: initWebsocket.");
                initWebsocket("/wopi/files/0?access_token=anything");

                WSD_CMD("load url=" + getWopiSrc());
            }
            break;

            case Phase::WaitKit:
            {
                // The kit side ends the test.
                break;
            }
        }
    }
};

/// Checks, in the kit, the load command and the sizes the session read from it.
class UnitKitThumbnailSizes : public UnitKit
{
    Session* _session = nullptr;
    bool _loadSeen = false;

public:
    UnitKitThumbnailSizes()
        : UnitKit("UnitKitThumbnailSizes")
    {
    }

    void postKitSessionCreated(Session* session) override { _session = session; }

    bool filterKitMessage(WebSocketHandler*, std::string& message) override
    {
        if (message.find("load url=") == std::string::npos)
            return false;

        TST_LOG("Load command: [" << message << ']');
        if (message.find(" thumbnailSizes=1000x1000,256x256") == std::string::npos)
        {
            exitTest(TestResult::Failed,
                     "The load command does not carry the thumbnail sizes the host listed");
        }

        _loadSeen = true;
        return false;
    }

    void preKitPollCallback() override
    {
        if (!_loadSeen || !_session || isFinished())
            return;

        // The session reads the sizes when it handles the load command, which may be after
        // the command was filtered, so wait until its state shows them.
        std::ostringstream state;
        static_cast<ChildSession*>(_session)->dumpState(state);
        if (state.str().find("thumbnailSizes: 1000x1000 256x256") != std::string::npos)
            exitTest(TestResult::Ok, "The session read the thumbnail sizes");
    }
};

UnitBase* unit_create_wsd(void) { return new UnitWOPIThumbnailSizes(); }

UnitBase* unit_create_kit(void) { return new UnitKitThumbnailSizes(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
