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
#include <typelib/typedescription.hxx>

#include "bridges/cpp_uno/shared/bridge.hxx"
#include "bridges/cpp_uno/shared/cppinterfaceproxy.hxx"
#include "bridges/cpp_uno/shared/types.hxx"
#include "bridges/cpp_uno/shared/vtablefactory.hxx"

#include "msci.hxx"

using namespace ::com::sun::star::uno;

namespace
{


static inline typelib_TypeClass cpp2uno_call(
    bridges::cpp_uno::shared::CppInterfaceProxy * pThis,
    const typelib_TypeDescription * pMemberTypeDescr,
    typelib_TypeDescriptionReference * pReturnTypeRef, 
    sal_Int32 nParams, typelib_MethodParameter * pParams,
    void ** pCallStack,
    sal_Int64 * pRegisterReturn /* space for register return */ )
{
    
    char * pCppStack = (char *)(pCallStack +2);

    
    typelib_TypeDescription * pReturnTypeDescr = 0;
    if (pReturnTypeRef)
    {
        TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );
    }

    void * pUnoReturn = 0;
    void * pCppReturn = 0; 

    if (pReturnTypeDescr)
    {
        if (bridges::cpp_uno::shared::isSimpleType( pReturnTypeDescr ))
        {
            pUnoReturn = pRegisterReturn; 
        }
        else 
        {
            pCppReturn = *(void **)pCppStack;
            pCppStack += sizeof(void *);

            pUnoReturn = (bridges::cpp_uno::shared::relatesToInterfaceType(
                              pReturnTypeDescr )
                          ? alloca( pReturnTypeDescr->nSize )
                          : pCppReturn); 
        }
    }

    
    OSL_ENSURE( sizeof(void *) == sizeof(sal_Int32), "### unexpected size!" );
    
    void ** pUnoArgs = (void **)alloca( 4 * sizeof(void *) * nParams );
    void ** pCppArgs = pUnoArgs + nParams;
    
    sal_Int32 * pTempIndices = (sal_Int32 *)(pUnoArgs + (2 * nParams));
    
    typelib_TypeDescription ** ppTempParamTypeDescr = (typelib_TypeDescription **)(pUnoArgs + (3 * nParams));

    sal_Int32 nTempIndices = 0;

    for ( sal_Int32 nPos = 0; nPos < nParams; ++nPos )
    {
        const typelib_MethodParameter & rParam = pParams[nPos];
        typelib_TypeDescription * pParamTypeDescr = 0;
        TYPELIB_DANGER_GET( &pParamTypeDescr, rParam.pTypeRef );

        if (!rParam.bOut
            && bridges::cpp_uno::shared::isSimpleType( pParamTypeDescr ))
            
        {
            pCppArgs[nPos] = pCppStack;
            pUnoArgs[nPos] = pCppStack;
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
            pCppArgs[nPos] = *(void **)pCppStack;

            if (! rParam.bIn) 
            {
                
                pUnoArgs[nPos] = alloca( pParamTypeDescr->nSize );
                pTempIndices[nTempIndices] = nPos;
                
                ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
            }
            
            else if (bridges::cpp_uno::shared::relatesToInterfaceType(
                         pParamTypeDescr ))
            {
                ::uno_copyAndConvertData(
                    pUnoArgs[nPos] = alloca( pParamTypeDescr->nSize ),
                    *(void **)pCppStack, pParamTypeDescr,
                    pThis->getBridge()->getCpp2Uno() );
                pTempIndices[nTempIndices] = nPos; 
                
                ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
            }
            else 
            {
                pUnoArgs[nPos] = *(void **)pCppStack;
                
                TYPELIB_DANGER_RELEASE( pParamTypeDescr );
            }
        }
        pCppStack += sizeof(sal_Int32); 
    }

    
    uno_Any aUnoExc; 
    uno_Any * pUnoExc = &aUnoExc;

    
    (*pThis->getUnoI()->pDispatcher)(
        pThis->getUnoI(), pMemberTypeDescr, pUnoReturn, pUnoArgs, &pUnoExc );

    
    if (pUnoExc)
    {
        
        while (nTempIndices--)
        {
            sal_Int32 nIndex = pTempIndices[nTempIndices];

            if (pParams[nIndex].bIn) 
            {
                ::uno_destructData( pUnoArgs[nIndex], ppTempParamTypeDescr[nTempIndices], 0 );
            }
            TYPELIB_DANGER_RELEASE( ppTempParamTypeDescr[nTempIndices] );
        }
        if (pReturnTypeDescr)
        {
            TYPELIB_DANGER_RELEASE( pReturnTypeDescr );
        }

        CPPU_CURRENT_NAMESPACE::msci_raiseException(
            &aUnoExc, pThis->getBridge()->getUno2Cpp() );
            
        
        return typelib_TypeClass_VOID;
    }
    else 
    {
        
        while (nTempIndices--)
        {
            sal_Int32 nIndex = pTempIndices[nTempIndices];
            typelib_TypeDescription * pParamTypeDescr = ppTempParamTypeDescr[nTempIndices];

            if (pParams[nIndex].bOut) 
            {
                
                ::uno_destructData(
                    pCppArgs[nIndex], pParamTypeDescr, cpp_release );
                ::uno_copyAndConvertData(
                    pCppArgs[nIndex], pUnoArgs[nIndex], pParamTypeDescr,
                    pThis->getBridge()->getUno2Cpp() );
            }
            
            ::uno_destructData( pUnoArgs[nIndex], pParamTypeDescr, 0 );

            TYPELIB_DANGER_RELEASE( pParamTypeDescr );
        }
        
        if (pCppReturn) 
        {
            if (pUnoReturn != pCppReturn) 
            {
                ::uno_copyAndConvertData(
                    pCppReturn, pUnoReturn, pReturnTypeDescr,
                    pThis->getBridge()->getUno2Cpp() );
                
                ::uno_destructData(
                    pUnoReturn, pReturnTypeDescr, 0 );
            }
            
            *(void **)pRegisterReturn = pCppReturn;
        }
        if (pReturnTypeDescr)
        {
            typelib_TypeClass eRet = (typelib_TypeClass)pReturnTypeDescr->eTypeClass;
            TYPELIB_DANGER_RELEASE( pReturnTypeDescr );
            return eRet;
        }
        else
            return typelib_TypeClass_VOID;
    }
}


