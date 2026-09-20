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
 * Unit test for WOPI document locking functionality.
 */

#include <config.h>

#include <lokassert.hpp>
#include <Unit.hpp>
#include <WopiTestServer.hpp>
#include <common/Log.hpp>
#include <helpers.hpp>
#include <wsd/ClientSession.hpp>

#include <Poco/Net/HTTPRequest.h>

#include <chrono>

/// This is to test that we unlock before unloading the last editor.
class UnitWopiLock : public WopiTestServer
{
    STATE_ENUM(Phase, Load, Lock, Unlock, Done) _phase;

    std::string _lockState;
    std::string _lockToken;
    std::size_t _checkFileInfoCount;
    std::size_t _viewCount;

public:
    UnitWopiLock()
        : WopiTestServer("UnitWopiLock")
        , _phase(Phase::Load)
        , _lockState("UNLOCK")
        , _checkFileInfoCount(0)
        , _viewCount(0)
    {
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& /*request*/,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        // Make the first session the editor, subsequent ones read-only.
        const bool firstView = _checkFileInfoCount == 0;
        ++_checkFileInfoCount;

        TST_LOG("CheckFileInfo: " << (firstView ? "editor" : "viewer"));

        fileInfo->set("SupportsLocks", "true");
        fileInfo->set("UserCanWrite", firstView ? "true" : "false");

        // An extension that doesn't allow commenting. By omitting this,
        // the test fails because we allow commenting and consider the
        // document editable, and don't unlock when the editor disconnects.
        fileInfo->set("BaseFileName", "doc.odt");
    }

    std::unique_ptr<http::Response>
    assertLockRequest(const Poco::Net::HTTPRequest& request) override
    {
        const std::string lockToken = request.get("X-WOPI-Lock", std::string());
        const std::string newLockState = request.get("X-WOPI-Override", std::string());
        TST_LOG("In " << name(_phase) << ", X-WOPI-Lock: " << lockToken << ", X-WOPI-Override: "
                      << newLockState << ", for URI: " << request.getURI());

        if (_phase == Phase::Lock)
        {
            LOK_ASSERT_EQUAL_MESSAGE("Expected X-WOPI-Override:LOCK", std::string("LOCK"),
                                     newLockState);
            LOK_ASSERT_MESSAGE("Lock token cannot be empty", !lockToken.empty());
            _lockState = newLockState;
            _lockToken = lockToken;
        }
        else if (_phase == Phase::Unlock)
        {
            LOK_ASSERT_EQUAL_MESSAGE("Expected X-WOPI-Override:UNLOCK", std::string("UNLOCK"),
                                     newLockState);
            LOK_ASSERT_EQUAL_MESSAGE("Document is not locked", std::string("LOCK"), _lockState);
            LOK_ASSERT_EQUAL_MESSAGE("The lock token has changed", _lockToken, lockToken);

            TRANSITION_STATE(_phase, Phase::Done);
            exitTest(TestResult::Ok);
        }
        else
        {
            LOK_ASSERT_FAIL("Unexpected lock-state change while in " << name(_phase));
        }

        return nullptr; // Success.
    }

    void onDocBrokerViewLoaded(const std::string&,
                               const std::shared_ptr<ClientSession>& session) override
    {
        TST_LOG("View #" << _viewCount + 1 << " [" << session->getName() << "] loaded");

        ++_viewCount;
        if (_viewCount == 2)
        {
            // Transition before disconnecting.
            TRANSITION_STATE(_phase, Phase::Unlock);

            // force kill the session with edit permission
            TST_LOG("Disconnecting first connection with edit permission");
            deleteSocketAt(0);
        }
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                // Always transition before issuing commands.
                TRANSITION_STATE(_phase, Phase::Lock);

                TST_LOG("Creating first connection");
                initWebsocket("/wopi/files/0?access_token=anything");

                TST_LOG("Creating second connection");
                addWebSocket();

                TST_LOG("Loading first view (editor)");
                WSD_CMD_BY_CONNECTION_INDEX(0, "load url=" + getWopiSrc());
                TST_LOG("Loading second view (viewer)");
                WSD_CMD_BY_CONNECTION_INDEX(1, "load url=" + getWopiSrc());
                break;
            }
            case Phase::Lock:
            case Phase::Unlock:
                break;
            case Phase::Done:
            {
                // just wait for the results
                break;
            }
        }
    }
};

