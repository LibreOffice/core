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

#include <Poco/File.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Path.h>
#include <algorithm>
#include <chrono>
#include <csignal>
#include <ctime>
#include <memory>

namespace
{
std::vector<std::string> getQuarantineFiles(const std::string testname,
                                            const std::string& quarantinePath)
{
    std::vector<std::string> files;
    Poco::File(quarantinePath).list(files);

    // The names start with the time of quarantining, so this puts the oldest first.
    std::sort(files.begin(), files.end());

    TST_LOG("Found " << files.size() << " quarantine file in [" << quarantinePath << ']');
    for (std::size_t i = 0; i < files.size(); ++i)
    {
        TST_LOG("Found quarantine file #" << (i + 1) << ": [" << files[i] << ']');
    }

    return files;
}

std::string readQuarantineFile(const std::string& quarantinePath, const std::string& file)
{
    const std::unique_ptr<std::vector<char>> contents =
        FileUtil::readFile(quarantinePath + '/' + file);
    return contents ? std::string(contents->begin(), contents->end()) : std::string();
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

    static constexpr std::size_t LimitStoreFailures = 2;
    static constexpr bool SaveOnExit = true;

public:
    UnitQuarantineConflict()
        : Base("UnitQuarantineConflict", OriginalDocContent)
        , _unloadingModifiedDocDetected(true)
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
        }

        // Every failed upload is followed by a CheckFileInfo to reconcile with
        // storage, on top of the one we do when loading.
        setExpectedCheckFileInfo(getExpectedPutFile() + 1);

        // Each scenario checks only what it quarantined itself. The document is downloaded, and
        // quarantined as it was opened, only after this.
        FileUtil::removeFile(_quarantinePath + '/' + docKey, true);
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

        // We don't expect overwriting by forced uploading. Nothing writes to
        // storage here, so there is no conflict and every upload, the retry and
        // the always_save_on_exit one included, keeps its timestamp guard.
        LOK_ASSERT_EQUAL_MESSAGE("Unexpected overwritting the document in storage", false, force);

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

        // Uploading fails permanently, but nothing ever writes to storage, so the
        // timestamp there never moves and there is nothing to conflict with.
        LOK_ASSERT_MESSAGE("Expect only savefailed errors",
                           message.starts_with("error: cmd=storage kind=savefailed"));

        // Close the document.
        TST_LOG("Closing the document");
        WSD_CMD("closedocument");

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

        const std::string documentUrl = Uri::encode(getWopiHostURI() + "/wopi/files/0");
        const std::string quarantinePath = _quarantinePath + '/' + documentUrl;
        const std::vector<std::string> files = getQuarantineFiles(testname, quarantinePath);

        // The document as it was opened comes first, then the modified version that could not be
        // uploaded.
        const std::size_t expected = _scenario == Scenario::VerifyOverwrite ? 1 : 2;
        LOK_ASSERT_EQUAL_MESSAGE("Unexpected number of quarantined files in [" << quarantinePath
                                                                                << ']',
                                 expected, files.size());
        LOK_ASSERT_EQUAL_MESSAGE("Expected the document as it was opened first",
                                 std::string(OriginalDocContent),
                                 readQuarantineFile(quarantinePath, files[0]));
        if (expected == 2)
            LOK_ASSERT_EQUAL_MESSAGE("Expected the modified document last",
                                     std::string(ModifiedOriginalDocContent),
                                     readQuarantineFile(quarantinePath, files[1]));

        Base::onDocBrokerDestroy(docKey);
    }
};

/// This test simulates a crashing kit with modifications.
class UnitQuarantineCrash : public WopiTestServer
{
    using Base = WopiTestServer;

    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitModifyStatus, WaitUpload, Unload, Done) _phase;
    std::string _quarantinePath;
    /// The kit that hosts the document, -1 until it is attached.
    pid_t _docKitPid;

