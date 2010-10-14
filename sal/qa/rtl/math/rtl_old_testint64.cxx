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

#include <testshl/simpleheader.hxx>

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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
