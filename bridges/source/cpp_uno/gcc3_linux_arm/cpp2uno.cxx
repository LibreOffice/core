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
#include <sal/log.hxx>

#include <com/sun/star/uno/genfunc.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <uno/data.h>
#include <typelib/typedescription.hxx>

#include <bridge.hxx>
#include <cppinterfaceproxy.hxx>
#include <types.hxx>
#include <vtablefactory.hxx>

#include "share.hxx"
#include "call.hxx"

#include <dlfcn.h>

#ifdef ANDROID
#include <unistd.h>
#endif

using namespace ::osl;
using namespace ::com::sun::star::uno;

namespace
{

    typelib_TypeClass cpp2uno_call(
        bridges::cpp_uno::shared::CppInterfaceProxy* pThis,
        const typelib_TypeDescription * pMemberTypeDescr,
        typelib_TypeDescriptionReference * pReturnTypeRef,
        sal_Int32 nParams, typelib_MethodParameter * pParams,
        void ** pCallStack,
        sal_Int64 * pRegisterReturn /* space for register return */ )
    {
        // pCallStack: ret, [return ptr], this, params
        char * pTopStack = reinterpret_cast<char *>(pCallStack + 0);
        char * pCppStack = pTopStack;

#ifdef __ARM_PCS_VFP
        int dc = 0;
        char * pFloatArgs = reinterpret_cast<char *>(pCppStack - 64);
#endif
        // return
        typelib_TypeDescription * pReturnTypeDescr = nullptr;
        if (pReturnTypeRef)
            TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );

        void * pUnoReturn = nullptr;
        // complex return ptr: if != 0 && != pUnoReturn, reconversion need
        void * pCppReturn = nullptr;

