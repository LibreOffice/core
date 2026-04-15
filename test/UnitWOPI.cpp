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
 * Unit test for WOPI protocol functionality.
 */

#include <config.h>

#include <common/ProcUtil.hpp>
#include <common/Util.hpp>
#include <lokassert.hpp>

#include <WopiTestServer.hpp>

#include <wsd/DocumentBroker.hpp>
#include <wsd/Process.hpp>

#include <Poco/Net/HTTPRequest.h>

#include <chrono>
#include <thread>
#include <sys/types.h>
#include <unistd.h>

class UnitWOPI : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitModifiedStatus, Done) _phase;

    STATE_ENUM(SavingPhase, Unmodified, Modified) _savingPhase;

    bool _finishedSaveUnmodified;
    bool _finishedSaveModified;

public:
    UnitWOPI()
        : WopiTestServer("UnitWOPI")
        , _phase(Phase::Load)
        , _savingPhase(SavingPhase::Unmodified)
        , _finishedSaveUnmodified(false)
        , _finishedSaveModified(false)
    {
    }

    bool isAutosave() override
    {
        TST_LOG("In SavingPhase " << name(_savingPhase));

        // we fake autosave when saving the modified document
        const bool res = _savingPhase == SavingPhase::Modified;
        TST_LOG("isAutosave: " << std::boolalpha << res);
        return res;
    }

    std::unique_ptr<http::Response>
    assertPutFileRequest(const Poco::Net::HTTPRequest& request) override
    {
        TST_LOG("In SavingPhase " << name(_savingPhase));

        if (_savingPhase == SavingPhase::Unmodified)
        {
            LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);

            // the document is not modified
            LOK_ASSERT_EQUAL_STR("false", request.get("X-COOL-WOPI-IsModifiedByUser"));

            // but the save action is an explicit user's request
            LOK_ASSERT_EQUAL_STR("false", request.get("X-COOL-WOPI-IsAutosave"));

            _finishedSaveUnmodified = true;

            // Modify to test the modified phase.
            TRANSITION_STATE(_phase, Phase::WaitModifiedStatus);
            WSD_CMD("key type=input char=97 key=0");
            WSD_CMD("key type=up char=0 key=512");
        }
        else if (_savingPhase == SavingPhase::Modified)
        {
            LOK_ASSERT_STATE(_phase, Phase::WaitModifiedStatus);

            // the document is modified
            LOK_ASSERT_EQUAL_STR("true", request.get("X-COOL-WOPI-IsModifiedByUser"));

            // and this test fakes that it's an autosave
            LOK_ASSERT_EQUAL_STR("true", request.get("X-COOL-WOPI-IsAutosave"));

            // Check that we get the extended data.
            LOK_ASSERT_EQUAL_STR("CustomFlag=Custom Value;AnotherFlag=AnotherValue",
                                 request.get("X-COOL-WOPI-ExtendedData"));

            _finishedSaveModified = true;

            TRANSITION_STATE(_phase, Phase::Done);
        }

        if (_finishedSaveUnmodified && _finishedSaveModified)
            passTest("Headers for both modified and unmodified received as expected.");

        return nullptr;
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("In SavingPhase " << name(_savingPhase) << ": [" << message << ']');
        LOK_ASSERT_STATE(_savingPhase, SavingPhase::Unmodified);
        LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);

        // Save unmodified.
        WSD_CMD("save dontTerminateEdit=1 dontSaveIfUnmodified=0");
        return true;
    }

    bool onDocumentModified(const std::string& message) override
    {
        TST_LOG("In SavingPhase " << name(_savingPhase) << ": [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitModifiedStatus);

        TRANSITION_STATE(_savingPhase, SavingPhase::Modified);

        // Save modified.
        WSD_CMD("save dontTerminateEdit=0 dontSaveIfUnmodified=0 "
                "extendedData=CustomFlag%3DCustom%20Value%3BAnotherFlag%3DAnotherValue");

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
                initWebsocket("/wopi/files/0?access_token=anything");

                WSD_CMD("load url=" + getWopiSrc());
            }
            break;

            case Phase::WaitLoadStatus:
            case Phase::WaitModifiedStatus:
            case Phase::Done:
            {
                // just wait for the results
                break;
            }
        }
    }
};

