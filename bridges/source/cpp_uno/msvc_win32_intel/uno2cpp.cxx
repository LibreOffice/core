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
#include <uno/data.h>

#include <bridge.hxx>
#include <types.hxx>
#include <unointerfaceproxy.hxx>
#include <vtables.hxx>

#include <msvc/except.hxx>

using namespace ::com::sun::star;

namespace
{

inline void callVirtualMethod(
    void * pAdjustedThisPtr, sal_Int32 nVtableIndex,
    void * pRegisterReturn, typelib_TypeClass eReturnTypeClass,
    sal_Int32 * pStackLongs, sal_Int32 nStackLongs )
{
    // parameter list is mixed list of * and values
    // reference parameters are pointers

    assert(pStackLongs && pAdjustedThisPtr);
    static_assert( (sizeof(void *) == 4) &&
                 (sizeof(sal_Int32) == 4), "### unexpected size of int!" );

__asm
    {
        mov     eax, nStackLongs
        test    eax, eax
        je      Lcall
        // copy values
        mov     ecx, eax
        shl     eax, 2           // sizeof(sal_Int32) == 4
        add     eax, pStackLongs // params stack space
Lcopy:  sub     eax, 4
        push    dword ptr [eax]
        dec     ecx
        jne     Lcopy
Lcall:
        // call
        mov     ecx, pAdjustedThisPtr
        push    ecx             // this ptr
        mov     edx, [ecx]      // pvft
        mov     eax, nVtableIndex
        shl     eax, 2          // sizeof(void *) == 4
        add     edx, eax
        call    [edx]           // interface method call must be __cdecl!!!

        // register return
        mov     ecx, eReturnTypeClass
        cmp     ecx, typelib_TypeClass_VOID
        je      Lcleanup
        mov     ebx, pRegisterReturn
// int32
        cmp     ecx, typelib_TypeClass_LONG
        je      Lint32
        cmp     ecx, typelib_TypeClass_UNSIGNED_LONG
        je      Lint32
        cmp     ecx, typelib_TypeClass_ENUM
        je      Lint32
// int8
        cmp     ecx, typelib_TypeClass_BOOLEAN
        je      Lint8
        cmp     ecx, typelib_TypeClass_BYTE
        je      Lint8
// int16
        cmp     ecx, typelib_TypeClass_CHAR
        je      Lint16
        cmp     ecx, typelib_TypeClass_SHORT
        je      Lint16
        cmp     ecx, typelib_TypeClass_UNSIGNED_SHORT
        je      Lint16
// float
        cmp     ecx, typelib_TypeClass_FLOAT
        je      Lfloat
// double
        cmp     ecx, typelib_TypeClass_DOUBLE
        je      Ldouble
// int64
        cmp     ecx, typelib_TypeClass_HYPER
        je      Lint64
        cmp     ecx, typelib_TypeClass_UNSIGNED_HYPER
          je        Lint64
        jmp     Lcleanup // no simple type
Lint8:
        mov     byte ptr [ebx], al
        jmp     Lcleanup
Lint16:
        mov     word ptr [ebx], ax
        jmp     Lcleanup
Lfloat:
        fstp    dword ptr [ebx]
        jmp     Lcleanup
Ldouble:
        fstp    qword ptr [ebx]
        jmp     Lcleanup
Lint64:
        mov     dword ptr [ebx], eax
        mov     dword ptr [ebx+4], edx
        jmp     Lcleanup
Lint32:
        mov     dword ptr [ebx], eax
        jmp     Lcleanup
Lcleanup:
        // cleanup stack (obsolete though because of function)
        mov     eax, nStackLongs
        shl     eax, 2          // sizeof(sal_Int32) == 4
        add     eax, 4          // this ptr
        add     esp, eax
    }
}

void cpp_call(
    bridges::cpp_uno::shared::UnoInterfaceProxy * pThis,
    bridges::cpp_uno::shared::VtableSlot aVtableSlot,
    typelib_TypeDescriptionReference * pReturnTypeRef,
    sal_Int32 nParams,
    typelib_MethodParameter * pParams,
    void * pUnoReturn,
    void * pUnoArgs[],
    uno_Any ** ppUnoExc ) throw ()
{
    // max space for: [complex ret ptr], values|ptr ...
    char * pCppStack        = (char *)alloca( sizeof(sal_Int32) + (nParams * sizeof(sal_Int64)) );
    char * pCppStackStart   = pCppStack;

    // return type
    typelib_TypeDescription * pReturnTD = nullptr;
    TYPELIB_DANGER_GET( &pReturnTD, pReturnTypeRef );
    assert(pReturnTD);

    void * pCppReturn = nullptr; // if != 0 && != pUnoReturn, needs reconversion

    if (pReturnTD)
    {
        if (bridges::cpp_uno::shared::isSimpleType( pReturnTD ))
        {
            pCppReturn = pUnoReturn; // direct way for simple types
        }
        else
        {
            // complex return via ptr
            pCppReturn = *(void **)pCppStack
                = (bridges::cpp_uno::shared::relatesToInterfaceType(
                       pReturnTD )
                   ? alloca( pReturnTD->nSize )
                   : pUnoReturn); // direct way
            pCppStack += sizeof(void *);
        }
    }

    // stack space

    static_assert(sizeof(void *) == sizeof(sal_Int32), "### unexpected size!");
    // args
    void ** pCppArgs  = (void **)alloca( 3 * sizeof(void *) * nParams );
    // indices of values this have to be converted (interface conversion cpp<=>uno)
    sal_Int32 * pTempIndexes = (sal_Int32 *)(pCppArgs + nParams);
    // type descriptions for reconversions
    typelib_TypeDescription ** pTempParamTD = (typelib_TypeDescription **)(pCppArgs + (2 * nParams));

    sal_Int32 nTempIndexes   = 0;

    for (int nPos = 0; nPos < nParams; ++nPos)
    {
        const typelib_MethodParameter & rParam = pParams[nPos];
        typelib_TypeDescription * pParamTD = nullptr;
        TYPELIB_DANGER_GET( &pParamTD, rParam.pTypeRef );

        if (!rParam.bOut && bridges::cpp_uno::shared::isSimpleType(pParamTD))
        {
            ::uno_copyAndConvertData(
                pCppArgs[nPos] = pCppStack, pUnoArgs[nPos], pParamTD,
                pThis->getBridge()->getUno2Cpp() );

            switch (pParamTD->eTypeClass)
            {
            case typelib_TypeClass_HYPER:
            case typelib_TypeClass_UNSIGNED_HYPER:
            case typelib_TypeClass_DOUBLE:
                pCppStack += sizeof(sal_Int32); // extra long
                break;
            default:
                break;
            }
            // no longer needed
            TYPELIB_DANGER_RELEASE( pParamTD );
        }
        else // ptr to complex value | ref
        {
            if (! rParam.bIn) // is pure out
            {
                // C++ out is constructed mem, UNO out is not!
                ::uno_constructData(
                    *(void **)pCppStack = pCppArgs[nPos] = alloca( pParamTD->nSize ),
                    pParamTD );

                // default constructed for C++ call
                pTempIndexes[nTempIndexes] = nPos;

                // will be released at reconversion
                pTempParamTD[nTempIndexes++] = pParamTD;
            }
            // is in/inout
            else if (bridges::cpp_uno::shared::relatesToInterfaceType(pParamTD))
            {
                ::uno_copyAndConvertData(
                    *(void **)pCppStack = pCppArgs[nPos] = alloca( pParamTD->nSize ),
                    pUnoArgs[nPos], pParamTD,
                    pThis->getBridge()->getUno2Cpp() );

                // has to be reconverted
                pTempIndexes[nTempIndexes] = nPos;

                // will be released at reconversion
                pTempParamTD[nTempIndexes++] = pParamTD;
            }
            else // direct way
            {
                *(void **)pCppStack = pCppArgs[nPos] = pUnoArgs[nPos];
                // no longer needed
                TYPELIB_DANGER_RELEASE( pParamTD );
            }
        }
        pCppStack += sizeof(sal_Int32); // standard parameter length
    }

    __try
    {
        // pCppI is msci this pointer
        callVirtualMethod(
            reinterpret_cast< void ** >(pThis->getCppI()) + aVtableSlot.offset,
            aVtableSlot.index,
            pCppReturn, pReturnTD->eTypeClass,
            (sal_Int32 *)pCppStackStart,
            (pCppStack - pCppStackStart) / sizeof(sal_Int32) );
    }
    __except (msvc_filterCppException(
                  GetExceptionInformation(),
                  *ppUnoExc, pThis->getBridge()->getCpp2Uno() ))
   {
        // *ppUnoExc was constructed by filter function
        // temporary params
        while (nTempIndexes--)
        {
            sal_Int32 nIndex = pTempIndexes[nTempIndexes];
            // destroy temp C++ param => C++: every param was constructed
            ::uno_destructData(
                pCppArgs[nIndex], pTempParamTD[nTempIndexes],
                uno::cpp_release );
            TYPELIB_DANGER_RELEASE( pTempParamTD[nTempIndexes] );
        }

        // return type
        if (pReturnTD)
            TYPELIB_DANGER_RELEASE( pReturnTD );

        return;
    }

    // NO exception occurred
    *ppUnoExc = nullptr;

    // reconvert temporary params
    while (nTempIndexes--)
    {
        int nIndex = pTempIndexes[nTempIndexes];
        typelib_TypeDescription * pParamTD =
            pTempParamTD[nTempIndexes];

        if (pParams[nIndex].bIn)
        {
            if (pParams[nIndex].bOut) // inout
            {
                ::uno_destructData(
                    pUnoArgs[nIndex], pParamTD, nullptr ); // destroy UNO value
                ::uno_copyAndConvertData(
                    pUnoArgs[nIndex], pCppArgs[nIndex], pParamTD,
                    pThis->getBridge()->getCpp2Uno() );
            }
        }
        else // pure out
        {
            ::uno_copyAndConvertData(
                pUnoArgs[nIndex], pCppArgs[nIndex], pParamTD,
                pThis->getBridge()->getCpp2Uno() );
        }

        // destroy temp C++ param => C++: every param was constructed
        ::uno_destructData(
            pCppArgs[nIndex], pParamTD, uno::cpp_release );

        TYPELIB_DANGER_RELEASE( pParamTD );
    }

    // return value
    if (pCppReturn && pUnoReturn != pCppReturn)
    {
        ::uno_copyAndConvertData(
            pUnoReturn, pCppReturn, pReturnTD,
            pThis->getBridge()->getCpp2Uno() );
        ::uno_destructData(
            pCppReturn, pReturnTD, uno::cpp_release );
    }

    // return type
    if ( pReturnTD )
        TYPELIB_DANGER_RELEASE( pReturnTD );
}

} // namespace

