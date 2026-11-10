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
 * Writing slides out for another presentation runs in a forked process.
 */

#include <config.h>

#include <test/UnitWSDClient.hpp>
#include <Unit.hpp>
#include <common/FileUtil.hpp>
#include <JailUtil.hpp>
#include <helpers.hpp>
#include <test/lokassert.hpp>

#include <fstream>
#include <string>

namespace
{
/// Gains a line in the jail for every process forked to do work, so the test can count them.
constexpr auto ForkStampName = "slideexportforks";

/// Where the stamp file sits inside the document's jail. The jail's own /tmp is resolved
/// first, because buildLocalPathToJail creates a directory for every component it is given.
std::string jailStampPath(const std::string& jailRoot)
{
    const std::string jailTmp =
        FileUtil::buildLocalPathToJail(JailUtil::isMountNamespacesEnabled(), jailRoot, "/tmp/");
    return jailTmp + '/' + ForkStampName;
}

/// How many processes were forked, from the lines of the stamp file.
int countForks(const std::string& jailRoot)
{
    std::ifstream stamp(jailStampPath(jailRoot));
    int forks = 0;
    std::string line;
    while (std::getline(stamp, line))
        ++forks;
    return forks;
}
}

/// The slides go out from a forked process, so the Kit stays free for the people editing the
/// presentation. A second request that comes while the first one runs gets its slides as well,
/// from a forked process of its own.
class UnitBgExportSlides : public UnitWSDClient
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitExports, Done) _phase;
    int _written;

public:
    UnitBgExportSlides()
        : UnitWSDClient("UnitBgExportSlides")
        , _phase(Phase::Load)
        , _written(0)
    {
        setHasKitHooks();
    }

    void configure(Poco::Util::LayeredConfiguration & config) override {
        UnitWSDClient::configure(config);
        // The server writes slides out only with remote links enabled:
        config.setBool("remote_links.enable", true);
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("Got: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);

        TRANSITION_STATE(_phase, Phase::WaitExports);

        WSD_CMD("exportslides");
        WSD_CMD("exportslides");

        return true;
    }

    bool onFilterSendWebSocketMessage(const std::string_view message, const WSOpCode /* code */,
                                      const bool /* flush */, int& /*unitReturn*/) override
    {
        if (!message.starts_with("exportslides:"))
            return false;

        TST_LOG("Got: [" << COOLProtocol::getFirstLine(message.data(), message.size()) << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitExports);
        LOK_ASSERT_MESSAGE("the slides were not written",
                           message.starts_with("exportslides: {\"status\":\"written\"}\n"));

        if (++_written < 2)
            return false;

        TRANSITION_STATE(_phase, Phase::Done);
        LOK_ASSERT_EQUAL_MESSAGE("each export did not run in a forked process of its own", 2,
                                 countForks(getJailRoot()));

        passTest("Both exports were written by forked processes");
        return false;
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                TRANSITION_STATE(_phase, Phase::WaitLoadStatus);

                TST_LOG("Loading document");
                connectAndLoadLocalDocument("hello.odp");
                break;
            }
            case Phase::WaitLoadStatus:
            case Phase::WaitExports:
            case Phase::Done:
                break;
        }
    }
};

/// Kit-side half: adds a line to the stamp whenever a process is forked to do work.
class UnitKitBgExportSlides : public UnitKit
{
public:
    UnitKitBgExportSlides()
        : UnitKit("bgexportslides")
    {
    }

    void initialize() override
    {
        // Empty, so no socket poll thread is started. The Kit keeps a single thread, and the
        // export can then run in a forked process.
    }

    void postBackgroundSaveFork() override
    {
        std::ofstream stamp(std::string("/tmp/") + ForkStampName, std::ios::app);
        stamp << "forked\n";
    }
};

UnitBase* unit_create_wsd(void) { return new UnitBgExportSlides(); }

UnitBase* unit_create_kit(void) { return new UnitKitBgExportSlides(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
