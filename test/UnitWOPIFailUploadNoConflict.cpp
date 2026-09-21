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

/// An upload gets no answer, but it did land: storage ends up holding exactly
/// the bytes we sent. The host reports a SHA256, which matches ours, so we can
/// claim that version as our own and carry on rather than raise a conflict.
class UnitWOPITimeoutHashMatches : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitModifiedStatus, WaitFailedUpload,
               WaitSuccessfulUpload, Done)
    _phase;

    static constexpr int ConnectionTimeoutSeconds = 1;

    static constexpr auto OriginalDocContent = "Original contents";
    static constexpr auto ModifiedDocContent = "aOriginal contents\n";

public:
    UnitWOPITimeoutHashMatches()
        : WopiTestServer("UnitWOPITimeoutHashMatches", OriginalDocContent)
        , _phase(Phase::Load)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);

        config.setUInt("per_document.min_time_between_saves_ms", 100);
        config.setUInt("per_document.min_time_between_uploads_ms", 100);
        config.setUInt("per_document.limit_store_failures", 3);
        config.setUInt("net.connection_timeout_secs", ConnectionTimeoutSeconds);
        config.setBool("per_document.always_save_on_exit", false);
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& /*request*/,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        fileInfo->set("SHA256", getFileContentSha256Base64());
    }

    std::unique_ptr<http::Response>
    assertPutFileRequest(const Poco::Net::HTTPRequest& /*request*/) override
    {
        if (getCountPutFile() == 1)
        {
            // Stall past the timeout and then accept, so the write lands but we
            // never hear about it.
            TST_LOG("PutFile #1: stalling past the connection timeout, then accepting");
            sleep(ConnectionTimeoutSeconds);
            usleep(300'000);

            return nullptr; // Success, recorded after we have given up.
        }

        TST_LOG("PutFile #" << getCountPutFile() << ": accepting");
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
        LOK_ASSERT_STATE(_phase, Phase::WaitModifiedStatus);

        TRANSITION_STATE(_phase, Phase::WaitFailedUpload);

        WSD_CMD("save dontTerminateEdit=0 dontSaveIfUnmodified=0");

        return true;
    }

    bool onDocumentError(const std::string& message) override
    {
        TST_LOG("onDocumentError: [" << message << ']');

        // The upload we gave up on is what storage holds, so nothing is in
        // conflict; only the failure we saw should be reported.
        LOK_ASSERT_MESSAGE("Unexpected documentconflict when our own upload landed: " + message,
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
            TRANSITION_STATE(_phase, Phase::WaitSuccessfulUpload);
            return;
        }

        LOK_ASSERT_STATE(_phase, Phase::WaitSuccessfulUpload);
        TRANSITION_STATE(_phase, Phase::Done);

        // The retry could only be accepted because we adopted the timestamp of
        // the version we recognised as ours; otherwise the host would have
        // rejected it on the stale timestamp we would still be carrying.
        LOK_ASSERT_EQUAL_MESSAGE("Expected our content in storage",
                                 std::string(ModifiedDocContent), getFileContent());

        passTest("Our own landed upload was recognised by its hash");
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

/// An upload gets no answer and another writer replaces the file with contents
/// of exactly the same length. The size cannot tell the two apart and would
/// have us adopt, and then overwrite, someone else's document. The hash can.
class UnitWOPITimeoutHashDiffersSameSize : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitModifiedStatus, WaitFailedUpload, WaitConflict,
               Done)
    _phase;

    static constexpr int ConnectionTimeoutSeconds = 1;

    static constexpr auto OriginalDocContent = "Original contents";

    /// What we upload after typing an 'a' at the start.
    static constexpr auto ModifiedDocContent = "aOriginal contents\n";

    /// Somebody else's document, the same length as ours to the byte.
    static constexpr auto ConflictingDocContent = "bDifferent stuff!!\n";

public:
    UnitWOPITimeoutHashDiffersSameSize()
        : WopiTestServer("UnitWOPITimeoutHashDiffersSameSize", OriginalDocContent)
        , _phase(Phase::Load)
    {
        static_assert(sizeof(ModifiedDocContent) == sizeof(ConflictingDocContent),
                      "The point of this test is that the sizes match");
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);

        config.setUInt("per_document.min_time_between_saves_ms", 100);
        config.setUInt("per_document.min_time_between_uploads_ms", 100);
        config.setUInt("net.connection_timeout_secs", ConnectionTimeoutSeconds);
        config.setBool("per_document.always_save_on_exit", false);
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& /*request*/,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        fileInfo->set("SHA256", getFileContentSha256Base64());
    }

    std::unique_ptr<http::Response>
    assertPutFileRequest(const Poco::Net::HTTPRequest& /*request*/) override
    {
        LOK_ASSERT_STATE(_phase, Phase::WaitFailedUpload);
        LOK_ASSERT_EQUAL_MESSAGE("Expected a single upload attempt", std::size_t(1),
                                 getCountPutFile());

        TST_LOG("Another writer replaces the document with same-sized contents");
        setFileContent(ConflictingDocContent);

        LOK_ASSERT_EQUAL_MESSAGE("The sizes must collide for this test to mean anything",
                                 std::string(ModifiedDocContent).size(), getFileContent().size());

        // Stall past the timeout, then refuse. We are left not knowing whether
        // our upload landed, and the size in storage says it did.
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

        passTest("A same-sized document from another writer was caught by its hash");
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

/// A conflict must stop us uploading of our own accord. Once storage has changed
/// under us, what we hold is no longer a newer version of what is there: it is a
/// competing one, and sending it would overwrite the other writer without the
/// user ever choosing to. The only upload allowed from that point on is the one
/// the user asks for, which arrives as a forced upload and does not come through
/// the usual "is there a newer version to send?" path.
///
/// The retry throttle alone is not what protects us here, so the throttle is set
/// low enough that an unguarded retry would fire well inside the settle window.
class UnitWOPINoUploadWhileConflicted : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitModifiedStatus, WaitFailedUpload, WaitConflict,
               SettleNoUpload, Done)
    _phase;

    /// How long to let wsd run after the conflict, watching for an upload it
    /// should not make. Many times the upload throttle set in configure().
    static constexpr std::chrono::seconds SettleDuration{ 2 };

    static constexpr auto OriginalDocContent = "Original contents";
    static constexpr auto ConflictingDocContent = "Someone else's contents";

    /// When the settle window ends and we can conclude no upload was attempted.
    std::chrono::steady_clock::time_point _settleUntil;

