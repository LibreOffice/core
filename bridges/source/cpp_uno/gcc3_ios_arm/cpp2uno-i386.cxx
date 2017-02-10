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

#ifdef __i386

// For the iOS emulator (i386). Basically a copy of
// ../gcc3_macosx_intel/cpp2uno.cxx with some cleanups and necessary
// changes: To match what we do on iOS devices, we don't do any
// dynamic code generation on the emulator either (even if it as such
// wouldn't be prohibited).

// No attempts at factoring out the large amounts of more or less
// common code in this and cpp2uno-arm.cxx have been done. Which is
// sad. But then the whole bridges/source/cpp_uno is full of
// copy/paste. So I continue in that tradition...

#include <com/sun/star/uno/RuntimeException.hpp>
#include <sal/log.hxx>
#include <uno/data.h>
#include <typelib/typedescription.hxx>

#include "bridge.hxx"
#include "cppinterfaceproxy.hxx"
#include "types.hxx"
#include "vtablefactory.hxx"

#include "share.hxx"

extern "C" {
    extern int nFunIndexes, nVtableOffsets;
    extern int codeSnippets[];
}

using namespace ::com::sun::star::uno;

namespace
{

void cpp2uno_call(
    bridges::cpp_uno::shared::CppInterfaceProxy * pThis,
    const typelib_TypeDescription * pMemberTypeDescr,
    typelib_TypeDescriptionReference * pReturnTypeRef, // 0 indicates void return
    sal_Int32 nParams, typelib_MethodParameter * pParams,
    void ** pCallStack,
    void * pReturnValue )
{
    // pCallStack: ret, [return ptr], this, params
    char * pCppStack = (char *)(pCallStack +1);

    // return
    typelib_TypeDescription * pReturnTypeDescr = 0;
    if (pReturnTypeRef)
        TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );

    void * pUnoReturn = 0;
    void * pCppReturn = 0; // complex return ptr: if != 0 && != pUnoReturn, reconversion need

    if (pReturnTypeDescr)
    {
        // xxx todo: test PolyStructy<STRUCT<long>> foo()
        if (CPPU_CURRENT_NAMESPACE::isSimpleReturnType( pReturnTypeDescr ))
        {
            pUnoReturn = pReturnValue; // direct way for simple types
        }
        else // complex return via ptr (pCppReturn)
        {
            pCppReturn = *(void **)pCppStack;
            pCppStack += sizeof(void *);
            pUnoReturn = (bridges::cpp_uno::shared::relatesToInterfaceType(
                              pReturnTypeDescr )
                          ? alloca( pReturnTypeDescr->nSize )
                          : pCppReturn); // direct way
        }
    }
    // pop this
    pCppStack += sizeof( void* );

    // stack space
    static_assert(sizeof(void *) == sizeof(sal_Int32), "### unexpected size!");
    // parameters
    void ** pUnoArgs = (void **)alloca( 4 * sizeof(void *) * nParams );
    void ** pCppArgs = pUnoArgs + nParams;
    // indices of values this have to be converted (interface conversion cpp<=>uno)
    sal_Int32 * pTempIndices = (sal_Int32 *)(pUnoArgs + (2 * nParams));
    // type descriptions for reconversions
    typelib_TypeDescription ** ppTempParamTypeDescr = (typelib_TypeDescription **)(pUnoArgs + (3 * nParams));

    sal_Int32 nTempIndices   = 0;

    for ( sal_Int32 nPos = 0; nPos < nParams; ++nPos )
    {
        const typelib_MethodParameter & rParam = pParams[nPos];
        typelib_TypeDescription * pParamTypeDescr = 0;
        TYPELIB_DANGER_GET( &pParamTypeDescr, rParam.pTypeRef );

        if (!rParam.bOut
            && bridges::cpp_uno::shared::isSimpleType( pParamTypeDescr ))
            // value
        {
            pCppArgs[nPos] = pCppStack;
            pUnoArgs[nPos] = pCppStack;
            switch (pParamTypeDescr->eTypeClass)
            {
            case typelib_TypeClass_HYPER:
            case typelib_TypeClass_UNSIGNED_HYPER:
            case typelib_TypeClass_DOUBLE:
                pCppStack += sizeof(sal_Int32); // extra long
                SAL_FALLTHROUGH;
            default:
                break;
            }
            // no longer needed
            TYPELIB_DANGER_RELEASE( pParamTypeDescr );
        }
        else // ptr to complex value | ref
        {
            pCppArgs[nPos] = *(void **)pCppStack;

            if (! rParam.bIn) // is pure out
            {
                // uno out is unconstructed mem!
                pUnoArgs[nPos] = alloca( pParamTypeDescr->nSize );
                pTempIndices[nTempIndices] = nPos;
                // will be released at reconversion
                ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
            }
            // is in/inout
            else if (bridges::cpp_uno::shared::relatesToInterfaceType(
                         pParamTypeDescr ))
            {
                uno_copyAndConvertData( pUnoArgs[nPos] = alloca( pParamTypeDescr->nSize ),
                                        *(void **)pCppStack, pParamTypeDescr,
                                        pThis->getBridge()->getCpp2Uno() );
                pTempIndices[nTempIndices] = nPos; // has to be reconverted
                // will be released at reconversion
                ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
            }
            else // direct way
            {
                pUnoArgs[nPos] = *(void **)pCppStack;
                // no longer needed
                TYPELIB_DANGER_RELEASE( pParamTypeDescr );
            }
        }
        pCppStack += sizeof(sal_Int32); // standard parameter length
    }

