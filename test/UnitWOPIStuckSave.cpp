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
#include <wsd/ClientSession.hpp>

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

/// The only editor modifies the document, then the host rejects its token on a lock refresh,
/// which makes its view read-only. The document is then left idle, still modified. The token is
/// the only one there is, so it is sent anyway, once, to upload the document before unloading.
class UnitWOPIIdleRejectedToken : public WopiTestServer
{
public:
    STATE_ENUM(Scenario,
               UploadFails, ///< The host refuses the upload with the rejected token too.
               UploadSucceeds ///< The host takes the upload with the rejected token.
    );

private:
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitModifiedStatus, RefuseRefresh, WaitUnload, Done)
    _phase;

    const Scenario _scenario;

    /// Saves after the token was rejected before we call it a loop.
    static constexpr std::size_t MaxSaves = 3;

    std::atomic<std::size_t> _saves;
    std::size_t _uploads;
    bool _dataLoss;

public:
    UnitWOPIIdleRejectedToken(const std::string& name, Scenario scenario)
        : WopiTestServer(name)
        , _phase(Phase::Load)
        , _scenario(scenario)
        , _saves(0)
        , _uploads(0)
        , _dataLoss(false)
    {
        setTimeout(120s);
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);

        config.setUInt("per_document.idle_timeout_secs", 3);
        config.setInt("storage.wopi.locking.refresh", 1);

        // The test client never answers a request for a new token.
        config.setUInt("storage.wopi.access_token.refresh_timeout_secs", 2);
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& /*request*/,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        fileInfo->set("SupportsLocks", "true");
    }

    std::unique_ptr<http::Response>
    assertLockRequest(const Poco::Net::HTTPRequest& request) override
    {
        const std::string op = request.get("X-WOPI-Override", std::string());
        TST_LOG(op << " request in " << name(_phase));

        if (op == "LOCK" && _phase == Phase::RefuseRefresh)
        {
            TST_LOG("Rejecting the editor's token");
            TRANSITION_STATE(_phase, Phase::WaitUnload);
            return std::make_unique<http::Response>(http::StatusCode::Unauthorized);
        }

        return nullptr;
    }

    std::unique_ptr<http::Response>
    assertPutFileRequest(const Poco::Net::HTTPRequest& request) override
    {
        ++_uploads;
        TST_LOG("PutFile #" << _uploads << " in " << name(_phase) << ": " << request.getURI());
        LOK_ASSERT_STATE(_phase, Phase::WaitUnload);

        LOK_ASSERT_MESSAGE("Expected a single upload with the rejected token", _uploads == 1);
        LOK_ASSERT_MESSAGE("Expected the upload to carry the token",
                           request.getURI().find("access_token=anything") != std::string::npos);
        LOK_ASSERT_EQUAL_STR("Bearer anything", request.get("Authorization", std::string()));

        if (_scenario == Scenario::UploadFails)
            return std::make_unique<http::Response>(http::StatusCode::Unauthorized);

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
            TRANSITION_STATE_MSG(_phase, Phase::RefuseRefresh, "Waiting for the lock refresh");

        return true;
    }

    bool onFilterLOKitMessage(const std::shared_ptr<Message>& message) override
    {
        if (_phase == Phase::WaitUnload && message->contains(".uno:Save"))
        {
            const std::size_t saves = ++_saves;
            TST_LOG("Save #" << saves << " after the token was rejected: ["
                             << message->firstLine() << ']');
            if (saves > MaxSaves)
            {
                failTest("Saved the document " + std::to_string(saves) +
                         " times after the token was rejected");
            }
        }

        return false;
    }

    bool onDataLoss(const std::string& reason) override
    {
        TST_LOG("onDataLoss: " << reason);
        if (_scenario == Scenario::UploadSucceeds)
        {
            failTest("Unexpected data loss after the upload was taken: " + reason);
            return failed();
        }

        LOK_ASSERT_STATE(_phase, Phase::WaitUnload);
        _dataLoss = true;

        // The loss is expected here. The test ends when the document is destroyed, so that any
        // upload while the document shuts down is counted too.
        return false;
    }

    void onDocBrokerDestroy(const std::string& docKey) override
    {
        TST_LOG("Destroyed dockey [" << docKey << "] in " << name(_phase));
        LOK_ASSERT_STATE(_phase, Phase::WaitUnload);
        TRANSITION_STATE(_phase, Phase::Done);

        LOK_ASSERT_EQUAL_MESSAGE("Expected one upload with the rejected token", std::size_t(1),
                                 _uploads);
        if (_scenario == Scenario::UploadFails)
        {
            LOK_ASSERT_MESSAGE("Expected the unsaved changes to be reported lost", _dataLoss);
            passTest("Gave up on the idle document after the upload with the rejected token "
                     "failed");
        }
        else
        {
            passTest("Uploaded the idle document with the rejected token and unloaded it");
        }
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
            case Phase::RefuseRefresh:
            case Phase::WaitUnload:
            case Phase::Done:
                break;
        }
    }
};

