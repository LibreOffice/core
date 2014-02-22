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
#include <boost/unordered_map.hpp>

#include <rtl/alloc.h>
#include <osl/mutex.hxx>

#include <com/sun/star/uno/genfunc.hxx>
#include "com/sun/star/uno/RuntimeException.hpp"
#include <uno/data.h>
#include <typelib/typedescription.hxx>

#include "bridges/cpp_uno/shared/bridge.hxx"
#include "bridges/cpp_uno/shared/cppinterfaceproxy.hxx"
#include "bridges/cpp_uno/shared/types.hxx"
#include "bridges/cpp_uno/shared/vtablefactory.hxx"

#include "share.hxx"

#include <dlfcn.h>

#ifdef ANDROID
#include <unistd.h>
#endif

using namespace ::osl;
using namespace ::rtl;
using namespace ::com::sun::star::uno;

namespace
{

    static typelib_TypeClass cpp2uno_call(
        bridges::cpp_uno::shared::CppInterfaceProxy* pThis,
        const typelib_TypeDescription * pMemberTypeDescr,
        typelib_TypeDescriptionReference * pReturnTypeRef,
        sal_Int32 nParams, typelib_MethodParameter * pParams,
        void ** pCallStack,
        sal_Int64 * pRegisterReturn /* space for register return */ )
    {
        
        char * pTopStack = (char *)(pCallStack + 0);
        char * pCppStack = pTopStack;

#ifdef __ARM_PCS_VFP
        int dc = 0;
        char * pFloatArgs = (char *)(pCppStack - 64);
#endif
        
        typelib_TypeDescription * pReturnTypeDescr = 0;
        if (pReturnTypeRef)
            TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );

        void * pUnoReturn = 0;
        
        void * pCppReturn = 0;

