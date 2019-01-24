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
#ifndef INCLUDED_CPPU_SOURCE_UNO_PRIM_HXX
#define INCLUDED_CPPU_SOURCE_UNO_PRIM_HXX

#include <typelib/typedescription.h>
#include <typelib/typeclass.h>
#include <uno/sequence2.h>
#include <uno/any2.h>
#include <uno/data.h>
#include <uno/mapping.h>
#include <uno/dispatcher.h>

#include <osl/interlck.h>
#include <stdint.h>

namespace cppu
{

extern uno_Sequence g_emptySeq;
extern typelib_TypeDescriptionReference * g_pVoidType;


inline void * _map(
    void * p,
    typelib_TypeDescriptionReference * pType, typelib_TypeDescription * pTypeDescr,
    uno_Mapping * mapping )

{
    void * pRet = nullptr;
    if (p)
    {
        if (pTypeDescr)
        {
            (*mapping->mapInterface)(
                mapping, &pRet, p, reinterpret_cast<typelib_InterfaceTypeDescription *>(pTypeDescr) );
        }
        else
        {
            TYPELIB_DANGER_GET( &pTypeDescr, pType );
            (*mapping->mapInterface)(
                mapping, &pRet, p, reinterpret_cast<typelib_InterfaceTypeDescription *>(pTypeDescr) );
            TYPELIB_DANGER_RELEASE( pTypeDescr );
        }
    }
    return pRet;
}

inline void _acquire( void * p, uno_AcquireFunc acquire )
{
    if (p)
    {
        if (acquire)
        {
            (*acquire)( p );
        }
        else
        {
            (*static_cast<uno_Interface *>(p)->acquire)( static_cast<uno_Interface *>(p) );
        }
    }
}

inline void _release( void * p, uno_ReleaseFunc release )
{
    if (p)
    {
        if (release)
        {
            (*release)( p );
        }
        else
        {
            (*static_cast<uno_Interface *>(p)->release)( static_cast<uno_Interface *>(p) );
        }
    }
}


inline sal_uInt32 calcSeqMemSize(
    sal_Int32 nElementSize, sal_Int32 nElements )
{
    sal_uInt64 nSize =
        static_cast<sal_uInt64>(SAL_SEQUENCE_HEADER_SIZE) +
        (static_cast<sal_uInt64>(nElementSize) * static_cast<sal_uInt64>(nElements));
    if (nSize > 0xffffffffU)
        return 0;
    else
        return static_cast<sal_uInt32>(nSize);
}


inline uno_Sequence * createEmptySequence()
{
    osl_atomic_increment( &g_emptySeq.nRefCount );
    return &g_emptySeq;
}

inline typelib_TypeDescriptionReference * _getVoidType()
{
    if (! g_pVoidType)
    {
        g_pVoidType = * ::typelib_static_type_getByTypeClass( typelib_TypeClass_VOID );
    }
    ::typelib_typedescriptionreference_acquire( g_pVoidType );
    return g_pVoidType;
}

inline void CONSTRUCT_EMPTY_ANY(uno_Any * pAny) {
    pAny->pType = _getVoidType();
#if OSL_DEBUG_LEVEL > 0
    pAny->pData = reinterpret_cast<void *>(uintptr_t(0xdeadbeef));
#else
    pAny->pData = pAny;
#endif
}

#define TYPE_ACQUIRE( pType ) \
    osl_atomic_increment( &(pType)->nRefCount );


extern "C" void * binuno_queryInterface(
    void * pUnoI, typelib_TypeDescriptionReference * pDestType );


inline bool _type_equals(
    typelib_TypeDescriptionReference const * pType1, typelib_TypeDescriptionReference const * pType2 )

{
    return (pType1 == pType2 ||
            (pType1->eTypeClass == pType2->eTypeClass &&
             pType1->pTypeName->length == pType2->pTypeName->length &&
             ::rtl_ustr_compare( pType1->pTypeName->buffer, pType2->pTypeName->buffer ) == 0));
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
