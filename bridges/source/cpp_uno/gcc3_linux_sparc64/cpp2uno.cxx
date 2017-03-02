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
#include "bridge.hxx"
#include "cppinterfaceproxy.hxx"
#include "types.hxx"
#include "vtablefactory.hxx"
#include "share.hxx"

#define GET_FP(n, p) \
            __asm__( "ldx %0, %%l0\n\t" \
                     "std %%f" #n ", [%%l0]\n" \
                     : : "m"(p) );

using namespace com::sun::star::uno;

namespace CPPU_CURRENT_NAMESPACE
{
  bool is_complex_struct(const typelib_TypeDescription * type)
  {
      for (const typelib_CompoundTypeDescription * p
              = reinterpret_cast< const typelib_CompoundTypeDescription * >(type);
              p != NULL; p = p->pBaseTypeDescription)
      {
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
      }
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

          //A Composite Type not larger than 32 bytes is returned in up to two GPRs
          bool bRet = pTypeDescr->nSize > 32 || is_complex_struct(pTypeDescr);

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
    void ** pCallStack,
    sal_Int64 * pRegisterReturn /* space for register return */ )
{
    // pCallStack: [ret ptr], this, params
    char * pCppStack = (char *)pCallStack;

    // return
    typelib_TypeDescription * pReturnTypeDescr = 0;
    if (pReturnTypeRef)
        TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );

    void * pUnoReturn = 0;
    void * pCppReturn = 0; // complex return ptr: if != 0 && != pUnoReturn, reconversion need

    int paramsOffset;
    if (pReturnTypeDescr)
    {
        if (CPPU_CURRENT_NAMESPACE::return_in_hidden_param( pReturnTypeRef ) )
        {
            pCppReturn = *(void**)pCppStack; // complex return via ptr (pCppReturn)
            pUnoReturn = (bridges::cpp_uno::shared::relatesToInterfaceType(
                                   pReturnTypeDescr )
                          ? alloca( pReturnTypeDescr->nSize )
                          : pCppReturn); // direct way
            pCppStack += sizeof( void* );
            paramsOffset = 2;
        }
        else
        {
            pUnoReturn = pRegisterReturn; // direct way for simple types
            paramsOffset = 1;
        }
    }
    else
    {
        paramsOffset = 1;
    }
    // pop this
    pCppStack += sizeof( void* );

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

