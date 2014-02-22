/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <malloc.h>
#include <rtl/alloc.h>

#include <com/sun/star/uno/genfunc.hxx>
#include "com/sun/star/uno/RuntimeException.hpp"
#include <uno/data.h>

#include <bridges/cpp_uno/shared/bridge.hxx>
#include <bridges/cpp_uno/shared/types.hxx>
#include <bridges/cpp_uno/shared/unointerfaceproxy.hxx>
#include <bridges/cpp_uno/shared/vtables.hxx>

#include "share.hxx"

#include <stdio.h>
#include <string.h>

using namespace ::rtl;
using namespace ::com::sun::star::uno;

void MapReturn(long d0, long d1, typelib_TypeClass eReturnType, long *pRegisterReturn)
{
    register float fret asm("fp0");
    register double dret asm("fp0");

    switch( eReturnType )
    {
        case typelib_TypeClass_HYPER:
        case typelib_TypeClass_UNSIGNED_HYPER:
            pRegisterReturn[1] = d1;
        case typelib_TypeClass_LONG:
        case typelib_TypeClass_UNSIGNED_LONG:
        case typelib_TypeClass_ENUM:
        case typelib_TypeClass_CHAR:
        case typelib_TypeClass_SHORT:
        case typelib_TypeClass_UNSIGNED_SHORT:
        case typelib_TypeClass_BOOLEAN:
        case typelib_TypeClass_BYTE:
            pRegisterReturn[0] = d0;
            break;
        case typelib_TypeClass_FLOAT:
            *(float*)pRegisterReturn = fret;
        break;
        case typelib_TypeClass_DOUBLE:
            *(double*)pRegisterReturn = dret;
            break;
        default:
            break;
    }
}

namespace
{


void callVirtualMethod(
    void * pThis,
    sal_Int32 nVtableIndex,
    void * pRegisterReturn,
    typelib_TypeClass eReturnType,
    sal_uInt32 *pStack,
    sal_uInt32 nStack) __attribute__((noinline));

void callVirtualMethod(
    void * pThis,
    sal_Int32 nVtableIndex,
    void * pRegisterReturn,
    typelib_TypeClass eReturnType,
    sal_uInt32 *pStack,
    sal_uInt32 nStack)
{
    
    if (! pThis)
        CPPU_CURRENT_NAMESPACE::dummy_can_throw_anything("xxx"); 

    if ( nStack )
    {
    
    
        sal_uInt32 nStackBytes = nStack * sizeof(sal_uInt32);
        sal_uInt32 *stack = (sal_uInt32 *) __builtin_alloca( nStackBytes );
        memcpy( stack, pStack, nStackBytes );
    }

#if OSL_DEBUG_LEVEL > 2
        
        {
                fprintf( stderr, "\nStack (%d): ", nStack );
                for ( unsigned int i = 0; i < nStack; ++i )
                        fprintf( stderr, "0x%lx, ", pStack[i] );
                fprintf( stderr, "\n" );
                fprintf( stderr, "pRegisterReturn is %p\n", pRegisterReturn);
        }
#endif

    sal_uInt32 pMethod = *((sal_uInt32*)pThis);
    pMethod += 4 * nVtableIndex;
    pMethod = *((sal_uInt32 *)pMethod);

    typedef long (*FunctionCall )();
    FunctionCall pFunc = (FunctionCall)pMethod;

    
    asm volatile("movel %0,%%a1" : : "m"(pRegisterReturn) : );

    long d0 = (*pFunc)();

    register long d1 asm("d1");

    MapReturn(d0, d1, eReturnType, (long*)pRegisterReturn);
}
}

#define INSERT_INT32( pSV, pDS )\
        *pDS++ = *reinterpret_cast<sal_uInt32 *>( pSV );

#define INSERT_INT64( pSV, pDS )\
        INSERT_INT32( pSV, pDS ) \
        INSERT_INT32( ((sal_uInt32*)pSV)+1, pDS )

#define INSERT_FLOAT( pSV, pDS ) \
    INSERT_INT32( pSV, pDS )

#define INSERT_DOUBLE( pSV, pDS ) \
    INSERT_INT64( pSV, pDS )

