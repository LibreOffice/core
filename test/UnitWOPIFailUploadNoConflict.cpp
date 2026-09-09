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
 * Unit tests for how a failed upload is reconciled against storage.
 *
 * After a failed upload we issue CheckFileInfo and compare the timestamp in
 * storage against the last one we know of. An unchanged timestamp means the
 * file is untouched, so there is nothing to conflict with and the upload is
 * simply retried. A timestamp that has moved is a conflict, unless we never
 * got an answer from the host, in which case our own upload may be what moved
 * it and the size is all we have to go on.
 */

#include <config.h>

#include <string>

#include <Poco/Net/HTTPRequest.h>
#include <Poco/Util/LayeredConfiguration.h>

#include <HttpRequest.hpp>
#include <Unit.hpp>
#include <WopiTestServer.hpp>
#include <common/Log.hpp>
#include <lokassert.hpp>

/// The WOPI host rejects PutFile but leaves the file in storage untouched.
/// This is what a transient host-side lock looks like: the upload fails,
/// yet nothing has changed in storage, so this is not a conflict and the
/// upload must simply be retried until it succeeds.
///
/// Run for both status codes a host may use to report a lock it holds itself:
/// 423, which says so plainly, and 500, which some hosts still return. Neither
/// is a WOPI lock-mismatch, and both must behave the same way here.
class UnitWOPIFailUploadIntactStorage : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitModifiedStatus, WaitFailedUpload,
               WaitSuccessfulUpload, Done)
    _phase;

    /// How many PutFile attempts to reject before letting one through.
    static constexpr std::size_t UploadsToFail = 2;

    static constexpr auto OriginalDocContent = "Original contents";
    static constexpr auto ModifiedDocContent = "aOriginal contents\n";

    /// The timestamp in storage before the first upload attempt.
    std::string _originalTimestamp;

    /// Number of savefailed errors reported to the client.
    std::size_t _saveFailures;

    /// The status the host rejects our uploads with.
    const http::StatusCode _rejectionStatus;