static typelib_TypeClass __cdecl cpp_mediate(
    void ** pCallStack, sal_Int32 nFunctionIndex, sal_Int32 nVtableOffset,
    sal_Int64 * pRegisterReturn /* space for register return */ )
{
    OSL_ENSURE( sizeof(sal_Int32)==sizeof(void *), "### unexpected!" );

    
    void * pThis = static_cast< char * >(pCallStack[1]) - nVtableOffset;
    bridges::cpp_uno::shared::CppInterfaceProxy * pCppI
        = bridges::cpp_uno::shared::CppInterfaceProxy::castInterfaceToProxy(
            pThis);

    typelib_InterfaceTypeDescription * pTypeDescr = pCppI->getTypeDescr();
    OSL_ENSURE( nFunctionIndex < pTypeDescr->nMapFunctionIndexToMemberIndex,
                 "### illegal vtable index!" );
    if (nFunctionIndex >= pTypeDescr->nMapFunctionIndexToMemberIndex)
    {
        throw RuntimeException("illegal vtable index!",
                                (XInterface *)pThis );
    }

    
    sal_Int32 nMemberPos = pTypeDescr->pMapFunctionIndexToMemberIndex[nFunctionIndex];
    OSL_ENSURE( nMemberPos < pTypeDescr->nAllMembers, "### illegal member index!" );

    TypeDescription aMemberDescr( pTypeDescr->ppAllMembers[nMemberPos] );

    typelib_TypeClass eRet = typelib_TypeClass_VOID;
    switch (aMemberDescr.get()->eTypeClass)
    {
    case typelib_TypeClass_INTERFACE_ATTRIBUTE:
    {
        if (pTypeDescr->pMapMemberIndexToFunctionIndex[nMemberPos] == nFunctionIndex)
        {
            
            eRet = cpp2uno_call(
                pCppI, aMemberDescr.get(),
                ((typelib_InterfaceAttributeTypeDescription *)aMemberDescr.get())->pAttributeTypeRef,
                0, 0, 
                pCallStack, pRegisterReturn );
        }
        else
        {
            
            typelib_MethodParameter aParam;
            aParam.pTypeRef =
                ((typelib_InterfaceAttributeTypeDescription *)aMemberDescr.get())->pAttributeTypeRef;
            aParam.bIn      = sal_True;
            aParam.bOut     = sal_False;

            eRet = cpp2uno_call(
                pCppI, aMemberDescr.get(),
                0, 
                1, &aParam,
                pCallStack, pRegisterReturn );
        }
        break;
    }
    case typelib_TypeClass_INTERFACE_METHOD:
    {
        
        switch (nFunctionIndex)
        {
            
        case 1: 
            pCppI->acquireProxy(); 
            eRet = typelib_TypeClass_VOID;
            break;
        case 2: 
            pCppI->releaseProxy(); 
            eRet = typelib_TypeClass_VOID;
            break;
        case 0: 
        {
            typelib_TypeDescription * pTD = 0;
            TYPELIB_DANGER_GET( &pTD, reinterpret_cast< Type * >( pCallStack[3] )->getTypeLibType() );
            if (pTD)
            {
                XInterface * pInterface = 0;
                (*pCppI->getBridge()->getCppEnv()->getRegisteredInterface)(
                    pCppI->getBridge()->getCppEnv(),
                    (void **)&pInterface, pCppI->getOid().pData,
                    (typelib_InterfaceTypeDescription *)pTD );

                if (pInterface)
                {
                    ::uno_any_construct(
                        reinterpret_cast< uno_Any * >( pCallStack[2] ),
                        &pInterface, pTD, cpp_acquire );
                    pInterface->release();
                    TYPELIB_DANGER_RELEASE( pTD );
                    *(void **)pRegisterReturn = pCallStack[2];
                    eRet = typelib_TypeClass_ANY;
                    break;
                }
                TYPELIB_DANGER_RELEASE( pTD );
            }
        } 
        default:
            eRet = cpp2uno_call(
                pCppI, aMemberDescr.get(),
                ((typelib_InterfaceMethodTypeDescription *)aMemberDescr.get())->pReturnTypeRef,
                ((typelib_InterfaceMethodTypeDescription *)aMemberDescr.get())->nParams,
                ((typelib_InterfaceMethodTypeDescription *)aMemberDescr.get())->pParams,
                pCallStack, pRegisterReturn );
        }
        break;
    }
    default:
    {
        throw RuntimeException(
            OUString("no member description found!"),
            (XInterface *)pThis );
    }
    }

    return eRet;
}


