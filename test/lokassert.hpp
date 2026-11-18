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
 * Custom assertion utilities and output operators for test framework integration.
 */

#pragma once

#include <testlog.hpp>

#include <cassert>
#include <vector>
#include <ostream>

#include <cppunit/TestAssert.h>

inline std::ostream& operator<<(std::ostream& os, const std::vector<char>& v)
{
    const std::size_t size = v.size();
    if (size <= 32)
        os << std::string(v.data(), size);
    else
        os << std::string(v.data(), 32) << "...";

    return os;
}

template <typename T, typename U>
inline std::string lokFormatAssertEq(const std::string& expectedName, const T& expected,
                                     const std::string& actualName, const U& actual)
{
    std::ostringstream oss;
    oss << "Expected " << actualName << " [" << std::boolalpha << (actual)
        << "] == " << expectedName << " [" << (expected) << ']';
    return oss.str();
}

template <>
std::string inline lokFormatAssertEq(const std::string& expected_name, const std::string& expected,
                                     const std::string& actual_name, const std::string& actual)
{
    std::string expected_prefix = "Expected (" + expected_name + "): ";
    std::string actual_prefix = "Actual   (" + actual_name + "): ";
    if (expected_prefix.size() > actual_prefix.size())
    {
        while (expected_prefix.size() > actual_prefix.size())
            actual_prefix.append(" ");
    }
    else if (expected_prefix.size() < actual_prefix.size())
    {
        while (expected_prefix.size() < actual_prefix.size())
            expected_prefix.append(" ");
    }

    expected_prefix.append("[");
    actual_prefix.append("[");

    std::ostringstream oss;
    oss << '\n';
    oss << expected_prefix << expected << "]\n";
    oss << actual_prefix << actual << "]\n";

    auto space_count = std::max(expected_prefix.size(), actual_prefix.size());
    while (--space_count > 0)
    {
        oss << ' ';
    }

    oss << '[';

    const auto minSize = std::min(expected.size(), actual.size());
    for (std::size_t i = 0; i < minSize; ++i)
    {
        oss << (expected[i] == actual[i] ? ' ' : '^');
    }

    const auto maxSize = std::max(expected.size(), actual.size());
    for (std::size_t i = minSize; i < maxSize; ++i)
    {
        oss << '^';
    }

    oss << "]\n";

    return oss.str();
}

#if defined(LOK_ABORT_ON_ASSERTION) || defined(__COVERITY__)
#define LOK_ASSERT_IMPL(X) assert(X);
#else
#define LOK_ASSERT_IMPL(X)
#endif //LOK_ABORT_ON_ASSERTION

// When enabled, assertions that pass will be logged.
// configure with --enable-logging-test-assert
#if LOK_LOG_ASSERTIONS
#define LOK_TRACE(X) TST_LOG(X)
#else
#define LOK_TRACE(X)                                                                               \
    do                                                                                             \
    {                                                                                              \
    } while (false)
#endif

namespace test
{
namespace detail
{
/// For tests that don't have a 'failed' member.
constexpr bool failed() { return false; }
} // namespace detail
} // namespace test

#if !defined(__COVERITY__)
#define LOK_ASSERT_MESSAGE_PRIOR_FAILURE failed()
#else
#define LOK_ASSERT_MESSAGE_PRIOR_FAILURE false
#endif

