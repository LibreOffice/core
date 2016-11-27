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
#include <com/sun/star/uno/genfunc.hxx>
#include <uno/data.h>

#include "bridge.hxx"
#include "types.hxx"
#include "unointerfaceproxy.hxx"
#include "vtables.hxx"

#include "share.hxx"

#define SET_FP(n, p) \
            __asm__( "ldx %0, %%l0\n\t" \
                     "ldd [%%l0], %%f" #n "\n" \
                     : : "m"(p) );

using namespace com::sun::star::uno;

namespace
{
    void fillReturn(const typelib_TypeDescription * pTypeDescr,
            long long * oret, float * fret, double * dret, void * pRegisterReturn)
    {
        for (const typelib_CompoundTypeDescription *p =
                reinterpret_cast<const typelib_CompoundTypeDescription*>( pTypeDescr );
            p != NULL; p = p->pBaseTypeDescription)
        {

            for (sal_Int32 i = 0; i < p->nMembers; ++i)
            {
                typelib_TypeDescriptionReference *pTypeInStruct = p->ppTypeRefs[ i ];

                sal_Int32 nOff = p->pMemberOffsets[ i ];

                switch (pTypeInStruct->eTypeClass)
                {
                    case typelib_TypeClass_HYPER:
                    case typelib_TypeClass_UNSIGNED_HYPER:
                        *(long*)((char *)pRegisterReturn + nOff) = *(long*)((char *)oret + nOff);
                        break;
                    case typelib_TypeClass_LONG:
                    case typelib_TypeClass_UNSIGNED_LONG:
                    case typelib_TypeClass_ENUM:
                        *(int*)((char *)pRegisterReturn + nOff) = *(int*)((char *)oret + nOff);
                        break;
                    case typelib_TypeClass_CHAR:
                    case typelib_TypeClass_SHORT:
                    case typelib_TypeClass_UNSIGNED_SHORT:
                        *(unsigned short*)((char *)pRegisterReturn + nOff) = *(unsigned short*)((char *)oret + nOff);
                        break;
                    case typelib_TypeClass_BOOLEAN:
                    case typelib_TypeClass_BYTE:
                        *(unsigned char*)((char *)pRegisterReturn + nOff) = *(unsigned char*)((char *)oret + nOff);
                        break;
                    case typelib_TypeClass_FLOAT:
                        *(float*)((char *)pRegisterReturn + nOff) = *(float*)((char *)fret + nOff);
                        break;
                    case typelib_TypeClass_DOUBLE:
                        *(double*)((char *)pRegisterReturn + nOff) = *(double*)((char *)dret + nOff);
                        break;
                    case typelib_TypeClass_STRUCT:
                    case typelib_TypeClass_EXCEPTION:
                        {
                            typelib_TypeDescription * td = NULL;
                            TYPELIB_DANGER_GET(&td, pTypeInStruct);
                            fillReturn(td,
                                    (long long *)((char *)oret + nOff),
                                    (float *)((char *)fret + nOff),
                                    (double *)((char *)dret + nOff),
                                    (char *)pRegisterReturn + nOff);
                            TYPELIB_DANGER_RELEASE(td);
                        }
                    default:
                        break;
                }
            }
        }
    }

// The call instruction within the asm section of callVirtualMethod may throw
// exceptions.  So that the compiler handles this correctly, it is important
// that (a) callVirtualMethod might call dummy_can_throw_anything (although this
// never happens at runtime), which in turn can throw exceptions, and (b)
// callVirtualMethod is not inlined at its call site (so that any exceptions are
// caught which are thrown from the instruction calling callVirtualMethod):

void callVirtualMethod( void * pAdjustedThisPtr,
                        sal_Int32 nVtableIndex,
                        void * pRegisterReturn,
                        typelib_TypeDescriptionReference * pReturnTypeRef,
                        sal_Int64 * pStackHypers,
                        sal_Int32 nStackHypers,
                        typelib_MethodParameter * pParams, sal_Int32 nParams) __attribute__((noinline));

void callVirtualMethod( void * pAdjustedThisPtr,
                        sal_Int32 /* nVtableIndex */,
                        void * pRegisterReturn,
                        typelib_TypeDescriptionReference * pReturnTypeRef,
                        sal_Int64 * pStackHypers,
#if OSL_DEBUG_LEVEL > 0
                        sal_Int32 nStackHypers,
#else
//                        sal_Int64 * /*pStackHypers*/,
                        sal_Int32 /*nStackHypers*/,
#endif
                        typelib_MethodParameter * pParams, sal_Int32 nParams)
{
    // parameter list is mixed list of * and values
    // reference parameters are pointers

    assert(pStackHypers && pAdjustedThisPtr);
    static_assert( (sizeof(void *) == 8) &&
                 (sizeof(sal_Int64) == 8), "### unexpected size of int!" );
    assert(nStackHypers && pStackHypers && "### no stack in callVirtualMethod !");

    // never called
    if (! pAdjustedThisPtr) CPPU_CURRENT_NAMESPACE::dummy_can_throw_anything("xxx"); // address something

    long bSimpleReturn = !CPPU_CURRENT_NAMESPACE::return_in_hidden_param( pReturnTypeRef );

    int paramsOffset = bSimpleReturn ? 1 : 2;
    for (sal_Int32 i = 0; i < nParams; ++i)
    {
        if (!pParams[i].bOut)
        {
            switch (pParams[i].pTypeRef->eTypeClass) {
                case typelib_TypeClass_FLOAT:
                case typelib_TypeClass_DOUBLE:
                    {
                        int paramArrayIdx = i + paramsOffset;
                        assert(paramArrayIdx < nStackHypers);
                        void *p = &pStackHypers[paramArrayIdx];
                        switch (paramArrayIdx) {
                            // Cannot be 0 - paramsOffset >= 1
                            case 1:
                                SET_FP(2, p);
                                break;
                            case 2:
                                SET_FP(4, p);
                                break;
                            case 3:
                                SET_FP(6, p);
                                break;
                            case 4:
                                SET_FP(8, p);
                                break;
                            case 5:
                                SET_FP(10, p);
                                break;
                            case 6:
                                SET_FP(12, p);
                                break;
                            case 7:
                                SET_FP(14, p);
                                break;
                            case 8:
                                SET_FP(16, p);
                                break;
                            case 9:
                                SET_FP(18, p);
                                break;
                            case 10:
                                SET_FP(20, p);
                                break;
                            case 11:
                                SET_FP(22, p);
                                break;
                            case 12:
                                SET_FP(24, p);
                                break;
                            case 13:
                                SET_FP(26, p);
                                break;
                            case 14:
                                SET_FP(28, p);
                                break;
                            case 15:
                                SET_FP(30, p);
                                break;
                                // Anything larger is passed on the stack
                        }
                        break;
                    }
                default:
                    break;
            }
        }
    }

    //long o0;
    //double f0d;
    //float f0f;
    volatile long long saveReg[14];

    long long oret[4];
    union {
        float f[8];
        double d[4];
    } fdret;

    __asm__ (
        // save registers
        "stx %%l0, [%[saveReg]]\n\t"
        "stx %%l1, [%[saveReg]+8]\n\t"
        "mov %[saveReg], %%l1\n\t"
        "add %%l1, 16, %%l0\n\t"
        "stx %%l2, [%%l0]\n\t"
        "add %%l0, 8, %%l0\n\t"
        "stx %%l3, [%%l0]\n\t"
        "add %%l0, 8, %%l0\n\t"
        "stx %%l4, [%%l0]\n\t"
        "add %%l0, 8, %%l0\n\t"
        "stx %%l5, [%%l0]\n\t"
        "add %%l0, 8, %%l0\n\t"
        "stx %%o0, [%%l0]\n\t"
        "add %%l0, 8, %%l0\n\t"
        "stx %%o1, [%%l0]\n\t"
        "add %%l0, 8, %%l0\n\t"
        "stx %%o2, [%%l0]\n\t"
        "add %%l0, 8, %%l0\n\t"
        "stx %%o3, [%%l0]\n\t"
        "add %%l0, 8, %%l0\n\t"
        "stx %%o4, [%%l0]\n\t"
        "add %%l0, 8, %%l0\n\t"
        "stx %%o5, [%%l0]\n\t"
        "add %%l0, 8, %%l0\n\t"
        "stx %%l6, [%%l0]\n\t"
        "add %%l0, 8, %%l0\n\t"
        "stx %%l7, [%%l0]\n\t"
        "mov %%l1, %%l7\n\t"

        // increase our own stackframe if necessary
        "mov %%sp, %%l3\n\t"        // save stack ptr for readjustment

        "subcc %%i5, 6, %%l0\n\t"
        "ble .LmoveOn\n\t"
        "nop\n\t"

        "sllx %%l0, 3, %%l0\n\t"
        "add %%l0, 192, %%l0\n\t"
        "add %%sp, 2047, %%l1\n\t"        // old stack ptr
        "sub %%l1, %%l0, %%l0\n\t"  // future stack ptr
        "andcc %%l0, 15, %%g0\n\t"   // align stack to 16
        "be .LisAligned\n\t"
        "nop\n\t"
        "sub %%l0, 8, %%l0\n"
    ".LisAligned:\n\t"
        "mov %%l0, %%o5\n\t"            // save newly computed stack ptr
        "add %%g0, 16, %%o4\n"

        // now copy hypers down to save register window
        // and local variables
    ".LcopyDown:\n\t"
        "ldx [%%l1], %%l2\n\t"
        "stx %%l2,[%%l0]\n\t"
        "add %%l0, 8, %%l0\n\t"
        "add %%l1, 8, %%l1\n\t"
        "subcc %%o4, 1, %%o4\n\t"
        "bne .LcopyDown\n\t"
        "nop\n\t"

        "sub %%o5, 2047, %%sp\n\t"        // move new stack ptr (hopefully) atomically - with bias
        // while register window is valid in both spaces
        // (scheduling might hit in copyDown loop)

        "sub %%i5, 6, %%l0\n\t"     // copy parameters past the sixth to stack
        "add %%i4, 48, %%l1\n\t"
        "add %%sp, 2223, %%l2\n" // 2047+176
    ".LcopyLong:\n\t"
        "ldx [%%l1], %%o0\n\t"
        "stx %%o0, [%%l2]\n\t"
        "add %%l1, 8, %%l1\n\t"
        "add %%l2, 8, %%l2\n\t"
        "subcc %%l0, 1, %%l0\n\t"
        "bne .LcopyLong\n\t"
        "nop\n"

    ".LmoveOn:\n\t"
        "mov %%i5, %%l0\n\t"        // prepare out registers
        "mov %%i4, %%l1\n\t"

        "ldx [%%l1], %%o0\n\t"//       // prepare complex return ptr
        //"ldd [%%l1], %%f0\n\t"
        //"stx %%o0, [%%sp+2047+128]\n\t"
        "sub %%l0, 1, %%l0\n\t"
        "add %%l1, 8, %%l1\n\t"
        //"subxcc %%o0, %%g0, %%o0\n\t"
        //"bne .LhadComplexReturn\n\t"
        //"nop\n\t"

        // No complex return ptr - this (next on stack) goes in %o0

        //"ldx [%%l1], %%o0\n\t"
        //"subcc %%l0, 1, %%l0\n\t"
        //"be .LdoCall\n\t"
        //"nop\n\t"
        //"add %%l1, 8, %%l1\n\t"

    //".LhadComplexReturn:\n\t"
        "ldx [%%l1], %%o1\n\t"
        //"ldd [%%l1], %%f2\n\t"
        "subcc %%l0, 1, %%l0\n\t"
        "be .LdoCall\n\t"
        "nop\n\t"

        "add %%l1, 8, %%l1\n\t"
        "ldx [%%l1], %%o2\n\t"
        //"ldd [%%l1], %%f4\n\t"
        "subcc %%l0, 1, %%l0\n\t"
        "be .LdoCall\n\t"
        "nop\n\t"

        "add %%l1, 8, %%l1\n\t"
        "ldx [%%l1], %%o3\n\t"
        //"ldd [%%l1], %%f6\n\t"
        "subcc %%l0, 1, %%l0\n\t"
        "be .LdoCall\n\t"
        "nop\n\t"

        "add %%l1, 8, %%l1\n\t"
        "ldx [%%l1], %%o4\n\t"
        //"ldd [%%l1], %%f8\n\t"
        "subcc %%l0, 1, %%l0\n\t"
        "be .LdoCall\n\t"
        "nop\n\t"

        "add %%l1, 8, %%l1\n\t"
        "ldx [%%l1], %%o5\n"
        //"ldd [%%l1], %%f10\n\t"

    ".LdoCall:\n\t"
        "ldx [%%i0], %%l0\n\t"       // get vtable ptr

"sllx %%i1, 3, %%l6\n\t"
//        "add %%l6, 8, %%l6\n\t"
        "add %%l6, %%l0, %%l0\n\t"
//      // vtable has 8byte wide entries,
//      // upper half contains 2 half words, of which the first
//      // is the this ptr patch !
//      // first entry is (or __tf)

//      "ldsh [%%l0], %%l6\n\t"     // load this ptr patch
//      "add %%l6, %%o0, %%o0\n\t"  // patch this ptr

//      "add %%l0, 4, %%l0\n\t"     // get virtual function ptr
        "ldx [%%l0], %%l0\n\t"

//      "ldx %0, %%l2\n\t"
//      "subcc %%l2, %%g0, %%l2\n\t"
//      "be .LcomplexCall\n\t"
//      "nop\n\t"
        "call %%l0\n\t"
        "nop\n\t"
//      "ba .LcallReturned\n\t"
//      "nop\n"
//  ".LcomplexCall:\n\t"
//      "call %%l0\n\t"
//      "nop\n\t"
//      "unimp\n"

//  ".LcallReturned:\n\t"
        "subcc %%l3, %%sp, %%g0\n\t"
        "be .LcopiedUp\n\t"
        "nop\n\t"
    // Copy register save area back up
    // Note: copy in reverse order (top down) in case areas overlap
        "add %%sp, 2167, %%l0\n\t" // 2047+120
        "add %%l3, 2167, %%l1\n\t"
        "add %%g0, 16, %%o4\n\t"
    ".LcopyUp:\n\t"
        "ldx [%%l0], %%l2\n\t"
        "stx %%l2, [%%l1]\n\t"
        "sub %%l0, 8, %%l0\n\t"
        "sub %%l1, 8, %%l1\n\t"
        "subcc %%o4, 1, %%o4\n\t"
        "bne .LcopyUp\n\t"
        "nop\n\t"

    ".LcopiedUp:\n\t"
        "mov %%l3, %%sp\n\t"        // readjust stack so that our locals are where they belong

        // save possible return registers into our locals
        "stx %%o0, %[oret0]\n\t"
        "stx %%o1, %[oret1]\n\t"
        "stx %%o2, %[oret2]\n\t"
        "stx %%o3, %[oret3]\n\t"
        "std %%f0, %[dret0]\n\t"
        "std %%f2, %[dret1]\n\t"
        "std %%f4, %[dret2]\n\t"
        "std %%f6, %[dret3]\n\t"
        //"st %%f0, %3\n\t"

        // restore registers
        "ldx [%%l7], %%l0\n\t"
        "add %%l7, 8, %%l7\n\t"
        "ldx [%%l7], %%l1\n\t"
        "add %%l7, 8, %%l7\n\t"
        "ldx [%%l7], %%l2\n\t"
        "add %%l7, 8, %%l7\n\t"
        "ldx [%%l7], %%l3\n\t"
        "add %%l7, 8, %%l7\n\t"
        "ldx [%%l7], %%l4\n\t"
        "add %%l7, 8, %%l7\n\t"
        "ldx [%%l7], %%l5\n\t"
        "add %%l7, 8, %%l7\n\t"
        "ldx [%%l7], %%o0\n\t"
        "add %%l7, 8, %%l7\n\t"
        "ldx [%%l7], %%o1\n\t"
        "add %%l7, 8, %%l7\n\t"
        "ldx [%%l7], %%o2\n\t"
        "add %%l7, 8, %%l7\n\t"
        "ldx [%%l7], %%o3\n\t"
        "add %%l7, 8, %%l7\n\t"
        "ldx [%%l7], %%o4\n\t"
        "add %%l7, 8, %%l7\n\t"
        "ldx [%%l7], %%o5\n\t"
        "add %%l7, 8, %%l7\n\t"
        "ldx [%%l7], %%l6\n\t"
        "add %%l7, 8, %%l7\n\t"
        "ldx [%%l7], %%l7\n\t"
        :
        //"=m"(bSimpleReturn),
        [oret0]"=m"(oret[0]), [oret1]"=m"(oret[1]), [oret2]"=m"(oret[2]), [oret3]"=m"(oret[3]),
        [dret0]"=m"(fdret.d[0]), [dret1]"=m"(fdret.d[1]), [dret2]"=m"(fdret.d[2]), [dret3]"=m"(fdret.d[3])
        //"=m"(f0f)
        :
        [saveReg]"r"(&saveReg[0])
        :
        "memory"
        );
    switch(pReturnTypeRef->eTypeClass)
    {
        case typelib_TypeClass_HYPER:
        case typelib_TypeClass_UNSIGNED_HYPER:
            *(long*)pRegisterReturn = oret[0];
            break;
        case typelib_TypeClass_LONG:
        case typelib_TypeClass_UNSIGNED_LONG:
        case typelib_TypeClass_ENUM:
            *(int*)pRegisterReturn = (int)oret[0];
            break;
        case typelib_TypeClass_CHAR:
        case typelib_TypeClass_SHORT:
        case typelib_TypeClass_UNSIGNED_SHORT:
            *(unsigned short*)pRegisterReturn = (unsigned short)oret[0];
            break;
        case typelib_TypeClass_BOOLEAN:
        case typelib_TypeClass_BYTE:
            *(unsigned char*)pRegisterReturn = (unsigned char)oret[0];
            break;
        case typelib_TypeClass_FLOAT:
            *(float*)pRegisterReturn = fdret.f[0];
            break;
        case typelib_TypeClass_DOUBLE:
            *(double*)pRegisterReturn = fdret.d[0];
            break;
        case typelib_TypeClass_STRUCT:
        case typelib_TypeClass_EXCEPTION:
            {
                sal_Int32 const nRetSize = pReturnTypeRef->pType->nSize;
                if (bSimpleReturn && nRetSize <= 32 && nRetSize > 0)
                {
                    typelib_TypeDescription * pTypeDescr = 0;
                    TYPELIB_DANGER_GET( &pTypeDescr, pReturnTypeRef );
                    fillReturn(pTypeDescr, oret, fdret.f, fdret.d, pRegisterReturn);
                    TYPELIB_DANGER_RELEASE( pTypeDescr );
                }
                break;
            }
        default:
            break;
    }
}

static void cpp_call(
    bridges::cpp_uno::shared::UnoInterfaceProxy * pThis,
    bridges::cpp_uno::shared::VtableSlot aVtableSlot,
    typelib_TypeDescriptionReference * pReturnTypeRef,
    sal_Int32 nParams, typelib_MethodParameter * pParams,
    void * pUnoReturn, void * pUnoArgs[], uno_Any ** ppUnoExc )
{
      // max space for: complex ret ptr, this, values|ptr ...
      char * pCppStack  =
          (char *)alloca( (nParams+2) * sizeof(sal_Int64) );
      char * pCppStackStart = pCppStack;

    //fprintf(stderr, "pCppStack: %p, pCppStackStart: %p\n", pCppStack, pCppStackStart);

    // return
    typelib_TypeDescription * pReturnTypeDescr = 0;
    TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );
    assert(pReturnTypeDescr);

