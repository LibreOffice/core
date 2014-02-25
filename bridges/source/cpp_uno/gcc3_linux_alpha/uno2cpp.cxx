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

void MapReturn(long r0, typelib_TypeClass eTypeClass, sal_uInt64* pRegisterReturn)
{
    register float fret asm("$f0");
    register double dret asm("$f0");

#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr,"Mapping Return with %lx %ld %f\n", r0, r0, dret);
#endif
    switch (eTypeClass)
    {
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
            *pRegisterReturn = r0;
            break;
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
    case typelib_TypeClass_ENUM:
            *(unsigned int*)pRegisterReturn = (unsigned int)r0;
            break;
    case typelib_TypeClass_CHAR:
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
            *(unsigned short*)pRegisterReturn = (unsigned short)r0;
            break;
    case typelib_TypeClass_BOOLEAN:
    case typelib_TypeClass_BYTE:
            *(unsigned char*)pRegisterReturn = (unsigned char)r0;
            break;
    case typelib_TypeClass_FLOAT:
            *reinterpret_cast<float *>( pRegisterReturn ) = fret;
            break;
    case typelib_TypeClass_DOUBLE:
            *reinterpret_cast<double *>( pRegisterReturn ) = dret;
            break;
    default:
            break;
    }
#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr, "end of MapReturn with %x\n", pRegisterReturn ? *pRegisterReturn : 0);
#endif
}

#define INSERT_FLOAT( pSV, nr, pFPR, pDS ) \
    { \
        if ( nr < axp::MAX_WORDS_IN_REGS ) \
        { \
                pFPR[nr++] = *reinterpret_cast<float *>( pSV ); \
        } \
        else \
                *pDS++ = *reinterpret_cast<sal_uInt64 *>( pSV ); \
    }

#define INSERT_DOUBLE( pSV, nr, pFPR, pDS ) \
        if ( nr < axp::MAX_WORDS_IN_REGS ) \
                pFPR[nr++] = *reinterpret_cast<double *>( pSV ); \
        else \
                *pDS++ = *reinterpret_cast<sal_uInt64 *>( pSV ); // verbatim!

#define INSERT_INT64( pSV, nr, pGPR, pDS ) \
        if ( nr < axp::MAX_WORDS_IN_REGS ) \
                pGPR[nr++] = *reinterpret_cast<sal_uInt64 *>( pSV ); \
        else \
                *pDS++ = *reinterpret_cast<sal_uInt64 *>( pSV );

#define INSERT_INT32( pSV, nr, pGPR, pDS ) \
        if ( nr < axp::MAX_WORDS_IN_REGS ) \
                pGPR[nr++] = *reinterpret_cast<sal_uInt32 *>( pSV ); \
        else \
                *pDS++ = *reinterpret_cast<sal_uInt32 *>( pSV );

#define INSERT_INT16( pSV, nr, pGPR, pDS ) \
        if ( nr < axp::MAX_WORDS_IN_REGS ) \
                pGPR[nr++] = *reinterpret_cast<sal_uInt16 *>( pSV ); \
        else \
                *pDS++ = *reinterpret_cast<sal_uInt16 *>( pSV );

#define INSERT_INT8( pSV, nr, pGPR, pDS ) \
        if ( nr < axp::MAX_WORDS_IN_REGS ) \
                pGPR[nr++] = *reinterpret_cast<sal_uInt8 *>( pSV ); \
        else \
                *pDS++ = *reinterpret_cast<sal_uInt8 *>( pSV );

namespace
{

void callVirtualMethod(
    void * pThis, sal_Int32 nVtableIndex,
    void * pRegisterReturn, typelib_TypeDescription * pReturnTypeDescr,
    sal_uInt64 *pStack, sal_uInt32 nStack,
    sal_uInt64 *pGPR, sal_uInt32 nGPR,
    double *pFPR, sal_uInt32 nFPR)
{
    // Should not happen, but...
    if ( nFPR > axp::MAX_SSE_REGS )
        nFPR = axp::MAX_SSE_REGS;
    if ( nGPR > axp::MAX_GPR_REGS )
        nGPR = axp::MAX_GPR_REGS;

#if OSL_DEBUG_LEVEL > 2
        // Let's figure out what is really going on here
        {
            fprintf( stderr, "= nStack is %d\n", nStack );
            fprintf( stderr, "= callVirtualMethod() =\nGPR's (%d): ", nGPR );
            for ( unsigned int i = 0; i < nGPR; ++i )
                fprintf( stderr, "0x%lx, ", pGPR[i] );
            fprintf( stderr, "\nFPR's (%d): ", nFPR );
            for ( unsigned int i = 0; i < nFPR; ++i )
                fprintf( stderr, "0x%lx (%f), ", pFPR[i], pFPR[i] );
            fprintf( stderr, "\nStack (%d): ", nStack );
            for ( unsigned int i = 0; i < nStack; ++i )
                fprintf( stderr, "0x%lx, ", pStack[i] );
            fprintf( stderr, "\n" );
            fprintf( stderr, "pRegisterReturn is %p\n", pRegisterReturn);
        }
#endif

    // Load parameters to stack, if necessary
    // Stack, if used, must be 8-bytes aligned
    sal_uInt64 *stack = (sal_uInt64 *) __builtin_alloca( nStack * 8 );
    memcpy( stack, pStack, nStack * 8 );

    // To get pointer to method
    // a) get the address of the vtable
    sal_uInt64 pMethod = *((sal_uInt64 *)pThis);
    // b) get the address from the vtable entry at offset
    pMethod += 8 * nVtableIndex;
    pMethod = *((sal_uInt64 *)pMethod);

    typedef void (* FunctionCall )( sal_uInt64, sal_uInt64, sal_uInt64, sal_uInt64, sal_uInt64, sal_uInt64 );
    FunctionCall pFunc = (FunctionCall)pMethod;

    switch (nFPR) //deliberate fall through
    {
        case 6:
            asm volatile("ldt $f16,%0" :: "m"(pFPR[5]) : "$f16");
        case 5:
            asm volatile("ldt $f17,%0" :: "m"(pFPR[4]) : "$f17");
        case 4:
            asm volatile("ldt $f18,%0" :: "m"(pFPR[3]) : "$f18");
        case 3:
            asm volatile("ldt $f19,%0" :: "m"(pFPR[2]) : "$f19");
        case 2:
            asm volatile("ldt $f20,%0" :: "m"(pFPR[1]) : "$f20");
        case 1:
            asm volatile("ldt $f21,%0" :: "m"(pFPR[0]) : "$f21");
        default:
            break;
    }

    (*pFunc)(pGPR[0], pGPR[1], pGPR[2], pGPR[3], pGPR[4], pGPR[5]);
    register sal_uInt64 r0 __asm__("$0");
    MapReturn(r0, pReturnTypeDescr->eTypeClass, (sal_uInt64*)pRegisterReturn);
}


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