/**
 * is called on incoming vtable calls
 * (called by asm snippets)
 */
static __declspec(naked) void __cdecl cpp_vtable_call(void)
{
__asm
    {
        sub     esp, 8      
        push    esp
        push    edx         
        push    eax         
        mov     eax, esp
        add     eax, 20
        push    eax         

        call    cpp_mediate
        add     esp, 16

        cmp     eax, typelib_TypeClass_FLOAT
        je      Lfloat
        cmp     eax, typelib_TypeClass_DOUBLE
        je      Ldouble
        cmp     eax, typelib_TypeClass_HYPER
        je      Lhyper
        cmp     eax, typelib_TypeClass_UNSIGNED_HYPER
        je      Lhyper
        
        pop     eax
        add     esp, 4
        ret
Lhyper:
        pop     eax
        pop     edx
        ret
Lfloat:
        fld     dword ptr [esp]
        add     esp, 8
        ret
Ldouble:
        fld     qword ptr [esp]
        add     esp, 8
        ret
    }
}


int const codeSnippetSize = 16;

unsigned char * codeSnippet(
    unsigned char * code, sal_Int32 functionIndex, sal_Int32 vtableOffset)
{
    unsigned char * p = code;
    OSL_ASSERT(sizeof (sal_Int32) == 4);
    
    *p++ = 0xB8;
    *reinterpret_cast< sal_Int32 * >(p) = functionIndex;
    p += sizeof (sal_Int32);
    
    *p++ = 0xBA;
    *reinterpret_cast< sal_Int32 * >(p) = vtableOffset;
    p += sizeof (sal_Int32);
    
    *p++ = 0xE9;
    *reinterpret_cast< sal_Int32 * >(p)
        = ((unsigned char *) cpp_vtable_call) - p - sizeof (sal_Int32);
    p += sizeof (sal_Int32);
    OSL_ASSERT(p - code <= codeSnippetSize);
    return code + codeSnippetSize;
}

}

struct bridges::cpp_uno::shared::VtableFactory::Slot { void * fn; };

bridges::cpp_uno::shared::VtableFactory::Slot *
bridges::cpp_uno::shared::VtableFactory::mapBlockToVtable(void * block)
{
    return static_cast< Slot * >(block) + 1;
}

sal_Size bridges::cpp_uno::shared::VtableFactory::getBlockSize(
    sal_Int32 slotCount)
{
    return (slotCount + 1) * sizeof (Slot) + slotCount * codeSnippetSize;
}

bridges::cpp_uno::shared::VtableFactory::Slot *
bridges::cpp_uno::shared::VtableFactory::initializeBlock(
    void * block, sal_Int32 slotCount)
{
    struct Rtti {
        sal_Int32 n0, n1, n2;
        type_info * rtti;
        Rtti():
            n0(0), n1(0), n2(0),
            rtti(CPPU_CURRENT_NAMESPACE::msci_getRTTI(
                     OUString(
                                       "com.sun.star.uno.XInterface")))
        {}
    };
    static Rtti rtti;

    Slot * slots = mapBlockToVtable(block);
    slots[-1].fn = &rtti;
    return slots + slotCount;
}

unsigned char * bridges::cpp_uno::shared::VtableFactory::addLocalFunctions(
    Slot ** slots, unsigned char * code,
    typelib_InterfaceTypeDescription const *, sal_Int32 functionOffset,
    sal_Int32 functionCount, sal_Int32 vtableOffset)
{
    (*slots) -= functionCount;
    Slot * s = *slots;
    for (sal_Int32 i = 0; i < functionCount; ++i) {
        (s++)->fn = code;
        code = codeSnippet(code, functionOffset++, vtableOffset);
    }
    return code;
}

void bridges::cpp_uno::shared::VtableFactory::flushCode(
    unsigned char const *, unsigned char const *)
{}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
