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

#include "bridges/cpp_uno/shared/bridge.hxx"
#include "bridges/cpp_uno/shared/types.hxx"
#include "bridges/cpp_uno/shared/unointerfaceproxy.hxx"
#include "bridges/cpp_uno/shared/vtables.hxx"

#include "share.hxx"

#include <stdio.h>
#include <string.h>


using namespace ::rtl;
using namespace ::com::sun::star::uno;

void MapReturn(long r3, double dret, typelib_TypeClass eTypeClass, void *pRegisterReturn)
{
    switch (eTypeClass)
    {
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
            *reinterpret_cast<sal_uInt64 *>( pRegisterReturn ) = r3;
            break;
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
    case typelib_TypeClass_ENUM:
            *reinterpret_cast<sal_uInt32 *>( pRegisterReturn ) = r3;
            break;
    case typelib_TypeClass_CHAR:
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
            *reinterpret_cast<sal_uInt16 *>( pRegisterReturn ) = (unsigned short)r3;
            break;
    case typelib_TypeClass_BOOLEAN:
    case typelib_TypeClass_BYTE:
            *reinterpret_cast<sal_uInt8 *>( pRegisterReturn ) = (unsigned char)r3;
            break;
    case typelib_TypeClass_FLOAT:
            *reinterpret_cast<float *>( pRegisterReturn ) = dret;
        break;
    case typelib_TypeClass_DOUBLE:
            *reinterpret_cast<double *>( pRegisterReturn ) = dret;
            break;
    default:
            break;
    }
}