    void * pCppReturn = 0; // if != 0 && != pUnoReturn, needs reconversion

    if (pReturnTypeDescr)
    {
        if ( CPPU_CURRENT_NAMESPACE::return_in_hidden_param( pReturnTypeRef ) )
        {
            // complex return via ptr
            pCppReturn = *(void **)pCppStack = (bridges::cpp_uno::shared::relatesToInterfaceType(pReturnTypeDescr )
                                                ? alloca( pReturnTypeDescr->nSize )
                                                : pUnoReturn); // direct way
            pCppStack += sizeof(void*);
        }
        else
        {
            pCppReturn = pUnoReturn; // direct way for simple types
        }
    }
    // push this
    void * pAdjustedThisPtr = reinterpret_cast< void ** >(pThis->getCppI())
             + aVtableSlot.offset;
    *(void**)pCppStack = pAdjustedThisPtr;
    pCppStack += sizeof( void* );

    // stack space
    static_assert(sizeof(void *) == sizeof(sal_Int64), "### unexpected size!");
    // args
    void ** pCppArgs  = (void **)alloca( 3 * sizeof(void *) * nParams );
    // indices of values this have to be converted (interface conversion cpp<=>uno)
    sal_Int32 * pTempIndices = (sal_Int32 *)(pCppArgs + nParams);
    // type descriptions for reconversions
    typelib_TypeDescription ** ppTempParamTypeDescr = (typelib_TypeDescription **)(pCppArgs + (2 * nParams));

