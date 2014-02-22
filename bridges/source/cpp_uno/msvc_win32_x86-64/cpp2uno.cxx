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
#include <typelib/typedescription.hxx>

#include "bridges/cpp_uno/shared/bridge.hxx"
#include "bridges/cpp_uno/shared/cppinterfaceproxy.hxx"
#include "bridges/cpp_uno/shared/types.hxx"
#include "bridges/cpp_uno/shared/vtablefactory.hxx"

#include "mscx.hxx"

using namespace ::com::sun::star::uno;

static inline typelib_TypeClass cpp2uno_call(
    bridges::cpp_uno::shared::CppInterfaceProxy * pThis,
    const typelib_TypeDescription * pMemberTD,
    typelib_TypeDescriptionReference * pReturnTypeRef, 
    sal_Int32 nParams,
    typelib_MethodParameter * pParams,
    void ** pStack )
{
    
    typelib_TypeDescription * pReturnTD = NULL;
    if ( pReturnTypeRef )
        TYPELIB_DANGER_GET( &pReturnTD, pReturnTypeRef );

    int nFirstRealParam = 3;    
                                
                                

    void * pUnoReturn = NULL;
    void * pCppReturn = NULL;   

    if ( pReturnTD )
    {
        if ( bridges::cpp_uno::shared::isSimpleType( pReturnTD ) )
        {
            pUnoReturn = pStack;
        }
        else
        {
            pCppReturn = pStack[nFirstRealParam++];

            pUnoReturn = ( bridges::cpp_uno::shared::relatesToInterfaceType( pReturnTD )
                           ? alloca( pReturnTD->nSize )
                           : pCppReturn ); 
        }
    }

    void ** pCppIncomingParams = pStack + nFirstRealParam;

    
    

    
    void ** pUnoArgs = (void **)alloca( sizeof(void *) * nParams );

    
    void ** pCppArgs = (void **)alloca( sizeof(void *) * nParams );

    
    int * pTempIndexes =
        (int *)alloca( sizeof(int) * nParams );

    
    typelib_TypeDescription ** ppTempParamTD =
        (typelib_TypeDescription **)alloca( sizeof(void *) * nParams );

    int nTempIndexes = 0;

    for ( int nPos = 0; nPos < nParams; ++nPos )
    {
        const typelib_MethodParameter & rParam = pParams[nPos];

        typelib_TypeDescription * pParamTD = NULL;
        TYPELIB_DANGER_GET( &pParamTD, rParam.pTypeRef );

        if ( !rParam.bOut &&
             bridges::cpp_uno::shared::isSimpleType( pParamTD ) )
        {
            pCppArgs[nPos] = pUnoArgs[nPos] = pCppIncomingParams++;

            TYPELIB_DANGER_RELEASE( pParamTD );
        }
        else 
        {
            void * pCppStack;

            pCppArgs[nPos] = pCppStack = *pCppIncomingParams++;

            if ( !rParam.bIn ) 
            {
                
                pUnoArgs[nPos] = alloca( pParamTD->nSize );
                pTempIndexes[nTempIndexes] = nPos;
                
                ppTempParamTD[nTempIndexes++] = pParamTD;
            }
            //
            else if ( bridges::cpp_uno::shared::relatesToInterfaceType( pParamTD ) )
            {
                ::uno_copyAndConvertData(
                    pUnoArgs[nPos] = alloca( pParamTD->nSize ),
                    pCppStack, pParamTD,
                    pThis->getBridge()->getCpp2Uno() );
                pTempIndexes[nTempIndexes] = nPos; 
                
                ppTempParamTD[nTempIndexes++] = pParamTD;
            }
            else 
            {
                pUnoArgs[nPos] = pCppStack;
                
                TYPELIB_DANGER_RELEASE( pParamTD );
            }
        }
    }

    
    uno_Any aUnoExc; 
    uno_Any * pUnoExc = &aUnoExc;

    
    (*pThis->getUnoI()->pDispatcher)(
        pThis->getUnoI(), pMemberTD, pUnoReturn, pUnoArgs, &pUnoExc );

    
    if ( pUnoExc )
    {
        
        while ( nTempIndexes-- )
        {
            int nIndex = pTempIndexes[nTempIndexes];

            if ( pParams[nIndex].bIn ) 
            {
                ::uno_destructData( pUnoArgs[nIndex], ppTempParamTD[nTempIndexes], 0 );
            }
            TYPELIB_DANGER_RELEASE( ppTempParamTD[nTempIndexes] );
        }
        if ( pReturnTD )
            TYPELIB_DANGER_RELEASE( pReturnTD );

        CPPU_CURRENT_NAMESPACE::mscx_raiseException(
            &aUnoExc, pThis->getBridge()->getUno2Cpp() ); 

        
        return typelib_TypeClass_VOID;
    }
    else 
    {
        
        while (nTempIndexes--)
        {
            int nIndex = pTempIndexes[nTempIndexes];
            typelib_TypeDescription * pParamTD = ppTempParamTD[nTempIndexes];

            if ( pParams[nIndex].bOut ) 
            {
                
                ::uno_destructData(
                    pCppArgs[nIndex], pParamTD, cpp_release );
                ::uno_copyAndConvertData(
                    pCppArgs[nIndex], pUnoArgs[nIndex], pParamTD,
                    pThis->getBridge()->getUno2Cpp() );
            }
            
            ::uno_destructData( pUnoArgs[nIndex], pParamTD, 0 );

            TYPELIB_DANGER_RELEASE( pParamTD );
        }
        
        if ( pCppReturn ) 
        {
            if ( pUnoReturn != pCppReturn ) 
            {
                ::uno_copyAndConvertData(
                    pCppReturn, pUnoReturn, pReturnTD,
                    pThis->getBridge()->getUno2Cpp() );
                
                ::uno_destructData( pUnoReturn, pReturnTD, 0 );
            }
            
            pStack[0] = pCppReturn;
        }
        if ( pReturnTD )
        {
            typelib_TypeClass eRet = (typelib_TypeClass)pReturnTD->eTypeClass;
            TYPELIB_DANGER_RELEASE( pReturnTD );
            return eRet;
        }
        else
            return typelib_TypeClass_VOID;
    }
}

