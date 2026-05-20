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
 * Unit test for WOPI owner termination scenarios.
 */

#include <config.h>

#include <common/Unit.hpp>
#include <common/Util.hpp>
#include <test/WopiTestServer.hpp>
#include <test/lokassert.hpp>

#include <Poco/Net/HTTPRequest.h>

#include <csignal>

using namespace std::literals;

/// This tests the rejection logic and messages that
/// happen when a document is connected to while
/// it is being unloaded.
/// Unfortunately, there is an inherent race here
/// in that we might have already unloaded by the
/// time we request loading via a different
/// connection. This race becomes more common the
/// faster we unload. Also, the test is poorly named.
class UnitWopiOwnertermination : public WopiTestServer
{
    STATE_ENUM(Phase, Start, Load, WaitLoadStatus, WaitModifiedStatus, WaitDocClose, Done) _phase;

    int _loadedIndex; ///< The connection index that is loaded now.

public:
    UnitWopiOwnertermination()
        : WopiTestServer("UnitWOPIOwnerTermination")
        , _phase(Phase::Start)
        , _loadedIndex(0)
    {
        setTimeout(1min);
    }

    std::unique_ptr<http::Response>
    assertPutFileRequest(const Poco::Net::HTTPRequest& /*request*/) override
    {
        LOK_ASSERT_STATE(_phase, Phase::WaitDocClose);

        // Load again, while we are still unloading.
        TRANSITION_STATE(_phase, Phase::Load);

        return nullptr;
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("Loaded #" << (_loadedIndex + 1) << ": [" << message << ']');

        TRANSITION_STATE(_phase, Phase::WaitModifiedStatus);

        // Modify the document.
        TST_LOG("Modifying");
        WSD_CMD_BY_CONNECTION_INDEX(_loadedIndex, "key type=input char=97 key=0");
        WSD_CMD_BY_CONNECTION_INDEX(_loadedIndex, "key type=up char=0 key=512");

        return true;
    }

    /// The document is modified. Save, modify, and close it.
    bool onDocumentModified(const std::string& message) override
    {
        TST_LOG("Modified #" << (_loadedIndex + 1) << ": [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitModifiedStatus);

        TRANSITION_STATE(_phase, Phase::WaitDocClose);

        TST_LOG("Closing");
        WSD_CMD_BY_CONNECTION_INDEX(_loadedIndex, "closedocument");

        return true;
    }

    bool onDocumentError(const std::string& message) override
    {
        LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);

        LOK_ASSERT_EQUAL_MESSAGE("Expect only documentunloading errors",
                                 std::string("error: cmd=load kind=docunloading"), message);

        TRANSITION_STATE(_phase, Phase::Done);

        return true;
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Start:
            {
                // First time loading, transition.
                TRANSITION_STATE(_phase, Phase::WaitLoadStatus);

                TST_LOG("Creating first connection");
                initWebsocket("/wopi/files/0?access_token=anything");

                TST_LOG("Loading through first connection");
                WSD_CMD_BY_CONNECTION_INDEX(0, "load url=" + getWopiSrc());

                break;
            }
            case Phase::Load:
            {
                TRANSITION_STATE(_phase, Phase::WaitLoadStatus);

                ++_loadedIndex;

                TST_LOG("Creating connection #" << (_loadedIndex + 1));
                addWebSocket();
                TST_LOG("Loading through connection #" << (_loadedIndex + 1));
                WSD_CMD_BY_CONNECTION_INDEX(_loadedIndex, "load url=" + getWopiSrc());

                break;
            }
            case Phase::WaitLoadStatus:
            case Phase::WaitModifiedStatus:
            case Phase::WaitDocClose:
                break;
            case Phase::Done:
                passTest("Reload while unloading failed as expected");
                break;
        }
    }
};

/// Test crashing a document after modifications.
class UnitWOPICrashModified : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitModifiedStatus, WaitDocClose) _phase;

    /// The PID of the Kit process.
    int _pid;

public:
    UnitWOPICrashModified()
        : WopiTestServer("UnitWOPICrashModified")
        , _phase(Phase::Load)
        , _pid(-1)
    {
    }

    void kitSegfault(int /* count */) override { /* ignore */ }

    std::unique_ptr<http::Response> assertPutFileRequest(const Poco::Net::HTTPRequest&) override
    {
        failTest("Unexpected PutFile when there should be no file on disk to upload");

        return nullptr;
    }

    void onDocBrokerAttachKitProcess(const std::string& docBroker, int pid) override
    {
        TST_LOG("DocBroker [" << docBroker << "] attached to pid: " << pid);
        _pid = pid;
    }

    /// The document is loaded.
    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("Got: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);

        TRANSITION_STATE(_phase, Phase::WaitModifiedStatus);

        TST_LOG("Modifying");
        WSD_CMD("key type=input char=97 key=0");
        WSD_CMD("key type=up char=0 key=512");

        return true;
    }

    bool onDocumentModified(const std::string& message) override
    {
        TST_LOG("Got: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitModifiedStatus);

        TRANSITION_STATE(_phase, Phase::WaitDocClose);

        TST_LOG("Killing Kit with PID " << _pid);
        if (kill(_pid, SIGKILL) == -1)
        {
            const int onrre = errno;
            TST_LOG("kill(" << _pid << ", SIGKILL) failed: " << Util::symbolicErrno(onrre) << ": "
                            << std::strerror(onrre));
        }

        return true;
    }

    bool onDataLoss(const std::string& reason) override
    {
        LOK_ASSERT_STATE(_phase, Phase::WaitDocClose);
        passTest("Finished with the data-loss check: " + reason);
        return failed();
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                TRANSITION_STATE(_phase, Phase::WaitLoadStatus);

                TST_LOG("Load: initWebsocket.");
                initWebsocket("/wopi/files/0?access_token=anything");
                WSD_CMD("load url=" + getWopiSrc());
                break;
            }
            case Phase::WaitLoadStatus:
            case Phase::WaitModifiedStatus:
            case Phase::WaitDocClose:
            {
                // just wait for the results
                break;
            }
        }
    }
};

