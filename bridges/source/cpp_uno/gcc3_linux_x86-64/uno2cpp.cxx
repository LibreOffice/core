/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: uno2cpp.cxx,v $
 * $Revision: 1.10 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_bridges.hxx"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rtl/alloc.h>

#include <com/sun/star/uno/genfunc.hxx>
#include "com/sun/star/uno/RuntimeException.hpp"
#include <uno/data.h>

#include <bridges/cpp_uno/shared/bridge.hxx>
#include <bridges/cpp_uno/shared/types.hxx>
#include "bridges/cpp_uno/shared/unointerfaceproxy.hxx"
#include "bridges/cpp_uno/shared/vtables.hxx"

#include "abi.hxx"
#include "share.hxx"

using namespace ::rtl;
using namespace ::com::sun::star::uno;

//==================================================================================================
static void callVirtualMethod(void * pThis, sal_uInt32 nVtableIndex,
                              void * pRegisterReturn, typelib_TypeDescription * pReturnTypeDescr, bool bSimpleReturn,
                              sal_uInt64 *pStack, sal_uInt32 nStack,
                              sal_uInt64 *pGPR, sal_uInt32 nGPR,
                              double *pFPR, sal_uInt32 nFPR) __attribute__((noinline));

static void callVirtualMethod(void * pThis, sal_uInt32 nVtableIndex,
                              void * pRegisterReturn, typelib_TypeDescription * pReturnTypeDescr, bool bSimpleReturn,
                              sal_uInt64 *pStack, sal_uInt32 nStack,
                              sal_uInt64 *pGPR, sal_uInt32 nGPR,
                              double *pFPR, sal_uInt32 nFPR)
{
#if OSL_DEBUG_LEVEL > 1
    // Let's figure out what is really going on here
    {
        fprintf( stderr, "= callVirtualMethod() =\nGPR's (%d): ", nGPR );
        for ( unsigned int i = 0; i < nGPR; ++i )
            fprintf( stderr, "0x%lx, ", pGPR[i] );
        fprintf( stderr, "\nFPR's (%d): ", nFPR );
        for ( unsigned int i = 0; i < nFPR; ++i )
            fprintf( stderr, "%f, ", pFPR[i] );
        fprintf( stderr, "\nStack (%d): ", nStack );
        for ( unsigned int i = 0; i < nStack; ++i )
            fprintf( stderr, "0x%lx, ", pStack[i] );
        fprintf( stderr, "\n" );
    }
#endif

    // The call instruction within the asm section of callVirtualMethod may throw
    // exceptions.  So that the compiler handles this correctly, it is important
    // that (a) callVirtualMethod might call dummy_can_throw_anything (although this
    // never happens at runtime), which in turn can throw exceptions, and (b)
    // callVirtualMethod is not inlined at its call site (so that any exceptions are
    // caught which are thrown from the instruction calling callVirtualMethod):
    if ( !pThis )
        CPPU_CURRENT_NAMESPACE::dummy_can_throw_anything( "xxx" ); // address something

    // Should not happen, but...
    if ( nFPR > x86_64::MAX_SSE_REGS )
        nFPR = x86_64::MAX_SSE_REGS;
    if ( nGPR > x86_64::MAX_GPR_REGS )
        nGPR = x86_64::MAX_GPR_REGS;

    // Get pointer to method
    sal_uInt64 pMethod = *((sal_uInt64 *)pThis);
    pMethod += 8 * nVtableIndex;
    pMethod = *((sal_uInt64 *)pMethod);

    // Load parameters to stack, if necessary
    if ( nStack )
    {
        // 16-bytes aligned
        sal_uInt32 nStackBytes = ( ( nStack + 1 ) >> 1 ) * 16;
        sal_uInt64 *pCallStack = (sal_uInt64 *) __builtin_alloca( nStackBytes );
        memcpy( pCallStack, pStack, nStackBytes );
    }

    // Return values
    sal_uInt64 rax;
    sal_uInt64 rdx;
    double xmm0;

    asm volatile (

        // Fill the xmm registers
        "movq %2, %%rax\n\t"

        "movsd   (%%rax), %%xmm0\n\t"
        "movsd  8(%%rax), %%xmm1\n\t"
        "movsd 16(%%rax), %%xmm2\n\t"
        "movsd 24(%%rax), %%xmm3\n\t"
        "movsd 32(%%rax), %%xmm4\n\t"
        "movsd 40(%%rax), %%xmm5\n\t"
        "movsd 48(%%rax), %%xmm6\n\t"
        "movsd 56(%%rax), %%xmm7\n\t"

        // Fill the general purpose registers
        "movq %1, %%rax\n\t"

        "movq    (%%rax), %%rdi\n\t"
        "movq   8(%%rax), %%rsi\n\t"
        "movq  16(%%rax), %%rdx\n\t"
        "movq  24(%%rax), %%rcx\n\t"
        "movq  32(%%rax), %%r8\n\t"
        "movq  40(%%rax), %%r9\n\t"

        // Perform the call
        "movq %0, %%r11\n\t"
        "movq %3, %%rax\n\t"
        "call *%%r11\n\t"

        // Fill the return values
        "movq   %%rax, %4\n\t"
        "movq   %%rdx, %5\n\t"
        "movsd %%xmm0, %6\n\t"
        :
        : "m" ( pMethod ), "m" ( pGPR ), "m" ( pFPR ), "m" ( nFPR ),
          "m" ( rax ), "m" ( rdx ), "m" ( xmm0 )
        : "rax", "rdi", "rsi", "rdx", "rcx", "r8", "r9", "r11"
    );

    switch (pReturnTypeDescr->eTypeClass)
    {
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
        *reinterpret_cast<sal_uInt64 *>( pRegisterReturn ) = rax;
        break;
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
    case typelib_TypeClass_ENUM:
        *reinterpret_cast<sal_uInt32 *>( pRegisterReturn ) = *reinterpret_cast<sal_uInt32*>( &rax );
        break;
    case typelib_TypeClass_CHAR:
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
        *reinterpret_cast<sal_uInt16 *>( pRegisterReturn ) = *reinterpret_cast<sal_uInt16*>( &rax );
        break;
    case typelib_TypeClass_BOOLEAN:
    case typelib_TypeClass_BYTE:
        *reinterpret_cast<sal_uInt8 *>( pRegisterReturn ) = *reinterpret_cast<sal_uInt8*>( &rax );
        break;
    case typelib_TypeClass_FLOAT:
    case typelib_TypeClass_DOUBLE:
        *reinterpret_cast<double *>( pRegisterReturn ) = xmm0;
        break;
    default:
        {
            sal_Int32 const nRetSize = pReturnTypeDescr->nSize;
            if (bSimpleReturn && nRetSize <= 16 && nRetSize > 0)
            {
                if (nRetSize > 8)
                    static_cast<sal_uInt64 *>(pRegisterReturn)[1] = rdx;
                static_cast<sal_uInt64 *>(pRegisterReturn)[0] = rax;
            }
            break;
        }
    }
}

