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
 * Unit tests for hex utilities.
 */

#include <config.h>

#include <test/lokassert.hpp>

#include <common/HexUtil.hpp>
#include <common/Util.hpp>

#include <cppunit/extensions/HelperMacros.h>

#include <cstdint>
#include <iomanip>
#include <sstream>

/// HexUtil unit-tests.
class HexUtilTests : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(HexUtilTests);

    CPPUNIT_TEST(testStringifyHexLine);
    CPPUNIT_TEST(testHexify);
    CPPUNIT_TEST(testBytesToHex);
    CPPUNIT_TEST(testHexDigitFromChar);
    CPPUNIT_TEST(testNumberToHex);
    CPPUNIT_TEST(testEncodeIdKnownValues);
    CPPUNIT_TEST(testEncodeIdOstream);
    CPPUNIT_TEST(testEncodeIdPadChar);
    CPPUNIT_TEST(testEncodeIdZeroSize);
    CPPUNIT_TEST(testDecodeIdKnownValues);
    CPPUNIT_TEST(testEncodeDecodeRoundTrip);

    CPPUNIT_TEST_SUITE_END();

    void testStringifyHexLine();
    void testHexify();
    void testBytesToHex();
    void testHexDigitFromChar();
    void testNumberToHex();
    void testEncodeIdKnownValues();
    void testEncodeIdOstream();
    void testEncodeIdPadChar();
    void testEncodeIdZeroSize();
    void testDecodeIdKnownValues();
    void testEncodeDecodeRoundTrip();
};

void HexUtilTests::testStringifyHexLine()
{
    constexpr std::string_view testname = __func__;

    std::string test("hello here\ntest");
    std::string result1("68 65 6C 6C 6F 20 68 65  72 65 0A 74 65 73 74"
                        "                                                       "
                        "| hello here.test                 ");
    std::string result2("68 65 72 65 0A 74  | here.t");
    LOK_ASSERT_EQUAL(result1, HexUtil::stringifyHexLine(test, 0));
    LOK_ASSERT_EQUAL(result2, HexUtil::stringifyHexLine(test, 6, 6));
}

void HexUtilTests::testHexify()
{
    constexpr std::string_view testname = __func__;

    const std::string s1 = "some ascii text with !@#$%^&*()_+/-\\|";
    const auto hex = HexUtil::dataToHexString(s1, 0, s1.size());
    std::string decoded;
    LOK_ASSERT(HexUtil::dataFromHexString(hex, decoded));
    LOK_ASSERT_EQUAL(s1, decoded);

    for (std::size_t randStrLen = 1; randStrLen < 129; ++randStrLen)
    {
        const auto s2 = Util::rng::getBytes(randStrLen);
        LOK_ASSERT_EQUAL(randStrLen, s2.size());
        const auto hex2 = HexUtil::dataToHexString(s2, 0, s2.size());
        LOK_ASSERT_EQUAL(randStrLen * 2, hex2.size());
        std::vector<char> decoded2;
        LOK_ASSERT(HexUtil::dataFromHexString(hex2, decoded2));
        LOK_ASSERT_EQUAL(randStrLen, decoded2.size());
        LOK_ASSERT_EQUAL(Util::toString(s2), Util::toString(decoded2));
    }
}

void HexUtilTests::testBytesToHex()
{
    constexpr std::string_view testname = __func__;

    {
        const std::string d("Some text");
        const std::string hex = HexUtil::bytesToHexString(d);
        const std::string s = HexUtil::hexStringToBytes(hex);
        LOK_ASSERT_EQUAL(d, s);
    }
}

static std::string hexifyStd(const std::uint64_t number, int width, std::size_t size = INTMAX_MAX)
{
    std::ostringstream oss;
    oss << std::hex << std::setw(width) << std::setfill('0') << number;
    std::string str = oss.str();
    if (size < str.size())
    {
        return str.substr(str.size() - size);
    }

    return str;
}

