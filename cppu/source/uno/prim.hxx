/*************************************************************************
 *
 *  $RCSfile: prim.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: dbo $ $Date: 2001-03-09 12:10:57 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef __PRIM_HXX__
#define __PRIM_HXX__

#ifndef _TYPELIB_TYPEDESCRIPTION_H_
#include <typelib/typedescription.h>
#endif
#ifndef _typelib_TypeClass_H_
#include <typelib/typeclass.h>
#endif
#ifndef _UNO_SEQUENCE2_H_
#include <uno/sequence2.h>
#endif
#ifndef _UNO_ANY2_H_
#include <uno/any2.h>
#endif
#ifndef _UNO_DATA_H_
#include <uno/data.h>
#endif
#ifndef _UNO_MAPPING_H_
#include <uno/mapping.h>
#endif
#ifndef _UNO_DISPATCHER_H_
#include <uno/dispatcher.h>
#endif

#ifndef _OSL_INTERLCK_H
#include <osl/interlck.h>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _RTL_USTRING_HXX
#include <rtl/ustring.hxx>
#endif
#ifndef _RTL_ALLOC_H_
#include <rtl/alloc.h>
#endif

#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/uno/XInterface.hpp>

namespace cppu
{

extern uno_Sequence * s_pSeq;
extern uno_Sequence s_seq;
extern typelib_TypeDescriptionReference * s_pVoidType;
extern typelib_TypeDescription * s_pQITD;

//--------------------------------------------------------------------------------------------------
inline void * __map(
    void * p,
    typelib_TypeDescriptionReference * pType,
    typelib_TypeDescription * pTypeDescr,
    uno_Mapping * mapping )
    SAL_THROW( () )
{
    void * pRet = 0;
    if (p)
    {
        if (pTypeDescr)
        {
            (*mapping->mapInterface)( mapping, &pRet, p, (typelib_InterfaceTypeDescription *)pTypeDescr );
        }
        else
        {
            TYPELIB_DANGER_GET( &pTypeDescr, pType );
            (*mapping->mapInterface)( mapping, &pRet, p, (typelib_InterfaceTypeDescription *)pTypeDescr );
            TYPELIB_DANGER_RELEASE( pTypeDescr );
        }
    }
    return pRet;
}
//--------------------------------------------------------------------------------------------------
inline void __acquire( void * p, uno_AcquireFunc acquire )
    SAL_THROW( () )
{
    if (p)
    {
        if (acquire)
            (*acquire)( p );
        else
            (*((uno_Interface *)p)->acquire)( (uno_Interface *)p );
    }
}
//--------------------------------------------------------------------------------------------------
inline void __releaseRef( void ** pRef, uno_ReleaseFunc release )
    SAL_THROW( () )
{
    if (*pRef)
    {
        if (release)
            (*release)( *pRef );
        else
            (*((uno_Interface *)*pRef)->release)( (uno_Interface *)*pRef );
    }
}

//--------------------------------------------------------------------------------------------------
inline uno_Sequence * __getEmptySequence() SAL_THROW( () )
{
    if (! s_pSeq)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! s_pSeq)
        {
            s_seq.nRefCount = 1;
            s_seq.nElements = 0;
            s_pSeq = &s_seq;
        }
    }
    ::osl_incrementInterlockedCount( &s_pSeq->nRefCount );
    return s_pSeq;
}
//--------------------------------------------------------------------------------------------------
inline typelib_TypeDescriptionReference * __getVoidType()
    SAL_THROW( () )
{
    if (! s_pVoidType)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! s_pVoidType)
        {
            s_pVoidType = * ::typelib_static_type_getByTypeClass( typelib_TypeClass_VOID );
        }
    }
    ::typelib_typedescriptionreference_acquire( s_pVoidType );
    return s_pVoidType;
}

//--------------------------------------------------------------------------------------------------
#ifdef _DEBUG
#define __CONSTRUCT_EMPTY_ANY( pAny ) \
(pAny)->pType = __getVoidType(); \
(pAny)->pData = (void *)0xdeadbeef;
#else
#define __CONSTRUCT_EMPTY_ANY( pAny ) \
(pAny)->pType = __getVoidType(); \
(pAny)->pData = 0;
#endif

//--------------------------------------------------------------------------------------------------
#define TYPE_ACQUIRE( pType ) \
    ::osl_incrementInterlockedCount( &(pType)->nRefCount );

//--------------------------------------------------------------------------------------------------
inline typelib_TypeDescription * __getQueryInterfaceTypeDescr()
    SAL_THROW( () )
{
    if (! s_pQITD)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! s_pQITD)
        {
            typelib_InterfaceTypeDescription * pTXInterfaceDescr = 0;
            const com::sun::star::uno::Type & rXIType =
                ::getCppuType( (const com::sun::star::uno::Reference< com::sun::star::uno::XInterface > *)0 );
            TYPELIB_DANGER_GET( (typelib_TypeDescription **)&pTXInterfaceDescr, rXIType.getTypeLibType() );
            OSL_ASSERT( pTXInterfaceDescr->ppAllMembers );
            ::typelib_typedescriptionreference_getDescription(
                &s_pQITD, pTXInterfaceDescr->ppAllMembers[0] );
            TYPELIB_DANGER_RELEASE( (typelib_TypeDescription *)pTXInterfaceDescr );
        }
    }
    ::typelib_typedescription_acquire( s_pQITD );
    return s_pQITD;
}

//--------------------------------------------------------------------------------------------------
inline typelib_TypeDescriptionReference * __unionGetSetType(
    void * pUnion, typelib_TypeDescription * pTD )
    SAL_THROW( () )
{
    typelib_TypeDescriptionReference * pRet;

    sal_Int64 * pDiscr = ((typelib_UnionTypeDescription *)pTD)->pDiscriminants;
    sal_Int64 nDiscr   = *(sal_Int64 *)pUnion;
    for ( sal_Int32 nPos = ((typelib_UnionTypeDescription *)pTD)->nMembers; nPos--; )
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
inline sal_Bool __type_equals(
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
