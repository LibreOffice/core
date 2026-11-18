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
 * Unit test for WOPI slow connection scenarios.
 */

#include <config.h>

#include <chrono>

#include <HttpRequest.hpp>
#include <common/Util.hpp>
#include <lokassert.hpp>

#include <WopiTestServer.hpp>
#include <common/Log.hpp>
#include <Unit.hpp>
#include <UnitHTTP.hpp>
#include <cstddef>
#include <helpers.hpp>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Util/LayeredConfiguration.h>

using namespace std::literals;

/// Test slow saving/uploading.
/// We modify the document, save, and immediately
/// modify again followed by closing the connection.
/// In this scenario, it's not just that the document
/// is modified at the time of unloading, which is
/// covered by the UnitWOPIAsncUpload_ModifyClose
/// test. Instead, here we close the connection
/// while the document is being saved and uploaded.
/// Unlike the failed upload scenario, this one
/// will hit "upload in progress" and will test
/// that in such a case we don't drop the latest
/// changes, which were done while save/upload
/// were in progress.
/// Modify, Save, Modify, Close -> No data loss.
class UnitWOPISlow : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitModifiedStatus, WaitPutFile) _phase;

    static constexpr auto LargeDocumentFilename = "large-six-hundred.odt";

    /// The number of key input sent.
    std::size_t _inputCount;

public:
    UnitWOPISlow()
        : WopiTestServer("UnitWOPISlow", LargeDocumentFilename)
        , _phase(Phase::Load)
        , _inputCount(0)
    {
        // We need more time than the default.
        setTimeout(10min);
    }

    std::unique_ptr<http::Response>
    assertPutFileRequest(const Poco::Net::HTTPRequest& request) override
    {
        TST_LOG("PutFile");
        LOK_ASSERT_STATE(_phase, Phase::WaitPutFile);

        // Triggered while closing.
        LOK_ASSERT_EQUAL_STR("false", request.get("X-COOL-WOPI-IsAutosave"));

        LOK_ASSERT_EQUAL_STR("true", request.get("X-COOL-WOPI-IsModifiedByUser"));

        passTest("Document uploaded on closing as expected.");
        return nullptr;
    }

    void onDocBrokerDestroy(const std::string& docKey) override
    {
        passTest("Document [" + docKey + "] uploaded and closed cleanly.");
    }

    /// The document is loaded.
    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("Doc (" << name(_phase) << "): [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);

        // Modify and wait for the notification.
        TRANSITION_STATE(_phase, Phase::WaitModifiedStatus);

        TST_LOG("Sending key input #" << ++_inputCount);
        WSD_CMD("key type=input char=97 key=0");
        WSD_CMD("key type=up char=0 key=512");

        return true;
    }

    /// The document is modified. Save, modify, and close it.
    bool onDocumentModified(const std::string& message) override
    {
        // We modify the document multiple times.
        // Only the first time is handled here.
        if (_phase == Phase::WaitModifiedStatus)
        {
            TST_LOG("Doc (" << name(_phase) << "): [" << message << ']');
            LOK_ASSERT_STATE(_phase, Phase::WaitModifiedStatus);

            // Save and immediately modify, then close the connection.
            WSD_CMD("save dontTerminateEdit=0 dontSaveIfUnmodified=0 "
                    "extendedData=CustomFlag%3DCustom%20Value%3BAnotherFlag%3DAnotherValue");

            TST_LOG("Sending key input #" << ++_inputCount);
            WSD_CMD("key type=input char=97 key=0");
            WSD_CMD("key type=up char=0 key=512");

            TST_LOG("Closing the connection.");
            deleteSocketAt(0);

            // Don't transition to WaitPutFile until after closing the socket.
            TRANSITION_STATE(_phase, Phase::WaitPutFile);
        }

        return true;
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                TRANSITION_STATE(_phase, Phase::WaitLoadStatus);

                TST_LOG("Load: initWebsocket.");
                initWebsocket("/wopi/files/large-six-hundred.odt?access_token=anything");

                WSD_CMD("load url=" + getWopiSrc());
                break;
            }
            case Phase::WaitLoadStatus:
                break;
            case Phase::WaitModifiedStatus:
                break;
            case Phase::WaitPutFile:
                break;
        }
    }
};

/// Test superfluous saves.
class UnitSuperfluousSaves : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitModifiedStatus, WaitPutFile, Done) _phase;

    Util::Stopwatch _stopwatch;

    /// The number of key input sent.
    std::size_t _saveCount;
    int _uploadCount; ///< The number of times we uploaded.

public:
    UnitSuperfluousSaves()
        : WopiTestServer("UnitSuperfluousSaves")
        , _phase(Phase::Load)
        , _saveCount(0)
        , _uploadCount(0)
    {
        // We need more time than the default.
        setTimeout(2min);
    }

    std::unique_ptr<http::Response>
    assertPutFileRequest(const Poco::Net::HTTPRequest& request) override
    {
        ++_uploadCount;
        TST_LOG("PutFile #" << _uploadCount);

        LOK_ASSERT_EQUAL_STR("false", request.get("X-COOL-WOPI-IsAutosave"));
        LOK_ASSERT_EQUAL_STR("false", request.get("X-COOL-WOPI-IsExitSave"));

        if (_phase == Phase::WaitPutFile)
        {
            LOK_ASSERT_EQUAL_STR("true", request.get("X-COOL-WOPI-IsModifiedByUser"));
            LOK_ASSERT_EQUAL_MESSAGE("Expected to be in Phase::WaitPutFile", 1, _uploadCount);
            TRANSITION_STATE(_phase, Phase::Done);
        }
        else
        {
            LOK_ASSERT_EQUAL_STR("false", request.get("X-COOL-WOPI-IsModifiedByUser"));
            LOK_ASSERT_STATE(_phase, Phase::Done);
            // LOK_ASSERT_EQUAL_MESSAGE("Expected to be in Phase::WaitPutFile", 2, _uploadCount);
        }

        return nullptr;
    }

    /// The document is loaded.
    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("Doc (" << name(_phase) << "): [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);

        // Modify and wait for the notification.
        TRANSITION_STATE(_phase, Phase::WaitModifiedStatus);

        WSD_CMD("key type=input char=97 key=0");
        WSD_CMD("key type=up char=0 key=512");

        return true;
    }

    /// The document is modified. Save, modify, and close it.
    bool onDocumentModified(const std::string& message) override
    {
        TST_LOG("Doc (" << name(_phase) << "): [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitModifiedStatus);

        _stopwatch.restart();

        // Don't transition to WaitPutFile until after closing the socket.
        TRANSITION_STATE(_phase, Phase::WaitPutFile);

        return true;
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                TRANSITION_STATE(_phase, Phase::WaitLoadStatus);

                TST_LOG("Load: initWebsocket.");
                initWebsocket("/wopi/files/" + getTestname() + "?access_token=anything");

                WSD_CMD("load url=" + getWopiSrc());
                break;
            }
            case Phase::WaitLoadStatus:
                break;
            case Phase::WaitModifiedStatus:
                break;
            case Phase::WaitPutFile:
            {
                // Save while we're waiting.
                TST_LOG("Sending key input #" << _saveCount);
                WSD_CMD("save dontTerminateEdit=0 dontSaveIfUnmodified=0");
            }
            break;
            case Phase::Done:
            {
                if (_stopwatch.elapsed(10s))
                {
                    passTest("No unexpected conditions met");
                }
            }
            break;
        }
    }
};

UnitBase** unit_create_wsd_multi(void)
{
    return new UnitBase* [3] { new UnitWOPISlow(), new UnitSuperfluousSaves(), nullptr };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
