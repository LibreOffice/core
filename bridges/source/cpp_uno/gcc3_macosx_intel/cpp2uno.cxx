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


#include <com/sun/star/uno/genfunc.hxx>
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include <uno/data.h>
#include <typelib/typedescription.hxx>

#include "bridges/cpp_uno/shared/bridge.hxx"
#include "bridges/cpp_uno/shared/cppinterfaceproxy.hxx"
#include "bridges/cpp_uno/shared/types.hxx"
#include "bridges/cpp_uno/shared/vtablefactory.hxx"

#include "share.hxx"

#include "boost/static_assert.hpp"
#include <stdio.h>

using namespace ::com::sun::star::uno;

namespace
{


void cpp2uno_call(
    bridges::cpp_uno::shared::CppInterfaceProxy * pThis,
    const typelib_TypeDescription * pMemberTypeDescr,
    typelib_TypeDescriptionReference * pReturnTypeRef, 
    sal_Int32 nParams, typelib_MethodParameter * pParams,
    void ** pCallStack,
    void * pReturnValue )
{
    
    char * pCppStack = (char *)(pCallStack +1);

    
    typelib_TypeDescription * pReturnTypeDescr = 0;
    if (pReturnTypeRef)
        TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );

    void * pUnoReturn = 0;
    void * pCppReturn = 0; 

    if (pReturnTypeDescr)
    {
        
        if (CPPU_CURRENT_NAMESPACE::isSimpleReturnType( pReturnTypeDescr ))
        {
            pUnoReturn = pReturnValue; 
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
    
    pCppStack += sizeof( void* );

    
    OSL_ENSURE( sizeof(void *) == sizeof(sal_Int32), "### unexpected size!" );
    
    void ** pUnoArgs = (void **)alloca( 4 * sizeof(void *) * nParams );
    void ** pCppArgs = pUnoArgs + nParams;
    
    sal_Int32 * pTempIndices = (sal_Int32 *)(pUnoArgs + (2 * nParams));
    
    typelib_TypeDescription ** ppTempParamTypeDescr = (typelib_TypeDescription **)(pUnoArgs + (3 * nParams));

    sal_Int32 nTempIndices   = 0;

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
                uno_copyAndConvertData( pUnoArgs[nPos] = alloca( pParamTypeDescr->nSize ),
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
        
        for ( ; nTempIndices--; )
        {
            sal_Int32 nIndex = pTempIndices[nTempIndices];

            if (pParams[nIndex].bIn) 
                uno_destructData( pUnoArgs[nIndex], ppTempParamTypeDescr[nTempIndices], 0 );
            TYPELIB_DANGER_RELEASE( ppTempParamTypeDescr[nTempIndices] );
        }
        if (pReturnTypeDescr)
            TYPELIB_DANGER_RELEASE( pReturnTypeDescr );

        CPPU_CURRENT_NAMESPACE::raiseException(
            &aUnoExc, pThis->getBridge()->getUno2Cpp() );
            
    }
    else 
    {
        
        for ( ; nTempIndices--; )
        {
            sal_Int32 nIndex = pTempIndices[nTempIndices];
            typelib_TypeDescription * pParamTypeDescr = ppTempParamTypeDescr[nTempIndices];

            if (pParams[nIndex].bOut) 
            {
                
                uno_destructData( pCppArgs[nIndex], pParamTypeDescr, cpp_release );
                uno_copyAndConvertData( pCppArgs[nIndex], pUnoArgs[nIndex], pParamTypeDescr,
                                        pThis->getBridge()->getUno2Cpp() );
            }
            
            uno_destructData( pUnoArgs[nIndex], pParamTypeDescr, 0 );

            TYPELIB_DANGER_RELEASE( pParamTypeDescr );
        }
        
        if (pCppReturn) 
        {
            if (pUnoReturn != pCppReturn) 
            {
                uno_copyAndConvertData( pCppReturn, pUnoReturn, pReturnTypeDescr,
                                        pThis->getBridge()->getUno2Cpp() );
                
                uno_destructData( pUnoReturn, pReturnTypeDescr, 0 );
            }
            if (pReturnValue != pCppReturn) {
                
                
                *static_cast< void ** >(pReturnValue) = pCppReturn;
            }
        }
        if (pReturnTypeDescr)
        {
            TYPELIB_DANGER_RELEASE( pReturnTypeDescr );
        }
    }
}



extern "C" void cpp_vtable_call(
    int nFunctionIndex, int nVtableOffset, void** pCallStack,
    void * pReturnValue )
{
    OSL_ENSURE( sizeof(sal_Int32)==sizeof(void *), "### unexpected!" );

    
    void * pThis;
    if( nFunctionIndex & 0x80000000 )
    {
        nFunctionIndex &= 0x7fffffff;
        pThis = pCallStack[2];
    }
    else
    {
        pThis = pCallStack[1];
    }
    pThis = static_cast< char * >(pThis) - nVtableOffset;
    bridges::cpp_uno::shared::CppInterfaceProxy * pCppI
        = bridges::cpp_uno::shared::CppInterfaceProxy::castInterfaceToProxy(
            pThis);
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "%p %p %p pThis=%p, pCppI=%p, function index=%d, vtable offset=%d\n", pCallStack[0], pCallStack[1], pCallStack[2], pThis, pCppI, nFunctionIndex, nVtableOffset );
#endif
    typelib_InterfaceTypeDescription * pTypeDescr = pCppI->getTypeDescr();
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "name=%s\n", OUStringToOString(pTypeDescr->aBase.pTypeName, RTL_TEXTENCODING_UTF8).getStr() );
#endif
    OSL_ENSURE( nFunctionIndex < pTypeDescr->nMapFunctionIndexToMemberIndex, "### illegal vtable index!" );
    if (nFunctionIndex >= pTypeDescr->nMapFunctionIndexToMemberIndex)
    {
        throw RuntimeException(
            OUString( "illegal vtable index!" ),
            (XInterface *)pThis );
    }

    
    sal_Int32 nMemberPos = pTypeDescr->pMapFunctionIndexToMemberIndex[nFunctionIndex];
    OSL_ENSURE( nMemberPos < pTypeDescr->nAllMembers, "### illegal member index!" );

