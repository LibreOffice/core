/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#ifdef __arm










#include <com/sun/star/uno/RuntimeException.hpp>

#include "bridges/cpp_uno/shared/bridge.hxx"
#include "bridges/cpp_uno/shared/types.hxx"
#include "bridges/cpp_uno/shared/unointerfaceproxy.hxx"
#include "bridges/cpp_uno/shared/vtables.hxx"

#include "share.hxx"

using namespace ::com::sun::star::uno;

namespace arm
{
    bool is_complex_struct(const typelib_TypeDescription * type)
    {
        const typelib_CompoundTypeDescription * p
            = reinterpret_cast< const typelib_CompoundTypeDescription * >(type);
        for (sal_Int32 i = 0; i < p->nMembers; ++i)
        {
            if (p->ppTypeRefs[i]->eTypeClass == typelib_TypeClass_STRUCT ||
                p->ppTypeRefs[i]->eTypeClass == typelib_TypeClass_EXCEPTION)
            {
                typelib_TypeDescription * t = 0;
                TYPELIB_DANGER_GET(&t, p->ppTypeRefs[i]);
                bool b = is_complex_struct(t);
                TYPELIB_DANGER_RELEASE(t);
                if (b) {
                    return true;
                }
            }
            else if (!bridges::cpp_uno::shared::isSimpleType(p->ppTypeRefs[i]->eTypeClass))
                return true;
        }
        if (p->pBaseTypeDescription != 0)
            return is_complex_struct(&p->pBaseTypeDescription->aBase);
        return false;
    }

#ifdef __ARM_PCS_VFP
    bool is_float_only_struct(const typelib_TypeDescription * type)
    {
        const typelib_CompoundTypeDescription * p
            = reinterpret_cast< const typelib_CompoundTypeDescription * >(type);
        for (sal_Int32 i = 0; i < p->nMembers; ++i)
        {
            if (p->ppTypeRefs[i]->eTypeClass != typelib_TypeClass_FLOAT &&
                p->ppTypeRefs[i]->eTypeClass != typelib_TypeClass_DOUBLE)
                return false;
        }
        return true;
    }
#endif
    bool return_in_hidden_param( typelib_TypeDescriptionReference *pTypeRef )
    {
        if (bridges::cpp_uno::shared::isSimpleType(pTypeRef))
            return false;
        else if (pTypeRef->eTypeClass == typelib_TypeClass_STRUCT || pTypeRef->eTypeClass == typelib_TypeClass_EXCEPTION)
        {
            typelib_TypeDescription * pTypeDescr = 0;
            TYPELIB_DANGER_GET( &pTypeDescr, pTypeRef );

            
            bool bRet = pTypeDescr->nSize > 4 || is_complex_struct(pTypeDescr);

#ifdef __ARM_PCS_VFP
            
            
            if( pTypeDescr->nSize <= 16 && is_float_only_struct(pTypeDescr))
                bRet = false;
#endif

            TYPELIB_DANGER_RELEASE( pTypeDescr );
            return bRet;
        }
        return true;
    }
}

