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
        long r8, void ** gpreg, double *fpreg, void ** ovrflw,
        sal_Int64 * pRegisterReturn /* space for register return */ )
    {
        void ** startovrflw = ovrflw;
        int nregs = 0; //number of words passed in registers

#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr, "cpp2uno_call\n");
#endif
        // return
        typelib_TypeDescription * pReturnTypeDescr = 0;
        if (pReturnTypeRef)
            TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );

        void * pUnoReturn = 0;
        // complex return ptr: if != 0 && != pUnoReturn, reconversion need
        void * pCppReturn = 0;

        if (pReturnTypeDescr)
        {
            if (hppa::isRegisterReturn(pReturnTypeRef))
            {
#if OSL_DEBUG_LEVEL > 2
        fprintf(stderr, "simple return\n");
#endif
                pUnoReturn = pRegisterReturn; // direct way for simple types
            }
            else
            {
#if OSL_DEBUG_LEVEL > 2
        fprintf(stderr, "complex return via r8\n");
#endif
                pCppReturn = (void *)r8;

                pUnoReturn = (bridges::cpp_uno::shared::relatesToInterfaceType( pReturnTypeDescr )
                    ? alloca( pReturnTypeDescr->nSize )
                    : pCppReturn); // direct way
            }
        }
        // pop this
        gpreg++;
        fpreg++;
        nregs++;

        // stack space
        OSL_ENSURE( sizeof(void *) == sizeof(sal_Int32), "### unexpected size!" );
        // parameters
        void ** pUnoArgs = (void **)alloca( 4 * sizeof(void *) * nParams );
        void ** pCppArgs = pUnoArgs + nParams;
        // indices of values this have to be converted (interface conversion
        // cpp<=>uno)
        sal_Int32 * pTempIndices = (sal_Int32 *)(pUnoArgs + (2 * nParams));
        // type descriptions for reconversions
        typelib_TypeDescription ** ppTempParamTypeDescr =
            (typelib_TypeDescription **)(pUnoArgs + (3 * nParams));

        sal_Int32 nTempIndices   = 0;
        bool bOverFlowUsed = false;
        for ( sal_Int32 nPos = 0; nPos < nParams; ++nPos )
        {
            const typelib_MethodParameter & rParam = pParams[nPos];
            typelib_TypeDescription * pParamTypeDescr = 0;
            TYPELIB_DANGER_GET( &pParamTypeDescr, rParam.pTypeRef );

            if (!rParam.bOut && bridges::cpp_uno::shared::isSimpleType( pParamTypeDescr ))
            {
                switch (pParamTypeDescr->eTypeClass)
                {
                    case typelib_TypeClass_DOUBLE:
                        if (nregs < hppa::MAX_WORDS_IN_REGS && (nregs & 1))
                        {
                            gpreg++;
                            fpreg++;
                            nregs++;
                        }
                        if (nregs < hppa::MAX_WORDS_IN_REGS-1)
                        {
                            fpreg++;
                            pCppArgs[nPos] = pUnoArgs[nPos] = fpreg;
                            gpreg+=2;
                            fpreg+=2;
                            nregs+=2;
                        }
                        else
                        {
                            if ((startovrflw-ovrflw) & 1)
                                ovrflw--;
                            pCppArgs[nPos] = pUnoArgs[nPos] = ((char*)ovrflw - 4);
                            bOverFlowUsed = true;
                        }
                        if (bOverFlowUsed) ovrflw-=2;
                        break;
                    case typelib_TypeClass_FLOAT:
                        if (nregs < hppa::MAX_WORDS_IN_REGS)
                        {
                            pCppArgs[nPos] = pUnoArgs[nPos] = fpreg;
                            gpreg++;
                            fpreg++;
                            nregs++;
                        }
                        else
                        {
                            pCppArgs[nPos] = pUnoArgs[nPos] = ovrflw;
                            bOverFlowUsed = true;
                        }
                        if (bOverFlowUsed) ovrflw--;
                        break;
                    case typelib_TypeClass_HYPER:
                    case typelib_TypeClass_UNSIGNED_HYPER:
                        if (nregs < hppa::MAX_WORDS_IN_REGS && (nregs & 1))
                        {
                            gpreg++;
                            fpreg++;
                            nregs++;
                        }
                        if (nregs < hppa::MAX_WORDS_IN_REGS-1)
                        {
                            pCppArgs[nPos] = pUnoArgs[nPos] = gpreg;
                            gpreg+=2;
                            fpreg+=2;
                            nregs+=2;
                        }
                        else
                        {
                            if ((startovrflw-ovrflw) & 1)
                                ovrflw--;
                            pCppArgs[nPos] = pUnoArgs[nPos] = ((char*)ovrflw - 4);
                            bOverFlowUsed = true;
                        }
                        if (bOverFlowUsed) ovrflw-=2;
                        break;
                    case typelib_TypeClass_BYTE:
                    case typelib_TypeClass_BOOLEAN:
                        if (nregs < hppa::MAX_WORDS_IN_REGS)
                        {
                            pCppArgs[nPos] = pUnoArgs[nPos] = ((char*)gpreg + 3);
                            gpreg++;
                            fpreg++;
                            nregs++;
                        }
                        else
                        {
                            pCppArgs[nPos] = pUnoArgs[nPos] = ((char*)ovrflw+3);
                            bOverFlowUsed = true;
                        }
                        if (bOverFlowUsed) ovrflw--;
                        break;
                    case typelib_TypeClass_CHAR:
                    case typelib_TypeClass_SHORT:
                    case typelib_TypeClass_UNSIGNED_SHORT:
                        if (nregs < hppa::MAX_WORDS_IN_REGS)
                        {
                            pCppArgs[nPos] = pUnoArgs[nPos] = ((char*)gpreg+2);
                            gpreg++;
                            fpreg++;
                            nregs++;
                        }
                        else
                        {
                            pCppArgs[nPos] = pUnoArgs[nPos] = ((char*)ovrflw+2);
                            bOverFlowUsed = true;
                        }
                        if (bOverFlowUsed) ovrflw--;
                        break;
                    case typelib_TypeClass_ENUM:
                    case typelib_TypeClass_LONG:
                    case typelib_TypeClass_UNSIGNED_LONG:
                    default:
                        if (nregs < hppa::MAX_WORDS_IN_REGS)
                        {
                            pCppArgs[nPos] = pUnoArgs[nPos] = gpreg;
                            gpreg++;
                            fpreg++;
                            nregs++;
                        }
                        else
                        {
                            pCppArgs[nPos] = pUnoArgs[nPos] = ovrflw;
                            bOverFlowUsed = true;
                        }
                        if (bOverFlowUsed) ovrflw--;
                        break;
                }
                // no longer needed
                TYPELIB_DANGER_RELEASE( pParamTypeDescr );
            }
            else // ptr to complex value | ref
            {
                void *pCppStack;

                if (nregs < hppa::MAX_WORDS_IN_REGS)
                {
                    pCppArgs[nPos] = pCppStack = *gpreg;
                    gpreg++;
                    fpreg++;
                    nregs++;
                }
                else
                {
                    pCppArgs[nPos] = pCppStack = *ovrflw;
                    bOverFlowUsed = true;
                }
                if (bOverFlowUsed) ovrflw--;

                if (! rParam.bIn) // is pure out
                {
                    // uno out is unconstructed mem!
                    pUnoArgs[nPos] = alloca( pParamTypeDescr->nSize );
                    pTempIndices[nTempIndices] = nPos;
                    // will be released at reconversion
                    ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
                }
                // is in/inout
                else if (bridges::cpp_uno::shared::relatesToInterfaceType(
                    pParamTypeDescr ))
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

        // ExceptionHolder
        uno_Any aUnoExc; // Any will be constructed by callee
        uno_Any * pUnoExc = &aUnoExc;

#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr, "before dispatch\n");
#endif
        // invoke uno dispatch call
        (*pThis->getUnoI()->pDispatcher)(
          pThis->getUnoI(), pMemberTypeDescr, pUnoReturn, pUnoArgs, &pUnoExc );

#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr, "after dispatch\n");
#endif

        // in case an exception occurred...
        if (pUnoExc)
        {
            // destruct temporary in/inout params
            for ( ; nTempIndices--; )
            {
                sal_Int32 nIndex = pTempIndices[nTempIndices];

                if (pParams[nIndex].bIn) // is in/inout => was constructed
                    uno_destructData( pUnoArgs[nIndex],
                        ppTempParamTypeDescr[nTempIndices], 0 );
                TYPELIB_DANGER_RELEASE( ppTempParamTypeDescr[nTempIndices] );
            }
            if (pReturnTypeDescr)
                TYPELIB_DANGER_RELEASE( pReturnTypeDescr );

            CPPU_CURRENT_NAMESPACE::raiseException( &aUnoExc,
                pThis->getBridge()->getUno2Cpp() ); // has to destruct the any
            // is here for dummy
            return typelib_TypeClass_VOID;
        }
        else // else no exception occurred...
        {
            // temporary params
            for ( ; nTempIndices--; )
            {
                sal_Int32 nIndex = pTempIndices[nTempIndices];
                typelib_TypeDescription * pParamTypeDescr =
                    ppTempParamTypeDescr[nTempIndices];

                if (pParams[nIndex].bOut) // inout/out
                {
                    // convert and assign
                    uno_destructData( pCppArgs[nIndex], pParamTypeDescr,
                        cpp_release );
                    uno_copyAndConvertData( pCppArgs[nIndex], pUnoArgs[nIndex],
                        pParamTypeDescr, pThis->getBridge()->getUno2Cpp() );
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
                    uno_copyAndConvertData( pCppReturn, pUnoReturn,
                        pReturnTypeDescr, pThis->getBridge()->getUno2Cpp() );
                    // destroy temp uno return
                    uno_destructData( pUnoReturn, pReturnTypeDescr, 0 );
                }
                // complex return ptr is set to eax
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
        void ** gpreg, double* fpreg,
        long sp, long r8,
        sal_Int64 * pRegisterReturn /* space for register return */ )

    {
    void ** ovrflw = (void**)(sp);
#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr, "cpp_mediate with\n");
    fprintf(stderr, "%x %x\n", nFunctionIndex, nVtableOffset);
    fprintf(stderr, "and %x %x\n", (long)(ovrflw[0]), (long)(ovrflw[-1]));
    fprintf(stderr, "and %x %x\n", (long)(ovrflw[-2]), (long)(ovrflw[-3]));
    fprintf(stderr, "and %x %x\n", (long)(ovrflw[-4]), (long)(ovrflw[-5]));
    fprintf(stderr, "and %x %x\n", (long)(ovrflw[-6]), (long)(ovrflw[-7]));
    fprintf(stderr, "and %x %x\n", (long)(ovrflw[-8]), (long)(ovrflw[-9]));
    fprintf(stderr, "and %x %x\n", (long)(ovrflw[-10]), (long)(ovrflw[-11]));
    fprintf(stderr, "and %x %x\n", (long)(ovrflw[-12]), (long)(ovrflw[-13]));
    fprintf(stderr, "and %x %x\n", (long)(ovrflw[-14]), (long)(ovrflw[-15]));
#endif
        OSL_ENSURE( sizeof(sal_Int32)==sizeof(void *), "### unexpected!" );

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

        // determine called method
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
                // is GET method
                eRet = cpp2uno_call(
                    pCppI, aMemberDescr.get(),
                    ((typelib_InterfaceAttributeTypeDescription *)aMemberDescr.get())->pAttributeTypeRef,
                    0, 0, // no params
                    r8, gpreg, fpreg, ovrflw, pRegisterReturn );
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
                    r8, gpreg, fpreg, ovrflw, pRegisterReturn );
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
                TYPELIB_DANGER_GET(&pTD,
                    reinterpret_cast<Type *>(gpreg[1])->getTypeLibType());
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
                            reinterpret_cast< uno_Any * >( r8 ),
                            &pInterface, pTD, cpp_acquire );
                        pInterface->release();
                        TYPELIB_DANGER_RELEASE( pTD );
                        *(void **)pRegisterReturn = (void*)r8;
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
                    r8, gpreg, fpreg, ovrflw, pRegisterReturn );
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

sal_Int64 cpp_vtable_call( sal_uInt32 in0, sal_uInt32 in1, sal_uInt32 in2, sal_uInt32 in3, sal_uInt32 firstonstack )
{
    register sal_Int32 r21 asm("r21");
    register sal_Int32 r22 asm("r22");
    register sal_Int32 r28 asm("r28");
    sal_Int32 functionIndex = r21;
    sal_Int32 vtableOffset = r22;
    sal_Int32 r8 = r28;

    long sp = (long)&firstonstack;

    sal_uInt32 gpreg[hppa::MAX_GPR_REGS];
    gpreg[0] = in0;
    gpreg[1] = in1;
    gpreg[2] = in2;
    gpreg[3] = in3;

    float fpreg[hppa::MAX_SSE_REGS]; //todo
    register float f0 asm("fr4"); fpreg[0] = f0;
    register float f1 asm("fr5"); fpreg[1] = f1;
    register float f2 asm("fr6"); fpreg[2] = f2;
    register float f3 asm("fr7"); fpreg[3] = f3;

    double dpreg[hppa::MAX_SSE_REGS]; //todo
    register double d0 asm("fr4"); dpreg[0] = d0;
    register double d1 asm("fr5"); dpreg[1] = d1;
    register double d2 asm("fr6"); dpreg[2] = d2;
    register double d3 asm("fr7"); dpreg[3] = d3;


#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr, "got to cpp_vtable_call with %x %x\n", functionIndex, vtableOffset);
    for (int i = 0; i < hppa::MAX_GPR_REGS; ++i)
    fprintf(stderr, "reg %d is %d %x\n", i, gpreg[i], gpreg[i]);
    for (int i = 0; i < hppa::MAX_SSE_REGS; ++i)
    fprintf(stderr, "float reg %d is %f %x\n", i, fpreg[i], ((long*)fpreg)[i]);
    for (int i = 0; i < 4; ++i)
    fprintf(stderr, "double reg %d is %f %llx\n", i, dpreg[i], ((long long*)dpreg)[i]);
#endif

    sal_Int64 nRegReturn;

    typelib_TypeClass aType =
        cpp_mediate( functionIndex, vtableOffset, (void**)gpreg, dpreg, sp, r8, &nRegReturn);

    switch( aType )
    {
        case typelib_TypeClass_FLOAT:
            f0 = (*((float*)&nRegReturn));
            break;
        case typelib_TypeClass_DOUBLE:
            d0 = (*((double*)&nRegReturn));
            break;
        default:
            break;
    }

    return nRegReturn;
}


namespace
{
    const int codeSnippetSize = 44;

#   define unldil(v) (((v & 0x7c) << 14) | ((v & 0x180) << 7) | ((v & 0x3) << 12) | ((v & 0xffe00) >> 8) | ((v & 0x100000) >> 20))
#   define L21(v)  unldil(((unsigned long)(v) >> 11) & 0x1fffff) //Left 21 bits
#   define R11(v)  (((unsigned long)(v) & 0x7FF) << 1) //Right 11 bits

    unsigned char *codeSnippet(unsigned char* code, sal_Int32 functionIndex,
        sal_Int32 vtableOffset, bool bHasHiddenParam)
    {
        if (bHasHiddenParam)
            functionIndex |= 0x80000000;

        unsigned char * p = code;
        *(unsigned long*)&p[0]  = 0xeaa00000; // b,l 0x8,r21
        *(unsigned long*)&p[4]  = 0xd6a01c1e; // depwi 0,31,2,r21
        *(unsigned long*)&p[8]  = 0x4aa10040; // ldw 32(r21),r1

        *(unsigned long*)&p[12] = 0x22A00000 | L21(functionIndex); // ldil L<functionIndex>,r21
        *(unsigned long*)&p[16] = 0x36B50000 | R11(functionIndex); // ldo R<functionIndex>,r21

        *(unsigned long*)&p[20] = 0x22C00000 | L21(vtableOffset); // ldil L<vtableOffset>,r22
        *(unsigned long*)&p[24] = 0x36D60000 | R11(vtableOffset); // ldo R<vtableOffset>,r22

        *(unsigned long*)&p[28] = 0x0c201094; // ldw 0(r1),r20
        *(unsigned long*)&p[32] = 0xea80c000; // bv r0(r20)
        *(unsigned long*)&p[36] = 0x0c281093; // ldw 4(r1),r19
        *(unsigned long*)&p[40] = ((unsigned long)(cpp_vtable_call) & ~2);

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
    sal_Int32 functionCount, sal_Int32 vtableOffset)
{
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
                // Getter:
                (s++)->fn = code + writetoexecdiff;
                code = codeSnippet(code, functionOffset++, vtableOffset, false);
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
            {
                (s++)->fn = code + writetoexecdiff;
                code = codeSnippet(code, functionOffset++, vtableOffset, false);
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
    void *p = (void*)((size_t)beg & ~31);
    size_t stride = 32;
    while (p < end)
    {
        asm volatile("fdc (%0)\n\t"
                     "sync\n\t"
                     "fic,m %1(%%sr4, %0)\n\t"
                     "sync" : "+r"(p) : "r"(stride) : "memory");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
