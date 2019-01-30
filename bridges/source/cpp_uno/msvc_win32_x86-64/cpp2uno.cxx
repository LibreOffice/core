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
#include <sal/log.hxx>
#include <uno/data.h>
#include <typelib/typedescription.hxx>

#include <bridge.hxx>
#include <cppinterfaceproxy.hxx>
#include <types.hxx>
#include <vtablefactory.hxx>

#include "call.hxx"
#include "mscx.hxx"

using namespace ::com::sun::star::uno;

static typelib_TypeClass cpp2uno_call(
    bridges::cpp_uno::shared::CppInterfaceProxy * pThis,
    const typelib_TypeDescription * pMemberTD,
    typelib_TypeDescriptionReference * pReturnTypeRef, // NULL indicates void return
    sal_Int32 nParams,
    typelib_MethodParameter * pParams,
    void ** pStack )
{
    // Return type
    typelib_TypeDescription * pReturnTD = nullptr;
    if ( pReturnTypeRef )
        TYPELIB_DANGER_GET( &pReturnTD, pReturnTypeRef );

    int nFirstRealParam = 3;    // Index into pStack, past return
                                // value, return address and 'this'
                                // pointer.

    void * pUnoReturn = nullptr;
    void * pCppReturn = nullptr; // Complex return ptr: if != NULL && != pUnoReturn, reconversion need

    if ( pReturnTD )
    {
        if ( bridges::cpp_uno::shared::isSimpleType( pReturnTD ) )
        {
            pUnoReturn = pStack;
        }
        else
        {
            pCppReturn = pStack[nFirstRealParam++];

            pUnoReturn = ( bridges::cpp_uno::shared::relatesToInterfaceType( pReturnTD )
                           ? alloca( pReturnTD->nSize )
                           : pCppReturn ); // direct way
        }
    }

    void ** pCppIncomingParams = pStack + nFirstRealParam;

    // Unlike this method for other archs, prefer clarity to
    // micro-optimization, and allocate these array separately

    // Parameters passed to the UNO function
    void ** pUnoArgs = static_cast<void **>(alloca( sizeof(void *) * nParams ));

    // Parameters received from C++
    void ** pCppArgs = static_cast<void **>(alloca( sizeof(void *) * nParams ));

    // Indexes of values this have to be converted (interface conversion C++<=>UNO)
    int * pTempIndexes =
        static_cast<int *>(alloca( sizeof(int) * nParams ));

    // Type descriptions for reconversions
    typelib_TypeDescription ** ppTempParamTD =
        static_cast<typelib_TypeDescription **>(alloca( sizeof(void *) * nParams ));

    int nTempIndexes = 0;

    for ( int nPos = 0; nPos < nParams; ++nPos )
    {
        const typelib_MethodParameter & rParam = pParams[nPos];

        typelib_TypeDescription * pParamTD = nullptr;
        TYPELIB_DANGER_GET( &pParamTD, rParam.pTypeRef );

        if ( !rParam.bOut &&
             bridges::cpp_uno::shared::isSimpleType( pParamTD ) )
        {
            pCppArgs[nPos] = pUnoArgs[nPos] = pCppIncomingParams++;

            TYPELIB_DANGER_RELEASE( pParamTD );
        }
        else // ptr to complex value | ref
        {
            void * pCppStack;

            pCppArgs[nPos] = pCppStack = *pCppIncomingParams++;

            if ( !rParam.bIn ) // Pure out
            {
                // UNO out is unconstructed mem
                pUnoArgs[nPos] = alloca( pParamTD->nSize );
                pTempIndexes[nTempIndexes] = nPos;
                // pParamTD will be released at reconversion
                ppTempParamTD[nTempIndexes++] = pParamTD;
            }
            else if ( bridges::cpp_uno::shared::relatesToInterfaceType( pParamTD ) )
            {
                ::uno_copyAndConvertData(
                    pUnoArgs[nPos] = alloca( pParamTD->nSize ),
                    pCppStack, pParamTD,
                    pThis->getBridge()->getCpp2Uno() );
                pTempIndexes[nTempIndexes] = nPos; // Has to be reconverted
                // pParamTD will be released at reconversion
                ppTempParamTD[nTempIndexes++] = pParamTD;
            }
            else // direct way
            {
                pUnoArgs[nPos] = pCppStack;
                // No longer needed
                TYPELIB_DANGER_RELEASE( pParamTD );
            }
        }
    }

    // ExceptionHolder
    uno_Any aUnoExc; // Any will be constructed by callee
    uno_Any * pUnoExc = &aUnoExc;

    // invoke UNO dispatch call
    (*pThis->getUnoI()->pDispatcher)(
        pThis->getUnoI(), pMemberTD, pUnoReturn, pUnoArgs, &pUnoExc );

    // in case an exception occurred...
    if ( pUnoExc )
    {
        // Destruct temporary in/inout params
        while ( nTempIndexes-- )
        {
            int nIndex = pTempIndexes[nTempIndexes];

            if ( pParams[nIndex].bIn ) // Is in/inout => was constructed
            {
                ::uno_destructData( pUnoArgs[nIndex], ppTempParamTD[nTempIndexes], nullptr );
            }
            TYPELIB_DANGER_RELEASE( ppTempParamTD[nTempIndexes] );
        }
        if ( pReturnTD )
            TYPELIB_DANGER_RELEASE( pReturnTD );

        CPPU_CURRENT_NAMESPACE::mscx_raiseException(
            &aUnoExc, pThis->getBridge()->getUno2Cpp() ); // Has to destruct the any

        // Is here for dummy
        return typelib_TypeClass_VOID;
    }
    else // Else, no exception occurred...
    {
        // Temporary params
        while (nTempIndexes--)
        {
            int nIndex = pTempIndexes[nTempIndexes];
            typelib_TypeDescription * pParamTD = ppTempParamTD[nTempIndexes];

            if ( pParams[nIndex].bOut ) // inout/out
            {
                // Convert and assign
                ::uno_destructData(
                    pCppArgs[nIndex], pParamTD, cpp_release );
                ::uno_copyAndConvertData(
                    pCppArgs[nIndex], pUnoArgs[nIndex], pParamTD,
                    pThis->getBridge()->getUno2Cpp() );
            }
            // Destroy temp UNO param
            ::uno_destructData( pUnoArgs[nIndex], pParamTD, nullptr );

            TYPELIB_DANGER_RELEASE( pParamTD );
        }
        // Return
        if ( pCppReturn ) // Has complex return
        {
            if ( pUnoReturn != pCppReturn ) // Needs reconversion
            {
                ::uno_copyAndConvertData(
                    pCppReturn, pUnoReturn, pReturnTD,
                    pThis->getBridge()->getUno2Cpp() );
                // Destroy temp UNO return
                ::uno_destructData( pUnoReturn, pReturnTD, nullptr );
            }
            // Complex return ptr is set to eax
            pStack[0] = pCppReturn;
        }
        if ( pReturnTD )
        {
            typelib_TypeClass eRet = pReturnTD->eTypeClass;
            TYPELIB_DANGER_RELEASE( pReturnTD );
            return eRet;
        }
        else
            return typelib_TypeClass_VOID;
    }
}

