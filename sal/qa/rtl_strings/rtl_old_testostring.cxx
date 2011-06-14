/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sal.hxx"

// LLA:
// this file is converted to use with testshl2
// original was placed in sal/test/textenc.cxx

#include <string.h>
#include <stdio.h>

#include <rtl/string.hxx>

#include <testshl/simpleheader.hxx>

#define TEST_ENSURE(c, m) CPPUNIT_ASSERT_MESSAGE((m), (c))
using ::rtl::OString;
namespace rtl_OString
{
    class oldtests : public CppUnit::TestFixture
    {
    public:
        void test_OString();

        CPPUNIT_TEST_SUITE( oldtests );
        CPPUNIT_TEST( test_OString );
        CPPUNIT_TEST_SUITE_END( );
    };


#ifdef WNT
#pragma warning( disable : 4723 )
#endif

void oldtests::test_OString()
{
    TEST_ENSURE( sal_True, "_USENAMEPSACE defined");

    //          "Mein erster RTL OString\n"
    //           |    |    |    |    |
    //  Index    0    5    10   15   20
    OString s1("Mein erster RTL OString\n");
    TEST_ENSURE( s1 == "Mein erster RTL OString\n", "test_OString error 1");
    TEST_ENSURE( s1.getLength() == 24, "test_OString error 2");

    OString s2 = s1;
    TEST_ENSURE( s2[16] == 'O', "test_OString error 3");
    TEST_ENSURE( s2.equals(s1), "test_OString error 4");
    TEST_ENSURE( s2.indexOf('O') == 16, "test_OString error 5");
    TEST_ENSURE( s2.indexOf('O', 5) == 16, "test_OString error 5a");
    TEST_ENSURE( s2.lastIndexOf('r') == 19, "test_OString error 6");
    TEST_ENSURE( s2[19] == 'r', "test_OString error 7");
    TEST_ENSURE( s2[23] == '\n', "test_OString error 8");
    TEST_ENSURE( s2.lastIndexOf('\n') == 23, "test_OString error 9");
    TEST_ENSURE( s2.lastIndexOf('M') == 0, "test_OString error 10");
    TEST_ENSURE( s2.lastIndexOf('t', s2.getLength() - 8) == 8, "test_OString error 9");


    //          "Mein erster RTL OString ist ein String aus der RTL Library\n"
    //           |    |    |    |    |    |    |    |    |    |    |    |
    //  Index    0    5    10   15   20   25   30   35   40   45   50   55
    OString s3 = s2.copy(0, s2.getLength() - 1);
    OString s4 = s3.concat(" ist ein String aus der RTL Library\n");
    TEST_ENSURE( s4.getLength() == 59, "test_OString error 11");

    s1 = s4.copy(0, 38);
    OString s5;
    s5 = s1 + " aus der RTL Library\n";
    TEST_ENSURE( s5.compareTo(s4) == 0 , "test_OString error 12");
    TEST_ENSURE( s5.indexOf("RTL") == 12, "test_OString error 13");
    TEST_ENSURE( s5.lastIndexOf("RTL") == 47, "test_OString error 13");

    sal_Bool b = sal_False;
    OString s6 = s5.valueOf(b);
    TEST_ENSURE( s6.compareTo("false") == 0, "test_OString error 14");
    s6 = s5.valueOf('H');
    TEST_ENSURE( s6.compareTo("H") == 0, "test_OString error 15");
    sal_Int32 n = 123456789L;
    s6 = s5.valueOf(n);
    TEST_ENSURE( s6.compareTo("123456789") == 0, "test_OString error 16");

#ifndef SAL_OS2
#ifdef SAL_UNX
    sal_Int64 m = -3223372036854775807LL;
#elif defined(SAL_OS2)
    sal_Int64 m;
    sal_setInt64(&m, 3965190145L, -750499787L);
#else
    sal_Int64 m = -3223372036854775807;
#endif
    s6 = s5.valueOf(m);
    TEST_ENSURE( s6.compareTo("-3223372036854775807") == 0, "test_OString error 17");
#endif

    OString s7("HaLLo");
    s7 = s7.toAsciiLowerCase();
    TEST_ENSURE( s7 == "hallo", "test_OString error 19");
    s7 = s7.toAsciiUpperCase();
    TEST_ENSURE( s7 == "HALLO", "test_OString error 20");

    OString s8("HaLLo ICH BIn eIn StRiNg");
    s7 = s8.toAsciiLowerCase();

    TEST_ENSURE( s8.equalsIgnoreAsciiCase(s7), "test_OString error 21");

    s8 = s7.toAsciiUpperCase();
    TEST_ENSURE( s8 == "HALLO ICH BIN EIN STRING", "test_OString error 22");

    s7 = "          ";
    s8 = s7 + s8 + "          ";
    TEST_ENSURE( s8 == "          HALLO ICH BIN EIN STRING          ",
                  "test_OString error 23");

    s7 = s8.trim();
    TEST_ENSURE( s7 == "HALLO ICH BIN EIN STRING", "test_OString error 24");
    TEST_ENSURE( strcmp(s7.getStr(), "HALLO ICH BIN EIN STRING") == 0,
                  "test_OString error 25");

    s7 = "Hallo";
    s8 = "aber Hallo";

    TEST_ENSURE( s7 < s8, "test_OString error 26");
    TEST_ENSURE( s8 > s7, "test_OString error 27");
    TEST_ENSURE( s7 != s8, "test_OString error 28");
    TEST_ENSURE( s7 != "blabla", "test_OString error 29");
    TEST_ENSURE( "blabla" != s7, "test_OString error 30");

    s8 = "Hallo";
    TEST_ENSURE( s7 <= s8, "test_OString error 31");
    TEST_ENSURE( s7 >= s8, "test_OString error 32");

    s8 = s8.replace('l', 'r');
    TEST_ENSURE( s8 == "Harro", "test_OString error 33");

    sal_Int32 nIndex = 0;
    s8 = "|hallo1|hallo2|hallo3|hallo4|hallo5|hallo6|hallo7|hallo8|";
    TEST_ENSURE( s8.getToken(3,'|', nIndex) == "hallo3", "test_OString error 40");

    char* Tokens[10] = { "", "hallo1", "hallo2", "hallo3", "hallo4",
                         "hallo5", "hallo6", "hallo7", "hallo8", "" };

    nIndex = 0;
    sal_Int32 i = 0;
    do
    {
        TEST_ENSURE( s8.getToken(0,'|',nIndex) == Tokens[i], "test_OString error 40e");
        i++;
    }
    while ( nIndex >= 0 );

    s7 = "";
    s7 += s8;
    TEST_ENSURE( s7 == s8, "test_OString error 41");

    s7 = s8.replaceAt(8, 6, "mmmmmmmmmm");
    TEST_ENSURE( s7.getLength() == 61, "test_OString error 42");

    s8 = s7.replaceAt(8, 11, "");
    TEST_ENSURE( s8.getLength() == 50, "test_OString error 43");

    s7 = s8.replaceAt(8, 0, "hallo2|");
    TEST_ENSURE( s7.getLength() == 57, "test_OString error 44");

    sal_Int32 pos = 0;
    while ((pos = s7.indexOf("|")) >= 0)
    {
        s8 = s7.replaceAt(pos, 1, "**");
        s7 = s8;
    }
    TEST_ENSURE( s7.getLength() == 66, "test_OString error 45");

    TEST_ENSURE( OString( "aaa" ).compareTo( OString( "bbb" ) ) < 0, "test_OString error 46" );
    TEST_ENSURE( OString( "aaa" ).compareTo( OString( "aaa" ) ) == 0, "test_OString error 47" );
    TEST_ENSURE( OString( "bbb" ).compareTo( OString( "aaa" ) ) > 0, "test_OString error 48" );
    TEST_ENSURE( OString( "aaaa" ).compareTo( OString( "bbb" ) ) < 0, "test_OString error 49" );
    TEST_ENSURE( OString( "aaa" ).compareTo( OString( "bbbb" ) ) < 0, "test_OString error 50" );
    TEST_ENSURE( OString( "aaa" ).compareTo( OString( "aaaa" ) ) < 0, "test_OString error 51" );
    TEST_ENSURE( OString( "aaaa" ).compareTo( OString( "aaa" ) ) > 0, "test_OString error 52" );
    TEST_ENSURE( OString( "bbbb" ).compareTo( OString( "bbb" ) ) > 0, "test_OString error 53" );
    TEST_ENSURE( OString( "bbb" ) == OString( "bbb" ), "test_OString error 54" );
    TEST_ENSURE( OString( "bbb" ) == "bbb", "test_OString error 55" );

/*
 * As clarified in #104229#, calling copy with invalid arguments causes
 * undefined behaviour, so the following test does no longer work:

    s7 = "Hallo jetzt komm ich";
    s8 = s7.copy(0, s7.indexOf(':'));
    TEST_ENSURE( s8.getLength() == 0, "test_OString error 56");
    TEST_ENSURE( s8.compareTo("") == 0, "test_OString error 57");
*/

    double f = OString("1.7e-10").toDouble();
    TEST_ENSURE(f > 1E-10 && f < 2E-10, "1.7e-10 problem");
    f = OString("1.7e+10").toDouble();
    TEST_ENSURE(f > 1E10 && f < 2E10, "1.7e+10 problem");
    f = OString("1.7e10").toDouble();
    TEST_ENSURE(f > 1E10 && f < 2E10, "1.7e308 problem");

    {
        float f0 = 0;
        float f1 = 1;
        float fInf = f1 / f0;
        OString aStr1(OString::valueOf(fInf));
        OString aStr2("1.#INF");
        bool bSuccess = aStr1 == aStr2;
        if (!bSuccess)
            printf("ERROR: OString::valueOf(1f/0f): %s\n", aStr1.getStr());
        TEST_ENSURE(bSuccess, "OString::valueOf(1f/0f)");
    }

    printf("test_OString OK !!!\n");
    return;
}

} // namespace rtl_OString

// -----------------------------------------------------------------------------
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( rtl_OString::oldtests, "rtl_OString" );

// -----------------------------------------------------------------------------
NOADDITIONAL;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
