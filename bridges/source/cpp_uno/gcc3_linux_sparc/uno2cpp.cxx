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

#include "share.hxx"

using namespace com::sun::star::uno;

namespace
{








void callVirtualMethod( void * pAdjustedThisPtr,
                        sal_Int32 nVtableIndex,
                        void * pRegisterReturn,
                        typelib_TypeClass eReturnType,
                        sal_Int32 * pStackLongs,
                        sal_Int32 nStackLongs ) __attribute__((noinline));

void callVirtualMethod( void * pAdjustedThisPtr,
                        sal_Int32 /* nVtableIndex */,
                        void * pRegisterReturn,
                        typelib_TypeClass eReturnType,
#if OSL_DEBUG_LEVEL > 0
                        sal_Int32 * pStackLongs,
                        sal_Int32 nStackLongs)
#else
                        sal_Int32 * /*pStackLongs*/,
                        sal_Int32 /*nStackLongs*/)
#endif
{
    
    

    OSL_ENSURE( pStackLongs && pAdjustedThisPtr, "### null ptr!" );
    OSL_ENSURE( (sizeof(void *) == 4) &&
                 (sizeof(sal_Int32) == 4), "### unexpected size of int!" );
    OSL_ENSURE( nStackLongs && pStackLongs, "### no stack in callVirtualMethod !" );

    
    if (! pAdjustedThisPtr) CPPU_CURRENT_NAMESPACE::dummy_can_throw_anything("xxx"); 

    long o0, o1; 
    double f0d;
    float f0f;
    volatile long long saveReg[7];

    __asm__ (
        
        "std %%l0, [%4]\n\t"
        "mov %4, %%l0\n\t"
        "mov %%l0, %%l1\n\t"
        "add %%l0, 8, %%l0\n\t"
        "std %%l2, [%%l0]\n\t"
        "add %%l0, 8, %%l0\n\t"
        "std %%l4, [%%l0]\n\t"
        "add %%l0, 8, %%l0\n\t"
        "std %%o0, [%%l0]\n\t"
        "add %%l0, 8, %%l0\n\t"
        "std %%o2, [%%l0]\n\t"
        "add %%l0, 8, %%l0\n\t"
        "std %%o4, [%%l0]\n\t"
        "add %%l0, 8, %%l0\n\t"
        "std %%l6, [%%l0]\n\t"
        "mov %%l1, %%l7\n\t"

        
        "mov %%sp, %%l3\n\t"        

        "subcc %%i5, 7, %%l0\n\t"
        "ble .LmoveOn\n\t"
        "nop\n\t"

        "sll %%l0, 2, %%l0\n\t"
        "add %%l0, 96, %%l0\n\t"
        "mov %%sp, %%l1\n\t"        
        "sub %%sp, %%l0, %%l0\n\t"  
        "andcc %%l0, 7, %%g0\n\t"   
        "be .LisAligned\n\t"
        "nop\n\t"
        "sub %%l0, 4, %%l0\n"
    ".LisAligned:\n\t"
        "mov %%l0, %%o5\n\t"            
        "add %%g0, 16, %%o4\n"

        
        
    ".LcopyDown:\n\t"
        "ld [%%l1], %%l2\n\t"
        "st %%l2,[%%l0]\n\t"
        "add %%l0, 4, %%l0\n\t"
        "add %%l1, 4, %%l1\n\t"
        "subcc %%o4, 1, %%o4\n\t"
        "bne .LcopyDown\n\t"

        "mov %%o5, %%sp\n\t"        
        
        

        "sub %%i5, 7, %%l0\n\t"     
        "add %%i4, 28, %%l1\n\t"
        "add %%sp, 92, %%l2\n"
    ".LcopyLong:\n\t"
        "ld [%%l1], %%o0\n\t"
        "st %%o0, [%%l2]\n\t"
        "add %%l1, 4, %%l1\n\t"
        "add %%l2, 4, %%l2\n\t"
        "subcc %%l0, 1, %%l0\n\t"
        "bne .LcopyLong\n\t"
        "nop\n"

    ".LmoveOn:\n\t"
        "mov %%i5, %%l0\n\t"        
        "mov %%i4, %%l1\n\t"

        "ld [%%l1], %%o0\n\t"       
        "st %%o0, [%%sp+64]\n\t"
        "sub %%l0, 1, %%l0\n\t"
        "add %%l1, 4, %%l1\n\t"

        "ld [%%l1], %%o0\n\t"
        "subcc %%l0, 1, %%l0\n\t"
        "be .LdoCall\n\t"
        "nop\n\t"

        "add %%l1, 4, %%l1\n\t"
        "ld [%%l1], %%o1\n\t"
        "subcc %%l0, 1, %%l0\n\t"
        "be .LdoCall\n\t"
        "nop\n\t"

        "add %%l1, 4, %%l1\n\t"
        "ld [%%l1], %%o2\n\t"
        "subcc %%l0, 1, %%l0\n\t"
        "be .LdoCall\n\t"
        "nop\n\t"

        "add %%l1, 4, %%l1\n\t"
        "ld [%%l1], %%o3\n\t"
        "subcc %%l0, 1, %%l0\n\t"
        "be .LdoCall\n\t"
        "nop\n\t"

        "add %%l1, 4, %%l1\n\t"
        "ld [%%l1], %%o4\n\t"
        "subcc %%l0, 1, %%l0\n\t"
        "be .LdoCall\n\t"
        "nop\n\t"

        "add %%l1, 4, %%l1\n\t"
        "ld [%%l1], %%o5\n"

    ".LdoCall:\n\t"
        "ld [%%i0], %%l0\n\t"       

"sll %%i1, 2, %%l6\n\t"

        "add %%l6, %%l0, %%l0\n\t"









        "ld [%%l0], %%l0\n\t"

        "ld [%%i4], %%l2\n\t"
        "subcc %%l2, %%g0, %%l2\n\t"
        "bne .LcomplexCall\n\t"
        "nop\n\t"
        "call %%l0\n\t"
        "nop\n\t"
        "ba .LcallReturned\n\t"
        "nop\n"
    ".LcomplexCall:\n\t"
        "call %%l0\n\t"
        "nop\n\t"
        "unimp\n"

    ".LcallReturned:\n\t"
        "mov %%l3, %%sp\n\t"        
        "st %%o0, %0\n\t"           
        "st %%o1, %1\n\t"
        "std %%f0, %2\n\t"
        "st %%f0, %3\n\t"

        
        "ldd [%%l7], %%l0\n\t"
        "add %%l7, 8, %%l7\n\t"
        "ldd [%%l7], %%l2\n\t"
        "add %%l7, 8, %%l7\n\t"
        "ldd [%%l7], %%l4\n\t"
        "add %%l7, 8, %%l7\n\t"
        "ldd [%%l7], %%o0\n\t"
        "add %%l7, 8, %%l7\n\t"
        "ldd [%%l7], %%o2\n\t"
        "add %%l7, 8, %%l7\n\t"
        "ldd [%%l7], %%o4\n\t"
        "add %%l7, 8, %%l7\n\t"
        "ldd [%%l7], %%l6\n\t"
        :
        "=m"(o0),
        "=m"(o1),
        "=m"(f0d),
        "=m"(f0f),
        :
        "r"(&saveReg[0])
        :
        "memory"
        );
    switch( eReturnType )
    {
        case typelib_TypeClass_HYPER:
        case typelib_TypeClass_UNSIGNED_HYPER:
            ((long*)pRegisterReturn)[1] = o1;
        case typelib_TypeClass_LONG:
        case typelib_TypeClass_UNSIGNED_LONG:
        case typelib_TypeClass_ENUM:
            ((long*)pRegisterReturn)[0] = o0;
            break;
        case typelib_TypeClass_CHAR:
        case typelib_TypeClass_SHORT:
        case typelib_TypeClass_UNSIGNED_SHORT:
            *(unsigned short*)pRegisterReturn = (unsigned short)o0;
            break;
        case typelib_TypeClass_BOOLEAN:
        case typelib_TypeClass_BYTE:
            *(unsigned char*)pRegisterReturn = (unsigned char)o0;
            break;
        case typelib_TypeClass_FLOAT:
            *(float*)pRegisterReturn = f0f;
            break;
        case typelib_TypeClass_DOUBLE:
            *(double*)pRegisterReturn = f0d;
            break;
        default:
            break;
    }
}