/// Tests that filenames with '%' character are handled correctly.
/// The special file '3' is named "he%llo.txt" in CheckFileInfo.
class UnitWOPILoadEncoded : public WopiTestServer
{
    STATE_ENUM(Phase, LoadEncoded, WaitLoadStatus, CloseDoc)
    _phase;

public:
    UnitWOPILoadEncoded()
        : WopiTestServer("UnitWOPILoadEncoded")
        , _phase(Phase::LoadEncoded)
    {
    }

    /// The document is loaded.
    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);

        TRANSITION_STATE(_phase, Phase::CloseDoc);

        WSD_CMD("closedocument");

        return true;
    }

    /// Wait for clean unloading.
    void onDocBrokerDestroy(const std::string& docKey) override
    {
        TST_LOG("Testing with dockey [" << docKey << "] closed");
        LOK_ASSERT_STATE(_phase, Phase::CloseDoc);

        exitTest(TestResult::Ok);
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::LoadEncoded:
            {
                TRANSITION_STATE(_phase, Phase::WaitLoadStatus);

                // File '3' is named "he%llo.txt".
                initWebsocket("/wopi/files/3?access_token=anything");
                WSD_CMD("load url=" + getWopiSrc());

                break;
            }
            case Phase::WaitLoadStatus:
            case Phase::CloseDoc:
            {
                break;
            }
        }
    }
};

class UnitOverload : public WopiTestServer
{
    STATE_ENUM(Phase, HalfOpen, Load, WaitLoadStatus, WaitModifiedStatus, Done) _phase;

    std::thread _dosThread;
    std::vector<pid_t> _children;
    std::vector<std::shared_ptr<http::WebSocketSession>> _webSessions;
    int _count;

    std::size_t getMemoryUsage() const
    {
        std::size_t total = ProcUtil::getMemoryUsageRSS(getpid()) + ProcUtil::getMemoryUsagePSS(getpid());
        for (const pid_t pid : _children)
            total += ProcUtil::getMemoryUsageRSS(pid) + ProcUtil::getMemoryUsagePSS(pid);

        return total;
    }

public:
    UnitOverload()
        : WopiTestServer("UnitOverload")
        , _phase(Phase::Load)
        , _count(0)
    {
    }

    void newChild(const std::shared_ptr<ChildProcess>& child) override
    {
        _children.emplace_back(child->getPid());
        TST_LOG(">>> Child #" << _children.size());
    }

    virtual std::unique_ptr<http::Response>
    assertCheckFileInfoRequest(const Poco::Net::HTTPRequest& /*request*/) override
    {
        TST_LOG(">>> CheckFileInfo #" << _count << ", total memory: " << getMemoryUsage() << " KB");

        SocketPoll::wakeupWorld();
        return std::make_unique<http::Response>(http::StatusCode::NotFound);
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::HalfOpen:
            {
                TRANSITION_STATE(_phase, Phase::Done);
                ++_count;
                TST_LOG("Open #" << _count);
                initWebsocket("/wopi/files/" + std::to_string(_count) + "?access_token=anything");
            }
            break;

            case Phase::Load:
            {
                TRANSITION_STATE(_phase, Phase::Done);
                _dosThread = std::thread(
                    [this]
                    {
                        for (;;)
                        {
                            ++_count;
                            TST_LOG(">>> Open #" << _count << ", total memory: " << getMemoryUsage()
                                                 << " KB");

                            const std::string wopiPath = "/wopi/files/invalid_" +
                                                         std::to_string(_count) +
                                                         "?access_token=anything";
                            const Poco::URI wopiURL(helpers::getTestServerURI() + wopiPath +
                                                    "&testname=" + getTestname());

                            const std::string wopiSrc = Uri::encode(wopiURL.toString());
                            const std::string documentURL = "/cool/" + wopiSrc + "/ws";

                            // This is just a client connection that is used from the tests.
                            TST_LOG("Connecting test client to COOL (#"
                                    << _count << " connection): " << documentURL);

                            Poco::URI uri(helpers::getTestServerURI());

                            std::shared_ptr<http::WebSocketSession> ws =
                                http::WebSocketSession::create(uri.toString());
                            TST_LOG("Connection to " << uri.toString() << " is "
                                                     << (ws->secure() ? "secure" : "plain"));

                            http::Request req(documentURL);
                            if (ws->asyncRequest(req, socketPoll()))
                            {
                                _webSessions.emplace_back(ws);
                                TST_LOG("Load #" << _count);
                                helpers::sendTextFrame(ws, "load url=" + wopiSrc, getTestname());
                            }
                            else
                            {
                                TST_LOG("Failed async request #" << _count << " to "
                                                                 << documentURL);
                            }
                        }
                    });
            }
            break;

            case Phase::WaitLoadStatus:
            case Phase::WaitModifiedStatus:
            case Phase::Done:
            {
                // just wait for the results
                break;
            }
        }
    }
};

