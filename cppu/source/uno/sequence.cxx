/*************************************************************************
 *
 *  $RCSfile: sequence.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-04 10:53:42 $
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

//------------------------------------------------------------------------------
static inline uno_Sequence * reallocSeq(
    uno_Sequence * pReallocate, sal_Int32 nElementSize, sal_Int32 nElements )
{
    OSL_ASSERT( nElementSize >= 0 && nElements >= 0 );
    uno_Sequence * pNew = 0;
    sal_uInt32 nSize = calcSeqMemSize( nElementSize, nElements );
    if (nSize > 0)
    {
        if (pReallocate == 0)
        {
            pNew = (uno_Sequence *) rtl_allocateMemory( nSize );
        }
        else
        {
            pNew = (uno_Sequence *) rtl_reallocateMemory( pReallocate, nSize );
        }
        if (pNew != 0)
        {
            // header init
            pNew->nRefCount = 1;
            pNew->nElements = nElements;
        }
    }
    return pNew;
}

//------------------------------------------------------------------------------
static inline bool idefaultConstructElements(
    uno_Sequence ** ppSeq,
    typelib_TypeDescriptionReference * pElementType,
    sal_Int32 nStartIndex, sal_Int32 nStopIndex,
    sal_Int32 nAlloc = -1 ) // >= 0 means (re)alloc memory for nAlloc elements
{
    uno_Sequence * pSeq = *ppSeq;
    switch (pElementType->eTypeClass)
    {
    case typelib_TypeClass_CHAR:
        if (nAlloc >= 0)
            pSeq = reallocSeq( pSeq, sizeof(sal_Unicode), nAlloc );
        if (pSeq != 0)
        {
            ::rtl_zeroMemory(
                pSeq->elements + (sizeof(sal_Unicode) * nStartIndex),
                sizeof(sal_Unicode) * (nStopIndex - nStartIndex) );
        }
        break;
    case typelib_TypeClass_BOOLEAN:
        if (nAlloc >= 0)
            pSeq = reallocSeq( pSeq, sizeof(sal_Bool), nAlloc );
        if (pSeq != 0)
        {
            ::rtl_zeroMemory(
                pSeq->elements + (sizeof(sal_Bool) * nStartIndex),
                sizeof(sal_Bool) * (nStopIndex - nStartIndex) );
        }
        break;
    case typelib_TypeClass_BYTE:
        if (nAlloc >= 0)
            pSeq = reallocSeq( pSeq, sizeof(sal_Int8), nAlloc );
        if (pSeq != 0)
        {
            ::rtl_zeroMemory(
                pSeq->elements + (sizeof(sal_Int8) * nStartIndex),
                sizeof(sal_Int8) * (nStopIndex - nStartIndex) );
        }
        break;
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
        if (nAlloc >= 0)
            pSeq = reallocSeq( pSeq, sizeof(sal_Int16), nAlloc );
        if (pSeq != 0)
        {
            ::rtl_zeroMemory(
                pSeq->elements + (sizeof(sal_Int16) * nStartIndex),
                sizeof(sal_Int16) * (nStopIndex - nStartIndex) );
        }
        break;
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
        if (nAlloc >= 0)
            pSeq = reallocSeq( pSeq, sizeof(sal_Int32), nAlloc );
        if (pSeq != 0)
        {
            ::rtl_zeroMemory(
                pSeq->elements + (sizeof(sal_Int32) * nStartIndex),
                sizeof(sal_Int32) * (nStopIndex - nStartIndex) );
        }
        break;
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
        if (nAlloc >= 0)
            pSeq = reallocSeq( pSeq, sizeof(sal_Int64), nAlloc );
        if (pSeq != 0)
        {
            ::rtl_zeroMemory(
                pSeq->elements + (sizeof(sal_Int64) * nStartIndex),
                sizeof(sal_Int64) * (nStopIndex - nStartIndex) );
        }
        break;
    case typelib_TypeClass_FLOAT:
    {
        if (nAlloc >= 0)
            pSeq = reallocSeq( pSeq, sizeof(float), nAlloc );
        if (pSeq != 0)
        {
            float * pElements = (float *) pSeq->elements;
            for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
            {
                pElements[nPos] = 0.0;
            }
        }
        break;
    }
    case typelib_TypeClass_DOUBLE:
    {
        if (nAlloc >= 0)
            pSeq = reallocSeq( pSeq, sizeof(double), nAlloc );
        if (pSeq != 0)
        {
            double * pElements = (double *) pSeq->elements;
            for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
            {
                pElements[nPos] = 0.0;
            }
        }
        break;
    }
    case typelib_TypeClass_STRING:
    {
        if (nAlloc >= 0)
            pSeq = reallocSeq( pSeq, sizeof(rtl_uString *), nAlloc );
        if (pSeq != 0)
        {
            rtl_uString ** pElements = (rtl_uString **) pSeq->elements;
            for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
            {
                pElements[nPos] = 0;
                rtl_uString_new( &pElements[nPos] );
            }
        }
        break;
    }
    case typelib_TypeClass_TYPE:
    {
        if (nAlloc >= 0)
        {
            pSeq = reallocSeq(
                pSeq, sizeof(typelib_TypeDescriptionReference *), nAlloc );
        }
        if (pSeq != 0)
        {
            typelib_TypeDescriptionReference ** pElements =
                (typelib_TypeDescriptionReference **) pSeq->elements;
            for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
            {
                pElements[nPos] = _getVoidType();
            }
        }
        break;
    }
    case typelib_TypeClass_ANY:
    {
        if (nAlloc >= 0)
            pSeq = reallocSeq( pSeq, sizeof(uno_Any), nAlloc );
        if (pSeq != 0)
        {
            uno_Any * pElements = (uno_Any *) pSeq->elements;
            for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
            {
                CONSTRUCT_EMPTY_ANY( &pElements[nPos] );
            }
        }
        break;
    }
    case typelib_TypeClass_ENUM:
    {
        if (nAlloc >= 0)
            pSeq = reallocSeq( pSeq, sizeof(sal_Int32), nAlloc );
        if (pSeq != 0)
        {
            typelib_TypeDescription * pElementTypeDescr = 0;
            TYPELIB_DANGER_GET( &pElementTypeDescr, pElementType );
            sal_Int32 eEnum =
                ((typelib_EnumTypeDescription *)
                 pElementTypeDescr)->nDefaultEnumValue;
            TYPELIB_DANGER_RELEASE( pElementTypeDescr );

            sal_Int32 * pElements = (sal_Int32 *) pSeq->elements;
            for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
            {
                pElements[nPos] = eEnum;
            }
        }
        break;
    }
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
    {
        typelib_TypeDescription * pElementTypeDescr = 0;
        TYPELIB_DANGER_GET( &pElementTypeDescr, pElementType );
        sal_Int32 nElementSize = pElementTypeDescr->nSize;

        if (nAlloc >= 0)
            pSeq = reallocSeq( pSeq, nElementSize, nAlloc );
        if (pSeq != 0)
        {
            char * pElements = pSeq->elements;
            for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
            {
                _defaultConstructStruct(
                    pElements + (nElementSize * nPos),
                    (typelib_CompoundTypeDescription *)pElementTypeDescr );
            }
        }

        TYPELIB_DANGER_RELEASE( pElementTypeDescr );
        break;
    }
    case typelib_TypeClass_ARRAY:
    {
        typelib_TypeDescription * pElementTypeDescr = 0;
        TYPELIB_DANGER_GET( &pElementTypeDescr, pElementType );
        sal_Int32 nElementSize = pElementTypeDescr->nSize;

        if (nAlloc >= 0)
            pSeq = reallocSeq( pSeq, nElementSize, nAlloc );
        if (pSeq != 0)
        {
            char * pElements = pSeq->elements;
            for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
            {
                _defaultConstructArray(
                    pElements + (nElementSize * nPos),
                    (typelib_ArrayTypeDescription *)pElementTypeDescr );
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

        if (nAlloc >= 0)
            pSeq = reallocSeq( pSeq, nElementSize, nAlloc );
        if (pSeq != 0)
        {
            sal_Int32 nValueOffset =
                ((typelib_UnionTypeDescription *)
                 pElementTypeDescr)->nValueOffset;
            sal_Int64 nDefaultDiscr =
                ((typelib_UnionTypeDescription *)
                 pElementTypeDescr)->nDefaultDiscriminant;

            typelib_TypeDescription * pDefaultTypeDescr = 0;
            TYPELIB_DANGER_GET(
                &pDefaultTypeDescr,
                ((typelib_UnionTypeDescription *)
                 pElementTypeDescr)->pDefaultTypeRef );

            char * pElements = pSeq->elements;
            for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
            {
                char * pMem = pElements + (nElementSize * nPos);
                ::uno_constructData(
                    (char *)pMem + nValueOffset, pDefaultTypeDescr );
                *(sal_Int64 *)pMem = nDefaultDiscr;
            }
            TYPELIB_DANGER_RELEASE( pDefaultTypeDescr );
        }

        TYPELIB_DANGER_RELEASE( pElementTypeDescr );
        break;
    }
    case typelib_TypeClass_SEQUENCE:
    {
        if (nAlloc >= 0)
            pSeq = reallocSeq( pSeq, sizeof(uno_Sequence *), nAlloc );
        if (pSeq != 0)
        {
            uno_Sequence ** pElements =
                (uno_Sequence **) pSeq->elements;
            for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
            {
                pElements[nPos] = createEmptySequence();
            }
        }
        break;
    }
    case typelib_TypeClass_INTERFACE: // either C++ or C-UNO interface
        if (nAlloc >= 0)
            pSeq = reallocSeq( pSeq, sizeof(void *), nAlloc );
        if (pSeq != 0)
        {
            ::rtl_zeroMemory(
                pSeq->elements + (sizeof(void *) * nStartIndex),
                sizeof(void *) * (nStopIndex - nStartIndex) );
        }
        break;
    default:
        OSL_ENSURE( 0, "### unexpected element type!" );
        pSeq = 0;
        break;
    }

    if (pSeq == 0)
    {
        OSL_ASSERT( nAlloc >= 0 ); // must have been an allocation failure
        return false;
    }
    else
    {
        *ppSeq = pSeq;
        return true;
    }
}

//------------------------------------------------------------------------------
static inline bool icopyConstructFromElements(
    uno_Sequence ** ppSeq, void * pSourceElements,
    typelib_TypeDescriptionReference * pElementType,
    sal_Int32 nStartIndex, sal_Int32 nStopIndex,
    uno_AcquireFunc acquire,
    sal_Int32 nAlloc = -1 ) // >= 0 means (re)alloc memory for nAlloc elements
{
    uno_Sequence * pSeq = *ppSeq;
    switch (pElementType->eTypeClass)
    {
    case typelib_TypeClass_CHAR:
        if (nAlloc >= 0)
            pSeq = reallocSeq( pSeq, sizeof(sal_Unicode), nAlloc );
        if (pSeq != 0)
        {
            ::rtl_copyMemory(
                pSeq->elements + (sizeof(sal_Unicode) * nStartIndex),
                (char *)pSourceElements + (sizeof(sal_Unicode) * nStartIndex),
                sizeof(sal_Unicode) * (nStopIndex - nStartIndex) );
        }
        break;
    case typelib_TypeClass_BOOLEAN:
        if (nAlloc >= 0)
            pSeq = reallocSeq( pSeq, sizeof(sal_Bool), nAlloc );
        if (pSeq != 0)
        {
            ::rtl_copyMemory(
                pSeq->elements + (sizeof(sal_Bool) * nStartIndex),
                (char *)pSourceElements + (sizeof(sal_Bool) * nStartIndex),
                sizeof(sal_Bool) * (nStopIndex - nStartIndex) );
        }
        break;
    case typelib_TypeClass_BYTE:
        if (nAlloc >= 0)
            pSeq = reallocSeq( pSeq, sizeof(sal_Int8), nAlloc );
        if (pSeq != 0)
        {
            ::rtl_copyMemory(
                pSeq->elements + (sizeof(sal_Int8) * nStartIndex),
                (char *)pSourceElements + (sizeof(sal_Int8) * nStartIndex),
                sizeof(sal_Int8) * (nStopIndex - nStartIndex) );
        }
        break;
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
        if (nAlloc >= 0)
            pSeq = reallocSeq( pSeq, sizeof(sal_Int16), nAlloc );
        if (pSeq != 0)
        {
            ::rtl_copyMemory(
                pSeq->elements + (sizeof(sal_Int16) * nStartIndex),
                (char *)pSourceElements + (sizeof(sal_Int16) * nStartIndex),
                sizeof(sal_Int16) * (nStopIndex - nStartIndex) );
        }
        break;
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
        if (nAlloc >= 0)
            pSeq = reallocSeq( pSeq, sizeof(sal_Int32), nAlloc );
        if (pSeq != 0)
        {
            ::rtl_copyMemory(
                pSeq->elements + (sizeof(sal_Int32) * nStartIndex),
                (char *)pSourceElements + (sizeof(sal_Int32) * nStartIndex),
                sizeof(sal_Int32) * (nStopIndex - nStartIndex) );
        }
        break;
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
        if (nAlloc >= 0)
            pSeq = reallocSeq( pSeq, sizeof(sal_Int64), nAlloc );
        if (pSeq != 0)
        {
            ::rtl_copyMemory(
                pSeq->elements + (sizeof(sal_Int64) * nStartIndex),
                (char *)pSourceElements + (sizeof(sal_Int64) * nStartIndex),
                sizeof(sal_Int64) * (nStopIndex - nStartIndex) );
        }
        break;
    case typelib_TypeClass_FLOAT:
        if (nAlloc >= 0)
            pSeq = reallocSeq( pSeq, sizeof(float), nAlloc );
        if (pSeq != 0)
        {
            ::rtl_copyMemory(
                pSeq->elements + (sizeof(float) * nStartIndex),
                (char *)pSourceElements + (sizeof(float) * nStartIndex),
                sizeof(float) * (nStopIndex - nStartIndex) );
        }
        break;
    case typelib_TypeClass_DOUBLE:
        if (nAlloc >= 0)
            pSeq = reallocSeq( pSeq, sizeof(double), nAlloc );
        if (pSeq != 0)
        {
            ::rtl_copyMemory(
                pSeq->elements + (sizeof(double) * nStartIndex),
                (char *)pSourceElements + (sizeof(double) * nStartIndex),
                sizeof(double) * (nStopIndex - nStartIndex) );
        }
        break;
    case typelib_TypeClass_ENUM:
        if (nAlloc >= 0)
            pSeq = reallocSeq( pSeq, sizeof(sal_Int32), nAlloc );
        if (pSeq != 0)
        {
            ::rtl_copyMemory(
                pSeq->elements + (sizeof(sal_Int32) * nStartIndex),
                (char *)pSourceElements + (sizeof(sal_Int32) * nStartIndex),
                sizeof(sal_Int32) * (nStopIndex - nStartIndex) );
        }
        break;
    case typelib_TypeClass_STRING:
    {
        if (nAlloc >= 0)
            pSeq = reallocSeq( pSeq, sizeof(rtl_uString *), nAlloc );
        if (pSeq != 0)
        {
            rtl_uString ** pDestElements = (rtl_uString **) pSeq->elements;
            for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
            {
                ::rtl_uString_acquire(
                    ((rtl_uString **)pSourceElements)[nPos] );
                pDestElements[nPos] = ((rtl_uString **)pSourceElements)[nPos];
            }
        }
        break;
    }
    case typelib_TypeClass_TYPE:
    {
        if (nAlloc >= 0)
        {
            pSeq = reallocSeq(
                pSeq, sizeof(typelib_TypeDescriptionReference *), nAlloc );
        }
        if (pSeq != 0)
        {
            typelib_TypeDescriptionReference ** pDestElements =
                (typelib_TypeDescriptionReference **) pSeq->elements;
            for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
            {
                TYPE_ACQUIRE(
                    ((typelib_TypeDescriptionReference **)
                     pSourceElements)[nPos] );
                pDestElements[nPos] =
                    ((typelib_TypeDescriptionReference **)
                     pSourceElements)[nPos];
            }
        }
        break;
    }
    case typelib_TypeClass_ANY:
    {
        if (nAlloc >= 0)
            pSeq = reallocSeq( pSeq, sizeof(uno_Any), nAlloc );
        if (pSeq != 0)
        {
            uno_Any * pDestElements = (uno_Any *) pSeq->elements;
            for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
            {
                uno_Any * pSource = (uno_Any *)pSourceElements + nPos;
                _copyConstructAny(
                    &pDestElements[nPos],
                    pSource->pData,
                    pSource->pType, 0,
                    acquire, 0 );
            }
        }
        break;
    }
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
    {
        typelib_TypeDescription * pElementTypeDescr = 0;
        TYPELIB_DANGER_GET( &pElementTypeDescr, pElementType );
        sal_Int32 nElementSize = pElementTypeDescr->nSize;

        if (nAlloc >= 0)
            pSeq = reallocSeq( pSeq, nElementSize, nAlloc );
        if (pSeq != 0)
        {
            char * pDestElements = pSeq->elements;

            typelib_CompoundTypeDescription * pTypeDescr =
                (typelib_CompoundTypeDescription *)pElementTypeDescr;
            for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
            {
                char * pDest =
                    pDestElements + (nElementSize * nPos);
                char * pSource =
                    (char *)pSourceElements + (nElementSize * nPos);

                if (pTypeDescr->pBaseTypeDescription)
                {
                    // copy base value
                    _copyConstructStruct(
                        pDest, pSource,
                        pTypeDescr->pBaseTypeDescription, acquire, 0 );
                }

                // then copy members
                typelib_TypeDescriptionReference ** ppTypeRefs =
                    pTypeDescr->ppTypeRefs;
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
        }

        TYPELIB_DANGER_RELEASE( pElementTypeDescr );
        break;
    }
    case typelib_TypeClass_UNION:
    {
        typelib_TypeDescription * pElementTypeDescr = 0;
        TYPELIB_DANGER_GET( &pElementTypeDescr, pElementType );
        sal_Int32 nElementSize = pElementTypeDescr->nSize;

        if (nAlloc >= 0)
            pSeq = reallocSeq( pSeq, nElementSize, nAlloc );
        if (pSeq != 0)
        {
            char * pDestElements = pSeq->elements;

            sal_Int32 nValueOffset =
                ((typelib_UnionTypeDescription *)
                 pElementTypeDescr)->nValueOffset;
            for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
            {
                char * pDest =
                    pDestElements + (nElementSize * nPos);
                char * pSource =
                    (char *)pSourceElements + (nElementSize * nPos);

                typelib_TypeDescriptionReference * pSetType = _unionGetSetType(
                    pSource, pElementTypeDescr );
                ::uno_type_copyData(
                    pDest + nValueOffset,
                    pSource + nValueOffset,
                    pSetType, acquire );
                *(sal_Int64 *)pDest = *(sal_Int64 *)pSource;
                typelib_typedescriptionreference_release( pSetType );
            }
        }

        TYPELIB_DANGER_RELEASE( pElementTypeDescr );
        break;
    }
    case typelib_TypeClass_SEQUENCE: // sequence of sequence
    {
        if (nAlloc >= 0)
            pSeq = reallocSeq( pSeq, sizeof(uno_Sequence *), nAlloc );
        if (pSeq != 0)
        {
            typelib_TypeDescription * pElementTypeDescr = 0;
            TYPELIB_DANGER_GET( &pElementTypeDescr, pElementType );
            typelib_TypeDescriptionReference * pSeqElementType =
                ((typelib_IndirectTypeDescription *) pElementTypeDescr)->pType;
            uno_Sequence ** pDestElements = (uno_Sequence **) pSeq->elements;
            for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
            {
                uno_Sequence * pNew = icopyConstructSequence(
                    ((uno_Sequence **) pSourceElements)[nPos],
                    pSeqElementType, acquire, 0 );
                OSL_ASSERT( pNew != 0 );
                // ought never be a memory allocation problem,
                // because of reference counted sequence handles
                pDestElements[ nPos ] = pNew;
            }
            TYPELIB_DANGER_RELEASE( pElementTypeDescr );
        }
        break;
    }
    case typelib_TypeClass_INTERFACE:
    {
        if (nAlloc >= 0)
            pSeq = reallocSeq( pSeq, sizeof(void *), nAlloc );
        if (pSeq != 0)
        {
            void ** pDestElements = (void **) pSeq->elements;
            for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
            {
                _acquire( pDestElements[nPos] =
                          ((void **)pSourceElements)[nPos], acquire );
            }
        }
        break;
    }
    default:
        OSL_ENSURE( 0, "### unexpected element type!" );
        pSeq = 0;
        break;
    }

    if (pSeq == 0)
    {
        OSL_ASSERT( nAlloc >= 0 ); // must have been an allocation failure
        return false;
    }
    else
    {
        *ppSeq = pSeq;
        return true;
    }
}

//------------------------------------------------------------------------------
static inline bool ireallocSequence(
    uno_Sequence ** ppSequence,
    typelib_TypeDescriptionReference * pElementType,
    sal_Int32 nSize,
    uno_AcquireFunc acquire, uno_ReleaseFunc release )
{
    bool ret = true;
    uno_Sequence * pSeq = *ppSequence;
    sal_Int32 nElements = pSeq->nElements;

    if (pSeq->nRefCount > 1 ||
        // not mem-copyable elements?
        typelib_TypeClass_ANY == pElementType->eTypeClass ||
        typelib_TypeClass_STRUCT == pElementType->eTypeClass ||
        typelib_TypeClass_EXCEPTION == pElementType->eTypeClass)
    {
        // split sequence and construct new one from scratch
        uno_Sequence * pNew = 0;

        sal_Int32 nRest = nSize - nElements;
        sal_Int32 nCopy = (nRest > 0 ? nElements : nSize);

        if (nCopy >= 0)
        {
            ret = icopyConstructFromElements(
                &pNew, pSeq->elements, pElementType,
                0, nCopy, acquire,
                nSize ); // alloc to nSize
        }
        if (ret && nRest > 0)
        {
            ret = idefaultConstructElements(
                &pNew, pElementType,
                nCopy, nSize,
                nCopy >= 0 ? -1 /* no mem allocation */ : nSize );
        }

        if (ret)
        {
            // destruct sequence
            if (osl_decrementInterlockedCount( &pSeq->nRefCount ) == 0)
            {
                if (nElements > 0)
                {
                    idestructElements(
                        pSeq->elements, pElementType,
                        0, nElements, release );
                }
                rtl_freeMemory( pSeq );
            }
            *ppSequence = pNew;
        }
    }
    else
    {
        OSL_ASSERT( pSeq->nRefCount == 1 );
        if (nSize > nElements) // default construct the rest
        {
            ret = idefaultConstructElements(
                ppSequence, pElementType,
                nElements, nSize,
                nSize ); // realloc to nSize
        }
        else // or destruct the rest and realloc mem
        {
            sal_Int32 nElementSize = idestructElements(
                pSeq->elements, pElementType,
                nSize, nElements, release );
            // warning: it is assumed that the following will never fail,
            //          else this leads to a sequence null handle
            *ppSequence = reallocSeq( pSeq, nElementSize, nSize );
            OSL_ASSERT( *ppSequence != 0 );
            ret = (*ppSequence != 0);
        }
    }

    return ret;
}

}

