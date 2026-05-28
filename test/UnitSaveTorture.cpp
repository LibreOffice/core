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
 * Unit test for stress testing document saving under heavy load.
 */

#include <config.h>

#include <test/UnitWSDClient.hpp>
#include <Unit.hpp>
#include <common/Util.hpp>
#include <common/JsonUtil.hpp>
#include <common/FileUtil.hpp>
#include <common/Log.hpp>
#include <JailUtil.hpp>
#include <helpers.hpp>
#include <common/StringVector.hpp>
#include <WebSocketSession.hpp>
#include <unistd.h>
#include <wsd/COOLWSD.hpp>
#include <wsd/DocumentBroker.hpp>
#include <test/lokassert.hpp>
#include <Poco/Util/LayeredConfiguration.h>

#include <chrono>
#include <string>
#include <thread>

using namespace std::literals;

constexpr auto StampFileCheckPeriodMs = 100ms;

/// Base class for Save Torture test cases.
class UnitSaveTortureBase : public UnitWSDClient
{
    bool _forceAutosave;

protected:
    UnitSaveTortureBase(const std::string& name)
        : UnitWSDClient(name)
        , _forceAutosave(false)
    {
        setHasKitHooks();
        // 4x the default.
        constexpr std::chrono::minutes timeout_minutes(2);
        setTimeout(timeout_minutes);
    }

    void modifyDocument()
    {
        TST_LOG("Modifying");

        // move to another cell?
        WSD_CMD("key type=input char=13 key=1280");
        WSD_CMD("key type=up char=0 key=1280");
        // enter - some text.
        WSD_CMD("textinput id=0 text=foo");
        // enter - commit to a cell in calc eg.
        WSD_CMD("key type=input char=13 key=1280");
        WSD_CMD("key type=up char=0 key=1280");
    }

    std::string getJailRootPath(const std::string& name)
    {
        return FileUtil::buildLocalPathToJail(JailUtil::isMountNamespacesEnabled(), getJailRoot(),
                                              "/tmp/" + name);
    }

    void createStamp(const std::string& name)
    {
        const auto path = getJailRootPath(name);
        TST_LOG("create stamp " << name << ": " << path);
        std::ofstream stamp(path);
        stamp.close();
    }

    void removeStamp(const std::string& name)
    {
        FileUtil::removeFile(getJailRootPath(name));
        TST_LOG("removed stamp " << name);
    }

    // Force background autosave when saving the modified document
    bool isAutosave() override { return _forceAutosave; }
    void forceAutosave() { _forceAutosave = true; }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        UnitWSD::configure(config);

        // Force much faster auto-saving
        config.setBool("per_document.background_autosave", true);
    }
};

class UnitModified : public UnitSaveTortureBase
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitModifiedStatus) _phase;
    int _modifyCycleCount; ///< Number of times to modify.

public:
    UnitModified()
        : UnitSaveTortureBase("UnitModified")
        , _phase(Phase::Load)
        , _modifyCycleCount(4)
    {
    }

    /// The document is loaded.
    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("Got: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);

        TRANSITION_STATE(_phase, Phase::WaitModifiedStatus);

        modifyDocument();

        return true;
    }

    bool onDocumentModified(const std::string& message) override
    {
        TST_LOG("Got: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitModifiedStatus);

        if (--_modifyCycleCount == 0)
        {
            passTest("Force-modified successfully multiple times");
        }
        else
        {
            // It is vital that we can change the modified status successfully
            // and also get correct notifications from the core for bgsave to work.
            const std::string args =
                "{ \"Modified\": { \"type\": \"boolean\", \"value\": \"false\" } }";
            TST_LOG("post force modified command: .uno:Modified " << args);
            WSD_CMD("uno .uno:Modified " + args);

            modifyDocument();
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

                const std::string docName = "empty.ods";
                TST_LOG("Loading document: " << docName);
                connectAndLoadLocalDocument(docName);
                break;
            }
            case Phase::WaitLoadStatus:
            case Phase::WaitModifiedStatus:
            {
                // just wait for the results
                break;
            }
        }
    }
};

class UnitTileCombineRace : public UnitSaveTortureBase
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitDocClose) _phase;