/// This is to test that we unlock before unloading the last editor
/// when the first view is read-only.
class UnitWopiLockReadOnly : public WopiTestServer
{
    STATE_ENUM(Phase, Connect, FirstCheckFileInfo, Lock, LoadViewer, WaitModify, Upload, Unlock,
               WaitUnload, Done)
    _phase;

    std::string _lockState;
    std::string _lockToken;
    std::size_t _checkFileInfoCount;
    std::size_t _viewCount;

public:
    UnitWopiLockReadOnly()
        : WopiTestServer("UnitWopiLockReadOnly")
        , _phase(Phase::Connect)
        , _lockState("UNLOCK")
        , _checkFileInfoCount(0)
        , _viewCount(0)
    {
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& /*request*/,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        // Make the first session the editor, subsequent ones read-only.
        ++_checkFileInfoCount;

        const bool firstView = _checkFileInfoCount == 1;

        TST_LOG("CheckFileInfo: " << (firstView ? "viewer" : "editor"));

        fileInfo->set("SupportsLocks", "true");
        fileInfo->set("UserCanWrite", firstView ? "false" : "true");

        // An extension that doesn't allow commenting. By omitting this,
        // the test fails because we allow commenting and consider the
        // document editable, and don't unlock when the editor disconnects.
        fileInfo->set("BaseFileName", "doc.odt");
    }

    std::unique_ptr<http::Response>
    assertLockRequest(const Poco::Net::HTTPRequest& request) override
    {
        const std::string lockToken = request.get("X-WOPI-Lock", std::string());
        const std::string newLockState = request.get("X-WOPI-Override", std::string());
        TST_LOG("In " << name(_phase) << ", X-WOPI-Lock: " << lockToken << ", X-WOPI-Override: "
                      << newLockState << ", for URI: " << request.getURI());

        LOG_ASSERT_MSG(_checkFileInfoCount == 2, "Must have had two CheckFileInfo requests");

        if (_phase == Phase::Lock)
        {
            LOK_ASSERT_EQUAL_MESSAGE("Expected X-WOPI-Override:LOCK", std::string("LOCK"),
                                     newLockState);
            LOK_ASSERT_MESSAGE("Lock token cannot be empty", !lockToken.empty());
            _lockState = newLockState;
            _lockToken = lockToken;
        }
        else if (_phase == Phase::Unlock)
        {
            LOK_ASSERT_EQUAL_MESSAGE("Expected X-WOPI-Override:UNLOCK", std::string("UNLOCK"),
                                     newLockState);
            LOK_ASSERT_EQUAL_MESSAGE("Document is not locked", std::string("LOCK"), _lockState);
            LOK_ASSERT_EQUAL_MESSAGE("The lock token has changed", _lockToken, lockToken);

            TRANSITION_STATE(_phase, Phase::WaitUnload);
            // exitTest(TestResult::Ok);
        }
        else
        {
            LOK_ASSERT_FAIL("Unexpected lock-state change while in " << name(_phase));
        }

        return nullptr; // Success.
    }

    std::unique_ptr<http::Response>
    assertPutFileRequest(const Poco::Net::HTTPRequest& request) override
    {
        LOK_ASSERT_STATE(_phase, Phase::Upload);
        TRANSITION_STATE(_phase, Phase::Unlock);

        // The document is modified.
        LOK_ASSERT_EQUAL_STR("true", request.get("X-COOL-WOPI-IsModifiedByUser"));
        LOK_ASSERT_EQUAL(false, request.has("X-LOOL-WOPI-IsModifiedByUser"));

        // Triggered manually or during closing, not auto-save.
        LOK_ASSERT_EQUAL_STR("false", request.get("X-COOL-WOPI-IsAutosave"));
        LOK_ASSERT_EQUAL(false, request.has("X-LOOL-WOPI-IsAutosave"));

        // The only editor goes away.
        // LOK_ASSERT_EQUAL_STR("true", request.get("X-COOL-WOPI-IsExitSave"));
        // LOK_ASSERT_EQUAL_STR("true", request.get("X-LOOL-WOPI-IsExitSave"));

        // Simulate the viewer closing browser.
        TST_LOG("Disconnecting Viewer");
        deleteSocketAt(0);

        return nullptr; // Success.
    }

