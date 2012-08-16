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
#include <com/sun/star/uno/genfunc.hxx>
#include <typelib/typedescription.hxx>
#include <uno/data.h>
#include <osl/endian.h>
#include "bridges/cpp_uno/shared/bridge.hxx"
#include "bridges/cpp_uno/shared/cppinterfaceproxy.hxx"
#include "bridges/cpp_uno/shared/types.hxx"
#include "bridges/cpp_uno/shared/vtablefactory.hxx"
#include "share.hxx"

#include <stdio.h>
#include <string.h>

using namespace com::sun::star::uno;

//#define BRDEBUG

#ifdef BRDEBUG
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>
#include <osl/mutex.hxx>
using namespace ::std;
using namespace ::osl;
using namespace ::rtl;
#endif

#ifndef ANDROID
#include <sys/sysmips.h>
#endif

#ifdef ANDROID
#include <unistd.h>
#endif

#ifdef OSL_BIGENDIAN
#define IS_BIG_ENDIAN 1
#else
#define IS_BIG_ENDIAN 0
#endif

using namespace ::com::sun::star::uno;

namespace
{

  //==================================================================================================
  static typelib_TypeClass cpp2uno_call(
      bridges::cpp_uno::shared::CppInterfaceProxy * pThis,
      const typelib_TypeDescription * pMemberTypeDescr,
      typelib_TypeDescriptionReference * pReturnTypeRef, // 0 indicates void return
      sal_Int32 nParams, typelib_MethodParameter * pParams,
      void ** gpreg, void ** /*fpreg*/, void ** ovrflw,
      sal_Int64 * pRegisterReturn /* space for register return */ )
  {
    /*  Most MIPS ABIs view the arguments as a struct, of which the
        first N words go in registers and the rest go on the stack.  If I < N, the
        Ith word might go in Ith integer argument register or the Ith
        floating-point one.  For these ABIs, we only need to remember the number
        of words passed so far.  We are interested only in o32 ABI,so it is the
        case.
        */
    int nw = 0; // number of words used by arguments

#ifdef BRDEBUG
    fprintf(stderr,"cpp2uno_call1\n");
#endif

    /* C++ has [ret *] or this as the first arguments, so no arguments will
     * be passed in floating-point registers?
     */
    //int int_seen = 0; // have we seen integer arguments?

    void ** pCppStack; //temporary stack pointer

    // gpreg:  [ret *], this, [gpr params]
    // fpreg:  [fpr params]
    // ovrflw: [gpr or fpr params (properly aligned)]

    // return
    typelib_TypeDescription * pReturnTypeDescr = 0;
    if (pReturnTypeRef)
      TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );

    void * pUnoReturn = 0;
    void * pCppReturn = 0; // complex return ptr: if != 0 && != pUnoReturn, reconversion need

    if (pReturnTypeDescr)
    {
      if (bridges::cpp_uno::shared::isSimpleType( pReturnTypeDescr ))
      {
        pUnoReturn = pRegisterReturn; // direct way for simple types
#ifdef BRDEBUG
    fprintf(stderr,"cpp2uno_call:simplereturn\n");
#endif
      }
      else // complex return via ptr (pCppReturn)
      {
        pCppReturn = *(void **)gpreg;
        gpreg++;
        nw++;

        pUnoReturn = (bridges::cpp_uno::shared::relatesToInterfaceType( pReturnTypeDescr )
            ? alloca( pReturnTypeDescr->nSize )
            : pCppReturn); // direct way
#ifdef BRDEBUG
    fprintf(stderr,"cpp2uno_call:complexreturn\n");
#endif
      }
    }

    // pop this
    gpreg++;
    nw++;

    // stack space
    OSL_ENSURE( sizeof(void *) == sizeof(sal_Int32), "### unexpected size!" );
    // parameters
    void ** pUnoArgs = (void **)alloca( 4 * sizeof(void *) * nParams );
    void ** pCppArgs = pUnoArgs + nParams;
    // indizes of values this have to be converted (interface conversion cpp<=>uno)
    sal_Int32 * pTempIndizes = (sal_Int32 *)(pUnoArgs + (2 * nParams));
    // type descriptions for reconversions
    typelib_TypeDescription ** ppTempParamTypeDescr = (typelib_TypeDescription **)(pUnoArgs + (3 * nParams));

    sal_Int32 nTempIndizes   = 0;

#ifdef BRDEBUG
    fprintf(stderr,"cpp2uno_call:nParams=%d\n",nParams);
#endif

