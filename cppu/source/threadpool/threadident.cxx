/*************************************************************************
 *
 *  $RCSfile: threadident.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jbu $ $Date: 2000-10-13 12:21:44 $
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
#include <stdio.h>
#include <assert.h>

#include <list>
#include <hash_map>

#include <osl/mutex.hxx>
#include <osl/thread.h>
#include <osl/diagnose.h>

#include <rtl/process.h>
#include <rtl/byteseq.hxx>

#include <uno/threadpool.h>

#include <com/sun/star/uno/Sequence.hxx>

using namespace ::std;
using namespace ::osl;
using namespace ::rtl;


static sal_Bool g_bInitialized;
static oslThreadKey g_key;

namespace cppu_threadpool
{
struct IdContainer
{
    sal_Sequence *pLocalThreadId;
    sal_Int32     nRefCountOfCurrentId;
    sal_Sequence *pCurrentId;
};
}
using namespace cppu_threadpool;

static void SAL_CALL destructIdContainer( void *p )
{
    if( p )
    {
        IdContainer *pId = (IdContainer * ) p;
        rtl_byte_sequence_release( pId->pLocalThreadId );
        rtl_byte_sequence_release( pId->pCurrentId );
        rtl_freeMemory( p );
    }
}

static inline oslThreadKey getKey()
{
    if( ! g_bInitialized )
    {
        ::osl::MutexGuard guard( ::osl::Mutex::getGlobalMutex() );
        if( ! g_bInitialized )
        {
            g_key = osl_createThreadKey( destructIdContainer );
            g_bInitialized = sal_True;
        }
    }
    return g_key;
}

static inline void createLocalId( sal_Sequence **ppThreadId )
{
    rtl_byte_sequence_constructNoDefault( ppThreadId , 4 + 16 );
    *((sal_Int32*) ((*ppThreadId)->elements)) = osl_getThreadIdentifier(0);

    rtl_getGlobalProcessId( (sal_uInt8 * ) &( (*ppThreadId)->elements[4]) );
}


extern "C" SAL_DLLEXPORT void SAL_CALL
uno_getIdOfCurrentThread( sal_Sequence **ppThreadId )
{
    IdContainer * p = (IdContainer * ) osl_getThreadKeyData( getKey() );
    if( ! p )
    {
        // first time, that the thread enters the bridge
        createLocalId( ppThreadId );

        // TODO
        // note : this is a leak !
        IdContainer *p = (IdContainer *) rtl_allocateMemory( sizeof( IdContainer ) );
        p->pLocalThreadId = *ppThreadId;
        p->pCurrentId = *ppThreadId;
        p->nRefCountOfCurrentId = 1;
        rtl_byte_sequence_acquire( p->pLocalThreadId );
        rtl_byte_sequence_acquire( p->pCurrentId );

        OSL_VERIFY( osl_setThreadKeyData( getKey(), p ) );
    }
    else
    {
        p->nRefCountOfCurrentId ++;
        if( *ppThreadId )
        {
            rtl_byte_sequence_release( *ppThreadId );
        }
        *ppThreadId = p->pCurrentId;
        rtl_byte_sequence_acquire( *ppThreadId );
    }
}


extern "C" SAL_DLLEXPORT  void SAL_CALL uno_releaseIdFromCurrentThread()
{
    IdContainer *p = (IdContainer * ) osl_getThreadKeyData( getKey() );
    OSL_ASSERT( p );
    OSL_ASSERT( p->nRefCountOfCurrentId );

    p->nRefCountOfCurrentId --;
    if( ! p->nRefCountOfCurrentId && (p->pLocalThreadId != p->pCurrentId) )
    {
        rtl_byte_sequence_assign( &(p->pCurrentId) , p->pLocalThreadId );
    }
}

extern "C" SAL_DLLEXPORT sal_Bool SAL_CALL uno_bindIdToCurrentThread( sal_Sequence *pThreadId )
{
    IdContainer *p = (IdContainer * ) osl_getThreadKeyData( getKey() );
    if( ! p )
    {
        IdContainer *p = (IdContainer * ) rtl_allocateMemory( sizeof( IdContainer ) );

        p->pLocalThreadId = 0;
        createLocalId( &(p->pLocalThreadId) );
        p->nRefCountOfCurrentId = 1;
        p->pCurrentId = pThreadId;
        rtl_byte_sequence_acquire( p->pCurrentId );
        osl_setThreadKeyData( getKey() , p );
    }
    else
    {
        OSL_ASSERT( 0 == p->nRefCountOfCurrentId );
        if( 0 == p->nRefCountOfCurrentId )
        {
            rtl_byte_sequence_assign(&( p->pCurrentId ), pThreadId );
            p->nRefCountOfCurrentId ++;
        }
        else
        {
            return sal_False;
        }

    }
    return sal_True;
}
