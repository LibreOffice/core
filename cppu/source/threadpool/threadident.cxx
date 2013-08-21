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

#include <list>

#include <osl/mutex.hxx>
#include <osl/thread.hxx>
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
    rtl_byte_sequence_constructNoDefault( ppThreadId , 4 + 16 );
    sal_uInt32 id = osl::Thread::getCurrentIdentifier();
    (*ppThreadId)->elements[0] = id & 0xFF;
    (*ppThreadId)->elements[1] = (id >> 8) & 0xFF;
    (*ppThreadId)->elements[2] = (id >> 16) & 0xFF;
    (*ppThreadId)->elements[3] = (id >> 24) & 0xFF;
    rtl_getGlobalProcessId( (sal_uInt8 * ) &( (*ppThreadId)->elements[4]) );
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


extern "C" void SAL_CALL uno_releaseIdFromCurrentThread()
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

extern "C" sal_Bool SAL_CALL uno_bindIdToCurrentThread( sal_Sequence *pThreadId )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
