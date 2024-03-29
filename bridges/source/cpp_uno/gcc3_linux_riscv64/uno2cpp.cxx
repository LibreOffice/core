/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <sal/config.h>

#include <exception>
#include <malloc.h>
#include <cstring>
#include <typeinfo>

#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/genfunc.hxx>
#include <o3tl/runtimetooustring.hxx>
#include <uno/data.h>

#include "bridge.hxx"
#include "types.hxx"
#include "unointerfaceproxy.hxx"
#include "vtables.hxx"

#include "share.hxx"
#include "abi.hxx"

using namespace ::com::sun::star::uno;

namespace
{
void insertArgs(sal_uInt64 value, sal_uInt64& nGPR, sal_uInt64* pGPR, sal_uInt64*& sp)
{
    if (nGPR < MAX_GP_REGS)
        pGPR[nGPR++] = value;
    else
        *(sp++) = value;
}

static void callVirtualMethod(void* pAdjustedThisPtr, sal_Int32 nVtableIndex, void* pRegisterReturn,
                              typelib_TypeDescriptionReference* pReturnTypeRef, bool bSimpleReturn,
                              sal_uInt64* pStack, sal_uInt32 nStack, sal_uInt64* pGPR, double* pFPR,
                              typelib_TypeDescription* pReturnTypeDescr)
{
    BRIDGE_LOG("In callVirtualMethod:\n");
    BRIDGE_LOG(
        "pAdjustedThisPtr = %p, nVtableIndex = %d, pRegisterReturn = %p, pReturnTypeRef = %p\n",
        pAdjustedThisPtr, nVtableIndex, pRegisterReturn, pReturnTypeRef);
    BRIDGE_LOG(
        "bSimpleReturn = %d, pStack = %p, nStack = %d, pGPR = %p, pFPR = %p, pReturnTypeDescr = "
        "%p\n",
        bSimpleReturn, pStack, nStack, pGPR, pFPR, pReturnTypeDescr);
    // Get pointer to method
    sal_uInt64 pMethod = *((sal_uInt64*)pAdjustedThisPtr);
    pMethod += 8 * nVtableIndex;
    void* mfunc = (void*)*((sal_uInt64*)pMethod);
    BRIDGE_LOG("calling function %p\n", mfunc);

    // Load parameters to stack, if necessary
    sal_uInt64* pCallStack = NULL;
    if (nStack)
    {
        // 16-bytes aligned
        sal_uInt32 nStackBytes = ((nStack + 1) >> 1) * 16;
        pCallStack = (sal_uInt64*)__builtin_alloca(nStackBytes);
        std::memcpy(pCallStack, pStack, nStackBytes);
    }

    sal_Int64* gret = (sal_Int64*)malloc(2 * sizeof(sal_Int64));
    sal_Int64* gret1 = gret;
    sal_Int64* gret2 = gret + 1;
    double* fret = (double*)malloc(2 * sizeof(double));
    double* fret1 = fret;
    double* fret2 = fret + 1;
    asm volatile(
        //".set push \n\t"
        //".set riscv64 \n\t"
        // Fill the general purpose registers
        "ld a0, 0(%[gpr]) \n\t"
        "ld a1, 8(%[gpr]) \n\t"
        "ld a2, 16(%[gpr]) \n\t"
        "ld a3, 24(%[gpr]) \n\t"
        "ld a4, 32(%[gpr]) \n\t"
        "ld a5, 40(%[gpr]) \n\t"
        "ld a6, 48(%[gpr]) \n\t"
        "ld a7, 56(%[gpr]) \n\t"
        // Fill the floating pointer registers
        "fld fa0, 0(%[fpr]) \n\t"
        "fld fa1, 8(%[fpr]) \n\t"
        "fld fa2, 16(%[fpr]) \n\t"
        "fld fa3, 24(%[fpr]) \n\t"
        "fld fa4, 32(%[fpr]) \n\t"
        "fld fa5, 40(%[fpr]) \n\t"
        "fld fa6, 48(%[fpr]) \n\t"
        "fld fa7, 56(%[fpr]) \n\t"
        // Perform the call
        "jalr ra,%[mfunc],0 \n\t"
        // Fill the return values
        "add %[gret1], a0,zero \n\t"
        "add %[gret2], a1,zero \n\t"
        "fmv.d %[fret1], fa0 \n\t"
        "fmv.d %[fret2], fa1 \n\t"
        //".set pop \n\t"
        : [gret1] "=&r"(*gret1), [gret2] "=&r"(*gret2), [fret1] "=&f"(*fret1), [fret2] "=&f"(*fret2)
        : [gpr] "r"(pGPR), [fpr] "r"(pFPR), [mfunc] "r"(mfunc),
          [stack] "m"(
              pCallStack) // dummy input to prevent the compiler from optimizing the alloca out
        : "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "ra", "fa0", "fa1", "fa2", "fa3", "fa4",
          "fa5", "fa6", "fa7", "memory");
    BRIDGE_LOG("In callVirtualMethod, fret = %p, gret = %p\n", fret, gret);

    switch (pReturnTypeRef->eTypeClass)
    {
        case typelib_TypeClass_HYPER:
        case typelib_TypeClass_UNSIGNED_HYPER:
        case typelib_TypeClass_LONG:
        case typelib_TypeClass_UNSIGNED_LONG:
        case typelib_TypeClass_ENUM:
        case typelib_TypeClass_CHAR:
        case typelib_TypeClass_SHORT:
        case typelib_TypeClass_UNSIGNED_SHORT:
        case typelib_TypeClass_BOOLEAN:
        case typelib_TypeClass_BYTE:
            *reinterpret_cast<sal_Int64*>(pRegisterReturn) = gret[0];
            break;
        case typelib_TypeClass_FLOAT:
        case typelib_TypeClass_DOUBLE:
            *reinterpret_cast<double*>(pRegisterReturn) = fret[0];
            break;
        case typelib_TypeClass_STRUCT:
        case typelib_TypeClass_EXCEPTION:
        {
            sal_Int32 const nRetSize = pReturnTypeRef->pType->nSize;
            BRIDGE_LOG("nRetSize = %d\n", nRetSize);
            if (bSimpleReturn && nRetSize <= 16 && nRetSize > 0)
            {
                typelib_TypeDescription* pTypeDescr = 0;
                TYPELIB_DANGER_GET(&pTypeDescr, pReturnTypeRef);
                abi_riscv64::fillUNOStruct(pTypeDescr, gret, fret, pRegisterReturn);
                TYPELIB_DANGER_RELEASE(pTypeDescr);
            }
            break;
        }
        default:
            BRIDGE_LOG("unhandled return type %u\n", pReturnTypeRef->eTypeClass);
            break;
    }
}

static void cpp_call(bridges::cpp_uno::shared::UnoInterfaceProxy* pThis,
                     bridges::cpp_uno::shared::VtableSlot aVtableSlot,
                     typelib_TypeDescriptionReference* pReturnTypeRef, sal_Int32 nParams,
                     typelib_MethodParameter* pParams, void* pUnoReturn, void* pUnoArgs[],
                     uno_Any** ppUnoExc)
{
    BRIDGE_LOG("In cpp_call\n");
    BRIDGE_LOG("pThis = %p, aVtableSlot = %p, pReturnTypeRef = %p, nParams = %d\n", pThis,
               aVtableSlot, pReturnTypeRef, nParams);
    BRIDGE_LOG("pParams = %p , pUnoReturn = %p, pUnoArgs = %p\n", pParams, pUnoReturn, pUnoArgs);
    // max space for: [complex ret ptr], values|ptr ...
    sal_uInt64* pStack = (sal_uInt64*)__builtin_alloca(((nParams + 3) * sizeof(sal_Int64)));
    sal_uInt64* pStackStart = pStack;

    sal_uInt64 pGPR[MAX_GP_REGS];
    sal_uInt64 nGPR = 0;

    double pFPR[MAX_FP_REGS];
    sal_uInt32 nFPR = 0;
    BRIDGE_LOG("pGPR = %p, pFPR = %p\n", pGPR, pFPR);

    // return
    typelib_TypeDescription* pReturnTypeDescr = 0;
    TYPELIB_DANGER_GET(&pReturnTypeDescr, pReturnTypeRef);
    assert(pReturnTypeDescr);

    void* pCppReturn = 0; // if != 0 && != pUnoReturn, needs reconversion

    bool bSimpleReturn = true;
    if (pReturnTypeDescr)
    {
        if (CPPU_CURRENT_NAMESPACE::return_in_hidden_param(pReturnTypeRef))
        {
            bSimpleReturn = false;
            // complex return via ptr
            pCppReturn = bridges::cpp_uno::shared::relatesToInterfaceType(pReturnTypeDescr)
                             ? __builtin_alloca(pReturnTypeDescr->nSize)
                             : pUnoReturn;
            pGPR[nGPR++] = reinterpret_cast<sal_uInt64>(pCppReturn);
        }
        else
        {
            pCppReturn = pUnoReturn; // direct way for simple types
        }
    }

    // push this
    void* pAdjustedThisPtr = reinterpret_cast<void**>(pThis->getCppI()) + aVtableSlot.offset;
    pGPR[nGPR++] = reinterpret_cast<sal_uInt64>(pAdjustedThisPtr);

    // args
    void** pCppArgs = (void**)alloca(3 * sizeof(void*) * nParams);
    // indices of values this have to be converted (interface conversion cpp<=>uno)
    sal_Int32* pTempIndices = (sal_Int32*)(pCppArgs + nParams);
    // type descriptions for reconversions
    typelib_TypeDescription** ppTempParamTypeDescr
        = (typelib_TypeDescription**)(pCppArgs + (2 * nParams));

    sal_Int32 nTempIndices = 0;
    BRIDGE_LOG("In cpp_call, nParams = %d\n", nParams);
    BRIDGE_LOG("pCppArgs = %p, pStack = %p\n", pCppArgs, pStack);
    for (sal_Int32 nPos = 0; nPos < nParams; ++nPos)
    {
        BRIDGE_LOG("In cpp_call, nPos = %d\n", nPos);
        const typelib_MethodParameter& rParam = pParams[nPos];
        typelib_TypeDescription* pParamTypeDescr = 0;
        TYPELIB_DANGER_GET(&pParamTypeDescr, rParam.pTypeRef);

        if (!rParam.bOut && bridges::cpp_uno::shared::isSimpleType(pParamTypeDescr))
        {
            BRIDGE_LOG("Before uno_copyAndConvertData and tons of switch.\n");
            uno_copyAndConvertData(pCppArgs[nPos] = alloca(8), pUnoArgs[nPos], pParamTypeDescr,
                                   pThis->getBridge()->getUno2Cpp());
            BRIDGE_LOG("Type = %d, Param = 0x%lx\n", pParamTypeDescr->eTypeClass,
                       *reinterpret_cast<sal_uInt64*>(pCppArgs[nPos]));
            switch (pParamTypeDescr->eTypeClass)
            {
                // In types.h:
                // typedef unsigned char sal_Bool
                case typelib_TypeClass_BOOLEAN:
                    insertArgs(*static_cast<sal_Bool*>(pCppArgs[nPos]), nGPR, pGPR, pStack);
                    break;
                case typelib_TypeClass_BYTE:
                    insertArgs(*static_cast<sal_Int8*>(pCppArgs[nPos]), nGPR, pGPR, pStack);
                    break;
                // typedef sal_uInt16 sal_Unicode
                case typelib_TypeClass_CHAR:
                    insertArgs(*static_cast<sal_Unicode*>(pCppArgs[nPos]), nGPR, pGPR, pStack);
                    break;
                case typelib_TypeClass_UNSIGNED_SHORT:
                    insertArgs(*static_cast<sal_uInt16*>(pCppArgs[nPos]), nGPR, pGPR, pStack);
                    break;
                case typelib_TypeClass_SHORT:
                    insertArgs(*static_cast<sal_Int16*>(pCppArgs[nPos]), nGPR, pGPR, pStack);
                    break;
                case typelib_TypeClass_UNSIGNED_LONG:
                    insertArgs(*static_cast<sal_uInt32*>(pCppArgs[nPos]), nGPR, pGPR, pStack);
                    break;
                case typelib_TypeClass_LONG:
                    insertArgs(*static_cast<sal_Int32*>(pCppArgs[nPos]), nGPR, pGPR, pStack);
                    break;
                // Todo: what type is enum?
                case typelib_TypeClass_ENUM:
                case typelib_TypeClass_UNSIGNED_HYPER:
                    insertArgs(*static_cast<sal_uInt64*>(pCppArgs[nPos]), nGPR, pGPR, pStack);
                    break;
                case typelib_TypeClass_HYPER:
                    insertArgs(*static_cast<sal_Int64*>(pCppArgs[nPos]), nGPR, pGPR, pStack);
                    break;
                // Floating point register -> General purpose register -> Stack
                case typelib_TypeClass_FLOAT:
                    char* higher32Bit;
                    if (nFPR < MAX_FP_REGS)
                    {
                        higher32Bit = reinterpret_cast<char*>(&pFPR[nFPR]) + 4;
                        std::memcpy(&(pFPR[nFPR++]), pCppArgs[nPos], 4);
                    }
                    else if (nGPR < MAX_GP_REGS)
                    {
                        higher32Bit = reinterpret_cast<char*>(&pGPR[nGPR]) + 4;
                        std::memcpy(&(pGPR[nGPR++]), pCppArgs[nPos], 4);
                    }
                    else
                    {
                        higher32Bit = reinterpret_cast<char*>(pStack) + 4;
                        std::memcpy(pStack++, pCppArgs[nPos], 4);
                    }
                    // Assure that the higher 32 bits are set to 1
                    std::memset(higher32Bit, 0xFF, 4);
                    break;
                case typelib_TypeClass_DOUBLE:
                    if (nFPR < MAX_FP_REGS)
                    {
                        std::memcpy(&(pFPR[nFPR++]), pCppArgs[nPos], 8);
                    }
                    else if (nGPR < MAX_GP_REGS)
                    {
                        std::memcpy(&(pGPR[nGPR++]), pCppArgs[nPos], 8);
                    }
                    else
                    {
                        std::memcpy(pStack++, pCppArgs[nPos], 8);
                    }
                    break;
                default:
                    break;
            }

            // no longer needed
            TYPELIB_DANGER_RELEASE(pParamTypeDescr);
        }
        else // ptr to complex value | ref
        {
            if (!rParam.bIn) // is pure out
            {
                // cpp out is constructed mem, uno out is not!
                uno_constructData(pCppArgs[nPos] = alloca(pParamTypeDescr->nSize), pParamTypeDescr);
                pTempIndices[nTempIndices] = nPos; // default constructed for cpp call
                // will be released at reconversion
                ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
            }
            // is in/inout
            else if (bridges::cpp_uno::shared::relatesToInterfaceType(pParamTypeDescr))
            {
                uno_copyAndConvertData(pCppArgs[nPos] = alloca(pParamTypeDescr->nSize),
                                       pUnoArgs[nPos], pParamTypeDescr,
                                       pThis->getBridge()->getUno2Cpp());

                pTempIndices[nTempIndices] = nPos; // has to be reconverted
                // will be released at reconversion
                ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
            }
            else // direct way
            {
                pCppArgs[nPos] = pUnoArgs[nPos];
                // no longer needed
                TYPELIB_DANGER_RELEASE(pParamTypeDescr);
            }
            insertArgs(reinterpret_cast<sal_uInt64>(pCppArgs[nPos]), nGPR, pGPR, pStack);
        }
    }

    try
    {
        try
        {
            callVirtualMethod(pAdjustedThisPtr, aVtableSlot.index, pCppReturn, pReturnTypeRef,
                              bSimpleReturn, pStackStart, (pStack - pStackStart), pGPR, pFPR,
                              pReturnTypeDescr);
        }
        catch (css::uno::Exception&)
        {
            throw;
        }
        catch (std::exception& e)
        {
            throw css::uno::RuntimeException("C++ code threw "
                                             + o3tl::runtimeToOUString(typeid(e).name()) + ": "
                                             + o3tl::runtimeToOUString(e.what()));
        }
        catch (...)
        {
            throw css::uno::RuntimeException("C++ code threw unknown exception");
        }
        // NO exception occurred...
        *ppUnoExc = 0;

        // reconvert temporary params
        for (; nTempIndices--;)
        {
            sal_Int32 nIndex = pTempIndices[nTempIndices];
            typelib_TypeDescription* pParamTypeDescr = ppTempParamTypeDescr[nTempIndices];

            if (pParams[nIndex].bIn)
            {
                if (pParams[nIndex].bOut) // inout
                {
                    uno_destructData(pUnoArgs[nIndex], pParamTypeDescr, 0); // destroy uno value
                    uno_copyAndConvertData(pUnoArgs[nIndex], pCppArgs[nIndex], pParamTypeDescr,
                                           pThis->getBridge()->getCpp2Uno());
                }
            }
            else // pure out
            {
                uno_copyAndConvertData(pUnoArgs[nIndex], pCppArgs[nIndex], pParamTypeDescr,
                                       pThis->getBridge()->getCpp2Uno());
            }
            // destroy temp cpp param => cpp: every param was constructed
            uno_destructData(pCppArgs[nIndex], pParamTypeDescr, cpp_release);

            TYPELIB_DANGER_RELEASE(pParamTypeDescr);
        }
        // return value
        if (pCppReturn && pUnoReturn != pCppReturn)
        {
            uno_copyAndConvertData(pUnoReturn, pCppReturn, pReturnTypeDescr,
                                   pThis->getBridge()->getCpp2Uno());
            uno_destructData(pCppReturn, pReturnTypeDescr, cpp_release);
        }
    }
    catch (...)
    {
        // fill uno exception
        CPPU_CURRENT_NAMESPACE::fillUnoException(*ppUnoExc, pThis->getBridge()->getCpp2Uno());

        // temporary params
        for (; nTempIndices--;)
        {
            sal_Int32 nIndex = pTempIndices[nTempIndices];
            // destroy temp cpp param => cpp: every param was constructed
            uno_destructData(pCppArgs[nIndex], ppTempParamTypeDescr[nTempIndices], cpp_release);
            TYPELIB_DANGER_RELEASE(ppTempParamTypeDescr[nTempIndices]);
        }
        // return type
        if (pReturnTypeDescr)
            TYPELIB_DANGER_RELEASE(pReturnTypeDescr);
    }
}
}

