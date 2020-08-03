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
#pragma once

#include "prim.hxx"
#include "constr.hxx"
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <type_traits>

namespace cppu
{


//#### copy construction ###########################################################################

namespace {

// The non-dynamic prefix of sal_Sequence (aka uno_Sequence):
struct SequencePrefix {
    sal_Int32 nRefCount;
    sal_Int32 nElements;
};
static_assert(sizeof (SequencePrefix) < sizeof (uno_Sequence));
static_assert(offsetof(SequencePrefix, nRefCount) == offsetof(uno_Sequence, nRefCount));
static_assert(
    std::is_same_v<decltype(SequencePrefix::nRefCount), decltype(uno_Sequence::nRefCount)>);
static_assert(offsetof(SequencePrefix, nElements) == offsetof(uno_Sequence, nElements));
static_assert(
    std::is_same_v<decltype(SequencePrefix::nElements), decltype(uno_Sequence::nElements)>);

}

inline uno_Sequence * allocSeq(
    sal_Int32 nElementSize, sal_Int32 nElements )
{
    OSL_ASSERT( nElements >= 0 && nElementSize >= 0 );
    uno_Sequence * pSeq = nullptr;
    sal_uInt32 nSize = calcSeqMemSize( nElementSize, nElements );
    if (nSize > 0)
    {
        pSeq = static_cast<uno_Sequence *>(std::malloc( nSize ));
        if (pSeq != nullptr)
        {
            // header init, going via SequencePrefix to avoid UBSan insufficient-object-size
            // warnings when `nElements == 0` and thus `nSize < sizeof (uno_Sequence)`:
            auto const header = reinterpret_cast<SequencePrefix *>(pSeq);
            header->nRefCount = 1;
            header->nElements = nElements;
        }
    }
    return pSeq;
}


void copyConstructStruct(
    void * pDest, void * pSource,
    typelib_CompoundTypeDescription * pTypeDescr,
    uno_AcquireFunc acquire, uno_Mapping * mapping );

inline void _copyConstructStruct(
    void * pDest, void * pSource,
    typelib_CompoundTypeDescription * pTypeDescr,
    uno_AcquireFunc acquire, uno_Mapping * mapping )
{
    if (pTypeDescr->pBaseTypeDescription)
    {
        // copy base value
        copyConstructStruct( pDest, pSource, pTypeDescr->pBaseTypeDescription, acquire, mapping );
    }

    // then copy members
    typelib_TypeDescriptionReference ** ppTypeRefs = pTypeDescr->ppTypeRefs;
    sal_Int32 * pMemberOffsets = pTypeDescr->pMemberOffsets;
    sal_Int32 nDescr = pTypeDescr->nMembers;

    if (mapping)
    {
        while (nDescr--)
        {
            ::uno_type_copyAndConvertData(
                static_cast<char *>(pDest) + pMemberOffsets[nDescr],
                static_cast<char *>(pSource) + pMemberOffsets[nDescr],
                ppTypeRefs[nDescr], mapping );
        }
    }
    else
    {
        while (nDescr--)
        {
            ::uno_type_copyData(
                static_cast<char *>(pDest) + pMemberOffsets[nDescr],
                static_cast<char *>(pSource) + pMemberOffsets[nDescr],
                ppTypeRefs[nDescr], acquire );
        }
    }
}


uno_Sequence * copyConstructSequence(
    uno_Sequence * pSource,
    typelib_TypeDescriptionReference * pElementType,
    uno_AcquireFunc acquire, uno_Mapping * mapping );


inline void _copyConstructAnyFromData(
    uno_Any * pDestAny, void * pSource,
    typelib_TypeDescriptionReference * pType, typelib_TypeDescription * pTypeDescr,
    uno_AcquireFunc acquire, uno_Mapping * mapping )
{
    TYPE_ACQUIRE( pType );
    pDestAny->pType = pType;

    switch (pType->eTypeClass)
    {
    case typelib_TypeClass_CHAR:
        pDestAny->pData = &pDestAny->pReserved;
        *static_cast<sal_Unicode *>(pDestAny->pData) = *static_cast<sal_Unicode *>(pSource);
        break;
    case typelib_TypeClass_BOOLEAN:
        pDestAny->pData = &pDestAny->pReserved;
        *static_cast<sal_Bool *>(pDestAny->pData) = bool(*static_cast<sal_Bool *>(pSource));
        break;
    case typelib_TypeClass_BYTE:
        pDestAny->pData = &pDestAny->pReserved;
        *static_cast<sal_Int8 *>(pDestAny->pData) = *static_cast<sal_Int8 *>(pSource);
        break;
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
        pDestAny->pData = &pDestAny->pReserved;
        *static_cast<sal_Int16 *>(pDestAny->pData) = *static_cast<sal_Int16 *>(pSource);
        break;
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
        pDestAny->pData = &pDestAny->pReserved;
        *static_cast<sal_Int32 *>(pDestAny->pData) = *static_cast<sal_Int32 *>(pSource);
        break;
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
        if (sizeof(void *) >= sizeof(sal_Int64))
            pDestAny->pData = &pDestAny->pReserved;
        else
            pDestAny->pData = std::malloc( sizeof(sal_Int64) );
        assert(pDestAny->pData);
        *static_cast<sal_Int64 *>(pDestAny->pData) = *static_cast<sal_Int64 *>(pSource);
        break;
    case typelib_TypeClass_FLOAT:
        if (sizeof(void *) >= sizeof(float))
            pDestAny->pData = &pDestAny->pReserved;
        else
            pDestAny->pData = std::malloc( sizeof(float) );
        assert(pDestAny->pData);
        *static_cast<float *>(pDestAny->pData) = *static_cast<float *>(pSource);
        break;
    case typelib_TypeClass_DOUBLE:
        if (sizeof(void *) >= sizeof(double))
            pDestAny->pData = &pDestAny->pReserved;
        else
            pDestAny->pData = std::malloc( sizeof(double) );
        assert(pDestAny->pData);
        *static_cast<double *>(pDestAny->pData) = *static_cast<double *>(pSource);
        break;
    case typelib_TypeClass_STRING:
        ::rtl_uString_acquire( *static_cast<rtl_uString **>(pSource) );
        pDestAny->pData = &pDestAny->pReserved;
        *static_cast<rtl_uString **>(pDestAny->pData) = *static_cast<rtl_uString **>(pSource);
        break;
    case typelib_TypeClass_TYPE:
        TYPE_ACQUIRE( *static_cast<typelib_TypeDescriptionReference **>(pSource) );
        pDestAny->pData = &pDestAny->pReserved;
        *static_cast<typelib_TypeDescriptionReference **>(pDestAny->pData) = *static_cast<typelib_TypeDescriptionReference **>(pSource);
        break;
    case typelib_TypeClass_ANY:
        OSL_FAIL( "### unexpected nested any!" );
        break;
    case typelib_TypeClass_ENUM:
        pDestAny->pData = &pDestAny->pReserved;
        // enum is forced to 32bit long
        *static_cast<sal_Int32 *>(pDestAny->pData) = *static_cast<sal_Int32 *>(pSource);
        break;
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
        if (pTypeDescr)
        {
            pDestAny->pData = std::malloc( pTypeDescr->nSize );
            _copyConstructStruct(
                pDestAny->pData, pSource,
                reinterpret_cast<typelib_CompoundTypeDescription *>(pTypeDescr),
                acquire, mapping );
        }
        else
        {
            TYPELIB_DANGER_GET( &pTypeDescr, pType );
            pDestAny->pData = std::malloc( pTypeDescr->nSize );
            _copyConstructStruct(
                pDestAny->pData, pSource,
                reinterpret_cast<typelib_CompoundTypeDescription *>(pTypeDescr),
                acquire, mapping );
            TYPELIB_DANGER_RELEASE( pTypeDescr );
        }
        break;
    case typelib_TypeClass_SEQUENCE:
        pDestAny->pData = &pDestAny->pReserved;
        if (pTypeDescr)
        {
            *static_cast<uno_Sequence **>(pDestAny->pData) = copyConstructSequence(
                *static_cast<uno_Sequence **>(pSource),
                reinterpret_cast<typelib_IndirectTypeDescription *>(pTypeDescr)->pType,
                acquire, mapping );
        }
        else
        {
            TYPELIB_DANGER_GET( &pTypeDescr, pType );
            *static_cast<uno_Sequence **>(pDestAny->pData) = copyConstructSequence(
                *static_cast<uno_Sequence **>(pSource),
                reinterpret_cast<typelib_IndirectTypeDescription *>(pTypeDescr)->pType,
                acquire, mapping );
            TYPELIB_DANGER_RELEASE( pTypeDescr );
        }
        break;
    case typelib_TypeClass_INTERFACE:
        pDestAny->pData = &pDestAny->pReserved;
        if (mapping)
        {
            pDestAny->pReserved = _map( *static_cast<void **>(pSource), pType, pTypeDescr, mapping );
        }
        else
        {
            pDestAny->pReserved = *static_cast<void **>(pSource);
            _acquire( pDestAny->pReserved, acquire );
        }
        break;
    default:
        OSL_ASSERT(false);
        break;
    }
}

inline void _copyConstructAny(
    uno_Any * pDestAny, void * pSource,
    typelib_TypeDescriptionReference * pType, typelib_TypeDescription * pTypeDescr,
    uno_AcquireFunc acquire, uno_Mapping * mapping )
{
    if (typelib_TypeClass_VOID == pType->eTypeClass)
    {
        CONSTRUCT_EMPTY_ANY( pDestAny );
    }
    else
    {
        if (typelib_TypeClass_ANY == pType->eTypeClass)
        {
            if (pSource)
            {
                pType = static_cast<uno_Any *>(pSource)->pType;
                if (typelib_TypeClass_VOID == pType->eTypeClass)
                {
                    CONSTRUCT_EMPTY_ANY( pDestAny );
                    return;
                }
                pTypeDescr = nullptr;
                pSource = static_cast<uno_Any *>(pSource)->pData;
            }
            else
            {
                CONSTRUCT_EMPTY_ANY( pDestAny );
                return;
            }
        }
        if (pSource)
        {
            _copyConstructAnyFromData( pDestAny, pSource, pType, pTypeDescr, acquire, mapping );
        }
        else // default construct
        {
            TYPE_ACQUIRE( pType );
            pDestAny->pType = pType;
            switch (pType->eTypeClass)
            {
            case typelib_TypeClass_CHAR:
                pDestAny->pData = &pDestAny->pReserved;
                *static_cast<sal_Unicode *>(pDestAny->pData) = '\0';
                break;
            case typelib_TypeClass_BOOLEAN:
                pDestAny->pData = &pDestAny->pReserved;
                *static_cast<sal_Bool *>(pDestAny->pData) = false;
                break;
            case typelib_TypeClass_BYTE:
                pDestAny->pData = &pDestAny->pReserved;
                *static_cast<sal_Int8 *>(pDestAny->pData) = 0;
                break;
            case typelib_TypeClass_SHORT:
            case typelib_TypeClass_UNSIGNED_SHORT:
                pDestAny->pData = &pDestAny->pReserved;
                *static_cast<sal_Int16 *>(pDestAny->pData) = 0;
                break;
            case typelib_TypeClass_LONG:
            case typelib_TypeClass_UNSIGNED_LONG:
                pDestAny->pData = &pDestAny->pReserved;
                *static_cast<sal_Int32 *>(pDestAny->pData) = 0;
                break;
            case typelib_TypeClass_HYPER:
            case typelib_TypeClass_UNSIGNED_HYPER:
                if (sizeof(void *) >= sizeof(sal_Int64))
                    pDestAny->pData = &pDestAny->pReserved;
                else
                    pDestAny->pData = std::malloc( sizeof(sal_Int64) );
                assert(pDestAny->pData);
                *static_cast<sal_Int64 *>(pDestAny->pData) = 0;
                break;
            case typelib_TypeClass_FLOAT:
                if (sizeof(void *) >= sizeof(float))
                    pDestAny->pData = &pDestAny->pReserved;
                else
                    pDestAny->pData = std::malloc( sizeof(float) );
                assert(pDestAny->pData);
                *static_cast<float *>(pDestAny->pData) = 0.0;
                break;
            case typelib_TypeClass_DOUBLE:
                if (sizeof(void *) >= sizeof(double))
                    pDestAny->pData = &pDestAny->pReserved;
                else
                    pDestAny->pData = std::malloc( sizeof(double) );
                assert(pDestAny->pData);
                *static_cast<double *>(pDestAny->pData) = 0.0;
                break;
            case typelib_TypeClass_STRING:
                pDestAny->pData = &pDestAny->pReserved;
                *static_cast<rtl_uString **>(pDestAny->pData) = nullptr;
                ::rtl_uString_new( static_cast<rtl_uString **>(pDestAny->pData) );
                break;
            case typelib_TypeClass_TYPE:
                pDestAny->pData = &pDestAny->pReserved;
                *static_cast<typelib_TypeDescriptionReference **>(pDestAny->pData) = _getVoidType();
                break;
            case typelib_TypeClass_ENUM:
                pDestAny->pData = &pDestAny->pReserved;
                if (pTypeDescr)
                {
                    *static_cast<sal_Int32 *>(pDestAny->pData) = reinterpret_cast<typelib_EnumTypeDescription *>(pTypeDescr)->nDefaultEnumValue;
                }
                else
                {
                    TYPELIB_DANGER_GET( &pTypeDescr, pType );
                    *static_cast<sal_Int32 *>(pDestAny->pData) = reinterpret_cast<typelib_EnumTypeDescription *>(pTypeDescr)->nDefaultEnumValue;
                    TYPELIB_DANGER_RELEASE( pTypeDescr );
                }
                break;
            case typelib_TypeClass_STRUCT:
            case typelib_TypeClass_EXCEPTION:
                if (pTypeDescr)
                {
                    pDestAny->pData = std::malloc( pTypeDescr->nSize );
                    _defaultConstructStruct(
                        pDestAny->pData, reinterpret_cast<typelib_CompoundTypeDescription *>(pTypeDescr) );
                }
                else
                {
                    TYPELIB_DANGER_GET( &pTypeDescr, pType );
                    pDestAny->pData = std::malloc( pTypeDescr->nSize );
                    _defaultConstructStruct(
                        pDestAny->pData, reinterpret_cast<typelib_CompoundTypeDescription *>(pTypeDescr) );
                    TYPELIB_DANGER_RELEASE( pTypeDescr );
                }
                break;
            case typelib_TypeClass_SEQUENCE:
                pDestAny->pData = &pDestAny->pReserved;
                *static_cast<uno_Sequence **>(pDestAny->pData) = createEmptySequence();
                break;
            case typelib_TypeClass_INTERFACE:
                pDestAny->pData = &pDestAny->pReserved;
                pDestAny->pReserved = nullptr; // either cpp or c-uno interface
                break;
            default:
                OSL_ASSERT(false);
                break;
            }
        }
    }
}

inline uno_Sequence * icopyConstructSequence(
    uno_Sequence * pSource,
    typelib_TypeDescriptionReference * pElementType,
    uno_AcquireFunc acquire, uno_Mapping * mapping )
{
    typelib_TypeClass eTypeClass = pElementType->eTypeClass;
    if (!mapping ||
        (eTypeClass <= typelib_TypeClass_ENUM &&
         eTypeClass != typelib_TypeClass_ANY))
    {
        osl_atomic_increment( &pSource->nRefCount );
        return pSource;
    }
    else // create new sequence
    {
        uno_Sequence * pDest;
        sal_Int32 nElements = pSource->nElements;
        if (nElements)
        {
            switch (eTypeClass)
            {
            case typelib_TypeClass_ANY:
            {
                pDest = allocSeq( sizeof (uno_Any), nElements );
                if (pDest != nullptr)
                {
                    uno_Any * pDestElements = reinterpret_cast<uno_Any *>(pDest->elements);
                    uno_Any * pSourceElements = reinterpret_cast<uno_Any *>(pSource->elements);
                    for ( sal_Int32 nPos = nElements; nPos--; )
                    {
                        typelib_TypeDescriptionReference * pType =
                            pSourceElements[nPos].pType;
                        if (typelib_TypeClass_VOID == pType->eTypeClass)
                        {
                            CONSTRUCT_EMPTY_ANY( &pDestElements[nPos] );
                        }
                        else
                        {
                            _copyConstructAnyFromData(
                                &pDestElements[nPos],
                                pSourceElements[nPos].pData,
                                pType, nullptr,
                                acquire, mapping );
                        }
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
                char * pSourceElements = pSource->elements;
                pDest = allocSeq( nElementSize, nElements );
                if (pDest != nullptr)
                {
                    char * pElements = pDest->elements;
                    for ( sal_Int32 nPos = nElements; nPos--; )
                    {
                        _copyConstructStruct(
                            pElements + (nPos * nElementSize),
                            pSourceElements + (nPos * nElementSize),
                            reinterpret_cast<typelib_CompoundTypeDescription *>(
                                pElementTypeDescr),
                            acquire, mapping );
                    }
                }
                TYPELIB_DANGER_RELEASE( pElementTypeDescr );
                break;
            }
            case typelib_TypeClass_SEQUENCE: // sequence of sequence
            {
                // coverity[suspicious_sizeof] - sizeof(uno_Sequence*) is correct here
                pDest = allocSeq( sizeof (uno_Sequence *), nElements );
                if (pDest != nullptr)
                {
                    typelib_TypeDescription * pElementTypeDescr = nullptr;
                    TYPELIB_DANGER_GET( &pElementTypeDescr, pElementType );
                    typelib_TypeDescriptionReference * pSeqElementType =
                        reinterpret_cast<typelib_IndirectTypeDescription *>(
                            pElementTypeDescr)->pType;

                    uno_Sequence ** pDestElements =
                        reinterpret_cast<uno_Sequence **>(pDest->elements);
                    uno_Sequence ** pSourceElements =
                        reinterpret_cast<uno_Sequence **>(pSource->elements);
                    for ( sal_Int32 nPos = nElements; nPos--; )
                    {
                        uno_Sequence * pNew = copyConstructSequence(
                            pSourceElements[nPos],
                            pSeqElementType,
                            acquire, mapping );
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
                pDest = allocSeq( sizeof (void *), nElements );
                if (pDest != nullptr)
                {
                    char * pElements = pDest->elements;
                    void ** pSourceElements = reinterpret_cast<void **>(pSource->elements);
                    typelib_TypeDescription * pElementTypeDescr = nullptr;
                    TYPELIB_DANGER_GET( &pElementTypeDescr, pElementType );
                    for ( sal_Int32 nPos = nElements; nPos--; )
                    {
                        reinterpret_cast<void **>(pElements)[nPos] = nullptr;
                        if (pSourceElements[nPos])
                        {
                            (*mapping->mapInterface)(
                                mapping, reinterpret_cast<void **>(pElements) + nPos,
                                pSourceElements[nPos],
                                reinterpret_cast<typelib_InterfaceTypeDescription *>(
                                    pElementTypeDescr) );
                        }
                    }
                    TYPELIB_DANGER_RELEASE( pElementTypeDescr );
                }
                break;
            }
            default:
                OSL_FAIL( "### unexpected sequence element type!" );
                pDest = nullptr;
                break;
            }
        }
        else // empty sequence
        {
            pDest = allocSeq( 0, 0 );
        }

        return pDest;
    }
}


inline void _copyConstructData(
    void * pDest, void * pSource,
    typelib_TypeDescriptionReference * pType, typelib_TypeDescription * pTypeDescr,
    uno_AcquireFunc acquire, uno_Mapping * mapping )
{
    switch (pType->eTypeClass)
    {
    case typelib_TypeClass_CHAR:
        *static_cast<sal_Unicode *>(pDest) = *static_cast<sal_Unicode *>(pSource);
        break;
    case typelib_TypeClass_BOOLEAN:
        *static_cast<sal_Bool *>(pDest) = bool(*static_cast<sal_Bool *>(pSource));
        break;
    case typelib_TypeClass_BYTE:
        *static_cast<sal_Int8 *>(pDest) = *static_cast<sal_Int8 *>(pSource);
        break;
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
        *static_cast<sal_Int16 *>(pDest) = *static_cast<sal_Int16 *>(pSource);
        break;
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
        *static_cast<sal_Int32 *>(pDest) = *static_cast<sal_Int32 *>(pSource);
        break;
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
        *static_cast<sal_Int64 *>(pDest) = *static_cast<sal_Int64 *>(pSource);
        break;
    case typelib_TypeClass_FLOAT:
        *static_cast<float *>(pDest) = *static_cast<float *>(pSource);
        break;
    case typelib_TypeClass_DOUBLE:
        *static_cast<double *>(pDest) = *static_cast<double *>(pSource);
        break;
    case typelib_TypeClass_STRING:
        ::rtl_uString_acquire( *static_cast<rtl_uString **>(pSource) );
        *static_cast<rtl_uString **>(pDest) = *static_cast<rtl_uString **>(pSource);
        break;
    case typelib_TypeClass_TYPE:
        TYPE_ACQUIRE( *static_cast<typelib_TypeDescriptionReference **>(pSource) );
        *static_cast<typelib_TypeDescriptionReference **>(pDest) = *static_cast<typelib_TypeDescriptionReference **>(pSource);
        break;
    case typelib_TypeClass_ANY:
        _copyConstructAny(
            static_cast<uno_Any *>(pDest), static_cast<uno_Any *>(pSource)->pData,
            static_cast<uno_Any *>(pSource)->pType, nullptr,
            acquire, mapping );
        break;
    case typelib_TypeClass_ENUM:
        *static_cast<sal_Int32 *>(pDest) = *static_cast<sal_Int32 *>(pSource);
        break;
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
        if (pTypeDescr)
        {
            _copyConstructStruct(
                pDest, pSource,
                reinterpret_cast<typelib_CompoundTypeDescription *>(pTypeDescr),
                acquire, mapping );
        }
        else
        {
            TYPELIB_DANGER_GET( &pTypeDescr, pType );
            _copyConstructStruct(
                pDest, pSource,
                reinterpret_cast<typelib_CompoundTypeDescription *>(pTypeDescr),
                acquire, mapping );
            TYPELIB_DANGER_RELEASE( pTypeDescr );
        }
        break;
    case typelib_TypeClass_SEQUENCE:
        if (mapping)
        {
            if (pTypeDescr)
            {
                *static_cast<uno_Sequence **>(pDest) = icopyConstructSequence(
                    *static_cast<uno_Sequence **>(pSource),
                    reinterpret_cast<typelib_IndirectTypeDescription *>(pTypeDescr)->pType,
                    acquire, mapping );
            }
            else
            {
                TYPELIB_DANGER_GET( &pTypeDescr, pType );
                *static_cast<uno_Sequence **>(pDest) = icopyConstructSequence(
                    *static_cast<uno_Sequence **>(pSource),
                    reinterpret_cast<typelib_IndirectTypeDescription *>(pTypeDescr)->pType,
                    acquire, mapping );
                TYPELIB_DANGER_RELEASE( pTypeDescr );
            }
        }
        else
        {
            osl_atomic_increment( &(*static_cast<uno_Sequence **>(pSource))->nRefCount );
            *static_cast<uno_Sequence **>(pDest) = *static_cast<uno_Sequence **>(pSource);
        }
        break;
    case typelib_TypeClass_INTERFACE:
        if (mapping)
            *static_cast<void **>(pDest) = _map( *static_cast<void **>(pSource), pType, pTypeDescr, mapping );
        else
        {
            *static_cast<void **>(pDest) = *static_cast<void **>(pSource);
            _acquire( *static_cast<void **>(pDest), acquire );
        }
        break;
    default:
        break;
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
