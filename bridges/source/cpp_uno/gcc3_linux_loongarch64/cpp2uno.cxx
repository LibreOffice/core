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

#include <com/sun/star/uno/genfunc.hxx>
#include <sal/log.hxx>
#include <typelib/typedescription.hxx>
#include <uno/data.h>
#include <osl/endian.h>
#include "bridge.hxx"
#include "cppinterfaceproxy.hxx"
#include "types.hxx"
#include "vtablefactory.hxx"
#include "call.hxx"
#include "share.hxx"

#include <stdio.h>
#include <string.h>

using namespace com::sun::star::uno;


namespace CPPU_CURRENT_NAMESPACE
{
  bool is_complex_struct(const typelib_TypeDescription * type)
  {
      const typelib_CompoundTypeDescription * p
          = reinterpret_cast< const typelib_CompoundTypeDescription * >(type);
      for (sal_Int32 i = 0; i < p->nMembers; ++i)
      {
          if (p->ppTypeRefs[i]->eTypeClass == typelib_TypeClass_STRUCT ||
              p->ppTypeRefs[i]->eTypeClass == typelib_TypeClass_EXCEPTION)
          {
              typelib_TypeDescription * t = 0;
              TYPELIB_DANGER_GET(&t, p->ppTypeRefs[i]);
              bool b = is_complex_struct(t);
              TYPELIB_DANGER_RELEASE(t);
              if (b) {
                  return true;
              }
          }
          else if (!bridges::cpp_uno::shared::isSimpleType(p->ppTypeRefs[i]->eTypeClass))
              return true;
      }
      if (p->pBaseTypeDescription != 0)
          return is_complex_struct(&p->pBaseTypeDescription->aBase);
      return false;
  }

  bool return_in_hidden_param( typelib_TypeDescriptionReference *pTypeRef )
  {
      if (bridges::cpp_uno::shared::isSimpleType(pTypeRef))
          return false;
      else if (pTypeRef->eTypeClass == typelib_TypeClass_STRUCT ||
               pTypeRef->eTypeClass == typelib_TypeClass_EXCEPTION)
      {
          typelib_TypeDescription * pTypeDescr = 0;
          TYPELIB_DANGER_GET( &pTypeDescr, pTypeRef );

          //A Composite Type not larger than 16 bytes is returned in up to two GPRs
          bool bRet = pTypeDescr->nSize > 16 || is_complex_struct(pTypeDescr);

          TYPELIB_DANGER_RELEASE( pTypeDescr );
          return bRet;
      }
      return true;
  }
}

namespace
{

  static typelib_TypeClass cpp2uno_call(
      bridges::cpp_uno::shared::CppInterfaceProxy * pThis,
      const typelib_TypeDescription * pMemberTypeDescr,
      typelib_TypeDescriptionReference * pReturnTypeRef, // 0 indicates void return
      sal_Int32 nParams, typelib_MethodParameter * pParams,
      void ** gpreg, void ** fpreg, void ** ovrflw,
      sal_uInt64 * pRegisterReturn /* space for register return */ )
  {

    unsigned int nREG = 0;

    // return
    typelib_TypeDescription * pReturnTypeDescr = 0;
    if (pReturnTypeRef)
      TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );

    void * pUnoReturn = 0;
    void * pCppReturn = 0; // complex return ptr: if != 0 && != pUnoReturn, reconversion need

    if (pReturnTypeDescr)
    {
      if (CPPU_CURRENT_NAMESPACE::return_in_hidden_param( pReturnTypeRef ) )
      {
        pCppReturn = gpreg[nREG]; // complex return via ptr (pCppReturn)
        nREG++;

        pUnoReturn = ( bridges::cpp_uno::shared::relatesToInterfaceType( pReturnTypeDescr )
            ? alloca( pReturnTypeDescr->nSize )
            : pCppReturn); // direct way
      }
      else
      {
        pUnoReturn = pRegisterReturn; // direct way for simple types
      }
    }

    // pop this
    nREG++;

    // stack space
    static_assert(sizeof(void *) == sizeof(sal_Int64), "### unexpected size!");
    // parameters
    void ** pUnoArgs = (void **)alloca( 4 * sizeof(void *) * nParams );
    void ** pCppArgs = pUnoArgs + nParams;
    // indices of values this have to be converted (interface conversion cpp<=>uno)
    sal_Int32 * pTempIndices = (sal_Int32 *)(pUnoArgs + (2 * nParams));
    // type descriptions for reconversions
    typelib_TypeDescription ** ppTempParamTypeDescr = (typelib_TypeDescription **)(pUnoArgs + (3 * nParams));