namespace
{
//==================================================================================================
static void callVirtualMethod(void * pThis, sal_uInt32 nVtableIndex,
    void * pRegisterReturn, typelib_TypeDescription * pReturnTypeDescr,
        sal_uInt64 *pStack, sal_uInt32 nStack,
        sal_uInt64 *pGPR, sal_uInt32 nGPR,
        double *pFPR, sal_uInt32 nFPR)
{
    // Stack, if used, must be 16-bytes aligned
    if ( nStack )
        nStack = ( nStack + 1 ) & ~1;

    // Should not happen, but...
    if ( nFPR > ppc64::MAX_SSE_REGS )
        nFPR = ppc64::MAX_SSE_REGS;
    if ( nGPR > ppc64::MAX_GPR_REGS )
        nGPR = ppc64::MAX_GPR_REGS;

#if OSL_DEBUG_LEVEL > 2
        // Let's figure out what is really going on here
        {
                fprintf( stderr, "= callVirtualMethod() =\nGPR's (%d): ", nGPR );
                for ( int i = 0; i < nGPR; ++i )
                        fprintf( stderr, "0x%lx, ", pGPR[i] );
                fprintf( stderr, "\nFPR's (%d): ", nFPR );
                for ( int i = 0; i < nFPR; ++i )
                        fprintf( stderr, "0x%lx (%f), ", pFPR[i], pFPR[i] );
                fprintf( stderr, "\nStack (%d): ", nStack );
                for ( int i = 0; i < nStack; ++i )
                        fprintf( stderr, "0x%lx, ", pStack[i] );
                fprintf( stderr, "\n" );
        }
#endif

    // Load parameters to stack, if necessary
    sal_uInt64 *stack = (sal_uInt64 *) __builtin_alloca( nStack * 8 );
    memcpy( stack, pStack, nStack * 8 );

    // Get pointer to method
    sal_uInt64 pMethod = *((sal_uInt64 *)pThis);
    pMethod += 8 * nVtableIndex;
    pMethod = *((sal_uInt64 *)pMethod);

    typedef void (* FunctionCall )( sal_uInt64, sal_uInt64, sal_uInt64, sal_uInt64, sal_uInt64, sal_uInt64, sal_uInt64, sal_uInt64 );
    FunctionCall pFunc = (FunctionCall)pMethod;

    volatile double dret;

    //  fill registers
    __asm__ __volatile__ (
                "ld   3,  0(%0)\n\t"
                "ld   4,  8(%0)\n\t"
                "ld   5, 16(%0)\n\t"
                "ld   6, 24(%0)\n\t"
                "ld   7, 32(%0)\n\t"
                "ld   8, 40(%0)\n\t"
                "ld   9, 48(%0)\n\t"
                "ld  10, 56(%0)\n\t"
                "lfd  1,  0(%1)\n\t"
                "lfd  2,  8(%1)\n\t"
                "lfd  3, 16(%1)\n\t"
                "lfd  4, 24(%1)\n\t"
                "lfd  5, 32(%1)\n\t"
                "lfd  6, 40(%1)\n\t"
                "lfd  7, 48(%1)\n\t"
                "lfd  8, 56(%1)\n\t"
                "lfd  9, 64(%1)\n\t"
                "lfd 10, 72(%1)\n\t"
                "lfd 11, 80(%1)\n\t"
                "lfd 12, 88(%1)\n\t"
                "lfd 13, 96(%1)\n\t"
                : : "r" (pGPR), "r" (pFPR)
              : "r0", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10",
                  "fr1", "fr2", "fr3", "fr4", "fr5", "fr6", "fr7", "fr8", "fr9",
                  "fr10", "fr11", "fr12", "fr13"
    );

    // tell gcc that r3 to r11 are not available to it for doing the TOC and exception munge on the func call
    register sal_uInt64 r3 asm("r3");
    register sal_uInt64 r4 asm("r4");
    register sal_uInt64 r5 asm("r5");
    register sal_uInt64 r6 asm("r6");
    register sal_uInt64 r7 asm("r7");
    register sal_uInt64 r8 asm("r8");
    register sal_uInt64 r9 asm("r9");
    register sal_uInt64 r10 asm("r10");
    register sal_uInt64 r11 asm("r11");

    (*pFunc)(r3, r4, r5, r6, r7, r8, r9, r10);

    // get return value
    __asm__ __volatile__ (
                "mr     %1,     3\n\t"
                "mr     %2,     4\n\t"
                "fmr    %0,     1\n\t"
                : "=f" (dret), "=r" (r3), "=r" (r4) : );

    MapReturn(r3, dret, pReturnTypeDescr->eTypeClass, pRegisterReturn);
}

// Macros for easier insertion of values to registers or stack
// pSV - pointer to the source
// nr - order of the value [will be increased if stored to register]
// pFPR, pGPR - pointer to the registers
// pDS - pointer to the stack [will be increased if stored here]

// The value in %xmm register is already prepared to be retrieved as a float,
// thus we treat float and double the same
#define INSERT_FLOAT( pSV, nr, pFPR, pDS, bOverflow ) \
        if ( nr < ppc64::MAX_SSE_REGS ) \
                pFPR[nr++] = *reinterpret_cast<float *>( pSV ); \
        else \
            bOverFlow = true; \
        if (bOverFlow) \
                *pDS++ = *reinterpret_cast<sal_uInt64 *>( pSV ); // verbatim!

#define INSERT_DOUBLE( pSV, nr, pFPR, pDS, bOverflow ) \
        if ( nr < ppc64::MAX_SSE_REGS ) \
                pFPR[nr++] = *reinterpret_cast<double *>( pSV ); \
        else \
            bOverFlow = true; \
        if (bOverFlow) \
                *pDS++ = *reinterpret_cast<sal_uInt64 *>( pSV ); // verbatim!

#define INSERT_INT64( pSV, nr, pGPR, pDS, bOverflow ) \
        if ( nr < ppc64::MAX_GPR_REGS ) \
                pGPR[nr++] = *reinterpret_cast<sal_uInt64 *>( pSV ); \
        else \
        bOverFlow = true; \
    if (bOverFlow) \
                *pDS++ = *reinterpret_cast<sal_uInt64 *>( pSV );

#define INSERT_INT32( pSV, nr, pGPR, pDS, bOverflow ) \
        if ( nr < ppc64::MAX_GPR_REGS ) \
                pGPR[nr++] = *reinterpret_cast<sal_uInt32 *>( pSV ); \
        else \
                bOverFlow = true; \
        if (bOverFlow) \
                *pDS++ = *reinterpret_cast<sal_uInt32 *>( pSV );

#define INSERT_INT16( pSV, nr, pGPR, pDS, bOverflow ) \
        if ( nr < ppc64::MAX_GPR_REGS ) \
                pGPR[nr++] = *reinterpret_cast<sal_uInt16 *>( pSV ); \
        else \
                bOverFlow = true; \
        if (bOverFlow) \
                *pDS++ = *reinterpret_cast<sal_uInt16 *>( pSV );

#define INSERT_INT8( pSV, nr, pGPR, pDS, bOverflow ) \
        if ( nr < ppc64::MAX_GPR_REGS ) \
                pGPR[nr++] = *reinterpret_cast<sal_uInt8 *>( pSV ); \
        else \
                bOverFlow = true; \
        if (bOverFlow) \
                *pDS++ = *reinterpret_cast<sal_uInt8 *>( pSV );

//==================================================================================================
static void cpp_call(
    bridges::cpp_uno::shared::UnoInterfaceProxy * pThis,
    bridges::cpp_uno::shared::VtableSlot  aVtableSlot,
    typelib_TypeDescriptionReference * pReturnTypeRef,
    sal_Int32 nParams, typelib_MethodParameter * pParams,
    void * pUnoReturn, void * pUnoArgs[], uno_Any ** ppUnoExc )
{
      // max space for: [complex ret ptr], values|ptr ...
      sal_uInt64 * pStack = (sal_uInt64 *)alloca( (nParams+3) * sizeof(sal_Int64) );
      sal_uInt64 * pStackStart = pStack;

    sal_uInt64 pGPR[ppc64::MAX_GPR_REGS];
    sal_uInt32 nGPR = 0;

    double pFPR[ppc64::MAX_SSE_REGS];
    sal_uInt32 nFPR = 0;

    // return
    typelib_TypeDescription * pReturnTypeDescr = 0;
    TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );
    OSL_ENSURE( pReturnTypeDescr, "### expected return type description!" );

