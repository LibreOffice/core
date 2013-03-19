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

#include <stdio.h>
#include <string.h>


using namespace ::rtl;
using namespace ::com::sun::star::uno;

void MapReturn(const ia64::RegReturn &rRet, double dret, typelib_TypeDescription * pReturnTypeDescr, bool bSimpleReturn, sal_uInt64 *pRegisterReturn)
{
    switch (pReturnTypeDescr->eTypeClass)
    {
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
    case typelib_TypeClass_ENUM:
            *pRegisterReturn = rRet.r8;
            break;
    case typelib_TypeClass_CHAR:
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
            *pRegisterReturn = (unsigned short)rRet.r8;
            break;
    case typelib_TypeClass_BOOLEAN:
    case typelib_TypeClass_BYTE:
            *pRegisterReturn = (unsigned char)rRet.r8;
            break;
    case typelib_TypeClass_FLOAT:
            *reinterpret_cast<float *>( pRegisterReturn ) = dret;
        break;
    case typelib_TypeClass_DOUBLE:
            *reinterpret_cast<double *>( pRegisterReturn ) = dret;
            break;
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
        {
            sal_uInt32 nRetSize = pReturnTypeDescr->nSize;
            if (bSimpleReturn && nRetSize <= 32 && nRetSize > 0)
                memcpy(pRegisterReturn, (void*)&rRet, nRetSize);
            break;
        }
    default:
    break;
    }
}

namespace ia64
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

    bool is_complex_struct( typelib_TypeDescriptionReference *pTypeRef )
    {
        if (pTypeRef->eTypeClass == typelib_TypeClass_STRUCT || pTypeRef->eTypeClass == typelib_TypeClass_EXCEPTION)
    {
            typelib_TypeDescription * pTypeDescr = 0;
            TYPELIB_DANGER_GET( &pTypeDescr, pTypeRef );

            bool bRet = is_complex_struct( pTypeDescr );
            TYPELIB_DANGER_RELEASE( pTypeDescr );

            return bRet;
       }
       return false;
    }

    bool return_via_r8_buffer( typelib_TypeDescriptionReference *pTypeRef )
    {
        if (pTypeRef->eTypeClass == typelib_TypeClass_STRUCT || pTypeRef->eTypeClass == typelib_TypeClass_EXCEPTION)
    {
            if (is_complex_struct( pTypeRef )) return false;

            typelib_TypeDescription * pTypeDescr = 0;
            TYPELIB_DANGER_GET( &pTypeDescr, pTypeRef );

            /* If the struct is larger than 32 bytes, then there is a buffer at r8 to stick the return value into  */
            bool bRet = pTypeDescr->nSize > 32;
            TYPELIB_DANGER_RELEASE( pTypeDescr );
            return bRet;
       }
       return false;
    }

    bool return_in_hidden_param( typelib_TypeDescriptionReference *pTypeRef )
    {
        if (bridges::cpp_uno::shared::isSimpleType(pTypeRef))
            return false;
        else if (pTypeRef->eTypeClass == typelib_TypeClass_STRUCT || pTypeRef->eTypeClass == typelib_TypeClass_EXCEPTION)
            return is_complex_struct( pTypeRef );
        return true;
    }


}

