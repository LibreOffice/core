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


#include <stdio.h>
#include <stdlib.h>
#include <boost/unordered_map.hpp>

#include <rtl/alloc.h>
#include <osl/mutex.hxx>

#include <com/sun/star/uno/genfunc.hxx>
#include "com/sun/star/uno/RuntimeException.hpp"
#include <uno/data.h>
#include <typelib/typedescription.hxx>

#include "bridges/cpp_uno/shared/bridge.hxx"
#include "bridges/cpp_uno/shared/cppinterfaceproxy.hxx"
#include "bridges/cpp_uno/shared/types.hxx"
#include "bridges/cpp_uno/shared/vtablefactory.hxx"

#include "abi.hxx"
#include "share.hxx"

using namespace ::osl;
using namespace ::rtl;
using namespace ::com::sun::star::uno;




//


//



//



static typelib_TypeClass cpp2uno_call(
    bridges::cpp_uno::shared::CppInterfaceProxy * pThis,
    const typelib_TypeDescription * pMemberTypeDescr,
    typelib_TypeDescriptionReference * pReturnTypeRef, 
    sal_Int32 nParams, typelib_MethodParameter * pParams,
    void ** gpreg, void ** fpreg, void ** ovrflw,
    sal_uInt64 * pRegisterReturn /* space for register return */ )
{
    unsigned int nr_gpr = 0; 
    unsigned int nr_fpr = 0; 

    
    typelib_TypeDescription * pReturnTypeDescr = 0;
    if (pReturnTypeRef)
        TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );

    void * pUnoReturn = 0;
    void * pCppReturn = 0; 

    if ( pReturnTypeDescr )
    {
        if ( x86_64::return_in_hidden_param( pReturnTypeRef ) )
        {
            pCppReturn = *gpreg++;
            nr_gpr++;

            pUnoReturn = ( bridges::cpp_uno::shared::relatesToInterfaceType( pReturnTypeDescr )
                           ? alloca( pReturnTypeDescr->nSize )
                           : pCppReturn ); 
        }
        else
            pUnoReturn = pRegisterReturn; 
    }

    
    gpreg++;
    nr_gpr++;

    
    
    void ** pUnoArgs = (void **)alloca( 4 * sizeof(void *) * nParams );
    void ** pCppArgs = pUnoArgs + nParams;
    
    sal_Int32 * pTempIndices = (sal_Int32 *)(pUnoArgs + (2 * nParams));
    
    typelib_TypeDescription ** ppTempParamTypeDescr = (typelib_TypeDescription **)(pUnoArgs + (3 * nParams));

    sal_Int32 nTempIndices = 0;

    for ( sal_Int32 nPos = 0; nPos < nParams; ++nPos )
    {
        const typelib_MethodParameter & rParam = pParams[nPos];

        int nUsedGPR = 0;
        int nUsedSSE = 0;
#if OSL_DEBUG_LEVEL > 0
        bool bFitsRegisters =
#endif
            x86_64::examine_argument( rParam.pTypeRef, false, nUsedGPR, nUsedSSE );
        if ( !rParam.bOut && bridges::cpp_uno::shared::isSimpleType( rParam.pTypeRef ) ) 
        {
            
            OSL_ASSERT( bFitsRegisters && ( ( nUsedSSE == 1 && nUsedGPR == 0 ) || ( nUsedSSE == 0 && nUsedGPR == 1 ) ) );

            if ( nUsedSSE == 1 )
            {
                if ( nr_fpr < x86_64::MAX_SSE_REGS )
                {
                    pCppArgs[nPos] = pUnoArgs[nPos] = fpreg++;
                    nr_fpr++;
                }
                else
                    pCppArgs[nPos] = pUnoArgs[nPos] = ovrflw++;
            }
            else if ( nUsedGPR == 1 )
            {
                if ( nr_gpr < x86_64::MAX_GPR_REGS )
                {
                    pCppArgs[nPos] = pUnoArgs[nPos] = gpreg++;
                    nr_gpr++;
                }
                else
                    pCppArgs[nPos] = pUnoArgs[nPos] = ovrflw++;
            }
        }
        else 
        {
            typelib_TypeDescription * pParamTypeDescr = 0;
            TYPELIB_DANGER_GET( &pParamTypeDescr, rParam.pTypeRef );

            void *pCppStack;
            if ( nr_gpr < x86_64::MAX_GPR_REGS )
            {
                pCppArgs[nPos] = pCppStack = *gpreg++;
                nr_gpr++;
            }
            else
                pCppArgs[nPos] = pCppStack = *ovrflw++;

            if (! rParam.bIn) 
            {
                
                pUnoArgs[nPos] = alloca( pParamTypeDescr->nSize );
                pTempIndices[nTempIndices] = nPos;
                
                ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
            }
            else if ( bridges::cpp_uno::shared::relatesToInterfaceType( pParamTypeDescr ) ) 
            {
                uno_copyAndConvertData( pUnoArgs[nPos] = alloca( pParamTypeDescr->nSize ),
                                        pCppStack, pParamTypeDescr,
                                        pThis->getBridge()->getCpp2Uno() );
                pTempIndices[nTempIndices] = nPos; 
                
                ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
            }
            else 
            {
                pUnoArgs[nPos] = pCppStack;
                
                TYPELIB_DANGER_RELEASE( pParamTypeDescr );
            }
        }
    }

    
    uno_Any aUnoExc; 
    uno_Any * pUnoExc = &aUnoExc;

    
    (*pThis->getUnoI()->pDispatcher)( pThis->getUnoI(), pMemberTypeDescr, pUnoReturn, pUnoArgs, &pUnoExc );

    
    if ( pUnoExc )
    {
        
        for ( ; nTempIndices--; )
        {
            sal_Int32 nIndex = pTempIndices[nTempIndices];

            if (pParams[nIndex].bIn) 
                uno_destructData( pUnoArgs[nIndex], ppTempParamTypeDescr[nTempIndices], 0 );
            TYPELIB_DANGER_RELEASE( ppTempParamTypeDescr[nTempIndices] );
        }
        if (pReturnTypeDescr)
            TYPELIB_DANGER_RELEASE( pReturnTypeDescr );

        CPPU_CURRENT_NAMESPACE::raiseException( &aUnoExc, pThis->getBridge()->getUno2Cpp() ); 
        
        return typelib_TypeClass_VOID;
    }
    else 
    {
        
        for ( ; nTempIndices--; )
        {
            sal_Int32 nIndex = pTempIndices[nTempIndices];
            typelib_TypeDescription * pParamTypeDescr = ppTempParamTypeDescr[nTempIndices];

            if ( pParams[nIndex].bOut ) 
            {
                
                uno_destructData( pCppArgs[nIndex], pParamTypeDescr, cpp_release );
                uno_copyAndConvertData( pCppArgs[nIndex], pUnoArgs[nIndex], pParamTypeDescr,
                                        pThis->getBridge()->getUno2Cpp() );
            }
            
            uno_destructData( pUnoArgs[nIndex], pParamTypeDescr, 0 );

            TYPELIB_DANGER_RELEASE( pParamTypeDescr );
        }
        
        if ( pCppReturn ) 
        {
            if ( pUnoReturn != pCppReturn ) 
            {
                uno_copyAndConvertData( pCppReturn, pUnoReturn, pReturnTypeDescr,
                                        pThis->getBridge()->getUno2Cpp() );
                
                uno_destructData( pUnoReturn, pReturnTypeDescr, 0 );
            }
            
            *(void **)pRegisterReturn = pCppReturn;
        }
        if ( pReturnTypeDescr )
        {
            typelib_TypeClass eRet = (typelib_TypeClass)pReturnTypeDescr->eTypeClass;
            TYPELIB_DANGER_RELEASE( pReturnTypeDescr );
            return eRet;
        }
        else
            return typelib_TypeClass_VOID;
    }
}



