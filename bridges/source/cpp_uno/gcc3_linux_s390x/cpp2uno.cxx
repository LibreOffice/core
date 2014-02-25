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
        if (bridges::cpp_uno::shared::isSimpleType( pReturnTypeDescr ))
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
    OSL_ENSURE( sizeof(void *) == sizeof(sal_Int64), "### unexpected size!" );
    // parameters
    void ** pUnoArgs = (void **)alloca( 4 * sizeof(void *) * nParams );
    void ** pCppArgs = pUnoArgs + nParams;
    // indices of values this have to be converted (interface conversion cpp<=>uno)
    sal_Int32 * pTempIndices = (sal_Int32 *)(pUnoArgs + (2 * nParams));
    // type descriptions for reconversions
    typelib_TypeDescription ** ppTempParamTypeDescr = (typelib_TypeDescription **)(pUnoArgs + (3 * nParams));

    sal_Int32 nTempIndices   = 0;
    for ( sal_Int32 nPos = 0; nPos < nParams; ++nPos )
    {
        const typelib_MethodParameter & rParam = pParams[nPos];
        typelib_TypeDescription * pParamTypeDescr = 0;
        TYPELIB_DANGER_GET( &pParamTypeDescr, rParam.pTypeRef );

#if OSL_DEBUG_LEVEL > 2
        fprintf(stderr, "arg %d of %d\n", nPos, nParams);
#endif

        if (!rParam.bOut && bridges::cpp_uno::shared::isSimpleType( pParamTypeDescr )) // value
        {
#if OSL_DEBUG_LEVEL > 2
            fprintf(stderr, "simple\n");
#endif

            switch (pParamTypeDescr->eTypeClass)
            {
                case typelib_TypeClass_FLOAT:
                case typelib_TypeClass_DOUBLE:
                    if (nf < s390x::MAX_SSE_REGS)
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
                        ovrflw++;
                    }
                    break;
                case typelib_TypeClass_BYTE:
                case typelib_TypeClass_BOOLEAN:
                    if (ng < s390x::MAX_GPR_REGS)
                    {
                        pCppArgs[nPos] = pUnoArgs[nPos] = (((char *)gpreg) + (sizeof(void*)-1));
                        ng++;
                        gpreg++;
                    }
                    else
                    {
                        pCppArgs[nPos] = pUnoArgs[nPos] = (((char *)ovrflw) + (sizeof(void*)-1));
                        ovrflw++;
                    }
                    break;
                case typelib_TypeClass_CHAR:
                case typelib_TypeClass_SHORT:
                case typelib_TypeClass_UNSIGNED_SHORT:
                    if (ng < s390x::MAX_GPR_REGS)
                    {
                        pCppArgs[nPos] = pUnoArgs[nPos] = (((char *)gpreg) + (sizeof(void*)-2));
                        ng++;
                        gpreg++;
                    }
                    else
                    {
                        pCppArgs[nPos] = pUnoArgs[nPos] = (((char *)ovrflw) + (sizeof(void*)-2));
                        ovrflw++;
                    }
                    break;
                case typelib_TypeClass_ENUM:
                case typelib_TypeClass_LONG:
                case typelib_TypeClass_UNSIGNED_LONG:
                    if (ng < s390x::MAX_GPR_REGS)
                    {
                        pCppArgs[nPos] = pUnoArgs[nPos] = (((char *)gpreg) + (sizeof(void*)-4));
                        ng++;
                        gpreg++;
                    }
                    else
                    {
                        pCppArgs[nPos] = pUnoArgs[nPos] = (((char *)ovrflw) + (sizeof(void*)-4));
                        ovrflw++;
                    }
                    break;
                default:
                    if (ng < s390x::MAX_GPR_REGS)
                    {
                        pCppArgs[nPos] = pUnoArgs[nPos] = gpreg++;
                        ng++;
                    }
                    else
                    {
                        pCppArgs[nPos] = pUnoArgs[nPos] = ovrflw;
                        ovrflw++;
                    }
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

            if (ng < s390x::MAX_GPR_REGS)
            {
                pCppArgs[nPos] = pCppStack = *gpreg++;
                ng++;
            }
            else
            {
                pCppArgs[nPos] = pCppStack = *ovrflw;
                ovrflw++;
            }

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

        CPPU_CURRENT_NAMESPACE::raiseException( &aUnoExc, pThis->getBridge()->getUno2Cpp() ); // has to destruct the any
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


static typelib_TypeClass cpp_mediate(
    sal_uInt64 nOffsetAndIndex,
    void ** gpreg, void ** fpreg, void ** ovrflw,
    sal_Int64 * pRegisterReturn /* space for register return */ )
{
    OSL_ENSURE( sizeof(sal_Int64)==sizeof(void *), "### unexpected!" );

    sal_Int32 nVtableOffset = (nOffsetAndIndex >> 32);
    sal_Int32 nFunctionIndex = (nOffsetAndIndex & 0xFFFFFFFF);

#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr, "nVTableOffset, nFunctionIndex are %x %x\n", nVtableOffset, nFunctionIndex);
#endif

#if OSL_DEBUG_LEVEL > 2
        // Let's figure out what is really going on here
        {
            fprintf( stderr, "= cpp_mediate () =\nGPR's (%d): ", 5 );
            for ( unsigned int i = 0; i < 5; ++i )
                fprintf( stderr, "0x%lx, ", gpreg[i] );
            fprintf( stderr, "\n");
            fprintf( stderr, "\nFPR's (%d): ", 4 );
            for ( unsigned int i = 0; i < 4; ++i )
                fprintf( stderr, "0x%lx (%f), ", fpreg[i], fpreg[i] );
            fprintf( stderr, "\n");
        }
#endif


    // gpreg:  [ret *], this, [other gpr params]
    // fpreg:  [fpr params]
    // ovrflw: [gpr or fpr params (properly aligned)]

    // _this_ ptr is patched cppu_XInterfaceProxy object
    void * pThis;
    if( nFunctionIndex & 0x80000000 )
    {
        nFunctionIndex &= 0x7fffffff;
        pThis = gpreg[1];
    }
    else
    {
        pThis = gpreg[0];
    }

    pThis = static_cast< char * >(pThis) - nVtableOffset;

    bridges::cpp_uno::shared::CppInterfaceProxy * pCppI
        = bridges::cpp_uno::shared::CppInterfaceProxy::castInterfaceToProxy(
            pThis);

    typelib_InterfaceTypeDescription * pTypeDescr = pCppI->getTypeDescr();


    OSL_ENSURE( nFunctionIndex < pTypeDescr->nMapFunctionIndexToMemberIndex, "### illegal vtable index!" );
    if (nFunctionIndex >= pTypeDescr->nMapFunctionIndexToMemberIndex)
    {
        throw RuntimeException(
            OUString( "illegal vtable index!" ),
            (XInterface *)pCppI );
    }

    // determine called method
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
        throw RuntimeException(
            OUString( "no member description found!" ),
            (XInterface *)pCppI );
    }
    }

    return eRet;
}

