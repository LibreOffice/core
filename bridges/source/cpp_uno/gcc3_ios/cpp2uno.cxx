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
#include <com/sun/star/uno/RuntimeException.hpp>
#include <sal/log.hxx>
#include <uno/data.h>
#include <typelib/typedescription.hxx>
#include "bridge.hxx"
#include "cppinterfaceproxy.hxx"
#include "types.hxx"
#include "vtablefactory.hxx"
#include "share.hxx"



extern "C" int codeSnippets[];
const int nFunIndexes = 8;
const int nVtableOffsets = 4;



using namespace ::com::sun::star::uno;

namespace
{
    static typelib_TypeClass cpp2uno_call(
        bridges::cpp_uno::shared::CppInterfaceProxy* pThis,
        const typelib_TypeDescription * pMemberTypeDescr,
        typelib_TypeDescriptionReference * pReturnTypeRef,
        sal_Int32 nParams,
        typelib_MethodParameter * pParams,
        void ** pCallStack,
        sal_Int64 * pRegisterReturn /* space for register return */ )
    {
        // pCallStack: x8, lr, d0..d7, x0..x7, rest of params originally on stack
        char *pTopStack = (char *)pCallStack;
        char *pFloatRegs = pTopStack + 2;
        char *pGPRegs =  pTopStack + (2+8)*8;
        char *pStackedArgs = pTopStack + (2+8+8)*8;

        int nGPR = 0;
        int nFPR = 0;

        // return
        typelib_TypeDescription * pReturnTypeDescr = 0;
        if (pReturnTypeRef)
            TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );

        void * pUnoReturn = 0;
        // complex return ptr: if != 0 && != pUnoReturn, reconversion need
        void * pCppReturn = 0;

        if (pReturnTypeDescr)
        {
            if (!arm::return_in_x8(pReturnTypeRef))
                pUnoReturn = pRegisterReturn; // direct way for simple types
            else // complex return via x8
            {
                pCppReturn = pCallStack[0];

                pUnoReturn = (bridges::cpp_uno::shared::relatesToInterfaceType(
                    pReturnTypeDescr )
                        ? alloca( pReturnTypeDescr->nSize )
                        : pCppReturn); // direct way
            }
        }

        // Skip 'this'
        pGPRegs += 8;
        nGPR++;

        // Parameters
        void ** pUnoArgs = (void **)alloca( sizeof(void *) * nParams );
        void ** pCppArgs = (void **)alloca( sizeof(void *) * nParams );

        // Indices of values this have to be converted (interface conversion
        // cpp<=>uno)
        int * pTempIndices = (sal_Int32 *)alloca( sizeof(int) * nParams);

        // Type descriptions for reconversions
        typelib_TypeDescription ** ppTempParamTypeDescr = (typelib_TypeDescription **)alloca( sizeof(typelib_TypeDescription *) * nParams);

        int nTempIndices = 0;

