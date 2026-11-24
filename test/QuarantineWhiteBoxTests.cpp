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

#include <config.h>

#include <common/ConfigUtil.hpp>
#include <common/FileUtil.hpp>
#include <wsd/QuarantineUtil.hpp>

#include <test/lokassert.hpp>

#include <Poco/AutoPtr.h>
#include <Poco/Util/MapConfiguration.h>

#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>

#include <ctime>
#include <fstream>
#include <mutex>
#include <sstream>
#include <string>
#include <string_view>
#include <sys/stat.h>
#include <utility>

/// Quarantine unit-tests.
class QuarantineWhiteBoxTests : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(QuarantineWhiteBoxTests);
    CPPUNIT_TEST(testStateAtStart);
    CPPUNIT_TEST(testInvalidSettings);
    CPPUNIT_TEST_SUITE_END();

    void testStateAtStart();
    void testInvalidSettings();

public:
    /// Puts the config and the quarantine back as they were, disabled, for the tests that follow,
    /// also when an assertion fails, and removes the given directory.
    class Restore
    {
    public:
        Restore(const Poco::Util::AbstractConfiguration* config, std::string root)
            : _previous(ConfigUtil::setConfigForTest(config))
            , _root(std::move(root))
        {
        }

        ~Restore()
        {
            {
                std::lock_guard<std::mutex> lock(Quarantine::Mutex);
                Quarantine::QuarantineMap.clear();
                Quarantine::OpenDocKeys.clear();
                Quarantine::QuarantinePath.clear();
            }

            ConfigUtil::setConfigForTest(_previous);
            FileUtil::removeFile(_root, /*recursive=*/true);
        }

    private:
        const Poco::Util::AbstractConfiguration* _previous;
        std::string _root;
    };
};

namespace
{
void writeFile(const std::string& path, const std::string& text)
{
    std::ofstream file(path);
    file << text;
}

std::string readFile(const std::string& path)
{
    std::ifstream file(path);
    std::ostringstream text;
    text << file.rdbuf();
    return text.str();
}
} // namespace

/// At the start, a document that was open when wsd went away has its versions protected, a
/// document that ended badly keeps the protection of its versions, and an unfinished copy goes.
void QuarantineWhiteBoxTests::testStateAtStart()
{
    constexpr std::string_view testname = __func__;

    const std::string root = FileUtil::createRandomTmpDir();
    const std::string path = root + "/quarantine/";
    const std::string time = std::to_string(std::time(nullptr) - 60);

    // A document that was open, with one version.
    const std::string open = path + "open";
    const std::string openVersion = time + "_1_open.txt";
    LOK_ASSERT_EQUAL(0, ::mkdir(path.c_str(), S_IRWXU));
    LOK_ASSERT_EQUAL(0, ::mkdir(open.c_str(), S_IRWXU));
    writeFile(open + '/' + openVersion, "open");
    writeFile(open + "/state", "open\n");

    // A document that ended badly, with a protected version and a later one.
    const std::string bad = path + "bad";
    const std::string badVersion = time + "_2_bad.txt";
    const std::string laterVersion = std::to_string(std::time(nullptr) - 30) + "_2_bad.txt";
    LOK_ASSERT_EQUAL(0, ::mkdir(bad.c_str(), S_IRWXU));
    writeFile(bad + '/' + badVersion, "bad");
    writeFile(bad + '/' + laterVersion, "later");
    writeFile(bad + "/state", "closed\n" + badVersion + '\n');

    // A document that ended well, with one version and an unfinished copy.
    const std::string well = path + "well";
    LOK_ASSERT_EQUAL(0, ::mkdir(well.c_str(), S_IRWXU));
    writeFile(well + '/' + time + "_3_well.txt", "well");
    writeFile(well + "/copy-unfinished", "unfin");

    Poco::AutoPtr<Poco::Util::MapConfiguration> config(new Poco::Util::MapConfiguration);
    config->setBool("quarantine_files[@enable]", true);
    config->setString("quarantine_files.path", path);

    const Restore restore(config, root);

    Quarantine::initialize(path);

    LOK_ASSERT_EQUAL_MESSAGE("Expected the versions of the open document protected",
                             "closed\n" + openVersion + '\n', readFile(open + "/state"));
    LOK_ASSERT_EQUAL_MESSAGE("Expected the protected version still protected",
                             "closed\n" + badVersion + '\n', readFile(bad + "/state"));
    LOK_ASSERT_MESSAGE("Expected no state for the document that ended well",
                       !FileUtil::Stat(well + "/state").exists());
    LOK_ASSERT_MESSAGE("Expected the unfinished copy removed",
                       !FileUtil::Stat(well + "/copy-unfinished").exists());
}

/// Settings that can't be read, or are negative, are taken as their defaults, and the quarantine
/// starts all the same.
void QuarantineWhiteBoxTests::testInvalidSettings()
{
    constexpr std::string_view testname = __func__;

    const std::string root = FileUtil::createRandomTmpDir();
    const std::string path = root + "/quarantine/";

    Poco::AutoPtr<Poco::Util::MapConfiguration> config(new Poco::Util::MapConfiguration);
    config->setBool("quarantine_files[@enable]", true);
    config->setString("quarantine_files.path", path);
    config->setString("quarantine_files.limit_dir_size_mb", "");
    config->setString("quarantine_files.expiry_min", "-1");
    config->setString("quarantine_files.max_versions_to_maintain", "-5");
    const Restore restore(config, root);

    Quarantine::initialize(path);

    LOK_ASSERT(Quarantine::isEnabled());
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(250 * 1024 * 1024), Quarantine::MaxSizeBytes);
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(3000 * 60), Quarantine::MaxAgeSecs);
    LOK_ASSERT_EQUAL(static_cast<std::size_t>(5), Quarantine::MaxVersions);
}

CPPUNIT_TEST_SUITE_REGISTRATION(QuarantineWhiteBoxTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