public:
    explicit UnitWOPIFailUploadIntactStorage(http::StatusCode rejectionStatus)
        : WopiTestServer("UnitWOPIFailUploadIntactStorage_" +
                             std::to_string(static_cast<int>(rejectionStatus)),
                         OriginalDocContent)
        , _phase(Phase::Load)
        , _saveFailures(0)
        , _rejectionStatus(rejectionStatus)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);

        // Retry promptly, so the test doesn't idle.
        config.setUInt("per_document.min_time_between_saves_ms", 100);
        config.setUInt("per_document.min_time_between_uploads_ms", 100);
        config.setUInt("per_document.limit_store_failures", UploadsToFail + 2);
        config.setBool("per_document.always_save_on_exit", false);
    }

    std::unique_ptr<http::Response>
    assertPutFileRequest(const Poco::Net::HTTPRequest& request) override
    {
        // Every attempt must carry the timestamp, so the host can reject a clobber.
        // Losing it would mean we had given up on detecting a real conflict.
        LOK_ASSERT_MESSAGE("Expected the upload to be guarded by a timestamp",
                           request.has("X-COOL-WOPI-Timestamp"));
        LOK_ASSERT_EQUAL_STR(_originalTimestamp, request.get("X-COOL-WOPI-Timestamp"));

        if (getCountPutFile() <= UploadsToFail)
        {
            TST_LOG("PutFile #" << getCountPutFile() << ": rejecting with "
                                << static_cast<int>(_rejectionStatus)
                                << ", storage stays intact");

            // The host refuses the write without touching the file, as it would
            // when the file is locked. Returning a failure leaves both the content
            // and the timestamp in storage unchanged.
            return std::make_unique<http::Response>(_rejectionStatus);
        }

        TST_LOG("PutFile #" << getCountPutFile() << ": accepting");
        LOK_ASSERT_STATE(_phase, Phase::WaitSuccessfulUpload);

        return nullptr; // Success.
    }

    std::unique_ptr<http::Response>
    assertCheckFileInfoRequest(const Poco::Net::HTTPRequest& /*request*/) override
    {
        // The reconciliation CheckFileInfo must report the timestamp we already
        // know, since no upload has landed. That is what makes this not a conflict.
        if (!_originalTimestamp.empty())
        {
            LOK_ASSERT_EQUAL_MESSAGE(
                "The file in storage must not have changed during the test",
                _originalTimestamp, Util::getIso8601FracformatTime(getFileLastModifiedTime()));
        }

        return nullptr; // Success.
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);

        _originalTimestamp = Util::getIso8601FracformatTime(getFileLastModifiedTime());
        TST_LOG("Timestamp in storage before modifying: [" << _originalTimestamp << ']');

        TRANSITION_STATE(_phase, Phase::WaitModifiedStatus);

        WSD_CMD("key type=input char=97 key=0");
        WSD_CMD("key type=up char=0 key=512");

        return true;
    }

    bool onDocumentModified(const std::string& message) override
    {
        TST_LOG("onDocumentModified: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitModifiedStatus);

        TRANSITION_STATE(_phase, Phase::WaitFailedUpload);

        WSD_CMD("save dontTerminateEdit=0 dontSaveIfUnmodified=0");

        return true;
    }

    bool onDocumentError(const std::string& message) override
    {
        TST_LOG("onDocumentError: [" << message << ']');

        // The upload failed, so savefailed is expected. A documentconflict is not:
        // nothing in storage has changed, and telling the user otherwise would
        // make them discard or force-overwrite for no reason.
        LOK_ASSERT_MESSAGE("Unexpected documentconflict on intact storage: " + message,
                           !message.starts_with("error: cmd=storage kind=documentconflict"));
        LOK_ASSERT_MESSAGE("Expected only savefailed errors: " + message,
                           message.starts_with("error: cmd=storage kind=savefailed"));

        ++_saveFailures;
        LOK_ASSERT_MESSAGE("More save failures than upload attempts we reject",
                           _saveFailures <= UploadsToFail);

        return true;
    }

    void onDocumentUploaded(bool success) override
    {
        TST_LOG("onDocumentUploaded: " << (success ? "success" : "failure") << ", PutFile count "
                                       << getCountPutFile());

        if (!success)
        {
            LOK_ASSERT_STATE(_phase, Phase::WaitFailedUpload);

            if (getCountPutFile() == UploadsToFail)
            {
                // The rejections are done; wsd must retry on its own from here.
                TRANSITION_STATE(_phase, Phase::WaitSuccessfulUpload);
            }

            return;
        }

        LOK_ASSERT_STATE(_phase, Phase::WaitSuccessfulUpload);
        TRANSITION_STATE(_phase, Phase::Done);

        LOK_ASSERT_EQUAL_MESSAGE("Expected the rejected uploads to be retried",
                                 UploadsToFail + 1, getCountPutFile());
        LOK_ASSERT_EQUAL_MESSAGE("Expected the modified document in storage",
                                 std::string(ModifiedDocContent), getFileContent());

        passTest("Upload retried to success without a spurious conflict");
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
            case Phase::WaitFailedUpload:
            case Phase::WaitSuccessfulUpload:
            case Phase::Done:
                break;
        }
    }
};

/// The WOPI host rejects PutFile and the file in storage changes at the same
/// time. The moved timestamp means we can no longer assume our version is the
/// only one, so the user must be told and given the choice.
class UnitWOPIFailUploadChangedStorage : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitModifiedStatus, WaitFailedUpload, WaitConflict,
               Done)
    _phase;

    static constexpr auto OriginalDocContent = "Original contents";
    static constexpr auto ConflictingDocContent = "Modified in-storage contents";

    std::string _originalTimestamp;