extern "C" typelib_TypeClass cpp_vtable_call(
    sal_Int32 nFunctionIndex, sal_Int32 nVtableOffset,
    void ** gpreg, void ** fpreg, void ** ovrflw,
    sal_uInt64 * pRegisterReturn /* space for register return */ )
{
    
    
    
    void * pThis;
    if ( nFunctionIndex & 0x80000000 )
    {
        nFunctionIndex &= 0x7fffffff;
        pThis = gpreg[1];
    }
    else
    {
        pThis = gpreg[0];
    }
    pThis = static_cast<char *>( pThis ) - nVtableOffset;

    bridges::cpp_uno::shared::CppInterfaceProxy * pCppI =
        bridges::cpp_uno::shared::CppInterfaceProxy::castInterfaceToProxy( pThis );

    typelib_InterfaceTypeDescription * pTypeDescr = pCppI->getTypeDescr();

    OSL_ENSURE( nFunctionIndex < pTypeDescr->nMapFunctionIndexToMemberIndex, "### illegal vtable index!\n" );
    if ( nFunctionIndex >= pTypeDescr->nMapFunctionIndexToMemberIndex )
    {
        throw RuntimeException("illegal vtable index!",
                                reinterpret_cast<XInterface *>( pCppI ) );
    }

    
    sal_Int32 nMemberPos = pTypeDescr->pMapFunctionIndexToMemberIndex[nFunctionIndex];
    OSL_ENSURE( nMemberPos < pTypeDescr->nAllMembers, "### illegal member index!\n" );

    TypeDescription aMemberDescr( pTypeDescr->ppAllMembers[nMemberPos] );

    typelib_TypeClass eRet;
    switch ( aMemberDescr.get()->eTypeClass )
    {
        case typelib_TypeClass_INTERFACE_ATTRIBUTE:
        {
            typelib_TypeDescriptionReference *pAttrTypeRef =
                reinterpret_cast<typelib_InterfaceAttributeTypeDescription *>( aMemberDescr.get() )->pAttributeTypeRef;

            if ( pTypeDescr->pMapMemberIndexToFunctionIndex[nMemberPos] == nFunctionIndex )
            {
                
                eRet = cpp2uno_call( pCppI, aMemberDescr.get(), pAttrTypeRef,
                        0, 0, 
                        gpreg, fpreg, ovrflw, pRegisterReturn );
            }
            else
            {
                
                typelib_MethodParameter aParam;
                aParam.pTypeRef = pAttrTypeRef;
                aParam.bIn      = sal_True;
                aParam.bOut     = sal_False;

                eRet = cpp2uno_call( pCppI, aMemberDescr.get(),
                        0, 
                        1, &aParam,
                        gpreg, fpreg, ovrflw, pRegisterReturn );
            }
            break;
        }
        case typelib_TypeClass_INTERFACE_METHOD:
        {
            
            switch ( nFunctionIndex )
            {
                case 1: 
                    pCppI->acquireProxy(); 
                    eRet = typelib_TypeClass_VOID;
                    break;
                case 2: 
                    pCppI->releaseProxy(); 
                    eRet = typelib_TypeClass_VOID;
                    break;
                case 0: 
                {
                    typelib_TypeDescription * pTD = 0;
                    TYPELIB_DANGER_GET( &pTD, reinterpret_cast<Type *>( gpreg[2] )->getTypeLibType() );
                    if ( pTD )
                    {
                        XInterface * pInterface = 0;
                        (*pCppI->getBridge()->getCppEnv()->getRegisteredInterface)
                            ( pCppI->getBridge()->getCppEnv(),
                              (void **)&pInterface,
                              pCppI->getOid().pData,
                              reinterpret_cast<typelib_InterfaceTypeDescription *>( pTD ) );

                        if ( pInterface )
                        {
                            ::uno_any_construct( reinterpret_cast<uno_Any *>( gpreg[0] ),
                                                 &pInterface, pTD, cpp_acquire );

                            pInterface->release();
                            TYPELIB_DANGER_RELEASE( pTD );

                            reinterpret_cast<void **>( pRegisterReturn )[0] = gpreg[0];
                            eRet = typelib_TypeClass_ANY;
                            break;
                        }
                        TYPELIB_DANGER_RELEASE( pTD );
                    }
                } 
                default:
                {
                    typelib_InterfaceMethodTypeDescription *pMethodTD =
                        reinterpret_cast<typelib_InterfaceMethodTypeDescription *>( aMemberDescr.get() );

                    eRet = cpp2uno_call( pCppI, aMemberDescr.get(),
                                         pMethodTD->pReturnTypeRef,
                                         pMethodTD->nParams,
                                         pMethodTD->pParams,
                                         gpreg, fpreg, ovrflw, pRegisterReturn );
                }
            }
            break;
        }
        default:
        {
            throw RuntimeException("no member description found!",
                                    reinterpret_cast<XInterface *>( pCppI ) );
        }
    }

    return eRet;
}


