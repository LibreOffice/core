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
#ifndef INCLUDED_CPPU_SOURCE_UNO_EQ_HXX
#define INCLUDED_CPPU_SOURCE_UNO_EQ_HXX

#include <cmath>
#include <string.h>

#include <osl/diagnose.h>
#include <rtl/ustring.hxx>

#include "prim.hxx"


namespace cppu
{


//#### equality ####################################################################################


inline bool _equalObject(
    void * pI1, void * pI2,
    uno_QueryInterfaceFunc queryInterface, uno_ReleaseFunc release )
{
    if (pI1 == pI2)
        return true;
    if ((nullptr == pI1) || (nullptr == pI2))
        return false;
    bool bRet = false;

    typelib_TypeDescriptionReference * type_XInterface =
        * typelib_static_type_getByTypeClass( typelib_TypeClass_INTERFACE );
    if (nullptr == queryInterface)
        queryInterface = binuno_queryInterface;
    pI1 = (*queryInterface)( pI1, type_XInterface );
    if (nullptr != pI1)
    {
        pI2 = (*queryInterface)( pI2, type_XInterface );
        if (nullptr != pI2)
        {
            bRet = (pI1 == pI2);
            _release( pI2, release );
        }
        _release( pI1, release );
    }
    return bRet;
}


bool equalStruct(
    void * pDest, void *pSource,
    typelib_CompoundTypeDescription * pTypeDescr,
    uno_QueryInterfaceFunc queryInterface, uno_ReleaseFunc release );

inline bool _equalStruct(
    void * pDest, void *pSource,
    typelib_CompoundTypeDescription * pTypeDescr,
    uno_QueryInterfaceFunc queryInterface, uno_ReleaseFunc release )
{
    if (pTypeDescr->pBaseTypeDescription &&
        !equalStruct( pDest, pSource, pTypeDescr->pBaseTypeDescription, queryInterface, release ))
    {
        return false;
    }

    typelib_TypeDescriptionReference ** ppTypeRefs = pTypeDescr->ppTypeRefs;
    sal_Int32 * pMemberOffsets = pTypeDescr->pMemberOffsets;
    sal_Int32 nDescr = pTypeDescr->nMembers;

    while (nDescr--)
    {
        sal_Int32 nOffset = pMemberOffsets[nDescr];
        if (! ::uno_type_equalData( static_cast<char *>(pDest) + nOffset,
                                    ppTypeRefs[nDescr],
                                    static_cast<char *>(pSource) + nOffset,
                                    ppTypeRefs[nDescr],
                                    queryInterface, release ))
        {
            return false;
        }
    }
    return true;
}

bool equalSequence(
    uno_Sequence * pDest, uno_Sequence * pSource,
    typelib_TypeDescriptionReference * pElementType,
    uno_QueryInterfaceFunc queryInterface, uno_ReleaseFunc release );

inline bool _equalSequence(
    uno_Sequence * pDest, uno_Sequence * pSource,
    typelib_TypeDescriptionReference * pElementType,
    uno_QueryInterfaceFunc queryInterface, uno_ReleaseFunc release )
{
    if (pDest == pSource)
        return true;
    sal_Int32 nElements = pDest->nElements;
    if (nElements != pSource->nElements)
        return false;
    if (! nElements)
        return true;

    void * pDestElements = pDest->elements;
    void * pSourceElements = pSource->elements;

    switch (pElementType->eTypeClass)
    {
    case typelib_TypeClass_CHAR:
        return (0 == memcmp( pDestElements, pSourceElements, sizeof(sal_Unicode) * nElements ));
    case typelib_TypeClass_BOOLEAN:
    {
        for ( sal_Int32 nPos = nElements; nPos--; )
        {
            if (bool(static_cast<sal_Bool *>(pDestElements)[nPos]) !=
                bool(static_cast<sal_Bool *>(pSourceElements)[nPos]))
            {
                return false;
            }
        }
        return true;
    }
    case typelib_TypeClass_BYTE:
        return (0 == memcmp( pDestElements, pSourceElements, sizeof(sal_Int8) * nElements ));
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
        return (0 == memcmp( pDestElements, pSourceElements, sizeof(sal_Int16) * nElements ));
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
        return (0 == memcmp( pDestElements, pSourceElements, sizeof(sal_Int32) * nElements ));
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
        return (0 == memcmp( pDestElements, pSourceElements, sizeof(sal_Int64) * nElements ));
    case typelib_TypeClass_FLOAT:
    {
        for ( sal_Int32 nPos = nElements; nPos--; )
        {
            if (static_cast<float *>(pDestElements)[nPos] != static_cast<float *>(pSourceElements)[nPos])
                return false;
        }
        return true;
    }
    case typelib_TypeClass_DOUBLE:
    {
        for ( sal_Int32 nPos = nElements; nPos--; )
        {
            if (static_cast<double *>(pDestElements)[nPos] != static_cast<double *>(pSourceElements)[nPos])
                return false;
        }
        return true;
    }
    case typelib_TypeClass_STRING:
    {
        for ( sal_Int32 nPos = nElements; nPos--; )
        {
            if ( static_cast<OUString *>(pDestElements)[nPos] != static_cast<const OUString *>(pSourceElements)[nPos] )
                return false;
        }
        return true;
    }
    case typelib_TypeClass_TYPE:
    {
        for ( sal_Int32 nPos = nElements; nPos--; )
        {
            if (! _type_equals( static_cast<typelib_TypeDescriptionReference **>(pDestElements)[nPos],
                                static_cast<typelib_TypeDescriptionReference **>(pSourceElements)[nPos] ))
            {
                return false;
            }
        }
        return true;
    }
    case typelib_TypeClass_ANY:
    {
        for ( sal_Int32 nPos = nElements; nPos--; )
        {
            uno_Any * pDest2 = static_cast<uno_Any *>(pDestElements) + nPos;
            uno_Any * pSource2 = static_cast<uno_Any *>(pSourceElements) + nPos;
            if (! ::uno_type_equalData( pDest2->pData, pDest2->pType,
                                        pSource2->pData, pSource2->pType,
                                        queryInterface, release ))
            {
                return false;
            }
        }
        return true;
    }
    case typelib_TypeClass_ENUM:
        return (0 == memcmp( pDestElements, pSourceElements, sizeof(sal_Int32) * nElements ));
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
    {
        typelib_TypeDescription * pElementTypeDescr = nullptr;
        TYPELIB_DANGER_GET( &pElementTypeDescr, pElementType );
        sal_Int32 nElementSize = pElementTypeDescr->nSize;
        for ( sal_Int32 nPos = nElements; nPos--; )
        {
            if (! _equalStruct( static_cast<char *>(pDestElements) + (nPos * nElementSize),
                                static_cast<char *>(pSourceElements) + (nPos * nElementSize),
                                reinterpret_cast<typelib_CompoundTypeDescription *>(pElementTypeDescr),
                                queryInterface, release ))
            {
                TYPELIB_DANGER_RELEASE( pElementTypeDescr );
                return false;
            }
        }
        TYPELIB_DANGER_RELEASE( pElementTypeDescr );
        return true;
    }
    case typelib_TypeClass_SEQUENCE: // sequence of sequence
    {
        typelib_TypeDescription * pElementTypeDescr = nullptr;
        TYPELIB_DANGER_GET( &pElementTypeDescr, pElementType );
        typelib_TypeDescriptionReference * pSeqElementType =
            reinterpret_cast<typelib_IndirectTypeDescription *>(pElementTypeDescr)->pType;
        for ( sal_Int32 nPos = nElements; nPos--; )
        {
            if (! equalSequence( static_cast<uno_Sequence **>(pDestElements)[nPos],
                                 static_cast<uno_Sequence **>(pSourceElements)[nPos],
                                 pSeqElementType, queryInterface, release ))
            {
                TYPELIB_DANGER_RELEASE( pElementTypeDescr );
                return false;
            }
        }
        TYPELIB_DANGER_RELEASE( pElementTypeDescr );
        return true;
    }
    case typelib_TypeClass_INTERFACE:
    {
        for ( sal_Int32 nPos = nElements; nPos--; )
        {
            if (! _equalObject( static_cast<void **>(pDestElements)[nPos], static_cast<void **>(pSourceElements)[nPos],
                                queryInterface, release ))
            {
                return false;
            }
        }
        return true;
    }
    default:
        OSL_ASSERT(false);
        return false;
    }
}

inline bool _equalData(
    void * pDest,
    typelib_TypeDescriptionReference * pDestType, typelib_TypeDescription * pDestTypeDescr,
    void * pSource,
    typelib_TypeDescriptionReference * pSourceType,
    uno_QueryInterfaceFunc queryInterface, uno_ReleaseFunc release )
{
    typelib_TypeClass eSourceTypeClass, eDestTypeClass;
    while (typelib_TypeClass_ANY == (eDestTypeClass = pDestType->eTypeClass))
    {
        pDestTypeDescr = nullptr;
        pDestType = static_cast<uno_Any *>(pDest)->pType;
        pDest = static_cast<uno_Any *>(pDest)->pData;
    }
    while (typelib_TypeClass_ANY == (eSourceTypeClass = pSourceType->eTypeClass))
    {
        pSourceType = static_cast<uno_Any *>(pSource)->pType;
        pSource = static_cast<uno_Any *>(pSource)->pData;
    }

    switch (eDestTypeClass)
    {
    case typelib_TypeClass_VOID:
        return eSourceTypeClass == typelib_TypeClass_VOID;
    case typelib_TypeClass_CHAR:
        return eSourceTypeClass == typelib_TypeClass_CHAR
            && *static_cast<sal_Unicode *>(pDest) == *static_cast<sal_Unicode *>(pSource);
    case typelib_TypeClass_BOOLEAN:
        return eSourceTypeClass == typelib_TypeClass_BOOLEAN
            && (bool(*static_cast<sal_Bool *>(pDest))
                == bool(*static_cast<sal_Bool *>(pSource)));
    case typelib_TypeClass_BYTE:
        switch (eSourceTypeClass)
        {
        case typelib_TypeClass_BYTE:
            return (*static_cast<sal_Int8 *>(pDest) == *static_cast<sal_Int8 *>(pSource));
        case typelib_TypeClass_SHORT:
            return (static_cast<sal_Int16>(*static_cast<sal_Int8 *>(pDest)) == *static_cast<sal_Int16 *>(pSource));
        case typelib_TypeClass_UNSIGNED_SHORT:
            return (static_cast<sal_Int32>(*static_cast<sal_Int8 *>(pDest)) == static_cast<sal_Int32>(*static_cast<sal_uInt16 *>(pSource)));
        case typelib_TypeClass_LONG:
            return (static_cast<sal_Int32>(*static_cast<sal_Int8 *>(pDest)) == *static_cast<sal_Int32 *>(pSource));
        case typelib_TypeClass_UNSIGNED_LONG:
            return (static_cast<sal_Int64>(*static_cast<sal_Int8 *>(pDest)) == static_cast<sal_Int64>(*static_cast<sal_uInt32 *>(pSource)));
        case typelib_TypeClass_HYPER:
            return (static_cast<sal_Int64>(*static_cast<sal_Int8 *>(pDest)) == *static_cast<sal_Int64 *>(pSource));
        case typelib_TypeClass_UNSIGNED_HYPER:
            return (*static_cast<sal_Int8 *>(pDest) >= 0 &&
                    static_cast<sal_Int64>(*static_cast<sal_Int8 *>(pDest)) == *static_cast<sal_Int64 *>(pSource)); // same size
        case typelib_TypeClass_FLOAT:
            return (static_cast<float>(*static_cast<sal_Int8 *>(pDest)) == *static_cast<float *>(pSource));
        case typelib_TypeClass_DOUBLE:
            return (static_cast<double>(*static_cast<sal_Int8 *>(pDest)) == *static_cast<double *>(pSource));
        default:
            return false;
        }
    case typelib_TypeClass_SHORT:
        switch (eSourceTypeClass)
        {
        case typelib_TypeClass_BYTE:
            return (*static_cast<sal_Int16 *>(pDest) == static_cast<sal_Int16>(*static_cast<sal_Int8 *>(pSource)));
        case typelib_TypeClass_SHORT:
            return (*static_cast<sal_Int16 *>(pDest) == *static_cast<sal_Int16 *>(pSource));
        case typelib_TypeClass_UNSIGNED_SHORT:
            return (static_cast<sal_Int32>(*static_cast<sal_Int16 *>(pDest)) == static_cast<sal_Int32>(*static_cast<sal_uInt16 *>(pSource)));
        case typelib_TypeClass_LONG:
            return (static_cast<sal_Int32>(*static_cast<sal_Int16 *>(pDest)) == *static_cast<sal_Int32 *>(pSource));
        case typelib_TypeClass_UNSIGNED_LONG:
            return (static_cast<sal_Int64>(*static_cast<sal_Int16 *>(pDest)) == static_cast<sal_Int64>(*static_cast<sal_uInt32 *>(pSource)));
        case typelib_TypeClass_HYPER:
            return (static_cast<sal_Int64>(*static_cast<sal_Int16 *>(pDest)) == *static_cast<sal_Int64 *>(pSource));
        case typelib_TypeClass_UNSIGNED_HYPER:
            return (*static_cast<sal_Int16 *>(pDest) >= 0 &&
                    static_cast<sal_Int64>(*static_cast<sal_Int16 *>(pDest)) == *static_cast<sal_Int64 *>(pSource)); // same size
        case typelib_TypeClass_FLOAT:
            return (static_cast<float>(*static_cast<sal_Int16 *>(pDest)) == *static_cast<float *>(pSource));
        case typelib_TypeClass_DOUBLE:
            return (static_cast<double>(*static_cast<sal_Int16 *>(pDest)) == *static_cast<double *>(pSource));
        default:
            return false;
        }
    case typelib_TypeClass_UNSIGNED_SHORT:
        switch (eSourceTypeClass)
        {
        case typelib_TypeClass_BYTE:
            return (static_cast<sal_Int32>(*static_cast<sal_uInt16 *>(pDest)) == static_cast<sal_Int32>(*static_cast<sal_Int8 *>(pSource)));
        case typelib_TypeClass_SHORT:
            return (static_cast<sal_Int32>(*static_cast<sal_uInt16 *>(pDest)) == static_cast<sal_Int32>(*static_cast<sal_Int16 *>(pSource)));
        case typelib_TypeClass_UNSIGNED_SHORT:
            return (*static_cast<sal_uInt16 *>(pDest) == *static_cast<sal_uInt16 *>(pSource));
        case typelib_TypeClass_LONG:
            return (static_cast<sal_Int32>(*static_cast<sal_uInt16 *>(pDest)) == *static_cast<sal_Int32 *>(pSource));
        case typelib_TypeClass_UNSIGNED_LONG:
            return (static_cast<sal_uInt32>(*static_cast<sal_uInt16 *>(pDest)) == *static_cast<sal_uInt32 *>(pSource));
        case typelib_TypeClass_HYPER:
            return (static_cast<sal_Int64>(*static_cast<sal_uInt16 *>(pDest)) == *static_cast<sal_Int64 *>(pSource));
        case typelib_TypeClass_UNSIGNED_HYPER:
            return (static_cast<sal_uInt64>(*static_cast<sal_uInt16 *>(pDest)) == *static_cast<sal_uInt64 *>(pSource));
        case typelib_TypeClass_FLOAT:
            return (static_cast<float>(*static_cast<sal_uInt16 *>(pDest)) == *static_cast<float *>(pSource));
        case typelib_TypeClass_DOUBLE:
            return (static_cast<double>(*static_cast<sal_uInt16 *>(pDest)) == *static_cast<double *>(pSource));
        default:
            return false;
        }
    case typelib_TypeClass_LONG:
        switch (eSourceTypeClass)
        {
        case typelib_TypeClass_BYTE:
            return (*static_cast<sal_Int32 *>(pDest) == static_cast<sal_Int32>(*static_cast<sal_Int8 *>(pSource)));
        case typelib_TypeClass_SHORT:
            return (*static_cast<sal_Int32 *>(pDest) == static_cast<sal_Int32>(*static_cast<sal_Int16 *>(pSource)));
        case typelib_TypeClass_UNSIGNED_SHORT:
            return (*static_cast<sal_Int32 *>(pDest) == static_cast<sal_Int32>(*static_cast<sal_uInt16 *>(pSource)));
        case typelib_TypeClass_LONG:
            return (*static_cast<sal_Int32 *>(pDest) == *static_cast<sal_Int32 *>(pSource));
        case typelib_TypeClass_UNSIGNED_LONG:
            return (static_cast<sal_Int64>(*static_cast<sal_Int32 *>(pDest)) == static_cast<sal_Int64>(*static_cast<sal_uInt32 *>(pSource)));
        case typelib_TypeClass_HYPER:
            return (static_cast<sal_Int64>(*static_cast<sal_Int32 *>(pDest)) == *static_cast<sal_Int64 *>(pSource));
        case typelib_TypeClass_UNSIGNED_HYPER:
            return (*static_cast<sal_Int32 *>(pDest) >= 0 &&
                    static_cast<sal_Int64>(*static_cast<sal_Int32 *>(pDest)) == *static_cast<sal_Int64 *>(pSource)); // same size
        case typelib_TypeClass_FLOAT:
            return (static_cast<float>(*static_cast<sal_Int32 *>(pDest)) == *static_cast<float *>(pSource));
        case typelib_TypeClass_DOUBLE:
            return (static_cast<double>(*static_cast<sal_Int32 *>(pDest)) == *static_cast<double *>(pSource));
        default:
            return false;
        }
    case typelib_TypeClass_UNSIGNED_LONG:
        switch (eSourceTypeClass)
        {
        case typelib_TypeClass_BYTE:
            return (static_cast<sal_Int64>(*static_cast<sal_uInt32 *>(pDest)) == static_cast<sal_Int64>(*static_cast<sal_Int8 *>(pSource)));
        case typelib_TypeClass_SHORT:
            return (static_cast<sal_Int64>(*static_cast<sal_uInt32 *>(pDest)) == static_cast<sal_Int64>(*static_cast<sal_Int16 *>(pSource)));
        case typelib_TypeClass_UNSIGNED_SHORT:
            return (*static_cast<sal_uInt32 *>(pDest) == static_cast<sal_uInt32>(*static_cast<sal_uInt16 *>(pSource)));
        case typelib_TypeClass_LONG:
            return (static_cast<sal_Int64>(*static_cast<sal_uInt32 *>(pDest)) == static_cast<sal_Int64>(*static_cast<sal_Int32 *>(pSource)));
        case typelib_TypeClass_UNSIGNED_LONG:
            return (*static_cast<sal_uInt32 *>(pDest) == *static_cast<sal_uInt32 *>(pSource));
        case typelib_TypeClass_HYPER:
            return (static_cast<sal_Int64>(*static_cast<sal_uInt32 *>(pDest)) == *static_cast<sal_Int64 *>(pSource));
        case typelib_TypeClass_UNSIGNED_HYPER:
            return (static_cast<sal_uInt64>(*static_cast<sal_uInt32 *>(pDest)) == *static_cast<sal_uInt64 *>(pSource));
        case typelib_TypeClass_FLOAT:
            return (static_cast<float>(*static_cast<sal_uInt32 *>(pDest)) == *static_cast<float *>(pSource));
        case typelib_TypeClass_DOUBLE:
            return (static_cast<double>(*static_cast<sal_uInt32 *>(pDest)) == *static_cast<double *>(pSource));
        default:
            return false;
        }
    case typelib_TypeClass_HYPER:
        switch (eSourceTypeClass)
        {
        case typelib_TypeClass_BYTE:
            return (*static_cast<sal_Int64 *>(pDest) == static_cast<sal_Int64>(*static_cast<sal_Int8 *>(pSource)));
        case typelib_TypeClass_SHORT:
            return (*static_cast<sal_Int64 *>(pDest) == static_cast<sal_Int64>(*static_cast<sal_Int16 *>(pSource)));
        case typelib_TypeClass_UNSIGNED_SHORT:
            return (*static_cast<sal_Int64 *>(pDest) == static_cast<sal_Int64>(*static_cast<sal_uInt16 *>(pSource)));
        case typelib_TypeClass_LONG:
            return (*static_cast<sal_Int64 *>(pDest) == static_cast<sal_Int64>(*static_cast<sal_Int32 *>(pSource)));
        case typelib_TypeClass_UNSIGNED_LONG:
            return (*static_cast<sal_Int64 *>(pDest) == static_cast<sal_Int64>(*static_cast<sal_uInt32 *>(pSource)));
        case typelib_TypeClass_HYPER:
            return (*static_cast<sal_Int64 *>(pDest) == *static_cast<sal_Int64 *>(pSource));
        case typelib_TypeClass_UNSIGNED_HYPER:
            return (*static_cast<sal_Int64 *>(pDest) >= 0 &&
                    *static_cast<sal_Int64 *>(pDest) == *static_cast<sal_Int64 *>(pSource)); // same size
        case typelib_TypeClass_FLOAT:
            return (static_cast<float>(*static_cast<sal_Int64 *>(pDest)) == *static_cast<float *>(pSource));
        case typelib_TypeClass_DOUBLE:
            return (static_cast<double>(*static_cast<sal_Int64 *>(pDest)) == *static_cast<double *>(pSource));
        default:
            return false;
        }
    case typelib_TypeClass_UNSIGNED_HYPER:
        switch (eSourceTypeClass)
        {
        case typelib_TypeClass_BYTE:
            return (*static_cast<sal_Int8 *>(pSource) >= 0 &&
                    *static_cast<sal_uInt64 *>(pDest) == static_cast<sal_uInt64>(*static_cast<sal_Int8 *>(pSource)));
        case typelib_TypeClass_SHORT:
            return (*static_cast<sal_Int16 *>(pSource) >= 0 &&
                    *static_cast<sal_uInt64 *>(pDest) == static_cast<sal_uInt64>(*static_cast<sal_Int16 *>(pSource)));
        case typelib_TypeClass_UNSIGNED_SHORT:
            return (*static_cast<sal_uInt64 *>(pDest) == static_cast<sal_uInt64>(*static_cast<sal_uInt16 *>(pSource)));
        case typelib_TypeClass_LONG:
            return (*static_cast<sal_Int32 *>(pSource) >= 0 &&
                    *static_cast<sal_uInt64 *>(pDest) == static_cast<sal_uInt64>(*static_cast<sal_Int32 *>(pSource)));
        case typelib_TypeClass_UNSIGNED_LONG:
            return (*static_cast<sal_uInt64 *>(pDest) == static_cast<sal_uInt64>(*static_cast<sal_uInt32 *>(pSource)));
        case typelib_TypeClass_HYPER:
            return (*static_cast<sal_Int64 *>(pSource) >= 0 &&
                    *static_cast<sal_uInt64 *>(pDest) == static_cast<sal_uInt64>(*static_cast<sal_Int64 *>(pSource)));
        case typelib_TypeClass_UNSIGNED_HYPER:
            return (*static_cast<sal_uInt64 *>(pDest) == *static_cast<sal_uInt64 *>(pSource));
        case typelib_TypeClass_FLOAT:
            if (::floor( *static_cast<float *>(pSource) ) != *static_cast<float *>(pSource) || *static_cast<float *>(pSource) < 0)
                return false;
            return (*static_cast<sal_uInt64 *>(pDest) == static_cast<sal_uInt64>(*static_cast<float *>(pSource)));
        case typelib_TypeClass_DOUBLE:
            if (::floor( *static_cast<double *>(pSource) ) != *static_cast<double *>(pSource) || *static_cast<double *>(pSource) < 0)
                return false;
            return (*static_cast<sal_uInt64 *>(pDest) == static_cast<sal_uInt64>(*static_cast<double *>(pSource)));
        default:
            return false;
        }
    case typelib_TypeClass_FLOAT:
        switch (eSourceTypeClass)
        {
        case typelib_TypeClass_BYTE:
            return (*static_cast<float *>(pDest) == static_cast<float>(*static_cast<sal_Int8 *>(pSource)));
        case typelib_TypeClass_SHORT:
            return (*static_cast<float *>(pDest) == static_cast<float>(*static_cast<sal_Int16 *>(pSource)));
        case typelib_TypeClass_UNSIGNED_SHORT:
            return (*static_cast<float *>(pDest) == static_cast<float>(*static_cast<sal_uInt16 *>(pSource)));
        case typelib_TypeClass_LONG:
            return (*static_cast<float *>(pDest) == static_cast<float>(*static_cast<sal_Int32 *>(pSource)));
        case typelib_TypeClass_UNSIGNED_LONG:
            return (*static_cast<float *>(pDest) == static_cast<float>(*static_cast<sal_uInt32 *>(pSource)));
        case typelib_TypeClass_HYPER:
            return (*static_cast<float *>(pDest) == static_cast<float>(*static_cast<sal_Int64 *>(pSource)));
        case typelib_TypeClass_UNSIGNED_HYPER:
            if (::floor( *static_cast<float *>(pDest) ) != *static_cast<float *>(pDest) || *static_cast<float *>(pDest) < 0)
                return false;
            return (static_cast<sal_uInt64>(*static_cast<float *>(pDest)) == *static_cast<sal_uInt64 *>(pSource));
        case typelib_TypeClass_FLOAT:
            return (*static_cast<float *>(pDest) == *static_cast<float *>(pSource));
        case typelib_TypeClass_DOUBLE:
            return (static_cast<double>(*static_cast<float *>(pDest)) == *static_cast<double *>(pSource));
        default:
            return false;
        }
    case typelib_TypeClass_DOUBLE:
        switch (eSourceTypeClass)
        {
        case typelib_TypeClass_BYTE:
            return (*static_cast<double *>(pDest) == static_cast<double>(*static_cast<sal_Int8 *>(pSource)));
        case typelib_TypeClass_SHORT:
            return (*static_cast<double *>(pDest) == static_cast<double>(*static_cast<sal_Int16 *>(pSource)));
        case typelib_TypeClass_UNSIGNED_SHORT:
            return (*static_cast<double *>(pDest) == static_cast<double>(*static_cast<sal_uInt16 *>(pSource)));
        case typelib_TypeClass_LONG:
            return (*static_cast<double *>(pDest) == static_cast<double>(*static_cast<sal_Int32 *>(pSource)));
        case typelib_TypeClass_UNSIGNED_LONG:
            return (*static_cast<double *>(pDest) == static_cast<double>(*static_cast<sal_uInt32 *>(pSource)));
        case typelib_TypeClass_HYPER:
            return (*static_cast<double *>(pDest) == static_cast<double>(*static_cast<sal_Int64 *>(pSource)));
        case typelib_TypeClass_UNSIGNED_HYPER:
            if (::floor( *static_cast<double *>(pDest) ) != *static_cast<double *>(pDest) || *static_cast<double *>(pDest) < 0)
                return false;
            return (static_cast<sal_uInt64>(*static_cast<double *>(pDest)) == *static_cast<sal_uInt64 *>(pSource));
        case typelib_TypeClass_FLOAT:
            return (*static_cast<double *>(pDest) == static_cast<double>(*static_cast<float *>(pSource)));
        case typelib_TypeClass_DOUBLE:
            return (*static_cast<double *>(pDest) == *static_cast<double *>(pSource));
        default:
            return false;
        }
    case typelib_TypeClass_STRING:
        return eSourceTypeClass == typelib_TypeClass_STRING
            && *static_cast<OUString *>(pDest) ==
                *static_cast<OUString const *>(pSource);
    case typelib_TypeClass_TYPE:
        return eSourceTypeClass == typelib_TypeClass_TYPE
            && _type_equals(
                *static_cast<typelib_TypeDescriptionReference **>(pDest),
                *static_cast<typelib_TypeDescriptionReference **>(pSource) );
    case typelib_TypeClass_ENUM:
        return (_type_equals( pDestType, pSourceType ) &&
                *static_cast<sal_Int32 *>(pDest) == *static_cast<sal_Int32 *>(pSource));
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
        if (! _type_equals( pDestType, pSourceType ))
            return false;
        if (pDestTypeDescr)
        {
            return _equalStruct(
                pDest, pSource,
                reinterpret_cast<typelib_CompoundTypeDescription *>(pDestTypeDescr),
                queryInterface, release );
        }
        else
        {
            TYPELIB_DANGER_GET( &pDestTypeDescr, pDestType );
            bool bRet = _equalStruct(
                pDest, pSource,
                reinterpret_cast<typelib_CompoundTypeDescription *>(pDestTypeDescr),
                queryInterface, release );
            TYPELIB_DANGER_RELEASE( pDestTypeDescr );
            return bRet;
        }
    case typelib_TypeClass_SEQUENCE:
        if (_type_equals( pDestType, pSourceType ))
        {
            if (pDestTypeDescr)
            {
                return _equalSequence(
                    *static_cast<uno_Sequence **>(pDest), *static_cast<uno_Sequence **>(pSource),
                    reinterpret_cast<typelib_IndirectTypeDescription *>(pDestTypeDescr)->pType,
                    queryInterface, release );
            }
            else
            {
                TYPELIB_DANGER_GET( &pDestTypeDescr, pDestType );
                bool bRet = _equalSequence(
                    *static_cast<uno_Sequence **>(pDest), *static_cast<uno_Sequence **>(pSource),
                    reinterpret_cast<typelib_IndirectTypeDescription *>(pDestTypeDescr)->pType,
                    queryInterface, release );
                TYPELIB_DANGER_RELEASE( pDestTypeDescr );
                return bRet;
            }
        }
        return false;
    case typelib_TypeClass_INTERFACE:
        if (typelib_TypeClass_INTERFACE == eSourceTypeClass)
            return _equalObject( *static_cast<void **>(pDest), *static_cast<void **>(pSource), queryInterface, release );
        break;
    default:
        OSL_ASSERT(false);
        break;
    }
    return false;
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
