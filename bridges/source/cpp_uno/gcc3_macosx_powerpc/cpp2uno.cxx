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

using namespace ::com::sun::star::uno;

namespace
{

//==================================================================================================
static typelib_TypeClass cpp2uno_call(
    bridges::cpp_uno::shared::CppInterfaceProxy * pThis,
    const typelib_TypeDescription * pMemberTypeDescr,
    typelib_TypeDescriptionReference * pReturnTypeRef, // 0 indicates void return
    sal_Int32 nParams, typelib_MethodParameter * pParams,
        void ** gpreg, void ** fpreg, void ** ovrflw,
    sal_Int64 * pRegisterReturn /* space for register return */ )
{

        // gpreg:  [ret *], this, [gpr params]
        // fpreg:  [fpr params]
        // ovrflw: [gpr or fpr params (space for entire parameter list in structure format properly aligned)]

    // return
    typelib_TypeDescription * pReturnTypeDescr = 0;
    if (pReturnTypeRef)
        TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );

    void * pUnoReturn = 0;
    void * pCppReturn = 0; // complex return ptr: if != 0 && != pUnoReturn, reconversion need

    sal_Int32 ngpreg = 0;
    sal_Int32 nfpreg = 0;


    if (pReturnTypeDescr)
    {
        if (bridges::cpp_uno::shared::isSimpleType( pReturnTypeDescr ))
            pUnoReturn = pRegisterReturn; // direct way for simple types
        else // complex return via ptr (pCppReturn)
        {
            pCppReturn = *gpreg;
            ngpreg++;
            ++ovrflw;

            pUnoReturn = (bridges::cpp_uno::shared::relatesToInterfaceType( pReturnTypeDescr )
                          ? alloca( pReturnTypeDescr->nSize )
                          : pCppReturn); // direct way
        }
    }
    // pop this
    ngpreg++;
    ++ovrflw;

        // after handling optional return pointer and "this"
        // make use of the space that is allocated to store all parameters in the callers stack
        // by comying the proper registers filled with parameters to that space
    char * pCppStack = (char *)ovrflw;


    sal_Int32 nPos;

    for ( nPos = 0; nPos < nParams; ++nPos )
    {
        const typelib_MethodParameter & rParam = pParams[nPos];
        if (rParam.bOut)
        {
            if (ngpreg < 8)
            {
                *(sal_Int32 *)pCppStack = ((sal_Int32 *)gpreg)[ngpreg++];
            }
            pCppStack += sizeof (sal_Int32);
        }
        else
        {
        switch (rParam.pTypeRef->eTypeClass)
        {
        case typelib_TypeClass_FLOAT:
            if (nfpreg < 13)
            {
                *(float *)pCppStack = ((double *)fpreg)[nfpreg++];
            }
            pCppStack += sizeof (float);
            ngpreg += 1;
            break;
        case typelib_TypeClass_DOUBLE:
            if (nfpreg < 13)
            {
                *(double *)pCppStack = ((double *)fpreg)[nfpreg++];
            }
            pCppStack += sizeof (double);
            ngpreg += 2;
            break;
        case typelib_TypeClass_UNSIGNED_HYPER:
        case typelib_TypeClass_HYPER:
            if (ngpreg < 8)
            {
                *(sal_Int32 *)pCppStack = ((sal_Int32 *)gpreg)[ngpreg++];
            }
            pCppStack += sizeof (sal_Int32);
                        // fall through on purpose
        default:
            if (ngpreg < 8)
            {
                *(sal_Int32 *)pCppStack = ((sal_Int32 *)gpreg)[ngpreg++];
            }
            pCppStack += sizeof (sal_Int32);
        }
        }
    }

        // now the stack has all of the parameters stored in it ready to be processed
        // so we are ready to build the uno call stack
    pCppStack = (char *)ovrflw;

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

