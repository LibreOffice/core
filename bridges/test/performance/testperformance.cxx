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

#include <stdio.h>
#include <math.h>

#include <osl/interlck.h>
#include <osl/mutex.hxx>
#include <osl/semaphor.h>

#include <rtl/string.hxx>
#include <rtl/byteseq.hxx>

#include <com/sun/star/uno/Sequence.hxx>

#ifdef SAL_W32
#include <windows.h>
#else
#include <sys/times.h>
#endif
#ifndef ULONG_MAX
#define ULONG_MAX 0xffffffff
#endif

using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star::uno;

static inline sal_uInt32 getSystemTicks()
{
#ifdef SAL_W32
    return (sal_uInt32)GetTickCount();
#else // only UNX supported for now
    static sal_uInt32   nImplTicksPerSecond = 0;
    static double       dImplTicksPerSecond;
    static double       dImplTicksULONGMAX;

    struct tms          aTms;
    sal_uInt32 nTicks = (sal_uInt32)times( &aTms );

    if ( !nImplTicksPerSecond )
    {
        nImplTicksPerSecond = CLK_TCK;
        dImplTicksPerSecond = nImplTicksPerSecond;
        dImplTicksULONGMAX  = (double)(sal_uInt32)ULONG_MAX;
    }

    double fTicks = nTicks;
    fTicks *= 1000;
    fTicks /= dImplTicksPerSecond;
    fTicks = fmod (fTicks, dImplTicksULONGMAX);

    return (sal_uInt32)fTicks;
#endif
}

class MyTimer
{
public:
    MyTimer(  const OString &descString ) :
        nStart( getSystemTicks() ),
        m_descString( descString )
        {
        }
    ~MyTimer( )
        {
            printf( "%f s : %s\n", (getSystemTicks() -nStart) / 1000., m_descString.getStr() );
        }
private:
    sal_uInt32 nStart;
    OString m_descString;
};

void main()
{
    // interlocked count
    {
        MyTimer timer( "performance - 1000*10000 interlocked count" );
        oslInterlockedCount count;
        for( int i = 0 ; i < 1000*10000 ; i ++  )
        {
            osl_atomic_increment( &count );
        }
    }
    {
        OString myDummyString( "blubber" );
        MyTimer timer( "performance - 1000*10000 acquiring/releasing a refcounted string(without destruction)" );
        for( int i = 0 ; i < 1000*10000 ; i ++  )
        {
            OString myNextDummyString = myDummyString ;
        }
    }

    printf( "--------------------\n" );
    {
        Mutex mutex;
        MyTimer timer( "performance - 1000*10000 acquiring/releasing an osl::Mutex" );
        for( int i = 0 ; i < 1000*10000 ; i ++  )
        {
            MutexGuard guard( mutex );
        }
    }

    {
        oslSemaphore sema = osl_createSemaphore(1);
        MyTimer timer( "performance - 1000*10000 acquiring/releasing an osl::Semaphore" );
        for( int i = 0 ; i < 1000*10000 ; i ++  )
        {
            osl_acquireSemaphore( sema );
            osl_releaseSemaphore( sema );
        }
    }

    printf( "--------------------\n" );
    {
        MyTimer timer( "performance - 1000*10000 rtl::ByteSequence(500)" );
        for( int i = 0 ; i < 1000*1000 ; i ++  )
        {
            ByteSequence seq(500);
        }
    }

    {
        MyTimer timer( "performance - 1000*1000 rtl::ByteSequence(500,BYTESEQ_NODEFAULT)" );
        for( int i = 0 ; i < 1000*1000 ; i ++  )
        {
            ByteSequence seq(500, BYTESEQ_NODEFAULT);
        }
    }
    {
        MyTimer timer( "performance - 1000*1000 com::sun::star::uno::Sequence< sal_Int8 > (500)" );
        for( int i = 0 ; i < 1000*1000 ; i ++  )
        {
            Sequence< sal_Int8> seq(500);
        }
    }
    {
        MyTimer timer( "performance - 1000*1000 rtl_freeMemory( rtl_allocateMemory( 512 ) )" );
        for( int i = 0 ; i < 1000*1000 ; i ++  )
        {
            rtl_freeMemory( rtl_allocateMemory( 512 ) );
        }
    }

    printf( "--------------------\n" );
    {
        MyTimer timer( "performance - 1000*1000 byte  string construction/destruction" );
        for( int i = 0 ; i < 1000*1000 ; i ++  )
        {
            OString textEnc( "this is a test string" );
        }
    }

    {
        MyTimer timer( "performance - 1000*1000 unicode string construction/destruction" );
        for( int i = 0 ; i < 1000*1000 ; i ++  )
        {
            OUString textEnc( RTL_CONSTASCII_USTRINGPARAM( "this is a test string" ) );
        }
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
