/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cpp2uno.cxx,v $
 * $Revision: 1.14 $
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
#include <com/sun/star/uno/genfunc.hxx>
#include <typelib/typedescription.hxx>
#include <uno/data.h>
#include "bridges/cpp_uno/shared/bridge.hxx"
#include "bridges/cpp_uno/shared/cppinterfaceproxy.hxx"
#include "bridges/cpp_uno/shared/types.hxx"
#include "bridges/cpp_uno/shared/vtablefactory.hxx"
#include "share.hxx"

using namespace com::sun::star::uno;

namespace
{
//==================================================================================================
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

    if (pReturnTypeDescr)
    {
        if (bridges::cpp_uno::shared::isSimpleType( pReturnTypeDescr ))
            pUnoReturn = pRegisterReturn; // direct way for simple types
        else // complex return via ptr (pCppReturn)
        {
            pCppReturn = *(void**)pCppStack;
            pUnoReturn = (bridges::cpp_uno::shared::relatesToInterfaceType(
                                   pReturnTypeDescr )
                          ? alloca( pReturnTypeDescr->nSize )
                          : pCppReturn); // direct way
            pCppStack += sizeof( void* );
        }
    }
    // pop this
    pCppStack += sizeof( void* );

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

         if (!rParam.bOut && bridges::cpp_uno::shared::isSimpleType( pParamTypeDescr ))  // value
        {
            pCppArgs[nPos] = pUnoArgs[nPos] = CPPU_CURRENT_NAMESPACE::adjustPointer(pCppStack, pParamTypeDescr);
            switch (pParamTypeDescr->eTypeClass)
            {
            case typelib_TypeClass_HYPER:
            case typelib_TypeClass_UNSIGNED_HYPER:
                    case typelib_TypeClass_DOUBLE:
                    {
            if ((reinterpret_cast< long >(pCppStack) & 7) != 0)
                      {
                   OSL_ASSERT( sizeof (double) == sizeof (sal_Int64) );
                           void * pDest = alloca( sizeof (sal_Int64) );
                           *reinterpret_cast< sal_Int32 * >(pDest) =
                           *reinterpret_cast< sal_Int32 const * >(pCppStack);
                           *(reinterpret_cast< sal_Int32 * >(pDest) + 1) =
                           *(reinterpret_cast< sal_Int32 const * >(pCppStack) + 1);
                           pCppArgs[nPos] = pUnoArgs[nPos] = pDest;
            }
               pCppStack += sizeof (sal_Int32); // extra long
                       break;
            default:
            break;
            }
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
            else if (bridges::cpp_uno::shared::relatesToInterfaceType(
                              pParamTypeDescr ))
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
    (*pThis->getUnoI()->pDispatcher)(pThis->getUnoI(), pMemberTypeDescr, pUnoReturn, pUnoArgs, &pUnoExc );

    // in case an exception occured...
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
        CPPU_CURRENT_NAMESPACE::raiseException(&aUnoExc, pThis->getBridge()->getUno2Cpp() );
                 // has to destruct the any
        // is here for dummy
        return typelib_TypeClass_VOID;
    }
    else // else no exception occured...
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


//==================================================================================================
static typelib_TypeClass cpp_mediate(
    sal_Int32   nFunctionIndex,
    sal_Int32   nVtableOffset,
    void ** pCallStack,
    sal_Int64 * pRegisterReturn /* space for register return */ )
{
    OSL_ENSURE( sizeof(sal_Int32)==sizeof(void *), "### unexpected!" );

    // pCallStack: this, params
    // eventual [ret*] lies at pCallStack -1
    // so count down pCallStack by one to keep it simple
    // pCallStack: this, params
    // eventual [ret*] lies at pCallStack -1
    // so count down pCallStack by one to keep it simple
    bridges::cpp_uno::shared::CppInterfaceProxy * pCppI
        = bridges::cpp_uno::shared::CppInterfaceProxy::castInterfaceToProxy(
    static_cast< char * >(*pCallStack) - nVtableOffset);
    if ((nFunctionIndex & 0x80000000) != 0) {
        nFunctionIndex &= 0x7FFFFFFF;
        --pCallStack;
    }

    typelib_InterfaceTypeDescription * pTypeDescr = pCppI->getTypeDescr();

    OSL_ENSURE( nFunctionIndex < pTypeDescr->nMapFunctionIndexToMemberIndex,
                 "### illegal vtable index!" );
    if (nFunctionIndex >= pTypeDescr->nMapFunctionIndexToMemberIndex)
    {
        throw RuntimeException( rtl::OUString::createFromAscii("illegal vtable index!"), (XInterface *)pCppI );
    }

    // determine called method
    sal_Int32 nMemberPos = pTypeDescr->pMapFunctionIndexToMemberIndex[nFunctionIndex];
    OSL_ENSURE( nMemberPos < pTypeDescr->nAllMembers, "### illegal member index!" );

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
        throw RuntimeException(rtl::OUString::createFromAscii("no member description found!"), (XInterface *)pCppI );
        // is here for dummy
        eRet = typelib_TypeClass_VOID;
    }
    }
    return eRet;
}



//==================================================================================================
/**
 * is called on incoming vtable calls
 * (called by asm snippets)
 */
