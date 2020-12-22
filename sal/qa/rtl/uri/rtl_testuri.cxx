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

#include <config_locales.h>

#include <rtl/strbuf.hxx>
#include <rtl/uri.hxx>
#include <rtl/ustrbuf.hxx>

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

    OUString aText1;
    OUString aText2;

    // Check that all characters map back to themselves when encoded/decoded:

    aText1 = OUString(
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
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "failure 1",
            aText2,
            rtl::Uri::decode(
                rtl::Uri::encode(
                    aText1, eCharClass, rtl_UriEncodeKeepEscapes,
                    RTL_TEXTENCODING_ISO_8859_1),
                rtl_UriDecodeWithCharset, RTL_TEXTENCODING_ASCII_US));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "failure 2",
            aText2,
            rtl::Uri::decode(
                rtl::Uri::encode(
                    aText1, eCharClass, rtl_UriEncodeCheckEscapes,
                    RTL_TEXTENCODING_ISO_8859_1),
                rtl_UriDecodeWithCharset, RTL_TEXTENCODING_ASCII_US));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "failure 3",
            aText2,
            rtl::Uri::decode(
                rtl::Uri::encode(
                    aText1, eCharClass, rtl_UriEncodeKeepEscapes,
                    RTL_TEXTENCODING_ISO_8859_1),
                rtl_UriDecodeWithCharset, RTL_TEXTENCODING_ISO_8859_1));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "failure 4",
            aText2,
            rtl::Uri::decode(
                rtl::Uri::encode(
                    aText1, eCharClass, rtl_UriEncodeCheckEscapes,
                    RTL_TEXTENCODING_ISO_8859_1),
                rtl_UriDecodeWithCharset, RTL_TEXTENCODING_ISO_8859_1));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "failure 5",
            aText2,
            rtl::Uri::decode(
                rtl::Uri::encode(
                    aText1, eCharClass, rtl_UriEncodeKeepEscapes,
                    RTL_TEXTENCODING_ISO_8859_1),
                rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "failure 6",
            aText2,
            rtl::Uri::decode(
                rtl::Uri::encode(
                    aText1, eCharClass, rtl_UriEncodeCheckEscapes,
                    RTL_TEXTENCODING_ISO_8859_1),
                rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8));
    }

    aText1 = OUString(
        ("\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
         "\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
         "\x20\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2A\x2B\x2C\x2D\x2E\x2F"
         "\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\x3A\x3B\x3C\x3D\x3E\x3F"
         "\x40\x41\x42\x43\x44\x45\x46\x47\x48\x49\x4A\x4B\x4C\x4D\x4E\x4F"
         "\x50\x51\x52\x53\x54\x55\x56\x57\x58\x59\x5A\x5B\x5C\x5D\x5E\x5F"
         "\x60\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6A\x6B\x6C\x6D\x6E\x6F"
         "\x70\x71\x72\x73\x74\x75\x76\x77\x78\x79\x7A\x7B\x7C\x7D\x7E\x7F"
         "\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8A\x8B\x8C\x8D\x8E\x8F"
         "\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9A\x9B\x9C\x9D\x9E\x9F"
         "\xA0\xA1\xA2\xA3\xA4\xA5\xA6\xA7\xA8\xA9\xAA\xAB\xAC\xAD\xAE\xAF"
         "\xB0\xB1\xB2\xB3\xB4\xB5\xB6\xB7\xB8\xB9\xBA\xBB\xBC\xBD\xBE\xBF"
         "\xC0\xC1\xC2\xC3\xC4\xC5\xC6\xC7\xC8\xC9\xCA\xCB\xCC\xCD\xCE\xCF"
         "\xD0\xD1\xD2\xD3\xD4\xD5\xD6\xD7\xD8\xD9\xDA\xDB\xDC\xDD\xDE\xDF"
         "\xE0\xE1\xE2\xE3\xE4\xE5\xE6\xE7\xE8\xE9\xEA\xEB\xEC\xED\xEE\xEF"
         "\xF0\xF1\xF2\xF3\xF4\xF5\xF6\xF7\xF8\xF9\xFA\xFB\xFC\xFD\xFE\xFF"),
        256, RTL_TEXTENCODING_ISO_8859_1);
    aText2 = aText1;
    for (rtl_UriCharClass eCharClass = eFirstCharClass;
         eCharClass <= eLastCharClass;
         eCharClass = static_cast< rtl_UriCharClass >(eCharClass + 1))
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "failure 7",
            aText2,
            rtl::Uri::decode(
                rtl::Uri::encode(
                    aText1, eCharClass, rtl_UriEncodeKeepEscapes,
                    RTL_TEXTENCODING_ISO_8859_1),
                rtl_UriDecodeWithCharset, RTL_TEXTENCODING_ISO_8859_1));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "failure 8",
            aText2,
            rtl::Uri::decode(
                rtl::Uri::encode(
                    aText1, eCharClass, rtl_UriEncodeCheckEscapes,
                    RTL_TEXTENCODING_ISO_8859_1),
                rtl_UriDecodeWithCharset, RTL_TEXTENCODING_ISO_8859_1));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "failure 9",
            aText2,
            rtl::Uri::decode(
                rtl::Uri::encode(
                    aText1, eCharClass, rtl_UriEncodeKeepEscapes,
                    RTL_TEXTENCODING_UTF8),
                rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "failure 10",
            aText2,
            rtl::Uri::decode(
                rtl::Uri::encode(
                    aText1, eCharClass, rtl_UriEncodeCheckEscapes,
                    RTL_TEXTENCODING_UTF8),
                rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8));
    }

    // Check surrogate handling:

    aText1 = u"\xD800" // %ED%A0%80
             u"\U000103FF" // 0xD800,0xDFFF -> %F0%90%8F%BF
             u"\xDFFF" // %ED%BF%BF
             u"A"; // A
    aText2 = "%ED%A0%80" "%F0%90%8F%BF" "%ED%BF%BF" "A";
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "failure 11",
        aText2,
        rtl::Uri::encode(
            aText1, rtl_UriCharClassUric, rtl_UriEncodeIgnoreEscapes,
            RTL_TEXTENCODING_UTF8));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "failure 12",
        aText2,
        rtl::Uri::encode(
            aText1, rtl_UriCharClassUric, rtl_UriEncodeKeepEscapes,
            RTL_TEXTENCODING_UTF8));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "failure 13",
        aText2,
        rtl::Uri::encode(
            aText1, rtl_UriCharClassUric, rtl_UriEncodeCheckEscapes,
            RTL_TEXTENCODING_UTF8));

    aText1 = "%ed%a0%80" "%f0%90%8f%bf" "%ed%bf%bf" "A";
    aText2 = u"%ED%A0%80" u"\U000103FF" u"%ED%BF%BF" u"A";
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "failure 14",
        aText2,
        rtl::Uri::decode(aText1, rtl_UriDecodeToIuri, RTL_TEXTENCODING_UTF8));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "failure 15",
        aText2,
        rtl::Uri::decode(
            aText1, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8));

    // Check UTF-8 handling:

    aText1 = "%E0%83%BF";
        // \U+00FF encoded with three instead of two bytes
    aText2 = aText1;
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "failure 16",
        aText2,
        rtl::Uri::encode(
            aText1, rtl_UriCharClassUric, rtl_UriEncodeCheckEscapes,
            RTL_TEXTENCODING_UTF8));

    aText1 = "%EF%BF%BF";
        // \U+FFFF is no legal character
    aText2 = aText1;
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "failure 17",
        aText2,
        rtl::Uri::encode(
            aText1, rtl_UriCharClassUric, rtl_UriEncodeCheckEscapes,
            RTL_TEXTENCODING_UTF8));

    // Check IURI handling:

    aText1 = "%30%C3%BF";
    aText2 = u"%30\u00FF";
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "failure 18",
        aText2,
        rtl::Uri::decode(aText1, rtl_UriDecodeToIuri, RTL_TEXTENCODING_UTF8));

    // Check modified rtl_UriCharClassUnoParamValue (removed '[' and ']'):

    aText1 = "[]%5B%5D";
    aText2 = "%5B%5D%5B%5D";
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "failure 19",
        aText2,
        rtl::Uri::encode(
            aText1, rtl_UriCharClassUnoParamValue, rtl_UriEncodeCheckEscapes,
            RTL_TEXTENCODING_ASCII_US));

    // Check Uri::convertRelToAbs:

    struct RelToAbsTest
    {
        char const * pBase;
        char const * pRel;
        char const * pAbs;
    };
    static RelToAbsTest const aRelToAbsTest[]
        = { // The following tests are taken from RFC 3986:
            { "http://a/b/c/d;p?q", "g:h", "g:h" },
            { "http://a/b/c/d;p?q", "g", "http://a/b/c/g" },
            { "http://a/b/c/d;p?q", "./g", "http://a/b/c/g" },
            { "http://a/b/c/d;p?q", "g/", "http://a/b/c/g/" },
            { "http://a/b/c/d;p?q", "/g", "http://a/g" },
            { "http://a/b/c/d;p?q", "//g", "http://g" },
            { "http://a/b/c/d;p?q", "?y", "http://a/b/c/d;p?y" },
            { "http://a/b/c/d;p?q", "g?y", "http://a/b/c/g?y" },
            { "http://a/b/c/d;p?q", "#s", "http://a/b/c/d;p?q#s" },
            { "http://a/b/c/d;p?q", "g#s", "http://a/b/c/g#s" },
            { "http://a/b/c/d;p?q", "g?y#s", "http://a/b/c/g?y#s" },
            { "http://a/b/c/d;p?q", ";x", "http://a/b/c/;x" },
            { "http://a/b/c/d;p?q", "g;x", "http://a/b/c/g;x" },
            { "http://a/b/c/d;p?q", "g;x?y#s", "http://a/b/c/g;x?y#s" },
            { "http://a/b/c/d;p?q", "", "http://a/b/c/d;p?q" },
            { "http://a/b/c/d;p?q", ".", "http://a/b/c/" },
            { "http://a/b/c/d;p?q", "./", "http://a/b/c/" },
            { "http://a/b/c/d;p?q", "..", "http://a/b/" },
            { "http://a/b/c/d;p?q", "../", "http://a/b/" },
            { "http://a/b/c/d;p?q", "../g", "http://a/b/g" },
            { "http://a/b/c/d;p?q", "../..", "http://a/" },
            { "http://a/b/c/d;p?q", "../../", "http://a/" },
            { "http://a/b/c/d;p?q", "../../g", "http://a/g" },
            { "http://a/b/c/d;p?q", "../../../g", "http://a/g" },
            { "http://a/b/c/d;p?q", "../../../../g", "http://a/g" },
            { "http://a/b/c/d;p?q", "/./g", "http://a/g" },
            { "http://a/b/c/d;p?q", "/../g", "http://a/g" },
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
            { "http!://a/b/c/d;p?q", "g", nullptr },
            { "http:b/c/d;p?q", "g:h", "g:h" },
            { "http:b/c/d;p?q", "g", "http:b/c/g" },
            { "http://a/b/../", "../c", "http://a/c" },
            { "http://a/b/..", "../c", "http://a/c" },
            { "http://a/./b/", ".././.././../c", "http://a/c" },
            { "http://a", "b", "http://a/b" },
            { "", "http://a/b/../c", "http://a/c" },

            { "http://a/b/c", "d", "http://a/b/d" },
            { "http://a/b/c/", "d", "http://a/b/c/d" },
            { "http://a/b/c//", "d", "http://a/b/c//d" } };

    for (std::size_t i = 0; i < SAL_N_ELEMENTS(aRelToAbsTest); ++i)
    {
        OUString aAbs;
        bool bMalformed = false;
        try {
            aAbs = rtl::Uri::convertRelToAbs(
                OUString::createFromAscii(aRelToAbsTest[i].pBase),
                OUString::createFromAscii(aRelToAbsTest[i].pRel));
        } catch (const rtl::MalformedUriException &) {
            bMalformed = true;
        }
        if (bMalformed
            ? aRelToAbsTest[i].pAbs != nullptr
            : (aRelToAbsTest[i].pAbs == nullptr
               || !aAbs.equalsAscii(aRelToAbsTest[i].pAbs)))
        {
            printf(
                "FAILED convertRelToAbs(%s, %s) -> %s != %s\n",
                aRelToAbsTest[i].pBase, aRelToAbsTest[i].pRel,
                (bMalformed
                 ? "<MALFORMED>"
                 : OUStringToOString(
                     aAbs, RTL_TEXTENCODING_UTF8).getStr()),
                (aRelToAbsTest[i].pAbs == nullptr
                 ? "<MALFORMED>" : aRelToAbsTest[i].pAbs));
            CPPUNIT_ASSERT(false);
        }
    }

    // Check encode with unusual text encodings:

    {
        sal_Unicode const aText1U[] = u" !\u0401\u045F";
        aText1 = OUString(aText1U);
        aText2 = "%20!%A1%FF";
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "failure 20",
            aText2,
            rtl::Uri::encode(
                aText1, rtl_UriCharClassUric, rtl_UriEncodeIgnoreEscapes,
                RTL_TEXTENCODING_ISO_8859_5));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "failure 20a",
            aText1,
            rtl::Uri::decode(
                aText2, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_ISO_8859_5));
    }
    {
        sal_Unicode const aText1U[] = u" !\u0401\u0700\u045F";
        aText1 = OUString(aText1U);
        sal_Unicode const aText2U[] =
            u"%20!%A1\u0700%FF";
        aText2 = OUString(aText2U);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "failure 21",
            aText2,
            rtl::Uri::encode(
                aText1, rtl_UriCharClassUric, rtl_UriEncodeIgnoreEscapes,
                RTL_TEXTENCODING_ISO_8859_5));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "failure 21a",
            aText1,
            rtl::Uri::decode(
                aText2, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_ISO_8859_5));
    }