//==================================================================================================

// Macros for easier insertion of values to registers or stack
// pSV - pointer to the source
// nr - order of the value [will be increased if stored to register]
// pFPR, pGPR - pointer to the registers
// pDS - pointer to the stack [will be increased if stored here]

// The value in %xmm register is already prepared to be retrieved as a float,
// thus we treat float and double the same
#define INSERT_FLOAT_DOUBLE( pSV, nr, pFPR, pDS ) \
    if ( nr < x86_64::MAX_SSE_REGS ) \
        pFPR[nr++] = *reinterpret_cast<double *>( pSV ); \
    else \
        *pDS++ = *reinterpret_cast<sal_uInt64 *>( pSV ); // verbatim!

#define INSERT_INT64( pSV, nr, pGPR, pDS ) \
    if ( nr < x86_64::MAX_GPR_REGS ) \
        pGPR[nr++] = *reinterpret_cast<sal_uInt64 *>( pSV ); \
    else \
        *pDS++ = *reinterpret_cast<sal_uInt64 *>( pSV );

#define INSERT_INT32( pSV, nr, pGPR, pDS ) \
    if ( nr < x86_64::MAX_GPR_REGS ) \
        pGPR[nr++] = *reinterpret_cast<sal_uInt32 *>( pSV ); \
    else \
        *pDS++ = *reinterpret_cast<sal_uInt32 *>( pSV );

