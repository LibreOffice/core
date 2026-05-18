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
    CPPUNIT_TEST(testNumberToHex);

    CPPUNIT_TEST_SUITE_END();

    void testStringifyHexLine();
    void testHexify();
    void testBytesToHex();
    void testNumberToHex();
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
        LOK_ASSERT_EQUAL_STR(hexifyStd(number, width), Util::encodeId(number, width));
        LOK_ASSERT_EQUAL_STR(hexifyStd(~number, width), Util::encodeId(~number, width));

        for (int shift = 0; shift < 64; ++shift)
        {
            number = 1UL << shift;
            LOK_TRACE("Number: " << std::hex << number << std::dec << " (shift: " << shift
                                 << "), width: " << width);
            LOK_ASSERT_EQUAL_STR(hexifyStd(number, width), Util::encodeId(number, width));
            LOK_ASSERT_EQUAL_STR(hexifyStd(~number, width), Util::encodeId(~number, width));
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
                                 Util::encodeId(buffer, size, number, width));
            LOK_ASSERT_EQUAL_STR(hexifyStd(~number, width, size),
                                 Util::encodeId(buffer, size, ~number, width));

            for (int shift = 0; shift < 64; ++shift)
            {
                number = 1UL << shift;
                LOK_TRACE("Number: " << std::hex << number << std::dec << " (shift: " << shift
                                     << "), width: " << width << ", size: " << size);
                LOK_ASSERT_EQUAL_STR(hexifyStd(number, width, size),
                                     Util::encodeId(buffer, size, number, width));
                LOK_ASSERT_EQUAL_STR(hexifyStd(~number, width, size),
                                     Util::encodeId(buffer, size, ~number, width));
            }
        }
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(HexUtilTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
