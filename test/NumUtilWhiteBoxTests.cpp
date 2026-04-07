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

#include <common/NumUtil.hpp>

#include <test/lokassert.hpp>

#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>

#include <cerrno>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>

/// Numeric utility unit-tests.
class NumUtilWhiteBoxTests : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(NumUtilWhiteBoxTests);
    CPPUNIT_TEST(testI32FromString);
    CPPUNIT_TEST(testI64FromString);
    CPPUNIT_TEST(testU32FromString);
    CPPUNIT_TEST(testU64FromString);
    CPPUNIT_TEST(testStoi);
    CPPUNIT_TEST(testStrtoint64MatchesStrtol);
    CPPUNIT_TEST(testStrtouint64MatchesStrtoul);
    CPPUNIT_TEST(testStrtoint64MatchesStrtoll);
    CPPUNIT_TEST(testStrtouint64MatchesStrtoull);
    CPPUNIT_TEST(testParseStrTo);
    CPPUNIT_TEST_SUITE_END();

    void testI32FromString();
    void testI64FromString();
    void testU32FromString();
    void testU64FromString();
    void testStoi();
    void testStrtoint64MatchesStrtol();
    void testStrtouint64MatchesStrtoul();
    void testStrtoint64MatchesStrtoll();
    void testStrtouint64MatchesStrtoull();
    void testParseStrTo();
    void testStrtoiWithOffset();

    void stoiCompare(std::int64_t num);
    void stoiTest(const std::string& str, std::int64_t num);
};

void NumUtilWhiteBoxTests::testI32FromString()
{
    constexpr std::string_view testname = __func__;

    // Basic positive numbers.
    {
        const auto [value, success] = NumUtil::i32FromString("0");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(static_cast<std::int32_t>(0), value);
    }
    {
        const auto [value, success] = NumUtil::i32FromString("42");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(static_cast<std::int32_t>(42), value);
    }
    {
        const auto [value, success] = NumUtil::i32FromString("12345");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(static_cast<std::int32_t>(12345), value);
    }
    {
        const auto [value, success] = NumUtil::i32FromString("12345,");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(static_cast<std::int32_t>(12345), value);
    }

    // Negative numbers.
    {
        const auto [value, success] = NumUtil::i32FromString("-1");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(static_cast<std::int32_t>(-1), value);
    }
    {
        const auto [value, success] = NumUtil::i32FromString("-999");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(static_cast<std::int32_t>(-999), value);
    }
    {
        const auto [value, success] = NumUtil::i32FromString("-999,");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(static_cast<std::int32_t>(-999), value);
    }

    // INT32_MAX boundary.
    {
        const auto [value, success] = NumUtil::i32FromString("2147483647");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(std::numeric_limits<std::int32_t>::max(), value);
    }

    // INT32_MIN boundary.
    {
        const auto [value, success] = NumUtil::i32FromString("-2147483648");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(std::numeric_limits<std::int32_t>::min(), value);
    }

    // Overflow returns failure.
    {
        const auto [value, success] = NumUtil::i32FromString("99999999999999");
        LOK_ASSERT(!success);
    }
    {
        const auto [value, success] = NumUtil::i32FromString("-99999999999999");
        LOK_ASSERT(!success);
    }

    // Empty and invalid strings.
    {
        const auto [value, success] = NumUtil::i32FromString("");
        LOK_ASSERT(!success);
    }
    {
        const auto [value, success] = NumUtil::i32FromString("abc");
        LOK_ASSERT(!success);
    }

    // Trailing non-numeric characters (partial parse, still succeeds).
    {
        const auto [value, success] = NumUtil::i32FromString("42xy");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(static_cast<std::int32_t>(42), value);
    }

    // Leading whitespace.
    {
        const auto [value, success] = NumUtil::i32FromString("  42");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(static_cast<std::int32_t>(42), value);
    }
    {
        const auto [value, success] = NumUtil::i32FromString("\t -7");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(static_cast<std::int32_t>(-7), value);
    }

    // Leading '+' sign.
    {
        const auto [value, success] = NumUtil::i32FromString("+99");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(static_cast<std::int32_t>(99), value);
    }

    // Whitespace-only is empty/failure.
    {
        const auto [value, success] = NumUtil::i32FromString("   ");
        LOK_ASSERT(!success);
    }

    // Just a sign with no digits.
    {
        const auto [value, success] = NumUtil::i32FromString("+");
        LOK_ASSERT(!success);
    }
    {
        const auto [value, success] = NumUtil::i32FromString("-");
        LOK_ASSERT(!success);
    }

    // INT32_MAX + 1 overflows.
    {
        const auto [value, success] = NumUtil::i32FromString("2147483648");
        LOK_ASSERT(!success);
    }

    // INT32_MIN - 1 overflows.
    {
        const auto [value, success] = NumUtil::i32FromString("-2147483649");
        LOK_ASSERT(!success);
    }

    // Single digit.
    {
        const auto [value, success] = NumUtil::i32FromString("7");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(static_cast<std::int32_t>(7), value);
    }

    // Zero with trailing content.
    {
        const auto [value, success] = NumUtil::i32FromString("0abc");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(static_cast<std::int32_t>(0), value);
    }

    // Default-value overload.
    LOK_ASSERT_EQUAL(static_cast<std::int32_t>(42), NumUtil::i32FromString("42", -1));
    LOK_ASSERT_EQUAL(static_cast<std::int32_t>(-1), NumUtil::i32FromString("", -1));
    LOK_ASSERT_EQUAL(static_cast<std::int32_t>(-1), NumUtil::i32FromString("abc", -1));
    LOK_ASSERT_EQUAL(static_cast<std::int32_t>(99), NumUtil::i32FromString("99999999999999", 99));
    LOK_ASSERT_EQUAL(std::numeric_limits<std::int32_t>::max(),
                     NumUtil::i32FromString("2147483647", 0));
    LOK_ASSERT_EQUAL(std::numeric_limits<std::int32_t>::min(),
                     NumUtil::i32FromString("-2147483648", 0));
    LOK_ASSERT_EQUAL(static_cast<std::int32_t>(55), NumUtil::i32FromString("  55", -1));
    LOK_ASSERT_EQUAL(static_cast<std::int32_t>(-1), NumUtil::i32FromString("   ", -1));
    LOK_ASSERT_EQUAL(static_cast<std::int32_t>(-1), NumUtil::i32FromString("+", -1));
    LOK_ASSERT_EQUAL(static_cast<std::int32_t>(99), NumUtil::i32FromString("2147483648", 99));
}

