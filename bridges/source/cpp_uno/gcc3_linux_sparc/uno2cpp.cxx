/*************************************************************************
 *
 *  $RCSfile: uno2cpp.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hjs $ $Date: 2003-08-18 14:56:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <malloc.h>
#ifndef _RTL_ALLOC_H_
#include <rtl/alloc.h>
#endif

#ifndef _UNO_DATA_H_
#include <uno/data.h>
#endif
#ifndef _BRIDGES_CPP_UNO_BRIDGE_HXX_
#include <bridges/cpp_uno/bridge.hxx>
#endif
#ifndef _BRIDGES_CPP_UNO_TYPE_MISC_HXX_
#include <bridges/cpp_uno/type_misc.hxx>
#endif

#include "share.hxx"

using namespace rtl;
using namespace com::sun::star::uno;

namespace CPPU_CURRENT_NAMESPACE
{

void dummy_can_throw_anything( char const * );

//==================================================================================================
static void callVirtualMethod( void * pThis,
                                      sal_Int32 nVtableIndex,
                                      void * pRegisterReturn,
                                      typelib_TypeClass eReturnType,
                                      sal_Int32 * pStackLongs,
                                      sal_Int32 nStackLongs )
{
    // parameter list is mixed list of * and values
    // reference parameters are pointers

    OSL_ENSURE( pStackLongs && pThis, "### null ptr!" );
    OSL_ENSURE( (sizeof(void *) == 4) &&
                 (sizeof(sal_Int32) == 4), "### unexpected size of int!" );
    OSL_ENSURE( nStackLongs && pStackLongs, "### no stack in callVirtualMethod !" );

    // never called
    if (! pThis) dummy_can_throw_anything("xxx"); // address something

    volatile long o0 = 0, o1 = 0; // for register returns
    volatile double f0d = 0;
    volatile float f0f = 0;
    volatile long long saveReg[7];

    __asm__ (
        // save registers
        "std %%l0, [%4]\n\t"
        "mov %4, %%l0\n\t"
        "mov %%l0, %%l1\n\t"
        "add %%l0, 8, %%l0\n\t"
        "std %%l2, [%%l0]\n\t"
        "add %%l0, 8, %%l0\n\t"
        "std %%l4, [%%l0]\n\t"
        "add %%l0, 8, %%l0\n\t"
        "std %%o0, [%%l0]\n\t"
        "add %%l0, 8, %%l0\n\t"
        "std %%o2, [%%l0]\n\t"
        "add %%l0, 8, %%l0\n\t"
        "std %%o4, [%%l0]\n\t"
        "add %%l0, 8, %%l0\n\t"
        "std %%l6, [%%l0]\n\t"
        "mov %%l1, %%l7\n\t"

        // increase our own stackframe if necessary
        "mov %%sp, %%l3\n\t"        // save stack ptr for readjustment

        "subcc %%i5, 7, %%l0\n\t"
        "ble .LmoveOn\n\t"
        "nop\n\t"

        "sll %%l0, 2, %%l0\n\t"
        "add %%l0, 96, %%l0\n\t"
        "mov %%sp, %%l1\n\t"        // old stack ptr
        "sub %%sp, %%l0, %%l0\n\t"  // future stack ptr
        "andcc %%l0, 7, %%g0\n\t"   // align stack to 8
        "be .LisAligned\n\t"
        "nop\n\t"
        "sub %%l0, 4, %%l0\n"
    ".LisAligned:\n\t"
        "mov %%l0, %%o5\n\t"            // save newly computed stack ptr
        "add %%g0, 16, %%o4\n"

        // now copy longs down to save register window
        // and local variables
    ".LcopyDown:\n\t"
        "ld [%%l1], %%l2\n\t"
        "st %%l2,[%%l0]\n\t"
        "add %%l0, 4, %%l0\n\t"
        "add %%l1, 4, %%l1\n\t"
        "subcc %%o4, 1, %%o4\n\t"
        "bne .LcopyDown\n\t"

        "mov %%o5, %%sp\n\t"        // move new stack ptr (hopefully) atomically
        // while register window is valid in both spaces
        // (scheduling might hit in copyDown loop)

        "sub %%i5, 7, %%l0\n\t"     // copy parameters past the sixth to stack
        "add %%i4, 28, %%l1\n\t"
        "add %%sp, 92, %%l2\n"
    ".LcopyLong:\n\t"
        "ld [%%l1], %%o0\n\t"
        "st %%o0, [%%l2]\n\t"
        "add %%l1, 4, %%l1\n\t"
        "add %%l2, 4, %%l2\n\t"
        "subcc %%l0, 1, %%l0\n\t"
        "bne .LcopyLong\n\t"
        "nop\n"

    ".LmoveOn:\n\t"
        "mov %%i5, %%l0\n\t"        // prepare out registers
        "mov %%i4, %%l1\n\t"

        "ld [%%l1], %%o0\n\t"       // prepare complex return ptr
        "st %%o0, [%%sp+64]\n\t"
        "sub %%l0, 1, %%l0\n\t"
        "add %%l1, 4, %%l1\n\t"

        "ld [%%l1], %%o0\n\t"
        "subcc %%l0, 1, %%l0\n\t"
        "be .LdoCall\n\t"
        "nop\n\t"

        "add %%l1, 4, %%l1\n\t"
        "ld [%%l1], %%o1\n\t"
        "subcc %%l0, 1, %%l0\n\t"
        "be .LdoCall\n\t"
        "nop\n\t"

        "add %%l1, 4, %%l1\n\t"
        "ld [%%l1], %%o2\n\t"
        "subcc %%l0, 1, %%l0\n\t"
        "be .LdoCall\n\t"
        "nop\n\t"

        "add %%l1, 4, %%l1\n\t"
        "ld [%%l1], %%o3\n\t"
        "subcc %%l0, 1, %%l0\n\t"
        "be .LdoCall\n\t"
        "nop\n\t"

        "add %%l1, 4, %%l1\n\t"
        "ld [%%l1], %%o4\n\t"
        "subcc %%l0, 1, %%l0\n\t"
        "be .LdoCall\n\t"
        "nop\n\t"

        "add %%l1, 4, %%l1\n\t"
        "ld [%%l1], %%o5\n"

    ".LdoCall:\n\t"
        "ld [%%i0], %%l0\n\t"       // get vtable ptr

"sll %%i1, 2, %%l6\n\t"
//         "add %%l6, 8, %%l6\n\t"
        "add %%l6, %%l0, %%l0\n\t"
//      // vtable has 8byte wide entries,
//      // upper half contains 2 half words, of which the first
//      // is the this ptr patch !
//      // first entry is (or __tf)

//      "ldsh [%%l0], %%l6\n\t"     // load this ptr patch
//      "add %%l6, %%o0, %%o0\n\t"  // patch this ptr

//      "add %%l0, 4, %%l0\n\t"     // get virtual function ptr
        "ld [%%l0], %%l0\n\t"

        "ld [%%i4], %%l2\n\t"
        "subcc %%l2, %%g0, %%l2\n\t"
        "bne .LcomplexCall\n\t"
        "nop\n\t"
        "call %%l0\n\t"
        "nop\n\t"
        "ba .LcallReturned\n\t"
        "nop\n"
    ".LcomplexCall:\n\t"
        "call %%l0\n\t"
        "nop\n\t"
        "unimp\n"

    ".LcallReturned:\n\t"
        "mov %%l3, %%sp\n\t"        // readjust stack so that our locals are where they belong
        "st %%o0, %0\n\t"           // save possible return registers into our locals
        "st %%o1, %1\n\t"
        "std %%f0, %2\n\t"
        "st %%f0, %3\n\t"

        // restore registers
        "ldd [%%l7], %%l0\n\t"
        "add %%l7, 8, %%l7\n\t"
        "ldd [%%l7], %%l2\n\t"
        "add %%l7, 8, %%l7\n\t"
        "ldd [%%l7], %%l4\n\t"
        "add %%l7, 8, %%l7\n\t"
        "ldd [%%l7], %%o0\n\t"
        "add %%l7, 8, %%l7\n\t"
        "ldd [%%l7], %%o2\n\t"
        "add %%l7, 8, %%l7\n\t"
        "ldd [%%l7], %%o4\n\t"
        "add %%l7, 8, %%l7\n\t"
        "ldd [%%l7], %%l6\n\t"
        : :
        "m"(o0),
        "m"(o1),
        "m"(f0d),
        "m"(f0f),
        "r"(&saveReg[0])
        );
    switch( eReturnType )
    {
        case typelib_TypeClass_HYPER:
        case typelib_TypeClass_UNSIGNED_HYPER:
            ((long*)pRegisterReturn)[1] = o1;
        case typelib_TypeClass_LONG:
        case typelib_TypeClass_UNSIGNED_LONG:
        case typelib_TypeClass_ENUM:
            ((long*)pRegisterReturn)[0] = o0;
            break;
        case typelib_TypeClass_CHAR:
        case typelib_TypeClass_SHORT:
        case typelib_TypeClass_UNSIGNED_SHORT:
            *(unsigned short*)pRegisterReturn = (unsigned short)o0;
            break;
        case typelib_TypeClass_BOOLEAN:
        case typelib_TypeClass_BYTE:
            *(unsigned char*)pRegisterReturn = (unsigned char)o0;
            break;
        case typelib_TypeClass_FLOAT:
            *(float*)pRegisterReturn = f0f;
            break;
        case typelib_TypeClass_DOUBLE:
            *(double*)pRegisterReturn = f0d;
            break;
    }
}

//==================================================================================================
static void cpp_call(
    cppu_unoInterfaceProxy * pThis,
    sal_Int32 nVtableCall,
    typelib_TypeDescriptionReference * pReturnTypeRef,
    sal_Int32 nParams, typelib_MethodParameter * pParams,
    void * pUnoReturn, void * pUnoArgs[], uno_Any ** ppUnoExc )
{
      // max space for: complex ret ptr, this, values|ptr ...
      char * pCppStack      =
          (char *)alloca( (nParams+2) * sizeof(sal_Int64) );
      char * pCppStackStart = pCppStack;

    // return
    typelib_TypeDescription * pReturnTypeDescr = 0;
    TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );
    OSL_ENSURE( pReturnTypeDescr, "### expected return type description!" );

    void * pCppReturn = 0; // if != 0 && != pUnoReturn, needs reconversion

    if (pReturnTypeDescr)
    {
        if (cppu_isSimpleType( pReturnTypeDescr ))
        {
            pCppReturn = pUnoReturn; // direct way for simple types
            *(void**)pCppStack = NULL;
        }
        else
        {
            // complex return via ptr
            pCppReturn = *(void **)pCppStack = (cppu_relatesToInterface( pReturnTypeDescr )
                                                ? alloca( pReturnTypeDescr->nSize )
                                                : pUnoReturn); // direct way
        }
        pCppStack += sizeof(void*);
    }
    // push this
    *(void**)pCppStack = pThis->pCppI;
    pCppStack += sizeof( void* );

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

        if (!rParam.bOut && cppu_isSimpleType( pParamTypeDescr ))
        {
            pCppArgs[ nPos ] = adjustPointer( pCppStack, pParamTypeDescr );
            uno_copyAndConvertData( pCppArgs[nPos], pUnoArgs[nPos], pParamTypeDescr,
                                    &pThis->pBridge->aUno2Cpp );

            switch (pParamTypeDescr->eTypeClass)
            {
            case typelib_TypeClass_HYPER:
            case typelib_TypeClass_UNSIGNED_HYPER:
            case typelib_TypeClass_DOUBLE:
                pCppStack += sizeof(sal_Int32); // extra long
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
            else if (cppu_relatesToInterface( pParamTypeDescr ))
            {
                uno_copyAndConvertData(
                    *(void **)pCppStack = pCppArgs[nPos] = alloca( pParamTypeDescr->nSize ),
                    pUnoArgs[nPos], pParamTypeDescr, &pThis->pBridge->aUno2Cpp );

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
        }
        pCppStack += sizeof(sal_Int32); // standard parameter length
    }

    try
    {
        OSL_ENSURE( !( (pCppStack - pCppStackStart ) & 3), "UNALIGNED STACK !!! (Please DO panic" );
        callVirtualMethod(
            pThis->pCppI, nVtableCall,
            pCppReturn, pReturnTypeDescr->eTypeClass,
            (sal_Int32 *)pCppStackStart, (pCppStack - pCppStackStart) / sizeof(sal_Int32) );
        // NO exception occured...
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
                                            &pThis->pBridge->aCpp2Uno );
                }
            }
            else // pure out
            {
                uno_copyAndConvertData( pUnoArgs[nIndex], pCppArgs[nIndex], pParamTypeDescr,
                                        &pThis->pBridge->aCpp2Uno );
            }
            // destroy temp cpp param => cpp: every param was constructed
            uno_destructData( pCppArgs[nIndex], pParamTypeDescr, cpp_release );

            TYPELIB_DANGER_RELEASE( pParamTypeDescr );
        }
        // return value
        if (pCppReturn && pUnoReturn != pCppReturn)
        {
            uno_copyAndConvertData( pUnoReturn, pCppReturn, pReturnTypeDescr,
                                    &pThis->pBridge->aCpp2Uno );
            uno_destructData( pCppReturn, pReturnTypeDescr, cpp_release );
        }
    }
     catch( ... )
     {
          // get exception
        fillUnoException(
            __cxa_get_globals()->caughtExceptions, *ppUnoExc, &pThis->pBridge->aCpp2Uno );

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


//==================================================================================================
void SAL_CALL cppu_unoInterfaceProxy_dispatch(
    uno_Interface * pUnoI, const typelib_TypeDescription * pMemberDescr,
    void * pReturn, void * pArgs[], uno_Any ** ppException ) throw ()
{
#if defined BRIDGES_DEBUG
    OString cstr( OUStringToOString( pMemberDescr->pTypeName, RTL_TEXTENCODING_ASCII_US ) );
    fprintf( stderr, "received dispatch( %s )\n", cstr.getStr() );
#endif

    // is my surrogate
    cppu_unoInterfaceProxy * pThis = (cppu_unoInterfaceProxy *)pUnoI;
    typelib_InterfaceTypeDescription * pTypeDescr = pThis->pTypeDescr;

    switch (pMemberDescr->eTypeClass)
    {
    case typelib_TypeClass_INTERFACE_ATTRIBUTE:
    {
        // determine vtable call index
        sal_Int32 nMemberPos = ((typelib_InterfaceMemberTypeDescription *)pMemberDescr)->nPosition;
        OSL_ENSURE( nMemberPos < pTypeDescr->nAllMembers, "### member pos out of range!" );

        sal_Int32 nVtableCall = pTypeDescr->pMapMemberIndexToFunctionIndex[nMemberPos];
        OSL_ENSURE( nVtableCall < pTypeDescr->nMapFunctionIndexToMemberIndex, "### illegal vtable index!" );

        if (pReturn)
        {
            // dependent dispatch
            cpp_call(
                pThis, nVtableCall,
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
            OUString aVoidName( RTL_CONSTASCII_USTRINGPARAM("void") );
            typelib_typedescriptionreference_new(
                &pReturnTypeRef, typelib_TypeClass_VOID, aVoidName.pData );

            // dependent dispatch
            cpp_call(
                pThis, nVtableCall +1, // get, then set method
                pReturnTypeRef,
                1, &aParam,
                pReturn, pArgs, ppException );

            typelib_typedescriptionreference_release( pReturnTypeRef );
        }

        break;
    }
    case typelib_TypeClass_INTERFACE_METHOD:
    {
        // determine vtable call index
        sal_Int32 nMemberPos = ((typelib_InterfaceMemberTypeDescription *)pMemberDescr)->nPosition;
        OSL_ENSURE( nMemberPos < pTypeDescr->nAllMembers, "### member pos out of range!" );

        sal_Int32 nVtableCall = pTypeDescr->pMapMemberIndexToFunctionIndex[nMemberPos];
        OSL_ENSURE( nVtableCall < pTypeDescr->nMapFunctionIndexToMemberIndex, "### illegal vtable index!" );

        switch (nVtableCall)
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
                (*pThis->pBridge->pUnoEnv->getRegisteredInterface)(
                    pThis->pBridge->pUnoEnv,
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
                pThis, nVtableCall,
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
            OUString( RTL_CONSTASCII_USTRINGPARAM("illegal member type description!") ),
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >() );

        Type const & rExcType = ::getCppuType( &aExc );
        // binary identical null reference
        ::uno_type_any_construct( *ppException, &aExc, rExcType.getTypeLibType(), 0 );
    }
    }
}

}

