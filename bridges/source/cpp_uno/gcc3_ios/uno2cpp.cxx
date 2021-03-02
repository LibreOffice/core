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

#ifdef __arm64

#include <sal/config.h>

#include <exception>
#include <typeinfo>

#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <o3tl/runtimetooustring.hxx>

#include "bridge.hxx"
#include "types.hxx"
#include "unointerfaceproxy.hxx"
#include "vtables.hxx"

#include "share.hxx"

using namespace ::com::sun::star::uno;

namespace arm
{
    bool is_hfa_struct(const typelib_TypeDescription * type)
    {
        const typelib_CompoundTypeDescription * p
            = reinterpret_cast< const typelib_CompoundTypeDescription * >(type);
        if (p->nMembers >= 4)
            return false;
        for (sal_Int32 i = 0; i < p->nMembers; ++i)
        {
            if ((p->ppTypeRefs[i]->eTypeClass != typelib_TypeClass_FLOAT &&
                 p->ppTypeRefs[i]->eTypeClass != typelib_TypeClass_DOUBLE) ||
                p->ppTypeRefs[i]->eTypeClass != p->ppTypeRefs[0]->eTypeClass)
                return false;
        }
        return true;
    }

    bool return_in_x8( typelib_TypeDescriptionReference *pTypeRef )
    {
        if (bridges::cpp_uno::shared::isSimpleType(pTypeRef))
            return false;
        else if (pTypeRef->eTypeClass == typelib_TypeClass_STRUCT || pTypeRef->eTypeClass == typelib_TypeClass_EXCEPTION)
        {
            typelib_TypeDescription * pTypeDescr = 0;
            TYPELIB_DANGER_GET( &pTypeDescr, pTypeRef );

            // A Composite Type not larger than 16 bytes is returned in x0, x1
            bool bRet = pTypeDescr->nSize > 16;

            if (is_hfa_struct(pTypeDescr))
                bRet = false;

            TYPELIB_DANGER_RELEASE( pTypeDescr );
            return bRet;
        }
        return true;
    }
}

void MapReturn(sal_uInt64 x0, sal_uInt64 x1, double d0, typelib_TypeDescriptionReference *pReturnType, sal_uInt64 *pRegisterReturn)
{
    switch( pReturnType->eTypeClass )
    {
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
        pRegisterReturn[1] = x1;
        [[fallthrough]];
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
    case typelib_TypeClass_ENUM:
    case typelib_TypeClass_CHAR:
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
    case typelib_TypeClass_BOOLEAN:
    case typelib_TypeClass_BYTE:
        pRegisterReturn[0] = x0;
        break;
    case typelib_TypeClass_FLOAT:
        *(float*)pRegisterReturn = *(float*)&d0;
        break;
    case typelib_TypeClass_DOUBLE:
        *(double*)pRegisterReturn = d0;
        break;
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
        if (!arm::return_in_x8(pReturnType))
        {
            pRegisterReturn[0] = x0;
            pRegisterReturn[1] = x1;
        }
        break;
    default:
        break;
    }
}

