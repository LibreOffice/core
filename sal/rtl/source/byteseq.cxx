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

#include <string.h>

#include <osl/diagnose.h>
#include <osl/interlck.h>

#include <rtl/byteseq.h>
#include <rtl/alloc.h>
#include <rtl/memory.h>

/* static data to be referenced by all empty strings
 * the refCount is predefined to 1 and must never become 0 !
 */
static sal_Sequence aEmpty_rtl_ByteSeq =
{
    1,      /* sal_Int32    refCount;   */
    0,      /* sal_Int32    length;     */
    { 0 }   /* sal_Unicode  buffer[1];  */
};

//==================================================================================================
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

//==================================================================================================
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

//==================================================================================================
void SAL_CALL rtl_byte_sequence_acquire( sal_Sequence *pSequence )
    SAL_THROW_EXTERN_C()
{
    OSL_ASSERT( pSequence );
    osl_atomic_increment( &(pSequence->nRefCount) );
}

//==================================================================================================
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

//==================================================================================================
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

//==================================================================================================
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

//==================================================================================================
void SAL_CALL rtl_byte_sequence_constructFromArray(
    sal_Sequence **ppSequence, const sal_Int8 *pData , sal_Int32 nLength )
    SAL_THROW_EXTERN_C()
{
    rtl_byte_sequence_constructNoDefault( ppSequence , nLength );
    if ( *ppSequence != 0 )
        memcpy( (*ppSequence)->elements, pData, nLength );
}

//==================================================================================================
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

//==================================================================================================
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
    return (sal_Bool)
        (rtl_compareMemory(
            pSequence1->elements, pSequence2->elements, pSequence1->nElements )
         == 0);
}


//==================================================================================================
const sal_Int8 *SAL_CALL rtl_byte_sequence_getConstArray( sal_Sequence *pSequence )
    SAL_THROW_EXTERN_C()
{
    return ((const sal_Int8*)(pSequence->elements));
}

//==================================================================================================
sal_Int32 SAL_CALL rtl_byte_sequence_getLength( sal_Sequence *pSequence )
    SAL_THROW_EXTERN_C()
{
    return pSequence->nElements;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