void NumUtilWhiteBoxTests::testI64FromString()
{
    constexpr std::string_view testname = __func__;

    // Basic positive numbers.
    {
        const auto [value, success] = NumUtil::i64FromString("0");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(static_cast<std::int64_t>(0), value);
    }
    {
        const auto [value, success] = NumUtil::i64FromString("42");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(static_cast<std::int64_t>(42), value);
    }
    {
        const auto [value, success] = NumUtil::i64FromString("123456789012345");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(static_cast<std::int64_t>(123456789012345LL), value);
    }

    // Trailing non-numeric characters (partial parse, still succeeds).
    {
        const auto [value, success] = NumUtil::i64FromString("999abc");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(static_cast<std::int64_t>(999), value);
    }
    {
        const auto [value, success] = NumUtil::i64FromString("100,200");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(static_cast<std::int64_t>(100), value);
    }

    // Negative numbers.
    {
        const auto [value, success] = NumUtil::i64FromString("-1");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(static_cast<std::int64_t>(-1), value);
    }
    {
        const auto [value, success] = NumUtil::i64FromString("-123456789012345");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(static_cast<std::int64_t>(-123456789012345LL), value);
    }

    // INT64_MAX boundary.
    {
        const auto [value, success] = NumUtil::i64FromString("9223372036854775807");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(std::numeric_limits<std::int64_t>::max(), value);
    }

    // INT64_MIN boundary.
    {
        const auto [value, success] = NumUtil::i64FromString("-9223372036854775808");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(std::numeric_limits<std::int64_t>::min(), value);
    }

    // INT64_MAX + 1 overflows.
    {
        const auto [value, success] = NumUtil::i64FromString("9223372036854775808");
        LOK_ASSERT(!success);
    }

    // INT64_MIN - 1 overflows.
    {
        const auto [value, success] = NumUtil::i64FromString("-9223372036854775809");
        LOK_ASSERT(!success);
    }

    // Empty and invalid strings.
    {
        const auto [value, success] = NumUtil::i64FromString("");
        LOK_ASSERT(!success);
    }
    {
        const auto [value, success] = NumUtil::i64FromString("abc");
        LOK_ASSERT(!success);
    }

    // Leading whitespace.
    {
        const auto [value, success] = NumUtil::i64FromString("  42");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(static_cast<std::int64_t>(42), value);
    }
    {
        const auto [value, success] = NumUtil::i64FromString("\t -7");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(static_cast<std::int64_t>(-7), value);
    }

    // Leading '+' sign.
    {
        const auto [value, success] = NumUtil::i64FromString("+99");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(static_cast<std::int64_t>(99), value);
    }

    // Whitespace-only is failure.
    {
        const auto [value, success] = NumUtil::i64FromString("   ");
        LOK_ASSERT(!success);
    }

    // Just a sign with no digits.
    {
        const auto [value, success] = NumUtil::i64FromString("+");
        LOK_ASSERT(!success);
    }
    {
        const auto [value, success] = NumUtil::i64FromString("-");
        LOK_ASSERT(!success);
    }

    // Single digit.
    {
        const auto [value, success] = NumUtil::i64FromString("7");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(static_cast<std::int64_t>(7), value);
    }

    // Default-value overload.
    LOK_ASSERT_EQUAL(static_cast<std::int64_t>(42), NumUtil::i64FromString("42", -1));
    LOK_ASSERT_EQUAL(static_cast<std::int64_t>(-1), NumUtil::i64FromString("", -1));
    LOK_ASSERT_EQUAL(static_cast<std::int64_t>(-1), NumUtil::i64FromString("abc", -1));
    LOK_ASSERT_EQUAL(static_cast<std::int64_t>(55), NumUtil::i64FromString("  55", -1));
    LOK_ASSERT_EQUAL(static_cast<std::int64_t>(-1), NumUtil::i64FromString("   ", -1));
    LOK_ASSERT_EQUAL(static_cast<std::int64_t>(-1), NumUtil::i64FromString("+", -1));
    LOK_ASSERT_EQUAL(static_cast<std::int64_t>(99),
                     NumUtil::i64FromString("9223372036854775808", 99));
    LOK_ASSERT_EQUAL(static_cast<std::int64_t>(99),
                     NumUtil::i64FromString("-9223372036854775809", 99));
}

void NumUtilWhiteBoxTests::testU32FromString()
{
    constexpr std::string_view testname = __func__;

    // Basic positive numbers.
    {
        const auto [value, success] = NumUtil::u32FromString("0");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(static_cast<std::uint32_t>(0), value);
    }
    {
        const auto [value, success] = NumUtil::u32FromString("42");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(static_cast<std::uint32_t>(42), value);
    }
    {
        const auto [value, success] = NumUtil::u32FromString("12345");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(static_cast<std::uint32_t>(12345), value);
    }

    // Trailing non-numeric characters (partial parse, still succeeds).
    {
        const auto [value, success] = NumUtil::u32FromString("42xy");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(static_cast<std::uint32_t>(42), value);
    }
    {
        const auto [value, success] = NumUtil::u32FromString("100,200");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(static_cast<std::uint32_t>(100), value);
    }

    // UINT32_MAX boundary.
    {
        const auto [value, success] = NumUtil::u32FromString("4294967295");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(std::numeric_limits<std::uint32_t>::max(), value);
    }

    // UINT32_MAX + 1 overflows.
    {
        const auto [value, success] = NumUtil::u32FromString("4294967296");
        LOK_ASSERT(!success);
    }

    // Large overflow.
    {
        const auto [value, success] = NumUtil::u32FromString("99999999999999");
        LOK_ASSERT(!success);
    }

    // Negative values are invalid for unsigned.
    {
        const auto [value, success] = NumUtil::u32FromString("-1");
        LOK_ASSERT(!success);
    }
    {
        const auto [value, success] = NumUtil::u32FromString("-0");
        LOK_ASSERT(!success);
    }

    // Empty and invalid strings.
    {
        const auto [value, success] = NumUtil::u32FromString("");
        LOK_ASSERT(!success);
    }
    {
        const auto [value, success] = NumUtil::u32FromString("abc");
        LOK_ASSERT(!success);
    }

    // Leading whitespace.
    {
        const auto [value, success] = NumUtil::u32FromString("  42");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(static_cast<std::uint32_t>(42), value);
    }

    // Leading '+' sign.
    {
        const auto [value, success] = NumUtil::u32FromString("+99");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(static_cast<std::uint32_t>(99), value);
    }

    // Whitespace-only is failure.
    {
        const auto [value, success] = NumUtil::u32FromString("   ");
        LOK_ASSERT(!success);
    }

    // Just a sign with no digits.
    {
        const auto [value, success] = NumUtil::u32FromString("+");
        LOK_ASSERT(!success);
    }

    // Single digit.
    {
        const auto [value, success] = NumUtil::u32FromString("7");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(static_cast<std::uint32_t>(7), value);
    }

    // Default-value overload.
    LOK_ASSERT_EQUAL(static_cast<std::uint32_t>(42), NumUtil::u32FromString("42", 99));
    LOK_ASSERT_EQUAL(static_cast<std::uint32_t>(99), NumUtil::u32FromString("", 99));
    LOK_ASSERT_EQUAL(static_cast<std::uint32_t>(99), NumUtil::u32FromString("abc", 99));
    LOK_ASSERT_EQUAL(static_cast<std::uint32_t>(99), NumUtil::u32FromString("-1", 99));
    LOK_ASSERT_EQUAL(static_cast<std::uint32_t>(88), NumUtil::u32FromString("  ", 88));
    LOK_ASSERT_EQUAL(static_cast<std::uint32_t>(50), NumUtil::u32FromString("  50", 99));
    LOK_ASSERT_EQUAL(static_cast<std::uint32_t>(77), NumUtil::u32FromString("4294967296", 77));
}

