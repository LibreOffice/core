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
#ifndef EQ_HXX
#define EQ_HXX

#include <math.h>

#include "prim.hxx"
#include "destr.hxx"


namespace cppu
{

//##################################################################################################
//#### equality ####################################################################################
//##################################################################################################

//--------------------------------------------------------------------------------------------------
inline bool _equalObject(
    void * pI1, void * pI2,
    uno_QueryInterfaceFunc queryInterface, uno_ReleaseFunc release )
    SAL_THROW(())
{
    if (pI1 == pI2)
        return true;
    if ((0 == pI1) || (0 == pI2))
        return false;
    bool bRet = false;

    typelib_TypeDescriptionReference * type_XInterface =
        * typelib_static_type_getByTypeClass( typelib_TypeClass_INTERFACE );
    if (0 == queryInterface)
        queryInterface = binuno_queryInterface;
    pI1 = (*queryInterface)( pI1, type_XInterface );
    if (0 != pI1)
    {
        pI2 = (*queryInterface)( pI2, type_XInterface );
        if (0 != pI2)
        {
            bRet = (pI1 == pI2);
            _release( pI2, release );
        }
        _release( pI1, release );
    }
    return bRet;
}

//==================================================================================================
bool equalStruct(
    void * pDest, void *pSource,
    typelib_CompoundTypeDescription * pTypeDescr,
    uno_QueryInterfaceFunc queryInterface, uno_ReleaseFunc release )
    SAL_THROW(());
//--------------------------------------------------------------------------------------------------
inline bool _equalStruct(
    void * pDest, void *pSource,
    typelib_CompoundTypeDescription * pTypeDescr,
    uno_QueryInterfaceFunc queryInterface, uno_ReleaseFunc release )
    SAL_THROW(())
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
        if (! ::uno_type_equalData( (char *)pDest + nOffset,
                                    ppTypeRefs[nDescr],
                                    (char *)pSource + nOffset,
                                    ppTypeRefs[nDescr],
                                    queryInterface, release ))
        {
            return false;
        }
    }
    return true;
}
//==================================================================================================
bool equalSequence(
    uno_Sequence * pDest, uno_Sequence * pSource,
    typelib_TypeDescriptionReference * pElementType,
    uno_QueryInterfaceFunc queryInterface, uno_ReleaseFunc release )
    SAL_THROW(());
