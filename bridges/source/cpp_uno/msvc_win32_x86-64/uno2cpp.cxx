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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_bridges.hxx"

#include <malloc.h>

#include <com/sun/star/uno/genfunc.hxx>
#include <uno/data.h>

#include "bridges/cpp_uno/shared/bridge.hxx"
#include "bridges/cpp_uno/shared/types.hxx"
#include "bridges/cpp_uno/shared/unointerfaceproxy.hxx"
#include "bridges/cpp_uno/shared/vtables.hxx"

#include "mscx.hxx"

using namespace ::rtl;
using namespace ::com::sun::star::uno;

namespace
{

// As "long" is 32 bit also in x64 Windows we don't use "longs" in names
// to indicate pointer-sized stack slots etc like in the other x64 archs,
// but "qword" as in ml64.

//==================================================================================================
// In asmbits.asm
extern void callVirtualMethod(
    void * pAdjustedThisPtr, sal_Int32 nVtableIndex,
    void * pReturn, typelib_TypeClass eReturnTypeClass,
    sal_Int64 * pStack, sal_Int32 nStack,
    sal_uInt64 *pGPR,
    double *pFPR);

#if OSL_DEBUG_LEVEL > 1
inline void callVirtualMethodwrapper(
    void * pAdjustedThisPtr, sal_Int32 nVtableIndex,
    void * pReturn, typelib_TypeDescriptionReference * pReturnTypeRef,
    sal_Int64 * pStack, sal_Int32 nStack,
    sal_uInt64 *pGPR, sal_uInt32 nGPR,
    double *pFPR, sal_uInt32 nFPR)
{
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

    callVirtualMethod( pAdjustedThisPtr, nVtableIndex,
                       pReturn, pReturnTypeRef->eTypeClass,
                       pStack, nStack,
                       pGPR,
                       pFPR);
}

#define callVirtualMethod callVirtualMethodwrapper

#endif

//==================================================================================================
static void cpp_call(
    bridges::cpp_uno::shared::UnoInterfaceProxy * pThis,
    bridges::cpp_uno::shared::VtableSlot aVtableSlot,
    typelib_TypeDescriptionReference * pReturnTypeRef,
    sal_Int32 nParams, typelib_MethodParameter * pParams,
    void * pUnoReturn, void * pUnoArgs[], uno_Any ** ppUnoExc ) throw ()
{
    const int MAXPARAMS = 20;

    if (nParams > MAXPARAMS)
    {
        // We have a hard limit on the number of parameters so that we
        // don't need any assembler stuff but can call the function
        // using normal C++.

        // What is the proper way to abort with at least some
        // information given to the user?
        abort();
    }

    // table with optional complex return value ptr, this pointer, and the parameters
    union {
        sal_Int64 i;
        void *p;
        double d;
    } aCppParams[MAXPARAMS+2], uRetVal;
    int nCppParamIndex = 0;

    // Return type
    typelib_TypeDescription * pReturnTypeDescr = 0;
    TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );
    OSL_ENSURE( pReturnTypeDescr, "### expected return type description!" );

    bool bSimpleReturn = true;
    if (pReturnTypeDescr)
    {
        if (pReturnTypeDescr->nSize > 8)
        {
            // complex return via ptr
            bSimpleReturn = false;
            aCppParams[nCppParamIndex++].p = alloca( pReturnTypeDescr->nSize );
        }
    }

    // "this"
    sal_Int64 * pCppThis = (sal_Int64 *) (pThis->getCppI()) + aVtableSlot.offset;
    aCppParams[nCppParamIndex++].p = pCppThis;

    // Indexes of values this have to be converted (interface conversion cpp<=>uno)
    int pTempCppIndexes[MAXPARAMS];
    int pTempIndexes[MAXPARAMS];
    int nTempIndexes = 0;

    // Type descriptions for reconversions
    typelib_TypeDescription *pTempParamTypeDescr[MAXPARAMS];

    for ( int nPos = 0; nPos < nParams; ++nPos )
    {
        const typelib_MethodParameter & rParam = pParams[nPos];
        typelib_TypeDescription * pParamTypeDescr = 0;
        TYPELIB_DANGER_GET( &pParamTypeDescr, rParam.pTypeRef );

        if (!rParam.bOut
            && bridges::cpp_uno::shared::isSimpleType( pParamTypeDescr ))
        {
            ::uno_copyAndConvertData(
                aCppParams[nCppParamIndex++].p, pUnoArgs[nPos], pParamTypeDescr,
                pThis->getBridge()->getUno2Cpp() );

            // no longer needed
            TYPELIB_DANGER_RELEASE( pParamTypeDescr );
        }
        else // ptr to complex value | ref
        {
            if (! rParam.bIn) // is pure out
            {
                // cpp out is constructed mem, uno out is not!
                ::uno_constructData(
                    aCppParams[nCppParamIndex].p = alloca( pParamTypeDescr->nSize ),
                    pParamTypeDescr );

                pTempCppIndexes[nTempIndexes] = nCppParamIndex;
                pTempIndexes[nTempIndexes] = nPos;

                // will be released at reconversion
                pTempParamTypeDescr[nTempIndexes++] = pParamTypeDescr;

                nCppParamIndex++;
            }
            // is in/inout
            else if (bridges::cpp_uno::shared::relatesToInterfaceType(
                         pParamTypeDescr ))
            {
                ::uno_copyAndConvertData(
                    aCppParams[nCppParamIndex].p = alloca( pParamTypeDescr->nSize ),
                    pUnoArgs[nPos], pParamTypeDescr,
                    pThis->getBridge()->getUno2Cpp() );

                pTempCppIndexes[nTempIndexes] = nCppParamIndex;
                pTempIndexes[nTempIndexes] = nPos;

                // will be released at reconversion
                pTempParamTypeDescr[nTempIndexes++] = pParamTypeDescr;

                nCppParamIndex++;
            }
            else // direct way
            {
                aCppParams[nCppParamIndex++].p = pUnoArgs[nPos];
                // no longer needed
                TYPELIB_DANGER_RELEASE( pParamTypeDescr );
            }
        }
    }