    void * pCppReturn = 0; // if != 0 && != pUnoReturn, needs reconversion

        bool bOverFlow = false;

    if (pReturnTypeDescr)
    {
#if OSL_DEBUG_LEVEL > 2
        fprintf(stderr, "return type is %d\n", pReturnTypeDescr->eTypeClass);
#endif
        if (bridges::cpp_uno::shared::isSimpleType( pReturnTypeDescr ))
        {
            pCppReturn = pUnoReturn; // direct way for simple types
#if OSL_DEBUG_LEVEL > 2
            fprintf(stderr, "simple return\n");
#endif
        }
        else
        {
            // complex return via ptr
            pCppReturn = (bridges::cpp_uno::shared::relatesToInterfaceType( pReturnTypeDescr )
                   ? alloca( pReturnTypeDescr->nSize ) : pUnoReturn);
#if OSL_DEBUG_LEVEL > 2
            fprintf(stderr, "pCppReturn/pUnoReturn is %lx/%lx", pCppReturn, pUnoReturn);
#endif
            INSERT_INT64( &pCppReturn, nGPR, pGPR, pStack, bOverFlow );
        }
    }
    // push "this" pointer
        void * pAdjustedThisPtr = reinterpret_cast< void ** >( pThis->getCppI() ) + aVtableSlot.offset;
#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr, "this pointer is %p\n", pAdjustedThisPtr);
#endif
    INSERT_INT64( &pAdjustedThisPtr, nGPR, pGPR, pStack, bOverFlow );

        // Args
        void ** pCppArgs = (void **)alloca( 3 * sizeof(void *) * nParams );
    // indizes of values this have to be converted (interface conversion cpp<=>uno)
    sal_Int32 * pTempIndizes = (sal_Int32 *)(pCppArgs + nParams);
    // type descriptions for reconversions
    typelib_TypeDescription ** ppTempParamTypeDescr = (typelib_TypeDescription **)(pCppArgs + (2 * nParams));

    sal_Int32 nTempIndizes   = 0;

#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr, "n params is %d\n", nParams);
#endif

