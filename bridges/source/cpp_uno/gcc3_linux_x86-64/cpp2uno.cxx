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


#include <stdio.h>
#include <stdlib.h>
#include <boost/unordered_map.hpp>

#include <rtl/alloc.h>
#include <osl/mutex.hxx>

#include <com/sun/star/uno/genfunc.hxx>
#include "com/sun/star/uno/RuntimeException.hpp"
#include <uno/data.h>
#include <typelib/typedescription.hxx>

#include "bridges/cpp_uno/shared/bridge.hxx"
#include "bridges/cpp_uno/shared/cppinterfaceproxy.hxx"
#include "bridges/cpp_uno/shared/types.hxx"
#include "bridges/cpp_uno/shared/vtablefactory.hxx"

#include "abi.hxx"
#include "share.hxx"

using namespace ::osl;
using namespace ::rtl;
using namespace ::com::sun::star::uno;

//==================================================================================================

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
    typelib_TypeDescription * pReturnTypeDescr = 0;
    if (pReturnTypeRef)
        TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );

    void * pUnoReturn = 0;
    void * pCppReturn = 0; // complex return ptr: if != 0 && != pUnoReturn, reconversion need

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
    void ** pUnoArgs = (void **)alloca( 4 * sizeof(void *) * nParams );
    void ** pCppArgs = pUnoArgs + nParams;
    // indizes of values this have to be converted (interface conversion cpp<=>uno)
    sal_Int32 * pTempIndizes = (sal_Int32 *)(pUnoArgs + (2 * nParams));
    // type descriptions for reconversions
    typelib_TypeDescription ** ppTempParamTypeDescr = (typelib_TypeDescription **)(pUnoArgs + (3 * nParams));

    sal_Int32 nTempIndizes = 0;

    for ( sal_Int32 nPos = 0; nPos < nParams; ++nPos )
    {
        const typelib_MethodParameter & rParam = pParams[nPos];

        int nUsedGPR = 0;
        int nUsedSSE = 0;
#if OSL_DEBUG_LEVEL > 0
        bool bFitsRegisters =
#endif
            x86_64::examine_argument( rParam.pTypeRef, false, nUsedGPR, nUsedSSE );
        if ( !rParam.bOut && bridges::cpp_uno::shared::isSimpleType( rParam.pTypeRef ) ) // value
        {
            // Simple types must fit exactly one register on x86_64
            OSL_ASSERT( bFitsRegisters && ( ( nUsedSSE == 1 && nUsedGPR == 0 ) || ( nUsedSSE == 0 && nUsedGPR == 1 ) ) );

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
            typelib_TypeDescription * pParamTypeDescr = 0;
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
                pTempIndizes[nTempIndizes] = nPos;
                // will be released at reconversion
                ppTempParamTypeDescr[nTempIndizes++] = pParamTypeDescr;
            }
            else if ( bridges::cpp_uno::shared::relatesToInterfaceType( pParamTypeDescr ) ) // is in/inout
            {
                uno_copyAndConvertData( pUnoArgs[nPos] = alloca( pParamTypeDescr->nSize ),
                                        pCppStack, pParamTypeDescr,
                                        pThis->getBridge()->getCpp2Uno() );
                pTempIndizes[nTempIndizes] = nPos; // has to be reconverted
                // will be released at reconversion
                ppTempParamTypeDescr[nTempIndizes++] = pParamTypeDescr;
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
        for ( ; nTempIndizes--; )
        {
            sal_Int32 nIndex = pTempIndizes[nTempIndizes];

            if (pParams[nIndex].bIn) // is in/inout => was constructed
                uno_destructData( pUnoArgs[nIndex], ppTempParamTypeDescr[nTempIndizes], 0 );
            TYPELIB_DANGER_RELEASE( ppTempParamTypeDescr[nTempIndizes] );
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
        for ( ; nTempIndizes--; )
        {
            sal_Int32 nIndex = pTempIndizes[nTempIndizes];
            typelib_TypeDescription * pParamTypeDescr = ppTempParamTypeDescr[nTempIndizes];

            if ( pParams[nIndex].bOut ) // inout/out
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
        if ( pCppReturn ) // has complex return
        {
            if ( pUnoReturn != pCppReturn ) // needs reconversion
            {
                uno_copyAndConvertData( pCppReturn, pUnoReturn, pReturnTypeDescr,
                                        pThis->getBridge()->getUno2Cpp() );
                // destroy temp uno return
                uno_destructData( pUnoReturn, pReturnTypeDescr, 0 );
            }
            // complex return ptr is set to return reg
            *(void **)pRegisterReturn = pCppReturn;
        }
        if ( pReturnTypeDescr )
        {
            typelib_TypeClass eRet = (typelib_TypeClass)pReturnTypeDescr->eTypeClass;
            TYPELIB_DANGER_RELEASE( pReturnTypeDescr );
            return eRet;
        }
        else
            return typelib_TypeClass_VOID;
    }
}


//==================================================================================================
extern "C" typelib_TypeClass cpp_vtable_call(
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

    OSL_ENSURE( nFunctionIndex < pTypeDescr->nMapFunctionIndexToMemberIndex, "### illegal vtable index!\n" );
    if ( nFunctionIndex >= pTypeDescr->nMapFunctionIndexToMemberIndex )
    {
        throw RuntimeException( OUString(RTL_CONSTASCII_USTRINGPARAM("illegal vtable index!")),
                                reinterpret_cast<XInterface *>( pCppI ) );
    }

    // determine called method
    sal_Int32 nMemberPos = pTypeDescr->pMapFunctionIndexToMemberIndex[nFunctionIndex];
    OSL_ENSURE( nMemberPos < pTypeDescr->nAllMembers, "### illegal member index!\n" );

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
                        0, 0, // no params
                        gpreg, fpreg, ovrflw, pRegisterReturn );
            }
            else
            {
                // is SET method
                typelib_MethodParameter aParam;
                aParam.pTypeRef = pAttrTypeRef;
                aParam.bIn      = sal_True;
                aParam.bOut     = sal_False;

                eRet = cpp2uno_call( pCppI, aMemberDescr.get(),
                        0, // indicates void return
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
                    typelib_TypeDescription * pTD = 0;
                    TYPELIB_DANGER_GET( &pTD, reinterpret_cast<Type *>( gpreg[2] )->getTypeLibType() );
                    if ( pTD )
                    {
                        XInterface * pInterface = 0;
                        (*pCppI->getBridge()->getCppEnv()->getRegisteredInterface)
                            ( pCppI->getBridge()->getCppEnv(),
                              (void **)&pInterface,
                              pCppI->getOid().pData,
                              reinterpret_cast<typelib_InterfaceTypeDescription *>( pTD ) );

                        if ( pInterface )
                        {
                            ::uno_any_construct( reinterpret_cast<uno_Any *>( gpreg[0] ),
                                                 &pInterface, pTD, cpp_acquire );

                            pInterface->release();
                            TYPELIB_DANGER_RELEASE( pTD );

                            reinterpret_cast<void **>( pRegisterReturn )[0] = gpreg[0];
                            eRet = typelib_TypeClass_ANY;
                            break;
                        }
                        TYPELIB_DANGER_RELEASE( pTD );
                    }
                } // else perform queryInterface()
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
            throw RuntimeException( OUString(RTL_CONSTASCII_USTRINGPARAM("no member description found!")),
                                    reinterpret_cast<XInterface *>( pCppI ) );
        }
    }

    return eRet;
}

