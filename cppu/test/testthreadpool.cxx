/*************************************************************************
 *
 *  $RCSfile: testthreadpool.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-14 09:28:08 $
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
#include <osl/diagnose.h>

#ifndef _OSL_TIME_H_
#include <osl/time.h>
#endif

#include <rtl/uuid.h>

#include <uno/threadpool.h>

#include <vos/thread.hxx>

#define TEST_ENSURE OSL_ENSURE

using namespace ::vos;


class OThread1 : public OThread
{
public:
    OThread1( sal_uInt8 *pCallerUuid );
    virtual void run();

public:
    sal_uInt8 *m_pCallerUuid;
    sal_Int8 *m_pThreadIdentifier;
    sal_Int32 m_nThreadIdentifierLength;

    void *m_pThreadData;
};

OThread1::OThread1( sal_uInt8 *pCallerUuid ) :
    m_pCallerUuid( pCallerUuid ),
    m_pThreadData( (void*) 1 ),
    m_pThreadIdentifier( 0 ),
    m_nThreadIdentifierLength( 0 )
{

}

void OThread1::run()
{

    uno_threadpool_Ticket *pTicket = uno_threadpool_createTicket( m_pCallerUuid );

    uno_threadIdent_retrieve( &m_pThreadIdentifier , &m_nThreadIdentifierLength );

    uno_threadpool_waitOnTicket( pTicket , &m_pThreadData );

    uno_threadIdent_revoke();
}


void SAL_CALL doIt( void *pThreadData )
{
    *( sal_Int32 *) pThreadData = 2;
}

void testthreadpool()
{
    printf( "Testing threadpool ..." );
    fflush( stdout );

    sal_uInt8 pCallerUuid1[16];
    sal_uInt8 pCallerUuid2[16];
    rtl_createUuid( pCallerUuid1, 0 , sal_True );
    rtl_createUuid( pCallerUuid2, 0 , sal_True );

    //------------
    // Test reply
    //------------
    {
        OThread1 thread1( pCallerUuid1 );

        thread1.create();

        // do a busy wait
        while( ! thread1.m_pThreadIdentifier && ! thread1.m_nThreadIdentifierLength );

        void *pThreadData = (void*)0xdeadbabe;
        uno_threadpool_reply( thread1.m_pThreadIdentifier ,
                              thread1.m_nThreadIdentifierLength,
                              pThreadData );

        // do a busy wait
        while( (void*)1 ==  thread1.m_pThreadData );

        TEST_ENSURE( pThreadData == thread1.m_pThreadData, "uno_threadpool_reply error" );
    }

    //---------------
    // Test request
    //---------------
    {
        OThread1 thread1( pCallerUuid1 );

        thread1.create();

        // do a busy wait
        while( ! thread1.m_pThreadIdentifier && ! thread1.m_nThreadIdentifierLength );

        // do a request
        sal_Int32 i = 1;
        uno_threadpool_request( thread1.m_pThreadIdentifier ,
                                thread1.m_nThreadIdentifierLength,
                                &i,
                                doIt,
                                sal_False);

        // do a busy wait
        while( 1 == i );
        TEST_ENSURE( 2 == i, "uno_threadpool_request error" );

        // get it out of the pool
        void *pThreadData = (void*)0xdeadbabe;
        uno_threadpool_reply( thread1.m_pThreadIdentifier ,
                              thread1.m_nThreadIdentifierLength,
                              pThreadData );

        // do a busy wait
        while( pThreadData !=  thread1.m_pThreadData );

    }

    //---------------
    // Test dispose threads
    //---------------
    {
        OThread1 thread1( pCallerUuid1 );
        OThread1 thread2( pCallerUuid2 );

        thread1.create();
        thread2.create();

        // do a busy wait
        while( ! thread1.m_pThreadIdentifier && ! thread1.m_nThreadIdentifierLength  &&
               ! thread2.m_pThreadIdentifier && ! thread2.m_nThreadIdentifierLength );

        // dispose the first
        uno_threadpool_disposeThreads( pCallerUuid1 );

        while( (void*)1 == thread1.m_pThreadData );
        TEST_ENSURE( (void*)0 == thread1.m_pThreadData, "disposing threads failed" );

        TimeValue value = {1,0};
        osl_waitThread( &value );
        TEST_ENSURE( (void*)1 == thread2.m_pThreadData, "wrong thread disposed !" );

        // test, if new threads are directly disposed
        OThread1 thread3( pCallerUuid1 );
        thread3.create();

        while( (void*)1 == thread3.m_pThreadData );
        TEST_ENSURE( (void*)0 == thread3.m_pThreadData ,
                     "new threads entering threadpool are not disposed" );

        uno_threadpool_reply( thread2.m_pThreadIdentifier ,
                              thread2.m_nThreadIdentifierLength,
                              (void*)0x2 );

        while( (void*)1 == thread2.m_pThreadData );
        TEST_ENSURE( (void*)2 == thread2.m_pThreadData , "reply does not work correctly" );

        uno_threadpool_stopDisposeThreads( pCallerUuid1 );
    }
    printf( "Done\n" );
}
