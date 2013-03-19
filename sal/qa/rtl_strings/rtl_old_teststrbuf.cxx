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

#include <rtl/strbuf.hxx>

#include <rtl/ustrbuf.hxx>

#include <testshl/simpleheader.hxx>

using ::rtl::OUString;
using ::rtl::OString;
using ::rtl::OUStringBuffer;
using ::rtl::OStringBuffer;
#define TEST_ENSURE(c, m) CPPUNIT_ASSERT_MESSAGE((m), (c))


namespace rtl_OStringBuffer
{
    class oldtests : public CppUnit::TestFixture
    {
    public:
        void test_OStringBuffer();

        CPPUNIT_TEST_SUITE( oldtests );
        CPPUNIT_TEST( test_OStringBuffer );
        CPPUNIT_TEST_SUITE_END( );
    };


void oldtests::test_OStringBuffer()
{
    //         "Mein erster RTL OString\n"
    //          |    |    |    |    |
    //  Index   0    5    10   15   20
    OString s1("Mein erster RTL OString\n");

    OStringBuffer b1(s1);

    TEST_ENSURE( b1.getCapacity() == 16 + s1.getLength(), "test_OStringBuffer error 1");

    b1.insert(b1.getLength() - 1, "Buffer");
    s1 = "Mein erster RTL OStringBuffer\n";
    TEST_ENSURE( s1 == b1.getStr(), "test_OStringBuffer error 2");

    b1.insert(b1.getLength() - 1, " ist viel zu gross fuer den alten Buffer");
    TEST_ENSURE( b1.getCapacity() == b1.getLength(), "test_OStringBuffer error 3");

    OStringBuffer b2(30);

    s1 = "false";
    sal_Bool b = sal_False;
    b2.append(b);
    TEST_ENSURE( s1 == b2.getStr(), "test_OStringBuffer error 4");

    sal_Int32 n = 123456789L;
    s1 += " 123456789";
    b2.append(" ");
    b2.append(n);
    TEST_ENSURE( s1 == b2.getStr(), "test_OStringBuffer error 5");

#ifdef SAL_UNX
    sal_Int64 m = -3223372036854775807LL;
#else
    sal_Int64 m = -3223372036854775807;
#endif
    s1 += " -3223372036854775807";
    b2.append(" ");
    b2.append(m);
    TEST_ENSURE( s1 == b2.getStr(), "test_OStringBuffer error 6");

    OString s2(b2.makeStringAndClear());
    TEST_ENSURE( s1 == s2, "test_OStringBuffer error 7");

    b2.ensureCapacity(50);
    TEST_ENSURE( b2.getCapacity() == 50, "test_OStringBuffer error 8");

    b2.append("Hier fuege ich jetzt ein > <\n");
    b2.insert(26, " Hallo");
    s2 = "Hier fuege ich jetzt ein > Hallo <\n";
    TEST_ENSURE( s2 == b2.getStr(), "test_OStringBuffer error 9");

    b2.insert(26, b);
    b2.insert(26, " ");
    s2 = "Hier fuege ich jetzt ein > false Hallo <\n";
    TEST_ENSURE( s2 == b2.getStr(), "test_OStringBuffer error 10");

    b2.insert(26, n);
    b2.insert(26, " ");
    s2 = "Hier fuege ich jetzt ein > 123456789 false Hallo <\n";
    TEST_ENSURE( s2 == b2.getStr(), "test_OStringBuffer error 11");

    b2.insert(26, m);
    b2.insert(26, " ");
    s2 = "Hier fuege ich jetzt ein > -3223372036854775807 123456789 false Hallo <\n";
    TEST_ENSURE( s2 == b2.getStr(), "test_OStringBuffer error 12");

    printf("test_OStringBuffer OK !!!\n");
    return;
}
} // namespace rtl_OStringBuffer

// -----------------------------------------------------------------------------

namespace rtl_OUStringBuffer
{
    class oldtests : public CppUnit::TestFixture
    {
    public:
        void test_OUStringBuffer();