extern "C" typelib_TypeClass cpp_vtable_call(
    sal_Int64 nOffsetAndIndex,
    void ** pStack )
{
    sal_Int32 nFunctionIndex = (nOffsetAndIndex & 0xFFFFFFFF);
    sal_Int32 nVtableOffset = ((nOffsetAndIndex >> 32) & 0xFFFFFFFF);

    
    
    
    
    
    
    
    
    

    void * pThis = static_cast<char *>( pStack[2] ) - nVtableOffset;

    bridges::cpp_uno::shared::CppInterfaceProxy * pCppI =
        bridges::cpp_uno::shared::CppInterfaceProxy::castInterfaceToProxy( pThis );

    typelib_InterfaceTypeDescription * pTD = pCppI->getTypeDescr();

    OSL_ENSURE( nFunctionIndex < pTD->nMapFunctionIndexToMemberIndex, "### illegal vtable index!\n" );
    if ( nFunctionIndex >= pTD->nMapFunctionIndexToMemberIndex )
        throw RuntimeException("Illegal vtable index!",
                                reinterpret_cast<XInterface *>( pCppI ) );

    
    int nMemberPos = pTD->pMapFunctionIndexToMemberIndex[nFunctionIndex];
    OSL_ENSURE( nMemberPos < pTD->nAllMembers, "### illegal member index!\n" );

    TypeDescription aMemberDescr( pTD->ppAllMembers[nMemberPos] );

    typelib_TypeClass eRet;
    switch ( aMemberDescr.get()->eTypeClass )
    {
        case typelib_TypeClass_INTERFACE_ATTRIBUTE:
        {
            typelib_TypeDescriptionReference *pAttrTypeRef =
                reinterpret_cast<typelib_InterfaceAttributeTypeDescription *>( aMemberDescr.get() )->pAttributeTypeRef;

            if ( pTD->pMapMemberIndexToFunctionIndex[nMemberPos] == nFunctionIndex )
            {
                
                eRet = cpp2uno_call( pCppI, aMemberDescr.get(), pAttrTypeRef,
                        0, NULL, 
                        pStack );
            }
            else
            {
                
                typelib_MethodParameter aParam;
                aParam.pTypeRef = pAttrTypeRef;
                aParam.bIn      = sal_True;
                aParam.bOut     = sal_False;

                eRet = cpp2uno_call( pCppI, aMemberDescr.get(),
                        NULL, 
                        1, &aParam,
                        pStack );
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
                    typelib_TypeDescription * pTD = NULL;

                    
                    
                    
                    

                    TYPELIB_DANGER_GET( &pTD, reinterpret_cast<Type *>( pStack[4] )->getTypeLibType() );

                    if ( pTD )
                    {
                        XInterface * pInterface = NULL;
                        (*pCppI->getBridge()->getCppEnv()->getRegisteredInterface)
                            ( pCppI->getBridge()->getCppEnv(),
                              (void **)&pInterface,
                              pCppI->getOid().pData,
                              reinterpret_cast<typelib_InterfaceTypeDescription *>( pTD ) );

                        if ( pInterface )
                        {
                            
                            ::uno_any_construct( reinterpret_cast<uno_Any *>( pStack[3] ),
                                                 &pInterface, pTD, cpp_acquire );

                            pInterface->release();
                            TYPELIB_DANGER_RELEASE( pTD );

                            eRet = typelib_TypeClass_ANY;
                            break;
                        }
                        TYPELIB_DANGER_RELEASE( pTD );
                    }
                } 
                default:
                {
                    typelib_InterfaceMethodTypeDescription * pMethodTD =
                        reinterpret_cast<typelib_InterfaceMethodTypeDescription *>( aMemberDescr.get() );

                    eRet = cpp2uno_call( pCppI, aMemberDescr.get(),
                                         pMethodTD->pReturnTypeRef,
                                         pMethodTD->nParams,
                                         pMethodTD->pParams,
                                         pStack );
                }
            }
            break;
        }
        default:
        {
            throw RuntimeException("No member description found!",
                                    reinterpret_cast<XInterface *>( pCppI ) );
        }
    }

    return eRet;
}

