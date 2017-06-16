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


#include <stdio.h>
#include <stdlib.h>

#include <rtl/alloc.h>
#include <sal/log.hxx>

#include <com/sun/star/uno/genfunc.hxx>
#include "com/sun/star/uno/RuntimeException.hpp"
#include <config_options.h>
#include <uno/data.h>
#include <typelib/typedescription.hxx>

#include "bridge.hxx"
#include "cppinterfaceproxy.hxx"
#include "types.hxx"
#include "vtablefactory.hxx"

#include "abi.hxx"
#include "call.hxx"
#include "rtti.hxx"
#include "share.hxx"

using namespace ::com::sun::star::uno;

// Perform the UNO call
//
// We must convert the parameters stored in gpreg, fpreg and ovrflw to UNO
// arguments and call pThis->getUnoI()->pDispatcher.
//
// gpreg:  [ret *], this, [gpr params]
// fpreg:  [fpr params]
// ovrflw: [gpr or fpr params (properly aligned)]
//
// [ret *] is present when we are returning a structure bigger than 16 bytes
// Simple types are returned in rax, rdx (int), or xmm0, xmm1 (fp).
// Similarly structures <= 16 bytes are in rax, rdx, xmm0, xmm1 as necessary.
static typelib_TypeClass cpp2uno_call(
    bridges::cpp_uno::shared::CppInterfaceProxy * pThis,
    const typelib_TypeDescription * pMemberTypeDescr,
    typelib_TypeDescriptionReference * pReturnTypeRef, // 0 indicates void return
    sal_Int32 nParams, typelib_MethodParameter * pParams,
    void ** gpreg, void ** fpreg, void ** ovrflw,
    sal_uInt64 * pRegisterReturn /* space for register return */ )
{
    unsigned int nr_gpr = 0; //number of gpr registers used
    unsigned int nr_fpr = 0; //number of fpr registers used

    // return
    typelib_TypeDescription * pReturnTypeDescr = nullptr;
    if (pReturnTypeRef)
        TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );

    void * pUnoReturn = nullptr;
    void * pCppReturn = nullptr; // complex return ptr: if != 0 && != pUnoReturn, reconversion need

    if ( pReturnTypeDescr )
    {
        if ( x86_64::return_in_hidden_param( pReturnTypeRef ) )
        {
            pCppReturn = *gpreg++;
            nr_gpr++;

            pUnoReturn = ( bridges::cpp_uno::shared::relatesToInterfaceType( pReturnTypeDescr )
                           ? alloca( pReturnTypeDescr->nSize )
                           : pCppReturn ); // direct way
        }
        else
            pUnoReturn = pRegisterReturn; // direct way for simple types
    }

    // pop this
    gpreg++;
    nr_gpr++;

    // stack space
    // parameters
    void ** pUnoArgs = static_cast<void **>(alloca( 4 * sizeof(void *) * nParams ));
    void ** pCppArgs = pUnoArgs + nParams;
    // indices of values this have to be converted (interface conversion cpp<=>uno)
    sal_Int32 * pTempIndices = reinterpret_cast<sal_Int32 *>(pUnoArgs + (2 * nParams));
    // type descriptions for reconversions
    typelib_TypeDescription ** ppTempParamTypeDescr = reinterpret_cast<typelib_TypeDescription **>(pUnoArgs + (3 * nParams));

    sal_Int32 nTempIndices = 0;

    for ( sal_Int32 nPos = 0; nPos < nParams; ++nPos )
    {
        const typelib_MethodParameter & rParam = pParams[nPos];

        int nUsedGPR = 0;
        int nUsedSSE = 0;
        bool bFitsRegisters = x86_64::examine_argument( rParam.pTypeRef, false, nUsedGPR, nUsedSSE );
        if ( !rParam.bOut && bridges::cpp_uno::shared::isSimpleType( rParam.pTypeRef ) ) // value
        {
            // Simple types must fit exactly one register on x86_64
            assert( bFitsRegisters && ( ( nUsedSSE == 1 && nUsedGPR == 0 ) || ( nUsedSSE == 0 && nUsedGPR == 1 ) ) ); (void)bFitsRegisters;

            if ( nUsedSSE == 1 )
            {
                if ( nr_fpr < x86_64::MAX_SSE_REGS )
                {
                    pCppArgs[nPos] = pUnoArgs[nPos] = fpreg++;
                    nr_fpr++;
                }
                else
                    pCppArgs[nPos] = pUnoArgs[nPos] = ovrflw++;
            }
            else if ( nUsedGPR == 1 )
            {
                if ( nr_gpr < x86_64::MAX_GPR_REGS )
                {
                    pCppArgs[nPos] = pUnoArgs[nPos] = gpreg++;
                    nr_gpr++;
                }
                else
                    pCppArgs[nPos] = pUnoArgs[nPos] = ovrflw++;
            }
        }
        else // struct <= 16 bytes || ptr to complex value || ref
        {
            typelib_TypeDescription * pParamTypeDescr = nullptr;
            TYPELIB_DANGER_GET( &pParamTypeDescr, rParam.pTypeRef );

            void *pCppStack;
            if ( nr_gpr < x86_64::MAX_GPR_REGS )
            {
                pCppArgs[nPos] = pCppStack = *gpreg++;
                nr_gpr++;
            }
            else
                pCppArgs[nPos] = pCppStack = *ovrflw++;

            if (! rParam.bIn) // is pure out
            {
                // uno out is unconstructed mem!
                pUnoArgs[nPos] = alloca( pParamTypeDescr->nSize );
                pTempIndices[nTempIndices] = nPos;
                // will be released at reconversion
                ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
            }
            else if ( bridges::cpp_uno::shared::relatesToInterfaceType( pParamTypeDescr ) ) // is in/inout
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

    // ExceptionHolder
    uno_Any aUnoExc; // Any will be constructed by callee
    uno_Any * pUnoExc = &aUnoExc;

    // invoke uno dispatch call
    (*pThis->getUnoI()->pDispatcher)( pThis->getUnoI(), pMemberTypeDescr, pUnoReturn, pUnoArgs, &pUnoExc );

    // in case an exception occurred...
    if ( pUnoExc )
    {
        // destruct temporary in/inout params
        for ( ; nTempIndices--; )
        {
            sal_Int32 nIndex = pTempIndices[nTempIndices];

            if (pParams[nIndex].bIn) // is in/inout => was constructed
                uno_destructData( pUnoArgs[nIndex], ppTempParamTypeDescr[nTempIndices], nullptr );
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

            if ( pParams[nIndex].bOut ) // inout/out
            {
                // convert and assign
                uno_destructData( pCppArgs[nIndex], pParamTypeDescr, cpp_release );
                uno_copyAndConvertData( pCppArgs[nIndex], pUnoArgs[nIndex], pParamTypeDescr,
                                        pThis->getBridge()->getUno2Cpp() );
            }
            // destroy temp uno param
            uno_destructData( pUnoArgs[nIndex], pParamTypeDescr, nullptr );

            TYPELIB_DANGER_RELEASE( pParamTypeDescr );
        }
        // return
        if ( pCppReturn ) // has complex return
        {
            if ( pUnoReturn != pCppReturn ) // needs reconversion
            {
                uno_copyAndConvertData( pCppReturn, pUnoReturn, pReturnTypeDescr,
                                        pThis->getBridge()->getUno2Cpp() );
                // destroy temp uno return
                uno_destructData( pUnoReturn, pReturnTypeDescr, nullptr );
            }
            // complex return ptr is set to return reg
            *reinterpret_cast<void **>(pRegisterReturn) = pCppReturn;
        }
        if ( pReturnTypeDescr )
        {
            typelib_TypeClass eRet = pReturnTypeDescr->eTypeClass;
            TYPELIB_DANGER_RELEASE( pReturnTypeDescr );
            return eRet;
        }
        else
            return typelib_TypeClass_VOID;
    }
}


typelib_TypeClass cpp_vtable_call(
    sal_Int32 nFunctionIndex, sal_Int32 nVtableOffset,
    void ** gpreg, void ** fpreg, void ** ovrflw,
    sal_uInt64 * pRegisterReturn /* space for register return */ )
{
    // gpreg:  [ret *], this, [other gpr params]
    // fpreg:  [fpr params]
    // ovrflw: [gpr or fpr params (properly aligned)]
    void * pThis;
    if ( nFunctionIndex & 0x80000000 )
    {
        nFunctionIndex &= 0x7fffffff;
        pThis = gpreg[1];
    }
    else
    {
        pThis = gpreg[0];
    }
    pThis = static_cast<char *>( pThis ) - nVtableOffset;

    bridges::cpp_uno::shared::CppInterfaceProxy * pCppI =
        bridges::cpp_uno::shared::CppInterfaceProxy::castInterfaceToProxy( pThis );

    typelib_InterfaceTypeDescription * pTypeDescr = pCppI->getTypeDescr();

    if ( nFunctionIndex >= pTypeDescr->nMapFunctionIndexToMemberIndex )
    {
        SAL_WARN(
            "bridges",
            "illegal " << OUString::unacquired(&pTypeDescr->aBase.pTypeName)
                << " vtable index " << nFunctionIndex << "/"
                << pTypeDescr->nMapFunctionIndexToMemberIndex);
        throw RuntimeException(
            ("illegal " + OUString::unacquired(&pTypeDescr->aBase.pTypeName)
             + " vtable index " + OUString::number(nFunctionIndex) + "/"
             + OUString::number(pTypeDescr->nMapFunctionIndexToMemberIndex)),
            reinterpret_cast<XInterface *>( pCppI ) );
    }

    // determine called method
    sal_Int32 nMemberPos = pTypeDescr->pMapFunctionIndexToMemberIndex[nFunctionIndex];
    assert(nMemberPos < pTypeDescr->nAllMembers);

    TypeDescription aMemberDescr( pTypeDescr->ppAllMembers[nMemberPos] );

    typelib_TypeClass eRet;
    switch ( aMemberDescr.get()->eTypeClass )
    {
        case typelib_TypeClass_INTERFACE_ATTRIBUTE:
        {
            typelib_TypeDescriptionReference *pAttrTypeRef =
                reinterpret_cast<typelib_InterfaceAttributeTypeDescription *>( aMemberDescr.get() )->pAttributeTypeRef;

            if ( pTypeDescr->pMapMemberIndexToFunctionIndex[nMemberPos] == nFunctionIndex )
            {
                // is GET method
                eRet = cpp2uno_call( pCppI, aMemberDescr.get(), pAttrTypeRef,
                        0, nullptr, // no params
                        gpreg, fpreg, ovrflw, pRegisterReturn );
            }
            else
            {
                // is SET method
                typelib_MethodParameter aParam;
                aParam.pTypeRef = pAttrTypeRef;
                aParam.bIn      = true;
                aParam.bOut     = false;

                eRet = cpp2uno_call( pCppI, aMemberDescr.get(),
                        nullptr, // indicates void return
                        1, &aParam,
                        gpreg, fpreg, ovrflw, pRegisterReturn );
            }
            break;
        }
        case typelib_TypeClass_INTERFACE_METHOD:
        {
            // is METHOD
            switch ( nFunctionIndex )
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
                    typelib_TypeDescription * pTD = nullptr;
                    TYPELIB_DANGER_GET( &pTD, static_cast<Type *>( gpreg[2] )->getTypeLibType() );
                    if ( pTD )
                    {
                        XInterface * pInterface = nullptr;
                        (*pCppI->getBridge()->getCppEnv()->getRegisteredInterface)
                            ( pCppI->getBridge()->getCppEnv(),
                              reinterpret_cast<void **>(&pInterface),
                              pCppI->getOid().pData,
                              reinterpret_cast<typelib_InterfaceTypeDescription *>( pTD ) );

                        if ( pInterface )
                        {
                            ::uno_any_construct( static_cast<uno_Any *>( gpreg[0] ),
                                                 &pInterface, pTD, cpp_acquire );

                            pInterface->release();
                            TYPELIB_DANGER_RELEASE( pTD );

                            reinterpret_cast<void **>( pRegisterReturn )[0] = gpreg[0];
                            eRet = typelib_TypeClass_ANY;
                            break;
                        }
                        TYPELIB_DANGER_RELEASE( pTD );
                    }
                    SAL_FALLTHROUGH; // else perform queryInterface()
                }
                default:
                {
                    typelib_InterfaceMethodTypeDescription *pMethodTD =
                        reinterpret_cast<typelib_InterfaceMethodTypeDescription *>( aMemberDescr.get() );

                    eRet = cpp2uno_call( pCppI, aMemberDescr.get(),
                                         pMethodTD->pReturnTypeRef,
                                         pMethodTD->nParams,
                                         pMethodTD->pParams,
                                         gpreg, fpreg, ovrflw, pRegisterReturn );
                }
            }
            break;
        }
        default:
        {
            throw RuntimeException("no member description found!",
                                    reinterpret_cast<XInterface *>( pCppI ) );
        }
    }

    return eRet;
}

