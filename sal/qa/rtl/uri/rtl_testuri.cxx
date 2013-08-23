/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include "rtl/strbuf.hxx"
#include "rtl/uri.hxx"
#include "rtl/ustrbuf.hxx"

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <cstddef>
#include <stdio.h>

namespace {

struct Test: public CppUnit::TestFixture {
    void test_Uri();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(test_Uri);
    CPPUNIT_TEST_SUITE_END();
};

void Test::test_Uri() {
    rtl_UriCharClass const eFirstCharClass = rtl_UriCharClassNone;
    rtl_UriCharClass const eLastCharClass = rtl_UriCharClassUnoParamValue;

    rtl::OUStringBuffer aBuffer;
    rtl::OUString aText1;
    rtl::OUString aText2;

    // Check that all characters map back to themselves when encoded/decoded:

    aText1 = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM(
            "\x00\x01\x02\x03\x04\x05\x06\x07"
            "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
            "\x10\x11\x12\x13\x14\x15\x16\x17"
            "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
            "\x20\x21\x22\x23\x24\x25\x26\x27"
            "\x28\x29\x2A\x2B\x2C\x2D\x2E\x2F"
            "\x30\x31\x32\x33\x34\x35\x36\x37"
            "\x38\x39\x3A\x3B\x3C\x3D\x3E\x3F"
            "\x40\x41\x42\x43\x44\x45\x46\x47"
            "\x48\x49\x4A\x4B\x4C\x4D\x4E\x4F"
            "\x50\x51\x52\x53\x54\x55\x56\x57"
            "\x58\x59\x5A\x5B\x5C\x5D\x5E\x5F"
            "\x60\x61\x62\x63\x64\x65\x66\x67"
            "\x68\x69\x6A\x6B\x6C\x6D\x6E\x6F"
            "\x70\x71\x72\x73\x74\x75\x76\x77"
            "\x78\x79\x7A\x7B\x7C\x7D\x7E\x7F"));
    aText2 = aText1;
    for (rtl_UriCharClass eCharClass = eFirstCharClass;
         eCharClass <= eLastCharClass;
         eCharClass = static_cast< rtl_UriCharClass >(eCharClass + 1))
    {
        CPPUNIT_ASSERT_MESSAGE(
            "failure 1",
            (rtl::Uri::decode(
                rtl::Uri::encode(
                    aText1, eCharClass, rtl_UriEncodeKeepEscapes,
                    RTL_TEXTENCODING_ISO_8859_1),
                rtl_UriDecodeWithCharset, RTL_TEXTENCODING_ASCII_US)
             == aText2));
        CPPUNIT_ASSERT_MESSAGE(
            "failure 2",
            (rtl::Uri::decode(
                rtl::Uri::encode(
                    aText1, eCharClass, rtl_UriEncodeCheckEscapes,
                    RTL_TEXTENCODING_ISO_8859_1),
                rtl_UriDecodeWithCharset, RTL_TEXTENCODING_ASCII_US)
             == aText2));
        CPPUNIT_ASSERT_MESSAGE(
            "failure 3",
            (rtl::Uri::decode(
                rtl::Uri::encode(
                    aText1, eCharClass, rtl_UriEncodeKeepEscapes,
                    RTL_TEXTENCODING_ISO_8859_1),
                rtl_UriDecodeWithCharset, RTL_TEXTENCODING_ISO_8859_1)
             == aText2));
        CPPUNIT_ASSERT_MESSAGE(
            "failure 4",
            (rtl::Uri::decode(
                rtl::Uri::encode(
                    aText1, eCharClass, rtl_UriEncodeCheckEscapes,
                    RTL_TEXTENCODING_ISO_8859_1),
                rtl_UriDecodeWithCharset, RTL_TEXTENCODING_ISO_8859_1)
             == aText2));
        CPPUNIT_ASSERT_MESSAGE(
            "failure 5",
            (rtl::Uri::decode(
                rtl::Uri::encode(
                    aText1, eCharClass, rtl_UriEncodeKeepEscapes,
                    RTL_TEXTENCODING_ISO_8859_1),
                rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8)
             == aText2));
        CPPUNIT_ASSERT_MESSAGE(
            "failure 6",
            (rtl::Uri::decode(
                rtl::Uri::encode(
                    aText1, eCharClass, rtl_UriEncodeCheckEscapes,
                    RTL_TEXTENCODING_ISO_8859_1),
                rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8)
             == aText2));
    }