int const codeSnippetSize = 48;

extern "C" char privateSnippetExecutor;







unsigned char * codeSnippet(
    unsigned char * code,
    char param_kind[4],
    sal_Int32 nFunctionIndex,
    sal_Int32 nVtableOffset )
{
    sal_uInt64 nOffsetAndIndex = ( ( (sal_uInt64) nVtableOffset ) << 32 ) | ( (sal_uInt64) nFunctionIndex );
    unsigned char *p = code;

    
    if ( param_kind[0] == CPPU_CURRENT_NAMESPACE::REGPARAM_INT )
    {
        
        *p++ = 0x48; *p++ = 0x89; *p++ = 0x4C; *p++ = 0x24; *p++ = 0x08;
    }
    else
    {
        
        *p++ = 0xF2; *p++ = 0x0F; *p++ = 0x11; *p++ = 0x44; *p++ = 0x24; *p++ = 0x08;
    }
    if ( param_kind[1] == CPPU_CURRENT_NAMESPACE::REGPARAM_INT )
    {
        
        *p++ = 0x48; *p++ = 0x89; *p++ = 0x54; *p++ = 0x24; *p++ = 0x10;
    }
    else
    {
        
        *p++ = 0xF2; *p++ = 0x0F; *p++ = 0x11; *p++ = 0x4C; *p++ = 0x24; *p++ = 0x10;
    }
    if ( param_kind[2] == CPPU_CURRENT_NAMESPACE::REGPARAM_INT )
    {
        
        *p++ = 0x4C; *p++ = 0x89; *p++ = 0x44; *p++ = 0x24; *p++ = 0x18;
    }
    else
    {
        
        *p++ = 0xF2; *p++ = 0x0F; *p++ = 0x11; *p++ = 0x54; *p++ = 0x24; *p++ = 0x18;
    }
    if ( param_kind[3] == CPPU_CURRENT_NAMESPACE::REGPARAM_INT )
    {
        
        *p++ = 0x4C;*p++ = 0x89; *p++ = 0x4C; *p++ = 0x24; *p++ = 0x20;
    }
    else
    {
        
        *p++ = 0xF2; *p++ = 0x0F; *p++ = 0x11; *p++ = 0x5C; *p++ = 0x24; *p++ = 0x20;
    }

    
    *p++ = 0x48; *p++ = 0xB9;
    *((sal_uInt64 *)p) = nOffsetAndIndex; p += 8;

    
    *p++ = 0x49; *p++ = 0xBB;
    *((void **)p) = &privateSnippetExecutor; p += 8;

    
    *p++ = 0x41; *p++ = 0xFF; *p++ = 0xE3;

    OSL_ASSERT( p < code + codeSnippetSize );

    return code + codeSnippetSize;
}

struct bridges::cpp_uno::shared::VtableFactory::Slot { void * fn; };

bridges::cpp_uno::shared::VtableFactory::Slot *
bridges::cpp_uno::shared::VtableFactory::mapBlockToVtable(
    void * block )
{
    return static_cast< Slot * >(block) + 1;
}

sal_Size bridges::cpp_uno::shared::VtableFactory::getBlockSize(
    sal_Int32 slotCount )
{
    return (slotCount + 1) * sizeof (Slot) + slotCount * codeSnippetSize;
}

