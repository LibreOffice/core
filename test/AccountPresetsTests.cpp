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

#include <config.h>

#include <common/AccountPresets.hpp>
#include <common/FileUtil.hpp>

#include <test/lokassert.hpp>

#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <string_view>
#include <vector>

/// Tests of the files that CODA keeps for one cloud account.
class AccountPresetsTests : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(AccountPresetsTests);
    CPPUNIT_TEST(testFileNameOfUri);
    CPPUNIT_TEST(testStoreFileAndOutdatedUris);
    CPPUNIT_TEST(testRemoveUnlisted);
    CPPUNIT_TEST(testStoreFileRefuses);
    CPPUNIT_TEST(testFileLimit);
    CPPUNIT_TEST(testStoreFileInParts);
    CPPUNIT_TEST(testStoreFilePartRefuses);
    CPPUNIT_TEST(testTemplates);
    CPPUNIT_TEST(testRemoveAll);
    CPPUNIT_TEST_SUITE_END();

    void testFileNameOfUri();
    void testStoreFileAndOutdatedUris();
    void testRemoveUnlisted();
    void testStoreFileRefuses();
    void testFileLimit();
    void testStoreFileInParts();
    void testStoreFilePartRefuses();
    void testTemplates();
    void testRemoveAll();

public:
    void setUp() override { _folder = FileUtil::createRandomTmpDir(); }

    void tearDown() override
    {
        std::error_code error;
        std::filesystem::remove_all(_folder, error);
    }

private:
    static std::string templateBytes()
    {
        std::ifstream in(TDOC "/presets-default/shared/templateshared.otp", std::ios::binary);
        return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
    }

    /// How many regular files the folder of the account holds, at any depth.
    std::size_t storedFileCount() const
    {
        std::size_t count = 0;
        for (const auto& item : std::filesystem::recursive_directory_iterator(_folder))
            count += item.is_regular_file() ? 1 : 0;
        return count;
    }

    /// The folder of the account.
    std::string _folder;
};

namespace
{
const std::string SharedUri = "http://host/settings/token1/shared/templateshared.otp?testname=t";
const std::string UserUri = "http://host/settings/token1/user/templateshared.otp";
} // namespace

void AccountPresetsTests::testFileNameOfUri()
{
    constexpr std::string_view testname = __func__;

    LOK_ASSERT_EQUAL(std::string("a.otp"),
                     AccountPresets::fileNameOfUri("http://h/x/shared/a.otp?testname=t")
                         .value_or(std::string()));
    LOK_ASSERT_EQUAL(
        std::string("b.otp"),
        AccountPresets::fileNameOfUri("http://h/x/user/?file_name=b.otp&t=1")
            .value_or(std::string()));
    LOK_ASSERT_EQUAL(std::string("a b.otp"),
                     AccountPresets::fileNameOfUri("http://h/x/a%20b.otp").value_or(std::string()));

    LOK_ASSERT(!AccountPresets::fileNameOfUri("http://h/x/?file_name=../b.otp"));
    LOK_ASSERT(!AccountPresets::fileNameOfUri("http://h/x/a%2Fb.otp"));
    LOK_ASSERT(!AccountPresets::fileNameOfUri("http://h/x/.."));
    LOK_ASSERT(!AccountPresets::fileNameOfUri("http://h/x/"));
    LOK_ASSERT(!AccountPresets::fileNameOfUri(""));
}

void AccountPresetsTests::testStoreFileAndOutdatedUris()
{
    constexpr std::string_view testname = __func__;

    AccountPresets presets(_folder);
    LOK_ASSERT(presets.storeFile("shared", "template", SharedUri, "1", templateBytes()));

    LOK_ASSERT(presets.outdatedUris("shared", "template", { { SharedUri, "1" } }).empty());

    const std::vector<std::string> changed =
        presets.outdatedUris("shared", "template", { { SharedUri, "2" } });
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), changed.size());
    LOK_ASSERT_EQUAL(SharedUri, changed[0]);

    // The same file under a uri with another token in its path.
    LOK_ASSERT(presets
                   .outdatedUris("shared", "template",
                                 { { "http://host/settings/token2/shared/templateshared.otp", "1" } })
                   .empty());

    // A file that is not stored yet.
    const std::string otherUri = "http://host/settings/token1/shared/other.ott";
    const std::vector<std::string> missing =
        presets.outdatedUris("shared", "template", { { SharedUri, "1" }, { otherUri, "1" } });
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), missing.size());
    LOK_ASSERT_EQUAL(otherUri, missing[0]);

    // A stamp of the largest length that is stored.
    const std::string longStamp(AccountPresets::MaxStampBytes, 's');
    LOK_ASSERT(presets.storeFile("shared", "template", otherUri, longStamp, templateBytes()));
    LOK_ASSERT(presets.outdatedUris("shared", "template", { { otherUri, longStamp } }).empty());
}

