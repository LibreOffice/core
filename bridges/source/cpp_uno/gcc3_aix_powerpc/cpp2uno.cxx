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

#include <string.h>

using namespace ::com::sun::star::uno;

namespace
{


static typelib_TypeClass cpp2uno_call(
        bridges::cpp_uno::shared::CppInterfaceProxy * pThis,
        const typelib_TypeDescription * pMemberTypeDescr,
        typelib_TypeDescriptionReference * pReturnTypeRef, 
        sal_Int32 nParams, typelib_MethodParameter * pParams,
        void ** gpreg, double * fpreg, void ** ovrflw,
        sal_uInt64 * pRegisterReturn /* space for register return */ )
{
        
        
        

        
        typelib_TypeDescription * pReturnTypeDescr = 0;
        if (pReturnTypeRef)
                TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );

        void * pUnoReturn = 0;
        void * pCppReturn = 0; 

        sal_Int32 ng = 0;
        sal_Int32 nf = 0;

    ovrflw -= ppc::MAX_GPR_REGS;

        if (pReturnTypeDescr)
        {
                if (bridges::cpp_uno::shared::isSimpleType( pReturnTypeDescr ))
                        pUnoReturn = pRegisterReturn; 
                else 
                {
                        pCppReturn = *gpreg;
                        ++gpreg;
                        ++ng;
                        ++ovrflw;

                        pUnoReturn = (bridges::cpp_uno::shared::relatesToInterfaceType( pReturnTypeDescr )
                                                  ? __builtin_alloca( pReturnTypeDescr->nSize )
                                                  : pCppReturn); 
                }
        }
        
        ++gpreg;
        ++ng;
        ++ovrflw;

        
        OSL_ENSURE( sizeof(void *) == sizeof(sal_Int32), "### unexpected size!" );
        
