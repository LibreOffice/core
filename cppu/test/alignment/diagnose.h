/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: diagnose.h,v $
 * $Revision: 1.7 $
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
#ifndef DIAGNOSE_H
#define DIAGNOSE_H

#include <sal/types.h>
#include <stdio.h>

#if defined(__GNUC__) && (defined(LINUX) || defined(FREEBSD)) && defined(INTEL)
#define ALIGNMENT(s, n) __alignof__ (s)
#else
#define ALIGNMENT(s, n) n
#endif

#define OFFSET_OF( s, m ) ((sal_Size)((char *)&((s *)16)->m -16))

#define BINTEST_VERIFY( c ) \
    if (! (c)) { fprintf( stderr, "### binary compatibility test failed: " #c " [line %d]!!!\n", __LINE__ ); abort(); }

#if OSL_DEBUG_LEVEL > 1

#define BINTEST_VERIFYOFFSET( s, m, n ) \
    fprintf( stderr, "> OFFSET_OF(" #s ", " #m ") = %lu\n", static_cast< unsigned long >(OFFSET_OF(s, m)) ); \
    if (OFFSET_OF(s, m) != n) { fprintf( stderr, "### OFFSET_OF(" #s ", " #m ") = %d instead of expected %d!!!\n", OFFSET_OF(s, m), n ); abort(); }
#define BINTEST_VERIFYSIZE( s, n ) \
    fprintf( stderr, "> sizeof (" #s ") = %lu\n", static_cast< unsigned long >(sizeof(s)) ); \
    if (sizeof(s) != n) { fprintf( stderr, "### sizeof(" #s ") = %d instead of expected %d!!!\n", sizeof(s), n ); abort(); }
#define BINTEST_VERIFYALIGNMENT( s, n ) \
    fprintf( stderr, "> alignment of " #s " = %d\n", ALIGNMENT(s, n) ); \
    if (ALIGNMENT(s, n) != n) { fprintf( stderr, "### alignment of " #s " = %d instead of expected %d!!!\n", ALIGNMENT(s, n), n ); abort(); }

#else

#define BINTEST_VERIFYOFFSET( s, m, n ) \
    if (OFFSET_OF(s, m) != n) { fprintf( stderr, "### OFFSET_OF(" #s ", " #m ") = %lu instead of expected %lu!!!\n", static_cast< unsigned long >(OFFSET_OF(s, m)), static_cast< unsigned long >(n) ); abort(); }
#define BINTEST_VERIFYSIZE( s, n ) \
    if (sizeof(s) != n) { fprintf( stderr, "### sizeof(" #s ") = %lu instead of expected %lu!!!\n", static_cast< unsigned long >(sizeof(s)), static_cast< unsigned long >(n) ); abort(); }
#define BINTEST_VERIFYALIGNMENT( s, n ) \
    if (ALIGNMENT(s, n) != n) { fprintf( stderr, "### alignment of " #s " = %d instead of expected %d!!!\n", ALIGNMENT(s, n), n ); abort(); }

#endif

#endif