namespace
{
//==================================================================================================
static void callVirtualMethod(void * pThis, sal_uInt32 nVtableIndex,
    void * pRegisterReturn, typelib_TypeDescription * pReturnTypeDescr, bool bSimpleReturn,
        sal_uInt64 *pStack, sal_uInt32 nStack,
        sal_uInt64 *pGPR, sal_uInt32 nGPR,
        double *pFPR, sal_uInt32 nFPR)
{
    // Stack, if used, must be 16-bytes aligned
    if ( nStack )
        nStack = ( nStack + 1 ) & ~1;

    // Should not happen, but...
    if ( nFPR > ia64::MAX_SSE_REGS )
        nFPR = ia64::MAX_SSE_REGS;
    if ( nGPR > ia64::MAX_GPR_REGS )
        nGPR = ia64::MAX_GPR_REGS;

#if OSL_DEBUG_LEVEL > 2
        // Let's figure out what is really going on here
        {
                fprintf( stderr, "= callVirtualMethod() =\nGPR's (%d): ", nGPR );
                for ( unsigned int i = 0; i < nGPR; ++i )
                        fprintf( stderr, "0x%lx, ", pGPR[i] );
                fprintf( stderr, "\nFPR's (%d): ", nFPR );
                for ( unsigned int i = 0; i < nFPR; ++i )
                        fprintf( stderr, "0x%lx (%f), ", pFPR[i], pFPR[i] );
                fprintf( stderr, "\nStack (%d): ", nStack );
                for ( unsigned int i = 0; i < nStack; ++i )
                        fprintf( stderr, "0x%lx, ", pStack[i] );
                fprintf( stderr, "\n" );
        fprintf( stderr, "pRegisterReturn is %p\n", pRegisterReturn);
        }
#endif

    // Load parameters to stack, if necessary
    sal_uInt64 *stack = (sal_uInt64 *) __builtin_alloca( nStack * 8 );
    memcpy( stack, pStack, nStack * 8 );

    // To get pointer to method
    // a) get the address of the vtable
    sal_uInt64 pMethod = *((sal_uInt64 *)pThis);
    // b) get the address from the vtable entry at offset, each entry is 16bytes,
    // 8 for function pointer, and 8 for global pointer
    pMethod += 16 * nVtableIndex;

    typedef void (* FunctionCall )( sal_uInt64, sal_uInt64, sal_uInt64, sal_uInt64, sal_uInt64, sal_uInt64, sal_uInt64, sal_uInt64 );
    FunctionCall pFunc = (FunctionCall)pMethod;

    switch (nFPR) //deliberate fall through
    {
        case 8:
            asm volatile("ldfd f15=%0" : : "m"(pFPR[7]) : "f15");
        case 7:
            asm volatile("ldfd f14=%0" : : "m"(pFPR[6]) : "f14");
        case 6:
            asm volatile("ldfd f13=%0" : : "m"(pFPR[5]) : "f13");
        case 5:
            asm volatile("ldfd f12=%0" : : "m"(pFPR[4]) : "f12");
        case 4:
            asm volatile("ldfd f11=%0" : : "m"(pFPR[3]) : "f11");
        case 3:
            asm volatile("ldfd f10=%0" : : "m"(pFPR[2]) : "f10");
        case 2:
            asm volatile("ldfd f9=%0" : : "m"(pFPR[1]) : "f9");
        case 1:
            asm volatile("ldfd f8=%0" : : "m"(pFPR[0]) : "f8");
        default:
            break;
    }

    //stick the return area into r8 for big struct returning
    asm volatile("ld8 r8=%0" : : "m"(pRegisterReturn) : "r8");

    (*pFunc)(pGPR[0], pGPR[1], pGPR[2], pGPR[3], pGPR[4], pGPR[5], pGPR[6], pGPR[7]);

    register double f8 asm("f8");
    ia64::RegReturn ret;
    {
        register long r8 asm("r8"); ret.r8 = r8;
        register long r9 asm("r9"); ret.r9 = r9;
        register long r10 asm("r10"); ret.r10 = r10;
        register long r11 asm("r11"); ret.r11 = r11;
    }

    MapReturn(ret, f8, pReturnTypeDescr, bSimpleReturn, (sal_uInt64*)pRegisterReturn);
}

// Macros for easier insertion of values to registers or stack
// pSV - pointer to the source
// nr - order of the value [will be increased if stored to register]
// pFPR, pGPR - pointer to the registers
// pDS - pointer to the stack [will be increased if stored here]

// The value in %xmm register is already prepared to be retrieved as a float,
// thus we treat float and double the same
#define INSERT_FLOAT( pSV, nfr, pFPR, ngr, pGPR, pDS, bOverflow ) \
        if ( nfr < ia64::MAX_SSE_REGS && ngr < ia64::MAX_GPR_REGS ) \
                pFPR[nfr++] = *reinterpret_cast<float *>( pSV ); \
        if ( ngr < ia64::MAX_GPR_REGS ) \
                pGPR[ngr++] = *reinterpret_cast<sal_uInt64 *>( pSV ); \
        else \
            bOverFlow = true; \
        if (bOverFlow) \
                *pDS++ = *reinterpret_cast<sal_uInt64 *>( pSV ); // verbatim!

#define INSERT_DOUBLE( pSV, nfr, pFPR, ngr, pGPR, pDS, bOverflow ) \
        if ( nfr < ia64::MAX_SSE_REGS && ngr < ia64::MAX_GPR_REGS ) \
                pFPR[nfr++] = *reinterpret_cast<double *>( pSV ); \
        if ( ngr < ia64::MAX_GPR_REGS ) \
                pGPR[ngr++] = *reinterpret_cast<sal_uInt64 *>( pSV ); \
        else \
            bOverFlow = true; \
        if (bOverFlow) \
                *pDS++ = *reinterpret_cast<sal_uInt64 *>( pSV ); // verbatim!

#define INSERT_INT64( pSV, nr, pGPR, pDS, bOverflow ) \
        if ( nr < ia64::MAX_GPR_REGS ) \
                pGPR[nr++] = *reinterpret_cast<sal_uInt64 *>( pSV ); \
        else \
        bOverFlow = true; \
    if (bOverFlow) \
                *pDS++ = *reinterpret_cast<sal_uInt64 *>( pSV );

#define INSERT_INT32( pSV, nr, pGPR, pDS, bOverflow ) \
        if ( nr < ia64::MAX_GPR_REGS ) \
                pGPR[nr++] = *reinterpret_cast<sal_uInt32 *>( pSV ); \
        else \
                bOverFlow = true; \
        if (bOverFlow) \
                *pDS++ = *reinterpret_cast<sal_uInt32 *>( pSV );

#define INSERT_INT16( pSV, nr, pGPR, pDS, bOverflow ) \
        if ( nr < ia64::MAX_GPR_REGS ) \
                pGPR[nr++] = *reinterpret_cast<sal_uInt16 *>( pSV ); \
        else \
                bOverFlow = true; \
        if (bOverFlow) \
                *pDS++ = *reinterpret_cast<sal_uInt16 *>( pSV );

#define INSERT_INT8( pSV, nr, pGPR, pDS, bOverflow ) \
        if ( nr < ia64::MAX_GPR_REGS ) \
                pGPR[nr++] = *reinterpret_cast<sal_uInt8 *>( pSV ); \
        else \
                bOverFlow = true; \
        if (bOverFlow) \
                *pDS++ = *reinterpret_cast<sal_uInt8 *>( pSV );

//==================================================================================================
static void cpp_call(
    bridges::cpp_uno::shared::UnoInterfaceProxy * pThis,
    bridges::cpp_uno::shared::VtableSlot  aVtableSlot,
    typelib_TypeDescriptionReference * pReturnTypeRef,
    sal_Int32 nParams, typelib_MethodParameter * pParams,
    void * pUnoReturn, void * pUnoArgs[], uno_Any ** ppUnoExc )
{
      // max space for: [complex ret ptr], values|ptr ...
      sal_uInt64 * pStack = (sal_uInt64 *)alloca( (nParams+3) * sizeof(sal_Int64) );
      sal_uInt64 * pStackStart = pStack;

    sal_uInt64 pGPR[ia64::MAX_GPR_REGS];
    sal_uInt32 nGPR = 0;

    double pFPR[ia64::MAX_SSE_REGS];
    sal_uInt32 nFPR = 0;

    // return
    typelib_TypeDescription * pReturnTypeDescr = 0;
    TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );
    OSL_ENSURE( pReturnTypeDescr, "### expected return type description!" );

