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
#include <osl/mutex.hxx>

#include <com/sun/star/uno/genfunc.hxx>
#include "com/sun/star/uno/RuntimeException.hpp"
#include <uno/data.h>
#include <typelib/typedescription.hxx>

#include "bridges/cpp_uno/shared/bridge.hxx"
#include "bridges/cpp_uno/shared/cppinterfaceproxy.hxx"
#include "bridges/cpp_uno/shared/types.hxx"
#include "bridges/cpp_uno/shared/vtablefactory.hxx"

#include "share.hxx"

#include <dlfcn.h>


using namespace ::osl;
using namespace ::com::sun::star::uno;

namespace
{

    static typelib_TypeClass cpp2uno_call(
        bridges::cpp_uno::shared::CppInterfaceProxy* pThis,
        const typelib_TypeDescription * pMemberTypeDescr,
        typelib_TypeDescriptionReference * pReturnTypeRef,
        sal_Int32 nParams, typelib_MethodParameter * pParams,
        long r8, void ** pCallStack,
        sal_Int64 * pRegisterReturn /* space for register return */ )
    {
        // pCallStack: ret, [return ptr], this, params
        char * pTopStack = (char *)(pCallStack + 0);
        char * pCppStack = pTopStack;
#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr, "cpp2uno_call\n");
#endif
        // return
        typelib_TypeDescription * pReturnTypeDescr = 0;
        if (pReturnTypeRef)
            TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );

        void * pUnoReturn = 0;
        // complex return ptr: if != 0 && != pUnoReturn, reconversion need
        void * pCppReturn = 0;

        if (pReturnTypeDescr)
        {
            if (bridges::cpp_uno::shared::isSimpleType( pReturnTypeDescr ))
            {
#if OSL_DEBUG_LEVEL > 2
        fprintf(stderr, "simple return\n");
#endif
                pUnoReturn = pRegisterReturn; // direct way for simple types
            }
            else // complex return via ptr (pCppReturn)
            {
#if OSL_DEBUG_LEVEL > 2
        fprintf(stderr, "complex return\n");
#endif
                pCppReturn = (void *)r8;

                pUnoReturn = (bridges::cpp_uno::shared::relatesToInterfaceType( pReturnTypeDescr )
                    ? alloca( pReturnTypeDescr->nSize )
                    : pCppReturn); // direct way
            }
        }
        // pop this
        pCppStack += sizeof( void* );

        // stack space
        static_assert( sizeof(void *) == sizeof(sal_Int32),
            "### unexpected size!" );
        // parameters
        void ** pUnoArgs = (void **)alloca( 4 * sizeof(void *) * nParams );
        void ** pCppArgs = pUnoArgs + nParams;
        // indices of values this have to be converted (interface conversion
        // cpp<=>uno)
        sal_Int32 * pTempIndices = (sal_Int32 *)(pUnoArgs + (2 * nParams));
        // type descriptions for reconversions
        typelib_TypeDescription ** ppTempParamTypeDescr =
            (typelib_TypeDescription **)(pUnoArgs + (3 * nParams));

        sal_Int32 nTempIndices   = 0;

