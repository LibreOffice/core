/*************************************************************************
 *
 *  $RCSfile: byteseq.c,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:17:24 $
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
    0       /* sal_Unicode  buffer[1];  */
};

//==================================================================================================
void SAL_CALL rtl_byte_sequence_reference2One(
    sal_Sequence ** ppSequence )
{
    sal_Sequence * pSequence, * pNew;
    sal_Int32 nElements;

    OSL_ENSHURE( ppSequence, "### null ptr!" );
    pSequence = *ppSequence;

    if (pSequence->nRefCount > 1)
    {
        nElements = pSequence->nElements;
        if (nElements)
        {
            pNew = (sal_Sequence *)rtl_allocateMemory( SAL_SEQUENCE_HEADER_SIZE + nElements );

            rtl_copyMemory( pNew->elements, pSequence->elements, nElements );

            if (! osl_decrementInterlockedCount( &pSequence->nRefCount ))
                rtl_freeMemory( pSequence );
        }
        else
        {
            pNew = (sal_Sequence *)rtl_allocateMemory( SAL_SEQUENCE_HEADER_SIZE );
        }

        pNew->nRefCount = 1;
        pNew->nElements = nElements;
        *ppSequence = pNew;
    }
}

//==================================================================================================
void SAL_CALL rtl_byte_sequence_realloc(
    sal_Sequence ** ppSequence, sal_Int32 nSize )
{
    sal_Sequence * pSequence, * pNew;
    sal_Int32 nElements;

    OSL_ENSHURE( ppSequence, "### null ptr!" );
    pSequence = *ppSequence;
    nElements = pSequence->nElements;

    if (nElements == nSize)
        return;

    if (pSequence->nRefCount > 1) // split
    {
        pNew = (sal_Sequence *)rtl_allocateMemory( SAL_SEQUENCE_HEADER_SIZE + nSize );

        if (nSize > nElements)
        {
            rtl_copyMemory( pNew->elements, pSequence->elements, nElements );
            rtl_zeroMemory( pNew->elements + nElements, nSize - nElements );
        }
        else
        {
            rtl_copyMemory( pNew->elements, pSequence->elements, nElements );
        }

        if (! osl_decrementInterlockedCount( &pSequence->nRefCount ))
            rtl_freeMemory( pSequence );

        pSequence = pNew;
    }
    else
    {
        pSequence = (sal_Sequence *)rtl_reallocateMemory(
            pSequence, SAL_SEQUENCE_HEADER_SIZE + nSize );
    }
    pSequence->nRefCount = 1;
    pSequence->nElements = nSize;
    *ppSequence = pSequence;
}

//==================================================================================================
void SAL_CALL rtl_byte_sequence_acquire( sal_Sequence *pSequence )
{
    OSL_ASSERT( pSequence );
    osl_incrementInterlockedCount( &(pSequence->nRefCount) );
}

//==================================================================================================
void SAL_CALL rtl_byte_sequence_release( sal_Sequence *pSequence )
{
    OSL_ASSERT( pSequence );
    if (! osl_decrementInterlockedCount( &(pSequence->nRefCount )) )
    {
        rtl_freeMemory( pSequence );
    }
}

//==================================================================================================
void SAL_CALL rtl_byte_sequence_construct( sal_Sequence **ppSequence , sal_Int32 nLength )
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

        (*ppSequence)->nRefCount = 1;
        (*ppSequence)->nElements = nLength;
    }
    else
    {
        *ppSequence = &aEmpty_rtl_ByteSeq;
        rtl_byte_sequence_acquire( *ppSequence );
    }
}

//==================================================================================================
void SAL_CALL rtl_byte_sequence_constructNoDefault( sal_Sequence **ppSequence , sal_Int32 nLength )
{
    OSL_ASSERT( ppSequence );
    if( *ppSequence )
    {
        rtl_byte_sequence_release( *ppSequence );
        *ppSequence = 0;
    }
    *ppSequence = (sal_Sequence *) rtl_allocateMemory( SAL_SEQUENCE_HEADER_SIZE + nLength );

    (*ppSequence)->nRefCount = 1;
    (*ppSequence)->nElements = nLength;
}

//==================================================================================================
void SAL_CALL rtl_byte_sequence_constructFromArray(
    sal_Sequence **ppSequence, const sal_Int8 *pData , sal_Int32 nLength )
{
    rtl_byte_sequence_constructNoDefault( ppSequence , nLength );
    rtl_copyMemory( (*ppSequence)->elements, pData, nLength );
}

//==================================================================================================
void SAL_CALL rtl_byte_sequence_assign( sal_Sequence **ppSequence , sal_Sequence *pSequence )
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
    return (0 == rtl_compareMemory( pSequence1->elements, pSequence2->elements, pSequence1->nElements ));
}


//==================================================================================================
const sal_Int8 *SAL_CALL rtl_byte_sequence_getConstArray( sal_Sequence *pSequence )
{
    return pSequence->elements;
}

//==================================================================================================
sal_Int32 SAL_CALL rtl_byte_sequence_getLength( sal_Sequence *pSequence )
{
    return pSequence->nElements;
}
