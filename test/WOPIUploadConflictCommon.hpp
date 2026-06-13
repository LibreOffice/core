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
 * Common utilities for WOPI upload conflict testing scenarios.
 */

#pragma once

#include <string>

#include <Poco/Net/HTTPRequest.h>

#include <WopiTestServer.hpp>

/**
 * This is a base class with a number of test cases which assert that the
 * unsaved changes in the opened document are discarded in case document
 * is changed in storage behind our back. We don't want to overwrite
 * the document which is in storage when the user asks us to
 * upload to storage, without giving the user the opportunity to decide.
 *
 * There are multiple scenarios to test.
 *
 * The way this works is as follows:
 * 1. Load a document.
 * 2. When we get onDocumentLoaded ('loaded:'), we modify it.
 * 3. Simulate content-change in storage and attempt to save it.
 *  4a. Disconnect and the modified data must be discarded.
 *  4b. Save and, on getting the documentconflict error, discard.
 *  4c. Close and, on getting the documentconflict error, discard.
 *  4d. Save and, on getting the documentconflict error, overwrite.
 * 5. Load the document again and verify the expected contents.
 * 6. Move to the next test scenario.
 */
class WOPIUploadConflictCommon : public WopiTestServer
{
private:
    std::size_t _expectedCheckFileInfo;
    std::size_t _expectedGetFile;
    std::size_t _expectedPutRelative;
    std::size_t _expectedPutFile;

protected:
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitModifiedStatus, WaitDocClose) _phase;

    /// The different test scenarios. All but VerifyOverwrite modify the document.
    /// See the documentation above.
    STATE_ENUM(Scenario,
               Disconnect, ///< Scenario 4a.
               SaveDiscard, ///< Scenario 4b.
               CloseDiscard, ///< Scenario 4c.
               SaveOverwrite, ///< Scenario 4d.
               VerifyOverwrite) ///< Scenario 5.
    _scenario;

    static constexpr auto OriginalDocContent = "Original contents";
    static constexpr auto ModifiedOriginalDocContent = "aOriginal contents\n";
    static constexpr auto ConflictingDocContent = "Modified in-storage contents";

    std::size_t getExpectedCheckFileInfo() const { return _expectedCheckFileInfo; }
    void setExpectedCheckFileInfo(std::size_t value)
    {
        _expectedCheckFileInfo = value;
        TST_LOG("Expecting " << _expectedCheckFileInfo << " CheckFileInfo requests.");
    }

    std::size_t getExpectedGetFile() const { return _expectedGetFile; }
    void setExpectedGetFile(std::size_t value)
    {
        _expectedGetFile = value;
        TST_LOG("Expecting " << _expectedGetFile << " GetFile requests.");
    }

    std::size_t getExpectedPutRelative() const { return _expectedPutRelative; }
    void setExpectedPutRelative(std::size_t value)
    {
        _expectedPutRelative = value;
        TST_LOG("Expecting " << _expectedPutRelative << " PutRelative requests.");
    }

    std::size_t getExpectedPutFile() const { return _expectedPutFile; }
    void setExpectedPutFile(std::size_t value)
    {
        _expectedPutFile = value;
        TST_LOG("Expecting " << _expectedPutFile << " PutFile requests.");
    }