        for ( sal_Int32 nPos = 0; nPos < nParams; ++nPos )
        {
            const typelib_MethodParameter & rParam = pParams[nPos];
            typelib_TypeDescription * pParamTypeDescr = 0;
            TYPELIB_DANGER_GET( &pParamTypeDescr, rParam.pTypeRef );

            if (!rParam.bOut &&
                bridges::cpp_uno::shared::isSimpleType( pParamTypeDescr ))
            {
                switch (pParamTypeDescr->eTypeClass)
                {
                    case typelib_TypeClass_BYTE:
                    case typelib_TypeClass_BOOLEAN:
                        pCppArgs[nPos] = pCppStack + 3;
                        pUnoArgs[nPos] = pCppStack + 3;
                        break;
                    case typelib_TypeClass_CHAR:
                    case typelib_TypeClass_SHORT:
                    case typelib_TypeClass_UNSIGNED_SHORT:
                        pCppArgs[nPos] = pCppStack + 2;
                        pUnoArgs[nPos] = pCppStack + 2;
                        break;
                    case typelib_TypeClass_HYPER:
                    case typelib_TypeClass_UNSIGNED_HYPER:
                    case typelib_TypeClass_DOUBLE:
                        pCppArgs[nPos] = pCppStack;
                        pUnoArgs[nPos] = pCppStack;
                        pCppStack += sizeof(sal_Int32); // extra long
                        break;
                    default:
                        pCppArgs[nPos] = pCppStack;
                        pUnoArgs[nPos] = pCppStack;
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

#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr, "before dispatch\n");
#endif
        // invoke uno dispatch call
        (*pThis->getUnoI()->pDispatcher)(
          pThis->getUnoI(), pMemberTypeDescr, pUnoReturn, pUnoArgs, &pUnoExc );

#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr, "after dispatch\n");
#endif

        // in case an exception occurred...
        if (pUnoExc)
        {
            // destruct temporary in/inout params
            for ( ; nTempIndices--; )
            {
                sal_Int32 nIndex = pTempIndices[nTempIndices];

                if (pParams[nIndex].bIn) // is in/inout => was constructed
                    uno_destructData( pUnoArgs[nIndex],
                        ppTempParamTypeDescr[nTempIndices], 0 );
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
                uno_destructData( pUnoArgs[nIndex], pParamTypeDescr, 0 );

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
                    uno_destructData( pUnoReturn, pReturnTypeDescr, 0 );
                }
                // complex return ptr is set to return reg
                *(void **)pRegisterReturn = pCppReturn;
            }
            if (pReturnTypeDescr)
            {
                typelib_TypeClass eRet =
                    (typelib_TypeClass)pReturnTypeDescr->eTypeClass;
                TYPELIB_DANGER_RELEASE( pReturnTypeDescr );
                return eRet;
            }
            else
                return typelib_TypeClass_VOID;
        }
    }


    static typelib_TypeClass cpp_mediate(
        sal_Int32 nFunctionIndex, sal_Int32 nVtableOffset,
        long sp, long r8,
        sal_Int64 * pRegisterReturn /* space for register return */ )
    {
    void ** pCallStack = (void**)(sp);
#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr, "cpp_mediate with\n");
    fprintf(stderr, "%x %x\n", nFunctionIndex, nVtableOffset);
    fprintf(stderr, "and %x %x\n", pCallStack, pRegisterReturn);
    fprintf(stderr, "and %x %x\n", pCallStack[0], pCallStack[1]);
#endif
        static_assert( sizeof(sal_Int32)==sizeof(void *), "### unexpected!" );

        void *pThis = pCallStack[0];

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
                (XInterface *)pCppI);
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
                    ((typelib_InterfaceAttributeTypeDescription *)aMemberDescr.get())->pAttributeTypeRef,
                    0, 0, // no params
                    r8, pCallStack, pRegisterReturn );
            }
            else
            {
                // is SET method
                typelib_MethodParameter aParam;
                aParam.pTypeRef =
                    ((typelib_InterfaceAttributeTypeDescription *)aMemberDescr.get())->pAttributeTypeRef;
                aParam.bIn      = sal_True;
                aParam.bOut     = sal_False;

                eRet = cpp2uno_call(
                    pCppI, aMemberDescr.get(),
                    0, // indicates void return
                    1, &aParam,
                    r8, pCallStack, pRegisterReturn );
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
                typelib_TypeDescription * pTD = 0;
                TYPELIB_DANGER_GET(&pTD,
                    reinterpret_cast<Type *>(pCallStack[1])->getTypeLibType());
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
                            reinterpret_cast< uno_Any * >( r8 ),
                            &pInterface, pTD, cpp_acquire );
                        pInterface->release();
                        TYPELIB_DANGER_RELEASE( pTD );
                        *(void **)pRegisterReturn = (void*)r8;
                        eRet = typelib_TypeClass_ANY;
                        break;
                    }
                    TYPELIB_DANGER_RELEASE( pTD );
                }
            } // else perform queryInterface()
            default:
                eRet = cpp2uno_call(
                    pCppI, aMemberDescr.get(),
                    ((typelib_InterfaceMethodTypeDescription *)aMemberDescr.get())->pReturnTypeRef,
                    ((typelib_InterfaceMethodTypeDescription *)aMemberDescr.get())->nParams,
                    ((typelib_InterfaceMethodTypeDescription *)aMemberDescr.get())->pParams,
                    r8, pCallStack, pRegisterReturn );
            }
            break;
        }
        default:
        {
            throw RuntimeException( "no member description found!", (XInterface *)pCppI );
        }
        }

        return eRet;
    }
}