public:
    UnitWOPIFailUploadChangedStorage()
        : WopiTestServer("UnitWOPIFailUploadChangedStorage", OriginalDocContent)
        , _phase(Phase::Load)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);

        config.setUInt("per_document.min_time_between_saves_ms", 100);
        config.setUInt("per_document.min_time_between_uploads_ms", 100);
        config.setBool("per_document.always_save_on_exit", false);
    }

    std::unique_ptr<http::Response>
    assertPutFileRequest(const Poco::Net::HTTPRequest& /*request*/) override
    {
        LOK_ASSERT_STATE(_phase, Phase::WaitFailedUpload);
        LOK_ASSERT_EQUAL_MESSAGE("Expected a single upload attempt", std::size_t(1),
                                 getCountPutFile());

        // Someone else writes the file while our upload is in flight, then our
        // upload fails. We cannot tell whether our write landed first, so from
        // here the only honest answer is that the document is in conflict.
        TST_LOG("Changing the document in storage behind our back");
        setFileContent(ConflictingDocContent);

        LOK_ASSERT_MESSAGE("Expected the timestamp in storage to have moved",
                           _originalTimestamp !=
                               Util::getIso8601FracformatTime(getFileLastModifiedTime()));

        TRANSITION_STATE(_phase, Phase::WaitConflict);

        return std::make_unique<http::Response>(http::StatusCode::InternalServerError);
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);

        _originalTimestamp = Util::getIso8601FracformatTime(getFileLastModifiedTime());

        TRANSITION_STATE(_phase, Phase::WaitModifiedStatus);

        WSD_CMD("key type=input char=97 key=0");
        WSD_CMD("key type=up char=0 key=512");

        return true;
    }

    bool onDocumentModified(const std::string& message) override
    {
        TST_LOG("onDocumentModified: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitModifiedStatus);

        TRANSITION_STATE(_phase, Phase::WaitFailedUpload);

        WSD_CMD("save dontTerminateEdit=0 dontSaveIfUnmodified=0");

        return true;
    }

    bool onDocumentError(const std::string& message) override
    {
        TST_LOG("onDocumentError: [" << message << ']');

        if (message.starts_with("error: cmd=storage kind=savefailed"))
        {
            // Reported for the failed upload itself, before we reconcile.
            return true;
        }

        LOK_ASSERT_STATE(_phase, Phase::WaitConflict);
        LOK_ASSERT_MESSAGE("Expected a documentconflict error: " + message,
                           message.starts_with("error: cmd=storage kind=documentconflict"));

        TRANSITION_STATE(_phase, Phase::Done);

        // Discard our changes; storage must keep what the other writer put there.
        TST_LOG("Discarding own changes via closedocument");
        WSD_CMD("closedocument");

        return true;
    }

    bool onDataLoss(const std::string& reason) override
    {
        TST_LOG("Modified document being unloaded: " << reason);

        // Expected: we discard our changes rather than clobber the other writer.
        LOK_ASSERT_MESSAGE("Expected reason to be 'Data-loss detected'",
                           reason.starts_with("Data-loss detected"));

        return failed();
    }

    void onDocBrokerDestroy(const std::string& docKey) override
    {
        TST_LOG("Destroyed dockey [" << docKey << ']');
        LOK_ASSERT_STATE(_phase, Phase::Done);

        LOK_ASSERT_EQUAL_MESSAGE("Expected storage to keep the other writer's contents",
                                 std::string(ConflictingDocContent), getFileContent());

        passTest("Conflict detected when the timestamp in storage moved");
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
            case Phase::WaitFailedUpload:
            case Phase::WaitConflict:
            case Phase::Done:
                break;
        }
    }
};