    for ( sal_Int32 nPos = 0; nPos < nParams; ++nPos )
    {
        const typelib_MethodParameter & rParam = pParams[nPos];
        typelib_TypeDescription * pParamTypeDescr = 0;
        TYPELIB_DANGER_GET( &pParamTypeDescr, rParam.pTypeRef );

#if OSL_DEBUG_LEVEL > 2
        fprintf(stderr, "param %d is %d %d %d\n", nPos, rParam.bOut, bridges::cpp_uno::shared::isSimpleType( pParamTypeDescr ),
            pParamTypeDescr->eTypeClass);
#endif

        if (!rParam.bOut && bridges::cpp_uno::shared::isSimpleType( pParamTypeDescr ))
        {
//          uno_copyAndConvertData( pCppArgs[nPos] = alloca( 8 ), pUnoArgs[nPos], pParamTypeDescr,
            uno_copyAndConvertData( pCppArgs[nPos] = pStack, pUnoArgs[nPos], pParamTypeDescr,
                                    pThis->getBridge()->getUno2Cpp() );
                switch (pParamTypeDescr->eTypeClass)
                        {
                        case typelib_TypeClass_HYPER:
                        case typelib_TypeClass_UNSIGNED_HYPER:
#if OSL_DEBUG_LEVEL > 2
                fprintf(stderr, "hyper is %lx\n", pCppArgs[nPos]);
#endif
                                INSERT_INT64( pCppArgs[nPos], nGPR, pGPR, pStack, bOverFlow );
                                break;
                        case typelib_TypeClass_LONG:
                        case typelib_TypeClass_UNSIGNED_LONG:
                        case typelib_TypeClass_ENUM:
#if OSL_DEBUG_LEVEL > 2
                fprintf(stderr, "long is %x\n", pCppArgs[nPos]);
#endif
                                INSERT_INT32( pCppArgs[nPos], nGPR, pGPR, pStack, bOverFlow );
                                break;
                        case typelib_TypeClass_SHORT:
                        case typelib_TypeClass_CHAR:
                        case typelib_TypeClass_UNSIGNED_SHORT:
                                INSERT_INT16( pCppArgs[nPos], nGPR, pGPR, pStack, bOverFlow );
                                break;
                        case typelib_TypeClass_BOOLEAN:
                        case typelib_TypeClass_BYTE:
                                INSERT_INT8( pCppArgs[nPos], nGPR, pGPR, pStack, bOverFlow );
                                break;
                        case typelib_TypeClass_FLOAT:
                                INSERT_FLOAT( pCppArgs[nPos], nFPR, pFPR, pStack, bOverFlow );
                break;
                        case typelib_TypeClass_DOUBLE:
                                INSERT_DOUBLE( pCppArgs[nPos], nFPR, pFPR, pStack, bOverFlow );
                                break;
                        }

                        // no longer needed
                        TYPELIB_DANGER_RELEASE( pParamTypeDescr );

        }
        else // ptr to complex value | ref
        {
#if OSL_DEBUG_LEVEL > 2
            fprintf(stderr, "complex type again %d\n", rParam.bIn);
#endif
                        if (! rParam.bIn) // is pure out
                        {
#if OSL_DEBUG_LEVEL > 2
                fprintf(stderr, "complex size is %d\n", pParamTypeDescr->nSize );
#endif
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
#if OSL_DEBUG_LEVEL > 2
                fprintf(stderr, "this one\n");
#endif
                                uno_copyAndConvertData(
                                        pCppArgs[nPos] = alloca( pParamTypeDescr->nSize ),
                                        pUnoArgs[nPos], pParamTypeDescr, pThis->getBridge()->getUno2Cpp() );

                                pTempIndizes[nTempIndizes] = nPos; // has to be reconverted
                                // will be released at reconversion
                                ppTempParamTypeDescr[nTempIndizes++] = pParamTypeDescr;
                        }
                        else // direct way
                        {
#if OSL_DEBUG_LEVEL > 2
                fprintf(stderr, "that one, passing %lx through\n", pUnoArgs[nPos]);
#endif
                                pCppArgs[nPos] = pUnoArgs[nPos];
                                // no longer needed
                                TYPELIB_DANGER_RELEASE( pParamTypeDescr );
                        }
                        INSERT_INT64( &(pCppArgs[nPos]), nGPR, pGPR, pStack, bOverFlow );
        }
    }

    try
    {
               callVirtualMethod(
                        pAdjustedThisPtr, aVtableSlot.index,
                        pCppReturn, pReturnTypeDescr,
                        pStackStart, ( pStack - pStackStart ),
                        pGPR, nGPR,
                        pFPR, nFPR );
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
          // fill uno exception
        fillUnoException( CPPU_CURRENT_NAMESPACE::__cxa_get_globals()->caughtExceptions,
                                  *ppUnoExc, pThis->getBridge()->getCpp2Uno() );

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
        = static_cast< bridges::cpp_uno::shared::UnoInterfaceProxy *> (pUnoI);

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
            OUString aVoidName( RTL_CONSTASCII_USTRINGPARAM("void") );
            typelib_typedescriptionreference_new(
                &pReturnTypeRef, typelib_TypeClass_VOID, aVoidName.pData );

            // dependent dispatch
                        aVtableSlot.index += 1; //get then set method
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

        Type const & rExcType = ::getCppuType( &aExc );
        // binary identical null reference
        ::uno_type_any_construct( *ppException, &aExc, rExcType.getTypeLibType(), 0 );
    }
    }
}

} } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