    for ( sal_Int32 nPos = 0; nPos < nParams; ++nPos )
    {
      const typelib_MethodParameter & rParam = pParams[nPos];
      typelib_TypeDescription * pParamTypeDescr = 0;
      TYPELIB_DANGER_GET( &pParamTypeDescr, rParam.pTypeRef );

      if (!rParam.bOut && bridges::cpp_uno::shared::isSimpleType( pParamTypeDescr ))
        // value
      {

        switch (pParamTypeDescr->eTypeClass)
        {
          case typelib_TypeClass_DOUBLE:
          case typelib_TypeClass_HYPER:
          case typelib_TypeClass_UNSIGNED_HYPER:
#ifdef BRDEBUG
    fprintf(stderr,"cpp2uno_call:hyper=%d,%p\n",pParamTypeDescr->eTypeClass,gpreg[0]);
#endif
            if (nw < 3) {
              if (nw & 1) {
                nw++;
                gpreg++;
              }
#ifdef BRDEBUG
    fprintf(stderr,"cpp2uno_call:gpreg=%p,%p\n",gpreg[0],gpreg[1]);
#endif
              pCppArgs[nPos] = gpreg;
              pUnoArgs[nPos] = gpreg;
              nw += 2;
              gpreg += 2;
            } else {
              if (((long)ovrflw) & 4) ovrflw++;
#ifdef BRDEBUG
    fprintf(stderr,"cpp2uno_call:overflw=%p,%p\n",ovrflw[0],ovrflw[1]);
#endif
              pCppArgs[nPos] = ovrflw;
              pUnoArgs[nPos] = ovrflw;
              ovrflw += 2;
            }
            break;

          case typelib_TypeClass_BYTE:
          case typelib_TypeClass_BOOLEAN:
#ifdef BRDEBUG
    fprintf(stderr,"cpp2uno_call:byte=%p,%p\n",gpreg[0],ovrflw[0]);
#endif
            if (nw < 4) {
              pCppArgs[nPos] = ((char *)gpreg + 3*IS_BIG_ENDIAN);
              pUnoArgs[nPos] = ((char *)gpreg + 3*IS_BIG_ENDIAN);
              nw++;
              gpreg++;
            } else {
              pCppArgs[nPos] = ((char *)ovrflw + 3*IS_BIG_ENDIAN);
              pUnoArgs[nPos] = ((char *)ovrflw + 3*IS_BIG_ENDIAN);
              ovrflw++;
            }
            break;


          case typelib_TypeClass_CHAR:
          case typelib_TypeClass_SHORT:
          case typelib_TypeClass_UNSIGNED_SHORT:
#ifdef BRDEBUG
    fprintf(stderr,"cpp2uno_call:char=%p,%p\n",gpreg[0],ovrflw[0]);
#endif
            if (nw < 4) {
              pCppArgs[nPos] = ((char *)gpreg + 2*IS_BIG_ENDIAN);
              pUnoArgs[nPos] = ((char *)gpreg + 2*IS_BIG_ENDIAN);
              nw++;
              gpreg++;
            } else {
              pCppArgs[nPos] = ((char *)ovrflw + 2*IS_BIG_ENDIAN);
              pUnoArgs[nPos] = ((char *)ovrflw + 2*IS_BIG_ENDIAN);
              ovrflw++;
            }
            break;


          default:
#ifdef BRDEBUG
    fprintf(stderr,"cpp2uno_call:def=%p,%p\n",gpreg[0],ovrflw[0]);
#endif
            if (nw < 4) {
              pCppArgs[nPos] = gpreg;
              pUnoArgs[nPos] = gpreg;
              nw++;
              gpreg++;
            } else {
              pCppArgs[nPos] = ovrflw;
              pUnoArgs[nPos] = ovrflw;
              ovrflw++;
            }
            break;

        }
        // no longer needed
        TYPELIB_DANGER_RELEASE( pParamTypeDescr );
      }
      else // ptr to complex value | ref
      {

#ifdef BRDEBUG
    fprintf(stderr,"cpp2uno_call:ptr|ref\n");
#endif
        if (nw < 4) {
          pCppArgs[nPos] = *(void **)gpreg;
          pCppStack = gpreg;
          nw++;
          gpreg++;
        } else {
          pCppArgs[nPos] = *(void **)ovrflw;
          pCppStack = ovrflw;
          ovrflw++;
        }
#ifdef BRDEBUG
    fprintf(stderr,"cpp2uno_call:pCppStack=%p\n",pCppStack);
#endif

        if (! rParam.bIn) // is pure out
        {
          // uno out is unconstructed mem!
          pUnoArgs[nPos] = alloca( pParamTypeDescr->nSize );
          pTempIndizes[nTempIndizes] = nPos;
          // will be released at reconversion
          ppTempParamTypeDescr[nTempIndizes++] = pParamTypeDescr;
        }
        // is in/inout
        else if (bridges::cpp_uno::shared::relatesToInterfaceType( pParamTypeDescr ))
        {
          uno_copyAndConvertData( pUnoArgs[nPos] = alloca( pParamTypeDescr->nSize ),
              *(void **)pCppStack, pParamTypeDescr,
              pThis->getBridge()->getCpp2Uno() );
          pTempIndizes[nTempIndizes] = nPos; // has to be reconverted
          // will be released at reconversion
          ppTempParamTypeDescr[nTempIndizes++] = pParamTypeDescr;
#ifdef BRDEBUG
    fprintf(stderr,"cpp2uno_call:related to interface,%p,%d,pUnoargs[%d]=%p\n",*(void**)pCppStack,pParamTypeDescr->nSize,nPos,pUnoArgs[nPos]);
#endif
        }
        else // direct way
        {
          pUnoArgs[nPos] = *(void **)pCppStack;
#ifdef BRDEBUG
    fprintf(stderr,"cpp2uno_call:direct,pUnoArgs[%d]=%p\n",nPos,pUnoArgs[nPos]);
#endif
          // no longer needed
          TYPELIB_DANGER_RELEASE( pParamTypeDescr );
        }
      }
    }
#ifdef BRDEBUG
    fprintf(stderr,"cpp2uno_call2,%p,unoargs=%p\n",pThis->getUnoI()->pDispatcher,pUnoArgs);
#endif