void NumUtilWhiteBoxTests::testU64FromString()
{
    constexpr std::string_view testname = __func__;

    // Basic positive numbers.
    {
        const auto [value, success] = NumUtil::u64FromString("0");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(static_cast<std::uint64_t>(0), value);
    }
    {
        const auto [value, success] = NumUtil::u64FromString("42");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(static_cast<std::uint64_t>(42), value);
    }
    {
        const auto [value, success] = NumUtil::u64FromString("123456789");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(static_cast<std::uint64_t>(123456789), value);
    }
    {
        const auto [value, success] = NumUtil::u64FromString("123456789,");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(static_cast<std::uint64_t>(123456789), value);
    }

    // Large values.
    {
        const auto [value, success] = NumUtil::u64FromString("18446744073709551615");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(std::numeric_limits<std::uint64_t>::max(), value);
    }

    // Overflow returns failure.
    {
        const auto [value, success] = NumUtil::u64FromString("18446744073709551616");
        LOK_ASSERT(!success);
    }

    // Empty and invalid strings.
    {
        const auto [value, success] = NumUtil::u64FromString("");
        LOK_ASSERT(!success);
    }
    {
        const auto [value, success] = NumUtil::u64FromString("abc");
        LOK_ASSERT(!success);
    }

    // Trailing non-numeric characters.
    {
        const auto [value, success] = NumUtil::u64FromString("42xy");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(static_cast<std::uint64_t>(42), value);
    }

    // Negative values are invalid for unsigned.
    {
        const auto [value, success] = NumUtil::u64FromString("-1");
        LOK_ASSERT(!success);
    }
    {
        const auto [value, success] = NumUtil::u64FromString("-0");
        LOK_ASSERT(!success);
    }

    // Leading whitespace.
    {
        const auto [value, success] = NumUtil::u64FromString("  100");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(static_cast<std::uint64_t>(100), value);
    }

    // Leading '+' sign.
    {
        const auto [value, success] = NumUtil::u64FromString("+42");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(static_cast<std::uint64_t>(42), value);
    }

    // Whitespace-only is failure.
    {
        const auto [value, success] = NumUtil::u64FromString("   ");
        LOK_ASSERT(!success);
    }

    // Just a sign with no digits.
    {
        const auto [value, success] = NumUtil::u64FromString("+");
        LOK_ASSERT(!success);
    }

    // Zero with trailing content.
    {
        const auto [value, success] = NumUtil::u64FromString("0xyz");
        LOK_ASSERT(success);
        LOK_ASSERT_EQUAL(static_cast<std::uint64_t>(0), value);
    }

    // Default-value overload.
    LOK_ASSERT_EQUAL(static_cast<std::uint64_t>(42), NumUtil::u64FromString("42", 99));
    LOK_ASSERT_EQUAL(static_cast<std::uint64_t>(99), NumUtil::u64FromString("", 99));
    LOK_ASSERT_EQUAL(static_cast<std::uint64_t>(99), NumUtil::u64FromString("abc", 99));
    LOK_ASSERT_EQUAL(static_cast<std::uint64_t>(77),
                     NumUtil::u64FromString("18446744073709551616", 77));
    LOK_ASSERT_EQUAL(static_cast<std::uint64_t>(99), NumUtil::u64FromString("-1", 99));
    LOK_ASSERT_EQUAL(static_cast<std::uint64_t>(88), NumUtil::u64FromString("  ", 88));
    LOK_ASSERT_EQUAL(static_cast<std::uint64_t>(50), NumUtil::u64FromString("  50", 99));
}

void NumUtilWhiteBoxTests::stoiTest(const std::string& str, std::int64_t num)
{
    constexpr std::string_view testname = __func__;

    bool unexpectedThrow = false;
    try
    {
        const auto value = std::stoi(str);
        unexpectedThrow = true; // std::stoi() didn't throw, so we shouldn't either.

        LOK_ASSERT_EQUAL(num, static_cast<std::int64_t>(value));
        LOK_ASSERT_EQUAL(num, static_cast<std::int64_t>(NumUtil::stoi(str)));
    }
    catch (const std::invalid_argument&)
    {
        if (!unexpectedThrow)
        {
            TST_LOG("std::stoi(" << str
                                 << ") threw invalid_argument, now checking NumUtil::stoi()");
        }
        else
        {
            LOK_ASSERT_FAIL("Unexpected to get invalid_argument exception for [" << str << ']');
        }

        try
        {
            LOK_ASSERT_EQUAL_CTX(num, static_cast<std::int64_t>(NumUtil::stoi(str)), str);
            LOK_ASSERT_FAIL("Expected invalid_argument exception to be thrown for [" << str << ']');
        }
        catch (const std::invalid_argument&)
        {
            LOK_ASSERT_PASS("Got invalid_argument exception as expected for [" << str << ']');
        }
    }
    catch (const std::out_of_range&)
    {
        if (!unexpectedThrow)
        {
            TST_LOG("std::stoi(" << str << ") threw out_of_range, now checking NumUtil::stoi()");
        }
        else
        {
            LOK_ASSERT_FAIL("Unexpected to get out_of_range exception for [" << str << ']');
        }

        try
        {
            LOK_ASSERT_EQUAL_CTX(num, static_cast<std::int64_t>(NumUtil::stoi(str)), str);
            LOK_ASSERT_FAIL("Expected out_of_range exception to be thrown for [" << str << ']');
        }
        catch (const std::out_of_range&)
        {
            LOK_ASSERT_PASS("Got out_of_range exception as expected for [" << str << ']');
        }
    }
    catch (const std::exception&)
    {
        if (!unexpectedThrow)
        {
            TST_LOG("std::stoi(" << str << ") threw an exception, now checking NumUtil::stoi()");
        }
        else
        {
            LOK_ASSERT_FAIL("Unexpected to get exception for [" << str << ']');
        }

        try
        {
            LOK_ASSERT_EQUAL_CTX(num, static_cast<std::int64_t>(NumUtil::stoi(str)), str);
            LOK_ASSERT_FAIL("Expected exception to be thrown for [" << str << ']');
        }
        catch (const std::exception&)
        {
            LOK_ASSERT_PASS("Got exception as expected for [" << str << ']');
        }
    }
}

void NumUtilWhiteBoxTests::stoiCompare(std::int64_t num)
{
    constexpr std::string_view testname = __func__;

    LOK_ASSERT_EQUAL(num, num);
    const auto str = std::to_string(num);
    LOK_ASSERT(!str.empty());

    stoiTest(str, num);
}

