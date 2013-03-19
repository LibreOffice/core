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

using namespace ::rtl;
using namespace ::com::sun::star::uno;

void callVirtualMethod(void * pThis, sal_uInt32 nVtableIndex,
    void * pRegisterReturn, typelib_TypeDescription *pReturnTypeDescr, bool bRegisterReturn,
    sal_uInt32 *pStack, sal_uInt32 nStack, sal_uInt32 *pGPR, double *pFPR);

#define INSERT_INT32( pSV, nr, pGPR, pDS, bOverFlow )\
        if (nr < hppa::MAX_WORDS_IN_REGS) \
        { \
            pGPR[nr++] = *reinterpret_cast<sal_uInt32 *>( pSV ); \
        } \
        else \
            bOverFlow = true; \
        if (bOverFlow) \
            *pDS++ = *reinterpret_cast<sal_uInt32 *>( pSV );

#define INSERT_INT64( pSV, nr, pGPR, pDS, pStart, bOverFlow )\
    if ( (nr < hppa::MAX_WORDS_IN_REGS) && (nr % 2) ) \
    { \
        ++nr; \
    } \
    if ( nr < hppa::MAX_WORDS_IN_REGS ) \
    { \
        pGPR[nr++] = *reinterpret_cast<sal_uInt32 *>( pSV ); \
        pGPR[nr++] = *(reinterpret_cast<sal_uInt32 *>( pSV ) + 1); \
    } \
    else \
        bOverFlow = true; \
    if ( bOverFlow ) \
    { \
        if ( (pDS - pStart) % 2) \
            ++pDS; \
        *pDS++ = reinterpret_cast<sal_uInt32 *>( pSV )[1]; \
        *pDS++ = reinterpret_cast<sal_uInt32 *>( pSV )[0]; \
    }

#define INSERT_FLOAT( pSV, nr, pFPR, pDS, bOverFlow ) \
    if (nr < hppa::MAX_WORDS_IN_REGS) \
    { \
        sal_uInt32 *pDouble = (sal_uInt32 *)&(pFPR[nr++]); \
        pDouble[0] = *reinterpret_cast<sal_uInt32 *>( pSV ); \
    } \
    else \
        bOverFlow = true; \
    if (bOverFlow) \
        *pDS++ = *reinterpret_cast<sal_uInt32 *>( pSV );

#define INSERT_DOUBLE( pSV, nr, pFPR, pDS, pStart, bOverFlow ) \
    if ( (nr < hppa::MAX_WORDS_IN_REGS) && (nr % 2) ) \
    { \
        ++nr; \
    } \
    if ( nr < hppa::MAX_WORDS_IN_REGS ) \
    { \
        sal_uInt32 *pDouble = (sal_uInt32 *)&(pFPR[nr+1]); \
        pDouble[0] = *reinterpret_cast<sal_uInt32 *>( pSV ); \
        pDouble[1] = *(reinterpret_cast<sal_uInt32 *>( pSV ) + 1); \
        nr+=2; \
    } \
    else \
        bOverFlow = true; \
    if ( bOverFlow ) \
    { \
        if ( (pDS - pStart) % 2) \
            ++pDS; \
        *pDS++ = reinterpret_cast<sal_uInt32 *>( pSV )[1]; \
        *pDS++ = reinterpret_cast<sal_uInt32 *>( pSV )[0]; \
    }

#define INSERT_INT16( pSV, nr, pGPR, pDS, bOverFlow ) \
    if ( nr < hppa::MAX_WORDS_IN_REGS ) \
        pGPR[nr++] = *reinterpret_cast<sal_uInt16 *>( pSV ); \
    else \
        bOverFlow = true; \
    if (bOverFlow) \
        *pDS++ = *reinterpret_cast<sal_uInt16 *>( pSV );

#define INSERT_INT8(  pSV, nr, pGPR, pDS, bOverFlow ) \
    if ( nr < hppa::MAX_WORDS_IN_REGS ) \
        pGPR[nr++] = *reinterpret_cast<sal_uInt8 *>( pSV ); \
    else \
        bOverFlow = true; \
    if (bOverFlow) \
        *pDS++ = *reinterpret_cast<sal_uInt8 *>( pSV );