static void cpp_call(
    bridges::cpp_uno::shared::UnoInterfaceProxy * pThis,
    bridges::cpp_uno::shared::VtableSlot aVtableSlot,
    typelib_TypeDescriptionReference * pReturnTypeRef,
    sal_Int32 nParams, typelib_MethodParameter * pParams,
    void * pUnoReturn, void * pUnoArgs[], uno_Any ** ppUnoExc )
{
      
      char * pCppStack  =
          (char *)alloca( (nParams+2) * sizeof(sal_Int64) );
      char * pCppStackStart = pCppStack;

    
    typelib_TypeDescription * pReturnTypeDescr = 0;
    TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );
    OSL_ENSURE( pReturnTypeDescr, "### expected return type description!" );

    void * pCppReturn = 0; 

    if (pReturnTypeDescr)
    {
        if (bridges::cpp_uno::shared::isSimpleType( pReturnTypeDescr ))
        {
            pCppReturn = pUnoReturn; 
            *(void**)pCppStack = NULL;
        }
        else
        {
            
            pCppReturn = *(void **)pCppStack = (bridges::cpp_uno::shared::relatesToInterfaceType(pReturnTypeDescr )
                                                ? alloca( pReturnTypeDescr->nSize )
                                                : pUnoReturn); 
        }
        pCppStack += sizeof(void*);
    }
    
    void * pAdjustedThisPtr = reinterpret_cast< void ** >(pThis->getCppI())
             + aVtableSlot.offset;
             *(void**)pCppStack = pAdjustedThisPtr;
    pCppStack += sizeof( void* );

    
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
            pCppArgs[ nPos ] = CPPU_CURRENT_NAMESPACE::adjustPointer(pCppStack, pParamTypeDescr );

            switch (pParamTypeDescr->eTypeClass)
            {
            case typelib_TypeClass_HYPER:
            case typelib_TypeClass_UNSIGNED_HYPER:
            case typelib_TypeClass_DOUBLE:
                        OSL_ASSERT( sizeof (double) == sizeof (sal_Int64) );
                          *reinterpret_cast< sal_Int32 * >(pCppStack) =
                          *reinterpret_cast< sal_Int32 const * >(pUnoArgs[ nPos ]);
                          pCppStack += sizeof (sal_Int32);
                          *reinterpret_cast< sal_Int32 * >(pCppStack) =
                          *(reinterpret_cast< sal_Int32 const * >(pUnoArgs[ nPos ] ) + 1);
                          break;
                    default:
                          uno_copyAndConvertData(
                             pCppArgs[nPos], pUnoArgs[nPos], pParamTypeDescr,
                            pThis->getBridge()->getUno2Cpp() );
                          break;
                        }
            
            TYPELIB_DANGER_RELEASE( pParamTypeDescr );
        }
        else 
        {
            if (! rParam.bIn) 
            {
                
                uno_constructData(
                    *(void **)pCppStack = pCppArgs[nPos] = alloca( pParamTypeDescr->nSize ),
                    pParamTypeDescr );
                pTempIndices[nTempIndices] = nPos; 
                
                ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
            }
            
            else if (bridges::cpp_uno::shared::relatesToInterfaceType(
                              pParamTypeDescr ))
            {
                uno_copyAndConvertData(
                    *(void **)pCppStack = pCppArgs[nPos] = alloca( pParamTypeDescr->nSize ),
                                    pUnoArgs[nPos], pParamTypeDescr,
                    pThis->getBridge()->getUno2Cpp() );

                pTempIndices[nTempIndices] = nPos; 
                
                ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
            }
            else 
            {
                *(void **)pCppStack = pCppArgs[nPos] = pUnoArgs[nPos];
                
                TYPELIB_DANGER_RELEASE( pParamTypeDescr );
            }
        }
        pCppStack += sizeof(sal_Int32); 
    }

    try
    {
        int nStackLongs = (pCppStack - pCppStackStart)/sizeof(sal_Int32);
        OSL_ENSURE( !( (pCppStack - pCppStackStart ) & 3), "UNALIGNED STACK !!! (Please DO panic" );

        if( nStackLongs & 1 )
            
            nStackLongs++;
        callVirtualMethod(
            pAdjustedThisPtr,
            aVtableSlot.index,
            pCppReturn,
            pReturnTypeDescr->eTypeClass,
            (sal_Int32 *)pCppStackStart,
             nStackLongs);
        
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
     catch( ... )
     {
         
           fillUnoException( CPPU_CURRENT_NAMESPACE::__cxa_get_globals()->caughtExceptions,
                                *ppUnoExc, pThis->getBridge()->getCpp2Uno() );

        
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
#if defined BRIDGES_DEBUG
    OString cstr( OUStringToOString( pMemberDescr->pTypeName, RTL_TEXTENCODING_ASCII_US ) );
    fprintf( stderr, "received dispatch( %s )\n", cstr.getStr() );
#endif

    
    bridges::cpp_uno::shared::UnoInterfaceProxy * pThis
       = static_cast< bridges::cpp_uno::shared::UnoInterfaceProxy * >(pUnoI);


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
