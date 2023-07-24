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
void pushArgs(unsigned long value, unsigned long* const stack, sal_Int32* const sp,
              unsigned long* const regs, sal_Int32* const nregs)
{
    (*nregs != 8 ? regs[(*nregs)++] : stack[(*sp)++]) = value;
}

static void callVirtualMethod(void* pAdjustedThisPtr, sal_Int32 nVtableIndex, void* pRegisterReturn,
                              typelib_TypeDescriptionReference* pReturnTypeRef, bool bSimpleReturn,
                              sal_uInt64* pStack, sal_uInt32 nStack, sal_uInt64* pGPR, double* pFPR)
{
    // Get pointer to method
    sal_uInt64 pMethod = *((sal_uInt64*)pAdjustedThisPtr);
    pMethod += 8 * nVtableIndex;
    void* mfunc = (void*)*((sal_uInt64*)pMethod);
    // Load parameters to stack, if necessary
    sal_uInt64* pCallStack = NULL;
    if (nStack)
    {
        // 16-bytes aligned
        sal_uInt32 nStackBytes = ((nStack + 1) >> 1) * 16;
        pCallStack = (sal_uInt64*)__builtin_alloca(nStackBytes);
        std::memcpy(pCallStack, pStack, nStackBytes);
    }

    sal_Int64 gret[2];
    double fret[2];
    asm volatile(
        // Fill the general purpose registers
        "ld.d $r4,%[gpr],0 \n\t"
        "ld.d $r5,%[gpr],8 \n\t"
        "ld.d $r6,%[gpr],16 \n\t"
        "ld.d $r7,%[gpr],24 \n\t"
        "ld.d $r8,%[gpr],32 \n\t"
        "ld.d $r9,%[gpr],40 \n\t"
        "ld.d $r10,%[gpr],48 \n\t"
        "ld.d $r11,%[gpr],56 \n\t"
        // Fill the floating pointer registers
        "fld.d $f0,%[fpr],0 \n\t"
        "fld.d $f1,%[fpr],8 \n\t"
        "fld.d $f2,%[fpr],16 \n\t"
        "fld.d $f3,%[fpr],24 \n\t"
        "fld.d $f4,%[fpr],32 \n\t"
        "fld.d $f5,%[fpr],40 \n\t"
        "fld.d $f6,%[fpr],48 \n\t"
        "fld.d $f7,%[fpr],56 \n\t"
        // Perform the call
        "jirl $ra,%[mfunc],0 \n\t"
        // Fill the return values
        "move %[gret1], $a0 \n\t"
        "move %[gret2], $a1 \n\t"
        "fmov.d %[fret1], $f0 \n\t"
        "fmov.d %[fret2], $f1 \n\t"
        : [gret1] "=r"(gret[0]), [gret2] "=r"(gret[1]), [fret1] "=f"(fret[0]), [fret2] "=f"(fret[1])
        : [gpr] "r"(pGPR), [fpr] "r"(pFPR), [mfunc] "r"(mfunc),
          [stack] "m"(
              pCallStack) // dummy input to prevent the compiler from optimizing the alloca out
        : "$r4", "$r5", "$r6", "$r7", "$r8", "$r9", "$r10", "$r11", "$r1", "$f0", "$f1", "$f2",
          "$f3", "$f4", "$f5", "$f6", "$f7", "memory");

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
            if (bSimpleReturn && nRetSize <= 16 && nRetSize > 0)
            {
                loongarch64::fillReturn(pReturnTypeRef, gret, fret, pRegisterReturn);
            }
            break;
        }
        default:
            break;
    }
}

