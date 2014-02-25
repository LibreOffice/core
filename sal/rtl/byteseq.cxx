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

#include <string.h>

#include <osl/diagnose.h>
#include <osl/interlck.h>

#include <rtl/byteseq.h>
#include <rtl/alloc.h>

/* static data to be referenced by all empty strings
 * the refCount is predefined to 1 and must never become 0 !
 */
static sal_Sequence aEmpty_rtl_ByteSeq =
{
    1,      /* sal_Int32    refCount;   */
    0,      /* sal_Int32    length;     */
    { 0 }   /* sal_Unicode  buffer[1];  */
};


void SAL_CALL rtl_byte_sequence_reference2One(
    sal_Sequence ** ppSequence ) SAL_THROW_EXTERN_C()
{
    sal_Sequence * pSequence, * pNew;
    sal_Int32 nElements;

    OSL_ENSURE( ppSequence, "### null ptr!" );
    pSequence = *ppSequence;

    if (pSequence->nRefCount > 1)
    {
        nElements = pSequence->nElements;
        if (nElements)
        {
            pNew = (sal_Sequence *)rtl_allocateMemory( SAL_SEQUENCE_HEADER_SIZE + nElements );

            if ( pNew != 0 )
                memcpy( pNew->elements, pSequence->elements, nElements );

            if (! osl_atomic_decrement( &pSequence->nRefCount ))
                rtl_freeMemory( pSequence );
        }
        else
        {
            pNew = (sal_Sequence *)rtl_allocateMemory( SAL_SEQUENCE_HEADER_SIZE );
        }

        if ( pNew != 0 )
        {
            pNew->nRefCount = 1;
            pNew->nElements = nElements;
        }

        *ppSequence = pNew;
    }
}


void SAL_CALL rtl_byte_sequence_realloc(
    sal_Sequence ** ppSequence, sal_Int32 nSize ) SAL_THROW_EXTERN_C()
{
    sal_Sequence * pSequence, * pNew;
    sal_Int32 nElements;

    OSL_ENSURE( ppSequence, "### null ptr!" );
    pSequence = *ppSequence;
    nElements = pSequence->nElements;

    if (nElements == nSize)
        return;

    if (pSequence->nRefCount > 1) // split
    {
        pNew = (sal_Sequence *)rtl_allocateMemory( SAL_SEQUENCE_HEADER_SIZE + nSize );

        if ( pNew != 0 )
        {
            if (nSize > nElements)
            {
                memcpy( pNew->elements, pSequence->elements, nElements );
                memset( pNew->elements + nElements, 0, nSize - nElements );
            }
            else
            {
                memcpy( pNew->elements, pSequence->elements, nSize );
            }
        }

        if (! osl_atomic_decrement( &pSequence->nRefCount ))
            rtl_freeMemory( pSequence );
        pSequence = pNew;
    }
    else
    {
        pSequence = (sal_Sequence *)rtl_reallocateMemory(
            pSequence, SAL_SEQUENCE_HEADER_SIZE + nSize );
    }

    if ( pSequence != 0 )
    {
        pSequence->nRefCount = 1;
        pSequence->nElements = nSize;
    }

    *ppSequence = pSequence;
}


void SAL_CALL rtl_byte_sequence_acquire( sal_Sequence *pSequence )
    SAL_THROW_EXTERN_C()
{
    OSL_ASSERT( pSequence );
    osl_atomic_increment( &(pSequence->nRefCount) );
}


void SAL_CALL rtl_byte_sequence_release( sal_Sequence *pSequence )
    SAL_THROW_EXTERN_C()
{
    if ( pSequence != 0 )
    {
        if (! osl_atomic_decrement( &(pSequence->nRefCount )) )
        {
            rtl_freeMemory( pSequence );
        }
    }
}


void SAL_CALL rtl_byte_sequence_construct( sal_Sequence **ppSequence , sal_Int32 nLength )
    SAL_THROW_EXTERN_C()
{
    OSL_ASSERT( ppSequence );
    if( *ppSequence )
    {
        rtl_byte_sequence_release( *ppSequence );
        *ppSequence = 0;
    }

    if( nLength )
    {
        *ppSequence = (sal_Sequence *) rtl_allocateZeroMemory( SAL_SEQUENCE_HEADER_SIZE + nLength );

        if ( *ppSequence != 0 )
        {
            (*ppSequence)->nRefCount = 1;
            (*ppSequence)->nElements = nLength;
        }
    }
    else
    {
        *ppSequence = &aEmpty_rtl_ByteSeq;
        rtl_byte_sequence_acquire( *ppSequence );
    }
}


void SAL_CALL rtl_byte_sequence_constructNoDefault( sal_Sequence **ppSequence , sal_Int32 nLength )
    SAL_THROW_EXTERN_C()
{
    OSL_ASSERT( ppSequence );
    if( *ppSequence )
    {
        rtl_byte_sequence_release( *ppSequence );
        *ppSequence = 0;
    }

    *ppSequence = (sal_Sequence *) rtl_allocateMemory( SAL_SEQUENCE_HEADER_SIZE + nLength );

    if ( *ppSequence != 0 )
    {
        (*ppSequence)->nRefCount = 1;
        (*ppSequence)->nElements = nLength;
    }
}


void SAL_CALL rtl_byte_sequence_constructFromArray(
    sal_Sequence **ppSequence, const sal_Int8 *pData , sal_Int32 nLength )
    SAL_THROW_EXTERN_C()
{
    rtl_byte_sequence_constructNoDefault( ppSequence , nLength );
    if ( *ppSequence != 0 )
        memcpy( (*ppSequence)->elements, pData, nLength );
}


void SAL_CALL rtl_byte_sequence_assign( sal_Sequence **ppSequence , sal_Sequence *pSequence )
    SAL_THROW_EXTERN_C()
{
    if ( *ppSequence != pSequence)
    {
        if( *ppSequence )
        {
            rtl_byte_sequence_release( *ppSequence );
        }
        *ppSequence = pSequence;
        rtl_byte_sequence_acquire( *ppSequence );
    }
//  else
//      nothing to do

}


sal_Bool SAL_CALL rtl_byte_sequence_equals( sal_Sequence *pSequence1 , sal_Sequence *pSequence2 )
    SAL_THROW_EXTERN_C()
{
    OSL_ASSERT( pSequence1 );
    OSL_ASSERT( pSequence2 );
    if (pSequence1 == pSequence2)
    {
        return sal_True;
    }
    if (pSequence1->nElements != pSequence2->nElements)
    {
        return sal_False;
    }
    return
        memcmp(
            pSequence1->elements, pSequence2->elements, pSequence1->nElements )
        == 0;
}



const sal_Int8 *SAL_CALL rtl_byte_sequence_getConstArray( sal_Sequence *pSequence )
    SAL_THROW_EXTERN_C()
{
    return ((const sal_Int8*)(pSequence->elements));
}


sal_Int32 SAL_CALL rtl_byte_sequence_getLength( sal_Sequence *pSequence )
    SAL_THROW_EXTERN_C()
{
    return pSequence->nElements;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
