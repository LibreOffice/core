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

#include <sys/types.h>
#include <sys/malloc.h>

#include <com/sun/star/uno/genfunc.hxx>
#include <uno/data.h>

#include "bridges/cpp_uno/shared/bridge.hxx"
#include "bridges/cpp_uno/shared/types.hxx"
#include "bridges/cpp_uno/shared/unointerfaceproxy.hxx"
#include "bridges/cpp_uno/shared/vtables.hxx"

#include "share.hxx"
#include <stdio.h>
#include <string.h>

using namespace ::rtl;
using namespace ::com::sun::star::uno;

namespace
{

void MapReturn(sal_uInt32 r3, sal_uInt32 r4, double dret, typelib_TypeClass eReturnType, void *pRegisterReturn)
{
    switch( eReturnType )
        {
                case typelib_TypeClass_HYPER:
                case typelib_TypeClass_UNSIGNED_HYPER:
                        ((long*)pRegisterReturn)[1] = r4;
            
                case typelib_TypeClass_LONG:
                case typelib_TypeClass_UNSIGNED_LONG:
                case typelib_TypeClass_ENUM:
                        ((long*)pRegisterReturn)[0] = r3;
                        break;
                case typelib_TypeClass_CHAR:
                case typelib_TypeClass_SHORT:
                case typelib_TypeClass_UNSIGNED_SHORT:
                    *(unsigned short*)pRegisterReturn = (unsigned short)r3;
                        break;
                case typelib_TypeClass_BOOLEAN:
                case typelib_TypeClass_BYTE:
                    *(unsigned char*)pRegisterReturn = (unsigned char)r3;
                        break;
                case typelib_TypeClass_FLOAT:
                    *(float*)pRegisterReturn = (float)dret;
                        break;
                case typelib_TypeClass_DOUBLE:
                        *(double*)pRegisterReturn = dret;
                        break;
                default:
                        break;
        }
}

#define DISPLACEMENT -2

static void callVirtualMethod(
    void * pThis,
    sal_uInt32 nVtableIndex,
    void * pRegisterReturn,
    typelib_TypeClass eReturnType,
    sal_uInt32 * pStack,
    sal_uInt32 nStack,
    double *pFPR,
    sal_uInt32 nFPR)
{
    sal_uInt32 nStackWords = nStack;
    if (nStackWords < ppc::MAX_GPR_REGS)
        nStackWords = 0;
    else
        nStackWords-=ppc::MAX_GPR_REGS;
    if (nStackWords)
        nStackWords = ( nStackWords + 1) & ~1;
    sal_uInt32 *stack = (sal_uInt32*)__builtin_alloca( nStackWords * sizeof(sal_uInt32) );
    memcpy(stack+DISPLACEMENT, pStack+ppc::MAX_GPR_REGS, nStack * sizeof(sal_uInt32));

    
    sal_uInt32 pMethod = *((sal_uInt32 *)pThis);
    pMethod += 4 * nVtableIndex;
    pMethod = *((sal_uInt32 *)pMethod);

    typedef void (* FunctionCall )( sal_uInt32, sal_uInt32, sal_uInt32, sal_uInt32, sal_uInt32, sal_uInt32, sal_uInt32, sal_uInt32 );
    FunctionCall pFunc = (FunctionCall)pMethod;

    register double d0 asm("fr1"); d0 = pFPR[0];
    register double d1 asm("fr2"); d1 = pFPR[1];
    register double d2 asm("fr3"); d2 = pFPR[2];
    register double d3 asm("fr4"); d3 = pFPR[3];
    register double d4 asm("fr5"); d4 = pFPR[4];
    register double d5 asm("fr6"); d5 = pFPR[5];
    register double d6 asm("fr7"); d6 = pFPR[6];
    register double d7 asm("fr8"); d7 = pFPR[7];
    register double d8 asm("fr9"); d8 = pFPR[8];
    register double d9 asm("fr10"); d9 = pFPR[9];
    register double d10 asm("fr11"); d10 = pFPR[10];
    register double d11 asm("fr12"); d11 = pFPR[11];
    register double d12 asm("fr13"); d12 = pFPR[12];

    (*pFunc)(pStack[0], pStack[1], pStack[2], pStack[3], pStack[4], pStack[5], pStack[6], pStack[7]);

    register sal_uInt32 r3 asm("r3");
    register sal_uInt32 r4 asm("r4");
    MapReturn(r3, r4, d0, eReturnType, pRegisterReturn);
}

#define INSERT_INT32(pSV, pDS) \
{ \
    *pDS++ = *reinterpret_cast<sal_uInt32 *>(pSV); \
}

#define INSERT_INT16(pSV, pDS) \
{ \
    *pDS++ = *reinterpret_cast<sal_uInt16 *>(pSV); \
}

#define INSERT_INT8(pSV, pDS) \
{ \
    *pDS++ = *reinterpret_cast<sal_uInt8 *>(pSV); \
}

#define INSERT_FLOAT(pSV, nr, pFPR, pDS) \
{ \
    if (nr < ppc::MAX_SSE_REGS) \
    { \
        sal_uInt32 *pDouble = (sal_uInt32 *)&(pFPR[nr++]); \
        pDouble[0] = *reinterpret_cast<sal_uInt32 *>(pSV); \
    } \
    *pDS++ = *reinterpret_cast<sal_uInt32 *>(pSV); \
}

#define INSERT_DOUBLE(pSV, nr, pFPR, pDS) \
{ \
    if (nr < ppc::MAX_SSE_REGS) \
    { \
        pFPR[nr++] = *reinterpret_cast<double *>(pSV); \
    } \
    *pDS++ = reinterpret_cast<sal_uInt32 *>(pSV)[1]; \
    *pDS++ = reinterpret_cast<sal_uInt32 *>(pSV)[0]; \
}

#define INSERT_INT64(pSV, pDS) \
{ \
    INSERT_INT32(pSV, pDS) \
    INSERT_INT32(((sal_uInt32*)pSV)+1, pDS) \
}

static void cpp_call(
        bridges::cpp_uno::shared::UnoInterfaceProxy * pThis,
        bridges::cpp_uno::shared::VtableSlot aVtableSlot,
        typelib_TypeDescriptionReference * pReturnTypeRef,
        sal_Int32 nParams, typelib_MethodParameter * pParams,
        void * pUnoReturn, void * pUnoArgs[], uno_Any ** ppUnoExc )
{
    
    sal_uInt32 * pStack = (sal_uInt32*)__builtin_alloca( sizeof(sal_Int32) + ((nParams+2) * sizeof(sal_Int64)) );
    sal_uInt32 * pStackStart = pStack;

    double pFPR[ppc::MAX_SSE_REGS];
    sal_uInt32 nFPR = 0;

    
    typelib_TypeDescription * pReturnTypeDescr = 0;
    TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );
    OSL_ENSURE( pReturnTypeDescr, "### expected return type description!" );