/// Test Saving and Async uploading after modifying and immediately closing.
/// We modify the document and close immediately.
class UnitWOPIAsyncUpload_ModifyClose : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitPutFile, WaitDestroy) _phase;

public:
    UnitWOPIAsyncUpload_ModifyClose()
        : WopiTestServer("UnitWOPIAsyncUpload_ModifyClose")
        , _phase(Phase::Load)
    {
    }

    std::unique_ptr<http::Response>
    assertPutFileRequest(const Poco::Net::HTTPRequest& request) override
    {
        LOK_ASSERT_STATE(_phase, Phase::WaitPutFile);

        // Expect PutFile after closing, since the document is modified.
        if (_phase == Phase::WaitPutFile)
        {
            // The document is modified.
            LOK_ASSERT_EQUAL_STR("true", request.get("X-COOL-WOPI-IsModifiedByUser"));
            LOK_ASSERT_EQUAL(false, request.has("X-LOOL-WOPI-IsModifiedByUser"));

            // Triggered manually or during closing, not auto-save.
            LOK_ASSERT_EQUAL_STR("false", request.get("X-COOL-WOPI-IsAutosave"));
            LOK_ASSERT_EQUAL(false, request.has("X-LOOL-WOPI-IsAutosave"));

            TRANSITION_STATE(_phase, Phase::WaitDestroy);

            return std::make_unique<http::Response>(http::StatusCode::OK);
        }

        // This during closing the document.
        TST_LOG("assertPutFileRequest: unexpected");
        failTest("PutFile multiple times.");

        return nullptr;
    }

    /// The document is loaded.
    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);

        TRANSITION_STATE(_phase, Phase::WaitPutFile);

        WSD_CMD("key type=input char=97 key=0");
        WSD_CMD("key type=up char=0 key=512");
        WSD_CMD("closedocument");

        return true;
    }

    // Wait for clean unloading.
    void onDocBrokerDestroy(const std::string& docKey) override
    {
        TST_LOG("Destroyed dockey [" << docKey << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitDestroy);

        passTest("Document unloaded as expected.");
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                TRANSITION_STATE(_phase, Phase::WaitLoadStatus);

                TST_LOG("Load: initWebsocket.");
                initWebsocket("/wopi/files/0?access_token=anything");

                WSD_CMD("load url=" + getWopiSrc());
                break;
            }
            case Phase::WaitLoadStatus:
            case Phase::WaitPutFile:
            case Phase::WaitDestroy:
                break;
        }
    }
};

class UnitStorage : public WopiTestServer
{
    STATE_ENUM(Phase,
               Load, // load the document
               Filter, // throw filter exception
               Reload, // re-load the document
               Done)
    _phase;

public:
    UnitStorage()
        : WopiTestServer("UnitStorage")
        , _phase(Phase::Load)
    {
    }

    bool filterCheckDiskSpace(const std::string& /* path */, bool& newResult) override
    {
        // Fail the disk-space check in Filter phase.
        newResult = _phase != Phase::Filter;
        TST_LOG("Result: " << (newResult ? "success" : "out-of-disk-space"));
        return true;
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("Loaded: [" << message << ']');

        LOK_ASSERT_STATE(_phase, Phase::Done);
        passTest("Loaded successfully");

        return true;
    }

    bool onDocumentError(const std::string& message) override
    {
        // This may trigger multiple times.
        LOK_ASSERT_MESSAGE("Expect only diskfull errors",
                           message.starts_with("error: cmd=internal kind=diskfull"));

        LOK_ASSERT_STATE(_phase, Phase::Filter);

        return true;
    }

    // When we fail to load, we must destroy the DocBroker.
    void onDocBrokerDestroy(const std::string&) override
    {
        if (_phase == Phase::Filter)
        {
            TRANSITION_STATE(_phase, Phase::Reload);
        }
    }

    void invokeWSDTest() override
    {
        LOG_TRC("invokeWSDTest: " << name(_phase));
        switch (_phase)
        {
            case Phase::Load:
                TRANSITION_STATE(_phase, Phase::Filter);
                initWebsocket("/wopi/files/0?access_token=anything");
                WSD_CMD("load url=" + getWopiSrc());
                break;
            case Phase::Filter:
                break;
            case Phase::Reload:
                TST_LOG("Reloading the document");
                TRANSITION_STATE(_phase, Phase::Done);
                initWebsocket("/wopi/files/0?access_token=anything");
                WSD_CMD("load url=" + getWopiSrc());
                break;
            case Phase::Done:
                break;
        }
    }
};

UnitBase** unit_create_wsd_multi(void)
{
    return new UnitBase* []
    {
        new UnitWopiOwnertermination(), new UnitWOPICrashModified(),
            new UnitWOPIAsyncUpload_ModifyClose(), new UnitStorage(), nullptr
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