        if (pReturnTypeDescr)
        {
            if (!arm::return_in_hidden_param(pReturnTypeRef))
                pUnoReturn = pRegisterReturn; // direct way for simple types
            else // complex return via ptr (pCppReturn)
            {
                pCppReturn = *reinterpret_cast<void **>(pCppStack);
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
        static_assert(sizeof(void *) == sizeof(sal_Int32),
            "### unexpected size!");
        // parameters
        void ** pUnoArgs = static_cast<void **>(alloca( 4 * sizeof(void *) * nParams ));
        void ** pCppArgs = pUnoArgs + nParams;
        // indices of values this have to be converted (interface conversion
        // cpp<=>uno)
        sal_Int32 * pTempIndices = reinterpret_cast<sal_Int32 *>(pUnoArgs + (2 * nParams));
        // type descriptions for reconversions
        typelib_TypeDescription ** ppTempParamTypeDescr =
            reinterpret_cast<typelib_TypeDescription **>(pUnoArgs + (3 * nParams));

        sal_Int32 nTempIndices   = 0;

        for ( sal_Int32 nPos = 0; nPos < nParams; ++nPos )
        {
            const typelib_MethodParameter & rParam = pParams[nPos];
            typelib_TypeDescription * pParamTypeDescr = nullptr;
            TYPELIB_DANGER_GET( &pParamTypeDescr, rParam.pTypeRef );

            if (!rParam.bOut &&
                bridges::cpp_uno::shared::isSimpleType( pParamTypeDescr ))
            {
#ifdef __ARM_EABI__
                switch (pParamTypeDescr->eTypeClass)
                {
                    case typelib_TypeClass_HYPER:
                    case typelib_TypeClass_UNSIGNED_HYPER:
#ifndef __ARM_PCS_VFP
                    case typelib_TypeClass_DOUBLE:
#endif
                        if ((pCppStack - pTopStack) % 8) pCppStack+=sizeof(sal_Int32); //align to 8
                        break;
                    default:
                        break;
                }
#endif

// For armhf we get the floating point arguments from a different area of the stack
#ifdef __ARM_PCS_VFP
                if (pParamTypeDescr->eTypeClass == typelib_TypeClass_FLOAT)
                {
                    pCppArgs[nPos] =  pUnoArgs[nPos] = pFloatArgs;
                    pFloatArgs += sizeof(float);
                } else
                if (pParamTypeDescr->eTypeClass == typelib_TypeClass_DOUBLE)
                {
                    if ((pFloatArgs - pTopStack) % 8) pFloatArgs+=sizeof(float); //align to 8
                    pCppArgs[nPos] = pUnoArgs[nPos] = pFloatArgs;
                    pFloatArgs += sizeof(double);
                    if (++dc == arm::MAX_FPR_REGS) {
                        if (pCppStack - pTopStack < 16)
                            pCppStack = pTopStack + 16;
                        pFloatArgs = pCppStack;
                    }
                } else
#endif
                    pCppArgs[nPos] = pUnoArgs[nPos] = pCppStack;

                switch (pParamTypeDescr->eTypeClass)
                {
                    case typelib_TypeClass_HYPER:
                    case typelib_TypeClass_UNSIGNED_HYPER:
#ifndef __ARM_PCS_VFP
                    case typelib_TypeClass_DOUBLE:
#endif
                        pCppStack += sizeof(sal_Int32); // extra long
                        break;
                    default:
                        break;
                }
                // no longer needed
                TYPELIB_DANGER_RELEASE( pParamTypeDescr );
            }
            else // ptr to complex value | ref
            {
                pCppArgs[nPos] = *reinterpret_cast<void **>(pCppStack);

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
                    uno_copyAndConvertData( pUnoArgs[nPos] =
                        alloca( pParamTypeDescr->nSize ),
                        *reinterpret_cast<void **>(pCppStack), pParamTypeDescr,
                        pThis->getBridge()->getCpp2Uno() );
                    pTempIndices[nTempIndices] = nPos; // has to be reconverted
                    // will be released at reconversion
                    ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
                }
                else // direct way
                {
                    pUnoArgs[nPos] = *reinterpret_cast<void **>(pCppStack);
                    // no longer needed
                    TYPELIB_DANGER_RELEASE( pParamTypeDescr );
                }
            }
#ifdef __ARM_PCS_VFP
            // use the stack for output parameters or non floating point values
                if (rParam.bOut ||
                        ((pParamTypeDescr->eTypeClass != typelib_TypeClass_DOUBLE)
                         && (pParamTypeDescr->eTypeClass != typelib_TypeClass_FLOAT))
                    )
#endif
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
                    uno_destructData( pUnoArgs[nIndex],
                        ppTempParamTypeDescr[nTempIndices], nullptr );
                TYPELIB_DANGER_RELEASE( ppTempParamTypeDescr[nTempIndices] );
            }
            if (pReturnTypeDescr)
                TYPELIB_DANGER_RELEASE( pReturnTypeDescr );

            CPPU_CURRENT_NAMESPACE::raiseException( &aUnoExc,
                pThis->getBridge()->getUno2Cpp() ); // has to destruct the any
            // is here for dummy
            return typelib_TypeClass_VOID;
        }
        else // else no exception occurred...
        {
            // temporary params
            for ( ; nTempIndices--; )
            {
                sal_Int32 nIndex = pTempIndices[nTempIndices];
                typelib_TypeDescription * pParamTypeDescr =
                    ppTempParamTypeDescr[nTempIndices];

                if (pParams[nIndex].bOut) // inout/out
                {
                    // convert and assign
                    uno_destructData( pCppArgs[nIndex], pParamTypeDescr,
                        cpp_release );
                    uno_copyAndConvertData( pCppArgs[nIndex], pUnoArgs[nIndex],
                        pParamTypeDescr, pThis->getBridge()->getUno2Cpp() );
                }
                // destroy temp uno param
                uno_destructData( pUnoArgs[nIndex], pParamTypeDescr, nullptr );

                TYPELIB_DANGER_RELEASE( pParamTypeDescr );
            }
            // return
            if (pCppReturn) // has complex return
            {
                if (pUnoReturn != pCppReturn) // needs reconversion
                {
                    uno_copyAndConvertData( pCppReturn, pUnoReturn,
                        pReturnTypeDescr, pThis->getBridge()->getUno2Cpp() );
                    // destroy temp uno return
                    uno_destructData( pUnoReturn, pReturnTypeDescr, nullptr );
                }
                // complex return ptr is set to eax
                *reinterpret_cast<void **>(pRegisterReturn) = pCppReturn;
            }
            if (pReturnTypeDescr)
            {
                typelib_TypeClass eRet =
                    static_cast<typelib_TypeClass>(pReturnTypeDescr->eTypeClass);
                TYPELIB_DANGER_RELEASE( pReturnTypeDescr );
                return eRet;
            }
            else
                return typelib_TypeClass_VOID;
        }
    }


    typelib_TypeClass cpp_mediate(
        sal_Int32 nFunctionIndex, sal_Int32 nVtableOffset,
        void ** pCallStack,
        sal_Int64 * pRegisterReturn /* space for register return */ )
    {
        static_assert(sizeof(sal_Int32)==sizeof(void *), "### unexpected!");

        // pCallStack: [ret *], this, params
        // _this_ ptr is patched cppu_XInterfaceProxy object
        void *pThis;
        if( nFunctionIndex & 0x80000000 )
        {
            nFunctionIndex &= 0x7fffffff;
            pThis = pCallStack[1];
        }
        else
        {
            pThis = pCallStack[0];
        }

        pThis = static_cast< char * >(pThis) - nVtableOffset;
        bridges::cpp_uno::shared::CppInterfaceProxy * pCppI =
            bridges::cpp_uno::shared::CppInterfaceProxy::castInterfaceToProxy(
                pThis);

        typelib_InterfaceTypeDescription * pTypeDescr = pCppI->getTypeDescr();

        if (nFunctionIndex >= pTypeDescr->nMapFunctionIndexToMemberIndex)
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
                reinterpret_cast<XInterface *>(pCppI));
        }

        // determine called method
        assert(nFunctionIndex < pTypeDescr->nMapFunctionIndexToMemberIndex);
        sal_Int32 nMemberPos =
            pTypeDescr->pMapFunctionIndexToMemberIndex[nFunctionIndex];
        assert(nMemberPos < pTypeDescr->nAllMembers);

        TypeDescription aMemberDescr( pTypeDescr->ppAllMembers[nMemberPos] );

        typelib_TypeClass eRet;
        switch (aMemberDescr.get()->eTypeClass)
        {
        case typelib_TypeClass_INTERFACE_ATTRIBUTE:
        {
            if (pTypeDescr->pMapMemberIndexToFunctionIndex[nMemberPos] ==
                nFunctionIndex)
            {
                // is GET method
                eRet = cpp2uno_call(
                    pCppI, aMemberDescr.get(),
                    reinterpret_cast<typelib_InterfaceAttributeTypeDescription *>(aMemberDescr.get())->pAttributeTypeRef,
                    0, nullptr, // no params
                    pCallStack, pRegisterReturn );
            }
            else
            {
                // is SET method
                typelib_MethodParameter aParam;
                aParam.pTypeRef =
                    reinterpret_cast<typelib_InterfaceAttributeTypeDescription *>(aMemberDescr.get())->pAttributeTypeRef;
                aParam.bIn      = true;
                aParam.bOut     = false;

                eRet = cpp2uno_call(
                    pCppI, aMemberDescr.get(),
                    nullptr, // indicates void return
                    1, &aParam,
                    pCallStack, pRegisterReturn );
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
                eRet = typelib_TypeClass_VOID;
                break;
            case 2: // release()
                pCppI->releaseProxy(); // non virtual call!
                eRet = typelib_TypeClass_VOID;
                break;
            case 0: // queryInterface() opt
            {
                typelib_TypeDescription * pTD = nullptr;
                TYPELIB_DANGER_GET(&pTD,
                    reinterpret_cast<Type *>(pCallStack[2])->getTypeLibType());
                if (pTD)
                {
                    XInterface * pInterface = nullptr;
                    (*pCppI->getBridge()->getCppEnv()->getRegisteredInterface)(
                        pCppI->getBridge()->getCppEnv(),
                        reinterpret_cast<void **>(&pInterface), pCppI->getOid().pData,
                        reinterpret_cast<typelib_InterfaceTypeDescription *>(pTD) );

                    if (pInterface)
                    {
                        ::uno_any_construct(
                            reinterpret_cast< uno_Any * >( pCallStack[0] ),
                            &pInterface, pTD, cpp_acquire );
                        pInterface->release();
                        TYPELIB_DANGER_RELEASE( pTD );
                        *reinterpret_cast<void **>(pRegisterReturn) = pCallStack[0];
                        eRet = typelib_TypeClass_ANY;
                        break;
                    }
                    TYPELIB_DANGER_RELEASE( pTD );
                }
            } [[fallthrough]]; // else perform queryInterface()
            default:
                eRet = cpp2uno_call(
                    pCppI, aMemberDescr.get(),
                    reinterpret_cast<typelib_InterfaceMethodTypeDescription *>(aMemberDescr.get())->pReturnTypeRef,
                    reinterpret_cast<typelib_InterfaceMethodTypeDescription *>(aMemberDescr.get())->nParams,
                    reinterpret_cast<typelib_InterfaceMethodTypeDescription *>(aMemberDescr.get())->pParams,
                    pCallStack, pRegisterReturn );
            }
            break;
        }
        default:
        {
            throw RuntimeException( "no member description found!", reinterpret_cast<XInterface *>(pCppI) );
        }
        }

        return eRet;
    }
}