public:
    UnitWOPINoUploadWhileConflicted()
        : WopiTestServer("UnitWOPINoUploadWhileConflicted", OriginalDocContent)
        , _phase(Phase::Load)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);

        // Retry promptly, so an upload we must not make has every chance to
        // happen within the settle window rather than being merely throttled.
        config.setUInt("per_document.min_time_between_saves_ms", 100);
        config.setUInt("per_document.min_time_between_uploads_ms", 100);
        config.setUInt("per_document.limit_store_failures", 10);

        // always_save_on_exit deliberately uploads through a conflict; that is a
        // different decision from the one under test here.
        config.setBool("per_document.always_save_on_exit", false);
    }

    std::unique_ptr<http::Response>
    assertPutFileRequest(const Poco::Net::HTTPRequest& /*request*/) override
    {
        // Only the first upload reaches here. Any further one carries the
        // timestamp from before storage changed, so the host rejects it on the
        // timestamp guard without consulting us. Those attempts are counted
        // though, and the settle window below is what checks for them.
        LOK_ASSERT_STATE(_phase, Phase::WaitFailedUpload);
        LOK_ASSERT_EQUAL_MESSAGE("Expected this to be the first upload", std::size_t(1),
                                 getCountPutFile());

        TST_LOG("PutFile #1: changing the document in storage and rejecting the upload");
        setFileContent(ConflictingDocContent);

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

        LOK_ASSERT_MESSAGE("Expected a documentconflict error: " + message,
                           message.starts_with("error: cmd=storage kind=documentconflict"));

        if (_phase == Phase::WaitConflict)
        {
            _settleUntil = std::chrono::steady_clock::now() + SettleDuration;

            TRANSITION_STATE(_phase, Phase::SettleNoUpload);

            // Leave the conflict unresolved, as a user who has not answered the
            // dialog yet would, and watch what wsd does on its own.
            TST_LOG("Conflict raised; watching for " << SettleDuration
                                                     << " that no upload follows");
        }

        return true;
    }

    // onDataLoss is left to the base, which fails the test: discarding via
    // closedocument must not be reported as losing the user's work.

    void onDocBrokerDestroy(const std::string& docKey) override
    {
        TST_LOG("Destroyed dockey [" << docKey << ']');
        LOK_ASSERT_STATE(_phase, Phase::Done);

        LOK_ASSERT_EQUAL_MESSAGE("Expected storage to keep the other writer's contents",
                                 std::string(ConflictingDocContent), getFileContent());

        passTest("No upload attempted while the conflict was unresolved");
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
            case Phase::SettleNoUpload:
            {
                if (std::chrono::steady_clock::now() < _settleUntil)
                {
                    break;
                }

                // Any upload beyond the first is one we made of our own accord
                // while the user had not resolved the conflict.
                LOK_ASSERT_EQUAL_MESSAGE(
                    "Expected no upload while the conflict was unresolved, but the document "
                    "was sent to storage again, overwriting the other writer",
                    std::size_t(1), getCountPutFile());

                TRANSITION_STATE(_phase, Phase::Done);

                TST_LOG("Discarding own changes via closedocument");
                WSD_CMD("closedocument");
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


/// A conflict does not always mean our own upload was refused. A peer joining a
/// document whose timestamp moved raises one too, and there the upload path is
/// still perfectly good. Giving up on uploads for the rest of the document's
/// life would mean every later edit is silently never stored: no upload, no
/// error, and a broker that exits believing it has nothing to save.
///
/// Note what this does and does not cover. Holding back only while our own
/// upload was refused leaves this case alone, so the test passes with that
/// condition absent as well as present. What it guards is the wider version of
/// the same idea - standing down on any conflict at all - which is the shape a
/// later fix is most likely to take.
class UnitWOPIUploadAfterJoinConflict : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitModifiedStatus, JoinPeer, WaitConflict,
               WaitUploadAttempt, Done)
    _phase;

    static constexpr auto OriginalDocContent = "Original contents";
    static constexpr auto ForeignDocContent = "Someone else's contents";

    /// How long to wait for the upload before calling it lost. Many times the
    /// upload throttle, so only a document that will never upload runs out.
    static constexpr std::chrono::seconds UploadDeadline{ 4 };

    /// When we asked for the save that must reach storage.
    std::chrono::steady_clock::time_point _savedAt;

public:
    UnitWOPIUploadAfterJoinConflict()
        : WopiTestServer("UnitWOPIUploadAfterJoinConflict", OriginalDocContent)
        , _phase(Phase::Load)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);

        config.setUInt("per_document.min_time_between_saves_ms", 100);
        config.setUInt("per_document.min_time_between_uploads_ms", 100);
        config.setUInt("per_document.limit_store_failures", 10);
        config.setBool("per_document.always_save_on_exit", false);
    }

    std::unique_ptr<http::Response>
    assertPutFileRequest(const Poco::Net::HTTPRequest& /*request*/) override
    {
        // The upload we must still be making. Which version wins is for the
        // host to arbitrate from the timestamp we send; the point here is that
        // we ask at all.
        TST_LOG("PutFile #" << getCountPutFile() << ": the upload path is still alive");

        // The verdict is left to invokeWSDTest, which owns _phase; deciding it
        // from this thread races with the deadline check below.
        return nullptr; // Success.
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');

        if (_phase != Phase::WaitLoadStatus)
            return true; // The peer's load, which we don't drive from here.

        TRANSITION_STATE(_phase, Phase::WaitModifiedStatus);

        WSD_CMD("key type=input char=97 key=0");
        WSD_CMD("key type=up char=0 key=512");

        return true;
    }

    bool onDocumentModified(const std::string& message) override
    {
        TST_LOG("onDocumentModified: [" << message << ']');

        if (_phase != Phase::WaitModifiedStatus)
            return true; // The document stays dirty until an upload lands.

        TRANSITION_STATE(_phase, Phase::JoinPeer);

        // Somebody else writes the document while we hold unsaved changes, then
        // a second view joins and notices the timestamp has moved.
        TST_LOG("Changing the document in storage, then joining a second view");
        setFileContent(ForeignDocContent);

        addWebSocket();
        WSD_CMD_BY_CONNECTION_INDEX(1, "load url=" + getWopiSrc());

        return true;
    }

    bool onDocumentError(const std::string& message) override
    {
        TST_LOG("onDocumentError: [" << message << ']');

        if (!message.starts_with("error: cmd=storage kind=documentconflict"))
            return true;

        if (_phase == Phase::JoinPeer || _phase == Phase::WaitConflict)
        {
            _savedAt = std::chrono::steady_clock::now();

            TRANSITION_STATE(_phase, Phase::WaitUploadAttempt);

            // The conflict is raised and left unresolved, as a user who has not
            // answered the dialog would leave it. Saving must still try.
            TST_LOG("Conflict raised on join; saving must still reach storage");
            WSD_CMD("save dontTerminateEdit=0 dontSaveIfUnmodified=0");
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
            case Phase::Done:
            {
                if (getCountPutFile() > 0)
                    passTest("Uploads still work after a conflict raised by a joining peer");
                break;
            }
            case Phase::WaitUploadAttempt:
            {
                if (getCountPutFile() > 0)
                {
                    TRANSITION_STATE(_phase, Phase::Done);
                    break;
                }

                if (std::chrono::steady_clock::now() - _savedAt < UploadDeadline)
                    break;

                TRANSITION_STATE(_phase, Phase::Done);
                failTest("The document was never uploaded after a conflict raised by a joining "
                         "peer: the save produced no PutFile and no error, so the edit would be "
                         "lost when the document unloads");
                break;
            }
            case Phase::WaitLoadStatus:
            case Phase::WaitModifiedStatus:
            case Phase::JoinPeer:
            case Phase::WaitConflict:
                break;
        }
    }
};