long privateSnippetExecutor(long r2, long r3, long r4, long r5, long r6, long firstonstack)
{
    register long r0 asm("r0");
    sal_uInt64 nOffsetAndIndex = r0;

    long sp = (long)&firstonstack;

    sal_uInt64 gpreg[s390x::MAX_GPR_REGS];
    gpreg[0] = r2;
    gpreg[1] = r3;
    gpreg[2] = r4;
    gpreg[3] = r5;
    gpreg[4] = r6;

    double fpreg[s390x::MAX_SSE_REGS];
    register double f0  asm("f0");  fpreg[0] = f0;
    register double f2  asm("f2");  fpreg[1] = f2;
    register double f4  asm("f4");  fpreg[2] = f4;
    register double f6  asm("f6");  fpreg[3] = f6;

    volatile long nRegReturn[1];
#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr, "before mediate with %lx\n",nOffsetAndIndex);
    fprintf(stderr, "doubles are %f %f %f %f\n", fpreg[0], fpreg[1], fpreg[2], fpreg[3]);
#endif
    typelib_TypeClass aType =
        cpp_mediate( nOffsetAndIndex, (void**)gpreg, (void**)fpreg, (void**)sp,
            (sal_Int64*)nRegReturn );
#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr, "after mediate ret is %lx %ld\n", nRegReturn[0], nRegReturn[0]);
#endif

    switch( aType )
    {
        case typelib_TypeClass_BOOLEAN:
        case typelib_TypeClass_BYTE:
            nRegReturn[0] = (unsigned long)(*(unsigned char *)nRegReturn);
            break;
        case typelib_TypeClass_CHAR:
        case typelib_TypeClass_UNSIGNED_SHORT:
        case typelib_TypeClass_SHORT:
            nRegReturn[0] = (unsigned long)(*(unsigned short *)nRegReturn);
            break;
        case typelib_TypeClass_ENUM:
        case typelib_TypeClass_UNSIGNED_LONG:
        case typelib_TypeClass_LONG:
            nRegReturn[0] = (unsigned long)(*(unsigned int *)nRegReturn);
            break;
        case typelib_TypeClass_VOID:
        default:
            break;
        case typelib_TypeClass_FLOAT:
            {
                double tmp = (double) (*((float *)nRegReturn));
                (*((double *) nRegReturn)) = tmp;
            }
            //deliberate fall through
        case typelib_TypeClass_DOUBLE:
            __asm__ ( "ld 0,%0\n\t"
                : : "m" (*((double*)nRegReturn)) );
            break;
    }
    return nRegReturn[0];
}

const int codeSnippetSize = 32;

unsigned char *codeSnippet( unsigned char * code, sal_Int32 nFunctionIndex, sal_Int32 nVtableOffset, bool simple_ret_type )
{
    sal_uInt64 nOffsetAndIndex = ( ( (sal_uInt64) nVtableOffset ) << 32 ) | ( (sal_Int64) nFunctionIndex );

    if (! simple_ret_type)
        nOffsetAndIndex |= 0x80000000;

    unsigned char * p = code;
    *(short *)&p[0] = 0x0d10;   /* basr %r1,0 */
    *(short *)&p[2] = 0xeb01;   /* lmg %r0,%r1,14(%r1) */
    *(short *)&p[4] = 0x100e;
    *(short *)&p[6] = 0x0004;
    *(short *)&p[8] = 0x07f1;   /* br %r1 */
    *(long  *)&p[16] = (long)nOffsetAndIndex;
    *(long  *)&p[24] = (long)&privateSnippetExecutor;
    return (code + codeSnippetSize);
}
}

void bridges::cpp_uno::shared::VtableFactory::flushCode(unsigned char const *, unsigned char const *)
{
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
            // Getter:
            (s++)->fn = code + writetoexecdiff;
            code = codeSnippet(
                code, functionOffset++, vtableOffset,
                bridges::cpp_uno::shared::isSimpleType(
                    reinterpret_cast<
                    typelib_InterfaceAttributeTypeDescription * >(
                        member)->pAttributeTypeRef));

            // Setter:
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