#define INSERT_INT16( pSV, pDS ) \
        *pDS++ = *reinterpret_cast<sal_uInt16 *>( pSV );

#define INSERT_INT8( pSV, pDS ) \
        *pDS++ = *reinterpret_cast<sal_uInt8 *>( pSV );

namespace {

static void cpp_call(
    bridges::cpp_uno::shared::UnoInterfaceProxy * pThis,
    bridges::cpp_uno::shared::VtableSlot aVtableSlot,
    typelib_TypeDescriptionReference * pReturnTypeRef,
    sal_Int32 nParams, typelib_MethodParameter * pParams,
    void * pUnoReturn, void * pUnoArgs[], uno_Any ** ppUnoExc )
{

    
    sal_uInt32 * pStack = (sal_uInt32 *)__builtin_alloca(
        sizeof(sal_Int32) + ((nParams+2) * sizeof(sal_Int64)) );
    sal_uInt32 * pStackStart = pStack;

    
    typelib_TypeDescription * pReturnTypeDescr = 0;
    TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );
    OSL_ENSURE( pReturnTypeDescr, "### expected return type description!" );

    void * pCppReturn = 0; 

    if (pReturnTypeDescr)
    {

        if (bridges::cpp_uno::shared::isSimpleType( pReturnTypeDescr ))
        {
            pCppReturn = pUnoReturn; 
        }
        else
        {
            
            pCppReturn = (bridges::cpp_uno::shared::relatesToInterfaceType( pReturnTypeDescr )
                    ? __builtin_alloca( pReturnTypeDescr->nSize )
                    : pUnoReturn); 


        }
    }
    
    void * pAdjustedThisPtr = reinterpret_cast< void ** >(pThis->getCppI())
        + aVtableSlot.offset;
    INSERT_INT32( &pAdjustedThisPtr, pStack );

    
    OSL_ENSURE( sizeof(void *) == sizeof(sal_Int32), "### unexpected size!" );
    
    void ** pCppArgs  = (void **)alloca( 3 * sizeof(void *) * nParams );
    
    sal_Int32 * pTempIndices = (sal_Int32 *)(pCppArgs + nParams);
    
    typelib_TypeDescription ** ppTempParamTypeDescr = (typelib_TypeDescription **)(pCppArgs + (2 * nParams));

    sal_Int32 nTempIndices   = 0;

    for ( sal_Int32 nPos = 0; nPos < nParams; ++nPos )
    {
        const typelib_MethodParameter & rParam = pParams[nPos];
        typelib_TypeDescription * pParamTypeDescr = 0;
        TYPELIB_DANGER_GET( &pParamTypeDescr, rParam.pTypeRef );

        if (!rParam.bOut && bridges::cpp_uno::shared::isSimpleType( pParamTypeDescr ))
        {

            uno_copyAndConvertData( pCppArgs[nPos] = alloca(8), pUnoArgs[nPos],
                pParamTypeDescr, pThis->getBridge()->getUno2Cpp() );

            switch (pParamTypeDescr->eTypeClass)
            {
            case typelib_TypeClass_HYPER:
            case typelib_TypeClass_UNSIGNED_HYPER:
#if OSL_DEBUG_LEVEL > 2
                fprintf(stderr, "hyper is %lx\n", pCppArgs[nPos]);
#endif
                INSERT_INT64( pCppArgs[nPos], pStack );
                break;
            case typelib_TypeClass_LONG:
            case typelib_TypeClass_UNSIGNED_LONG:
            case typelib_TypeClass_ENUM:
#if OSL_DEBUG_LEVEL > 2
                fprintf(stderr, "long is %x\n", pCppArgs[nPos]);
#endif
                INSERT_INT32( pCppArgs[nPos], pStack );
                break;
            case typelib_TypeClass_SHORT:
            case typelib_TypeClass_CHAR:
            case typelib_TypeClass_UNSIGNED_SHORT:
                INSERT_INT16( pCppArgs[nPos], pStack );
                break;
            case typelib_TypeClass_BOOLEAN:
            case typelib_TypeClass_BYTE:
                INSERT_INT8( pCppArgs[nPos], pStack );
                break;
            case typelib_TypeClass_FLOAT:
                INSERT_FLOAT( pCppArgs[nPos], pStack );
                break;
            case typelib_TypeClass_DOUBLE:
                INSERT_DOUBLE( pCppArgs[nPos], pStack );
                break;
            }
            
            TYPELIB_DANGER_RELEASE( pParamTypeDescr );
        }
        else 
        {
            if (! rParam.bIn) 
            {
                
                uno_constructData(
                    pCppArgs[nPos] = alloca( pParamTypeDescr->nSize ),
                    pParamTypeDescr );
                pTempIndices[nTempIndices] = nPos; 
                
                ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
            }
            
            else if (bridges::cpp_uno::shared::relatesToInterfaceType( pParamTypeDescr ))
            {
                uno_copyAndConvertData(
                    pCppArgs[nPos] = alloca( pParamTypeDescr->nSize ),
                    pUnoArgs[nPos], pParamTypeDescr, pThis->getBridge()->getUno2Cpp() );

                pTempIndices[nTempIndices] = nPos; 
                
                ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
            }
            else 
            {
                pCppArgs[nPos] = pUnoArgs[nPos];
                
                TYPELIB_DANGER_RELEASE( pParamTypeDescr );
            }
            INSERT_INT32( &(pCppArgs[nPos]), pStack );
        }
    }