        void ** pUnoArgs = (void **)__builtin_alloca( 4 * sizeof(void *) * nParams );
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
                        switch (pParamTypeDescr->eTypeClass)
                        {
                        case typelib_TypeClass_BOOLEAN:
                        case typelib_TypeClass_BYTE:
                                if (ng < ppc::MAX_GPR_REGS)
                                {
                                    *ovrflw = *gpreg++;
                                    ++ng;
                }
                pCppArgs[nPos] = pUnoArgs[nPos] = (((char *)ovrflw) + (sizeof(void*)-1));
                ++ovrflw;
                                break;
                        case typelib_TypeClass_CHAR:
                        case typelib_TypeClass_SHORT:
                        case typelib_TypeClass_UNSIGNED_SHORT:
                                if (ng < ppc::MAX_GPR_REGS)
                                {
                                    *ovrflw = *gpreg++;
                                    ++ng;
                                }
                                pCppArgs[nPos] = pUnoArgs[nPos] = (((char *)ovrflw) + (sizeof(void*)-2));
                                ++ovrflw;
                                break;
                        case typelib_TypeClass_HYPER:
                        case typelib_TypeClass_UNSIGNED_HYPER:
                                pCppArgs[nPos] = pUnoArgs[nPos] = ovrflw;
                            for (int i = 0; i < 2; ++i)
                            {
                                if (ng < ppc::MAX_GPR_REGS)
                                {
                                        *ovrflw = *gpreg++;
                                        ++ng;
                                }
                                    ++ovrflw;
                            }
                            break;
                        case typelib_TypeClass_DOUBLE:
                                if (nf < ppc::MAX_SSE_REGS)
                                {
                                    pCppArgs[nPos] = pUnoArgs[nPos] = fpreg;
                                    ++fpreg;
                                    ++nf;
                                }
                                else
                                {
                                    pCppArgs[nPos] = pUnoArgs[nPos] = ovrflw;
                                }
                                if (ng < ppc::MAX_GPR_REGS)
                                {
                                    ng+=2;
                                    gpreg+=2;
                                }
                                ovrflw+=2;
                                break;
                        case typelib_TypeClass_FLOAT:
                            if (nf < ppc::MAX_SSE_REGS)
                            {
                                    pCppArgs[nPos] = pUnoArgs[nPos] = fpreg;
                                    ++fpreg;
                                    ++nf;
                            }
                            else
                            {
                                pCppArgs[nPos] = pUnoArgs[nPos] = ovrflw;
                            }
                                if (ng < ppc::MAX_GPR_REGS)
                                {
                                    ++gpreg;
                                    ++ng;
                                }
                ++ovrflw;
                            break;
                        default:
                                if (ng < ppc::MAX_GPR_REGS)
                                {
                                    *ovrflw = *gpreg++;
                                    ++ng;
                                }
                                pCppArgs[nPos] = pUnoArgs[nPos] = ovrflw;
                                ++ovrflw;
                            break;
                        }

                        
                        TYPELIB_DANGER_RELEASE( pParamTypeDescr );
                }
                else 
                {
                        void *pCppStack; 

                        if (ng < ppc::MAX_GPR_REGS)
                        {
                            *ovrflw = *gpreg++;
                            ++ng;
                        }
                    pCppArgs[nPos] = pCppStack = *ovrflw++;

                        if (! rParam.bIn) 
                        {
                                
                                pUnoArgs[nPos] = __builtin_alloca( pParamTypeDescr->nSize );
                                pTempIndices[nTempIndices] = nPos;
                                
                                ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
                        }
                        
                        else if (bridges::cpp_uno::shared::relatesToInterfaceType( pParamTypeDescr ))
                        {
                                uno_copyAndConvertData( pUnoArgs[nPos] = __builtin_alloca( pParamTypeDescr->nSize ),
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
        sal_Int32 nFunctionIndex,
        sal_Int32 nVtableOffset,
        void ** gpreg, double * fpreg, void ** ovrflw,
        sal_uInt64 * pRegisterReturn /* space for register return */ )
{
        OSL_ENSURE( sizeof(sal_Int32)==sizeof(void *), "### unexpected!" );

        
        
        

        void * pThis;
        if( nFunctionIndex & 0x8000 )
        {
                nFunctionIndex &= 0x7fff;
                pThis = gpreg[1];
        }
        else
        {
                pThis = gpreg[0];
        }

        pThis = static_cast< char * >(pThis) - nVtableOffset;

        bridges::cpp_uno::shared::CppInterfaceProxy * pCppI
        = bridges::cpp_uno::shared::CppInterfaceProxy::castInterfaceToProxy(pThis);

        typelib_InterfaceTypeDescription * pTypeDescr = pCppI->getTypeDescr();

        OSL_ENSURE( nFunctionIndex < pTypeDescr->nMapFunctionIndexToMemberIndex, "### illegal vtable index!" );
        if (nFunctionIndex >= pTypeDescr->nMapFunctionIndexToMemberIndex)
        {
                throw RuntimeException( "illegal vtable index!", (XInterface *)pThis );
        }

        
        sal_Int32 nMemberPos = pTypeDescr->pMapFunctionIndexToMemberIndex[nFunctionIndex];
        OSL_ENSURE( nMemberPos < pTypeDescr->nAllMembers, "### illegal member index!" );

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
                        aParam.bIn              = sal_True;
                        aParam.bOut             = sal_False;

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
                    (void **)&pInterface, pCppI->getOid().pData, (typelib_InterfaceTypeDescription *)pTD );

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
                throw RuntimeException( "no member description found!", (XInterface *)pThis );
        }
        }

        return eRet;
}


/**
 * is called on incoming vtable calls
 * (called by asm snippets)
 */