//--------------------------------------------------------------------------------------------------
inline bool _equalSequence(
    uno_Sequence * pDest, uno_Sequence * pSource,
    typelib_TypeDescriptionReference * pElementType,
    uno_QueryInterfaceFunc queryInterface, uno_ReleaseFunc release )
    SAL_THROW(())
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
            if ((((sal_Bool *)pDestElements)[nPos] != sal_False) !=
                (((sal_Bool *)pSourceElements)[nPos] != sal_False))
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
            if (((float *)pDestElements)[nPos] != ((float *)pSourceElements)[nPos])
                return false;
        }
        return true;
    }
    case typelib_TypeClass_DOUBLE:
    {
        for ( sal_Int32 nPos = nElements; nPos--; )
        {
            if (((double *)pDestElements)[nPos] != ((double *)pSourceElements)[nPos])
                return false;
        }
        return true;
    }
    case typelib_TypeClass_STRING:
    {
        for ( sal_Int32 nPos = nElements; nPos--; )
        {
            if (! ((::rtl::OUString *)pDestElements +nPos)->equals( ((const ::rtl::OUString *)pSourceElements)[nPos] ))
                return false;
        }
        return true;
    }
    case typelib_TypeClass_TYPE:
    {
        for ( sal_Int32 nPos = nElements; nPos--; )
        {
            if (! _type_equals( ((typelib_TypeDescriptionReference **)pDestElements)[nPos],
                                ((typelib_TypeDescriptionReference **)pSourceElements)[nPos] ))
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
            uno_Any * pDest2 = (uno_Any *)pDestElements + nPos;
            uno_Any * pSource2 = (uno_Any *)pSourceElements + nPos;
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
        typelib_TypeDescription * pElementTypeDescr = 0;
        TYPELIB_DANGER_GET( &pElementTypeDescr, pElementType );
        sal_Int32 nElementSize = pElementTypeDescr->nSize;
        for ( sal_Int32 nPos = nElements; nPos--; )
        {
            if (! _equalStruct( (char *)pDestElements + (nPos * nElementSize),
                                (char *)pSourceElements + (nPos * nElementSize),
                                (typelib_CompoundTypeDescription *)pElementTypeDescr,
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
        typelib_TypeDescription * pElementTypeDescr = 0;
        TYPELIB_DANGER_GET( &pElementTypeDescr, pElementType );
        typelib_TypeDescriptionReference * pSeqElementType =
            ((typelib_IndirectTypeDescription *)pElementTypeDescr)->pType;
        for ( sal_Int32 nPos = nElements; nPos--; )
        {
            if (! equalSequence( ((uno_Sequence **)pDestElements)[nPos],
                                 ((uno_Sequence **)pSourceElements)[nPos],
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
            if (! _equalObject( ((void **)pDestElements)[nPos], ((void **)pSourceElements)[nPos],
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
//--------------------------------------------------------------------------------------------------
inline bool _equalData(
    void * pDest,
    typelib_TypeDescriptionReference * pDestType, typelib_TypeDescription * pDestTypeDescr,
    void * pSource,
    typelib_TypeDescriptionReference * pSourceType,
    uno_QueryInterfaceFunc queryInterface, uno_ReleaseFunc release )
    SAL_THROW(())
{
    typelib_TypeClass eSourceTypeClass, eDestTypeClass;
    while (typelib_TypeClass_ANY == (eDestTypeClass = pDestType->eTypeClass))
    {
        pDestTypeDescr = 0;
        pDestType = ((uno_Any *)pDest)->pType;
        pDest = ((uno_Any *)pDest)->pData;
    }
    while (typelib_TypeClass_ANY == (eSourceTypeClass = pSourceType->eTypeClass))
    {
        pSourceType = ((uno_Any *)pSource)->pType;
        pSource = ((uno_Any *)pSource)->pData;
    }

    switch (eDestTypeClass)
    {
    case typelib_TypeClass_VOID:
        return eSourceTypeClass == typelib_TypeClass_VOID;
    case typelib_TypeClass_CHAR:
        return eSourceTypeClass == typelib_TypeClass_CHAR
            && *(sal_Unicode *)pDest == *(sal_Unicode *)pSource;
    case typelib_TypeClass_BOOLEAN:
        return eSourceTypeClass == typelib_TypeClass_BOOLEAN
            && ((*(sal_Bool *)pDest != sal_False)
                == (*(sal_Bool *)pSource != sal_False));
    case typelib_TypeClass_BYTE:
        switch (eSourceTypeClass)
        {
        case typelib_TypeClass_BYTE:
            return (*(sal_Int8 *)pDest == *(sal_Int8 *)pSource);
        case typelib_TypeClass_SHORT:
            return ((sal_Int16)*(sal_Int8 *)pDest == *(sal_Int16 *)pSource);
        case typelib_TypeClass_UNSIGNED_SHORT:
            return ((sal_Int32)*(sal_Int8 *)pDest == (sal_Int32)*(sal_uInt16 *)pSource);
        case typelib_TypeClass_LONG:
            return ((sal_Int32)*(sal_Int8 *)pDest == *(sal_Int32 *)pSource);
        case typelib_TypeClass_UNSIGNED_LONG:
            return ((sal_Int64)*(sal_Int8 *)pDest == (sal_Int64)*(sal_uInt32 *)pSource);
        case typelib_TypeClass_HYPER:
            return ((sal_Int64)*(sal_Int8 *)pDest == *(sal_Int64 *)pSource);
        case typelib_TypeClass_UNSIGNED_HYPER:
            return (*(sal_Int8 *)pDest >= 0 &&
                    (sal_Int64)*(sal_Int8 *)pDest == *(sal_Int64 *)pSource); // same size
        case typelib_TypeClass_FLOAT:
            return ((float)*(sal_Int8 *)pDest == *(float *)pSource);
        case typelib_TypeClass_DOUBLE:
            return ((double)*(sal_Int8 *)pDest == *(double *)pSource);
        default:
            return false;
        }
    case typelib_TypeClass_SHORT:
        switch (eSourceTypeClass)
        {
        case typelib_TypeClass_BYTE:
            return (*(sal_Int16 *)pDest == (sal_Int16)*(sal_Int8 *)pSource);
        case typelib_TypeClass_SHORT:
            return (*(sal_Int16 *)pDest == *(sal_Int16 *)pSource);
        case typelib_TypeClass_UNSIGNED_SHORT:
            return ((sal_Int32)*(sal_Int16 *)pDest == (sal_Int32)*(sal_uInt16 *)pSource);
        case typelib_TypeClass_LONG:
            return ((sal_Int32)*(sal_Int16 *)pDest == *(sal_Int32 *)pSource);
        case typelib_TypeClass_UNSIGNED_LONG:
            return ((sal_Int64)*(sal_Int16 *)pDest == (sal_Int64)*(sal_uInt32 *)pSource);
        case typelib_TypeClass_HYPER:
            return ((sal_Int64)*(sal_Int16 *)pDest == *(sal_Int64 *)pSource);
        case typelib_TypeClass_UNSIGNED_HYPER:
            return (*(sal_Int16 *)pDest >= 0 &&
                    (sal_Int64)*(sal_Int16 *)pDest == *(sal_Int64 *)pSource); // same size
        case typelib_TypeClass_FLOAT:
            return ((float)*(sal_Int16 *)pDest == *(float *)pSource);
        case typelib_TypeClass_DOUBLE:
            return ((double)*(sal_Int16 *)pDest == *(double *)pSource);
        default:
            return false;
        }
    case typelib_TypeClass_UNSIGNED_SHORT:
        switch (eSourceTypeClass)
        {
        case typelib_TypeClass_BYTE:
            return ((sal_Int32)*(sal_uInt16 *)pDest == (sal_Int32)*(sal_Int8 *)pSource);
        case typelib_TypeClass_SHORT:
            return ((sal_Int32)*(sal_uInt16 *)pDest == (sal_Int32)*(sal_Int16 *)pSource);
        case typelib_TypeClass_UNSIGNED_SHORT:
            return (*(sal_uInt16 *)pDest == *(sal_uInt16 *)pSource);
        case typelib_TypeClass_LONG:
            return ((sal_Int32)*(sal_uInt16 *)pDest == *(sal_Int32 *)pSource);
        case typelib_TypeClass_UNSIGNED_LONG:
            return ((sal_uInt32)*(sal_uInt16 *)pDest == *(sal_uInt32 *)pSource);
        case typelib_TypeClass_HYPER:
            return ((sal_Int64)*(sal_uInt16 *)pDest == *(sal_Int64 *)pSource);
        case typelib_TypeClass_UNSIGNED_HYPER:
            return ((sal_uInt64)*(sal_uInt16 *)pDest == *(sal_uInt64 *)pSource);
        case typelib_TypeClass_FLOAT:
            return ((float)*(sal_uInt16 *)pDest == *(float *)pSource);
        case typelib_TypeClass_DOUBLE:
            return ((double)*(sal_uInt16 *)pDest == *(double *)pSource);
        default:
            return false;
        }
    case typelib_TypeClass_LONG:
        switch (eSourceTypeClass)
        {
        case typelib_TypeClass_BYTE:
            return (*(sal_Int32 *)pDest == (sal_Int32)*(sal_Int8 *)pSource);
        case typelib_TypeClass_SHORT:
            return (*(sal_Int32 *)pDest == (sal_Int32)*(sal_Int16 *)pSource);
        case typelib_TypeClass_UNSIGNED_SHORT:
            return (*(sal_Int32 *)pDest == (sal_Int32)*(sal_uInt16 *)pSource);
        case typelib_TypeClass_LONG:
            return (*(sal_Int32 *)pDest == *(sal_Int32 *)pSource);
        case typelib_TypeClass_UNSIGNED_LONG:
            return ((sal_Int64)*(sal_Int32 *)pDest == (sal_Int64)*(sal_uInt32 *)pSource);
        case typelib_TypeClass_HYPER:
            return ((sal_Int64)*(sal_Int32 *)pDest == *(sal_Int64 *)pSource);
        case typelib_TypeClass_UNSIGNED_HYPER:
            return (*(sal_Int32 *)pDest >= 0 &&
                    (sal_Int64)*(sal_Int32 *)pDest == *(sal_Int64 *)pSource); // same size
        case typelib_TypeClass_FLOAT:
            return ((float)*(sal_Int32 *)pDest == *(float *)pSource);
        case typelib_TypeClass_DOUBLE:
            return ((double)*(sal_Int32 *)pDest == *(double *)pSource);
        default:
            return false;
        }
    case typelib_TypeClass_UNSIGNED_LONG:
        switch (eSourceTypeClass)
        {
        case typelib_TypeClass_BYTE:
            return ((sal_Int64)*(sal_uInt32 *)pDest == (sal_Int64)*(sal_Int8 *)pSource);
        case typelib_TypeClass_SHORT:
            return ((sal_Int64)*(sal_uInt32 *)pDest == (sal_Int64)*(sal_Int16 *)pSource);
        case typelib_TypeClass_UNSIGNED_SHORT:
            return (*(sal_uInt32 *)pDest == (sal_uInt32)*(sal_uInt16 *)pSource);
        case typelib_TypeClass_LONG:
            return ((sal_Int64)*(sal_uInt32 *)pDest == (sal_Int64)*(sal_Int32 *)pSource);
        case typelib_TypeClass_UNSIGNED_LONG:
            return (*(sal_uInt32 *)pDest == *(sal_uInt32 *)pSource);
        case typelib_TypeClass_HYPER:
            return ((sal_Int64)*(sal_uInt32 *)pDest == *(sal_Int64 *)pSource);
        case typelib_TypeClass_UNSIGNED_HYPER:
            return ((sal_uInt64)*(sal_uInt32 *)pDest == *(sal_uInt64 *)pSource);
        case typelib_TypeClass_FLOAT:
            return ((float)*(sal_uInt32 *)pDest == *(float *)pSource);
        case typelib_TypeClass_DOUBLE:
            return ((double)*(sal_uInt32 *)pDest == *(double *)pSource);
        default:
            return false;
        }
    case typelib_TypeClass_HYPER:
        switch (eSourceTypeClass)
        {
        case typelib_TypeClass_BYTE:
            return (*(sal_Int64 *)pDest == (sal_Int64)*(sal_Int8 *)pSource);
        case typelib_TypeClass_SHORT:
            return (*(sal_Int64 *)pDest == (sal_Int64)*(sal_Int16 *)pSource);
        case typelib_TypeClass_UNSIGNED_SHORT:
            return (*(sal_Int64 *)pDest == (sal_Int64)*(sal_uInt16 *)pSource);
        case typelib_TypeClass_LONG:
            return (*(sal_Int64 *)pDest == (sal_Int64)*(sal_Int32 *)pSource);
        case typelib_TypeClass_UNSIGNED_LONG:
            return (*(sal_Int64 *)pDest == (sal_Int64)*(sal_uInt32 *)pSource);
        case typelib_TypeClass_HYPER:
            return (*(sal_Int64 *)pDest == *(sal_Int64 *)pSource);
        case typelib_TypeClass_UNSIGNED_HYPER:
            return (*(sal_Int64 *)pDest >= 0 &&
                    *(sal_Int64 *)pDest == *(sal_Int64 *)pSource); // same size
        case typelib_TypeClass_FLOAT:
            return ((float)*(sal_Int64 *)pDest == *(float *)pSource);
        case typelib_TypeClass_DOUBLE:
            return ((double)*(sal_Int64 *)pDest == *(double *)pSource);
        default:
            return false;
        }
    case typelib_TypeClass_UNSIGNED_HYPER:
        switch (eSourceTypeClass)
        {
        case typelib_TypeClass_BYTE:
            return (*(sal_Int8 *)pSource >= 0 &&
                    *(sal_uInt64 *)pDest == (sal_uInt64)*(sal_Int8 *)pSource);
        case typelib_TypeClass_SHORT:
            return (*(sal_Int16 *)pSource >= 0 &&
                    *(sal_uInt64 *)pDest == (sal_uInt64)*(sal_Int16 *)pSource);
        case typelib_TypeClass_UNSIGNED_SHORT:
            return (*(sal_uInt64 *)pDest == (sal_uInt64)*(sal_uInt16 *)pSource);
        case typelib_TypeClass_LONG:
            return (*(sal_Int32 *)pSource >= 0 &&
                    *(sal_uInt64 *)pDest == (sal_uInt64)*(sal_Int32 *)pSource);
        case typelib_TypeClass_UNSIGNED_LONG:
            return (*(sal_uInt64 *)pDest == (sal_uInt64)*(sal_uInt32 *)pSource);
        case typelib_TypeClass_HYPER:
            return (*(sal_Int64 *)pSource >= 0 &&
                    *(sal_uInt64 *)pDest == (sal_uInt64)*(sal_Int64 *)pSource);
        case typelib_TypeClass_UNSIGNED_HYPER:
            return (*(sal_uInt64 *)pDest == *(sal_uInt64 *)pSource);
        case typelib_TypeClass_FLOAT:
            if (::floor( *(float *)pSource ) != *(float *)pSource || *(float *)pSource < 0)
                return false;
            return (*(sal_uInt64 *)pDest == (sal_uInt64)*(float *)pSource);
        case typelib_TypeClass_DOUBLE:
            if (::floor( *(double *)pSource ) != *(double *)pSource || *(double *)pSource < 0)
                return false;
            return (*(sal_uInt64 *)pDest == (sal_uInt64)*(double *)pSource);
        default:
            return false;
        }
    case typelib_TypeClass_FLOAT:
        switch (eSourceTypeClass)
        {
        case typelib_TypeClass_BYTE:
            return (*(float *)pDest == (float)*(sal_Int8 *)pSource);
        case typelib_TypeClass_SHORT:
            return (*(float *)pDest == (float)*(sal_Int16 *)pSource);
        case typelib_TypeClass_UNSIGNED_SHORT:
            return (*(float *)pDest == (float)*(sal_uInt16 *)pSource);
        case typelib_TypeClass_LONG:
            return (*(float *)pDest == (float)*(sal_Int32 *)pSource);
        case typelib_TypeClass_UNSIGNED_LONG:
            return (*(float *)pDest == (float)*(sal_uInt32 *)pSource);
        case typelib_TypeClass_HYPER:
            return (*(float *)pDest == (float)*(sal_Int64 *)pSource);
        case typelib_TypeClass_UNSIGNED_HYPER:
            if (::floor( *(float *)pDest ) != *(float *)pDest || *(float *)pDest < 0)
                return false;
            return ((sal_uInt64)*(float *)pDest == *(sal_uInt64 *)pSource);
        case typelib_TypeClass_FLOAT:
            return (*(float *)pDest == *(float *)pSource);
        case typelib_TypeClass_DOUBLE:
            return ((double)*(float *)pDest == *(double *)pSource);
        default:
            return false;
        }
    case typelib_TypeClass_DOUBLE:
        switch (eSourceTypeClass)
        {
        case typelib_TypeClass_BYTE:
            return (*(double *)pDest == (double)*(sal_Int8 *)pSource);
        case typelib_TypeClass_SHORT:
            return (*(double *)pDest == (double)*(sal_Int16 *)pSource);
        case typelib_TypeClass_UNSIGNED_SHORT:
            return (*(double *)pDest == (double)*(sal_uInt16 *)pSource);
        case typelib_TypeClass_LONG:
            return (*(double *)pDest == (double)*(sal_Int32 *)pSource);
        case typelib_TypeClass_UNSIGNED_LONG:
            return (*(double *)pDest == (double)*(sal_uInt32 *)pSource);
        case typelib_TypeClass_HYPER:
            return (*(double *)pDest == (double)*(sal_Int64 *)pSource);
        case typelib_TypeClass_UNSIGNED_HYPER:
            if (::floor( *(double *)pDest ) != *(double *)pDest || *(double *)pDest < 0)
                return false;
            return ((sal_uInt64)*(double *)pDest == *(sal_uInt64 *)pSource);
        case typelib_TypeClass_FLOAT:
            return (*(double *)pDest == (double)*(float *)pSource);
        case typelib_TypeClass_DOUBLE:
            return (*(double *)pDest == *(double *)pSource);
        default:
            return false;
        }
    case typelib_TypeClass_STRING:
        return eSourceTypeClass == typelib_TypeClass_STRING
            && ((::rtl::OUString *)pDest)->equals(
                *(::rtl::OUString const *)pSource );
    case typelib_TypeClass_TYPE:
        return eSourceTypeClass == typelib_TypeClass_TYPE
            && _type_equals(
                *(typelib_TypeDescriptionReference **)pDest,
                *(typelib_TypeDescriptionReference **)pSource );
    case typelib_TypeClass_ENUM:
        return (_type_equals( pDestType, pSourceType ) &&
                *(sal_Int32 *)pDest == *(sal_Int32 *)pSource);
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
        if (! _type_equals( pDestType, pSourceType ))
            return false;
        if (pDestTypeDescr)
        {
            return _equalStruct(
                pDest, pSource,
                (typelib_CompoundTypeDescription *)pDestTypeDescr,
                queryInterface, release );
        }
        else
        {
            TYPELIB_DANGER_GET( &pDestTypeDescr, pDestType );
            bool bRet = _equalStruct(
                pDest, pSource,
                (typelib_CompoundTypeDescription *)pDestTypeDescr,
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
                    *(uno_Sequence **)pDest, *(uno_Sequence **)pSource,
                    ((typelib_IndirectTypeDescription *)pDestTypeDescr)->pType,
                    queryInterface, release );
            }
            else
            {
                TYPELIB_DANGER_GET( &pDestTypeDescr, pDestType );
                bool bRet = _equalSequence(
                    *(uno_Sequence **)pDest, *(uno_Sequence **)pSource,
                    ((typelib_IndirectTypeDescription *)pDestTypeDescr)->pType,
                    queryInterface, release );
                TYPELIB_DANGER_RELEASE( pDestTypeDescr );
                return bRet;
            }
        }
        return false;
    case typelib_TypeClass_INTERFACE:
        if (typelib_TypeClass_INTERFACE == eSourceTypeClass)
            return _equalObject( *(void **)pDest, *(void **)pSource, queryInterface, release );
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