public:
    UnitTileCombineRace()
        : UnitSaveTortureBase("UnitTileCombineRace")
        , _phase(Phase::Load)
    {
    }

    /// The document is loaded.
    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("Got: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);

        TRANSITION_STATE(_phase, Phase::WaitDocClose);

        modifyDocument();

        // We need the tilecombine and save in the same drainQueue in this order:
        createStamp("holddrainqueue");

        WSD_CMD("tilecombine nviewid=0 part=0 width=256 height=256 tileposx=0,3840,7680 "
                "tileposy=0,0,0 tilewidth=3840 tileheight=3840");

        // Force a background save-as-auto-save now.
        forceAutosave();
        WSD_CMD("save dontTerminateEdit=0 dontSaveIfUnmodified=0");

        removeStamp("holddrainqueue");

        return true;
    }

    bool onDocumentSaved(const std::string& message, bool success,
                         [[maybe_unused]] const std::string& result) override
    {
        TST_LOG("Save result: " << message);

        // Check the save succeeded & kit didn't crash.
        LOK_ASSERT_MESSAGE("Expected save to succeed", success);

        exitTest(success ? TestResult::Ok : TestResult::Failed);

        return true;
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                TRANSITION_STATE(_phase, Phase::WaitLoadStatus);

                const std::string docName = "empty.ods";
                TST_LOG("Loading document: " << docName);
                connectAndLoadLocalDocument(docName);
                break;
            }
            case Phase::WaitLoadStatus:
            case Phase::WaitDocClose:
            {
                // just wait for the results
                break;
            }
        }
    }
};

class UnitBgSaveCrash : public UnitSaveTortureBase
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitModifiedStatus, WaitDocClose) _phase;
    STATE_ENUM(Case, Background, Foreground) _case;

public:
    UnitBgSaveCrash()
        : UnitSaveTortureBase("UnitBgSaveCrash")
        , _phase(Phase::Load)
        , _case(Case::Background)
    {
    }

    /// The document is loaded.
    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("Got: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);

        TRANSITION_STATE(_phase, Phase::WaitModifiedStatus);

        modifyDocument();

        return true;
    }

    bool onDocumentModified(const std::string& message) override
    {
        TST_LOG("Got: [" << message << ']');

        // When the BG save fails, we get the unmodified state again.
        if (_case == Case::Background)
        {
            LOK_ASSERT_STATE(_phase, Phase::WaitModifiedStatus);
            TRANSITION_STATE(_phase, Phase::WaitDocClose);
        }
        else
        {
            LOK_ASSERT_STATE(_phase, Phase::WaitDocClose);
        }

        createStamp("crashkitonsave");

        forceAutosave();

        // force a crashing save ...
        TST_LOG("Sending save request");
        WSD_CMD("save dontTerminateEdit=0 dontSaveIfUnmodified=0");

        return true;
    }

    bool onDocumentSaved(const std::string& message, bool success,
                         const std::string& result) override
    {
        TST_LOG("Save result: " << result);
        switch (_case)
        {
            case Case::Background:
                if (success)
                {
                    TST_LOG("Document failed to save");
                    failTest("Failed to save the document (Core is out-of-date or it has a "
                             "regression: " +
                             message);
                }
                else
                {
                    TST_LOG("Background save exited early as expected");
                    TRANSITION_STATE(_case, Case::Foreground);

                    TST_LOG("Sending save request to verify that foreground-saving is now used");
                    WSD_CMD("save dontTerminateEdit=0 dontSaveIfUnmodified=0");
                }
                break;
            case Case::Foreground:
                if (success)
                {
                    TST_LOG("(non)-background save succeeded on 2nd attempt");
                    passTest("Saved using foreground succeeded");
                }
                else
                {
                    TST_LOG("Document failed to save");
                    failTest("Failed to save the document (Core is out-of-date or it has a "
                             "regression: " +
                             message);
                }
                break;
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

                const std::string docName = "empty.ods";
                TST_LOG("Loading document: " << docName);
                connectAndLoadLocalDocument(docName);
                break;
            }
            case Phase::WaitLoadStatus:
            case Phase::WaitModifiedStatus:
            case Phase::WaitDocClose:
            {
                // just wait for the results
                break;
            }
        }
    }
};