    __try
    {
        // The first real parameter is always a pointer: either the
        // address of where to store a complex return value or "this".

        // The Windows x64 calling convention is very regular and
        // elegant (even if perhaps then slightly slower than the
        // Linux x64 one): The first four parameters, never more, are
        // passed in registers, as long as they are a qword ins size
        // or less. (If larger, a pointer to a temp copy is passed, so
        // it's equivalent anyway.) Floating point values are passed
        // in XMM0..3 register, others in RCX, RDX, R8, R9. Now, the
        // nice thing for us is that for varargs functions,
        // floating-point parameters among the four first ones are
        // always passed *both* in XMM and integer registers. So we
        // don't need to bother here calling the method different ways
        // depending on what types of parameters it actually expects,
        // just pretend parameters 3..4 are doubles and they will be
        // passed both in XMM and integer registers, callee will find
        // them where it expects. (The callee is not actually varargs,
        // of course.)

        sal_Int64 (*pMethod)(sal_Int64, ...) =
            (sal_Int64 (*)(sal_Int64, ...))
            (((sal_Int64 *)pCppThis) + aVtableSlot.index);

        // Pass parameters 2..4 as double so that it gets put in both XMM and integer
        // registers per the
        uRetVal.i =
            pMethod (aCppParams[0].i, aCppParams[1].d, aCppParams[2].d, aCppParams[3].d,
                     aCppParams[4].i, aCppParams[5].i, aCppParams[6].i, aCppParams[7].i,
                     aCppParams[8].i, aCppParams[9].i, aCppParams[10].i, aCppParams[11].i,
                     aCppParams[12].i, aCppParams[13].i, aCppParams[14].i, aCppParams[15].i,
                     aCppParams[16].i, aCppParams[17].i, aCppParams[18].i, aCppParams[19].i );
    }
    __except (CPPU_CURRENT_NAMESPACE::mscx_filterCppException(
                  GetExceptionInformation(),
                  *ppUnoExc, pThis->getBridge()->getCpp2Uno() ))
   {
        // *ppUnoExc was constructed by filter function
        // temporary params
        while (nTempIndexes--)
        {
            int nCppIndex = pTempCppIndexes[nTempIndexes];
            // destroy temp cpp param => cpp: every param was constructed
            ::uno_destructData(
                aCppParams[nCppIndex].p, pTempParamTypeDescr[nTempIndexes],
                cpp_release );
            TYPELIB_DANGER_RELEASE( pTempParamTypeDescr[nTempIndexes] );
        }
        // return type
        if (pReturnTypeDescr)
        {
            TYPELIB_DANGER_RELEASE( pReturnTypeDescr );
        }
        // end here
        return;
    }

    // NO exception occurred
    *ppUnoExc = 0;

    // Reconvert temporary params
    while (nTempIndexes--)
    {
        int nCppIndex = pTempCppIndexes[nTempIndexes];
        int nIndex = pTempIndexes[nTempIndexes];
        typelib_TypeDescription * pParamTypeDescr =
            pTempParamTypeDescr[nTempIndexes];

        if (pParams[nIndex].bIn)
        {
            if (pParams[nIndex].bOut) // inout
            {
                ::uno_destructData(
                    pUnoArgs[nIndex], pParamTypeDescr, 0 ); // destroy uno value
                ::uno_copyAndConvertData(
                    pUnoArgs[nIndex], aCppParams[nCppIndex].p, pParamTypeDescr,
                    pThis->getBridge()->getCpp2Uno() );
            }
        }
        else // pure out
        {
            ::uno_copyAndConvertData(
                pUnoArgs[nIndex], aCppParams[nCppIndex].p, pParamTypeDescr,
                pThis->getBridge()->getCpp2Uno() );
        }
        // destroy temp cpp param => cpp: every param was constructed
        ::uno_destructData(
            aCppParams[nCppIndex].p, pParamTypeDescr, cpp_release );

        TYPELIB_DANGER_RELEASE( pParamTypeDescr );
    }
    // return value
    if (!bSimpleReturn)
    {
        ::uno_copyAndConvertData(
            pUnoReturn, uRetVal.p, pReturnTypeDescr,
            pThis->getBridge()->getCpp2Uno() );
        ::uno_destructData(
            aCppParams[0].p, pReturnTypeDescr, cpp_release );
    }
    // return type
    if (pReturnTypeDescr)
    {
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