//==================================================================================================
extern "C" void privateSnippetExecutor( ... );

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
        bool bHasHiddenParam ) SAL_THROW(())
{
    sal_uInt64 nOffsetAndIndex = ( ( (sal_uInt64) nVtableOffset ) << 32 ) | ( (sal_uInt64) nFunctionIndex );

    if ( bHasHiddenParam )
        nOffsetAndIndex |= 0x80000000;

    // movq $<nOffsetAndIndex>, %r10
    *reinterpret_cast<sal_uInt16 *>( code ) = 0xba49;
    *reinterpret_cast<sal_uInt64 *>( code + 2 ) = nOffsetAndIndex;

    // movq $<address of the privateSnippetExecutor>, %r11
    *reinterpret_cast<sal_uInt16 *>( code + 10 ) = 0xbb49;
    *reinterpret_cast<sal_uInt64 *>( code + 12 ) = reinterpret_cast<sal_uInt64>( privateSnippetExecutor );

    // jmpq *%r11
    *reinterpret_cast<sal_uInt32 *>( code + 20 ) = 0x00e3ff49;

#if OSL_DEBUG_LEVEL > 1
    fprintf(stderr,
            "==> codeSnippet, functionIndex=%d%s, vtableOffset=%d\n",
            nFunctionIndex, (bHasHiddenParam ? "|0x80000000":""), nVtableOffset);
#endif

    return code + codeSnippetSize;
}