/// The upload gets no response at all and, while we were waiting, the file in
/// storage changed into something that isn't what we uploaded. We cannot claim
/// the version in storage as our own, so this is a conflict.
class UnitWOPIFailUploadTimeoutChangedStorage : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitModifiedStatus, WaitFailedUpload, WaitConflict,
               Done)
    _phase;

    static constexpr int ConnectionTimeoutSeconds = 1;

    static constexpr auto OriginalDocContent = "Original contents";

    /// Deliberately a different length from the "aOriginal contents\n" we upload,
    /// so that the size in storage cannot be mistaken for our own upload.
    static constexpr auto ConflictingDocContent = "Someone else's much longer contents";

public:
    UnitWOPIFailUploadTimeoutChangedStorage()
        : WopiTestServer("UnitWOPIFailUploadTimeoutChangedStorage", OriginalDocContent)
        , _phase(Phase::Load)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);

        config.setUInt("per_document.min_time_between_saves_ms", 100);
        config.setUInt("per_document.min_time_between_uploads_ms", 100);
        config.setUInt("net.connection_timeout_secs", ConnectionTimeoutSeconds);
        config.setBool("per_document.always_save_on_exit", false);
    }

    std::unique_ptr<http::Response>
    assertPutFileRequest(const Poco::Net::HTTPRequest& /*request*/) override
    {
        LOK_ASSERT_STATE(_phase, Phase::WaitFailedUpload);
        LOK_ASSERT_EQUAL_MESSAGE("Expected a single upload attempt", std::size_t(1),
                                 getCountPutFile());

        TST_LOG("Changing the document in storage behind our back");
        setFileContent(ConflictingDocContent);

        // Stall until wsd gives up on us, so it never learns what became of its
        // upload. It has to work that out from what it finds in storage.
        TST_LOG("PutFile: stalling past the connection timeout");
        sleep(ConnectionTimeoutSeconds);
        usleep(300'000);

        TRANSITION_STATE(_phase, Phase::WaitConflict);

        return std::make_unique<http::Response>(http::StatusCode::InternalServerError);
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
        LOK_ASSERT_STATE(_phase, Phase::WaitModifiedStatus);

        TRANSITION_STATE(_phase, Phase::WaitFailedUpload);

        WSD_CMD("save dontTerminateEdit=0 dontSaveIfUnmodified=0");

        return true;
    }

    bool onDocumentError(const std::string& message) override
    {
        TST_LOG("onDocumentError: [" << message << ']');

        if (message.starts_with("error: cmd=storage kind=savefailed"))
        {
            return true;
        }

        LOK_ASSERT_STATE(_phase, Phase::WaitConflict);
        LOK_ASSERT_MESSAGE("Expected a documentconflict error: " + message,
                           message.starts_with("error: cmd=storage kind=documentconflict"));

        TRANSITION_STATE(_phase, Phase::Done);

        TST_LOG("Discarding own changes via closedocument");
        WSD_CMD("closedocument");

        return true;
    }

    bool onDataLoss(const std::string& reason) override
    {
        TST_LOG("Modified document being unloaded: " << reason);

        LOK_ASSERT_MESSAGE("Expected reason to be 'Data-loss detected'",
                           reason.starts_with("Data-loss detected"));

        return failed();
    }

    void onDocBrokerDestroy(const std::string& docKey) override
    {
        TST_LOG("Destroyed dockey [" << docKey << ']');
        LOK_ASSERT_STATE(_phase, Phase::Done);

        LOK_ASSERT_EQUAL_MESSAGE("Expected storage to keep the other writer's contents",
                                 std::string(ConflictingDocContent), getFileContent());

        passTest("Conflict detected when an unanswered upload found a different document");
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
            case Phase::WaitFailedUpload:
            case Phase::WaitConflict:
            case Phase::Done:
                break;
        }
    }
};

/// The host rejects PutFile with 409 and an X-WOPI-Lock header. Per the WOPI
/// protocol that is a lock mismatch, not a change in storage: someone else holds
/// the lock. Nothing was written, so this must not be reported as a conflict.
class UnitWOPIFailUploadLockMismatch : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitModifiedStatus, WaitFailedUpload,
               WaitSuccessfulUpload, Done)
    _phase;

    /// How many PutFile attempts to reject before letting one through.
    static constexpr std::size_t UploadsToFail = 2;

    static constexpr auto OriginalDocContent = "Original contents";
    static constexpr auto ModifiedDocContent = "aOriginal contents\n";

