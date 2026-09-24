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
 * Unit test for WOPI stuck save scenarios.
 */

#include <config.h>

#include <common/Message.hpp>
#include <common/Unit.hpp>
#include <net/HttpRequest.hpp>
#include <test/WopiTestServer.hpp>
#include <test/helpers.hpp>
#include <test/lokassert.hpp>

#include <Poco/Net/HTTPRequest.h>
#include <Poco/Util/LayeredConfiguration.h>

#include <atomic>
#include <chrono>
#include <string>
#include <thread>

using namespace std::literals;

/// Test saving with simulated failing.
/// We modify the document and close.
/// The document must then be saved, but
/// the save notification is consumed in
/// the test and never reaches the DocBroker.
class UnitWOPIStuckSave : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitModifiedStatus, WaitClose)
    _phase;

public:
    UnitWOPIStuckSave()
        : WopiTestServer("UnitWOPIStuckSave")
        , _phase(Phase::Load)
    {
        // We need more time to retry saving.
        setTimeout(200s);
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);

        // Small value to shorten the test run time.
        config.setUInt("per_document.limit_store_failures", 2);
        config.setBool("per_document.always_save_on_exit", true);
    }

    std::unique_ptr<http::Response>
    assertPutFileRequest(const Poco::Net::HTTPRequest& /*request*/) override
    {
        LOK_ASSERT_FAIL("Unexpected PutFile");

        return nullptr;
    }

    /// The document is loaded.
    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);

        TRANSITION_STATE(_phase, Phase::WaitModifiedStatus);

        WSD_CMD("key type=input char=97 key=0");
        WSD_CMD("key type=up char=0 key=512");

        return true;
    }

    /// The document is modified. Save it.
    bool onDocumentModified(const std::string& message) override
    {
        TST_LOG("onDocumentModified: Doc (WaitModifiedStatus): [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitModifiedStatus);

        TRANSITION_STATE(_phase, Phase::WaitClose);

        TST_LOG("Closing the document, expecting saving, which will get 'stuck'");
        WSD_CMD("closedocument");

        return true;
    }

    bool onFilterLOKitMessage(const std::shared_ptr<Message>& message) override
    {
        TST_LOG("Filtering: [" << message->firstLine() << ']');

        constexpr std::string_view unoSave = ".uno:Save";
        constexpr std::string_view unoModifiedStatus = ".uno:ModifiedStatus";
        if (message->contains(unoSave))
        {
            TST_LOG("Dropping .uno:Save to simulate stuck save");
            return true; // Do not process the message further.
        }
        else if (message->contains(unoModifiedStatus))
        {
            if (message->tokens().size() > 1)
            {
                StringVector stateTokens(StringVector::tokenize(message->tokens()[1], '='));
                if (stateTokens.size() == 2 && stateTokens.equals(0, ".uno:ModifiedStatus"))
                {
                    // Filter out all the ModifiedStatus=false messages.
                    // This will leave the doc modified.
                    return !stateTokens.equals(1, "true");
                }
            }
        }

        return false;
    }

    bool onDataLoss(const std::string& reason) override
    {
        LOK_ASSERT_STATE(_phase, Phase::WaitClose);
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

                TST_LOG("Load: initWebsocket");
                initWebsocket("/wopi/files/0?access_token=anything");

                WSD_CMD("load url=" + getWopiSrc());
                break;
            }
            case Phase::WaitLoadStatus:
                break;
            case Phase::WaitModifiedStatus:
                break;
            case Phase::WaitClose:
                break;
        }
    }
};

class UnitWOPIInfiniteSave : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitModifiedStatus, WaitFail)
    _phase;

    std::size_t _versions;

public:
    UnitWOPIInfiniteSave()
        : WopiTestServer("UnitWOPIInfiniteSave")
        , _phase(Phase::Load)
        , _versions(0)
    {
        // We need more time to retry saving.
        setTimeout(200s);
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);

        // Small value to shorten the test run time.
        config.setUInt("per_document.limit_store_failures", 2);
        config.setBool("per_document.always_save_on_exit", true);
    }

    std::unique_ptr<http::Response>
    assertPutFileRequest(const Poco::Net::HTTPRequest& /*request*/) override
    {
        // Simulate slow upload so the subsequent one overlaps with it.
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        return nullptr;
    }

    /// The document is loaded.
    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);

        TRANSITION_STATE(_phase, Phase::WaitModifiedStatus);

        WSD_CMD("key type=input char=97 key=0");
        WSD_CMD("key type=up char=0 key=512");

        return true;
    }

    /// The document is modified. Save it.
    bool onDocumentModified(const std::string& message) override
    {
        TST_LOG("onDocumentModified: Doc (WaitModifiedStatus): [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitModifiedStatus);

        TRANSITION_STATE(_phase, Phase::WaitFail);
        WSD_CMD("save dontTerminateEdit=0 dontSaveIfUnmodified=0");

        return true;
    }

    bool onDocumentSaved(const std::string& message, bool success,
                         [[maybe_unused]] const std::string& result) override
    {
        TST_LOG("Save result: " << message);

        LOK_ASSERT_MESSAGE("Expected save to succeed", success);
        if (++_versions >= 6)
        {
            passTest("Saved " + std::to_string(_versions) + " versions without errors");
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

                TST_LOG("Load: initWebsocket");
                initWebsocket("/wopi/files/0?access_token=anything");

                WSD_CMD("load url=" + getWopiSrc());
                break;
            }
            case Phase::WaitLoadStatus:
            case Phase::WaitModifiedStatus:
                break;
            case Phase::WaitFail:
            {
                const std::string res = helpers::getResponseString(
                    getWs()->getWebSocket(), "error:", getTestname(), std::chrono::milliseconds(1));
                if (!res.empty())
                {
                    failTest("Unexpected to get save failure: " + res);
                }

                WSD_CMD("save dontTerminateEdit=0 dontSaveIfUnmodified=0");
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                break;
            }
        }
    }
};

