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
#include <typelib/typedescription.hxx>
#include <uno/data.h>
#include "bridges/cpp_uno/shared/bridge.hxx"
#include "bridges/cpp_uno/shared/cppinterfaceproxy.hxx"
#include "bridges/cpp_uno/shared/types.hxx"
#include "bridges/cpp_uno/shared/vtablefactory.hxx"
#include "share.hxx"

using namespace com::sun::star::uno;

namespace
{

static typelib_TypeClass cpp2uno_call(
     bridges::cpp_uno::shared::CppInterfaceProxy * pThis,
    const typelib_TypeDescription * pMemberTypeDescr,
    typelib_TypeDescriptionReference * pReturnTypeRef, 
    sal_Int32 nParams, typelib_MethodParameter * pParams,
    void ** pCallStack,
    sal_Int64 * pRegisterReturn /* space for register return */ )
{
    
    char * pCppStack = (char *)pCallStack;

    
    typelib_TypeDescription * pReturnTypeDescr = 0;
    if (pReturnTypeRef)
        TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );

    void * pUnoReturn = 0;
    void * pCppReturn = 0; 

    if (pReturnTypeDescr)
    {
        if (bridges::cpp_uno::shared::isSimpleType( pReturnTypeDescr ))
            pUnoReturn = pRegisterReturn; 
        else 
        {
            pCppReturn = *(void**)pCppStack;
            pUnoReturn = (bridges::cpp_uno::shared::relatesToInterfaceType(
                                   pReturnTypeDescr )
                          ? alloca( pReturnTypeDescr->nSize )
                          : pCppReturn); 
            pCppStack += sizeof( void* );
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

         if (!rParam.bOut && bridges::cpp_uno::shared::isSimpleType( pParamTypeDescr ))  
        {
            pCppArgs[nPos] = pUnoArgs[nPos] = CPPU_CURRENT_NAMESPACE::adjustPointer(pCppStack, pParamTypeDescr);
            switch (pParamTypeDescr->eTypeClass)
            {
            case typelib_TypeClass_HYPER:
            case typelib_TypeClass_UNSIGNED_HYPER:
                    case typelib_TypeClass_DOUBLE:
                    {
            if ((reinterpret_cast< long >(pCppStack) & 7) != 0)
                      {
                   OSL_ASSERT( sizeof (double) == sizeof (sal_Int64) );
                           void * pDest = alloca( sizeof (sal_Int64) );
                           *reinterpret_cast< sal_Int32 * >(pDest) =
                           *reinterpret_cast< sal_Int32 const * >(pCppStack);
                           *(reinterpret_cast< sal_Int32 * >(pDest) + 1) =
                           *(reinterpret_cast< sal_Int32 const * >(pCppStack) + 1);
                           pCppArgs[nPos] = pUnoArgs[nPos] = pDest;
            }
               pCppStack += sizeof (sal_Int32); 
                       break;
            default:
            break;
            }
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

    
    (*pThis->getUnoI()->pDispatcher)(pThis->getUnoI(), pMemberTypeDescr, pUnoReturn, pUnoArgs, &pUnoExc );

    
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
        CPPU_CURRENT_NAMESPACE::raiseException(&aUnoExc, pThis->getBridge()->getUno2Cpp() );
                 
        
        return typelib_TypeClass_VOID;
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



static typelib_TypeClass cpp_mediate(
    sal_Int32   nFunctionIndex,
    sal_Int32   nVtableOffset,
    void ** pCallStack,
    sal_Int64 * pRegisterReturn /* space for register return */ )
{
    OSL_ENSURE( sizeof(sal_Int32)==sizeof(void *), "### unexpected!" );

    
    
    
    
    
    
    bridges::cpp_uno::shared::CppInterfaceProxy * pCppI
        = bridges::cpp_uno::shared::CppInterfaceProxy::castInterfaceToProxy(
    static_cast< char * >(*pCallStack) - nVtableOffset);
    if ((nFunctionIndex & 0x80000000) != 0) {
        nFunctionIndex &= 0x7FFFFFFF;
        --pCallStack;
    }

    typelib_InterfaceTypeDescription * pTypeDescr = pCppI->getTypeDescr();

    OSL_ENSURE( nFunctionIndex < pTypeDescr->nMapFunctionIndexToMemberIndex,
                 "### illegal vtable index!" );
    if (nFunctionIndex >= pTypeDescr->nMapFunctionIndexToMemberIndex)
    {
        throw RuntimeException( "illegal vtable index!", (XInterface *)pCppI );
    }

    
    sal_Int32 nMemberPos = pTypeDescr->pMapFunctionIndexToMemberIndex[nFunctionIndex];
    OSL_ENSURE( nMemberPos < pTypeDescr->nAllMembers, "### illegal member index!" );

    TypeDescription aMemberDescr( pTypeDescr->ppAllMembers[nMemberPos] );

#if defined BRIDGES_DEBUG
    OString cstr( OUStringToOString( aMemberDescr.get()->pTypeName, RTL_TEXTENCODING_ASCII_US ) );
    fprintf( stderr, "calling %s, nFunctionIndex=%d\n", cstr.getStr(), nFunctionIndex );
#endif

    typelib_TypeClass eRet;
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
            TYPELIB_DANGER_GET( &pTD, reinterpret_cast< Type * >( pCallStack[2] )->getTypeLibType() );
            if (pTD)
            {
                XInterface * pInterface = 0;
        (*pCppI->getBridge()->getCppEnv()->getRegisteredInterface)(
            pCppI->getBridge()->getCppEnv(),
            (void **)&pInterface, pCppI->getOid().pData, (typelib_InterfaceTypeDescription *)pTD );

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
        throw RuntimeException( "no member description found!", (XInterface *)pCppI );
    }
    }
    return eRet;
}




/**
 * is called on incoming vtable calls
 * (called by asm snippets)
 */
static void cpp_vtable_call()
{
    sal_Int64 nRegReturn;
    int nFunctionIndex;
    void** pCallStack;
    int vTableOffset;

void * pRegReturn = &nRegReturn;

    __asm__( "st %%i0, %0\n\t"
            "st %%i1, %1\n\t"
             "st %%i2, %2\n\t"
            : : "m"(nFunctionIndex), "m"(pCallStack), "m"(vTableOffset) );




    sal_Bool bComplex = nFunctionIndex & 0x80000000 ? sal_True : sal_False;
    typelib_TypeClass aType =
        cpp_mediate( nFunctionIndex, vTableOffset, pCallStack+17, (sal_Int64*)&nRegReturn );

    switch( aType )
    {
        case typelib_TypeClass_BOOLEAN:
        case typelib_TypeClass_BYTE:
            __asm__( "ld %0, %%l0\n\t"
                     "ldsb [%%l0], %%i0\n"
                     : : "m"(pRegReturn) );
            break;
        case typelib_TypeClass_CHAR:
        case typelib_TypeClass_SHORT:
        case typelib_TypeClass_UNSIGNED_SHORT:
            __asm__( "ld %0, %%l0\n\t"
                     "ldsh [%%l0], %%i0\n"
                     : : "m"(pRegReturn) );
            break;
        case typelib_TypeClass_HYPER:
        case typelib_TypeClass_UNSIGNED_HYPER:
            __asm__( "ld %0, %%l0\n\t"
                     "ld [%%l0], %%i0\n\t"
                     "add %%l0, 4, %%l0\n\t"
                     "ld [%%l0], %%i1\n\t"
                      : : "m"(pRegReturn) );

            break;
        case typelib_TypeClass_FLOAT:
            __asm__( "ld %0, %%l0\n\t"
                     "ld [%%l0], %%f0\n"
                     : : "m"(pRegReturn) );
            break;
        case typelib_TypeClass_DOUBLE:
            __asm__( "ld %0, %%l0\n\t"
                     "ldd [%%l0], %%f0\n"
                     : : "m"(pRegReturn) );
            break;
        case typelib_TypeClass_VOID:
            break;
        default:
            __asm__( "ld %0, %%l0\n\t"
                     "ld [%%l0], %%i0\n"
                     : : "m"(pRegReturn) );
            break;
    }

    if( bComplex )
    {
        __asm__( "add %i7, 4, %i7\n\t" );
        
    }

}


int const codeSnippetSize = 56;
unsigned char * codeSnippet(
    unsigned char * code, sal_Int32 functionIndex, sal_Int32 vtableOffset,
    bool simpleRetType)
{
    sal_uInt32 index = functionIndex;
    if (!simpleRetType) {
        index |= 0x80000000;
    }
    unsigned int * p = reinterpret_cast< unsigned int * >(code);
    OSL_ASSERT(sizeof (unsigned int) == 4);
    
    *p++ = 0xD023A044;
    
    *p++ = 0xD223A048;
    
    *p++ = 0xD423A04C;
    
    *p++ = 0xD623A050;
    
    *p++ = 0xD823A054;
    
    *p++ = 0xDA23A058;
    
    *p++ = 0x11000000 | (index >> 10);
    
    *p++ = 0x90122000 | (index & 0x3FF);
    
    *p++ = 0x15000000 | (vtableOffset >> 10);
    
    *p++ = 0x9412A000 | (vtableOffset & 0x3FF);
    
    *p++ = 0x17000000 | (reinterpret_cast< unsigned int >(cpp_vtable_call) >> 10);
    
    *p++ = 0x9612E000 | (reinterpret_cast< unsigned int >(cpp_vtable_call) & 0x3FF);
    
    *p++ = 0x81C2C000;
    
    *p++ = 0x9210000E;
    OSL_ASSERT(
        reinterpret_cast< unsigned char * >(p) - code <= codeSnippetSize);
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
    Slot ** slots, unsigned char * code, sal_PtrDiff writetoexecdiff,
    typelib_InterfaceTypeDescription const * type, sal_Int32 functionOffset,
    sal_Int32 functionCount, sal_Int32 vTableOffset)
{
    (*slots) -= functionCount;
    Slot * s = *slots;
    for (sal_Int32 i = 0; i < type->nMembers; ++i) {
        typelib_TypeDescription * member = 0;
        TYPELIB_DANGER_GET(&member, type->ppMembers[i]);
        OSL_ASSERT(member != 0);
        switch (member->eTypeClass) {
        case typelib_TypeClass_INTERFACE_ATTRIBUTE:
            
            (s++)->fn = code + writetoexecdiff;
            code = codeSnippet(
                code, functionOffset++, vTableOffset,
                bridges::cpp_uno::shared::isSimpleType(
                    reinterpret_cast<
            typelib_InterfaceAttributeTypeDescription * >(
            member)->pAttributeTypeRef));
            
            if (!reinterpret_cast<
                typelib_InterfaceAttributeTypeDescription * >(
                    member)->bReadOnly)
            {
                (s++)->fn = code + writetoexecdiff;
                code = codeSnippet(code, functionOffset++, vTableOffset, true);
            }
            break;

        case typelib_TypeClass_INTERFACE_METHOD:
            (s++)->fn = code + writetoexecdiff;
            code = codeSnippet(
                code, functionOffset++, vTableOffset,
                bridges::cpp_uno::shared::isSimpleType(
                    reinterpret_cast<
                    typelib_InterfaceMethodTypeDescription * >(
                        member)->pReturnTypeRef));
            break;

        default:
            OSL_ASSERT(false);
            break;
        }
        TYPELIB_DANGER_RELEASE(member);
    }
    return code;
}



extern "C" void doFlushCode(unsigned long address, unsigned long count);

void bridges::cpp_uno::shared::VtableFactory::flushCode(
    unsigned char const * begin, unsigned char const * end)
{
    unsigned long n = end - begin;
    if (n != 0) {
        unsigned long adr = reinterpret_cast< unsigned long >(begin);
        unsigned long off = adr & 7;
        doFlushCode(adr - off, (n + off + 7) >> 3);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
