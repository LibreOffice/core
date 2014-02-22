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


#include <malloc.h>

#include <com/sun/star/uno/genfunc.hxx>
#include <uno/data.h>

#include "bridges/cpp_uno/shared/bridge.hxx"
#include "bridges/cpp_uno/shared/types.hxx"
#include "bridges/cpp_uno/shared/unointerfaceproxy.hxx"
#include "bridges/cpp_uno/shared/vtables.hxx"

#include "mscx.hxx"

#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif

using namespace ::rtl;
using namespace ::com::sun::star::uno;

namespace
{

static bool cpp_call(
    bridges::cpp_uno::shared::UnoInterfaceProxy * pThis,
    bridges::cpp_uno::shared::VtableSlot aVtableSlot,
    typelib_TypeDescriptionReference * pReturnTypeRef,
    sal_Int32 nParams,
    typelib_MethodParameter * pParams,
    void * pUnoReturn,
    void * pUnoArgs[],
    uno_Any ** ppUnoExc ) throw ()
{
    const int MAXPARAMS = 20;

    if ( nParams > MAXPARAMS )
    {
        
        
        

        return false;
    }

    
    union {
        sal_Int64 i;
        void *p;
        double d;
    } aCppParams[MAXPARAMS+2], uRetVal;
    int nCppParamIndex = 0;

    
    typelib_TypeDescription * pReturnTD = NULL;
    TYPELIB_DANGER_GET( &pReturnTD, pReturnTypeRef );
    OSL_ENSURE( pReturnTD, "### expected return type description!" );

    
    void * pAdjustedThisPtr = (void **)( pThis->getCppI() ) + aVtableSlot.offset;
    aCppParams[nCppParamIndex++].p = pAdjustedThisPtr;

    bool bSimpleReturn = true;
    if ( pReturnTD )
    {
        if ( !bridges::cpp_uno::shared::isSimpleType( pReturnTD ) )
        {
            
            bSimpleReturn = false;
            aCppParams[nCppParamIndex++].p =
                bridges::cpp_uno::shared::relatesToInterfaceType( pReturnTD )?
                         alloca( pReturnTD->nSize ) : pUnoReturn;
        }
    }

    
    int pTempCppIndexes[MAXPARAMS];
    int pTempIndexes[MAXPARAMS];
    int nTempIndexes = 0;

    
    typelib_TypeDescription *pTempParamTypeDescr[MAXPARAMS];

    for ( int nPos = 0; nPos < nParams; ++nPos, ++nCppParamIndex )
    {
        const typelib_MethodParameter & rParam = pParams[nPos];

        typelib_TypeDescription * pParamTD = NULL;
        TYPELIB_DANGER_GET( &pParamTD, rParam.pTypeRef );

        if ( !rParam.bOut &&
             bridges::cpp_uno::shared::isSimpleType( pParamTD ) )
        {
            ::uno_copyAndConvertData(
                &aCppParams[nCppParamIndex], pUnoArgs[nPos], pParamTD,
                pThis->getBridge()->getUno2Cpp() );

            
            TYPELIB_DANGER_RELEASE( pParamTD );
        }
        else 
        {
            if ( !rParam.bIn ) 
            {
                
                ::uno_constructData(
                    aCppParams[nCppParamIndex].p = alloca( pParamTD->nSize ),
                    pParamTD );

                pTempCppIndexes[nTempIndexes] = nCppParamIndex;
                pTempIndexes[nTempIndexes] = nPos;

                
                pTempParamTypeDescr[nTempIndexes++] = pParamTD;

            }
            
            else if ( bridges::cpp_uno::shared::relatesToInterfaceType( pParamTD ) )
            {
                ::uno_copyAndConvertData(
                    aCppParams[nCppParamIndex].p = alloca( pParamTD->nSize ),
                    pUnoArgs[nPos], pParamTD,
                    pThis->getBridge()->getUno2Cpp() );

                pTempCppIndexes[nTempIndexes] = nCppParamIndex;
                pTempIndexes[nTempIndexes] = nPos;

                
                pTempParamTypeDescr[nTempIndexes++] = pParamTD;
            }
            else 
            {
                aCppParams[nCppParamIndex].p = pUnoArgs[nPos];

                
                TYPELIB_DANGER_RELEASE( pParamTD );
            }
        }
    }

    __try
    {
        

        
        
        
        
        
        
        

        
        
        
        
        
        
        
        
        

        sal_Int64 (*pIMethod)(sal_Int64, ...) =
            (sal_Int64 (*)(sal_Int64, ...))
            (*((sal_uInt64 **)pAdjustedThisPtr))[aVtableSlot.index];

        double (*pFMethod)(sal_Int64, ...) =
            (double (*)(sal_Int64, ...))
            (*((sal_uInt64 **)pAdjustedThisPtr))[aVtableSlot.index];

        
        
        
        

        if ( pReturnTD &&
             (pReturnTD->eTypeClass == typelib_TypeClass_FLOAT ||
              pReturnTD->eTypeClass == typelib_TypeClass_DOUBLE) )
            uRetVal.d =
                pFMethod (aCppParams[0].i, aCppParams[1].d, aCppParams[2].d, aCppParams[3].d,
                          aCppParams[4].i, aCppParams[5].i, aCppParams[6].i, aCppParams[7].i,
                          aCppParams[8].i, aCppParams[9].i, aCppParams[10].i, aCppParams[11].i,
                          aCppParams[12].i, aCppParams[13].i, aCppParams[14].i, aCppParams[15].i,
                          aCppParams[16].i, aCppParams[17].i, aCppParams[18].i, aCppParams[19].i );
        else
            uRetVal.i =
                pIMethod (aCppParams[0].i, aCppParams[1].d, aCppParams[2].d, aCppParams[3].d,
                          aCppParams[4].i, aCppParams[5].i, aCppParams[6].i, aCppParams[7].i,
                          aCppParams[8].i, aCppParams[9].i, aCppParams[10].i, aCppParams[11].i,
                          aCppParams[12].i, aCppParams[13].i, aCppParams[14].i, aCppParams[15].i,
                          aCppParams[16].i, aCppParams[17].i, aCppParams[18].i, aCppParams[19].i );
    }
    __except (CPPU_CURRENT_NAMESPACE::mscx_filterCppException(
                  GetExceptionInformation(),
                  *ppUnoExc, pThis->getBridge()->getCpp2Uno() ))
   {
        
        
        while ( nTempIndexes-- )
        {
            int nCppIndex = pTempCppIndexes[nTempIndexes];
            
            ::uno_destructData(
                aCppParams[nCppIndex].p, pTempParamTypeDescr[nTempIndexes],
                cpp_release );
            TYPELIB_DANGER_RELEASE( pTempParamTypeDescr[nTempIndexes] );
        }
        
        if ( pReturnTD )
            TYPELIB_DANGER_RELEASE( pReturnTD );

        
        return true;
    }

    
    *ppUnoExc = NULL;

    
    while ( nTempIndexes-- )
    {
        int nCppIndex = pTempCppIndexes[nTempIndexes];
        int nIndex = pTempIndexes[nTempIndexes];
        typelib_TypeDescription * pParamTD =
            pTempParamTypeDescr[nTempIndexes];

        if ( pParams[nIndex].bIn )
        {
            if ( pParams[nIndex].bOut ) 
            {
                ::uno_destructData(
                    pUnoArgs[nIndex], pParamTD, 0 ); 
                ::uno_copyAndConvertData(
                    pUnoArgs[nIndex], aCppParams[nCppIndex].p, pParamTD,
                    pThis->getBridge()->getCpp2Uno() );
            }
        }
        else 
        {
            ::uno_copyAndConvertData(
                pUnoArgs[nIndex], aCppParams[nCppIndex].p, pParamTD,
                pThis->getBridge()->getCpp2Uno() );
        }

        
        ::uno_destructData(
            aCppParams[nCppIndex].p, pParamTD, cpp_release );

        TYPELIB_DANGER_RELEASE( pParamTD );
    }

    
    if ( !bSimpleReturn )
    {
        ::uno_copyAndConvertData(
            pUnoReturn, uRetVal.p, pReturnTD,
            pThis->getBridge()->getCpp2Uno() );
        ::uno_destructData(
            aCppParams[1].p, pReturnTD, cpp_release );
    }
    else if ( pUnoReturn )
        *(sal_Int64*)pUnoReturn = uRetVal.i;

    if ( pReturnTD )
        TYPELIB_DANGER_RELEASE( pReturnTD );

    return true;
}

}