class UnitSaveTortureOne : public UnitSaveTortureBase
{
    STATE_ENUM(Phase, Load, WaitLoadStatus, WaitFirstModifiedStatus, WaitAfterSaveModifiedStatus,
               WaitSaveStatus, WaitUnmodifiedStatus)
    _phase;

    Util::Stopwatch _stopwatch;
    const std::string _filename;
    const bool _modifyFirst;
    const bool _modifyAfterSaveStarts;

    void saveAndModifyDocument()
    {
        TST_LOG("Hold saving");
        createStamp("holdsave");

        if (!_modifyAfterSaveStarts)
        {
            TRANSITION_STATE(_phase, Phase::WaitSaveStatus);
        }

        // Force a background save-as-auto-save now.
        forceAutosave();
        WSD_CMD("save dontTerminateEdit=0 dontSaveIfUnmodified=0");

        if (_modifyAfterSaveStarts)
        {
            TST_LOG("Modify after saving starts");
            TRANSITION_STATE(_phase, Phase::WaitAfterSaveModifiedStatus);
            modifyDocument();
            TST_LOG("Wait for modified status");
        }
        else
        {
            TST_LOG("Allow saving to continue");
            removeStamp("holdsave");
            TRANSITION_STATE(_phase, Phase::WaitSaveStatus);
        }
    }

public:
    UnitSaveTortureOne(std::string filename, bool modifyFirst, bool modifyAfterSaveStarts,
                       const std::string& description)
        : UnitSaveTortureBase("UnitSaveTortureOne_" + description)
        , _phase(Phase::Load)
        , _filename(std::move(filename))
        , _modifyFirst(modifyFirst)
        , _modifyAfterSaveStarts(modifyAfterSaveStarts)
    {
    }

    /// The document is loaded.
    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("Phase: [" << name(_phase) << "] got: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoadStatus);

        // Tests assume all are background save.
        createStamp("abortonsyncsave");

        if (_modifyFirst)
        {
            TST_LOG("Modify First");
            TRANSITION_STATE(_phase, Phase::WaitFirstModifiedStatus);
            modifyDocument();
            TST_LOG("wait for first modified status");
        }
        else
        {
            TST_LOG("Save First");
            saveAndModifyDocument();
        }

        return true;
    }

    bool onDocumentModified(const std::string& message) override
    {
        TST_LOG("Phase: [" << name(_phase) << "] got: [" << message << ']');

        if (_phase == Phase::WaitFirstModifiedStatus)
        {
            TST_LOG("Save and modify after first modify");
            saveAndModifyDocument();
        }
        else if (_phase == Phase::WaitAfterSaveModifiedStatus)
        {
            TST_LOG("Allow saving to continue");
            removeStamp("holdsave");
            TRANSITION_STATE(_phase, Phase::WaitSaveStatus);
        }

        return true;
    }

    bool onDocumentUnmodified(const std::string& message) override
    {
        TST_LOG("Phase: [" << name(_phase) << "] got: [" << message << ']');

        // We get unmodified when loading and when saving; ignore them.
        if (_phase == Phase::WaitUnmodifiedStatus)
        {
            passTest();
        }

        return true;
    }