extern "C" typelib_TypeClass cpp_vtable_call(
    sal_Int64 nOffsetAndIndex,
    void ** pStack )
{
    sal_Int32 nFunctionIndex = (nOffsetAndIndex & 0xFFFFFFFF);
    sal_Int32 nVtableOffset = ((nOffsetAndIndex >> 32) & 0xFFFFFFFF);

    // pStack points to space for return value allocated by
    // privateSnippetExecutor() in call.asm, after which follows our
    // return address (uninteresting), then the integer or
    // floating-point register parameters (spilled by
    // privateSnippetExecutor()) from the call to the trampoline,
    // followed by stacked parameters. The first parameter is the
    // 'this' pointer. If the callee returns a large value, the
    // parameter after that is actually a pointer to where the callee
    // should store its return value.

    void * pThis = static_cast<char *>( pStack[2] ) - nVtableOffset;

    bridges::cpp_uno::shared::CppInterfaceProxy * pCppI =
        bridges::cpp_uno::shared::CppInterfaceProxy::castInterfaceToProxy( pThis );

    typelib_InterfaceTypeDescription * pTD = pCppI->getTypeDescr();

    SAL_INFO( "bridges", "cpp_vtable_call: pCallStack=[" <<
            std::hex << pStack[0] << "," << pStack[1] << "," << pStack[2] << ",...], pThis=" <<
            pThis << ", pCppI=" << pCppI <<
            std::dec << ", nFunctionIndex=" << nFunctionIndex << ", nVtableOffset=" << nVtableOffset );
    SAL_INFO( "bridges", "name=" << OUString::unacquired(&pTD->aBase.pTypeName) );

    if ( nFunctionIndex >= pTD->nMapFunctionIndexToMemberIndex )
    {
        SAL_WARN(
            "bridges",
            "illegal " << OUString::unacquired(&pTD->aBase.pTypeName)
                << " vtable index " << nFunctionIndex << "/"
                << pTD->nMapFunctionIndexToMemberIndex);
        throw RuntimeException(
            ("illegal " + OUString::unacquired(&pTD->aBase.pTypeName)
             + " vtable index " + OUString::number(nFunctionIndex) + "/"
             + OUString::number(pTD->nMapFunctionIndexToMemberIndex)),
            reinterpret_cast<XInterface *>( pCppI ) );
    }

    // Determine called method
    int nMemberPos = pTD->pMapFunctionIndexToMemberIndex[nFunctionIndex];
    assert(nMemberPos < pTD->nAllMembers);

    TypeDescription aMemberDescr( pTD->ppAllMembers[nMemberPos] );

    SAL_INFO( "bridges", "Calling " << OUString::unacquired(&aMemberDescr.get()->pTypeName) );

    typelib_TypeClass eRet;
    switch ( aMemberDescr.get()->eTypeClass )
    {
        case typelib_TypeClass_INTERFACE_ATTRIBUTE:
        {
            typelib_TypeDescriptionReference *pAttrTypeRef =
                reinterpret_cast<typelib_InterfaceAttributeTypeDescription *>( aMemberDescr.get() )->pAttributeTypeRef;

            if ( pTD->pMapMemberIndexToFunctionIndex[nMemberPos] == nFunctionIndex )
            {
                // is GET method
                eRet = cpp2uno_call( pCppI, aMemberDescr.get(), pAttrTypeRef,
                        0, nullptr, // No params
                        pStack );
            }
            else
            {
                // is SET method
                typelib_MethodParameter aParam;
                aParam.pTypeRef = pAttrTypeRef;
                aParam.bIn      = true;
                aParam.bOut     = false;

                eRet = cpp2uno_call( pCppI, aMemberDescr.get(),
                        nullptr, // Indicates void return
                        1, &aParam,
                        pStack );
            }
            break;
        }
        case typelib_TypeClass_INTERFACE_METHOD:
        {
            // is METHOD
            switch ( nFunctionIndex )
            {
                case 1: // acquire()
                    pCppI->acquireProxy(); // Non virtual call!
                    eRet = typelib_TypeClass_VOID;
                    break;
                case 2: // release()
                    pCppI->releaseProxy(); // non virtual call!
                    eRet = typelib_TypeClass_VOID;
                    break;
                case 0: // queryInterface() opt
                {
                    typelib_TypeDescription * pTD2 = nullptr;

                    // the incoming C++ parameters are: The this
                    // pointer, the hidden return value pointer, and
                    // then the actual queryInterface() only
                    // parameter. Thus pStack[4]..

                    TYPELIB_DANGER_GET( &pTD2, static_cast<Type *>( pStack[4] )->getTypeLibType() );

                    if ( pTD2 )
                    {
                        XInterface * pInterface = nullptr;
                        (*pCppI->getBridge()->getCppEnv()->getRegisteredInterface)
                            ( pCppI->getBridge()->getCppEnv(),
                              reinterpret_cast<void **>(&pInterface),
                              pCppI->getOid().pData,
                              reinterpret_cast<typelib_InterfaceTypeDescription *>( pTD2 ) );

                        if ( pInterface )
                        {
                            // pStack[3] = hidden return value pointer
                            ::uno_any_construct( static_cast<uno_Any *>( pStack[3] ),
                                                 &pInterface, pTD2, cpp_acquire );

                            pInterface->release();
                            TYPELIB_DANGER_RELEASE( pTD2 );

                            pStack[0] = pStack[3];
                            eRet = typelib_TypeClass_ANY;
                            break;
                        }
                        TYPELIB_DANGER_RELEASE( pTD2 );
                    }
                    [[fallthrough]];
                }
                default:
                {
                    typelib_InterfaceMethodTypeDescription * pMethodTD =
                        reinterpret_cast<typelib_InterfaceMethodTypeDescription *>( aMemberDescr.get() );

                    eRet = cpp2uno_call( pCppI, aMemberDescr.get(),
                                         pMethodTD->pReturnTypeRef,
                                         pMethodTD->nParams,
                                         pMethodTD->pParams,
                                         pStack );
                }
            }
            break;
        }
        default:
        {
            throw RuntimeException("No member description found!",
                                    reinterpret_cast<XInterface *>( pCppI ) );
        }
    }

    return eRet;
}

