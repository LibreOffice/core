/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <com/sun/star/uno/genfunc.hxx>
#include <sal/log.hxx>
#include <uno/data.h>
#include <typelib/typedescription.hxx>
#include <osl/endian.h>
#include "bridge.hxx"
#include "cppinterfaceproxy.hxx"
#include "types.hxx"
#include "vtablefactory.hxx"

#include "share.hxx"
#include <stdio.h>
#include <string.h>

#ifdef OSL_BIGENDIAN
#define IS_BIG_ENDIAN 1
#else
#define IS_BIG_ENDIAN 0
#endif

using namespace ::com::sun::star::uno;

namespace
{


static typelib_TypeClass cpp2uno_call(
    bridges::cpp_uno::shared::CppInterfaceProxy * pThis,
    const typelib_TypeDescription * pMemberTypeDescr,
    typelib_TypeDescriptionReference * pReturnTypeRef, // 0 indicates void return
    sal_Int32 nParams, typelib_MethodParameter * pParams,
        void ** gpreg, void ** fpreg, void ** ovrflw,
    sal_Int64 * pRegisterReturn /* space for register return */ )
{
#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr, "as far as cpp2uno_call\n");
#endif

    int ng = 0; //number of gpr registers used
    int nf = 0; //number of fpr registers used

    // gpreg:  [ret *], this, [gpr params]
    // fpreg:  [fpr params]
    // ovrflw: [gpr or fpr params (properly aligned)]

    // return
    typelib_TypeDescription * pReturnTypeDescr = 0;
    if (pReturnTypeRef)
        TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );

    void * pUnoReturn = 0;
    void * pCppReturn = 0; // complex return ptr: if != 0 && != pUnoReturn, reconversion need

    if (pReturnTypeDescr)
    {
        if (!ppc64::return_in_hidden_param(pReturnTypeRef))
        {
            pUnoReturn = pRegisterReturn; // direct way for simple types
        }
        else // complex return via ptr (pCppReturn)
        {
            pCppReturn = *(void **)gpreg;
            gpreg++;
            ng++;

            pUnoReturn = (bridges::cpp_uno::shared::relatesToInterfaceType( pReturnTypeDescr )
                          ? alloca( pReturnTypeDescr->nSize )
                          : pCppReturn); // direct way
        }
    }
    // pop this
    gpreg++;
    ng++;

    // stack space
    static_assert(sizeof(void *) == sizeof(sal_Int64), "### unexpected size!");
    // parameters
    void ** pUnoArgs = (void **)alloca( 4 * sizeof(void *) * nParams );
    void ** pCppArgs = pUnoArgs + nParams;
    // indices of values this have to be converted (interface conversion cpp<=>uno)
    sal_Int32 * pTempIndices = (sal_Int32 *)(pUnoArgs + (2 * nParams));
    // type descriptions for reconversions
    typelib_TypeDescription ** ppTempParamTypeDescr = (typelib_TypeDescription **)(pUnoArgs + (3 * nParams));

    sal_Int32 nTempIndices = 0;
    bool bOverflowUsed = false;
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