void MapReturn(sal_uInt32 r0, sal_uInt32 r1, typelib_TypeDescriptionReference * pReturnType, sal_uInt32* pRegisterReturn)
{
    switch( pReturnType->eTypeClass )
    {
        case typelib_TypeClass_HYPER:
        case typelib_TypeClass_UNSIGNED_HYPER:
            pRegisterReturn[1] = r1;
        case typelib_TypeClass_LONG:
        case typelib_TypeClass_UNSIGNED_LONG:
        case typelib_TypeClass_ENUM:
        case typelib_TypeClass_CHAR:
        case typelib_TypeClass_SHORT:
        case typelib_TypeClass_UNSIGNED_SHORT:
        case typelib_TypeClass_BOOLEAN:
        case typelib_TypeClass_BYTE:
            pRegisterReturn[0] = r0;
            break;
        case typelib_TypeClass_FLOAT:
#if !defined(__ARM_PCS_VFP) && (defined(__ARM_EABI__) || defined(__SOFTFP__) || defined(IOS))
            pRegisterReturn[0] = r0;
#else
            register float fret asm("s0");
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
            *(float*)pRegisterReturn = fret;
#pragma GCC diagnostic pop
#endif
            break;
        case typelib_TypeClass_DOUBLE:
#if !defined(__ARM_PCS_VFP) && (defined(__ARM_EABI__) || defined(__SOFTFP__) || defined(IOS))
            pRegisterReturn[1] = r1;
            pRegisterReturn[0] = r0;
#else
            register double dret asm("d0");
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
            *(double*)pRegisterReturn = dret;
#pragma GCC diagnostic pop
#endif
            break;
        case typelib_TypeClass_STRUCT:
        case typelib_TypeClass_EXCEPTION:
        {
            if (!arm::return_in_hidden_param(pReturnType))
                pRegisterReturn[0] = r0;
            break;
        }
        default:
            break;
    }
}

namespace
{


void callVirtualMethod(
    void * pThis,
    sal_Int32 nVtableIndex,
    void * pRegisterReturn,
    typelib_TypeDescriptionReference * pReturnType,
    sal_uInt32 *pStack,
    sal_uInt32 nStack,
    sal_uInt32 *pGPR,
    sal_uInt32 nGPR,
    double *pFPR) __attribute__((noinline));

void callVirtualMethod(
    void * pThis,
    sal_Int32 nVtableIndex,
    void * pRegisterReturn,
    typelib_TypeDescriptionReference * pReturnType,
    sal_uInt32 *pStack,
    sal_uInt32 nStack,
    sal_uInt32 *pGPR,
    sal_uInt32 nGPR,
    double *pFPR)
{
    
    if (! pThis)
        CPPU_CURRENT_NAMESPACE::dummy_can_throw_anything("xxx"); 

    if ( nStack )
    {
        
        sal_uInt32 nStackBytes = ( ( nStack + 1 ) >> 1 ) * 8;
        sal_uInt32 *stack = (sal_uInt32 *) __builtin_alloca( nStackBytes );
        memcpy( stack, pStack, nStackBytes );
    }

    
    if ( nGPR > arm::MAX_GPR_REGS )
        nGPR = arm::MAX_GPR_REGS;

    sal_uInt32 pMethod = *((sal_uInt32*)pThis);
    pMethod += 4 * nVtableIndex;
    pMethod = *((sal_uInt32 *)pMethod);

    
    sal_uInt32 r0;
    sal_uInt32 r1;

    __asm__ __volatile__ (
        
        "ldr r4, %[pgpr]\n\t"
        "ldmia r4, {r0-r3}\n\t"

#ifdef __ARM_PCS_VFP
        
        "ldr r4, %[pfpr]\n\t"
        "vldmia r4, {d0-d7}\n\t"
#endif
        
        "ldr r5, %[pmethod]\n\t"
#ifndef __ARM_ARCH_4T__
        "blx r5\n\t"
#else
        "mov lr, pc ; bx r5\n\t"
#endif

        
        "mov %[r0], r0\n\t"
        "mov %[r1], r1\n\t"
        : [r0]"=r" (r0), [r1]"=r" (r1)
        : [pmethod]"m" (pMethod), [pgpr]"m" (pGPR), [pfpr]"m" (pFPR)
        : "r0", "r1", "r2", "r3", "r4", "r5");

    MapReturn(r0, r1, pReturnType, (sal_uInt32*)pRegisterReturn);
}
}

#define INSERT_INT32( pSV, nr, pGPR, pDS ) \
        if ( nr < arm::MAX_GPR_REGS ) \
                pGPR[nr++] = *reinterpret_cast<sal_uInt32 *>( pSV ); \
        else \
                *pDS++ = *reinterpret_cast<sal_uInt32 *>( pSV );