    // ExceptionHolder
    uno_Any aUnoExc; // Any will be constructed by callee
    uno_Any * pUnoExc = &aUnoExc;

    // invoke uno dispatch call
    (*pThis->getUnoI()->pDispatcher)(
        pThis->getUnoI(), pMemberTypeDescr, pUnoReturn, pUnoArgs, &pUnoExc );

    // in case an exception occurred...
    if (pUnoExc)
    {
        // destruct temporary in/inout params
        for ( ; nTempIndices--; )
        {
            sal_Int32 nIndex = pTempIndices[nTempIndices];

            if (pParams[nIndex].bIn) // is in/inout => was constructed
                uno_destructData( pUnoArgs[nIndex], ppTempParamTypeDescr[nTempIndices], 0 );
            TYPELIB_DANGER_RELEASE( ppTempParamTypeDescr[nTempIndices] );
        }
        if (pReturnTypeDescr)
            TYPELIB_DANGER_RELEASE( pReturnTypeDescr );

        CPPU_CURRENT_NAMESPACE::raiseException(
            &aUnoExc, pThis->getBridge()->getUno2Cpp() );
            // has to destruct the any
    }
    else // else no exception occurred...
    {
        // temporary params
        for ( ; nTempIndices--; )
        {
            sal_Int32 nIndex = pTempIndices[nTempIndices];
            typelib_TypeDescription * pParamTypeDescr = ppTempParamTypeDescr[nTempIndices];

            if (pParams[nIndex].bOut) // inout/out
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
        if (pCppReturn) // has complex return
        {
            if (pUnoReturn != pCppReturn) // needs reconversion
            {
                uno_copyAndConvertData( pCppReturn, pUnoReturn, pReturnTypeDescr,
                                        pThis->getBridge()->getUno2Cpp() );
                // destroy temp uno return
                uno_destructData( pUnoReturn, pReturnTypeDescr, 0 );
            }
            if (pReturnValue != pCppReturn) {
                // complex return ptr is set to eax if return value
                // is not transferred via eax[/edx]:
                *static_cast< void ** >(pReturnValue) = pCppReturn;
            }
        }
        if (pReturnTypeDescr)
        {
            TYPELIB_DANGER_RELEASE( pReturnTypeDescr );
        }
    }
}


extern "C" void cpp_vtable_call(
    int nFunctionIndex, int nVtableOffset, void** pCallStack,
    void * pReturnValue )
{
    static_assert(sizeof(sal_Int32)==sizeof(void *), "### unexpected!");

    // pCallStack: ret adr, [ret *], this, params
    void * pThis;
    if( nFunctionIndex & 0x80000000 )
    {
        nFunctionIndex &= 0x7fffffff;
        pThis = pCallStack[2];
    }
    else
    {
        pThis = pCallStack[1];
    }
    pThis = static_cast< char * >(pThis) - nVtableOffset;
    bridges::cpp_uno::shared::CppInterfaceProxy * pCppI
        = bridges::cpp_uno::shared::CppInterfaceProxy::castInterfaceToProxy(
            pThis);

    typelib_InterfaceTypeDescription * pTypeDescr = pCppI->getTypeDescr();

    SAL_INFO( "bridges.ios", "cpp_vtable_call: pCallStack=[" <<
              std::hex << pCallStack[0] << "," << pCallStack[1] << "," << pCallStack[2] << ",...]" <<
              ", pThis=" << pThis << ", pCppI=" << pCppI <<
              std::dec << ", nFunctionIndex=" << nFunctionIndex << ", nVtableOffset=" << nVtableOffset );

    SAL_INFO( "bridges.ios", "name=" << OUString::unacquired(&pTypeDescr->aBase.pTypeName) );

    assert( nFunctionIndex < pTypeDescr->nMapFunctionIndexToMemberIndex );

    if (nFunctionIndex >= pTypeDescr->nMapFunctionIndexToMemberIndex)
    {
        throw RuntimeException( "illegal vtable index!", (XInterface *)pThis );
    }

    // determine called method
    sal_Int32 nMemberPos = pTypeDescr->pMapFunctionIndexToMemberIndex[nFunctionIndex];

    assert( nMemberPos < pTypeDescr->nAllMembers );

    TypeDescription aMemberDescr( pTypeDescr->ppAllMembers[nMemberPos] );

    SAL_INFO( "bridges.ios", "Calling " << OUString::unacquired(&aMemberDescr.get()->pTypeName) );

    switch (aMemberDescr.get()->eTypeClass)
    {
    case typelib_TypeClass_INTERFACE_ATTRIBUTE:
    {
        if (pTypeDescr->pMapMemberIndexToFunctionIndex[nMemberPos] == nFunctionIndex)
        {
            // is GET method
            cpp2uno_call(
                pCppI, aMemberDescr.get(),
                ((typelib_InterfaceAttributeTypeDescription *)aMemberDescr.get())->pAttributeTypeRef,
                0, 0, // no params
                pCallStack, pReturnValue );
        }
        else
        {
            // is SET method
            typelib_MethodParameter aParam;
            aParam.pTypeRef =
                ((typelib_InterfaceAttributeTypeDescription *)aMemberDescr.get())->pAttributeTypeRef;
            aParam.bIn      = sal_True;
            aParam.bOut     = sal_False;

            cpp2uno_call(
                pCppI, aMemberDescr.get(),
                0, // indicates void return
                1, &aParam,
                pCallStack, pReturnValue );
        }
        break;
    }
    case typelib_TypeClass_INTERFACE_METHOD:
    {
        // is METHOD
        switch (nFunctionIndex)
        {
        case 1: // acquire()
            pCppI->acquireProxy(); // non virtual call!
            break;
        case 2: // release()
            pCppI->releaseProxy(); // non virtual call!
            break;
        case 0: // queryInterface() opt
        {
            typelib_TypeDescription * pTD = 0;
            TYPELIB_DANGER_GET( &pTD, reinterpret_cast< Type * >( pCallStack[3] )->getTypeLibType() );
            if (pTD)
            {
                XInterface * pInterface = 0;
                (*pCppI->getBridge()->getCppEnv()->getRegisteredInterface)(
                    pCppI->getBridge()->getCppEnv(),
                    (void **)&pInterface, pCppI->getOid().pData,
                    (typelib_InterfaceTypeDescription *)pTD );

                if (pInterface)
                {
                    ::uno_any_construct(
                        reinterpret_cast< uno_Any * >( pCallStack[1] ),
                        &pInterface, pTD, cpp_acquire );
                    pInterface->release();
                    TYPELIB_DANGER_RELEASE( pTD );
                    *static_cast< void ** >(pReturnValue) = pCallStack[1];
                    break;
                }
                TYPELIB_DANGER_RELEASE( pTD );
            }
        } // else perform queryInterface()
        SAL_FALLTHROUGH;
        default:
            cpp2uno_call(
                pCppI, aMemberDescr.get(),
                ((typelib_InterfaceMethodTypeDescription *)aMemberDescr.get())->pReturnTypeRef,
                ((typelib_InterfaceMethodTypeDescription *)aMemberDescr.get())->nParams,
                ((typelib_InterfaceMethodTypeDescription *)aMemberDescr.get())->pParams,
                pCallStack, pReturnValue );
        }
        break;
    }
    default:
    {
        throw RuntimeException( "no member description found!", (XInterface *)pThis );
    }
    }
}

typedef enum { privateSnippetExecutorGeneral, privateSnippetExecutorVoid, privateSnippetExecutorHyper, privateSnippetExecutorFloat, privateSnippetExecutorDouble, privateSnippetExecutorClass } snippetExecutorKind;

static const char * snippetExecutorClassName(
    snippetExecutorKind exec)
{
    switch (exec) {
    case privateSnippetExecutorGeneral: return "General";
    case privateSnippetExecutorVoid: return "Void";
    case privateSnippetExecutorHyper: return "Hyper";
    case privateSnippetExecutorFloat: return "Float";
    case privateSnippetExecutorDouble: return "Double";
    case privateSnippetExecutorClass: return "Class";
    default:
        abort();
    }
}

unsigned char * codeSnippet(
    sal_Int32 functionIndex, sal_Int32 vtableOffset,
    typelib_TypeDescriptionReference * pReturnTypeRef)
{
    assert(functionIndex < nFunIndexes);
    if (!(functionIndex < nFunIndexes))
        return NULL;

    assert(vtableOffset < nVtableOffsets);
    if (!(vtableOffset < nVtableOffsets))
        return NULL;

    snippetExecutorKind exec;
    bool bHasHiddenParam = false;
    if (pReturnTypeRef == 0) {
        exec = privateSnippetExecutorVoid;
    }
    else {
        switch (pReturnTypeRef->eTypeClass) {
        case typelib_TypeClass_VOID:
            exec = privateSnippetExecutorVoid;
            break;
        case typelib_TypeClass_HYPER:
        case typelib_TypeClass_UNSIGNED_HYPER:
            exec = privateSnippetExecutorHyper;
            break;
        case typelib_TypeClass_FLOAT:
            exec = privateSnippetExecutorFloat;
            break;
        case typelib_TypeClass_DOUBLE:
            exec = privateSnippetExecutorDouble;
            break;
        case typelib_TypeClass_STRUCT:
        case typelib_TypeClass_EXCEPTION: {
            typelib_TypeDescription * pReturnTypeDescr = 0;
            TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );
            bool const bSimpleReturnStruct =
                CPPU_CURRENT_NAMESPACE::isSimpleReturnType(pReturnTypeDescr);
            sal_Int32 const nRetSize = pReturnTypeDescr->nSize;
            TYPELIB_DANGER_RELEASE( pReturnTypeDescr );
            if (bSimpleReturnStruct && nRetSize <= 8) {
                exec = privateSnippetExecutorGeneral; // fills eax
                if (nRetSize > 4)
                    exec = privateSnippetExecutorHyper; // fills eax/edx
                break;
            }
        }
        SAL_FALLTHROUGH;
        case typelib_TypeClass_STRING:
        case typelib_TypeClass_TYPE:
        case typelib_TypeClass_SEQUENCE:
        case typelib_TypeClass_INTERFACE:
        case typelib_TypeClass_ANY:
            bHasHiddenParam = 1;
            exec = privateSnippetExecutorClass;
            break;
        default:
            exec = privateSnippetExecutorGeneral;
            break;
        }
    }

