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
#ifndef ASSIGN_HXX
#define ASSIGN_HXX

#include <string.h>

#include "prim.hxx"
#include "destr.hxx"
#include "constr.hxx"
#include "copy.hxx"


namespace cppu
{

//##################################################################################################
//#### assignment ##################################################################################
//##################################################################################################


//--------------------------------------------------------------------------------------------------
inline void _assignInterface(
    void ** ppDest, void * pSource,
    uno_AcquireFunc acquire, uno_ReleaseFunc release )
    SAL_THROW(())
{
    _acquire( pSource, acquire );
    void * const pToBeReleased = *ppDest;
    *ppDest = pSource;
    _release( pToBeReleased, release );
}
//--------------------------------------------------------------------------------------------------
inline void * _queryInterface(
    void * pSource,
    typelib_TypeDescriptionReference * pDestType,
    uno_QueryInterfaceFunc queryInterface )
    SAL_THROW(())
{
    if (pSource)
    {
        if (0 == queryInterface)
            queryInterface = binuno_queryInterface;
        pSource = (*queryInterface)( pSource, pDestType );
    }
    return pSource;
}
//==================================================================================================
sal_Bool assignStruct(
    void * pDest, void * pSource,
    typelib_CompoundTypeDescription * pTypeDescr,
    uno_QueryInterfaceFunc queryInterface, uno_AcquireFunc acquire, uno_ReleaseFunc release )
    SAL_THROW(());
//--------------------------------------------------------------------------------------------------
inline sal_Bool _assignStruct(
    void * pDest, void * pSource,
    typelib_CompoundTypeDescription * pTypeDescr,
    uno_QueryInterfaceFunc queryInterface, uno_AcquireFunc acquire, uno_ReleaseFunc release )
    SAL_THROW(())
{
    if (pTypeDescr->pBaseTypeDescription)
    {
        // copy base value
        if (! assignStruct( pDest, pSource, pTypeDescr->pBaseTypeDescription,
                            queryInterface, acquire, release ))
        {
            return sal_False;
        }
    }
    // then copy members
    typelib_TypeDescriptionReference ** ppTypeRefs = pTypeDescr->ppTypeRefs;
    sal_Int32 * pMemberOffsets = pTypeDescr->pMemberOffsets;
    sal_Int32 nDescr = pTypeDescr->nMembers;
    while (nDescr--)
    {
        if (! ::uno_type_assignData( (char *)pDest + pMemberOffsets[nDescr],
                                     ppTypeRefs[nDescr],
                                     (char *)pSource + pMemberOffsets[nDescr],
                                     ppTypeRefs[nDescr],
                                     queryInterface, acquire, release ))
        {
            return sal_False;
        }
    }
    return sal_True;
}
//--------------------------------------------------------------------------------------------------
inline sal_Bool _assignArray(
    void * pDest, void * pSource,
    typelib_ArrayTypeDescription * pTypeDescr,
    uno_QueryInterfaceFunc queryInterface, uno_AcquireFunc acquire, uno_ReleaseFunc release )
{
    typelib_TypeDescriptionReference * pElementTypeRef =
        ((typelib_IndirectTypeDescription *)pTypeDescr)->pType;
    typelib_TypeDescription * pElementTypeDescr = NULL;
    TYPELIB_DANGER_GET( &pElementTypeDescr, pElementTypeRef );
    sal_Int32 nTotalElements = pTypeDescr->nTotalElements;
    sal_Int32 nElementSize = pElementTypeDescr->nSize;
    sal_Int32 i;
    sal_Bool bRet = sal_False;

    switch ( pElementTypeRef->eTypeClass )
    {
    case typelib_TypeClass_CHAR:
    case typelib_TypeClass_BOOLEAN:
    case typelib_TypeClass_BYTE:
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
    case typelib_TypeClass_FLOAT:
    case typelib_TypeClass_DOUBLE:
        for (i=0; i < nTotalElements; i++)
        {
            memcpy((sal_Char *)pDest + i * nElementSize,
                             (sal_Char *)pSource + i * nElementSize,
                             nElementSize);
        }
        bRet = sal_True;
        break;
    case typelib_TypeClass_STRING:
        for (i=0; i < nTotalElements; i++)
        {
            ::rtl_uString_assign( (rtl_uString **)pDest + i,
                                  ((rtl_uString **)pSource)[i] );
        }
        bRet = sal_True;
        break;
    case typelib_TypeClass_TYPE:
        for (i=0; i < nTotalElements; i++)
        {
            typelib_TypeDescriptionReference ** pp = (typelib_TypeDescriptionReference **)pDest + i;
            ::typelib_typedescriptionreference_release( *pp );
            *pp = *((typelib_TypeDescriptionReference **)pSource + i);
            TYPE_ACQUIRE( *pp );
        }
        bRet = sal_True;
        break;
    case typelib_TypeClass_ANY:
        for (i=0; i < nTotalElements; i++)
        {
            _destructAny( (uno_Any *)pDest + i, release );
            _copyConstructAny( (uno_Any *)pDest + i, (uno_Any *)pSource + i,
                               pElementTypeRef, pElementTypeDescr, acquire, 0 );
        }
        bRet = sal_True;
        break;
    case typelib_TypeClass_ENUM:
        for (i=0; i < nTotalElements; i++)
        {
            *((sal_Int32 *)pDest + i) = *((sal_Int32 *)pSource + i);
        }
        bRet = sal_True;
        break;
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
        for (i=0; i < nTotalElements; i++)
        {
            bRet = _assignStruct( (sal_Char *)pDest + i * nElementSize,
                                  (sal_Char *)pSource + i * nElementSize,
                                  (typelib_CompoundTypeDescription *)pElementTypeDescr,
                                  queryInterface, acquire, release );
            if (! bRet)
                break;
        }
        bRet = sal_True;
        break;
    case typelib_TypeClass_UNION:
        for (i=0; i < nTotalElements; i++)
        {
            _destructUnion( (sal_Char*)pDest + i * nElementSize, pElementTypeDescr, release );
            _copyConstructUnion( (sal_Char*)pDest + i * nElementSize,
                                 (sal_Char*)pSource + i * nElementSize,
                                 pElementTypeDescr, acquire, 0 );
        }
        bRet = sal_True;
        break;
    case typelib_TypeClass_SEQUENCE:
        for (i=0; i < nTotalElements; i++)
        {
            osl_atomic_increment( &(*((uno_Sequence **)pSource + i))->nRefCount );
            idestructSequence(
                *((uno_Sequence **)pDest + i),
                pElementTypeRef, pElementTypeDescr, release );
            *((uno_Sequence **)pDest + i) = *((uno_Sequence **)pSource + i);
        }
        bRet = sal_True;
        break;
    case typelib_TypeClass_INTERFACE:
        for (i=0; i < nTotalElements; i++)
        {
            _assignInterface(
                (void **)((sal_Char*)pDest + i * nElementSize),
                *(void **)((sal_Char*)pSource + i * nElementSize),
                acquire, release );
        }
        bRet = sal_True;
        break;
    default:
        OSL_ASSERT(false);
        break;
    }

    TYPELIB_DANGER_RELEASE( pElementTypeDescr );
    return bRet;
}
//--------------------------------------------------------------------------------------------------
inline sal_Bool _assignData(
    void * pDest,
    typelib_TypeDescriptionReference * pDestType, typelib_TypeDescription * pDestTypeDescr,
    void * pSource,
    typelib_TypeDescriptionReference * pSourceType, typelib_TypeDescription * pSourceTypeDescr,
    uno_QueryInterfaceFunc queryInterface, uno_AcquireFunc acquire, uno_ReleaseFunc release )
    SAL_THROW(())
{
    if (pDest == pSource)
        return _type_equals( pDestType, pSourceType );

    if (! pSource)
    {
        _destructData( pDest, pDestType, pDestTypeDescr, release );
        _defaultConstructData( pDest, pDestType, pDestTypeDescr );
        return sal_True;
    }
    while (typelib_TypeClass_ANY == pSourceType->eTypeClass)
    {
        pSourceTypeDescr = 0;
        pSourceType = ((uno_Any *)pSource)->pType;
        pSource = ((uno_Any *)pSource)->pData;
        if (pDest == pSource)
            return sal_True;
    }

    switch (pDestType->eTypeClass)
    {
    case typelib_TypeClass_VOID:
        return pSourceType->eTypeClass == typelib_TypeClass_VOID;
    case typelib_TypeClass_CHAR:
        switch (pSourceType->eTypeClass)
        {
        case typelib_TypeClass_CHAR:
            *(sal_Unicode *)pDest = *(sal_Unicode *)pSource;
            return sal_True;
        default:
            return sal_False;
        }
    case typelib_TypeClass_BOOLEAN:
        switch (pSourceType->eTypeClass)
        {
        case typelib_TypeClass_BOOLEAN:
            *(sal_Bool *)pDest = (*(sal_Bool *)pSource != sal_False);
            return sal_True;
        default:
            return sal_False;
        }
    case typelib_TypeClass_BYTE:
        switch (pSourceType->eTypeClass)
        {
        case typelib_TypeClass_BYTE:
            *(sal_Int8 *)pDest = *(sal_Int8 *)pSource;
            return sal_True;
        default:
            return sal_False;
        }
    case typelib_TypeClass_SHORT:
        switch (pSourceType->eTypeClass)
        {
        case typelib_TypeClass_BYTE:
            *(sal_Int16 *)pDest = *(sal_Int8 *)pSource;
            return sal_True;
        case typelib_TypeClass_SHORT:
        case typelib_TypeClass_UNSIGNED_SHORT:
            *(sal_Int16 *)pDest = *(sal_Int16 *)pSource;
            return sal_True;
        default:
            return sal_False;
        }
    case typelib_TypeClass_UNSIGNED_SHORT:
        switch (pSourceType->eTypeClass)
        {
        case typelib_TypeClass_BYTE:
            *(sal_uInt16 *)pDest = *(sal_Int8 *)pSource;
            return sal_True;
        case typelib_TypeClass_SHORT:
        case typelib_TypeClass_UNSIGNED_SHORT:
            *(sal_uInt16 *)pDest = *(sal_uInt16 *)pSource;
            return sal_True;
        default:
            return sal_False;
        }
    case typelib_TypeClass_LONG:
        switch (pSourceType->eTypeClass)
        {
        case typelib_TypeClass_BYTE:
            *(sal_Int32 *)pDest = *(sal_Int8 *)pSource;
            return sal_True;
        case typelib_TypeClass_SHORT:
            *(sal_Int32 *)pDest = *(sal_Int16 *)pSource;
            return sal_True;
        case typelib_TypeClass_UNSIGNED_SHORT:
            *(sal_Int32 *)pDest = *(sal_uInt16 *)pSource;
            return sal_True;
        case typelib_TypeClass_LONG:
        case typelib_TypeClass_UNSIGNED_LONG:
            *(sal_Int32 *)pDest = *(sal_Int32 *)pSource;
            return sal_True;
        default:
            return sal_False;
        }
    case typelib_TypeClass_UNSIGNED_LONG:
        switch (pSourceType->eTypeClass)
        {
        case typelib_TypeClass_BYTE:
            *(sal_uInt32 *)pDest = *(sal_Int8 *)pSource;
            return sal_True;
        case typelib_TypeClass_SHORT:
            *(sal_uInt32 *)pDest = *(sal_Int16 *)pSource;
            return sal_True;
        case typelib_TypeClass_UNSIGNED_SHORT:
            *(sal_uInt32 *)pDest = *(sal_uInt16 *)pSource;
            return sal_True;
        case typelib_TypeClass_LONG:
        case typelib_TypeClass_UNSIGNED_LONG:
            *(sal_uInt32 *)pDest = *(sal_uInt32 *)pSource;
            return sal_True;
        default:
            return sal_False;
        }
    case typelib_TypeClass_HYPER:
        switch (pSourceType->eTypeClass)
        {
        case typelib_TypeClass_BYTE:
            *(sal_Int64 *)pDest = *(sal_Int8 *)pSource;
            return sal_True;
        case typelib_TypeClass_SHORT:
            *(sal_Int64 *)pDest = *(sal_Int16 *)pSource;
            return sal_True;
        case typelib_TypeClass_UNSIGNED_SHORT:
            *(sal_Int64 *)pDest = *(sal_uInt16 *)pSource;
            return sal_True;
        case typelib_TypeClass_LONG:
            *(sal_Int64 *)pDest = *(sal_Int32 *)pSource;
            return sal_True;
        case typelib_TypeClass_UNSIGNED_LONG:
            *(sal_Int64 *)pDest = *(sal_uInt32 *)pSource;
            return sal_True;
        case typelib_TypeClass_HYPER:
        case typelib_TypeClass_UNSIGNED_HYPER:
            *(sal_Int64 *)pDest = *(sal_Int64 *)pSource;
            return sal_True;
        default:
            return sal_False;
        }
    case typelib_TypeClass_UNSIGNED_HYPER:
        switch (pSourceType->eTypeClass)
        {
        case typelib_TypeClass_BYTE:
            *(sal_uInt64 *)pDest = *(sal_Int8 *)pSource;
            return sal_True;
        case typelib_TypeClass_SHORT:
            *(sal_uInt64 *)pDest = *(sal_Int16 *)pSource;
            return sal_True;
        case typelib_TypeClass_UNSIGNED_SHORT:
            *(sal_uInt64 *)pDest = *(sal_uInt16 *)pSource;
            return sal_True;
        case typelib_TypeClass_LONG:
            *(sal_uInt64 *)pDest = *(sal_Int32 *)pSource;
            return sal_True;
        case typelib_TypeClass_UNSIGNED_LONG:
            *(sal_uInt64 *)pDest = *(sal_uInt32 *)pSource;
            return sal_True;
        case typelib_TypeClass_HYPER:
        case typelib_TypeClass_UNSIGNED_HYPER:
            *(sal_uInt64 *)pDest = *(sal_uInt64 *)pSource;
            return sal_True;
        default:
            return sal_False;
        }
    case typelib_TypeClass_FLOAT:
        switch (pSourceType->eTypeClass)
        {
        case typelib_TypeClass_BYTE:
            *(float *)pDest = *(sal_Int8 *)pSource;
            return sal_True;
        case typelib_TypeClass_SHORT:
            *(float *)pDest = *(sal_Int16 *)pSource;
            return sal_True;
        case typelib_TypeClass_UNSIGNED_SHORT:
            *(float *)pDest = *(sal_uInt16 *)pSource;
            return sal_True;
        case typelib_TypeClass_FLOAT:
            *(float *)pDest = *(float *)pSource;
            return sal_True;
        default:
            return sal_False;
        }
    case typelib_TypeClass_DOUBLE:
        switch (pSourceType->eTypeClass)
        {
        case typelib_TypeClass_BYTE:
            *(double *)pDest = *(sal_Int8 *)pSource;
            return sal_True;
        case typelib_TypeClass_SHORT:
            *(double *)pDest = *(sal_Int16 *)pSource;
            return sal_True;
        case typelib_TypeClass_UNSIGNED_SHORT:
            *(double *)pDest = *(sal_uInt16 *)pSource;
            return sal_True;
        case typelib_TypeClass_LONG:
            *(double *)pDest = *(sal_Int32 *)pSource;
            return sal_True;
        case typelib_TypeClass_UNSIGNED_LONG:
            *(double *)pDest = *(sal_uInt32 *)pSource;
            return sal_True;
        case typelib_TypeClass_FLOAT:
            *(double *)pDest = *(float *)pSource;
            return sal_True;
        case typelib_TypeClass_DOUBLE:
            *(double *)pDest = *(double *)pSource;
            return sal_True;
        default:
            return sal_False;
        }
    case typelib_TypeClass_STRING:
        switch (pSourceType->eTypeClass)
        {
        case typelib_TypeClass_STRING:
            ::rtl_uString_assign( (rtl_uString **)pDest, *(rtl_uString **)pSource );
            return sal_True;
        default:
            return sal_False;
        }
    case typelib_TypeClass_TYPE:
        switch (pSourceType->eTypeClass)
        {
        case typelib_TypeClass_TYPE:
        {
            typelib_TypeDescriptionReference ** pp = (typelib_TypeDescriptionReference **)pDest;
            ::typelib_typedescriptionreference_release( *pp );
            *pp = *(typelib_TypeDescriptionReference **)pSource;
            TYPE_ACQUIRE( *pp );
            return sal_True;
        }
        default:
            return sal_False;
        }
    case typelib_TypeClass_ANY:
        _destructAny( (uno_Any *)pDest, release );
        _copyConstructAny( (uno_Any *)pDest, pSource, pSourceType, pSourceTypeDescr, acquire, 0 );
        return sal_True;
    case typelib_TypeClass_ENUM:
        if (_type_equals( pDestType, pSourceType ))
        {
            *(sal_Int32 *)pDest = *(sal_Int32 *)pSource;
            return sal_True;
        }
        return sal_False;
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
        if (typelib_TypeClass_STRUCT == pSourceType->eTypeClass ||
            typelib_TypeClass_EXCEPTION == pSourceType->eTypeClass)
        {
            sal_Bool bRet = sal_False;
            if (pSourceTypeDescr)
            {
                typelib_CompoundTypeDescription * pTypeDescr =
                    (typelib_CompoundTypeDescription *)pSourceTypeDescr;
                while (pTypeDescr &&
                       !_type_equals(
                           ((typelib_TypeDescription *)pTypeDescr)->pWeakRef, pDestType ))
                {
                    pTypeDescr = pTypeDescr->pBaseTypeDescription;
                }
                if (pTypeDescr)
                {
                    bRet = _assignStruct(
                        pDest, pSource, pTypeDescr, queryInterface, acquire, release );
                }
            }
            else
            {
                TYPELIB_DANGER_GET( &pSourceTypeDescr, pSourceType );
                typelib_CompoundTypeDescription * pTypeDescr =
                    (typelib_CompoundTypeDescription *)pSourceTypeDescr;
                while (pTypeDescr &&
                       !_type_equals(
                           ((typelib_TypeDescription *)pTypeDescr)->pWeakRef, pDestType ))
                {
                    pTypeDescr = pTypeDescr->pBaseTypeDescription;
                }
                if (pTypeDescr)
                {
                    bRet = _assignStruct(
                        pDest, pSource, pTypeDescr, queryInterface, acquire, release );
                }
                TYPELIB_DANGER_RELEASE( pSourceTypeDescr );
            }
            return bRet;
        }
        return sal_False;
    case typelib_TypeClass_ARRAY:
        {
            sal_Bool bRet = sal_False;
            if (pSourceTypeDescr)
            {
                typelib_ArrayTypeDescription * pTypeDescr =
                    (typelib_ArrayTypeDescription *)pSourceTypeDescr;
                bRet = _assignArray( pDest, pSource, pTypeDescr, queryInterface, acquire, release );
            }
            else
            {
                TYPELIB_DANGER_GET( &pSourceTypeDescr, pSourceType );
                typelib_ArrayTypeDescription * pTypeDescr =
                    (typelib_ArrayTypeDescription *)pSourceTypeDescr;
                if ( pTypeDescr )
                {
                    bRet = _assignArray(
                        pDest, pSource, pTypeDescr, queryInterface, acquire, release );
                }
                TYPELIB_DANGER_RELEASE( pSourceTypeDescr );
            }
            return bRet;
        }
    case typelib_TypeClass_UNION:
        if (_type_equals( pDestType, pSourceType ))
        {
            if (pDestTypeDescr)
            {
                _destructUnion( pDest, pDestTypeDescr, release );
                _copyConstructUnion( pDest, pSource, pDestTypeDescr, acquire, 0 );
            }
            else
            {
                TYPELIB_DANGER_GET( &pDestTypeDescr, pDestType );
                _destructUnion( pDest, pDestTypeDescr, release );
                _copyConstructUnion( pDest, pSource, pDestTypeDescr, acquire, 0 );
                TYPELIB_DANGER_RELEASE( pDestTypeDescr );
            }
            return sal_True;
        }
        return sal_False;
    case typelib_TypeClass_SEQUENCE:
        if (typelib_TypeClass_SEQUENCE != pSourceType->eTypeClass)
            return sal_False;
        // self assignment:
        if (*(uno_Sequence **)pSource == *(uno_Sequence **)pDest)
            return sal_True;
        if (_type_equals( pDestType, pSourceType ))
        {
            osl_atomic_increment( &(*(uno_Sequence **)pSource)->nRefCount );
            idestructSequence(
                *(uno_Sequence **)pDest, pDestType, pDestTypeDescr, release );
            *(uno_Sequence **)pDest = *(uno_Sequence **)pSource;
            return sal_True;
        }
        return sal_False;
    case typelib_TypeClass_INTERFACE:
        if (typelib_TypeClass_INTERFACE != pSourceType->eTypeClass)
            return sal_False;
        if (_type_equals( pDestType, pSourceType ))
        {
            _assignInterface( (void **)pDest, *(void **)pSource, acquire, release );
            return sal_True;
        }
        else if (*static_cast< void ** >(pSource) == 0)
        {
            // A null reference of any interface type can be converted to a null
            // reference of any other interface type:
            void * const pToBeReleased = *static_cast< void ** >(pDest);
            *static_cast< void ** >(pDest) = 0;
            _release( pToBeReleased, release );
            return true;
        }
        else
        {
            if (pSourceTypeDescr)
            {
                typelib_TypeDescription * pTD = pSourceTypeDescr;
                while (pTD && !_type_equals( pTD->pWeakRef, pDestType ))
                {
                    pTD = (typelib_TypeDescription *)
                        ((typelib_InterfaceTypeDescription *)pTD)->pBaseTypeDescription;
                }
                if (pTD) // is base of dest
                {
                    _assignInterface( (void **)pDest, *(void **)pSource, acquire, release );
                    return true;
                }
            }

            // query for interface:
            void * pQueried = _queryInterface( *static_cast<void **>(pSource),
                                               pDestType, queryInterface );
            if (pQueried != 0) {
                void * const pToBeReleased = *static_cast<void **>(pDest);
                *static_cast<void **>(pDest) = pQueried;
                _release( pToBeReleased, release );
            }
            return (pQueried != 0);
        }
    default:
        OSL_ASSERT(false);
        return sal_False;
    }
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