#ifdef __ARM_EABI__
#define INSERT_INT64( pSV, nr, pGPR, pDS, pStart ) \
        if ( (nr < arm::MAX_GPR_REGS) && (nr % 2) ) \
        { \
                ++nr; \
        } \
        if ( nr < arm::MAX_GPR_REGS ) \
        { \
                pGPR[nr++] = *reinterpret_cast<sal_uInt32 *>( pSV ); \
                pGPR[nr++] = *(reinterpret_cast<sal_uInt32 *>( pSV ) + 1); \
        } \
        else \
    { \
        if ( (pDS - pStart) % 2) \
                { \
                    ++pDS; \
                } \
                *pDS++ = reinterpret_cast<sal_uInt32 *>( pSV )[0]; \
                *pDS++ = reinterpret_cast<sal_uInt32 *>( pSV )[1]; \
    }
#else
#define INSERT_INT64( pSV, nr, pGPR, pDS, pStart ) \
        INSERT_INT32( pSV, nr, pGPR, pDS ) \
        INSERT_INT32( ((sal_uInt32*)pSV)+1, nr, pGPR, pDS )
#endif

#ifdef __ARM_PCS_VFP





//



#define INSERT_FLOAT( pSV, nr, pGPR, pDS ) \
        if (nSR % 2 == 0) {\
            nSR = 2*nDR; \
        }\
        if ( nSR < arm::MAX_FPR_REGS*2 ) {\
                pSPR[nSR++] = *reinterpret_cast<float *>( pSV ); \
                if ((nSR % 2 == 1) && (nSR > 2*nDR)) {\
                    nDR++; \
                }\
        }\
        else \
        {\
                *pDS++ = *reinterpret_cast<float *>( pSV );\
        }
#define INSERT_DOUBLE( pSV, nr, pGPR, pDS, pStart ) \
        if ( nDR < arm::MAX_FPR_REGS ) { \
                pFPR[nDR++] = *reinterpret_cast<double *>( pSV ); \
        }\
        else\
        {\
            if ( (pDS - pStart) % 2) \
                { \
                    ++pDS; \
                } \
            *(double *)pDS = *reinterpret_cast<double *>( pSV );\
            pDS += 2;\
        }
#else
#define INSERT_FLOAT( pSV, nr, pFPR, pDS ) \
        INSERT_INT32( pSV, nr, pGPR, pDS )

#define INSERT_DOUBLE( pSV, nr, pFPR, pDS, pStart ) \
        INSERT_INT64( pSV, nr, pGPR, pDS, pStart )
#endif

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

namespace {

static void cpp_call(
    bridges::cpp_uno::shared::UnoInterfaceProxy * pThis,
    bridges::cpp_uno::shared::VtableSlot aVtableSlot,
    typelib_TypeDescriptionReference * pReturnTypeRef,
    sal_Int32 nParams, typelib_MethodParameter * pParams,
    void * pUnoReturn, void * pUnoArgs[], uno_Any ** ppUnoExc )
{
    
    sal_uInt32 * pStack = (sal_uInt32 *)__builtin_alloca(
        sizeof(sal_Int32) + ((nParams+2) * sizeof(sal_Int64)) );
    sal_uInt32 * pStackStart = pStack;

    sal_uInt32 pGPR[arm::MAX_GPR_REGS];
    sal_uInt32 nGPR = 0;

    
    double pFPR[arm::MAX_FPR_REGS];
#ifdef __ARM_PCS_VFP
    sal_uInt32 nDR = 0;
    float *pSPR = reinterpret_cast< float *>(&pFPR);
    sal_uInt32 nSR = 0;
#endif

    
    typelib_TypeDescription * pReturnTypeDescr = 0;
    TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );
    OSL_ENSURE( pReturnTypeDescr, "### expected return type description!" );

    void * pCppReturn = 0; 