        if (!rParam.bOut && bridges::cpp_uno::shared::isSimpleType( pParamTypeDescr ))  // value
        {
            pCppArgs[nPos] = pUnoArgs[nPos] = CPPU_CURRENT_NAMESPACE::adjustPointer(pCppStack, pParamTypeDescr);
            switch (pParamTypeDescr->eTypeClass) {
                case typelib_TypeClass_FLOAT:
                case typelib_TypeClass_DOUBLE:
                    {
                        int paramArrayIdx = nPos + paramsOffset;
                        assert(paramArrayIdx < nParams + paramsOffset);
                        switch (paramArrayIdx) {
                            // Cannot be 0 - paramsOffset >= 1
                            case 1:
                                GET_FP(2, pCppStack);
                                break;
                            case 2:
                                GET_FP(4, pCppStack);
                                break;
                            case 3:
                                GET_FP(6, pCppStack);
                                break;
                            case 4:
                                GET_FP(8, pCppStack);
                                break;
                            case 5:
                                GET_FP(10, pCppStack);
                                break;
                            case 6:
                                GET_FP(12, pCppStack);
                                break;
                            case 7:
                                GET_FP(14, pCppStack);
                                break;
                            case 8:
                                GET_FP(16, pCppStack);
                                break;
                            case 9:
                                GET_FP(18, pCppStack);
                                break;
                            case 10:
                                GET_FP(20, pCppStack);
                                break;
                            case 11:
                                GET_FP(22, pCppStack);
                                break;
                            case 12:
                                GET_FP(24, pCppStack);
                                break;
                            case 13:
                                GET_FP(26, pCppStack);
                                break;
                            case 14:
                                GET_FP(28, pCppStack);
                                break;
                            case 15:
                                GET_FP(30, pCppStack);
                                break;
                                // Anything larger is passed on the stack
                        }
                        break;
                    }
                default:
                    break;
            }
            // no longer needed
            TYPELIB_DANGER_RELEASE( pParamTypeDescr );
        }
        else // ptr to complex value | ref
        {
            pCppArgs[nPos] = *(void **)pCppStack;

            if (! rParam.bIn) // is pure out
            {
                // uno out is unconstructed mem!
                pUnoArgs[nPos] = alloca( pParamTypeDescr->nSize );
                pTempIndices[nTempIndices] = nPos;
                // will be released at reconversion
                ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
            }
            // is in/inout
            else if (bridges::cpp_uno::shared::relatesToInterfaceType(
                              pParamTypeDescr ))
            {
                uno_copyAndConvertData( pUnoArgs[nPos] = alloca( pParamTypeDescr->nSize ),
                                        *(void **)pCppStack, pParamTypeDescr,
                                          pThis->getBridge()->getCpp2Uno() );
                pTempIndices[nTempIndices] = nPos; // has to be reconverted
                // will be released at reconversion
                ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
            }
            else // direct way
            {
                pUnoArgs[nPos] = *(void **)pCppStack;
                // no longer needed
                TYPELIB_DANGER_RELEASE( pParamTypeDescr );
            }
        }
        pCppStack += sizeof(sal_Int64); // standard parameter length
    }

    // ExceptionHolder
    uno_Any aUnoExc; // Any will be constructed by callee
    uno_Any * pUnoExc = &aUnoExc;

    // invoke uno dispatch call
    (*pThis->getUnoI()->pDispatcher)(pThis->getUnoI(), pMemberTypeDescr, pUnoReturn, pUnoArgs, &pUnoExc );

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
        CPPU_CURRENT_NAMESPACE::raiseException(&aUnoExc, pThis->getBridge()->getUno2Cpp() );
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
            // complex return ptr is set to eax
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


static typelib_TypeClass cpp_mediate(
    sal_Int32   nFunctionIndex,
    sal_Int32   nVtableOffset,
    void ** pCallStack,
    sal_Int64 * pRegisterReturn /* space for register return */ )
{
    static_assert(sizeof(sal_Int64)==sizeof(void *), "### unexpected!");

    // pCallStack: [ret*], this, params
    void * pThis;
    if (nFunctionIndex & 0x80000000)
    {
        nFunctionIndex &= 0x7fffffff;
        pThis = pCallStack[1];
    }
    else
    {
        pThis = pCallStack[0];
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
            (XInterface *)pCppI);
    }

    // determine called method
    sal_Int32 nMemberPos = pTypeDescr->pMapFunctionIndexToMemberIndex[nFunctionIndex];
    assert(nMemberPos < pTypeDescr->nAllMembers);

    TypeDescription aMemberDescr( pTypeDescr->ppAllMembers[nMemberPos] );

#if defined BRIDGES_DEBUG
    OString cstr( OUStringToOString( aMemberDescr.get()->pTypeName, RTL_TEXTENCODING_ASCII_US ) );
    fprintf( stderr, "calling %s, nFunctionIndex=%d\n", cstr.getStr(), nFunctionIndex );
