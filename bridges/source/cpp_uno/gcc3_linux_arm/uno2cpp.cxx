/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <malloc.h>
#include <rtl/alloc.h>

#include <com/sun/star/uno/genfunc.hxx>
#include "com/sun/star/uno/RuntimeException.hpp"
#include <uno/data.h>

#include <bridges/cpp_uno/shared/bridge.hxx>
#include <bridges/cpp_uno/shared/types.hxx>
#include <bridges/cpp_uno/shared/unointerfaceproxy.hxx>
#include <bridges/cpp_uno/shared/vtables.hxx>

#include "share.hxx"

#include <stdio.h>
#include <string.h>

/*
 * Based on http://gcc.gnu.org/PR41443
 * References to __SOFTFP__ are incorrect for EABI; the __SOFTFP__ code
 * should be used for *soft-float ABI* whether or not VFP is enabled,
 * and __SOFTFP__ does specifically mean soft-float not soft-float ABI.
 *
 * Changing the conditionals to __SOFTFP__ || __ARM_EABI__ then
 * -mfloat-abi=softfp should work.  -mfloat-abi=hard won't; that would
 * need both a new macro to identify the hard-VFP ABI.
 */
#if !defined(__ARM_EABI__) && !defined(__SOFTFP__)
#error Not Implemented

