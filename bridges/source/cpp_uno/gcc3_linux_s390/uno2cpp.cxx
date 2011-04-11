/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_bridges.hxx"

#include <malloc.h>

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
static sal_Int32
invoke_count_words(char * pPT)
{
     sal_Int32 overflow = 0, gpr = 0, fpr = 0;
     int c;                        // character of parameter type being decoded

     while (*pPT != 'X') {
       c = *pPT;
       switch (c) {
       case 'D':                   /* type is double */
            if (fpr < 2) fpr++; else overflow+=2;
            break;

       case 'F':                   /* type is float */
            if (fpr < 2) fpr++; else overflow++;
            break;

       case 'H':                /* type is long long */
            if (gpr < 4) gpr+=2; else gpr=5, overflow+=2;
            break;

       case 'S':
       case 'T':
       case 'B':
       case 'C':
            if (gpr < 5) gpr++; else overflow++;
            break;

       default:
            if (gpr < 5) gpr++; else overflow++;
            break;
       }
       pPT++;
     }
     /* Round up number of overflow words to ensure stack
       stays aligned to 8 bytes.  */
     return (overflow + 1) & ~1;
}

static void
//invoke_copy_to_stack(sal_Int32 paramCount, sal_Int32 * pStackLongs, char * pPT, sal_Int32* d_ov, sal_Int32 overflow)
invoke_copy_to_stack(sal_Int32 * pStackLongs, char * pPT, sal_Int32* d_ov, sal_Int32 overflow)
{
    sal_Int32 *d_gpr = d_ov + overflow;
    sal_Int64 *d_fpr = (sal_Int64 *)(d_gpr + 5);
    sal_Int32 gpr = 0, fpr = 0;
    char c;

     while (*pPT != 'X') {
       c = *pPT;
       switch (c) {
       case 'D':                   /* type is double */
            if (fpr < 2)
                *((double*)  d_fpr) = *((double *)pStackLongs), d_fpr++, fpr++;
            else
                *((double*)  d_ov ) = *((double *)pStackLongs), d_ov+=2;

            pStackLongs += 2;
            break;

       case 'F':                   /* type is float */
            if (fpr < 2) {
                *((sal_Int64*) d_fpr) = 0;
                *((float*)   d_fpr) = *((float *)pStackLongs), d_fpr++, fpr++;
              }
            else {
                *((sal_Int64*) d_ov) = 0;
                *((float*)   d_ov ) = *((float *)pStackLongs), d_ov++;
              }

            pStackLongs += 1;
            break;

       case 'H':                /* type is long long */
            if (gpr < 4) {
                *((sal_Int64*) d_gpr) = *((sal_Int64*) pStackLongs), d_gpr+=2, gpr+=2;
              }
            else {
                *((sal_Int64*) d_ov ) = *((sal_Int64*) pStackLongs), d_ov+=2, gpr=5;
              }
            pStackLongs += 2;
            break;

       case 'S':
            if (gpr < 5)
                *((sal_uInt32*)d_gpr) = *((unsigned short*)pStackLongs), d_gpr++, gpr++;
            else
                *((sal_uInt32*)d_ov ) = *((unsigned short*)pStackLongs), d_ov++;
            pStackLongs += 1;
            break;

       case 'T':
            if (gpr < 5)
                *((sal_Int32*)d_gpr) = *((signed short*)pStackLongs), d_gpr++, gpr++;
            else
                *((sal_Int32*)d_ov ) = *((signed short*)pStackLongs), d_ov++;
            pStackLongs += 1;
            break;

       case 'B':
            if (gpr < 5)
                *((sal_uInt32*)d_gpr) = *((unsigned char*)pStackLongs), d_gpr++, gpr++;
            else
                *((sal_uInt32*)d_ov ) = *((unsigned char*)pStackLongs), d_ov++;
            pStackLongs += 1;
            break;

       case 'C':
            if (gpr < 5)
                *((sal_Int32*)d_gpr) = *((signed char*)pStackLongs), d_gpr++, gpr++;
            else
                *((sal_Int32*)d_ov ) = *((signed char*)pStackLongs), d_ov++;
            pStackLongs += 1;
            break;

       default:
            if (gpr < 5)
                *((sal_Int32*)d_gpr) = *pStackLongs, d_gpr++, gpr++;
            else
                *((sal_Int32*)d_ov ) = *pStackLongs, d_ov++;
            pStackLongs += 1;
            break;
       }
       pPT++;
     }
}