/// This is to test that when the WOPI host is
/// unavailable, we are still able to unload.
class UnitWopiUnavailable : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, Done)
    _phase;

    std::string _lockState;
    std::string _lockToken;

    std::chrono::steady_clock::time_point _refreshTime;

    std::atomic_bool _saved;
    std::atomic_bool _uploaded;
    int _cfiCount;
    int _lockCount;

    static constexpr int RefreshPeriodSeconds = 1;

public:
    UnitWopiUnavailable()
        : WopiTestServer("UnitWopiUnavailable")
        , _phase(Phase::Load)
        , _lockState("UNLOCK")
        , _saved(false)
        , _uploaded(false)
        , _cfiCount(0)
        , _lockCount(0)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);

        config.setUInt("storage.wopi.locking.refresh", RefreshPeriodSeconds);
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& /*request*/,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        fileInfo->set("SupportsLocks", "true");
    }

    bool handleCheckFileInfoRequest(const Poco::Net::HTTPRequest& request,
                                    const std::shared_ptr<StreamSocket>& socket) override
    {
        ++_cfiCount;

        if (_cfiCount == 1)
        {
            // The first one must succeed to load the document.
            TST_LOG("Succeeding the CheckFileInfo request (default handler)");
            return WopiTestServer::handleCheckFileInfoRequest(request, socket);
        }

        if (_lockCount > 1)
        {
            TST_LOG("Failing the CheckFileInfo request (Unauthorized): lock count=" << _lockCount);
            std::unique_ptr<http::Response> httpResponse =
                std::make_unique<http::Response>(http::StatusCode::Unauthorized);
            socket->sendAndShutdown(*httpResponse);
        }

        return true;
    }

    std::unique_ptr<http::Response>
    assertPutFileRequest(const Poco::Net::HTTPRequest& request) override
    {
        LOK_ASSERT_EQUAL_STR("true", request.get("X-COOL-WOPI-IsModifiedByUser"));
        LOK_ASSERT_EQUAL(false, request.has("X-LOOL-WOPI-IsModifiedByUser"));

        LOK_ASSERT_EQUAL(true, request.has("X-COOL-WOPI-IsAutosave"));
        LOK_ASSERT_EQUAL(false, request.has("X-LOOL-WOPI-IsAutosave"));

        // Triggered while closing.
        LOK_ASSERT_EQUAL(true, request.has("X-COOL-WOPI-IsExitSave"));
        LOK_ASSERT_EQUAL(false, request.has("X-LOOL-WOPI-IsExitSave"));

        // Fail with error.
        TST_LOG("Returning 500 to simulate PutFile failure");
        return std::make_unique<http::Response>(http::StatusCode::InternalServerError);
    }

    std::unique_ptr<http::Response>
    assertLockRequest(const Poco::Net::HTTPRequest& request) override
    {
        const std::string lockToken = request.get("X-WOPI-Lock", std::string());
        const std::string newLockState = request.get("X-WOPI-Override", std::string());
        TST_LOG("In " << name(_phase) << ", X-WOPI-Lock: " << lockToken << ", X-WOPI-Override: "
                      << newLockState << ", for URI: " << request.getURI());

        ++_lockCount;

        if (_lockCount == 2)
        {
            TST_LOG("Disconnecting");
            deleteSocketAt(0);
        }

        if (_lockCount > 1)
        {
            // Fail with error.
            TST_LOG("Returning 503 to simulate Lock failure");
            return std::make_unique<http::Response>(http::StatusCode::ServiceUnavailable);
        }

        if ("LOCK" == newLockState)
        {
            LOK_ASSERT_EQUAL_MESSAGE("Expected X-WOPI-Override:LOCK", std::string("LOCK"),
                                     newLockState);
            LOK_ASSERT_MESSAGE("Lock token cannot be empty", !lockToken.empty());
            _lockState = newLockState;
            _lockToken = lockToken;

            _refreshTime = std::chrono::steady_clock::now();

            return nullptr; // Succeed in locking.
        }
        else if ("UNLOCK" == newLockState)
        {
            LOK_ASSERT_EQUAL_MESSAGE("Expected X-WOPI-Override:UNLOCK", std::string("UNLOCK"),
                                     newLockState);
            LOK_ASSERT_EQUAL_MESSAGE("Document is not locked", std::string("LOCK"), _lockState);
            LOK_ASSERT_EQUAL_MESSAGE("The lock token has changed", _lockToken, lockToken);
        }
        else
        {
            LOK_ASSERT_FAIL("Unexpected lock-state change while in " << name(_phase));
        }

        // Fail with error.
        TST_LOG("Returning 503 to simulate Lock failure");
        return std::make_unique<http::Response>(http::StatusCode::ServiceUnavailable);
    }

    /// The document is loaded.
    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("Got: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);

        TST_LOG("Modifying the document");
        TRANSITION_STATE(_phase, Phase::Done);

        // Modify the currently opened document; type 'a'.
        WSD_CMD("key type=input char=97 key=0");
        WSD_CMD("key type=up char=0 key=512");

        return true;
    }

    bool onDataLoss(const std::string& reason) override
    {
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
            case Phase::Done:
            {
                // just wait for the results
                break;
            }
        }
    }
};

