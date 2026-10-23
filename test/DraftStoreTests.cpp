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
 * Unit tests for the drafts folder (common/DraftStore.cpp).
 */

#include <config.h>

#include <test/lokassert.hpp>

#include <common/DraftStore.hpp>
#include <common/FileUtil.hpp>

#include <cppunit/extensions/HelperMacros.h>

#include <fstream>
#include <iterator>
#include <string>
#include <vector>

#include <sys/stat.h>
#include <unistd.h>

class DraftStoreTests : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(DraftStoreTests);

    CPPUNIT_TEST(testNextFreeName);
    CPPUNIT_TEST(testMarkedNameIsTaken);
    CPPUNIT_TEST(testCreateFromTemplate);
    CPPUNIT_TEST(testDiscardAndDeleteMarked);
    CPPUNIT_TEST(testDeleteMarkedKeepsMarkOfUndeletable);
    CPPUNIT_TEST(testLeftovers);
    CPPUNIT_TEST(testContains);
    CPPUNIT_TEST(testHostPathOfTarget);

    CPPUNIT_TEST_SUITE_END();

    void testNextFreeName();
    void testMarkedNameIsTaken();
    void testCreateFromTemplate();
    void testDiscardAndDeleteMarked();
    void testDeleteMarkedKeepsMarkOfUndeletable();
    void testLeftovers();
    void testContains();
    void testHostPathOfTarget();

    std::string _root;

public:
    void setUp() override
    {
        constexpr std::string_view testname = __func__;
        _root = FileUtil::createRandomTmpDir();
        LOK_ASSERT(drafts::initialize(_root));
    }

    void tearDown() override
    {
        if (!_root.empty())
            FileUtil::removeFile(_root, /*recursive=*/true);
    }

private:
    std::string draftsDir() const { return _root + "/drafts"; }

    static void writeFile(const std::string& path, const std::string& content = "x")
    {
        std::ofstream stream(path, std::ios::trunc | std::ios::binary);
        stream << content;
    }

    static std::string readFile(const std::string& path)
    {
        std::ifstream stream(path, std::ios::binary);
        return std::string(std::istreambuf_iterator<char>(stream), {});
    }

    static bool exists(const std::string& path)
    {
        return FileUtil::Stat(path, /*link=*/true).exists();
    }
};

void DraftStoreTests::testNextFreeName()
{
    constexpr std::string_view testname = __func__;

    LOK_ASSERT_EQUAL(draftsDir(), drafts::directory());
    LOK_ASSERT(FileUtil::Stat(draftsDir()).isDirectory());

    LOK_ASSERT_EQUAL(draftsDir() + "/Text Document.odt",
                     drafts::nextFreeName("Text Document", "odt"));

    writeFile(draftsDir() + "/Text Document.odt");
    LOK_ASSERT_EQUAL(draftsDir() + "/Text Document (1).odt",
                     drafts::nextFreeName("Text Document", "odt"));

    writeFile(draftsDir() + "/Text Document (1).odt");
    LOK_ASSERT_EQUAL(draftsDir() + "/Text Document (2).odt",
                     drafts::nextFreeName("Text Document", "odt"));

    // Another extension is another name.
    LOK_ASSERT_EQUAL(draftsDir() + "/Text Document.ods",
                     drafts::nextFreeName("Text Document", "ods"));

    // UTF-8 names come back byte for byte.
    LOK_ASSERT_EQUAL(draftsDir() + "/Belge \xc3\xa7\xc4\x9f.odt",
                     drafts::nextFreeName("Belge \xc3\xa7\xc4\x9f", "odt"));

    // A name that would leave the folder or that is no name at all.
    LOK_ASSERT_EQUAL(std::string(), drafts::nextFreeName("../escape", "odt"));
    LOK_ASSERT_EQUAL(std::string(), drafts::nextFreeName("sub/name", "odt"));
    LOK_ASSERT_EQUAL(std::string(), drafts::nextFreeName("sub\\name", "odt"));
    LOK_ASSERT_EQUAL(std::string(), drafts::nextFreeName("line\nbreak", "odt"));
}