    void * pCppReturn = 0; // if != 0 && != pUnoReturn, needs reconversion

        bool bOverFlow = false;

    bool bSimpleReturn = true;
    if (pReturnTypeDescr)
    {
#if OSL_DEBUG_LEVEL > 2
        fprintf(stderr, "return type is %d\n", pReturnTypeDescr->eTypeClass);
#endif
        if ( ia64::return_in_hidden_param(pReturnTypeRef) || ia64::return_via_r8_buffer(pReturnTypeRef) )
                        bSimpleReturn = false;

                if ( bSimpleReturn )
        {
            pCppReturn = pUnoReturn; // direct way for simple types
#if OSL_DEBUG_LEVEL > 2
            fprintf(stderr, "simple return\n");
#endif
        }
        else
        {
            // complex return via ptr
            pCppReturn = (bridges::cpp_uno::shared::relatesToInterfaceType( pReturnTypeDescr )
                   ? alloca( pReturnTypeDescr->nSize ) : pUnoReturn);
#if OSL_DEBUG_LEVEL > 2
            fprintf(stderr, "pCppReturn/pUnoReturn is %lx/%lx", pCppReturn, pUnoReturn);
#endif
                        if (!ia64::return_via_r8_buffer(pReturnTypeRef))
                INSERT_INT64( &pCppReturn, nGPR, pGPR, pStack, bOverFlow );
        }
    }
    // push "this" pointer
        void * pAdjustedThisPtr = reinterpret_cast< void ** >( pThis->getCppI() ) + aVtableSlot.offset;

#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr, "this pointer is %p\n", pAdjustedThisPtr);
#endif
    INSERT_INT64( &pAdjustedThisPtr, nGPR, pGPR, pStack, bOverFlow );

