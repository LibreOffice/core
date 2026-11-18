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
 * Unit test for WOPI save-on-exit functionality.
 */

#include <config.h>

#include <WOPIUploadConflictCommon.hpp>

#include <atomic>
#include <string>
#include <memory>

#include <Poco/Net/HTTPRequest.h>

#include <Unit.hpp>
#include <lokassert.hpp>

class UnitWOPISaveOnExit : public WOPIUploadConflictCommon
{
    using Base = WOPIUploadConflictCommon;

    using Base::Phase;
    using Base::Scenario;

    using Base::ConflictingDocContent;
    using Base::ModifiedOriginalDocContent;
    using Base::OriginalDocContent;

public:
    UnitWOPISaveOnExit()
        : Base("UnitWOPISaveOnExit", OriginalDocContent)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        Base::configure(config);

        // Small value to shorten the test run time.
        config.setUInt("per_document.limit_store_failures", 2);
        config.setBool("per_document.always_save_on_exit", true);
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
                TST_LOG("Clobbered in the disconnect scenario");
                break;
            case Scenario::SaveDiscard:
            case Scenario::CloseDiscard:
            case Scenario::VerifyOverwrite:
                break;
            case Scenario::SaveOverwrite:
                if (getCountPutFile() < 3)
                {
                    // The first two times the content should be the conflicting one.
                    LOK_ASSERT_EQUAL_MESSAGE("Unexpected contents in storage",
                                             std::string(ConflictingDocContent), getFileContent());
                }
                else
                {
                    // The second time will overwrite with the modified content.
                    LOK_ASSERT_EQUAL_MESSAGE("Unexpected contents in storage",
                                             std::string(ModifiedOriginalDocContent),
                                             getFileContent());
                }
                break;
        }

        return nullptr;
    }

    void onDocBrokerCreate(const std::string& docKey) override
    {
        Base::onDocBrokerCreate(docKey);

        if (_scenario == Scenario::SaveOverwrite)
        {
            // When overwriting, we will do so thrice.
            // Once to find out that we have a conflict and another
            // to force overwriting it. Finally, always_save_on_exit.
            setExpectedPutFile(3);
        }
        else
        {
            // With always_save_on_exit, we expect exactly one PutFile per document.
            setExpectedPutFile(1);
        }
    }

    void onDocumentUploaded(bool success) override
    {
        TST_LOG("Uploaded: " << (success ? "success" : "failure"));

        switch (_scenario)
        {
            case Scenario::Disconnect:
            case Scenario::SaveDiscard:
            case Scenario::CloseDiscard:
            case Scenario::VerifyOverwrite:
                break;
            case Scenario::SaveOverwrite:
                if (getCountPutFile() == 2)
                {
                    TST_LOG("Closing the document to verify its contents after reloading");
                    WSD_CMD("closedocument");
                }
                break;
        }
    }

    void onDocBrokerDestroy(const std::string& docKey) override
    {
        TST_LOG("Testing " << name(_scenario) << " with dockey [" << docKey << "] closed.");
        LOK_ASSERT_STATE(_phase, Phase::WaitDocClose);

        std::string expectedContents;
        switch (_scenario)
        {
            case Scenario::Disconnect:
            case Scenario::SaveDiscard:
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

/// Test upload behavior with always_save_on_exit.
/// The test verifies that a modified document that
/// is manually saved and uploaded, still gets
/// uploaded due to always_save_on_exit=true.
class UnitSaveOnExitSaved : public WopiTestServer
{
    using Base = WopiTestServer;

    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitModifiedStatus, WaitUploadAfterSave,
               WaitUploadOnExit, Done)
    _phase;

    std::atomic_bool _saved;
    std::atomic_bool _uploaded;

public:
    UnitSaveOnExitSaved()
        : WopiTestServer("UnitSaveOnExitSaved")
        , _phase(Phase::Load)
        , _saved(false)
        , _uploaded(false)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);

        // Make it more likely to force uploading.
        config.setBool("per_document.always_save_on_exit", true);
    }

    std::unique_ptr<http::Response>
    assertPutFileRequest(const Poco::Net::HTTPRequest& /*request*/) override
    {
        if (_phase != Phase::WaitUploadAfterSave)
        {
            LOK_ASSERT_STATE(_phase, Phase::WaitUploadOnExit);
        }

        return nullptr;
    }

    /// The document is loaded.
    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("Got: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);

        TST_LOG("Modifying the document");
        TRANSITION_STATE(_phase, Phase::WaitModifiedStatus);

        // Modify the currently opened document; type 'a'.
        WSD_CMD("key type=input char=97 key=0");
        WSD_CMD("key type=up char=0 key=512");

        return true;
    }

    bool onDocumentModified(const std::string& message) override
    {
        TST_LOG("Got: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitModifiedStatus);

        TRANSITION_STATE(_phase, Phase::WaitUploadAfterSave);

        // Save.
        TST_LOG("Saving the document");
        WSD_CMD("save dontTerminateEdit=0 dontSaveIfUnmodified=0");

        return true;
    }

    void onDocumentUploaded(bool success) override
    {
        LOK_ASSERT_MESSAGE("Upload failed unexpectedly", success);

        if (_phase == Phase::WaitUploadAfterSave)
        {
            _uploaded = true;
            if (_saved && _uploaded)
            {
                // Just disconnect.
                TRANSITION_STATE(_phase, Phase::WaitUploadOnExit);
                TST_LOG("Disconnecting");
                deleteSocketAt(0);
            }
        }
        else
        {
            LOK_ASSERT_STATE(_phase, Phase::WaitUploadOnExit);

            TRANSITION_STATE(_phase, Phase::Done);
            passTest("Uploaded on exit as expected");
        }
    }

    /// Wait for ModifiedStatus=false before disconnecting.
    bool onDocumentUnmodified(const std::string& message) override
    {
        TST_LOG("Got: [" << message << ']');

        LOK_ASSERT_STATE(_phase, Phase::WaitUploadAfterSave);

        _saved = true;
        if (_saved && _uploaded)
        {
            // Just disconnect.
            TRANSITION_STATE(_phase, Phase::WaitUploadOnExit);
            TST_LOG("Disconnecting");
            deleteSocketAt(0);
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
                initWebsocket("/wopi/files/0?access_token=anything");

                WSD_CMD("load url=" + getWopiSrc());
                break;
            }
            case Phase::WaitLoadStatus:
            case Phase::WaitModifiedStatus:
            case Phase::WaitUploadAfterSave:
            case Phase::WaitUploadOnExit:
            case Phase::Done:
            {
                // just wait for the results
                break;
            }
        }
    }
};