/// A modified document that is left open, idle, with every save failing. After the configured
/// number of failed saves it is unloaded, the same as a document being closed, instead of
/// trying to save for as long as the view stays connected.
class UnitWOPIIdleFailingSave : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitModifiedStatus, WaitUnload, Done)
    _phase;

    static constexpr std::size_t LimitStoreFailures = 2;

    /// Saves that may fail on top of the limit before we call it a loop. A failed save and the
    /// wait before the next one take over half a minute, so one keeps the failing run short.
    static constexpr std::size_t ExtraFailures = 1;

    std::atomic<std::size_t> _failedSaves;

public:
    UnitWOPIIdleFailingSave()
        : WopiTestServer("UnitWOPIIdleFailingSave")
        , _phase(Phase::Load)
        , _failedSaves(0)
    {
        // A failed save and the wait before the next one take over half a minute.
        setTimeout(180s);
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);

        config.setUInt("per_document.idle_timeout_secs", 3);
        config.setUInt("per_document.limit_store_failures", LimitStoreFailures);
    }

    std::unique_ptr<http::Response>
    assertPutFileRequest(const Poco::Net::HTTPRequest& /*request*/) override
    {
        LOK_ASSERT_FAIL("Unexpected PutFile, as no save succeeds");

        return nullptr;
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);

        TRANSITION_STATE(_phase, Phase::WaitModifiedStatus);

        WSD_CMD("key type=input char=97 key=0");
        WSD_CMD("key type=up char=0 key=512");

        return true;
    }

    bool onDocumentModified(const std::string& message) override
    {
        TST_LOG("onDocumentModified: [" << message << ']');
        if (_phase == Phase::WaitModifiedStatus)
        {
            // The view stays connected and does nothing more, so the document goes idle.
            TRANSITION_STATE_MSG(_phase, Phase::WaitUnload,
                                 "Leaving the document idle, expecting failed saves");
        }

        return true;
    }

    bool onFilterLOKitMessage(const std::shared_ptr<Message>& message) override
    {
        if (message->contains(".uno:Save"))
        {
            // The save result never arrives, so the save times out and counts as failed. The
            // count stops once the document has unloaded.
            if (_phase != Phase::WaitUnload)
                return true;

            const std::size_t failed = ++_failedSaves;
            TST_LOG("Dropping save result #" << failed << ": [" << message->firstLine() << ']');
            if (failed > LimitStoreFailures + ExtraFailures)
            {
                failTest("Still saving the idle document after " + std::to_string(failed) +
                         " failed saves, with limit_store_failures of " +
                         std::to_string(LimitStoreFailures));
            }

            return true;
        }

        if (message->contains(".uno:ModifiedStatus") && message->tokens().size() > 1)
        {
            // Keep the document modified.
            StringVector stateTokens(StringVector::tokenize(message->tokens()[1], '='));
            if (stateTokens.size() == 2 && stateTokens.equals(0, ".uno:ModifiedStatus"))
                return !stateTokens.equals(1, "true");
        }

        return false;
    }

    bool onDataLoss(const std::string& reason) override
    {
        TST_LOG("onDataLoss: " << reason);
        LOK_ASSERT_STATE(_phase, Phase::WaitUnload);
        TRANSITION_STATE(_phase, Phase::Done);

        LOK_ASSERT_MESSAGE("Expected the saves to fail up to the limit before unloading",
                           _failedSaves >= LimitStoreFailures);
        passTest("Unloaded the idle document after " + std::to_string(_failedSaves) +
                 " failed saves");
        return failed();
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                TRANSITION_STATE(_phase, Phase::WaitLoadStatus);

                TST_LOG("Load: initWebsocket");
                initWebsocket("/wopi/files/0?access_token=anything");

                WSD_CMD("load url=" + getWopiSrc());
                break;
            }
            case Phase::WaitLoadStatus:
            case Phase::WaitModifiedStatus:
            case Phase::WaitUnload:
            case Phase::Done:
                break;
        }
    }
};

UnitBase** unit_create_wsd_multi(void)
{
    return new UnitBase* [] { new UnitWOPIStuckSave(), new UnitWOPIInfiniteSave(),
                              new UnitWOPIIdleFailingSave(), nullptr };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
