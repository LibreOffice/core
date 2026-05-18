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
 * Unit test for utility functions including character conversion.
 */

#include <config.h>

#include <test/lokassert.hpp>

#include <common/CharacterConverter.hpp>
#include <common/Util.hpp>

#include <cppunit/extensions/HelperMacros.h>

#include <sstream>

/// Util unit-tests.
class UtilTests : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(UtilTests);

    CPPUNIT_TEST(testCharacterConverter);
#if ENABLE_DEBUG
    CPPUNIT_TEST(testUtf8);
#endif
    CPPUNIT_TEST(testEliminatePrefix);
    CPPUNIT_TEST(testStreamMatch);
    CPPUNIT_TEST(testBase64Encode);
    CPPUNIT_TEST(testBase64Decode);

    CPPUNIT_TEST_SUITE_END();

    void testCharacterConverter();
    void testUtf8();
    void testEliminatePrefix();
    void testStreamMatch();
    void testBase64Encode();
    void testBase64Decode();
};

void UtilTests::testCharacterConverter()
{
    constexpr std::string_view testname = __func__;

    const std::string utf8 = "Ḽơᶉëᶆ ȋṕšᶙṁ ḍỡḽǭᵳ ʂǐť";
#ifndef __APPLE__
    const std::string utf7 = "+HjwBoR2JAOsdhg +AgseVQFhHZkeQQ +Hg0e4R49Ae0dcw +AoIB0AFl-";
#else
    // The macOS iconv gives slightly different results
    const std::string utf7 = "+HjwBoR2JAOsdhg +AgseVQFhHZkeQQ +Hg0e4R49Ae0dcw +AoIB0AFl";
#endif
    {
        Util::CharacterConverter utf8_to_7("UTF-8", "UTF-7");
        LOK_ASSERT_EQUAL_STR(utf7, utf8_to_7.convert(utf8));
        LOK_ASSERT_EQUAL_STR(utf7, utf8_to_7.convert(utf8)); // Convert again.

        Util::CharacterConverter utf7_to_8("UTF-7", "UTF-8");
        LOK_ASSERT_EQUAL_STR(utf8, utf7_to_8.convert(utf7));
        LOK_ASSERT_EQUAL_STR(utf8, utf7_to_8.convert(utf7)); // Convert again.
    }

    {
        const std::string utf8l =
            R"xxx(ăѣ𝔠ծềſģȟᎥ𝒋ǩľḿꞑȯ𝘱𝑞𝗋𝘴ȶ𝞄𝜈ψ𝒙𝘆𝚣1234567890!@#$%^&*()-_=+[{]};:'",<.>/?~𝘈Ḇ𝖢𝕯٤ḞԍНǏ𝙅ƘԸⲘ𝙉০Ρ𝗤Ɍ𝓢ȚЦ𝒱Ѡ𝓧ƳȤѧᖯć𝗱ễ𝑓𝙜Ⴙ𝞲𝑗𝒌ļṃŉо𝞎𝒒ᵲꜱ𝙩ừ𝗏ŵ𝒙𝒚ź1234567890!@#$%^&*()-_=+[{]};:'",<.>/?~АḂⲤ𝗗𝖤𝗙ꞠꓧȊ𝐉𝜥ꓡ𝑀𝑵Ǭ𝙿𝑄Ŗ𝑆𝒯𝖴𝘝𝘞ꓫŸ𝜡ả𝘢ƀ𝖼ḋếᵮℊ𝙝Ꭵ𝕛кιṃդⱺ𝓅𝘲𝕣𝖘ŧ𝑢ṽẉ𝘅ყž1234567890!@#$%^&*()-_=+[{]};:'",<.>/?~Ѧ𝙱ƇᗞΣℱԍҤ١𝔍К𝓛𝓜ƝȎ𝚸𝑄Ṛ𝓢ṮṺƲᏔꓫ𝚈𝚭𝜶Ꮟçძ𝑒𝖿𝗀ḧ𝗂𝐣ҝɭḿ𝕟𝐨𝝔𝕢ṛ𝓼тú𝔳ẃ⤬𝝲𝗓1234567890!@#$%^&*()-_=+[{]};:'",<.>/?~𝖠Β𝒞𝘋𝙴𝓕ĢȞỈ𝕵ꓗʟ𝙼ℕ০𝚸𝗤ՀꓢṰǓⅤ𝔚Ⲭ𝑌𝙕𝘢𝕤)xxx";
#ifndef __APPLE__
        const std::string utf7l =
            R"xxx(+AQMEY9g13SAFbh7BAX8BIwIfE6XYNdyLAekBPh4/p5ECL9g13jHYNdxe2DXdy9g13jQCNtg134TYNd8IA8jYNdyZ2DXeBtg13qM-1234567890+ACEAQAAjACQAJQBeACYAKg()-+AF8APQArAFsAewBdAH0AOw:'+ACI,+ADw.+AD4-/?+AH7YNd4IHgbYNd2i2DXdbwZkHh4FDQQdAc/YNd5FAZgFOCyY2DXeSQnmA6HYNd3kAkzYNdziAhoEJtg13LEEYNg13OcBswIkBGcVrwEH2DXd8R7F2DXcU9g13lwQudg137LYNdxX2DXcjAE8HkMBSQQ+2DXfjtg13JIdcqcx2DXeaR7r2DXdzwF12DXcmdg13JoBeg-1234567890+ACEAQAAjACQAJQBeACYAKg()-+AF8APQArAFsAewBdAH0AOw:'+ACI,+ADw.+AD4-/?+AH4EEB4CLKTYNd3X2DXdpNg13dmnoKTnAgrYNdwJ2DXfJaTh2DXcQNg13HUB7Ng13n/YNdxEAVbYNdxG2DXcr9g13bTYNd4d2DXeHqTrAXjYNd8hHqPYNd4iAYDYNd28Hgsevx1uIQrYNd5dE6XYNd1bBDoDuR5DBWQsetg13MXYNd4y2DXdY9g13ZgBZ9g13GIefR6J2DXeBRDnAX4-1234567890+ACEAQAAjACQAJQBeACYAKg()-+AF8APQArAFsAewBdAH0AOw:'+ACI,+ADw.+AD4-/?+AH4EZtg13nEBhxXeA6MhMQUNBKQGYdg13Q0EGtg13NvYNdzcAZ0CDtg13rjYNdxEHlrYNdziHm4eegGyE9Sk69g13ojYNd6t2DXfNhPPAOcQ69g13FLYNd2/2DXdwB4n2DXdwtg13CMEnQJtHj/YNd1f2DXcKNg131TYNd1iHlvYNdz8BEIA+tg13TMegyks2DXfctg13dM-1234567890+ACEAQAAjACQAJQBeACYAKg()-+AF8APQArAFsAewBdAH0AOw:'+ACI,+ADw.+AD4-/?+AH7YNd2gA5LYNdye2DXeC9g13nTYNdzVASICHh7I2DXddaTXAp/YNd58IRUJ5tg13rjYNd3kBUCk4h5wAdMhZNg13RosrNg13EzYNd5V2DXeItg13WQ-)xxx";
#else
        // The macOS iconv gives slightly different results
        const std::string utf7l =
            R"xxx(+AQMEY9g13SAFbh7BAX8BIwIfE6XYNdyLAekBPh4/p5ECL9g13jHYNdxe2DXdy9g13jQCNtg134TYNd8IA8jYNdyZ2DXeBtg13qM-1234567890+ACEAQAAjACQAJQBeACYAKg()-+AF8APQ-+-+AFsAewBdAH0AOw:'+ACI,+ADw.+AD4-/?+AH7YNd4IHgbYNd2i2DXdbwZkHh4FDQQdAc/YNd5FAZgFOCyY2DXeSQnmA6HYNd3kAkzYNdziAhoEJtg13LEEYNg13OcBswIkBGcVrwEH2DXd8R7F2DXcU9g13lwQudg137LYNdxX2DXcjAE8HkMBSQQ+2DXfjtg13JIdcqcx2DXeaR7r2DXdzwF12DXcmdg13JoBeg-1234567890+ACEAQAAjACQAJQBeACYAKg()-+AF8APQ-+-+AFsAewBdAH0AOw:'+ACI,+ADw.+AD4-/?+AH4EEB4CLKTYNd3X2DXdpNg13dmnoKTnAgrYNdwJ2DXfJaTh2DXcQNg13HUB7Ng13n/YNdxEAVbYNdxG2DXcr9g13bTYNd4d2DXeHqTrAXjYNd8hHqPYNd4iAYDYNd28Hgsevx1uIQrYNd5dE6XYNd1bBDoDuR5DBWQsetg13MXYNd4y2DXdY9g13ZgBZ9g13GIefR6J2DXeBRDnAX4-1234567890+ACEAQAAjACQAJQBeACYAKg()-+AF8APQ-+-+AFsAewBdAH0AOw:'+ACI,+ADw.+AD4-/?+AH4EZtg13nEBhxXeA6MhMQUNBKQGYdg13Q0EGtg13NvYNdzcAZ0CDtg13rjYNdxEHlrYNdziHm4eegGyE9Sk69g13ojYNd6t2DXfNhPPAOcQ69g13FLYNd2/2DXdwB4n2DXdwtg13CMEnQJtHj/YNd1f2DXcKNg131TYNd1iHlvYNdz8BEIA+tg13TMegyks2DXfctg13dM-1234567890+ACEAQAAjACQAJQBeACYAKg()-+AF8APQ-+-+AFsAewBdAH0AOw:'+ACI,+ADw.+AD4-/?+AH7YNd2gA5LYNdye2DXeC9g13nTYNdzVASICHh7I2DXddaTXAp/YNd58IRUJ5tg13rjYNd3kBUCk4h5wAdMhZNg13RosrNg13EzYNd5V2DXeItg13WQ-)xxx";
#endif

        Util::CharacterConverter utf8_to_7("UTF-8", "UTF-7");
        LOK_ASSERT_EQUAL_STR(utf7, utf8_to_7.convert(utf8));
        LOK_ASSERT_EQUAL_STR(utf7l, utf8_to_7.convert(utf8l));
        LOK_ASSERT_EQUAL_STR(utf7, utf8_to_7.convert(utf8));
        LOK_ASSERT_EQUAL_STR(utf7l, utf8_to_7.convert(utf8l));

        Util::CharacterConverter utf7_to_8("UTF-7", "UTF-8");
        LOK_ASSERT_EQUAL_STR(utf8, utf7_to_8.convert(utf7));
#ifndef __APPLE__
        LOK_ASSERT_EQUAL_STR(utf8l, utf7_to_8.convert(utf7l));
#endif
        LOK_ASSERT_EQUAL_STR(utf8, utf7_to_8.convert(utf7));
#ifndef __APPLE__
        LOK_ASSERT_EQUAL_STR(utf8l, utf7_to_8.convert(utf7l));
#endif
    }
}