namespace bridges { namespace cpp_uno { namespace shared {

void unoInterfaceProxyDispatch(
    uno_Interface * pUnoI,
    const typelib_TypeDescription * pMemberTD,
    void * pReturn,
    void * pArgs[],
    uno_Any ** ppException )
{
    
    bridges::cpp_uno::shared::UnoInterfaceProxy * pThis
        = static_cast< bridges::cpp_uno::shared::UnoInterfaceProxy * >(pUnoI);
#if OSL_DEBUG_LEVEL > 0
    typelib_InterfaceTypeDescription * pTypeDescr = pThis->pTypeDescr;
#endif

    switch (pMemberTD->eTypeClass)
    {
    case typelib_TypeClass_INTERFACE_ATTRIBUTE:
    {
#if OSL_DEBUG_LEVEL > 0
        
        sal_Int32 nMemberPos = ((typelib_InterfaceMemberTypeDescription *)pMemberTD)->nPosition;
        OSL_ENSURE( nMemberPos < pTypeDescr->nAllMembers, "### member pos out of range!" );
#endif
        VtableSlot aVtableSlot(
            getVtableSlot(
                reinterpret_cast<
                    typelib_InterfaceAttributeTypeDescription const * >(
                        pMemberTD)));
        if ( pReturn )
        {
            
            cpp_call(
                pThis, aVtableSlot,
                ((typelib_InterfaceAttributeTypeDescription *)pMemberTD)->pAttributeTypeRef,
                0, NULL, 
                pReturn, pArgs, ppException );
        }
        else
        {
            
            typelib_MethodParameter aParam;
            aParam.pTypeRef =
                ((typelib_InterfaceAttributeTypeDescription *)pMemberTD)->pAttributeTypeRef;
            aParam.bIn      = sal_True;
            aParam.bOut     = sal_False;

            typelib_TypeDescriptionReference * pReturnTypeRef = NULL;
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
        
        sal_Int32 nMemberPos = ((typelib_InterfaceMemberTypeDescription *)pMemberTD)->nPosition;
        OSL_ENSURE( nMemberPos < pTypeDescr->nAllMembers, "### member pos out of range!" );
#endif
        VtableSlot aVtableSlot(
            getVtableSlot(
                reinterpret_cast<
                    typelib_InterfaceMethodTypeDescription const * >(
                        pMemberTD)));

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
            typelib_TypeDescription * pTD = NULL;
            TYPELIB_DANGER_GET( &pTD, reinterpret_cast< Type * >( pArgs[0] )->getTypeLibType() );

            if ( pTD )
            {
                uno_Interface * pInterface = NULL;
                (*pThis->getBridge()->getUnoEnv()->getRegisteredInterface)(
                    pThis->getBridge()->getUnoEnv(),
                    (void **)&pInterface, pThis->oid.pData, (typelib_InterfaceTypeDescription *)pTD );

                if ( pInterface )
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
            if ( ! cpp_call(
                     pThis, aVtableSlot,
                     ((typelib_InterfaceMethodTypeDescription *)pMemberTD)->pReturnTypeRef,
                     ((typelib_InterfaceMethodTypeDescription *)pMemberTD)->nParams,
                     ((typelib_InterfaceMethodTypeDescription *)pMemberTD)->pParams,
                     pReturn, pArgs, ppException ) )
            {
                RuntimeException aExc(
                    OUString("Too many parameters!"),
                    Reference< XInterface >() );

                Type const & rExcType = ::getCppuType( &aExc );
                ::uno_type_any_construct( *ppException, &aExc, rExcType.getTypeLibType(), 0 );
            }
        }
        break;
    }
    default:
    {
        RuntimeException aExc(
            OUString("Illegal member type description!"),
            Reference< XInterface >() );

        Type const & rExcType = ::getCppuType( &aExc );
        
        ::uno_type_any_construct( *ppException, &aExc, rExcType.getTypeLibType(), 0 );
    }
    }
}

} } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
