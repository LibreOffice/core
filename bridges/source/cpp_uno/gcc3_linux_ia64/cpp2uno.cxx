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
#include <uno/data.h>
#include <typelib/typedescription.hxx>

#include "bridges/cpp_uno/shared/bridge.hxx"
#include "bridges/cpp_uno/shared/cppinterfaceproxy.hxx"
#include "bridges/cpp_uno/shared/types.hxx"
#include "bridges/cpp_uno/shared/vtablefactory.hxx"

#include "share.hxx"
#include <stdio.h>

extern "C" { extern void (*privateSnippetExecutor)(); }

using namespace ::com::sun::star::uno;

namespace
{
//==================================================================================================
static typelib_TypeClass cpp2uno_call(
    bridges::cpp_uno::shared::CppInterfaceProxy * pThis,
    const typelib_TypeDescription * pMemberTypeDescr,
    typelib_TypeDescriptionReference * pReturnTypeRef, // 0 indicates void return
    sal_Int32 nParams, typelib_MethodParameter * pParams, long r8,
        void ** gpreg, void ** fpreg, void ** ovrflw,
    sal_Int64 * pRegisterReturn /* space for register return */ )
{
#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr, "as far as cpp2uno_call\n");
#endif

    int ng = 0; //number of gpr registers used
    int nf = 0; //number of fpr regsiters used

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
        if ( ia64::return_in_hidden_param( pReturnTypeRef ) ) // complex return via ptr passed as hidden parameter reg (pCppReturn)
        {
            pCppReturn = *(void **)gpreg;
            gpreg++;
            ng++;

            pUnoReturn = (bridges::cpp_uno::shared::relatesToInterfaceType( pReturnTypeDescr )
                          ? alloca( pReturnTypeDescr->nSize )
                          : pCppReturn); // direct way
        }
        else if ( ia64::return_via_r8_buffer( pReturnTypeRef ) ) // complex return via ptr passed in r8
        {
            pCppReturn = (void *)r8;

            pUnoReturn = (bridges::cpp_uno::shared::relatesToInterfaceType( pReturnTypeDescr )
                          ? alloca( pReturnTypeDescr->nSize )
                          : pCppReturn); // direct way
        }

        else
            pUnoReturn = pRegisterReturn; // direct way for simple types
    }
    // pop this
    gpreg++;
    ng++;

    // stack space
    OSL_ENSURE( sizeof(void *) == sizeof(sal_Int64), "### unexpected size!" );
    // parameters
    void ** pUnoArgs = (void **)alloca( 4 * sizeof(void *) * nParams );
    void ** pCppArgs = pUnoArgs + nParams;
    // indizes of values this have to be converted (interface conversion cpp<=>uno)
    sal_Int32 * pTempIndizes = (sal_Int32 *)(pUnoArgs + (2 * nParams));
    // type descriptions for reconversions
    typelib_TypeDescription ** ppTempParamTypeDescr = (typelib_TypeDescription **)(pUnoArgs + (3 * nParams));