    // ExceptionHolder
    uno_Any aUnoExc; // Any will be constructed by callee
    uno_Any * pUnoExc = &aUnoExc;

    // invoke uno dispatch call
    (*pThis->getUnoI()->pDispatcher)( pThis->getUnoI(), pMemberTypeDescr, pUnoReturn, pUnoArgs, &pUnoExc );
#ifdef BRDEBUG
    fprintf(stderr,"cpp2uno_call2,after dispatch\n");
#endif

    // in case an exception occurred...
    if (pUnoExc)
    {
      // destruct temporary in/inout params
      for ( ; nTempIndizes--; )
      {
        sal_Int32 nIndex = pTempIndizes[nTempIndizes];

        if (pParams[nIndex].bIn) // is in/inout => was constructed
          uno_destructData( pUnoArgs[nIndex], ppTempParamTypeDescr[nTempIndizes], 0 );
        TYPELIB_DANGER_RELEASE( ppTempParamTypeDescr[nTempIndizes] );
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
      for ( ; nTempIndizes--; )
      {
        sal_Int32 nIndex = pTempIndizes[nTempIndizes];
        typelib_TypeDescription * pParamTypeDescr = ppTempParamTypeDescr[nTempIndizes];

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


  //==================================================================================================
  static typelib_TypeClass cpp_mediate(
      sal_Int32 nFunctionIndex,
      sal_Int32 nVtableOffset,
      void ** gpreg, void ** fpreg, void ** ovrflw,
      sal_Int64 * pRegisterReturn /* space for register return */ )
  {
    OSL_ENSURE( sizeof(sal_Int32)==sizeof(void *), "### unexpected!" );

#ifdef BRDEBUG
    fprintf(stderr,"cpp_mediate1 gp=%p,fp=%p,ov=%p\n",gpreg,fpreg,ovrflw);
    fprintf(stderr,"gp=%x,%x,%x,%x\n",gpreg[0],gpreg[1],gpreg[2],gpreg[3]);
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
    fprintf(stderr,"cpp_mediate12,pThis=%p, nFunctionIndex=%d,nVtableOffset=%d\n",pThis,nFunctionIndex,nVtableOffset);
#endif

    pThis = static_cast< char * >(pThis) - nVtableOffset;
    bridges::cpp_uno::shared::CppInterfaceProxy * pCppI
      = bridges::cpp_uno::shared::CppInterfaceProxy::castInterfaceToProxy(
          pThis);
#ifdef BRDEBUG
    fprintf(stderr,"cpp_mediate13,pCppI=%p\n",pCppI);
#endif

    typelib_InterfaceTypeDescription * pTypeDescr = pCppI->getTypeDescr();

#ifdef BRDEBUG
    fprintf(stderr,"cpp_mediate2\n");
#endif
    OSL_ENSURE( nFunctionIndex < pTypeDescr->nMapFunctionIndexToMemberIndex, "### illegal vtable index!" );
    if (nFunctionIndex >= pTypeDescr->nMapFunctionIndexToMemberIndex)
    {
      throw RuntimeException(
          rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "illegal vtable index!" )),
          (XInterface *)pThis );
    }

    // determine called method
    sal_Int32 nMemberPos = pTypeDescr->pMapFunctionIndexToMemberIndex[nFunctionIndex];
    OSL_ENSURE( nMemberPos < pTypeDescr->nAllMembers, "### illegal member index!" );

    TypeDescription aMemberDescr( pTypeDescr->ppAllMembers[nMemberPos] );

#ifdef BRDEBUG
    fprintf(stderr,"cpp_mediate3\n");
    OString cstr( OUStringToOString( aMemberDescr.get()->pTypeName, RTL_TEXTENCODING_ASCII_US ) );
    fprintf( stderr, "calling %s, nFunctionIndex=%d\n", cstr.getStr(), nFunctionIndex );
#endif
    typelib_TypeClass eRet;
    switch (aMemberDescr.get()->eTypeClass)
    {
      case typelib_TypeClass_INTERFACE_ATTRIBUTE:
        {
#ifdef BRDEBUG
    fprintf(stderr,"cpp_mediate4\n");
#endif
          if (pTypeDescr->pMapMemberIndexToFunctionIndex[nMemberPos] == nFunctionIndex)
          {
            // is GET method
            eRet = cpp2uno_call(
                pCppI, aMemberDescr.get(),
                ((typelib_InterfaceAttributeTypeDescription *)aMemberDescr.get())->pAttributeTypeRef,
                0, 0, // no params
                gpreg, fpreg, ovrflw, pRegisterReturn );
          }
          else
          {
            // is SET method
            typelib_MethodParameter aParam;
            aParam.pTypeRef =
              ((typelib_InterfaceAttributeTypeDescription *)aMemberDescr.get())->pAttributeTypeRef;
            aParam.bIn      = sal_True;
            aParam.bOut     = sal_False;

            eRet = cpp2uno_call(
                pCppI, aMemberDescr.get(),
                0, // indicates void return
                1, &aParam,
                gpreg, fpreg, ovrflw, pRegisterReturn );
          }
          break;
        }
      case typelib_TypeClass_INTERFACE_METHOD:
        {
#ifdef BRDEBUG
    fprintf(stderr,"cpp_mediate5\n");
#endif
          // is METHOD
          switch (nFunctionIndex)
          {
            case 1: // acquire()
              pCppI->acquireProxy(); // non virtual call!
              eRet = typelib_TypeClass_VOID;
              break;
            case 2: // release()
#ifdef BRDEBUG
    fprintf(stderr,"cpp_mediate51\n");
#endif
              pCppI->releaseProxy(); // non virtual call!
              eRet = typelib_TypeClass_VOID;
#ifdef BRDEBUG
    fprintf(stderr,"cpp_mediate52\n");
#endif
              break;
            case 0: // queryInterface() opt
              {
                typelib_TypeDescription * pTD = 0;
                TYPELIB_DANGER_GET( &pTD, reinterpret_cast< Type * >( gpreg[2] )->getTypeLibType() );
                if (pTD)
                {
                  XInterface * pInterface = 0;
                  (*pCppI->getBridge()->getCppEnv()->getRegisteredInterface)(
                                                                             pCppI->getBridge()->getCppEnv(),
                                                                             (void **)&pInterface, pCppI->getOid().pData,
                                                                             (typelib_InterfaceTypeDescription *)pTD );

                  if (pInterface)
                  {
                    ::uno_any_construct(
                        reinterpret_cast< uno_Any * >( gpreg[0] ),
                        &pInterface, pTD, cpp_acquire );
                    pInterface->release();
                    TYPELIB_DANGER_RELEASE( pTD );
                    *(void **)pRegisterReturn = gpreg[0];
                    eRet = typelib_TypeClass_ANY;
                    break;
                  }
                  TYPELIB_DANGER_RELEASE( pTD );
                }
              } // else perform queryInterface()
            default:
              eRet = cpp2uno_call(
                  pCppI, aMemberDescr.get(),
                  ((typelib_InterfaceMethodTypeDescription *)aMemberDescr.get())->pReturnTypeRef,
                  ((typelib_InterfaceMethodTypeDescription *)aMemberDescr.get())->nParams,
                  ((typelib_InterfaceMethodTypeDescription *)aMemberDescr.get())->pParams,
                  gpreg, fpreg, ovrflw, pRegisterReturn );
          }
          break;
        }
      default:
        {
#ifdef BRDEBUG
    fprintf(stderr,"cpp_mediate6\n");
#endif
          throw RuntimeException(
              rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "no member description found!" )),
              (XInterface *)pThis );
        }
    }

