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

#include <malloc.h>

#include <com/sun/star/uno/genfunc.hxx>
#include <uno/data.h>

#include "bridges/cpp_uno/shared/bridge.hxx"
#include "bridges/cpp_uno/shared/types.hxx"
#include "bridges/cpp_uno/shared/unointerfaceproxy.hxx"
#include "bridges/cpp_uno/shared/vtables.hxx"

#include "share.hxx"

//#define BRDEBUG
#ifdef BRDEBUG
#include <stdio.h>
#endif


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
      sal_Int32 /*nStackLongs*/)
  {

    // parameter list is mixed list of * and values
    // reference parameters are pointers

    unsigned long * mfunc;        // actual function to be invoked
    void (*ptr)();
    int gpr[4];                   // storage for gpregisters, map to a0-a3
    int off;                      // offset used to find function
    int nw;                       // number of words mapped
    long *p;                      // pointer to parameter overflow area
    int c;                        // character of parameter type being decoded
    int iret, iret2;              // temporary function return values

    // never called
    if (! pAdjustedThisPtr ) CPPU_CURRENT_NAMESPACE::dummy_can_throw_anything("xxx"); // address something

#ifdef BRDEBUG
    fprintf(stderr,"in CallVirtualMethod\n");
#endif

    // Because of the MIPS O32 calling conventions we could be passing
    // parameters in both register types and on the stack. To create the
    // stack parameter area we need we now simply allocate local
    // variable storage param[] that is at least the size of the parameter stack
    // (more than enough space) which we can overwrite the parameters into.

    /* p = sp - 512; new sp will be p - 16, but we don't change sp
     * at this time to avoid breaking ABI--not sure whether changing sp will break
     * references to local variables. For the same reason, we use absolute value.
     */
    __asm__ __volatile__ (
        "addiu $2,$29,-512\n\t"
        "move %0,$2\n\t"
        :"=r"(p): : "$2","$29" );

#ifdef BRDEBUG
     if (nStackLongs * 4 > 512 )
         fprintf(stderr,"too many arguments");
#endif

    // now begin to load the C++ function arguments into storage
    nw = 0;

    // now we need to parse the entire signature string */
    // until we get the END indicator */

    // treat complex return pointer like any other parameter

#ifdef BRDEBUG
    fprintf(stderr,"overflow area pointer p=%p\n",p);

    /* Let's figure out what is really going on here*/
    fprintf(stderr,"callVirtualMethod parameters string is %s\n",pPT);
    int k = nStackLongs;
    long * q = (long *)pStackLongs;
    while (k > 0) {
      fprintf(stderr,"uno stack is: %x\n",(unsigned int)*q);
      k--;
      q++;
    }
#endif

    /* parse the argument list up to the ending ) */
    while (*pPT != 'X') {
      c = *pPT;
      switch (c) {
        case 'D':                   /* type is double */
          /* treat the same as long long */
        case 'H':                /* type is long long */
          if (nw & 1) nw++;     /* note even elements gpr[] will map to
                               odd registers*/
          if (nw < 4) {
            gpr[nw++] = *pStackLongs;
            gpr[nw++] = *(pStackLongs+1);
          } else {
            if (((long) p) & 4)
              p++;
            *p++ = *pStackLongs;
            *p++ = *(pStackLongs+1);
          }
          pStackLongs += 2;
          break;

        case 'S':
          if (nw < 4) {
            gpr[nw++] = *((unsigned short*)pStackLongs);
          } else {
            *p++ = *((unsigned short *)pStackLongs);
          }
          pStackLongs += 1;
          break;

        case 'B':
          if (nw < 4) {
            gpr[nw++] = *((char *)pStackLongs);
          } else {
            *p++ = *((char *)pStackLongs);
          }
          pStackLongs += 1;
          break;

        default:
          if (nw < 4) {
            gpr[nw++] = *pStackLongs;
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
    ptr = (void (*)())mfunc;

#ifdef BRDEBUG
    fprintf(stderr,"calling function %p\n",mfunc);
#endif

    /* Set up the machine registers and invoke the function */

    __asm__ __volatile__ (
        "lw $4, 0(%0)\n\t"
        "lw $5, 4(%0)\n\t"
        "lw $6, 8(%0)\n\t"
        "lw $7, 12(%0)\n\t"
        : : "r" (gpr)
        : "$4", "$5", "$6", "$7"
        );

    __asm__ __volatile__ ("addiu $29,$29,-528\r\n":::"$29");

    (*ptr)();

    __asm__ __volatile__ ("addiu $29,$29,528\r\n":::"$29");

    __asm__ __volatile__ (
        "sw $2,%0 \n\t"
        "sw $3,%1 \n\t"
        : "=m" (iret), "=m" (iret2) : );
    register float fret asm("$f0");
    register double dret asm("$f0");

    switch( eReturnType )
    {
      case typelib_TypeClass_HYPER:
      case typelib_TypeClass_UNSIGNED_HYPER:
          ((long*)pRegisterReturn)[1] = iret2;  // fall through
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
    // max space for: [complex ret ptr], values|ptr ...
    char * pCppStack        =
      (char *)alloca( sizeof(sal_Int32) + ((nParams+2) * sizeof(sal_Int64)) );
    char * pCppStackStart   = pCppStack;

    // need to know parameter types for callVirtualMethod so generate a signature string
    char * pParamType = (char *) alloca(nParams+2);
    char * pPT = pParamType;

#ifdef BRDEBUG
  fprintf(stderr,"in cpp_call\n");
#endif

    // return
    typelib_TypeDescription * pReturnTypeDescr = 0;
    TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );
    // assert(pReturnTypeDescr);

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
        pCppReturn = *(void **)pCppStack =
          (bridges::cpp_uno::shared::relatesToInterfaceType( pReturnTypeDescr )
           ? alloca( pReturnTypeDescr->nSize ): pUnoReturn); // direct way
        *pPT++ = 'I'; //signify that a complex return type on stack
        pCppStack += sizeof(void *);
      }
    }
    // push this
    void* pAdjustedThisPtr = reinterpret_cast< void **>(pThis->getCppI()) + aVtableSlot.offset;
    *(void**)pCppStack = pAdjustedThisPtr;
    pCppStack += sizeof( void* );
    *pPT++ = 'I';

    // stack space
    // static_assert(sizeof(void *) == sizeof(sal_Int32), "### unexpected size!");
    // args
    void ** pCppArgs  = (void **)alloca( 3 * sizeof(void *) * nParams );
    // indices of values this have to be converted (interface conversion cpp<=>uno)
    sal_Int32 * pTempIndices = (sal_Int32 *)(pCppArgs + nParams);
    // type descriptions for reconversions
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
              *(void **)pCppStack = pCppArgs[nPos] = alloca( pParamTypeDescr->nSize ),
              pParamTypeDescr );
          pTempIndices[nTempIndices] = nPos; // default constructed for cpp call
          // will be released at reconversion
          ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
        }
        // is in/inout
        else if (bridges::cpp_uno::shared::relatesToInterfaceType( pParamTypeDescr ))
        {
          uno_copyAndConvertData(
              *(void **)pCppStack = pCppArgs[nPos] = alloca( pParamTypeDescr->nSize ),
              pUnoArgs[nPos], pParamTypeDescr,
              pThis->getBridge()->getUno2Cpp() );

          pTempIndices[nTempIndices] = nPos; // has to be reconverted
          // will be released at reconversion
          ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
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
      assert( !( (pCppStack - pCppStackStart ) & 3) && "UNALIGNED STACK !!! (Please DO panic)" );
      callVirtualMethod(
          pAdjustedThisPtr, aVtableSlot.index,
          pCppReturn, pReturnTypeDescr->eTypeClass, pParamType,
          (sal_Int32 *)pCppStackStart, (pCppStack - pCppStackStart) / sizeof(sal_Int32) );
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
      fillUnoException( CPPU_CURRENT_NAMESPACE::__cxa_get_globals()->caughtExceptions,
          *ppUnoExc, pThis->getBridge()->getCpp2Uno() );

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


namespace bridges { namespace cpp_uno { namespace shared {

void unoInterfaceProxyDispatch(
    uno_Interface * pUnoI, const typelib_TypeDescription * pMemberDescr,
    void * pReturn, void * pArgs[], uno_Any ** ppException )
{
  // is my surrogate
  bridges::cpp_uno::shared::UnoInterfaceProxy * pThis
    = static_cast< bridges::cpp_uno::shared::UnoInterfaceProxy *> (pUnoI);
  //typelib_InterfaceTypeDescription * pTypeDescr = pThis->pTypeDescr;

#ifdef BRDEBUG
  fprintf(stderr,"in dispatch\n");
#endif

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
          aParam.bIn        = sal_True;
          aParam.bOut       = sal_False;

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

        Type const & rExcType = cppu::UnoType<decltype(aExc)>::get();
        // binary identical null reference
        ::uno_type_any_construct( *ppException, &aExc, rExcType.getTypeLibType(), 0 );
      }
  }
}
}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
