/*************************************************************************
 *
 *  $RCSfile: rtl_old_teststrbuf.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-05-03 09:19:21 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// LLA:
// this file is converted to use with testshl2
// original was placed in sal/test/textenc.cxx


// -----------------------------------------------------------------------------

#include <string.h>
#include <stdio.h>

// #ifndef _OSL_DIAGNOSE_H_
// #include <osl/diagnose.h>
// #endif

#ifndef _RTL_STRBUF_HXX
#include <rtl/strbuf.hxx>
#endif

#ifndef _RTL_WSTRBUF_HXX
#include <rtl/ustrbuf.hxx>
#endif

#include <cppunit/simpleheader.hxx>

using namespace rtl;

#define TEST_ENSURE(c, m) CPPUNIT_ASSERT_MESSAGE((m), (c))

// #if OSL_DEBUG_LEVEL > 0
// #define TEST_ENSHURE(c, m)   OSL_ENSURE(c, m)
// #else
// #define TEST_ENSHURE(c, m)   OSL_VERIFY(c)
// #endif

// -----------------------------------------------------------------------------
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

#ifndef SAL_OS2
#ifdef SAL_UNX
    sal_Int64 m = -3223372036854775807LL;
#elif defined(SAL_OS2)
    sal_Int64 m;
    sal_setInt64(&m, 3965190145L, -750499787L);
#else
    sal_Int64 m = -3223372036854775807;
#endif
    s1 += " -3223372036854775807";
    b2.append(" ");
    b2.append(m);
    TEST_ENSURE( s1 == b2.getStr(), "test_OStringBuffer error 6");
#endif

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

#ifndef SAL_OS2
    b2.insert(26, m);
    b2.insert(26, " ");
    s2 = "Hier fuege ich jetzt ein > -3223372036854775807 123456789 false Hallo <\n";
    TEST_ENSURE( s2 == b2.getStr(), "test_OStringBuffer error 12");
#endif

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
    OUString s1(OUString::createFromAscii("Mein erster RTL OUString\n"));

    OUStringBuffer b1(s1);

    TEST_ENSURE( b1.getCapacity() == 16 + s1.getLength(), "test_OWStringBuffer error 1");

    b1.insert(b1.getLength() - 1, OUString::createFromAscii("Buffer"));
    s1 = OUString::createFromAscii("Mein erster RTL OUStringBuffer\n");
    TEST_ENSURE( s1 == b1.getStr(), "test_OWStringBuffer error 2");

    b1.insert(b1.getLength() - 1, OUString::createFromAscii(" ist viel zu gross fuer den alten Buffer"));
    //TEST_ENSURE( b1.getCapacity() == b1.getLength(), "test_OWStringBuffer error 3");

    OUStringBuffer b2(30);

    s1 = OUString::createFromAscii("false");
    sal_Bool b = sal_False;
    b2.append(b);
    TEST_ENSURE( s1 == b2.getStr(), "test_OWStringBuffer error 4");

    sal_Int32 n = 123456789L;
    s1 += OUString::createFromAscii(" 123456789");
    b2.append(OUString::createFromAscii(" "));
    b2.append(n);
    TEST_ENSURE( s1 == b2.getStr(), "test_OWStringBuffer error 5");

#ifndef SAL_OS2
#ifdef SAL_UNX
    sal_Int64 m = -3223372036854775807LL;
#elif defined(SAL_OS2)
    sal_Int64 m;
    sal_setInt64(&m, 3965190145L, -750499787L);
#else
    sal_Int64 m = -3223372036854775807;
#endif
    s1 += OUString::createFromAscii(" -3223372036854775807");
    b2.append(OUString::createFromAscii(" "));
    b2.append(m);
    TEST_ENSURE( s1 == b2.getStr(), "test_OWStringBuffer error 6");
#endif

    OUString s2(b2.makeStringAndClear());
    TEST_ENSURE( s1 == s2, "test_OWStringBuffer error 7");

    b2.ensureCapacity(50);
    TEST_ENSURE( b2.getCapacity() == 50, "test_OWStringBuffer error 8");

    b2.append(OUString::createFromAscii("Hier fuege ich jetzt ein > <\n"));
    b2.insert(26, OUString::createFromAscii(" Hallo"));
    s2 = OUString::createFromAscii("Hier fuege ich jetzt ein > Hallo <\n");
    TEST_ENSURE( s2 == b2.getStr(), "test_OWStringBuffer error 9");

    b2.insert(26, b);
    b2.insert(26, OUString::createFromAscii(" "));
    s2 = OUString::createFromAscii("Hier fuege ich jetzt ein > false Hallo <\n");
    TEST_ENSURE( s2 == b2.getStr(), "test_OWStringBuffer error 10");

    b2.insert(26, n);
    b2.insert(26, OUString::createFromAscii(" "));
    s2 = OUString::createFromAscii("Hier fuege ich jetzt ein > 123456789 false Hallo <\n");
    TEST_ENSURE( s2 == b2.getStr(), "test_OWStringBuffer error 11");

#ifndef SAL_OS2
    b2.insert(26, m);
    b2.insert(26, OUString::createFromAscii(" "));
    s2 = OUString::createFromAscii("Hier fuege ich jetzt ein > -3223372036854775807 123456789 false Hallo <\n");
    TEST_ENSURE( s2 == b2.getStr(), "test_OWStringBuffer error 12");
#endif

    // ASCII-Schnittstelle, AB 15.10.1999
    OUString s3(OUString::createFromAscii("Noch'n RTL OUString"));
    OUStringBuffer b3(s3);
    sal_Char aAsciiStr[] = " mit appendetem ASCII\n";
    //sal_Char aAsciiStr[] = " mit angefügtem ASCII";
    b3.appendAscii( aAsciiStr );
    s3 = OUString::createFromAscii("Noch'n RTL OUString mit appendetem ASCII\n");
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
