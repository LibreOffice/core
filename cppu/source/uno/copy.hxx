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
#ifndef COPY_HXX
#define COPY_HXX

#include "prim.hxx"
#include "constr.hxx"


namespace cppu
{


//#### copy construction ###########################################################################



inline uno_Sequence * allocSeq(
    sal_Int32 nElementSize, sal_Int32 nElements )
{
    OSL_ASSERT( nElements >= 0 && nElementSize >= 0 );
    uno_Sequence * pSeq = 0;
    sal_uInt32 nSize = calcSeqMemSize( nElementSize, nElements );
    if (nSize > 0)
    {
        pSeq = (uno_Sequence *) rtl_allocateMemory( nSize );
        if (pSeq != 0)
        {
            // header init
            pSeq->nRefCount = 1;
            pSeq->nElements = nElements;
        }
    }
    return pSeq;
}


void copyConstructStruct(
    void * pDest, void * pSource,
    typelib_CompoundTypeDescription * pTypeDescr,
    uno_AcquireFunc acquire, uno_Mapping * mapping )
    SAL_THROW (());

inline void _copyConstructStruct(
    void * pDest, void * pSource,
    typelib_CompoundTypeDescription * pTypeDescr,
    uno_AcquireFunc acquire, uno_Mapping * mapping )
    SAL_THROW (())
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
                (char *)pDest + pMemberOffsets[nDescr],
                (char *)pSource + pMemberOffsets[nDescr],
                ppTypeRefs[nDescr], mapping );
        }
    }
    else
    {
        while (nDescr--)
        {
            ::uno_type_copyData(
                (char *)pDest + pMemberOffsets[nDescr],
                (char *)pSource + pMemberOffsets[nDescr],
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
    SAL_THROW (())
{
    TYPE_ACQUIRE( pType );
    pDestAny->pType = pType;

    switch (pType->eTypeClass)
    {
    case typelib_TypeClass_CHAR:
        pDestAny->pData = &pDestAny->pReserved;
        *(sal_Unicode *)pDestAny->pData = *(sal_Unicode *)pSource;
        break;
    case typelib_TypeClass_BOOLEAN:
        pDestAny->pData = &pDestAny->pReserved;
        *(sal_Bool *)pDestAny->pData = (*(sal_Bool *)pSource != sal_False);
        break;
    case typelib_TypeClass_BYTE:
        pDestAny->pData = &pDestAny->pReserved;
        *(sal_Int8 *)pDestAny->pData = *(sal_Int8 *)pSource;
        break;
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
        pDestAny->pData = &pDestAny->pReserved;
        *(sal_Int16 *)pDestAny->pData = *(sal_Int16 *)pSource;
        break;
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
        pDestAny->pData = &pDestAny->pReserved;
        *(sal_Int32 *)pDestAny->pData = *(sal_Int32 *)pSource;
        break;
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
        if (sizeof(void *) >= sizeof(sal_Int64))
            pDestAny->pData = &pDestAny->pReserved;
        else
            pDestAny->pData = ::rtl_allocateMemory( sizeof(sal_Int64) );
        *(sal_Int64 *)pDestAny->pData = *(sal_Int64 *)pSource;
        break;
    case typelib_TypeClass_FLOAT:
        if (sizeof(void *) >= sizeof(float))
            pDestAny->pData = &pDestAny->pReserved;
        else
            pDestAny->pData = ::rtl_allocateMemory( sizeof(float) );
        *(float *)pDestAny->pData = *(float *)pSource;
        break;
    case typelib_TypeClass_DOUBLE:
        if (sizeof(void *) >= sizeof(double))
            pDestAny->pData = &pDestAny->pReserved;
        else
            pDestAny->pData = ::rtl_allocateMemory( sizeof(double) );
        *(double *)pDestAny->pData = *(double *)pSource;
        break;
    case typelib_TypeClass_STRING:
        ::rtl_uString_acquire( *(rtl_uString **)pSource );
        pDestAny->pData = &pDestAny->pReserved;
        *(rtl_uString **)pDestAny->pData = *(rtl_uString **)pSource;
        break;
    case typelib_TypeClass_TYPE:
        TYPE_ACQUIRE( *(typelib_TypeDescriptionReference **)pSource );
        pDestAny->pData = &pDestAny->pReserved;
        *(typelib_TypeDescriptionReference **)pDestAny->pData = *(typelib_TypeDescriptionReference **)pSource;
        break;
    case typelib_TypeClass_ANY:
        OSL_FAIL( "### unexpected nested any!" );
        break;
    case typelib_TypeClass_ENUM:
        pDestAny->pData = &pDestAny->pReserved;
        // enum is forced to 32bit long
        *(sal_Int32 *)pDestAny->pData = *(sal_Int32 *)pSource;
        break;
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
        if (pTypeDescr)
        {
            pDestAny->pData = ::rtl_allocateMemory( pTypeDescr->nSize );
            _copyConstructStruct(
                pDestAny->pData, pSource,
                (typelib_CompoundTypeDescription *)pTypeDescr,
                acquire, mapping );
        }
        else
        {
            TYPELIB_DANGER_GET( &pTypeDescr, pType );
            pDestAny->pData = ::rtl_allocateMemory( pTypeDescr->nSize );
            _copyConstructStruct(
                pDestAny->pData, pSource,
                (typelib_CompoundTypeDescription *)pTypeDescr,
                acquire, mapping );
            TYPELIB_DANGER_RELEASE( pTypeDescr );
        }
        break;
    case typelib_TypeClass_SEQUENCE:
        pDestAny->pData = &pDestAny->pReserved;
        if (pTypeDescr)
        {
            *(uno_Sequence **)pDestAny->pData = copyConstructSequence(
                *(uno_Sequence **)pSource,
                ((typelib_IndirectTypeDescription *)pTypeDescr)->pType,
                acquire, mapping );
        }
        else
        {
            TYPELIB_DANGER_GET( &pTypeDescr, pType );
            *(uno_Sequence **)pDestAny->pData = copyConstructSequence(
                *(uno_Sequence **)pSource,
                ((typelib_IndirectTypeDescription *)pTypeDescr)->pType,
                acquire, mapping );
            TYPELIB_DANGER_RELEASE( pTypeDescr );
        }
        break;
    case typelib_TypeClass_INTERFACE:
        pDestAny->pData = &pDestAny->pReserved;
        if (mapping)
        {
            pDestAny->pReserved = _map( *(void **)pSource, pType, pTypeDescr, mapping );
        }
        else
        {
            _acquire( pDestAny->pReserved = *(void **)pSource, acquire );
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
    SAL_THROW (())
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
                pType = ((uno_Any *)pSource)->pType;
                if (typelib_TypeClass_VOID == pType->eTypeClass)
                {
                    CONSTRUCT_EMPTY_ANY( pDestAny );
                    return;
                }
                pTypeDescr = 0;
                pSource = ((uno_Any *)pSource)->pData;
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
                *(sal_Unicode *)pDestAny->pData = '\0';
                break;
            case typelib_TypeClass_BOOLEAN:
                pDestAny->pData = &pDestAny->pReserved;
                *(sal_Bool *)pDestAny->pData = sal_False;
                break;
            case typelib_TypeClass_BYTE:
                pDestAny->pData = &pDestAny->pReserved;
                *(sal_Int8 *)pDestAny->pData = 0;
                break;
            case typelib_TypeClass_SHORT:
            case typelib_TypeClass_UNSIGNED_SHORT:
                pDestAny->pData = &pDestAny->pReserved;
                *(sal_Int16 *)pDestAny->pData = 0;
                break;
            case typelib_TypeClass_LONG:
            case typelib_TypeClass_UNSIGNED_LONG:
                pDestAny->pData = &pDestAny->pReserved;
                *(sal_Int32 *)pDestAny->pData = 0;
                break;
            case typelib_TypeClass_HYPER:
            case typelib_TypeClass_UNSIGNED_HYPER:
                if (sizeof(void *) >= sizeof(sal_Int64))
                    pDestAny->pData = &pDestAny->pReserved;
                else
                    pDestAny->pData = ::rtl_allocateMemory( sizeof(sal_Int64) );
                *(sal_Int64 *)pDestAny->pData = 0;
                break;
            case typelib_TypeClass_FLOAT:
                if (sizeof(void *) >= sizeof(float))
                    pDestAny->pData = &pDestAny->pReserved;
                else
                    pDestAny->pData = ::rtl_allocateMemory( sizeof(float) );
                *(float *)pDestAny->pData = 0.0;
                break;
            case typelib_TypeClass_DOUBLE:
                if (sizeof(void *) >= sizeof(double))
                    pDestAny->pData = &pDestAny->pReserved;
                else
                    pDestAny->pData = ::rtl_allocateMemory( sizeof(double) );
                *(double *)pDestAny->pData = 0.0;
                break;
            case typelib_TypeClass_STRING:
                pDestAny->pData = &pDestAny->pReserved;
                *(rtl_uString **)pDestAny->pData = 0;
                ::rtl_uString_new( (rtl_uString **)pDestAny->pData );
                break;
            case typelib_TypeClass_TYPE:
                pDestAny->pData = &pDestAny->pReserved;
                *(typelib_TypeDescriptionReference **)pDestAny->pData = _getVoidType();
                break;
            case typelib_TypeClass_ENUM:
                pDestAny->pData = &pDestAny->pReserved;
                if (pTypeDescr)
                {
                    *(sal_Int32 *)pDestAny->pData = ((typelib_EnumTypeDescription *)pTypeDescr)->nDefaultEnumValue;
                }
                else
                {
                    TYPELIB_DANGER_GET( &pTypeDescr, pType );
                    *(sal_Int32 *)pDestAny->pData = ((typelib_EnumTypeDescription *)pTypeDescr)->nDefaultEnumValue;
                    TYPELIB_DANGER_RELEASE( pTypeDescr );
                }
                break;
            case typelib_TypeClass_STRUCT:
            case typelib_TypeClass_EXCEPTION:
                if (pTypeDescr)
                {
                    pDestAny->pData = ::rtl_allocateMemory( pTypeDescr->nSize );
                    _defaultConstructStruct(
                        pDestAny->pData, (typelib_CompoundTypeDescription *)pTypeDescr );
                }
                else
                {
                    TYPELIB_DANGER_GET( &pTypeDescr, pType );
                    pDestAny->pData = ::rtl_allocateMemory( pTypeDescr->nSize );
                    _defaultConstructStruct(
                        pDestAny->pData, (typelib_CompoundTypeDescription *)pTypeDescr );
                    TYPELIB_DANGER_RELEASE( pTypeDescr );
                }
                break;
            case typelib_TypeClass_SEQUENCE:
                pDestAny->pData = &pDestAny->pReserved;
                *(uno_Sequence **)pDestAny->pData = createEmptySequence();
                break;
            case typelib_TypeClass_INTERFACE:
                pDestAny->pData = &pDestAny->pReserved;
                pDestAny->pReserved = 0; // either cpp or c-uno interface
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
                if (pDest != 0)
                {
                    uno_Any * pDestElements = (uno_Any *)pDest->elements;
                    uno_Any * pSourceElements = (uno_Any *)pSource->elements;
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
                                pType, 0,
                                acquire, mapping );
                        }
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
                char * pSourceElements = pSource->elements;
                pDest = allocSeq( nElementSize, nElements );
                if (pDest != 0)
                {
                    char * pElements = pDest->elements;
                    for ( sal_Int32 nPos = nElements; nPos--; )
                    {
                        _copyConstructStruct(
                            pElements + (nPos * nElementSize),
                            pSourceElements + (nPos * nElementSize),
                            (typelib_CompoundTypeDescription *)
                            pElementTypeDescr,
                            acquire, mapping );
                    }
                }
                TYPELIB_DANGER_RELEASE( pElementTypeDescr );
                break;
            }
            case typelib_TypeClass_SEQUENCE: // sequence of sequence
            {
                pDest = allocSeq( sizeof (uno_Sequence *), nElements );
                if (pDest != 0)
                {
                    typelib_TypeDescription * pElementTypeDescr = 0;
                    TYPELIB_DANGER_GET( &pElementTypeDescr, pElementType );
                    typelib_TypeDescriptionReference * pSeqElementType =
                        ((typelib_IndirectTypeDescription *)
                         pElementTypeDescr)->pType;

                    uno_Sequence ** pDestElements =
                        (uno_Sequence **) pDest->elements;
                    uno_Sequence ** pSourceElements =
                        (uno_Sequence **) pSource->elements;
                    for ( sal_Int32 nPos = nElements; nPos--; )
                    {
                        uno_Sequence * pNew = copyConstructSequence(
                            pSourceElements[nPos],
                            pSeqElementType,
                            acquire, mapping );
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
                pDest = allocSeq( sizeof (void *), nElements );
                if (pDest != 0)
                {
                    char * pElements = pDest->elements;
                    void ** pSourceElements = (void **)pSource->elements;
                    typelib_TypeDescription * pElementTypeDescr = 0;
                    TYPELIB_DANGER_GET( &pElementTypeDescr, pElementType );
                    for ( sal_Int32 nPos = nElements; nPos--; )
                    {
                        ((void **)pElements)[nPos] = 0;
                        if (((void **)pSourceElements)[nPos])
                        {
                            (*mapping->mapInterface)(
                                mapping, (void **)pElements + nPos,
                                pSourceElements[nPos],
                                (typelib_InterfaceTypeDescription *)
                                pElementTypeDescr );
                        }
                    }
                    TYPELIB_DANGER_RELEASE( pElementTypeDescr );
                }
                break;
            }
            default:
                OSL_FAIL( "### unexepcted sequence element type!" );
                pDest = 0;
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
    SAL_THROW (())
{
    switch (pType->eTypeClass)
    {
    case typelib_TypeClass_CHAR:
        *(sal_Unicode *)pDest = *(sal_Unicode *)pSource;
        break;
    case typelib_TypeClass_BOOLEAN:
        *(sal_Bool *)pDest = (*(sal_Bool *)pSource != sal_False);
        break;
    case typelib_TypeClass_BYTE:
        *(sal_Int8 *)pDest = *(sal_Int8 *)pSource;
        break;
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
        *(sal_Int16 *)pDest = *(sal_Int16 *)pSource;
        break;
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
        *(sal_Int32 *)pDest = *(sal_Int32 *)pSource;
        break;
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
        *(sal_Int64 *)pDest = *(sal_Int64 *)pSource;
        break;
    case typelib_TypeClass_FLOAT:
        *(float *)pDest = *(float *)pSource;
        break;
    case typelib_TypeClass_DOUBLE:
        *(double *)pDest = *(double *)pSource;
        break;
    case typelib_TypeClass_STRING:
        ::rtl_uString_acquire( *(rtl_uString **)pSource );
        *(rtl_uString **)pDest = *(rtl_uString **)pSource;
        break;
    case typelib_TypeClass_TYPE:
        TYPE_ACQUIRE( *(typelib_TypeDescriptionReference **)pSource );
        *(typelib_TypeDescriptionReference **)pDest = *(typelib_TypeDescriptionReference **)pSource;
        break;
    case typelib_TypeClass_ANY:
        _copyConstructAny(
            (uno_Any *)pDest, ((uno_Any *)pSource)->pData,
            ((uno_Any *)pSource)->pType, 0,
            acquire, mapping );
        break;
    case typelib_TypeClass_ENUM:
        *(sal_Int32 *)pDest = *(sal_Int32 *)pSource;
        break;
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
        if (pTypeDescr)
        {
            _copyConstructStruct(
                pDest, pSource,
                (typelib_CompoundTypeDescription *)pTypeDescr,
                acquire, mapping );
        }
        else
        {
            TYPELIB_DANGER_GET( &pTypeDescr, pType );
            _copyConstructStruct(
                pDest, pSource,
                (typelib_CompoundTypeDescription *)pTypeDescr,
                acquire, mapping );
            TYPELIB_DANGER_RELEASE( pTypeDescr );
        }
        break;
    case typelib_TypeClass_SEQUENCE:
        if (mapping)
        {
            if (pTypeDescr)
            {
                *(uno_Sequence **)pDest = icopyConstructSequence(
                    *(uno_Sequence **)pSource,
                    ((typelib_IndirectTypeDescription *)pTypeDescr)->pType,
                    acquire, mapping );
            }
            else
            {
                TYPELIB_DANGER_GET( &pTypeDescr, pType );
                *(uno_Sequence **)pDest = icopyConstructSequence(
                    *(uno_Sequence **)pSource,
                    ((typelib_IndirectTypeDescription *)pTypeDescr)->pType,
                    acquire, mapping );
                TYPELIB_DANGER_RELEASE( pTypeDescr );
            }
        }
        else
        {
            osl_atomic_increment( &(*(uno_Sequence **)pSource)->nRefCount );
            *(uno_Sequence **)pDest = *(uno_Sequence **)pSource;
        }
        break;
    case typelib_TypeClass_INTERFACE:
        if (mapping)
            *(void **)pDest = _map( *(void **)pSource, pType, pTypeDescr, mapping );
        else
            _acquire( *(void **)pDest = *(void **)pSource, acquire );
        break;
    default:
        break;
    }
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
