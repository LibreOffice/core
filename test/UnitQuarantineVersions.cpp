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
 * The quarantine keeps at most max_versions_to_maintain versions of a document, the newest ones,
 * however recent the older ones are.
 */

#include <config.h>

#include <WopiTestServer.hpp>
#include <Unit.hpp>
#include <lokassert.hpp>
#include <testlog.hpp>
#include <common/FileUtil.hpp>

#include <Poco/File.h>
#include <Poco/Path.h>
#include <Poco/Util/LayeredConfiguration.h>

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

class UnitQuarantineVersions : public WopiTestServer
{
    using Base = WopiTestServer;

    /// How many versions the quarantine keeps of the document.
    static constexpr std::size_t MaxVersions = 2;
    /// How many times the document is modified and saved.
    static constexpr int Saves = 3;

    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitModifyStatus, WaitSaved, Done) _phase;
    std::string _quarantinePath;
    int _saves;
    /// Whether the result of the last save came, which is after its version is quarantined.
    bool _saved;
    /// Whether the document was reported unmodified after the last save.
    bool _unmodified;

    void modify()
    {
        TST_LOG("Modifying");
        WSD_CMD("key type=input char=97 key=0");
        WSD_CMD("key type=up char=0 key=512");
    }

    /// Modifies the document again once the last save is done and the document was reported
    /// unmodified, in whichever order those come.
    void modifyAfterSave()
    {
        if (!_saved || !_unmodified)
            return;

        _saved = false;
        _unmodified = false;
        TRANSITION_STATE(_phase, Phase::WaitModifyStatus);
        modify();
    }

public:
    UnitQuarantineVersions()
        : Base("UnitQuarantineVersions")
        , _phase(Phase::Load)
        , _saves(0)
        , _saved(false)
        , _unmodified(false)
    {
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
        config.setUInt("quarantine_files.max_versions_to_maintain", MaxVersions);
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);

        TRANSITION_STATE(_phase, Phase::WaitModifyStatus);
        modify();
        return true;
    }

    bool onDocumentModified(const std::string& message) override
    {
        TST_LOG("Got [" << message << ']');
        if (_phase != Phase::WaitModifyStatus)
            return true;

        TRANSITION_STATE(_phase, Phase::WaitSaved);
        TST_LOG("Saving the document");
        WSD_CMD("save dontTerminateEdit=0 dontSaveIfUnmodified=0");
        return true;
    }

    bool onDocumentUnmodified(const std::string& message) override
    {
        TST_LOG("Got [" << message << ']');

        // A save can be followed by more than one notice that the document is unmodified.
        if (_phase != Phase::WaitSaved)
            return true;

        _unmodified = true;
        modifyAfterSave();
        return true;
    }

    bool onDocumentSaved(const std::string& message, bool success,
                         const std::string& result) override
    {
        TST_LOG("Saved: [" << message << "], success: " << success << ", result: " << result);
        LOK_ASSERT_STATE(_phase, Phase::WaitSaved);
        LOK_ASSERT_MESSAGE("Expected the save to succeed", success);

        ++_saves;
        if (_saves < Saves)
        {
            _saved = true;
            modifyAfterSave();
            return true;
        }

        TRANSITION_STATE(_phase, Phase::Done);

        // The document as it was opened and the first save are gone, although they are recent.
        const std::string documentUrl = Uri::encode(getWopiHostURI() + "/wopi/files/0");
        const std::string quarantinePath = _quarantinePath + '/' + documentUrl;
        std::vector<std::string> files;
        Poco::File(quarantinePath).list(files);
        std::sort(files.begin(), files.end());
        for (const std::string& file : files)
            TST_LOG("Found quarantined file [" << file << ']');

        LOK_ASSERT_EQUAL_MESSAGE("Expected only the newest versions quarantined in ["
                                     << quarantinePath << ']',
                                 MaxVersions, files.size());

        const std::unique_ptr<std::vector<char>> newest =
            FileUtil::readFile(quarantinePath + '/' + files[files.size() - 1]);
        LOK_ASSERT(newest);
        const std::string newestContents(newest->begin(), newest->end());
        LOK_ASSERT_MESSAGE("Expected the last save as the newest version, got [" << newestContents
                                                                                 << ']',
                           newestContents.starts_with(std::string(Saves, 'a') + "Hello"));

        passTest("Only the newest versions are quarantined");
        return true;
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                TRANSITION_STATE(_phase, Phase::WaitLoadStatus);

                TST_LOG("Loading view");
                initWebsocket("/wopi/files/0?access_token=anything");
                WSD_CMD_BY_CONNECTION_INDEX(0, "load url=" + getWopiSrc());
                break;
            }
            case Phase::WaitLoadStatus:
            case Phase::WaitModifyStatus:
            case Phase::WaitSaved:
            case Phase::Done:
                break;
        }
    }
};

UnitBase* unit_create_wsd(void) { return new UnitQuarantineVersions(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
