/*************************************************************************
 *
 *  $RCSfile: sequence.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: dbo $ $Date: 2001-03-09 12:10:57 $
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

#ifndef _RTL_MEMORY_H_
#include <rtl/memory.h>
#endif
#ifndef _RTL_ALLOC_H_
#include <rtl/alloc.h>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _OSL_INTERLCK_H_
#include <osl/interlck.h>
#endif

#ifndef _TYPELIB_TYPEDESCRIPTION_H_
#include <typelib/typedescription.h>
#endif
#ifndef _UNO_DATA_H_
#include <uno/data.h>
#endif
#ifndef _UNO_DISPATCHER_H_
#include <uno/dispatcher.h>
#endif
#ifndef _UNO_SEQUENCE2_H_
#include <uno/sequence2.h>
#endif

#include "constr.hxx"
#include "copy.hxx"
#include "destr.hxx"

using namespace cppu;

namespace cppu
{
//--------------------------------------------------------------------------------------------------
static inline void allocSeq(
    uno_Sequence ** ppSeq, sal_Int32 nElementSize, sal_Int32 nElements )
    SAL_THROW( () )
{
    if (nElements >= 0) // (re)alloc memory?
    {
        uno_Sequence * pSeq = (uno_Sequence *)
            (*ppSeq
             ? ::rtl_reallocateMemory( *ppSeq, SAL_SEQUENCE_HEADER_SIZE + (nElementSize * nElements) )
             : ::rtl_allocateMemory( SAL_SEQUENCE_HEADER_SIZE + (nElementSize * nElements) ));
        pSeq->nRefCount = 1;
        pSeq->nElements = nElements;
        *ppSeq = pSeq;
    }
}
//--------------------------------------------------------------------------------------------------
static inline void __defaultConstructElements(
    uno_Sequence ** ppSequence,
    typelib_TypeDescriptionReference * pElementType,
    sal_Int32 nStartIndex, sal_Int32 nStopIndex,
    sal_Int32 nAlloc = -1 ) // >= 0 means (re)alloc memory for nAlloc elements
    SAL_THROW( () )
{
    switch (pElementType->eTypeClass)
    {
    case typelib_TypeClass_CHAR:
        allocSeq( ppSequence, sizeof(sal_Unicode), nAlloc );
        ::rtl_zeroMemory(
            (*ppSequence)->elements + (sizeof(sal_Unicode) * nStartIndex),
            sizeof(sal_Unicode) * (nStopIndex - nStartIndex) );
        break;
    case typelib_TypeClass_BOOLEAN:
        allocSeq( ppSequence, sizeof(sal_Bool), nAlloc );
        ::rtl_zeroMemory(
            (*ppSequence)->elements + (sizeof(sal_Bool) * nStartIndex),
            sizeof(sal_Bool) * (nStopIndex - nStartIndex) );
        break;
    case typelib_TypeClass_BYTE:
        allocSeq( ppSequence, sizeof(sal_Int8), nAlloc );
        ::rtl_zeroMemory(
            (*ppSequence)->elements + (sizeof(sal_Int8) * nStartIndex),
            sizeof(sal_Int8) * (nStopIndex - nStartIndex) );
        break;
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
        allocSeq( ppSequence, sizeof(sal_Int16), nAlloc );
        ::rtl_zeroMemory(
            (*ppSequence)->elements + (sizeof(sal_Int16) * nStartIndex),
            sizeof(sal_Int16) * (nStopIndex - nStartIndex) );
        break;
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
        allocSeq( ppSequence, sizeof(sal_Int32), nAlloc );
        ::rtl_zeroMemory(
            (*ppSequence)->elements + (sizeof(sal_Int32) * nStartIndex),
            sizeof(sal_Int32) * (nStopIndex - nStartIndex) );
        break;
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
        allocSeq( ppSequence, sizeof(sal_Int64), nAlloc );
        ::rtl_zeroMemory(
            (*ppSequence)->elements + (sizeof(sal_Int64) * nStartIndex),
            sizeof(sal_Int64) * (nStopIndex - nStartIndex) );
        break;
    case typelib_TypeClass_FLOAT:
    {
        allocSeq( ppSequence, sizeof(float), nAlloc );

        float * pElements = (float *)(*ppSequence)->elements;
        for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
        {
            pElements[nPos] = 0.0;
        }
        break;
    }
    case typelib_TypeClass_DOUBLE:
    {
        allocSeq( ppSequence, sizeof(double), nAlloc );

        double * pElements = (double *)(*ppSequence)->elements;
        for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
        {
            pElements[nPos] = 0.0;
        }
        break;
    }
    case typelib_TypeClass_STRING:
    {
        allocSeq( ppSequence, sizeof(rtl_uString *), nAlloc );

        rtl_uString ** pElements = (rtl_uString **)(*ppSequence)->elements;
        for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
        {
            pElements[nPos] = 0;
            RTL_USTRING_NEW( &pElements[nPos] );
        }
        break;
    }
    case typelib_TypeClass_TYPE:
    {
        allocSeq( ppSequence, sizeof(typelib_TypeDescriptionReference *), nAlloc );

        typelib_TypeDescriptionReference ** pElements =
            (typelib_TypeDescriptionReference **)(*ppSequence)->elements;
        for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
        {
            pElements[nPos] = __getVoidType();
        }
        break;
    }
    case typelib_TypeClass_ANY:
    {
        allocSeq( ppSequence, sizeof(uno_Any), nAlloc );

        uno_Any * pElements = (uno_Any *)(*ppSequence)->elements;
        for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
        {
            __CONSTRUCT_EMPTY_ANY( &pElements[nPos] );
        }
        break;
    }
    case typelib_TypeClass_ENUM:
    {
        allocSeq( ppSequence, sizeof(int), nAlloc );

        typelib_TypeDescription * pElementTypeDescr = 0;
        TYPELIB_DANGER_GET( &pElementTypeDescr, pElementType );
        int eEnum = ((typelib_EnumTypeDescription *)pElementTypeDescr)->nDefaultEnumValue;
        TYPELIB_DANGER_RELEASE( pElementTypeDescr );

        int * pElements = (int *)(*ppSequence)->elements;
        for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
        {
            pElements[nPos] = eEnum;
        }
        break;
    }
#ifdef CPPU_ASSERTIONS
    case typelib_TypeClass_TYPEDEF:
        OSL_ENSHURE( sal_False, "### unexpected typedef!" );
        break;
#endif
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
    {
        typelib_TypeDescription * pElementTypeDescr = 0;
        TYPELIB_DANGER_GET( &pElementTypeDescr, pElementType );
        sal_Int32 nElementSize = pElementTypeDescr->nSize;

        allocSeq( ppSequence, nElementSize, nAlloc );

        char * pElements = (*ppSequence)->elements;
        for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
        {
            __defaultConstructStruct(
                pElements + (nElementSize * nPos),
                (typelib_CompoundTypeDescription *)pElementTypeDescr );
        }
        TYPELIB_DANGER_RELEASE( pElementTypeDescr );
        break;
    }
    case typelib_TypeClass_UNION:
    {
        typelib_TypeDescription * pElementTypeDescr = 0;
        TYPELIB_DANGER_GET( &pElementTypeDescr, pElementType );

        sal_Int32 nElementSize = pElementTypeDescr->nSize;

        allocSeq( ppSequence, nElementSize, nAlloc );

        sal_Int32 nValueOffset = ((typelib_UnionTypeDescription *)pElementTypeDescr)->nValueOffset;
        sal_Int64 nDefaultDiscr = ((typelib_UnionTypeDescription *)pElementTypeDescr)->nDefaultDiscriminant;

        typelib_TypeDescription * pDefaultTypeDescr = 0;
        TYPELIB_DANGER_GET( &pDefaultTypeDescr, ((typelib_UnionTypeDescription *)pElementTypeDescr)->pDefaultTypeRef );
        TYPELIB_DANGER_RELEASE( pElementTypeDescr );

        char * pElements = (*ppSequence)->elements;
        for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
        {
            char * pMem = pElements + (nElementSize * nPos);
            ::uno_constructData( (char *)pMem + nValueOffset, pDefaultTypeDescr );
            *(sal_Int64 *)pMem = nDefaultDiscr;
        }
        TYPELIB_DANGER_RELEASE( pDefaultTypeDescr );
        break;
    }
    case typelib_TypeClass_SEQUENCE:
    {
        allocSeq( ppSequence, sizeof(uno_Sequence *), nAlloc );

        uno_Sequence ** pElements = (uno_Sequence **)(*ppSequence)->elements;
        for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
        {
            pElements[nPos] = __getEmptySequence();
        }
        break;
    }
    case typelib_TypeClass_INTERFACE: // either cpp or c-uno interface
        allocSeq( ppSequence, sizeof(void *), nAlloc );

        ::rtl_zeroMemory(
            (*ppSequence)->elements + (sizeof(void *) * nStartIndex),
            sizeof(void *) * (nStopIndex - nStartIndex) );
        break;
    }
}
//--------------------------------------------------------------------------------------------------
static inline void __copyConstructElements(
    uno_Sequence ** ppSequence, void * pSourceElements,
    typelib_TypeDescriptionReference * pElementType,
    sal_Int32 nStartIndex, sal_Int32 nStopIndex,
    uno_AcquireFunc acquire,
    sal_Int32 nAlloc = -1 ) // >= 0 means (re)alloc memory for nAlloc elements
    SAL_THROW( () )
{
    switch (pElementType->eTypeClass)
    {
    case typelib_TypeClass_CHAR:
        allocSeq( ppSequence, sizeof(sal_Unicode), nAlloc );
        ::rtl_copyMemory(
            (*ppSequence)->elements + (sizeof(sal_Unicode) * nStartIndex),
            (char *)pSourceElements + (sizeof(sal_Unicode) * nStartIndex),
            sizeof(sal_Unicode) * (nStopIndex - nStartIndex) );
        break;
    case typelib_TypeClass_BOOLEAN:
        allocSeq( ppSequence, sizeof(sal_Bool), nAlloc );
        ::rtl_copyMemory(
            (*ppSequence)->elements + (sizeof(sal_Bool) * nStartIndex),
            (char *)pSourceElements + (sizeof(sal_Bool) * nStartIndex),
            sizeof(sal_Bool) * (nStopIndex - nStartIndex) );
        break;
    case typelib_TypeClass_BYTE:
        allocSeq( ppSequence, sizeof(sal_Int8), nAlloc );
        ::rtl_copyMemory(
            (*ppSequence)->elements + (sizeof(sal_Int8) * nStartIndex),
            (char *)pSourceElements + (sizeof(sal_Int8) * nStartIndex),
            sizeof(sal_Int8) * (nStopIndex - nStartIndex) );
        break;
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
        allocSeq( ppSequence, sizeof(sal_Int16), nAlloc );
        ::rtl_copyMemory(
            (*ppSequence)->elements + (sizeof(sal_Int16) * nStartIndex),
            (char *)pSourceElements + (sizeof(sal_Int16) * nStartIndex),
            sizeof(sal_Int16) * (nStopIndex - nStartIndex) );
        break;
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
        allocSeq( ppSequence, sizeof(sal_Int32), nAlloc );
        ::rtl_copyMemory(
            (*ppSequence)->elements + (sizeof(sal_Int32) * nStartIndex),
            (char *)pSourceElements + (sizeof(sal_Int32) * nStartIndex),
            sizeof(sal_Int32) * (nStopIndex - nStartIndex) );
        break;
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
        allocSeq( ppSequence, sizeof(sal_Int64), nAlloc );
        ::rtl_copyMemory(
            (*ppSequence)->elements + (sizeof(sal_Int64) * nStartIndex),
            (char *)pSourceElements + (sizeof(sal_Int64) * nStartIndex),
            sizeof(sal_Int64) * (nStopIndex - nStartIndex) );
        break;
    case typelib_TypeClass_FLOAT:
        allocSeq( ppSequence, sizeof(float), nAlloc );
        ::rtl_copyMemory(
            (*ppSequence)->elements + (sizeof(float) * nStartIndex),
            (char *)pSourceElements + (sizeof(float) * nStartIndex),
            sizeof(float) * (nStopIndex - nStartIndex) );
        break;
    case typelib_TypeClass_DOUBLE:
        allocSeq( ppSequence, sizeof(double), nAlloc );
        ::rtl_copyMemory(
            (*ppSequence)->elements + (sizeof(double) * nStartIndex),
            (char *)pSourceElements + (sizeof(double) * nStartIndex),
            sizeof(double) * (nStopIndex - nStartIndex) );
        break;
    case typelib_TypeClass_ENUM:
        allocSeq( ppSequence, sizeof(int), nAlloc );
        ::rtl_copyMemory(
            (*ppSequence)->elements + (sizeof(int) * nStartIndex),
            (char *)pSourceElements + (sizeof(int) * nStartIndex),
            sizeof(int) * (nStopIndex - nStartIndex) );
        break;
    case typelib_TypeClass_STRING:
    {
        allocSeq( ppSequence, sizeof(rtl_uString *), nAlloc );

        rtl_uString ** pDestElements = (rtl_uString **)(*ppSequence)->elements;
        for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
        {
            ::rtl_uString_acquire( ((rtl_uString **)pSourceElements)[nPos] );
            pDestElements[nPos] = ((rtl_uString **)pSourceElements)[nPos];
        }
        break;
    }
    case typelib_TypeClass_TYPE:
    {
        allocSeq( ppSequence, sizeof(typelib_TypeDescriptionReference *), nAlloc );

        typelib_TypeDescriptionReference ** pDestElements =
            (typelib_TypeDescriptionReference **)(*ppSequence)->elements;
        for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
        {
            TYPE_ACQUIRE( ((typelib_TypeDescriptionReference **)pSourceElements)[nPos] );
            pDestElements[nPos] = ((typelib_TypeDescriptionReference **)pSourceElements)[nPos];
        }
        break;
    }
    case typelib_TypeClass_ANY:
    {
        allocSeq( ppSequence, sizeof(uno_Any), nAlloc );

        uno_Any * pDestElements = (uno_Any *)(*ppSequence)->elements;
        for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
        {
            uno_Any * pSource = (uno_Any *)pSourceElements + nPos;
            __copyConstructAny(
                &pDestElements[nPos],
                pSource->pData,
                pSource->pType, 0,
                acquire, 0 );
        }
        break;
    }
#ifdef CPPU_ASSERTIONS
    case typelib_TypeClass_TYPEDEF:
        OSL_ENSHURE( sal_False, "### unexpected typedef!" );
        break;
#endif
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
    {
        typelib_TypeDescription * pElementTypeDescr = 0;
        TYPELIB_DANGER_GET( &pElementTypeDescr, pElementType );
        sal_Int32 nElementSize = pElementTypeDescr->nSize;

        allocSeq( ppSequence, nElementSize, nAlloc );

        char * pDestElements = (*ppSequence)->elements;

        typelib_CompoundTypeDescription * pTypeDescr = (typelib_CompoundTypeDescription *)pElementTypeDescr;
        for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
        {
            char * pDest = pDestElements + (nElementSize * nPos);
            char * pSource = (char *)pSourceElements + (nElementSize * nPos);

            if (pTypeDescr->pBaseTypeDescription)
            {
                // copy base value
                __copyConstructStruct( pDest, pSource, pTypeDescr->pBaseTypeDescription, acquire, 0 );
            }

            // then copy members
            typelib_TypeDescriptionReference ** ppTypeRefs = pTypeDescr->ppTypeRefs;
            sal_Int32 * pMemberOffsets = pTypeDescr->pMemberOffsets;
            sal_Int32 nDescr = pTypeDescr->nMembers;

            while (nDescr--)
            {
                ::uno_type_copyData(
                    pDest + pMemberOffsets[nDescr],
                    pSource + pMemberOffsets[nDescr],
                    ppTypeRefs[nDescr], acquire );
            }
        }
        TYPELIB_DANGER_RELEASE( pElementTypeDescr );
        break;
    }
    case typelib_TypeClass_UNION:
    {
        typelib_TypeDescription * pElementTypeDescr = 0;
        TYPELIB_DANGER_GET( &pElementTypeDescr, pElementType );
        sal_Int32 nElementSize = pElementTypeDescr->nSize;

        allocSeq( ppSequence, nElementSize, nAlloc );

        char * pDestElements = (*ppSequence)->elements;

        sal_Int32 nValueOffset = ((typelib_UnionTypeDescription *)pElementTypeDescr)->nValueOffset;
        for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
        {
            char * pDest = pDestElements + (nElementSize * nPos);
            char * pSource = (char *)pSourceElements + (nElementSize * nPos);

            typelib_TypeDescriptionReference * pSetType = __unionGetSetType( pSource, pElementTypeDescr );
            uno_type_copyData( pDest + nValueOffset,
                               pSource + nValueOffset,
                               pSetType, acquire );
            *(sal_Int64 *)pDest = *(sal_Int64 *)pSource;
            typelib_typedescriptionreference_release( pSetType );
        }
        TYPELIB_DANGER_RELEASE( pElementTypeDescr );
        break;
    }
    case typelib_TypeClass_SEQUENCE:
    {
        allocSeq( ppSequence, sizeof(uno_Sequence *), nAlloc );

        typelib_TypeDescription * pElementTypeDescr = 0;
        TYPELIB_DANGER_GET( &pElementTypeDescr, pElementType );
        uno_Sequence ** pDestElements = (uno_Sequence **)(*ppSequence)->elements;
        for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
        {
            __copyConstructSequence(
                &pDestElements[nPos],
                ((uno_Sequence **)pSourceElements)[nPos],
                ((typelib_IndirectTypeDescription *)pElementTypeDescr)->pType,
                acquire, 0 );
        }
        TYPELIB_DANGER_RELEASE( pElementTypeDescr );
        break;
    }
    case typelib_TypeClass_INTERFACE:
    {
        allocSeq( ppSequence, sizeof(void *), nAlloc );

        void ** pDestElements = (void **)(*ppSequence)->elements;
        for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
        {
            __acquire( pDestElements[nPos] = ((void **)pSourceElements)[nPos], acquire );
        }
        break;
    }
    }
}
//--------------------------------------------------------------------------------------------------
static inline void __reallocSequence(
    uno_Sequence ** ppSequence,
    typelib_TypeDescriptionReference * pElementType,
    sal_Int32 nSize,
    uno_AcquireFunc acquire, uno_ReleaseFunc release )
    SAL_THROW( () )
{
    uno_Sequence * pSource = *ppSequence;
    sal_Int32 nSourceElements = pSource->nElements;

    if (pSource->nRefCount > 1) // split
    {
        uno_Sequence * pNew = 0;

        sal_Int32 nRest = nSize - nSourceElements;
        sal_Int32 nCopy = (nRest > 0 ? nSourceElements : nSize);

        if (nCopy >= 0)
        {
            __copyConstructElements(
                &pNew, pSource->elements, pElementType,
                0, nCopy, acquire,
                nSize ); // alloc to nSize
        }
        if (nRest > 0)
        {
            __defaultConstructElements(
                &pNew, pElementType,
                nCopy, nSize,
                nCopy >= 0 ? -1 /* already allocated */ : nSize );
        }

        // destruct sequence
        if (! ::osl_decrementInterlockedCount( &(*ppSequence)->nRefCount ))
        {
            if ((*ppSequence)->nElements)
            {
                __destructElements(
                    (*ppSequence)->elements, pElementType, 0, (*ppSequence)->nElements, release );
            }
            ::rtl_freeMemory( *ppSequence );
        }

        *ppSequence = pNew;
    }
    else
    {
        if (nSize > nSourceElements) // default construct the rest
        {
            __defaultConstructElements(
                ppSequence, pElementType,
                nSourceElements, nSize,
                nSize ); // realloc to nSize
        }
        else // or destruct the rest and realloc mem
        {
            sal_Int32 nElementSize = __destructElements(
                (*ppSequence)->elements, pElementType,
                nSize, nSourceElements, release );
            *ppSequence = (uno_Sequence *)::rtl_reallocateMemory(
                *ppSequence, SAL_SEQUENCE_HEADER_SIZE + (nSize * nElementSize) );
            (*ppSequence)->nElements = nSize;
        }
    }
}

}

