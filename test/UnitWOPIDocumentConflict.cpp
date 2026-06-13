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
 * Unit test for WOPI document conflict resolution.
 */

#include <config.h>

#include <WOPIUploadConflictCommon.hpp>

#include <ctime>
#include <string>
#include <memory>

#include <Poco/Net/HTTPRequest.h>

#include <lokassert.hpp>

class UnitWOPIDocumentConflict : public WOPIUploadConflictCommon
{
    using Base = WOPIUploadConflictCommon;

    using Base::Phase;
    using Base::Scenario;

    using Base::ConflictingDocContent;
    using Base::ModifiedOriginalDocContent;
    using Base::OriginalDocContent;

public:
    UnitWOPIDocumentConflict()
        : Base("UnitWOPIDocumentConflict", OriginalDocContent)
    {
    }

    std::unique_ptr<http::Response>
    assertGetFileRequest(const Poco::Net::HTTPRequest& /*request*/) override
    {
        TST_LOG("Testing " << name(_scenario));
        LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);

        assertGetFileCount();

        return nullptr; // Success.
    }

    std::unique_ptr<http::Response>
    assertPutFileRequest(const Poco::Net::HTTPRequest& /*request*/) override
    {
        TST_LOG("Testing " << name(_scenario));
        LOK_ASSERT_STATE(_phase, Phase::WaitDocClose);

        assertPutFileCount();

        switch (_scenario)
        {
            case Scenario::Disconnect:
            case Scenario::SaveDiscard:
            case Scenario::CloseDiscard:
            case Scenario::VerifyOverwrite:
                LOK_ASSERT_FAIL("Unexpectedly overwritting the document in storage");
                break;
            case Scenario::SaveOverwrite:
                TST_LOG("Closing the document to verify its contents after reloading");
                WSD_CMD("closedocument");
                break;
        }

        return nullptr;
    }

    void onDocBrokerDestroy(const std::string& docKey) override
    {
        TST_LOG("Testing " << name(_scenario) << " with dockey [" << docKey << "] closed.");
        LOK_ASSERT_STATE(_phase, Phase::WaitDocClose);

        std::string expectedContents;
        switch (_scenario)
        {
            case Scenario::Disconnect:
                expectedContents = ConflictingDocContent; //TODO: save-as in this case.
                break;
            case Scenario::SaveDiscard:
                expectedContents = ConflictingDocContent;
                break;
            case Scenario::CloseDiscard:
                expectedContents = ConflictingDocContent;
                break;
            case Scenario::SaveOverwrite:
                expectedContents = ModifiedOriginalDocContent;
                break;
            case Scenario::VerifyOverwrite:
                expectedContents = OriginalDocContent;
                break;
        }

        LOK_ASSERT_EQUAL_MESSAGE("Unexpected contents in storage", expectedContents,
                                 getFileContent());

        Base::onDocBrokerDestroy(docKey);
    }
};

/// Simulate an upload timeout that precipitates a "conflict."
/// When the upload times out (i.e. takes longer than *we* expected),
/// there are really two outcomes: 1) the upload eventually was
/// successful (this case), and the document has been updated in storage,
/// or, 2) the upload fails (UnitConflictAfterTimeoutFailure), and the
/// document is not updated in storage. Either way, since we timed out, we
/// never know the result. In case #1, the modified timestamp in
/// storage would have changed, and any subsequent upload will fail,
/// since the timestamp that we will send with subsequent uploads,
/// which represents the expected last-modify date/time, will be invalid.
/// This test simulates this particular scenario and verifies that
/// we are able to recover gracefully from it.
class UnitConflictAfterTimeoutSuccess : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitModifiedStatus, WaitUploaded, WaitDestroy)
    _phase;

    std::atomic_int _uploadAttemptCount;

    static constexpr int ConnectionTimeoutSeconds = 1;