int const codeSnippetSize = 48;

extern "C" char privateSnippetExecutor;

// This function generates the code that acts as a proxy for the UNO function to be called.
// The generated code does the following:
// - Spills register parameters on stack
// - Loads functionIndex and vtableOffset into scratch registers
// - Jumps to privateSnippetExecutor

static unsigned char * codeSnippet(
    unsigned char * code,
    CPPU_CURRENT_NAMESPACE::RegParamKind param_kind[4],
    sal_Int32 nFunctionIndex,
    sal_Int32 nVtableOffset )
{
    sal_uInt64 nOffsetAndIndex = ( static_cast<sal_uInt64>(nVtableOffset) << 32 ) | static_cast<sal_uInt64>(nFunctionIndex);
    unsigned char *p = code;

    // Spill parameters
    if ( param_kind[0] == CPPU_CURRENT_NAMESPACE::REGPARAM_INT )
    {
        // mov qword ptr 8[rsp], rcx
        *p++ = 0x48; *p++ = 0x89; *p++ = 0x4C; *p++ = 0x24; *p++ = 0x08;
    }
    else
    {
        // movsd qword ptr 8[rsp], xmm0
        *p++ = 0xF2; *p++ = 0x0F; *p++ = 0x11; *p++ = 0x44; *p++ = 0x24; *p++ = 0x08;
    }
    if ( param_kind[1] == CPPU_CURRENT_NAMESPACE::REGPARAM_INT )
    {
        // mov qword ptr 16[rsp], rdx
        *p++ = 0x48; *p++ = 0x89; *p++ = 0x54; *p++ = 0x24; *p++ = 0x10;
    }
    else
    {
        // movsd qword ptr 16[rsp], xmm1
        *p++ = 0xF2; *p++ = 0x0F; *p++ = 0x11; *p++ = 0x4C; *p++ = 0x24; *p++ = 0x10;
    }
    if ( param_kind[2] == CPPU_CURRENT_NAMESPACE::REGPARAM_INT )
    {
        // mov qword ptr 24[rsp], r8
        *p++ = 0x4C; *p++ = 0x89; *p++ = 0x44; *p++ = 0x24; *p++ = 0x18;
    }
    else
    {
        // movsd qword ptr 24[rsp], xmm2
        *p++ = 0xF2; *p++ = 0x0F; *p++ = 0x11; *p++ = 0x54; *p++ = 0x24; *p++ = 0x18;
    }
    if ( param_kind[3] == CPPU_CURRENT_NAMESPACE::REGPARAM_INT )
    {
        // mov qword ptr 32[rsp], r9
        *p++ = 0x4C;*p++ = 0x89; *p++ = 0x4C; *p++ = 0x24; *p++ = 0x20;
    }
    else
    {
        // movsd qword ptr 32[rsp], xmm3
        *p++ = 0xF2; *p++ = 0x0F; *p++ = 0x11; *p++ = 0x5C; *p++ = 0x24; *p++ = 0x20;
    }

    // mov rcx, nOffsetAndIndex
    *p++ = 0x48; *p++ = 0xB9;
    *reinterpret_cast<sal_uInt64 *>(p) = nOffsetAndIndex; p += 8;

    // mov r11, privateSnippetExecutor
    *p++ = 0x49; *p++ = 0xBB;
    *reinterpret_cast<void **>(p) = &privateSnippetExecutor; p += 8;

    // jmp r11
    *p++ = 0x41; *p++ = 0xFF; *p++ = 0xE3;

    assert(p < code + codeSnippetSize);

    return code + codeSnippetSize;
}

