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

#include "mscx.hxx"

#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif

using namespace ::rtl;
using namespace ::com::sun::star::uno;

namespace
{

static bool cpp_call(
    bridges::cpp_uno::shared::UnoInterfaceProxy * pThis,
    bridges::cpp_uno::shared::VtableSlot aVtableSlot,
    typelib_TypeDescriptionReference * pReturnTypeRef,
    sal_Int32 nParams,
    typelib_MethodParameter * pParams,
    void * pUnoReturn,
    void * pUnoArgs[],
    uno_Any ** ppUnoExc ) throw ()
{
    const int MAXPARAMS = 20;

    if ( nParams > MAXPARAMS )
    {
        // We have a hard limit on the number of parameters so that we
        // don't need any assembler code here but can call the
        // function using normal C++.

        return false;
    }

    // Table with this pointer, optional complex return value ptr, and the parameters
    union {
        sal_Int64 i;
        void *p;
        double d;
    } aCppParams[MAXPARAMS+2], uRetVal;
    int nCppParamIndex = 0;

    // Return type
    typelib_TypeDescription * pReturnTD = NULL;
    TYPELIB_DANGER_GET( &pReturnTD, pReturnTypeRef );
    OSL_ENSURE( pReturnTD, "### expected return type description!" );

    // 'this'
    void * pAdjustedThisPtr = (void **)( pThis->getCppI() ) + aVtableSlot.offset;
    aCppParams[nCppParamIndex++].p = pAdjustedThisPtr;

    bool bSimpleReturn = true;
    if ( pReturnTD )
    {
        if ( !bridges::cpp_uno::shared::isSimpleType( pReturnTD ) )
        {
            // Complex return via ptr
            bSimpleReturn = false;
            aCppParams[nCppParamIndex++].p =
                bridges::cpp_uno::shared::relatesToInterfaceType( pReturnTD )?
                         alloca( pReturnTD->nSize ) : pUnoReturn;
        }
    }

    // Indexes of values this have to be converted (interface conversion C++<=>UNO)
    int pTempCppIndexes[MAXPARAMS];
    int pTempIndexes[MAXPARAMS];
    int nTempIndexes = 0;

    // Type descriptions for reconversions
    typelib_TypeDescription *pTempParamTypeDescr[MAXPARAMS];

    for ( int nPos = 0; nPos < nParams; ++nPos, ++nCppParamIndex )
    {
        const typelib_MethodParameter & rParam = pParams[nPos];

        typelib_TypeDescription * pParamTD = NULL;
        TYPELIB_DANGER_GET( &pParamTD, rParam.pTypeRef );

        if ( !rParam.bOut &&
             bridges::cpp_uno::shared::isSimpleType( pParamTD ) )
        {
            ::uno_copyAndConvertData(
                &aCppParams[nCppParamIndex], pUnoArgs[nPos], pParamTD,
                pThis->getBridge()->getUno2Cpp() );

            // No longer needed
            TYPELIB_DANGER_RELEASE( pParamTD );
        }
        else // Ptr to complex value | ref
        {
            if ( !rParam.bIn ) // Is pure out
            {
                // C++ out is constructed mem, UNO out is not!
                ::uno_constructData(
                    aCppParams[nCppParamIndex].p = alloca( pParamTD->nSize ),
                    pParamTD );

                pTempCppIndexes[nTempIndexes] = nCppParamIndex;
                pTempIndexes[nTempIndexes] = nPos;

                // Will be released at reconversion
                pTempParamTypeDescr[nTempIndexes++] = pParamTD;

            }
            // Is in/inout
            else if ( bridges::cpp_uno::shared::relatesToInterfaceType( pParamTD ) )
            {
                ::uno_copyAndConvertData(
                    aCppParams[nCppParamIndex].p = alloca( pParamTD->nSize ),
                    pUnoArgs[nPos], pParamTD,
                    pThis->getBridge()->getUno2Cpp() );

                pTempCppIndexes[nTempIndexes] = nCppParamIndex;
                pTempIndexes[nTempIndexes] = nPos;

                // Will be released at reconversion
                pTempParamTypeDescr[nTempIndexes++] = pParamTD;
            }
            else // direct way
            {
                aCppParams[nCppParamIndex].p = pUnoArgs[nPos];

                // No longer needed
                TYPELIB_DANGER_RELEASE( pParamTD );
            }
        }
    }

    __try
    {
        // The first real parameter is always 'this'.

        // The Windows x64 calling convention is very regular and
        // elegant (even if perhaps then slightly slower than the
        // Linux x64 one): The first four parameters, never more, are
        // passed in registers, as long as they are a qword in size
        // or less. (If larger, a pointer to a temp copy is passed, so
        // it's equivalent anyway.) Floating point values are passed
        // in XMM0..3 registers, others in RCX, RDX, R8, R9.

        // Now, the nice thing for us is that when calling varargs
        // functions, floating-point parameters among the four first
        // ones are always passed *both* in an XMM and integer
        // register. So we don't need to bother here calling the
        // method different ways depending on what types of parameters
        // it actually expects. We just pretend parameters 3..4 are
        // doubles, and they will be passed both in XMM and integer
        // registers, and the callee will find them where it
        // expects. (The callee is not actually varargs, of course.)

        sal_Int64 (*pIMethod)(sal_Int64, ...) =
            (sal_Int64 (*)(sal_Int64, ...))
            (*((sal_uInt64 **)pAdjustedThisPtr))[aVtableSlot.index];

        double (*pFMethod)(sal_Int64, ...) =
            (double (*)(sal_Int64, ...))
            (*((sal_uInt64 **)pAdjustedThisPtr))[aVtableSlot.index];

        // Pass parameters 2..4 as if it was a floating-point value so
        // that it gets put in both XMM and integer registers per the
        // calling convention. It doesn't matter if it actually is a
        // fp or not.

        if ( pReturnTD &&
             (pReturnTD->eTypeClass == typelib_TypeClass_FLOAT ||
              pReturnTD->eTypeClass == typelib_TypeClass_DOUBLE) )
            uRetVal.d =
                pFMethod (aCppParams[0].i, aCppParams[1].d, aCppParams[2].d, aCppParams[3].d,
                          aCppParams[4].i, aCppParams[5].i, aCppParams[6].i, aCppParams[7].i,
                          aCppParams[8].i, aCppParams[9].i, aCppParams[10].i, aCppParams[11].i,
                          aCppParams[12].i, aCppParams[13].i, aCppParams[14].i, aCppParams[15].i,
                          aCppParams[16].i, aCppParams[17].i, aCppParams[18].i, aCppParams[19].i );
        else
            uRetVal.i =
                pIMethod (aCppParams[0].i, aCppParams[1].d, aCppParams[2].d, aCppParams[3].d,
                          aCppParams[4].i, aCppParams[5].i, aCppParams[6].i, aCppParams[7].i,
                          aCppParams[8].i, aCppParams[9].i, aCppParams[10].i, aCppParams[11].i,
                          aCppParams[12].i, aCppParams[13].i, aCppParams[14].i, aCppParams[15].i,
                          aCppParams[16].i, aCppParams[17].i, aCppParams[18].i, aCppParams[19].i );
    }
    __except (CPPU_CURRENT_NAMESPACE::mscx_filterCppException(
                  GetExceptionInformation(),
                  *ppUnoExc, pThis->getBridge()->getCpp2Uno() ))
   {
        // *ppUnoExc was constructed by filter function.
        // Temporary params
        while ( nTempIndexes-- )
        {
            int nCppIndex = pTempCppIndexes[nTempIndexes];
            // Destroy temp C++ param => C++: every param was constructed
            ::uno_destructData(
                aCppParams[nCppIndex].p, pTempParamTypeDescr[nTempIndexes],
                cpp_release );
            TYPELIB_DANGER_RELEASE( pTempParamTypeDescr[nTempIndexes] );
        }
        // Return type
        if ( pReturnTD )
            TYPELIB_DANGER_RELEASE( pReturnTD );

        // End here
        return true;
    }

    // No exception occurred
    *ppUnoExc = NULL;

    // Reconvert temporary params
    while ( nTempIndexes-- )
    {
        int nCppIndex = pTempCppIndexes[nTempIndexes];
        int nIndex = pTempIndexes[nTempIndexes];
        typelib_TypeDescription * pParamTD =
            pTempParamTypeDescr[nTempIndexes];

        if ( pParams[nIndex].bIn )
        {
            if ( pParams[nIndex].bOut ) // Inout
            {
                ::uno_destructData(
                    pUnoArgs[nIndex], pParamTD, 0 ); // Destroy UNO value
                ::uno_copyAndConvertData(
                    pUnoArgs[nIndex], aCppParams[nCppIndex].p, pParamTD,
                    pThis->getBridge()->getCpp2Uno() );
            }
        }
        else // Pure out
        {
            ::uno_copyAndConvertData(
                pUnoArgs[nIndex], aCppParams[nCppIndex].p, pParamTD,
                pThis->getBridge()->getCpp2Uno() );
        }

        // Destroy temp C++ param => C++: every param was constructed
        ::uno_destructData(
            aCppParams[nCppIndex].p, pParamTD, cpp_release );

        TYPELIB_DANGER_RELEASE( pParamTD );
    }

    // Return value
    if ( !bSimpleReturn )
    {
        ::uno_copyAndConvertData(
            pUnoReturn, uRetVal.p, pReturnTD,
            pThis->getBridge()->getCpp2Uno() );
        ::uno_destructData(
            aCppParams[1].p, pReturnTD, cpp_release );
    }
    else if ( pUnoReturn )
        *(sal_Int64*)pUnoReturn = uRetVal.i;

    if ( pReturnTD )
        TYPELIB_DANGER_RELEASE( pReturnTD );

    return true;
}

}