        CPPUNIT_TEST_SUITE( oldtests );
        CPPUNIT_TEST( test_OUStringBuffer );
        CPPUNIT_TEST_SUITE_END( );
    };


void oldtests::test_OUStringBuffer()
{
    //          "Mein erster RTL OUString\n"
    //           |    |    |    |    |
    //  Index    0    5    10   15   20
    OUString s1(OUString("Mein erster RTL OUString\n"));

    OUStringBuffer b1(s1);

    TEST_ENSURE( b1.getCapacity() == 16 + s1.getLength(), "test_OWStringBuffer error 1");

    b1.insert(b1.getLength() - 1, OUString("Buffer"));
    s1 = OUString("Mein erster RTL OUStringBuffer\n");
    TEST_ENSURE( s1 == b1.getStr(), "test_OWStringBuffer error 2");

    b1.insert(b1.getLength() - 1, OUString(" ist viel zu gross fuer den alten Buffer"));
    //TEST_ENSURE( b1.getCapacity() == b1.getLength(), "test_OWStringBuffer error 3");

    OUStringBuffer b2(30);

    s1 = OUString("false");
    sal_Bool b = sal_False;
    b2.append(b);
    TEST_ENSURE( s1 == b2.getStr(), "test_OWStringBuffer error 4");

    sal_Int32 n = 123456789L;
    s1 += " 123456789";
    b2.append(OUString(" "));
    b2.append(n);
    TEST_ENSURE( s1 == b2.getStr(), "test_OWStringBuffer error 5");

#ifdef SAL_UNX
    sal_Int64 m = -3223372036854775807LL;
#else
    sal_Int64 m = -3223372036854775807;
#endif
    s1 += " -3223372036854775807";
    b2.append(OUString(" "));
    b2.append(m);
    TEST_ENSURE( s1 == b2.getStr(), "test_OWStringBuffer error 6");

    OUString s2(b2.makeStringAndClear());
    TEST_ENSURE( s1 == s2, "test_OWStringBuffer error 7");

    b2.ensureCapacity(50);
    TEST_ENSURE( b2.getCapacity() == 50, "test_OWStringBuffer error 8");

    b2.append(OUString("Hier fuege ich jetzt ein > <\n"));
    b2.insert(26, OUString(" Hallo"));
    s2 = OUString("Hier fuege ich jetzt ein > Hallo <\n");
    TEST_ENSURE( s2 == b2.getStr(), "test_OWStringBuffer error 9");

    b2.insert(26, b);
    b2.insert(26, OUString(" "));
    s2 = OUString("Hier fuege ich jetzt ein > false Hallo <\n");
    TEST_ENSURE( s2 == b2.getStr(), "test_OWStringBuffer error 10");

    b2.insert(26, n);
    b2.insert(26, OUString(" "));
    s2 = OUString("Hier fuege ich jetzt ein > 123456789 false Hallo <\n");
    TEST_ENSURE( s2 == b2.getStr(), "test_OWStringBuffer error 11");

    b2.insert(26, m);
    b2.insert(26, OUString(" "));
    s2 = OUString("Hier fuege ich jetzt ein > -3223372036854775807 123456789 false Hallo <\n");
    TEST_ENSURE( s2 == b2.getStr(), "test_OWStringBuffer error 12");

    // ASCII-Schnittstelle, AB 15.10.1999
    OUString s3(OUString("Noch'n RTL OUString"));
    OUStringBuffer b3(s3);
    sal_Char aAsciiStr[] = " mit appendetem ASCII\n";
    b3.appendAscii( aAsciiStr );
    s3 = OUString("Noch'n RTL OUString mit appendetem ASCII\n");
    TEST_ENSURE( b3.getStr() == s3 , "test_OWStringBuffer error 13");



    printf("test_OWStringBuffer OK !!!\n");
    return;
}

} // namespace rtl_OUStringBuffer


// -----------------------------------------------------------------------------
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( rtl_OUStringBuffer::oldtests, "rtl_OUStringBuffer" );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( rtl_OStringBuffer::oldtests, "rtl_OStringBuffer" );

// -----------------------------------------------------------------------------
NOADDITIONAL;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
