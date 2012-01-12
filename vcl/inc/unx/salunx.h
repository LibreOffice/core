/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SALUNX_H
#define _SALUNX_H

// -=-= #includes =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#if defined SCO || defined LINUX || defined HPUX || defined FREEBSD || defined NETBSD
#include <sys/time.h>
#elif defined AIX
#include <time.h>
#include <sys/time.h>
#include <strings.h>
#endif
#include <unx/svunx.h>
#include <unx/salstd.hxx>

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
    return sal_False;
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

inline timeval operator + ( const timeval &t1, const timeval &t2 )
{
    timeval t0 = t1;
    return t0 += t2;
}

inline timeval operator + ( const timeval &t1, sal_uIntPtr t2 )
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