void HexUtilTests::testNumberToHex()
{
    constexpr std::string_view testname = __func__;

    for (int width = 0; width < 33; ++width)
    {
        std::uint64_t number = 0;
        LOK_TRACE("Number: " << std::hex << number << std::dec << " (shift: 0"
                             << "), width: " << width);
        LOK_ASSERT_EQUAL_STR(hexifyStd(number, width), HexUtil::encodeId(number, width));
        LOK_ASSERT_EQUAL_STR(hexifyStd(~number, width), HexUtil::encodeId(~number, width));

        for (int shift = 0; shift < 64; ++shift)
        {
            number = 1UL << shift;
            LOK_TRACE("Number: " << std::hex << number << std::dec << " (shift: " << shift
                                 << "), width: " << width);
            LOK_ASSERT_EQUAL_STR(hexifyStd(number, width), HexUtil::encodeId(number, width));
            LOK_ASSERT_EQUAL_STR(hexifyStd(~number, width), HexUtil::encodeId(~number, width));
        }
    }

    char buffer[32];
    for (ssize_t sz = sizeof(buffer); sz >= 0; --sz)
    {
        std::size_t size = static_cast<std::size_t>(sz);
        for (int width = 0; width < 33; ++width)
        {
            std::uint64_t number = 0;
            LOK_TRACE("Number: " << std::hex << number << std::dec << " (shift: 0"
                                 << "), width: " << width << ", size: " << size);
            LOK_ASSERT_EQUAL_STR(hexifyStd(number, width, size),
                                 HexUtil::encodeId(buffer, size, number, width));
            LOK_ASSERT_EQUAL_STR(hexifyStd(~number, width, size),
                                 HexUtil::encodeId(buffer, size, ~number, width));

            for (int shift = 0; shift < 64; ++shift)
            {
                number = 1UL << shift;
                LOK_TRACE("Number: " << std::hex << number << std::dec << " (shift: " << shift
                                     << "), width: " << width << ", size: " << size);
                LOK_ASSERT_EQUAL_STR(hexifyStd(number, width, size),
                                     HexUtil::encodeId(buffer, size, number, width));
                LOK_ASSERT_EQUAL_STR(hexifyStd(~number, width, size),
                                     HexUtil::encodeId(buffer, size, ~number, width));
            }
        }
    }
}

void HexUtilTests::testHexDigitFromChar()
{
    constexpr std::string_view testname = __func__;

    for (int i = 0; i <= 9; ++i)
        LOK_ASSERT_EQUAL(i, HexUtil::hexDigitFromChar(static_cast<char>('0' + i)));
    for (int i = 0; i < 6; ++i)
    {
        LOK_ASSERT_EQUAL(10 + i, HexUtil::hexDigitFromChar(static_cast<char>('a' + i)));
        LOK_ASSERT_EQUAL(10 + i, HexUtil::hexDigitFromChar(static_cast<char>('A' + i)));
    }

    // Non-hex chars return -1.
    for (char c : { ' ', '\0', '/', ':', '@', 'G', '`', 'g', 'z' })
        LOK_ASSERT_EQUAL(-1, HexUtil::hexDigitFromChar(c));

    // High-bit bytes (interpreted via unsigned char) also return -1.
    LOK_ASSERT_EQUAL(-1, HexUtil::hexDigitFromChar(static_cast<char>(0x80)));
    LOK_ASSERT_EQUAL(-1, HexUtil::hexDigitFromChar(static_cast<char>(0xff)));
}

void HexUtilTests::testEncodeIdKnownValues()
{
    constexpr std::string_view testname = __func__;

    // Default width is 5, default pad is '0'.
    LOK_ASSERT_EQUAL_STR("00000", HexUtil::encodeId(0));
    LOK_ASSERT_EQUAL_STR("00001", HexUtil::encodeId(1));
    LOK_ASSERT_EQUAL_STR("0000a", HexUtil::encodeId(0xa));
    LOK_ASSERT_EQUAL_STR("000ff", HexUtil::encodeId(0xff));
    LOK_ASSERT_EQUAL_STR("12345", HexUtil::encodeId(0x12345));

    // Numbers wider than the requested width: width is a *minimum*, not a cap.
    LOK_ASSERT_EQUAL_STR("123456", HexUtil::encodeId(0x123456));
    LOK_ASSERT_EQUAL_STR("ffffffffffffffff", HexUtil::encodeId(UINT64_MAX));

    // Width 0 means "no padding".
    LOK_ASSERT_EQUAL_STR("0", HexUtil::encodeId(0, 0));
    LOK_ASSERT_EQUAL_STR("ff", HexUtil::encodeId(0xff, 0));

    // Lowercase hex output.
    LOK_ASSERT_EQUAL_STR("deadbeef", HexUtil::encodeId(0xDEADBEEF, 0));
}

void HexUtilTests::testEncodeIdOstream()
{
    constexpr std::string_view testname = __func__;

    // The stream overload appends and returns the same stream.
    std::ostringstream oss;
    oss << "prefix-";
    std::ostringstream& ret = HexUtil::encodeId(oss, 0x42, 4);
    LOK_ASSERT_EQUAL_STR(std::string("prefix-0042"), oss.str());
    LOK_ASSERT(&ret == &oss);

    // Stream and string overloads agree.
    for (std::uint64_t n : { std::uint64_t(0), std::uint64_t(1), std::uint64_t(0xabc),
                             std::uint64_t(0x123456789abcdef0), UINT64_MAX })
    {
        for (int width : { 0, 1, 5, 16 })
        {
            std::ostringstream o;
            HexUtil::encodeId(o, n, width);
            LOK_ASSERT_EQUAL_STR(HexUtil::encodeId(n, width), o.str());
        }
    }
}