void DraftStoreTests::testMarkedNameIsTaken()
{
    constexpr std::string_view testname = __func__;

    // A discarded draft can come back with the teardown save of its document, so its name
    // stays taken until the next start deletes it.
    const std::string discarded = draftsDir() + "/Spreadsheet.ods";
    drafts::markForDeletion(discarded);
    LOK_ASSERT(drafts::isMarkedForDeletion(discarded));
    LOK_ASSERT(!exists(discarded));
    LOK_ASSERT_EQUAL(draftsDir() + "/Spreadsheet (1).ods",
                     drafts::nextFreeName("Spreadsheet", "ods"));

    // The mark is a file, so it outlives the process.
    LOK_ASSERT(exists(draftsDir() + "/.marked/Spreadsheet.ods"));
    LOK_ASSERT(drafts::initialize(_root));
    LOK_ASSERT(drafts::isMarkedForDeletion(discarded));

    // The folder part of the path is not part of the mark.
    LOK_ASSERT(drafts::isMarkedForDeletion("/elsewhere/Spreadsheet.ods"));
    // The file system decides what is the same name.
    LOK_ASSERT(!drafts::isMarkedForDeletion(draftsDir() + "/Spreadsheet (1).ods"));

    drafts::unmarkForDeletion(discarded);
    LOK_ASSERT(!drafts::isMarkedForDeletion(discarded));
    LOK_ASSERT_EQUAL(discarded, drafts::nextFreeName("Spreadsheet", "ods"));

    // Unmarking an unmarked name, and marking twice, are harmless.
    drafts::unmarkForDeletion(discarded);
    drafts::markForDeletion(discarded);
    drafts::markForDeletion(discarded);
    LOK_ASSERT(drafts::isMarkedForDeletion(discarded));
}

void DraftStoreTests::testCreateFromTemplate()
{
    constexpr std::string_view testname = __func__;

    const std::string templatePath = _root + "/template.ott";
    writeFile(templatePath, "template content");
    ::chmod(templatePath.c_str(), 0444);

    const std::string first = drafts::createFromTemplate(templatePath, "Letter", "odt");
    LOK_ASSERT_EQUAL(draftsDir() + "/Letter.odt", first);
    LOK_ASSERT_EQUAL(std::string("template content"), readFile(first));
    // A read-only template still gives a draft the app can save.
    LOK_ASSERT_EQUAL(0, ::access(first.c_str(), W_OK));

    // Never replaces an existing draft.
    writeFile(first, "edited");
    const std::string second = drafts::createFromTemplate(templatePath, "Letter", "odt");
    LOK_ASSERT_EQUAL(draftsDir() + "/Letter (1).odt", second);
    LOK_ASSERT_EQUAL(std::string("edited"), readFile(first));

    LOK_ASSERT_EQUAL(std::string(), drafts::createFromTemplate(templatePath, "a/b", "odt"));

    // A missing template leaves no empty draft behind.
    LOK_ASSERT_EQUAL(std::string(),
                     drafts::createFromTemplate(_root + "/missing.ott", "Missing", "odt"));
    LOK_ASSERT(!exists(draftsDir() + "/Missing.odt"));
}

void DraftStoreTests::testDiscardAndDeleteMarked()
{
    constexpr std::string_view testname = __func__;

    const std::string draft = draftsDir() + "/Presentation.odp";
    writeFile(draft);
    LOK_ASSERT(drafts::discard(draft));
    LOK_ASSERT(!exists(draft));
    LOK_ASSERT(drafts::isMarkedForDeletion(draft));

    // The teardown save writes it again; the next start deletes it and frees the name.
    writeFile(draft);
    const std::string kept = draftsDir() + "/Drawing.odg";
    writeFile(kept);
    drafts::deleteMarked();
    LOK_ASSERT(!exists(draft));
    LOK_ASSERT(!drafts::isMarkedForDeletion(draft));
    LOK_ASSERT(exists(kept));
    LOK_ASSERT_EQUAL(draft, drafts::nextFreeName("Presentation", "odp"));

    // A mark whose draft never came back goes too.
    drafts::markForDeletion(draftsDir() + "/Never Written.odt");
    drafts::deleteMarked();
    LOK_ASSERT(!drafts::isMarkedForDeletion(draftsDir() + "/Never Written.odt"));
}

void DraftStoreTests::testDeleteMarkedKeepsMarkOfUndeletable()
{
    constexpr std::string_view testname = __func__;

    if (::geteuid() == 0)
        return; // root deletes from a read-only folder.

    const std::string draft = draftsDir() + "/Locked.odt";
    writeFile(draft);
    drafts::markForDeletion(draft);

    ::chmod(draftsDir().c_str(), 0500);
    drafts::deleteMarked();
    const std::vector<std::string> recovered = drafts::leftovers();
    ::chmod(draftsDir().c_str(), 0700);

    LOK_ASSERT(exists(draft));
    LOK_ASSERT(drafts::isMarkedForDeletion(draft));
    // Discarded, so not offered back as a draft.
    LOK_ASSERT(recovered.empty());

    drafts::deleteMarked();
    LOK_ASSERT(!exists(draft));
}