public:
    UnitWOPIFailUploadLockMismatch()
        : WopiTestServer("UnitWOPIFailUploadLockMismatch", OriginalDocContent)
        , _phase(Phase::Load)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);

        config.setUInt("per_document.min_time_between_saves_ms", 100);
        config.setUInt("per_document.min_time_between_uploads_ms", 100);
        config.setUInt("per_document.limit_store_failures", UploadsToFail + 2);
        config.setBool("per_document.always_save_on_exit", false);
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& /*request*/,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        // A lock mismatch only arises where locking is in play.
        fileInfo->set("SupportsLocks", "true");
    }

    std::unique_ptr<http::Response>
    assertPutFileRequest(const Poco::Net::HTTPRequest& /*request*/) override
    {
        if (getCountPutFile() <= UploadsToFail)
        {
            TST_LOG("PutFile #" << getCountPutFile() << ": rejecting with a lock mismatch");

            // 409 plus the lock currently held. Storage is left untouched.
            auto response = std::make_unique<http::Response>(http::StatusCode::Conflict);
            response->set("X-WOPI-Lock", "another-editors-lock-token");
            return response;
        }

        TST_LOG("PutFile #" << getCountPutFile() << ": accepting");
        LOK_ASSERT_STATE(_phase, Phase::WaitSuccessfulUpload);

        return nullptr; // Success.
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
        LOK_ASSERT_STATE(_phase, Phase::WaitModifiedStatus);

        TRANSITION_STATE(_phase, Phase::WaitFailedUpload);

        WSD_CMD("save dontTerminateEdit=0 dontSaveIfUnmodified=0");

        return true;
    }

    bool onDocumentError(const std::string& message) override
    {
        TST_LOG("onDocumentError: [" << message << ']');

        // A lock mismatch says nothing about the document having changed.
        LOK_ASSERT_MESSAGE("Unexpected documentconflict on a lock mismatch: " + message,
                           !message.starts_with("error: cmd=storage kind=documentconflict"));
        LOK_ASSERT_MESSAGE("Expected only savefailed errors: " + message,
                           message.starts_with("error: cmd=storage kind=savefailed"));

        return true;
    }

    void onDocumentUploaded(bool success) override
    {
        TST_LOG("onDocumentUploaded: " << (success ? "success" : "failure") << ", PutFile count "
                                       << getCountPutFile());

        if (!success)
        {
            LOK_ASSERT_STATE(_phase, Phase::WaitFailedUpload);

            if (getCountPutFile() == UploadsToFail)
            {
                TRANSITION_STATE(_phase, Phase::WaitSuccessfulUpload);
            }

            return;
        }

        LOK_ASSERT_STATE(_phase, Phase::WaitSuccessfulUpload);
        TRANSITION_STATE(_phase, Phase::Done);

        LOK_ASSERT_EQUAL_MESSAGE("Expected the rejected uploads to be retried",
                                 UploadsToFail + 1, getCountPutFile());
        LOK_ASSERT_EQUAL_MESSAGE("Expected the modified document in storage",
                                 std::string(ModifiedDocContent), getFileContent());

        passTest("Lock mismatch retried to success without a spurious conflict");
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
            case Phase::WaitFailedUpload:
            case Phase::WaitSuccessfulUpload:
            case Phase::Done:
                break;
        }
    }
};

/// A bare 409, with neither the DOC_CHANGED marker nor a lock header, stays a
/// conflict. We cannot tell what such a host means, and hosts that send it mean
/// the document changed, so leave that reading alone.
class UnitWOPIFailUploadBare409 : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitModifiedStatus, WaitFailedUpload, Done) _phase;

    static constexpr auto OriginalDocContent = "Original contents";