void HexUtilTests::testEncodeIdPadChar()
{
    constexpr std::string_view testname = __func__;

    LOK_ASSERT_EQUAL_STR("    a", HexUtil::encodeId(0xa, 5, ' '));
    LOK_ASSERT_EQUAL_STR("xxx12", HexUtil::encodeId(0x12, 5, 'x'));
    // No padding needed: pad char is irrelevant.
    LOK_ASSERT_EQUAL_STR("12345", HexUtil::encodeId(0x12345, 5, ' '));
    LOK_ASSERT_EQUAL_STR("123456", HexUtil::encodeId(0x123456, 5, ' '));
}

void HexUtilTests::testEncodeIdZeroSize()
{
    constexpr std::string_view testname = __func__;

    // A zero-sized buffer must produce an empty result without writing.
    char buffer[1] = { '\xCC' };
    const auto out = HexUtil::encodeId(buffer, 0, 0x12345, 5);
    LOK_ASSERT_EQUAL(std::size_t(0), out.size());
    LOK_ASSERT_EQUAL(char('\xCC'), buffer[0]);
}

void HexUtilTests::testDecodeIdKnownValues()
{
    constexpr std::string_view testname = __func__;

    LOK_ASSERT_EQUAL(std::uint64_t(0), HexUtil::decodeId(""));
    LOK_ASSERT_EQUAL(std::uint64_t(0), HexUtil::decodeId("0"));
    LOK_ASSERT_EQUAL(std::uint64_t(0), HexUtil::decodeId("00000"));
    LOK_ASSERT_EQUAL(std::uint64_t(1), HexUtil::decodeId("1"));
    LOK_ASSERT_EQUAL(std::uint64_t(1), HexUtil::decodeId("00001"));
    LOK_ASSERT_EQUAL(std::uint64_t(0xff), HexUtil::decodeId("ff"));
    LOK_ASSERT_EQUAL(std::uint64_t(0xff), HexUtil::decodeId("FF"));
    LOK_ASSERT_EQUAL(std::uint64_t(0xdeadbeef), HexUtil::decodeId("deadbeef"));
    LOK_ASSERT_EQUAL(std::uint64_t(0xdeadbeef), HexUtil::decodeId("DEADBEEF"));
    LOK_ASSERT_EQUAL(UINT64_MAX, HexUtil::decodeId("ffffffffffffffff"));

    // Trailing non-hex chars: parse stops at the first invalid char and the
    // already-parsed prefix is returned.
    LOK_ASSERT_EQUAL(std::uint64_t(0xff), HexUtil::decodeId("ff_garbage"));
    LOK_ASSERT_EQUAL(std::uint64_t(0xabc), HexUtil::decodeId("abc xyz"));

    // No parseable hex prefix: returns 0.
    LOK_ASSERT_EQUAL(std::uint64_t(0), HexUtil::decodeId("zzz"));
    LOK_ASSERT_EQUAL(std::uint64_t(0), HexUtil::decodeId(" 1"));

    // Overflow (>16 hex digits): from_chars reports out-of-range and the
    // returned value stays at 0 (the initial default).
    LOK_ASSERT_EQUAL(std::uint64_t(0), HexUtil::decodeId("1ffffffffffffffff"));
}

void HexUtilTests::testEncodeDecodeRoundTrip()
{
    constexpr std::string_view testname = __func__;

    // Sweep specific values and bit positions.
    const std::uint64_t base[] = { 0, 1, 0xa, 0xff, 0x100, 0x12345, UINT64_MAX };
    for (std::uint64_t n : base)
    {
        LOK_TRACE("Round-trip: 0x" << std::hex << n);
        LOK_ASSERT_EQUAL(n, HexUtil::decodeId(HexUtil::encodeId(n, 0)));
        LOK_ASSERT_EQUAL(n, HexUtil::decodeId(HexUtil::encodeId(n, 16)));
    }

    for (int shift = 0; shift < 64; ++shift)
    {
        const std::uint64_t n = std::uint64_t(1) << shift;
        LOK_TRACE("Round-trip shift " << shift << ": 0x" << std::hex << n);
        LOK_ASSERT_EQUAL(n, HexUtil::decodeId(HexUtil::encodeId(n, 0)));
        LOK_ASSERT_EQUAL(n, HexUtil::decodeId(HexUtil::encodeId(n, 16)));
        const std::uint64_t inv = ~n;
        LOK_TRACE("Round-trip ~shift " << shift << ": 0x" << std::hex << inv);
        LOK_ASSERT_EQUAL(inv, HexUtil::decodeId(HexUtil::encodeId(inv, 0)));
        LOK_ASSERT_EQUAL(inv, HexUtil::decodeId(HexUtil::encodeId(inv, 16)));
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(HexUtilTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