void NumUtilWhiteBoxTests::testStoi()
{
    constexpr std::string_view testname = __func__;

    try
    {
        stoiCompare(0);
        stoiCompare(1);
        stoiCompare(-1);
        stoiCompare(1L << 34);
        stoiCompare(-(1L << 34));
        for (int i = 0; i < 10000; ++i)
        {
            stoiCompare(Util::rng::getNext());
        }

        // Test empty string - should throw invalid_argument.
        stoiTest("", 0);

        // Test whitespace only - should throw invalid_argument.
        stoiTest("   ", 0);

        // Test non-numeric string - should throw invalid_argument.
        stoiTest("abc", 0);

        // Test string starting with letters - should throw invalid_argument.
        stoiTest("abc123", 0);

        // Test leading whitespace with valid number - should parse successfully.
        LOK_ASSERT_EQUAL(123, NumUtil::stoi("  123"));
        LOK_ASSERT_EQUAL(456, NumUtil::stoi("\t456"));
        LOK_ASSERT_EQUAL(789, NumUtil::stoi("   \t  789"));

        // Test trailing non-numeric characters - should parse the numeric part.
        LOK_ASSERT_EQUAL(123, NumUtil::stoi("123abc"));
        LOK_ASSERT_EQUAL(456, NumUtil::stoi("456xyz"));
        LOK_ASSERT_EQUAL(789, NumUtil::stoi("789   "));

        // Test plus sign prefix - should work.
        LOK_ASSERT_EQUAL(123, NumUtil::stoi("+123"));
        LOK_ASSERT_EQUAL(0, NumUtil::stoi("+0"));

        // Test minus sign prefix - should work.
        LOK_ASSERT_EQUAL(-123, NumUtil::stoi("-123"));
        LOK_ASSERT_EQUAL(0, NumUtil::stoi("-0"));

        // Test just a minus sign - should throw invalid_argument.
        stoiTest("-", 0);

        // Test just a plus sign - should throw invalid_argument.
        stoiTest("+", 0);

        // Test double signs - should throw invalid_argument.
        stoiTest("--123", 0);

        // Test double signs + should throw invalid_argument.
        stoiTest("++123", 0);

        // Test leading zeros - should work.
        LOK_ASSERT_EQUAL(123, NumUtil::stoi("00123"));
        LOK_ASSERT_EQUAL(0, NumUtil::stoi("0000"));
        LOK_ASSERT_EQUAL(-123, NumUtil::stoi("-00123"));

        // Test INT32_MAX boundary.
        LOK_ASSERT_EQUAL(std::numeric_limits<std::int32_t>::max(), NumUtil::stoi("2147483647"));

        // Test INT32_MIN boundary.
        LOK_ASSERT_EQUAL(std::numeric_limits<std::int32_t>::min(), NumUtil::stoi("-2147483648"));

        // Test INT32_MAX + 1 - should throw out_of_range.
        stoiTest("2147483648", 0);

        // Test INT32_MIN - 1 - should throw out_of_range.
        stoiTest("-2147483649", 0);

        // Test very large positive number - should throw out_of_range.
        stoiTest("9999999999999999999", 0);

        // Test very large negative number - should throw out_of_range.
        stoiTest("-9999999999999999999", 0);

        // Test single zero.
        LOK_ASSERT_EQUAL(0, NumUtil::stoi("0"));

        // Test negative zero.
        LOK_ASSERT_EQUAL(0, NumUtil::stoi("-0"));

        // Test positive zero.
        LOK_ASSERT_EQUAL(0, NumUtil::stoi("+0"));

        // Test single digit numbers.
        LOK_ASSERT_EQUAL(1, NumUtil::stoi("1"));
        LOK_ASSERT_EQUAL(9, NumUtil::stoi("9"));
        LOK_ASSERT_EQUAL(-1, NumUtil::stoi("-1"));
        LOK_ASSERT_EQUAL(-9, NumUtil::stoi("-9"));

        // Test common values.
        LOK_ASSERT_EQUAL(100, NumUtil::stoi("100"));
        LOK_ASSERT_EQUAL(1000, NumUtil::stoi("1000"));
        LOK_ASSERT_EQUAL(1000000, NumUtil::stoi("1000000"));
        LOK_ASSERT_EQUAL(-100, NumUtil::stoi("-100"));
        LOK_ASSERT_EQUAL(-1000, NumUtil::stoi("-1000"));
        LOK_ASSERT_EQUAL(-1000000, NumUtil::stoi("-1000000"));

        // Test whitespace + bare sign - should throw invalid_argument.
        stoiTest(" -", 0);
        stoiTest("\t+", 0);
        stoiTest("  +", 0);

        // Test whitespace + sign + non-digit - should throw invalid_argument.
        stoiTest(" -a", 0);

        // Test tab-only whitespace - should throw invalid_argument.
        stoiTest("\t", 0);
        stoiTest("\t\t", 0);

        // Test mixed double signs - should throw invalid_argument.
        stoiTest("+-123", 0);
    }
    catch (const std::exception& exc)
    {
        LOK_ASSERT_FAIL("Unexpected: " << exc.what());
    }
}

