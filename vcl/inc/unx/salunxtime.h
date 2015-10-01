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

#ifndef INCLUDED_VCL_INC_UNX_SALUNXTIME_H
#define INCLUDED_VCL_INC_UNX_SALUNXTIME_H

#if defined LINUX || defined FREEBSD || \
    defined NETBSD || defined OPENBSD || defined DRAGONFLY
#include <sys/time.h>
#elif defined AIX
#include <time.h>
#include <sys/time.h>
#include <strings.h>
#endif
#include <sal/types.h>

inline bool operator >= ( const timeval &t1, const timeval &t2 )
{
    if( t1.tv_sec == t2.tv_sec )
        return t1.tv_usec >= t2.tv_usec;
    return t1.tv_sec > t2.tv_sec;
}

inline bool operator > ( const timeval &t1, const timeval &t2 )
{
    if( t1.tv_sec == t2.tv_sec )
        return t1.tv_usec > t2.tv_usec;
    return t1.tv_sec > t2.tv_sec;
}

inline bool operator == ( const timeval &t1, const timeval &t2 )
{
    return t1.tv_sec == t2.tv_sec && t1.tv_usec == t2.tv_usec;
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

inline timeval &operator += ( timeval &t1, sal_uIntPtr t2 )
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

inline timeval operator - ( const timeval &t1, const timeval &t2 )
{
    timeval t0 = t1;
    return t0 -= t2;
}

#endif // INCLUDED_VCL_INC_UNX_SALUNXTIME_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