/// A host that rejects an upload with 503 and a Retry-After is telling us both
/// that it may or may not have written the file, and how long to leave it alone.
/// We must wait the period it asked for before asking what it holds, rather than
/// our own much shorter pacing.
class UnitWOPIRetryAfterHonoured : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitModifiedStatus, WaitFailedUpload,
               WaitSuccessfulUpload, Done)
    _phase;

    /// What we ask the client to wait. Comfortably longer than the throttle
    /// below, so that honouring it is distinguishable from ignoring it.
    static constexpr int RetryAfterSeconds = 2;

    /// The pacing we would fall back on had the host not asked for anything.
    static constexpr int ThrottleMs = 100;

    static constexpr auto OriginalDocContent = "Original contents";
    static constexpr auto ModifiedDocContent = "aOriginal contents\n";

    /// When we rejected the upload, to measure the wait against.
    std::chrono::steady_clock::time_point _rejectedAt;

public:
    UnitWOPIRetryAfterHonoured()
        : WopiTestServer("UnitWOPIRetryAfterHonoured", OriginalDocContent)
        , _phase(Phase::Load)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);

        config.setUInt("per_document.min_time_between_saves_ms", ThrottleMs);
        config.setUInt("per_document.min_time_between_uploads_ms", ThrottleMs);
        config.setUInt("per_document.limit_store_failures", 10);
        config.setBool("per_document.always_save_on_exit", false);
    }

    std::unique_ptr<http::Response>
    assertPutFileRequest(const Poco::Net::HTTPRequest& /*request*/) override
    {
        if (getCountPutFile() == 1)
        {
            LOK_ASSERT_STATE(_phase, Phase::WaitFailedUpload);

            TST_LOG("PutFile #1: rejecting with 503 and Retry-After: " << RetryAfterSeconds);
            _rejectedAt = std::chrono::steady_clock::now();

            TRANSITION_STATE(_phase, Phase::WaitSuccessfulUpload);

            auto response =
                std::make_unique<http::Response>(http::StatusCode::ServiceUnavailable);
            response->add("Retry-After", std::to_string(RetryAfterSeconds));
            return response;
        }

        TST_LOG("PutFile #" << getCountPutFile() << ": accepting");
        LOK_ASSERT_STATE(_phase, Phase::WaitSuccessfulUpload);

        return nullptr; // Success.
    }

    std::unique_ptr<http::Response>
    assertCheckFileInfoRequest(const Poco::Net::HTTPRequest& /*request*/) override
    {
        // The one that follows the rejected upload is the one under test; the
        // first is the ordinary one at load.
        if (_rejectedAt.time_since_epoch().count() && getCountCheckFileInfo() > 1)
        {
            const auto waited = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - _rejectedAt);

            TST_LOG("CheckFileInfo after the rejected upload came " << waited << " later");

            // Allow for the poll's own granularity, but stay far above the
            // throttle we would have used had Retry-After been ignored.
            constexpr std::chrono::milliseconds least(RetryAfterSeconds * 1000 - 250);
            LOK_ASSERT_MESSAGE("Expected to wait the Retry-After the host asked for, not our "
                               "own much shorter pacing; waited only " +
                                   std::to_string(waited.count()) + "ms",
                               waited >= least);
        }

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

        // Storage was never touched, so there is nothing to conflict with.
        LOK_ASSERT_MESSAGE("Unexpected documentconflict after a 503: " + message,
                           !message.starts_with("error: cmd=storage kind=documentconflict"));

        return true;
    }

    void onDocumentUploaded(bool success) override
    {
        TST_LOG("onDocumentUploaded: " << (success ? "success" : "failure") << ", PutFile count "
                                       << getCountPutFile());

        if (!success)
            return;

        LOK_ASSERT_STATE(_phase, Phase::WaitSuccessfulUpload);
        TRANSITION_STATE(_phase, Phase::Done);

        LOK_ASSERT_EQUAL_MESSAGE("Expected the modified document in storage",
                                 std::string(ModifiedDocContent), getFileContent());

        passTest("Waited the Retry-After the host asked for, then uploaded");
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

/// A host that answers 500 has not told us whether it wrote the file. It may
/// have committed our bytes and fallen over afterwards, which is exactly what a
/// host too slow to answer in time does as well. Treating the error as proof it
/// did not write means calling our own upload somebody else's work and asking
/// the user to resolve a conflict that never happened.
///
/// Here storage ends up holding precisely what we sent, and the hash says so.
class UnitWOPITransientUploadLanded : public WopiTestServer
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitModifiedStatus, WaitFailedUpload, WaitRecovery,
               Done)
    _phase;

    static constexpr auto OriginalDocContent = "Original contents";
    static constexpr auto ModifiedDocContent = "aOriginal contents\n";

