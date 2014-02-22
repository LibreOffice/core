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

#ifdef __i386





#include <com/sun/star/uno/RuntimeException.hpp>

#include "bridges/cpp_uno/shared/bridge.hxx"
#include "bridges/cpp_uno/shared/types.hxx"
#include "bridges/cpp_uno/shared/unointerfaceproxy.hxx"
#include "bridges/cpp_uno/shared/vtables.hxx"

#include "share.hxx"

using namespace ::rtl;
using namespace ::com::sun::star::uno;

namespace
{








void callVirtualMethod(
    void * pAdjustedThisPtr,
    sal_Int32 nVtableIndex,
    void * pRegisterReturn,
    typelib_TypeDescription * pReturnTypeDescr, bool bSimpleReturn,
    sal_Int32 * pStackLongs,
    sal_Int32 nStackLongs ) __attribute__((noinline));

void callVirtualMethod(
    void * pAdjustedThisPtr,
    sal_Int32 nVtableIndex,
    void * pRegisterReturn,
    typelib_TypeDescription * pReturnTypeDescr, bool bSimpleReturn,
    sal_Int32 * pStackLongs,
    sal_Int32 nStackLongs )
{
    
    

    OSL_ENSURE( pStackLongs && pAdjustedThisPtr, "### null ptr!" );
    OSL_ENSURE( (sizeof(void *) == 4) && (sizeof(sal_Int32) == 4), "### unexpected size of int!" );
    OSL_ENSURE( nStackLongs && pStackLongs, "### no stack in callVirtualMethod !" );

    
    if (! pAdjustedThisPtr) CPPU_CURRENT_NAMESPACE::dummy_can_throw_anything("xxx"); 

    long edx, eax; 
    void * stackptr;
    asm volatile (
        "mov   %%esp, %2\n\t"
        "mov   %3, %%eax\n\t"
        "mov   %%eax, %%edx\n\t"
                
        "shl   $2, %%eax\n\t"
        "neg   %%eax\n\t"
        "add   %%esp, %%eax\n\t"
        "and   $0xf, %%eax\n\t"
        "sub   %%eax, %%esp\n\t"
                
        "mov   %%edx, %%eax\n\t"
        "dec   %%edx\n\t"
        "shl   $2, %%edx\n\t"
        "add   %4, %%edx\n"
        "Lcopy:\n\t"
        "pushl 0(%%edx)\n\t"
        "sub   $4, %%edx\n\t"
        "dec   %%eax\n\t"
        "jne   Lcopy\n\t"
        
        "mov   %5, %%edx\n\t"
        "mov   0(%%edx), %%edx\n\t"
        "mov   %6, %%eax\n\t"
        "shl   $2, %%eax\n\t"
        "add   %%eax, %%edx\n\t"
        "mov   0(%%edx), %%edx\n\t"
        "call  *%%edx\n\t"
        
         "mov   %%eax, %0\n\t"
         "mov   %%edx, %1\n\t"
        
        "mov   %2, %%esp\n\t"
        : "=m"(eax), "=m"(edx), "=m"(stackptr)
        : "m"(nStackLongs), "m"(pStackLongs), "m"(pAdjustedThisPtr), "m"(nVtableIndex)
        : "eax", "edx" );
    switch( pReturnTypeDescr->eTypeClass )
    {
    case typelib_TypeClass_VOID:
        break;
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
        ((long*)pRegisterReturn)[1] = edx;
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
    case typelib_TypeClass_CHAR:
    case typelib_TypeClass_ENUM:
        ((long*)pRegisterReturn)[0] = eax;
        break;
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
        *(unsigned short*)pRegisterReturn = eax;
        break;
    case typelib_TypeClass_BOOLEAN:
    case typelib_TypeClass_BYTE:
        *(unsigned char*)pRegisterReturn = eax;
        break;
    case typelib_TypeClass_FLOAT:
        asm ( "fstps %0" : : "m"(*(char *)pRegisterReturn) );
        break;
    case typelib_TypeClass_DOUBLE:
        asm ( "fstpl %0\n\t" : : "m"(*(char *)pRegisterReturn) );
        break;
    default: {
        sal_Int32 const nRetSize = pReturnTypeDescr->nSize;
        if (bSimpleReturn && nRetSize <= 8 && nRetSize > 0) {
            if (nRetSize > 4)
                static_cast<long *>(pRegisterReturn)[1] = edx;
            static_cast<long *>(pRegisterReturn)[0] = eax;
        }
        break;
    }
    }
}


static void cpp_call(
    bridges::cpp_uno::shared::UnoInterfaceProxy * pThis,
    bridges::cpp_uno::shared::VtableSlot aVtableSlot,
    typelib_TypeDescriptionReference * pReturnTypeRef,
    sal_Int32 nParams, typelib_MethodParameter * pParams,
    void * pUnoReturn, void * pUnoArgs[], uno_Any ** ppUnoExc )
{
      
      char * pCppStack      =
          (char *)alloca( sizeof(sal_Int32) + ((nParams+2) * sizeof(sal_Int64)) );
      char * pCppStackStart = pCppStack;

    
    typelib_TypeDescription * pReturnTypeDescr = 0;
    TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );
    OSL_ENSURE( pReturnTypeDescr, "### expected return type description!" );

    void * pCppReturn = 0; 
    bool bSimpleReturn = true;