    void onDocBrokerViewLoaded(const std::string&,
                               const std::shared_ptr<ClientSession>& session) override
    {
        TST_LOG("View #" << _viewCount + 1 << " [" << session->getName()
                         << "] loaded, phase: " << name(_phase));

        ++_viewCount;
        if (_viewCount == 1)
        {
            LOK_ASSERT_STATE(_phase, Phase::LoadViewer);
            TRANSITION_STATE(_phase, Phase::Lock);

            TST_LOG("Loading second view (editor)");
            WSD_CMD_BY_CONNECTION_INDEX(1, "load url=" + getWopiSrc());
        }
        else if (_viewCount == 2)
        {
            // Transition before modifying.
            TRANSITION_STATE(_phase, Phase::WaitModify);

            // Modify the doc.
            TST_LOG("Modifying (editor)");
            WSD_CMD_BY_CONNECTION_INDEX(1, "key type=input char=97 key=0");
            WSD_CMD_BY_CONNECTION_INDEX(1, "key type=up char=0 key=512");
        }
    }

    /// The document is modified. Disconnect editor.
    bool onDocumentModified(const std::string& message) override
    {
        TST_LOG("onDocumentModified: [" << message << "], phase: " << name(_phase));

        // We get this twice, skip the second one.
        if (_phase != Phase::Upload)
        {
            LOK_ASSERT_STATE(_phase, Phase::WaitModify);
            TRANSITION_STATE_MSG(_phase, Phase::Upload, "Disconnecting Editor, expecting PutFile");
            // Simulate the editor closing browser.
            deleteSocketAt(1);
        }

        return true;
    }

    void onDocBrokerRemoveSession(const std::string&,
                                  const std::shared_ptr<ClientSession>& session) override
    {
        TST_LOG("Removing session [" << session->getName() << "], phase: " << name(_phase));
        if (_phase == Phase::Unlock)
        {
            // LOK_ASSERT_STATE(_phase, Phase::WaitUnload);
        }
    }

    void onDocBrokerDestroy(const std::string& docKey) override
    {
        TST_LOG("Destroyed dockey [" << docKey << "], phase: " << name(_phase));
        LOK_ASSERT_STATE(_phase, Phase::WaitUnload);

        TRANSITION_STATE(_phase, Phase::Done);
        passTest("No modification or unexpected PutFile on read-only doc");
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Connect:
            {
                // Always transition before issuing commands.
                TRANSITION_STATE_MSG(_phase, Phase::FirstCheckFileInfo,
                                     "Creating first connection, expecting first CheckFileInfo");
                initWebsocket("/wopi/files/0?access_token=anything");

                // With async loading, we download based the initial connection,
                // ahead of the load command. By then, we have done CheckFileInfo
                // and found out that this is an editor, and so must take the lock.
                TRANSITION_STATE_MSG(
                    _phase, Phase::Lock,
                    "Creating second connection, expecting second CheckFileInfo+Lock");
                addWebSocket();

                TRANSITION_STATE_MSG(_phase, Phase::LoadViewer, "Loading viewer");
                WSD_CMD_BY_CONNECTION_INDEX(0, "load url=" + getWopiSrc());
                break;
            }
            case Phase::FirstCheckFileInfo:
            case Phase::Lock:
            case Phase::LoadViewer:
            case Phase::WaitModify:
            case Phase::Upload:
            case Phase::Unlock:
            case Phase::WaitUnload:
            case Phase::Done:
            {
                // just wait for the results
                break;
            }
        }
    }
};

/// This is to test the behavior when locking fails.
class UnitWopiLockFail : public WopiTestServer
{
    STATE_ENUM(Phase, Load, Lock, RefreshLock, Done) _phase;

    std::string _lockState;
    std::string _lockToken;
    std::size_t _lockRefreshCount;
    std::chrono::steady_clock::time_point _refreshTime;