namespace bridges { namespace cpp_uno { namespace shared {

void unoInterfaceProxyDispatch(
    uno_Interface * pUnoI,
    const typelib_TypeDescription * pMemberTD,
    void * pReturn,
    void * pArgs[],
    uno_Any ** ppException )
{
    // is my surrogate
    bridges::cpp_uno::shared::UnoInterfaceProxy * pThis
        = static_cast< bridges::cpp_uno::shared::UnoInterfaceProxy * >(pUnoI);
#if OSL_DEBUG_LEVEL > 0
    typelib_InterfaceTypeDescription * pTypeDescr = pThis->pTypeDescr;
#endif

    switch (pMemberTD->eTypeClass)
    {
    case typelib_TypeClass_INTERFACE_ATTRIBUTE:
    {
#if OSL_DEBUG_LEVEL > 0
        // determine vtable call index
        sal_Int32 nMemberPos = ((typelib_InterfaceMemberTypeDescription *)pMemberTD)->nPosition;
        OSL_ENSURE( nMemberPos < pTypeDescr->nAllMembers, "### member pos out of range!" );
#endif
        VtableSlot aVtableSlot(
            getVtableSlot(
                reinterpret_cast<
                    typelib_InterfaceAttributeTypeDescription const * >(
                        pMemberTD)));
        if ( pReturn )
        {
            // Is GET
            cpp_call(
                pThis, aVtableSlot,
                ((typelib_InterfaceAttributeTypeDescription *)pMemberTD)->pAttributeTypeRef,
                0, NULL, // no params
                pReturn, pArgs, ppException );
        }
        else
        {
            // Is SET
            typelib_MethodParameter aParam;
            aParam.pTypeRef =
                ((typelib_InterfaceAttributeTypeDescription *)pMemberTD)->pAttributeTypeRef;
            aParam.bIn      = sal_True;
            aParam.bOut     = sal_False;

            typelib_TypeDescriptionReference * pReturnTypeRef = NULL;
            OUString aVoidName("void");
            typelib_typedescriptionreference_new(
                &pReturnTypeRef, typelib_TypeClass_VOID, aVoidName.pData );

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
#if OSL_DEBUG_LEVEL > 0
        // determine vtable call index
        sal_Int32 nMemberPos = ((typelib_InterfaceMemberTypeDescription *)pMemberTD)->nPosition;
        OSL_ENSURE( nMemberPos < pTypeDescr->nAllMembers, "### member pos out of range!" );
#endif
        VtableSlot aVtableSlot(
            getVtableSlot(
                reinterpret_cast<
                    typelib_InterfaceMethodTypeDescription const * >(
                        pMemberTD)));

        switch (aVtableSlot.index)
        {
        // Standard calls
        case 1: // Acquire UNO interface
            (*pUnoI->acquire)( pUnoI );
            *ppException = 0;
            break;
        case 2: // Release UNO interface
            (*pUnoI->release)( pUnoI );
            *ppException = 0;
            break;
        case 0: // queryInterface() opt
        {
            typelib_TypeDescription * pTD = NULL;
            TYPELIB_DANGER_GET( &pTD, reinterpret_cast< Type * >( pArgs[0] )->getTypeLibType() );

            if ( pTD )
            {
                uno_Interface * pInterface = NULL;
                (*pThis->getBridge()->getUnoEnv()->getRegisteredInterface)(
                    pThis->getBridge()->getUnoEnv(),
                    (void **)&pInterface, pThis->oid.pData, (typelib_InterfaceTypeDescription *)pTD );

                if ( pInterface )
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
        } // Else perform queryInterface()
        default:
            if ( ! cpp_call(
                     pThis, aVtableSlot,
                     ((typelib_InterfaceMethodTypeDescription *)pMemberTD)->pReturnTypeRef,
                     ((typelib_InterfaceMethodTypeDescription *)pMemberTD)->nParams,
                     ((typelib_InterfaceMethodTypeDescription *)pMemberTD)->pParams,
                     pReturn, pArgs, ppException ) )
            {
                RuntimeException aExc(
                    OUString("Too many parameters!"),
                    Reference< XInterface >() );

                Type const & rExcType = ::getCppuType( &aExc );
                ::uno_type_any_construct( *ppException, &aExc, rExcType.getTypeLibType(), 0 );
            }
        }
        break;
    }
    default:
    {
        RuntimeException aExc(
            OUString("Illegal member type description!"),
            Reference< XInterface >() );

        Type const & rExcType = ::getCppuType( &aExc );
        // Binary identical null reference (whatever that comment means...)
        ::uno_type_any_construct( *ppException, &aExc, rExcType.getTypeLibType(), 0 );
    }
    }
}

} } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