class UnitWopiHttpHeaders : public WopiTestServer
{
    STATE_ENUM(Phase, Load, Done)
    _phase;

protected:
    virtual std::unique_ptr<http::Response>
    assertCheckFileInfoRequest(const Poco::Net::HTTPRequest& request) override
    {
        assertHeaders(request);

        return nullptr; // Success.
    }

    std::unique_ptr<http::Response>
    assertGetFileRequest(const Poco::Net::HTTPRequest& request) override
    {
        assertHeaders(request);
        exitTest(TestResult::Ok); //TODO: Remove when we add put/rename cases.

        return nullptr; // Success.
    }

    std::unique_ptr<http::Response>
    assertPutFileRequest(const Poco::Net::HTTPRequest& request) override
    {
        assertHeaders(request);
        exitTest(TestResult::Ok);
        return nullptr;
    }

    void assertPutRelativeFileRequest(const Poco::Net::HTTPRequest& request) override
    {
        assertHeaders(request);
        exitTest(TestResult::Ok);
    }

    void assertRenameFileRequest(const Poco::Net::HTTPRequest& request) override
    {
        assertHeaders(request);
        exitTest(TestResult::Ok);
    }

    void assertHeaders(const Poco::Net::HTTPRequest& request) const
    {
        static const std::map<std::string, std::string> Headers{
            { "Authorization", "Bearer xyz123abc456vwc789z" },
            { "X-Requested-With", "XMLHttpRequest" },
        };

        for (const auto& pair : Headers)
        {
            LOK_ASSERT_MESSAGE("Request must have [" + pair.first + ']', request.has(pair.first));
            LOK_ASSERT_EQUAL(pair.second, request[pair.first]);
        }
    }

public:
    UnitWopiHttpHeaders()
        : WopiTestServer("UnitWOPIHttpHeaders")
        , _phase(Phase::Load)
    {
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                TRANSITION_STATE(_phase, Phase::Done);

                // Technically, having an empty line in the header
                // is invalid (it signifies the end of headers), but
                // this is to illustrate that we are able to overcome
                // such issues and generate valid headers.
                const std::string params =
                    "access_header=Authorization%3A%2520Bearer%"
                    "2520xyz123abc456vwc789z%250D%250A%250D%250AX-Requested-With%"
                    "3A%2520XMLHttpRequest&reuse_cookies=language%3Den-us%3AK%3DGS1&permission="
                    "edit";

                initWebsocket("/wopi/files/0?" + params);

                WSD_CMD("load url=" + getWopiSrc());

                break;
            }
            case Phase::Done:
            {
                // Just wait for the results.
                break;
            }
        }
    }
};

/// This tests that loading a document with access_token_ttl
/// works correctly and the token expiry is tracked.
class UnitWopiAccessTokenTtl : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, Modify, Save, Done)
    _phase;