//==================================================================================================
struct bridges::cpp_uno::shared::VtableFactory::Slot { void * fn; };

bridges::cpp_uno::shared::VtableFactory::Slot *
bridges::cpp_uno::shared::VtableFactory::mapBlockToVtable(void * block)
{
    return static_cast< Slot * >(block) + 2;
}

//==================================================================================================
sal_Size bridges::cpp_uno::shared::VtableFactory::getBlockSize(
    sal_Int32 slotCount)
{
    return (slotCount + 2) * sizeof (Slot) + slotCount * codeSnippetSize;
}

//==================================================================================================
bridges::cpp_uno::shared::VtableFactory::Slot *
bridges::cpp_uno::shared::VtableFactory::initializeBlock(
    void * block, sal_Int32 slotCount)
{
    Slot * slots = mapBlockToVtable(block);
    slots[-2].fn = 0;
    slots[-1].fn = 0;
    return slots + slotCount;
}

//==================================================================================================

unsigned char * bridges::cpp_uno::shared::VtableFactory::addLocalFunctions(
    Slot ** slots, unsigned char * code, sal_PtrDiff writetoexecdiff,
    typelib_InterfaceTypeDescription const * type, sal_Int32 nFunctionOffset,
    sal_Int32 functionCount, sal_Int32 nVtableOffset )
{
    (*slots) -= functionCount;
    Slot * s = *slots;
    for ( sal_Int32 nPos = 0; nPos < type->nMembers; ++nPos )
    {
        typelib_TypeDescription * pTD = 0;

        TYPELIB_DANGER_GET( &pTD, type->ppMembers[ nPos ] );
        OSL_ASSERT( pTD );

        if ( typelib_TypeClass_INTERFACE_ATTRIBUTE == pTD->eTypeClass )
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
        else if ( typelib_TypeClass_INTERFACE_METHOD == pTD->eTypeClass )
        {
            typelib_InterfaceMethodTypeDescription *pMethodTD =
                reinterpret_cast<typelib_InterfaceMethodTypeDescription *>( pTD );

            (s++)->fn = code + writetoexecdiff;
            code = codeSnippet( code, nFunctionOffset++, nVtableOffset,
                                x86_64::return_in_hidden_param( pMethodTD->pReturnTypeRef ) );
        }
        else
            OSL_ASSERT( false );

        TYPELIB_DANGER_RELEASE( pTD );
    }
    return code;
}

//==================================================================================================
void bridges::cpp_uno::shared::VtableFactory::flushCode(
    SAL_UNUSED_PARAMETER unsigned char const *,
    SAL_UNUSED_PARAMETER unsigned char const * )
{}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