    sal_uInt64 pGPR[axp::MAX_GPR_REGS];
    double pFPR[axp::MAX_SSE_REGS];
    sal_uInt32 nRegs = 0;

    // return
    typelib_TypeDescription * pReturnTypeDescr = 0;
    TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );
    OSL_ENSURE( pReturnTypeDescr, "### expected return type description!" );

    void * pCppReturn = 0; // if != 0 && != pUnoReturn, needs reconversion

    if (pReturnTypeDescr)
    {
        if (bridges::cpp_uno::shared::isSimpleType( pReturnTypeDescr ))
        {
            pCppReturn = pUnoReturn; // direct way for simple types
        }
        else
        {
            // complex return via ptr
            pCppReturn = (bridges::cpp_uno::shared::relatesToInterfaceType( pReturnTypeDescr )
                                                ? alloca( pReturnTypeDescr->nSize )
                                                : pUnoReturn); // direct way
            INSERT_INT64( &pCppReturn, nRegs, pGPR, pStack );
        }
    }
        // push "this" pointer
    void * pAdjustedThisPtr = reinterpret_cast< void ** >( pThis->getCppI() ) + aVtableSlot.offset;

    INSERT_INT64( &pAdjustedThisPtr, nRegs, pGPR, pStack );

    // stack space
    OSL_ENSURE( sizeof(void *) == sizeof(sal_Int64), "### unexpected size!" );
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
                case typelib_TypeClass_HYPER:
                case typelib_TypeClass_UNSIGNED_HYPER:
                    INSERT_INT64( pCppArgs[nPos], nRegs, pGPR, pStack );
                    break;
                case typelib_TypeClass_LONG:
                case typelib_TypeClass_UNSIGNED_LONG:
                case typelib_TypeClass_ENUM:
                    INSERT_INT32( pCppArgs[nPos], nRegs, pGPR, pStack );
                    break;
                case typelib_TypeClass_SHORT:
                case typelib_TypeClass_CHAR:
                case typelib_TypeClass_UNSIGNED_SHORT:
                    INSERT_INT16( pCppArgs[nPos], nRegs, pGPR, pStack );
                    break;
                case typelib_TypeClass_BOOLEAN:
                case typelib_TypeClass_BYTE:
                    INSERT_INT8( pCppArgs[nPos], nRegs, pGPR, pStack );
                    break;
                case typelib_TypeClass_FLOAT:
                    INSERT_FLOAT( pCppArgs[nPos], nRegs, pFPR, pStack );
                    break;
                case typelib_TypeClass_DOUBLE:
                    INSERT_DOUBLE( pCppArgs[nPos], nRegs, pFPR, pStack );
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
                pTempIndices[nTempIndices] = nPos; // default constructed for cpp call
                // will be released at reconversion
                ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
            }
            // is in/inout
            else if (bridges::cpp_uno::shared::relatesToInterfaceType( pParamTypeDescr ))
            {
                uno_copyAndConvertData(
                    pCppArgs[nPos] = alloca( pParamTypeDescr->nSize ),
                    pUnoArgs[nPos], pParamTypeDescr, pThis->getBridge()->getUno2Cpp() );

                pTempIndices[nTempIndices] = nPos; // has to be reconverted
                // will be released at reconversion
                ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
            }
            else // direct way
            {
                pCppArgs[nPos] = pUnoArgs[nPos];
                // no longer needed
                TYPELIB_DANGER_RELEASE( pParamTypeDescr );
            }
            INSERT_INT64( &(pCppArgs[nPos]), nRegs, pGPR, pStack );
        }
    }

    try
    {
        callVirtualMethod(
            pAdjustedThisPtr, aVtableSlot.index,
            pCppReturn, pReturnTypeDescr,
            pStackStart, (pStack - pStackStart),
            pGPR, nRegs,
            pFPR, nRegs );
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
    catch (...)
    {
        // fill uno exception
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
#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr, "unoInterfaceProxyDispatch\n");
#endif


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
            OUString aVoidName("void");
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
