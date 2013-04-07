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
#include <uno/data.h>
#include <typelib/typedescription.hxx>

#include "bridges/cpp_uno/shared/bridge.hxx"
#include "bridges/cpp_uno/shared/cppinterfaceproxy.hxx"
#include "bridges/cpp_uno/shared/types.hxx"
#include "bridges/cpp_uno/shared/vtablefactory.hxx"

#include "share.hxx"
#include <stdio.h>
#include <string.h>

using namespace ::com::sun::star::uno;

namespace
{

static typelib_TypeClass cpp2uno_call(
    bridges::cpp_uno::shared::CppInterfaceProxy * pThis,
    const typelib_TypeDescription * pMemberTypeDescr,
    typelib_TypeDescriptionReference * pReturnTypeRef, // 0 indicates void return
    sal_Int32 nParams, typelib_MethodParameter * pParams,
        void ** gpreg, void ** fpreg, void ** ovrflw,
    sal_Int64 * pRegisterReturn /* space for register return */ )
{
        int ng = 0; //number of gpr registers used
        int nf = 0; //number of fpr regsiters used
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
        }
        else // complex return via ptr (pCppReturn)
        {
            pCppReturn = *(void **)gpreg;
                        gpreg++;
                        ng++;

            pUnoReturn = (bridges::cpp_uno::shared::relatesToInterfaceType( pReturnTypeDescr )
                          ? alloca( pReturnTypeDescr->nSize )
                          : pCppReturn); // direct way
        }
    }
    // pop this
        gpreg++;
        ng++;

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

    for ( sal_Int32 nPos = 0; nPos < nParams; ++nPos )
    {
        const typelib_MethodParameter & rParam = pParams[nPos];
        typelib_TypeDescription * pParamTypeDescr = 0;
        TYPELIB_DANGER_GET( &pParamTypeDescr, rParam.pTypeRef );

        if (!rParam.bOut && bridges::cpp_uno::shared::isSimpleType( pParamTypeDescr )) // value
        {

            switch (pParamTypeDescr->eTypeClass)
            {

              case typelib_TypeClass_DOUBLE:
               if (nf < 2) {
                  pCppArgs[nPos] = fpreg;
                  pUnoArgs[nPos] = fpreg;
                  nf++;
                  fpreg += 2;
               } else {
                pCppArgs[nPos] = ovrflw;
                pUnoArgs[nPos] = ovrflw;
                    ovrflw += 2;
               }
               break;

               case typelib_TypeClass_FLOAT:
                // fpreg are all double values so need to
                // modify fpreg to be a single word float value
                if (nf < 2) {
//                 float tmp = (float) (*((double *)fpreg));
//                 (*((float *) fpreg)) = tmp;
                   pCppArgs[nPos] = fpreg;
                   pUnoArgs[nPos] = fpreg;
                   nf++;
                   fpreg += 2;
                } else {
                              pCppArgs[nPos] = ovrflw;
                  pUnoArgs[nPos] = ovrflw;
                  ovrflw += 1;
                            }
                break;

            case typelib_TypeClass_HYPER:
            case typelib_TypeClass_UNSIGNED_HYPER:
             if (ng < 4) {
                pCppArgs[nPos] = gpreg;
                pUnoArgs[nPos] = gpreg;
                ng += 2;
                gpreg += 2;
             } else {
                pCppArgs[nPos] = ovrflw;
                pUnoArgs[nPos] = ovrflw;
                ovrflw += 2;
              }
              break;

            case typelib_TypeClass_BYTE:
            case typelib_TypeClass_BOOLEAN:
             if (ng < 5) {
                  pCppArgs[nPos] = (((char *)gpreg) + 3);
                  pUnoArgs[nPos] = (((char *)gpreg) + 3);
                  ng++;
                  gpreg++;
             } else {
                  pCppArgs[nPos] = (((char *)ovrflw) + 3);
                  pUnoArgs[nPos] = (((char *)ovrflw) + 3);
                  ovrflw++;
             }
             break;


               case typelib_TypeClass_CHAR:
               case typelib_TypeClass_SHORT:
               case typelib_TypeClass_UNSIGNED_SHORT:
            if (ng < 5) {
                  pCppArgs[nPos] = (((char *)gpreg)+ 2);
                  pUnoArgs[nPos] = (((char *)gpreg)+ 2);
                  ng++;
                  gpreg++;
            } else {
                  pCppArgs[nPos] = (((char *)ovrflw) + 2);
                  pUnoArgs[nPos] = (((char *)ovrflw) + 2);
                  ovrflw++;
            }
            break;


              default:
            if (ng < 5) {
                  pCppArgs[nPos] = gpreg;
                  pUnoArgs[nPos] = gpreg;
                  ng++;
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

                if (ng < 5) {
                  pCppArgs[nPos] = *(void **)gpreg;
                  pCppStack = gpreg;
                  ng++;
                  gpreg++;
                } else {
                  pCppArgs[nPos] = *(void **)ovrflw;
                  pCppStack = ovrflw;
                 ovrflw++;
                }

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
            }
            else // direct way
            {
                pUnoArgs[nPos] = *(void **)pCppStack;
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
        for ( ; nTempIndizes--; )
        {
            sal_Int32 nIndex = pTempIndizes[nTempIndizes];

            if (pParams[nIndex].bIn) // is in/inout => was constructed
                uno_destructData( pUnoArgs[nIndex], ppTempParamTypeDescr[nTempIndizes], 0 );
            TYPELIB_DANGER_RELEASE( ppTempParamTypeDescr[nTempIndizes] );
        }
        if (pReturnTypeDescr)
            TYPELIB_DANGER_RELEASE( pReturnTypeDescr );

        CPPU_CURRENT_NAMESPACE::raiseException( &aUnoExc, pThis->getBridge()->getUno2Cpp() ); // has to destruct the any
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


//============================================================================
static typelib_TypeClass cpp_mediate(
    sal_uInt32 nOffsetAndIndex,
    void ** gpreg, void ** fpreg, void ** ovrflw,
    sal_Int64 * pRegisterReturn /* space for register return */ )
{
    OSL_ENSURE( sizeof(sal_Int32)==sizeof(void *), "### unexpected!" );

    sal_Int16 nVtableOffset = (nOffsetAndIndex >> 16);
    sal_Int16 nFunctionIndex = (nOffsetAndIndex & 0xFFFF);

    // gpreg:  [ret *], this, [other gpr params]
    // fpreg:  [fpr params]
    // ovrflw: [gpr or fpr params (properly aligned)]

    // _this_ ptr is patched cppu_XInterfaceProxy object
    void * pThis;
    if( nFunctionIndex & 0x8000 )
    {
        nFunctionIndex &= 0x7fff;
        pThis = gpreg[1];
    }
    else
    {
        pThis = gpreg[0];
    }

    pThis = static_cast< char * >(pThis) - nVtableOffset;

    bridges::cpp_uno::shared::CppInterfaceProxy * pCppI
        = bridges::cpp_uno::shared::CppInterfaceProxy::castInterfaceToProxy(
            pThis);

    typelib_InterfaceTypeDescription * pTypeDescr = pCppI->getTypeDescr();


    OSL_ENSURE( nFunctionIndex < pTypeDescr->nMapFunctionIndexToMemberIndex, "### illegal vtable index!" );
    if (nFunctionIndex >= pTypeDescr->nMapFunctionIndexToMemberIndex)
    {
        throw RuntimeException(
            OUString( "illegal vtable index!" ),
            (XInterface *)pCppI );
    }

    // determine called method
    OSL_ENSURE( nVtableCall < pTypeDescr->nMapFunctionIndexToMemberIndex, "### illegal vtable index!" );
    sal_Int32 nMemberPos = pTypeDescr->pMapFunctionIndexToMemberIndex[nFunctionIndex];
    OSL_ENSURE( nMemberPos < pTypeDescr->nAllMembers, "### illegal member index!" );

    TypeDescription aMemberDescr( pTypeDescr->ppAllMembers[nMemberPos] );

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
        throw RuntimeException(
            OUString( "no member description found!" ),
            (XInterface *)pCppI );
    }
    }

    return eRet;
}

//==================================================================================================
/**
 * is called on incoming vtable calls
 * (called by asm snippets)
 */
static void privateSnippetExecutor( sal_uInt32 nOffsetAndIndex, void** gpregptr, void** fpregptr, void** ovrflw)
{
#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr, "privateSnippetExecutor\n");
#endif


    sal_Int32     gpreg[8];
    double        fpreg[8];

    memcpy( gpreg, gpregptr, 32);
    memcpy( fpreg, fpregptr, 64);

    volatile long nRegReturn[2];

    typelib_TypeClass aType =
        cpp_mediate( nOffsetAndIndex, (void**)gpreg, (void**)fpreg, ovrflw,
        (sal_Int64*)nRegReturn );

    switch( aType )
    {

                // move return value into register space
                // (will be loaded by machine code snippet)

                case typelib_TypeClass_BOOLEAN:
          {
                  unsigned long tmp = (unsigned long)(*(unsigned char *)nRegReturn);
                  __asm__ volatile ( "l 2,%0\n\t" : :
               "m"(tmp) : "2" );
                  break;
          }
                case typelib_TypeClass_BYTE:
          {
                  long tmp = (long)(*(signed char *)nRegReturn);
                  __asm__ volatile ( "l 2,%0\n\t" : :
               "m"(tmp) : "2" );
                  break;
          }
                case typelib_TypeClass_CHAR:
                case typelib_TypeClass_UNSIGNED_SHORT:
          {
                  unsigned long tmp = (unsigned long)(*(unsigned short *)nRegReturn);
                  __asm__ volatile ( "l 2,%0\n\t" : :
               "m"(tmp) : "2" );
                  break;
          }
                case typelib_TypeClass_SHORT:
          {
                  long tmp = (long)(*(short *)nRegReturn);
                  __asm__ volatile ( "l 2,%0\n\t" : :
               "m"(tmp) : "2" );
                  break;
          }
        case typelib_TypeClass_FLOAT:
                  __asm__ volatile ( "le 0,%0\n\t" : :
                           "m" (*((float*)nRegReturn)) : "16" );
          break;

        case typelib_TypeClass_DOUBLE:
          __asm__ volatile ( "ld 0,%0\n\t" : :
                           "m" (*((double*)nRegReturn)) : "16" );
          break;

        case typelib_TypeClass_HYPER:
        case typelib_TypeClass_UNSIGNED_HYPER:
          __asm__ volatile ( "lm 2,3,%0\n\t" : :
                           "m"(nRegReturn[0]) : "2", "3" );
          break;

        default:
          __asm__ volatile ( "l 2,%0\n\t" : :
                           "m"(nRegReturn[0]) : "2" );
          break;
    }
}

const int codeSnippetSize = 50;

unsigned char* codeSnippet( unsigned char * code, sal_Int16 nFunctionIndex, sal_Int16 nVtableOffset, bool simple_ret_type )
{
    sal_uInt32 nOffsetAndIndex = ( ( nVtableOffset ) << 16 ) | ( nFunctionIndex );

    if (! simple_ret_type)
        nOffsetAndIndex |= 0x8000;

    OSL_ASSERT( sizeof (long) == 4 );

     /* generate this code */
     // lr      %r0,%r13
     // bras    %r13,0x6
     // .long   privateSnippetExecutor
     // .long   nOffsetAndIndex
     // stm     %r2,%r6,8(%r15)
     // std     %f0,64(%r15)
     // std     %f2,72(%r15)

     // l       %r2,4(%r13)
     // la      %r3,8(%r15)
     // la      %r4,64(%r15)
     // la      %r5,96(%r15)
     // l       %r1,0(%r13)
     // lr      %r13,%r0
     // br      %r1

    unsigned char * p = code;
    *reinterpret_cast< sal_Int16 * >(p) = 0x180d;
    p += sizeof(sal_Int16);
    *reinterpret_cast< sal_Int32 * >(p) = 0xa7d50006;
    p += sizeof(sal_Int32);
    *reinterpret_cast< sal_Int32 * >(p) =
        reinterpret_cast< sal_Int32 >(privateSnippetExecutor);
    p += sizeof(sal_Int32);
    *reinterpret_cast< sal_Int32 * >(p) = nOffsetAndIndex;
    p += sizeof(sal_Int32);
    *reinterpret_cast< sal_Int32 * >(p) = 0x9026f008;
    p += sizeof(sal_Int32);
    *reinterpret_cast< sal_Int32 * >(p) = 0x6000f040;
    p += sizeof(sal_Int32);
    *reinterpret_cast< sal_Int32 * >(p) = 0x6020f048;
    p += sizeof(sal_Int32);
    *reinterpret_cast< sal_Int32 * >(p) = 0x5820d004;
    p += sizeof(sal_Int32);
    *reinterpret_cast< sal_Int32 * >(p) = 0x4130f008;
    p += sizeof(sal_Int32);
    *reinterpret_cast< sal_Int32 * >(p) = 0x4140f040;
    p += sizeof(sal_Int32);
    *reinterpret_cast< sal_Int32 * >(p) = 0x4150f060;
    p += sizeof(sal_Int32);
    *reinterpret_cast< sal_Int32 * >(p) = 0x5810d000;
    p += sizeof(sal_Int32);
    *reinterpret_cast< sal_Int16 * >(p) = 0x18d0;
    p += sizeof(sal_Int16);
    *reinterpret_cast< sal_Int16 * >(p) = 0x07f1;
    p += sizeof(sal_Int16);

    return (code + codeSnippetSize);
}
}

void bridges::cpp_uno::shared::VtableFactory::flushCode(unsigned char const *, unsigned char const *)
{
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
    slots[-2].fn = 0;
    slots[-1].fn = 0;
    return slots + slotCount;
}

unsigned char * bridges::cpp_uno::shared::VtableFactory::addLocalFunctions(
    Slot ** slots, unsigned char * code, sal_PtrDiff writetoexecdiff,
    typelib_InterfaceTypeDescription const * type, sal_Int32 functionOffset,
    sal_Int32 functionCount, sal_Int32 vtableOffset)
{
     (*slots) -= functionCount;
     Slot * s = *slots;
#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr, "in addLocalFunctions functionOffset is %x\n",functionOffset);
    fprintf(stderr, "in addLocalFunctions vtableOffset is %x\n",vtableOffset);
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