                        if (ng < ppc64::MAX_GPR_REGS)
                        {
                            ng++;
                            gpreg++;
                        }
                    }
                    else
                    {
                        pCppArgs[nPos] = pUnoArgs[nPos] = ovrflw;
                        bOverflowUsed = true;
                    }
                    if (bOverflowUsed) ovrflw++;
                    break;
                case typelib_TypeClass_BYTE:
                case typelib_TypeClass_BOOLEAN:
                    if (ng < ppc64::MAX_GPR_REGS)
                    {
                        pCppArgs[nPos] = pUnoArgs[nPos] = (((char *)gpreg) + 7*IS_BIG_ENDIAN);
                        ng++;
                        gpreg++;
                    }
                    else
                    {
                        pCppArgs[nPos] = pUnoArgs[nPos] = (((char *)ovrflw) + 7*IS_BIG_ENDIAN);
                        bOverflowUsed = true;
                    }
                    if (bOverflowUsed) ovrflw++;
                    break;
                case typelib_TypeClass_CHAR:
                case typelib_TypeClass_SHORT:
                case typelib_TypeClass_UNSIGNED_SHORT:
                    if (ng < ppc64::MAX_GPR_REGS)
                    {
                        pCppArgs[nPos] = pUnoArgs[nPos] = (((char *)gpreg) + 6*IS_BIG_ENDIAN);
                        ng++;
                        gpreg++;
                    }
                    else
                    {
                        pCppArgs[nPos] = pUnoArgs[nPos] = (((char *)ovrflw) + 6*IS_BIG_ENDIAN);
                        bOverflowUsed = true;
                    }
                    if (bOverflowUsed) ovrflw++;
                    break;
        case typelib_TypeClass_ENUM:
                case typelib_TypeClass_LONG:
                case typelib_TypeClass_UNSIGNED_LONG:
                    if (ng < ppc64::MAX_GPR_REGS)
                    {
                        pCppArgs[nPos] = pUnoArgs[nPos] = (((char *)gpreg) + 4*IS_BIG_ENDIAN);
                        ng++;
                        gpreg++;
                    }
                    else
                    {
                        pCppArgs[nPos] = pUnoArgs[nPos] = (((char *)ovrflw) + 4*IS_BIG_ENDIAN);
                        bOverflowUsed = true;
                    }
                    if (bOverflowUsed) ovrflw++;
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
                        bOverflowUsed = true;
                    }
                    if (bOverflowUsed) ovrflw++;
                    break;
                }

                // no longer needed
                TYPELIB_DANGER_RELEASE( pParamTypeDescr );
        }
        else // ptr to complex value | ref
        {
#if OSL_DEBUG_LEVEL > 2
            fprintf(stderr, "complex, ng is %d\n", ng);
#endif
            void *pCppStack; //temporary stack pointer

            if (ng < ppc64::MAX_GPR_REGS)
            {
                pCppArgs[nPos] = pCppStack = *gpreg++;
                ng++;
            }
            else
            {
                pCppArgs[nPos] = pCppStack = *ovrflw;
                bOverflowUsed = true;
            }
            if (bOverflowUsed) ovrflw++;

            if (! rParam.bIn) // is pure out
            {
                // uno out is unconstructed mem!
                pUnoArgs[nPos] = alloca( pParamTypeDescr->nSize );
                pTempIndices[nTempIndices] = nPos;
                // will be released at reconversion
                ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
            }
            // is in/inout
            else if (bridges::cpp_uno::shared::relatesToInterfaceType( pParamTypeDescr ))
            {
                uno_copyAndConvertData( pUnoArgs[nPos] = alloca( pParamTypeDescr->nSize ),
                                        pCppStack, pParamTypeDescr,
                                        pThis->getBridge()->getCpp2Uno() );
                pTempIndices[nTempIndices] = nPos; // has to be reconverted
                // will be released at reconversion
                ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
            }
            else // direct way
            {
                pUnoArgs[nPos] = pCppStack;
                // no longer needed
                TYPELIB_DANGER_RELEASE( pParamTypeDescr );
            }
        }
    }

