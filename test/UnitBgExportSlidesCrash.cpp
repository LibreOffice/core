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
 * A slide export whose forked process dies leaves the way open for the next one.
 */

#include <config.h>

#include <test/UnitWSDClient.hpp>
#include <Unit.hpp>
#include <common/FileUtil.hpp>
#include <helpers.hpp>
#include <test/lokassert.hpp>

#include <fstream>
#include <iostream>
#include <string>

#include <unistd.h>

namespace
{
/// Exists in the jail once the first forked process has started.
constexpr auto ForkStampName = "slideexportcrashed";
}

/// The first export's forked process exits without writing anything. That request gets a
/// failure back, and the second request, which waits for the first to end, still gets its
/// slides.
class UnitBgExportSlidesCrash : public UnitWSDClient
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitFailed, WaitWritten, Done) _phase;

public:
    UnitBgExportSlidesCrash()
        : UnitWSDClient("UnitBgExportSlidesCrash")
        , _phase(Phase::Load)
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

        TRANSITION_STATE(_phase, Phase::WaitFailed);

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

        if (_phase == Phase::WaitFailed)
        {
            LOK_ASSERT_MESSAGE("the export whose process died did not fail",
                               message.starts_with("exportslides: {\"status\":\"failed\""));
            TRANSITION_STATE(_phase, Phase::WaitWritten);
            return false;
        }

        LOK_ASSERT_STATE(_phase, Phase::WaitWritten);
        LOK_ASSERT_MESSAGE("the export after the failed one was not written",
                           message.starts_with("exportslides: {\"status\":\"written\"}\n"));

        TRANSITION_STATE(_phase, Phase::Done);
        passTest("The export after the failed one was written");
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
            case Phase::WaitFailed:
            case Phase::WaitWritten:
            case Phase::Done:
                break;
        }
    }
};

/// Kit-side half: the first forked process exits at once, without a result. Later ones work.
class UnitKitBgExportSlidesCrash : public UnitKit
{
public:
    UnitKitBgExportSlidesCrash()
        : UnitKit("bgexportslidescrash")
    {
    }

    void initialize() override
    {
        // Empty, so no socket poll thread is started. The Kit keeps a single thread, and the
        // export can then run in a forked process.
    }

    void postBackgroundSaveFork() override
    {
        const std::string stampPath = std::string("/tmp/") + ForkStampName;
        if (FileUtil::Stat(stampPath).exists())
            return;

        std::ofstream stamp(stampPath);
        stamp.close();

        std::cerr << "Exit the first slide export process to simulate a crash\n";
        _exit(0);
    }
};

UnitBase* unit_create_wsd(void) { return new UnitBgExportSlidesCrash(); }

UnitBase* unit_create_kit(void) { return new UnitKitBgExportSlidesCrash(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