extern "C" void privateSnippetExecutor( ... );

const int codeSnippetSize = 24;



//



//



unsigned char * codeSnippet( unsigned char * code,
        sal_Int32 nFunctionIndex, sal_Int32 nVtableOffset,
        bool bHasHiddenParam ) SAL_THROW(())
{
    sal_uInt64 nOffsetAndIndex = ( ( (sal_uInt64) nVtableOffset ) << 32 ) | ( (sal_uInt64) nFunctionIndex );

    if ( bHasHiddenParam )
        nOffsetAndIndex |= 0x80000000;

    
    *reinterpret_cast<sal_uInt16 *>( code ) = 0xba49;
    *reinterpret_cast<sal_uInt64 *>( code + 2 ) = nOffsetAndIndex;

    
    *reinterpret_cast<sal_uInt16 *>( code + 10 ) = 0xbb49;
    *reinterpret_cast<sal_uInt64 *>( code + 12 ) = reinterpret_cast<sal_uInt64>( privateSnippetExecutor );

    
    *reinterpret_cast<sal_uInt32 *>( code + 20 ) = 0x00e3ff49;

#if OSL_DEBUG_LEVEL > 1
    fprintf(stderr,
            "==> codeSnippet, functionIndex=%d%s, vtableOffset=%d\n",
            nFunctionIndex, (bHasHiddenParam ? "|0x80000000":""), nVtableOffset);
#endif

    return code + codeSnippetSize;
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
    void * block, sal_Int32 slotCount)
{
    Slot * slots = mapBlockToVtable(block);
    slots[-2].fn = 0;
    slots[-1].fn = 0;
    return slots + slotCount;
}