public:
    UnitConflictAfterTimeoutSuccess()
        : WopiTestServer("UnitConflictAfterTimeoutSuccess")
        , _phase(Phase::Load)
        , _uploadAttemptCount(0)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);

        // We intentionally fail uploading twice, so need at least 3 tries.
        config.setUInt("per_document.limit_store_failures", 3);
        config.setBool("per_document.always_save_on_exit", false);
        config.setUInt("net.connection_timeout_secs", ConnectionTimeoutSeconds); // Timeout quickly.
        config.setUInt("per_document.min_time_between_uploads_ms", 100); // Short retry interval.
    }

    std::unique_ptr<http::Response>
    assertPutFileRequest(const Poco::Net::HTTPRequest& request) override
    {
        ++_uploadAttemptCount;
        TST_LOG("PutFile: " << _uploadAttemptCount << ", Phase: " << name(_phase));

        const std::string wopiTimestamp = request.get("X-COOL-WOPI-Timestamp", std::string());
        LOK_ASSERT_MESSAGE("Unexpected forced upload", !wopiTimestamp.empty());

        if (_uploadAttemptCount == 1)
        {
            // First attempt, timeout.
            TST_LOG("PutFile: sleeping");
            sleep(ConnectionTimeoutSeconds); // Connection timeout.
            usleep(300'000); // Go over, to be certain we timed-out the upload.
            // Don't sleep for 2 x ConnectionTimeoutSeconds, as the
            // CheckFileInfo request may timeout, since we're stuck here.
        }

        // Success.
        TST_LOG("PutFile: returning success");
        return nullptr;
    }

    /// The document is loaded.
    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("Got: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);

        TRANSITION_STATE(_phase, Phase::WaitModifiedStatus);
        WSD_CMD("key type=input char=97 key=0");
        WSD_CMD("key type=up char=0 key=512");

        return true;
    }

    /// The document is modified. Save it.
    bool onDocumentModified(const std::string& message) override
    {
        TST_LOG("Got: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitModifiedStatus);

        TRANSITION_STATE(_phase, Phase::WaitUploaded);
        WSD_CMD("save dontTerminateEdit=0 dontSaveIfUnmodified=0");

        return true;
    }

    void onDocumentUploaded(bool success) override
    {
        TST_LOG("Uploaded: " << (success ? "success" : "failure"));
        LOK_ASSERT_STATE(_phase, Phase::WaitUploaded);
        LOK_ASSERT_EQUAL_MESSAGE("Expected the first upload to fail", _uploadAttemptCount == 1,
                                 !success);

        if (success)
        {
            // We are done!
            TRANSITION_STATE(_phase, Phase::WaitDestroy);

            WSD_CMD("closedocument");
        }
    }

    // Wait for clean unloading.
    void onDocBrokerDestroy(const std::string& docKey) override
    {
        TST_LOG("Destroyed dockey [" << docKey << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitDestroy);

        passTest("Document uploaded on closing as expected.");
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
            case Phase::WaitUploaded:
            case Phase::WaitDestroy:
                break;
        }
    }
};

/// Simulate an upload timeout that precipitates a "conflict."
/// When the upload times out (i.e. takes longer than *we* expected),
/// there are really two outcomes: 1) the upload eventually was
/// successful (UnitConflictAfterTimeoutSuccess), and the document
/// has been updated in storage, or, 2) the upload fails (this case),
/// and the document is not updated in storage. Either way, since we
/// timed out, we never know the result. In case #1, the modified
/// timestamp in storage would have changed, and any subsequent upload
/// will fail, since the timestamp that we will send with subsequent uploads,
/// which represents the expected last-modify date/time, will be invalid.
/// This test simulates this particular scenario and verifies that
/// we are able to recover gracefully from it.
class UnitConflictAfterTimeoutFailure : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitModifiedStatus, WaitUploaded, WaitDestroy)
    _phase;

    std::atomic_int _uploadAttemptCount;

    static constexpr int ConnectionTimeoutSeconds = 1;