void NumUtilWhiteBoxTests::testStrtoint64MatchesStrtol()
{
    constexpr std::string_view testname = __func__;

    auto compareWithStrtol = [&](const std::string& str)
    {
        TST_LOG("Converting [" << str << ']');

        char* endptr = nullptr;
        errno = 0;
        const long stdResult = std::strtol(str.c_str(), &endptr, 10);
        const int stdErrno = errno;
        const std::size_t stdOffset = endptr - str.c_str();
        TST_LOG("Std converted [" << str << "] to [" << stdResult << "] with errno [" << stdErrno
                                  << "] and offset [" << stdOffset << ']');

        std::size_t numUtilOffset = 0;
        const auto [numUtilResult, numUtilState] = NumUtil::parseStrToInt64(str, numUtilOffset);
        TST_LOG("Num converted [" << str << "] to [" << numUtilResult << "] with state ["
                                  << static_cast<int>(numUtilState) << "] and offset ["
                                  << numUtilOffset << ']');

        // Compare results.
        if (stdErrno == ERANGE)
        {
            // Overflow case.
            LOK_ASSERT_CTX(numUtilState == NumUtil::StrToState::Overflow, str);
            // For overflow, NumUtil should return INT_MIN or INT_MAX.
            LOK_ASSERT_CTX(numUtilResult == std::numeric_limits<std::int64_t>::min() ||
                               numUtilResult == std::numeric_limits<std::int64_t>::max(),
                           str);
        }
        else if (endptr == str.c_str())
        {
            // No conversion - NumUtil returns 0 for empty/invalid input.
            LOK_ASSERT_EQUAL_CTX(static_cast<std::int64_t>(0), numUtilResult, str);
        }
        else
        {
            // Valid conversion - results should match (within int range).
            LOK_ASSERT_EQUAL_CTX(static_cast<int64_t>(stdResult), numUtilResult, str);
        }

        // Offset should match endptr position.
        LOK_ASSERT_EQUAL_CTX(stdOffset, numUtilOffset, str);
    };

    // Test basic positive numbers with strtol.
    compareWithStrtol("0");
    compareWithStrtol("1");
    compareWithStrtol("42");
    compareWithStrtol("123");
    compareWithStrtol("999");
    compareWithStrtol("12345");

    // Test negative numbers with strtol.
    compareWithStrtol("-1");
    compareWithStrtol("-42");
    compareWithStrtol("-123");
    compareWithStrtol("-999");
    compareWithStrtol("-12345");

    // Test with leading whitespace.
    compareWithStrtol("  42");
    compareWithStrtol("\t123");
    compareWithStrtol("   -456");

    // Test with leading zeros.
    compareWithStrtol("0042");
    compareWithStrtol("00123");
    compareWithStrtol("-00456");

    // Test with plus sign.
    compareWithStrtol("+42");
    compareWithStrtol("+123");
    compareWithStrtol("+0");

    // Test INT_MAX boundary.
    compareWithStrtol("2147483647");
    compareWithStrtol("-2147483648");

    // Test overflow cases (should set errno to ERANGE).
    compareWithStrtol("2147483648"); // INT_MAX + 1.
    compareWithStrtol("-2147483649"); // INT_MIN - 1.
    compareWithStrtol("9999999999999");

    // Test empty and invalid strings.
    compareWithStrtol("");
    compareWithStrtol("   ");
    compareWithStrtol("abc");
    compareWithStrtol("-");
    compareWithStrtol("+");

    // Test whitespace + bare sign.
    compareWithStrtol(" -");
    compareWithStrtol("\t+");

    // Test sign followed by non-digit.
    compareWithStrtol("-a");
    compareWithStrtol("+x");

    // Test double signs.
    compareWithStrtol("--1");
    compareWithStrtol("+-1");

    // Test trailing text (verifies offset matches endptr).
    compareWithStrtol("123abc");
}

void NumUtilWhiteBoxTests::testStrtouint64MatchesStrtoul()
{
    constexpr std::string_view testname = __func__;

    // Helper lambda to compare NumUtil::strtoul with std::strtoul.
    auto compareWithStrtoul = [&](const std::string& str)
    {
        TST_LOG("Converting [" << str << ']');

        char* endptr = nullptr;
        errno = 0;
        const unsigned long stdResult = std::strtoul(str.c_str(), &endptr, 10);
        const int stdErrno = errno;
        const std::size_t stdOffset = endptr - str.c_str();
        TST_LOG("Std converted [" << str << "] to [" << stdResult << "] with errno [" << stdErrno
                                  << "] and offset [" << stdOffset << ']');

        std::size_t numUtilOffset = 0;
        const auto [numUtilResult, numUtilState] = NumUtil::parseStrToUint64(str, numUtilOffset);
        TST_LOG("Num converted [" << str << "] to [" << numUtilResult << "] with state ["
                                  << static_cast<int>(numUtilState) << "] and offset ["
                                  << numUtilOffset << ']');

        // Compare results.
        if (stdErrno == ERANGE)
        {
            // Overflow case.
            LOK_ASSERT_CTX(numUtilState == NumUtil::StrToState::Overflow, str);
            // For overflow, NumUtil should return UINT_MAX.
            LOK_ASSERT_EQUAL_CTX(std::numeric_limits<uint64_t>::max(), numUtilResult, str);
        }
        else if (endptr == str.c_str())
        {
            // No conversion - NumUtil returns 0 for empty/invalid input.
            LOK_ASSERT_EQUAL_CTX(static_cast<uint64_t>(0), numUtilResult, str);
        }
        else
        {
            // Valid conversion - results should match (within unsigned range).
            LOK_ASSERT_EQUAL_CTX(static_cast<uint64_t>(stdResult), numUtilResult, str);
        }

        // Offset should match endptr position.
        LOK_ASSERT_EQUAL_CTX(stdOffset, numUtilOffset, str);
    };

    // Test unsigned numbers with strtoul.
    compareWithStrtoul("0");
    compareWithStrtoul("1");
    compareWithStrtoul("42");
    compareWithStrtoul("123");
    compareWithStrtoul("999");
    compareWithStrtoul("12345");

    // Test large unsigned numbers.
    compareWithStrtoul("4294967295"); // UINT_MAX.

    // Test with leading whitespace.
    compareWithStrtoul("  42");
    compareWithStrtoul("\t123");

    // Test with leading zeros.
    compareWithStrtoul("0042");
    compareWithStrtoul("00123");

    // Test with plus sign.
    compareWithStrtoul("+42");
    compareWithStrtoul("+123");

    // Test overflow for unsigned (should set errno to ERANGE).
    compareWithStrtoul("4294967296"); // UINT_MAX + 1.
    compareWithStrtoul("9999999999999");

    // Test empty and invalid strings.
    compareWithStrtoul("");
    compareWithStrtoul("   ");
    compareWithStrtoul("abc");

    // Test bare signs.
    compareWithStrtoul("-");
    compareWithStrtoul("+");

    // Test negative numbers (strtoul wraps these).
    compareWithStrtoul("-1"); // We return overflow, but that's the same value.
    // compareWithStrtoul("-42"); // We return overflow.

    // Test whitespace + bare sign.
    compareWithStrtoul(" -");
    compareWithStrtoul("\t+");
}