extern "C"
{

//##############################################################################
sal_Bool SAL_CALL uno_type_sequence_construct(
    uno_Sequence ** ppSequence, typelib_TypeDescriptionReference * pType,
    void * pElements, sal_Int32 len,
    uno_AcquireFunc acquire )
    SAL_THROW_EXTERN_C()
{
    bool ret;
    if (len)
    {
        typelib_TypeDescription * pTypeDescr = 0;
        TYPELIB_DANGER_GET( &pTypeDescr, pType );

        typelib_TypeDescriptionReference * pElementType =
            ((typelib_IndirectTypeDescription *)pTypeDescr)->pType;

        *ppSequence = 0;
        if (pElements == 0)
        {
            ret = idefaultConstructElements(
                ppSequence, pElementType,
                0, len,
                len ); // alloc to len
        }
        else
        {
            ret = icopyConstructFromElements(
                ppSequence, pElements, pElementType,
                0, len, acquire,
                len ); // alloc to len
        }

        TYPELIB_DANGER_RELEASE( pTypeDescr );
    }
    else
    {
        *ppSequence = createEmptySequence();
        ret = true;
    }

    OSL_ASSERT( (*ppSequence != 0) == ret );
    return ret;
}

//##############################################################################
sal_Bool SAL_CALL uno_sequence_construct(
    uno_Sequence ** ppSequence, typelib_TypeDescription * pTypeDescr,
    void * pElements, sal_Int32 len,
    uno_AcquireFunc acquire )
    SAL_THROW_EXTERN_C()
{
    bool ret;
    if (len > 0)
    {
        typelib_TypeDescriptionReference * pElementType =
            ((typelib_IndirectTypeDescription *)pTypeDescr)->pType;

        *ppSequence = 0;
        if (pElements == 0)
        {
            ret = idefaultConstructElements(
                ppSequence, pElementType,
                0, len,
                len ); // alloc to len
        }
        else
        {
            ret = icopyConstructFromElements(
                ppSequence, pElements, pElementType,
                0, len, acquire,
                len ); // alloc to len
        }
    }
    else
    {
        *ppSequence = createEmptySequence();
        ret = true;
    }

    OSL_ASSERT( (*ppSequence != 0) == ret );
    return ret;
}

//##############################################################################
sal_Bool SAL_CALL uno_type_sequence_realloc(
    uno_Sequence ** ppSequence, typelib_TypeDescriptionReference * pType,
    sal_Int32 nSize, uno_AcquireFunc acquire, uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C()
{
    OSL_ENSURE( ppSequence, "### null ptr!" );
    OSL_ENSURE( nSize >= 0, "### new size must be at least 0!" );

    bool ret = true;
    if (nSize != (*ppSequence)->nElements)
    {
        typelib_TypeDescription * pTypeDescr = 0;
        TYPELIB_DANGER_GET( &pTypeDescr, pType );
        ret = ireallocSequence(
            ppSequence, ((typelib_IndirectTypeDescription *)pTypeDescr)->pType,
            nSize, acquire, release );
        TYPELIB_DANGER_RELEASE( pTypeDescr );
    }
    return ret;
}

//##############################################################################
sal_Bool SAL_CALL uno_sequence_realloc(
    uno_Sequence ** ppSequence, typelib_TypeDescription * pTypeDescr,
    sal_Int32 nSize, uno_AcquireFunc acquire, uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C()
{
    OSL_ENSURE( ppSequence, "### null ptr!" );
    OSL_ENSURE( nSize >= 0, "### new size must be at least 0!" );

    bool ret = true;
    if (nSize != (*ppSequence)->nElements)
    {
        ret = ireallocSequence(
            ppSequence, ((typelib_IndirectTypeDescription *)pTypeDescr)->pType,
            nSize, acquire, release );
    }
    return ret;
}

//##############################################################################
sal_Bool SAL_CALL uno_type_sequence_reference2One(
    uno_Sequence ** ppSequence,
    typelib_TypeDescriptionReference * pType,
    uno_AcquireFunc acquire, uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C()
{
    OSL_ENSURE( ppSequence, "### null ptr!" );
    bool ret = true;
    uno_Sequence * pSequence = *ppSequence;
    if (pSequence->nRefCount > 1)
    {
        uno_Sequence * pNew = 0;
        if (pSequence->nElements > 0)
        {
            typelib_TypeDescription * pTypeDescr = 0;
            TYPELIB_DANGER_GET( &pTypeDescr, pType );

            ret = icopyConstructFromElements(
                &pNew, pSequence->elements,
                ((typelib_IndirectTypeDescription *)pTypeDescr)->pType,
                0, pSequence->nElements, acquire,
                pSequence->nElements ); // alloc nElements
            if (ret)
            {
                idestructSequence( *ppSequence, pType, pTypeDescr, release );
                *ppSequence = pNew;
            }

            TYPELIB_DANGER_RELEASE( pTypeDescr );
        }
        else
        {
            pNew = allocSeq( 0, 0 );
            ret = (pNew != 0);
            if (ret)
            {
                // easy destruction of empty sequence:
                if (osl_decrementInterlockedCount( &pSequence->nRefCount ) == 0)
                    rtl_freeMemory( pSequence );
                *ppSequence = pNew;
            }
        }
    }
    return ret;
}

//##############################################################################
sal_Bool SAL_CALL uno_sequence_reference2One(
    uno_Sequence ** ppSequence,
    typelib_TypeDescription * pTypeDescr,
    uno_AcquireFunc acquire, uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C()
{
    OSL_ENSURE( ppSequence, "### null ptr!" );
    bool ret = true;
    uno_Sequence * pSequence = *ppSequence;
    if (pSequence->nRefCount > 1)
    {
        uno_Sequence * pNew = 0;
        if (pSequence->nElements > 0)
        {
            ret = icopyConstructFromElements(
                &pNew, pSequence->elements,
                ((typelib_IndirectTypeDescription *)pTypeDescr)->pType,
                0, pSequence->nElements, acquire,
                pSequence->nElements ); // alloc nElements
            if (ret)
            {
                idestructSequence(
                    pSequence, pTypeDescr->pWeakRef, pTypeDescr, release );
                *ppSequence = pNew;
            }
        }
        else
        {
            pNew = allocSeq( 0, 0 );
            ret = (pNew != 0);
            if (ret)
            {
                // easy destruction of empty sequence:
                if (osl_decrementInterlockedCount( &pSequence->nRefCount ) == 0)
                    rtl_freeMemory( pSequence );
                *ppSequence = pNew;
            }
        }

    }
    return ret;
}

//##############################################################################
void SAL_CALL uno_sequence_assign(
    uno_Sequence ** ppDest,
    uno_Sequence * pSource,
    typelib_TypeDescription * pTypeDescr,
    uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C()
{
    if (*ppDest != pSource)
    {
        ::osl_incrementInterlockedCount( &pSource->nRefCount );
        idestructSequence( *ppDest, pTypeDescr->pWeakRef, pTypeDescr, release );
        *ppDest = pSource;
    }
}

//##############################################################################
void SAL_CALL uno_type_sequence_assign(
    uno_Sequence ** ppDest,
    uno_Sequence * pSource,
    typelib_TypeDescriptionReference * pType,
    uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C()
{
    if (*ppDest != pSource)
    {
        ::osl_incrementInterlockedCount( &pSource->nRefCount );
        idestructSequence( *ppDest, pType, 0, release );
        *ppDest = pSource;
    }
}

}