#define INSERT_INT16( pSV, nr, pGPR, pDS ) \
    if ( nr < x86_64::MAX_GPR_REGS ) \
        pGPR[nr++] = *reinterpret_cast<sal_uInt16 *>( pSV ); \
    else \
        *pDS++ = *reinterpret_cast<sal_uInt16 *>( pSV );

#define INSERT_INT8( pSV, nr, pGPR, pDS ) \
    if ( nr < x86_64::MAX_GPR_REGS ) \
        pGPR[nr++] = *reinterpret_cast<sal_uInt8 *>( pSV ); \
    else \
        *pDS++ = *reinterpret_cast<sal_uInt8 *>( pSV );

//==================================================================================================

static void cpp_call(
    bridges::cpp_uno::shared::UnoInterfaceProxy * pThis,
    bridges::cpp_uno::shared::VtableSlot aVtableSlot,
    typelib_TypeDescriptionReference * pReturnTypeRef,
    sal_Int32 nParams, typelib_MethodParameter * pParams,
    void * pUnoReturn, void * pUnoArgs[], uno_Any ** ppUnoExc )
{
    // Maxium space for [complex ret ptr], values | ptr ...
    // (but will be used less - some of the values will be in pGPR and pFPR)
      sal_uInt64 *pStack = (sal_uInt64 *)__builtin_alloca( (nParams + 3) * sizeof(sal_uInt64) );
      sal_uInt64 *pStackStart = pStack;

    sal_uInt64 pGPR[x86_64::MAX_GPR_REGS];
    sal_uInt32 nGPR = 0;

    double pFPR[x86_64::MAX_SSE_REGS];
    sal_uInt32 nFPR = 0;

    // Return
    typelib_TypeDescription * pReturnTypeDescr = 0;
    TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );
    OSL_ENSURE( pReturnTypeDescr, "### expected return type description!" );

    void * pCppReturn = 0; // if != 0 && != pUnoReturn, needs reconversion (see below)

    bool bSimpleReturn = true;
    if ( pReturnTypeDescr )
    {
        if ( x86_64::return_in_hidden_param( pReturnTypeRef ) )
            bSimpleReturn = false;

        if ( bSimpleReturn )
            pCppReturn = pUnoReturn; // direct way for simple types
        else
        {
            // complex return via ptr
            pCppReturn = bridges::cpp_uno::shared::relatesToInterfaceType( pReturnTypeDescr )?
                         __builtin_alloca( pReturnTypeDescr->nSize ) : pUnoReturn;
            INSERT_INT64( &pCppReturn, nGPR, pGPR, pStack );
        }
    }

    // Push "this" pointer
    void * pAdjustedThisPtr = reinterpret_cast< void ** >( pThis->getCppI() ) + aVtableSlot.offset;
    INSERT_INT64( &pAdjustedThisPtr, nGPR, pGPR, pStack );

    // Args
    void ** pCppArgs = (void **)alloca( 3 * sizeof(void *) * nParams );
    // Indizes of values this have to be converted (interface conversion cpp<=>uno)
    sal_Int32 * pTempIndizes = (sal_Int32 *)(pCppArgs + nParams);
    // Type descriptions for reconversions
    typelib_TypeDescription ** ppTempParamTypeDescr = (typelib_TypeDescription **)(pCppArgs + (2 * nParams));

    sal_Int32 nTempIndizes = 0;

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
                INSERT_INT64( pCppArgs[nPos], nGPR, pGPR, pStack );
                break;
            case typelib_TypeClass_LONG:
            case typelib_TypeClass_UNSIGNED_LONG:
            case typelib_TypeClass_ENUM:
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
            case typelib_TypeClass_DOUBLE:
                INSERT_FLOAT_DOUBLE( pCppArgs[nPos], nFPR, pFPR, pStack );
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
            INSERT_INT64( &(pCppArgs[nPos]), nGPR, pGPR, pStack );
        }
    }

    try
    {
        callVirtualMethod(
            pAdjustedThisPtr, aVtableSlot.index,
            pCppReturn, pReturnTypeDescr, bSimpleReturn,
            pStackStart, ( pStack - pStackStart ),
            pGPR, nGPR,
            pFPR, nFPR );
        // NO exception occured...
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

//==================================================================================================

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
            aVtableSlot.index += 1; // get, then set method
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
