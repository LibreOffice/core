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
 * Downloading a document in another format runs in a forked process.
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
/// Left behind in the jail by the forked process, so the test can tell one ran.
constexpr auto ForkStampName = "downloadwasforked";

/// Asks the forked process to raise a dialog, standing in for the warning core raises
/// when a spreadsheet of several sheets is written to a format that holds one.
constexpr auto DialogStampName = "downloadraisesdialog";

/// Where a stamp file of the given name sits inside the document's jail. The jail's own
/// /tmp is resolved first, because buildLocalPathToJail creates a directory for every
/// component it is given.
std::string jailStampPath(const std::string& jailRoot, const std::string& name)
{
    const std::string jailTmp =
        FileUtil::buildLocalPathToJail(JailUtil::isMountNamespacesEnabled(), jailRoot, "/tmp/");
    return jailTmp + '/' + name;
}
}

/// A download hands the export to a forked process, so the Kit stays free for the
/// other views, and the client still gets its download link once the copy is written.
class UnitBgDownload : public UnitWSDClient
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitDownload, Done) _phase;

public:
    UnitBgDownload()
        : UnitWSDClient("UnitBgDownload")
        , _phase(Phase::Load)
    {
        setHasKitHooks();
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("Got: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);

        TRANSITION_STATE(_phase, Phase::WaitDownload);

        WSD_CMD("downloadas name=copy.xlsx id=export format=xlsx options=");

        return true;
    }

    bool onFilterSendWebSocketMessage(const std::string_view message, const WSOpCode /* code */,
                                      const bool /* flush */, int& /*unitReturn*/) override
    {
        if (!message.starts_with("downloadas:"))
            return false;

        LOK_ASSERT_STATE(_phase, Phase::WaitDownload);
        TRANSITION_STATE(_phase, Phase::Done);

        TST_LOG("Got: [" << message << ']');

        LOK_ASSERT_MESSAGE(
            "the export did not run in a forked process",
            FileUtil::Stat(jailStampPath(getJailRoot(), ForkStampName)).isFile());

        passTest("Downloaded a copy exported by a forked process");
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
                connectAndLoadLocalDocument("empty.ods");
                break;
            }
            case Phase::WaitLoadStatus:
            case Phase::WaitDownload:
            case Phase::Done:
                break;
        }
    }
};

/// Kit-side half: leaves a stamp behind whenever a process is forked to do work.
class UnitKitBgDownload : public UnitKit
{
public:
    UnitKitBgDownload()
        : UnitKit("bgdownload")
    {
    }

    void initialize() override
    {
        // Empty, so no socket poll thread is started. Forking needs the Kit down to a
        // single thread, and an extra one here would send the export back to the
        // foreground and defeat the test.
    }

    void postBackgroundSaveFork() override
    {
        std::ofstream stamp(std::string("/tmp/") + ForkStampName);
        stamp.close();
    }

    std::string getBackgroundSaveInjectMessage() override
    {
        if (!FileUtil::Stat(std::string("/tmp/") + DialogStampName).exists())
            return std::string();

        // Stands in for the warning core raises while writing the copy.
        return "client-0000 jsdialog: { \"id\": 13, \"jsontype\": \"dialog\", "
               "\"action\": \"show\" }";
    }
};

/// Core can ask the person something while writing the copy, and the forked process has
/// nobody to ask. The Kit takes the export back and runs it itself, where the question can
/// be put and answered, so the download still arrives rather than failing.
class UnitBgDownloadDialog : public UnitWSDClient
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitDownload, Done) _phase;

public:
    UnitBgDownloadDialog()
        : UnitWSDClient("UnitBgDownloadDialog")
        , _phase(Phase::Load)
    {
        setHasKitHooks();
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("Got: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);

        TRANSITION_STATE(_phase, Phase::WaitDownload);

        std::ofstream stamp(jailStampPath(getJailRoot(), DialogStampName));
        stamp.close();

        WSD_CMD("downloadas name=copy.xlsx id=export format=xlsx options=");

        return true;
    }

    bool onFilterSendWebSocketMessage(const std::string_view message, const WSOpCode /* code */,
                                      const bool /* flush */, int& /*unitReturn*/) override
    {
        if (message.starts_with("error: cmd=downloadas"))
        {
            TST_LOG("Got: [" << message << ']');
            LOK_ASSERT_FAIL("the download was abandoned over a dialog rather than re-run");
        }

        if (!message.starts_with("downloadas:"))
            return false;

        LOK_ASSERT_STATE(_phase, Phase::WaitDownload);
        TST_LOG("Got: [" << message << ']');

        LOK_ASSERT_MESSAGE(
            "the export did not start out in a forked process",
            FileUtil::Stat(jailStampPath(getJailRoot(), ForkStampName)).isFile());

        TRANSITION_STATE(_phase, Phase::Done);
        passTest("An export that needed an answer was run by the Kit and still arrived");
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
                connectAndLoadLocalDocument("empty.ods");
                break;
            }
            case Phase::WaitLoadStatus:
            case Phase::WaitDownload:
            case Phase::Done:
                break;
        }
    }
};

UnitBase** unit_create_wsd_multi(void)
{
    return new UnitBase* [] { new UnitBgDownload(), new UnitBgDownloadDialog(), nullptr };
}

UnitBase* unit_create_kit(void) { return new UnitKitBgDownload(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
