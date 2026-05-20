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
 * Unit test for document quarantine functionality.
 */

#include <config.h>

#include <WopiTestServer.hpp>
#include <WOPIUploadConflictCommon.hpp>
#include <Unit.hpp>
#include <lokassert.hpp>
#include <testlog.hpp>
#include <common/FileUtil.hpp>
#include <wsd/DocumentBroker.hpp>
#include <wsd/Process.hpp>

#include <Poco/Net/HTTPRequest.h>
#include <csignal>
#include <ctime>

namespace
{
std::vector<std::string> getQuarantineFiles(const std::string testname,
                                            const std::string& quarantinePath)
{
    std::vector<std::string> files;
    Poco::File(quarantinePath).list(files);

    TST_LOG("Found " << files.size() << " quarantine file in [" << quarantinePath << ']');
    for (std::size_t i = 0; i < files.size(); ++i)
    {
        TST_LOG("Found quarantine file #" << (i + 1) << ": [" << files[i] << ']');
    }

    return files;
}
} // namespace

/// This test simulates a permanently-failing upload.
class UnitQuarantineConflict : public WOPIUploadConflictCommon
{
    using Base = WOPIUploadConflictCommon;

    using Base::Phase;
    using Base::Scenario;

    using Base::OriginalDocContent;

    std::string _quarantinePath;
    bool _unloadingModifiedDocDetected;
    bool _putFailed;

    static constexpr std::size_t LimitStoreFailures = 2;
    static constexpr bool SaveOnExit = true;

public:
    UnitQuarantineConflict()
        : Base("UnitQuarantineConflict", OriginalDocContent)
        , _unloadingModifiedDocDetected(true)
        , _putFailed(false)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        Base::configure(config);

        // Small value to shorten the test run time.
        config.setUInt("per_document.limit_store_failures", LimitStoreFailures);
        config.setBool("per_document.always_save_on_exit", SaveOnExit);

        if (!config.getBool("quarantine_files[@enable]", false) ||
            config.getString("quarantine_files.path", std::string()).empty())
        {
            config.setBool("quarantine_files[@enable]", true);
            auto rootPath = Poco::Path(config.getString("child_root_path", ""));
            rootPath.popDirectory().pushDirectory("quarantine");
            _quarantinePath = FileUtil::createRandomTmpDir(rootPath.toString());
            TST_LOG("Quarantine path set to [" << _quarantinePath << ']');
            config.setString("quarantine_files.path", _quarantinePath);
        }
        else
        {
            _quarantinePath = config.getString("quarantine_files.path", std::string());
            TST_LOG("Quarantine path found at [" << _quarantinePath << ']');
        }