    return eRet;
  }

  //==================================================================================================
  /**
   * is called on incoming vtable calls
   * (called by asm snippets)
   */
//  static void cpp_vtable_call( int nFunctionIndex, int nVtableOffset, void** gpregptr, void** fpregptr, void** ovrflw)
//  static void cpp_vtable_call( int nFunctionIndex, int nVtableOffset, void** gpregptr, void** ovrflw)
  static void cpp_vtable_call(void)
  {
    int nFunctionIndex;
    int vTableOffset;
    void** pCallStack;
    void** ovrflw;

    sal_Int32     gpreg[4];
    double        fpreg[2];

    //memcpy( fpreg, fpregptr, 16);

    volatile long nRegReturn[2];

    __asm__( "sw $4, %0\n\t"
         "sw $5, %1\n\t"
         "sw $6, %2\n\t"
         "sw $7, %3\n\t"
            ::"m"(nFunctionIndex), "m"(vTableOffset), "m"(pCallStack), "m"(ovrflw) );

    memcpy( gpreg, pCallStack, 16);

#ifdef BRDEBUG
    fprintf(stderr,"in cpp_vtable_call nFunctionIndex is %d\n",nFunctionIndex);
    fprintf(stderr,"in cpp_vtable_call nVtableOffset is %d\n",vTableOffset);
    fprintf(stderr,"gp=%x,%x,%x,%x\n",gpreg[0],gpreg[1],gpreg[2],gpreg[3]);
#endif

    //sal_Bool bComplex = nFunctionIndex & 0x80000000 ? sal_True : sal_False;

    typelib_TypeClass aType =
      cpp_mediate( nFunctionIndex, vTableOffset, (void**)gpreg, (void**)fpreg, ovrflw, (sal_Int64*)nRegReturn );

    switch( aType )
    {

      // move return value into register space
      // (will be loaded by machine code snippet)

      case typelib_TypeClass_BOOLEAN:
      case typelib_TypeClass_BYTE:
        __asm__( "lbu $2,%0\n\t" : :
            "m"(nRegReturn[0]) );
        break;

      case typelib_TypeClass_CHAR:
      case typelib_TypeClass_UNSIGNED_SHORT:
        __asm__( "lhu $2,%0\n\t" : :
            "m"(nRegReturn[0]) );
        break;

      case typelib_TypeClass_SHORT:
        __asm__( "lh $2,%0\n\t" : :
            "m"(nRegReturn[0]) );
        break;


      case typelib_TypeClass_FLOAT:
        __asm__( "lwc1 $f0,%0\n\t" : :
            "m" (*((float*)nRegReturn)) );
        break;

      case typelib_TypeClass_DOUBLE:
          { register double dret asm("$f0");
            dret = (*((double*)nRegReturn));
            (void) dret;
          }
        break;

      case typelib_TypeClass_HYPER:
      case typelib_TypeClass_UNSIGNED_HYPER:
        __asm__( "lw $3,%0\n\t" : :
            "m"(nRegReturn[1]) );  // fall through

      default:
        __asm__( "lw $2,%0\n\t" : :
            "m"(nRegReturn[0]) );
        break;
    }
  }


  int const codeSnippetSize = 56;

  unsigned char *  codeSnippet( unsigned char * code, sal_Int32 functionIndex, sal_Int32 vtableOffset,
      bool simpleRetType)
  {

#ifdef BRDEBUG
     fprintf(stderr,"in codeSnippet functionIndex is %d\n", functionIndex);
     fprintf(stderr,"in codeSnippet vtableOffset is %d\n", vtableOffset);
     fflush(stderr);
#endif

    if (! simpleRetType )
      functionIndex |= 0x80000000;

    unsigned long * p = (unsigned long *) code;

    // OSL_ASSERT( sizeof (long) == 4 );
    OSL_ASSERT((((unsigned long)code) & 0x3) == 0 );  //aligned to 4 otherwise a mistake

    /* generate this code */
    /*
       #save regs into argument space required by mips abi
            c:   afa40000        sw      a0,0(sp)
       10:   afa50004        sw      a1,4(sp)
       14:   afa60008        sw      a2,8(sp)
       18:   afa7000c        sw      a3,12(sp)
       #a0=index
       1c:   3c040000        lui     a0,0x0
       20:   34840000        ori     a0,a0,0x0
       #a1=offset
       24:   3c050000        lui     a1,0x0
       28:   34a50000        ori     a1,a1,0x0
       #a2=gpregptr
       2c:   27a60000        addiu   a2,sp,0
       #a3=ovrflw
       30:   27a70010        addiu   a3,sp,16
       #load cpp_vtable_call addr
       34:   3c190000        lui     t9,0x0
       38:   37390000        ori     t9,t9,0
       #jmp to the function,note: we don't use jalr, that will destroy $ra
       #but be sure to use t9! gp calculation depends on it
       3c:   03200008        jr      t9
       40:   00000000        nop

       be careful, we use the argument space reserved by the caller to
       write down regs. This can avoid the need to make use of arbitary far away
       stack space or to allocate a function frame for this code snippet itself.
       Since only functions with variable arguments will overwrite the space,
       cpp_vtable_call should be safe.
       ??? gcc seems change this behavior! cpp_vtable_call overwrite the space!
     */

    * p++ = 0xafa40000;
    * p++ = 0xafa50004;
    * p++ = 0xafa60008;
    * p++ = 0xafa7000c;
    * p++ = 0x3c040000 | ((functionIndex>>16) & 0x0000ffff);
    * p++ = 0x34840000 | (functionIndex & 0x0000ffff);
    * p++ = 0x3c050000 | ((vtableOffset>>16) & 0x0000ffff);
    * p++ = 0x34a50000 | (vtableOffset & 0x0000ffff);
    * p++ = 0x27a60000;
    * p++ = 0x27a70010;
    * p++ = 0x3c190000 | ((((unsigned long)cpp_vtable_call) >> 16) & 0x0000ffff);
    * p++ = 0x37390000 | (((unsigned long)cpp_vtable_call) & 0x0000FFFF);
    * p++ = 0x03200008;
    * p++ = 0x00000000;
    return (code + codeSnippetSize);

  }


}


