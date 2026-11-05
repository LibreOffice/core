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

#include <kit/OomKills.hpp>

#include <test/lokassert.hpp>

#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>

#include <sys/wait.h>
#include <unistd.h>

#include <chrono>
#include <cstdint>
#include <string_view>

class OomKillsWhiteBoxTests : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(OomKillsWhiteBoxTests);
    CPPUNIT_TEST(testKillsBeforeStartAreNotCounted);
    CPPUNIT_TEST(testEachKillIsCountedOnce);
    CPPUNIT_TEST(testKillNotMatchedInTimeIsDropped);
    CPPUNIT_TEST(testKillMatchedInTimeIsCounted);
    CPPUNIT_TEST(testKillCountedInSubforkitIsNotCountedAgain);
    CPPUNIT_TEST(testKillDroppedInSubforkitIsNotCounted);
    CPPUNIT_TEST(testNothingIsCountedWithoutSharedCount);
    CPPUNIT_TEST_SUITE_END();

    void testKillsBeforeStartAreNotCounted();
    void testEachKillIsCountedOnce();
    void testKillNotMatchedInTimeIsDropped();
    void testKillMatchedInTimeIsCounted();
    void testKillCountedInSubforkitIsNotCountedAgain();
    void testKillDroppedInSubforkitIsNotCounted();
    void testNothingIsCountedWithoutSharedCount();
};

namespace
{
constexpr std::chrono::seconds MatchWindow(10);

/// LOK_ASSERT evaluates its condition again when it fails, so assert on the result of this.
int countKills(OomKillCounter& counter)
{
    int count = 0;
    while (counter.take())
        ++count;
    return count;
}

/// Runs function in a child process, as a subforkit.
template <typename Function> int runInChild(Function function)
{
    constexpr std::string_view testname = __func__;

    const pid_t pid = fork();
    LOK_ASSERT_MESSAGE("Failed to fork", pid >= 0);
    if (pid == 0)
        _exit(function());

    int status = 0;
    const pid_t waited = waitpid(pid, &status, 0);
    LOK_ASSERT_EQUAL(pid, waited);
    LOK_ASSERT_MESSAGE("Expected the child to exit", WIFEXITED(status));
    return WEXITSTATUS(status);
}
} // namespace

void OomKillsWhiteBoxTests::testKillsBeforeStartAreNotCounted()
{
    constexpr std::string_view testname = __func__;

    std::uint64_t kernelCount = 5;
    OomKillCounter counter([&kernelCount] { return kernelCount; }, MatchWindow);
    const bool mapped = counter.map();
    LOK_ASSERT(mapped);

    counter.update(std::chrono::steady_clock::now());
    const int counted = countKills(counter);
    LOK_ASSERT_EQUAL(0, counted);
}

void OomKillsWhiteBoxTests::testEachKillIsCountedOnce()
{
    constexpr std::string_view testname = __func__;

    std::uint64_t kernelCount = 5;
    OomKillCounter counter([&kernelCount] { return kernelCount; }, MatchWindow);
    const bool mapped = counter.map();
    LOK_ASSERT(mapped);

    // Two kits are killed before the next reap.
    kernelCount = 7;
    counter.update(std::chrono::steady_clock::now());
    const int counted = countKills(counter);
    LOK_ASSERT_EQUAL(2, counted);
}

void OomKillsWhiteBoxTests::testKillNotMatchedInTimeIsDropped()
{
    constexpr std::string_view testname = __func__;

    std::uint64_t kernelCount = 5;
    OomKillCounter counter([&kernelCount] { return kernelCount; }, MatchWindow);
    const bool mapped = counter.map();
    LOK_ASSERT(mapped);

    // A process that no forkit reaps, such as a background save.
    kernelCount = 6;
    const auto start = std::chrono::steady_clock::now();
    counter.update(start);

    counter.update(start + MatchWindow + std::chrono::seconds(1));
    const int counted = countKills(counter);
    LOK_ASSERT_EQUAL(0, counted);
}

void OomKillsWhiteBoxTests::testKillMatchedInTimeIsCounted()
{
    constexpr std::string_view testname = __func__;

    std::uint64_t kernelCount = 5;
    OomKillCounter counter([&kernelCount] { return kernelCount; }, MatchWindow);
    const bool mapped = counter.map();
    LOK_ASSERT(mapped);

    kernelCount = 6;
    const auto start = std::chrono::steady_clock::now();
    counter.update(start);

    counter.update(start + MatchWindow - std::chrono::seconds(1));
    const int counted = countKills(counter);
    LOK_ASSERT_EQUAL(1, counted);
}

void OomKillsWhiteBoxTests::testKillCountedInSubforkitIsNotCountedAgain()
{
    constexpr std::string_view testname = __func__;

    std::uint64_t kernelCount = 5;
    OomKillCounter counter([&kernelCount] { return kernelCount; }, MatchWindow);
    const bool mapped = counter.map();
    LOK_ASSERT(mapped);

    kernelCount = 6;
    const auto start = std::chrono::steady_clock::now();
    const int countedInSubforkit = runInChild(
        [&]
        {
            counter.update(start);
            return countKills(counter);
        });
    LOK_ASSERT_EQUAL(1, countedInSubforkit);

    counter.update(start);
    const int counted = countKills(counter);
    LOK_ASSERT_EQUAL(0, counted);
}

void OomKillsWhiteBoxTests::testKillDroppedInSubforkitIsNotCounted()
{
    constexpr std::string_view testname = __func__;

    std::uint64_t kernelCount = 5;
    OomKillCounter counter([&kernelCount] { return kernelCount; }, MatchWindow);
    const bool mapped = counter.map();
    LOK_ASSERT(mapped);

    // No reap matches the kill in time.
    kernelCount = 6;
    const auto start = std::chrono::steady_clock::now();
    const int countedInSubforkit = runInChild(
        [&]
        {
            counter.update(start);
            counter.update(start + MatchWindow + std::chrono::seconds(1));
            return countKills(counter);
        });
    LOK_ASSERT_EQUAL(0, countedInSubforkit);

    counter.update(start + MatchWindow + std::chrono::seconds(1));
    const int counted = countKills(counter);
    LOK_ASSERT_EQUAL(0, counted);
}

void OomKillsWhiteBoxTests::testNothingIsCountedWithoutSharedCount()
{
    constexpr std::string_view testname = __func__;

    std::uint64_t kernelCount = 5;
    OomKillCounter counter([&kernelCount] { return kernelCount; }, MatchWindow);

    kernelCount = 6;
    counter.update(std::chrono::steady_clock::now());
    const int countedBeforeMap = countKills(counter);
    LOK_ASSERT_EQUAL(0, countedBeforeMap);

    // A kit unmaps the count after it is forked.
    const bool mapped = counter.map();
    LOK_ASSERT(mapped);
    kernelCount = 7;
    counter.unmap();
    counter.update(std::chrono::steady_clock::now());
    const int countedAfterUnmap = countKills(counter);
    LOK_ASSERT_EQUAL(0, countedAfterUnmap);
}

CPPUNIT_TEST_SUITE_REGISTRATION(OomKillsWhiteBoxTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
