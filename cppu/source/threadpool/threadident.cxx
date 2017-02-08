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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cppu.hxx"
#include <stdio.h>
#include <string.h>

#include <list>

#include <osl/mutex.hxx>
#include <osl/thread.h>
#include <osl/diagnose.h>

#include <rtl/process.h>
#include <rtl/byteseq.hxx>

#include <uno/threadpool.h>

#include "current.hxx"


using namespace ::std;
using namespace ::osl;
using namespace ::rtl;
using namespace ::cppu;


static inline void createLocalId( sal_Sequence **ppThreadId )
{
    rtl_byte_sequence_constructNoDefault( ppThreadId , sizeof(oslThreadIdentifier) + 16 );

    oslThreadIdentifier hIdent = osl_getThreadIdentifier(0);
    memcpy((*ppThreadId)->elements, &hIdent, sizeof(oslThreadIdentifier));

    rtl_getGlobalProcessId( (sal_uInt8 * ) &( (*ppThreadId)->elements[sizeof(oslThreadIdentifier)]) );
}


extern "C" void SAL_CALL
uno_getIdOfCurrentThread( sal_Sequence **ppThreadId )
    SAL_THROW_EXTERN_C()
{
    IdContainer * p = getIdContainer();
    if( ! p->bInit )
    {
        // first time, that the thread enters the bridge
        createLocalId( ppThreadId );

        // TODO
        // note : this is a leak !
        p->pLocalThreadId = *ppThreadId;
        p->pCurrentId = *ppThreadId;
        p->nRefCountOfCurrentId = 1;
        rtl_byte_sequence_acquire( p->pLocalThreadId );
        rtl_byte_sequence_acquire( p->pCurrentId );
        p->bInit = sal_True;
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


extern "C"   void SAL_CALL uno_releaseIdFromCurrentThread()
    SAL_THROW_EXTERN_C()
{
    IdContainer *p = getIdContainer();
    OSL_ASSERT( p );
    OSL_ASSERT( p->nRefCountOfCurrentId );

    p->nRefCountOfCurrentId --;
    if( ! p->nRefCountOfCurrentId && (p->pLocalThreadId != p->pCurrentId) )
    {
        rtl_byte_sequence_assign( &(p->pCurrentId) , p->pLocalThreadId );
    }
}

extern "C"  sal_Bool SAL_CALL uno_bindIdToCurrentThread( sal_Sequence *pThreadId )
    SAL_THROW_EXTERN_C()
{
    IdContainer *p = getIdContainer();
    if( ! p->bInit )
    {
        p->pLocalThreadId = 0;
        createLocalId( &(p->pLocalThreadId) );
        p->nRefCountOfCurrentId = 1;
        p->pCurrentId = pThreadId;
        rtl_byte_sequence_acquire( p->pCurrentId );
        p->bInit = sal_True;
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
