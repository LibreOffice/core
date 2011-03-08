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

#ifndef _SALUNX_H
#define _SALUNX_H

// -=-= #includes =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#if defined LINUX || defined FREEBSD || \
    defined NETBSD || defined OPENBSD || defined DRAGONFLY
#include <sys/time.h>
#elif defined AIX
#include <time.h>
#include <sys/time.h>
#include <strings.h>
#endif
#include <svunx.h>
#include <salstd.hxx>

// -=-= #defines -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#define capacityof(a)   (sizeof(a)/sizeof(*a))

// -=-= inlines =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
inline long Divide( long nDividend, long nDivisor )
{ return (nDividend + nDivisor/2) / nDivisor; }

inline long DPI( long pixel, long mm )
{ return Divide( pixel*254, mm*10 ); }

// -=-= timeval =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
inline int operator >= ( const timeval &t1, const timeval &t2 )
{
    if( t1.tv_sec == t2.tv_sec )
        return t1.tv_usec >= t2.tv_usec;
    return t1.tv_sec > t2.tv_sec;
}

inline int operator > ( const timeval &t1, const timeval &t2 )
{
    if( t1.tv_sec == t2.tv_sec )
        return t1.tv_usec > t2.tv_usec;
    return t1.tv_sec > t2.tv_sec;
}

inline int operator == ( const timeval &t1, const timeval &t2 )
{
    if( t1.tv_sec == t2.tv_sec )
        return t1.tv_usec == t2.tv_usec;
    return FALSE;
}

inline timeval &operator -= ( timeval &t1, const timeval &t2 )
{
    if( t1.tv_usec < t2.tv_usec )
    {
        t1.tv_sec--;
        t1.tv_usec += 1000000;
    }
    t1.tv_sec  -= t2.tv_sec;
    t1.tv_usec -= t2.tv_usec;
    return t1;
}

inline timeval &operator += ( timeval &t1, const timeval &t2 )
{
    t1.tv_sec  += t2.tv_sec;
    t1.tv_usec += t2.tv_usec;
    if( t1.tv_usec > 1000000 )
    {
        t1.tv_sec++;
        t1.tv_usec -= 1000000;
    }
    return t1;
}

inline timeval &operator += ( timeval &t1, ULONG t2 )
{
    t1.tv_sec  += t2 / 1000;
    t1.tv_usec += t2 ? (t2 % 1000) * 1000 : 500;
    if( t1.tv_usec > 1000000 )
    {
        t1.tv_sec++;
        t1.tv_usec -= 1000000;
    }
    return t1;
}

inline timeval operator + ( const timeval &t1, const timeval &t2 )
{
    timeval t0 = t1;
    return t0 += t2;
}

inline timeval operator + ( const timeval &t1, ULONG t2 )
{
    timeval t0 = t1;
    return t0 += t2;
}

inline timeval operator - ( const timeval &t1, const timeval &t2 )
{
    timeval t0 = t1;
    return t0 -= t2;
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