extern "C"
{
//##################################################################################################
SAL_DLLEXPORT void SAL_CALL uno_type_sequence_construct(
    uno_Sequence ** ppSequence, typelib_TypeDescriptionReference * pType,
    void * pElements, sal_Int32 len,
    uno_AcquireFunc acquire )
    SAL_THROW_EXTERN_C()
{
    if (len)
    {
        typelib_TypeDescription * pTypeDescr = 0;
        TYPELIB_DANGER_GET( &pTypeDescr, pType );

        typelib_TypeDescriptionReference * pElementType =
            ((typelib_IndirectTypeDescription *)pTypeDescr)->pType;

        uno_Sequence * pSequence = 0;

        if (pElements)
        {
            __copyConstructElements(
                &pSequence, pElements, pElementType,
                0, len, acquire,
                len ); // alloc to len
        }
        else
        {
            __defaultConstructElements(
                &pSequence,
                pElementType,
                0, len,
                len ); // alloc to len
        }

        *ppSequence = pSequence;

        TYPELIB_DANGER_RELEASE( pTypeDescr );
    }
    else
    {
        *ppSequence = __getEmptySequence();
    }
}
//##################################################################################################
SAL_DLLEXPORT void SAL_CALL uno_sequence_construct(
    uno_Sequence ** ppSequence, typelib_TypeDescription * pTypeDescr,
    void * pElements, sal_Int32 len,
    uno_AcquireFunc acquire )
    SAL_THROW_EXTERN_C()
{
    if (len)
    {
        typelib_TypeDescriptionReference * pElementType =
            ((typelib_IndirectTypeDescription *)pTypeDescr)->pType;

        uno_Sequence * pSequence = 0;

        if (pElements)
        {
            __copyConstructElements(
                &pSequence, pElements, pElementType,
                0, len, acquire,
                len ); // alloc to len
        }
        else
        {
            __defaultConstructElements(
                &pSequence,
                pElementType,
                0, len,
                len ); // alloc to len
        }

        *ppSequence = pSequence;
    }
    else
    {
        *ppSequence = __getEmptySequence();
    }
}
//##################################################################################################
SAL_DLLEXPORT void SAL_CALL uno_type_sequence_realloc(
    uno_Sequence ** ppSequence, typelib_TypeDescriptionReference * pType, sal_Int32 nSize,
    uno_AcquireFunc acquire, uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C()
{
    OSL_ENSHURE( ppSequence, "### null ptr!" );
    OSL_ENSHURE( nSize >= 0, "### new size must be at least 0!" );

    if (nSize != (*ppSequence)->nElements)
    {
        typelib_TypeDescription * pTypeDescr = 0;
        TYPELIB_DANGER_GET( &pTypeDescr, pType );
        __reallocSequence(
            ppSequence, ((typelib_IndirectTypeDescription *)pTypeDescr)->pType,
            nSize, acquire, release );
        TYPELIB_DANGER_RELEASE( pTypeDescr );
    }
}
//##################################################################################################
SAL_DLLEXPORT void SAL_CALL uno_sequence_realloc(
    uno_Sequence ** ppSequence, typelib_TypeDescription * pTypeDescr, sal_Int32 nSize,
    uno_AcquireFunc acquire, uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C()
{
    OSL_ENSHURE( ppSequence, "### null ptr!" );
    OSL_ENSHURE( nSize >= 0, "### new size must be at least 0!" );

    if (nSize != (*ppSequence)->nElements)
    {
        __reallocSequence(
            ppSequence, ((typelib_IndirectTypeDescription *)pTypeDescr)->pType,
            nSize, acquire, release );
    }
}
//##################################################################################################
SAL_DLLEXPORT void SAL_CALL uno_type_sequence_reference2One(
    uno_Sequence ** ppSequence,
    typelib_TypeDescriptionReference * pType,
    uno_AcquireFunc acquire, uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C()
{
    OSL_ENSHURE( ppSequence, "### null ptr!" );
    uno_Sequence * pSequence = *ppSequence;
    if (pSequence->nRefCount > 1)
    {
        if (pSequence->nElements)
        {
            typelib_TypeDescription * pTypeDescr = 0;
            TYPELIB_DANGER_GET( &pTypeDescr, pType );

            uno_Sequence * pNew = 0;
            __copyConstructElements(
                &pNew, pSequence->elements,
                ((typelib_IndirectTypeDescription *)pTypeDescr)->pType,
                0, pSequence->nElements, acquire,
                pSequence->nElements ); // alloc nElements

            __destructSequence( *ppSequence, pType, pTypeDescr, release );
            *ppSequence = pNew;

            TYPELIB_DANGER_RELEASE( pTypeDescr );
        }
        else
        {
            __destructSequence( *ppSequence, pType, 0, release );

            uno_Sequence * pNew = (uno_Sequence *)::rtl_allocateMemory( SAL_SEQUENCE_HEADER_SIZE );
            pNew->nRefCount = 1;
            pNew->nElements = 0;
            *ppSequence = pNew;
        }
    }
}
//##################################################################################################
SAL_DLLEXPORT void SAL_CALL uno_sequence_reference2One(
    uno_Sequence ** ppSequence,
    typelib_TypeDescription * pTypeDescr,
    uno_AcquireFunc acquire, uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C()
{
    OSL_ENSHURE( ppSequence, "### null ptr!" );
    uno_Sequence * pSequence = *ppSequence;
    if (pSequence->nRefCount > 1)
    {
        if (pSequence->nElements)
        {
            uno_Sequence * pNew = 0;
            __copyConstructElements(
                &pNew, pSequence->elements,
                ((typelib_IndirectTypeDescription *)pTypeDescr)->pType,
                0, pSequence->nElements, acquire,
                pSequence->nElements ); // alloc nElements

            __destructSequence( *ppSequence, pTypeDescr->pWeakRef, pTypeDescr, release );
            *ppSequence = pNew;
        }
        else
        {
            __destructSequence( *ppSequence, pTypeDescr->pWeakRef, pTypeDescr, release );

            uno_Sequence * pNew = (uno_Sequence *)::rtl_allocateMemory( SAL_SEQUENCE_HEADER_SIZE );
            pNew->nRefCount = 1;
            pNew->nElements = 0;
            *ppSequence = pNew;
        }
    }
}
//##################################################################################################
SAL_DLLEXPORT void SAL_CALL uno_sequence_assign(
    uno_Sequence ** ppDest,
    uno_Sequence * pSource,
    typelib_TypeDescription * pTypeDescr,
    uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C()
{
    if (*ppDest != pSource)
    {
        ::osl_incrementInterlockedCount( &pSource->nRefCount );
        __destructSequence( *ppDest, pTypeDescr->pWeakRef, pTypeDescr, release );
        *ppDest = pSource;
    }
}
//##################################################################################################
SAL_DLLEXPORT void SAL_CALL uno_type_sequence_assign(
    uno_Sequence ** ppDest,
    uno_Sequence * pSource,
    typelib_TypeDescriptionReference * pType,
    uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C()
{
    if (*ppDest != pSource)
    {
        ::osl_incrementInterlockedCount( &pSource->nRefCount );
        __destructSequence( *ppDest, pType, 0, release );
        *ppDest = pSource;
    }
}

}