    sal_Int32 nTempIndices   = 0;

    for ( sal_Int32 nPos = 0; nPos < nParams; ++nPos )
    {
      const typelib_MethodParameter & rParam = pParams[nPos];

      typelib_TypeDescription * pParamTypeDescr = 0;
      TYPELIB_DANGER_GET( &pParamTypeDescr, rParam.pTypeRef );

      if (!rParam.bOut && bridges::cpp_uno::shared::isSimpleType( pParamTypeDescr )) // value
      {
        switch (pParamTypeDescr->eTypeClass)
        {
          case typelib_TypeClass_FLOAT:
          case typelib_TypeClass_DOUBLE:
            if (nREG < MAX_FP_REGS) {
              pCppArgs[nPos] = &(fpreg[nREG]);
              pUnoArgs[nPos] = &(fpreg[nREG]);
            } else {
              pCppArgs[nPos] = &(ovrflw[nREG - MAX_FP_REGS]);
              pUnoArgs[nPos] = &(ovrflw[nREG - MAX_FP_REGS]);
            }
            nREG++;
            break;


          default:
            if (nREG < MAX_GP_REGS) {
              pCppArgs[nPos] = &(gpreg[nREG]);
              pUnoArgs[nPos] = &(gpreg[nREG]);
            } else {
              pCppArgs[nPos] = &(ovrflw[nREG - MAX_GP_REGS]);
              pUnoArgs[nPos] = &(ovrflw[nREG - MAX_GP_REGS]);
            }
            nREG++;
            break;

        }
        // no longer needed
        TYPELIB_DANGER_RELEASE( pParamTypeDescr );
      }
      else // ptr to complex value | ref
      {
        void *pCppStack;
        if (nREG < MAX_GP_REGS) {
          pCppArgs[nPos] = pCppStack = gpreg[nREG];
        } else {
          pCppArgs[nPos] = pCppStack = ovrflw[nREG - MAX_GP_REGS];
        }
        nREG++;

        if (! rParam.bIn) // is pure out
        {
          // uno out is unconstructed mem!
          pUnoArgs[nPos] = alloca( pParamTypeDescr->nSize );
          pTempIndices[nTempIndices] = nPos;
          // will be released at reconversion
          ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
        }
        // is in/inout
        else if (bridges::cpp_uno::shared::relatesToInterfaceType( pParamTypeDescr ))
        {
          uno_copyAndConvertData( pUnoArgs[nPos] = alloca( pParamTypeDescr->nSize ),
              pCppStack, pParamTypeDescr,
              pThis->getBridge()->getCpp2Uno() );
          pTempIndices[nTempIndices] = nPos; // has to be reconverted
          // will be released at reconversion
          ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
        }
        else // direct way
        {
          pUnoArgs[nPos] = pCppStack;
          // no longer needed
          TYPELIB_DANGER_RELEASE( pParamTypeDescr );
        }
      }
    }
    // ExceptionHolder
    uno_Any aUnoExc; // Any will be constructed by callee
    uno_Any * pUnoExc = &aUnoExc;