#if WITH_LOCALE_ALL || WITH_LOCALE_zh
    {
        sal_Unicode const aText1U[] = u" !\u028A\U00022513";
        aText1 = OUString(aText1U);
        aText2 = "%20!%81%30%B1%33%95%39%C5%37";
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "failure 22",
            aText2,
            rtl::Uri::encode(
                aText1, rtl_UriCharClassUric, rtl_UriEncodeIgnoreEscapes,
                RTL_TEXTENCODING_GB_18030));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "failure 22a",
            aText1,
            rtl::Uri::decode(
                aText2, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_GB_18030));
    }
#endif
    // Check strict mode:

    {
        sal_Unicode const aText1U[] = u" !\u0401\u0700\u045F";
        aText1 = OUString(aText1U);
        aText2 = OUString();
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "failure 23",
            aText2,
            rtl::Uri::encode(
                aText1, rtl_UriCharClassUric, rtl_UriEncodeStrict,
                RTL_TEXTENCODING_ISO_8859_5));
    }
    {
        aText1 = "%20%C4%80%FF";
        aText2 = OUString();
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "failure 24",
            aText2,
            rtl::Uri::decode(
                aText1, rtl_UriDecodeStrict, RTL_TEXTENCODING_UTF8));
    }
#if WITH_LOCALE_ALL || WITH_LOCALE_zh
    {
        aText1 = "%81 ";
        aText2 = OUString();
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "failure 25",
            aText2,
            rtl::Uri::decode(
                aText1, rtl_UriDecodeStrict, RTL_TEXTENCODING_GB_18030));
    }
    {
        aText1 = "%81%20";
        aText2 = OUString();
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "failure 26",
            aText2,
            rtl::Uri::decode(
                aText1, rtl_UriDecodeStrict, RTL_TEXTENCODING_GB_18030));
    }
    {
        aText1 = "%81%30%B1%33";
        sal_Unicode const aText2U[] = u"\u028A";
        aText2 = OUString(aText2U);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "failure 27",
            aText2,
            rtl::Uri::decode(
                aText1, rtl_UriDecodeStrict, RTL_TEXTENCODING_GB_18030));
    }
    {
        aText1 = "%810%B13";
        sal_Unicode const aText2U[] = u"\u028A";
        aText2 = OUString(aText2U);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "failure 28",
            aText2,
            rtl::Uri::decode(
                aText1, rtl_UriDecodeStrict, RTL_TEXTENCODING_GB_18030));
    }
#endif
    // Check rtl_UriEncodeStrictKeepEscapes mode:

    {
        aText1 = "%%ea%c3%aa";
        aText2 = "%25%EA%C3%AA";
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "failure 29",
            aText2,
            rtl::Uri::encode(
                aText1, rtl_UriCharClassUric, rtl_UriEncodeStrictKeepEscapes,
                RTL_TEXTENCODING_UTF8));
    }
    {
        sal_Unicode const aText1U[] = u"\u00EA";
        aText1 = OUString(aText1U);
        aText2 = "%C3%AA";
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "failure 30",
            aText2,
            rtl::Uri::encode(
                aText1, rtl_UriCharClassUric, rtl_UriEncodeStrictKeepEscapes,
                RTL_TEXTENCODING_UTF8));
    }
    {
        sal_Unicode const aText1U[] = u" !\u0401\u0700\u045F";
        aText1 = OUString(aText1U);
        aText2 = OUString();
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "failure 23",
            aText2,
            rtl::Uri::encode(
                aText1, rtl_UriCharClassUric, rtl_UriEncodeStrictKeepEscapes,
                RTL_TEXTENCODING_ISO_8859_5));
    }
}

}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