/*
 some possibly handy code to detect that we have VFP registers
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <elf.h>

#define HWCAP_ARM_VFP 64

int hasVFP(void)
{
    int fd = open ("/proc/self/auxv", O_RDONLY);
    if (fd == -1)
        return -1;

    int ret = -1;

    Elf32_auxv_t buf[128];
    ssize_t n;
    while ((ret == -1) && ((n = read(fd, buf, sizeof (buf))) > 0))
    {
        for (int i = 0; i < 128; ++i)
        {
        if (buf[i].a_type == AT_HWCAP)
        {
                ret = (buf[i].a_un.a_val & HWCAP_ARM_VFP) ? true : false;
                break;
        }
            else if (buf[i].a_type == AT_NULL)
            {
                ret = -2;
                break;
            }
        }
    }

    close (fd);
    return ret;
}

#endif

using namespace ::rtl;
using namespace ::com::sun::star::uno;

namespace arm
{
    bool is_complex_struct(const typelib_TypeDescription * type)
    {
        const typelib_CompoundTypeDescription * p
            = reinterpret_cast< const typelib_CompoundTypeDescription * >(type);
        for (sal_Int32 i = 0; i < p->nMembers; ++i)
        {
            if (p->ppTypeRefs[i]->eTypeClass == typelib_TypeClass_STRUCT ||
                p->ppTypeRefs[i]->eTypeClass == typelib_TypeClass_EXCEPTION)
            {
                typelib_TypeDescription * t = 0;
                TYPELIB_DANGER_GET(&t, p->ppTypeRefs[i]);
                bool b = is_complex_struct(t);
                TYPELIB_DANGER_RELEASE(t);
                if (b) {
                    return true;
                }
            }
            else if (!bridges::cpp_uno::shared::isSimpleType(p->ppTypeRefs[i]->eTypeClass))
                return true;
        }
        if (p->pBaseTypeDescription != 0)
            return is_complex_struct(&p->pBaseTypeDescription->aBase);
        return false;
    }

#ifdef __ARM_PCS_VFP
    bool is_float_only_struct(const typelib_TypeDescription * type)
    {
        const typelib_CompoundTypeDescription * p
            = reinterpret_cast< const typelib_CompoundTypeDescription * >(type);
        for (sal_Int32 i = 0; i < p->nMembers; ++i)
        {
            if (p->ppTypeRefs[i]->eTypeClass != typelib_TypeClass_FLOAT &&
                p->ppTypeRefs[i]->eTypeClass != typelib_TypeClass_DOUBLE)
                return false;
        }
        return true;
    }
#endif
    bool return_in_hidden_param( typelib_TypeDescriptionReference *pTypeRef )
    {
        if (bridges::cpp_uno::shared::isSimpleType(pTypeRef))
            return false;
        else if (pTypeRef->eTypeClass == typelib_TypeClass_STRUCT || pTypeRef->eTypeClass == typelib_TypeClass_EXCEPTION)
        {
            typelib_TypeDescription * pTypeDescr = 0;
            TYPELIB_DANGER_GET( &pTypeDescr, pTypeRef );

            //A Composite Type not larger than 4 bytes is returned in r0
            bool bRet = pTypeDescr->nSize > 4 || is_complex_struct(pTypeDescr);

#ifdef __ARM_PCS_VFP
            // In the VFP ABI, structs with only float/double values that fit in
            // 16 bytes are returned in registers
            if( pTypeDescr->nSize <= 16 && is_float_only_struct(pTypeDescr))
                bRet = false;
#endif

            TYPELIB_DANGER_RELEASE( pTypeDescr );
            return bRet;
        }
        return true;
    }
}

void MapReturn(sal_uInt32 r0, sal_uInt32 r1, typelib_TypeDescriptionReference * pReturnType, sal_uInt32* pRegisterReturn)
{
    switch( pReturnType->eTypeClass )
    {
        case typelib_TypeClass_HYPER:
        case typelib_TypeClass_UNSIGNED_HYPER:
            pRegisterReturn[1] = r1;
        case typelib_TypeClass_LONG:
        case typelib_TypeClass_UNSIGNED_LONG:
        case typelib_TypeClass_ENUM:
        case typelib_TypeClass_CHAR:
        case typelib_TypeClass_SHORT:
        case typelib_TypeClass_UNSIGNED_SHORT:
        case typelib_TypeClass_BOOLEAN:
        case typelib_TypeClass_BYTE:
            pRegisterReturn[0] = r0;
            break;
        case typelib_TypeClass_FLOAT:
#if !defined(__ARM_PCS_VFP) && (defined(__ARM_EABI__) || defined(__SOFTFP__))
            pRegisterReturn[0] = r0;
#else
            register float fret asm("s0");
            *(float*)pRegisterReturn = fret;
#endif
        break;
        case typelib_TypeClass_DOUBLE:
#if !defined(__ARM_PCS_VFP) && (defined(__ARM_EABI__) || defined(__SOFTFP__))
            pRegisterReturn[1] = r1;
            pRegisterReturn[0] = r0;
#else
            register double dret asm("d0");
            *(double*)pRegisterReturn = dret;
#endif
            break;
        case typelib_TypeClass_STRUCT:
        case typelib_TypeClass_EXCEPTION:
        {
            if (!arm::return_in_hidden_param(pReturnType))
                pRegisterReturn[0] = r0;
            break;
        }
        default:
            break;
    }
}

namespace
{
//================================================================

void callVirtualMethod(
    void * pThis,
    sal_Int32 nVtableIndex,
    void * pRegisterReturn,
    typelib_TypeDescriptionReference * pReturnType,
    sal_uInt32 *pStack,
    sal_uInt32 nStack,
    sal_uInt32 *pGPR,
    sal_uInt32 nGPR,
    double *pFPR) __attribute__((noinline));

void callVirtualMethod(
    void * pThis,
    sal_Int32 nVtableIndex,
    void * pRegisterReturn,
    typelib_TypeDescriptionReference * pReturnType,
    sal_uInt32 *pStack,
    sal_uInt32 nStack,
    sal_uInt32 *pGPR,
    sal_uInt32 nGPR,
    double *pFPR)
{
    // never called
    if (! pThis)
        CPPU_CURRENT_NAMESPACE::dummy_can_throw_anything("xxx"); // address something

    if ( nStack )
    {
        // 8-bytes aligned
        sal_uInt32 nStackBytes = ( ( nStack + 1 ) >> 1 ) * 8;
        sal_uInt32 *stack = (sal_uInt32 *) __builtin_alloca( nStackBytes );
        memcpy( stack, pStack, nStackBytes );
    }

    // Should not happen, but...
    if ( nGPR > arm::MAX_GPR_REGS )
        nGPR = arm::MAX_GPR_REGS;

    sal_uInt32 pMethod = *((sal_uInt32*)pThis);
    pMethod += 4 * nVtableIndex;
    pMethod = *((sal_uInt32 *)pMethod);

    //Return registers
    sal_uInt32 r0;
    sal_uInt32 r1;

    __asm__ __volatile__ (
        //Fill in general purpose register arguments
        "ldr r4, %[pgpr]\n\t"
        "ldmia r4, {r0-r3}\n\t"

#ifdef __ARM_PCS_VFP
        //Fill in VFP register arguments as double precision values
        "ldr r4, %[pfpr]\n\t"
        "vldmia r4, {d0-d7}\n\t"
#endif
        //Make the call
        "ldr r5, %[pmethod]\n\t"
#ifndef __ARM_ARCH_4T__
        "blx r5\n\t"
#else
        "mov lr, pc ; bx r5\n\t"
#endif

        //Fill in return values
        "mov %[r0], r0\n\t"
        "mov %[r1], r1\n\t"
        : [r0]"=r" (r0), [r1]"=r" (r1)
        : [pmethod]"m" (pMethod), [pgpr]"m" (pGPR), [pfpr]"m" (pFPR)
        : "r4", "r5");

    MapReturn(r0, r1, pReturnType, (sal_uInt32*)pRegisterReturn);
}
}

#define INSERT_INT32( pSV, nr, pGPR, pDS ) \
        if ( nr < arm::MAX_GPR_REGS ) \
                pGPR[nr++] = *reinterpret_cast<sal_uInt32 *>( pSV ); \
        else \
                *pDS++ = *reinterpret_cast<sal_uInt32 *>( pSV );

#ifdef __ARM_EABI__
#define INSERT_INT64( pSV, nr, pGPR, pDS, pStart ) \
        if ( (nr < arm::MAX_GPR_REGS) && (nr % 2) ) \
        { \
                ++nr; \
        } \
        if ( nr < arm::MAX_GPR_REGS ) \
        { \
                pGPR[nr++] = *reinterpret_cast<sal_uInt32 *>( pSV ); \
                pGPR[nr++] = *(reinterpret_cast<sal_uInt32 *>( pSV ) + 1); \
        } \
        else \
    { \
        if ( (pDS - pStart) % 2) \
                { \
                    ++pDS; \
                } \
                *pDS++ = reinterpret_cast<sal_uInt32 *>( pSV )[0]; \
                *pDS++ = reinterpret_cast<sal_uInt32 *>( pSV )[1]; \
    }
#else
#define INSERT_INT64( pSV, nr, pGPR, pDS, pStart ) \
        INSERT_INT32( pSV, nr, pGPR, pDS ) \
        INSERT_INT32( ((sal_uInt32*)pSV)+1, nr, pGPR, pDS )
#endif

#ifdef __ARM_PCS_VFP
// Since single and double arguments share the same register bank the filling of the
// registers is not always linear. Single values go to the first available single register,
// while doubles need to have an 8 byte alignment, so only go into double registers starting
// at every other single register. For ex a float, double, float sequence will fill registers
// s0, d1, and s1, actually corresponding to the linear order s0,s1, d1.
//
// These use the single/double register array and counters and ignore the pGPR argument
// nSR and nDR are the number of single and double precision registers that are no longer
// available
#define INSERT_FLOAT( pSV, nr, pGPR, pDS ) \
        if (nSR % 2 == 0) {\
            nSR = 2*nDR; \
        }\
        if ( nSR < arm::MAX_FPR_REGS*2 ) {\
                pSPR[nSR++] = *reinterpret_cast<float *>( pSV ); \
                if ((nSR % 2 == 1) && (nSR > 2*nDR)) {\
                    nDR++; \
                }\
        }\
        else \
        {\
                *pDS++ = *reinterpret_cast<float *>( pSV );\
        }
#define INSERT_DOUBLE( pSV, nr, pGPR, pDS, pStart ) \
        if ( nDR < arm::MAX_FPR_REGS ) { \
                pFPR[nDR++] = *reinterpret_cast<double *>( pSV ); \
        }\
        else\
        {\
            if ( (pDS - pStart) % 2) \
                { \
                    ++pDS; \
                } \
            *(double *)pDS = *reinterpret_cast<double *>( pSV );\
            pDS += 2;\
        }
#else
#define INSERT_FLOAT( pSV, nr, pFPR, pDS ) \
        INSERT_INT32( pSV, nr, pGPR, pDS )

#define INSERT_DOUBLE( pSV, nr, pFPR, pDS, pStart ) \
        INSERT_INT64( pSV, nr, pGPR, pDS, pStart )
#endif

#define INSERT_INT16( pSV, nr, pGPR, pDS ) \
        if ( nr < arm::MAX_GPR_REGS ) \
                pGPR[nr++] = *reinterpret_cast<sal_uInt16 *>( pSV ); \
        else \
                *pDS++ = *reinterpret_cast<sal_uInt16 *>( pSV );

#define INSERT_INT8( pSV, nr, pGPR, pDS ) \
        if ( nr < arm::MAX_GPR_REGS ) \
                pGPR[nr++] = *reinterpret_cast<sal_uInt8 *>( pSV ); \
        else \
                *pDS++ = *reinterpret_cast<sal_uInt8 *>( pSV );

namespace {
//=======================================================================
static void cpp_call(
    bridges::cpp_uno::shared::UnoInterfaceProxy * pThis,
    bridges::cpp_uno::shared::VtableSlot aVtableSlot,
    typelib_TypeDescriptionReference * pReturnTypeRef,
    sal_Int32 nParams, typelib_MethodParameter * pParams,
    void * pUnoReturn, void * pUnoArgs[], uno_Any ** ppUnoExc )
{
    // max space for: [complex ret ptr], values|ptr ...
    sal_uInt32 * pStack = (sal_uInt32 *)__builtin_alloca(
        sizeof(sal_Int32) + ((nParams+2) * sizeof(sal_Int64)) );
    sal_uInt32 * pStackStart = pStack;

    sal_uInt32 pGPR[arm::MAX_GPR_REGS];
    sal_uInt32 nGPR = 0;

    // storage and counters for single and double precision VFP registers
    double pFPR[arm::MAX_FPR_REGS];
#ifdef __ARM_PCS_VFP
    sal_uInt32 nDR = 0;
    float *pSPR = reinterpret_cast< float *>(&pFPR);
    sal_uInt32 nSR = 0;
#endif

    // return
    typelib_TypeDescription * pReturnTypeDescr = 0;
    TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );
    OSL_ENSURE( pReturnTypeDescr, "### expected return type description!" );

    void * pCppReturn = 0; // if != 0 && != pUnoReturn, needs reconversion

    bool bSimpleReturn = true;
    if (pReturnTypeDescr)
    {
        if (arm::return_in_hidden_param( pReturnTypeRef ) )
            bSimpleReturn = false;

        if (bSimpleReturn)
            pCppReturn = pUnoReturn; // direct way for simple types
        else
        {
            // complex return via ptr
            pCppReturn = (bridges::cpp_uno::shared::relatesToInterfaceType( pReturnTypeDescr )
                    ? __builtin_alloca( pReturnTypeDescr->nSize )
                    : pUnoReturn); // direct way

            INSERT_INT32( &pCppReturn, nGPR, pGPR, pStack );
        }
    }
    // push this
    void * pAdjustedThisPtr = reinterpret_cast< void ** >(pThis->getCppI())
        + aVtableSlot.offset;
    INSERT_INT32( &pAdjustedThisPtr, nGPR, pGPR, pStack );

    // stack space
    OSL_ENSURE( sizeof(void *) == sizeof(sal_Int32), "### unexpected size!" );
    // args
    void ** pCppArgs  = (void **)alloca( 3 * sizeof(void *) * nParams );
    // indizes of values this have to be converted (interface conversion cpp<=>uno)
    sal_Int32 * pTempIndizes = (sal_Int32 *)(pCppArgs + nParams);
    // type descriptions for reconversions
    typelib_TypeDescription ** ppTempParamTypeDescr = (typelib_TypeDescription **)(pCppArgs + (2 * nParams));

    sal_Int32 nTempIndizes   = 0;

    for ( sal_Int32 nPos = 0; nPos < nParams; ++nPos )
    {
        const typelib_MethodParameter & rParam = pParams[nPos];
        typelib_TypeDescription * pParamTypeDescr = 0;
        TYPELIB_DANGER_GET( &pParamTypeDescr, rParam.pTypeRef );

        if (!rParam.bOut && bridges::cpp_uno::shared::isSimpleType( pParamTypeDescr ))
        {
//            uno_copyAndConvertData( pCppArgs[nPos] = pStack, pUnoArgs[nPos],
            uno_copyAndConvertData( pCppArgs[nPos] = alloca(8), pUnoArgs[nPos],
                pParamTypeDescr, pThis->getBridge()->getUno2Cpp() );

            switch (pParamTypeDescr->eTypeClass)
            {
            case typelib_TypeClass_HYPER:
            case typelib_TypeClass_UNSIGNED_HYPER:
#if OSL_DEBUG_LEVEL > 2
                fprintf(stderr, "hyper is %p\n", pCppArgs[nPos]);
#endif
                INSERT_INT64( pCppArgs[nPos], nGPR, pGPR, pStack, pStackStart );
                break;
            case typelib_TypeClass_LONG:
            case typelib_TypeClass_UNSIGNED_LONG:
            case typelib_TypeClass_ENUM:
#if OSL_DEBUG_LEVEL > 2
                fprintf(stderr, "long is %p\n", pCppArgs[nPos]);
#endif
                INSERT_INT32( pCppArgs[nPos], nGPR, pGPR, pStack );
                break;
            case typelib_TypeClass_SHORT:
            case typelib_TypeClass_CHAR:
            case typelib_TypeClass_UNSIGNED_SHORT:
                INSERT_INT16( pCppArgs[nPos], nGPR, pGPR, pStack );
                break;
            case typelib_TypeClass_BOOLEAN:
            case typelib_TypeClass_BYTE:
                INSERT_INT8( pCppArgs[nPos], nGPR, pGPR, pStack );
                break;
            case typelib_TypeClass_FLOAT:
                INSERT_FLOAT( pCppArgs[nPos], nGPR, pGPR, pStack );
                break;
            case typelib_TypeClass_DOUBLE:
                INSERT_DOUBLE( pCppArgs[nPos], nGPR, pGPR, pStack, pStackStart );
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
                    pCppArgs[nPos] = alloca( pParamTypeDescr->nSize ),
                    pParamTypeDescr );
                pTempIndizes[nTempIndizes] = nPos; // default constructed for cpp call
                // will be released at reconversion
                ppTempParamTypeDescr[nTempIndizes++] = pParamTypeDescr;
            }
            // is in/inout
            else if (bridges::cpp_uno::shared::relatesToInterfaceType( pParamTypeDescr ))
            {
                uno_copyAndConvertData(
                    pCppArgs[nPos] = alloca( pParamTypeDescr->nSize ),
                    pUnoArgs[nPos], pParamTypeDescr, pThis->getBridge()->getUno2Cpp() );

                pTempIndizes[nTempIndizes] = nPos; // has to be reconverted
                // will be released at reconversion
                ppTempParamTypeDescr[nTempIndizes++] = pParamTypeDescr;
            }
            else // direct way
            {
                pCppArgs[nPos] = pUnoArgs[nPos];
                // no longer needed
                TYPELIB_DANGER_RELEASE( pParamTypeDescr );
            }
            INSERT_INT32( &(pCppArgs[nPos]), nGPR, pGPR, pStack );
        }
    }

    try
    {
        callVirtualMethod(
            pAdjustedThisPtr, aVtableSlot.index,
            pCppReturn, pReturnTypeRef,
            pStackStart,
            (pStack - pStackStart),
            pGPR, nGPR,
            pFPR);

        // NO exception occurred...
        *ppUnoExc = 0;

        // reconvert temporary params
        for ( ; nTempIndizes--; )
        {
            sal_Int32 nIndex = pTempIndizes[nTempIndizes];
            typelib_TypeDescription * pParamTypeDescr = ppTempParamTypeDescr[nTempIndizes];

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
    catch (...)
    {
//        __asm__ __volatile__ ("sub sp, sp, #2048\n");

        // fill uno exception
        fillUnoException( CPPU_CURRENT_NAMESPACE::__cxa_get_globals()->caughtExceptions, *ppUnoExc, pThis->getBridge()->getCpp2Uno() );

        // temporary params
        for ( ; nTempIndizes--; )
        {
            sal_Int32 nIndex = pTempIndizes[nTempIndizes];
            // destroy temp cpp param => cpp: every param was constructed
            uno_destructData( pCppArgs[nIndex], ppTempParamTypeDescr[nTempIndizes], cpp_release );
            TYPELIB_DANGER_RELEASE( ppTempParamTypeDescr[nTempIndizes] );
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
    // is my surrogate
    bridges::cpp_uno::shared::UnoInterfaceProxy * pThis
          = static_cast< bridges::cpp_uno::shared::UnoInterfaceProxy * >(pUnoI);
#if OSL_DEBUG_LEVEL > 0
    typelib_InterfaceTypeDescription * pTypeDescr = pThis->pTypeDescr;
#endif

    switch (pMemberDescr->eTypeClass)
    {
    case typelib_TypeClass_INTERFACE_ATTRIBUTE:
    {
#if OSL_DEBUG_LEVEL > 0
        // determine vtable call index
        sal_Int32 nMemberPos = ((typelib_InterfaceMemberTypeDescription *)pMemberDescr)->nPosition;
        OSL_ENSURE( nMemberPos < pTypeDescr->nAllMembers, "### member pos out of range!" );
#endif

        VtableSlot aVtableSlot(
            getVtableSlot(
            reinterpret_cast<typelib_InterfaceAttributeTypeDescription const *>
              (pMemberDescr)));

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
            OUString aVoidName( RTL_CONSTASCII_USTRINGPARAM("void") );
            typelib_typedescriptionreference_new(
                &pReturnTypeRef, typelib_TypeClass_VOID, aVoidName.pData );

            // dependent dispatch
            aVtableSlot.index += 1;
            cpp_call(
                pThis, aVtableSlot, // get, then set method
                pReturnTypeRef,
                1, &aParam,
                pReturn, pArgs, ppException );

            typelib_typedescriptionreference_release( pReturnTypeRef );
        }

        break;
    }
    case typelib_TypeClass_INTERFACE_METHOD:
    {
#if OSL_DEBUG_LEVEL > 0
        // determine vtable call index
        sal_Int32 nMemberPos = ((typelib_InterfaceMemberTypeDescription *)pMemberDescr)->nPosition;
        OSL_ENSURE( nMemberPos < pTypeDescr->nAllMembers, "### member pos out of range!" );
#endif

        VtableSlot aVtableSlot(
            getVtableSlot(
            reinterpret_cast<typelib_InterfaceMethodTypeDescription const *>
              (pMemberDescr)));

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
                (*pThis->getBridge()->getUnoEnv()->getRegisteredInterface)(
                    pThis->getBridge()->getUnoEnv(),
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
            OUString( RTL_CONSTASCII_USTRINGPARAM("illegal member type description!") ),
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >() );

        Type const & rExcType = ::getCppuType( &aExc );
        // binary identical null reference
        ::uno_type_any_construct( *ppException, &aExc, rExcType.getTypeLibType(), 0 );
    }
    }
}

} } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