/// Assert the truth of a condition, with a custom message.
/// Will break on failure if stop==true.
#define LOK_ASSERT_MESSAGE_IMPL(message, condition, silent, stop)                                  \
    do                                                                                             \
    {                                                                                              \
        using namespace test::detail;                                                              \
        if (!LOK_ASSERT_MESSAGE_PRIOR_FAILURE)                                                     \
        {                                                                                          \
            if (!(condition))                                                                      \
            {                                                                                      \
                std::ostringstream oss##__LINE__;                                                  \
                oss##__LINE__ << message;                                                          \
                const auto msg##__LINE__ = oss##__LINE__.str();                                    \
                TST_LOG("ERROR: " << (stop ? "Assertion" : "Check") << " failure: "                \
                                  << (msg##__LINE__.empty() ? "" : msg##__LINE__ + ". ")           \
                                  << "Condition: " << (#condition));                               \
                if (stop)                                                                          \
                {                                                                                  \
                    LOK_ASSERT_IMPL(!#condition); /* NOLINT(misc-static-assert) */                 \
                    CPPUNIT_ASSERT_MESSAGE((msg##__LINE__), condition);                            \
                }                                                                                  \
            }                                                                                      \
            else if (!silent)                                                                      \
            {                                                                                      \
                LOK_TRACE("PASS: " << (#condition) << " [true], context: [" << message << "]");    \
            }                                                                                      \
        }                                                                                          \
    } while (false)

/// Check the truth of a condition, with a custom message, logging on success.
#define LOK_CHECK_MESSAGE(message, condition)                                                      \
    LOK_ASSERT_MESSAGE_IMPL(message, condition, false, false)

/// Assert the truth of a condition, with a custom message, logging on success.
#define LOK_ASSERT_MESSAGE(message, condition)                                                     \
    LOK_ASSERT_MESSAGE_IMPL(message, condition, false, true)

/// Check the truth of a condition, with a custom message, without logging on success.
#define LOK_CHECK_MESSAGE_SILENT(message, condition)                                               \
    LOK_ASSERT_MESSAGE_IMPL(message, condition, true, false)

/// Assert the truth of a condition, with a custom message, without logging on success.
#define LOK_ASSERT_MESSAGE_SILENT(message, condition)                                              \
    LOK_ASSERT_MESSAGE_IMPL(message, condition, true, true)

/// Check the truth of a condition, logging on success.
#define LOK_CHECK(condition) LOK_ASSERT_MESSAGE_IMPL("", condition, false, false)

/// Assert the truth of a condition, logging on success, with a context.
#define LOK_ASSERT_CTX(condition, CTX) LOK_ASSERT_MESSAGE_IMPL(CTX, condition, false, true)

/// Assert the truth of a condition, logging on success.
#define LOK_ASSERT(condition) LOK_ASSERT_CTX(condition, "")

/// Check the truth of a condition without logging on success.
#define LOK_CHECK_SILENT(condition) LOK_ASSERT_MESSAGE_IMPL("", condition, true, false)

/// Assert the truth of a condition without logging on success.
#define LOK_ASSERT_SILENT(condition) LOK_ASSERT_MESSAGE_IMPL("", condition, true, true)

/// Assert the equality of two expressions with a custom message. WARNING: Multiple evaluations!
/// Captures full expressions, but only meaningful when they have no side-effects when evaluated.
/// Will break on failure if stop==true.
#define LOK_ASSERT_EQUAL_MESSAGE_UNSAFE(message, expected_name, expected, actual_name, actual,     \
                                        stop)                                                      \
    do                                                                                             \
    {                                                                                              \
        using namespace test::detail;                                                              \
        if (!failed() && !((expected) == (actual)))                                                \
        {                                                                                          \
            std::ostringstream oss##__LINE__;                                                      \
            oss##__LINE__ << message;                                                              \
            const auto msg##__LINE__ = oss##__LINE__.str();                                        \
            TST_LOG("ERROR: " << ((stop) ? "Assertion" : "Check") << " failure: "                  \
                              << (msg##__LINE__.empty() ? "" : msg##__LINE__ + ' ')                \
                              << lokFormatAssertEq(expected_name, expected, actual_name, actual)); \
            if (stop)                                                                              \
            {                                                                                      \
                LOK_ASSERT_IMPL((expected) == (actual));                                           \
                CPPUNIT_ASSERT_EQUAL_MESSAGE(msg##__LINE__, (expected), (actual));                 \
            }                                                                                      \
        }                                                                                          \
    } while (false)

/// Assert the equality of two expressions, and a custom message, with guaranteed single evaluation.
/// Will break on failure if STOP==true.
#define LOK_ASSERT_EQUAL_MESSAGE_IML(MSG, EXP, ACT, STOP)                                          \
    do                                                                                             \
    {                                                                                              \
        auto&& exp##__LINE__ = EXP;                                                                \
        auto&& act##__LINE__ = ACT;                                                                \
        if (!(exp##__LINE__ == act##__LINE__))                                                     \
        {                                                                                          \
            LOK_ASSERT_EQUAL_MESSAGE_UNSAFE(MSG, #EXP, exp##__LINE__, #ACT, act##__LINE__, STOP);  \
        }                                                                                          \
        else                                                                                       \
        {                                                                                          \
            LOK_TRACE("PASS: " << #EXP << " == " << #ACT << " == [" << act##__LINE__ << ']');      \
        }                                                                                          \
    } while (false)

/// Check the equality of two expressions, and a custom message, with guaranteed single evaluation.
/// Will *not* break on failure.
#define LOK_CHECK_EQUAL_MESSAGE(MSG, EXP, ACT) LOK_ASSERT_EQUAL_MESSAGE_IML(MSG, EXP, ACT, false)

/// Assert the equality of two expressions, and a custom message, with guaranteed single evaluation.
/// Will break on failure.
#define LOK_ASSERT_EQUAL_MESSAGE(MSG, EXP, ACT) LOK_ASSERT_EQUAL_MESSAGE_IML(MSG, EXP, ACT, true)

/// Check the equality of two expressions with guarantees of single evaluation.
#define LOK_CHECK_EQUAL(EXP, ACT) LOK_CHECK_EQUAL_MESSAGE((#EXP) << " != " << (#ACT), EXP, ACT)

/// Check the equality of two expressions with guarantees of single evaluation, with a context.
/// ERROR: Check failure: 1 != 0, context: [comparing 0 and 1], Expected 0 [0] == 1 [1]
#define LOK_CHECK_EQUAL_CTX(EXP, ACT, CTX)                                                         \
    LOK_CHECK_EQUAL_MESSAGE((#EXP) << " != " << (#ACT) << ", context: [" << CTX << "],", EXP, ACT)

/// Assert the equality of two expressions with guarantees of single evaluation.
#define LOK_ASSERT_EQUAL(EXP, ACT) LOK_ASSERT_EQUAL_MESSAGE((#EXP) << " != " << (#ACT), EXP, ACT)

/// Check the equality of two expressions with guarantees of single evaluation, with a context.
/// ERROR: Assertion failure: 1 != 0, context: [comparing 0 and 1], Expected 0 [0] == 1 [1]
#define LOK_ASSERT_EQUAL_CTX(EXP, ACT, CTX)                                                        \
    LOK_ASSERT_EQUAL_MESSAGE((#EXP) << " != " << (#ACT) << ", context: [" << CTX << "],", EXP, ACT)

/// Check the equality of two expressions with guarantees of single evaluation.
#define LOK_CHECK_EQUAL_STR(EXP, ACT)                                                              \
    LOK_CHECK_EQUAL_MESSAGE((#EXP) << " != " << (#ACT), Util::toString(EXP), Util::toString(ACT))

/// Assert the equality of two expressions with guarantees of single evaluation.
#define LOK_ASSERT_EQUAL_STR(EXP, ACT)                                                             \
    LOK_ASSERT_EQUAL_MESSAGE((#EXP) << " != " << (#ACT), Util::toString(EXP), Util::toString(ACT))

#define LOK_ASSERT_PASS(message) TST_LOG("PASS: " << message)

#define LOK_ASSERT_FAIL(message)                                                                   \
    do                                                                                             \
    {                                                                                              \
        TST_LOG("ERROR: Forced failure: " << message);                                             \
        LOK_ASSERT_IMPL(!"Forced failure: " #message); /* NOLINT(misc-static-assert) */            \
        std::stringstream dummyStringstream;                                                       \
        dummyStringstream << message;                                                              \
        CPPUNIT_FAIL(dummyStringstream.str().c_str());                                             \
    } while (false)

/// A failed assertion with context.
#define LOK_ASSERT_FAIL_CTX(message, CTX)                                                          \
    do                                                                                             \
    {                                                                                              \
        TST_LOG("ERROR: Forced failure: " << message << ' ' << CTX);                               \
        LOK_ASSERT_IMPL(!"Forced failure: " #message); /* NOLINT(misc-static-assert) */            \
        std::stringstream dummyStringstream;                                                       \
        dummyStringstream << message << ' ' << CTX;                                                \
        CPPUNIT_FAIL(dummyStringstream.str().c_str());                                             \
    } while (false)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
