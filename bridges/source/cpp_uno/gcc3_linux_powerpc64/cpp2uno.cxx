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
#include <uno/data.h>
#include <typelib/typedescription.hxx>

#include "bridges/cpp_uno/shared/bridge.hxx"
#include "bridges/cpp_uno/shared/cppinterfaceproxy.hxx"
#include "bridges/cpp_uno/shared/types.hxx"
#include "bridges/cpp_uno/shared/vtablefactory.hxx"

#include "share.hxx"
#include <stdio.h>
#include <string.h>


using namespace ::com::sun::star::uno;

namespace
{


static typelib_TypeClass cpp2uno_call(
    bridges::cpp_uno::shared::CppInterfaceProxy * pThis,
    const typelib_TypeDescription * pMemberTypeDescr,
    typelib_TypeDescriptionReference * pReturnTypeRef, 
    sal_Int32 nParams, typelib_MethodParameter * pParams,
        void ** gpreg, void ** fpreg, void ** ovrflw,
    sal_Int64 * pRegisterReturn /* space for register return */ )
{
#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr, "as far as cpp2uno_call\n");
#endif

    int ng = 0; 
    int nf = 0; 

    
    
    

    
    typelib_TypeDescription * pReturnTypeDescr = 0;
    if (pReturnTypeRef)
        TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );

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
            pCppReturn = *(void **)gpreg;
            gpreg++;
            ng++;