    // The codeSnippets table is indexed by functionIndex,
    // vtableOffset, exec and the has-hidden-param flag

    int index = functionIndex*nVtableOffsets*6*2 + vtableOffset*6*2 + exec*2 + bHasHiddenParam;
    unsigned char *result = ((unsigned char *) &codeSnippets) + codeSnippets[index];

    SAL_INFO( "bridges.ios", "codeSnippet: [" <<
              functionIndex << "," << vtableOffset << "," << snippetExecutorClassName(exec) << "," << bHasHiddenParam << "]=" <<
              (void *) result);

    return result;
}

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
    return (slotCount + 2) * sizeof (Slot);
}

bridges::cpp_uno::shared::VtableFactory::Slot *
bridges::cpp_uno::shared::VtableFactory::initializeBlock(
    void * block, sal_Int32 slotCount, sal_Int32,
    typelib_InterfaceTypeDescription *)
{
    Slot * slots = mapBlockToVtable(block);
    slots[-2].fn = 0;
    slots[-1].fn = 0;
    return slots + slotCount;
}

unsigned char * bridges::cpp_uno::shared::VtableFactory::addLocalFunctions(
    Slot ** slots, unsigned char * code,
    typelib_InterfaceTypeDescription const * type, sal_Int32 functionOffset,
    sal_Int32 functionCount, sal_Int32 vtableOffset)
{
    (*slots) -= functionCount;
    Slot * s = *slots;
    for (sal_Int32 i = 0; i < type->nMembers; ++i) {
        typelib_TypeDescription * member = 0;
        TYPELIB_DANGER_GET(&member, type->ppMembers[i]);
        assert(member != 0);
        switch (member->eTypeClass) {
        case typelib_TypeClass_INTERFACE_ATTRIBUTE:
            // Getter:
            (s++)->fn = codeSnippet(
                functionOffset++, vtableOffset,
                reinterpret_cast< typelib_InterfaceAttributeTypeDescription * >(
                    member)->pAttributeTypeRef);
            // Setter:
            if (!reinterpret_cast<
                typelib_InterfaceAttributeTypeDescription * >(
                    member)->bReadOnly)
            {
                (s++)->fn = codeSnippet(
                    functionOffset++, vtableOffset,
                    0 /* indicates VOID */);
            }
            break;

        case typelib_TypeClass_INTERFACE_METHOD:
            (s++)->fn = codeSnippet(
                functionOffset++, vtableOffset,
                reinterpret_cast< typelib_InterfaceMethodTypeDescription * >(
                    member)->pReturnTypeRef);
            break;

        default:
            assert(false);
            break;
        }
        TYPELIB_DANGER_RELEASE(member);
    }
    return code;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