    // invoke uno dispatch call
    (*pThis->getUnoI()->pDispatcher)( pThis->getUnoI(), pMemberTypeDescr, pUnoReturn, pUnoArgs, &pUnoExc );
    // in case an exception occurred...
    if (pUnoExc)
    {
      // destruct temporary in/inout params
      for ( ; nTempIndices--; )
      {
        sal_Int32 nIndex = pTempIndices[nTempIndices];

        if (pParams[nIndex].bIn) // is in/inout => was constructed
          uno_destructData( pUnoArgs[nIndex], ppTempParamTypeDescr[nTempIndices], 0 );
        TYPELIB_DANGER_RELEASE( ppTempParamTypeDescr[nTempIndices] );
      }
      if (pReturnTypeDescr)
        TYPELIB_DANGER_RELEASE( pReturnTypeDescr );

      CPPU_CURRENT_NAMESPACE::raiseException( &aUnoExc, pThis->getBridge()->getUno2Cpp() );
      // has to destruct the any
      // is here for dummy
      return typelib_TypeClass_VOID;
    }
    else // else no exception occurred...
    {
      // temporary params
      for ( ; nTempIndices--; )
      {
        sal_Int32 nIndex = pTempIndices[nTempIndices];
        typelib_TypeDescription * pParamTypeDescr = ppTempParamTypeDescr[nTempIndices];

        if (pParams[nIndex].bOut) // inout/out
        {
          // convert and assign
          uno_destructData( pCppArgs[nIndex], pParamTypeDescr, cpp_release );
          uno_copyAndConvertData( pCppArgs[nIndex], pUnoArgs[nIndex], pParamTypeDescr,
              pThis->getBridge()->getUno2Cpp() );
        }
        // destroy temp uno param
        uno_destructData( pUnoArgs[nIndex], pParamTypeDescr, 0 );

        TYPELIB_DANGER_RELEASE( pParamTypeDescr );
      }
      // return
      if (pCppReturn) // has complex return
      {
        if (pUnoReturn != pCppReturn) // needs reconversion
        {
          uno_copyAndConvertData( pCppReturn, pUnoReturn, pReturnTypeDescr,
              pThis->getBridge()->getUno2Cpp() );
          // destroy temp uno return
          uno_destructData( pUnoReturn, pReturnTypeDescr, 0 );
        }
        // complex return ptr is set to return reg
        *(void **)pRegisterReturn = pCppReturn;
      }
      if (pReturnTypeDescr)
      {
        typelib_TypeClass eRet = (typelib_TypeClass)pReturnTypeDescr->eTypeClass;
        TYPELIB_DANGER_RELEASE( pReturnTypeDescr );
        return eRet;
      }
      else
        return typelib_TypeClass_VOID;
    }
  }


  /**
   * is called on incoming vtable calls
   * (called by asm snippets)
   */
  typelib_TypeClass cpp_vtable_call(
      sal_Int32 nFunctionIndex,
      sal_Int32 nVtableOffset,
      void ** gpreg, void ** fpreg, void ** ovrflw,
      sal_uInt64 * pRegisterReturn /* space for register return */ )
  {
    static_assert( sizeof(sal_Int64)==sizeof(void *), "### unexpected!" );

    // gpreg:  [ret *], this, [other gpr params]
    // fpreg:  [fpr params]
    // ovrflw: [gpr or fpr params (properly aligned)]
    void * pThis;
    if (nFunctionIndex & 0x80000000 )
    {
      nFunctionIndex &= 0x7fffffff;
      pThis = gpreg[1];
    }
    else
    {
      pThis = gpreg[0];
    }
    pThis = static_cast< char * >(pThis) - nVtableOffset;
    bridges::cpp_uno::shared::CppInterfaceProxy * pCppI =
        bridges::cpp_uno::shared::CppInterfaceProxy::castInterfaceToProxy( pThis );
    typelib_InterfaceTypeDescription * pTypeDescr = pCppI->getTypeDescr();

    if (nFunctionIndex >= pTypeDescr->nMapFunctionIndexToMemberIndex)
    {
      SAL_WARN(
          "bridges",
          "illegal " << OUString::unacquired(&pTypeDescr->aBase.pTypeName)
              << " vtable index " << nFunctionIndex << "/"
              << pTypeDescr->nMapFunctionIndexToMemberIndex);
      throw RuntimeException(
          ("illegal " + OUString::unacquired(&pTypeDescr->aBase.pTypeName)
           + " vtable index " + OUString::number(nFunctionIndex) + "/"
           + OUString::number(pTypeDescr->nMapFunctionIndexToMemberIndex)),
          (XInterface *)pThis);
    }

    // determine called method
    sal_Int32 nMemberPos = pTypeDescr->pMapFunctionIndexToMemberIndex[nFunctionIndex];
    assert(nMemberPos < pTypeDescr->nAllMembers);

    TypeDescription aMemberDescr( pTypeDescr->ppAllMembers[nMemberPos] );

    typelib_TypeClass eRet;
    switch (aMemberDescr.get()->eTypeClass)
    {
      case typelib_TypeClass_INTERFACE_ATTRIBUTE:
        {
          typelib_TypeDescriptionReference *pAttrTypeRef =
              reinterpret_cast<typelib_InterfaceAttributeTypeDescription *>( aMemberDescr.get() )->pAttributeTypeRef;

          if (pTypeDescr->pMapMemberIndexToFunctionIndex[nMemberPos] == nFunctionIndex)
          {
            // is GET method
            eRet = cpp2uno_call( pCppI, aMemberDescr.get(), pAttrTypeRef,
                0, 0, // no params
                gpreg, fpreg, ovrflw, pRegisterReturn );
          }
          else
          {
            // is SET method
            typelib_MethodParameter aParam;
            aParam.pTypeRef = pAttrTypeRef;
            aParam.bIn      = sal_True;
            aParam.bOut     = sal_False;

            eRet = cpp2uno_call( pCppI, aMemberDescr.get(),
                0, // indicates void return
                1, &aParam,
                gpreg, fpreg, ovrflw, pRegisterReturn );
          }
          break;
        }
      case typelib_TypeClass_INTERFACE_METHOD:
        {
          // is METHOD
          switch (nFunctionIndex)
          {
            case 1: // acquire()
              pCppI->acquireProxy(); // non virtual call!
              eRet = typelib_TypeClass_VOID;
              break;
            case 2: // release()
              pCppI->releaseProxy(); // non virtual call!
              eRet = typelib_TypeClass_VOID;
              break;
            case 0: // queryInterface() opt
              {
                typelib_TypeDescription * pTD = 0;
                TYPELIB_DANGER_GET( &pTD, reinterpret_cast< Type * >( gpreg[2] )->getTypeLibType() );
                if (pTD)
                {
                  XInterface * pInterface = 0;
                  (*pCppI->getBridge()->getCppEnv()->getRegisteredInterface)
                      ( pCppI->getBridge()->getCppEnv(),
                      (void **)&pInterface,
                      pCppI->getOid().pData,
                      reinterpret_cast<typelib_InterfaceTypeDescription *>( pTD ) );

                  if (pInterface)
                  {
                    ::uno_any_construct( reinterpret_cast< uno_Any * >( gpreg[0] ),
                                         &pInterface, pTD, cpp_acquire );

                    pInterface->release();
                    TYPELIB_DANGER_RELEASE( pTD );

                    reinterpret_cast<void **>( pRegisterReturn )[0] = gpreg[0];
                    eRet = typelib_TypeClass_ANY;
                    break;
                  }
                  TYPELIB_DANGER_RELEASE( pTD );
                }
              } // else perform queryInterface()
            default:
              typelib_InterfaceMethodTypeDescription *pMethodTD =
                  reinterpret_cast<typelib_InterfaceMethodTypeDescription *>( aMemberDescr.get() );

              eRet = cpp2uno_call( pCppI, aMemberDescr.get(),
                  pMethodTD->pReturnTypeRef,
                  pMethodTD->nParams,
                  pMethodTD->pParams,
                  gpreg, fpreg, ovrflw, pRegisterReturn );
          }
          break;
        }
      default:
        {
          throw RuntimeException( "no member description found!", (XInterface *)pThis );
        }
    }

    return eRet;
  }

  extern "C" void privateSnippetExecutor( ... );

  int const codeSnippetSize = 0x34;

  unsigned char *  codeSnippet( unsigned char * code,
      sal_Int32 functionIndex, sal_Int32 vtableOffset,
      bool bHasHiddenParam )
  {

    if ( bHasHiddenParam )
      functionIndex |= 0x80000000;

    unsigned int * p = (unsigned int *) code;

    assert((((unsigned long)code) & 0x3) == 0 );  //aligned to 4 otherwise a mistake

    /* generate this code */
    /*
       # index
        0:   14000012        lu12i.w $t6,0x0
        4:   34420000        ori     $t6,$t6,0x0
       # privateSnippetExecutor
        8:   14000014        lu12i.w $t8,0x0
        c:   03800294        ori     $t8,$t8,0x0
       10:   16000014        lu32i.d $t8,0x0
       14:   03000294        lu52i.d $t8,$t8,0x0
       # cpp_vtable_call
       18:   14000011        lu12i.w $t5,0x0
       1c:   03800231        ori     $t5,$t5,0x0
       20:   16000011        lu32i.d $t5,0x0
       24:   03000231        lu52i.d $t5,$t5,0x0
       # offset
       28:   14000013        lu12i.w $t7,0x0
       2c:   03800273        ori     $t7,$t7,0x0
       30:   4c000280        jr      $t8
     */

    * p++ = 0x14000012 | (((functionIndex>>12) & 0x000fffff) << 5);
    * p++ = 0x03800252 | ((functionIndex & 0x00000fff) << 10);
    * p++ = 0x14000014 | (((((unsigned long)privateSnippetExecutor) >> 12) & 0x000fffff) << 5);
    * p++ = 0x03800294 | ((((unsigned long)privateSnippetExecutor) & 0x00000fff) << 10);
    * p++ = 0x16000014 | (((((unsigned long)privateSnippetExecutor) >> 32) & 0x000fffff) << 5);
    * p++ = 0x03000294 | (((((unsigned long)privateSnippetExecutor) >> 52) & 0x00000fff) << 10);
    * p++ = 0x14000011 | (((((unsigned long)cpp_vtable_call) >> 12) & 0x000fffff) << 5);
    * p++ = 0x03800231 | ((((unsigned long)cpp_vtable_call) & 0x00000fff) << 10);
    * p++ = 0x16000011 | (((((unsigned long)cpp_vtable_call) >> 32) & 0x000fffff) << 5);
    * p++ = 0x03000231 | (((((unsigned long)cpp_vtable_call) >> 52) & 0x00000fff) << 10);
    * p++ = 0x14000013 | (((vtableOffset>>12) & 0x000fffff) << 5);
    * p++ = 0x03800273 | ((vtableOffset & 0x00000fff) << 10);
    * p++ = 0x4c000280;
    return (code + codeSnippetSize);

  }

}


