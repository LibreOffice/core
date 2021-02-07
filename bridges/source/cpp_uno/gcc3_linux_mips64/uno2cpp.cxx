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

#include <sal/config.h>

#include <exception>
#include <malloc.h>
#include <cstring>
#include <typeinfo>

#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/genfunc.hxx>
#include <o3tl/runtimetooustring.hxx>
#include <uno/data.h>

#include "bridge.hxx"
#include "types.hxx"
#include "unointerfaceproxy.hxx"
#include "vtables.hxx"

#include "share.hxx"

//#define BRDEBUG
#ifdef BRDEBUG
#include <stdio.h>
#endif

#define INSERT_FLOAT_DOUBLE( pSV, nr, pFPR, pDS ) \
    if ( nr < MAX_FP_REGS ) \
        pFPR[nr++] = *reinterpret_cast<double *>( pSV ); \
    else \
        *pDS++ = *reinterpret_cast<sal_uInt64 *>( pSV ); // verbatim!

#define INSERT_INT64( pSV, nr, pGPR, pDS ) \
    if ( nr < MAX_GP_REGS ) \
        pGPR[nr++] = *reinterpret_cast<sal_Int64 *>( pSV ); \
    else \
        *pDS++ = *reinterpret_cast<sal_Int64 *>( pSV );

#define INSERT_INT32( pSV, nr, pGPR, pDS ) \
    if ( nr < MAX_GP_REGS ) \
        pGPR[nr++] = *reinterpret_cast<sal_Int32 *>( pSV ); \
    else \
        *pDS++ = *reinterpret_cast<sal_Int32 *>( pSV );

#define INSERT_INT16( pSV, nr, pGPR, pDS ) \
    if ( nr < MAX_GP_REGS ) \
        pGPR[nr++] = *reinterpret_cast<sal_Int16 *>( pSV ); \
    else \
        *pDS++ = *reinterpret_cast<sal_Int16 *>( pSV );

#define INSERT_UINT16( pSV, nr, pGPR, pDS ) \
    if ( nr < MAX_GP_REGS ) \
        pGPR[nr++] = *reinterpret_cast<sal_uInt16 *>( pSV ); \
    else \
        *pDS++ = *reinterpret_cast<sal_uInt16 *>( pSV );

#define INSERT_INT8( pSV, nr, pGPR, pDS ) \
    if ( nr < MAX_GP_REGS ) \
        pGPR[nr++] = *reinterpret_cast<sal_Int8 *>( pSV ); \
    else \
        *pDS++ = *reinterpret_cast<sal_Int8 *>( pSV );

using namespace ::com::sun::star::uno;

namespace
{

  bool isReturnInFPR(const typelib_TypeDescription * pTypeDescr, sal_uInt32 & nSize)
  {
      const typelib_CompoundTypeDescription *p =
                  reinterpret_cast<const typelib_CompoundTypeDescription*>( pTypeDescr );

      for (sal_Int32 i = 0; i < p->nMembers; ++i)
      {
          typelib_TypeDescriptionReference *pTypeInStruct = p->ppTypeRefs[ i ];

          switch (pTypeInStruct->eTypeClass)
          {
          case typelib_TypeClass_STRUCT:
          case typelib_TypeClass_EXCEPTION:
              {
                  typelib_TypeDescription * t = 0;
                  TYPELIB_DANGER_GET(&t, pTypeInStruct);
                  bool isFPR = isReturnInFPR(t, nSize);
                  TYPELIB_DANGER_RELEASE(t);
                  if (!isFPR)
                    return false;
              }
              break;
          case typelib_TypeClass_FLOAT:
          case typelib_TypeClass_DOUBLE:
              if (nSize >= 16)
                return false;
              nSize += 8;
              break;
          default:
              return false;
          }
      }
      return true;
  }

  void fillReturn(const typelib_TypeDescription * pTypeDescr,
                    sal_Int64 * gret, double * fret, void * pRegisterReturn)
  {
      sal_uInt32 nSize = 0;
      if (isReturnInFPR(pTypeDescr, nSize))
      {
          reinterpret_cast<double *>( pRegisterReturn )[0] = fret[0];
          reinterpret_cast<double *>( pRegisterReturn )[1] = fret[1];
      }
      else
      {
          reinterpret_cast<sal_Int64 *>( pRegisterReturn )[0] = gret[0];
          reinterpret_cast<sal_Int64 *>( pRegisterReturn )[1] = gret[1];
      }
  }

