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


#include <sys/types.h>
#include <sys/malloc.h>

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

//==================================================================================================
static void callVirtualMethod(
    void * pAdjustedThisPtr,
    sal_Int32 nVtableIndex,
    void * pRegisterReturn,
    typelib_TypeClass eReturnType,
    char * pPT,
    sal_Int32 * pStackLongs,
    sal_Int32 /* nStackLongs */)
{

  // parameter list is mixed list of * and values
  // reference parameters are pointers

  // the basic idea here is to use gpr[8] as a storage area for
  // the future values of registers r3 to r10 needed for the call,
  // and similarly fpr[13] as a storage area for the future values
  // of floating point registers f1 to f13

     unsigned long * mfunc;        // actual function to be invoked
     int gpr[8];                   // storage for gpregisters, map to r3-r10
     int off;                      // offset used to find function
     double fpr[13];               // storage for fpregisters, map to f1-f13
     int n;                        // number of gprs mapped so far
     int f;                        // number of fprs mapped so far
     volatile long *p;             // pointer to parameter overflow area
     int c;                        // character of parameter type being decoded
     volatile double dret;         // temporary function return values
     volatile int iret, iret2;

     // Because of the Power PC calling conventions we could be passing
     // parameters in both register types and on the stack. To create the
     // stack parameter area we need we now simply allocate local
     // variable storage param[] that is at least the size of the parameter stack
     // (more than enough space) which we can overwrite the parameters into.

     // Note: This keeps us from having to decode the signature twice and
     // prevents problems with later local variables.

     // FIXME: I do not believe the following is true but we will keep the
     // FIXME: extra space just to be safe until proven otherwise

     // Note: could require up to  2*nStackLongs words of parameter stack area
     // if the call has many float parameters (i.e. floats take up only 1
     // word on the stack but take 2 words in parameter area in the
     // stack frame .


     // unsigned long param[(2*nStackLongs)];

     /* now begin to load the C++ function arguments into storage */
     n = 0;
     f = 0;


     /* set up a pointer to the stack parameter area */
     __asm__ ( "addi %0,r1,24" : "=r" (p) : /* no inputs */ );

     // #i94421#, work around compiler error:
     volatile long * pCopy = p;
     (void) pCopy; // avoid warning about unused variable

     // never called
     // if (! pAdjustedThisPtr )CPPU_CURRENT_NAMESPACE::dummy_can_throw_anything("xxx"); // address something


     // now we need to parse the entire signature string
     // until we get the END indicator

     // treat complex return pointer like any other parameter

     // parse the argument list up to the ending )

     while (*pPT != 'X') {
       c = *pPT;
       switch (c) {

       case 'D':                   /* type is double */
            if (f < 13) {
               fpr[f++] = *((double *)pStackLongs);   /* store in register */
               n+=2;
               p+=2;
            } else {
               *p++ = *pStackLongs;       /* or on the parameter stack */
               *p++ = *(pStackLongs + 1);
            }
            pStackLongs += 2;
            break;

       case 'F':                   /* type is float */
         /* floats are stored as 1 32 bit word on param stack */
            if (f < 13) {
               fpr[f++] = *((float *)pStackLongs);
               n+=1;
           p++;
            } else {
               *((float *)p) = *((float *)pStackLongs);
               p += 1;
            }
            pStackLongs += 1;
            break;

       case 'H':                /* type is long long */
            if (n < 8)
            {
               gpr[n++] = *pStackLongs;
               p++;
            }
        else
               *p++ = *pStackLongs;
            if(n < 8)
            {
               gpr[n++] = *(pStackLongs+1);
               p++;
            }
            else
               *p++ = *(pStackLongs+1);
            pStackLongs += 2;
            break;

       case 'S':
            if (n < 8) {
               gpr[n++] = *((unsigned short*)pStackLongs);
               p++;
            } else {
               *p++ = *((unsigned short *)pStackLongs);
            }
            pStackLongs += 1;
            break;

       case 'B':
            if (n < 8) {
               gpr[n++] = *((char *)pStackLongs);
               p++;
            } else {
               *p++ = *((char *)pStackLongs);
            }
            pStackLongs += 1;
            break;

       default:
            if (n < 8) {
               gpr[n++] = *pStackLongs;
               p++;
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
     off = off * 4;                         // 4 bytes per slot
     mfunc = *((unsigned long **)pAdjustedThisPtr);    // get the address of the vtable
     mfunc = (unsigned long *)((char *)mfunc + off); // get the address from the vtable entry at offset
     mfunc = *((unsigned long **)mfunc);                 // the function is stored at the address
     typedef void (*FunctionCall)(sal_uInt32, sal_uInt32, sal_uInt32, sal_uInt32, sal_uInt32, sal_uInt32, sal_uInt32, sal_uInt32);
     FunctionCall ptr = (FunctionCall)mfunc;

    /* Set up the machine registers and invoke the function */

    __asm__ __volatile__ (
                "lwz     r3,      0(%0)\n\t"
                "lwz     r4,      4(%0)\n\t"
                "lwz     r5,      8(%0)\n\t"
                "lwz     r6,      12(%0)\n\t"
                "lwz     r7,      16(%0)\n\t"
                "lwz     r8,      20(%0)\n\t"
                "lwz     r9,      24(%0)\n\t"
                "lwz     r10,     28(%0)\n\t"
                "lfd     f1,      0(%1)\n\t"
                "lfd     f2,      8(%1)\n\t"
                "lfd     f3,      16(%1)\n\t"
                "lfd     f4,      24(%1)\n\t"
                "lfd     f5,      32(%1)\n\t"
                "lfd     f6,      40(%1)\n\t"
                "lfd     f7,      48(%1)\n\t"
                "lfd     f8,      56(%1)\n\t"
                "lfd     f9,      64(%1)\n\t"
                "lfd     f10,     72(%1)\n\t"
                "lfd     f11,     80(%1)\n\t"
                "lfd     f12,     88(%1)\n\t"
                "lfd     f13,     96(%1)\n\t"
          : : "r" (gpr), "r" (fpr)
          : "r0", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10",
            "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9",
            "f10", "f11", "f12", "f13"
    );

    // tell gcc that r3 to r10 are not available to it for doing the TOC and exception munge on the func call
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
                "stw     r3,      %1\n\t"
                "stw     r4,      %2\n\t"
                "stfd    f1,      %0\n\t"
        : : "m" (dret), "m" (iret), "m" (iret2)
    );


    switch( eReturnType )
    {
        case typelib_TypeClass_HYPER:
        case typelib_TypeClass_UNSIGNED_HYPER:
            ((long*)pRegisterReturn)[1] = iret2;
                        // fall thru on purpose
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
                *(float*)pRegisterReturn = (float)dret;
            break;

        case typelib_TypeClass_DOUBLE:
            *(double*)pRegisterReturn = dret;
            break;
        default:
            break;
    }
}


//==================================================================================================
static void cpp_call(
    bridges::cpp_uno::shared::UnoInterfaceProxy * pThis,
    bridges::cpp_uno::shared::VtableSlot aVtableSlot,
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
            pCppReturn = *(void **)pCppStack
                           = (bridges::cpp_uno::shared::relatesToInterfaceType( pReturnTypeDescr )
               ? alloca( pReturnTypeDescr->nSize )
               : pUnoReturn); // direct way
                        *pPT++ = 'C'; //signify that a complex return type on stack
            pCppStack += sizeof(void *);
        }
    }
    // push this
        void * pAdjustedThisPtr = reinterpret_cast< void ** >(pThis->getCppI())
      + aVtableSlot.offset;
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

        if (!rParam.bOut
                    && bridges::cpp_uno::shared::isSimpleType( pParamTypeDescr ))
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
                pCppStack += sizeof(sal_Int32); // extra long
                break;
            case typelib_TypeClass_HYPER:
            case typelib_TypeClass_UNSIGNED_HYPER:
                *pPT++ = 'H';
                pCppStack += sizeof(sal_Int32); // extra long
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
        fillUnoException( CPPU_CURRENT_NAMESPACE::__cxa_get_globals()->caughtExceptions, *ppUnoExc, pThis->getBridge()->getCpp2Uno() );

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
    // is my surrogate
    bridges::cpp_uno::shared::UnoInterfaceProxy * pThis
           = static_cast< bridges::cpp_uno::shared::UnoInterfaceProxy * > (pUnoI);
        //  typelib_InterfaceTypeDescription * pTypeDescr = pThis->pTypeDescr;

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
            OUString aVoidName("void");
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
            OUString("illegal member type description!"),
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >() );

        Type const & rExcType = ::getCppuType( &aExc );
        // binary identical null reference
        ::uno_type_any_construct( *ppException, &aExc, rExcType.getTypeLibType(), 0 );
    }
    }
}

} } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
