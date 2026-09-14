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
 * Unit test for WOPI file rename functionality.
 */

#include <config.h>

#include <common/Util.hpp>
#include <WopiTestServer.hpp>
#include <common/Log.hpp>
#include <Unit.hpp>
#include <UnitHTTP.hpp>
#include <helpers.hpp>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Util/LayeredConfiguration.h>

class UnitWOPIRenameFile : public WopiTestServer
{
    STATE_ENUM(Phase, Load, RejectNameWithPath, WaitNameRejected, RenameFile, WaitRenameNotification,
               Done)
    _phase;

    static constexpr auto FilenameUtf8 = "Ḽơᶉëᶆ ȋṕšᶙṁ ḍỡḽǭᵳ ʂǐť";
    static constexpr auto FilenameUtf7 =
        "+HjwBoR2JAOsdhg +AgseVQFhHZkeQQ +Hg0e4R49Ae0dcw +AoIB0AFl-";

    /// A host is free to take the requested name literally, and this one names
    /// two folders and a file.
    static constexpr auto FilenameWithPath = "Suivi test 20/05/2025";

public:
    UnitWOPIRenameFile()
        : WopiTestServer("UnitWOPIRenameFile")
        , _phase(Phase::Load)
    {
    }

    void assertRenameFileRequest(const Poco::Net::HTTPRequest& request) override
    {
        // The name with a path in it never gets this far, so the header still carries the
        // plain name. spec says UTF-7...
        LOK_ASSERT_EQUAL(std::string(FilenameUtf7), request.get("X-WOPI-RequestedName"));
    }

    bool onFilterSendWebSocketMessage(const std::string_view message, const WSOpCode /* code */,
                                      const bool /* flush */, int& /*unitReturn*/) override
    {
        if (_phase == Phase::WaitNameRejected &&
            message.find("error: cmd=renamefile kind=invalid") == 0)
        {
            TST_LOG("The name with a path in it was refused");
            TRANSITION_STATE(_phase, Phase::RenameFile);
            return false;
        }

        const std::string expected("renamefile filename=" + Uri::encode(FilenameUtf8));

        TST_LOG("Got [" << message << "], expect: [" << expected << ']');
        if (message.find(expected) == 0)
        {
            LOK_ASSERT_STATE(_phase, Phase::WaitRenameNotification);

            // successfully exit the test if we also got the outgoing message
            // notifying about saving the file
            // Don't end the test here, as we'd trigger dead-lock check on
            // joining the socket poll thread, which is where we're called from.
            TRANSITION_STATE(_phase, Phase::Done);
        }

        return false;
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::RejectNameWithPath);

        TRANSITION_STATE(_phase, Phase::WaitNameRejected);

        WSD_CMD("renamefile filename=" + Uri::encode(FilenameWithPath));

        return true;
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                TRANSITION_STATE(_phase, Phase::RejectNameWithPath);

                initWebsocket("/wopi/files/0?access_token=anything");

                WSD_CMD("load url=" + getWopiSrc());
                break;
            }
            case Phase::RejectNameWithPath:
            {
                // wait for the document to load
                break;
            }
            case Phase::WaitNameRejected:
            {
                // just wait for the results
                break;
            }
            case Phase::RenameFile:
            {
                TRANSITION_STATE(_phase, Phase::WaitRenameNotification);

                WSD_CMD("renamefile filename=" + Uri::encode(FilenameUtf8));
                break;
            }
            case Phase::WaitRenameNotification:
            {
                // just wait for the results
                break;
            }
            case Phase::Done:
            {
                passTest("Got the expected renamefile command");
                break;
            }
        }
    }
};

UnitBase *unit_create_wsd(void)
{
    return new UnitWOPIRenameFile();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