void AccountPresetsTests::testRemoveUnlisted()
{
    constexpr std::string_view testname = __func__;

    AccountPresets presets(_folder);
    LOK_ASSERT(presets.storeFile("shared", "template", SharedUri, "1", templateBytes()));
    LOK_ASSERT(presets.storeFile("user", "template", UserUri, "1", templateBytes()));

    // A file that the list still names stays.
    presets.removeUnlisted("shared", "template", { { SharedUri, "1" } });
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(2), presets.templates().size());

    presets.removeUnlisted("shared", "template", {});

    // The user list keeps its file.
    const std::vector<UserTemplates::Entry> templates = presets.templates();
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), templates.size());
    LOK_ASSERT_EQUAL(std::string("user/templateshared.otp"), templates[0].id);
}

void AccountPresetsTests::testStoreFileRefuses()
{
    constexpr std::string_view testname = __func__;

    AccountPresets presets(_folder);
    const std::string bytes = templateBytes();
    LOK_ASSERT(!presets.storeFile("other", "template", SharedUri, "1", bytes));
    LOK_ASSERT(!presets.storeFile("../shared", "template", SharedUri, "1", bytes));
    LOK_ASSERT(!presets.storeFile("shared", "xcu", SharedUri, "1", bytes));
    LOK_ASSERT(
        !presets.storeFile("shared", "template", "http://h/x/?file_name=../b.otp", "1", bytes));
    LOK_ASSERT(!presets.storeFile("shared", "template", "http://h/x/a%2Fb.otp", "1", bytes));
    LOK_ASSERT(!presets.storeFile("shared", "template", "http://h/x/", "1", bytes));
    // No template and no document.
    LOK_ASSERT(!presets.storeFile("shared", "template", "http://h/x/stamp", "1", bytes));
    LOK_ASSERT(!presets.storeFile("shared", "template", "http://h/x/run.sh", "1", bytes));
    // One byte more than the largest file that is stored.
    LOK_ASSERT(!presets.storeFile("shared", "template", SharedUri, "1",
                                  std::string(AccountPresets::MaxFileBytes + 1, 'x')));
    // One byte more than the longest stamp that is stored.
    const std::string longStamp(AccountPresets::MaxStampBytes + 1, 's');
    LOK_ASSERT(!presets.storeFile("shared", "template", SharedUri, longStamp, bytes));

    LOK_ASSERT_EQUAL(static_cast<std::size_t>(0), storedFileCount());

    // The uris that write refuses are not asked for.
    LOK_ASSERT(presets
                   .outdatedUris("shared", "template",
                                 { { "http://h/x/?file_name=../b.otp", "1" },
                                   { "http://h/x/run.sh", "1" },
                                   { SharedUri, longStamp } })
                   .empty());
}

void AccountPresetsTests::testFileLimit()
{
    constexpr std::string_view testname = __func__;

    AccountPresets presets(_folder);
    const std::string bytes = templateBytes();
    const auto uri = [](std::size_t number)
    { return "http://h/x/shared/t" + std::to_string(number) + ".otp"; };

    for (std::size_t i = 0; i < AccountPresets::MaxFilesPerGroup; ++i)
        LOK_ASSERT(presets.storeFile("shared", "template", uri(i), "1", bytes));

    // The group of the list is full, so it takes no file with a new name.
    const std::string newUri = "http://h/x/shared/new.otp";
    LOK_ASSERT(!presets.storeFile("shared", "template", newUri, "1", bytes));
    LOK_ASSERT(!presets.storeFilePart("shared", "template", newUri, 0, bytes));
    LOK_ASSERT_EQUAL(AccountPresets::MaxFilesPerGroup, presets.templates().size());

    // A new version of a stored file is asked for and stored, and a new name is not asked for.
    const std::vector<std::string> wanted =
        presets.outdatedUris("shared", "template", { { uri(0), "2" }, { newUri, "1" } });
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), wanted.size());
    LOK_ASSERT_EQUAL(uri(0), wanted[0]);
    LOK_ASSERT(presets.storeFile("shared", "template", uri(0), "2", bytes));

    // The user list has its own places.
    LOK_ASSERT(presets.storeFile("user", "template", UserUri, "1", bytes));

    // A list that names one file less leaves one place, and outdatedUris asks for one new file.
    std::vector<AccountPresets::Listed> listed;
    for (std::size_t i = 1; i < AccountPresets::MaxFilesPerGroup; ++i)
        listed.push_back({ uri(i), "1" });
    presets.removeUnlisted("shared", "template", listed);
    listed.push_back({ newUri, "1" });
    listed.push_back({ "http://h/x/shared/other.otp", "1" });
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1),
                     presets.outdatedUris("shared", "template", listed).size());
    LOK_ASSERT(presets.storeFile("shared", "template", newUri, "1", bytes));
}