    if (pReturnTypeDescr)
    {
        bSimpleReturn = CPPU_CURRENT_NAMESPACE::isSimpleReturnType(
            pReturnTypeDescr);
        if (bSimpleReturn)
        {
            pCppReturn = pUnoReturn; 
        }
        else
        {
            pCppReturn = (bridges::cpp_uno::shared::relatesToInterfaceType(
                              pReturnTypeDescr )
                          ? alloca( pReturnTypeDescr->nSize )
                          : pUnoReturn); 
            
            *(void **)pCppStack = pCppReturn;
            pCppStack += sizeof(void *);
        }
    }
    
    void * pAdjustedThisPtr = reinterpret_cast< void ** >(pThis->getCppI())
        + aVtableSlot.offset;
    *(void**)pCppStack = pAdjustedThisPtr;
    pCppStack += sizeof( void* );

    
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

        if (!rParam.bOut
            && bridges::cpp_uno::shared::isSimpleType( pParamTypeDescr ))
        {
            uno_copyAndConvertData( pCppArgs[nPos] = pCppStack, pUnoArgs[nPos], pParamTypeDescr,
                                    pThis->getBridge()->getUno2Cpp() );

            switch (pParamTypeDescr->eTypeClass)
            {
            case typelib_TypeClass_HYPER:
            case typelib_TypeClass_UNSIGNED_HYPER:
            case typelib_TypeClass_DOUBLE:
                pCppStack += sizeof(sal_Int32); 
            default:
                break;
            }
            
            TYPELIB_DANGER_RELEASE( pParamTypeDescr );
        }
        else 
        {
            if (! rParam.bIn) 
            {
                
                uno_constructData(
                    *(void **)pCppStack = pCppArgs[nPos] = alloca( pParamTypeDescr->nSize ),
                    pParamTypeDescr );
                pTempIndices[nTempIndices] = nPos; 
                
                ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
            }
            
            else if (bridges::cpp_uno::shared::relatesToInterfaceType(
                         pParamTypeDescr ))
            {
                uno_copyAndConvertData(
                    *(void **)pCppStack = pCppArgs[nPos] = alloca( pParamTypeDescr->nSize ),
                    pUnoArgs[nPos], pParamTypeDescr,
                    pThis->getBridge()->getUno2Cpp() );

                pTempIndices[nTempIndices] = nPos; 
                
                ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
            }
            else 
            {
                *(void **)pCppStack = pCppArgs[nPos] = pUnoArgs[nPos];
                
                TYPELIB_DANGER_RELEASE( pParamTypeDescr );
            }
        }
        pCppStack += sizeof(sal_Int32); 
    }

    try
    {
        OSL_ENSURE( !( (pCppStack - pCppStackStart ) & 3), "UNALIGNED STACK !!! (Please DO panic)" );
        callVirtualMethod(
            pAdjustedThisPtr, aVtableSlot.index,
            pCppReturn, pReturnTypeDescr, bSimpleReturn,
            (sal_Int32 *)pCppStackStart, (pCppStack - pCppStackStart) / sizeof(sal_Int32) );
        
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

        SAL_INFO( "bridges.ios", "caught C++ exception" );

          
        CPPU_CURRENT_NAMESPACE::fillUnoException( abi::__cxa_get_globals()->caughtExceptions, *ppUnoExc, pThis->getBridge()->getCpp2Uno() );

        
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

namespace CPPU_CURRENT_NAMESPACE {
bool isSimpleReturnType(typelib_TypeDescription * pTD, bool recursive)
{
    if (bridges::cpp_uno::shared::isSimpleType( pTD ))
        return true;
    
    
    
    if (pTD->eTypeClass == typelib_TypeClass_STRUCT &&
        (recursive || pTD->nSize <= 2 || pTD->nSize == 4 || pTD->nSize == 8))
    {
        typelib_CompoundTypeDescription *const pCompTD =
            (typelib_CompoundTypeDescription *) pTD;
        for ( sal_Int32 pos = pCompTD->nMembers; pos--; ) {
            typelib_TypeDescription * pMemberTD = 0;
            TYPELIB_DANGER_GET( &pMemberTD, pCompTD->ppTypeRefs[pos] );
            bool const b = isSimpleReturnType(pMemberTD, true);
            TYPELIB_DANGER_RELEASE( pMemberTD );
            if (! b)
                return false;
        }
        return true;
    }
    return false;
}
}



namespace bridges { namespace cpp_uno { namespace shared {
void unoInterfaceProxyDispatch(
    uno_Interface * pUnoI, const typelib_TypeDescription * pMemberDescr,
    void * pReturn, void * pArgs[], uno_Any ** ppException )
{
    
    bridges::cpp_uno::shared::UnoInterfaceProxy * pThis
        = static_cast< bridges::cpp_uno::shared::UnoInterfaceProxy * >(pUnoI);

    switch (pMemberDescr->eTypeClass)
    {
    case typelib_TypeClass_INTERFACE_ATTRIBUTE:
    {
        VtableSlot aVtableSlot(
            getVtableSlot(
                reinterpret_cast<
                    typelib_InterfaceAttributeTypeDescription const * >(
                        pMemberDescr)));
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
        VtableSlot aVtableSlot(
            getVtableSlot(
                reinterpret_cast<
                    typelib_InterfaceMethodTypeDescription const * >(
                        pMemberDescr)));
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
                (*pThis->pBridge->getUnoEnv()->getRegisteredInterface)(
                    pThis->pBridge->getUnoEnv(),
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

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