const int codeSnippetSize = 24;

// Generate a trampoline that redirects method calls to
// privateSnippetExecutor().
//
// privateSnippetExecutor() saves all the registers that are used for
// parameter passing on x86_64, and calls the cpp_vtable_call().
// When it returns, privateSnippetExecutor() sets the return value.
//
// Note: The code snippet we build here must not create a stack frame,
// otherwise the UNO exceptions stop working thanks to non-existing
// unwinding info.
unsigned char * codeSnippet( unsigned char * code,
        sal_Int32 nFunctionIndex, sal_Int32 nVtableOffset,
        bool bHasHiddenParam )
{
    sal_uInt64 nOffsetAndIndex = ( ( (sal_uInt64) nVtableOffset ) << 32 ) | ( (sal_uInt64) nFunctionIndex );

    if ( bHasHiddenParam )
        nOffsetAndIndex |= 0x80000000;

    // movq $<nOffsetAndIndex>, %r10
    *reinterpret_cast<sal_uInt16 *>( code ) = 0xba49;
    *reinterpret_cast<sal_uInt16 *>( code + 2 ) = nOffsetAndIndex & 0xFFFF;
    *reinterpret_cast<sal_uInt32 *>( code + 4 ) = nOffsetAndIndex >> 16;
    *reinterpret_cast<sal_uInt16 *>( code + 8 ) = nOffsetAndIndex >> 48;

    // movq $<address of the privateSnippetExecutor>, %r11
    *reinterpret_cast<sal_uInt16 *>( code + 10 ) = 0xbb49;
    *reinterpret_cast<sal_uInt32 *>( code + 12 )
        = reinterpret_cast<sal_uInt64>(privateSnippetExecutor);
    *reinterpret_cast<sal_uInt32 *>( code + 16 )
        = reinterpret_cast<sal_uInt64>(privateSnippetExecutor) >> 32;

    // jmpq *%r11
    *reinterpret_cast<sal_uInt32 *>( code + 20 ) = 0x00e3ff49;

#if OSL_DEBUG_LEVEL > 1
    fprintf(stderr,
            "==> codeSnippet, functionIndex=%d%s, vtableOffset=%d\n",
            nFunctionIndex, (bHasHiddenParam ? "|0x80000000":""), nVtableOffset);
#endif

    return code + codeSnippetSize;
}