unsigned char * bridges::cpp_uno::shared::VtableFactory::addLocalFunctions(
    Slot ** slots, unsigned char * code, typelib_InterfaceTypeDescription const * type,
    sal_Int32 nFunctionOffset, sal_Int32 functionCount, sal_Int32 nVtableOffset )
{
    (*slots) -= functionCount;
    Slot * s = *slots;
    for ( sal_Int32 nPos = 0; nPos < type->nMembers; ++nPos )
    {
        typelib_TypeDescription * pTD = 0;

        TYPELIB_DANGER_GET( &pTD, type->ppMembers[ nPos ] );
        OSL_ASSERT( pTD );

        if ( typelib_TypeClass_INTERFACE_ATTRIBUTE == pTD->eTypeClass )
        {
            typelib_InterfaceAttributeTypeDescription *pAttrTD =
                reinterpret_cast<typelib_InterfaceAttributeTypeDescription *>( pTD );

            
            (s++)->fn = code;
            code = codeSnippet( code, nFunctionOffset++, nVtableOffset,
                                x86_64::return_in_hidden_param( pAttrTD->pAttributeTypeRef ) );

            if ( ! pAttrTD->bReadOnly )
            {
                
                (s++)->fn = code;
                code = codeSnippet( code, nFunctionOffset++, nVtableOffset, false );
            }
        }
        else if ( typelib_TypeClass_INTERFACE_METHOD == pTD->eTypeClass )
        {
            typelib_InterfaceMethodTypeDescription *pMethodTD =
                reinterpret_cast<typelib_InterfaceMethodTypeDescription *>( pTD );

            (s++)->fn = code;
            code = codeSnippet( code, nFunctionOffset++, nVtableOffset,
                                x86_64::return_in_hidden_param( pMethodTD->pReturnTypeRef ) );
        }
        else
            OSL_ASSERT( false );

        TYPELIB_DANGER_RELEASE( pTD );
    }
    return code;
}


void bridges::cpp_uno::shared::VtableFactory::flushCode(
    SAL_UNUSED_PARAMETER unsigned char const *,
    SAL_UNUSED_PARAMETER unsigned char const * )
{}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