void NumUtilWhiteBoxTests::testStrtoint64MatchesStrtoll()
{
    constexpr std::string_view testname = __func__;

    // Helper lambda to compare NumUtil::strtoll with std::strtoll.
    auto compareWithStrtoll = [&](const std::string& str)
    {
        TST_LOG("Converting [" << str << ']');

        char* endptr = nullptr;
        errno = 0;
        const long long stdResult = std::strtoll(str.c_str(), &endptr, 10);
        const int stdErrno = errno;
        const std::size_t stdOffset = endptr - str.c_str();
        TST_LOG("Std converted [" << str << "] to [" << stdResult << "] with errno [" << stdErrno
                                  << "] and offset [" << stdOffset << ']');

        std::size_t numUtilOffset = 0;
        const auto [numUtilResult, numUtilState] = NumUtil::parseStrToInt64(str, numUtilOffset);
        TST_LOG("Num converted [" << str << "] to [" << numUtilResult << "] with state ["
                                  << static_cast<int>(numUtilState) << "] and offset ["
                                  << numUtilOffset << ']');

        // Compare results.
        if (stdErrno == ERANGE)
        {
            // Overflow case.
            LOK_ASSERT_CTX(numUtilState == NumUtil::StrToState::Overflow, str);
            // For overflow, NumUtil should return INT64_MIN or INT64_MAX.
            LOK_ASSERT_CTX(numUtilResult == std::numeric_limits<std::int64_t>::min() ||
                               numUtilResult == std::numeric_limits<std::int64_t>::max(),
                           str);
        }
        else if (endptr == str.c_str())
        {
            // No conversion - NumUtil returns 0 for empty/invalid input.
            LOK_ASSERT_EQUAL_CTX(static_cast<std::int64_t>(0), numUtilResult, str);
        }
        else
        {
            // Valid conversion - results should match.
            LOK_ASSERT_EQUAL_CTX(static_cast<std::int64_t>(stdResult), numUtilResult, str);
        }

        // Offset should match endptr position.
        LOK_ASSERT_EQUAL_CTX(stdOffset, numUtilOffset, str);
    };

    // Test basic positive int64_t numbers.
    compareWithStrtoll("0");
    compareWithStrtoll("1");
    compareWithStrtoll("42");
    compareWithStrtoll("123456789");
    compareWithStrtoll("1000000000");

    // Test negative int64_t numbers.
    compareWithStrtoll("-1");
    compareWithStrtoll("-42");
    compareWithStrtoll("-123456789");
    compareWithStrtoll("-1000000000");

    // Test large int64_t numbers.
    compareWithStrtoll("9223372036854775806"); // INT64_MAX - 1.
    compareWithStrtoll("9223372036854775807"); // INT64_MAX.
    compareWithStrtoll("-9223372036854775807"); // INT64_MIN + 1.
    compareWithStrtoll("-9223372036854775808"); // INT64_MIN.

    // Test int64_t with leading whitespace.
    compareWithStrtoll("  123456789");
    compareWithStrtoll("\t-987654321");

    // Test int64_t with leading zeros.
    compareWithStrtoll("00123456789");
    compareWithStrtoll("-00987654321");

    // Test int64_t with plus sign.
    compareWithStrtoll("+123456789");
    compareWithStrtoll("+0");

    // Test int64_t overflow cases (should set errno to ERANGE).
    compareWithStrtoll("9223372036854775808"); // INT64_MAX + 1.
    compareWithStrtoll("-9223372036854775809"); // INT64_MIN - 1.
    compareWithStrtoll("99999999999999999999");

    // Test int64_t empty and invalid strings.
    compareWithStrtoll("");
    compareWithStrtoll("   ");
    compareWithStrtoll("abc");
    compareWithStrtoll("-");
    compareWithStrtoll("+");

    // Test numbers at various magnitudes for int64_t.
    compareWithStrtoll("1000000000000"); // 1 trillion.
    compareWithStrtoll("1000000000000000"); // 1 quadrillion.
    compareWithStrtoll("-1000000000000"); // -1 trillion.
    compareWithStrtoll("-1000000000000000"); // -1 quadrillion.

    // Test whitespace + bare sign.
    compareWithStrtoll(" -");
    compareWithStrtoll("\t+");

    // Test sign followed by non-digit.
    compareWithStrtoll("-a");

    // Test double signs.
    compareWithStrtoll("--1");

    // Test trailing text (verifies offset matches endptr).
    compareWithStrtoll("123abc");
}

void NumUtilWhiteBoxTests::testStrtouint64MatchesStrtoull()
{
    constexpr std::string_view testname = __func__;

    // Helper lambda to compare NumUtil::strtoull with std::strtoull.
    auto compareWithStrtoull = [&](const std::string& str)
    {
        TST_LOG("Converting [" << str << ']');

        char* endptr = nullptr;
        errno = 0;
        const unsigned long long stdResult = std::strtoull(str.c_str(), &endptr, 10);
        const int stdErrno = errno;
        const std::size_t stdOffset = endptr - str.c_str();
        TST_LOG("Std converted [" << str << "] to [" << stdResult << "] with errno [" << stdErrno
                                  << "] and offset [" << stdOffset << ']');

        std::size_t numUtilOffset = 0;
        const auto [numUtilResult, numUtilState] = NumUtil::parseStrToUint64(str, numUtilOffset);
        TST_LOG("Num converted [" << str << "] to [" << numUtilResult << "] with state ["
                                  << static_cast<int>(numUtilState) << "] and offset ["
                                  << numUtilOffset << ']');

        // Compare results.
        if (stdErrno == ERANGE)
        {
            // Overflow case.
            LOK_ASSERT_CTX(numUtilState == NumUtil::StrToState::Overflow, str);
            // For overflow, NumUtil should return UINT64_MAX.
            LOK_ASSERT_EQUAL_CTX(std::numeric_limits<std::uint64_t>::max(), numUtilResult, str);
        }
        else if (endptr == str.c_str())
        {
            // No conversion - NumUtil returns 0 for empty/invalid input.
            LOK_ASSERT_EQUAL_CTX(static_cast<std::uint64_t>(0), numUtilResult, str);
        }
        else
        {
            // Valid conversion - results should match.
            LOK_ASSERT_EQUAL_CTX(static_cast<std::uint64_t>(stdResult), numUtilResult, str);
        }

        // Offset should match endptr position.
        LOK_ASSERT_EQUAL_CTX(stdOffset, numUtilOffset, str);
    };

    // Test basic positive uint64_t numbers.
    compareWithStrtoull("0");
    compareWithStrtoull("1");
    compareWithStrtoull("42");
    compareWithStrtoull("123456789");
    compareWithStrtoull("1000000000");

    // Test large uint64_t numbers.
    compareWithStrtoull("18446744073709551614"); // UINT64_MAX - 1.
    compareWithStrtoull("18446744073709551615"); // UINT64_MAX.

    // Test uint64_t with leading whitespace.
    compareWithStrtoull("  123456789");
    compareWithStrtoull("\t987654321");

    // Test uint64_t with leading zeros.
    compareWithStrtoull("00123456789");
    compareWithStrtoull("00987654321");

    // Test uint64_t with plus sign.
    compareWithStrtoull("+123456789");
    compareWithStrtoull("+0");

    // Test uint64_t overflow cases (should set errno to ERANGE).
    compareWithStrtoull("18446744073709551616"); // UINT64_MAX + 1.
    compareWithStrtoull("99999999999999999999");

    // Test uint64_t empty and invalid strings.
    compareWithStrtoull("");
    compareWithStrtoull("   ");
    compareWithStrtoull("abc");

    // Test numbers at various magnitudes for uint64_t.
    compareWithStrtoull("1000000000000"); // 1 trillion.
    compareWithStrtoull("1000000000000000"); // 1 quadrillion.
    compareWithStrtoull("10000000000000000000"); // 10 quintillion.

    // Test bare signs.
    compareWithStrtoull("-");
    compareWithStrtoull("+");

    // Test negative for unsigned.
    compareWithStrtoull("-1");

    // Test whitespace + bare sign.
    compareWithStrtoull(" -");
}