    for ( nPos = 0; nPos < nParams; ++nPos )
    {
        const typelib_MethodParameter & rParam = pParams[nPos];
        typelib_TypeDescription * pParamTypeDescr = 0;
        TYPELIB_DANGER_GET( &pParamTypeDescr, rParam.pTypeRef );

        if (!rParam.bOut && bridges::cpp_uno::shared::isSimpleType( pParamTypeDescr ))
                // value
        {
            switch (pParamTypeDescr->eTypeClass)
            {
            case typelib_TypeClass_BOOLEAN:
            case typelib_TypeClass_BYTE:
                pCppArgs[nPos] = pCppStack +3;
                pUnoArgs[nPos] = pCppStack +3;
                break;
            case typelib_TypeClass_CHAR:
            case typelib_TypeClass_SHORT:
            case typelib_TypeClass_UNSIGNED_SHORT:
                pCppArgs[nPos] = pCppStack +2;
                pUnoArgs[nPos] = pCppStack +2;
                break;
            case typelib_TypeClass_HYPER:
            case typelib_TypeClass_UNSIGNED_HYPER:
            case typelib_TypeClass_DOUBLE:
                pCppArgs[nPos] = pCppStack;
                pUnoArgs[nPos] = pCppStack;
                pCppStack += sizeof(sal_Int32); // extra long (two regs)
                break;
            default:
                pCppArgs[nPos] = pCppStack;
                pUnoArgs[nPos] = pCppStack;
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
        pCppStack += sizeof(sal_Int32); // standard parameter length
    }


    // ExceptionHolder
    uno_Any aUnoExc; // Any will be constructed by callee
    uno_Any * pUnoExc = &aUnoExc;

    // invoke uno dispatch call
    (*pThis->getUnoI()->pDispatcher)(
         pThis->getUnoI(), pMemberTypeDescr, pUnoReturn, pUnoArgs, &pUnoExc );

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

        CPPU_CURRENT_NAMESPACE::raiseException(
                    &aUnoExc, pThis->getBridge()->getUno2Cpp() );
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

    // gpreg:  [ret *], this, [other gpr params]
    // fpreg:  [fpr params]
    // ovrflw: [gpr or fpr params (in space allocated for all params properly aligned)]

        void * pThis;
    if( nFunctionIndex & 0x80000000 )
    {
        nFunctionIndex &= 0x7fffffff;
        pThis = gpreg[1];
    }
    else
        {
        pThis = gpreg[0];
        }

        pThis = static_cast< char * >(pThis) - nVtableOffset;
        bridges::cpp_uno::shared::CppInterfaceProxy * pCppI
    = bridges::cpp_uno::shared::CppInterfaceProxy::castInterfaceToProxy(pThis);


    typelib_InterfaceTypeDescription * pTypeDescr = pCppI->getTypeDescr();

    OSL_ENSURE( nFunctionIndex < pTypeDescr->nMapFunctionIndexToMemberIndex, "### illegal vtable index!" );
    if (nFunctionIndex >= pTypeDescr->nMapFunctionIndexToMemberIndex)
    {
        throw RuntimeException(
            rtl::OUString( "illegal vtable index!" ),
            (XInterface *)pThis );
    }

    // determine called method
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
                    (void **)&pInterface, pCppI->getOid().pData, (typelib_InterfaceTypeDescription *)pTD );

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
            rtl::OUString( "no member description found!" ),
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
static void cpp_vtable_call( int nFunctionIndex, int nVtableOffset, void** gpregptr, void** fpregptr, void** ovrflw)
{
        sal_Int32     gpreg[8];
        double        fpreg[13];

        // FIXME: why are we restoring the volatile ctr register here
    sal_Int32   ctrsave = ((sal_Int32*)gpregptr)[-1];

        memcpy( gpreg, gpregptr, 32);
        memcpy( fpreg, fpregptr, 104);

    volatile long nRegReturn[2];

        // sal_Bool bComplex = nFunctionIndex & 0x80000000 ? sal_True : sal_False;

    typelib_TypeClass aType =
             cpp_mediate( nFunctionIndex, nVtableOffset, (void**)gpreg, (void**)fpreg, ovrflw, (sal_Int64*)nRegReturn );

        // FIXME: why are we restoring the volatile ctr register here
        // FIXME: and why are we putting back the values for r4, r5, and r6 as well
        // FIXME: this makes no sense to me, all of these registers are volatile!
    __asm__( "lwz r4, %0\n\t"
        "mtctr r4\n\t"
        "lwz r4, %1\n\t"
        "lwz r5, %2\n\t"
        "lwz r6, %3\n\t"
         : : "m"(ctrsave), "m"(gpreg[1]), "m"(gpreg[2]), "m"(gpreg[3]) );

    switch( aType )
    {

                // move return value into register space
                // (will be loaded by machine code snippet)

                case typelib_TypeClass_BOOLEAN:
                case typelib_TypeClass_BYTE:
                  __asm__( "lbz r3,%0\n\t" : :
               "m"(nRegReturn[0]) );
                  break;

                case typelib_TypeClass_CHAR:
                case typelib_TypeClass_SHORT:
                case typelib_TypeClass_UNSIGNED_SHORT:
                  __asm__( "lhz r3,%0\n\t" : :
               "m"(nRegReturn[0]) );
                  break;

        case typelib_TypeClass_FLOAT:
                  __asm__( "lfs f1,%0\n\t" : :
                           "m" (*((float*)nRegReturn)) );
          break;

        case typelib_TypeClass_DOUBLE:
          __asm__( "lfd f1,%0\n\t" : :
                           "m" (*((double*)nRegReturn)) );
          break;

        case typelib_TypeClass_HYPER:
        case typelib_TypeClass_UNSIGNED_HYPER:
          __asm__( "lwz r4,%0\n\t" : :
                           "m"(nRegReturn[1]) );  // fall through

        default:
          __asm__( "lwz r3,%0\n\t" : :
                           "m"(nRegReturn[0]) );
          break;
    }
}


int const codeSnippetSize = 136;

unsigned char * codeSnippet( unsigned char * code, sal_Int32 functionIndex,
                  sal_Int32 vtableOffset, bool simpleRetType )
{
    if (! simpleRetType )
        functionIndex |= 0x80000000;

    // OSL_ASSERT( sizeof (long) == 4 );

    // FIXME: why are we leaving an 8k gap in the stack here
    // FIXME: is this to allow room for signal handling frames?
    // FIXME: seems like overkill here but this is what was done for Mac OSX for gcc2
    // FIXME: also why no saving of the non-volatile CR pieces here, to be safe
    // FIXME: we probably should

    /* generate this code */

    // # so first save gpr 3 to gpr 10 (aligned to 4)
    //  stw   r3, -8000(r1)
    //  stw   r4, -7996(r1)
    //  stw   r5, -7992(r1)
    //  stw   r6, -7988(r1)
    //  stw   r7, -7984(r1)
    //  stw   r8, -7980(r1)
    //  stw   r9, -7976(r1)
    //  stw   r10,-7972(r1)

    // # next save fpr 1 to fpr 13 (aligned to 8)
    //  stfd  f1, -7968(r1)
    //  stfd  f2, -7960(r1)
    //  stfd  f3, -7952(r1)
    //  stfd  f4, -7944(r1)
    //  stfd  f5, -7936(r1)
    //  stfd  f6, -7928(r1)
    //  stfd  f7, -7920(r1)
    //  stfd  f8, -7912(r1)
    //  stfd  f9, -7904(r1)
    //  stfd  f10,-7896(r1)
    //  stfd  f11,-7888(r1)
    //  stfd  f12,-7880(r1)
    //  stfd  f13,-7872(r1)

    // FIXME: ctr is volatile, while are we saving it and not CR?
    // mfctr r3
    // stw r3, -8004(r1)

    // # now here is where cpp_vtable_call must go
    // lis r3,0xdead
    // ori r3,r3,0xbeef
    // mtctr r3

    // # now load up the functionIndex number
    // lis r3, 0xdead
    // ori r3,r3,0xbeef

    // # now load up the vtableOffset
    // lis r4, 0xdead
    // ori r4,r4,0xbeef

    // #now load up the pointer to the saved gpr registers
    // addi r5,r1,-8000

    // #now load up the pointer to the saved fpr registers
    // addi r6,r1,-7968

    // #now load up the pointer to the overflow call stack
    // addi r7,r1,24 # frame pointer plus 24

    // bctr

    unsigned long * p = (unsigned long *) code;

    * p++ = 0x9061e0c0;
    * p++ = 0x9081e0c4;
    * p++ = 0x90a1e0c8;
    * p++ = 0x90c1e0cc;
    * p++ = 0x90e1e0d0;
    * p++ = 0x9101e0d4;
    * p++ = 0x9121e0d8;
    * p++ = 0x9141e0dc;
    * p++ = 0xd821e0e0;
    * p++ = 0xd841e0e8;
    * p++ = 0xd861e0f0;
    * p++ = 0xd881e0f8;
    * p++ = 0xd8a1e100;
    * p++ = 0xd8c1e108;
    * p++ = 0xd8e1e110;
    * p++ = 0xd901e118;
    * p++ = 0xd921e120;
    * p++ = 0xd941e128;
    * p++ = 0xd961e130;
    * p++ = 0xd981e138;
    * p++ = 0xd9a1e140;
    * p++ = 0x7c6902a6;
    * p++ = 0x9061e0bc;
    * p++ = 0x3c600000 | (((unsigned long)cpp_vtable_call) >> 16);
    * p++ = 0x60630000 | (((unsigned long)cpp_vtable_call) & 0x0000FFFF);
    * p++ = 0x7c6903a6;
    * p++ = 0x3c600000 | (((unsigned long)functionIndex) >> 16);
    * p++ = 0x60630000 | (((unsigned long)functionIndex) & 0x0000FFFF);
    * p++ = 0x3c800000 | (((unsigned long)vtableOffset) >> 16);
    * p++ = 0x60840000 | (((unsigned long)vtableOffset) & 0x0000FFFF);
    * p++ = 0x38a1e0c0;
    * p++ = 0x38c1e0e0;
    * p++ = 0x38e10018;
    * p++ = 0x4e800420;

    return (code + codeSnippetSize);

}


}

void bridges::cpp_uno::shared::VtableFactory::flushCode(unsigned char const * bptr, unsigned char const * eptr)
{
    int const lineSize = 32;
    for (unsigned char const * p = bptr; p < eptr + lineSize; p += lineSize) {
        __asm__ volatile ("dcbst 0, %0" : : "r"(p) : "memory");
    }
    __asm__ volatile ("sync" : : : "memory");
    for (unsigned char const * p = bptr; p < eptr + lineSize; p += lineSize) {
        __asm__ volatile ("icbi 0, %0" : : "r"(p) : "memory");
    }
    __asm__ volatile ("isync" : : : "memory");
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
    Slot ** slots, unsigned char * code,
    typelib_InterfaceTypeDescription const * type, sal_Int32 functionOffset,
    sal_Int32 functionCount, sal_Int32 vtableOffset)
{
    (*slots) -= functionCount;
    Slot * s = *slots;

  // fprintf(stderr, "in addLocalFunctions functionOffset is %x\n",functionOffset);
  // fprintf(stderr, "in addLocalFunctions vtableOffset is %x\n",vtableOffset);
  // fflush(stderr);

    for (sal_Int32 i = 0; i < type->nMembers; ++i) {
        typelib_TypeDescription * member = 0;
        TYPELIB_DANGER_GET(&member, type->ppMembers[i]);
        OSL_ASSERT(member != 0);
        switch (member->eTypeClass) {
        case typelib_TypeClass_INTERFACE_ATTRIBUTE:
            // Getter:
            (s++)->fn = code;
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
                (s++)->fn = code;
                code = codeSnippet(code, functionOffset++, vtableOffset, true);
            }
            break;

        case typelib_TypeClass_INTERFACE_METHOD:
            (s++)->fn = code;
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