    static constexpr int RefreshPeriodSeconds = 2;

public:
    UnitWopiLockFail()
        : WopiTestServer("UnitWopiLockFail")
        , _phase(Phase::Load)
        , _lockState("UNLOCK")
        , _lockRefreshCount(0)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);

        // Small value to shorten the test run time.
        config.setUInt("storage.wopi.locking.refresh", RefreshPeriodSeconds);
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& /*request*/,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        fileInfo->set("SupportsLocks", "true");
    }

    std::unique_ptr<http::Response>
    assertLockRequest(const Poco::Net::HTTPRequest& request) override
    {
        const std::string lockToken = request.get("X-WOPI-Lock", std::string());
        const std::string newLockState = request.get("X-WOPI-Override", std::string());
        TST_LOG("In " << name(_phase) << ", X-WOPI-Lock: " << lockToken << ", X-WOPI-Override: "
                      << newLockState << ", for URI: " << request.getURI());

        if (_phase == Phase::Lock)
        {
            LOK_ASSERT_EQUAL_MESSAGE("Expected X-WOPI-Override:LOCK", std::string("LOCK"),
                                     newLockState);
            LOK_ASSERT_MESSAGE("Lock token cannot be empty", !lockToken.empty());
            _lockState = newLockState;
            _lockToken = lockToken;

            _refreshTime = std::chrono::steady_clock::now();
            TRANSITION_STATE(_phase, Phase::RefreshLock);
        }
        else if (_phase == Phase::RefreshLock)
        {
            LOK_ASSERT_EQUAL_MESSAGE("Expected X-WOPI-Override:LOCK", std::string("LOCK"),
                                     newLockState);
            LOK_ASSERT_EQUAL_MESSAGE("Document is not locked", std::string("LOCK"), _lockState);
            LOK_ASSERT_EQUAL_MESSAGE("The lock token has changed", _lockToken, lockToken);

            ++_lockRefreshCount;
            LOK_ASSERT_EQUAL_MESSAGE("Lock refresh with expired token", 1UL, _lockRefreshCount);

            return std::make_unique<http::Response>(http::StatusCode::ServiceUnavailable);
        }
        else
        {
            LOK_ASSERT_FAIL("Unexpected lock-state change while in " << name(_phase));
        }

        return nullptr; // Success.
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                // Always transition before issuing commands.
                TRANSITION_STATE(_phase, Phase::Lock);

                TST_LOG("Creating first connection");
                initWebsocket("/wopi/files/0?access_token=anything");

                TST_LOG("Loading first view (editor)");
                WSD_CMD_BY_CONNECTION_INDEX(0, "load url=" + getWopiSrc());
                break;
            }
            case Phase::Lock:
                break;
            case Phase::RefreshLock:
            {
                // Wait for the modified status (and fail) in onDocumentModified.
                // Otherwise, save the document and wait for upload.
                const auto now = std::chrono::steady_clock::now();
                const auto elapsed =
                    std::chrono::duration_cast<std::chrono::milliseconds>(now - _refreshTime);
                if (_lockRefreshCount == 1 &&
                    elapsed >= std::chrono::seconds(RefreshPeriodSeconds * 3))
                {
                    TRANSITION_STATE(_phase, Phase::Done);
                    exitTest(TestResult::Ok);
                }
            }
            break;
            case Phase::Done:
            {
                // just wait for the results
                break;
            }
        }
    }
};

/// This is to test that we do not unlock before uploading
/// the document, right before unloading the last view.
class UnitWopiUnlock : public WopiTestServer
{
    STATE_ENUM(Phase, Load, Lock, Modify, Upload, Unlock, Done) _phase;