void DraftStoreTests::testLeftovers()
{
    constexpr std::string_view testname = __func__;

    LOK_ASSERT(drafts::leftovers().empty());

    writeFile(draftsDir() + "/b.odt");
    writeFile(draftsDir() + "/A.odt");
    writeFile(draftsDir() + "/a (1).ods");
    writeFile(draftsDir() + "/marked.odt");
    drafts::markForDeletion(draftsDir() + "/marked.odt");
    writeFile(draftsDir() + "/.hidden.odt");
    // The engine's save temporary and a copyAtomic temporary.
    writeFile(draftsDir() + "/lu12345abcd.tmp");
    writeFile(draftsDir() + "/report (2026-10-23 10.00).odtAbCdEfGhIjKl");
    ::mkdir((draftsDir() + "/folder.odt").c_str(), 0700);
    LOK_ASSERT_EQUAL(0, ::symlink((_root + "/elsewhere.odt").c_str(),
                                  (draftsDir() + "/link.odt").c_str()));
    writeFile(_root + "/elsewhere.odt");

    // Sorted by bytes.
    const std::vector<std::string> expected = { draftsDir() + "/A.odt",
                                                draftsDir() + "/a (1).ods",
                                                draftsDir() + "/b.odt" };
    LOK_ASSERT(expected == drafts::leftovers());
}

void DraftStoreTests::testContains()
{
    constexpr std::string_view testname = __func__;

    writeFile(draftsDir() + "/Existing.odt");
    LOK_ASSERT(drafts::contains(draftsDir() + "/Existing.odt"));
    // A Save As target does not exist yet.
    LOK_ASSERT(drafts::contains(draftsDir() + "/New Name.odt"));
    LOK_ASSERT(drafts::contains(draftsDir() + "/.marked/x"));

    LOK_ASSERT(!drafts::contains(draftsDir()));
    LOK_ASSERT(!drafts::contains(_root + "/Outside.odt"));
    // Whole folder names only.
    ::mkdir((_root + "/drafts-other").c_str(), 0700);
    LOK_ASSERT(!drafts::contains(_root + "/drafts-other/x.odt"));

    // Through a symlink to the drafts folder, existing file or new name.
    const std::string link = _root + "/link-to-drafts";
    LOK_ASSERT_EQUAL(0, ::symlink(draftsDir().c_str(), link.c_str()));
    LOK_ASSERT(drafts::contains(link + "/Existing.odt"));
    LOK_ASSERT(drafts::contains(link + "/New Name.odt"));

    // And with the drafts folder itself reached through a symlink.
    const std::string linkedRoot = _root + "-link";
    LOK_ASSERT_EQUAL(0, ::symlink(_root.c_str(), linkedRoot.c_str()));
    LOK_ASSERT(drafts::initialize(linkedRoot));
    const bool insideViaRealPath = drafts::contains(draftsDir() + "/New Name.odt");
    const bool outsideViaRealPath = drafts::contains(_root + "/Outside.odt");
    ::unlink(linkedRoot.c_str());
    LOK_ASSERT(insideViaRealPath);
    LOK_ASSERT(!outsideViaRealPath);
}

void DraftStoreTests::testHostPathOfTarget()
{
    constexpr std::string_view testname = __func__;

    const std::string real = FileUtil::realpath(_root);
    const std::string link = _root + "/link";
    ::mkdir((_root + "/folder").c_str(), 0700);
    LOK_ASSERT_EQUAL(0, ::symlink((_root + "/folder").c_str(), link.c_str()));

    // Existing files: the same as hostPath().
    writeFile(_root + "/folder/doc.odt");
    LOK_ASSERT_EQUAL(FileUtil::hostPath(link + "/doc.odt"),
                     FileUtil::hostPathOfTarget(link + "/doc.odt"));
    LOK_ASSERT_EQUAL(real + "/folder/doc.odt", FileUtil::hostPathOfTarget(link + "/doc.odt"));

    // A missing file keeps its name under the resolved folder.
    LOK_ASSERT_EQUAL(real + "/folder/new name.odt",
                     FileUtil::hostPathOfTarget(link + "/new name.odt"));
    LOK_ASSERT_EQUAL(real + "/folder/new.odt",
                     FileUtil::hostPathOfTarget(_root + "/folder/../folder/new.odt"));

    // Nothing to resolve: the path as given.
    LOK_ASSERT_EQUAL(_root + "/no/such/doc.odt",
                     FileUtil::hostPathOfTarget(_root + "/no/such/doc.odt"));
}

CPPUNIT_TEST_SUITE_REGISTRATION(DraftStoreTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