public:
    WOPIUploadConflictCommon(const std::string& name, const std::string& fileContent)
        : WopiTestServer(name, fileContent)
        , _expectedCheckFileInfo(0)
        , _expectedGetFile(0)
        , _expectedPutRelative(0)
        , _expectedPutFile(0)
        , _phase(Phase::Load)
        , _scenario(Scenario::Disconnect)
    {
        // We have multiple scenarios to cover.
        setTimeout(std::chrono::seconds(90));
    }

    virtual void startNewTest()
    {
        TST_LOG("===== Starting " << name(_scenario) << " test scenario =====");

        TST_LOG("Resetting the document in storage");
        setFileContent(OriginalDocContent); // Reset to test overwriting.

        resetCountCheckFileInfo();
        resetCountGetFile();
        resetCountPutFile();
        resetCountPutRelative();

        // We always load once per scenario.
        setExpectedGetFile(1); // All the tests GetFile once.
        setExpectedPutRelative(0); // No renaming in these tests.

        switch (_scenario)
        {
            case Scenario::Disconnect:
            {
                // When there is no client connected, there is no way
                // to decide how to resolve the conflict externally.
                // So we quarantine and let it be.
                setExpectedPutFile(1);
                setExpectedCheckFileInfo(1); // Conflict recovery requires second CFI.
            }
            break;
            case Scenario::SaveDiscard:
                setExpectedPutFile(1); // The client discards their changes; don't upload.
                setExpectedCheckFileInfo(2); // Conflict recovery requires second CFI.
                break;
            case Scenario::CloseDiscard:
                setExpectedPutFile(1); // The client discards their changes; don't upload.
                setExpectedCheckFileInfo(2); // Conflict recovery requires second CFI.
                break;
            case Scenario::SaveOverwrite:
                setExpectedPutFile(2); // Upload a second time to force client's changes.
                setExpectedCheckFileInfo(2); // Conflict recovery requires second CFI.
                break;
            case Scenario::VerifyOverwrite:
                // By default, we don't upload when verifying (unless always_save_on_exit is set).
                setExpectedPutFile(0);
                setExpectedCheckFileInfo(1); // No conflict to recover from.
                break;
        }
    }

    void assertGetFileCount()
    {
        if (getExpectedCheckFileInfo() < getCountCheckFileInfo())
        {
            LOK_ASSERT_EQUAL_MESSAGE("Too many CheckFileInfo requests", getExpectedCheckFileInfo(),
                                     getCountCheckFileInfo());
        }

        if (getExpectedGetFile() < getCountGetFile())
        {
            LOK_ASSERT_EQUAL_MESSAGE("Too many GetFile requests", getExpectedGetFile(),
                                     getCountGetFile());
        }
    }

    void assertPutFileCount()
    {
        TST_LOG("Testing " << name(_scenario));
        LOK_ASSERT_STATE(_phase, Phase::WaitDocClose);

        if (getExpectedPutRelative() < getCountPutRelative())
        {
            LOK_ASSERT_EQUAL_MESSAGE("Too many PutRelative requests", getExpectedPutRelative(),
                                     getCountPutRelative());
        }

        if (getExpectedPutFile() < getCountPutFile())
        {
            //FIXME: unreliable in SaveOnExit, which sometimes does 2 PutFile requests.
            LOK_ASSERT_EQUAL_MESSAGE("Too many PutFile requests", getExpectedPutFile(),
                                     getCountPutFile());
        }
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("Testing " << name(_scenario) << ": [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);

        if (_scenario != Scenario::VerifyOverwrite)
        {
            TST_LOG("Modifying the document");
            TRANSITION_STATE(_phase, Phase::WaitModifiedStatus);

            // modify the currently opened document; type 'a'
            WSD_CMD("key type=input char=97 key=0");
            WSD_CMD("key type=up char=0 key=512");
        }
        else
        {
            TST_LOG("Closing the document to finish testing");
            TRANSITION_STATE_MSG(_phase, Phase::WaitDocClose, "Skipping modifications");
            WSD_CMD("closedocument");
        }

        return true;
    }

    bool onDocumentModified(const std::string& message) override
    {
        TST_LOG("Testing " << name(_scenario) << ": [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitModifiedStatus);

        // Change the underlying document in storage.
        TST_LOG("Changing document contents in storage");
        setFileContent(ConflictingDocContent);

        TRANSITION_STATE(_phase, Phase::WaitDocClose);

        switch (_scenario)
        {
            case Scenario::Disconnect:
                TST_LOG("Disconnecting");
                deleteSocketAt(0);
                break;
            case Scenario::SaveDiscard:
            case Scenario::SaveOverwrite:
                // Save the document; wsd should detect now that document has
                // been changed underneath it and send us:
                // "error: cmd=storage kind=documentconflict"
                TST_LOG("Saving the document");
                WSD_CMD("save dontTerminateEdit=0 dontSaveIfUnmodified=0");
                break;
            case Scenario::CloseDiscard:
                // Close the document; wsd should detect now that document has
                // been changed underneath it and send us:
                // "error: cmd=storage kind=documentconflict"
                TST_LOG("Closing the document");
                WSD_CMD("closedocument");
                break;
            case Scenario::VerifyOverwrite:
                LOK_ASSERT_FAIL("Unexpected modification in " << name(_scenario));
                break;
        }

        return true;
    }

    bool onDocumentError(const std::string& message) override
    {
        TST_LOG("Testing " << name(_scenario) << ": [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitDocClose);

        // The conflict error may carry a trailing " errordetail=" token.
        LOK_ASSERT_MESSAGE("Expect only documentconflict errors",
                           message.starts_with("error: cmd=storage kind=documentconflict"));

        switch (_scenario)
        {
            case Scenario::Disconnect:
                LOK_ASSERT_FAIL("We can't possibly get anything after disconnecting");
                break;
            case Scenario::SaveDiscard:
            case Scenario::CloseDiscard:
                TST_LOG("Discarding own changes via closedocument");
                WSD_CMD("closedocument");
                break;
            case Scenario::SaveOverwrite:
                TST_LOG("Overwriting with own version via savetostorage");
                WSD_CMD("savetostorage force=1");
                break;
            case Scenario::VerifyOverwrite:
                LOK_ASSERT_FAIL("Unexpected error in " << name(_scenario));
                break;
        }

        return true;
    }

    // Called when we have modified document data at exit.
    bool onDataLoss(const std::string& reason) override
    {
        // We expect this to happen only with the disonnection test,
        // because only in that case there is no user input.
        LOK_ASSERT_MESSAGE("Expected reason to be 'Data-loss detected'",
                           reason.starts_with("Data-loss detected"));
        LOK_ASSERT_STATE(_phase, Phase::WaitDocClose);

        // In SaveOverwrite, we should not be in modified state, because we do save
        // and upload. But because we don't wait for the modified=false, we can end-up
        // here. Since we will verify after reloading that we have no data-loss, it's OK.
        LOK_ASSERT_MESSAGE(
            "Expected to be in Scenario::Disconnect OR Scenario::SaveOverwrite but was "
                << name(_scenario),
            (_scenario == Scenario::Disconnect) || (_scenario == Scenario::SaveOverwrite));

        return failed();
    }

    // Wait for clean unloading.
    void onDocBrokerDestroy(const std::string& docKey) override
    {
        TST_LOG("Testing " << name(_scenario) << " with dockey [" << docKey << "] closed.");
        LOK_ASSERT_STATE(_phase, Phase::WaitDocClose);

        LOK_ASSERT(getExpectedCheckFileInfo() >= getCountCheckFileInfo());
        LOK_ASSERT_EQUAL(getExpectedGetFile(), getCountGetFile());
        LOK_ASSERT_EQUAL(getExpectedPutRelative(), getCountPutRelative());
        // LOK_ASSERT_EQUAL(getExpectedPutFile(), getCountPutFile()); //FIXME: unreliable for some tests.

        TST_LOG("===== Finished " << name(_scenario) << " test scenario =====");

        if (_scenario != Scenario::VerifyOverwrite)
        {
            // Restart the next scenario, unless we are at the last one.
            TRANSITION_STATE(_phase, Phase::Load);
        }

        switch (_scenario)
        {
            case Scenario::Disconnect:
                TRANSITION_STATE(_scenario, Scenario::SaveDiscard);
                break;
            case Scenario::SaveDiscard:
                TRANSITION_STATE(_scenario, Scenario::CloseDiscard);
                break;
            case Scenario::CloseDiscard:
                TRANSITION_STATE(_scenario, Scenario::SaveOverwrite);
                break;
            case Scenario::SaveOverwrite:
                TRANSITION_STATE(_scenario, Scenario::VerifyOverwrite);
                break;
            case Scenario::VerifyOverwrite:
                passTest("Finished all test scenarios without issues");
                break;
        }
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                startNewTest();

                TST_LOG("Loading the document for " << name(_scenario));

                TRANSITION_STATE(_phase, Phase::WaitLoadStatus);

                initWebsocket("/wopi/files/0?access_token=anything");
                WSD_CMD("load url=" + getWopiSrc());
            }
            break;
            case Phase::WaitLoadStatus:
            {
            }
            break;
            case Phase::WaitModifiedStatus:
            {
            }
            break;
            case Phase::WaitDocClose:
            {
            }
            break;
        }
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