    try
    {
        callVirtualMethod(
            pAdjustedThisPtr, aVtableSlot.index,
            pCppReturn, pReturnTypeDescr->eTypeClass,
            pStackStart,
            (pStack - pStackStart));

        
        *ppUnoExc = 0;

        
        for ( ; nTempIndices--; )
        {
            sal_Int32 nIndex = pTempIndices[nTempIndices];
            typelib_TypeDescription * pParamTypeDescr = ppTempParamTypeDescr[nTempIndices];

            if (pParams[nIndex].bIn)
            {
                if (pParams[nIndex].bOut) 
                {
                    uno_destructData( pUnoArgs[nIndex], pParamTypeDescr, 0 ); 
                    uno_copyAndConvertData( pUnoArgs[nIndex], pCppArgs[nIndex], pParamTypeDescr,
                                            pThis->getBridge()->getCpp2Uno() );
                }
            }
            else 
            {
                uno_copyAndConvertData( pUnoArgs[nIndex], pCppArgs[nIndex], pParamTypeDescr,
                                        pThis->getBridge()->getCpp2Uno() );
            }
            
            uno_destructData( pCppArgs[nIndex], pParamTypeDescr, cpp_release );

            TYPELIB_DANGER_RELEASE( pParamTypeDescr );
        }
        
        if (pCppReturn && pUnoReturn != pCppReturn)
        {
            uno_copyAndConvertData( pUnoReturn, pCppReturn, pReturnTypeDescr,
                                    pThis->getBridge()->getCpp2Uno() );
            uno_destructData( pCppReturn, pReturnTypeDescr, cpp_release );
        }
    }
    catch (...)
    {
        
        fillUnoException( CPPU_CURRENT_NAMESPACE::__cxa_get_globals()->caughtExceptions, *ppUnoExc, pThis->getBridge()->getCpp2Uno() );

        
        for ( ; nTempIndices--; )
        {
            sal_Int32 nIndex = pTempIndices[nTempIndices];
            
            uno_destructData( pCppArgs[nIndex], ppTempParamTypeDescr[nTempIndices], cpp_release );
            TYPELIB_DANGER_RELEASE( ppTempParamTypeDescr[nTempIndices] );
        }

        
        if (pReturnTypeDescr)
            TYPELIB_DANGER_RELEASE( pReturnTypeDescr );
    }
}
}