    std::string _lockState;
    std::string _lockToken;
    std::size_t _sessionCount;

public:
    UnitWopiUnlock()
        : WopiTestServer("UnitWopiUnlock")
        , _phase(Phase::Load)
        , _lockState("UNLOCK")
        , _sessionCount(0)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);

        // Do *not* refresh.
        config.setUInt("storage.wopi.locking.refresh", 0);
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& /*request*/,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        fileInfo->set("SupportsLocks", "true");
    }

    std::unique_ptr<http::Response>
    assertPutFileRequest(const Poco::Net::HTTPRequest& /*request*/) override
    {
        TST_LOG("assertPutFileRequest");
        LOK_ASSERT_STATE(_phase, Phase::Upload);

        TRANSITION_STATE(_phase, Phase::Unlock);
        return nullptr;
    }

    std::unique_ptr<http::Response>
    assertLockRequest(const Poco::Net::HTTPRequest& request) override
    {
        const std::string lock = request.get("X-WOPI-Lock", std::string());
        const std::string newLockState = request.get("X-WOPI-Override", std::string());
        TST_LOG("In " << name(_phase) << ", X-WOPI-Lock: " << lock << ", X-WOPI-Override: "
                      << newLockState << ", for URI: " << request.getURI());

        if (_phase == Phase::Lock)
        {
            LOK_ASSERT_EQUAL_MESSAGE("Expected X-WOPI-Override:LOCK", std::string("LOCK"),
                                     newLockState);
            LOK_ASSERT_MESSAGE("Lock token cannot be empty", !lock.empty());
            _lockState = newLockState;
            _lockToken = lock;
            TRANSITION_STATE(_phase, Phase::Modify);
        }
        else if (_phase == Phase::Unlock)
        {
            LOK_ASSERT_EQUAL_MESSAGE("Expected X-WOPI-Override:UNLOCK", std::string("UNLOCK"),
                                     newLockState);
            LOK_ASSERT_EQUAL_MESSAGE("Document is not unlocked", std::string("LOCK"), _lockState);
            LOK_ASSERT_EQUAL_MESSAGE("The lock token has changed", _lockToken, lock);

            TRANSITION_STATE(_phase, Phase::Done);
            exitTest(TestResult::Ok);
        }
        else
        {
            LOK_ASSERT_FAIL("Unexpected lock-state change while in " << name(_phase));
        }

        return nullptr; // Success.
    }

    /// Called when a new client session is added to a DocumentBroker.
    void onDocBrokerAddSession(const std::string&,
                               const std::shared_ptr<ClientSession>& session) override
    {
        ++_sessionCount;
        TST_LOG("New Session [" << session->getName() << "] added. Have " << _sessionCount
                                << " sessions.");
    }

    void onDocBrokerViewLoaded(const std::string&,
                               const std::shared_ptr<ClientSession>& session) override
    {
        TST_LOG("View for session [" << session->getName() << "] loaded. Have " << _sessionCount
                                     << " sessions.");
    }

    /// The document is loaded.
    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::Modify);

        // Modify the doc.
        TST_LOG("Modifying");
        WSD_CMD("key type=input char=97 key=0");
        WSD_CMD("key type=up char=0 key=512");

        return true;
    }

    /// The document is modified. Load the viewer session.
    bool onDocumentModified(const std::string& message) override
    {
        TST_LOG("onDocumentModified: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::Modify);

        TRANSITION_STATE(_phase, Phase::Upload);

        TST_LOG("Disconnecting");
        deleteSocketAt(0);

        return true;
    }

    /// Called when a client session is removed to a DocumentBroker.
    void onDocBrokerRemoveSession(const std::string&,
                                  const std::shared_ptr<ClientSession>& session) override
    {
        --_sessionCount;
        TST_LOG("Session [" << session->getName() << "] removed. Have " << _sessionCount
                            << " sessions.");
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                // Always transition before issuing commands.
                TRANSITION_STATE(_phase, Phase::Lock);

                TST_LOG("Creating first connection");
                initWebsocket("/wopi/files/0?access_token=anything");

                TST_LOG("Loading view");
                WSD_CMD_BY_CONNECTION_INDEX(0, "load url=" + getWopiSrc());
                break;
            }
            case Phase::Lock:
            case Phase::Modify:
            case Phase::Upload:
            case Phase::Unlock:
            case Phase::Done:
            {
                // just wait for the results
                break;
            }
        }
    }
};

/// This is to test that when we unload an idle document,
/// we also unlock.
class UnitWopiLockIdle : public WopiTestServer
{
    STATE_ENUM(Phase, Load, Lock, Unlock, Done) _phase;

    std::string _lockState;
    std::string _lockToken;
    std::chrono::steady_clock::time_point _refreshTime;

