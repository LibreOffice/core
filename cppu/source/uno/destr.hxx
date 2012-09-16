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
#ifndef DESTR_HXX
#define DESTR_HXX

#include "prim.hxx"


namespace cppu
{

//##################################################################################################
//#### destruction #################################################################################
//##################################################################################################

//--------------------------------------------------------------------------------------------------
inline void _destructUnion(
    void * pValue,
    typelib_TypeDescription * pTypeDescr,
    uno_ReleaseFunc release )
    SAL_THROW(())
{
    typelib_TypeDescriptionReference * pType = _unionGetSetType( pValue, pTypeDescr );
    ::uno_type_destructData(
        (char *)pValue + ((typelib_UnionTypeDescription *)pTypeDescr)->nValueOffset,
        pType, release );
    ::typelib_typedescriptionreference_release( pType );
}
//==================================================================================================
void destructStruct(
    void * pValue,
    typelib_CompoundTypeDescription * pTypeDescr,
    uno_ReleaseFunc release )
    SAL_THROW(());
//--------------------------------------------------------------------------------------------------
inline void _destructStruct(
    void * pValue,
    typelib_CompoundTypeDescription * pTypeDescr,
    uno_ReleaseFunc release )
    SAL_THROW(())
{
    if (pTypeDescr->pBaseTypeDescription)
    {
        destructStruct( pValue, pTypeDescr->pBaseTypeDescription, release );
    }

    typelib_TypeDescriptionReference ** ppTypeRefs = pTypeDescr->ppTypeRefs;
    sal_Int32 * pMemberOffsets = pTypeDescr->pMemberOffsets;
    sal_Int32 nDescr = pTypeDescr->nMembers;
    while (nDescr--)
    {
        ::uno_type_destructData(
            (char *)pValue + pMemberOffsets[nDescr],
            ppTypeRefs[nDescr], release );
    }
}

//--------------------------------------------------------------------------------------------------
inline void _destructArray(
    void * pValue,
    typelib_ArrayTypeDescription * pTypeDescr,
    uno_ReleaseFunc release )
    throw ()
{
    typelib_TypeDescription * pElementType = NULL;
    TYPELIB_DANGER_GET( &pElementType, ((typelib_IndirectTypeDescription *)pTypeDescr)->pType );
    sal_Int32 nElementSize = pElementType->nSize;
    TYPELIB_DANGER_RELEASE( pElementType );

    sal_Int32 nTotalElements = pTypeDescr->nTotalElements;
    for(sal_Int32 i=0; i < nTotalElements; i++)
    {
        ::uno_type_destructData(
            (sal_Char *)pValue + i * nElementSize,
            ((typelib_IndirectTypeDescription *)pTypeDescr)->pType, release );
    }

    typelib_typedescriptionreference_release(((typelib_IndirectTypeDescription *)pTypeDescr)->pType);
}

//==============================================================================
void destructSequence(
    uno_Sequence * pSequence,
    typelib_TypeDescriptionReference * pType,
    typelib_TypeDescription * pTypeDescr,
    uno_ReleaseFunc release );

//--------------------------------------------------------------------------------------------------
inline void _destructAny(
    uno_Any * pAny,
    uno_ReleaseFunc release )
    SAL_THROW(())
{
    typelib_TypeDescriptionReference * pType = pAny->pType;

    switch (pType->eTypeClass)
    {
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
        if (sizeof(void *) < sizeof(sal_Int64))
        {
            ::rtl_freeMemory( pAny->pData );
        }
        break;
    case typelib_TypeClass_FLOAT:
        if (sizeof(void *) < sizeof(float))
        {
            ::rtl_freeMemory( pAny->pData );
        }
        break;
    case typelib_TypeClass_DOUBLE:
        if (sizeof(void *) < sizeof(double))
        {
            ::rtl_freeMemory( pAny->pData );
        }
        break;
    case typelib_TypeClass_STRING:
        ::rtl_uString_release( (rtl_uString *)pAny->pReserved );
        break;
    case typelib_TypeClass_TYPE:
        ::typelib_typedescriptionreference_release(
            (typelib_TypeDescriptionReference *)pAny->pReserved );
        break;
    case typelib_TypeClass_ANY:
        OSL_FAIL( "### unexpected nested any!" );
        ::uno_any_destruct( (uno_Any *)pAny->pData, release );
        ::rtl_freeMemory( pAny->pData );
        break;
    case typelib_TypeClass_TYPEDEF:
        OSL_FAIL( "### unexpected typedef!" );
        break;
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
    {
        typelib_TypeDescription * pTypeDescr = 0;
        TYPELIB_DANGER_GET( &pTypeDescr, pType );
        _destructStruct( pAny->pData, (typelib_CompoundTypeDescription *)pTypeDescr, release );
        TYPELIB_DANGER_RELEASE( pTypeDescr );
        ::rtl_freeMemory( pAny->pData );
        break;
    }
    case typelib_TypeClass_UNION:
    {
        typelib_TypeDescription * pTypeDescr = 0;
        TYPELIB_DANGER_GET( &pTypeDescr, pType );
        _destructUnion( pAny->pData, pTypeDescr, release );
        TYPELIB_DANGER_RELEASE( pTypeDescr );
        ::rtl_freeMemory( pAny->pData );
        break;
    }
    case typelib_TypeClass_SEQUENCE:
    {
        destructSequence(
            *(uno_Sequence **) &pAny->pReserved, pType, 0, release );
        break;
    }
    case typelib_TypeClass_INTERFACE:
        _release( pAny->pReserved, release );
        break;
    default:
        break;
    }
#if OSL_DEBUG_LEVEL > 0
    pAny->pData = (void *)0xdeadbeef;
#endif

    ::typelib_typedescriptionreference_release( pType );
}
//--------------------------------------------------------------------------------------------------
inline sal_Int32 idestructElements(
    void * pElements, typelib_TypeDescriptionReference * pElementType,
    sal_Int32 nStartIndex, sal_Int32 nStopIndex,
    uno_ReleaseFunc release )
    SAL_THROW(())
{
    switch (pElementType->eTypeClass)
    {
    case typelib_TypeClass_CHAR:
        return (sal_Int32)(sizeof(sal_Unicode));
    case typelib_TypeClass_BOOLEAN:
        return (sal_Int32)(sizeof(sal_Bool));
    case typelib_TypeClass_BYTE:
        return (sal_Int32)(sizeof(sal_Int8));
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
        return (sal_Int32)(sizeof(sal_Int16));
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
        return (sal_Int32)(sizeof(sal_Int32));
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
        return (sal_Int32)(sizeof(sal_Int64));
    case typelib_TypeClass_FLOAT:
        return (sal_Int32)(sizeof(float));
    case typelib_TypeClass_DOUBLE:
        return (sal_Int32)(sizeof(double));

    case typelib_TypeClass_STRING:
    {
        rtl_uString ** pDest = (rtl_uString **)pElements;
        for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
        {
            ::rtl_uString_release( pDest[nPos] );
        }
        return (sal_Int32)(sizeof(rtl_uString *));
    }
    case typelib_TypeClass_TYPE:
    {
        typelib_TypeDescriptionReference ** pDest = (typelib_TypeDescriptionReference **)pElements;
        for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
        {
            ::typelib_typedescriptionreference_release( pDest[nPos] );
        }
        return (sal_Int32)(sizeof(typelib_TypeDescriptionReference *));
    }
    case typelib_TypeClass_ANY:
    {
        uno_Any * pDest = (uno_Any *)pElements;
        for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
        {
            _destructAny( &pDest[nPos], release );
        }
        return (sal_Int32)(sizeof(uno_Any));
    }
    case typelib_TypeClass_ENUM:
        return (sal_Int32)(sizeof(sal_Int32));
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
    {
        typelib_TypeDescription * pElementTypeDescr = 0;
        TYPELIB_DANGER_GET( &pElementTypeDescr, pElementType );
        sal_Int32 nElementSize = pElementTypeDescr->nSize;
        for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
        {
            _destructStruct(
                (char *)pElements + (nElementSize * nPos),
                (typelib_CompoundTypeDescription *)pElementTypeDescr,
                release );
        }
        sal_Int32 nSize = pElementTypeDescr->nSize;
        TYPELIB_DANGER_RELEASE( pElementTypeDescr );
        return nSize;
    }
    case typelib_TypeClass_UNION:
    {
        typelib_TypeDescription * pElementTypeDescr = 0;
        TYPELIB_DANGER_GET( &pElementTypeDescr, pElementType );
        sal_Int32 nElementSize = pElementTypeDescr->nSize;
        for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
        {
            _destructUnion(
                (char *)pElements + (nElementSize * nPos),
                pElementTypeDescr,
                release );
        }
        sal_Int32 nSize = pElementTypeDescr->nSize;
        TYPELIB_DANGER_RELEASE( pElementTypeDescr );
        return nSize;
    }
    case typelib_TypeClass_SEQUENCE:
    {
        typelib_TypeDescription * pElementTypeDescr = 0;
        TYPELIB_DANGER_GET( &pElementTypeDescr, pElementType );
        uno_Sequence ** pDest = (uno_Sequence **)pElements;
        for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
        {
            destructSequence(
                pDest[nPos],
                pElementTypeDescr->pWeakRef, pElementTypeDescr,
                release );
        }
        TYPELIB_DANGER_RELEASE( pElementTypeDescr );
        return (sal_Int32)(sizeof(uno_Sequence *));
    }
    case typelib_TypeClass_INTERFACE:
    {
        if (release)
        {
            for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
            {
                void * p = ((void **)pElements)[nPos];
                if (p)
                {
                    (*release)( p );
                }
            }
        }
        else
        {
            for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
            {
                uno_Interface * p = ((uno_Interface **)pElements)[nPos];
                if (p)
                {
                    (*p->release)( p );
                }
            }
        }
        return (sal_Int32)(sizeof(void *));
    }
    default:
        OSL_ASSERT(false);
        return 0;
    }
}

//------------------------------------------------------------------------------
inline void idestructSequence(
    uno_Sequence * pSeq,
    typelib_TypeDescriptionReference * pType,
    typelib_TypeDescription * pTypeDescr,
    uno_ReleaseFunc release )
{
    if (osl_atomic_decrement( &pSeq->nRefCount ) == 0)
    {
        if (pSeq->nElements > 0)
        {
            if (pTypeDescr)
            {
                idestructElements(
                    pSeq->elements,
                    ((typelib_IndirectTypeDescription *) pTypeDescr)->pType, 0,
                    pSeq->nElements, release );
            }
            else
            {
                TYPELIB_DANGER_GET( &pTypeDescr, pType );
                idestructElements(
                    pSeq->elements,
                    ((typelib_IndirectTypeDescription *) pTypeDescr)->pType, 0,
                    pSeq->nElements, release );
                TYPELIB_DANGER_RELEASE( pTypeDescr );
            }
        }
        ::rtl_freeMemory( pSeq );
    }
}

//--------------------------------------------------------------------------------------------------
inline void _destructData(
    void * pValue,
    typelib_TypeDescriptionReference * pType,
    typelib_TypeDescription * pTypeDescr,
    uno_ReleaseFunc release )
    SAL_THROW(())
{
    switch (pType->eTypeClass)
    {
    case typelib_TypeClass_STRING:
        ::rtl_uString_release( *(rtl_uString **)pValue );
        break;
    case typelib_TypeClass_TYPE:
        ::typelib_typedescriptionreference_release( *(typelib_TypeDescriptionReference **)pValue );
        break;
    case typelib_TypeClass_ANY:
        _destructAny( (uno_Any *)pValue, release );
        break;
    case typelib_TypeClass_TYPEDEF:
        OSL_FAIL( "### unexpected typedef!" );
        break;
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
        if (pTypeDescr)
        {
            _destructStruct( pValue, (typelib_CompoundTypeDescription *)pTypeDescr, release );
        }
        else
        {
            TYPELIB_DANGER_GET( &pTypeDescr, pType );
            _destructStruct( pValue, (typelib_CompoundTypeDescription *)pTypeDescr, release );
            TYPELIB_DANGER_RELEASE( pTypeDescr );
        }
        break;
    case typelib_TypeClass_ARRAY:
        if (pTypeDescr)
        {
            _destructArray( pValue, (typelib_ArrayTypeDescription *)pTypeDescr, release );
        }
        else
        {
            TYPELIB_DANGER_GET( &pTypeDescr, pType );
            _destructArray( pValue, (typelib_ArrayTypeDescription *)pTypeDescr, release );
            TYPELIB_DANGER_RELEASE( pTypeDescr );
        }
        break;
    case typelib_TypeClass_UNION:
        if (pTypeDescr)
        {
            _destructUnion( pValue, pTypeDescr, release );
        }
        else
        {
            TYPELIB_DANGER_GET( &pTypeDescr, pType );
            _destructUnion( pValue, pTypeDescr, release );
            TYPELIB_DANGER_RELEASE( pTypeDescr );
        }
        break;
    case typelib_TypeClass_SEQUENCE:
    {
        idestructSequence(
            *(uno_Sequence **)pValue, pType, pTypeDescr, release );
        break;
    }
    case typelib_TypeClass_INTERFACE:
        _release( *(void **)pValue, release );
        break;
    default:
        break;
    }
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