            pUnoReturn = (bridges::cpp_uno::shared::relatesToInterfaceType( pReturnTypeDescr )
                          ? alloca( pReturnTypeDescr->nSize )
                          : pCppReturn); 
        }
    }
    
    gpreg++;
    ng++;

    
    OSL_ENSURE( sizeof(void *) == sizeof(sal_Int64), "### unexpected size!" );
    
    void ** pUnoArgs = (void **)alloca( 4 * sizeof(void *) * nParams );
    void ** pCppArgs = pUnoArgs + nParams;
    
    sal_Int32 * pTempIndices = (sal_Int32 *)(pUnoArgs + (2 * nParams));
    
    typelib_TypeDescription ** ppTempParamTypeDescr = (typelib_TypeDescription **)(pUnoArgs + (3 * nParams));

    sal_Int32 nTempIndices = 0;
    bool bOverFlowUsed = false;
    for ( sal_Int32 nPos = 0; nPos < nParams; ++nPos )
    {
        const typelib_MethodParameter & rParam = pParams[nPos];
        typelib_TypeDescription * pParamTypeDescr = 0;
        TYPELIB_DANGER_GET( &pParamTypeDescr, rParam.pTypeRef );

#if OSL_DEBUG_LEVEL > 2
        fprintf(stderr, "arg %d of %d\n", nPos, nParams);
#endif

        if (!rParam.bOut && bridges::cpp_uno::shared::isSimpleType( pParamTypeDescr ))
        {
#if OSL_DEBUG_LEVEL > 2
            fprintf(stderr, "simple\n");
#endif

            switch (pParamTypeDescr->eTypeClass)
            {
                case typelib_TypeClass_FLOAT:
                case typelib_TypeClass_DOUBLE:
                    if (nf < ppc64::MAX_SSE_REGS)
                    {
                        if (pParamTypeDescr->eTypeClass == typelib_TypeClass_FLOAT)
                        {
                            float tmp = (float) (*((double *)fpreg));
                            (*((float *) fpreg)) = tmp;
                        }
                        pCppArgs[nPos] = pUnoArgs[nPos] = fpreg++;
                        nf++;
                    }
                    else
                    {
                        pCppArgs[nPos] = pUnoArgs[nPos] = ovrflw;
                        bOverFlowUsed = true;
                    }
                    if (bOverFlowUsed) ovrflw++;
                    break;
                case typelib_TypeClass_BYTE:
                case typelib_TypeClass_BOOLEAN:
                    if (ng < ppc64::MAX_GPR_REGS)
                    {
                        pCppArgs[nPos] = pUnoArgs[nPos] = (((char *)gpreg) + (sizeof(void*)-1));
                        ng++;
                        gpreg++;
                    }
                    else
                    {
                        pCppArgs[nPos] = pUnoArgs[nPos] = (((char *)ovrflw) + (sizeof(void*)-1));
                        bOverFlowUsed = true;
                    }
                    if (bOverFlowUsed) ovrflw++;
                    break;
                case typelib_TypeClass_CHAR:
                case typelib_TypeClass_SHORT:
                case typelib_TypeClass_UNSIGNED_SHORT:
                    if (ng < ppc64::MAX_GPR_REGS)
                    {
                        pCppArgs[nPos] = pUnoArgs[nPos] = (((char *)gpreg) + (sizeof(void*)-2));
                        ng++;
                        gpreg++;
                    }
                    else
                    {
                        pCppArgs[nPos] = pUnoArgs[nPos] = (((char *)ovrflw) + (sizeof(void*)-2));
                        bOverFlowUsed = true;
                    }
                    if (bOverFlowUsed) ovrflw++;
                    break;
        case typelib_TypeClass_ENUM:
                case typelib_TypeClass_LONG:
                case typelib_TypeClass_UNSIGNED_LONG:
                    if (ng < ppc64::MAX_GPR_REGS)
                    {
                        pCppArgs[nPos] = pUnoArgs[nPos] = (((char *)gpreg) + (sizeof(void*)-4));
                        ng++;
                        gpreg++;
                    }
                    else
                    {
                        pCppArgs[nPos] = pUnoArgs[nPos] = (((char *)ovrflw) + (sizeof(void*)-4));
                        bOverFlowUsed = true;
                    }
                    if (bOverFlowUsed) ovrflw++;
                    break;
                default:
                    if (ng < ppc64::MAX_GPR_REGS)
                    {
                        pCppArgs[nPos] = pUnoArgs[nPos] = gpreg++;
                        ng++;
                    }
                    else
                    {
                        pCppArgs[nPos] = pUnoArgs[nPos] = ovrflw;
                        bOverFlowUsed = true;
                    }
                    if (bOverFlowUsed) ovrflw++;
                    break;
                }

                
                TYPELIB_DANGER_RELEASE( pParamTypeDescr );
        }
        else 
        {
#if OSL_DEBUG_LEVEL > 2
            fprintf(stderr, "complex, ng is %d\n", ng);
#endif
            void *pCppStack; 

            if (ng < ppc64::MAX_GPR_REGS)
            {
                pCppArgs[nPos] = pCppStack = *gpreg++;
                ng++;
            }
            else
            {
                pCppArgs[nPos] = pCppStack = *ovrflw;
                bOverFlowUsed = true;
            }
            if (bOverFlowUsed) ovrflw++;

            if (! rParam.bIn) 
            {
                
                pUnoArgs[nPos] = alloca( pParamTypeDescr->nSize );
                pTempIndices[nTempIndices] = nPos;
                
                ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
            }
            
            else if (bridges::cpp_uno::shared::relatesToInterfaceType( pParamTypeDescr ))
            {
                uno_copyAndConvertData( pUnoArgs[nPos] = alloca( pParamTypeDescr->nSize ),
                                        pCppStack, pParamTypeDescr,
                                        pThis->getBridge()->getCpp2Uno() );
                pTempIndices[nTempIndices] = nPos; 
                
                ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
            }
            else 
            {
                pUnoArgs[nPos] = pCppStack;
                
                TYPELIB_DANGER_RELEASE( pParamTypeDescr );
            }
        }
    }