    TypeDescription aMemberDescr( pTypeDescr->ppAllMembers[nMemberPos] );
#if OSL_DEBUG_LEVEL > 1
    fprintf(stderr, "calling %s\n", OUStringToOString(aMemberDescr.get()->pTypeName, RTL_TEXTENCODING_UTF8).getStr());
#endif
    switch (aMemberDescr.get()->eTypeClass)
    {
    case typelib_TypeClass_INTERFACE_ATTRIBUTE:
    {
        if (pTypeDescr->pMapMemberIndexToFunctionIndex[nMemberPos] == nFunctionIndex)
        {
            
            cpp2uno_call(
                pCppI, aMemberDescr.get(),
                ((typelib_InterfaceAttributeTypeDescription *)aMemberDescr.get())->pAttributeTypeRef,
                0, 0, 
                pCallStack, pReturnValue );
        }
        else
        {
            
            typelib_MethodParameter aParam;
            aParam.pTypeRef =
                ((typelib_InterfaceAttributeTypeDescription *)aMemberDescr.get())->pAttributeTypeRef;
            aParam.bIn      = sal_True;
            aParam.bOut     = sal_False;

            cpp2uno_call(
                pCppI, aMemberDescr.get(),
                0, 
                1, &aParam,
                pCallStack, pReturnValue );
        }
        break;
    }
    case typelib_TypeClass_INTERFACE_METHOD:
    {
        
        switch (nFunctionIndex)
        {
        case 1: 
            pCppI->acquireProxy(); 
            break;
        case 2: 
            pCppI->releaseProxy(); 
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
                        reinterpret_cast< uno_Any * >( pCallStack[1] ),
                        &pInterface, pTD, cpp_acquire );
                    pInterface->release();
                    TYPELIB_DANGER_RELEASE( pTD );
                    *static_cast< void ** >(pReturnValue) = pCallStack[1];
                    break;
                }
                TYPELIB_DANGER_RELEASE( pTD );
            }
        } 
        default:
            cpp2uno_call(
                pCppI, aMemberDescr.get(),
                ((typelib_InterfaceMethodTypeDescription *)aMemberDescr.get())->pReturnTypeRef,
                ((typelib_InterfaceMethodTypeDescription *)aMemberDescr.get())->nParams,
                ((typelib_InterfaceMethodTypeDescription *)aMemberDescr.get())->pParams,
                pCallStack, pReturnValue );
        }
        break;
    }
    default:
    {
        throw RuntimeException(
            OUString( "no member description found!" ),
            (XInterface *)pThis );
    }
    }
}


extern "C" void privateSnippetExecutorGeneral();
extern "C" void privateSnippetExecutorVoid();
extern "C" void privateSnippetExecutorHyper();
extern "C" void privateSnippetExecutorFloat();
extern "C" void privateSnippetExecutorDouble();
extern "C" void privateSnippetExecutorClass();
extern "C" typedef void (*PrivateSnippetExecutor)();

int const codeSnippetSize = 16;