static void cpp_call(bridges::cpp_uno::shared::UnoInterfaceProxy* pThis,
                     bridges::cpp_uno::shared::VtableSlot aVtableSlot,
                     typelib_TypeDescriptionReference* pReturnTypeRef, sal_Int32 nParams,
                     typelib_MethodParameter* pParams, void* pUnoReturn, void* pUnoArgs[],
                     uno_Any** ppUnoExc)
{
    // max space for: [complex ret ptr], values|ptr ...
    sal_uInt64* pStack = (sal_uInt64*)__builtin_alloca(((nParams + 3) * sizeof(sal_Int64)));
    sal_uInt64* pStackStart = pStack;
    sal_Int32 sp = 0;

    sal_uInt64 pGPR[MAX_GP_REGS];
    sal_Int32 gCount = 0;
    double pFPR[MAX_FP_REGS];
    sal_Int32 fCount = 0;

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
            pGPR[gCount++] = reinterpret_cast<unsigned long>(pCppReturn);
        }
        else
        {
            pCppReturn = pUnoReturn; // direct way for simple types
        }
    }

    // push this
    void* pAdjustedThisPtr = reinterpret_cast<void**>(pThis->getCppI()) + aVtableSlot.offset;
    pGPR[gCount++] = reinterpret_cast<unsigned long>(pAdjustedThisPtr);

    // args
    void** pCppArgs = (void**)alloca(3 * sizeof(void*) * nParams);
    // indices of values this have to be converted (interface conversion cpp<=>uno)
    sal_Int32* pTempIndices = (sal_Int32*)(pCppArgs + nParams);
    // type descriptions for reconversions
    typelib_TypeDescription** ppTempParamTypeDescr
        = (typelib_TypeDescription**)(pCppArgs + (2 * nParams));

    sal_Int32 nTempIndices = 0;

    for (sal_Int32 nPos = 0; nPos < nParams; ++nPos)
    {
        const typelib_MethodParameter& rParam = pParams[nPos];
        typelib_TypeDescription* pParamTypeDescr = 0;
        TYPELIB_DANGER_GET(&pParamTypeDescr, rParam.pTypeRef);

        if (!rParam.bOut && bridges::cpp_uno::shared::isSimpleType(pParamTypeDescr))
        {
            uno_copyAndConvertData(pCppArgs[nPos] = alloca(8), pUnoArgs[nPos], pParamTypeDescr,
                                   pThis->getBridge()->getUno2Cpp());

            switch (pParamTypeDescr->eTypeClass)
            {
                case typelib_TypeClass_LONG:
                case typelib_TypeClass_ENUM:
                    pushArgs(*static_cast<sal_Int32*>(pCppArgs[nPos]), pStack, &sp, pGPR, &gCount);
                    break;
                case typelib_TypeClass_UNSIGNED_LONG:
                    pushArgs(*static_cast<sal_uInt32*>(pCppArgs[nPos]), pStack, &sp, pGPR, &gCount);
                    break;
                case typelib_TypeClass_CHAR:
                    pushArgs(*static_cast<sal_Unicode*>(pCppArgs[nPos]), pStack, &sp, pGPR,
                             &gCount);
                    break;
                case typelib_TypeClass_SHORT:
                    pushArgs(*static_cast<sal_Int16*>(pCppArgs[nPos]), pStack, &sp, pGPR, &gCount);
                    break;
                case typelib_TypeClass_UNSIGNED_SHORT:
                    pushArgs(*static_cast<sal_uInt16*>(pCppArgs[nPos]), pStack, &sp, pGPR, &gCount);
                    break;
                case typelib_TypeClass_BOOLEAN:
                    pushArgs(static_cast<unsigned long>(*static_cast<sal_Bool*>(pCppArgs[nPos])),
                             pStack, &sp, pGPR, &gCount);
                    break;
                case typelib_TypeClass_BYTE:
                    pushArgs(*static_cast<sal_Int8*>(pCppArgs[nPos]), pStack, &sp, pGPR, &gCount);
                    break;
                case typelib_TypeClass_FLOAT:
                case typelib_TypeClass_DOUBLE:
                    if (fCount != MAX_FP_REGS)
                    {
                        pFPR[fCount++] = *static_cast<double*>(pCppArgs[nPos]);
                    }
                    else if (gCount != MAX_GP_REGS)
                    {
                        pGPR[gCount++] = *static_cast<unsigned long*>(pCppArgs[nPos]);
                    }
                    else
                    {
                        pStack[sp++] = *static_cast<unsigned long*>(pCppArgs[nPos]);
                    }
                    break;
                case typelib_TypeClass_HYPER:
                    pushArgs(*static_cast<sal_Int64*>(pCppArgs[nPos]), pStack, &sp, pGPR, &gCount);
                    break;
                case typelib_TypeClass_UNSIGNED_HYPER:
                    pushArgs(*static_cast<sal_uInt64*>(pCppArgs[nPos]), pStack, &sp, pGPR, &gCount);
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
            pushArgs(reinterpret_cast<unsigned long>(pCppArgs[nPos]), pStack, &sp, pGPR, &gCount);
        }
    }

    try
    {
        try
        {
            callVirtualMethod(pAdjustedThisPtr, aVtableSlot.index, pCppReturn, pReturnTypeRef,
                              bSimpleReturn, pStackStart, sp, pGPR, pFPR);
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
    // is my surrogate
    bridges::cpp_uno::shared::UnoInterfaceProxy* pThis
        = static_cast<bridges::cpp_uno::shared::UnoInterfaceProxy*>(pUnoI);
    //typelib_InterfaceTypeDescription * pTypeDescr = pThis->pTypeDescr;

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
                } // else perform queryInterface()
                    [[fallthrough]];
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
