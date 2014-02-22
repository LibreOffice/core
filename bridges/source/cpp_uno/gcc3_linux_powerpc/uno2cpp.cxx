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


using namespace ::rtl;
using namespace ::com::sun::star::uno;

namespace
{



static void callVirtualMethod(
    void * pAdjustedThisPtr,
    sal_Int32 nVtableIndex,
    void * pRegisterReturn,
    typelib_TypeClass eReturnType,
    char * pPT,
    sal_Int32 * pStackLongs,
    sal_Int32 nStackLongs)
{

  
  

  
  
  
  

     unsigned long * mfunc;        
     int gpr[8];                   
     int off;                      
#ifndef __NO_FPRS__
     double fpr[8];                
     int f;                        
     double dret;                  
#endif
     int n;                        
     long *p;                      
     int c;                        
     int iret, iret2;

     
     
     
     
     

     
     

     
     
     
     

     
     
     

     
     
     __asm__ __volatile__ (
          "rlwinm %0,%0,3,3,28\n\t"
          "addi %0,%0,22\n\t"
          "rlwinm %0,%0,0,4,28\n\t"
          "lwz 0,0(1)\n\t"
          "subf 1,%0,1\n\t"
          "stw 0,0(1)\n\t"
          : : "r" (nStackLongs) : "0" );

     __asm__ __volatile__ ( "addi %0,1,8" : "=r" (p) : );

     
     


     
     n = 0;
#ifndef __NO_FPRS__
     f = 0;
#endif

     
     

     

#if 0
     /* Let's figure out what is really going on here*/
     fprintf(stderr,"callVirtualMethod parameters string is %s\n",pPT);
     int k = nStackLongs;
     long * q = (long *)pStackLongs;
     while (k > 0) {
       fprintf(stderr,"uno stack is: %x\n",*q);
       k--;
       q++;
     }
#endif

     /* parse the argument list up to the ending ) */
     while (*pPT != 'X') {
       c = *pPT;
       switch (c) {
       case 'D':                   /* type is double */
#ifndef __NO_FPRS__
            if (f < 8) {
               fpr[f++] = *((double *)pStackLongs);   /* store in register */
#else
            if (n & 1)
               n++;
            if (n < 8) {
               gpr[n++] = *pStackLongs;
               gpr[n++] = *(pStackLongs+1);
#endif
        } else {
           if (((long) p) & 4)
              p++;
               *p++ = *pStackLongs;       /* or on the parameter stack */
               *p++ = *(pStackLongs + 1);
        }
            pStackLongs += 2;
            break;

       case 'F':                   /* type is float */
     /* this assumes that floats are stored as 1 32 bit word on param
        stack and that if passed in parameter stack to C, should be
        as double word.

            Whoops: the abi is not actually followed by gcc, need to
            store floats as a *single* word on outgoing parameter stack
            to match what gcc actually does
     */
#ifndef __NO_FPRS__
            if (f < 8) {
               fpr[f++] = *((float *)pStackLongs);
#else
            if (n < 8) {
               gpr[n++] = *pStackLongs;
#endif
        } else {
#if 0 /* if abi were followed */
           if (((long) p) & 4)
              p++;
           *((double *)p) = *((float *)pStackLongs);
               p += 2;
#else
           *((float *)p) = *((float *)pStackLongs);
               p += 1;
#endif
        }
            pStackLongs += 1;
            break;

       case 'H':                /* type is long long */
            if (n & 1) n++;     /* note even elements gpr[] will map to
                                   odd registers*/
            if (n <= 6) {
               gpr[n++] = *pStackLongs;
               gpr[n++] = *(pStackLongs+1);
        } else {
           if (((long) p) & 4)
              p++;
               *p++ = *pStackLongs;
               *p++ = *(pStackLongs+1);
        }
            pStackLongs += 2;
            break;

       case 'S':
            if (n < 8) {
               gpr[n++] = *((unsigned short*)pStackLongs);
        } else {
               *p++ = *((unsigned short *)pStackLongs);
        }
            pStackLongs += 1;
            break;

       case 'B':
            if (n < 8) {
               gpr[n++] = *((char *)pStackLongs);
        } else {
               *p++ = *((char *)pStackLongs);
        }
            pStackLongs += 1;
            break;

       default:
            if (n < 8) {
               gpr[n++] = *pStackLongs;
        } else {
               *p++ = *pStackLongs;
        }
            pStackLongs += 1;
            break;
       }
       pPT++;
     }

     /* figure out the address of the function we need to invoke */
     off = nVtableIndex;
     off = off * 4;                         
     mfunc = *((unsigned long **)pAdjustedThisPtr);    
     mfunc = (unsigned long *)((char *)mfunc + off); 
     mfunc = *((unsigned long **)mfunc);                 
     typedef void (*FunctionCall)(sal_uInt32, sal_uInt32, sal_uInt32, sal_uInt32, sal_uInt32, sal_uInt32, sal_uInt32, sal_uInt32);
     FunctionCall ptr = (FunctionCall)mfunc;

    /* Set up the machine registers and invoke the function */

    __asm__ __volatile__ (
        "lwz    3,  0(%0)\n\t"
        "lwz    4,  4(%0)\n\t"
        "lwz    5,  8(%0)\n\t"
        "lwz    6,  12(%0)\n\t"
        "lwz    7,  16(%0)\n\t"
        "lwz    8,  20(%0)\n\t"
        "lwz    9,  24(%0)\n\t"
        "lwz    10, 28(%0)\n\t"
#ifndef __NO_FPRS__
        "lfd    1,  0(%1)\n\t"
        "lfd    2,  8(%1)\n\t"
        "lfd    3,  16(%1)\n\t"
        "lfd    4,  24(%1)\n\t"
        "lfd    5,  32(%1)\n\t"
        "lfd    6,  40(%1)\n\t"
        "lfd    7,  48(%1)\n\t"
        "lfd    8,  56(%1)\n\t"
            : : "r" (gpr), "r" (fpr)
#else
            : : "r" (gpr)
#endif
        : "0", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12"
    );

    
    register sal_uInt32 r3 __asm__("r3");
    register sal_uInt32 r4 __asm__("r4");
    register sal_uInt32 r5 __asm__("r5");
    register sal_uInt32 r6 __asm__("r6");
    register sal_uInt32 r7 __asm__("r7");
    register sal_uInt32 r8 __asm__("r8");
    register sal_uInt32 r9 __asm__("r9");
    register sal_uInt32 r10 __asm__("r10");

    (*ptr)(r3, r4, r5, r6, r7, r8, r9, r10);

    __asm__ __volatile__ (
       "mr     %0,     3\n\t"
       "mr     %1,     4\n\t"
#ifndef __NO_FPRS__
       "fmr    %2,     1\n\t"
       : "=r" (iret), "=r" (iret2), "=f" (dret)
#else
       : "=r" (iret), "=r" (iret2)
#endif
       : );

    switch( eReturnType )
    {
        case typelib_TypeClass_HYPER:
        case typelib_TypeClass_UNSIGNED_HYPER:
                ((long*)pRegisterReturn)[0] = iret;
            ((long*)pRegisterReturn)[1] = iret2;
        case typelib_TypeClass_LONG:
        case typelib_TypeClass_UNSIGNED_LONG:
        case typelib_TypeClass_ENUM:
            ((long*)pRegisterReturn)[0] = iret;
            break;
        case typelib_TypeClass_CHAR:
        case typelib_TypeClass_SHORT:
        case typelib_TypeClass_UNSIGNED_SHORT:
                *(unsigned short*)pRegisterReturn = (unsigned short)iret;
            break;
        case typelib_TypeClass_BOOLEAN:
        case typelib_TypeClass_BYTE:
                *(unsigned char*)pRegisterReturn = (unsigned char)iret;
            break;
        case typelib_TypeClass_FLOAT:
#ifndef __NO_FPRS__
                *(float*)pRegisterReturn = (float)dret;
#else
                ((unsigned int*)pRegisterReturn)[0] = iret;
#endif
            break;
        case typelib_TypeClass_DOUBLE:
#ifndef __NO_FPRS__
            *(double*)pRegisterReturn = dret;
#else
            ((unsigned int*)pRegisterReturn)[0] = iret;
            ((unsigned int*)pRegisterReturn)[1] = iret2;
#endif
            break;
        default:
            break;
    }
}



static void cpp_call(
    bridges::cpp_uno::shared::UnoInterfaceProxy * pThis,
    bridges::cpp_uno::shared::VtableSlot  aVtableSlot,
    typelib_TypeDescriptionReference * pReturnTypeRef,
    sal_Int32 nParams, typelib_MethodParameter * pParams,
    void * pUnoReturn, void * pUnoArgs[], uno_Any ** ppUnoExc )
{
      
      char * pCppStack      =
          (char *)alloca( sizeof(sal_Int32) + ((nParams+2) * sizeof(sal_Int64)) );
      char * pCppStackStart = pCppStack;

        
        char * pParamType = (char *) alloca(nParams+2);
        char * pPT = pParamType;

    
    typelib_TypeDescription * pReturnTypeDescr = 0;
    TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );
    

    void * pCppReturn = 0; 

    if (pReturnTypeDescr)
    {
        if (bridges::cpp_uno::shared::isSimpleType( pReturnTypeDescr ))
        {
            pCppReturn = pUnoReturn; 
        }
        else
        {
            
            pCppReturn = *(void **)pCppStack =
                              (bridges::cpp_uno::shared::relatesToInterfaceType( pReturnTypeDescr )
                   ? alloca( pReturnTypeDescr->nSize ): pUnoReturn); 
                        *pPT++ = 'I'; 
            pCppStack += sizeof(void *);
        }
    }
    
        void* pAdjustedThisPtr = reinterpret_cast< void **>(pThis->getCppI()) + aVtableSlot.offset;
    *(void**)pCppStack = pAdjustedThisPtr;
    pCppStack += sizeof( void* );
        *pPT++ = 'I';

    
    
    
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
            uno_copyAndConvertData( pCppArgs[nPos] = pCppStack, pUnoArgs[nPos], pParamTypeDescr,
                                    pThis->getBridge()->getUno2Cpp() );

            switch (pParamTypeDescr->eTypeClass)
            {

                          
                          
                          
                          
                          
                          
                          
                          
                          

                  case typelib_TypeClass_LONG:
                  case typelib_TypeClass_UNSIGNED_LONG:
                  case typelib_TypeClass_ENUM:
                *pPT++ = 'I';
                break;
                   case typelib_TypeClass_SHORT:
                  case typelib_TypeClass_CHAR:
                  case typelib_TypeClass_UNSIGNED_SHORT:
                            *pPT++ = 'S';
                            break;
                  case typelib_TypeClass_BOOLEAN:
                  case typelib_TypeClass_BYTE:
                            *pPT++ = 'B';
                            break;
                  case typelib_TypeClass_FLOAT:
                            *pPT++ = 'F';
                break;
                case typelib_TypeClass_DOUBLE:
                *pPT++ = 'D';
                pCppStack += sizeof(sal_Int32); 
                break;
            case typelib_TypeClass_HYPER:
            case typelib_TypeClass_UNSIGNED_HYPER:
                *pPT++ = 'H';
                pCppStack += sizeof(sal_Int32); 
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
                    *(void **)pCppStack = pCppArgs[nPos] = alloca( pParamTypeDescr->nSize ),
                    pParamTypeDescr );
                pTempIndices[nTempIndices] = nPos; 
                
                ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
            }
            
            else if (bridges::cpp_uno::shared::relatesToInterfaceType( pParamTypeDescr ))
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
                        
                        *pPT++='I';
        }
        pCppStack += sizeof(sal_Int32); 
    }

        
        *pPT++='X';
        *pPT=0;

    try
    {
        OSL_ENSURE( !( (pCppStack - pCppStackStart ) & 3), "UNALIGNED STACK !!! (Please DO panic)" );
        callVirtualMethod(
            pAdjustedThisPtr, aVtableSlot.index,
            pCppReturn, pReturnTypeDescr->eTypeClass, pParamType,
            (sal_Int32 *)pCppStackStart, (pCppStack - pCppStackStart) / sizeof(sal_Int32) );
        
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
    
        bridges::cpp_uno::shared::UnoInterfaceProxy * pThis
            = static_cast< bridges::cpp_uno::shared::UnoInterfaceProxy *> (pUnoI);

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
