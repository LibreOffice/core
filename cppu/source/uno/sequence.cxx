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

#include <sal/config.h>

#include <cassert>
#include <string.h>

#include <osl/diagnose.h>
#include <osl/interlck.h>
#include <typelib/typedescription.h>
#include <uno/data.h>
#include <uno/sequence2.h>

#include "constr.hxx"
#include "copy.hxx"
#include "destr.hxx"


using namespace cppu;

namespace cppu
{


static uno_Sequence * reallocSeq(
    uno_Sequence * pReallocate, std::size_t nElementSize, sal_Int32 nElements )
{
    OSL_ASSERT( nElements >= 0 );
    uno_Sequence * pNew = nullptr;
    sal_uInt32 nSize = calcSeqMemSize( nElementSize, nElements );
    if (nSize > 0)
    {
        if (pReallocate == nullptr)
        {
            pNew = static_cast<uno_Sequence *>(std::malloc( nSize ));
        }
        else
        {
            pNew = static_cast<uno_Sequence *>(std::realloc( pReallocate, nSize ));
        }
        if (pNew != nullptr)
        {
            // header init
            pNew->nRefCount = 1;
            pNew->nElements = nElements;
        }
    }
    return pNew;
}


static bool idefaultConstructElements(
    uno_Sequence ** ppSeq,
    typelib_TypeDescriptionReference * pElementType,
    sal_Int32 nStartIndex, sal_Int32 nStopIndex,
    sal_Int32 nAlloc ) // >= 0 means (re)alloc memory for nAlloc elements
{
    uno_Sequence * pSeq = *ppSeq;
    switch (pElementType->eTypeClass)
    {
    case typelib_TypeClass_CHAR:
        if (nAlloc >= 0)
            pSeq = reallocSeq( pSeq, sizeof(sal_Unicode), nAlloc );
        if (pSeq != nullptr)
        {
            memset(
                pSeq->elements + (sizeof(sal_Unicode) * nStartIndex),
                0,
                sizeof(sal_Unicode) * (nStopIndex - nStartIndex) );
        }
        break;
    case typelib_TypeClass_BOOLEAN:
        if (nAlloc >= 0)
            pSeq = reallocSeq( pSeq, sizeof(sal_Bool), nAlloc );
        if (pSeq != nullptr)
        {
            memset(
                pSeq->elements + (sizeof(sal_Bool) * nStartIndex),
                0,
                sizeof(sal_Bool) * (nStopIndex - nStartIndex) );
        }
        break;
    case typelib_TypeClass_BYTE:
        if (nAlloc >= 0)
            pSeq = reallocSeq( pSeq, sizeof(sal_Int8), nAlloc );
        if (pSeq != nullptr)
        {
            memset(
                pSeq->elements + (sizeof(sal_Int8) * nStartIndex),
                0,
                sizeof(sal_Int8) * (nStopIndex - nStartIndex) );
        }
        break;
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
        if (nAlloc >= 0)
            pSeq = reallocSeq( pSeq, sizeof(sal_Int16), nAlloc );
        if (pSeq != nullptr)
        {
            memset(
                pSeq->elements + (sizeof(sal_Int16) * nStartIndex),
                0,
                sizeof(sal_Int16) * (nStopIndex - nStartIndex) );
        }
        break;
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
        if (nAlloc >= 0)
            pSeq = reallocSeq( pSeq, sizeof(sal_Int32), nAlloc );
        if (pSeq != nullptr)
        {
            memset(
                pSeq->elements + (sizeof(sal_Int32) * nStartIndex),
                0,
                sizeof(sal_Int32) * (nStopIndex - nStartIndex) );
        }
        break;
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
        if (nAlloc >= 0)
            pSeq = reallocSeq( pSeq, sizeof(sal_Int64), nAlloc );
        if (pSeq != nullptr)
        {
            memset(
                pSeq->elements + (sizeof(sal_Int64) * nStartIndex),
                0,
                sizeof(sal_Int64) * (nStopIndex - nStartIndex) );
        }
        break;
    case typelib_TypeClass_FLOAT:
    {
        if (nAlloc >= 0)
            pSeq = reallocSeq( pSeq, sizeof(float), nAlloc );
        if (pSeq != nullptr)
        {
            float * pElements = reinterpret_cast<float *>(pSeq->elements);
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
        if (pSeq != nullptr)
        {
            double * pElements = reinterpret_cast<double *>(pSeq->elements);
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
        if (pSeq != nullptr)
        {
            rtl_uString ** pElements = reinterpret_cast<rtl_uString **>(pSeq->elements);
            for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
            {
                pElements[nPos] = nullptr;
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
        if (pSeq != nullptr)
        {
            typelib_TypeDescriptionReference ** pElements =
                reinterpret_cast<typelib_TypeDescriptionReference **>(pSeq->elements);
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
        if (pSeq != nullptr)
        {
            uno_Any * pElements = reinterpret_cast<uno_Any *>(pSeq->elements);
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
        if (pSeq != nullptr)
        {
            typelib_TypeDescription * pElementTypeDescr = nullptr;
            TYPELIB_DANGER_GET( &pElementTypeDescr, pElementType );
            sal_Int32 eEnum =
                reinterpret_cast<typelib_EnumTypeDescription *>(
                 pElementTypeDescr)->nDefaultEnumValue;
            TYPELIB_DANGER_RELEASE( pElementTypeDescr );

            sal_Int32 * pElements = reinterpret_cast<sal_Int32 *>(pSeq->elements);
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
        typelib_TypeDescription * pElementTypeDescr = nullptr;
        TYPELIB_DANGER_GET( &pElementTypeDescr, pElementType );
        sal_Int32 nElementSize = pElementTypeDescr->nSize;

        if (nAlloc >= 0)
            pSeq = reallocSeq( pSeq, nElementSize, nAlloc );
        if (pSeq != nullptr)
        {
            char * pElements = pSeq->elements;
            for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
            {
                _defaultConstructStruct(
                    pElements + (nElementSize * nPos),
                    reinterpret_cast<typelib_CompoundTypeDescription *>(pElementTypeDescr) );
            }
        }

        TYPELIB_DANGER_RELEASE( pElementTypeDescr );
        break;
    }
    case typelib_TypeClass_SEQUENCE:
    {
        if (nAlloc >= 0)
        {
            // coverity[suspicious_sizeof : FALSE] - sizeof(uno_Sequence*) is correct here
            pSeq = reallocSeq(pSeq, sizeof(uno_Sequence*), nAlloc);
        }
        if (pSeq != nullptr)
        {
            uno_Sequence ** pElements =
                reinterpret_cast<uno_Sequence **>(pSeq->elements);
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
        if (pSeq != nullptr)
        {
            memset(
                pSeq->elements + (sizeof(void *) * nStartIndex),
                0,
                sizeof(void *) * (nStopIndex - nStartIndex) );
        }
        break;
    default:
        OSL_FAIL( "### unexpected element type!" );
        pSeq = nullptr;
        break;
    }

    if (pSeq == nullptr)
    {
        OSL_ASSERT( nAlloc >= 0 ); // must have been an allocation failure
        return false;
    }
    *ppSeq = pSeq;
    return true;
}


static bool icopyConstructFromElements(
    uno_Sequence ** ppSeq, void * pSourceElements,
    typelib_TypeDescriptionReference * pElementType,
    sal_Int32 nStopIndex,
    uno_AcquireFunc acquire,
    sal_Int32 nAlloc )
{
    uno_Sequence * pSeq = *ppSeq;
    switch (pElementType->eTypeClass)
    {
    case typelib_TypeClass_CHAR:
        pSeq = reallocSeq( pSeq, sizeof(sal_Unicode), nAlloc );
        if (pSeq != nullptr)
        {
            memcpy(
                pSeq->elements,
                pSourceElements,
                sizeof(sal_Unicode) * nStopIndex );
        }
        break;
    case typelib_TypeClass_BOOLEAN:
        pSeq = reallocSeq( pSeq, sizeof(sal_Bool), nAlloc );
        if (pSeq != nullptr)
        {
            memcpy(
                pSeq->elements,
                pSourceElements,
                sizeof(sal_Bool) * nStopIndex );
        }
        break;
    case typelib_TypeClass_BYTE:
        pSeq = reallocSeq( pSeq, sizeof(sal_Int8), nAlloc );
        if (pSeq != nullptr)
        {
            memcpy(
                pSeq->elements,
                pSourceElements,
                sizeof(sal_Int8) * nStopIndex );
        }
        break;
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
        pSeq = reallocSeq( pSeq, sizeof(sal_Int16), nAlloc );
        if (pSeq != nullptr)
        {
            memcpy(
                pSeq->elements,
                pSourceElements,
                sizeof(sal_Int16) * nStopIndex );
        }
        break;
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
        pSeq = reallocSeq( pSeq, sizeof(sal_Int32), nAlloc );
        if (pSeq != nullptr)
        {
            memcpy(
                pSeq->elements,
                pSourceElements,
                sizeof(sal_Int32) * nStopIndex );
        }
        break;
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
        pSeq = reallocSeq( pSeq, sizeof(sal_Int64), nAlloc );
        if (pSeq != nullptr)
        {
            memcpy(
                pSeq->elements,
                pSourceElements,
                sizeof(sal_Int64) * nStopIndex );
        }
        break;
    case typelib_TypeClass_FLOAT:
        pSeq = reallocSeq( pSeq, sizeof(float), nAlloc );
        if (pSeq != nullptr)
        {
            memcpy(
                pSeq->elements,
                pSourceElements,
                sizeof(float) * nStopIndex );
        }
        break;
    case typelib_TypeClass_DOUBLE:
        pSeq = reallocSeq( pSeq, sizeof(double), nAlloc );
        if (pSeq != nullptr)
        {
            memcpy(
                pSeq->elements,
                pSourceElements,
                sizeof(double) * nStopIndex );
        }
        break;
    case typelib_TypeClass_ENUM:
        pSeq = reallocSeq( pSeq, sizeof(sal_Int32), nAlloc );
        if (pSeq != nullptr)
        {
            memcpy(
                pSeq->elements,
                pSourceElements,
                sizeof(sal_Int32) * nStopIndex );
        }
        break;
    case typelib_TypeClass_STRING:
    {
        pSeq = reallocSeq( pSeq, sizeof(rtl_uString *), nAlloc );
        if (pSeq != nullptr)
        {
            rtl_uString ** pDestElements = reinterpret_cast<rtl_uString **>(pSeq->elements);
            for ( sal_Int32 nPos = 0; nPos < nStopIndex; ++nPos )
            {
                // This code tends to trigger coverity's overrun-buffer-arg warning
                // coverity[index_parm_via_loop_bound] - https://communities.coverity.com/thread/2993
                ::rtl_uString_acquire(
                    static_cast<rtl_uString **>(pSourceElements)[nPos] );
                pDestElements[nPos] = static_cast<rtl_uString **>(pSourceElements)[nPos];
            }
        }
        break;
    }
    case typelib_TypeClass_TYPE:
    {
        pSeq = reallocSeq(
            pSeq, sizeof(typelib_TypeDescriptionReference *), nAlloc );
        if (pSeq != nullptr)
        {
            typelib_TypeDescriptionReference ** pDestElements =
                reinterpret_cast<typelib_TypeDescriptionReference **>(pSeq->elements);
            for ( sal_Int32 nPos = 0; nPos < nStopIndex; ++nPos )
            {
                TYPE_ACQUIRE(
                    static_cast<typelib_TypeDescriptionReference **>(
                     pSourceElements)[nPos] );
                pDestElements[nPos] =
                    static_cast<typelib_TypeDescriptionReference **>(
                     pSourceElements)[nPos];
            }
        }
        break;
    }
    case typelib_TypeClass_ANY:
    {
        pSeq = reallocSeq( pSeq, sizeof(uno_Any), nAlloc );
        if (pSeq != nullptr)
        {
            uno_Any * pDestElements = reinterpret_cast<uno_Any *>(pSeq->elements);
            for ( sal_Int32 nPos = 0; nPos < nStopIndex; ++nPos )
            {
                uno_Any * pSource = static_cast<uno_Any *>(pSourceElements) + nPos;
                _copyConstructAny(
                    &pDestElements[nPos],
                    pSource->pData,
                    pSource->pType, nullptr,
                    acquire, nullptr );
            }
        }
        break;
    }
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
    {
        typelib_TypeDescription * pElementTypeDescr = nullptr;
        TYPELIB_DANGER_GET( &pElementTypeDescr, pElementType );
        sal_Int32 nElementSize = pElementTypeDescr->nSize;

        pSeq = reallocSeq( pSeq, nElementSize, nAlloc );
        if (pSeq != nullptr)
        {
            char * pDestElements = pSeq->elements;

            typelib_CompoundTypeDescription * pTypeDescr =
                reinterpret_cast<typelib_CompoundTypeDescription *>(pElementTypeDescr);
            for ( sal_Int32 nPos = 0; nPos < nStopIndex; ++nPos )
            {
                char * pDest =
                    pDestElements + (nElementSize * nPos);
                char * pSource =
                    static_cast<char *>(pSourceElements) + (nElementSize * nPos);

                if (pTypeDescr->pBaseTypeDescription)
                {
                    // copy base value
                    _copyConstructStruct(
                        pDest, pSource,
                        pTypeDescr->pBaseTypeDescription, acquire, nullptr );
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
    case typelib_TypeClass_SEQUENCE: // sequence of sequence
    {
        // coverity[suspicious_sizeof : FALSE] - sizeof(uno_Sequence*) is correct here
        pSeq = reallocSeq(pSeq, sizeof(uno_Sequence*), nAlloc);
        if (pSeq != nullptr)
        {
            typelib_TypeDescription * pElementTypeDescr = nullptr;
            TYPELIB_DANGER_GET( &pElementTypeDescr, pElementType );
            typelib_TypeDescriptionReference * pSeqElementType =
                reinterpret_cast<typelib_IndirectTypeDescription *>(pElementTypeDescr)->pType;
            uno_Sequence ** pDestElements = reinterpret_cast<uno_Sequence **>(pSeq->elements);
            for ( sal_Int32 nPos = 0; nPos < nStopIndex; ++nPos )
            {
                uno_Sequence * pNew = icopyConstructSequence(
                    static_cast<uno_Sequence **>(pSourceElements)[nPos],
                    pSeqElementType, acquire, nullptr );
                OSL_ASSERT( pNew != nullptr );
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
        pSeq = reallocSeq( pSeq, sizeof(void *), nAlloc );
        if (pSeq != nullptr)
        {
            void ** pDestElements = reinterpret_cast<void **>(pSeq->elements);
            for ( sal_Int32 nPos = 0; nPos < nStopIndex; ++nPos )
            {
                _acquire( pDestElements[nPos] =
                          static_cast<void **>(pSourceElements)[nPos], acquire );
            }
        }
        break;
    }
    default:
        OSL_FAIL( "### unexpected element type!" );
        pSeq = nullptr;
        break;
    }

    if (pSeq == nullptr)
    {
        return false; // allocation failure
    }
    *ppSeq = pSeq;
    return true;
}


static bool ireallocSequence(
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
        uno_Sequence * pNew = nullptr;

        sal_Int32 nRest = nSize - nElements;
        sal_Int32 nCopy = (nRest > 0 ? nElements : nSize);

        if (nCopy >= 0)
        {
            ret = icopyConstructFromElements(
                &pNew, pSeq->elements, pElementType,
                nCopy, acquire,
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
            if (osl_atomic_decrement( &pSeq->nRefCount ) == 0)
            {
                if (nElements > 0)
                {
                    idestructElements(
                        pSeq->elements, pElementType,
                        0, nElements, release );
                }
                std::free( pSeq );
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
            OSL_ASSERT( *ppSequence != nullptr );
            ret = (*ppSequence != nullptr);
        }
    }

    return ret;
}

}

extern "C"
{


sal_Bool SAL_CALL uno_type_sequence_construct(
    uno_Sequence ** ppSequence, typelib_TypeDescriptionReference * pType,
    void * pElements, sal_Int32 len,
    uno_AcquireFunc acquire )
    SAL_THROW_EXTERN_C()
{
    assert( len >= 0 );
    bool ret;
    if (len)
    {
        typelib_TypeDescription * pTypeDescr = nullptr;
        TYPELIB_DANGER_GET( &pTypeDescr, pType );

        typelib_TypeDescriptionReference * pElementType =
            reinterpret_cast<typelib_IndirectTypeDescription *>(pTypeDescr)->pType;

        *ppSequence = nullptr;
        if (pElements == nullptr)
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
                len, acquire,
                len ); // alloc to len
        }

        TYPELIB_DANGER_RELEASE( pTypeDescr );
    }
    else
    {
        *ppSequence = createEmptySequence();
        ret = true;
    }

    OSL_ASSERT( (*ppSequence != nullptr) == ret );
    return ret;
}


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
            reinterpret_cast<typelib_IndirectTypeDescription *>(pTypeDescr)->pType;

        *ppSequence = nullptr;
        if (pElements == nullptr)
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
                len, acquire,
                len ); // alloc to len
        }
    }
    else
    {
        *ppSequence = createEmptySequence();
        ret = true;
    }

    OSL_ASSERT( (*ppSequence != nullptr) == ret );
    return ret;
}


sal_Bool SAL_CALL uno_type_sequence_realloc(
    uno_Sequence ** ppSequence, typelib_TypeDescriptionReference * pType,
    sal_Int32 nSize, uno_AcquireFunc acquire, uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C()
{
    assert(ppSequence && "### null ptr!");
    assert(nSize >= 0 && "### new size must be at least 0!");

    bool ret = true;
    if (nSize != (*ppSequence)->nElements)
    {
        typelib_TypeDescription * pTypeDescr = nullptr;
        TYPELIB_DANGER_GET( &pTypeDescr, pType );
        ret = ireallocSequence(
            ppSequence, reinterpret_cast<typelib_IndirectTypeDescription *>(pTypeDescr)->pType,
            nSize, acquire, release );
        TYPELIB_DANGER_RELEASE( pTypeDescr );
    }
    return ret;
}


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
            ppSequence, reinterpret_cast<typelib_IndirectTypeDescription *>(pTypeDescr)->pType,
            nSize, acquire, release );
    }
    return ret;
}


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
        uno_Sequence * pNew = nullptr;
        if (pSequence->nElements > 0)
        {
            typelib_TypeDescription * pTypeDescr = nullptr;
            TYPELIB_DANGER_GET( &pTypeDescr, pType );

            ret = icopyConstructFromElements(
                &pNew, pSequence->elements,
                reinterpret_cast<typelib_IndirectTypeDescription *>(pTypeDescr)->pType,
                pSequence->nElements, acquire,
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
            ret = (pNew != nullptr);
            if (ret)
            {
                // easy destruction of empty sequence:
                if (osl_atomic_decrement( &pSequence->nRefCount ) == 0)
                    std::free( pSequence );
                *ppSequence = pNew;
            }
        }
    }
    return ret;
}


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
        uno_Sequence * pNew = nullptr;
        if (pSequence->nElements > 0)
        {
            ret = icopyConstructFromElements(
                &pNew, pSequence->elements,
                reinterpret_cast<typelib_IndirectTypeDescription *>(pTypeDescr)->pType,
                pSequence->nElements, acquire,
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
            ret = (pNew != nullptr);
            if (ret)
            {
                // easy destruction of empty sequence:
                if (osl_atomic_decrement( &pSequence->nRefCount ) == 0)
                    std::free( pSequence );
                *ppSequence = pNew;
            }
        }

    }
    return ret;
}


void SAL_CALL uno_sequence_assign(
    uno_Sequence ** ppDest,
    uno_Sequence * pSource,
    typelib_TypeDescription * pTypeDescr,
    uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C()
{
    if (*ppDest != pSource)
    {
        osl_atomic_increment( &pSource->nRefCount );
        idestructSequence( *ppDest, pTypeDescr->pWeakRef, pTypeDescr, release );
        *ppDest = pSource;
    }
}


void SAL_CALL uno_type_sequence_assign(
    uno_Sequence ** ppDest,
    uno_Sequence * pSource,
    typelib_TypeDescriptionReference * pType,
    uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C()
{
    if (*ppDest != pSource)
    {
        osl_atomic_increment( &pSource->nRefCount );
        idestructSequence( *ppDest, pType, nullptr, release );
        *ppDest = pSource;
    }
}

void uno_type_sequence_destroy(
    uno_Sequence * sequence, typelib_TypeDescriptionReference * type,
    uno_ReleaseFunc release)
    SAL_THROW_EXTERN_C()
{
    idestroySequence(sequence, type, nullptr, release);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
