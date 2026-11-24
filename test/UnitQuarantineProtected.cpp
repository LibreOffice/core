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
 * The versions of documents whose kits crashed are protected: a document that is only opened
 * gets no version when there is room only by removing protected ones, and then no other version
 * goes for it either.
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
#include <fstream>
#include <memory>
#include <string>
#include <vector>

class UnitQuarantineProtected : public WopiTestServer
{
    using Base = WopiTestServer;

    /// The documents opened, by file id 1 to 4. The kits of the first two, which are large,
    /// crash. The third is small and stays open. The fourth is large again: two large ones fit into
    /// the quarantine, three do not.
    static constexpr std::size_t Documents = 4;
    static constexpr std::size_t Crashing = 2;
    static constexpr std::size_t Small = 2;

    STATE_ENUM(Phase, Load, WaitLoad, WaitCrash, Done) _phase;
    std::string _quarantinePath;
    /// The kit of the document being opened.
    pid_t _kitPid;
    /// How many documents were opened.
    std::size_t _loaded;

    static std::string fileId(std::size_t index) { return std::to_string(index + 1); }

    /// The contents of the document with the given index, a text of about 450 KB, or a small one.
    static std::string contents(std::size_t index)
    {
        if (index == Small)
            return "Small";

        std::string text;
        const std::string line = std::to_string(index) + std::string(1000, 'x') + '\n';
        while (text.size() < 450 * 1024)
            text += line;
        return text;
    }

    std::string documentPath(const std::string& id)
    {
        return _quarantinePath + '/' + Uri::encode(getWopiHostURI() + "/wopi/files/" + id);
    }

    /// The quarantined versions of the document with the given file id.
    std::vector<std::string> versions(const std::string& id)
    {
        std::vector<std::string> names;
        if (Poco::File(documentPath(id)).exists())
            Poco::File(documentPath(id)).list(names);

        std::vector<std::string> result;
        for (const std::string& name : names)
        {
            // The state file of a document is no version.
            if (name != "state")
                result.emplace_back(name);
        }

        TST_LOG("Document [" << id << "] has " << result.size() << " quarantined versions");
        return result;
    }

    /// The lines of the state file of the document with the given file id.
    std::vector<std::string> state(const std::string& id)
    {
        std::vector<std::string> lines;
        std::ifstream file(documentPath(id) + "/state");
        std::string line;
        while (std::getline(file, line))
            lines.emplace_back(line);
        return lines;
    }

public:
    UnitQuarantineProtected()
        : Base("UnitQuarantineProtected")
        , _phase(Phase::Load)
        , _kitPid(-1)
        , _loaded(0)
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
        return "document" + extractFilenameFromWopiUri(uri.getPath()) + ".txt";
    }

    void onDocBrokerAttachKitProcess(const std::string& docKey, int pid) override
    {
        TST_LOG("DocBroker [" << docKey << "] attached to kit: " << pid);
        _kitPid = pid;
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoad);
        ++_loaded;

        if (_loaded <= Crashing)
        {
            TRANSITION_STATE(_phase, Phase::WaitCrash);
            TST_LOG("Killing kit: " << _kitPid);
            ::kill(_kitPid, SIGKILL);
            return true;
        }

        if (_loaded < Documents)
        {
            TRANSITION_STATE(_phase, Phase::Load);
            return true;
        }

        TRANSITION_STATE(_phase, Phase::Done);

        // The last document did not fit without removing a protected version, so it got none,
        // and the version of the small document stayed.
        LOK_ASSERT_EQUAL_MESSAGE("Expected no version of the last document",
                                 static_cast<std::size_t>(0), versions(fileId(3)).size());
        LOK_ASSERT_EQUAL_MESSAGE("Expected the version of the small document kept",
                                 static_cast<std::size_t>(1), versions(fileId(Small)).size());
        const std::vector<std::string> openState = state(fileId(3));
        LOK_ASSERT_MESSAGE("Expected the state of the last document", !openState.empty());
        LOK_ASSERT_EQUAL_MESSAGE("Expected the last document noted as open",
                                 std::string("open"), openState[0]);

        for (std::size_t i = 0; i < Crashing; ++i)
        {
            const std::vector<std::string> kept = versions(fileId(i));
            LOK_ASSERT_EQUAL_MESSAGE("Expected the version of a crashed document kept",
                                     static_cast<std::size_t>(1), kept.size());

            // The state of a crashed document lists its version as protected.
            const std::vector<std::string> lines = state(fileId(i));
            LOK_ASSERT_EQUAL_MESSAGE("Expected the state of a crashed document",
                                     static_cast<std::size_t>(2), lines.size());
            LOK_ASSERT_EQUAL(std::string("closed"), lines[0]);
            LOK_ASSERT_EQUAL(kept[0], lines[1]);
        }

        passTest("The protected versions were kept");
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
        if (_phase == Phase::WaitCrash)
            TRANSITION_STATE(_phase, Phase::Load);
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                setFileContent(contents(_loaded));
                TRANSITION_STATE(_phase, Phase::WaitLoad);

                // The new connection is the first one.
                TST_LOG("Loading document " << fileId(_loaded));
                initWebsocket("/wopi/files/" + fileId(_loaded) + "?access_token=anything");
                WSD_CMD_BY_CONNECTION_INDEX(0, "load url=" + getWopiSrc());
                break;
            }
            case Phase::WaitLoad:
            case Phase::WaitCrash:
            case Phase::Done:
                break;
        }
    }
};

UnitBase* unit_create_wsd(void) { return new UnitQuarantineProtected(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