void NumUtilWhiteBoxTests::testParseStrTo()
{
    constexpr std::string_view testname = __func__;

    // Test basic offset parameter usage.
    {
        const std::string str = "123";
        std::size_t offset = 0;
        const auto [result, state] = NumUtil::parseStrToInt32(str, offset);
        LOK_ASSERT_EQUAL_CTX(123, result, str);
        LOK_ASSERT_EQUAL_CTX(std::size_t(3), offset, str); // Offset should be at end.
    }

    // Test offset starts in middle of string.
    {
        const std::string str = "abc123def";
        std::size_t offset = 3; // Start at '1'.
        const auto [result, state] = NumUtil::parseStrToInt32(str, offset);
        LOK_ASSERT_EQUAL_CTX(123, result, str);
        LOK_ASSERT_EQUAL_CTX(std::size_t(6), offset, str); // Should stop at 'd'.
    }

    // Test offset with negative number.
    {
        const std::string str = "xyz-456end";
        std::size_t offset = 3; // Start at '-'.
        const auto [result, state] = NumUtil::parseStrToInt32(str, offset);
        LOK_ASSERT_EQUAL_CTX(-456, result, str);
        LOK_ASSERT_EQUAL_CTX(std::size_t(7), offset, str); // Should stop at 'e'.
    }

    // Test offset skips leading whitespace.
    {
        const std::string str = "  789";
        std::size_t offset = 0;
        const auto [result, state] = NumUtil::parseStrToInt32(str, offset);
        LOK_ASSERT_EQUAL_CTX(789, result, str);
        LOK_ASSERT_EQUAL_CTX(std::size_t(5), offset, str); // Should be at end.
    }

    // Test offset with whitespace after offset position.
    {
        const std::string str = "abc  456xyz";
        std::size_t offset = 3; // Start at first space.
        const auto [result, state] = NumUtil::parseStrToInt32(str, offset);
        LOK_ASSERT_EQUAL_CTX(456, result, str);
        LOK_ASSERT_EQUAL_CTX(std::size_t(8), offset, str); // Should stop at 'x'.
    }

    // Test multiple numbers parsed sequentially.
    {
        const std::string str = "11 22 33 44 55";
        std::size_t offset = 0;

        LOK_ASSERT_EQUAL_CTX(11, NumUtil::parseStrToInt32(str, offset).first, str);
        LOK_ASSERT_EQUAL_CTX(std::size_t(2), offset, str); // At space after "11".

        offset++; // Skip space.
        LOK_ASSERT_EQUAL_CTX(22, NumUtil::parseStrToInt32(str, offset).first, str);
        LOK_ASSERT_EQUAL_CTX(std::size_t(5), offset, str);

        offset++; // Skip space.
        LOK_ASSERT_EQUAL_CTX(33, NumUtil::parseStrToInt32(str, offset).first, str);
        LOK_ASSERT_EQUAL_CTX(std::size_t(8), offset, str);

        offset++; // Skip space.
        LOK_ASSERT_EQUAL_CTX(44, NumUtil::parseStrToInt32(str, offset).first, str);
        LOK_ASSERT_EQUAL_CTX(std::size_t(11), offset, str);

        offset++; // Skip space.
        LOK_ASSERT_EQUAL_CTX(55, NumUtil::parseStrToInt32(str, offset).first, str);
        LOK_ASSERT_EQUAL_CTX(std::size_t(14), offset, str);
    }

    // Test offset with plus sign.
    {
        const std::string str = "data+42more";
        std::size_t offset = 4; // Start at '+'.
        const auto [result, state] = NumUtil::parseStrToInt32(str, offset);
        LOK_ASSERT_EQUAL_CTX(42, result, str);
        LOK_ASSERT_EQUAL_CTX(std::size_t(7), offset, str); // Should stop at 'm'.
    }

    // Test offset with zero.
    {
        const std::string str = "text0more";
        std::size_t offset = 4; // Start at '0'.
        const auto [result, state] = NumUtil::parseStrToInt32(str, offset);
        LOK_ASSERT_EQUAL_CTX(0, result, str);
        LOK_ASSERT_EQUAL_CTX(std::size_t(5), offset, str); // Should stop at 'm'.
    }

    // Test offset with leading zeros.
    {
        const std::string str = "00123";
        std::size_t offset = 0;
        const auto [result, state] = NumUtil::parseStrToInt32(str, offset);
        LOK_ASSERT_EQUAL_CTX(123, result, str);
        LOK_ASSERT_EQUAL_CTX(std::size_t(5), offset, str);
    }

    // Test offset stops at non-digit.
    {
        const std::string str = "start123abc";
        std::size_t offset = 5; // Start at '1'.
        const auto [result, state] = NumUtil::parseStrToInt32(str, offset);
        LOK_ASSERT_EQUAL_CTX(123, result, str);
        LOK_ASSERT_EQUAL_CTX(std::size_t(8), offset, str); // Should stop at 'a'.
    }

    // Test offset with int64_t.
    {
        const std::string str = "9223372036854775807end";
        std::size_t offset = 0;
        const auto [result, state] = NumUtil::parseStrToInt64(str, offset);
        LOK_ASSERT_EQUAL_CTX(std::numeric_limits<std::int64_t>::max(), result, str);
        LOK_ASSERT_EQUAL_CTX(std::size_t(19), offset, str);
    }

    // Test offset with negative int64_t.
    {
        const std::string str = "xxx-9223372036854775808yyy";
        std::size_t offset = 3;
        const auto [result, state] = NumUtil::parseStrToInt64(str, offset);
        LOK_ASSERT_EQUAL_CTX(std::numeric_limits<std::int64_t>::min(), result, str);
        LOK_ASSERT_EQUAL_CTX(std::size_t(23), offset, str);
    }

    // Test offset with uint64_t.
    {
        const std::string str = "data18446744073709551615xyz";
        std::size_t offset = 4;
        const auto [result, state] = NumUtil::parseStrToUint64(str, offset);
        LOK_ASSERT_EQUAL_CTX(std::numeric_limits<std::uint64_t>::max(), result, str);
        LOK_ASSERT_EQUAL_CTX(std::size_t(24), offset, str);
    }

    // Test offset with tab whitespace.
    {
        const std::string str = "abc\t\t999xyz";
        std::size_t offset = 3;
        const auto [result, state] = NumUtil::parseStrToInt32(str, offset);
        LOK_ASSERT_EQUAL_CTX(999, result, str);
        LOK_ASSERT_EQUAL_CTX(std::size_t(8), offset, str);
    }

    // Test offset at boundary - INT32_MAX.
    {
        const std::string str = "prefix2147483647suffix";
        std::size_t offset = 6;
        const auto [result, state] = NumUtil::parseStrToInt32(str, offset);
        LOK_ASSERT_EQUAL_CTX(std::numeric_limits<std::int32_t>::max(), result, str);
        LOK_ASSERT_EQUAL_CTX(std::size_t(16), offset, str);
    }

    // Test offset at boundary - INT32_MIN.
    {
        const std::string str = "start-2147483648end";
        std::size_t offset = 5;
        const auto [result, state] = NumUtil::parseStrToInt32(str, offset);
        LOK_ASSERT_EQUAL_CTX(std::numeric_limits<std::int32_t>::min(), result, str);
        LOK_ASSERT_EQUAL_CTX(std::size_t(16), offset, str);
    }

    // Test offset with overflowing int32_t.
    {
        const std::string str = "9999999999";
        std::size_t offset = 0;
        const auto [result, state] = NumUtil::parseStrToInt32(str, offset);
        LOK_ASSERT_EQUAL_CTX(std::numeric_limits<std::int32_t>::max(), result, str);
        LOK_ASSERT_EQUAL_CTX(std::size_t(10), offset, str);
    }

    // Test offset with overflowing uint32_t.
    {
        const std::string str = "9999999999";
        std::size_t offset = 0;
        const auto [result, state] = NumUtil::parseStrToUint32(str, offset);
        LOK_ASSERT_EQUAL_CTX(std::numeric_limits<std::uint32_t>::max(), result, str);
        LOK_ASSERT_EQUAL_CTX(std::size_t(10), offset, str);
    }

    // Test offset with uint32_t.
    {
        const std::string str = "value4294967295end";
        std::size_t offset = 5;
        const auto [result, state] = NumUtil::parseStrToUint32(str, offset);
        LOK_ASSERT_EQUAL_CTX(std::numeric_limits<std::uint32_t>::max(), result, str);
        LOK_ASSERT_EQUAL_CTX(std::size_t(15), offset, str);
    }

    // Test offset with -0.
    {
        const std::string str = "data-0more";
        std::size_t offset = 4;
        const auto [result, state] = NumUtil::parseStrToInt32(str, offset);
        LOK_ASSERT_EQUAL_CTX(0, result, str);
        LOK_ASSERT_EQUAL_CTX(std::size_t(6), offset, str);
    }

    // Test offset with +0.
    {
        const std::string str = "text+0end";
        std::size_t offset = 4;
        const auto [result, state] = NumUtil::parseStrToInt32(str, offset);
        LOK_ASSERT_EQUAL_CTX(0, result, str);
        LOK_ASSERT_EQUAL_CTX(std::size_t(6), offset, str);
    }

    // Test offset returns 0 for empty string.
    {
        const std::string str;
        std::size_t offset = 0;
        const auto [result, state] = NumUtil::parseStrToInt32(str, offset);
        LOK_ASSERT_EQUAL_CTX(0, result, str);
        LOK_ASSERT_EQUAL_CTX(std::size_t(0), offset, str);
    }

    // Test offset at end of string.
    {
        const std::string str = "test";
        std::size_t offset = 4; // At end.
        const auto [result, state] = NumUtil::parseStrToInt32(str, offset);
        LOK_ASSERT_EQUAL_CTX(0, result, str);
        LOK_ASSERT_EQUAL_CTX(std::size_t(4), offset, str); // Should stay at end.
    }

    // Test offset parsing comma-separated values.
    {
        const std::string str = "100,200,300,400";
        std::size_t offset = 0;

        LOK_ASSERT_EQUAL_CTX(100, NumUtil::parseStrToInt32(str, offset).first, str);
        offset++; // Skip comma.
        LOK_ASSERT_EQUAL_CTX(200, NumUtil::parseStrToInt32(str, offset).first, str);
        offset++; // Skip comma.
        LOK_ASSERT_EQUAL_CTX(300, NumUtil::parseStrToInt32(str, offset).first, str);
        offset++; // Skip comma.
        LOK_ASSERT_EQUAL_CTX(400, NumUtil::parseStrToInt32(str, offset).first, str);
    }

    // Test offset with negative numbers in sequence.
    {
        const std::string str = "-10 -20 -30";
        std::size_t offset = 0;

        LOK_ASSERT_EQUAL_CTX(-10, NumUtil::parseStrToInt32(str, offset).first, str);
        offset++; // Skip space.
        LOK_ASSERT_EQUAL_CTX(-20, NumUtil::parseStrToInt32(str, offset).first, str);
        offset++; // Skip space.
        LOK_ASSERT_EQUAL_CTX(-30, NumUtil::parseStrToInt32(str, offset).first, str);
    }

    // Test offset with overflow returns Overflow state.
    {
        const std::string str = "data2147483648more"; // INT32_MAX + 1.
        std::size_t offset = 4;
        const auto [result, state] = NumUtil::parseStrToInt32(str, offset);
        LOK_ASSERT_CTX(state == NumUtil::StrToState::Overflow, str);
        LOK_ASSERT_EQUAL_CTX(std::numeric_limits<int>::max(), result, str);
    }

    // Test offset with underflow returns Overflow state.
    {
        const std::string str = "x-2147483649y"; // INT32_MIN - 1.
        std::size_t offset = 1;
        const auto [result, state] = NumUtil::parseStrToInt32(str, offset);
        LOK_ASSERT_CTX(state == NumUtil::StrToState::Overflow, str);
        LOK_ASSERT_EQUAL_CTX(std::numeric_limits<int>::min(), result, str);
    }

    // Test offset parsing continues until non-digit.
    {
        const std::string str = "12345abcde67890";
        std::size_t offset = 0;
        const auto [result, state] = NumUtil::parseStrToInt32(str, offset);
        LOK_ASSERT_EQUAL_CTX(12345, result, str);
        LOK_ASSERT_EQUAL_CTX(std::size_t(5), offset, str); // Should stop at 'a'.
    }

    // Test offset with mixed whitespace.
    {
        const std::string str = " \t \t 999 ";
        std::size_t offset = 0;
        const auto [result, state] = NumUtil::parseStrToInt32(str, offset);
        LOK_ASSERT_EQUAL_CTX(999, result, str);
        LOK_ASSERT_EQUAL_CTX(std::size_t(8), offset, str);
    }

    // Test offset: sign at end of string with non-zero offset (exercises bug fix).
    {
        const std::string str = "x-";
        std::size_t offset = 1;
        const auto [result, state] = NumUtil::parseStrToInt32(str, offset);
        LOK_ASSERT_EQUAL_CTX(0, result, str);
        LOK_ASSERT_EQUAL_CTX(std::size_t(1), offset, str); // Offset restored.
    }
    {
        const std::string str = "ab+";
        std::size_t offset = 2;
        const auto [result, state] = NumUtil::parseStrToInt32(str, offset);
        LOK_ASSERT_EQUAL_CTX(0, result, str);
        LOK_ASSERT_EQUAL_CTX(std::size_t(2), offset, str); // Offset restored.
    }

    // Test offset restoration on failure from non-zero offset.
    {
        const std::string str = "abcxyz";
        std::size_t offset = 3; // Start at 'x' — not a digit.
        const auto [result, state] = NumUtil::parseStrToInt32(str, offset);
        LOK_ASSERT_EQUAL_CTX(0, result, str);
        LOK_ASSERT_EQUAL_CTX(std::size_t(3), offset, str); // Offset restored.
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(NumUtilWhiteBoxTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
