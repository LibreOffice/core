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

#include <com/sun/star/uno/genfunc.hxx>
#include <uno/data.h>

#include "bridges/cpp_uno/shared/bridge.hxx"
#include "bridges/cpp_uno/shared/types.hxx"
#include "bridges/cpp_uno/shared/unointerfaceproxy.hxx"
#include "bridges/cpp_uno/shared/vtables.hxx"

#include "msci.hxx"

using namespace ::rtl;
using namespace ::com::sun::star::uno;

namespace
{


inline static void callVirtualMethod(
    void * pAdjustedThisPtr, sal_Int32 nVtableIndex,
    void * pRegisterReturn, typelib_TypeClass eReturnTypeClass,
    sal_Int32 * pStackLongs, sal_Int32 nStackLongs )
{
    
    

    OSL_ENSURE( pStackLongs && pAdjustedThisPtr, "### null ptr!" );
    OSL_ENSURE( (sizeof(void *) == 4) &&
                 (sizeof(sal_Int32) == 4), "### unexpected size of int!" );

__asm
    {
        mov     eax, nStackLongs
        test    eax, eax
        je      Lcall
        
        mov     ecx, eax
        shl     eax, 2           
        add     eax, pStackLongs 
Lcopy:  sub     eax, 4
        push    dword ptr [eax]
        dec     ecx
        jne     Lcopy
Lcall:
        
        mov     ecx, pAdjustedThisPtr
        push    ecx             
        mov     edx, [ecx]      
        mov     eax, nVtableIndex
        shl     eax, 2          
        add     edx, eax
        call    [edx]           

        
        mov     ecx, eReturnTypeClass
        cmp     ecx, typelib_TypeClass_VOID
        je      Lcleanup
        mov     ebx, pRegisterReturn

        cmp     ecx, typelib_TypeClass_LONG
        je      Lint32
        cmp     ecx, typelib_TypeClass_UNSIGNED_LONG
        je      Lint32
        cmp     ecx, typelib_TypeClass_ENUM
        je      Lint32

        cmp     ecx, typelib_TypeClass_BOOLEAN
        je      Lint8
        cmp     ecx, typelib_TypeClass_BYTE
        je      Lint8

        cmp     ecx, typelib_TypeClass_CHAR
        je      Lint16
        cmp     ecx, typelib_TypeClass_SHORT
        je      Lint16
        cmp     ecx, typelib_TypeClass_UNSIGNED_SHORT
        je      Lint16

        cmp     ecx, typelib_TypeClass_FLOAT
        je      Lfloat

        cmp     ecx, typelib_TypeClass_DOUBLE
        je      Ldouble

        cmp     ecx, typelib_TypeClass_HYPER
        je      Lint64
        cmp     ecx, typelib_TypeClass_UNSIGNED_HYPER
          je        Lint64
        jmp     Lcleanup 
Lint8:
        mov     byte ptr [ebx], al
        jmp     Lcleanup
Lint16:
        mov     word ptr [ebx], ax
        jmp     Lcleanup
Lfloat:
        fstp    dword ptr [ebx]
        jmp     Lcleanup
Ldouble:
        fstp    qword ptr [ebx]
        jmp     Lcleanup
Lint64:
        mov     dword ptr [ebx], eax
        mov     dword ptr [ebx+4], edx
        jmp     Lcleanup
Lint32:
        mov     dword ptr [ebx], eax
        jmp     Lcleanup
Lcleanup:
        
        mov     eax, nStackLongs
        shl     eax, 2          
        add     eax, 4          
        add     esp, eax
    }
}


static void cpp_call(
    bridges::cpp_uno::shared::UnoInterfaceProxy * pThis,
    bridges::cpp_uno::shared::VtableSlot aVtableSlot,
    typelib_TypeDescriptionReference * pReturnTypeRef,
    sal_Int32 nParams, typelib_MethodParameter * pParams,
    void * pUnoReturn, void * pUnoArgs[], uno_Any ** ppUnoExc ) throw ()
{
    
    char * pCppStack        = (char *)alloca( sizeof(sal_Int32) + (nParams * sizeof(sal_Int64)) );
    char * pCppStackStart   = pCppStack;

    
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
            
            pCppReturn = *(void **)pCppStack
                = (bridges::cpp_uno::shared::relatesToInterfaceType(
                       pReturnTypeDescr )
                   ? alloca( pReturnTypeDescr->nSize )
                   : pUnoReturn); 
            pCppStack += sizeof(void *);
        }
    }

    

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
            ::uno_copyAndConvertData(
                pCppArgs[nPos] = pCppStack, pUnoArgs[nPos], pParamTypeDescr,
                pThis->getBridge()->getUno2Cpp() );

            switch (pParamTypeDescr->eTypeClass)
            {
            case typelib_TypeClass_HYPER:
            case typelib_TypeClass_UNSIGNED_HYPER:
            case typelib_TypeClass_DOUBLE:
                pCppStack += sizeof(sal_Int32); 
                break;
            default:
                break;
            }
            
            TYPELIB_DANGER_RELEASE( pParamTypeDescr );
        }
        else 
        {
            if (! rParam.bIn) 
            {
                
                ::uno_constructData(
                    *(void **)pCppStack = pCppArgs[nPos] = alloca( pParamTypeDescr->nSize ),
                    pParamTypeDescr );
                pTempIndices[nTempIndices] = nPos; 
                
                ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
            }
            
            else if (bridges::cpp_uno::shared::relatesToInterfaceType(
                         pParamTypeDescr ))
            {
                ::uno_copyAndConvertData(
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

    __try
    {
        
        callVirtualMethod(
            reinterpret_cast< void ** >(pThis->getCppI()) + aVtableSlot.offset,
            aVtableSlot.index,
            pCppReturn, pReturnTypeDescr->eTypeClass,
            (sal_Int32 *)pCppStackStart,
            (pCppStack - pCppStackStart) / sizeof(sal_Int32) );
    }
    __except (CPPU_CURRENT_NAMESPACE::msci_filterCppException(
                  GetExceptionInformation(),
                  *ppUnoExc, pThis->getBridge()->getCpp2Uno() ))
   {
        
        
        while (nTempIndices--)
        {
            sal_Int32 nIndex = pTempIndices[nTempIndices];
            
            ::uno_destructData(
                pCppArgs[nIndex], ppTempParamTypeDescr[nTempIndices],
                cpp_release );
            TYPELIB_DANGER_RELEASE( ppTempParamTypeDescr[nTempIndices] );
        }
        
        if (pReturnTypeDescr)
        {
            TYPELIB_DANGER_RELEASE( pReturnTypeDescr );
        }
        
        return;
    }

    
    *ppUnoExc = 0;

    
    while (nTempIndices--)
    {
        sal_Int32 nIndex = pTempIndices[nTempIndices];
        typelib_TypeDescription * pParamTypeDescr =
            ppTempParamTypeDescr[nTempIndices];

        if (pParams[nIndex].bIn)
        {
            if (pParams[nIndex].bOut) 
            {
                ::uno_destructData(
                    pUnoArgs[nIndex], pParamTypeDescr, 0 ); 
                ::uno_copyAndConvertData(
                    pUnoArgs[nIndex], pCppArgs[nIndex], pParamTypeDescr,
                    pThis->getBridge()->getCpp2Uno() );
            }
        }
        else 
        {
            ::uno_copyAndConvertData(
                pUnoArgs[nIndex], pCppArgs[nIndex], pParamTypeDescr,
                pThis->getBridge()->getCpp2Uno() );
        }
        
        ::uno_destructData(
            pCppArgs[nIndex], pParamTypeDescr, cpp_release );

        TYPELIB_DANGER_RELEASE( pParamTypeDescr );
    }
    
    if (pCppReturn && pUnoReturn != pCppReturn)
    {
        ::uno_copyAndConvertData(
            pUnoReturn, pCppReturn, pReturnTypeDescr,
            pThis->getBridge()->getCpp2Uno() );
        ::uno_destructData(
            pCppReturn, pReturnTypeDescr, cpp_release );
    }
    
    if (pReturnTypeDescr)
    {
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