#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr, "end of params\n");
#endif

    
    uno_Any aUnoExc; 
    uno_Any * pUnoExc = &aUnoExc;

    
    (*pThis->getUnoI()->pDispatcher)( pThis->getUnoI(), pMemberTypeDescr, pUnoReturn, pUnoArgs, &pUnoExc );

    
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

        CPPU_CURRENT_NAMESPACE::raiseException( &aUnoExc, pThis->getBridge()->getUno2Cpp() );
                
        
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
    sal_uInt64 nOffsetAndIndex,
        void ** gpreg, void ** fpreg, long sp,
    sal_Int64 * pRegisterReturn /* space for register return */ )
{
    OSL_ENSURE( sizeof(sal_Int64)==sizeof(void *), "### unexpected!" );

    sal_Int32 nVtableOffset = (nOffsetAndIndex >> 32);
    sal_Int32 nFunctionIndex = (nOffsetAndIndex & 0xFFFFFFFF);

    long sf = *(long*)sp;
    void ** ovrflw = (void**)(sf + 112);

    
    
    

    void * pThis;
    if (nFunctionIndex & 0x80000000 )
    {
    nFunctionIndex &= 0x7fffffff;
    pThis = gpreg[1];
#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr, "pThis is gpreg[1]\n");
#endif
    }
    else
    {
    pThis = gpreg[0];
#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr, "pThis is gpreg[0]\n");
#endif
    }

#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr, "pThis is %lx\n", pThis);
#endif

    pThis = static_cast< char * >(pThis) - nVtableOffset;

#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr, "pThis is now %lx\n", pThis);
#endif

    bridges::cpp_uno::shared::CppInterfaceProxy * pCppI
        = bridges::cpp_uno::shared::CppInterfaceProxy::castInterfaceToProxy(
            pThis);

    typelib_InterfaceTypeDescription * pTypeDescr = pCppI->getTypeDescr();

#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr, "indexes are %d %d\n", nFunctionIndex, pTypeDescr->nMapFunctionIndexToMemberIndex);
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

#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr, "members are %d %d\n", nMemberPos, pTypeDescr->nAllMembers);
#endif

    TypeDescription aMemberDescr( pTypeDescr->ppAllMembers[nMemberPos] );

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
                gpreg, fpreg, ovrflw, pRegisterReturn );
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
                gpreg, fpreg, ovrflw, pRegisterReturn );
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
            TYPELIB_DANGER_GET( &pTD, reinterpret_cast< Type * >( gpreg[2] )->getTypeLibType() );
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
                        reinterpret_cast< uno_Any * >( gpreg[0] ),
                        &pInterface, pTD, cpp_acquire );
                    pInterface->release();
                    TYPELIB_DANGER_RELEASE( pTD );
                    *(void **)pRegisterReturn = gpreg[0];
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
                gpreg, fpreg, ovrflw, pRegisterReturn );
        }
        break;
    }
    default:
    {
#if OSL_DEBUG_LEVEL > 2
        fprintf(stderr, "screwed\n");
#endif

        throw RuntimeException(
            OUString( "no member description found!" ),
            (XInterface *)pThis );
    }
    }

#if OSL_DEBUG_LEVEL > 2
        fprintf(stderr, "end of cpp_mediate\n");
#endif
    return eRet;
}

extern "C" void privateSnippetExecutor( ... )
{
    sal_uInt64 gpreg[ppc64::MAX_GPR_REGS];
    double fpreg[ppc64::MAX_SSE_REGS];

    __asm__ __volatile__ (
        "std 3,   0(%0)\t\n"
        "std 4,   8(%0)\t\n"
        "std 5,  16(%0)\t\n"
        "std 6,  24(%0)\t\n"
        "std 7,  32(%0)\t\n"
        "std 8,  40(%0)\t\n"
        "std 9,  48(%0)\t\n"
        "std 10, 56(%0)\t\n"
        "stfd 1,   0(%1)\t\n"
        "stfd 2,   8(%1)\t\n"
        "stfd 3,  16(%1)\t\n"
        "stfd 4,  24(%1)\t\n"
        "stfd 5,  32(%1)\t\n"
        "stfd 6,  40(%1)\t\n"
        "stfd 7,  48(%1)\t\n"
        "stfd 8,  56(%1)\t\n"
        "stfd 9,  64(%1)\t\n"
        "stfd 10, 72(%1)\t\n"
        "stfd 11, 80(%1)\t\n"
        "stfd 12, 88(%1)\t\n"
        "stfd 13, 96(%1)\t\n"
    : : "r" (gpreg), "r" (fpreg)
        : "r0", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11",
          "fr1", "fr2", "fr3", "fr4", "fr5", "fr6", "fr7", "fr8", "fr9",
          "fr10", "fr11", "fr12", "fr13"
    );

    volatile long nOffsetAndIndex;

    
    __asm__ __volatile__ (
                "mr     %0,    11\n\t"
                : "=r" (nOffsetAndIndex) : );

    volatile long sp;

    
    __asm__ __volatile__ (
                "mr     %0,    1\n\t"
                : "=r" (sp) : );

    volatile long nRegReturn[1];

    typelib_TypeClass aType =
        cpp_mediate( nOffsetAndIndex, (void**)gpreg, (void**)fpreg, sp, (sal_Int64*)nRegReturn);

    switch( aType )
    {
        case typelib_TypeClass_VOID:
        break;
        case typelib_TypeClass_BOOLEAN:
        case typelib_TypeClass_BYTE:
            __asm__( "lbz 3,%0\n\t"
                : : "m" (nRegReturn[0]) );
            break;
        case typelib_TypeClass_CHAR:
        case typelib_TypeClass_UNSIGNED_SHORT:
            __asm__( "lhz 3,%0\n\t"
                : : "m" (nRegReturn[0]) );
            break;
        case typelib_TypeClass_SHORT:
            __asm__( "lha 3,%0\n\t"
                : : "m" (nRegReturn[0]) );
            break;
        case typelib_TypeClass_ENUM:
        case typelib_TypeClass_UNSIGNED_LONG:
            __asm__( "lwz 3,%0\n\t"
                : : "m"(nRegReturn[0]) );
            break;
        case typelib_TypeClass_LONG:
            __asm__( "lwa 3,%0\n\t"
                : : "m"(nRegReturn[0]) );
            break;
        case typelib_TypeClass_FLOAT:
            __asm__( "lfs 1,%0\n\t"
                : : "m" (*((float*)nRegReturn)) );
            break;
        case typelib_TypeClass_DOUBLE:
            __asm__( "lfd 1,%0\n\t"
                : : "m" (*((double*)nRegReturn)) );
            break;
        default:
            __asm__( "ld 3,%0\n\t"
                : : "m" (nRegReturn[0]) );
            break;
    }
}