    bool bSimpleReturn = true;
    if (pReturnTypeDescr)
    {
        if (arm::return_in_hidden_param( pReturnTypeRef ) )
            bSimpleReturn = false;

        if (bSimpleReturn)
            pCppReturn = pUnoReturn; 
        else
        {
            
            pCppReturn = (bridges::cpp_uno::shared::relatesToInterfaceType( pReturnTypeDescr )
                    ? __builtin_alloca( pReturnTypeDescr->nSize )
                    : pUnoReturn); 

            INSERT_INT32( &pCppReturn, nGPR, pGPR, pStack );
        }
    }
    
    void * pAdjustedThisPtr = reinterpret_cast< void ** >(pThis->getCppI())
        + aVtableSlot.offset;
    INSERT_INT32( &pAdjustedThisPtr, nGPR, pGPR, pStack );

    
    OSL_ENSURE( sizeof(void *) == sizeof(sal_Int32), "### unexpected size!" );
    
    void ** pCppArgs  = (void **)alloca( 3 * sizeof(void *) * nParams );
    
    sal_Int32 * pTempIndices = (sal_Int32 *)(pCppArgs + nParams);
    
    typelib_TypeDescription ** ppTempParamTypeDescr = (typelib_TypeDescription **)(pCppArgs + (2 * nParams));

    sal_Int32 nTempIndices   = 0;

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

                SAL_INFO( "bridges.ios", "hyper is " << pCppArgs[nPos] );

                INSERT_INT64( pCppArgs[nPos], nGPR, pGPR, pStack, pStackStart );
                break;
            case typelib_TypeClass_LONG:
            case typelib_TypeClass_UNSIGNED_LONG:
            case typelib_TypeClass_ENUM:

                SAL_INFO( "bridges.ios", "long is " << pCppArgs[nPos] );

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
                INSERT_FLOAT( pCppArgs[nPos], nGPR, pGPR, pStack );
                break;
            case typelib_TypeClass_DOUBLE:
                INSERT_DOUBLE( pCppArgs[nPos], nGPR, pGPR, pStack, pStackStart );
                break;
            default:
                break;
            }
            
            TYPELIB_DANGER_RELEASE( pParamTypeDescr );
        }
        else 
        {
            if (! rParam.bIn) 
            {
                
                uno_constructData(
                    pCppArgs[nPos] = alloca( pParamTypeDescr->nSize ),
                    pParamTypeDescr );
                pTempIndices[nTempIndices] = nPos; 
                
                ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
            }
            
            else if (bridges::cpp_uno::shared::relatesToInterfaceType( pParamTypeDescr ))
            {
                uno_copyAndConvertData(
                    pCppArgs[nPos] = alloca( pParamTypeDescr->nSize ),
                    pUnoArgs[nPos], pParamTypeDescr, pThis->getBridge()->getUno2Cpp() );

                pTempIndices[nTempIndices] = nPos; 
                
                ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
            }
            else 
            {
                pCppArgs[nPos] = pUnoArgs[nPos];
                
                TYPELIB_DANGER_RELEASE( pParamTypeDescr );
            }
            INSERT_INT32( &(pCppArgs[nPos]), nGPR, pGPR, pStack );
        }
    }

    try
    {
        callVirtualMethod(
            pAdjustedThisPtr, aVtableSlot.index,
            pCppReturn, pReturnTypeRef,
            pStackStart,
            (pStack - pStackStart),
            pGPR, nGPR,
            pFPR);

        
        *ppUnoExc = 0;

        
        for ( ; nTempIndices--; )
        {
            sal_Int32 nIndex = pTempIndices[nTempIndices];
            typelib_TypeDescription * pParamTypeDescr = ppTempParamTypeDescr[nTempIndices];

            if (pParams[nIndex].bIn)
            {
                if (pParams[nIndex].bOut) 
                {
                    uno_destructData( pUnoArgs[nIndex], pParamTypeDescr, 0 ); 
                    uno_copyAndConvertData( pUnoArgs[nIndex], pCppArgs[nIndex], pParamTypeDescr,
                                            pThis->getBridge()->getCpp2Uno() );
                }
            }
            else 
            {
                uno_copyAndConvertData( pUnoArgs[nIndex], pCppArgs[nIndex], pParamTypeDescr,
                                        pThis->getBridge()->getCpp2Uno() );
            }
            
            uno_destructData( pCppArgs[nIndex], pParamTypeDescr, cpp_release );

            TYPELIB_DANGER_RELEASE( pParamTypeDescr );
        }
        
        if (pCppReturn && pUnoReturn != pCppReturn)
        {
            uno_copyAndConvertData( pUnoReturn, pCppReturn, pReturnTypeDescr,
                                    pThis->getBridge()->getCpp2Uno() );
            uno_destructData( pCppReturn, pReturnTypeDescr, cpp_release );
        }
    }
    catch (...)
    {


        
        CPPU_CURRENT_NAMESPACE::fillUnoException( abi::__cxa_get_globals()->caughtExceptions, *ppUnoExc, pThis->getBridge()->getCpp2Uno() );

        
        for ( ; nTempIndices--; )
        {
            sal_Int32 nIndex = pTempIndices[nTempIndices];
            
            uno_destructData( pCppArgs[nIndex], ppTempParamTypeDescr[nTempIndices], cpp_release );
            TYPELIB_DANGER_RELEASE( ppTempParamTypeDescr[nTempIndices] );
        }

        
        if (pReturnTypeDescr)
            TYPELIB_DANGER_RELEASE( pReturnTypeDescr );
    }
}
}