namespace hppa
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

    bool isRegisterReturn( typelib_TypeDescriptionReference *pTypeRef )
    {
        if (bridges::cpp_uno::shared::isSimpleType(pTypeRef))
            return true;
        else if (pTypeRef->eTypeClass == typelib_TypeClass_STRUCT || pTypeRef->eTypeClass == typelib_TypeClass_EXCEPTION)
        {
            typelib_TypeDescription * pTypeDescr = 0;
            TYPELIB_DANGER_GET( &pTypeDescr, pTypeRef );

            /* If the struct is larger than 8 bytes, then there is a buffer at r8 to stick the return value into  */
            bool bRet = pTypeDescr->nSize <= 8 && !is_complex_struct(pTypeDescr);

            TYPELIB_DANGER_RELEASE( pTypeDescr );
            return bRet;
        }
        return false;
    }
}


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

    sal_uInt32 pGPR[hppa::MAX_GPR_REGS];
    double pFPR[hppa::MAX_SSE_REGS];
    sal_uInt32 nRegs=0;

    // return
    typelib_TypeDescription * pReturnTypeDescr = 0;
    TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );
    OSL_ENSURE( pReturnTypeDescr, "### expected return type description!" );

    void * pCppReturn = 0; // if != 0 && != pUnoReturn, needs reconversion
    bool bOverFlow = false;
    bool bRegisterReturn = true;

    if (pReturnTypeDescr)
    {

        bRegisterReturn = hppa::isRegisterReturn(pReturnTypeRef);
        if (bRegisterReturn)
            pCppReturn = pUnoReturn; // direct way for simple types
        else
        {
            // complex return via ptr
            pCppReturn = (bridges::cpp_uno::shared::relatesToInterfaceType( pReturnTypeDescr )
                    ? __builtin_alloca( pReturnTypeDescr->nSize )
                    : pUnoReturn); // direct way
        }
    }
    // push this
    void * pAdjustedThisPtr = reinterpret_cast< void ** >(pThis->getCppI())
        + aVtableSlot.offset;
    INSERT_INT32( &pAdjustedThisPtr, nRegs, pGPR, pStack, bOverFlow );

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
            uno_copyAndConvertData( pCppArgs[nPos] = alloca(8), pUnoArgs[nPos],
                pParamTypeDescr, pThis->getBridge()->getUno2Cpp() );

            switch (pParamTypeDescr->eTypeClass)
            {
                case typelib_TypeClass_HYPER:
                case typelib_TypeClass_UNSIGNED_HYPER:
#if OSL_DEBUG_LEVEL > 2
                    fprintf(stderr, "hyper is %llx\n", *((long long*)pCppArgs[nPos]));
#endif
                    INSERT_INT64( pCppArgs[nPos], nRegs, pGPR, pStack, pStackStart, bOverFlow );
                break;
                case typelib_TypeClass_LONG:
                case typelib_TypeClass_UNSIGNED_LONG:
                case typelib_TypeClass_ENUM:
#if OSL_DEBUG_LEVEL > 2
                    fprintf(stderr, "long is %x\n", pCppArgs[nPos]);
#endif
                    INSERT_INT32( pCppArgs[nPos], nRegs, pGPR, pStack, bOverFlow );
                break;
                case typelib_TypeClass_SHORT:
                case typelib_TypeClass_CHAR:
                case typelib_TypeClass_UNSIGNED_SHORT:
                    INSERT_INT16( pCppArgs[nPos], nRegs, pGPR, pStack, bOverFlow );
                break;
                case typelib_TypeClass_BOOLEAN:
                case typelib_TypeClass_BYTE:
                    INSERT_INT8( pCppArgs[nPos], nRegs, pGPR, pStack, bOverFlow );
                break;
                case typelib_TypeClass_FLOAT:
                    INSERT_FLOAT( pCppArgs[nPos], nRegs, pFPR, pStack, bOverFlow );
                break;
                case typelib_TypeClass_DOUBLE:
                    INSERT_DOUBLE( pCppArgs[nPos], nRegs, pFPR, pStack, pStackStart, bOverFlow );
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
            INSERT_INT32( &(pCppArgs[nPos]), nRegs, pGPR, pStack, bOverFlow );
        }
    }

    try
    {
        callVirtualMethod(
            pAdjustedThisPtr, aVtableSlot.index,
            pCppReturn, pReturnTypeDescr, bRegisterReturn,
            pStackStart,
            (pStack - pStackStart), pGPR, pFPR);

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