struct bridges::cpp_uno::shared::VtableFactory::Slot { void * fn; };

bridges::cpp_uno::shared::VtableFactory::Slot *
bridges::cpp_uno::shared::VtableFactory::mapBlockToVtable(void * block)
{
    return static_cast< Slot * >(block) + 2;
}

std::size_t bridges::cpp_uno::shared::VtableFactory::getBlockSize(
    sal_Int32 slotCount)
{
    return (slotCount + 2) * sizeof (Slot) + slotCount * codeSnippetSize;
}

bridges::cpp_uno::shared::VtableFactory::Slot *
bridges::cpp_uno::shared::VtableFactory::initializeBlock(
    void * block, sal_Int32 slotCount, sal_Int32 vtableNumber,
    typelib_InterfaceTypeDescription * type)
{
    Slot * slots = mapBlockToVtable(block);
    slots[-2].fn = reinterpret_cast<void *>(-(vtableNumber * sizeof (void *)));
#if ENABLE_RUNTIME_OPTIMIZATIONS
    slots[-1].fn = nullptr;
    (void)type;
#else
    slots[-1].fn = x86_64::getRtti(type->aBase);
#endif
    return slots + slotCount;
}


unsigned char * bridges::cpp_uno::shared::VtableFactory::addLocalFunctions(
    Slot ** slots, unsigned char * code, sal_PtrDiff writetoexecdiff,
    typelib_InterfaceTypeDescription const * type, sal_Int32 nFunctionOffset,
    sal_Int32 functionCount, sal_Int32 nVtableOffset )
{
    (*slots) -= functionCount;
    Slot * s = *slots;
    for ( sal_Int32 nPos = 0; nPos < type->nMembers; ++nPos )
    {
        typelib_TypeDescription * pTD = nullptr;

        TYPELIB_DANGER_GET( &pTD, type->ppMembers[ nPos ] );
        assert(pTD);

        if ( pTD->eTypeClass == typelib_TypeClass_INTERFACE_ATTRIBUTE )
        {
            typelib_InterfaceAttributeTypeDescription *pAttrTD =
                reinterpret_cast<typelib_InterfaceAttributeTypeDescription *>( pTD );

            // get method
            (s++)->fn = code + writetoexecdiff;
            code = codeSnippet( code, nFunctionOffset++, nVtableOffset,
                                x86_64::return_in_hidden_param( pAttrTD->pAttributeTypeRef ) );

            if ( ! pAttrTD->bReadOnly )
            {
                // set method
                (s++)->fn = code + writetoexecdiff;
                code = codeSnippet( code, nFunctionOffset++, nVtableOffset, false );
            }
        }
        else if ( pTD->eTypeClass == typelib_TypeClass_INTERFACE_METHOD )
        {
            typelib_InterfaceMethodTypeDescription *pMethodTD =
                reinterpret_cast<typelib_InterfaceMethodTypeDescription *>( pTD );

            (s++)->fn = code + writetoexecdiff;
            code = codeSnippet( code, nFunctionOffset++, nVtableOffset,
                                x86_64::return_in_hidden_param( pMethodTD->pReturnTypeRef ) );
        }
        else
            assert(false);

        TYPELIB_DANGER_RELEASE( pTD );
    }
    return code;
}

void bridges::cpp_uno::shared::VtableFactory::flushCode(
    SAL_UNUSED_PARAMETER unsigned char const *,
    SAL_UNUSED_PARAMETER unsigned char const * )
{}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
