/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: diagnose.h,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2007-10-15 12:20:33 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