namespace
{
void callVirtualMethod(
    void *pThis,
    sal_Int32 nVtableIndex,
    void *pRegisterReturn,
    typelib_TypeDescriptionReference *pReturnType,
    sal_uInt64 *pStack,
    int nStack,
    sal_uInt64 *pGPR,
    double *pFPR)
{
    // never called
    if (! pThis)
        CPPU_CURRENT_NAMESPACE::dummy_can_throw_anything("xxx"); // address something

    if ( nStack )
    {
        // 16-bytes aligned
        sal_uInt32 nStackBytes = ( ( nStack + 3 ) >> 2 ) * 16;
        sal_uInt32 *stack = (sal_uInt32 *) alloca( nStackBytes );
        memcpy( stack, pStack, nStackBytes );
    }

    sal_uInt64 pMethod = *((sal_uInt64*)pThis);
    pMethod += 8 * nVtableIndex;
    pMethod = *((sal_uInt64 *)pMethod);

    // For value returned in registers
    sal_uInt64 x0;
    sal_uInt64 x1;
    double d0;

    __asm__ __volatile__
    (
     // Assembly string
     "  ldp x0, x1, %[pgpr_0]\n"
     "  ldp x2, x3, %[pgpr_2]\n"
     "  ldp x4, x5, %[pgpr_4]\n"
     "  ldp x6, x7, %[pgpr_6]\n"
     "  ldr x8, %[pregisterreturn]\n"
     "  ldp d0, d1, %[pfpr_0]\n"
     "  ldp d2, d3, %[pfpr_2]\n"
     "  ldp d4, d5, %[pfpr_4]\n"
     "  ldp d6, d7, %[pfpr_6]\n"
     "  blr %[pmethod]\n"
     "  str x0, %[x0]\n"
     "  str x1, %[x1]\n"
     "  str d0, %[d0]\n"
     // Output operands
     : [x0]"=m" (x0), [x1]"=m" (x1), [d0]"=m" (d0)
     // Input operands
     : [pgpr_0]"m" (pGPR[0]),
       [pgpr_2]"m" (pGPR[2]),
       [pgpr_4]"m" (pGPR[4]),
       [pgpr_6]"m" (pGPR[6]),
       [pregisterreturn]"m" (pRegisterReturn),
       [pfpr_0]"m" (pFPR[0]),
       [pfpr_2]"m" (pFPR[2]),
       [pfpr_4]"m" (pFPR[4]),
       [pfpr_6]"m" (pFPR[6]),
       [pmethod]"r" (pMethod)
     // Clobbers
     : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7"
     );

    MapReturn(x0, x1, d0, pReturnType, (sal_uInt64 *) pRegisterReturn);
}
}

#define INSERT_INT64( pSV, nr, pGPR, pDS ) \
        if ( nr < arm::MAX_GPR_REGS ) \
                pGPR[nr++] = *reinterpret_cast<sal_uInt64 *>( pSV ); \
        else \
                *pDS++ = *reinterpret_cast<sal_uInt64 *>( pSV );

#define INSERT_INT32( pSV, nr, pGPR, pDS ) \
        if ( nr < arm::MAX_GPR_REGS ) \
                pGPR[nr++] = *reinterpret_cast<sal_uInt32 *>( pSV ); \
        else \
                *pDS++ = *reinterpret_cast<sal_uInt32 *>( pSV );

#define INSERT_INT16( pSV, nr, pGPR, pDS ) \
        if ( nr < arm::MAX_GPR_REGS ) \
                pGPR[nr++] = *reinterpret_cast<sal_uInt16 *>( pSV ); \
        else \
                *pDS++ = *reinterpret_cast<sal_uInt16 *>( pSV );

#define INSERT_INT8( pSV, nr, pGPR, pDS ) \
        if ( nr < arm::MAX_GPR_REGS ) \
                pGPR[nr++] = *reinterpret_cast<sal_uInt8 *>( pSV ); \
        else \
                *pDS++ = *reinterpret_cast<sal_uInt8 *>( pSV );

#define INSERT_DOUBLE( pSV, nr, pFPR, pDS ) \
        if ( nr < arm::MAX_FPR_REGS ) \
                pFPR[nr++] = *reinterpret_cast<double *>( pSV ); \
        else \
                *pDS++ = *reinterpret_cast<double *>( pSV );

#define INSERT_FLOAT( pSV, nr, pFPR, pDS ) \
        INSERT_DOUBLE( pSV, nr, pGPR, pDS )

namespace {
static void cpp_call(
    bridges::cpp_uno::shared::UnoInterfaceProxy * pThis,
    bridges::cpp_uno::shared::VtableSlot aVtableSlot,
    typelib_TypeDescriptionReference * pReturnTypeRef,
    sal_Int32 nParams, typelib_MethodParameter * pParams,
    void * pUnoReturn, void * pUnoArgs[], uno_Any ** ppUnoExc )
{
    // max space for: values|ptr ...
    sal_uInt64 * pStack = (sal_uInt64 *)alloca( (nParams+2) * sizeof(sal_Int64) );
    sal_uInt64 * pStackStart = pStack;

    sal_uInt64 pGPR[arm::MAX_GPR_REGS];
    int nGPR = 0;

    // storage and counter for SIMD/FP registers
    double pFPR[arm::MAX_FPR_REGS];
    int nFPR = 0;

    // return
    typelib_TypeDescription * pReturnTypeDescr = 0;
    TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );
    assert( pReturnTypeDescr);