void bridges::cpp_uno::shared::VtableFactory::flushCode(unsigned char const *bptr, unsigned char const *eptr)
{
#ifndef ANDROID
  (void) bptr;
  (void) eptr;
  sysmips(FLUSH_CACHE,0,0,0);
#else
   cacheflush((long) bptr, (long) eptr, 0);
#endif
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
   fprintf(stderr, "in addLocalFunctions functionOffset is %d\n",functionOffset);
   fprintf(stderr, "in addLocalFunctions vtableOffset is %d\n",vtableOffset);
   fprintf(stderr, "nMembers=%d\n",type->nMembers);
   fflush(stderr);
#endif

  for (sal_Int32 i = 0; i < type->nMembers; ++i) {
    typelib_TypeDescription * member = 0;
    TYPELIB_DANGER_GET(&member, type->ppMembers[i]);
    OSL_ASSERT(member != 0);
    switch (member->eTypeClass) {
      case typelib_TypeClass_INTERFACE_ATTRIBUTE:
        // Getter:
        (s++)->fn = code + writetoexecdiff;
        code = codeSnippet(
            code, functionOffset++, vtableOffset,
            bridges::cpp_uno::shared::isSimpleType(
              reinterpret_cast<
              typelib_InterfaceAttributeTypeDescription * >(
                member)->pAttributeTypeRef));

        // Setter:
        if (!reinterpret_cast<
            typelib_InterfaceAttributeTypeDescription * >(
              member)->bReadOnly)
        {
          (s++)->fn = code + writetoexecdiff;
          code = codeSnippet(code, functionOffset++, vtableOffset, true);
        }
        break;

      case typelib_TypeClass_INTERFACE_METHOD:
        (s++)->fn = code + writetoexecdiff;
        code = codeSnippet(
            code, functionOffset++, vtableOffset,
            bridges::cpp_uno::shared::isSimpleType(
              reinterpret_cast<
              typelib_InterfaceMethodTypeDescription * >(
                member)->pReturnTypeRef));
        break;

      default:
        OSL_ASSERT(false);
        break;
    }
    TYPELIB_DANGER_RELEASE(member);
  }
  return code;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