    static constexpr int IdleTimeoutSeconds = 5;

public:
    UnitWopiLockIdle()
        : WopiTestServer("UnitWopiLockIdle")
        , _phase(Phase::Load)
        , _lockState("UNLOCK")
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);

        // Small value to shorten the test run time.
        config.setUInt("per_document.idle_timeout_secs", IdleTimeoutSeconds);
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& /*request*/,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        fileInfo->set("SupportsLocks", "true");
    }

    std::unique_ptr<http::Response>
    assertLockRequest(const Poco::Net::HTTPRequest& request) override
    {
        const std::string lockToken = request.get("X-WOPI-Lock", std::string());
        const std::string newLockState = request.get("X-WOPI-Override", std::string());
        TST_LOG("In " << name(_phase) << ", X-WOPI-Lock: " << lockToken << ", X-WOPI-Override: "
                      << newLockState << ", for URI: " << request.getURI());

        if (_phase == Phase::Lock)
        {
            LOK_ASSERT_EQUAL_MESSAGE("Expected X-WOPI-Override:LOCK", std::string("LOCK"),
                                     newLockState);
            LOK_ASSERT_MESSAGE("Lock token cannot be empty", !lockToken.empty());
            _lockState = newLockState;
            _lockToken = lockToken;

            _refreshTime = std::chrono::steady_clock::now();
            TRANSITION_STATE(_phase, Phase::Unlock);
        }
        else if (_phase == Phase::Unlock)
        {
            LOK_ASSERT_EQUAL_MESSAGE("Expected X-WOPI-Override:UNLOCK", std::string("UNLOCK"),
                                     newLockState);
            LOK_ASSERT_EQUAL_MESSAGE("Document is not locked", std::string("LOCK"), _lockState);
            LOK_ASSERT_EQUAL_MESSAGE("The lock token has changed", _lockToken, lockToken);

            TRANSITION_STATE(_phase, Phase::Done);
            exitTest(TestResult::Ok);
        }
        else
        {
            LOK_ASSERT_FAIL("Unexpected lock-state change while in " << name(_phase));
        }

        return nullptr; // Success.
    }

    /// The document is loaded.
    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << "] in " << name(_phase));
        // As locking is async, it can race with this loaded event.

        // Simulate some potential user modification.
        // This triggers the "maybe modified" logic.
        TST_LOG("Non-modifying key input");
        WSD_CMD("key type=input char=0 key=16402");

        return true;
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                // Always transition before issuing commands.
                TRANSITION_STATE(_phase, Phase::Lock);

                TST_LOG("Creating first connection");
                initWebsocket("/wopi/files/0?access_token=anything");

                TST_LOG("Loading first view (editor)");
                WSD_CMD_BY_CONNECTION_INDEX(0, "load url=" + getWopiSrc());
                break;
            }
            case Phase::Unlock:
            case Phase::Lock:
                break;
            case Phase::Done:
            {
                // just wait for the results
                break;
            }
        }
    }
};


/// A host too busy to answer a lock refresh has not refused us. We hold the
/// lock already and the lease has most of a refresh period left, so the answer
/// is to ask again shortly - not to take the document away from someone in the
/// middle of editing it by dropping their session to read-only.
class UnitWopiLockRefreshTransient : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoad, WaitRefusedRefresh, WaitAcceptedRefresh, Done) _phase;

    /// How long the host asks us to wait. Short, so the test doesn't idle, and
    /// it is the value under test: without it we would wait far longer.
    static constexpr int RetryAfterSeconds = 1;

    /// Refresh attempts to refuse with 503 before letting one through.
    static constexpr std::size_t RefreshesToRefuse = 2;

    /// LOCK requests seen, of any kind.
    std::size_t _lockCount;

    /// Refresh attempts refused so far.
    std::size_t _refused;