//==================================================================================================
static void callVirtualMethod(
    void * pThis,
    sal_Int32 nVtableIndex,
    void * pRegisterReturn,
    typelib_TypeClass eReturnType,
    char * pPT,
    sal_Int32 * pStackLongs,
    sal_Int32 nStackLongs)
{

  // parameter list is mixed list of * and values
  // reference parameters are pointers

  // the basic idea here is to use gpr[5] as a storage area for
  // the future values of registers r2 to r6 needed for the call,
  // and similarly fpr[2] as a storage area for the future values
  // of floating point registers f0 to f2

     sal_Int32 *vtable = *(sal_Int32 **)pThis;
//    sal_Int32 method = vtable[nVtableIndex + 2];
     sal_Int32 method = vtable[nVtableIndex];
     sal_Int32 overflow = invoke_count_words (pPT);
     sal_Int32 result;
     volatile double dret;                  // temporary function return values
     volatile float fret;
     volatile int iret, iret2;

     void * dummy = alloca(32); // dummy alloca to force r11 usage for exception handling

    __asm__ __volatile__
     (
        "lr    7,15\n\t"
        "ahi   7,-48\n\t"

        "lr    3,%2\n\t"
        "sll   3,2\n\t"
        "lcr   3,3\n\t"
        "l     2,0(15)\n\t"
        "la    15,0(3,7)\n\t"
        "st    2,0(15)\n\t"

        "lr    2,%0\n\t"
        "lr    3,%1\n\t"
        "la    4,96(15)\n\t"
        "lr    5,%2\n\t"
        "basr  14,%3\n\t"

        "ld    0,116(7)\n\t"
        "ld    2,124(7)\n\t"
        "lm    2,6,96(7)\n\t"
        :
        : "r" (pStackLongs),
          "r" (pPT),
          "r" (overflow),
          "a" (invoke_copy_to_stack),
          "a" (method),
          "X" (dummy)
        : "2", "3", "4", "5", "6", "7", "memory"
    );
//  "basr  14,%8\n\t"

    (*(void (*)())method)();

    __asm__ __volatile__
     (
        "la    15,48(7)\n\t"

        "lr    %2,2\n\t"
        "lr    %3,3\n\t"
        "ler   %0,0\n\t"
        "ldr   %1,0\n\t"

        : "=f" (fret), "=f" (dret), "=r" (iret), "=r" (iret2)
    );

    switch( eReturnType )
    {
        case typelib_TypeClass_HYPER:
        case typelib_TypeClass_UNSIGNED_HYPER:
//              ((long*)pRegisterReturn)[0] = iret;
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
                *(float*)pRegisterReturn = fret;
            break;
        case typelib_TypeClass_DOUBLE:
            *(double*)pRegisterReturn = dret;
            break;
    }
}


