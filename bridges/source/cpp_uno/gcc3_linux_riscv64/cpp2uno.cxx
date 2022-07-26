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

//#define BRDEBUG

#ifdef BRDEBUG
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
using namespace ::std;
using namespace ::osl;
using namespace ::rtl;
#endif

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
    /*  Most MIPS ABIs view the arguments as a struct, of which the
        first N words go in registers and the rest go on the stack.  If I < N, the
        With word might go in With integer argument register or the With
        floating-point one.  For these ABIs, we only need to remember the number
        of words passed so far.  We are interested only in n64 ABI,so it is the
        case.
        */
    unsigned int nREG = 0;

#ifdef BRDEBUG
    fprintf(stdout, "cpp2uno_call:begin\n");
#endif
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
#ifdef BRDEBUG
        fprintf(stdout, "cpp2uno_call:complexreturn\n");
#endif
      }
      else
      {
        pUnoReturn = pRegisterReturn; // direct way for simple types
#ifdef BRDEBUG
        fprintf(stdout, "cpp2uno_call:simplereturn\n");
#endif
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

#ifdef BRDEBUG
    fprintf(stdout, "cpp2uno_call:nParams=%d\n", nParams);
#endif
    for ( sal_Int32 nPos = 0; nPos < nParams; ++nPos )
    {
      const typelib_MethodParameter & rParam = pParams[nPos];

      typelib_TypeDescription * pParamTypeDescr = 0;
      TYPELIB_DANGER_GET( &pParamTypeDescr, rParam.pTypeRef );

      if (!rParam.bOut && bridges::cpp_uno::shared::isSimpleType( pParamTypeDescr )) // value
      {
#ifdef BRDEBUG
        fprintf(stdout, "cpp2uno_call:Param %u, type %u\n", nPos, pParamTypeDescr->eTypeClass);
#endif
        switch (pParamTypeDescr->eTypeClass)
        {
          case typelib_TypeClass_FLOAT:
          case typelib_TypeClass_DOUBLE:
            if (nREG < MAX_FP_REGS) {
#ifdef BRDEBUG
              fprintf(stdout, "cpp2uno_call:fpr=%p\n", fpreg[nREG]);
#endif
              pCppArgs[nPos] = &(fpreg[nREG]);
              pUnoArgs[nPos] = &(fpreg[nREG]);
            } else {
#ifdef BRDEBUG
              fprintf(stdout, "cpp2uno_call:fpr=%p\n", ovrflw[nREG - MAX_FP_REGS]);
#endif
              pCppArgs[nPos] = &(ovrflw[nREG - MAX_FP_REGS]);
              pUnoArgs[nPos] = &(ovrflw[nREG - MAX_FP_REGS]);
            }
            nREG++;
            break;


          default:
            if (nREG < MAX_GP_REGS) {
#ifdef BRDEBUG
              fprintf(stdout, "cpp2uno_call:gpr=%p\n", gpreg[nREG]);
#endif
              pCppArgs[nPos] = &(gpreg[nREG]);
              pUnoArgs[nPos] = &(gpreg[nREG]);
            } else {
#ifdef BRDEBUG
              fprintf(stdout, "cpp2uno_call:gpr=%p\n", ovrflw[nREG - MAX_GP_REGS]);
#endif
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
#ifdef BRDEBUG
        fprintf(stdout,"cpp2uno_call:ptr|ref\n");
#endif
        void *pCppStack;
        if (nREG < MAX_GP_REGS) {
          pCppArgs[nPos] = pCppStack = gpreg[nREG];
        } else {
          pCppArgs[nPos] = pCppStack = ovrflw[nREG - MAX_GP_REGS];
        }
        nREG++;
#ifdef BRDEBUG
        fprintf(stdout, "cpp2uno_call:pCppStack=%p\n", pCppStack);
#endif

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
#ifdef BRDEBUG
          fprintf(stdout, "cpp2uno_call:related to interface,%p,%d,pUnoargs[%d]=%p\n",
                          pCppStack, pParamTypeDescr->nSize, nPos, pUnoArgs[nPos]);
#endif
        }
        else // direct way
        {
          pUnoArgs[nPos] = pCppStack;
#ifdef BRDEBUG
          fprintf(stdout, "cpp2uno_call:direct,pUnoArgs[%d]=%p\n", nPos, pUnoArgs[nPos]);
#endif
          // no longer needed
          TYPELIB_DANGER_RELEASE( pParamTypeDescr );
        }
      }
    }
#ifdef BRDEBUG
    fprintf(stdout, "cpp2uno_call2,%p,unoargs=%p\n", pThis->getUnoI()->pDispatcher, pUnoArgs);
#endif

    // ExceptionHolder
    uno_Any aUnoExc; // Any will be constructed by callee
    uno_Any * pUnoExc = &aUnoExc;

    // invoke uno dispatch call
    (*pThis->getUnoI()->pDispatcher)( pThis->getUnoI(), pMemberTypeDescr, pUnoReturn, pUnoArgs, &pUnoExc );
#ifdef BRDEBUG
    fprintf(stdout, "cpp2uno_call2,after dispatch\n");
#endif

    // in case an exception occurred...
    if (pUnoExc)
    {
      fflush(stdout);
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

#ifdef BRDEBUG
    fprintf(stdout, "in cpp_vtable_call nFunctionIndex is %d\n", nFunctionIndex);
    fprintf(stdout, "in cpp_vtable_call nVtableOffset is %d\n", nVtableOffset);
    fprintf(stdout, "in cpp_vtable_call gp=%p, fp=%p, ov=%p\n", gpreg, fpreg, ovrflw);
#endif

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
#ifdef BRDEBUG
    fprintf(stdout, "cpp_vtable_call, pThis=%p, nFunctionIndex=%d, nVtableOffset=%d\n",
                pThis, nFunctionIndex, nVtableOffset);
#endif

    pThis = static_cast< char * >(pThis) - nVtableOffset;
    bridges::cpp_uno::shared::CppInterfaceProxy * pCppI =
        bridges::cpp_uno::shared::CppInterfaceProxy::castInterfaceToProxy( pThis );
#ifdef BRDEBUG
    fprintf(stdout, "cpp_vtable_call, pCppI=%p\n", pCppI);
#endif

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

#ifdef BRDEBUG
    //OString cstr( OUStringToOString( aMemberDescr.get()->pTypeName, RTL_TEXTENCODING_ASCII_US ) );
    //fprintf(stdout, "calling %s, nFunctionIndex=%d\n", cstr.getStr(), nFunctionIndex );
#endif
    typelib_TypeClass eRet;
    switch (aMemberDescr.get()->eTypeClass)
    {
      case typelib_TypeClass_INTERFACE_ATTRIBUTE:
        {
#ifdef BRDEBUG
    fprintf(stdout, "cpp_vtable_call interface attribute\n");
#endif
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
#ifdef BRDEBUG
    fprintf(stdout, "cpp_vtable_call interface method\n");
#endif
          // is METHOD
          switch (nFunctionIndex)
          {
            case 1: // acquire()
#ifdef BRDEBUG
    fprintf(stdout, "cpp_vtable_call method acquire\n");
#endif
              pCppI->acquireProxy(); // non virtual call!
              eRet = typelib_TypeClass_VOID;
              break;
            case 2: // release()
#ifdef BRDEBUG
    fprintf(stdout, "cpp_vtable_call method release\n");
#endif
              pCppI->releaseProxy(); // non virtual call!
              eRet = typelib_TypeClass_VOID;
              break;
            case 0: // queryInterface() opt
              {
#ifdef BRDEBUG
    fprintf(stdout, "cpp_vtable_call method query interface opt\n");
#endif
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
#ifdef BRDEBUG
    fprintf(stdout, "cpp_vtable_call method query interface\n");
#endif
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
#ifdef BRDEBUG
    fprintf(stdout, "cpp_vtable_call no member\n");
#endif
          throw RuntimeException( "no member description found!", (XInterface *)pThis );
        }
    }

    return eRet;
  }

  extern "C" void privateSnippetExecutor( ... );

  int const codeSnippetSize = 0x6c;

  unsigned char *  codeSnippet( unsigned char * code,
      sal_Int32 functionIndex, sal_Int32 vtableOffset,
      bool bHasHiddenParam )
  {
#ifdef BRDEBUG
     fprintf(stdout,"in codeSnippet functionIndex is %d\n", functionIndex);
     fprintf(stdout,"in codeSnippet vtableOffset is %d\n", vtableOffset);
     fprintf(stdout,"in codeSnippet privateSnippetExecutor is %lx\n", (unsigned long)privateSnippetExecutor);
     fprintf(stdout,"in codeSnippet cpp_vtable_call is %lx\n", (unsigned long)cpp_vtable_call);

     fflush(stdout);
#endif

    if ( bHasHiddenParam )
      functionIndex |= 0x80000000;

    unsigned int * p = (unsigned int *) code;

    assert((((unsigned long)code) & 0x3) == 0 );  //aligned to 4 otherwise a mistake

    /* generate this code */
    /*
       # load functionIndex to t4
             00000eb7       lui  t4,0x0
             000eee93       ori  t4,t4,0x0
       # load privateSnippetExecutor to t0
             000002b7       lui  t0,0x0
             02429293       slli t0,t0,36
             00000337       lui  t1,0x0
             01431313       slli t1,t1,20
             0062e2b3       or   t0,t0,t1
             00000337       lui  t1,0x0
             00431313       slli t1,t1,4
             0062e2b3       or   t0,t0,t1
             00000337       lui  t1,0x0
             00c35313       srli t1,t1,12
             0062e2b3       or   t0,t0,t1
       # load cpp_vtable_call to t6
             00000fb7       lui  t6,0x0
             024f9f93       slli t6,t6,36
             00000337       lui  t1,0x0
             01431313       slli t1,t1,20
             006fefb3       or   t6,t6,t1
             00000337       lui  t1,0x0
             00431313       slli t1,t1,4
             006fefb3       or   t6,t6,t1
             00000337       lui  t1,0x0
             00c35313       srli t1,t1,12
             006fefb3       or   t6,t6,t1
       # load vtableOffset to t5
             00000f37       lui  t5,0x0
             000f6f13       ori  t5,t5,0x0
       # jump to privateSnippetExecutor
             00028067       jalr zero,t0,0x0
    */

    * p++ = 0x00000eb7 | ((functionIndex) & 0xfffff000);
    * p++ = 0x000eee93 | ((functionIndex << 20 ) & 0xfff00000);

    // load privateSnippetExecutor to t0
    unsigned long functionEntry = ((unsigned long)privateSnippetExecutor);
    * p++ = 0x000002b7 | ((functionEntry >> 36) & 0x000000000ffff000);
    * p++ = 0x02429293;
    * p++ = 0x00000337 | ((functionEntry >> 20) & 0x000000000ffff000);
    * p++ = 0x01431313;
    * p++ = 0x0062e2b3;
    * p++ = 0x00000337 | ((functionEntry >> 4) & 0x000000000ffff000);
    * p++ = 0x00431313;
    * p++ = 0x0062e2b3;
    * p++ = 0x00000337 | ((functionEntry << 12) & 0x000000000ffff000);
    * p++ = 0x00c35313;
    * p++ = 0x0062e2b3;
    // load cpp_vtable_call to t6
    functionEntry = (unsigned long) cpp_vtable_call;
    * p++ = 0x00000fb7 | ((functionEntry >> 36) & 0x000000000ffff000);
    * p++ = 0x024f9f93;
    * p++ = 0x00000337 | ((functionEntry >> 20) & 0x000000000ffff000);
    * p++ = 0x01431313;
    * p++ = 0x006fefb3;
    * p++ = 0x00000337 | ((functionEntry >> 4) & 0x000000000ffff000);
    * p++ = 0x00431313;
    * p++ = 0x006fefb3;
    * p++ = 0x00000337 | ((functionEntry << 12) & 0x000000000ffff000);
    * p++ = 0x00c35313;
    * p++ = 0x006fefb3;
    // load vtableOffset to t5
    * p++ = 0x00000f37 | ((vtableOffset) & 0xfffff000);
    * p++ = 0x000f6f13 | ((vtableOffset << 20 ) & 0xfff00000);
    // jump to privateSnippetExecutor
    * p++ = 0x00028067;
    return (code + codeSnippetSize);

  }

}


void bridges::cpp_uno::shared::VtableFactory::flushCode(unsigned char const *bptr, unsigned char const *eptr)
{
	asm volatile(
	    "fence":::
	);
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

#ifdef BRDEBUG
   fprintf(stdout, "in addLocalFunctions functionOffset is %d\n", functionOffset);
   fprintf(stdout, "in addLocalFunctions vtableOffset is %d\n", vtableOffset);
   fprintf(stdout, "nMembers=%d\n", type->nMembers);
   fflush(stdout);
#endif

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