namespace bridges { namespace cpp_uno { namespace shared {

void unoInterfaceProxyDispatch(
    uno_Interface * pUnoI, const typelib_TypeDescription * pMemberDescr,
    void * pReturn, void * pArgs[], uno_Any ** ppException )
{
    
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
        
        sal_Int32 nMemberPos = ((typelib_InterfaceMemberTypeDescription *)pMemberDescr)->nPosition;
        OSL_ENSURE( nMemberPos < pTypeDescr->nAllMembers, "### member pos out of range!" );
#endif

        VtableSlot aVtableSlot(
            getVtableSlot(
            reinterpret_cast<typelib_InterfaceAttributeTypeDescription const *>
              (pMemberDescr)));

        if (pReturn)
        {
            
            cpp_call(
                pThis, aVtableSlot,
                ((typelib_InterfaceAttributeTypeDescription *)pMemberDescr)->pAttributeTypeRef,
                0, 0, 
                pReturn, pArgs, ppException );
        }
        else
        {
            
            typelib_MethodParameter aParam;
            aParam.pTypeRef =
                ((typelib_InterfaceAttributeTypeDescription *)pMemberDescr)->pAttributeTypeRef;
            aParam.bIn      = sal_True;
            aParam.bOut     = sal_False;

            typelib_TypeDescriptionReference * pReturnTypeRef = 0;
            OUString aVoidName("void");
            typelib_typedescriptionreference_new(
                &pReturnTypeRef, typelib_TypeClass_VOID, aVoidName.pData );

            
            aVtableSlot.index += 1;
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
#if OSL_DEBUG_LEVEL > 0
        
        sal_Int32 nMemberPos = ((typelib_InterfaceMemberTypeDescription *)pMemberDescr)->nPosition;
        OSL_ENSURE( nMemberPos < pTypeDescr->nAllMembers, "### member pos out of range!" );
#endif

        VtableSlot aVtableSlot(
            getVtableSlot(
            reinterpret_cast<typelib_InterfaceMethodTypeDescription const *>
              (pMemberDescr)));

        switch (aVtableSlot.index)
        {
            
        case 1: 
            (*pUnoI->acquire)( pUnoI );
            *ppException = 0;
            break;
        case 2: 
            (*pUnoI->release)( pUnoI );
            *ppException = 0;
            break;
        case 0: 
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
        } 
        default:
            
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
            OUString("illegal member type description!"),
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >() );

        Type const & rExcType = ::getCppuType( &aExc );
        
        ::uno_type_any_construct( *ppException, &aExc, rExcType.getTypeLibType(), 0 );
    }
    }
}

} } }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