#endif

    typelib_TypeClass eRet;
    switch (aMemberDescr.get()->eTypeClass)
    {
    case typelib_TypeClass_INTERFACE_ATTRIBUTE:
    {
        if (pTypeDescr->pMapMemberIndexToFunctionIndex[nMemberPos] == nFunctionIndex)
        {
            // is GET method
            eRet = cpp2uno_call(
                pCppI, aMemberDescr.get(),
                ((typelib_InterfaceAttributeTypeDescription *)aMemberDescr.get())->pAttributeTypeRef,
                0, 0, // no params
                pCallStack, pRegisterReturn );
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
                pCallStack, pRegisterReturn );
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
            TYPELIB_DANGER_GET( &pTD, reinterpret_cast< Type * >( pCallStack[2] )->getTypeLibType() );
            if (pTD)
            {
                XInterface * pInterface = 0;
        (*pCppI->getBridge()->getCppEnv()->getRegisteredInterface)(
            pCppI->getBridge()->getCppEnv(),
            (void **)&pInterface, pCppI->getOid().pData, (typelib_InterfaceTypeDescription *)pTD );

                if (pInterface)
                {
                    ::uno_any_construct(
                        reinterpret_cast< uno_Any * >( pCallStack[0] ),
                        &pInterface, pTD, cpp_acquire );
                    pInterface->release();
                    TYPELIB_DANGER_RELEASE( pTD );
                    *(void **)pRegisterReturn = pCallStack[0];
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
                pCallStack, pRegisterReturn );
        }
        break;
    }
    default:
    {
        throw RuntimeException( "no member description found!", (XInterface *)pCppI );
    }
    }
    return eRet;
}



/**
 * is called on incoming vtable calls
 * (called by asm snippets)
 */
static void cpp_vtable_call(int nFunctionIndex, void** pCallStack, int vTableOffset)
{
    sal_Int64 nRegReturn[4] = { 0 };
    void * pRegReturn = &nRegReturn[0];

    //__asm__( "st %%i0, %0\n\t"
    //        "stx %%i1, %1\n\t"
    //         "st %%i2, %2\n\t"
    //        : : "m"(nFunctionIndex), "m"(pCallStack), "m"(vTableOffset) );

//  fprintf(stderr,"cpp_mediate nFunctionIndex=%x\n",nFunctionIndex);
//  fflush(stderr);

    //const sal_Bool bComplex = (nFunctionIndex & 0x80000000) ? sal_True : sal_False;
    typelib_TypeClass aType =
        cpp_mediate( nFunctionIndex, vTableOffset, pCallStack+16, (sal_Int64*)&nRegReturn );

    switch( aType )
    {
        case typelib_TypeClass_BOOLEAN:
        case typelib_TypeClass_BYTE:
            __asm__( "ldx %0, %%l0\n\t"
                     "ldsb [%%l0], %%i0\n"
                     : : "m"(pRegReturn) );
            break;
        case typelib_TypeClass_CHAR:
        case typelib_TypeClass_SHORT:
        case typelib_TypeClass_UNSIGNED_SHORT:
            __asm__( "ldx %0, %%l0\n\t"
                     "ldsh [%%l0], %%i0\n"
                     : : "m"(pRegReturn) );
            break;
        case typelib_TypeClass_ENUM:
        case typelib_TypeClass_LONG:
        case typelib_TypeClass_UNSIGNED_LONG:
            __asm__( "ldx %0, %%l0\n\t"
                     "ld [%%l0], %%i0\n"
                     : : "m"(pRegReturn) );
            break;
        case typelib_TypeClass_HYPER:
        case typelib_TypeClass_UNSIGNED_HYPER:
            __asm__( "ldx %0, %%l0\n\t"
                     "ldx [%%l0], %%i0\n\t"
                      : : "m"(pRegReturn) );
            break;
        case typelib_TypeClass_FLOAT:
            __asm__( "ldx %0, %%l0\n\t"
                     "ld [%%l0], %%f0\n"
                     : : "m"(pRegReturn) );
            break;
        case typelib_TypeClass_DOUBLE:
            __asm__( "ldx %0, %%l0\n\t"
                     "ldd [%%l0], %%f0\n"
                     : : "m"(pRegReturn) );
            break;
        case typelib_TypeClass_VOID:
            break;
        case typelib_TypeClass_STRUCT:
        case typelib_TypeClass_EXCEPTION:
            __asm__( "ldx %0, %%l0\n\t"
                     "ldx [%%l0   ], %%i0\n\t"
                     "ldx [%%l0+ 8], %%i1\n\t"
                     "ldx [%%l0+16], %%i2\n\t"
                     "ldx [%%l0+24], %%i3\n\t"
                     "ldd [%%l0   ], %%f0\n\t"
                     "ldd [%%l0+ 8], %%f2\n\t"
                     "ldd [%%l0+16], %%f4\n\t"
                     "ldd [%%l0+24], %%f6\n\t"
                      : : "m"(pRegReturn) );
            break;
        default:
            break;
    }

    //if( bComplex )
    //{
    //    __asm__( "add %i7, 4, %i7\n\t" );
    //    // after call to complex return valued function there is an unimp instruction
    //}

}