  static void callVirtualMethod(
      void * pAdjustedThisPtr,
      sal_Int32 nVtableIndex,
      void * pRegisterReturn,
      typelib_TypeDescriptionReference * pReturnTypeRef,
      bool bSimpleReturn,
      sal_uInt64 *pStack,
      sal_uInt32 nStack,
      sal_uInt64 *pGPR,
      double *pFPR,
      sal_uInt32 nREG)
  {
    // Should not happen, but...
    static_assert(MAX_GP_REGS == MAX_FP_REGS, "must be the same size");
    if ( nREG > MAX_GP_REGS )
        nREG = MAX_GP_REGS;

    // Get pointer to method
    sal_uInt64 pMethod = *((sal_uInt64 *)pAdjustedThisPtr);
    pMethod += 8 * nVtableIndex;
    void *mfunc = (void *) *((sal_uInt64 *)pMethod);
#ifdef BRDEBUG
    fprintf(stderr, "calling function %p\n", mfunc);
#endif

    // Load parameters to stack, if necessary
    sal_uInt64* pCallStack = NULL;
    if ( nStack )
    {
        // 16-bytes aligned
        sal_uInt32 nStackBytes = ( ( nStack + 1 ) >> 1 ) * 16;
        pCallStack = (sal_uInt64 *) __builtin_alloca( nStackBytes );
        std::memcpy( pCallStack, pStack, nStackBytes );
    }

    sal_Int64 gret[2];
    double fret[2];
    asm volatile (
        ".set push \n\t"
        ".set mips64 \n\t"
        // Fill the general purpose registers
        "ld $4, 0(%[gpr]) \n\t"
        "ld $5, 8(%[gpr]) \n\t"
        "ld $6, 16(%[gpr]) \n\t"
        "ld $7, 24(%[gpr]) \n\t"
        "ld $8, 32(%[gpr]) \n\t"
        "ld $9, 40(%[gpr]) \n\t"
        "ld $10, 48(%[gpr]) \n\t"
        "ld $11, 56(%[gpr]) \n\t"
        // Fill the floating pointer registers
        "ldc1 $f12, 0(%[fpr]) \n\t"
        "ldc1 $f13, 8(%[fpr]) \n\t"
        "ldc1 $f14, 16(%[fpr]) \n\t"
        "ldc1 $f15, 24(%[fpr]) \n\t"
        "ldc1 $f16, 32(%[fpr]) \n\t"
        "ldc1 $f17, 40(%[fpr]) \n\t"
        "ldc1 $f18, 48(%[fpr]) \n\t"
        "ldc1 $f19, 56(%[fpr]) \n\t"
        // Perform the call
        "jalr %[mfunc] \n\t"
        // Fill the return values
        "move %[gret1], $2 \n\t"
        "move %[gret2], $3 \n\t"
        "mov.d %[fret1], $f0 \n\t"
        "mov.d %[fret2], $f2 \n\t"
        ".set pop \n\t"
        :[gret1]"=r"(gret[0]), [gret2]"=r"(gret[1]),
         [fret1]"=f"(fret[0]), [fret2]"=f"(fret[1])
        :[gpr]"r"(pGPR), [fpr]"r"(pFPR), [mfunc]"c"(mfunc),
         [stack]"m"(pCallStack) // dummy input to prevent the compiler from optimizing the alloca out
        :"$2", "$3", "$4", "$5", "$6", "$7", "$8",
         "$9", "$10", "$11", "$31",
         "$f0", "$f2", "$f12", "$f13", "$f14", "$f15",
         "$f16", "$f17", "$f18", "$f19", "memory"
    );

    switch (pReturnTypeRef->eTypeClass)
    {
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
    case typelib_TypeClass_ENUM:
    case typelib_TypeClass_CHAR:
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
    case typelib_TypeClass_BOOLEAN:
    case typelib_TypeClass_BYTE:
        *reinterpret_cast<sal_Int64 *>( pRegisterReturn ) = gret[0];
        break;
    case typelib_TypeClass_FLOAT:
    case typelib_TypeClass_DOUBLE:
        *reinterpret_cast<double *>( pRegisterReturn ) = fret[0];
        break;
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
        {
            sal_Int32 const nRetSize = pReturnTypeRef->pType->nSize;
            if (bSimpleReturn && nRetSize <= 16 && nRetSize > 0)
            {
                typelib_TypeDescription * pTypeDescr = 0;
                TYPELIB_DANGER_GET( &pTypeDescr, pReturnTypeRef );
                fillReturn(pTypeDescr, gret, fret, pRegisterReturn);
                TYPELIB_DANGER_RELEASE( pTypeDescr );
            }
            break;
        }
    default:
#ifdef BRDEBUG
        fprintf(stderr,"unhandled return type %u\n", pReturnTypeRef->eTypeClass);
#endif
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
    sal_uInt64 *pStack = (sal_uInt64 *)__builtin_alloca( ((nParams+3) * sizeof(sal_Int64)) );
    sal_uInt64 *pStackStart = pStack;

    sal_uInt64 pGPR[MAX_GP_REGS];
    double pFPR[MAX_FP_REGS];
    sal_uInt32 nREG = 0;

#ifdef BRDEBUG
  fprintf(stderr, "in cpp_call\n");
#endif

    // return
    typelib_TypeDescription * pReturnTypeDescr = 0;
    TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );
    assert(pReturnTypeDescr);

    void * pCppReturn = 0; // if != 0 && != pUnoReturn, needs reconversion

    bool bSimpleReturn = true;
    if (pReturnTypeDescr)
    {
      if ( CPPU_CURRENT_NAMESPACE::return_in_hidden_param( pReturnTypeRef ) )
      {
        bSimpleReturn = false;
        // complex return via ptr
        pCppReturn = bridges::cpp_uno::shared::relatesToInterfaceType( pReturnTypeDescr )?
                     __builtin_alloca( pReturnTypeDescr->nSize ) : pUnoReturn;
        INSERT_INT64( &pCppReturn, nREG, pGPR, pStack );
      }
      else
      {
        pCppReturn = pUnoReturn; // direct way for simple types
      }
    }

    // push this
    void* pAdjustedThisPtr = reinterpret_cast< void **>( pThis->getCppI() ) + aVtableSlot.offset;
    INSERT_INT64( &pAdjustedThisPtr, nREG, pGPR, pStack );

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
        uno_copyAndConvertData( pCppArgs[nPos] = alloca( 8 ), pUnoArgs[nPos], pParamTypeDescr,
            pThis->getBridge()->getUno2Cpp() );

        switch (pParamTypeDescr->eTypeClass)
        {
          case typelib_TypeClass_LONG:
          case typelib_TypeClass_UNSIGNED_LONG:
          case typelib_TypeClass_ENUM:
            INSERT_INT32( pCppArgs[nPos], nREG, pGPR, pStack );
            break;
          case typelib_TypeClass_CHAR:
          case typelib_TypeClass_SHORT:
            INSERT_INT16( pCppArgs[nPos], nREG, pGPR, pStack );
            break;
          case typelib_TypeClass_UNSIGNED_SHORT:
            INSERT_UINT16( pCppArgs[nPos], nREG, pGPR, pStack );
            break;
          case typelib_TypeClass_BOOLEAN:
          case typelib_TypeClass_BYTE:
            INSERT_INT8( pCppArgs[nPos], nREG, pGPR, pStack );
            break;
          case typelib_TypeClass_FLOAT:
          case typelib_TypeClass_DOUBLE:
            INSERT_FLOAT_DOUBLE( pCppArgs[nPos], nREG, pFPR, pStack );
            break;
          case typelib_TypeClass_HYPER:
          case typelib_TypeClass_UNSIGNED_HYPER:
            INSERT_INT64( pCppArgs[nPos], nREG, pGPR, pStack );
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
              pCppArgs[nPos] = alloca( pParamTypeDescr->nSize ),
              pParamTypeDescr );
          pTempIndices[nTempIndices] = nPos; // default constructed for cpp call
          // will be released at reconversion
          ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
        }
        // is in/inout
        else if (bridges::cpp_uno::shared::relatesToInterfaceType( pParamTypeDescr ))
        {
          uno_copyAndConvertData(
              pCppArgs[nPos] = alloca( pParamTypeDescr->nSize ),
              pUnoArgs[nPos], pParamTypeDescr, pThis->getBridge()->getUno2Cpp() );

          pTempIndices[nTempIndices] = nPos; // has to be reconverted
          // will be released at reconversion
          ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
        }
        else // direct way
        {
          pCppArgs[nPos] = pUnoArgs[nPos];
          // no longer needed
          TYPELIB_DANGER_RELEASE( pParamTypeDescr );
        }
        INSERT_INT64( &(pCppArgs[nPos]), nREG, pGPR, pStack );
      }
    }

    try
    {
      try {
          callVirtualMethod(
              pAdjustedThisPtr, aVtableSlot.index,
              pCppReturn, pReturnTypeRef, bSimpleReturn,
              pStackStart, ( pStack - pStackStart ),
              pGPR, pFPR, nREG);
      } catch (css::uno::Exception &) {
          throw;
      } catch (std::exception & e) {
          throw css::uno::RuntimeException(
              "C++ code threw " + o3tl::runtimeToOUString(typeid(e).name()) + ": "
              + o3tl::runtimeToOUString(e.what()));
      } catch (...) {
          throw css::uno::RuntimeException("C++ code threw unknown exception");
      }
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
      CPPU_CURRENT_NAMESPACE::fillUnoException(*ppUnoExc, pThis->getBridge()->getCpp2Uno());

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


namespace bridges::cpp_uno::shared {

void unoInterfaceProxyDispatch(
    uno_Interface * pUnoI, const typelib_TypeDescription * pMemberDescr,
    void * pReturn, void * pArgs[], uno_Any ** ppException )
{
  // is my surrogate
  bridges::cpp_uno::shared::UnoInterfaceProxy * pThis
    = static_cast< bridges::cpp_uno::shared::UnoInterfaceProxy *> (pUnoI);
  //typelib_InterfaceTypeDescription * pTypeDescr = pThis->pTypeDescr;

#ifdef BRDEBUG
  fprintf(stderr, "in dispatch\n");
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
                (*pThis->pBridge->getUnoEnv()->getRegisteredInterface)(pThis->pBridge->getUnoEnv(),
                                                                       (void **)&pInterface, pThis->oid.pData,
                                                                       (typelib_InterfaceTypeDescription *)pTD );

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
            "illegal member type description!",
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >() );

        Type const & rExcType = cppu::UnoType<decltype(aExc)>::get();
        // binary identical null reference
        ::uno_type_any_construct( *ppException, &aExc, rExcType.getTypeLibType(), 0 );
      }
  }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
