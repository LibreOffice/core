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
 * When the quarantine is full, the versions of a document whose kit crashed go last, even when
 * they are the oldest.
 */

#include <config.h>

#include <WopiTestServer.hpp>
#include <Unit.hpp>
#include <lokassert.hpp>
#include <testlog.hpp>
#include <common/FileUtil.hpp>

#include <Poco/File.h>
#include <Poco/Path.h>
#include <Poco/URI.h>
#include <Poco/Util/LayeredConfiguration.h>

#include <chrono>
#include <csignal>
#include <string>
#include <vector>

class UnitQuarantineEviction : public WopiTestServer
{
    using Base = WopiTestServer;

    /// How many large documents are opened after the crash. Together they are more than the
    /// quarantine holds, each alone is less.
    static constexpr std::size_t LargeDocuments = 3;

    STATE_ENUM(Phase, Load, WaitCrashedLoad, WaitModify, WaitCrash, LoadLarge, WaitLargeLoad,
               Done)
    _phase;
    std::string _quarantinePath;
    /// The kit of the document that crashes, -1 until it is attached.
    pid_t _crashKitPid;
    /// How many of the large documents were opened.
    std::size_t _largeLoaded;

    /// The file id of the large document with the given index.
    static std::string largeFileId(std::size_t index) { return std::to_string(index + 1); }

    /// The contents of the large document with the given index, a text of about 450 KB.
    static std::string largeContents(std::size_t index)
    {
        std::string contents;
        const std::string line = std::to_string(index) + std::string(1000, 'x') + '\n';
        while (contents.size() < 450 * 1024)
            contents += line;
        return contents;
    }

    /// The number of quarantined versions of the document with the given file id.
    std::size_t countVersions(const std::string& fileId)
    {
        const std::string docKey = Uri::encode(getWopiHostURI() + "/wopi/files/" + fileId);
        const std::string path = _quarantinePath + '/' + docKey;
        std::vector<std::string> names;
        if (Poco::File(path).exists())
            Poco::File(path).list(names);

        std::size_t versions = 0;
        for (const std::string& name : names)
        {
            // The state file of a document is no version.
            if (name != "state")
                ++versions;
        }

        TST_LOG("Document [" << fileId << "] has " << versions << " quarantined versions");
        return versions;
    }

    void loadLarge()
    {
        setFileContent(largeContents(_largeLoaded));

        TRANSITION_STATE(_phase, Phase::WaitLargeLoad);
        const std::string fileId = largeFileId(_largeLoaded);
        TST_LOG("Loading large document " << fileId);
        // The new connection is the first one.
        initWebsocket("/wopi/files/" + fileId + "?access_token=anything");
        WSD_CMD_BY_CONNECTION_INDEX(0, "load url=" + getWopiSrc());
    }

public:
    UnitQuarantineEviction()
        : Base("UnitQuarantineEviction")
        , _phase(Phase::Load)
        , _crashKitPid(-1)
        , _largeLoaded(0)
    {
        setTimeout(std::chrono::minutes(2));
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        Base::configure(config);

        config.setBool("quarantine_files[@enable]", true);
        Poco::Path rootPath(config.getString("child_root_path", ""));
        rootPath.popDirectory().pushDirectory("quarantine");
        _quarantinePath = FileUtil::createRandomTmpDir(rootPath.toString());
        TST_LOG("Quarantine path set to [" << _quarantinePath << ']');
        config.setString("quarantine_files.path", _quarantinePath);
        config.setUInt("quarantine_files.limit_dir_size_mb", 1);
    }

    std::string getFilename(const Poco::URI& uri) const override
    {
        const std::string fileId = extractFilenameFromWopiUri(uri.getPath());
        for (std::size_t i = 0; i < LargeDocuments; ++i)
        {
            if (fileId == largeFileId(i))
                return "large" + fileId + ".txt";
        }

        return Base::getFilename(uri);
    }

    void onDocBrokerAttachKitProcess(const std::string& docKey, int pid) override
    {
        TST_LOG("DocBroker [" << docKey << "] attached to kit: " << pid);
        if (_phase == Phase::WaitCrashedLoad)
            _crashKitPid = pid;
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');

        if (_phase == Phase::WaitCrashedLoad)
        {
            TRANSITION_STATE(_phase, Phase::WaitModify);
            WSD_CMD_BY_CONNECTION_INDEX(0, "key type=input char=97 key=0");
            WSD_CMD_BY_CONNECTION_INDEX(0, "key type=up char=0 key=512");
            return true;
        }

        LOK_ASSERT_STATE(_phase, Phase::WaitLargeLoad);
        ++_largeLoaded;
        if (_largeLoaded < LargeDocuments)
        {
            TRANSITION_STATE(_phase, Phase::LoadLarge);
            return true;
        }

        TRANSITION_STATE(_phase, Phase::Done);

        // The last document needed room. The oldest version was that of the crashed document,
        // but one of the first two large documents went instead. Those two can have the same
        // age, so either can go.
        LOK_ASSERT_EQUAL_MESSAGE("Expected the version of the crashed document kept",
                                 static_cast<std::size_t>(1), countVersions("0"));
        LOK_ASSERT_EQUAL_MESSAGE("Expected one of the first two large documents removed",
                                 static_cast<std::size_t>(1),
                                 countVersions(largeFileId(0)) + countVersions(largeFileId(1)));
        LOK_ASSERT_EQUAL_MESSAGE("Expected the version of the last large document kept",
                                 static_cast<std::size_t>(1), countVersions(largeFileId(2)));

        passTest("The versions of the crashed document were kept");
        return true;
    }

    bool onDocumentModified(const std::string& message) override
    {
        TST_LOG("Got [" << message << ']');
        if (_phase != Phase::WaitModify)
            return true;

        TRANSITION_STATE(_phase, Phase::WaitCrash);
        TST_LOG("Killing kit: " << _crashKitPid);
        ::kill(_crashKitPid, SIGKILL);
        return true;
    }

    bool onDataLoss(const std::string& reason) override
    {
        TST_LOG("Modified document being unloaded: " << reason);
        return failed();
    }

    void onDocBrokerDestroy(const std::string& docKey) override
    {
        TST_LOG("DocBroker [" << docKey << "] is gone");
        if (_phase != Phase::WaitCrash)
            return;

        LOK_ASSERT_EQUAL_MESSAGE("Expected the crashed document quarantined",
                                 static_cast<std::size_t>(1), countVersions("0"));
        TRANSITION_STATE(_phase, Phase::LoadLarge);
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                TRANSITION_STATE(_phase, Phase::WaitCrashedLoad);

                TST_LOG("Loading the document that crashes");
                initWebsocket("/wopi/files/0?access_token=anything");
                WSD_CMD_BY_CONNECTION_INDEX(0, "load url=" + getWopiSrc());
                break;
            }
            case Phase::LoadLarge:
                loadLarge();
                break;
            case Phase::WaitCrashedLoad:
            case Phase::WaitModify:
            case Phase::WaitCrash:
            case Phase::WaitLargeLoad:
            case Phase::Done:
                break;
        }
    }
};

UnitBase* unit_create_wsd(void) { return new UnitQuarantineEviction(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