public:
    UnitWOPITransientUploadLanded()
        : WopiTestServer("UnitWOPITransientUploadLanded", OriginalDocContent)
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

    void configCheckFileInfo(const Poco::Net::HTTPRequest& /*request*/,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        fileInfo->set("SHA256", getFileContentSha256Base64());
    }

    std::unique_ptr<http::Response>
    assertPutFileRequest(const Poco::Net::HTTPRequest& /*request*/) override
    {
        if (getCountPutFile() == 1)
        {
            LOK_ASSERT_STATE(_phase, Phase::WaitFailedUpload);

            // Commit the write, then fail the request. The harness only stores
            // the body on a successful response, so do it here to stand for a
            // host that got as far as writing and then fell over on the way back.
            TST_LOG("PutFile #1: writing the document, then answering 500");
            setFileContent(ModifiedDocContent);

            TRANSITION_STATE(_phase, Phase::WaitRecovery);

            return std::make_unique<http::Response>(http::StatusCode::InternalServerError);
        }

        // Recognizing our own upload spares the user a conflict; it does not yet
        // spare us the re-upload, which happens here as it does after a timeout.
        TST_LOG("PutFile #" << getCountPutFile() << ": accepting");
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
        LOK_ASSERT_STATE(_phase, Phase::WaitModifiedStatus);

        TRANSITION_STATE(_phase, Phase::WaitFailedUpload);

        WSD_CMD("save dontTerminateEdit=0 dontSaveIfUnmodified=0");

        return true;
    }

    bool onDocumentError(const std::string& message) override
    {
        TST_LOG("onDocumentError: [" << message << ']');

        // savefailed is honest - the request did fail. A conflict is not: the
        // document in storage is the one we just sent.
        LOK_ASSERT_MESSAGE("Reported a conflict over our own upload: " + message,
                           !message.starts_with("error: cmd=storage kind=documentconflict"));

        return true;
    }

    void onDocumentUploaded(bool success) override
    {
        TST_LOG("onDocumentUploaded: " << (success ? "success" : "failure") << ", PutFile count "
                                       << getCountPutFile());

        if (!success)
            return;

        LOK_ASSERT_STATE(_phase, Phase::WaitRecovery);
        TRANSITION_STATE(_phase, Phase::Done);

        LOK_ASSERT_EQUAL_MESSAGE("Expected our own upload to be left in storage",
                                 std::string(ModifiedDocContent), getFileContent());

        passTest("Recognized our own upload behind a 500 instead of raising a conflict");
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
            case Phase::WaitRecovery:
            case Phase::WaitLoadStatus:
            case Phase::WaitModifiedStatus:
            case Phase::WaitFailedUpload:
            case Phase::Done:
                break;
        }
    }
};

UnitBase** unit_create_wsd_multi(void)
{
    return new UnitBase* [14]
    {
        new UnitWOPIFailUploadIntactStorage(http::StatusCode::Locked),
            new UnitWOPIFailUploadIntactStorage(http::StatusCode::InternalServerError),
            new UnitWOPIFailUploadLockMismatch(), new UnitWOPIFailUploadBare409(),
            new UnitWOPIFailUploadChangedStorage(),
            new UnitWOPIFailUploadTimeoutChangedStorage(), new UnitWOPITimeoutHashMatches(),
            new UnitWOPITimeoutHashDiffersSameSize(), new UnitWOPINoLastKnownTimestamp(),
            new UnitWOPINoUploadWhileConflicted(),
            new UnitWOPIUploadAfterJoinConflict(),
            new UnitWOPIRetryAfterHonoured(),
            new UnitWOPITransientUploadLanded(), nullptr
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
