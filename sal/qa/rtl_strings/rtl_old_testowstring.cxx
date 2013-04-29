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


// LLA:
// this file is converted to use with testshl2
// original was placed in sal/test/textenc.cxx

#include <string.h>
#include <stdio.h>

#ifdef UNX
#include <wchar.h>
#endif

#include <rtl/ustring.hxx>

#include <rtl/string.hxx>

#include <testshl/simpleheader.hxx>

#define TEST_ENSURE(c, m) CPPUNIT_ASSERT_MESSAGE((m), (c))

using ::rtl::OUString;
using ::rtl::OUStringToOString;
using ::rtl::OStringToOUString;

namespace rtl_OUString
{
    class oldtests : public CppUnit::TestFixture
    {
    public:
        void test_OUString();
        void test_OString2OUStringAndViceVersa();

        CPPUNIT_TEST_SUITE( oldtests );
        CPPUNIT_TEST( test_OUString );
        CPPUNIT_TEST( test_OString2OUStringAndViceVersa );
        CPPUNIT_TEST_SUITE_END( );
    };


void oldtests::test_OUString()
{
    //          "Mein erster RTL OUString\n"
    //           |    |    |    |    |
    //  Index    0    5    10   15   20
    OUString s1(OUString("Mein erster RTL OUString\n"));
    TEST_ENSURE( s1 == OUString("Mein erster RTL OUString\n"), "test_OWString error 1");
    TEST_ENSURE( s1.getLength() == 25, "test_OWString error 2");

    OUString s2 = s1;
    TEST_ENSURE( s2[16] == (sal_Unicode)'O', "test_OWString error 3");
    TEST_ENSURE( s2.equals(s1), "test_OWString error 4");
    TEST_ENSURE( OUString( OUString("hallo")) == OUString("hallo"), "test_OWString error 4");
    TEST_ENSURE( s2.indexOf((sal_Unicode)'O') == 16, "test_OWString error 5");
    TEST_ENSURE( s2.indexOf((sal_Unicode)'O', 5) == 16, "test_OWString error 5a");
    TEST_ENSURE( s2.lastIndexOf((sal_Unicode)'r') == 20, "test_OWString error 6");
    TEST_ENSURE( s2[20] == (sal_Unicode)'r', "test_OWString error 7");
    TEST_ENSURE( s2[24] == (sal_Unicode)'\n', "test_OWString error 8");
    TEST_ENSURE( s2.lastIndexOf((sal_Unicode)'\n') == 24, "test_OWString error 9");
    TEST_ENSURE( s2.lastIndexOf((sal_Unicode)'M') == 0, "test_OWString error 10");
    TEST_ENSURE( s2.lastIndexOf((sal_Unicode)'t', s2.getLength() - 8) == 8, "test_OWString error 9");


    //          "Mein erster RTL OUString ist ein String aus der RTL Library\n"
    //           |    |    |    |    |    |    |    |    |    |    |    |
    //  Index    0    5    10   15   20   25   30   35   40   45   50   55
    OUString s3 = s2.copy(0, s2.getLength() - 1);
    OUString s4 = s3.concat( OUString(" ist ein String aus der RTL Library\n") );
    TEST_ENSURE( s4.getLength() == 60, "test_OWString error 11");

    s1 = s4.copy(0, 39);
    OUString s5;
    s5 = s1 + OUString(" aus der RTL Library\n");
    TEST_ENSURE( s5==s4 , "test_OWString error 12");
    TEST_ENSURE( s5.indexOf(OUString("RTL")) == 12, "test_OWString error 13");
    TEST_ENSURE( s5.lastIndexOf(OUString("RTL")) == 48, "test_OWString error 13");

    sal_Bool b = sal_False;
    OUString s6 = s5.valueOf(b);
//  TEST_ENSURE( s6.compareTo(OUString("False")) == 0, "test_OWString error 14");
    s6 = s5.valueOf((sal_Unicode)'H');
    TEST_ENSURE( s6 == OUString("H"), "test_OWString error 15");
    sal_Int32 n = 123456789L;
    s6 = s5.valueOf(n);
    TEST_ENSURE( s6 == OUString("123456789"), "test_OWString error 16");

#ifdef SAL_UNX
    sal_Int64 m = -3223372036854775807LL;
#else
    sal_Int64 m = -3223372036854775807;
#endif
    s6 = s5.valueOf(m);
    TEST_ENSURE( s6 ==  OUString("-3223372036854775807"), "test_OWString error 17");

     OUString s7;
    OUString s8(OUString("HALLO ICH BIN EIN SS"));
    s7 = OUString("          ");
    s8 = s7 + s8 + OUString("          ");
    TEST_ENSURE( s8 == OUString("          HALLO ICH BIN EIN SS          "),
                  "test_OWString error 22");

    s7 = s8.trim();
    TEST_ENSURE( s7 == OUString("HALLO ICH BIN EIN SS"), "test_OWString error 23");

    s7 = OUString("Hallo");
    s8 = OUString("aber Hallo");

    TEST_ENSURE( s7 < s8, "test_OWString error 25");
    TEST_ENSURE( s8 > s7, "test_OWString error 26");
    TEST_ENSURE( s7 != s8, "test_OWString error 27");
    TEST_ENSURE( s7 != OUString("blabla"), "test_OWString error 28");
    TEST_ENSURE( OUString("blabla") != s7, "test_OWString error 29");

    s8 = OUString("Hallo");
    TEST_ENSURE( s7 <= s8, "test_OWString error 30");
    TEST_ENSURE( s7 >= s8, "test_OwString error 31");

    s8 = s8.replace((sal_Unicode)'l', (sal_Unicode)'r');
    TEST_ENSURE( s8 == OUString("Harro"), "test_OWString error 32");

    //       "Ich bin ein String mit einem A und C und vielen m, m, m, m"
    //        |    |    |    |    |    |    |    |    |    |    |    |
    //index   0    5    10   15   20   25   30   35   40   45   50   55
    s8 = OUString("Ich bin ein String mit einem A und C und vielen m, m, m, m");

    TEST_ENSURE( OUString("aaa") < OUString("bbb") , "test_OWString error 46" );
    TEST_ENSURE( OUString("aaa") == OUString("aaa") , "test_OWString error 46" );
    TEST_ENSURE( OUString("bbb") > OUString("aaa") , "test_OWString error 47" );
    TEST_ENSURE( OUString("aaaa") < OUString("bbb") , "test_OWString error 48" );
    TEST_ENSURE( OUString("aaa") < OUString("bbbb")  , "test_OWString error 49" );
    TEST_ENSURE( OUString("aaa") < ( OUString("aaaa") , "test_OWString error 50" );
    TEST_ENSURE( OUString("aaaa") > ( OUString("aaa") , "test_OWString error 51" );
    TEST_ENSURE( OUString("bbbb") > ( OUString("bbb") ) , "test_OWString error 52" );
    TEST_ENSURE( OUString("bbb") == OUString("bbb"), "test_OWString error 53" );
    TEST_ENSURE( OUString("bbb") == OUString("bbb"), "test_OWString error 54" );

    {
        OUString uStr = OUString("Hallo");
        TEST_ENSURE( uStr.compareTo( OUString("Hallo"), 5 ) == 0, "test_OWString error 54.2.1" );
        TEST_ENSURE( uStr.compareTo( OUString("Halloa"), 6 ) < 0 , "test_OWString error 54.2.2" );
        TEST_ENSURE( uStr.compareTo( OUString("1Hallo"), 6 ) > 0, "test_OWString error 54.2.3" );
        TEST_ENSURE( uStr.compareTo( OUString("Aallo"), 5 ) > 0, "test_OWString error 54.2.4" );
        TEST_ENSURE( uStr.compareTo( OUString("Halla"), 5 ) > 0, "test_OWString error 54.2.5" );
        TEST_ENSURE( uStr.compareTo( OUString("Mallo"), 5 ) < 0, "test_OWString error 54.2.6" );
        TEST_ENSURE( uStr.compareTo( OUString("Hallp"), 5 ) < 0, "test_OWString error 54.2.7" );
    }

#if OSL_DEBUG_LEVEL == 0
//YD will fail copy assert on indexes, because ':' returns -1
    s7 = OUString("Hallo jetzt komm ich");
    s8 = s7.copy(0, s7.indexOf((sal_Unicode)':'));
    TEST_ENSURE( s8.isEmpty(), "test_OWString error 55");
    TEST_ENSURE( s8 == OUString(), "test_OWString error 56");
#endif

    // ASCII-Schnittstellen, AB 15.10.1999

    //       "Ich bin ein reiner ASCII-String mit ein paar Zahlen 0123456789 und Zeichen"
    //        |    |    |    |    |    |    |    |    |    |    |    |    |    |    |
    //index   0    5    10   15   20   25   30   35   40   45   50   55   60   65   70
    //
    //       "Ich bin ein weiterer ASCII-String"
    //        |    |    |    |    |    |    |
    //index   0    5    10   15   20   25   30
    sal_Char ascii_str1[] = "Ich bin ein reiner ASCII-String mit ein paar Zahlen 0123456789 und Zeichen";
    sal_Char ascii_str2[] = "Ich bin ein weiterer ASCII-String";
    OUString OWAsciiStr1 = OUString::createFromAscii( ascii_str1 );
    sal_Int32 nLen1 = OWAsciiStr1.getLength();
    TEST_ENSURE( nLen1 == 74, "test_OWString error 57" );
    OUString OWAsciiStr2 = OUString::createFromAscii( ascii_str2 );
    sal_Int32 nLen2 = OWAsciiStr2.getLength();
    TEST_ENSURE( nLen2 == 33, "test_OWString error 58" );

    sal_Int32 nCompareResult11 = OWAsciiStr1.compareToAscii( ascii_str1 );
    TEST_ENSURE( nCompareResult11 == 0, "test_OWString error 59" );
    sal_Int32 nCompareResult12 = OWAsciiStr1.compareToAscii( ascii_str2 );
    TEST_ENSURE( nCompareResult12 < 0, "test_OWString error 60" );

    sal_Int32 nCompareResult21 = OWAsciiStr2.compareToAscii( ascii_str1 );
    TEST_ENSURE( nCompareResult21 > 0, "test_OWString error 61" );
    sal_Int32 nCompareResult22 = OWAsciiStr2.compareToAscii( ascii_str2 );
    TEST_ENSURE( nCompareResult22 == 0, "test_OWString error 62" );

    sal_Int32 nCompareResult12_Len12 = OWAsciiStr1.compareToAscii( ascii_str2, 12 );
    TEST_ENSURE( nCompareResult12_Len12 == 0, "test_OWString error 63" );
    sal_Int32 nCompareResult12_Len13 = OWAsciiStr1.compareToAscii( ascii_str2, 13 );
    TEST_ENSURE( nCompareResult12_Len13 < 0, "test_OWString error 64" );

    sal_Int32 nCompareResult21_Len12 = OWAsciiStr2.compareToAscii( ascii_str1, 12 );
    TEST_ENSURE( nCompareResult21_Len12 == 0, "test_OWString error 65" );
    sal_Int32 nCompareResult21_Len13 = OWAsciiStr2.compareToAscii( ascii_str1, 13 );
    TEST_ENSURE( nCompareResult21_Len13 > 0, "test_OWString error 66" );

    {
        OUString uStr = OUString("Hallo");
        TEST_ENSURE( uStr.equalsAsciiL( "Hallo", 5 ), "test_OWString error 66.1.1" );
        TEST_ENSURE( !uStr.equalsAsciiL( "Hallo1", 6 ), "test_OWString error 66.1.2" );
        TEST_ENSURE( !uStr.equalsAsciiL( "1Hallo", 6 ), "test_OWString error 66.1.3" );
        TEST_ENSURE( !uStr.equalsAsciiL( "aallo", 5 ), "test_OWString error 66.1.2" );
        TEST_ENSURE( !uStr.equalsAsciiL( "Halla", 5 ), "test_OWString error 66.1.3" );

        TEST_ENSURE( uStr.reverseCompareToAsciiL( "Hallo", 5 ) == 0, "test_OWString error 66.2.1" );
        TEST_ENSURE( uStr.reverseCompareToAsciiL( "Halloa", 6 ) > 0 , "test_OWString error 66.2.2" );
        TEST_ENSURE( uStr.reverseCompareToAsciiL( "1Hallo", 6 ) < 0, "test_OWString error 66.2.3" );
        TEST_ENSURE( uStr.reverseCompareToAsciiL( "Aallo", 5 ) > 0, "test_OWString error 66.2.4" );
        TEST_ENSURE( uStr.reverseCompareToAsciiL( "Halla", 5 ) > 0, "test_OWString error 66.2.5" );
        TEST_ENSURE( uStr.reverseCompareToAsciiL( "Mallo", 5 ) < 0, "test_OWString error 66.2.6" );
        TEST_ENSURE( uStr.reverseCompareToAsciiL( "Hallp", 5 ) < 0, "test_OWString error 66.2.7" );
    }

    // toInt64
    OUString s9( OUString(" -3223372036854775807") );
    sal_Int64 ln1 = s9.toInt64();
#if (defined UNX)
    TEST_ENSURE( ln1 == -3223372036854775807LL, "test_OWString error 67" );
#else
    TEST_ENSURE( ln1 == -3223372036854775807, "test_OWString error 67" );
#endif
    OUString s10( OUString("13243A65f1H45") );
    sal_Int64 ln2 = s10.toInt64();
    TEST_ENSURE( ln2 == 13243, "test_OWString error 68" );

    sal_Int64 ln3 = s10.toInt64( 16 );
#if (defined UNX)
    TEST_ENSURE( ln3 == 0x13243A65F1LL, "test_OWString error 69" );
#else
    TEST_ENSURE( ln3 == 0x13243A65F1, "test_OWString error 69" );
#endif
    // Exotic base
    OUString s11( OUString("H4A") );
    sal_Int64 ln4 = s11.toInt64( 23 );
    TEST_ENSURE( ln4 == 23*23*17 + 4 * 23 + 10, "test_OWString error 70" );

    // toInt32
    OUString s12( OUString(" -220368507") );
    sal_Int32 n1 = s12.toInt32();
    TEST_ENSURE( n1 == -220368507, "test_OWString error 71" );

    OUString s13( OUString("4423A61H45") );
    sal_Int64 n2 = s13.toInt32();
    TEST_ENSURE( n2 == 4423, "test_OWString error 72" );

    sal_Int64 n3 = s13.toInt64( 16 );
    TEST_ENSURE( n3 == 0x4423A61, "test_OWString error 73" );

    printf("test_OWString OK !!!\n");
    return;
}

// -----------------------------------------------------------------------------

void oldtests::test_OString2OUStringAndViceVersa()
{
    OString s1("Hallo jetzt komm ich");
    OUString u1 = OStringToOUString(s1, RTL_TEXTENCODING_MS_1252);
    TEST_ENSURE( u1 == "Hallo jetzt komm ich", "test_OString2OWStringAndViceVersa error 1" );
    u1 = OStringToOUString(s1, RTL_TEXTENCODING_IBM_850);
    TEST_ENSURE( u1 == "Hallo jetzt komm ich", "test_OString2OWStringAndViceVersa error 2" );
    u1 = OStringToOUString(s1, RTL_TEXTENCODING_ISO_8859_15);
    TEST_ENSURE( u1 == "Hallo jetzt komm ich", "test_OString2OWStringAndViceVersa error 3" );
    u1 = OStringToOUString(s1, RTL_TEXTENCODING_ASCII_US);
    TEST_ENSURE( u1 == "Hallo jetzt komm ich", "test_OString2OWStringAndViceVersa error 4" );

    OString s2 = OUStringToOString(u1, RTL_TEXTENCODING_MS_1252);
    TEST_ENSURE( s2.equals("Hallo jetzt komm ich"), "test_OString2OWStringAndViceVersa error 5" );
    s2 = OUStringToOString(u1, RTL_TEXTENCODING_IBM_850);
    TEST_ENSURE( s2.equals("Hallo jetzt komm ich"), "test_OString2OWStringAndViceVersa error 6" );
    s2 = OUStringToOString(u1, RTL_TEXTENCODING_ISO_8859_15);
    TEST_ENSURE( s2.equals("Hallo jetzt komm ich"), "test_OString2OWStringAndViceVersa error 7" );
    s2 = OUStringToOString(u1, RTL_TEXTENCODING_ASCII_US);
    TEST_ENSURE( s2.equals("Hallo jetzt komm ich"), "test_OString2OWStringAndViceVersa error 8" );

    printf("test_OString2OWStringAndViceVersa OK !!!\n");
}

} // namespace rtl_OUString

// -----------------------------------------------------------------------------
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( rtl_OUString::oldtests, "rtl_OUString" );

// -----------------------------------------------------------------------------
NOADDITIONAL;


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
