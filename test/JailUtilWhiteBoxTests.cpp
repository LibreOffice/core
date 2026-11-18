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

#include <common/FileUtil.hpp>
#include <common/JailUtil.hpp>

#include <test/lokassert.hpp>

#include <fstream>
#include <sstream>
#include <ctime>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>

#include <string>
#include <string_view>
#include <vector>

/// Jail utility unit-tests.
class JailUtilWhiteBoxTests : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(JailUtilWhiteBoxTests);
#ifdef __linux__
    CPPUNIT_TEST(testSeccompModeReadFromStatusText);
    CPPUNIT_TEST(testSeccompModeReadFromLiveProc);
    CPPUNIT_TEST(testNamespaceRefusalBlamesHostSetting);
    CPPUNIT_TEST(testNamespaceRefusalBlamesAppArmor);
    CPPUNIT_TEST(testNamespaceRefusalBlamesSeccomp);
    CPPUNIT_TEST(testNamespaceRefusalSaysNothingWhenAllPermitted);
    CPPUNIT_TEST(testCrashNoteReplacesALink);
    CPPUNIT_TEST(testCrashNoteStaysInsideTheJail);
    CPPUNIT_TEST(testCrashedJailKeptOnlyForAFreshNote);
#endif // __linux__
    CPPUNIT_TEST_SUITE_END();

#ifdef __linux__
    void testSeccompModeReadFromStatusText();
    void testSeccompModeReadFromLiveProc();
    void testNamespaceRefusalBlamesHostSetting();
    void testNamespaceRefusalBlamesAppArmor();
    void testNamespaceRefusalBlamesSeccomp();
    void testNamespaceRefusalSaysNothingWhenAllPermitted();
    void testCrashNoteReplacesALink();
    void testCrashNoteStaysInsideTheJail();
    void testCrashedJailKeptOnlyForAFreshNote();
#endif // __linux__
};

#ifdef __linux__

namespace
{

/// An excerpt of /proc/self/status with the tab and the neighbouring Seccomp_filters field the
/// real file has, so that the field is read the way it actually appears.
std::string statusWithSeccompMode(const std::string& mode)
{
    return "Cpus_allowed_list:\t0-15\n"
           "Mems_allowed_list:\t0\n"
           "voluntary_ctxt_switches:\t1\n"
           "Seccomp:\t" +
           mode + "\nSeccomp_filters:\t1\nSpeculation_Store_Bypass:\tthread vulnerable\n";
}

std::string readFile(const std::string& path)
{
    std::ifstream file(path);
    std::ostringstream text;
    text << file.rdbuf();
    return text.str();
}

void writeFile(const std::string& path, const std::string& text)
{
    std::ofstream file(path);
    file << text;
}

} // namespace

/// Mode 0 in the status text means no filter, and the neighbouring Seccomp_filters field is not
/// mistaken for it.
void JailUtilWhiteBoxTests::testSeccompModeReadFromStatusText()
{
    constexpr std::string_view testname = __func__;

    LOK_ASSERT_EQUAL(0, JailUtil::seccompModeFromStatus(statusWithSeccompMode("0")));
    LOK_ASSERT_EQUAL(1, JailUtil::seccompModeFromStatus(statusWithSeccompMode("1")));
    LOK_ASSERT_EQUAL(2, JailUtil::seccompModeFromStatus(statusWithSeccompMode("2")));

    // A host whose status text has no such field, and an unreadable one, both count as no filter.
    LOK_ASSERT_EQUAL(0, JailUtil::seccompModeFromStatus("Name:\tcoolwsd\nPid:\t42\n"));
    LOK_ASSERT_EQUAL(0, JailUtil::seccompModeFromStatus(""));
}

/// A file under /proc reports a size of zero, and is still read whole. The Seccomp field of this
/// process parses to one of the three modes the kernel defines.
void JailUtilWhiteBoxTests::testSeccompModeReadFromLiveProc()
{
    constexpr std::string_view testname = __func__;

    std::vector<char> buffer;
    LOK_ASSERT(FileUtil::readFile("/proc/self/status", buffer, 8192) > 0);

    const std::string status(buffer.data(), buffer.size());
    LOK_ASSERT(status.find("Seccomp:") != std::string::npos);

    const int mode = JailUtil::seccompModeFromStatus(status);
    LOK_ASSERT(mode >= 0);
    LOK_ASSERT(mode <= 2);
}

/// A host that allows no user namespaces at all is named first, because raising that setting is
/// what has to happen before anything else can help.
void JailUtilWhiteBoxTests::testNamespaceRefusalBlamesHostSetting()
{
    constexpr std::string_view testname = __func__;

    // The trailing newline is what the file holds, so the reason has to survive it.
    const std::string reason = JailUtil::explainNamespaceRefusal("0\n", "1\n", 2);
    LOK_ASSERT(reason.find("max_user_namespaces") != std::string::npos);
}

/// AppArmor is reported when the host permits namespaces but restricts unprivileged ones, which is
/// the default on Ubuntu 24.04 and later.
void JailUtilWhiteBoxTests::testNamespaceRefusalBlamesAppArmor()
{
    constexpr std::string_view testname = __func__;

    const std::string reason = JailUtil::explainNamespaceRefusal("15000\n", "1\n", 2);
    LOK_ASSERT(reason.find("AppArmor") != std::string::npos);
}