public:
    UnitWopiAccessTokenTtl()
        : WopiTestServer("UnitWopiAccessTokenTtl")
        , _phase(Phase::Load)
    {
    }

    /// The document is loaded.
    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("Got: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);

        TRANSITION_STATE(_phase, Phase::Modify);

        // Modify the currently opened document; type 'a'.
        WSD_CMD("key type=input char=97 key=0");
        WSD_CMD("key type=up char=0 key=512");

        return true;
    }

    bool onDocumentModified(const std::string& message) override
    {
        TST_LOG("Got: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::Modify);

        TRANSITION_STATE(_phase, Phase::Save);
        WSD_CMD("save dontTerminateEdit=0 dontSaveIfUnmodified=0");

        return true;
    }

    std::unique_ptr<http::Response>
    assertPutFileRequest(const Poco::Net::HTTPRequest& /*request*/) override
    {
        LOK_ASSERT_STATE(_phase, Phase::Save);
        TRANSITION_STATE(_phase, Phase::Done);

        passTest("PutFile succeeded with access_token_ttl");
        return nullptr;
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                TRANSITION_STATE(_phase, Phase::WaitLoadStatus);

                // TTL 15 minutes from now, in milliseconds (as per WOPI spec).
                const uint64_t expiryTimeMs =
                    static_cast<uint64_t>(time(nullptr) + (15 * 60)) * 1000;
                initWebsocket("/wopi/files/0?access_token=secret&access_token_ttl=" +
                              std::to_string(expiryTimeMs));

                WSD_CMD("load url=" + getWopiSrc());
                break;
            }
            case Phase::WaitLoadStatus:
            case Phase::Modify:
            case Phase::Save:
            case Phase::Done:
            {
                // just wait for the results
                break;
            }
        }
    }
};

/// Test the save -> 401 -> tokenexpired -> resetaccesstoken -> retry -> success flow.
class UnitWopiTokenRefreshOnSave : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, Modify, WaitSave, WaitTokenExpired, Done)
    _phase;

    int _putFileCount;

    static constexpr auto OriginalToken = "original_token_123";
    static constexpr auto RefreshedToken = "refreshed_token_456";

public:
    UnitWopiTokenRefreshOnSave()
        : WopiTestServer("UnitWopiTokenRefreshOnSave")
        , _phase(Phase::Load)
        , _putFileCount(0)
    {
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("Got: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);

        TRANSITION_STATE(_phase, Phase::Modify);

        // Modify the document; type 'a'.
        WSD_CMD("key type=input char=97 key=0");
        WSD_CMD("key type=up char=0 key=512");

        return true;
    }

    bool onDocumentModified(const std::string& message) override
    {
        TST_LOG("Got: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::Modify);

        TRANSITION_STATE(_phase, Phase::WaitSave);
        WSD_CMD("save dontTerminateEdit=0 dontSaveIfUnmodified=0");

        return true;
    }

    std::unique_ptr<http::Response>
    assertPutFileRequest(const Poco::Net::HTTPRequest& request) override
    {
        ++_putFileCount;
        TST_LOG("PutFile #" << _putFileCount << " URI: " << request.getURI());

        if (_putFileCount == 1)
        {
            // First PutFile: verify original token and return 401.
            for (const auto& param : Poco::URI(request.getURI()).getQueryParameters())
            {
                if (param.first == "access_token")
                {
                    LOK_ASSERT_EQUAL_STR(std::string(OriginalToken), param.second);
                    break;
                }
            }

            TST_LOG("Returning 401 to trigger token refresh");
            return std::make_unique<http::Response>(http::StatusCode::Unauthorized);
        }

        // Second PutFile: verify the refreshed token is used.
        LOK_ASSERT_EQUAL(2, _putFileCount);
        for (const auto& param : Poco::URI(request.getURI()).getQueryParameters())
        {
            if (param.first == "access_token")
            {
                LOK_ASSERT_EQUAL_STR(std::string(RefreshedToken), param.second);
                break;
            }
        }

        TRANSITION_STATE(_phase, Phase::Done);
        passTest("PutFile retry with refreshed token succeeded");
        return nullptr;
    }

    bool onFilterSendWebSocketMessage(const std::string_view message, const WSOpCode /* code */,
                                      const bool /* flush */, int& /*unitReturn*/) override
    {
        if (message == "tokenexpired")
        {
            TST_LOG("Got tokenexpired, sending resetaccesstoken with new token and TTL");
            LOK_ASSERT_STATE(_phase, Phase::WaitSave);
            TRANSITION_STATE(_phase, Phase::WaitTokenExpired);

            // TTL 30 minutes from now, in milliseconds.
            const uint64_t expiryMs =
                static_cast<uint64_t>(time(nullptr) + 30 * 60) * 1000;
            WSD_CMD("resetaccesstoken " + std::string(RefreshedToken) + ' ' +
                    std::to_string(expiryMs));
        }

        return false;
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                TRANSITION_STATE(_phase, Phase::WaitLoadStatus);

                initWebsocket("/wopi/files/0?access_token=" + std::string(OriginalToken) +
                              "&access_token_ttl=0");

                WSD_CMD("load url=" + getWopiSrc());
                break;
            }
            case Phase::WaitLoadStatus:
            case Phase::Modify:
            case Phase::WaitSave:
            case Phase::WaitTokenExpired:
            case Phase::Done:
            {
                // just wait for the results
                break;
            }
        }
    }
};