    aText1 = rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM(
            "\x00\x01\x02\x03\x04\x05\x06\x07"
            "\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
            "\x10\x11\x12\x13\x14\x15\x16\x17"
            "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
            "\x20\x21\x22\x23\x24\x25\x26\x27"
            "\x28\x29\x2A\x2B\x2C\x2D\x2E\x2F"
            "\x30\x31\x32\x33\x34\x35\x36\x37"
            "\x38\x39\x3A\x3B\x3C\x3D\x3E\x3F"
            "\x40\x41\x42\x43\x44\x45\x46\x47"
            "\x48\x49\x4A\x4B\x4C\x4D\x4E\x4F"
            "\x50\x51\x52\x53\x54\x55\x56\x57"
            "\x58\x59\x5A\x5B\x5C\x5D\x5E\x5F"
            "\x60\x61\x62\x63\x64\x65\x66\x67"
            "\x68\x69\x6A\x6B\x6C\x6D\x6E\x6F"
            "\x70\x71\x72\x73\x74\x75\x76\x77"
            "\x78\x79\x7A\x7B\x7C\x7D\x7E\x7F"
            "\x80\x81\x82\x83\x84\x85\x86\x87"
            "\x88\x89\x8A\x8B\x8C\x8D\x8E\x8F"
            "\x90\x91\x92\x93\x94\x95\x96\x97"
            "\x98\x99\x9A\x9B\x9C\x9D\x9E\x9F"
            "\xA0\xA1\xA2\xA3\xA4\xA5\xA6\xA7"
            "\xA8\xA9\xAA\xAB\xAC\xAD\xAE\xAF"
            "\xB0\xB1\xB2\xB3\xB4\xB5\xB6\xB7"
            "\xB8\xB9\xBA\xBB\xBC\xBD\xBE\xBF"
            "\xC0\xC1\xC2\xC3\xC4\xC5\xC6\xC7"
            "\xC8\xC9\xCA\xCB\xCC\xCD\xCE\xCF"
            "\xD0\xD1\xD2\xD3\xD4\xD5\xD6\xD7"
            "\xD8\xD9\xDA\xDB\xDC\xDD\xDE\xDF"
            "\xE0\xE1\xE2\xE3\xE4\xE5\xE6\xE7"
            "\xE8\xE9\xEA\xEB\xEC\xED\xEE\xEF"
            "\xF0\xF1\xF2\xF3\xF4\xF5\xF6\xF7"
            "\xF8\xF9\xFA\xFB\xFC\xFD\xFE\xFF"));
    aText2 = aText1;
    for (rtl_UriCharClass eCharClass = eFirstCharClass;
         eCharClass <= eLastCharClass;
         eCharClass = static_cast< rtl_UriCharClass >(eCharClass + 1))
    {
        CPPUNIT_ASSERT_MESSAGE(
            "failure 7",
            (rtl::Uri::decode(
                rtl::Uri::encode(
                    aText1, eCharClass, rtl_UriEncodeKeepEscapes,
                    RTL_TEXTENCODING_ISO_8859_1),
                rtl_UriDecodeWithCharset, RTL_TEXTENCODING_ISO_8859_1)
             == aText2));
        CPPUNIT_ASSERT_MESSAGE(
            "failure 8",
            (rtl::Uri::decode(
                rtl::Uri::encode(
                    aText1, eCharClass, rtl_UriEncodeCheckEscapes,
                    RTL_TEXTENCODING_ISO_8859_1),
                rtl_UriDecodeWithCharset, RTL_TEXTENCODING_ISO_8859_1)
             == aText2));
        CPPUNIT_ASSERT_MESSAGE(
            "failure 9",
            (rtl::Uri::decode(
                rtl::Uri::encode(
                    aText1, eCharClass, rtl_UriEncodeKeepEscapes,
                    RTL_TEXTENCODING_UTF8),
                rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8)
             == aText2));
        CPPUNIT_ASSERT_MESSAGE(
            "failure 10",
            (rtl::Uri::decode(
                rtl::Uri::encode(
                    aText1, eCharClass, rtl_UriEncodeCheckEscapes,
                    RTL_TEXTENCODING_UTF8),
                rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8)
             == aText2));
    }

    // Check surrogate handling:

    aBuffer.append(static_cast< sal_Unicode >(0xD800)); // %ED%A0%80
    aBuffer.append(static_cast< sal_Unicode >(0xD800)); // %F0%90%8F%BF
    aBuffer.append(static_cast< sal_Unicode >(0xDFFF));
    aBuffer.append(static_cast< sal_Unicode >(0xDFFF)); // %ED%BF%BF
    aBuffer.append(static_cast< sal_Unicode >('A')); // A
    aText1 = aBuffer.makeStringAndClear();
    aText2 = rtl::OUString(
            "%ED%A0%80" "%F0%90%8F%BF" "%ED%BF%BF" "A");
    CPPUNIT_ASSERT_MESSAGE(
        "failure 11",
        (rtl::Uri::encode(
            aText1, rtl_UriCharClassUric, rtl_UriEncodeIgnoreEscapes,
            RTL_TEXTENCODING_UTF8)
         == aText2));
    CPPUNIT_ASSERT_MESSAGE(
        "failure 12",
        (rtl::Uri::encode(
            aText1, rtl_UriCharClassUric, rtl_UriEncodeKeepEscapes,
            RTL_TEXTENCODING_UTF8)
         == aText2));
    CPPUNIT_ASSERT_MESSAGE(
        "failure 13",
        (rtl::Uri::encode(
            aText1, rtl_UriCharClassUric, rtl_UriEncodeCheckEscapes,
            RTL_TEXTENCODING_UTF8)
         == aText2));

    aText1 = rtl::OUString(
            "%ed%a0%80" "%f0%90%8f%bf" "%ed%bf%bf" "A");
    aBuffer.appendAscii(RTL_CONSTASCII_STRINGPARAM("%ED%A0%80"));
    aBuffer.append(static_cast< sal_Unicode >(0xD800));
    aBuffer.append(static_cast< sal_Unicode >(0xDFFF));
    aBuffer.appendAscii(RTL_CONSTASCII_STRINGPARAM("%ED%BF%BF"));
    aBuffer.append(static_cast< sal_Unicode >('A'));
    aText2 = aBuffer.makeStringAndClear();
    CPPUNIT_ASSERT_MESSAGE(
        "failure 14",
        (rtl::Uri::decode(aText1, rtl_UriDecodeToIuri, RTL_TEXTENCODING_UTF8)
         == aText2));
    CPPUNIT_ASSERT_MESSAGE(
        "failure 15",
        (rtl::Uri::decode(
            aText1, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8)
         == aText2));

    // Check UTF-8 handling:

    aText1 = rtl::OUString("%E0%83%BF");
        // \U+00FF encoded with three instead of two bytes
    aText2 = aText1;
    CPPUNIT_ASSERT_MESSAGE(
        "failure 16",
        (rtl::Uri::encode(
            aText1, rtl_UriCharClassUric, rtl_UriEncodeCheckEscapes,
            RTL_TEXTENCODING_UTF8)
         == aText2));

    aText1 = rtl::OUString("%EF%BF%BF");
        // \U+FFFF is no legal character
    aText2 = aText1;
    CPPUNIT_ASSERT_MESSAGE(
        "failure 17",
        (rtl::Uri::encode(
            aText1, rtl_UriCharClassUric, rtl_UriEncodeCheckEscapes,
            RTL_TEXTENCODING_UTF8)
         == aText2));

    // Check IURI handling:

    aText1 = rtl::OUString("%30%C3%BF");
    aBuffer.appendAscii(RTL_CONSTASCII_STRINGPARAM("%30"));
    aBuffer.append(static_cast< sal_Unicode >(0x00FF));
    aText2 = aBuffer.makeStringAndClear();
    CPPUNIT_ASSERT_MESSAGE(
        "failure 18",
        (rtl::Uri::decode(aText1, rtl_UriDecodeToIuri, RTL_TEXTENCODING_UTF8)
         == aText2));

    // Check modified rtl_UriCharClassUnoParamValue (removed '[' and ']'):

    aText1 = rtl::OUString("[]%5B%5D");
    aText2 = rtl::OUString("%5B%5D%5B%5D");
    CPPUNIT_ASSERT_MESSAGE(
        "failure 19",
        (rtl::Uri::encode(
            aText1, rtl_UriCharClassUnoParamValue, rtl_UriEncodeCheckEscapes,
            RTL_TEXTENCODING_ASCII_US)
         == aText2));

    // Check Uri::convertRelToAbs:

    struct RelToAbsTest
    {
        char const * pBase;
        char const * pRel;
        char const * pAbs;
    };
    static RelToAbsTest const aRelToAbsTest[]
        = { // The following tests are taken from RFC 2396:
            { "http://a/b/c/d;p?q", "g:h", "g:h" },
            { "http://a/b/c/d;p?q", "g", "http://a/b/c/g" },
            { "http://a/b/c/d;p?q", "./g", "http://a/b/c/g" },
            { "http://a/b/c/d;p?q", "g/", "http://a/b/c/g/" },
            { "http://a/b/c/d;p?q", "/g", "http://a/g" },
            { "http://a/b/c/d;p?q", "//g", "http://g" },
            { "http://a/b/c/d;p?q", "?y", "http://a/b/c/?y" },
            { "http://a/b/c/d;p?q", "g?y", "http://a/b/c/g?y" },
            { "http://a/b/c/d;p?q", "#s", "http://a/b/c/d;p?q#s" },
            { "http://a/b/c/d;p?q", "g#s", "http://a/b/c/g#s" },
            { "http://a/b/c/d;p?q", "g?y#s", "http://a/b/c/g?y#s" },
            { "http://a/b/c/d;p?q", ";x", "http://a/b/c/;x" },
            { "http://a/b/c/d;p?q", "g;x", "http://a/b/c/g;x" },
            { "http://a/b/c/d;p?q", "g;x?y#s", "http://a/b/c/g;x?y#s" },
            { "http://a/b/c/d;p?q", ".", "http://a/b/c/" },
            { "http://a/b/c/d;p?q", "./", "http://a/b/c/" },
            { "http://a/b/c/d;p?q", "..", "http://a/b/" },
            { "http://a/b/c/d;p?q", "../", "http://a/b/" },
            { "http://a/b/c/d;p?q", "../g", "http://a/b/g" },
            { "http://a/b/c/d;p?q", "../..", "http://a/" },
            { "http://a/b/c/d;p?q", "../../", "http://a/" },
            { "http://a/b/c/d;p?q", "../../g", "http://a/g" },
            { "http://a/b/c/d;p?q", "", "http://a/b/c/d;p?q" },
            { "http://a/b/c/d;p?q", "../../../g", "http://a/../g" },
            { "http://a/b/c/d;p?q", "../../../../g", "http://a/../../g" },
            { "http://a/b/c/d;p?q", "/./g", "http://a/./g" },
            { "http://a/b/c/d;p?q", "/../g", "http://a/../g" },
            { "http://a/b/c/d;p?q", "g.", "http://a/b/c/g." },
            { "http://a/b/c/d;p?q", ".g", "http://a/b/c/.g" },
            { "http://a/b/c/d;p?q", "g..", "http://a/b/c/g.." },
            { "http://a/b/c/d;p?q", "..g", "http://a/b/c/..g" },
            { "http://a/b/c/d;p?q", "./../g", "http://a/b/g" },
            { "http://a/b/c/d;p?q", "./g/.", "http://a/b/c/g/" },
            { "http://a/b/c/d;p?q", "g/./h", "http://a/b/c/g/h" },
            { "http://a/b/c/d;p?q", "g/../h", "http://a/b/c/h" },
            { "http://a/b/c/d;p?q", "g;x=1/./y", "http://a/b/c/g;x=1/y" },
            { "http://a/b/c/d;p?q", "g;x=1/../y", "http://a/b/c/y" },
            { "http://a/b/c/d;p?q", "g?y/./x", "http://a/b/c/g?y/./x" },
            { "http://a/b/c/d;p?q", "g?y/../x", "http://a/b/c/g?y/../x" },
            { "http://a/b/c/d;p?q", "g#s/./x", "http://a/b/c/g#s/./x" },
            { "http://a/b/c/d;p?q", "g#s/../x", "http://a/b/c/g#s/../x" },
            { "http://a/b/c/d;p?q", "http:g", "http:g" },
            { "http!://a/b/c/d;p?q", "g:h", "g:h" },
            { "http!://a/b/c/d;p?q", "g", 0 },
            { "http:b/c/d;p?q", "g:h", "g:h" },
            { "http:b/c/d;p?q", "g", 0 },
            { "http://a/b/../", "../c", "http://a/b/../../c" },
            { "http://a/b/..", "../c", "http://a/c" },
            { "http://a/./b/", ".././.././../c", "http://a/./../../c" } };
    for (std::size_t i = 0; i < sizeof aRelToAbsTest / sizeof (RelToAbsTest); ++i)
    {
        rtl::OUString aAbs;
        bool bMalformed = false;
        try {
            aAbs = rtl::Uri::convertRelToAbs(
                rtl::OUString::createFromAscii(aRelToAbsTest[i].pBase),
                rtl::OUString::createFromAscii(aRelToAbsTest[i].pRel));
        } catch (const rtl::MalformedUriException &) {
            bMalformed = true;
        }
        if (bMalformed
            ? aRelToAbsTest[i].pAbs != 0
            : (aRelToAbsTest[i].pAbs == 0
               || !aAbs.equalsAscii(aRelToAbsTest[i].pAbs)))
        {
            printf(
                "FAILED convertRelToAbs(%s, %s) -> %s != %s\n",
                aRelToAbsTest[i].pBase, aRelToAbsTest[i].pRel,
                (bMalformed
                 ? "<MALFORMED>"
                 : rtl::OUStringToOString(
                     aAbs, RTL_TEXTENCODING_UTF8).getStr()),
                (aRelToAbsTest[i].pAbs == 0
                 ? "<MALFORMED>" : aRelToAbsTest[i].pAbs));
            CPPUNIT_ASSERT(false);
        }
    }

    // Check encode with unusual text encodings:

    {
        sal_Unicode const aText1U[] = { ' ', '!', 0x0401, 0x045F, 0 };
        aText1 = rtl::OUString(aText1U);
        aText2 = rtl::OUString("%20!%A1%FF");
        CPPUNIT_ASSERT_MESSAGE(
            "failure 20",
            (rtl::Uri::encode(
                aText1, rtl_UriCharClassUric, rtl_UriEncodeIgnoreEscapes,
                RTL_TEXTENCODING_ISO_8859_5)
             == aText2));
        CPPUNIT_ASSERT_MESSAGE(
            "failure 20a",
            (rtl::Uri::decode(
                aText2, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_ISO_8859_5)
             == aText1));
    }
    {
        sal_Unicode const aText1U[] = { ' ', '!', 0x0401, 0x0700, 0x045F, 0 };
        aText1 = rtl::OUString(aText1U);
        sal_Unicode const aText2U[] = {
            '%', '2', '0', '!', '%', 'A', '1', 0x0700, '%', 'F', 'F', 0 };
        aText2 = rtl::OUString(aText2U);
        CPPUNIT_ASSERT_MESSAGE(
            "failure 21",
            (rtl::Uri::encode(
                aText1, rtl_UriCharClassUric, rtl_UriEncodeIgnoreEscapes,
                RTL_TEXTENCODING_ISO_8859_5)
             == aText2));
        CPPUNIT_ASSERT_MESSAGE(
            "failure 21a",
            (rtl::Uri::decode(
                aText2, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_ISO_8859_5)
             == aText1));
    }
    {
        sal_Unicode const aText1U[] = { ' ', '!', 0x028A, 0xD849, 0xDD13, 0 };
        aText1 = rtl::OUString(aText1U);
        aText2 = rtl::OUString("%20!%81%30%B1%33%95%39%C5%37");
        CPPUNIT_ASSERT_MESSAGE(
            "failure 22",
            (rtl::Uri::encode(
                aText1, rtl_UriCharClassUric, rtl_UriEncodeIgnoreEscapes,
                RTL_TEXTENCODING_GB_18030)
             == aText2));
        CPPUNIT_ASSERT_MESSAGE(
            "failure 22a",
            (rtl::Uri::decode(
                aText2, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_GB_18030)
             == aText1));
    }

    // Check strict mode:

    {
        sal_Unicode const aText1U[] = { ' ', '!', 0x0401, 0x0700, 0x045F, 0 };
        aText1 = rtl::OUString(aText1U);
        aText2 = rtl::OUString();
        CPPUNIT_ASSERT_MESSAGE(
            "failure 23",
            (rtl::Uri::encode(
                aText1, rtl_UriCharClassUric, rtl_UriEncodeStrict,
                RTL_TEXTENCODING_ISO_8859_5)
             == aText2));
    }
    {
        aText1 = rtl::OUString("%20%C4%80%FF");
        aText2 = rtl::OUString();
        CPPUNIT_ASSERT_MESSAGE(
            "failure 24",
            (rtl::Uri::decode(
                aText1, rtl_UriDecodeStrict, RTL_TEXTENCODING_UTF8)
             == aText2));
    }
    {
        aText1 = rtl::OUString("%81 ");
        aText2 = rtl::OUString();
        CPPUNIT_ASSERT_MESSAGE(
            "failure 25",
            (rtl::Uri::decode(
                aText1, rtl_UriDecodeStrict, RTL_TEXTENCODING_GB_18030)
             == aText2));
    }
    {
        aText1 = rtl::OUString("%81%20");
        aText2 = rtl::OUString();
        CPPUNIT_ASSERT_MESSAGE(
            "failure 26",
            (rtl::Uri::decode(
                aText1, rtl_UriDecodeStrict, RTL_TEXTENCODING_GB_18030)
             == aText2));
    }
    {
        aText1 = rtl::OUString("%81%30%B1%33");
        sal_Unicode const aText2U[] = { 0x028A, 0 };
        aText2 = rtl::OUString(aText2U);
        CPPUNIT_ASSERT_MESSAGE(
            "failure 27",
            (rtl::Uri::decode(
                aText1, rtl_UriDecodeStrict, RTL_TEXTENCODING_GB_18030)
             == aText2));
    }
    {
        aText1 = rtl::OUString("%810%B13");
        sal_Unicode const aText2U[] = { 0x028A, 0 };
        aText2 = rtl::OUString(aText2U);
        CPPUNIT_ASSERT_MESSAGE(
            "failure 28",
            (rtl::Uri::decode(
                aText1, rtl_UriDecodeStrict, RTL_TEXTENCODING_GB_18030)
             == aText2));
    }

    // Check rtl_UriEncodeStrictKeepEscapes mode:

    {
        aText1 = rtl::OUString("%%ea%c3%aa");
        aText2 = rtl::OUString("%25%EA%C3%AA");
        CPPUNIT_ASSERT_MESSAGE(
            "failure 29",
            (rtl::Uri::encode(
                aText1, rtl_UriCharClassUric, rtl_UriEncodeStrictKeepEscapes,
                RTL_TEXTENCODING_UTF8)
             == aText2));
    }
    {
        sal_Unicode const aText1U[] = { 0x00EA, 0 };
        aText1 = rtl::OUString(aText1U);
        aText2 = rtl::OUString("%C3%AA");
        CPPUNIT_ASSERT_MESSAGE(
            "failure 30",
            (rtl::Uri::encode(
                aText1, rtl_UriCharClassUric, rtl_UriEncodeStrictKeepEscapes,
                RTL_TEXTENCODING_UTF8)
             == aText2));
    }
    {
        sal_Unicode const aText1U[] = { ' ', '!', 0x0401, 0x0700, 0x045F, 0 };
        aText1 = rtl::OUString(aText1U);
        aText2 = rtl::OUString();
        CPPUNIT_ASSERT_MESSAGE(
            "failure 23",
            (rtl::Uri::encode(
                aText1, rtl_UriCharClassUric, rtl_UriEncodeStrictKeepEscapes,
                RTL_TEXTENCODING_ISO_8859_5)
             == aText2));
    }
}

}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);
// NOADDITIONAL;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