/**
 * is called on incoming vtable calls
 * (called by asm snippets)
 */

extern "C" sal_Int64 cpp_vtable_call( long firstonstack )
{
    register long d0 asm("d0");
    long functionIndex = d0;

    register long a1 asm("a1");
    long r8 = a1;

    register long d1 asm("d1");
    long vtableOffset = d1;

    long sp = (long)&firstonstack;

    sal_Int64 nRegReturn;
    cpp_mediate( functionIndex, vtableOffset, sp, r8, &nRegReturn );
    return nRegReturn;
}

namespace
{
    const int codeSnippetSize = 20;

    //some m68k info: http://www2.biglobe.ne.jp/~inaba/trampolines.html

    unsigned char *codeSnippet(unsigned char* code, sal_Int32 functionIndex,
        sal_Int32 vtableOffset)
    {
        unsigned char * p = code;
        *(short *)&p[0] = 0x203C;       //movel functionIndex,d0
        *(long *)&p[2] = functionIndex;
        *(short *)&p[6] = 0x223C;       //movel functionIndex,d1
        *(long *)&p[8] = vtableOffset;
        *(short *)&p[12] = 0x4EF9;      //jmp cpp_vtable_call
        *(long *)&p[14] = (long)&cpp_vtable_call;
        *(short *)&p[18] = 0x4E71;      //nop
        return code + codeSnippetSize;
    }
}

struct bridges::cpp_uno::shared::VtableFactory::Slot { void * fn; };

bridges::cpp_uno::shared::VtableFactory::Slot *
bridges::cpp_uno::shared::VtableFactory::mapBlockToVtable(void * block)
{
    return static_cast< Slot * >(block) + 2;
}

sal_Size bridges::cpp_uno::shared::VtableFactory::getBlockSize(
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
    slots[-2].fn = 0;
    slots[-1].fn = 0;
    return slots + slotCount;
}

unsigned char * bridges::cpp_uno::shared::VtableFactory::addLocalFunctions(
    Slot ** slots, unsigned char * code, sal_PtrDiff writetoexecdiff,
    typelib_InterfaceTypeDescription const * type, sal_Int32 functionOffset,
    sal_Int32 functionCount, sal_Int32 vtableOffset)
{
    (*slots) -= functionCount;
    Slot * s = *slots;
    for (sal_Int32 i = 0; i < type->nMembers; ++i)
    {
        typelib_TypeDescription * member = 0;
        TYPELIB_DANGER_GET(&member, type->ppMembers[i]);
        assert(member != 0);
        switch (member->eTypeClass)
        {
            case typelib_TypeClass_INTERFACE_ATTRIBUTE:
                // Getter:
                (s++)->fn = code + writetoexecdiff;
                code = codeSnippet(code, functionOffset++, vtableOffset);
                // Setter:
                if (!reinterpret_cast<
                    typelib_InterfaceAttributeTypeDescription * >(
                        member)->bReadOnly)
                {
                    (s++)->fn = code + writetoexecdiff;
                    code = codeSnippet(code, functionOffset++, vtableOffset);
                }
                break;
            case typelib_TypeClass_INTERFACE_METHOD:
            {
                (s++)->fn = code + writetoexecdiff;

                typelib_InterfaceMethodTypeDescription *pMethodTD =
                    reinterpret_cast<
                        typelib_InterfaceMethodTypeDescription * >(member);

                code = codeSnippet(code, functionOffset++, vtableOffset);
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
    unsigned char const * /*beg*/, unsigned char const * /*end*/)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