    sal_Int32 nTempIndices   = 0;

    for ( sal_Int32 nPos = 0; nPos < nParams; ++nPos )
    {
        const typelib_MethodParameter & rParam = pParams[nPos];
        typelib_TypeDescription * pParamTypeDescr = 0;
        TYPELIB_DANGER_GET( &pParamTypeDescr, rParam.pTypeRef );
        if (!rParam.bOut && bridges::cpp_uno::shared::isSimpleType( pParamTypeDescr ))
        {
            uno_copyAndConvertData( pCppArgs[nPos] = alloca( 8 ), pUnoArgs[nPos], pParamTypeDescr,
                    pThis->getBridge()->getUno2Cpp() );

            switch (pParamTypeDescr->eTypeClass)
            {
                case typelib_TypeClass_LONG:
                    *(sal_Int64 *)pCppStack = *(sal_Int32 *)pCppArgs[nPos];
                    break;
                case typelib_TypeClass_UNSIGNED_LONG:
                case typelib_TypeClass_ENUM:
                    *(sal_Int64 *)pCppStack = *(sal_uInt32 *)pCppArgs[nPos];
                    break;
                case typelib_TypeClass_CHAR:
                case typelib_TypeClass_SHORT:
                    *(sal_Int64 *)pCppStack = *(sal_Int16 *)pCppArgs[nPos];
                    break;
                case typelib_TypeClass_UNSIGNED_SHORT:
                    *(sal_Int64 *)pCppStack = *(sal_uInt16 *)pCppArgs[nPos];
                    break;
                case typelib_TypeClass_BOOLEAN:
                case typelib_TypeClass_BYTE:
                    *(sal_Int64 *)pCppStack = *(sal_Int8 *)pCppArgs[nPos];
                    break;
                case typelib_TypeClass_FLOAT:
                    *(float *)(pCppStack+4) = *(float *)pCppArgs[nPos];
                    break;
                case typelib_TypeClass_DOUBLE:
                    *(double *)pCppStack = *(double *)pCppArgs[nPos];
                    break;
                case typelib_TypeClass_HYPER:
                case typelib_TypeClass_UNSIGNED_HYPER:
                    *(sal_Int64 *)pCppStack = *(sal_Int64 *)pCppArgs[nPos];
                    break;
                default:
                    break;
            }
            // no longer needed
            TYPELIB_DANGER_RELEASE( pParamTypeDescr );
        }
        else // ptr to complex value | ref
        {
            if (! rParam.bIn) // is pure out
            {
                // cpp out is constructed mem, uno out is not!
                uno_constructData(
                    *(void **)pCppStack = pCppArgs[nPos] = alloca( pParamTypeDescr->nSize ),
                    pParamTypeDescr );
                pTempIndices[nTempIndices] = nPos; // default constructed for cpp call
                // will be released at reconversion
                ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
            }
            // is in/inout
            else if (bridges::cpp_uno::shared::relatesToInterfaceType(
                              pParamTypeDescr ))
            {
                uno_copyAndConvertData(
                    *(void **)pCppStack = pCppArgs[nPos] = alloca( pParamTypeDescr->nSize ),
                                    pUnoArgs[nPos], pParamTypeDescr,
                    pThis->getBridge()->getUno2Cpp() );

                pTempIndices[nTempIndices] = nPos; // has to be reconverted
                // will be released at reconversion
                ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
            }
            else // direct way
            {
                *(void **)pCppStack = pCppArgs[nPos] = pUnoArgs[nPos];
                // no longer needed
                TYPELIB_DANGER_RELEASE( pParamTypeDescr );
            }
        }
        pCppStack += sizeof(sal_Int64); // standard parameter length
    }