public:
    UnitConflictAfterTimeoutFailure()
        : WopiTestServer("UnitConflictAfterTimeoutFailure")
        , _phase(Phase::Load)
        , _uploadAttemptCount(0)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);

        // We intentionally fail uploading twice, so need at least 3 tries.
        config.setUInt("per_document.limit_store_failures", 3);
        config.setBool("per_document.always_save_on_exit", false);
        config.setUInt("net.connection_timeout_secs", ConnectionTimeoutSeconds); // Timeout quickly.
        config.setUInt("per_document.min_time_between_uploads_ms", 100); // Short retry interval.
    }

    std::unique_ptr<http::Response>
    assertPutFileRequest(const Poco::Net::HTTPRequest& request) override
    {
        TST_LOG("PutFile: " << _uploadAttemptCount << ", Phase: " << name(_phase));

        const std::string wopiTimestamp = request.get("X-COOL-WOPI-Timestamp", std::string());
        LOK_ASSERT_MESSAGE("Unexpected forced upload", !wopiTimestamp.empty());

        // First attempt, timeout.
        TST_LOG("PutFile: sleeping");
        sleep(ConnectionTimeoutSeconds); // Connection timeout.
        usleep(300'000); // Go over, to be certain we timed-out the upload.
        // Don't sleep for 2 x ConnectionTimeoutSeconds, as the
        // CheckFileInfo request may timeout, since we're stuck here.

        // Simulate a conflict. Make the size different, otherwise
        // we can't yet detect same-size changes.
        // We do this after the first upload attempt (which will timeout)
        // because otherwise we would've not reached here and returned
        // 'conflict' sooner, in WopiTestServer::handleWopiUpload().
        TST_LOG("Simulating conflict in storage");
        setFileContent("Modified data in storage");

        // Fail.
        TST_LOG("PutFile: returning a bogus failure");
        return std::make_unique<http::Response>(http::StatusCode::InternalServerError);
    }

    /// The document is loaded.
    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("Got: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);

        TRANSITION_STATE(_phase, Phase::WaitModifiedStatus);
        WSD_CMD("key type=input char=97 key=0");
        WSD_CMD("key type=up char=0 key=512");

        return true;
    }

    /// The document is modified. Save it.
    bool onDocumentModified(const std::string& message) override
    {
        TST_LOG("Got: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitModifiedStatus);

        TRANSITION_STATE(_phase, Phase::WaitUploaded);
        WSD_CMD("save dontTerminateEdit=0 dontSaveIfUnmodified=0");

        return true;
    }

    void onDocumentUploaded(bool success) override
    {
        TST_LOG("Uploaded #" << _uploadAttemptCount << ": " << (success ? "success" : "failure"));
        LOK_ASSERT_STATE(_phase, Phase::WaitUploaded);
        LOK_ASSERT_MESSAGE("Expected Phase::WaitUploaded or Phase::WaitDestroy",
                           _phase == Phase::WaitUploaded || _phase == Phase::WaitDestroy);
        // LOK_ASSERT_EQUAL_MESSAGE("Expected the first upload to fail", _uploadAttemptCount == 1,
        //                          !success);
    }

    bool onDocumentError(const std::string& message) override
    {
        TST_LOG("Testing " << name(_phase) << ": [" << message << ']');
        // LOK_ASSERT_STATE(_phase, Phase::WaitDocClose);

        ++_uploadAttemptCount;
        TST_LOG("uploadAttemptCount: " << _uploadAttemptCount);
        if (_uploadAttemptCount == 1)
        {
            LOK_ASSERT_MESSAGE("Expect only savefailed errors",
                               message.starts_with("error: cmd=storage kind=savefailed"));
        }
        else
        {
            // The conflict error may carry a trailing " errordetail=" token.
            LOK_ASSERT_MESSAGE("Expect only documentconflict errors",
                               message.starts_with("error: cmd=storage kind=documentconflict"));

            // We are done!
            TRANSITION_STATE(_phase, Phase::WaitDestroy);

            TST_LOG("Discarding own changes via closedocument");
            WSD_CMD("closedocument");
        }

        return true;
    }

    // Wait for clean unloading.
    void onDocBrokerDestroy(const std::string& docKey) override
    {
        TST_LOG("Destroyed dockey [" << docKey << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitDestroy);

        passTest("Document uploaded on closing as expected.");
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
            case Phase::WaitUploaded:
            case Phase::WaitDestroy:
                break;
        }
    }
};

/// Simulate an upload timeout that precipitates a "conflict."
/// When the upload times out (i.e. takes longer than *we* expected),
/// there are really two outcomes: 1) the upload eventually was
/// successful (this case), and the document has been updated in storage,
/// or, 2) the upload fails (UnitConflictAfterTimeoutFailure), and the
/// document is not updated in storage. Either way, since we timed out, we
/// never know the result. In case #1, the modified timestamp in
/// storage would have changed, and any subsequent upload will fail,
/// since the timestamp that we will send with subsequent uploads,
/// which represents the expected last-modify date/time, will be invalid.
/// This test simulates this particular scenario and verifies that
/// we are able to recover gracefully from it.
class UnitConflictRecoveryTimeout : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitModifiedStatus, WaitUploaded, WaitDestroy)
    _phase;

    std::atomic_int _uploadAttemptCount;

    static constexpr int ConnectionTimeoutSeconds = 1;

