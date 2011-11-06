/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef PRIM_HXX
#define PRIM_HXX

#include "typelib/typedescription.h"
#ifndef _typelib_TypeClass_H_
#include "typelib/typeclass.h"
#endif
#include "uno/sequence2.h"
#include "uno/any2.h"
#include "uno/data.h"
#include "uno/mapping.h"
#include "uno/dispatcher.h"

#ifndef _OSL_INTERLCK_H
#include "osl/interlck.h"
#endif
#include "osl/diagnose.h"
#ifndef _RTL_USTRING_HXX
#include "rtl/ustring.hxx"
#endif
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
    SAL_THROW( () )
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
inline void _acquire( void * p, uno_AcquireFunc acquire ) SAL_THROW( () )
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
inline void _release( void * p, uno_ReleaseFunc release ) SAL_THROW( () )
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
inline uno_Sequence * createEmptySequence() SAL_THROW( () )
{
    ::osl_incrementInterlockedCount( &g_emptySeq.nRefCount );
    return &g_emptySeq;
}
//--------------------------------------------------------------------------------------------------
inline typelib_TypeDescriptionReference * _getVoidType()
    SAL_THROW( () )
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
    ::osl_incrementInterlockedCount( &(pType)->nRefCount );

//--------------------------------------------------------------------------------------------------
extern "C" void * binuno_queryInterface(
    void * pUnoI, typelib_TypeDescriptionReference * pDestType );

//--------------------------------------------------------------------------------------------------
inline typelib_TypeDescriptionReference * _unionGetSetType(
    void * pUnion, typelib_TypeDescription * pTD )
    SAL_THROW( () )
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
    SAL_THROW( () )
{
    return (pType1 == pType2 ||
            (pType1->eTypeClass == pType2->eTypeClass &&
             pType1->pTypeName->length == pType2->pTypeName->length &&
             ::rtl_ustr_compare( pType1->pTypeName->buffer, pType2->pTypeName->buffer ) == 0));
}

}

#endif