unsigned char * codeSnippet(
    const typelib_InterfaceTypeDescription *type,
    const typelib_TypeDescription *member,
    unsigned char * code, sal_Int32 functionIndex, sal_Int32 vtableOffset,
    typelib_TypeDescriptionReference * pReturnTypeRef)
{
    PrivateSnippetExecutor exec;
    if (pReturnTypeRef == 0) {
        exec = privateSnippetExecutorVoid;
    }
    else {
        switch (pReturnTypeRef->eTypeClass) {
        case typelib_TypeClass_VOID:
            exec = privateSnippetExecutorVoid;
            break;
        case typelib_TypeClass_HYPER:
        case typelib_TypeClass_UNSIGNED_HYPER:
            exec = privateSnippetExecutorHyper;
            break;
        case typelib_TypeClass_FLOAT:
            exec = privateSnippetExecutorFloat;
            break;
        case typelib_TypeClass_DOUBLE:
            exec = privateSnippetExecutorDouble;
            break;
        case typelib_TypeClass_STRUCT:
        case typelib_TypeClass_EXCEPTION: {
            typelib_TypeDescription * pReturnTypeDescr = 0;
            TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );
            bool const bSimpleReturnStruct =
                CPPU_CURRENT_NAMESPACE::isSimpleReturnType(pReturnTypeDescr);
            sal_Int32 const nRetSize = pReturnTypeDescr->nSize;
            TYPELIB_DANGER_RELEASE( pReturnTypeDescr );
            if (bSimpleReturnStruct && nRetSize <= 8) {
                exec = privateSnippetExecutorGeneral; 
                if (nRetSize > 4)
                    exec = privateSnippetExecutorHyper; 
                break;
            }
        }
        case typelib_TypeClass_STRING:
        case typelib_TypeClass_TYPE:
        case typelib_TypeClass_SEQUENCE:
        case typelib_TypeClass_INTERFACE:
        case typelib_TypeClass_ANY:
            functionIndex |= 0x80000000;
            exec = privateSnippetExecutorClass;
            break;
        default:
            exec = privateSnippetExecutorGeneral;
            break;
        }
    }
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
        = ((unsigned char *) exec) - p - sizeof (sal_Int32);
    p += sizeof (sal_Int32);
    OSL_ASSERT(p - code <= codeSnippetSize);

    SAL_INFO( "bridges.osx", "codeSnippet("
              << OUString(type->aBase.pTypeName) << "::" << OUString(member->pTypeName) << "): ["
              << (functionIndex & ~0x80000000) << ((functionIndex & 0x80000000) ? "|0x80000000":"") << "," << vtableOffset << ","
              << (exec == privateSnippetExecutorGeneral ? "General" :
                  (exec == privateSnippetExecutorVoid ? "Void" :
                   (exec == privateSnippetExecutorHyper ? "Hyper" :
                    (exec == privateSnippetExecutorFloat ? "Float" :
                     (exec == privateSnippetExecutorDouble ? "Double" :
                      (exec == privateSnippetExecutorClass ? "Class" :
                       "???"))))))
              << "]=" << (void *)(code + codeSnippetSize) );

    return code + codeSnippetSize;
}

}

struct bridges::cpp_uno::shared::VtableFactory::Slot { void * fn; };

bridges::cpp_uno::shared::VtableFactory::Slot *
bridges::cpp_uno::shared::VtableFactory::mapBlockToVtable(void * block)
{
    return static_cast< Slot * >(block) + 2;
}

sal_Size bridges::cpp_uno::shared::VtableFactory::getBlockSize(
    sal_Int32 slotCount)
{
    return (slotCount + 2) * sizeof (Slot) + slotCount * codeSnippetSize;
}

bridges::cpp_uno::shared::VtableFactory::Slot *
bridges::cpp_uno::shared::VtableFactory::initializeBlock(
    void * block, sal_Int32 slotCount)
{
    Slot * slots = mapBlockToVtable(block);
    slots[-2].fn = 0;
    slots[-1].fn = 0;
    return slots + slotCount;
}

unsigned char * bridges::cpp_uno::shared::VtableFactory::addLocalFunctions(
    Slot ** slots, unsigned char * code,
    typelib_InterfaceTypeDescription const * type, sal_Int32 functionOffset,
    sal_Int32 functionCount, sal_Int32 vtableOffset)
{
    (*slots) -= functionCount;
    Slot * s = *slots;
    for (sal_Int32 i = 0; i < type->nMembers; ++i) {
        typelib_TypeDescription * member = 0;
        TYPELIB_DANGER_GET(&member, type->ppMembers[i]);
        OSL_ASSERT(member != 0);
        switch (member->eTypeClass) {
        case typelib_TypeClass_INTERFACE_ATTRIBUTE:
            
            (s++)->fn = code;
            code = codeSnippet(
                type, member,
                code, functionOffset++, vtableOffset,
                reinterpret_cast< typelib_InterfaceAttributeTypeDescription * >(
                    member)->pAttributeTypeRef);
            
            if (!reinterpret_cast<
                typelib_InterfaceAttributeTypeDescription * >(
                    member)->bReadOnly)
            {
                (s++)->fn = code;
                code = codeSnippet(
                    type, member,
                    code, functionOffset++, vtableOffset,
                    0 /* indicates VOID */);
            }
            break;

        case typelib_TypeClass_INTERFACE_METHOD:
            (s++)->fn = code;
            code = codeSnippet(
                type, member,
                code, functionOffset++, vtableOffset,
                reinterpret_cast< typelib_InterfaceMethodTypeDescription * >(
                    member)->pReturnTypeRef);
            break;

        default:
            OSL_ASSERT(false);
            break;
        }
        TYPELIB_DANGER_RELEASE(member);
    }
    return code;
}

void bridges::cpp_uno::shared::VtableFactory::flushCode(
    unsigned char const *, unsigned char const *)
{}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