//============================================================================
static void cpp_call(
        bridges::cpp_uno::shared::UnoInterfaceProxy * pThis,
        bridges::cpp_uno::shared::VtableSlot  aVtableSlot,
        typelib_TypeDescriptionReference * pReturnTypeRef,
    sal_Int32 nParams, typelib_MethodParameter * pParams,
    void * pUnoReturn, void * pUnoArgs[], uno_Any ** ppUnoExc )
{
      // max space for: [complex ret ptr], values|ptr ...
      char * pCppStack      =
          (char *)alloca( sizeof(sal_Int32) + ((nParams+2) * sizeof(sal_Int64)) );
      char * pCppStackStart = pCppStack;

        // need to know parameter types for callVirtualMethod so generate a signature string
        char * pParamType = (char *) alloca(nParams+2);
        char * pPT = pParamType;

    // return
    typelib_TypeDescription * pReturnTypeDescr = 0;
    TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );
    OSL_ENSURE( pReturnTypeDescr, "### expected return type description!" );

    void * pCppReturn = 0; // if != 0 && != pUnoReturn, needs reconversion

    if (pReturnTypeDescr)
    {
        if (bridges::cpp_uno::shared::isSimpleType( pReturnTypeDescr ))
        {
            pCppReturn = pUnoReturn; // direct way for simple types
        }
        else
        {
            // complex return via ptr
            pCppReturn = *(void **)pCppStack = (bridges::cpp_uno::shared::relatesToInterfaceType( pReturnTypeDescr )
                                                ? alloca( pReturnTypeDescr->nSize )
                                                : pUnoReturn); // direct way
                        *pPT++ = 'I'; //signify that a complex return type on stack
            pCppStack += sizeof(void *);
        }
    }
        // push "this" pointer
        void * pAdjustedThisPtr = reinterpret_cast< void ** >( pThis->getCppI() ) + aVtableSlot.offset;
    *(void**)pCppStack = pAdjustedThisPtr;
    pCppStack += sizeof( void* );
        *pPT++ = 'I';

    // stack space
    OSL_ENSURE( sizeof(void *) == sizeof(sal_Int32), "### unexpected size!" );
    // args
    void ** pCppArgs  = (void **)alloca( 3 * sizeof(void *) * nParams );
    // indizes of values this have to be converted (interface conversion cpp<=>uno)
    sal_Int32 * pTempIndizes = (sal_Int32 *)(pCppArgs + nParams);
    // type descriptions for reconversions
    typelib_TypeDescription ** ppTempParamTypeDescr = (typelib_TypeDescription **)(pCppArgs + (2 * nParams));

    sal_Int32 nTempIndizes   = 0;

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

                          // we need to know type of each param so that we know whether to use
                          // gpr or fpr to pass in parameters:
                          // Key: I - int, long, pointer, etc means pass in gpr
                          //      B - byte value passed in gpr
                          //      S - short value passed in gpr
                          //      F - float value pass in fpr
                          //      D - double value pass in fpr
                          //      H - long long int pass in proper pairs of gpr (3,4) (5,6), etc
                          //      X - indicates end of parameter description string

                  case typelib_TypeClass_LONG:
                  case typelib_TypeClass_UNSIGNED_LONG:
                  case typelib_TypeClass_ENUM:
                *pPT++ = 'I';
                break;
                   case typelib_TypeClass_SHORT:
                            *pPT++ = 'T';
                            break;
                  case typelib_TypeClass_CHAR:
                  case typelib_TypeClass_UNSIGNED_SHORT:
                            *pPT++ = 'S';
                            break;
                  case typelib_TypeClass_BOOLEAN:
                            *pPT++ = 'B';
                            break;
                  case typelib_TypeClass_BYTE:
                            *pPT++ = 'C';
                            break;
                  case typelib_TypeClass_FLOAT:
                            *pPT++ = 'F';
                break;
                case typelib_TypeClass_DOUBLE:
                *pPT++ = 'D';
                pCppStack += sizeof(sal_Int32); // extra long
                break;
            case typelib_TypeClass_HYPER:
            case typelib_TypeClass_UNSIGNED_HYPER:
                *pPT++ = 'H';
                pCppStack += sizeof(sal_Int32); // extra long
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
                    *(void **)pCppStack = pCppArgs[nPos] = alloca( pParamTypeDescr->nSize ),
                    pParamTypeDescr );
                pTempIndizes[nTempIndizes] = nPos; // default constructed for cpp call
                // will be released at reconversion
                ppTempParamTypeDescr[nTempIndizes++] = pParamTypeDescr;
            }
            // is in/inout
            else if (bridges::cpp_uno::shared::relatesToInterfaceType( pParamTypeDescr ))
            {
                uno_copyAndConvertData(
                    *(void **)pCppStack = pCppArgs[nPos] = alloca( pParamTypeDescr->nSize ),
                    pUnoArgs[nPos], pParamTypeDescr, pThis->getBridge()->getUno2Cpp() );

                pTempIndizes[nTempIndizes] = nPos; // has to be reconverted
                // will be released at reconversion
                ppTempParamTypeDescr[nTempIndizes++] = pParamTypeDescr;
            }
            else // direct way
            {
                *(void **)pCppStack = pCppArgs[nPos] = pUnoArgs[nPos];
                // no longer needed
                TYPELIB_DANGER_RELEASE( pParamTypeDescr );
            }
                        // KBH: FIXME: is this the right way to pass these
                        *pPT++='I';
        }
        pCppStack += sizeof(sal_Int32); // standard parameter length
    }

        // terminate the signature string
        *pPT++='X';
        *pPT=0;

    try
    {
        OSL_ENSURE( !( (pCppStack - pCppStackStart ) & 3), "UNALIGNED STACK !!! (Please DO panic)" );
        callVirtualMethod(
            pAdjustedThisPtr, aVtableSlot.index,
            pCppReturn, pReturnTypeDescr->eTypeClass, pParamType,
            (sal_Int32 *)pCppStackStart, (pCppStack - pCppStackStart) / sizeof(sal_Int32) );
        // NO exception occurred...
        *ppUnoExc = 0;

        // reconvert temporary params
        for ( ; nTempIndizes--; )
        {
            sal_Int32 nIndex = pTempIndizes[nTempIndizes];
            typelib_TypeDescription * pParamTypeDescr = ppTempParamTypeDescr[nTempIndizes];

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
        fillUnoException( CPPU_CURRENT_NAMESPACE::__cxa_get_globals()->caughtExceptions,
            *ppUnoExc, pThis->getBridge()->getCpp2Uno() );


        // temporary params
        for ( ; nTempIndizes--; )
        {
            sal_Int32 nIndex = pTempIndizes[nTempIndizes];
            // destroy temp cpp param => cpp: every param was constructed
            uno_destructData( pCppArgs[nIndex], ppTempParamTypeDescr[nTempIndizes], cpp_release );
            TYPELIB_DANGER_RELEASE( ppTempParamTypeDescr[nTempIndizes] );
        }
        // return type
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
#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr, "unoInterfaceProxyDispatch\n");
#endif


    // is my surrogate
        bridges::cpp_uno::shared::UnoInterfaceProxy * pThis
            = static_cast< bridges::cpp_uno::shared::UnoInterfaceProxy *> (pUnoI);
    typelib_InterfaceTypeDescription * pTypeDescr = pThis->pTypeDescr;

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
            OUString aVoidName( RTL_CONSTASCII_USTRINGPARAM("void") );
            typelib_typedescriptionreference_new(
                &pReturnTypeRef, typelib_TypeClass_VOID, aVoidName.pData );

            // dependent dispatch
                        aVtableSlot.index += 1; //get then set method
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
        } // else perform queryInterface()
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
            OUString( RTL_CONSTASCII_USTRINGPARAM("illegal member type description!") ),
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >() );

        Type const & rExcType = ::getCppuType( &aExc );
        // binary identical null reference
        ::uno_type_any_construct( *ppException, &aExc, rExcType.getTypeLibType(), 0 );
    }
    }
}

} } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