namespace bridges::cpp_uno::shared
{
void unoInterfaceProxyDispatch(uno_Interface* pUnoI, const typelib_TypeDescription* pMemberDescr,
                               void* pReturn, void* pArgs[], uno_Any** ppException)
{
    BRIDGE_LOG("In unoInterfaceProxyDispatch:\n");
    BRIDGE_LOG("pMemberDescr = %p, pReturn = %p, pArgs = %p, ppExeption = %p\n", pMemberDescr,
               pReturn, pArgs, ppException);
    // is my surrogate
    bridges::cpp_uno::shared::UnoInterfaceProxy* pThis
        = static_cast<bridges::cpp_uno::shared::UnoInterfaceProxy*>(pUnoI);
    //typelib_InterfaceTypeDescription * pTypeDescr = pThis->pTypeDescr;

    BRIDGE_LOG("in dispatch\n");

    switch (pMemberDescr->eTypeClass)
    {
        case typelib_TypeClass_INTERFACE_ATTRIBUTE:
        {
            VtableSlot aVtableSlot(getVtableSlot(
                reinterpret_cast<typelib_InterfaceAttributeTypeDescription const*>(pMemberDescr)));

            if (pReturn)
            {
                // dependent dispatch
                cpp_call(
                    pThis, aVtableSlot,
                    ((typelib_InterfaceAttributeTypeDescription*)pMemberDescr)->pAttributeTypeRef,
                    0, 0, // no params
                    pReturn, pArgs, ppException);
            }
            else
            {
                // is SET
                typelib_MethodParameter aParam;
                aParam.pTypeRef
                    = ((typelib_InterfaceAttributeTypeDescription*)pMemberDescr)->pAttributeTypeRef;
                aParam.bIn = sal_True;
                aParam.bOut = sal_False;

                typelib_TypeDescriptionReference* pReturnTypeRef = 0;
                OUString aVoidName("void");
                typelib_typedescriptionreference_new(&pReturnTypeRef, typelib_TypeClass_VOID,
                                                     aVoidName.pData);

                // dependent dispatch
                aVtableSlot.index += 1; //get then set method
                cpp_call(pThis, aVtableSlot, pReturnTypeRef, 1, &aParam, pReturn, pArgs,
                         ppException);

                typelib_typedescriptionreference_release(pReturnTypeRef);
            }

            break;
        }
        case typelib_TypeClass_INTERFACE_METHOD:
        {
            VtableSlot aVtableSlot(getVtableSlot(
                reinterpret_cast<typelib_InterfaceMethodTypeDescription const*>(pMemberDescr)));
            switch (aVtableSlot.index)
            {
                // standard calls
                case 1: // acquire uno interface
                    (*pUnoI->acquire)(pUnoI);
                    *ppException = 0;
                    break;
                case 2: // release uno interface
                    (*pUnoI->release)(pUnoI);
                    *ppException = 0;
                    break;
                case 0: // queryInterface() opt
                {
                    typelib_TypeDescription* pTD = 0;
                    TYPELIB_DANGER_GET(&pTD, reinterpret_cast<Type*>(pArgs[0])->getTypeLibType());
                    if (pTD)
                    {
                        uno_Interface* pInterface = 0;
                        (*pThis->pBridge->getUnoEnv()->getRegisteredInterface)(
                            pThis->pBridge->getUnoEnv(), (void**)&pInterface, pThis->oid.pData,
                            (typelib_InterfaceTypeDescription*)pTD);

                        if (pInterface)
                        {
                            ::uno_any_construct(reinterpret_cast<uno_Any*>(pReturn), &pInterface,
                                                pTD, 0);
                            (*pInterface->release)(pInterface);
                            TYPELIB_DANGER_RELEASE(pTD);
                            *ppException = 0;
                            break;
                        }
                        TYPELIB_DANGER_RELEASE(pTD);
                    }
                    [[fallthrough]];
                } // else perform queryInterface()
                default:
                    // dependent dispatch
                    cpp_call(
                        pThis, aVtableSlot,
                        ((typelib_InterfaceMethodTypeDescription*)pMemberDescr)->pReturnTypeRef,
                        ((typelib_InterfaceMethodTypeDescription*)pMemberDescr)->nParams,
                        ((typelib_InterfaceMethodTypeDescription*)pMemberDescr)->pParams, pReturn,
                        pArgs, ppException);
            }
            break;
        }
        default:
        {
            ::com::sun::star::uno::RuntimeException aExc(
                "illegal member type description!",
                ::com::sun::star::uno::Reference<::com::sun::star::uno::XInterface>());

            Type const& rExcType = cppu::UnoType<decltype(aExc)>::get();
            // binary identical null reference
            ::uno_type_any_construct(*ppException, &aExc, rExcType.getTypeLibType(), 0);
        }
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