#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr, "end of params\n");
#endif

    // ExceptionHolder
    uno_Any aUnoExc; // Any will be constructed by callee
    uno_Any * pUnoExc = &aUnoExc;

    // invoke uno dispatch call
    (*pThis->getUnoI()->pDispatcher)( pThis->getUnoI(), pMemberTypeDescr, pUnoReturn, pUnoArgs, &pUnoExc );

    // in case an exception occurred...
    if (pUnoExc)
    {
        // destruct temporary in/inout params
        for ( ; nTempIndices--; )
        {
            sal_Int32 nIndex = pTempIndices[nTempIndices];

            if (pParams[nIndex].bIn) // is in/inout => was constructed
                uno_destructData( pUnoArgs[nIndex], ppTempParamTypeDescr[nTempIndices], 0 );
            TYPELIB_DANGER_RELEASE( ppTempParamTypeDescr[nTempIndices] );
        }
        if (pReturnTypeDescr)
            TYPELIB_DANGER_RELEASE( pReturnTypeDescr );

        CPPU_CURRENT_NAMESPACE::raiseException( &aUnoExc, pThis->getBridge()->getUno2Cpp() );
                // has to destruct the any
        // is here for dummy
        return typelib_TypeClass_VOID;
    }
    else // else no exception occurred...
    {
        // temporary params
        for ( ; nTempIndices--; )
        {
            sal_Int32 nIndex = pTempIndices[nTempIndices];
            typelib_TypeDescription * pParamTypeDescr = ppTempParamTypeDescr[nTempIndices];

            if (pParams[nIndex].bOut) // inout/out
            {
                // convert and assign
                uno_destructData( pCppArgs[nIndex], pParamTypeDescr, cpp_release );
                uno_copyAndConvertData( pCppArgs[nIndex], pUnoArgs[nIndex], pParamTypeDescr,
                                        pThis->getBridge()->getUno2Cpp() );
            }
            // destroy temp uno param
            uno_destructData( pUnoArgs[nIndex], pParamTypeDescr, 0 );

            TYPELIB_DANGER_RELEASE( pParamTypeDescr );
        }
        // return
        if (pCppReturn) // has complex return
        {
            if (pUnoReturn != pCppReturn) // needs reconversion
            {
                uno_copyAndConvertData( pCppReturn, pUnoReturn, pReturnTypeDescr,
                                        pThis->getBridge()->getUno2Cpp() );
                // destroy temp uno return
                uno_destructData( pUnoReturn, pReturnTypeDescr, 0 );
            }
            // complex return ptr is set to return reg
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

#if defined(_CALL_ELF) && _CALL_ELF == 2
#  define PARAMSAVE 32
#else
#  define PARAMSAVE 48
#endif

static typelib_TypeClass cpp_mediate(
    sal_uInt64 nOffsetAndIndex,
        void ** gpreg, void ** fpreg, long sp,
    sal_Int64 * pRegisterReturn /* space for register return */ )
{
    static_assert(sizeof(sal_Int64)==sizeof(void *), "### unexpected!");

    sal_Int32 nVtableOffset = (nOffsetAndIndex >> 32);
    sal_Int32 nFunctionIndex = (nOffsetAndIndex & 0xFFFFFFFF);

    long sf = *(long*)sp;
    void ** ovrflw = (void**)(sf + PARAMSAVE + 64);

    // gpreg:  [ret *], this, [other gpr params]
    // fpreg:  [fpr params]
    // ovrflw: [gpr or fpr params (properly aligned)]

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

    if (nFunctionIndex >= pTypeDescr->nMapFunctionIndexToMemberIndex)
    {
        SAL_WARN(
            "bridges",
            "illegal " << OUString::unacquired(&pTypeDescr->aBase.pTypeName)
                << " vtable index " << nFunctionIndex << "/"
                << pTypeDescr->nMapFunctionIndexToMemberIndex);
        throw RuntimeException(
            ("illegal " + OUString::unacquired(&pTypeDescr->aBase.pTypeName)
             + " vtable index " + OUString::number(nFunctionIndex) + "/"
             + OUString::number(pTypeDescr->nMapFunctionIndexToMemberIndex)),
            (XInterface *)pThis);
    }

    // determine called method
    sal_Int32 nMemberPos = pTypeDescr->pMapFunctionIndexToMemberIndex[nFunctionIndex];
    assert(nMemberPos < pTypeDescr->nAllMembers);

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
            // is GET method
            eRet = cpp2uno_call(
                pCppI, aMemberDescr.get(),
                ((typelib_InterfaceAttributeTypeDescription *)aMemberDescr.get())->pAttributeTypeRef,
                0, 0, // no params
                gpreg, fpreg, ovrflw, pRegisterReturn );
        }
        else
        {
            // is SET method
            typelib_MethodParameter aParam;
            aParam.pTypeRef =
                ((typelib_InterfaceAttributeTypeDescription *)aMemberDescr.get())->pAttributeTypeRef;
            aParam.bIn      = sal_True;
            aParam.bOut     = sal_False;

            eRet = cpp2uno_call(
                pCppI, aMemberDescr.get(),
                0, // indicates void return
                1, &aParam,
                gpreg, fpreg, ovrflw, pRegisterReturn );
        }
        break;
    }
    case typelib_TypeClass_INTERFACE_METHOD:
    {
        // is METHOD
        switch (nFunctionIndex)
        {
        case 1: // acquire()
            pCppI->acquireProxy(); // non virtual call!
            eRet = typelib_TypeClass_VOID;
            break;
        case 2: // release()
            pCppI->releaseProxy(); // non virtual call!
            eRet = typelib_TypeClass_VOID;
            break;
        case 0: // queryInterface() opt
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
        } // else perform queryInterface()
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

        throw RuntimeException( "no member description found!", (XInterface *)pThis );
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

    register long r3 asm("r3"); gpreg[0] = r3;
    register long r4 asm("r4"); gpreg[1] = r4;
    register long r5 asm("r5"); gpreg[2] = r5;
    register long r6 asm("r6"); gpreg[3] = r6;
    register long r7 asm("r7"); gpreg[4] = r7;
    register long r8 asm("r8"); gpreg[5] = r8;
    register long r9 asm("r9"); gpreg[6] = r9;
    register long r10 asm("r10"); gpreg[7] = r10;

    double fpreg[ppc64::MAX_SSE_REGS];

    __asm__ __volatile__ (
        "stfd 1,   0(%0)\t\n"
        "stfd 2,   8(%0)\t\n"
        "stfd 3,  16(%0)\t\n"
        "stfd 4,  24(%0)\t\n"
        "stfd 5,  32(%0)\t\n"
        "stfd 6,  40(%0)\t\n"
        "stfd 7,  48(%0)\t\n"
        "stfd 8,  56(%0)\t\n"
        "stfd 9,  64(%0)\t\n"
        "stfd 10, 72(%0)\t\n"
        "stfd 11, 80(%0)\t\n"
        "stfd 12, 88(%0)\t\n"
        "stfd 13, 96(%0)\t\n"
    : : "r" (fpreg)
        : "fr1", "fr2", "fr3", "fr4", "fr5", "fr6", "fr7", "fr8", "fr9",
          "fr10", "fr11", "fr12", "fr13"
    );

    register long r11 asm("r11");
    const long nOffsetAndIndex = r11;

    register long r1 asm("r1");
    const long sp = r1;

#if defined(_CALL_ELF) && _CALL_ELF == 2
    volatile long nRegReturn[2];
#else
    volatile long nRegReturn[1];
#endif

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
#if defined(_CALL_ELF) && _CALL_ELF == 2
            __asm__( "ld 4,%0\n\t"
                : : "m" (nRegReturn[1]) );
#endif
            break;
    }
}