static sal_uInt64 cpp_vtable_call(sal_Int32 r3, sal_Int32 r4, sal_Int32 r5,
    sal_Int32 r6, sal_Int32 r7, sal_Int32 r8, sal_Int32 r9,
    sal_Int32 r10, sal_Int32 firstonstack)
{
    volatile unsigned long nOffsetAndIndex;

    __asm__ __volatile__(
      "mr %0, 11\n\t"
      : "=r" (nOffsetAndIndex) : );

    sal_Int32 nVtableOffset = (nOffsetAndIndex >> 16);
    sal_Int32 nFunctionIndex = (nOffsetAndIndex & 0xFFFF);

    void ** ovrflw = (void**)&firstonstack;

    sal_Int32 gpreg[ppc::MAX_GPR_REGS];
    gpreg[0] = r3;
    gpreg[1] = r4;
    gpreg[2] = r5;
    gpreg[3] = r6;
    gpreg[4] = r7;
    gpreg[5] = r8;
    gpreg[6] = r9;
    gpreg[7] = r10;

    double fpreg[ppc::MAX_SSE_REGS];
    register double d0 asm("fr1"); fpreg[0] = d0;
    register double d1 asm("fr2"); fpreg[1] = d1;
    register double d2 asm("fr3"); fpreg[2] = d2;
    register double d3 asm("fr4"); fpreg[3] = d3;
    register double d4 asm("fr5"); fpreg[4] = d4;
    register double d5 asm("fr6"); fpreg[5] = d5;
    register double d6 asm("fr7"); fpreg[6] = d6;
    register double d7 asm("fr8"); fpreg[7] = d7;
    register double d8 asm("fr9"); fpreg[8] = d8;
    register double d9 asm("fr10"); fpreg[9] = d9;
    register double d10 asm("fr11"); fpreg[10] = d10;
    register double d11 asm("fr12"); fpreg[11] = d11;
    register double d12 asm("fr13"); fpreg[12] = d12;

#if OSL_DEBUG_LEVEL > 2
    for(int i = 0; i < 8; ++i)
    {
        fprintf(stderr, "general reg %d is %x\n", i, gpreg[i]);
    }
    for(int i = 0; i < 13; ++i)
    {
        fprintf(stderr, "sse reg %d is %f\n", i, fpreg[i]);
        fprintf(stderr, "sse reg %d is %llx\n", i, fpreg[i]);
    }
    for(int i = -8; i < 8; ++i)
    {
        fprintf(stderr, "overflow arg %d is %x\n", i, ovrflw[i]);
    }
#endif
        sal_uInt64 nRegReturn=0;

        typelib_TypeClass aType =
             cpp_mediate( nFunctionIndex, nVtableOffset, (void**)gpreg, fpreg, ovrflw, &nRegReturn );

        sal_uInt32 *pRegReturn = (sal_uInt32*)&nRegReturn;
        switch( aType )
        {
        case typelib_TypeClass_BOOLEAN:
            pRegReturn[0] = (sal_uInt32)(*(char *)pRegReturn);
            break;
        case typelib_TypeClass_BYTE:
            pRegReturn[0] = (sal_Int32)(*(unsigned char *)pRegReturn);
            break;
        case typelib_TypeClass_SHORT:
            pRegReturn[0] = (sal_Int32)(*(short *)pRegReturn);
            break;
        case typelib_TypeClass_CHAR:
        case typelib_TypeClass_UNSIGNED_SHORT:
            pRegReturn[0] = (sal_uInt32)(*(unsigned short *)pRegReturn);
            break;
                case typelib_TypeClass_FLOAT:
                        __asm__("lfs 1,%0\n\t" : : "m"(*((float*)&nRegReturn)));
                    break;
                case typelib_TypeClass_DOUBLE:
                        __asm__("lfd 1,%0\n\t" : : "m"(*((double*)&nRegReturn)));
                    break;
                case typelib_TypeClass_HYPER:
                case typelib_TypeClass_UNSIGNED_HYPER:
                    break;
                default:
            pRegReturn[0] = (sal_uInt32)(*(unsigned int*)pRegReturn);
                    break;
        }
    return nRegReturn;
}


int const codeSnippetSize = 3 * sizeof(void*);

unsigned char * codeSnippet( unsigned char * code, sal_Int16 functionIndex,
                  sal_Int16 vtableOffset, bool simpleRetType )
{
    sal_uInt32 nOffsetAndIndex = ( ( vtableOffset ) << 16 ) | (functionIndex );
    if (! simpleRetType )
        nOffsetAndIndex |= 0x8000;

    void **raw = (void**)&code[0];
    memcpy(raw, (char*)cpp_vtable_call, 2 * sizeof(void*));
    raw[2] = (void*)nOffsetAndIndex;

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
                code, functionOffset++, vtableOffset,
                bridges::cpp_uno::shared::isSimpleType(
                    reinterpret_cast<
                    typelib_InterfaceAttributeTypeDescription * >(
                        member)->pAttributeTypeRef));

            
            if (!reinterpret_cast<
                typelib_InterfaceAttributeTypeDescription * >(
                    member)->bReadOnly)
            {
                (s++)->fn = code;
                code = codeSnippet(code, functionOffset++, vtableOffset, true);
            }
            break;

        case typelib_TypeClass_INTERFACE_METHOD:
            (s++)->fn = code;
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
