/*************************************************************************
 *
 *  $RCSfile: rtl_old_testint64.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-05-03 09:10:54 $
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

// fndef    _OSL_DIAGNOSE_H_
// nclude <osl/diagnose.h>
// #endif

#include <sal/types.h>

#define TEST_ENSURE(c, m) CPPUNIT_ASSERT_MESSAGE((m), (c))

// #if OSL_DEBUG_LEVEL > 0
// #define TEST_ENSURE(c, m)   OSL_ENSURE(c, m)
// #else
// #define TEST_ENSURE(c, m)   OSL_VERIFY(c)
// #endif

#include <cppunit/simpleheader.hxx>

// -----------------------------------------------------------------------------
namespace rtl_math
{
    class int64 : public CppUnit::TestFixture
    {
    public:
        void test_int64();

        CPPUNIT_TEST_SUITE( int64 );
        CPPUNIT_TEST( test_int64 );
        CPPUNIT_TEST_SUITE_END( );
    };

void int64::test_int64()
{
#ifndef SAL_INT64_IS_STRUCT
#ifdef UNX
    sal_Int64 i1 = -3223372036854775807LL;
    sal_uInt64 u1 = 5223372036854775807ULL;
#else
    sal_Int64 i1 = -3223372036854775807;
    sal_uInt64 u1 = 5223372036854775807;
#endif
    sal_Int64 i2 = 0;
    sal_uInt64 u2 = 0;
#else
    sal_Int64 i1;
    sal_setInt64(&i1, 3965190145L, -750499787L);

    sal_Int64 i2 = { 0, 0 };

    sal_uInt64 u1;
    sal_setUInt64(&u1, 1651507199UL, 1216161073UL);

    sal_uInt64 u2 = {0, 0 };

#endif
    sal_uInt32 low = 0;
    sal_Int32 high = 0;

    sal_getInt64(i1, &low, &high);
    sal_setInt64(&i2, low, high);

    sal_uInt32 ulow = 0;
    sal_uInt32 uhigh = 0;

    sal_getUInt64(u1, &ulow, &uhigh);
    sal_setUInt64(&u2, ulow, uhigh);

#ifndef SAL_INT64_IS_STRUCT
    TEST_ENSURE( i1 == i2, "test_int64 error 1");

    TEST_ENSURE( u1 == u2, "test_int64 error 2");
#else
    TEST_ENSURE( (i1.Part1 == i2.Part1) && (i1.Part2 == i2.Part2),
                  "test_int64 error 1");

    TEST_ENSURE( (u1.Part1 == u2.Part1) && (u1.Part2 == u2.Part2),
                  "test_int64 error 2");
#endif
    return;
}

} // namespace rtl_math

// -----------------------------------------------------------------------------
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( rtl_math::int64, "rtl_math" );

// -----------------------------------------------------------------------------
NOADDITIONAL;