public:
    UnitConflictRecoveryTimeout()
        : WopiTestServer("UnitConflictRecoveryTimeout")
        , _phase(Phase::Load)
        , _uploadAttemptCount(0)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);

        // We intentionally fail uploading twice, so need at least 3 tries.
        config.setUInt("per_document.limit_store_failures", 3);
        config.setBool("per_document.always_save_on_exit", false);
        config.setUInt("net.connection_timeout_secs", ConnectionTimeoutSeconds); // Timeout quickly.
        config.setUInt("per_document.min_time_between_uploads_ms", 100); // Short retry interval.
    }

    std::unique_ptr<http::Response>
    assertPutFileRequest(const Poco::Net::HTTPRequest& request) override
    {
        ++_uploadAttemptCount;
        TST_LOG("PutFile: " << _uploadAttemptCount << ", Phase: " << name(_phase));

        const std::string wopiTimestamp = request.get("X-COOL-WOPI-Timestamp", std::string());

        if (_uploadAttemptCount == 1)
        {
            LOK_ASSERT_MESSAGE("Unexpected forced upload", !wopiTimestamp.empty());

            // First attempt, timeout.
            TST_LOG("PutFile: sleeping");
            // Don't sleep for 2 x ConnectionTimeoutSeconds, as the
            // CheckFileInfo request may timeout, since we're stuck here.
            sleep(ConnectionTimeoutSeconds); // Connection timeout.
            usleep(300'000); // Go over, to be certain we timed-out the upload.
            TST_LOG("PutFile: woke up");
        }
        else
        {
            LOK_ASSERT_MESSAGE("Expected forced upload", wopiTimestamp.empty());
        }

        // Success.
        TST_LOG("PutFile: returning success");
        return nullptr;
    }

    virtual std::unique_ptr<http::Response>
    assertCheckFileInfoRequest(const Poco::Net::HTTPRequest& /*request*/) override
    {
        if (_uploadAttemptCount == 1)
        {
            // The CheckFileInfo following an upload timeout.
            // While we're "processing" the slow upload request, close the document.
            TRANSITION_STATE(_phase, Phase::WaitDestroy);
            WSD_CMD("closedocument");

            TST_LOG("CheckFileInfo: sleeping");
            sleep(ConnectionTimeoutSeconds); // Connection timeout.
            usleep(300'000); // Go over, to be certain we timed-out the upload.
            TST_LOG("CheckFileInfo: woke up");
        }

        return nullptr; // Success.
    }

    /// The document is loaded.
    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("Got: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);

        TRANSITION_STATE(_phase, Phase::WaitModifiedStatus);
        WSD_CMD("key type=input char=97 key=0");
        WSD_CMD("key type=up char=0 key=512");

        return true;
    }

    /// The document is modified. Save it.
    bool onDocumentModified(const std::string& message) override
    {
        TST_LOG("Got: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitModifiedStatus);

        TRANSITION_STATE(_phase, Phase::WaitUploaded);
        WSD_CMD("save dontTerminateEdit=0 dontSaveIfUnmodified=0");

        return true;
    }

    void onDocumentUploaded(bool success) override
    {
        TST_LOG("Uploaded: " << (success ? "success" : "failure"));
        LOK_ASSERT(_phase == Phase::WaitUploaded || _phase == Phase::WaitDestroy);
        LOK_ASSERT_EQUAL_MESSAGE("Expected the first upload to fail", _uploadAttemptCount == 1,
                                 !success);
    }

    // Wait for clean unloading.
    void onDocBrokerDestroy(const std::string& docKey) override
    {
        TST_LOG("Destroyed dockey [" << docKey << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitDestroy);

        passTest("Document uploaded on closing as expected.");
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
            case Phase::WaitUploaded:
            case Phase::WaitDestroy:
                break;
        }
    }
};

UnitBase** unit_create_wsd_multi(void)
{
    return new UnitBase*[5]{ new UnitConflictRecoveryTimeout(), new UnitWOPIDocumentConflict(),
                             new UnitConflictAfterTimeoutSuccess(),
                             new UnitConflictAfterTimeoutFailure(), nullptr };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