/// With both host settings permitting namespaces, an installed seccomp filter is what is left to
/// explain the refusal, and the message points at the profile that permits the calls.
void JailUtilWhiteBoxTests::testNamespaceRefusalBlamesSeccomp()
{
    constexpr std::string_view testname = __func__;

    const std::string reason = JailUtil::explainNamespaceRefusal("15000\n", "0\n", 2);
    LOK_ASSERT(reason.find("seccomp") != std::string::npos);
    LOK_ASSERT(reason.find("cool-seccomp-profile.json") != std::string::npos);
}

/// Nothing is claimed when none of the three explains it, so that a wrong cause is never named.
void JailUtilWhiteBoxTests::testNamespaceRefusalSaysNothingWhenAllPermitted()
{
    constexpr std::string_view testname = __func__;

    LOK_ASSERT_EQUAL(std::string(), JailUtil::explainNamespaceRefusal("15000\n", "0\n", 0));

    // A host without the AppArmor setting has no such file to read, so empty text counts as
    // permitted.
    LOK_ASSERT_EQUAL(std::string(), JailUtil::explainNamespaceRefusal("15000\n", "", 0));
    LOK_ASSERT_EQUAL(std::string(), JailUtil::explainNamespaceRefusal("", "", 0));

    // The largest value the file can hold still means namespaces are permitted.
    LOK_ASSERT_EQUAL(std::string(), JailUtil::explainNamespaceRefusal("2147483647\n", "0\n", 0));
}

/// The crash note is a new regular file in the jail's tmp, even where a link of that name was,
/// and the file the link named keeps its contents.
void JailUtilWhiteBoxTests::testCrashNoteReplacesALink()
{
    constexpr std::string_view testname = __func__;

    const std::string root = FileUtil::createRandomTmpDir();
    const std::string jail = root + "/jail";
    const std::string other = root + "/other";
    LOK_ASSERT_EQUAL(0, ::mkdir(jail.c_str(), S_IRWXU));
    LOK_ASSERT_EQUAL(0, ::mkdir((jail + "/tmp").c_str(), S_IRWXU));
    writeFile(other, "contents");
    LOK_ASSERT_EQUAL(0, ::symlink(other.c_str(), (jail + "/tmp/kit-crashed").c_str()));

    JailUtil::noteCrashedJail(jail);

    const FileUtil::Stat note(jail + "/tmp/kit-crashed", /*link=*/true);
    LOK_ASSERT(note.good());
    LOK_ASSERT(note.isFile());
    LOK_ASSERT_EQUAL(std::string("contents"), readFile(other));

    FileUtil::removeFile(root, /*recursive=*/true);
}

/// When the jail's tmp is a link to a directory elsewhere, no crash note is written there.
void JailUtilWhiteBoxTests::testCrashNoteStaysInsideTheJail()
{
    constexpr std::string_view testname = __func__;

    const std::string root = FileUtil::createRandomTmpDir();
    const std::string jail = root + "/jail";
    const std::string other = root + "/other";
    LOK_ASSERT_EQUAL(0, ::mkdir(jail.c_str(), S_IRWXU));
    LOK_ASSERT_EQUAL(0, ::mkdir(other.c_str(), S_IRWXU));
    LOK_ASSERT_EQUAL(0, ::symlink(other.c_str(), (jail + "/tmp").c_str()));

    JailUtil::noteCrashedJail(jail);

    LOK_ASSERT(!FileUtil::Stat(other + "/kit-crashed", /*link=*/true).exists());

    FileUtil::removeFile(root, /*recursive=*/true);
}

/// A jail is kept for a crash note written just now, but not for one dated in the future, nor
/// for a note that is not a regular file.
void JailUtilWhiteBoxTests::testCrashedJailKeptOnlyForAFreshNote()
{
    constexpr std::string_view testname = __func__;

    const std::string root = FileUtil::createRandomTmpDir();
    const std::string jail = root + "/jail";
    const std::string note = jail + "/tmp/kit-crashed";
    LOK_ASSERT_EQUAL(0, ::mkdir(jail.c_str(), S_IRWXU));
    LOK_ASSERT_EQUAL(0, ::mkdir((jail + "/tmp").c_str(), S_IRWXU));

    LOK_ASSERT(!JailUtil::isCrashedJailKept(jail));

    JailUtil::noteCrashedJail(jail);
    LOK_ASSERT(JailUtil::isCrashedJailKept(jail));

    const struct timespec tomorrow = { std::time(nullptr) + 24 * 60 * 60, 0 };
    const struct timespec times[2] = { tomorrow, tomorrow };
    LOK_ASSERT_EQUAL(0, ::utimensat(AT_FDCWD, note.c_str(), times, 0));
    LOK_ASSERT(!JailUtil::isCrashedJailKept(jail));

    const std::string other = root + "/other";
    writeFile(other, "contents");
    LOK_ASSERT_EQUAL(0, ::unlink(note.c_str()));
    LOK_ASSERT_EQUAL(0, ::symlink(other.c_str(), note.c_str()));
    LOK_ASSERT(!JailUtil::isCrashedJailKept(jail));

    FileUtil::removeFile(root, /*recursive=*/true);
}

#endif // __linux__

CPPUNIT_TEST_SUITE_REGISTRATION(JailUtilWhiteBoxTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