public:
    UnitWOPIFailUploadBare409()
        : WopiTestServer("UnitWOPIFailUploadBare409", OriginalDocContent)
        , _phase(Phase::Load)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);

        config.setUInt("per_document.min_time_between_saves_ms", 100);
        config.setUInt("per_document.min_time_between_uploads_ms", 100);
        config.setBool("per_document.always_save_on_exit", false);
    }

    std::unique_ptr<http::Response>
    assertPutFileRequest(const Poco::Net::HTTPRequest& /*request*/) override
    {
        LOK_ASSERT_STATE(_phase, Phase::WaitFailedUpload);

        TST_LOG("PutFile: rejecting with a bare 409");

        return std::make_unique<http::Response>(http::StatusCode::Conflict);
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
        LOK_ASSERT_STATE(_phase, Phase::WaitModifiedStatus);

        TRANSITION_STATE(_phase, Phase::WaitFailedUpload);

        WSD_CMD("save dontTerminateEdit=0 dontSaveIfUnmodified=0");

        return true;
    }

    bool onDocumentError(const std::string& message) override
    {
        TST_LOG("onDocumentError: [" << message << ']');

        if (message.starts_with("error: cmd=storage kind=savefailed"))
        {
            return true;
        }

        LOK_ASSERT_STATE(_phase, Phase::WaitFailedUpload);
        LOK_ASSERT_MESSAGE("Expected a documentconflict error: " + message,
                           message.starts_with("error: cmd=storage kind=documentconflict"));

        TRANSITION_STATE(_phase, Phase::Done);

        TST_LOG("Discarding own changes via closedocument");
        WSD_CMD("closedocument");

        return true;
    }

    bool onDataLoss(const std::string& reason) override
    {
        TST_LOG("Modified document being unloaded: " << reason);

        LOK_ASSERT_MESSAGE("Expected reason to be 'Data-loss detected'",
                           reason.starts_with("Data-loss detected"));

        return failed();
    }

    void onDocBrokerDestroy(const std::string& docKey) override
    {
        TST_LOG("Destroyed dockey [" << docKey << ']');
        LOK_ASSERT_STATE(_phase, Phase::Done);

        passTest("A bare 409 is still treated as a conflict");
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
            case Phase::WaitFailedUpload:
            case Phase::Done:
                break;
        }
    }
};

/// A host that answers PutFile without a timestamp, as SharePoint does, leaves
/// us with no last-known time at all. A later failed upload then has no
/// baseline to compare against, and a timestamp we never had cannot be evidence
/// that somebody changed the document. Take what storage reports as the new
/// baseline instead, which also puts uploads back under the timestamp guard.
class UnitWOPINoLastKnownTimestamp : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitModifiedStatus, WaitUntimedUpload,
               WaitSecondModifiedStatus, WaitFailedUpload, WaitSuccessfulUpload, Done)
    _phase;

    static constexpr auto OriginalDocContent = "Original contents";