        // Args
        void ** pCppArgs = (void **)alloca( 3 * sizeof(void *) * nParams );
    // indizes of values this have to be converted (interface conversion cpp<=>uno)
    sal_Int32 * pTempIndizes = (sal_Int32 *)(pCppArgs + nParams);
    // type descriptions for reconversions
    typelib_TypeDescription ** ppTempParamTypeDescr = (typelib_TypeDescription **)(pCppArgs + (2 * nParams));

    sal_Int32 nTempIndizes   = 0;

#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr, "n params is %d\n", nParams);
#endif

    for ( sal_Int32 nPos = 0; nPos < nParams; ++nPos )
    {
        const typelib_MethodParameter & rParam = pParams[nPos];
        typelib_TypeDescription * pParamTypeDescr = 0;
        TYPELIB_DANGER_GET( &pParamTypeDescr, rParam.pTypeRef );

#if OSL_DEBUG_LEVEL > 2
        fprintf(stderr, "param %d is %d %d %d\n", nPos, rParam.bOut, bridges::cpp_uno::shared::isSimpleType( pParamTypeDescr ),
            pParamTypeDescr->eTypeClass);
#endif

        if (!rParam.bOut && bridges::cpp_uno::shared::isSimpleType( pParamTypeDescr ))
        {
//          uno_copyAndConvertData( pCppArgs[nPos] = alloca( 8 ), pUnoArgs[nPos], pParamTypeDescr,
            uno_copyAndConvertData( pCppArgs[nPos] = pStack, pUnoArgs[nPos], pParamTypeDescr,
                                    pThis->getBridge()->getUno2Cpp() );
                switch (pParamTypeDescr->eTypeClass)
                        {
                        case typelib_TypeClass_HYPER:
                        case typelib_TypeClass_UNSIGNED_HYPER:
#if OSL_DEBUG_LEVEL > 2
                fprintf(stderr, "hyper is %lx\n", *(unsigned long*)(pCppArgs[nPos]));
#endif
                                INSERT_INT64( pCppArgs[nPos], nGPR, pGPR, pStack, bOverFlow );
                                break;
                        case typelib_TypeClass_LONG:
                        case typelib_TypeClass_UNSIGNED_LONG:
                        case typelib_TypeClass_ENUM:
#if OSL_DEBUG_LEVEL > 2
                fprintf(stderr, "long is %lx\n", *(unsigned int*)(pCppArgs[nPos]));
#endif
                                INSERT_INT32( pCppArgs[nPos], nGPR, pGPR, pStack, bOverFlow );
                                break;
                        case typelib_TypeClass_SHORT:
                        case typelib_TypeClass_CHAR:
                        case typelib_TypeClass_UNSIGNED_SHORT:
#if OSL_DEBUG_LEVEL > 2
                fprintf(stderr, "short is %x\n", *(unsigned short*)(pCppArgs[nPos]));
#endif
                                INSERT_INT16( pCppArgs[nPos], nGPR, pGPR, pStack, bOverFlow );
                                break;
                        case typelib_TypeClass_BOOLEAN:
                        case typelib_TypeClass_BYTE:
#if OSL_DEBUG_LEVEL > 2
                fprintf(stderr, "byte is %x\n", *(unsigned char*)(pCppArgs[nPos]));
#endif
                                INSERT_INT8( pCppArgs[nPos], nGPR, pGPR, pStack, bOverFlow );
                                break;
                        case typelib_TypeClass_FLOAT:
#if OSL_DEBUG_LEVEL > 2
                fprintf(stderr, "a float is %f\n", *(float*)(pCppArgs[nPos]));
                fprintf(stderr, "b float is %f\n", *(double*)(pCppArgs[nPos]));
#endif
                                INSERT_FLOAT( pCppArgs[nPos], nFPR, pFPR, nGPR, pGPR, pStack, bOverFlow );
                break;
                        case typelib_TypeClass_DOUBLE:
#if OSL_DEBUG_LEVEL > 2
                fprintf(stderr, "double is %f\n", *(double*)(pCppArgs[nPos]));
#endif
                                INSERT_DOUBLE( pCppArgs[nPos], nFPR, pFPR, nGPR, pGPR, pStack, bOverFlow );
                                break;
            default:
                break;
                        }

                        // no longer needed
                        TYPELIB_DANGER_RELEASE( pParamTypeDescr );

        }
        else // ptr to complex value | ref
        {
#if OSL_DEBUG_LEVEL > 2
            fprintf(stderr, "complex type again %d\n", rParam.bIn);
#endif
                        if (! rParam.bIn) // is pure out
                        {
#if OSL_DEBUG_LEVEL > 2
                fprintf(stderr, "complex size is %d\n", pParamTypeDescr->nSize );
#endif
                                // cpp out is constructed mem, uno out is not!
                                uno_constructData(
                                        pCppArgs[nPos] = alloca( pParamTypeDescr->nSize ),
                                        pParamTypeDescr );
                                pTempIndizes[nTempIndizes] = nPos; // default constructed for cpp call
                                // will be released at reconversion
                                ppTempParamTypeDescr[nTempIndizes++] = pParamTypeDescr;
                        }
                        // is in/inout
                        else if (bridges::cpp_uno::shared::relatesToInterfaceType( pParamTypeDescr ))
                        {
#if OSL_DEBUG_LEVEL > 2
                fprintf(stderr, "this one\n");
#endif
                                uno_copyAndConvertData(
                                        pCppArgs[nPos] = alloca( pParamTypeDescr->nSize ),
                                        pUnoArgs[nPos], pParamTypeDescr, pThis->getBridge()->getUno2Cpp() );

                                pTempIndizes[nTempIndizes] = nPos; // has to be reconverted
                                // will be released at reconversion
                                ppTempParamTypeDescr[nTempIndizes++] = pParamTypeDescr;
                        }
                        else // direct way
                        {
#if OSL_DEBUG_LEVEL > 2
                fprintf(stderr, "that one, passing %lx through\n", pUnoArgs[nPos]);
#endif
                                pCppArgs[nPos] = pUnoArgs[nPos];
                                // no longer needed
                                TYPELIB_DANGER_RELEASE( pParamTypeDescr );
                        }
                        INSERT_INT64( &(pCppArgs[nPos]), nGPR, pGPR, pStack, bOverFlow );
        }
    }

    try
    {
               callVirtualMethod(
                        pAdjustedThisPtr, aVtableSlot.index,
                        pCppReturn, pReturnTypeDescr, bSimpleReturn,
                        pStackStart, ( pStack - pStackStart ),
                        pGPR, nGPR,
                        pFPR, nFPR );
        // NO exception occurred...
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

}

namespace bridges { namespace cpp_uno { namespace shared {

void unoInterfaceProxyDispatch(
    uno_Interface * pUnoI, const typelib_TypeDescription * pMemberDescr,
    void * pReturn, void * pArgs[], uno_Any ** ppException )
{
    // is my surrogate
        bridges::cpp_uno::shared::UnoInterfaceProxy * pThis
            = static_cast< bridges::cpp_uno::shared::UnoInterfaceProxy *> (pUnoI);

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
            aParam.bIn      = sal_True;
            aParam.bOut     = sal_False;

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

        Type const & rExcType = ::getCppuType( &aExc );
        // binary identical null reference
        ::uno_type_any_construct( *ppException, &aExc, rExcType.getTypeLibType(), 0 );
    }
    }
}

} } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