extern "C" void privateSnippetExecutor(...);

int const codeSnippetSize = 120;
unsigned char * codeSnippet(
    unsigned char * code, sal_Int32 functionIndex, sal_Int32 vtableOffset,
    bool bHasHiddenParam, sal_Int32 nParams)
{
    sal_uInt32 index = functionIndex;
    if (bHasHiddenParam) {
        index |= 0x80000000;
    }
    unsigned int * p = reinterpret_cast< unsigned int * >(code);
    static_assert(sizeof (unsigned int) == 4, "boo");
    static_assert(sizeof (unsigned long long) == 8, "boo");
    ++nParams; // implicit this ptr
    if (bHasHiddenParam) {
        ++nParams;
    }
    long long frameSize;
    if (nParams > 6) {
        frameSize = 128 + nParams * 8;
    } else {
        frameSize = 176;
    }
    assert(frameSize <= 4096);
    frameSize = -frameSize;
    switch (nParams) {
    default:
        assert(nParams >= 6);
        // stx %o5, [%sp+168+2047]:
        *p++ = 0xDA73A8A7;
    case 5:
        // stx %o4, [%sp+160+2047]:
        *p++ = 0xD873A89F;
    case 4:
        // stx %o3, [%sp+152+2047]:
        *p++ = 0xD673A897;
    case 3:
        // stx %o2, [%sp+144+2047]:
        *p++ = 0xD473A88F;
    case 2:
        // stx %o1, [%sp+136+2047]:
        *p++ = 0xD273A887;
    case 1:
        // stx %o0, [%sp+128+2047]:
        *p++ = 0xD073A87F;
    case 0:
        break;
    }
    // sethi %hi(index), %o0:
    *p++ = 0x11000000 | (index >> 10);
    // or %o0, %lo(index), %o0:
    *p++ = 0x90122000 | (index & 0x3FF);
    // sethi %hh(cpp_vtable_call), %o3:
    *p++ = 0x17000000 | (reinterpret_cast< unsigned long long >(cpp_vtable_call) >> 42);
    // or %o3, %hm(cpp_vtable_call), %o3:
    *p++ = 0x9612E000 | ((reinterpret_cast< unsigned long long >(cpp_vtable_call) >> 32) & 0x3FF);
    // sllx %o3, 32, %o3
    *p++ = 0x972AF020;
    // sethi %lm(cpp_vtable_call), %o2:
    *p++ = 0x15000000 | ((reinterpret_cast< unsigned long long >(cpp_vtable_call) >> 10) & 0x3FFFFF);
    // or %o2, %lo(cpp_vtable_call), %o2:
    *p++ = 0x9412A000 | (reinterpret_cast< unsigned long long >(cpp_vtable_call) & 0x3FF);
    // or %o2, %o3, %o3:
    *p++ = 0x9612800B;
    // sethi %hh(privateSnippetExecutor), %o1:
    *p++ = 0x13000000 | (reinterpret_cast< unsigned long long >(privateSnippetExecutor) >> 42);
    // or %o1, %hm(privateSnippetExecutor), %o1:
    *p++ = 0x92126000 | ((reinterpret_cast< unsigned long long >(privateSnippetExecutor) >> 32) & 0x3FF);
    // sllx %o1, 32, %o1:
    *p++ = 0x932a7020;
    // sethi %lm(privateSnippetExecutor), %o2:
    *p++ = 0x15000000 | ((reinterpret_cast< unsigned long long >(privateSnippetExecutor) >> 10) & 0x3FFFFF);
    // or %o2, %lo(privateSnippetExecutor), %o2:
    *p++ = 0x9412A000 | (reinterpret_cast< unsigned long long >(privateSnippetExecutor) & 0x3FF);
    // or %o2, %o1, %o1:
    *p++ = 0x92128009;
    // sethi %hh(frameSize), %o4:
    *p++ = 0x19000000 | (*reinterpret_cast< unsigned long long * >(&frameSize) >> 42);
    // or %o4, %hm(frameSize), %o4:
    *p++ = 0x98132000 | ((*reinterpret_cast< unsigned long long * >(&frameSize) >> 32) & 0x3FF);
    // sllx %o4, 32, %o4
    *p++ = 0x992B3020;
    // sethi %lm(frameSize), %o2:
    *p++ = 0x15000000 | ((*reinterpret_cast< unsigned long long * >(&frameSize) >> 10) & 0x3FFFFF);
    // or %o2, %lo(frameSize), %o2:
    *p++ = 0x9412A000 | (*reinterpret_cast< unsigned long long * >(&frameSize) & 0x3FF);
    // or %o2, %o4, %o4:
    *p++ = 0x9812800C;
    // sethi %hi(vtableOffset), %o2:
    *p++ = 0x15000000 | (vtableOffset >> 10);
    // or %o2, %lo(vtableOffset), %o2:
    *p++ = 0x9412A000 | (vtableOffset & 0x3FF);
    // save %sp, -frameSize, %sp
    //*p++ = 0x9DE3A000 | (*reinterpret_cast< unsigned int * >(&frameSize) & 0x1FFF);
    // jmpl %o1, %g0:
    *p++ = 0x81C24000;
    // add %sp, 2047, %o1:
    *p++ = 0x9203A7FF;
    assert(reinterpret_cast< unsigned char * >(p) - code <= codeSnippetSize);
    return code + codeSnippetSize;
}

} //end of namespace

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
    sal_Int32 functionCount, sal_Int32 vTableOffset)
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
                code, functionOffset++, vTableOffset,
                CPPU_CURRENT_NAMESPACE::return_in_hidden_param(
                    reinterpret_cast<
            typelib_InterfaceAttributeTypeDescription * >(
            member)->pAttributeTypeRef), 0);
            // Setter:
            if (!reinterpret_cast<
                typelib_InterfaceAttributeTypeDescription * >(
                    member)->bReadOnly)
            {
                (s++)->fn = code + writetoexecdiff;
                code = codeSnippet(code, functionOffset++, vTableOffset, false, 1);
            }
            break;

        case typelib_TypeClass_INTERFACE_METHOD:
            (s++)->fn = code + writetoexecdiff;
            code = codeSnippet(
                code, functionOffset++, vTableOffset,
                CPPU_CURRENT_NAMESPACE::return_in_hidden_param(
                    reinterpret_cast<
                    typelib_InterfaceMethodTypeDescription * >(
                        member)->pReturnTypeRef),
                reinterpret_cast<
                typelib_InterfaceMethodTypeDescription * >(
                    member)->nParams);
            break;

        default:
            assert(false);
            break;
        }
        TYPELIB_DANGER_RELEASE(member);
    }
    return code;
}

// use flush code from cc50_solaris_sparc

extern "C" void doFlushCode(unsigned long address, unsigned long count);

void bridges::cpp_uno::shared::VtableFactory::flushCode(
    unsigned char const * begin, unsigned char const * end)
{
    unsigned long n = end - begin;
    if (n != 0) {
        unsigned long adr = reinterpret_cast< unsigned long >(begin);
        unsigned long off = adr & 7;
        doFlushCode(adr - off, (n + off + 7) >> 3);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