public:
    UnitWopiLockRefreshTransient()
        : WopiTestServer("UnitWopiLockRefreshTransient")
        , _phase(Phase::Load)
        , _lockCount(0)
        , _refused(0)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);

        // Refresh almost immediately, so the test doesn't sit through the
        // fifteen minutes a deployment would wait.
        config.setInt("storage.wopi.locking.refresh", 1);
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& /*request*/,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        fileInfo->set("SupportsLocks", "true");
        fileInfo->set("UserCanWrite", "true");
        fileInfo->set("BaseFileName", "doc.odt");
    }

    std::unique_ptr<http::Response>
    assertLockRequest(const Poco::Net::HTTPRequest& request) override
    {
        const std::string op = request.get("X-WOPI-Override", std::string());
        ++_lockCount;
        TST_LOG("LOCK #" << _lockCount << ": " << op << " in " << name(_phase));

        if (op != "LOCK" || _phase == Phase::WaitLoad)
            return nullptr; // The initial lock, or an unlock on the way out.

        if (_phase == Phase::WaitRefusedRefresh && _refused < RefreshesToRefuse)
        {
            ++_refused;
            TST_LOG("Refusing refresh #" << _refused << " with 503 and Retry-After: "
                                         << RetryAfterSeconds);

            if (_refused == RefreshesToRefuse)
                TRANSITION_STATE(_phase, Phase::WaitAcceptedRefresh);

            auto response =
                std::make_unique<http::Response>(http::StatusCode::ServiceUnavailable);
            response->add("Retry-After", std::to_string(RetryAfterSeconds));
            return response;
        }

        if (_phase == Phase::WaitAcceptedRefresh)
        {
            // We rode out the refusals without losing the lock; the next
            // refresh gets through and the document carries on as before.
            TST_LOG("Accepting the refresh after " << _refused << " refusals");
            TRANSITION_STATE(_phase, Phase::Done);
        }

        return nullptr;
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoad);

        TRANSITION_STATE(_phase, Phase::WaitRefusedRefresh);

        return true;
    }

    bool onFilterSendWebSocketMessage(std::string_view message, const WSOpCode /*code*/,
                                      const bool /*flush*/, int& /*unitReturn*/) override
    {
        // The whole point: a busy host must not cost the user their session.
        if (message.starts_with("lockfailed:"))
            failTest("The session was made read-only over a lock refresh the host was merely "
                     "too busy to answer: " +
                     std::string(message));

        return false;
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                // Transition first: this runs on a timer, and a second pass
                // would open another session and another lock to refresh.
                TRANSITION_STATE(_phase, Phase::WaitLoad);

                TST_LOG("Load: initWebsocket");
                initWebsocket("/wopi/files/0?access_token=anything");
                WSD_CMD("load url=" + getWopiSrc());
                break;
            }
            case Phase::Done:
            {
                LOK_ASSERT_EQUAL_MESSAGE("Expected every refusal to be retried",
                                         RefreshesToRefuse, _refused);
                passTest("Rode out a busy host's refusals without losing the lock");
                break;
            }
            case Phase::WaitLoad:
            case Phase::WaitRefusedRefresh:
            case Phase::WaitAcceptedRefresh:
                break;
        }
    }
};

/// A lock we fail to release stays held until the host expires its lease, and
/// the next person to open the document gets a read-only session for their
/// trouble. A host that is merely busy is worth asking again.
class UnitWopiUnlockRetry : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoad, WaitUnlock, Done) _phase;

    /// Unlocks to refuse with 503 before letting one through. More than the
    /// handful that teardown attempts of its own accord, so that only the
    /// retry under test can get past them.
    static constexpr std::size_t UnlocksToRefuse = 6;

    /// Unlocks refused so far, and whether one has since been accepted.
    std::size_t _refused;
    bool _accepted;