public:
    UnitWOPINoLastKnownTimestamp()
        : WopiTestServer("UnitWOPINoLastKnownTimestamp", OriginalDocContent)
        , _phase(Phase::Load)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);

        config.setUInt("per_document.min_time_between_saves_ms", 100);
        config.setUInt("per_document.min_time_between_uploads_ms", 100);
        config.setUInt("per_document.limit_store_failures", 3);
        config.setBool("per_document.always_save_on_exit", false);
    }

    std::unique_ptr<http::Response>
    assertPutFileRequest(const Poco::Net::HTTPRequest& request) override
    {
        if (getCountPutFile() == 1)
        {
            LOK_ASSERT_STATE(_phase, Phase::WaitUntimedUpload);

            // Accept the write, but answer without the JSON that carries
            // LastModifiedTime. Our copy of the timestamp becomes unsafe.
            TST_LOG("PutFile #1: accepting, but answering without a timestamp");

            return std::make_unique<http::Response>(http::StatusCode::OK);
        }

        if (getCountPutFile() == 2)
        {
            LOK_ASSERT_STATE(_phase, Phase::WaitFailedUpload);

            // With no timestamp of our own there is nothing to guard with.
            LOK_ASSERT_MESSAGE("Expected no timestamp guard while ours is unsafe",
                               !request.has("X-COOL-WOPI-Timestamp"));

            TST_LOG("PutFile #2: rejecting, storage stays intact");

            return std::make_unique<http::Response>(http::StatusCode::InternalServerError);
        }

        LOK_ASSERT_STATE(_phase, Phase::WaitSuccessfulUpload);

        // Having taken storage's timestamp as our baseline, we can guard again.
        LOK_ASSERT_MESSAGE("Expected the timestamp guard to be restored",
                           request.has("X-COOL-WOPI-Timestamp"));

        TST_LOG("PutFile #" << getCountPutFile() << ": accepting");

        return nullptr; // Success, with a timestamp this time.
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
            TRANSITION_STATE(_phase, Phase::WaitUntimedUpload);
        }
        else
        {
            LOK_ASSERT_STATE(_phase, Phase::WaitSecondModifiedStatus);
            TRANSITION_STATE(_phase, Phase::WaitFailedUpload);
        }

        WSD_CMD("save dontTerminateEdit=0 dontSaveIfUnmodified=0");

        return true;
    }

    bool onDocumentError(const std::string& message) override
    {
        TST_LOG("onDocumentError: [" << message << ']');

        // A timestamp we never had says nothing about anyone changing the file.
        LOK_ASSERT_MESSAGE("Unexpected documentconflict with no baseline to compare: " + message,
                           !message.starts_with("error: cmd=storage kind=documentconflict"));
        LOK_ASSERT_MESSAGE("Expected only savefailed errors: " + message,
                           message.starts_with("error: cmd=storage kind=savefailed"));

        return true;
    }

    void onDocumentUploaded(bool success) override
    {
        TST_LOG("onDocumentUploaded: " << (success ? "success" : "failure") << ", PutFile count "
                                       << getCountPutFile());

        if (getCountPutFile() == 1)
        {
            // The untimed upload landed; modify once more so the next one fails
            // while we have no timestamp on record.
            LOK_ASSERT_STATE(_phase, Phase::WaitUntimedUpload);
            TRANSITION_STATE(_phase, Phase::WaitSecondModifiedStatus);

            WSD_CMD("key type=input char=98 key=0");
            WSD_CMD("key type=up char=0 key=512");
            return;
        }

        if (!success)
        {
            LOK_ASSERT_STATE(_phase, Phase::WaitFailedUpload);
            TRANSITION_STATE(_phase, Phase::WaitSuccessfulUpload);
            return;
        }

        LOK_ASSERT_STATE(_phase, Phase::WaitSuccessfulUpload);
        TRANSITION_STATE(_phase, Phase::Done);

        passTest("Recovered a baseline instead of reporting a conflict");
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
            case Phase::WaitUntimedUpload:
            case Phase::WaitSecondModifiedStatus:
            case Phase::WaitFailedUpload:
            case Phase::WaitSuccessfulUpload:
            case Phase::Done:
                break;
        }
    }
};

UnitBase** unit_create_wsd_multi(void)
{
    return new UnitBase* [8]
    {
        new UnitWOPIFailUploadIntactStorage(http::StatusCode::Locked),
            new UnitWOPIFailUploadIntactStorage(http::StatusCode::InternalServerError),
            new UnitWOPIFailUploadLockMismatch(), new UnitWOPIFailUploadBare409(),
            new UnitWOPIFailUploadChangedStorage(),
            new UnitWOPIFailUploadTimeoutChangedStorage(),
            new UnitWOPINoLastKnownTimestamp(), nullptr
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