    void * pCppReturn = 0; 

    if (pReturnTypeDescr)
    {
        if (bridges::cpp_uno::shared::isSimpleType( pReturnTypeDescr ))
        {
            pCppReturn = pUnoReturn; 
        }
        else
        {
                        
                        pCppReturn = (bridges::cpp_uno::shared::relatesToInterfaceType( pReturnTypeDescr )
                           ? __builtin_alloca( pReturnTypeDescr->nSize )
                           : pUnoReturn); 
                        INSERT_INT32(&pCppReturn, pStack);
                }
        }
        
    void * pAdjustedThisPtr = reinterpret_cast< void ** >(pThis->getCppI()) + aVtableSlot.offset;
        INSERT_INT32(&pAdjustedThisPtr, pStack);

        
        OSL_ENSURE( sizeof(void *) == sizeof(sal_Int32), "### unexpected size!" );
        
        void ** pCppArgs  = (void **)__builtin_alloca( 3 * sizeof(void *) * nParams );
        
        sal_Int32 * pTempIndices = (sal_Int32 *)(pCppArgs + nParams);
        
        typelib_TypeDescription ** ppTempParamTypeDescr = (typelib_TypeDescription **)(pCppArgs + (2 * nParams));

        sal_Int32 nTempIndices   = 0;