/// The editor modifies the document and saves, and the host answers the upload with 413 (too
/// large). That makes every view read-only, the editor's too, so the unsaved changes are left
/// with views that cannot upload them. Once idle, the document is unloaded and the changes are
/// reported lost.
class UnitWOPIIdleViewOnly : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitViews, WaitModifiedStatus, WaitUpload, WaitUnload, Done)
    _phase;

    /// How long the document may stay loaded after the refused upload: several times the idle
    /// timeout, with room for the save and upload retries.
    static constexpr std::chrono::seconds MaxUnloadDuration = std::chrono::seconds(20);

    std::size_t _checkFileInfoCount;
    std::size_t _viewCount;
    std::size_t _uploads;
    bool _dataLoss;
    std::chrono::steady_clock::time_point _refusedTime;

public:
    UnitWOPIIdleViewOnly()
        : WopiTestServer("UnitWOPIIdleViewOnly")
        , _phase(Phase::Load)
        , _checkFileInfoCount(0)
        , _viewCount(0)
        , _uploads(0)
        , _dataLoss(false)
    {
        setTimeout(120s);
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);

        config.setUInt("per_document.idle_timeout_secs", 3);
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& /*request*/,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        // The first session is the editor, the second a viewer.
        const bool editor = _checkFileInfoCount == 0;
        ++_checkFileInfoCount;
        TST_LOG("CheckFileInfo #" << _checkFileInfoCount << ": " << (editor ? "editor" : "viewer"));

        if (!editor)
            fileInfo->set("UserCanWrite", "false");
    }

    std::unique_ptr<http::Response>
    assertPutFileRequest(const Poco::Net::HTTPRequest& /*request*/) override
    {
        ++_uploads;
        TST_LOG("PutFile #" << _uploads << " in " << name(_phase));
        LOK_ASSERT_STATE(_phase, Phase::WaitUpload);

        TST_LOG("Refusing the upload as too large");
        _refusedTime = std::chrono::steady_clock::now();
        TRANSITION_STATE(_phase, Phase::WaitUnload);
        return std::make_unique<http::Response>(http::StatusCode::PayloadTooLarge);
    }

    void onDocBrokerViewLoaded(const std::string&,
                               const std::shared_ptr<ClientSession>& session) override
    {
        ++_viewCount;
        TST_LOG("View #" << _viewCount << " [" << session->getName() << "] loaded");

        if (_viewCount == 2)
        {
            TRANSITION_STATE(_phase, Phase::WaitModifiedStatus);

            TST_LOG("Modifying (editor)");
            WSD_CMD_BY_CONNECTION_INDEX(0, "key type=input char=97 key=0");
            WSD_CMD_BY_CONNECTION_INDEX(0, "key type=up char=0 key=512");
        }
    }

    bool onDocumentModified(const std::string& message) override
    {
        TST_LOG("onDocumentModified: [" << message << ']');
        if (_phase == Phase::WaitModifiedStatus)
        {
            TRANSITION_STATE_MSG(_phase, Phase::WaitUpload, "Saving (editor), expecting PutFile");
            WSD_CMD_BY_CONNECTION_INDEX(0, "save dontTerminateEdit=0 dontSaveIfUnmodified=0");
        }

        return true;
    }

    bool onDataLoss(const std::string& reason) override
    {
        TST_LOG("onDataLoss: " << reason);
        LOK_ASSERT_STATE(_phase, Phase::WaitUnload);
        _dataLoss = true;

        // The loss is expected here. The test ends when the document is destroyed.
        return false;
    }

    void onDocBrokerDestroy(const std::string& docKey) override
    {
        TST_LOG("Destroyed dockey [" << docKey << "] in " << name(_phase));
        LOK_ASSERT_STATE(_phase, Phase::WaitUnload);
        TRANSITION_STATE(_phase, Phase::Done);

        LOK_ASSERT_MESSAGE("Expected the unsaved changes to be reported lost", _dataLoss);
        passTest("Unloaded the idle document that no view could upload");
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                TRANSITION_STATE(_phase, Phase::WaitViews);

                TST_LOG("Creating the editor and viewer connections");
                initWebsocket("/wopi/files/0?access_token=anything");
                addWebSocket();

                WSD_CMD_BY_CONNECTION_INDEX(0, "load url=" + getWopiSrc());
                WSD_CMD_BY_CONNECTION_INDEX(1, "load url=" + getWopiSrc());
                break;
            }
            case Phase::WaitUnload:
            {
                if (std::chrono::steady_clock::now() - _refusedTime >= MaxUnloadDuration)
                {
                    failTest("The idle document is still loaded " +
                             std::to_string(MaxUnloadDuration.count()) +
                             " seconds after the upload was refused as too large, with no view "
                             "able to upload");
                }
                break;
            }
            case Phase::WaitViews:
            case Phase::WaitModifiedStatus:
            case Phase::WaitUpload:
            case Phase::Done:
                break;
        }
    }
};

UnitBase** unit_create_wsd_multi(void)
{
    return new UnitBase* [] { new UnitWOPIStuckSave(), new UnitWOPIInfiniteSave(),
                              new UnitWOPIIdleFailingSave(),
                              new UnitWOPIIdleRejectedToken(
                                  "UnitWOPIIdleRejectedTokenUploadFails",
                                  UnitWOPIIdleRejectedToken::Scenario::UploadFails),
                              new UnitWOPIIdleRejectedToken(
                                  "UnitWOPIIdleRejectedTokenUploadSucceeds",
                                  UnitWOPIIdleRejectedToken::Scenario::UploadSucceeds),
                              new UnitWOPIIdleViewOnly(),
                              nullptr };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