    bool onDocumentSaved(const std::string& message, bool success,
                         [[maybe_unused]] const std::string& result) override
    {
        TST_LOG("Phase: [" << name(_phase) << "] got: [" << message << ']');

        LOK_ASSERT_MESSAGE("Expected to be in WaitSaveStatus or WaitAfterSaveModifiedStatus or "
                           "WaitNoModifiedStatus",
                           _phase == Phase::WaitSaveStatus ||
                               _phase == Phase::WaitAfterSaveModifiedStatus ||
                               _phase == Phase::WaitUnmodifiedStatus);

        LOK_ASSERT_MESSAGE("Saved successfully", success);

        if (_phase != Phase::WaitUnmodifiedStatus)
        {
            _stopwatch.restart();
            TRANSITION_STATE(_phase, Phase::WaitUnmodifiedStatus);
        }

        if (!_modifyAfterSaveStarts)
        {
            if (_modifyFirst)
            {
                TST_LOG("wait for modified status");
            }
            else
            {
                passTest("We've never modified, so we'll never get modified status.");
            }
        }
        else
        {
            // Restore the document un-modified state
            WSD_CMD("save dontTerminateEdit=0 dontSaveIfUnmodified=0");
            TST_LOG("wait for cleanup of modified state before end of test");
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

                const std::string docName = "empty.ods";
                TST_LOG("Loading document: " << _filename);
                connectAndLoadLocalDocument(_filename);
                break;
            }
            case Phase::WaitLoadStatus:
            case Phase::WaitFirstModifiedStatus:
            case Phase::WaitAfterSaveModifiedStatus:
            case Phase::WaitSaveStatus:
                break;

            case Phase::WaitUnmodifiedStatus:
                if (_stopwatch.elapsed(std::chrono::seconds(5)))
                {
                    failTest("Timed out waiting for the un-modified state");
                }
                break;
        }
    }
};

// Inside the forkit & kit processes
class UnitKitSaveTorture : public UnitKit
{
    bool stampExists(const std::string& name, bool log = true)
    {
        const std::string path = "/tmp/" + name;
        const bool exists = FileUtil::Stat(path).exists();
        if (log)
            TST_LOG("Stamp [" << name << "] " << (exists ? "exists" : "missing"));
        return exists;
    }

    void waitWhileStamp(const std::string &name)
    {
        TST_LOG("waiting while stamp " << name << " exists");
        std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        while (stampExists(name))
        {
            TST_LOG("stamp exists " << name);
            if (std::chrono::steady_clock::now() - start > 10s)
            {
                LOK_ASSERT_FAIL("Timed out while waiting for stamp file " << name << " to go");
                return;
            }
            std::this_thread::sleep_for(StampFileCheckPeriodMs);
        }
        TST_LOG("stamp removed " << name);
    }

public:
    UnitKitSaveTorture() : UnitKit("savetorture")
    {
        // 4 times the default.
        constexpr std::chrono::minutes timeout_minutes(2);
        setTimeout(timeout_minutes);

        std::cerr << "\n\nYour Kit process has Save torturing hooks\n\n\n";
    }
    virtual bool filterKitMessage(WebSocketHandler *, std::string & /* message */) override
    {
        return false;
    }

    virtual bool filterDrainQueue() override { return stampExists("holddrainqueue", false); }

    virtual void preSaveHook() override
    {
        TST_LOG("Synchronous non-background save!");
        if (stampExists("abortonsyncsave"))
        {
            std::cerr << "Abort - unexpected non background save !\n\n";
            _exit(0); // otherwise we create segv's to count.
        }
    }

    virtual void postBackgroundSaveFork() override
    {
        if (stampExists("crashkitonsave"))
        {
            std::cerr << "Exit bgsave process to simulate crash\n\n";
            _exit(0); // otherwise we create segv's to count.
        }

        std::cerr << "\npost background save process fork\n\n";

        waitWhileStamp("holdsave");
    }

    virtual void preBackgroundSaveExit() override
    {
        std::cerr << "\n\npre exit of background save process\n\n\n";
    }
};

UnitBase** unit_create_wsd_multi(void)
{
    return new UnitBase* []
    {
        new UnitBgSaveCrash(), new UnitTileCombineRace(), new UnitModified(),
            new UnitSaveTortureOne("empty.ods", true, false, "simple_load-modify-bgsave"),
            new UnitSaveTortureOne("empty.odt", true, false, "simple_load-modify-bgsave"),
            new UnitSaveTortureOne("empty.ods", true, true,
                                   "load-modify-bgsave-start+modify+bgsave-end"),
            new UnitSaveTortureOne("empty.odt", true, true,
                                   "load-modify-bgsave-start+modify+bgsave-end"),
            new UnitSaveTortureOne("empty.ods", false, false, "un-modified-just-save-and-lets-see"),
            new UnitSaveTortureOne("empty.odt", false, false, "un-modified-just-save-and-lets-see"),
            nullptr
    };
}

UnitBase *unit_create_kit(void) { return new UnitKitSaveTorture(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