        for ( sal_Int32 nPos = 0; nPos < nParams; ++nPos )
        {
                const typelib_MethodParameter & rParam = pParams[nPos];
                typelib_TypeDescription * pParamTypeDescr = 0;
                TYPELIB_DANGER_GET( &pParamTypeDescr, rParam.pTypeRef );

                if (!rParam.bOut
                    && bridges::cpp_uno::shared::isSimpleType( pParamTypeDescr ))
                {
                        uno_copyAndConvertData( pCppArgs[nPos] = pStack, pUnoArgs[nPos], pParamTypeDescr,
                                                                        pThis->getBridge()->getUno2Cpp() );

                        switch (pParamTypeDescr->eTypeClass)
                        {
                        case typelib_TypeClass_LONG:
                        case typelib_TypeClass_UNSIGNED_LONG:
                        case typelib_TypeClass_ENUM:
                                INSERT_INT32(pCppArgs[nPos], pStack);
                                break;
                        case typelib_TypeClass_SHORT:
                        case typelib_TypeClass_CHAR:
                        case typelib_TypeClass_UNSIGNED_SHORT:
                    INSERT_INT16(pCppArgs[nPos], pStack);
                    break;
                        case typelib_TypeClass_BOOLEAN:
                        case typelib_TypeClass_BYTE:
                    INSERT_INT8(pCppArgs[nPos], pStack);
                    break;
                        case typelib_TypeClass_FLOAT:
                    INSERT_FLOAT(pCppArgs[nPos], nFPR, pFPR, pStack);
                                break;
                        case typelib_TypeClass_DOUBLE:
                                INSERT_DOUBLE(pCppArgs[nPos], nFPR, pFPR, pStack);
                                break;
                            case typelib_TypeClass_HYPER:
                            case typelib_TypeClass_UNSIGNED_HYPER:
                                INSERT_INT64(pCppArgs[nPos], pStack);
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
                                        pCppArgs[nPos] = __builtin_alloca( pParamTypeDescr->nSize ),
                                        pParamTypeDescr );
                                pTempIndices[nTempIndices] = nPos; 
                                
                                ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
                        }
                        
                        else if (bridges::cpp_uno::shared::relatesToInterfaceType( pParamTypeDescr ))
                        {
                                uno_copyAndConvertData(
                                        pCppArgs[nPos] = __builtin_alloca( pParamTypeDescr->nSize ),
                                        pUnoArgs[nPos], pParamTypeDescr, pThis->getBridge()->getUno2Cpp() );

                                pTempIndices[nTempIndices] = nPos; 
                                
                                ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
                        }
                        else 
                        {
                                pCppArgs[nPos] = pUnoArgs[nPos];
                                
                                TYPELIB_DANGER_RELEASE( pParamTypeDescr );
                        }
                        INSERT_INT32(&(pCppArgs[nPos]), pStack);
                }
        }

        try
        {
                OSL_ENSURE( !( (pCppStack - pCppStackStart ) & 3), "UNALIGNED STACK !!! (Please DO panic)" );
                callVirtualMethod(
                        pAdjustedThisPtr, aVtableSlot.index,
                        pCppReturn, pReturnTypeDescr->eTypeClass,
                        pStackStart, (pStack - pStackStart), pFPR, nFPR );
                
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
                
                fillUnoException( CPPU_CURRENT_NAMESPACE::__cxa_get_globals()->caughtExceptions, *ppUnoExc, pThis->getBridge()->getCpp2Uno() );

                
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
           = static_cast< bridges::cpp_uno::shared::UnoInterfaceProxy * > (pUnoI);
        

        switch (pMemberDescr->eTypeClass)
        {
        case typelib_TypeClass_INTERFACE_ATTRIBUTE:
        {

        VtableSlot aVtableSlot(
            getVtableSlot(
                reinterpret_cast<
                    typelib_InterfaceAttributeTypeDescription const * >(
                        pMemberDescr)));

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
                        aParam.bIn              = sal_True;
                        aParam.bOut             = sal_False;

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

        VtableSlot aVtableSlot(
            getVtableSlot(
                reinterpret_cast<
                    typelib_InterfaceMethodTypeDescription const * >(
                        pMemberDescr)));
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
                (*pThis->pBridge->getUnoEnv()->getRegisteredInterface)(
                    pThis->pBridge->getUnoEnv(),
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
