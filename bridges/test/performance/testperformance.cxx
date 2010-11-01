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
#include "precompiled_bridges.hxx"

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
            osl_incrementInterlockedCount( &count );
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