void UtilTests::testUtf8()
{
#if ENABLE_DEBUG
    constexpr std::string_view testname = __func__;
    LOK_ASSERT(Util::isValidUtf8("foo") > 3);
    LOK_ASSERT(Util::isValidUtf8("©") > 2); // 2 char
    LOK_ASSERT(Util::isValidUtf8("→ ") > 3); // 3 char
    LOK_ASSERT(Util::isValidUtf8("🏃 is not 🏊.") > 11);
    LOK_ASSERT(Util::isValidUtf8("\xff\x03") < 2);
#endif
}

void UtilTests::testEliminatePrefix()
{
    constexpr auto testname = __func__;

    LOK_ASSERT_EQUAL_STR(std::string(), Util::eliminatePrefix(std::string(), std::string()));
    LOK_ASSERT_EQUAL_STR("test", Util::eliminatePrefix("test", std::string()));
    LOK_ASSERT_EQUAL_STR("", Util::eliminatePrefix(std::string(), "test"));
    LOK_ASSERT_EQUAL_STR("what", Util::eliminatePrefix(std::string("testwhat"), "test"));
    LOK_ASSERT_EQUAL_STR("Command", Util::eliminatePrefix(std::string(".uno:Command"), ".uno:"));
    LOK_ASSERT_EQUAL_STR("", Util::eliminatePrefix(std::string(".uno:Command"), ".uno:Command"));
    LOK_ASSERT_EQUAL_STR(".uno:Command", Util::eliminatePrefix(std::string(".uno:Command"), ".uno:Commander"));
    LOK_ASSERT_EQUAL_STR("uno:Command", Util::eliminatePrefix(std::string(".uno:Command"), "."));
    LOK_ASSERT_EQUAL_STR(".uno:Command", Util::eliminatePrefix(std::string(".uno:Command"), ""));
}