/// Test upload behavior with always_save_on_exit.
/// The test verifies that an unmodified document
/// is *not* uploaded when always_save_on_exit=true
/// and is closed (ownertermination).
class UnitSaveOnExitUnmodifiedClosed : public WopiTestServer
{
    using Base = WopiTestServer;

protected:
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitDestroy, Done)
    _phase;

public:
    UnitSaveOnExitUnmodifiedClosed(const std::string& name = "UnitSaveOnExitUnmodifiedClosed")
        : Base(name)
        , _phase(Phase::Load)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        Base::configure(config);

        // Make it more likely to force uploading.
        config.setBool("per_document.always_save_on_exit", true);
    }

    std::unique_ptr<http::Response>
    assertPutFileRequest(const Poco::Net::HTTPRequest& /*request*/) override
    {
        TST_LOG("Checking X-COOL-WOPI headers");

        failTest("Unexpected PutFile on unmodified document");
        return nullptr;
    }

    /// Wait for ModifiedStatus=false before closing.
    /// This is sent right after loading.
    bool onDocumentUnmodified(const std::string& message) override
    {
        TST_LOG("Got: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);

        TRANSITION_STATE(_phase, Phase::WaitDestroy);

        TST_LOG("Closing document");
        WSD_CMD("closedocument");

        return true;
    }

    void onDocumentUploaded(bool success) override
    {
        LOK_ASSERT_MESSAGE("Upload failed unexpectedly", success);
        LOK_ASSERT_STATE(_phase, Phase::WaitDestroy);
    }

    // Wait for clean unloading.
    void onDocBrokerDestroy(const std::string& docKey) override
    {
        TST_LOG("Destroyed dockey [" << docKey << "] closed");
        LOK_ASSERT_STATE(_phase, Phase::WaitDestroy);

        TRANSITION_STATE(_phase, Phase::Done);
        passTest("Document uploaded on closing as expected");

        Base::onDocBrokerDestroy(docKey);
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
            case Phase::WaitDestroy:
            case Phase::Done:
            {
                // just wait for the results
                break;
            }
        }
    }
};

/// Test upload behavior with always_save_on_exit.
/// The test verifies that an unmodified document
/// is *not* uploaded when always_save_on_exit=true
/// and is disconnected.
class UnitSaveOnExitUnmodifiedDisconnect : public UnitSaveOnExitUnmodifiedClosed
{
public:
    UnitSaveOnExitUnmodifiedDisconnect()
        : UnitSaveOnExitUnmodifiedClosed("UnitSaveOnExitUnmodifiedDisconnect")
    {
    }

    /// Wait for ModifiedStatus=false before disconnecting.
    /// This is sent right after loading.
    bool onDocumentUnmodified(const std::string& message) override
    {
        TST_LOG("Got: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);

        TRANSITION_STATE(_phase, Phase::WaitDestroy);

        // Disconnect to trigger the auto-save logic.
        TST_LOG("Disconnecting");
        deleteSocketAt(0);

        return true;
    }
};

UnitBase** unit_create_wsd_multi(void)
{
    return new UnitBase* [5]
    {
        new UnitWOPISaveOnExit(), new UnitSaveOnExitSaved(), new UnitSaveOnExitUnmodifiedClosed(),
            new UnitSaveOnExitUnmodifiedDisconnect(), nullptr
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