    try
    {
        int nStackHypers = (pCppStack - pCppStackStart)/sizeof(sal_Int64);
        assert( !( (pCppStack - pCppStackStart ) & 7) && "UNALIGNED STACK !!! (Please DO panic" );

        //fprintf( stderr, "callVirtualMethod: %p, %lld, %p, %p, %p, %lld\n",
        //    pAdjustedThisPtr,
        //    (long long)aVtableSlot.index,
        //    pCppReturn,
        //    pReturnTypeRef,
        //    pCppStackStart,
        //    (long long)nStackHypers);
        callVirtualMethod(
            pAdjustedThisPtr,
            aVtableSlot.index,
            pCppReturn,
            pReturnTypeRef,
            (sal_Int64 *)pCppStackStart,
            nStackHypers,
            pParams,
            nParams);
        // NO exception occurred...
        *ppUnoExc = 0;

        // reconvert temporary params
        for ( ; nTempIndices--; )
        {
            sal_Int32 nIndex = pTempIndices[nTempIndices];
            typelib_TypeDescription * pParamTypeDescr = ppTempParamTypeDescr[nTempIndices];

            if (pParams[nIndex].bIn)
            {
                if (pParams[nIndex].bOut) // inout
                {
                    uno_destructData( pUnoArgs[nIndex], pParamTypeDescr, 0 ); // destroy uno value
                    uno_copyAndConvertData( pUnoArgs[nIndex], pCppArgs[nIndex], pParamTypeDescr,
                                            pThis->getBridge()->getCpp2Uno() );
                }
            }
            else // pure out
            {
                uno_copyAndConvertData( pUnoArgs[nIndex], pCppArgs[nIndex], pParamTypeDescr,
                                        pThis->getBridge()->getCpp2Uno() );
            }
            // destroy temp cpp param => cpp: every param was constructed
            uno_destructData( pCppArgs[nIndex], pParamTypeDescr, cpp_release );

            TYPELIB_DANGER_RELEASE( pParamTypeDescr );
        }
        // return value
        if (pCppReturn && pUnoReturn != pCppReturn)
        {
            uno_copyAndConvertData( pUnoReturn, pCppReturn, pReturnTypeDescr,
                                    pThis->getBridge()->getCpp2Uno() );
            uno_destructData( pCppReturn, pReturnTypeDescr, cpp_release );
        }
    }
     catch( ... )
     {
         // get exception
           fillUnoException( CPPU_CURRENT_NAMESPACE::__cxa_get_globals()->caughtExceptions,
                                *ppUnoExc, pThis->getBridge()->getCpp2Uno() );

        // temporary params
        for ( ; nTempIndices--; )
        {
            sal_Int32 nIndex = pTempIndices[nTempIndices];
            // destroy temp cpp param => cpp: every param was constructed
            uno_destructData( pCppArgs[nIndex], ppTempParamTypeDescr[nTempIndices], cpp_release );
            TYPELIB_DANGER_RELEASE( ppTempParamTypeDescr[nTempIndices] );
        }
        // return type
        if (pReturnTypeDescr)
            TYPELIB_DANGER_RELEASE( pReturnTypeDescr );
    }
}

}