public:
    UnitWopiUnlockRetry()
        : WopiTestServer("UnitWopiUnlockRetry")
        , _phase(Phase::Load)
        , _refused(0)
        , _accepted(false)
    {
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& /*request*/,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        fileInfo->set("SupportsLocks", "true");
        fileInfo->set("UserCanWrite", "true");
        fileInfo->set("BaseFileName", "doc.odt");
    }

    std::unique_ptr<http::Response>
    assertLockRequest(const Poco::Net::HTTPRequest& request) override
    {
        const std::string op = request.get("X-WOPI-Override", std::string());
        if (op != "UNLOCK")
            return nullptr; // The lock taken at load.

        if (_refused < UnlocksToRefuse)
        {
            ++_refused;
            TST_LOG("Refusing unlock #" << _refused << " with 503");
            return std::make_unique<http::Response>(http::StatusCode::ServiceUnavailable);
        }

        TST_LOG("Accepting the unlock after " << _refused << " refusals");
        _accepted = true;
        TRANSITION_STATE(_phase, Phase::Done);

        return nullptr;
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoad);

        TRANSITION_STATE(_phase, Phase::WaitUnlock);

        // Nothing to save, so this unloads straight into the unlock.
        WSD_CMD("closedocument");

        return true;
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                // Transition first: this runs on a timer, and a second pass
                // would open another session whose disconnect unlocks again.
                TRANSITION_STATE(_phase, Phase::WaitLoad);

                TST_LOG("Load: initWebsocket");
                initWebsocket("/wopi/files/0?access_token=anything");
                WSD_CMD("load url=" + getWopiSrc());
                break;
            }
            case Phase::Done:
            {
                LOK_ASSERT_MESSAGE("Expected the refused unlocks to be retried until one landed",
                                   _accepted && _refused == UnlocksToRefuse);
                passTest("Kept asking a busy host until the lock was released");
                break;
            }
            case Phase::WaitLoad:
            case Phase::WaitUnlock:
                break;
        }
    }
};

/// A host that rejects our token on an Unlock has told us the token is no good.
/// We used to note it in the log and carry on with it, because the synchronous
/// path reported every failure the same way. Expiring it stops the attempts
/// that follow from going out under a token the host has already refused.
class UnitWopiUnlockUnauthorized : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoad, WaitUnlock, Done) _phase;

    /// Unlocks the host saw. Teardown tries from more than one place, so
    /// without the token being expired there would be more than one.
    std::size_t _unlocks;

public:
    UnitWopiUnlockUnauthorized()
        : WopiTestServer("UnitWopiUnlockUnauthorized")
        , _phase(Phase::Load)
        , _unlocks(0)
    {
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& /*request*/,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        fileInfo->set("SupportsLocks", "true");
        fileInfo->set("UserCanWrite", "true");
        fileInfo->set("BaseFileName", "doc.odt");
    }

    std::unique_ptr<http::Response>
    assertLockRequest(const Poco::Net::HTTPRequest& request) override
    {
        const std::string op = request.get("X-WOPI-Override", std::string());
        if (op != "UNLOCK")
            return nullptr; // The lock taken at load.

        ++_unlocks;
        TST_LOG("Refusing unlock #" << _unlocks << " with 401");

        if (_phase == Phase::WaitUnlock)
            TRANSITION_STATE(_phase, Phase::Done);

        return std::make_unique<http::Response>(http::StatusCode::Unauthorized);
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoad);

        TRANSITION_STATE(_phase, Phase::WaitUnlock);

        // Nothing to save, so this unloads straight into the unlock.
        WSD_CMD("closedocument");

        return true;
    }

    void onDocBrokerDestroy(const std::string& docKey) override
    {
        TST_LOG("Destroyed dockey [" << docKey << "] after " << _unlocks << " unlock(s)");

        LOK_ASSERT_EQUAL_MESSAGE("Expected the refused token to be expired, stopping the attempts "
                                 "that follow from reusing it",
                                 std::size_t(1), _unlocks);

        passTest("Expired the token the host had just refused");
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                // Transition first: this runs on a timer, and a second pass
                // would open another session and another lock to release.
                TRANSITION_STATE(_phase, Phase::WaitLoad);

                TST_LOG("Load: initWebsocket");
                initWebsocket("/wopi/files/0?access_token=anything");
                WSD_CMD("load url=" + getWopiSrc());
                break;
            }
            case Phase::WaitLoad:
            case Phase::WaitUnlock:
            case Phase::Done:
                break;
        }
    }
};

UnitBase** unit_create_wsd_multi(void)
{
    return new UnitBase*[9]{ new UnitWopiLock(),     new UnitWopiLockReadOnly(),
                             new UnitWopiLockFail(), new UnitWopiUnlock(),
                             new UnitWopiLockIdle(), new UnitWopiLockRefreshTransient(),
                             new UnitWopiUnlockRetry(),
                             new UnitWopiUnlockUnauthorized(), nullptr };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