bridges::cpp_uno::shared::VtableFactory::Slot *
bridges::cpp_uno::shared::VtableFactory::initializeBlock(
    void * block,
    sal_Int32 slotCount )
{
    struct Rtti {
        sal_Int32 n0, n1, n2;
        type_info * rtti;
        Rtti():
            n0(0), n1(0), n2(0),
            rtti(CPPU_CURRENT_NAMESPACE::mscx_getRTTI(
                     OUString(
                                       "com.sun.star.uno.XInterface")))
        {}
    };
    static Rtti rtti;

    Slot * slots = mapBlockToVtable(block);
    slots[-1].fn = &rtti;
    return slots + slotCount;
}

unsigned char * bridges::cpp_uno::shared::VtableFactory::addLocalFunctions(
    Slot ** slots,
    unsigned char * code,
    typelib_InterfaceTypeDescription const * type,
    sal_Int32 nFunctionOffset,
    sal_Int32 functionCount,
    sal_Int32 nVtableOffset )
{
    (*slots) -= functionCount;
    Slot * s = *slots;

    for (int member = 0; member < type->nMembers; ++member) {
        typelib_TypeDescription * pTD = NULL;

        TYPELIB_DANGER_GET( &pTD, type->ppMembers[ member ] );
        OSL_ASSERT( pTD );

        char param_kind[4];
        int nr = 0;

        for (int i = 0; i < 4; ++i)
            param_kind[i] = CPPU_CURRENT_NAMESPACE::REGPARAM_INT;

        
        ++nr;

        if ( pTD->eTypeClass == typelib_TypeClass_INTERFACE_ATTRIBUTE )
        {
            typelib_InterfaceAttributeTypeDescription * pIfaceAttrTD =
                reinterpret_cast<typelib_InterfaceAttributeTypeDescription *>( pTD );

            

            (s++)->fn = code;
            code = codeSnippet( code, param_kind, nFunctionOffset++, nVtableOffset );
            if ( ! pIfaceAttrTD->bReadOnly )
            {
                typelib_TypeDescription * pAttrTD = NULL;
                TYPELIB_DANGER_GET( &pAttrTD, pIfaceAttrTD->pAttributeTypeRef );
                OSL_ASSERT( pAttrTD );

                
                if ( pAttrTD->eTypeClass == typelib_TypeClass_FLOAT ||
                     pAttrTD->eTypeClass == typelib_TypeClass_DOUBLE )
                    param_kind[nr++] = CPPU_CURRENT_NAMESPACE::REGPARAM_FLT;

                TYPELIB_DANGER_RELEASE( pAttrTD );

                (s++)->fn = code;
                code = codeSnippet( code, param_kind, nFunctionOffset++, nVtableOffset );
            }
        }
        else if ( pTD->eTypeClass == typelib_TypeClass_INTERFACE_METHOD )
        {
            typelib_InterfaceMethodTypeDescription * pMethodTD =
                reinterpret_cast<typelib_InterfaceMethodTypeDescription *>( pTD );

            typelib_TypeDescription * pReturnTD = NULL;
            TYPELIB_DANGER_GET( &pReturnTD, pMethodTD->pReturnTypeRef );
            OSL_ASSERT( pReturnTD );

            if ( !bridges::cpp_uno::shared::isSimpleType( pReturnTD ) )
            {
                
                ++nr;
            }

            for (int param = 0; nr < 4 && param < pMethodTD->nParams; ++param, ++nr)
            {
                typelib_TypeDescription * pParamTD = NULL;

                TYPELIB_DANGER_GET( &pParamTD, pMethodTD->pParams[param].pTypeRef );
                OSL_ASSERT( pParamTD );

                if ( pParamTD->eTypeClass == typelib_TypeClass_FLOAT ||
                     pParamTD->eTypeClass == typelib_TypeClass_DOUBLE )
                    param_kind[nr] = CPPU_CURRENT_NAMESPACE::REGPARAM_FLT;

                TYPELIB_DANGER_RELEASE( pParamTD );
            }
            (s++)->fn = code;
            code = codeSnippet( code, param_kind, nFunctionOffset++, nVtableOffset );

            TYPELIB_DANGER_RELEASE( pReturnTD );
        }
        else
            OSL_ASSERT( false );

        TYPELIB_DANGER_RELEASE( pTD );
    }
    return code;
}

void bridges::cpp_uno::shared::VtableFactory::flushCode(
    unsigned char const *,
    unsigned char const * )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