namespace bridges { namespace cpp_uno { namespace shared {

void unoInterfaceProxyDispatch(
    uno_Interface * pUnoI, const typelib_TypeDescription * pMemberDescr,
    void * pReturn, void * pArgs[], uno_Any ** ppException )
{
#if defined BRIDGES_DEBUG
    OString cstr( OUStringToOString( pMemberDescr->pTypeName, RTL_TEXTENCODING_ASCII_US ) );
    fprintf( stderr, "received dispatch( %s )\n", cstr.getStr() );
#endif

    // is my surrogate
    bridges::cpp_uno::shared::UnoInterfaceProxy * pThis
       = static_cast< bridges::cpp_uno::shared::UnoInterfaceProxy * >(pUnoI);
//  typelib_InterfaceTypeDescription * pTypeDescr = pThis->pTypeDescr;

    switch (pMemberDescr->eTypeClass)
    {
    case typelib_TypeClass_INTERFACE_ATTRIBUTE:
    {
     VtableSlot aVtableSlot(
                 getVtableSlot(
                     reinterpret_cast<
                         typelib_InterfaceAttributeTypeDescription const * >(
                             pMemberDescr)));
        if (pReturn)
        {
            // dependent dispatch
            cpp_call(
                pThis, aVtableSlot,
                ((typelib_InterfaceAttributeTypeDescription *)pMemberDescr)->pAttributeTypeRef,
                0, 0, // no params
                pReturn, pArgs, ppException );
        }
        else
        {
            // is SET
            typelib_MethodParameter aParam;
            aParam.pTypeRef =
                ((typelib_InterfaceAttributeTypeDescription *)pMemberDescr)->pAttributeTypeRef;
            aParam.bIn      = sal_True;
            aParam.bOut     = sal_False;

            typelib_TypeDescriptionReference * pReturnTypeRef = 0;
            OUString aVoidName("void");
            typelib_typedescriptionreference_new(
                &pReturnTypeRef, typelib_TypeClass_VOID, aVoidName.pData );

            // dependent dispatch
            aVtableSlot.index += 1; // get, then set method
            cpp_call(
                pThis, aVtableSlot,
                pReturnTypeRef,
                1, &aParam,
                pReturn, pArgs, ppException );

            typelib_typedescriptionreference_release( pReturnTypeRef );
        }

        break;
    }
    case typelib_TypeClass_INTERFACE_METHOD:
    {
        VtableSlot aVtableSlot(
        getVtableSlot(
         reinterpret_cast<
          typelib_InterfaceMethodTypeDescription const * >(
          pMemberDescr)));
        switch (aVtableSlot.index)
        {
            // standard calls
        case 1: // acquire uno interface
            (*pUnoI->acquire)( pUnoI );
            *ppException = 0;
            break;
        case 2: // release uno interface
            (*pUnoI->release)( pUnoI );
            *ppException = 0;
            break;
        case 0: // queryInterface() opt
        {
            typelib_TypeDescription * pTD = 0;
            TYPELIB_DANGER_GET( &pTD, reinterpret_cast< Type * >( pArgs[0] )->getTypeLibType() );
            if (pTD)
            {
                uno_Interface * pInterface = 0;
         (*pThis->pBridge->getUnoEnv()->getRegisteredInterface)(
           pThis->pBridge->getUnoEnv(),
                   (void **)&pInterface, pThis->oid.pData, (typelib_InterfaceTypeDescription *)pTD );

                if (pInterface)
                {
                    ::uno_any_construct(
                        reinterpret_cast< uno_Any * >( pReturn ),
                        &pInterface, pTD, 0 );
                    (*pInterface->release)( pInterface );
                    TYPELIB_DANGER_RELEASE( pTD );
                    *ppException = 0;
                    break;
                }
                TYPELIB_DANGER_RELEASE( pTD );
            }
        } // else perform queryInterface()
        default:
            // dependent dispatch
            cpp_call(
                pThis, aVtableSlot,
                ((typelib_InterfaceMethodTypeDescription *)pMemberDescr)->pReturnTypeRef,
                ((typelib_InterfaceMethodTypeDescription *)pMemberDescr)->nParams,
                ((typelib_InterfaceMethodTypeDescription *)pMemberDescr)->pParams,
                pReturn, pArgs, ppException );
        }
        break;
    }
    default:
    {
        ::com::sun::star::uno::RuntimeException aExc(
            OUString("illegal member type description!"),
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >() );

        Type const & rExcType = cppu::UnoType<decltype(aExc)>::get();
        // binary identical null reference
        ::uno_type_any_construct( *ppException, &aExc, rExcType.getTypeLibType(), 0 );
    }
    }
}

} } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