/// Test the save -> 401 -> tokenexpired -> no refresh -> timeout -> saveunauthorized flow.
/// We return 401 on the only PutFile attempt, send tokenexpired, and never reply with
/// resetaccesstoken. The auth state stays in TokenRefresh (so isValid() is false and
/// no further upload is attempted), and the poll loop's isTokenRefreshTimedOut() check
/// fires after refresh_timeout_secs to broadcast saveunauthorized.
class UnitWopiTokenRefreshTimeout : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, Modify, WaitSave, WaitTimeout, Done)
    _phase;
    int _defLifetimeMins;
    int _refreshTimeoutSecs;
    bool _gotTokenExpired;

public:
    UnitWopiTokenRefreshTimeout(int defLifetimeMins, int refreshTimeoutSecs)
        : WopiTestServer("UnitWopiTokenRefreshTimeout")
        , _phase(Phase::Load)
        , _defLifetimeMins(defLifetimeMins)
        , _refreshTimeoutSecs(refreshTimeoutSecs)
        , _gotTokenExpired(false)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);

        config.setUInt("storage.wopi.access_token.default_lifetime_mins", _defLifetimeMins);
        config.setUInt("storage.wopi.access_token.refresh_timeout_secs", _refreshTimeoutSecs);
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("Got: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);

        TRANSITION_STATE(_phase, Phase::Modify);

        // Modify the document; type 'a'.
        WSD_CMD("key type=input char=97 key=0");
        WSD_CMD("key type=up char=0 key=512");

        return true;
    }

    bool onDocumentModified(const std::string& message) override
    {
        TST_LOG("Got: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::Modify);

        TRANSITION_STATE(_phase, Phase::WaitSave);
        WSD_CMD("save dontTerminateEdit=0 dontSaveIfUnmodified=0");

        return true;
    }

    std::unique_ptr<http::Response>
    assertPutFileRequest(const Poco::Net::HTTPRequest& /*request*/) override
    {
        // Return 401. handleUploadToStorageFailed sends tokenexpired and switches
        // the auth to Type::TokenRefresh, which makes isValid() false -- no further
        // PutFile is attempted while we wait for the host. Since the host never
        // replies with resetaccesstoken, the poll loop eventually fires
        // isTokenRefreshTimedOut() and broadcasts saveunauthorized.
        TST_LOG("Returning 401 (no token refresh will arrive)");
        return std::make_unique<http::Response>(http::StatusCode::Unauthorized);
    }

    bool onFilterSendWebSocketMessage(const std::string_view message, const WSOpCode /* code */,
                                      const bool /* flush */, int& /*unitReturn*/) override
    {
        if (message == "tokenexpired")
        {
            TST_LOG("Got tokenexpired, deliberately NOT sending resetaccesstoken");
            LOK_ASSERT_STATE(_phase, Phase::WaitSave);
            TRANSITION_STATE(_phase, Phase::WaitTimeout);
            _gotTokenExpired = true;
            // Let the refresh-request time out.
        }

        return false;
    }

    bool onDocumentError(const std::string& message) override
    {
        TST_LOG("Got error: [" << message << ']');

        if (message.find("kind=saveunauthorized") != std::string::npos)
        {
            LOK_ASSERT_STATE(_phase, Phase::WaitTimeout);
            LOK_ASSERT_MESSAGE("tokenexpired should have been sent before timeout", _gotTokenExpired);

            TRANSITION_STATE(_phase, Phase::Done);
            passTest("Token refresh timeout correctly produced saveunauthorized");
            return true;
        }

        return false;
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                TRANSITION_STATE(_phase, Phase::WaitLoadStatus);

                initWebsocket("/wopi/files/0?access_token=anything&access_token_ttl=0");

                WSD_CMD("load url=" + getWopiSrc());
                break;
            }
            case Phase::WaitLoadStatus:
            case Phase::Modify:
            case Phase::WaitSave:
            case Phase::WaitTimeout:
            case Phase::Done:
            {
                // just wait for the results
                break;
            }
        }
    }
};