/**
 * is called on incoming vtable calls
 * (called by asm snippets)
 */

sal_Int64 cpp_vtable_call( long *pFunctionAndOffset,
    void **pCallStack )
{
    sal_Int64 nRegReturn;
    typelib_TypeClass aType = cpp_mediate( pFunctionAndOffset[0], pFunctionAndOffset[1], pCallStack,
        &nRegReturn );

    switch( aType )
    {
        case typelib_TypeClass_BOOLEAN:
        case typelib_TypeClass_BYTE:
            nRegReturn = static_cast<unsigned long>(*reinterpret_cast<unsigned char *>(&nRegReturn));
            break;
        case typelib_TypeClass_CHAR:
        case typelib_TypeClass_UNSIGNED_SHORT:
        case typelib_TypeClass_SHORT:
            nRegReturn = static_cast<unsigned long>(*reinterpret_cast<unsigned short *>(&nRegReturn));
            break;
        case typelib_TypeClass_ENUM:
        case typelib_TypeClass_UNSIGNED_LONG:
        case typelib_TypeClass_LONG:
            nRegReturn = static_cast<unsigned long>(*reinterpret_cast<unsigned int *>(&nRegReturn));
            break;
        case typelib_TypeClass_VOID:
        default:
            break;
    }

    return nRegReturn;
}