static void cpp_vtable_call()
{
    sal_Int64 nRegReturn;
    int nFunctionIndex;
    void** pCallStack;
    int vTableOffset;

void * pRegReturn = &nRegReturn;

    __asm__( "st %%i0, %0\n\t"
            "st %%i1, %1\n\t"
             "st %%i2, %2\n\t"
            : : "m"(nFunctionIndex), "m"(pCallStack), "m"(vTableOffset) );

//  fprintf(stderr,"cpp_mediate nFunctionIndex=%x\n",nFunctionIndex);
//  fflush(stderr);

    sal_Bool bComplex = nFunctionIndex & 0x80000000 ? sal_True : sal_False;
    typelib_TypeClass aType =
        cpp_mediate( nFunctionIndex, vTableOffset, pCallStack+17, (sal_Int64*)&nRegReturn );

    switch( aType )
    {
        case typelib_TypeClass_BOOLEAN:
        case typelib_TypeClass_BYTE:
            __asm__( "ld %0, %%l0\n\t"
                     "ldsb [%%l0], %%i0\n"
                     : : "m"(pRegReturn) );
            break;
        case typelib_TypeClass_CHAR:
        case typelib_TypeClass_SHORT:
        case typelib_TypeClass_UNSIGNED_SHORT:
            __asm__( "ld %0, %%l0\n\t"
                     "ldsh [%%l0], %%i0\n"
                     : : "m"(pRegReturn) );
            break;
        case typelib_TypeClass_HYPER:
        case typelib_TypeClass_UNSIGNED_HYPER:
            __asm__( "ld %0, %%l0\n\t"
                     "ld [%%l0], %%i0\n\t"
                     "add %%l0, 4, %%l0\n\t"
                     "ld [%%l0], %%i1\n\t"
                      : : "m"(pRegReturn) );

            break;
        case typelib_TypeClass_FLOAT:
            __asm__( "ld %0, %%l0\n\t"
                     "ld [%%l0], %%f0\n"
                     : : "m"(pRegReturn) );
            break;
        case typelib_TypeClass_DOUBLE:
            __asm__( "ld %0, %%l0\n\t"
                     "ldd [%%l0], %%f0\n"
                     : : "m"(pRegReturn) );
            break;
        case typelib_TypeClass_VOID:
            break;
        default:
            __asm__( "ld %0, %%l0\n\t"
                     "ld [%%l0], %%i0\n"
                     : : "m"(pRegReturn) );
            break;
    }

    if( bComplex )
    {
        __asm__( "add %i7, 4, %i7\n\t" );
        // after call to complex return valued funcion there is an unimp instruction
    }

}
//__________________________________________________________________________________________________

int const codeSnippetSize = 56;
unsigned char * codeSnippet(
    unsigned char * code, sal_Int32 functionIndex, sal_Int32 vtableOffset,
    bool simpleRetType)
{
    sal_uInt32 index = functionIndex;
    if (!simpleRetType) {
        index |= 0x80000000;
    }
    unsigned int * p = reinterpret_cast< unsigned int * >(code);
    OSL_ASSERT(sizeof (unsigned int) == 4);
    // st %o0, [%sp+68]:
    *p++ = 0xD023A044;
    // st %o1, [%sp+72]:
    *p++ = 0xD223A048;
    // st %o2, [%sp+76]:
    *p++ = 0xD423A04C;
    // st %o3, [%sp+80]:
    *p++ = 0xD623A050;
    // st %o4, [%sp+84]:
    *p++ = 0xD823A054;
    // st %o5, [%sp+88]:
    *p++ = 0xDA23A058;
    // sethi %hi(index), %o0:
    *p++ = 0x11000000 | (index >> 10);
    // or %o0, %lo(index), %o0:
    *p++ = 0x90122000 | (index & 0x3FF);
    // sethi %hi(vtableOffset), %o2:
    *p++ = 0x15000000 | (vtableOffset >> 10);
    // or %o2, %lo(vtableOffset), %o2:
    *p++ = 0x9412A000 | (vtableOffset & 0x3FF);
    // sethi %hi(cpp_vtable_call), %o3:
    *p++ = 0x17000000 | (reinterpret_cast< unsigned int >(cpp_vtable_call) >> 10);
    // or %o3, %lo(cpp_vtable_call), %o3:
    *p++ = 0x9612E000 | (reinterpret_cast< unsigned int >(cpp_vtable_call) & 0x3FF);
    // jmpl %o3, %g0:
    *p++ = 0x81C2C000;
    // mov %sp, %o1:
    *p++ = 0x9210000E;
    OSL_ASSERT(
        reinterpret_cast< unsigned char * >(p) - code <= codeSnippetSize);
    return code + codeSnippetSize;
}

} //end of namespace

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
    Slot ** slots, unsigned char * code,
    typelib_InterfaceTypeDescription const * type, sal_Int32 functionOffset,
    sal_Int32 functionCount, sal_Int32 vTableOffset)
{
    (*slots) -= functionCount;
    Slot * s = *slots;
    for (sal_Int32 i = 0; i < type->nMembers; ++i) {
        typelib_TypeDescription * member = 0;
        TYPELIB_DANGER_GET(&member, type->ppMembers[i]);
        OSL_ASSERT(member != 0);
        switch (member->eTypeClass) {
        case typelib_TypeClass_INTERFACE_ATTRIBUTE:
            // Getter:
            (s++)->fn = code;
            code = codeSnippet(
                code, functionOffset++, vTableOffset,
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
                code = codeSnippet(code, functionOffset++, vTableOffset, true);
            }
            break;

        case typelib_TypeClass_INTERFACE_METHOD:
            (s++)->fn = code;
            code = codeSnippet(
                code, functionOffset++, vTableOffset,
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