    void * pCppReturn = 0; // if != 0 && != pUnoReturn, needs reconversion

    if (pReturnTypeDescr)
    {
        if (!arm::return_in_x8( pReturnTypeRef ) )
            pCppReturn = pUnoReturn; // direct way for simple types
        else
        {
            // complex return via x8
            pCppReturn = (bridges::cpp_uno::shared::relatesToInterfaceType( pReturnTypeDescr )
                    ? alloca( pReturnTypeDescr->nSize )
                    : pUnoReturn); // direct way
        }
    }
    // push this
    void * pAdjustedThisPtr = reinterpret_cast< void ** >(pThis->getCppI())
        + aVtableSlot.offset;
    INSERT_INT64( &pAdjustedThisPtr, nGPR, pGPR, pStack );

    // stack space
    // args
    void ** pCppArgs  = (void **)alloca( sizeof(void *) * nParams );

    // indices of values this have to be converted (interface conversion cpp<=>uno)
    int * pTempIndices = (int *)alloca( sizeof(int) * nParams );

    // type descriptions for reconversions
    typelib_TypeDescription ** ppTempParamTypeDescr = (typelib_TypeDescription **)alloca( sizeof(void *) * nParams );

    sal_Int32 nTempIndices = 0;

    for ( sal_Int32 nPos = 0; nPos < nParams; ++nPos )
    {
        const typelib_MethodParameter & rParam = pParams[nPos];
        typelib_TypeDescription * pParamTypeDescr = 0;
        TYPELIB_DANGER_GET( &pParamTypeDescr, rParam.pTypeRef );

        if (!rParam.bOut && bridges::cpp_uno::shared::isSimpleType( pParamTypeDescr ))
        {
            uno_copyAndConvertData( pCppArgs[nPos] = alloca(8), pUnoArgs[nPos],
                pParamTypeDescr, pThis->getBridge()->getUno2Cpp() );

            switch (pParamTypeDescr->eTypeClass)
            {
            case typelib_TypeClass_HYPER:
            case typelib_TypeClass_UNSIGNED_HYPER:
                INSERT_INT64( pCppArgs[nPos], nGPR, pGPR, pStack );
                break;
            case typelib_TypeClass_LONG:
            case typelib_TypeClass_UNSIGNED_LONG:
            case typelib_TypeClass_ENUM:
                INSERT_INT32( pCppArgs[nPos], nGPR, pGPR, pStack );
                break;
            case typelib_TypeClass_SHORT:
            case typelib_TypeClass_CHAR:
            case typelib_TypeClass_UNSIGNED_SHORT:
                INSERT_INT16( pCppArgs[nPos], nGPR, pGPR, pStack );
                break;
            case typelib_TypeClass_BOOLEAN:
            case typelib_TypeClass_BYTE:
                INSERT_INT8( pCppArgs[nPos], nGPR, pGPR, pStack );
                break;
            case typelib_TypeClass_FLOAT:
                INSERT_FLOAT( pCppArgs[nPos], nFPR, pFPR, pStack );
                break;
            case typelib_TypeClass_DOUBLE:
                INSERT_DOUBLE( pCppArgs[nPos], nFPR, pFPR, pStack );
                break;
            default:
                break;
            }
            // no longer needed
            TYPELIB_DANGER_RELEASE( pParamTypeDescr );
        }
        else // ptr to complex value | ref
        {
            if (! rParam.bIn) // is pure out
            {
                // cpp out is constructed mem, uno out is not!
                uno_constructData(
                    pCppArgs[nPos] = alloca( pParamTypeDescr->nSize ),
                    pParamTypeDescr );
                pTempIndices[nTempIndices] = nPos; // default constructed for cpp call
                // will be released at reconversion
                ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
            }
            // is in/inout
            else if (bridges::cpp_uno::shared::relatesToInterfaceType( pParamTypeDescr ))
            {
                uno_copyAndConvertData(
                    pCppArgs[nPos] = alloca( pParamTypeDescr->nSize ),
                    pUnoArgs[nPos], pParamTypeDescr, pThis->getBridge()->getUno2Cpp() );

                pTempIndices[nTempIndices] = nPos; // has to be reconverted
                // will be released at reconversion
                ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
            }
            else // direct way
            {
                pCppArgs[nPos] = pUnoArgs[nPos];
                // no longer needed
                TYPELIB_DANGER_RELEASE( pParamTypeDescr );
            }
            INSERT_INT64( &(pCppArgs[nPos]), nGPR, pGPR, pStack );
        }
    }