    sal_Int32 nTempIndizes = 0;
    bool bOverFlowUsed = false;
    for ( sal_Int32 nPos = 0; nPos < nParams; ++nPos )
    {
        const typelib_MethodParameter & rParam = pParams[nPos];
        typelib_TypeDescription * pParamTypeDescr = 0;
        TYPELIB_DANGER_GET( &pParamTypeDescr, rParam.pTypeRef );

#if OSL_DEBUG_LEVEL > 2
        fprintf(stderr, "arg %d of %d\n", nPos, nParams);
#endif

        //I think it is impossible to get UNO to pass structs as parameters by copy
        if (!rParam.bOut && bridges::cpp_uno::shared::isSimpleType( pParamTypeDescr ))
        {
#if OSL_DEBUG_LEVEL > 2
            fprintf(stderr, "simple\n");
#endif

            switch (pParamTypeDescr->eTypeClass)
            {
                case typelib_TypeClass_FLOAT:
                    if (nf < ia64::MAX_SSE_REGS && ng < ia64::MAX_GPR_REGS)
                    {
                        float tmp = (float) (*((double *)fpreg));
                        (*((float *) fpreg)) = tmp;
                        pCppArgs[nPos] = pUnoArgs[nPos] = fpreg++;
                        nf++;
                        gpreg++;
                        ng++;
                    }
                    else
                    {
                        pCppArgs[nPos] = pUnoArgs[nPos] = ovrflw;
                        bOverFlowUsed = true;
                    }
                    if (bOverFlowUsed) ovrflw++;
                    break;
                case typelib_TypeClass_DOUBLE:
                    if (nf < ia64::MAX_SSE_REGS && ng < ia64::MAX_GPR_REGS)
                    {
                        pCppArgs[nPos] = pUnoArgs[nPos] = fpreg++;
                        nf++;
                        gpreg++;
                        ng++;
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
                case typelib_TypeClass_CHAR:
                case typelib_TypeClass_SHORT:
                case typelib_TypeClass_UNSIGNED_SHORT:
                case typelib_TypeClass_ENUM:
                case typelib_TypeClass_LONG:
                case typelib_TypeClass_UNSIGNED_LONG:
                default:
                    if (ng < ia64::MAX_GPR_REGS)
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

                // no longer needed
                TYPELIB_DANGER_RELEASE( pParamTypeDescr );
        }
        else // ptr to complex value | ref
        {
#if OSL_DEBUG_LEVEL > 2
            fprintf(stderr, "complex, ng is %d\n", ng);
#endif
            void *pCppStack; //temporary stack pointer

            if (ng < ia64::MAX_GPR_REGS)
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

            if (! rParam.bIn) // is pure out
            {
                // uno out is unconstructed mem!
                pUnoArgs[nPos] = alloca( pParamTypeDescr->nSize );
                pTempIndizes[nTempIndizes] = nPos;
                // will be released at reconversion
                ppTempParamTypeDescr[nTempIndizes++] = pParamTypeDescr;
            }
            // is in/inout
            else if (bridges::cpp_uno::shared::relatesToInterfaceType( pParamTypeDescr ))
            {
                uno_copyAndConvertData( pUnoArgs[nPos] = alloca( pParamTypeDescr->nSize ),
                                        pCppStack, pParamTypeDescr,
                                        pThis->getBridge()->getCpp2Uno() );
                pTempIndizes[nTempIndizes] = nPos; // has to be reconverted
                // will be released at reconversion
                ppTempParamTypeDescr[nTempIndizes++] = pParamTypeDescr;
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
        for ( ; nTempIndizes--; )
        {
            sal_Int32 nIndex = pTempIndizes[nTempIndizes];

            if (pParams[nIndex].bIn) // is in/inout => was constructed
                uno_destructData( pUnoArgs[nIndex], ppTempParamTypeDescr[nTempIndizes], 0 );
            TYPELIB_DANGER_RELEASE( ppTempParamTypeDescr[nTempIndizes] );
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
        for ( ; nTempIndizes--; )
        {
            sal_Int32 nIndex = pTempIndizes[nTempIndizes];
            typelib_TypeDescription * pParamTypeDescr = ppTempParamTypeDescr[nTempIndizes];

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
            typelib_TypeClass eRet = ia64::return_via_r8_buffer(pReturnTypeRef) ? typelib_TypeClass_VOID : (typelib_TypeClass)pReturnTypeDescr->eTypeClass;
            TYPELIB_DANGER_RELEASE( pReturnTypeDescr );
            return eRet;
        }
        else
            return typelib_TypeClass_VOID;
    }
}


//==================================================================================================
static typelib_TypeClass cpp_mediate(
    sal_uInt64 nOffsetAndIndex,
        void ** gpreg, void ** fpreg, long sp, long r8,
    sal_Int64 * pRegisterReturn /* space for register return */ )
{
    OSL_ENSURE( sizeof(sal_Int64)==sizeof(void *), "### unexpected!" );

    sal_Int32 nVtableOffset = (nOffsetAndIndex >> 32);
    sal_Int32 nFunctionIndex = (nOffsetAndIndex & 0xFFFFFFFF);

    void ** ovrflw = (void**)(sp);

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
    fprintf(stderr, "pThis is %p\n", pThis);
#endif

    pThis = static_cast< char * >(pThis) - nVtableOffset;

#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr, "pThis is now %p\n", pThis);
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

    // determine called method
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
                r8, gpreg, fpreg, ovrflw, pRegisterReturn );
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
}

extern "C" ia64::RegReturn cpp_vtable_call(
    long in0, long in1, long in2, long in3, long in4, long in5, long in6, long in7,
    long firstonstack
    )
{
    register long r15 asm("r15");
    long r8 = r15;

    register long r14 asm("r14");
    long nOffsetAndIndex = r14;

    long sp = (long)&firstonstack;

    sal_uInt64 gpreg[ia64::MAX_GPR_REGS];
    gpreg[0] = in0;
    gpreg[1] = in1;
    gpreg[2] = in2;
    gpreg[3] = in3;
    gpreg[4] = in4;
    gpreg[5] = in5;
    gpreg[6] = in6;
    gpreg[7] = in7;

    double fpreg[ia64::MAX_SSE_REGS];
    register double f8  asm("f8");  fpreg[0] =  f8;
    register double f9  asm("f9");  fpreg[1] =  f9;
    register double f10 asm("f10"); fpreg[2] = f10;
    register double f11 asm("f11"); fpreg[3] = f11;
    register double f12 asm("f12"); fpreg[4] = f12;
    register double f13 asm("f13"); fpreg[5] = f13;
    register double f14 asm("f14"); fpreg[6] = f14;
    register double f15 asm("f15"); fpreg[7] = f15;

#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr, "cpp_vtable_call called with %lx\n", nOffsetAndIndex);
    fprintf(stderr, "adump is %lx %lx %lx %lx %lx %lx %lx %lx\n", in0, in1, in2, in3, in4, in5, in6, in7);
    fprintf(stderr, "bdump is %f %f %f %f %f %f %f %f\n", f8, f9, f10, f11, f12, f13, f14, f15);
#endif

    volatile long nRegReturn[4] = { 0 };

    typelib_TypeClass aType =
        cpp_mediate( nOffsetAndIndex, (void**)gpreg, (void**)fpreg, sp, r8, (sal_Int64*)&nRegReturn[0]);

    ia64::RegReturn ret;
    switch( aType )
    {
        case typelib_TypeClass_VOID:
            break;
        case typelib_TypeClass_BOOLEAN:
        case typelib_TypeClass_BYTE:
        case typelib_TypeClass_CHAR:
        case typelib_TypeClass_UNSIGNED_SHORT:
        case typelib_TypeClass_SHORT:
        case typelib_TypeClass_ENUM:
        case typelib_TypeClass_UNSIGNED_LONG:
        case typelib_TypeClass_LONG:
        case typelib_TypeClass_UNSIGNED_HYPER:
        case typelib_TypeClass_HYPER:
            ret.r8 = nRegReturn[0];
            break;
        case typelib_TypeClass_FLOAT:
            asm volatile("ldfs f8=%0" : : "m"((*((float*)&nRegReturn))) : "f8");
            break;
        case typelib_TypeClass_DOUBLE:
            asm volatile("ldfd f8=%0" : : "m"((*((double*)&nRegReturn))) : "f8");
            break;
        case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
    {
            ret.r8 = nRegReturn[0];
            ret.r9 = nRegReturn[1];
            ret.r10 = nRegReturn[2];
            ret.r11 = nRegReturn[3];
            break;
    }
        default:
        break;
    }
    return ret;
}

namespace
{
const int codeSnippetSize = 40;

bridges::cpp_uno::shared::VtableFactory::Slot codeSnippet( unsigned char * code, sal_PtrDiff writetoexecdiff, sal_Int32 nFunctionIndex, sal_Int32 nVtableOffset,
                              bool bHasHiddenParam )
{
#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr, "size is %d\n", codeSnippetSize);
    fprintf(stderr,"in codeSnippet functionIndex is %x\n", nFunctionIndex);
    fprintf(stderr,"in codeSnippet vtableOffset is %x\n", nVtableOffset);
#endif

    sal_uInt64 nOffsetAndIndex = ( ( (sal_uInt64) nVtableOffset ) << 32 ) | ( (sal_uInt64) nFunctionIndex );

    if ( bHasHiddenParam )
        nOffsetAndIndex |= 0x80000000;

    long *raw = (long *)code;

    bridges::cpp_uno::shared::VtableFactory::Slot* destination = (bridges::cpp_uno::shared::VtableFactory::Slot*)cpp_vtable_call;

    raw[0] = (long)&privateSnippetExecutor;
    raw[1] = (long)&raw[2];
    raw[2] = nOffsetAndIndex;
    raw[3] = destination->gp_value;

    return *(bridges::cpp_uno::shared::VtableFactory::Slot*)(code+writetoexecdiff);
}
}

void bridges::cpp_uno::shared::VtableFactory::flushCode(unsigned char const *, unsigned char const *)
{
}

bridges::cpp_uno::shared::VtableFactory::Slot * bridges::cpp_uno::shared::VtableFactory::mapBlockToVtable(void * block)
{
    return static_cast< Slot * >(block) + 2;
}


sal_Size bridges::cpp_uno::shared::VtableFactory::getBlockSize(
    sal_Int32 slotCount)
{
    return (slotCount + 2) * sizeof (Slot) + slotCount * codeSnippetSize;
}

bridges::cpp_uno::shared::VtableFactory::Slot* bridges::cpp_uno::shared::VtableFactory::initializeBlock(void * block, sal_Int32 slotCount)
{
    Slot * slots = mapBlockToVtable(block);
    Slot foo = {0,0};
    slots[-2] = foo;
    slots[-1] = foo;
    return slots + slotCount;
}

unsigned char * bridges::cpp_uno::shared::VtableFactory::addLocalFunctions(
    Slot ** in_slots, unsigned char * code, sal_PtrDiff writetoexecdiff,
    typelib_InterfaceTypeDescription const * type, sal_Int32 functionOffset,
    sal_Int32 functionCount, sal_Int32 vtableOffset)
{
    (*in_slots) -= functionCount;
    Slot * slots = *in_slots;
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
            // Getter:
            *slots++ = codeSnippet(
                code, writetoexecdiff, functionOffset++, vtableOffset,
                ia64::return_in_hidden_param(
                    reinterpret_cast<
                    typelib_InterfaceAttributeTypeDescription * >(
                        member)->pAttributeTypeRef));
            code += codeSnippetSize;


            // Setter:
            if (!reinterpret_cast<
                typelib_InterfaceAttributeTypeDescription * >(
                    member)->bReadOnly)
            {
                *slots++ = codeSnippet(code, writetoexecdiff, functionOffset++, vtableOffset, false);
                code += codeSnippetSize;
            }
            break;

        case typelib_TypeClass_INTERFACE_METHOD:
            *slots++ = codeSnippet(
                code, writetoexecdiff, functionOffset++, vtableOffset,
                ia64::return_in_hidden_param(
                    reinterpret_cast<
                    typelib_InterfaceMethodTypeDescription * >(
                        member)->pReturnTypeRef));
            code += codeSnippetSize;
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