public:
    UnitQuarantineCrash()
        : Base("UnitQuarantineCrash")
        , _phase(Phase::Load)
        , _docKitPid(-1)
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

    void onDocBrokerAttachKitProcess(const std::string& docKey, int pid) override
    {
        TST_LOG("DocBroker [" << docKey << "] attached to kit: " << pid);
        _docKitPid = pid;
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

            // Kill only the kit that hosts the document, so the spare kits stay usable for the
            // next test.
            TST_LOG("Killing kit: " << _docKitPid);
            ::kill(_docKitPid, SIGKILL);
        }

        return true;
    }

    void kitKilled(int count) override
    {
        TST_LOG("Kit killed");
        LOK_ASSERT_EQUAL_MESSAGE("Expected only 1 killed kit", 1, count);
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

        const std::string documentUrl = Uri::encode(getWopiHostURI() + "/wopi/files/0");
        const std::string quarantinePath = _quarantinePath + '/' + documentUrl;
        const std::vector<std::string> files = getQuarantineFiles(testname, quarantinePath);
        // The document as it was opened, and the version saved before the crash.
        LOK_ASSERT_EQUAL_MESSAGE("Expected 2 quarantined files in [" << quarantinePath << ']',
                                 static_cast<std::size_t>(2), files.size());

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

/// The kit dies with modifications before the document was ever saved.
class UnitQuarantineCrashBeforeSave : public WopiTestServer
{
    using Base = WopiTestServer;

    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitModifyStatus, Unload, Done) _phase;
    std::string _quarantinePath;
    /// The kit that hosts the document, -1 until it is attached.
    pid_t _docKitPid;

public:
    UnitQuarantineCrashBeforeSave()
        : Base("UnitQuarantineCrashBeforeSave")
        , _phase(Phase::Load)
        , _docKitPid(-1)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        Base::configure(config);

        // The quarantine is set up once, with the configuration of the first test that runs.
        if (!config.getBool("quarantine_files[@enable]", false) ||
            config.getString("quarantine_files.path", std::string()).empty())
        {
            config.setBool("quarantine_files[@enable]", true);
            Poco::Path rootPath(config.getString("child_root_path", ""));
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

    void onDocBrokerAttachKitProcess(const std::string& docKey, int pid) override
    {
        TST_LOG("DocBroker [" << docKey << "] attached to kit: " << pid);
        _docKitPid = pid;
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);

        // The document is quarantined as soon as it is opened, before anything goes wrong.
        const std::string documentUrl = Uri::encode(getWopiHostURI() + "/wopi/files/0");
        const std::string quarantinePath = _quarantinePath + '/' + documentUrl;
        const std::vector<std::string> files = getQuarantineFiles(testname, quarantinePath);
        LOK_ASSERT_EQUAL_MESSAGE("Expected the opened document quarantined in [" << quarantinePath
                                                                                  << ']',
                                 static_cast<std::size_t>(1), files.size());
        LOK_ASSERT_EQUAL_MESSAGE("Expected the document as it was opened", getFileContent(),
                                 readQuarantineFile(quarantinePath, files[0]));

        TRANSITION_STATE(_phase, Phase::WaitModifyStatus);

        TST_LOG("Modifying");
        WSD_CMD("key type=input char=97 key=0");
        WSD_CMD("key type=up char=0 key=512");

        return true;
    }

    bool onDocumentModified(const std::string& message) override
    {
        TST_LOG("Got [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitModifyStatus);

        TRANSITION_STATE(_phase, Phase::Unload);

        // Kill only the kit that hosts the document, so the spare kits stay usable for the
        // next test.
        TST_LOG("Killing kit before any save: " << _docKitPid);
        ::kill(_docKitPid, SIGKILL);

        return true;
    }

    void kitKilled(int count) override
    {
        TST_LOG("Kit killed");
        LOK_ASSERT_EQUAL_MESSAGE("Expected only 1 killed kit", 1, count);
    }

    bool onDataLoss(const std::string& reason) override
    {
        TST_LOG("Modified document being unloaded: " << reason);
        LOK_ASSERT_STATE(_phase, Phase::Unload);
        return failed();
    }

    void onDocBrokerDestroy(const std::string& docKey) override
    {
        TST_LOG("Testing with dockey [" << docKey << "] closed.");
        LOK_ASSERT_STATE(_phase, Phase::Unload);

        // The document was quarantined when it was opened, as no save ever made a version.
        const std::string documentUrl = Uri::encode(getWopiHostURI() + "/wopi/files/0");
        const std::string quarantinePath = _quarantinePath + '/' + documentUrl;
        const std::vector<std::string> files = getQuarantineFiles(testname, quarantinePath);
        LOK_ASSERT_EQUAL_MESSAGE("Expected 1 quarantined file in [" << quarantinePath << ']',
                                 static_cast<std::size_t>(1), files.size());

        LOK_ASSERT_EQUAL_MESSAGE("Expected the document as it was opened", getFileContent(),
                                 readQuarantineFile(quarantinePath, files[0]));

        passTest("Found the original document quarantined as expected");
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
            case Phase::Unload:
            case Phase::Done:
            {
                // just wait for the results
                break;
            }
        }
    }
};