    assert( nGPR <= arm::MAX_GPR_REGS );
    assert( nFPR <= arm::MAX_FPR_REGS );

    try
    {
        try {
            callVirtualMethod(
                pAdjustedThisPtr, aVtableSlot.index,
                pCppReturn, pReturnTypeRef,
                pStackStart,
                (pStack - pStackStart),
                pGPR,
                pFPR);
        } catch (css::uno::Exception &) {
            throw;
        } catch (std::exception & e) {
            throw css::uno::RuntimeException(
                "C++ code threw " + o3tl::runtimeToOUString(typeid(e).name()) + ": "
                + o3tl::runtimeToOUString(e.what()));
        } catch (...) {
            throw css::uno::RuntimeException("C++ code threw unknown exception");
        }

        // NO exception occurred...
        *ppUnoExc = 0;

        // reconvert temporary params
        for ( ; nTempIndices--; )
        {
            sal_Int32 nIndex = pTempIndices[nTempIndices];
            typelib_TypeDescription * pParamTypeDescr = ppTempParamTypeDescr[nTempIndices];

            if (pParams[nIndex].bIn)
            {
                if (pParams[nIndex].bOut) // inout
                {
                    uno_destructData( pUnoArgs[nIndex], pParamTypeDescr, 0 ); // destroy uno value
                    uno_copyAndConvertData( pUnoArgs[nIndex], pCppArgs[nIndex], pParamTypeDescr,
                                            pThis->getBridge()->getCpp2Uno() );
                }
            }
            else // pure out
            {
                uno_copyAndConvertData( pUnoArgs[nIndex], pCppArgs[nIndex], pParamTypeDescr,
                                        pThis->getBridge()->getCpp2Uno() );
            }
            // destroy temp cpp param => cpp: every param was constructed
            uno_destructData( pCppArgs[nIndex], pParamTypeDescr, cpp_release );

            TYPELIB_DANGER_RELEASE( pParamTypeDescr );
        }
        // return value
        if (pCppReturn && pUnoReturn != pCppReturn)
        {
            uno_copyAndConvertData( pUnoReturn, pCppReturn, pReturnTypeDescr,
                                    pThis->getBridge()->getCpp2Uno() );
            uno_destructData( pCppReturn, pReturnTypeDescr, cpp_release );
        }
    }
    catch (...)
    {
        // fill uno exception
        CPPU_CURRENT_NAMESPACE::fillUnoException(*ppUnoExc, pThis->getBridge()->getCpp2Uno());

        // temporary params
        for ( ; nTempIndices--; )
        {
            sal_Int32 nIndex = pTempIndices[nTempIndices];
            // destroy temp cpp param => cpp: every param was constructed
            uno_destructData( pCppArgs[nIndex], ppTempParamTypeDescr[nTempIndices], cpp_release );
            TYPELIB_DANGER_RELEASE( ppTempParamTypeDescr[nTempIndices] );
        }

        // return type
        if (pReturnTypeDescr)
            TYPELIB_DANGER_RELEASE( pReturnTypeDescr );
    }
}
}