        if (pReturnTypeDescr)
        {
            if (!arm::return_in_hidden_param(pReturnTypeRef))
                pUnoReturn = pRegisterReturn; 
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

        
        OSL_ENSURE( sizeof(void *) == sizeof(sal_Int32),
            "### unexpected size!" );
        
        void ** pUnoArgs = (void **)alloca( 4 * sizeof(void *) * nParams );
        void ** pCppArgs = pUnoArgs + nParams;
        
        
        sal_Int32 * pTempIndices = (sal_Int32 *)(pUnoArgs + (2 * nParams));
        
        typelib_TypeDescription ** ppTempParamTypeDescr =
            (typelib_TypeDescription **)(pUnoArgs + (3 * nParams));

        sal_Int32 nTempIndices   = 0;

        for ( sal_Int32 nPos = 0; nPos < nParams; ++nPos )
        {
            const typelib_MethodParameter & rParam = pParams[nPos];
            typelib_TypeDescription * pParamTypeDescr = 0;
            TYPELIB_DANGER_GET( &pParamTypeDescr, rParam.pTypeRef );

            if (!rParam.bOut &&
                bridges::cpp_uno::shared::isSimpleType( pParamTypeDescr ))
            {
#ifdef __ARM_EABI__
                switch (pParamTypeDescr->eTypeClass)
                {
                    case typelib_TypeClass_HYPER:
                    case typelib_TypeClass_UNSIGNED_HYPER:
#ifndef __ARM_PCS_VFP
                    case typelib_TypeClass_DOUBLE:
#endif
                        if ((pCppStack - pTopStack) % 8) pCppStack+=sizeof(sal_Int32); 
                        break;
                    default:
                        break;
                }
#endif


#ifdef __ARM_PCS_VFP
                if (pParamTypeDescr->eTypeClass == typelib_TypeClass_FLOAT)
                {
                    pCppArgs[nPos] =  pUnoArgs[nPos] = pFloatArgs;
                    pFloatArgs += sizeof(float);
                } else
                if (pParamTypeDescr->eTypeClass == typelib_TypeClass_DOUBLE)
                {
                    if ((pFloatArgs - pTopStack) % 8) pFloatArgs+=sizeof(float); 
                    pCppArgs[nPos] = pUnoArgs[nPos] = pFloatArgs;
                    pFloatArgs += sizeof(double);
                    if (++dc == arm::MAX_FPR_REGS) {
                        if (pCppStack - pTopStack < 16)
                            pCppStack = pTopStack + 16;
                        pFloatArgs = pCppStack;
                    }
                } else
#endif
                    pCppArgs[nPos] = pUnoArgs[nPos] = pCppStack;

                switch (pParamTypeDescr->eTypeClass)
                {
                    case typelib_TypeClass_HYPER:
                    case typelib_TypeClass_UNSIGNED_HYPER:
#ifndef __ARM_PCS_VFP
                    case typelib_TypeClass_DOUBLE:
#endif
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
                    uno_copyAndConvertData( pUnoArgs[nPos] =
                        alloca( pParamTypeDescr->nSize ),
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
#ifdef __ARM_PCS_VFP
            
                if (rParam.bOut ||
                        ((pParamTypeDescr->eTypeClass != typelib_TypeClass_DOUBLE)
                         && (pParamTypeDescr->eTypeClass != typelib_TypeClass_FLOAT))
                    )
#endif
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
                    uno_destructData( pUnoArgs[nIndex],
                        ppTempParamTypeDescr[nTempIndices], 0 );
                TYPELIB_DANGER_RELEASE( ppTempParamTypeDescr[nTempIndices] );
            }
            if (pReturnTypeDescr)
                TYPELIB_DANGER_RELEASE( pReturnTypeDescr );

            CPPU_CURRENT_NAMESPACE::raiseException( &aUnoExc,
                pThis->getBridge()->getUno2Cpp() ); 
            
            return typelib_TypeClass_VOID;
        }
        else 
        {
            
            for ( ; nTempIndices--; )
            {
                sal_Int32 nIndex = pTempIndices[nTempIndices];
                typelib_TypeDescription * pParamTypeDescr =
                    ppTempParamTypeDescr[nTempIndices];

                if (pParams[nIndex].bOut) 
                {
                    
                    uno_destructData( pCppArgs[nIndex], pParamTypeDescr,
                        cpp_release );
                    uno_copyAndConvertData( pCppArgs[nIndex], pUnoArgs[nIndex],
                        pParamTypeDescr, pThis->getBridge()->getUno2Cpp() );
                }
                
                uno_destructData( pUnoArgs[nIndex], pParamTypeDescr, 0 );

                TYPELIB_DANGER_RELEASE( pParamTypeDescr );
            }
            
            if (pCppReturn) 
            {
                if (pUnoReturn != pCppReturn) 
                {
                    uno_copyAndConvertData( pCppReturn, pUnoReturn,
                        pReturnTypeDescr, pThis->getBridge()->getUno2Cpp() );
                    
                    uno_destructData( pUnoReturn, pReturnTypeDescr, 0 );
                }
                
                *(void **)pRegisterReturn = pCppReturn;
            }
            if (pReturnTypeDescr)
            {
                typelib_TypeClass eRet =
                    (typelib_TypeClass)pReturnTypeDescr->eTypeClass;
                TYPELIB_DANGER_RELEASE( pReturnTypeDescr );
                return eRet;
            }
            else
                return typelib_TypeClass_VOID;
        }
    }


    
    static typelib_TypeClass cpp_mediate(
        sal_Int32 nFunctionIndex, sal_Int32 nVtableOffset,
        void ** pCallStack,
        sal_Int64 * pRegisterReturn /* space for register return */ )
    {
        OSL_ENSURE( sizeof(sal_Int32)==sizeof(void *), "### unexpected!" );

        
        
        void *pThis;
        if( nFunctionIndex & 0x80000000 )
        {
            nFunctionIndex &= 0x7fffffff;
            pThis = pCallStack[1];
        }
        else
        {
            pThis = pCallStack[0];
        }

        pThis = static_cast< char * >(pThis) - nVtableOffset;
        bridges::cpp_uno::shared::CppInterfaceProxy * pCppI =
            bridges::cpp_uno::shared::CppInterfaceProxy::castInterfaceToProxy(
                pThis);

        typelib_InterfaceTypeDescription * pTypeDescr = pCppI->getTypeDescr();

        OSL_ENSURE( nFunctionIndex < pTypeDescr->nMapFunctionIndexToMemberIndex,
            "### illegal vtable index!" );
        if (nFunctionIndex >= pTypeDescr->nMapFunctionIndexToMemberIndex)
        {
            throw RuntimeException(
                OUString( "illegal vtable index!" ),
                (XInterface *)pCppI );
        }

        
        OSL_ENSURE( nFunctionIndex < pTypeDescr->nMapFunctionIndexToMemberIndex,
            "### illegal vtable index!" );
        sal_Int32 nMemberPos =
            pTypeDescr->pMapFunctionIndexToMemberIndex[nFunctionIndex];
        OSL_ENSURE( nMemberPos < pTypeDescr->nAllMembers,
            "### illegal member index!" );

        TypeDescription aMemberDescr( pTypeDescr->ppAllMembers[nMemberPos] );

        typelib_TypeClass eRet;
        switch (aMemberDescr.get()->eTypeClass)
        {
        case typelib_TypeClass_INTERFACE_ATTRIBUTE:
        {
            if (pTypeDescr->pMapMemberIndexToFunctionIndex[nMemberPos] ==
                nFunctionIndex)
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
                TYPELIB_DANGER_GET(&pTD,
                    reinterpret_cast<Type *>(pCallStack[2])->getTypeLibType());
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
                            reinterpret_cast< uno_Any * >( pCallStack[0] ),
                            &pInterface, pTD, cpp_acquire );
                        pInterface->release();
                        TYPELIB_DANGER_RELEASE( pTD );
                        *(void **)pRegisterReturn = pCallStack[0];
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
                OUString( "no member description found!" ),
                (XInterface *)pCppI );
        }
        }

        return eRet;
    }
}