        for ( int nPos = 0; nPos < nParams; ++nPos )
        {
            const typelib_MethodParameter & rParam = pParams[nPos];
            typelib_TypeDescription * pParamTypeDescr = 0;
            TYPELIB_DANGER_GET( &pParamTypeDescr, rParam.pTypeRef );

            if (!rParam.bOut &&
                bridges::cpp_uno::shared::isSimpleType( pParamTypeDescr ))
            {
                if (nFPR < 8 && (pParamTypeDescr->eTypeClass == typelib_TypeClass_FLOAT ||
                                 pParamTypeDescr->eTypeClass == typelib_TypeClass_DOUBLE))
                {
                    pCppArgs[nPos] = pUnoArgs[nPos] = pFloatRegs;
                    pFloatRegs += 8;
                    nFPR++;
                }
                else if (pParamTypeDescr->eTypeClass == typelib_TypeClass_FLOAT)
                {
                    if ((pStackedArgs - pTopStack) % 4)
                        pStackedArgs += 4 - ((pStackedArgs - pTopStack) % 4);
                    pCppArgs[nPos] = pUnoArgs[nPos] = pStackedArgs;
                    pStackedArgs += 4;
                }
                else if (pParamTypeDescr->eTypeClass == typelib_TypeClass_DOUBLE)
                {
                    if ((pStackedArgs - pTopStack) % 8)
                        pStackedArgs += 8 - ((pStackedArgs - pTopStack) % 8);
                    pCppArgs[nPos] = pUnoArgs[nPos] = pStackedArgs;
                    pStackedArgs += 8;
                }
                else if (nGPR < 8)
                {
                    pCppArgs[nPos] = pUnoArgs[nPos] = pGPRegs;
                    pGPRegs += 8;
                    nGPR++;
                }
                else
                switch (pParamTypeDescr->eTypeClass)
                {
                    case typelib_TypeClass_HYPER:
                    case typelib_TypeClass_UNSIGNED_HYPER:
                        if ((pStackedArgs - pTopStack) % 8)
                            pStackedArgs += 8 - ((pStackedArgs - pTopStack) % 8);
                        pCppArgs[nPos] = pUnoArgs[nPos] = pStackedArgs;
                        pStackedArgs += 8;
                        break;
                    case typelib_TypeClass_ENUM:
                    case typelib_TypeClass_LONG:
                    case typelib_TypeClass_UNSIGNED_LONG:
                        if ((pStackedArgs - pTopStack) % 4)
                            pStackedArgs += 4 - ((pStackedArgs - pTopStack) % 4);
                        pCppArgs[nPos] = pUnoArgs[nPos] = pStackedArgs;
                        pStackedArgs += 4;
                        break;
                    case typelib_TypeClass_CHAR:
                    case typelib_TypeClass_SHORT:
                    case typelib_TypeClass_UNSIGNED_SHORT:
                        if ((pStackedArgs - pTopStack) % 2)
                            pStackedArgs += 1;
                        pCppArgs[nPos] = pUnoArgs[nPos] = pStackedArgs;
                        pStackedArgs += 2;
                        break;
                    case typelib_TypeClass_BOOLEAN:
                    case typelib_TypeClass_BYTE:
                        pCppArgs[nPos] = pUnoArgs[nPos] = pStackedArgs;
                        pStackedArgs += 1;
                        break;
                    default:
                        assert(!"should not happen");
                        break;
                }
                // no longer needed
                TYPELIB_DANGER_RELEASE( pParamTypeDescr );
            }
            else // ptr to complex value | ref
            {
                if (nGPR < 8)
                {
                    pCppArgs[nPos] = *(void **)pGPRegs;
                    pGPRegs += 8;
                }
                else
                {
                    if ((pStackedArgs - pTopStack) % 8)
                        pStackedArgs += 8 - ((pStackedArgs - pTopStack) % 8);
                    pCppArgs[nPos] = pStackedArgs;
                    pStackedArgs += 8;
                }

                if (! rParam.bIn) // is pure out
                {
                    // uno out is unconstructed mem!
                    pUnoArgs[nPos] = alloca( pParamTypeDescr->nSize );
                    pTempIndices[nTempIndices] = nPos;
                    // will be released at reconversion
                    ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
                }
                // is in/inout
                else if (bridges::cpp_uno::shared::relatesToInterfaceType( pParamTypeDescr ))
                {
                    uno_copyAndConvertData( pUnoArgs[nPos] =
                        alloca( pParamTypeDescr->nSize ),
                        pCppArgs[nPos], pParamTypeDescr,
                        pThis->getBridge()->getCpp2Uno() );
                    pTempIndices[nTempIndices] = nPos; // has to be reconverted
                    // will be released at reconversion
                    ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
                }
                else // direct way
                {
                    pUnoArgs[nPos] = pCppArgs[nPos];
                    // no longer needed
                    TYPELIB_DANGER_RELEASE( pParamTypeDescr );
                }
            }
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
                int nIndex = pTempIndices[nTempIndices];

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
                int nIndex = pTempIndices[nTempIndices];
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


    static void cpp_mediate(sal_Int32 nFunctionIndex,
                            sal_Int32 nVtableOffset,
                            void ** pCallStack)
    {
        sal_Int64 nRegReturn;
        sal_Int64 *pRegisterReturn = &nRegReturn;

        // pCallStack: x8, lr, d0..d7, x0..x7, rest of params originally on stack
        // _this_ ptr is patched cppu_XInterfaceProxy object
        void *pThis = pCallStack[2 + 8];

        pThis = static_cast< char * >(pThis) - nVtableOffset;
        bridges::cpp_uno::shared::CppInterfaceProxy * pCppI =
            bridges::cpp_uno::shared::CppInterfaceProxy::castInterfaceToProxy(
                pThis);

        typelib_InterfaceTypeDescription * pTypeDescr = pCppI->getTypeDescr();

        // determine called method
        assert( nFunctionIndex < pTypeDescr->nMapFunctionIndexToMemberIndex );

        if (nFunctionIndex >= pTypeDescr->nMapFunctionIndexToMemberIndex)
        {
            throw RuntimeException( "illegal vtable index!", (XInterface *)pCppI );
        }

        sal_Int32 nMemberPos =
            pTypeDescr->pMapFunctionIndexToMemberIndex[nFunctionIndex];
        assert( nMemberPos < pTypeDescr->nAllMembers );

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
                    pCallStack, pRegisterReturn );
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
                typelib_TypeDescription * pTD = 0;
                TYPELIB_DANGER_GET(&pTD,
                    reinterpret_cast<Type *>(pCallStack[2])->getTypeLibType());
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
                            reinterpret_cast< uno_Any * >( pCallStack[0] ),
                            &pInterface, pTD, cpp_acquire );
                        pInterface->release();
                        TYPELIB_DANGER_RELEASE( pTD );
                        *(void **)pRegisterReturn = pCallStack[0];
                        eRet = typelib_TypeClass_ANY;
                        break;
                    }
                    TYPELIB_DANGER_RELEASE( pTD );
                }
            } // else perform queryInterface()
            [[fallthrough]];
            default:
                eRet = cpp2uno_call(
                    pCppI, aMemberDescr.get(),
                    ((typelib_InterfaceMethodTypeDescription *)aMemberDescr.get())->pReturnTypeRef,
                    ((typelib_InterfaceMethodTypeDescription *)aMemberDescr.get())->nParams,
                    ((typelib_InterfaceMethodTypeDescription *)aMemberDescr.get())->pParams,
                    pCallStack, pRegisterReturn );
            }
            break;
        }
        default:
        {
            throw RuntimeException( "no member description found!", (XInterface *)pCppI );
        }
        }

        (void)eRet;
        return;
    }
}