namespace bridges { namespace cpp_uno { namespace shared {

void unoInterfaceProxyDispatch(
    uno_Interface * pUnoI, const typelib_TypeDescription * pMemberDescr,
    void * pReturn, void * pArgs[], uno_Any ** ppException )
{
    
    bridges::cpp_uno::shared::UnoInterfaceProxy * pThis
          = static_cast< bridges::cpp_uno::shared::UnoInterfaceProxy * >(pUnoI);
    typelib_InterfaceTypeDescription * pTypeDescr = pThis->pTypeDescr;

    switch (pMemberDescr->eTypeClass)
    {
    case typelib_TypeClass_INTERFACE_ATTRIBUTE:
    {
        
        sal_Int32 nMemberPos = ((typelib_InterfaceMemberTypeDescription *)pMemberDescr)->nPosition;
        OSL_ENSURE( nMemberPos < pTypeDescr->nAllMembers, "### member pos out of range!" );

        VtableSlot aVtableSlot(
            getVtableSlot(
            reinterpret_cast<typelib_InterfaceAttributeTypeDescription const *>
              (pMemberDescr)));

        if (pReturn)
        {
            
            cpp_call(
                pThis, aVtableSlot,
                ((typelib_InterfaceAttributeTypeDescription *)pMemberDescr)->pAttributeTypeRef,
                0, 0, 
                pReturn, pArgs, ppException );
        }
        else
        {
            
            typelib_MethodParameter aParam;
            aParam.pTypeRef =
                ((typelib_InterfaceAttributeTypeDescription *)pMemberDescr)->pAttributeTypeRef;
            aParam.bIn      = sal_True;
            aParam.bOut     = sal_False;

            typelib_TypeDescriptionReference * pReturnTypeRef = 0;
            OUString aVoidName("void");
            typelib_typedescriptionreference_new(
                &pReturnTypeRef, typelib_TypeClass_VOID, aVoidName.pData );

            
            aVtableSlot.index += 1;
            cpp_call(
                pThis, aVtableSlot, 
                pReturnTypeRef,
                1, &aParam,
                pReturn, pArgs, ppException );

            typelib_typedescriptionreference_release( pReturnTypeRef );
        }

        break;
    }
    case typelib_TypeClass_INTERFACE_METHOD:
    {
        
        sal_Int32 nMemberPos = ((typelib_InterfaceMemberTypeDescription *)pMemberDescr)->nPosition;
        OSL_ENSURE( nMemberPos < pTypeDescr->nAllMembers, "### member pos out of range!" );

        VtableSlot aVtableSlot(
            getVtableSlot(
            reinterpret_cast<typelib_InterfaceMethodTypeDescription const *>
              (pMemberDescr)));

        switch (aVtableSlot.index)
        {
            
        case 1: 
            (*pUnoI->acquire)( pUnoI );
            *ppException = 0;
            break;
        case 2: 
            (*pUnoI->release)( pUnoI );
            *ppException = 0;
            break;
        case 0: 
        {
            typelib_TypeDescription * pTD = 0;
            TYPELIB_DANGER_GET( &pTD, reinterpret_cast< Type * >( pArgs[0] )->getTypeLibType() );
            if (pTD)
            {
                uno_Interface * pInterface = 0;
                (*pThis->getBridge()->getUnoEnv()->getRegisteredInterface)(
                    pThis->getBridge()->getUnoEnv(),
                    (void **)&pInterface, pThis->oid.pData, (typelib_InterfaceTypeDescription *)pTD );

                if (pInterface)
                {
                    ::uno_any_construct(
                        reinterpret_cast< uno_Any * >( pReturn ),
                        &pInterface, pTD, 0 );
                    (*pInterface->release)( pInterface );
                    TYPELIB_DANGER_RELEASE( pTD );
                    *ppException = 0;
                    break;
                }
                TYPELIB_DANGER_RELEASE( pTD );
            }
        } 
        default:
            
            cpp_call(
                pThis, aVtableSlot,
                ((typelib_InterfaceMethodTypeDescription *)pMemberDescr)->pReturnTypeRef,
                ((typelib_InterfaceMethodTypeDescription *)pMemberDescr)->nParams,
                ((typelib_InterfaceMethodTypeDescription *)pMemberDescr)->pParams,
                pReturn, pArgs, ppException );
        }
        break;
    }
    default:
    {
        ::com::sun::star::uno::RuntimeException aExc(
            OUString("illegal member type description!"),
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >() );

        Type const & rExcType = ::getCppuType( &aExc );
        
        ::uno_type_any_construct( *ppException, &aExc, rExcType.getTypeLibType(), 0 );
    }
    }
}

} } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