/// Verify the unauthorized (expired token) retry cap.
/// PutFile always returns 401. We answer every tokenexpired with a fresh
/// resetaccesstoken. After MaxTokenRefreshAttempts (3) cycles wsd must stop
/// asking for refreshes and broadcast saveunauthorized instead of sending a
/// 4th tokenexpired.
class UnitWopiTokenRefreshCap : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, Modify, WaitSave, Done) _phase;

    static constexpr int MaxAttempts = 3; // Must match DocumentBroker MaxTokenRefreshAttempts.

    int _tokenExpiredCount;
    bool _gotSaveUnauthorized;

public:
    UnitWopiTokenRefreshCap()
        : WopiTestServer("UnitWopiTokenRefreshCap")
        , _phase(Phase::Load)
        , _tokenExpiredCount(0)
        , _gotSaveUnauthorized(false)
    {
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("Got: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);

        TRANSITION_STATE(_phase, Phase::Modify);
        WSD_CMD("key type=input char=97 key=0");
        WSD_CMD("key type=up char=0 key=512");
        return true;
    }

    bool onDocumentModified(const std::string& message) override
    {
        TST_LOG("Got: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::Modify);

        TRANSITION_STATE(_phase, Phase::WaitSave);
        WSD_CMD("save dontTerminateEdit=0 dontSaveIfUnmodified=0");
        return true;
    }

    std::unique_ptr<http::Response>
    assertPutFileRequest(const Poco::Net::HTTPRequest& /*request*/) override
    {
        TST_LOG("Returning 401 to keep triggering tokenexpired");
        return std::make_unique<http::Response>(http::StatusCode::Unauthorized);
    }

    bool onFilterSendWebSocketMessage(const std::string_view message, const WSOpCode /* code */,
                                      const bool /* flush */, int& /*unitReturn*/) override
    {
        if (message == "tokenexpired")
        {
            ++_tokenExpiredCount;
            TST_LOG("Got tokenexpired #" << _tokenExpiredCount);

            // The cap must stop us at exactly kMaxAttempts. A 4th tokenexpired
            // means the cap didn't engage.
            LOK_ASSERT_MESSAGE("tokenexpired exceeded MaxTokenRefreshAttempts",
                               _tokenExpiredCount <= MaxAttempts);

            // Reply with a fresh resetaccesstoken so the auth flips back to Token
            // and the next save attempt would re-enter the refresh branch -- if
            // not for the cap.
            WSD_CMD("resetaccesstoken refreshed_" + std::to_string(_tokenExpiredCount) + " 0");
        }

        return false;
    }

    bool onDocumentError(const std::string& message) override
    {
        TST_LOG("Got error: [" << message << ']');

        if (message.find("kind=saveunauthorized") != std::string::npos)
        {
            LOK_ASSERT_EQUAL(MaxAttempts, _tokenExpiredCount);
            _gotSaveUnauthorized = true;
            TRANSITION_STATE(_phase, Phase::Done);
            passTest("Retry cap correctly produced saveunauthorized after " +
                     std::to_string(MaxAttempts) + " refreshes");
            return true;
        }

        return false;
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                TRANSITION_STATE(_phase, Phase::WaitLoadStatus);
                initWebsocket("/wopi/files/0?access_token=anything&access_token_ttl=0");
                WSD_CMD("load url=" + getWopiSrc());
                break;
            }
            case Phase::WaitLoadStatus:
            case Phase::Modify:
            case Phase::WaitSave:
            case Phase::Done:
            {
                break;
            }
        }
    }
};

UnitBase** unit_create_wsd_multi(void)
{
    return new UnitBase* []
    {
        new UnitWOPI(), new UnitWOPILoadEncoded(),
            /*new UnitOverload(),*/ new UnitWopiUnavailable(), new UnitWopiHttpHeaders(),
            new UnitWopiAccessTokenTtl(), new UnitWopiTokenRefreshOnSave(),
            new UnitWopiTokenRefreshTimeout(0, 5), new UnitWopiTokenRefreshCap(), nullptr
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