namespace
{
    const int codeSnippetSize = 20;

    unsigned char *codeSnippet(unsigned char* code, sal_Int32 functionIndex,
        sal_Int32 vtableOffset, bool bHasHiddenParam)
    {
        if (bHasHiddenParam)
            functionIndex |= 0x80000000;

        unsigned long * p = reinterpret_cast<unsigned long *>(code);

        // ARM (not thumb) mode instructions
        // mov ip, pc
        *p++ = 0xE1A0C00F;
        // ldr pc, [pc, #4]
        *p++ = 0xE59FF004;
        *p++ = static_cast<unsigned long>(functionIndex);
        *p++ = static_cast<unsigned long>(vtableOffset);
        *p++ = reinterpret_cast<unsigned long>(privateSnippetExecutor);

        return code + codeSnippetSize;
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
    return (slotCount + 2) * sizeof (Slot) + slotCount * codeSnippetSize;
}

bridges::cpp_uno::shared::VtableFactory::Slot *
bridges::cpp_uno::shared::VtableFactory::initializeBlock(
    void * block, sal_Int32 slotCount, sal_Int32,
    typelib_InterfaceTypeDescription *)
{
    Slot * slots = mapBlockToVtable(block);
    slots[-2].fn = nullptr;
    slots[-1].fn = nullptr;
    return slots + slotCount;
}

unsigned char * bridges::cpp_uno::shared::VtableFactory::addLocalFunctions(
    Slot ** slots, unsigned char * code,
#ifdef USE_DOUBLE_MMAP
    sal_PtrDiff writetoexecdiff,
#endif
    typelib_InterfaceTypeDescription const * type, sal_Int32 functionOffset,
    sal_Int32 functionCount, sal_Int32 vtableOffset)
{
#ifndef USE_DOUBLE_MMAP
    const sal_PtrDiff writetoexecdiff = 0;
#endif
    (*slots) -= functionCount;
    Slot * s = *slots;
    for (sal_Int32 i = 0; i < type->nMembers; ++i)
    {
        typelib_TypeDescription * member = nullptr;
        TYPELIB_DANGER_GET(&member, type->ppMembers[i]);
        assert(member != 0);
        switch (member->eTypeClass)
        {
            case typelib_TypeClass_INTERFACE_ATTRIBUTE:
            {
                typelib_InterfaceAttributeTypeDescription *pAttrTD =
                    reinterpret_cast<typelib_InterfaceAttributeTypeDescription *>( member );

                // Getter:
                (s++)->fn = code + writetoexecdiff;
                code = codeSnippet(
                    code, functionOffset++, vtableOffset,
                    arm::return_in_hidden_param( pAttrTD->pAttributeTypeRef ));

                // Setter:
                if (!pAttrTD->bReadOnly)
                {
                    (s++)->fn = code + writetoexecdiff;
                    code = codeSnippet(
                        code, functionOffset++, vtableOffset, false);
                }
                break;
            }
            case typelib_TypeClass_INTERFACE_METHOD:
            {
                (s++)->fn = code + writetoexecdiff;

                typelib_InterfaceMethodTypeDescription *pMethodTD =
                    reinterpret_cast<
                        typelib_InterfaceMethodTypeDescription * >(member);

                code = codeSnippet(code, functionOffset++, vtableOffset,
                    arm::return_in_hidden_param(pMethodTD->pReturnTypeRef));
                break;
            }
        default:
            assert(false);
            break;
        }
        TYPELIB_DANGER_RELEASE(member);
    }
    return code;
}

void bridges::cpp_uno::shared::VtableFactory::flushCode(
    unsigned char const *beg, unsigned char const *end)
{
#ifndef ANDROID
   static void (*clear_cache)(unsigned char const*, unsigned char const*)
       = (void (*)(unsigned char const*, unsigned char const*))
           dlsym(RTLD_DEFAULT, "__clear_cache");
   (*clear_cache)(beg, end);
#else
   cacheflush((long) beg, (long) end, 0);
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