const int codeSnippetSize = 24;

unsigned char *  codeSnippet( unsigned char * code, sal_Int32 nFunctionIndex, sal_Int32 nVtableOffset,
                              bool simpleRetType)
{
#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr,"in codeSnippet functionIndex is %x\n", nFunctionIndex);
    fprintf(stderr,"in codeSnippet vtableOffset is %x\n", nVtableOffset);
#endif

    sal_uInt64 nOffsetAndIndex = ( ( (sal_uInt64) nVtableOffset ) << 32 ) | ( (sal_uInt64) nFunctionIndex );

    if ( !simpleRetType )
        nOffsetAndIndex |= 0x80000000;

    void ** raw = (void **)&code[0];
    memcpy(raw, (char*) privateSnippetExecutor, 16);
    raw[2] = (void*) nOffsetAndIndex;
#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr, "in: offset/index is %x %x %d, %lx\n",
    nFunctionIndex, nVtableOffset, !simpleRetType, raw[2]);
#endif
    return (code + codeSnippetSize);
}

}

void bridges::cpp_uno::shared::VtableFactory::flushCode(unsigned char const * bptr, unsigned char const * eptr)
{
    int const lineSize = 32;
    for (unsigned char const * p = bptr; p < eptr + lineSize; p += lineSize) {
        __asm__ volatile ("dcbst 0, %0" : : "r"(p) : "memory");
    }
    __asm__ volatile ("sync" : : : "memory");
    for (unsigned char const * p = bptr; p < eptr + lineSize; p += lineSize) {
        __asm__ volatile ("icbi 0, %0" : : "r"(p) : "memory");
    }
    __asm__ volatile ("isync" : : : "memory");
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
    sal_Int32 functionCount, sal_Int32 vtableOffset)
{
     (*slots) -= functionCount;
     Slot * s = *slots;
#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr, "in addLocalFunctions functionOffset is %x\n",functionOffset);
    fprintf(stderr, "in addLocalFunctions vtableOffset is %x\n",vtableOffset);
#endif

    for (sal_Int32 i = 0; i < type->nMembers; ++i) {
        typelib_TypeDescription * member = 0;
        TYPELIB_DANGER_GET(&member, type->ppMembers[i]);
        OSL_ASSERT(member != 0);
        switch (member->eTypeClass) {
        case typelib_TypeClass_INTERFACE_ATTRIBUTE:
            
            (s++)->fn = code + writetoexecdiff;
            code = codeSnippet(
                code, functionOffset++, vtableOffset,
                bridges::cpp_uno::shared::isSimpleType(
                    reinterpret_cast<
                    typelib_InterfaceAttributeTypeDescription * >(
                        member)->pAttributeTypeRef));

            
            if (!reinterpret_cast<
                typelib_InterfaceAttributeTypeDescription * >(
                    member)->bReadOnly)
            {
                (s++)->fn = code + writetoexecdiff;
                code = codeSnippet(code, functionOffset++, vtableOffset, true);
            }
            break;

        case typelib_TypeClass_INTERFACE_METHOD:
            (s++)->fn = code + writetoexecdiff;
            code = codeSnippet(
                code, functionOffset++, vtableOffset,
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
