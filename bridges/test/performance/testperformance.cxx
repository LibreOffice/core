/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: testperformance.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-26 07:04:41 $
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