#if defined(_CALL_ELF) && _CALL_ELF == 2
const int codeSnippetSize = 32;
#else
const int codeSnippetSize = 24;
#endif

unsigned char *  codeSnippet( unsigned char * code, sal_Int32 nFunctionIndex, sal_Int32 nVtableOffset,
                              bool bHasHiddenParam)
{
#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr,"in codeSnippet functionIndex is %x\n", nFunctionIndex);
    fprintf(stderr,"in codeSnippet vtableOffset is %x\n", nVtableOffset);
#endif

    sal_uInt64 nOffsetAndIndex = ( ( (sal_uInt64) nVtableOffset ) << 32 ) | ( (sal_uInt64) nFunctionIndex );

    if ( bHasHiddenParam )
        nOffsetAndIndex |= 0x80000000;
#if defined(_CALL_ELF) && _CALL_ELF == 2
    unsigned int *raw = (unsigned int *)&code[0];

    raw[0] = 0xe96c0018;        /* 0:   ld      11,2f-0b(12)    */
    raw[1] = 0xe98c0010;        /*      ld      12,1f-0b(12)    */
    raw[2] = 0x7d8903a6;        /*      mtctr   12              */
    raw[3] = 0x4e800420;        /*      bctr                    */
                                /* 1:   .quad   function_addr   */
                                /* 2:   .quad   context         */
    *(void **)&raw[4] = (void *)privateSnippetExecutor;
    *(void **)&raw[6] = (void*)nOffsetAndIndex;
#else
    void ** raw = (void **)&code[0];
    memcpy(raw, (char*) privateSnippetExecutor, 16);
    raw[2] = (void*) nOffsetAndIndex;
#endif
#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr, "in: offset/index is %x %x %d, %lx\n",
    nFunctionIndex, nVtableOffset, bHasHiddenParam, raw[2]);
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

std::size_t bridges::cpp_uno::shared::VtableFactory::getBlockSize(
    sal_Int32 slotCount)
{
    return (slotCount + 2) * sizeof (Slot) + slotCount * codeSnippetSize;
}

bridges::cpp_uno::shared::VtableFactory::Slot *
bridges::cpp_uno::shared::VtableFactory::initializeBlock(
    void * block, sal_Int32 slotCount, sal_Int32,
    typelib_InterfaceTypeDescription *)
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
        assert(member != 0);
        switch (member->eTypeClass) {
        case typelib_TypeClass_INTERFACE_ATTRIBUTE:
            // Getter:
            (s++)->fn = code + writetoexecdiff;
            code = codeSnippet(
                code, functionOffset++, vtableOffset,
                ppc64::return_in_hidden_param(
                    reinterpret_cast<
                    typelib_InterfaceAttributeTypeDescription * >(
                        member)->pAttributeTypeRef));

            // Setter:
            if (!reinterpret_cast<
                typelib_InterfaceAttributeTypeDescription * >(
                    member)->bReadOnly)
            {
                (s++)->fn = code + writetoexecdiff;
                code = codeSnippet(code, functionOffset++, vtableOffset, false);
            }
            break;

        case typelib_TypeClass_INTERFACE_METHOD:
            (s++)->fn = code + writetoexecdiff;
            code = codeSnippet(
                code, functionOffset++, vtableOffset,
                ppc64::return_in_hidden_param(
                    reinterpret_cast<
                    typelib_InterfaceMethodTypeDescription * >(
                        member)->pReturnTypeRef));
            break;

        default:
            assert(false);
            break;
        }
        TYPELIB_DANGER_RELEASE(member);
    }
    return code;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
