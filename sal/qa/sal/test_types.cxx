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

#include "precompiled_sal.hxx"
#include "sal/config.h"

#include <cstddef>
#include <stdio.h> // C99 snprintf not necessarily in <cstdio>
#include <string.h> // wntmsci10 does not know <cstring> std::strcmp

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include "sal/types.h"

namespace {

template< typename T > void testPrintf(
    char const * result, char const * format, T argument)
{
    std::size_t const bufsize = 1000;
    char buf[bufsize];
    int n = snprintf(buf, bufsize, format, argument);
    CPPUNIT_ASSERT(n >= 0 && sal::static_int_cast< unsigned int >(n) < bufsize);
    CPPUNIT_ASSERT(strcmp(buf, result) == 0);
}

class Test: public CppUnit::TestFixture {
public:
    void test();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();
};

void Test::test() {
    testPrintf("-2147483648", "%" SAL_PRIdINT32, SAL_MIN_INT32);
    testPrintf("4294967295", "%" SAL_PRIuUINT32, SAL_MAX_UINT32);
    testPrintf("ffffffff", "%" SAL_PRIxUINT32, SAL_MAX_UINT32);
    testPrintf("FFFFFFFF", "%" SAL_PRIXUINT32, SAL_MAX_UINT32);
    testPrintf("-9223372036854775808", "%" SAL_PRIdINT64, SAL_MIN_INT64);
    testPrintf("18446744073709551615", "%" SAL_PRIuUINT64, SAL_MAX_UINT64);
    testPrintf("ffffffffffffffff", "%" SAL_PRIxUINT64, SAL_MAX_UINT64);
    testPrintf("FFFFFFFFFFFFFFFF", "%" SAL_PRIXUINT64, SAL_MAX_UINT64);
    testPrintf("123", "%" SAL_PRI_SIZET "u", static_cast< std::size_t >(123));
    testPrintf(
        "-123", "%" SAL_PRI_PTRDIFFT "d", static_cast< std::ptrdiff_t >(-123));
    testPrintf("-123", "%" SAL_PRIdINTPTR, static_cast< sal_IntPtr >(-123));
    testPrintf("123", "%" SAL_PRIuUINTPTR, static_cast< sal_uIntPtr >(123));
    testPrintf("abc", "%" SAL_PRIxUINTPTR, static_cast< sal_uIntPtr >(0xabc));
    testPrintf("ABC", "%" SAL_PRIXUINTPTR, static_cast< sal_uIntPtr >(0xabc));
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