/**
 * is called on incoming vtable calls
 * (called by asm snippets)
 */

extern "C" sal_Int64 cpp_vtable_call( long *pFunctionAndOffset,
    void **pCallStack )
{
    sal_Int64 nRegReturn;
    typelib_TypeClass aType = cpp_mediate( pFunctionAndOffset[0], pFunctionAndOffset[1], pCallStack,
        &nRegReturn );

    switch( aType )
    {
        case typelib_TypeClass_BOOLEAN:
        case typelib_TypeClass_BYTE:
            nRegReturn = (unsigned long)(*(unsigned char *)&nRegReturn);
            break;
        case typelib_TypeClass_CHAR:
        case typelib_TypeClass_UNSIGNED_SHORT:
        case typelib_TypeClass_SHORT:
            nRegReturn = (unsigned long)(*(unsigned short *)&nRegReturn);
            break;
        case typelib_TypeClass_ENUM:
        case typelib_TypeClass_UNSIGNED_LONG:
        case typelib_TypeClass_LONG:
            nRegReturn = (unsigned long)(*(unsigned int *)&nRegReturn);
            break;
        case typelib_TypeClass_VOID:
        default:
            break;
    }

    return nRegReturn;
}

extern "C" void privateSnippetExecutor(void);

namespace
{
    const int codeSnippetSize = 20;

    unsigned char *codeSnippet(unsigned char* code, sal_Int32 functionIndex,
        sal_Int32 vtableOffset, bool bHasHiddenParam)
    {
        if (bHasHiddenParam)
            functionIndex |= 0x80000000;

        unsigned long * p = (unsigned long *)code;

        
        
        *p++ = 0xE1A0C00F;
        
        *p++ = 0xE59FF004;
        *p++ = (unsigned long)functionIndex;
        *p++ = (unsigned long)vtableOffset;
        *p++ = (unsigned long)privateSnippetExecutor;

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
#ifdef USE_DOUBLE_MMAP
    sal_PtrDiff writetoexecdiff,
#endif
    typelib_InterfaceTypeDescription const * type, sal_Int32 functionOffset,
    sal_Int32 functionCount, sal_Int32 vtableOffset)
{
#ifndef USE_DOUBLE_MMAP
    const sal_PtrDiff writetoexecdiff = 0;
#endif
    (*slots) -= functionCount;
    Slot * s = *slots;
    for (sal_Int32 i = 0; i < type->nMembers; ++i)
    {
        typelib_TypeDescription * member = 0;
        TYPELIB_DANGER_GET(&member, type->ppMembers[i]);
        OSL_ASSERT(member != 0);
        switch (member->eTypeClass)
        {
            case typelib_TypeClass_INTERFACE_ATTRIBUTE:
            {
                typelib_InterfaceAttributeTypeDescription *pAttrTD =
                    reinterpret_cast<typelib_InterfaceAttributeTypeDescription *>( member );

                
                (s++)->fn = code + writetoexecdiff;
                code = codeSnippet(
                    code, functionOffset++, vtableOffset,
                    arm::return_in_hidden_param( pAttrTD->pAttributeTypeRef ));

                
                if (!pAttrTD->bReadOnly)
                {
                    (s++)->fn = code + writetoexecdiff;
                    code = codeSnippet(
                        code, functionOffset++, vtableOffset, false);
                }
                break;
            }
            case typelib_TypeClass_INTERFACE_METHOD:
            {
                (s++)->fn = code + writetoexecdiff;

                typelib_InterfaceMethodTypeDescription *pMethodTD =
                    reinterpret_cast<
                        typelib_InterfaceMethodTypeDescription * >(member);

                code = codeSnippet(code, functionOffset++, vtableOffset,
                    arm::return_in_hidden_param(pMethodTD->pReturnTypeRef));
                break;
            }
        default:
            OSL_ASSERT(false);
            break;
        }
        TYPELIB_DANGER_RELEASE(member);
    }
    return code;
}

void bridges::cpp_uno::shared::VtableFactory::flushCode(
    unsigned char const *beg, unsigned char const *end)
{
#ifndef ANDROID
   static void (*clear_cache)(unsigned char const*, unsigned char const*)
       = (void (*)(unsigned char const*, unsigned char const*))
           dlsym(RTLD_DEFAULT, "__clear_cache");
   (*clear_cache)(beg, end);
#else
   cacheflush((long) beg, (long) end, 0);
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