namespace bridges::cpp_uno::shared {

void unoInterfaceProxyDispatch(
    uno_Interface * pUnoI,
    const typelib_TypeDescription * pMemberTD,
    void * pReturn,
    void * pArgs[],
    uno_Any ** ppException )
{
    // is my surrogate
    bridges::cpp_uno::shared::UnoInterfaceProxy * pThis
        = static_cast< bridges::cpp_uno::shared::UnoInterfaceProxy * >(pUnoI);

    switch (pMemberTD->eTypeClass)
    {
    case typelib_TypeClass_INTERFACE_ATTRIBUTE:
    {
        VtableSlot aVtableSlot(
            getVtableSlot(
                reinterpret_cast<
                    typelib_InterfaceAttributeTypeDescription const * >(
                        pMemberTD)));
        if ( pReturn )
        {
            // is GET
            cpp_call(
                pThis, aVtableSlot,
                reinterpret_cast<typelib_InterfaceAttributeTypeDescription const *>(pMemberTD)->pAttributeTypeRef,
                0, nullptr, // no params
                pReturn, pArgs, ppException );
        }
        else
        {
            // is SET
            typelib_MethodParameter aParam;
            aParam.pTypeRef =
                reinterpret_cast<typelib_InterfaceAttributeTypeDescription const *>(pMemberTD)->pAttributeTypeRef;
            aParam.bIn      = sal_True;
            aParam.bOut     = sal_False;

            typelib_TypeDescriptionReference * pReturnTypeRef = nullptr;
            OUString aVoidName("void");
            typelib_typedescriptionreference_new(
                &pReturnTypeRef, typelib_TypeClass_VOID, aVoidName.pData );

            aVtableSlot.index += 1; // get, then set method
            cpp_call(
                pThis, aVtableSlot,
                pReturnTypeRef,
                1, &aParam,
                pReturn, pArgs, ppException );

            typelib_typedescriptionreference_release( pReturnTypeRef );
        }

        break;
    }
    case typelib_TypeClass_INTERFACE_METHOD:
    {
        VtableSlot aVtableSlot(
            getVtableSlot(
                reinterpret_cast<
                    typelib_InterfaceMethodTypeDescription const * >(
                        pMemberTD)));

        switch (aVtableSlot.index)
        {
        case 1: // acquire UNO interface
            (*pUnoI->acquire)( pUnoI );
            *ppException = nullptr;
            break;
        case 2: // release UNO interface
            (*pUnoI->release)( pUnoI );
            *ppException = nullptr;
            break;
        case 0: // queryInterface() opt
        {
            typelib_TypeDescription * pTD = nullptr;
            TYPELIB_DANGER_GET( &pTD, static_cast< uno::Type * >( pArgs[0] )->getTypeLibType() );

            if ( pTD )
            {
                uno_Interface * pInterface = nullptr;
                (*pThis->getBridge()->getUnoEnv()->getRegisteredInterface)(
                    pThis->getBridge()->getUnoEnv(),
                    reinterpret_cast<void **>(&pInterface), pThis->oid.pData, reinterpret_cast<typelib_InterfaceTypeDescription *>(pTD) );

                if ( pInterface )
                {
                    ::uno_any_construct(
                        static_cast< uno_Any * >( pReturn ),
                        &pInterface, pTD, nullptr );
                    (*pInterface->release)( pInterface );
                    TYPELIB_DANGER_RELEASE( pTD );
                    *ppException = nullptr;
                    break;
                }
                TYPELIB_DANGER_RELEASE( pTD );
            }
            [[fallthrough]]; // else perform queryInterface()
        }
        default:
            typelib_InterfaceMethodTypeDescription const* pMethodTD
                = reinterpret_cast<typelib_InterfaceMethodTypeDescription const *>(pMemberTD);

            cpp_call(pThis, aVtableSlot, pMethodTD->pReturnTypeRef, pMethodTD->nParams, pMethodTD->pParams,
                     pReturn, pArgs, ppException);
        }
        break;
    }
    default:
    {
        uno::RuntimeException aExc("Illegal member type description!", uno::Reference<uno::XInterface>());

        uno::Type const & rExcType = cppu::UnoType<decltype(aExc)>::get();
        // binary identical null reference
        ::uno_type_any_construct(*ppException, &aExc, rExcType.getTypeLibType(), nullptr);
    }
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