        // Make sure the quarantine directory is clean.
        FileUtil::removeFile(_quarantinePath, true);
    }

    void onDocBrokerCreate(const std::string& docKey) override
    {
        // reset for the next document
        _putFailed = false;

        Base::onDocBrokerCreate(docKey);

        if (_scenario == Scenario::VerifyOverwrite)
        {
            // By default, we don't upload when verifying (unless always_save_on_exit is set).
            setExpectedPutFile(SaveOnExit ? 2 : 0);
        }
        else
        {
            // With always_save_on_exit=true and limit_store_failures=LimitStoreFailures,
            // we expect exactly two PutFile requests per document.
            setExpectedPutFile(LimitStoreFailures);
            setExpectedCheckFileInfo(2); // Conflict recovery requires second CFI.
        }
    }

    std::unique_ptr<http::Response>
    assertGetFileRequest(const Poco::Net::HTTPRequest& /*request*/) override
    {
        TST_LOG("Testing " << name(_scenario));
        LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);

        assertGetFileCount();

        //FIXME: check that unloading modified documents trigger test failure.
        // LOK_ASSERT_EQUAL_MESSAGE("Expected modified document detection to have triggered", true,
        //                          _unloadingModifiedDocDetected);
        _unloadingModifiedDocDetected = false; // Reset.

        return nullptr; // Success.
    }

    std::unique_ptr<http::Response>
    assertPutFileRequest(const Poco::Net::HTTPRequest& request) override
    {
        TST_LOG("Testing " << name(_scenario));
        LOK_ASSERT_STATE(_phase, Phase::WaitDocClose);

        assertPutFileCount();

        const std::string wopiTimestamp = request.get("X-COOL-WOPI-Timestamp", std::string());
        const bool force = wopiTimestamp.empty(); // Without a timestamp we force to always store.

        // We don't expect overwriting by forced uploading.
        LOK_ASSERT_EQUAL_MESSAGE("Unexpected overwritting the document in storage", _putFailed, force);

        _putFailed = true;

        // Internal Server Error.
        return std::make_unique<http::Response>(http::StatusCode::InternalServerError);
    }

    bool onDocumentModified(const std::string& message) override
    {
        TST_LOG("Testing " << name(_scenario) << ": [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitModifiedStatus);

        TRANSITION_STATE(_phase, Phase::WaitDocClose);

        switch (_scenario)
        {
            case Scenario::Disconnect:
                // Just disconnect.
                TST_LOG("Disconnecting");
                deleteSocketAt(0);
                break;
            case Scenario::SaveDiscard:
            case Scenario::SaveOverwrite:
                // Save the document.
                TST_LOG("Saving the document");
                WSD_CMD("save dontTerminateEdit=0 dontSaveIfUnmodified=0");
                break;
            case Scenario::CloseDiscard:
                // Close the document.
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

        if (getCountCheckFileInfo() == 1)
        {
            LOK_ASSERT_MESSAGE("Expect only savefailed errors on first upload",
                               message.starts_with("error: cmd=storage kind=savefailed"));
        }
        else
        {
            // Once the first upload fails, we issue CheckFileInfo, which detects the conflict.
            LOK_ASSERT_EQUAL_MESSAGE(
                "Expect only documentconflict errors after the second CheckFileInfo",
                std::string("error: cmd=storage kind=documentconflict"), message);

            // Close the document.
            TST_LOG("Closing the document");
            WSD_CMD("closedocument");
        }

        return true;
    }

    // Called when we have modified document data at exit.
    bool onDataLoss(const std::string& reason) override
    {
        TST_LOG("Modified document being unloaded: " << reason);

        // We expect this to happen only with the disonnection test,
        // because only in that case there is no user input.
        LOK_ASSERT_MESSAGE("Expected reason to be 'Data-loss detected'",
                           reason.starts_with("Data-loss detected"));
        LOK_ASSERT_STATE(_phase, Phase::WaitDocClose);
        _unloadingModifiedDocDetected = true;

        return failed();
    }

    // Wait for clean unloading.
    void onDocBrokerDestroy(const std::string& docKey) override
    {
        TST_LOG("Testing " << name(_scenario) << " with dockey [" << docKey << "] closed.");
        LOK_ASSERT_STATE(_phase, Phase::WaitDocClose);

        // Uploading fails and we can't have anything but the original.
        LOK_ASSERT_EQUAL_MESSAGE("Unexpected contents in storage", std::string(OriginalDocContent),
                                 getFileContent());

        const std::string documentUrl = Uri::encode(helpers::getTestServerURI() + "/wopi/files/0");
        const std::string quarantinePath = _quarantinePath + '/' + documentUrl;
        const std::vector<std::string> files = getQuarantineFiles(testname, quarantinePath);
        LOK_ASSERT_MESSAGE("Expected 1 quaratined files in [" << quarantinePath << ']',
                           files.size() == 1);

        Base::onDocBrokerDestroy(docKey);
    }
};

/// This test simulates a crashing kit with modifications.
class UnitQuarantineCrash : public WopiTestServer
{
    using Base = WopiTestServer;

    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitModifyStatus, WaitUpload, Unload, Done) _phase;
    std::string _quarantinePath;
    std::vector<pid_t> _kitsPids;

public:
    UnitQuarantineCrash()
        : Base("UnitQuarantineCrash")
        , _phase(Phase::Load)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        Base::configure(config);

        if (!config.getBool("quarantine_files[@enable]", false) ||
            config.getString("quarantine_files.path", std::string()).empty())
        {
            config.setBool("quarantine_files[@enable]", true);
            auto rootPath = Poco::Path(config.getString("child_root_path", ""));
            rootPath.popDirectory().pushDirectory("quarantine");
            _quarantinePath = FileUtil::createRandomTmpDir(rootPath.toString());
            TST_LOG("Quarantine path set to [" << _quarantinePath << ']');
            config.setString("quarantine_files.path", _quarantinePath);
        }
        else
        {
            _quarantinePath = config.getString("quarantine_files.path", std::string());
            TST_LOG("Quarantine path found at [" << _quarantinePath << ']');
        }

        // Make sure the quarantine directory is clean.
        FileUtil::removeFile(_quarantinePath, true);
    }

    void newChild(const std::shared_ptr<ChildProcess>& child) override
    {
        _kitsPids.push_back(child->getPid());
        TST_LOG("New Kit PID: " << _kitsPids.back());
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);

        TRANSITION_STATE(_phase, Phase::WaitModifyStatus);

        // Modify the doc.
        TST_LOG("Modifying");
        WSD_CMD("key type=input char=97 key=0");
        WSD_CMD("key type=up char=0 key=512");

        return true;
    }

    bool onDocumentModified(const std::string& message) override
    {
        TST_LOG("Got [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitModifyStatus);

        TRANSITION_STATE(_phase, Phase::WaitUpload);

        TST_LOG("Saving the document");
        WSD_CMD("save dontTerminateEdit=0 dontSaveIfUnmodified=0");

        return true;
    }

    /// Wait for ModifiedStatus=false before crashing the kit.
    bool onDocumentUnmodified(const std::string& message) override
    {
        TST_LOG("Got: [" << message << ']');

        // We sometimes get two ModifiedStatus=false;
        // The first is from Document::notifySyntheticUnmodifiedState(),
        // the second is from the loKit, after we've
        // already transitioned from WaitUpload to Unload.
        // Tolerate ModifiedStatus=false after WaitUpload, but not before.
        if (_phase < Phase::WaitUpload)
        {
            LOK_ASSERT_STATE(_phase, Phase::WaitUpload);
        }
        else
        {
            TRANSITION_STATE(_phase, Phase::Unload);

            // Kill the kit.
            for (const auto& pid : _kitsPids)
            {
                TST_LOG("Killing kit: " << pid);
                ::kill(pid, SIGKILL);
            }
        }

        return true;
    }

    void kitKilled(int count) override
    {
        TST_LOG("Kit killed");
        LOK_ASSERT(static_cast<std::size_t>(count) <= _kitsPids.size());
    }

    // Called when we have modified document data at exit.
    bool onDataLoss(const std::string& reason) override
    {
        TST_LOG("Modified document being unloaded: " << reason);

        // We expect this to happen only with the disonnection test,
        // because only in that case there is no user input.
        LOK_ASSERT_MESSAGE("Expected reason to be 'Data-loss detected'",
                           reason.starts_with("Data-loss detected"));
        LOK_ASSERT_STATE(_phase, Phase::Unload);

        return failed();
    }

    // Wait for clean unloading.
    void onDocBrokerDestroy(const std::string& docKey) override
    {
        TST_LOG("Testing with dockey [" << docKey << "] closed.");
        LOK_ASSERT_STATE(_phase, Phase::Unload);

        const std::string documentUrl = Uri::encode(helpers::getTestServerURI() + "/wopi/files/0");
        const std::string quarantinePath = _quarantinePath + '/' + documentUrl;
        const std::vector<std::string> files = getQuarantineFiles(testname, quarantinePath);
        LOK_ASSERT_MESSAGE("Expected 1 quaratined files in [" << quarantinePath << ']',
                           files.size() == 1);

        passTest("Found the document quarantined as expected");
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                // Always transition before issuing commands.
                TRANSITION_STATE(_phase, Phase::WaitLoadStatus);

                TST_LOG("Creating first connection");
                initWebsocket("/wopi/files/0?access_token=anything");

                TST_LOG("Loading view");
                WSD_CMD_BY_CONNECTION_INDEX(0, "load url=" + getWopiSrc());
                break;
            }
            case Phase::WaitLoadStatus:
            case Phase::WaitModifyStatus:
            case Phase::WaitUpload:
            case Phase::Unload:
            case Phase::Done:
            {
                // just wait for the results
                break;
            }
        }
    }
};

UnitBase** unit_create_wsd_multi(void)
{
    return new UnitBase*[3]{
        new UnitQuarantineCrash(), // Crash first, since we need to know of all new Kit processes.
        new UnitQuarantineConflict(), nullptr
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