/**
 * is called on incoming vtable calls
 * (called by asm snippets)
 */

extern "C" void cpp_vtable_call( sal_Int32 func, sal_Int32 offset,
                                 void **pStack )
{
    cpp_mediate(func, offset, pStack);
}

namespace
{
    unsigned char *codeSnippet(const typelib_InterfaceTypeDescription *type,
                               const typelib_TypeDescription *member,
                               sal_Int32 functionIndex,
                               sal_Int32 vtableOffset)
    {
        // For now temporarily assert when we get here. The intent is
        // that we won't need the code snippets at all on iOS.
        assert(false);

        assert(functionIndex < nFunIndexes);
        if (!(functionIndex < nFunIndexes))
            return NULL;

        assert(vtableOffset < nVtableOffsets);
        if (!(vtableOffset < nVtableOffsets))
            return NULL;

        // The codeSnippets table is indexed by functionIndex and vtableOffset

        int index = functionIndex*nVtableOffsets + vtableOffset;
        unsigned char *result = ((unsigned char *) &codeSnippets) + codeSnippets[index];

        SAL_INFO( "bridges", "codeSnippet(" << OUString(type->aBase.pTypeName) << "::" << OUString(member->pTypeName) << "): [" << functionIndex << "," << vtableOffset << "]=" << (void *) result << " (" << std::hex << ((int*)result)[0] << "," << ((int*)result)[1] << "," << ((int*)result)[2] << "," << ((int*)result)[3] << ")");

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
    Slot ** slots,
    unsigned char * code,
    typelib_InterfaceTypeDescription const * type,
    sal_Int32 functionOffset,
    sal_Int32 functionCount,
    sal_Int32 vtableOffset)
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
            {
                typelib_InterfaceAttributeTypeDescription *pAttrTD =
                    reinterpret_cast<typelib_InterfaceAttributeTypeDescription *>( member );

                // Getter:
                (s++)->fn = codeSnippet( type, member, functionOffset++, vtableOffset );

                // Setter:
                if (!pAttrTD->bReadOnly)
                {
                    (s++)->fn = codeSnippet( type, member, functionOffset++, vtableOffset );
                }
                break;
            }
            case typelib_TypeClass_INTERFACE_METHOD:
            {
                (s++)->fn = codeSnippet( type, member, functionOffset++, vtableOffset );
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
    unsigned char const *, unsigned char const *)
{
    // No dynamic code generation so nothing to flush
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