void bridges::cpp_uno::shared::VtableFactory::flushCode(unsigned char const *bptr, unsigned char const *eptr)
{
    asm volatile("ibar 0":::);
}

struct bridges::cpp_uno::shared::VtableFactory::Slot { void * fn; };

bridges::cpp_uno::shared::VtableFactory::Slot *
bridges::cpp_uno::shared::VtableFactory::mapBlockToVtable(void * block)
{
    return static_cast< Slot * >(block) + 2;
}


std::size_t bridges::cpp_uno::shared::VtableFactory::getBlockSize(
    sal_Int32 slotCount)
{
    return (slotCount + 2) * sizeof (Slot) + slotCount * codeSnippetSize;
}

bridges::cpp_uno::shared::VtableFactory::Slot *
bridges::cpp_uno::shared::VtableFactory::initializeBlock(
    void * block, sal_Int32 slotCount, sal_Int32,
    typelib_InterfaceTypeDescription *)
{
    Slot * slots = mapBlockToVtable(block);
    slots[-2].fn = 0; //null
    slots[-1].fn = 0; //destructor
    return slots + slotCount;
}

unsigned char * bridges::cpp_uno::shared::VtableFactory::addLocalFunctions(
    Slot ** slots, unsigned char * code, sal_PtrDiff writetoexecdiff,
    typelib_InterfaceTypeDescription const * type, sal_Int32 functionOffset,
    sal_Int32 functionCount, sal_Int32 vtableOffset)
{
   (*slots) -= functionCount;
    Slot * s = *slots;

  for (sal_Int32 i = 0; i < type->nMembers; ++i) {
    typelib_TypeDescription * member = 0;
    TYPELIB_DANGER_GET(&member, type->ppMembers[i]);
    assert(member != 0);
    switch (member->eTypeClass) {
      case typelib_TypeClass_INTERFACE_ATTRIBUTE:
        // Getter:
        (s++)->fn = code + writetoexecdiff;
        code = codeSnippet(
            code, functionOffset++, vtableOffset,
            CPPU_CURRENT_NAMESPACE::return_in_hidden_param(
              reinterpret_cast<
              typelib_InterfaceAttributeTypeDescription * >(
                member)->pAttributeTypeRef));

        // Setter:
        if (!reinterpret_cast<
            typelib_InterfaceAttributeTypeDescription * >(
              member)->bReadOnly)
        {
          (s++)->fn = code + writetoexecdiff;
          code = codeSnippet(code, functionOffset++, vtableOffset, false);
        }
        break;

      case typelib_TypeClass_INTERFACE_METHOD:
        (s++)->fn = code + writetoexecdiff;
        code = codeSnippet(
            code, functionOffset++, vtableOffset,
            CPPU_CURRENT_NAMESPACE::return_in_hidden_param(
              reinterpret_cast<
              typelib_InterfaceMethodTypeDescription * >(
                member)->pReturnTypeRef));
        break;

      default:
        assert(false);
        break;
    }
    TYPELIB_DANGER_RELEASE(member);
  }
  return code;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
