/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: UnoObject.cxx,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cppu.hxx"

#include "osl/interlck.h"
#include "uno/environment.h"
#include "uno/mapping.h"
#include "uno/dispatcher.h"
#include "typelib/typedescription.h"
#include "rtl/alloc.h"
#include "com/sun/star/uno/Any.hxx"

#include "UnoObject.hxx"
#include "callee.hxx"


using namespace com::sun::star;


#ifdef LOG_LIFECYCLE
#  define LOG_LIFECYCLE_UnoObject
#endif

#define LOG_LIFECYCLE_UnoObject
#ifdef LOG_LIFECYCLE_UnoObject
#  include <iostream>
#  define LOG_LIFECYCLE_UnoObject_emit(x) x

#else
#  define LOG_LIFECYCLE_UnoObject_emit(x)

#endif


struct UnoObject : public uno_Interface
{
    oslInterlockedCount   m_nCount;
    Callee              * m_pCallee;
};



static bool s_isQueryInterfaceCall(rtl_uString    * pMethod,
                                   void           * pArgs[],
                                   const sal_Char * pQueriedType)
{
    static rtl::OString aPattern("com.sun.star.uno.XInterface::queryInterface");

    bool bIsQueryInterfaceCall =
        rtl_ustr_ascii_shortenedCompare_WithLength(
            rtl_uString_getStr( pMethod ),
            rtl_uString_getLength( pMethod ),
            aPattern.getStr(),
            aPattern.getLength() ) == 0;

    if (bIsQueryInterfaceCall)
    {
        typelib_TypeDescriptionReference * pTDR
            = *(typelib_TypeDescriptionReference **)pArgs[ 0 ];

        bIsQueryInterfaceCall =
            rtl_ustr_ascii_compare(
                rtl_uString_getStr( pTDR->pTypeName ),
                pQueriedType ) == 0;
    }

    return bIsQueryInterfaceCall;
}

static void s_UnoObject_delete(UnoObject * pUnoObject)
{
    LOG_LIFECYCLE_UnoObject_emit(fprintf(stderr, "LIFE: %s -> %p\n", "s_UnoObject_delete", pUnoObject));

    rtl_freeMemory(pUnoObject);
}


extern "C" {
static void SAL_CALL s_UnoObject_acquire(uno_Interface * pUnoI)
{
    UnoObject * pUnoObject = (UnoObject *)pUnoI;

    pUnoObject->m_pCallee(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uno.XInterface::acquire")).pData);

    osl_incrementInterlockedCount(&pUnoObject->m_nCount);
}

static void SAL_CALL s_UnoObject_release(uno_Interface * pUnoI)
{
    UnoObject * pUnoObject = (UnoObject *)pUnoI;

    pUnoObject->m_pCallee(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uno.XInterface::release")).pData);

    if (osl_decrementInterlockedCount(&pUnoObject->m_nCount) == 0)
        s_UnoObject_delete(pUnoObject);
}

static void SAL_CALL s_UnoObject_dispatch(
    uno_Interface                 * pUnoI,
    typelib_TypeDescription const * pMemberType,
    void                          * pReturn,
    void                          * pArgs[],
    uno_Any                      ** ppException )
{
    UnoObject * pUnoObject = (UnoObject *)pUnoI;
    *ppException = NULL;

    pUnoObject->m_pCallee(rtl::OUString(pMemberType->pTypeName).pData);

    if (s_isQueryInterfaceCall(pMemberType->pTypeName, pArgs, "com.sun.star.uno.XInterface"))
    {
        typelib_TypeDescriptionReference * type_XInterface =
            * typelib_static_type_getByTypeClass( typelib_TypeClass_INTERFACE );

        typelib_typedescriptionreference_acquire(type_XInterface);

        uno::Any * pRet = (uno::Any *)pReturn;
         pRet->pType = type_XInterface;
        pRet->pData = &pRet->pReserved;
        pRet->pReserved = pUnoObject;

        s_UnoObject_acquire(pUnoObject);
    }
    else
        abort();
}
}

uno_Interface * UnoObject_create(Callee * pCallee)
{
    UnoObject * pUnoObject = (UnoObject *)rtl_allocateMemory(sizeof(UnoObject));

    LOG_LIFECYCLE_UnoObject_emit(fprintf(stderr, "LIFE: %s -> %p\n", "UnoObject_create", pUnoObject));

    pUnoObject->m_nCount      = 1;
    pUnoObject->m_pCallee     = pCallee;

    pUnoObject->acquire       = s_UnoObject_acquire;
    pUnoObject->release       = s_UnoObject_release;
    pUnoObject->pDispatcher   = s_UnoObject_dispatch;

    pUnoObject->m_pCallee(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UnoObject_create")).pData);

    return pUnoObject;
}

void UnoObject_release(uno_Interface * pUnoI)
{
    pUnoI->release(pUnoI);
}


void UnoObject_call(uno_Interface * pUnoI)
{
    uno_Any   exception;
    uno_Any * pException = &exception;

    uno_Interface * pUno_XInv = NULL;

     {
        typelib_TypeDescription * g_pQITD = NULL;

        typelib_TypeDescriptionReference * type_XInterface =
            * typelib_static_type_getByTypeClass( typelib_TypeClass_INTERFACE );

        typelib_InterfaceTypeDescription * pTXInterfaceDescr = 0;
        TYPELIB_DANGER_GET( (typelib_TypeDescription **) &pTXInterfaceDescr, type_XInterface );
        typelib_typedescriptionreference_getDescription(
                &g_pQITD, pTXInterfaceDescr->ppAllMembers[ 0 ] );
        TYPELIB_DANGER_RELEASE( (typelib_TypeDescription *) pTXInterfaceDescr );



        uno_Any result;

        void * args[ 1 ];
        args[ 0 ] = &type_XInterface;

        pUnoI->pDispatcher(pUnoI, g_pQITD, &result, args, &pException);


        typelib_TypeDescriptionReference * ret_type = result.pType;
        switch (ret_type->eTypeClass)
        {
        case typelib_TypeClass_VOID: // common case
            typelib_typedescriptionreference_release( ret_type );
            break;
        case typelib_TypeClass_INTERFACE:
            // tweaky... avoiding acquire/ release pair
            typelib_typedescriptionreference_release( ret_type );
            pUno_XInv = (uno_Interface *) result.pReserved; // serving acquired interface
            break;
        default:
            uno_any_destruct(&result, 0);
            break;
        }
     }


    pUno_XInv->release(pUno_XInv);
}