void UtilTests::testStreamMatch()
{
    constexpr auto testname = __func__;

    std::string input("Lorem ipsum dolor sit amet consectetur adipiscing elit");
    std::istringstream is(input);
    std::ostringstream os;

    Util::copyToMatch(is, os, " amet ");
    std::string expected = "Lorem ipsum dolor sit";
    LOK_ASSERT_EQUAL_STR(expected, os.str());
    // input stream read position should be at the start of the match
    LOK_ASSERT_EQUAL(static_cast<std::streampos>(expected.size()), is.tellg());

    Util::seekToMatch(is, " adipiscing ");
    LOK_ASSERT_EQUAL(static_cast<std::streampos>(38), is.tellg());

    // copy as far as match that never occurs should copy to end of stream
    Util::copyToMatch(is, os, "nomatch");
    std::string final = "Lorem ipsum dolor sit adipiscing elit";
    LOK_ASSERT_EQUAL_STR(final, os.str());
}

void UtilTests::testBase64Encode()
{
    constexpr std::string_view testname = __func__;

    // Simple string.
    LOK_ASSERT_EQUAL(std::string("SGVsbG8="), Util::base64Encode("Hello"));

    // Input with newlines is encoded as-is (newlines become part of the encoding).
    std::string withNewLines = "Hello\nWorld\n";
    std::string encoded = Util::base64Encode(withNewLines);
    LOK_ASSERT_EQUAL(withNewLines, Util::base64Decode(encoded));

    // Round-trip with longer content.
    std::string longer = "The quick brown fox jumps over the lazy dog";
    LOK_ASSERT_EQUAL(longer, Util::base64Decode(Util::base64Encode(longer)));

    // Long input (>54 bytes) would trigger Poco line breaks every 72 output
    // chars. Verify the output contains no newlines.
    std::string longInput(200, 'A');
    std::string longencoded = Util::base64Encode(longInput);
    LOK_ASSERT(longencoded.find('\n') == std::string::npos);
    LOK_ASSERT(longencoded.find('\r') == std::string::npos);
}

void UtilTests::testBase64Decode()
{
    constexpr std::string_view testname = __func__;

    // Simple round-trip.
    LOK_ASSERT_EQUAL(std::string("Hello"), Util::base64Decode("SGVsbG8="));

    // Decode longer string.
    LOK_ASSERT_EQUAL(std::string("Hello World!"),
                     Util::base64Decode("SGVsbG8gV29ybGQh"));
}

CPPUNIT_TEST_SUITE_REGISTRATION(UtilTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
