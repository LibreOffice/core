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
#ifndef PRIM_HXX
#define PRIM_HXX

#include "typelib/typedescription.h"
#include "typelib/typeclass.h"
#include "uno/sequence2.h"
#include "uno/any2.h"
#include "uno/data.h"
#include "uno/mapping.h"
#include "uno/dispatcher.h"

#include "osl/interlck.h"
#include "osl/diagnose.h"
#include "rtl/ustring.hxx"
#include "rtl/alloc.h"

#if OSL_DEBUG_LEVEL > 1
#include "rtl/ustrbuf.hxx"
#include "rtl/string.hxx"
#endif


namespace cppu
{

extern uno_Sequence g_emptySeq;
extern typelib_TypeDescriptionReference * g_pVoidType;

//--------------------------------------------------------------------------------------------------
inline void * _map(
    void * p,
    typelib_TypeDescriptionReference * pType, typelib_TypeDescription * pTypeDescr,
    uno_Mapping * mapping )
    SAL_THROW(())
{
    void * pRet = 0;
    if (p)
    {
        if (pTypeDescr)
        {
            (*mapping->mapInterface)(
                mapping, &pRet, p, (typelib_InterfaceTypeDescription *)pTypeDescr );
        }
        else
        {
            TYPELIB_DANGER_GET( &pTypeDescr, pType );
            (*mapping->mapInterface)(
                mapping, &pRet, p, (typelib_InterfaceTypeDescription *)pTypeDescr );
            TYPELIB_DANGER_RELEASE( pTypeDescr );
        }
    }
    return pRet;
}
//--------------------------------------------------------------------------------------------------
inline void _acquire( void * p, uno_AcquireFunc acquire ) SAL_THROW(())
{
    if (p)
    {
        if (acquire)
        {
            (*acquire)( p );
        }
        else
        {
            (*((uno_Interface *)p)->acquire)( (uno_Interface *)p );
        }
    }
}
//--------------------------------------------------------------------------------------------------
inline void _release( void * p, uno_ReleaseFunc release ) SAL_THROW(())
{
    if (p)
    {
        if (release)
        {
            (*release)( p );
        }
        else
        {
            (*((uno_Interface *)p)->release)( (uno_Interface *)p );
        }
    }
}

//------------------------------------------------------------------------------
inline sal_uInt32 calcSeqMemSize(
    sal_Int32 nElementSize, sal_Int32 nElements )
{
    sal_uInt64 nSize =
        (sal_uInt64) SAL_SEQUENCE_HEADER_SIZE +
        ((sal_uInt64) nElementSize * (sal_uInt64) nElements);
    if (nSize > 0xffffffffU)
        return 0;
    else
        return (sal_uInt32) nSize;
}

//--------------------------------------------------------------------------------------------------
inline uno_Sequence * createEmptySequence() SAL_THROW(())
{
    osl_atomic_increment( &g_emptySeq.nRefCount );
    return &g_emptySeq;
}
//--------------------------------------------------------------------------------------------------
inline typelib_TypeDescriptionReference * _getVoidType()
    SAL_THROW(())
{
    if (! g_pVoidType)
    {
        g_pVoidType = * ::typelib_static_type_getByTypeClass( typelib_TypeClass_VOID );
    }
    ::typelib_typedescriptionreference_acquire( g_pVoidType );
    return g_pVoidType;
}

//--------------------------------------------------------------------------------------------------
#if OSL_DEBUG_LEVEL > 0
#define CONSTRUCT_EMPTY_ANY( pAny ) \
(pAny)->pType = _getVoidType(); \
(pAny)->pData = (void *)0xdeadbeef;
#else
#define CONSTRUCT_EMPTY_ANY( pAny ) \
(pAny)->pType = _getVoidType(); \
(pAny)->pData = (pAny);
#endif

//--------------------------------------------------------------------------------------------------
#define TYPE_ACQUIRE( pType ) \
    osl_atomic_increment( &(pType)->nRefCount );

//--------------------------------------------------------------------------------------------------
extern "C" void * binuno_queryInterface(
    void * pUnoI, typelib_TypeDescriptionReference * pDestType );

//--------------------------------------------------------------------------------------------------
inline typelib_TypeDescriptionReference * _unionGetSetType(
    void * pUnion, typelib_TypeDescription * pTD )
    SAL_THROW(())
{
    typelib_TypeDescriptionReference * pRet = 0;
    sal_Int32 nPos;

    sal_Int64 * pDiscr = ((typelib_UnionTypeDescription *)pTD)->pDiscriminants;
    sal_Int64 nDiscr   = *(sal_Int64 *)pUnion;
    for ( nPos = ((typelib_UnionTypeDescription *)pTD)->nMembers; nPos--; )
    {
        if (pDiscr[nPos] == nDiscr)
        {
            pRet = ((typelib_UnionTypeDescription *)pTD)->ppTypeRefs[nPos];
            break;
        }
    }
    if (nPos >= 0)
    {
        // default
        pRet = ((typelib_UnionTypeDescription *)pTD)->pDefaultTypeRef;
    }
    typelib_typedescriptionreference_acquire( pRet );
    return pRet;
}
//--------------------------------------------------------------------------------------------------
inline sal_Bool _type_equals(
    typelib_TypeDescriptionReference * pType1, typelib_TypeDescriptionReference * pType2 )
    SAL_THROW(())
{
    return (pType1 == pType2 ||
            (pType1->eTypeClass == pType2->eTypeClass &&
             pType1->pTypeName->length == pType2->pTypeName->length &&
             ::rtl_ustr_compare( pType1->pTypeName->buffer, pType2->pTypeName->buffer ) == 0));
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
