/*************************************************************************
 *
 *  $RCSfile: salunx.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: svesik $ $Date: 2001-02-16 01:01:48 $
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
//*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-//
//                                                                            //
// (C) 1997 Star Division GmbH, Hamburg, Germany                              //
//                                                                            //
// $Revision: 1.2 $  $Author: svesik $  $Date: 2001-02-16 01:01:48 $    //
//                                                                            //
// $Workfile:   salunx.h  $                                                   //
//  $Modtime:   14 Aug 1997 13:51:48  $                                       //
//                                                                            //
//*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-*=*-//

#ifndef _SALUNX_H
#define _SALUNX_H

// -=-= #includes =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#if defined SCO || defined LINUX || defined HPUX || defined FREEBSD || defined NETBSD
#include <sys/time.h>
#elif defined AIX
#include <time.h>
#include <sys/time.h>
#include <strings.h>
#elif defined IRIX
#include <sys/time.h>
#include <unistd.h>
#endif

#ifndef _SVUNX_H
#include <svunx.h>
#endif
#ifndef _SALSTD_HXX
#include <salstd.hxx>
#endif

// -=-= #defines -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#define persist
#define final
#define beta
#define alpha
#define unknown
#define nop             ((void*)0)
#define nobreak         ((void*)0)

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