/// A document that is opened again, unchanged, has its version renewed, so the version ages from
/// the second opening.
class UnitQuarantineReopen : public WopiTestServer
{
    using Base = WopiTestServer;

    STATE_ENUM(Phase, Load, WaitFirstLoad, WaitFirstClose, Reload, WaitSecondLoad, Done) _phase;
    std::string _quarantinePath;
    std::string _firstVersion;
    std::chrono::steady_clock::time_point _closed;
    /// The time of the second opening, in seconds since the epoch.
    std::time_t _reopened = 0;

    std::string quarantinePathOfDocument()
    {
        return _quarantinePath + '/' + Uri::encode(getWopiHostURI() + "/wopi/files/0");
    }

public:
    UnitQuarantineReopen()
        : Base("UnitQuarantineReopen")
        , _phase(Phase::Load)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        Base::configure(config);

        // The quarantine is set up once, with the configuration of the first test that runs.
        if (!config.getBool("quarantine_files[@enable]", false) ||
            config.getString("quarantine_files.path", std::string()).empty())
        {
            config.setBool("quarantine_files[@enable]", true);
            Poco::Path rootPath(config.getString("child_root_path", ""));
            rootPath.popDirectory().pushDirectory("quarantine");
            _quarantinePath = FileUtil::createRandomTmpDir(rootPath.toString());
            config.setString("quarantine_files.path", _quarantinePath);
        }
        else
        {
            _quarantinePath = config.getString("quarantine_files.path", std::string());
        }

        TST_LOG("Quarantine path at [" << _quarantinePath << ']');
        FileUtil::removeFile(_quarantinePath, true);
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');
        const std::vector<std::string> files =
            getQuarantineFiles(testname, quarantinePathOfDocument());
        LOK_ASSERT_EQUAL_MESSAGE("Expected one quarantined version", static_cast<std::size_t>(1),
                                 files.size());

        if (_phase == Phase::WaitFirstLoad)
        {
            _firstVersion = files[0];
            TRANSITION_STATE(_phase, Phase::WaitFirstClose);
            deleteSocketAt(0);
            return true;
        }

        LOK_ASSERT_STATE(_phase, Phase::WaitSecondLoad);
        TRANSITION_STATE(_phase, Phase::Done);

        // The names start with the time, so a renewed version sorts after the first one, and is
        // not older than the second opening.
        LOK_ASSERT_MESSAGE("Expected the version renewed, was [" << _firstVersion << "], is ["
                                                                 << files[0] << ']',
                           _firstVersion < files[0]);
        LOK_ASSERT_MESSAGE("Expected the version to age from the second opening, is ["
                               << files[0] << "], opened at " << _reopened,
                           std::stoll(files[0]) >= static_cast<long long>(_reopened));
        passTest("The version was renewed");
        return true;
    }

    void onDocBrokerDestroy(const std::string& docKey) override
    {
        TST_LOG("DocBroker [" << docKey << "] is gone");
        if (_phase == Phase::WaitFirstClose)
        {
            _closed = std::chrono::steady_clock::now();
            TRANSITION_STATE(_phase, Phase::Reload);
        }
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
                TRANSITION_STATE(_phase, Phase::WaitFirstLoad);
                initWebsocket("/wopi/files/0?access_token=anything");
                WSD_CMD_BY_CONNECTION_INDEX(0, "load url=" + getWopiSrc());
                break;
            case Phase::Reload:
                // The time in the name has a resolution of a second.
                if (std::chrono::steady_clock::now() - _closed < std::chrono::milliseconds(1100))
                    break;

                TRANSITION_STATE(_phase, Phase::WaitSecondLoad);
                _reopened = std::time(nullptr);
                initWebsocket("/wopi/files/0?access_token=anything");
                WSD_CMD_BY_CONNECTION_INDEX(0, "load url=" + getWopiSrc());
                break;
            case Phase::WaitFirstLoad:
            case Phase::WaitFirstClose:
            case Phase::WaitSecondLoad:
            case Phase::Done:
                break;
        }
    }
};

UnitBase** unit_create_wsd_multi(void)
{
    // UnitQuarantineReopen runs first, while no other test has given its document versions.
    return new UnitBase*[5]{
        new UnitQuarantineReopen(),
        new UnitQuarantineCrash(),
        new UnitQuarantineCrashBeforeSave(),
        new UnitQuarantineConflict(), nullptr
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