namespace bridges::cpp_uno::shared {

void unoInterfaceProxyDispatch(
    uno_Interface * pUnoI, const typelib_TypeDescription * pMemberDescr,
    void * pReturn, void * pArgs[], uno_Any ** ppException )
{
    // is my surrogate
    bridges::cpp_uno::shared::UnoInterfaceProxy * pThis
          = static_cast< bridges::cpp_uno::shared::UnoInterfaceProxy * >(pUnoI);
#if OSL_DEBUG_LEVEL > 0
    typelib_InterfaceTypeDescription * pTypeDescr = pThis->pTypeDescr;
#endif

    switch (pMemberDescr->eTypeClass)
    {
    case typelib_TypeClass_INTERFACE_ATTRIBUTE:
    {
#if OSL_DEBUG_LEVEL > 0
        // determine vtable call index
        sal_Int32 nMemberPos = ((typelib_InterfaceMemberTypeDescription *)pMemberDescr)->nPosition;
        assert( nMemberPos < pTypeDescr->nAllMembers && "### member pos out of range!");
#endif

        VtableSlot aVtableSlot(
            getVtableSlot(
            reinterpret_cast<typelib_InterfaceAttributeTypeDescription const *>
              (pMemberDescr)));

        if (pReturn)
        {
            // dependent dispatch
            cpp_call(
                pThis, aVtableSlot,
                ((typelib_InterfaceAttributeTypeDescription *)pMemberDescr)->pAttributeTypeRef,
                0, 0, // no params
                pReturn, pArgs, ppException );
        }
        else
        {
            // is SET
            typelib_MethodParameter aParam;
            aParam.pTypeRef =
                ((typelib_InterfaceAttributeTypeDescription *)pMemberDescr)->pAttributeTypeRef;
            aParam.bIn      = sal_True;
            aParam.bOut     = sal_False;

            typelib_TypeDescriptionReference * pReturnTypeRef = 0;
            OUString aVoidName("void");
            typelib_typedescriptionreference_new(
                &pReturnTypeRef, typelib_TypeClass_VOID, aVoidName.pData );

            // dependent dispatch
            aVtableSlot.index += 1;
            cpp_call(
                pThis, aVtableSlot, // get, then set method
                pReturnTypeRef,
                1, &aParam,
                pReturn, pArgs, ppException );

            typelib_typedescriptionreference_release( pReturnTypeRef );
        }

        break;
    }
    case typelib_TypeClass_INTERFACE_METHOD:
    {
#if OSL_DEBUG_LEVEL > 0
        // determine vtable call index
        sal_Int32 nMemberPos = ((typelib_InterfaceMemberTypeDescription *)pMemberDescr)->nPosition;
        assert(nMemberPos < pTypeDescr->nAllMembers && "### member pos out of range!");
#endif

        VtableSlot aVtableSlot(
            getVtableSlot(
            reinterpret_cast<typelib_InterfaceMethodTypeDescription const *>
              (pMemberDescr)));

        switch (aVtableSlot.index)
        {
            // standard calls
        case 1: // acquire uno interface
            (*pUnoI->acquire)( pUnoI );
            *ppException = 0;
            break;
        case 2: // release uno interface
            (*pUnoI->release)( pUnoI );
            *ppException = 0;
            break;
        case 0: // queryInterface() opt
        {
            typelib_TypeDescription * pTD = 0;
            TYPELIB_DANGER_GET( &pTD, reinterpret_cast< Type * >( pArgs[0] )->getTypeLibType() );
            if (pTD)
            {
                uno_Interface * pInterface = 0;
                (*pThis->getBridge()->getUnoEnv()->getRegisteredInterface)(
                    pThis->getBridge()->getUnoEnv(),
                    (void **)&pInterface, pThis->oid.pData, (typelib_InterfaceTypeDescription *)pTD );

                if (pInterface)
                {
                    ::uno_any_construct(
                        reinterpret_cast< uno_Any * >( pReturn ),
                        &pInterface, pTD, 0 );
                    (*pInterface->release)( pInterface );
                    TYPELIB_DANGER_RELEASE( pTD );
                    *ppException = 0;
                    break;
                }
                TYPELIB_DANGER_RELEASE( pTD );
            }
        } // else perform queryInterface()
        [[fallthrough]];
        default:
            // dependent dispatch
            cpp_call(
                pThis, aVtableSlot,
                ((typelib_InterfaceMethodTypeDescription *)pMemberDescr)->pReturnTypeRef,
                ((typelib_InterfaceMethodTypeDescription *)pMemberDescr)->nParams,
                ((typelib_InterfaceMethodTypeDescription *)pMemberDescr)->pParams,
                pReturn, pArgs, ppException );
        }
        break;
    }
    default:
    {
        ::com::sun::star::uno::RuntimeException aExc(
            "illegal member type description!",
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >() );

        Type const & rExcType = cppu::UnoType<decltype(aExc)>::get();
        // binary identical null reference
        ::uno_type_any_construct( *ppException, &aExc, rExcType.getTypeLibType(), 0 );
    }
    }
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