struct bridges::cpp_uno::shared::VtableFactory::Slot { void * fn; };

bridges::cpp_uno::shared::VtableFactory::Slot *
bridges::cpp_uno::shared::VtableFactory::mapBlockToVtable(
    void * block )
{
    return static_cast< Slot * >(block) + 1;
}

std::size_t bridges::cpp_uno::shared::VtableFactory::getBlockSize(
    sal_Int32 slotCount )
{
    return (slotCount + 1) * sizeof (Slot) + slotCount * codeSnippetSize;
}

bridges::cpp_uno::shared::VtableFactory::Slot *
bridges::cpp_uno::shared::VtableFactory::initializeBlock(
    void * block,
    sal_Int32 slotCount,
    sal_Int32, typelib_InterfaceTypeDescription * )
{
    struct Rtti {
        sal_Int32 n0, n1, n2;
        type_info * rtti;
        Rtti():
            n0(0), n1(0), n2(0),
            rtti(CPPU_CURRENT_NAMESPACE::mscx_getRTTI(
                     "com.sun.star.uno.XInterface"))
        {}
    };
    static Rtti rtti;

    Slot * slots = mapBlockToVtable(block);
    slots[-1].fn = &rtti;
    return slots + slotCount;
}

unsigned char * bridges::cpp_uno::shared::VtableFactory::addLocalFunctions(
    Slot ** slots,
    unsigned char * code,
    typelib_InterfaceTypeDescription const * type,
    sal_Int32 nFunctionOffset,
    sal_Int32 functionCount,
    sal_Int32 nVtableOffset )
{
    (*slots) -= functionCount;
    Slot * s = *slots;

    for (int member = 0; member < type->nMembers; ++member) {
        typelib_TypeDescription * pTD = nullptr;

        TYPELIB_DANGER_GET( &pTD, type->ppMembers[ member ] );
        assert(pTD);

        CPPU_CURRENT_NAMESPACE::RegParamKind param_kind[4];
        int nr = 0;

        for (int i = 0; i < 4; ++i)
            param_kind[i] = CPPU_CURRENT_NAMESPACE::REGPARAM_INT;

        // 'this'
        ++nr;

        if ( pTD->eTypeClass == typelib_TypeClass_INTERFACE_ATTRIBUTE )
        {
            typelib_InterfaceAttributeTypeDescription * pIfaceAttrTD =
                reinterpret_cast<typelib_InterfaceAttributeTypeDescription *>( pTD );

            // Getter

            (s++)->fn = code;
            code = codeSnippet( code, param_kind, nFunctionOffset++, nVtableOffset );
            if ( ! pIfaceAttrTD->bReadOnly )
            {
                typelib_TypeDescription * pAttrTD = nullptr;
                TYPELIB_DANGER_GET( &pAttrTD, pIfaceAttrTD->pAttributeTypeRef );
                assert(pAttrTD);

                // Setter
                if ( pAttrTD->eTypeClass == typelib_TypeClass_FLOAT ||
                     pAttrTD->eTypeClass == typelib_TypeClass_DOUBLE )
                    param_kind[nr++] = CPPU_CURRENT_NAMESPACE::REGPARAM_FLT;

                TYPELIB_DANGER_RELEASE( pAttrTD );

                (s++)->fn = code;
                code = codeSnippet( code, param_kind, nFunctionOffset++, nVtableOffset );
            }
        }
        else if ( pTD->eTypeClass == typelib_TypeClass_INTERFACE_METHOD )
        {
            typelib_InterfaceMethodTypeDescription * pMethodTD =
                reinterpret_cast<typelib_InterfaceMethodTypeDescription *>( pTD );

            typelib_TypeDescription * pReturnTD = nullptr;
            TYPELIB_DANGER_GET( &pReturnTD, pMethodTD->pReturnTypeRef );
            assert(pReturnTD);

            if ( !bridges::cpp_uno::shared::isSimpleType( pReturnTD ) )
            {
                // Return value
                ++nr;
            }

            for (int param = 0; nr < 4 && param < pMethodTD->nParams; ++param, ++nr)
            {
                typelib_TypeDescription * pParamTD = nullptr;

                TYPELIB_DANGER_GET( &pParamTD, pMethodTD->pParams[param].pTypeRef );
                assert(pParamTD);

                if ( pParamTD->eTypeClass == typelib_TypeClass_FLOAT ||
                     pParamTD->eTypeClass == typelib_TypeClass_DOUBLE )
                    param_kind[nr] = CPPU_CURRENT_NAMESPACE::REGPARAM_FLT;

                TYPELIB_DANGER_RELEASE( pParamTD );
            }
            (s++)->fn = code;
            code = codeSnippet( code, param_kind, nFunctionOffset++, nVtableOffset );

            TYPELIB_DANGER_RELEASE( pReturnTD );
        }
        else
            assert(false);

        TYPELIB_DANGER_RELEASE( pTD );
    }
    return code;
}

void bridges::cpp_uno::shared::VtableFactory::flushCode(
    unsigned char const *,
    unsigned char const * )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