void AccountPresetsTests::testStoreFileInParts()
{
    constexpr std::string_view testname = __func__;

    AccountPresets presets(_folder);
    const std::string bytes = templateBytes();
    const std::size_t third = bytes.size() / 3;
    LOK_ASSERT(third > 0);

    LOK_ASSERT(presets.storeFilePart("shared", "template", SharedUri, 0, bytes.substr(0, third)));
    LOK_ASSERT(
        presets.storeFilePart("shared", "template", SharedUri, third, bytes.substr(third, third)));

    // A file that is not finished is no template yet, and is still asked for.
    LOK_ASSERT(presets.templates().empty());
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1),
                     presets.outdatedUris("shared", "template", { { SharedUri, "1" } }).size());

    LOK_ASSERT(presets.storeFilePart("shared", "template", SharedUri, 2 * third,
                                     bytes.substr(2 * third)));
    LOK_ASSERT(presets.finishFile("shared", "template", SharedUri, "1"));

    const std::vector<UserTemplates::Entry> templates = presets.templates();
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), templates.size());
    std::ifstream in(templates[0].path, std::ios::binary);
    LOK_ASSERT(bytes ==
               std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()));
    LOK_ASSERT(presets.outdatedUris("shared", "template", { { SharedUri, "1" } }).empty());

    // The parts are used up, so the file cannot be finished a second time.
    LOK_ASSERT(!presets.finishFile("shared", "template", SharedUri, "2"));

    // A part at offset 0 starts the file anew.
    LOK_ASSERT(presets.storeFilePart("shared", "template", SharedUri, 0, "abc"));
    LOK_ASSERT(presets.storeFilePart("shared", "template", SharedUri, 0, bytes));
    LOK_ASSERT(presets.finishFile("shared", "template", SharedUri, "2"));
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(1), presets.templates().size());
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(bytes.size()),
                     static_cast<std::size_t>(std::filesystem::file_size(templates[0].path)));
}

void AccountPresetsTests::testStoreFilePartRefuses()
{
    constexpr std::string_view testname = __func__;

    AccountPresets presets(_folder);
    LOK_ASSERT(!presets.storeFilePart("other", "template", SharedUri, 0, "abc"));
    LOK_ASSERT(!presets.storeFilePart("shared", "template", "http://h/x/run.sh", 0, "abc"));
    LOK_ASSERT(!presets.finishFile("shared", "template", SharedUri, "1"));

    // A part goes where the parts before it end.
    LOK_ASSERT(!presets.storeFilePart("shared", "template", SharedUri, 3, "def"));
    LOK_ASSERT(presets.storeFilePart("shared", "template", SharedUri, 0, "abc"));
    LOK_ASSERT(!presets.storeFilePart("shared", "template", SharedUri, 2, "def"));
    LOK_ASSERT(!presets.storeFilePart("shared", "template", SharedUri, 4, "def"));
    LOK_ASSERT(presets.storeFilePart("shared", "template", SharedUri, 3, "def"));

    // One byte more than the largest file that is stored, over all parts.
    LOK_ASSERT(!presets.storeFilePart("shared", "template", SharedUri, 6,
                                      std::string(AccountPresets::MaxFileBytes - 5, 'x')));

    // A stamp that is too long ends the file.
    LOK_ASSERT(!presets.finishFile("shared", "template", SharedUri,
                                   std::string(AccountPresets::MaxStampBytes + 1, 's')));
    LOK_ASSERT(!presets.finishFile("shared", "template", SharedUri, "1"));
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(0), storedFileCount());
    LOK_ASSERT(presets.templates().empty());
}

void AccountPresetsTests::testTemplates()
{
    constexpr std::string_view testname = __func__;

    AccountPresets presets(_folder);
    LOK_ASSERT(presets.storeFile("user", "template", UserUri, "1", templateBytes()));
    LOK_ASSERT(presets.storeFile("shared", "template", SharedUri, "1", templateBytes()));

    const std::vector<UserTemplates::Entry> templates = presets.templates();
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(2), templates.size());
    LOK_ASSERT_EQUAL(std::string("shared/templateshared.otp"), templates[0].id);
    LOK_ASSERT_EQUAL(std::string("user/templateshared.otp"), templates[1].id);
    for (const UserTemplates::Entry& entry : templates)
    {
        LOK_ASSERT_EQUAL(std::string("templateshared"), entry.name);
        LOK_ASSERT_EQUAL(UserTemplates::documentTypeOfFile("templateshared.otp"), entry.type);
        LOK_ASSERT(std::filesystem::is_regular_file(entry.path));
        LOK_ASSERT_EQUAL(templateBytes().size(),
                         static_cast<std::size_t>(std::filesystem::file_size(entry.path)));
    }
}

void AccountPresetsTests::testRemoveAll()
{
    constexpr std::string_view testname = __func__;

    AccountPresets presets(_folder);
    LOK_ASSERT(presets.storeFile("shared", "template", SharedUri, "1", templateBytes()));
    presets.removeAll();

    LOK_ASSERT(presets.templates().empty());
    LOK_ASSERT(!std::filesystem::exists(std::filesystem::path(_folder) / "presets"));
}

CPPUNIT_TEST_SUITE_REGISTRATION(AccountPresetsTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
