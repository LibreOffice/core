/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: uno2cpp.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-06 14:34:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <rtl/alloc.h>

#include <com/sun/star/uno/genfunc.hxx>
#include "com/sun/star/uno/RuntimeException.hpp"
#include <uno/data.h>

#include <bridges/cpp_uno/shared/bridge.hxx>
#include <bridges/cpp_uno/shared/types.hxx>
#include "bridges/cpp_uno/shared/unointerfaceproxy.hxx"
#include "bridges/cpp_uno/shared/vtables.hxx"

#include "share.hxx"

using namespace ::rtl;
using namespace ::com::sun::star::uno;

void dummy_can_throw_anything( char const * );

// 6 integral parameters are passed in registers
const sal_uInt32 GPR_COUNT = 6;

// 8 floating point parameters are passed in SSE registers
const sal_uInt32 FPR_COUNT = 8;

static inline void
invoke_count_words(char *       pPT,      // Parameter Types
                   sal_uInt32 & nr_gpr,   // Number of arguments in GPRs
                   sal_uInt32 & nr_fpr,   // Number of arguments in FPRs
                   sal_uInt32 & nr_stack) // Number of arguments in stack
{
    nr_gpr = 0;
    nr_fpr = 0;
    nr_stack = 0;
    char c;

    while ((c = *pPT++) != 'X')
    {
        if (c == 'F' || c == 'D')
        {
            if (nr_fpr < FPR_COUNT)
                nr_fpr++;
            else
                nr_stack++;
        }
        else
        {
            if (nr_gpr < GPR_COUNT)
                nr_gpr++;
            else
                nr_stack++;
        }
    }
}

static void
invoke_copy_to_stack(sal_uInt64 * pDS,  // Stack Storage
                     char *       pPT,  // Parameter Types
                     sal_uInt64 * pSV,  // Source Values
                     sal_uInt64 * pGPR, // General Purpose Registers
                     double *     pFPR) // Floating-Point Registers
{
    sal_uInt32 nr_gpr = 0;
    sal_uInt32 nr_fpr = 0;
    sal_uInt64 value;
    char c;

    while ((c = *pPT++) != 'X')
    {
        switch (c)
        {
        case 'D': // Double
            if (nr_fpr < FPR_COUNT)
                pFPR[nr_fpr++] = *reinterpret_cast<double *>( pSV++ );
            else
                *pDS++ = *pSV++;
            break;

        case 'F': // Float
            if (nr_fpr < FPR_COUNT)
                // The value in %xmm register is already prepared to
                // be retrieved as a float. Therefore, we pass the
                // value verbatim, as a double without conversion.
                pFPR[nr_fpr++] = *reinterpret_cast<double *>( pSV++ );
            else
                *pDS++ = *reinterpret_cast<double *>( pSV++ );
            break;

        case 'H': // 64-bit Word
            if (nr_gpr < GPR_COUNT)
                pGPR[nr_gpr++] = *pSV++;
            else
                *pDS++ = *pSV++;
            break;

        case 'I': // 32-bit Word
            if (nr_gpr < GPR_COUNT)
                pGPR[nr_gpr++] = *reinterpret_cast<sal_uInt32 *>( pSV++ );
            else
                *pDS++ = *reinterpret_cast<sal_uInt32 *>( pSV++ );
            break;

        case 'S': // 16-bit Word
            if (nr_gpr < GPR_COUNT)
                pGPR[nr_gpr++] = *reinterpret_cast<sal_uInt16 *>( pSV++ );
            else
                *pDS++ = *reinterpret_cast<sal_uInt16 *>( pSV++ );
            break;

        case 'B': // Byte
            if (nr_gpr < GPR_COUNT)
                pGPR[nr_gpr++] = *reinterpret_cast<sal_uInt8 *>( pSV++ );
            else
                *pDS++ = *reinterpret_cast<sal_uInt8 *>( pSV++ );
            break;

        default: // Default, assume 64-bit values
            if (nr_gpr < GPR_COUNT)
                pGPR[nr_gpr++] = *pSV++;
            else
                *pDS++ = *pSV++;
            break;
        }
    }
}

//==================================================================================================
static void callVirtualMethod(void * pThis, sal_uInt32 nVtableIndex,
                              void * pRegisterReturn, typelib_TypeClass eReturnType,
                              char * pPT, sal_uInt64 * pStackLongs, sal_uInt32 nStackLongs)
{
    sal_uInt32 nr_gpr, nr_fpr, nr_stack;
    invoke_count_words(pPT, nr_gpr, nr_fpr, nr_stack);

    // Stack, if used, must be 16-bytes aligned
    if (nr_stack)
        nr_stack = (nr_stack + 1) & ~1;

    bool bReturnsSimpleType = bridges::cpp_uno::shared::isSimpleType( eReturnType );

#if OSL_DEBUG_LEVEL > 1
    // Let's figure out what is really going on here
    fprintf(stderr,"callVirtualMethod() parameters string is %s\n", pPT);
    {
        sal_uInt32  k = nStackLongs;
        sal_uInt64 *q = pStackLongs;
        while (k > 0)
        {
            fprintf(stderr, "uno stack is: %lx\n", *q);
            k--;
            q++;
        }
    }
#endif

    // Load parameters to stack, if necessary
    sal_uInt64 *stack = (sal_uInt64 *) __builtin_alloca(nr_stack * 8);
    sal_uInt64 gpregs[GPR_COUNT];
    double fpregs[FPR_COUNT];
    invoke_copy_to_stack(stack, pPT, pStackLongs, gpregs, fpregs);

    // Load FPR registers from fpregs[]
    register double d0 asm("xmm0");
    register double d1 asm("xmm1");
    register double d2 asm("xmm2");
    register double d3 asm("xmm3");
    register double d4 asm("xmm4");
    register double d5 asm("xmm5");
    register double d6 asm("xmm6");
    register double d7 asm("xmm7");

    switch (nr_fpr) {
#define ARG_FPR(N) \
    case N+1: d##N = fpregs[N];
        ARG_FPR(7);
        ARG_FPR(6);
        ARG_FPR(5);
        ARG_FPR(4);
        ARG_FPR(3);
        ARG_FPR(2);
        ARG_FPR(1);
        ARG_FPR(0);
    case 0:;
#undef ARG_FPR
    }

    // Load GPR registers from gpregs[]
    register sal_uInt64 a0 asm("rdi");
    register sal_uInt64 a1 asm("rsi");
    register sal_uInt64 a2 asm("rdx");
    register sal_uInt64 a3 asm("rcx");
    register sal_uInt64 a4 asm("r8");
    register sal_uInt64 a5 asm("r9");

    switch (nr_gpr) {
#define ARG_GPR(N) \
    case N+1: a##N = gpregs[N];
        ARG_GPR(5);
        ARG_GPR(4);
        ARG_GPR(3);
        ARG_GPR(2);
        ARG_GPR(1);
        ARG_GPR(0);
    case 0:;
#undef ARG_GPR
    }

    if ( bReturnsSimpleType )
        a0 = (sal_uInt64) pThis;
    else
        a1 = (sal_uInt64) pThis;

    // Ensure that assignments to SSE registers won't be optimized away
    asm("" ::
        "x" (d0), "x" (d1), "x" (d2), "x" (d3),
        "x" (d4), "x" (d5), "x" (d6), "x" (d7));

    // Get pointer to method
    sal_uInt64 pMethod = *((sal_uInt64 *)pThis);
    pMethod += 8 * nVtableIndex;
    pMethod = *((sal_uInt64 *)pMethod);

    union ReturnValue {
        struct {
            sal_uInt64 rax;
            sal_uInt64 rdx;
        } i;
        struct {
            double xmm0;
            double xmm1;
        } f;
    };

    typedef ReturnValue (* FunctionCall )( sal_uInt64, sal_uInt64, sal_uInt64, sal_uInt64, sal_uInt64, sal_uInt64 );

    // Perform the call
    ReturnValue aRet = ( ( FunctionCall ) pMethod )( a0, a1, a2, a3, a4, a5 );

    switch (eReturnType)
    {
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
        *reinterpret_cast<sal_uInt64 *>( pRegisterReturn ) = aRet.i.rax;
        break;
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
    case typelib_TypeClass_ENUM:
        *reinterpret_cast<sal_uInt32 *>( pRegisterReturn ) = *reinterpret_cast<sal_uInt32*>( &aRet.i.rax );
        break;
    case typelib_TypeClass_CHAR:
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
        *reinterpret_cast<sal_uInt16 *>( pRegisterReturn ) = *reinterpret_cast<sal_uInt16*>( &aRet.i.rax );
        break;
    case typelib_TypeClass_BOOLEAN:
    case typelib_TypeClass_BYTE:
        *reinterpret_cast<sal_uInt8 *>( pRegisterReturn ) = *reinterpret_cast<sal_uInt8*>( &aRet.i.rax );
        break;
    case typelib_TypeClass_FLOAT:
        *reinterpret_cast<float *>( pRegisterReturn ) = *reinterpret_cast<float*>( &aRet.f.xmm0 );
        break;
    case typelib_TypeClass_DOUBLE:
        *reinterpret_cast<double *>( pRegisterReturn ) = *reinterpret_cast<double*>( &aRet.f.xmm0 );
        break;
    }
}


//==================================================================================================
static void cpp_call(
    bridges::cpp_uno::shared::UnoInterfaceProxy * pThis,
    bridges::cpp_uno::shared::VtableSlot aVtableSlot,
    typelib_TypeDescriptionReference * pReturnTypeRef,
    sal_Int32 nParams, typelib_MethodParameter * pParams,
    void * pUnoReturn, void * pUnoArgs[], uno_Any ** ppUnoExc )
{
    // Maxium space for [complex ret ptr], values | ptr ...
      char * pCppStack      = (char *)__builtin_alloca( (nParams + 3) * sizeof(sal_uInt64) );
      char * pCppStackStart = pCppStack;

    // We need to know parameter types for callVirtualMethod() so generate a signature string
    char * pParamType       = (char *)__builtin_alloca( nParams + 3 );
    char * pPT              = pParamType;

    // Return
    typelib_TypeDescription * pReturnTypeDescr = 0;
    TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );
    OSL_ENSURE( pReturnTypeDescr, "### expected return type description!" );

    void * pCppReturn = 0; // if != 0 && != pUnoReturn, needs reconversion

    if (pReturnTypeDescr)
    {
        if (bridges::cpp_uno::shared::isSimpleType( pReturnTypeDescr ))
        {
            pCppReturn = pUnoReturn; // direct way for simple types
        }
        else
        {
            // complex return via ptr
            pCppReturn = *(void **)pCppStack = (bridges::cpp_uno::shared::relatesToInterfaceType( pReturnTypeDescr )
                                                ? __builtin_alloca( pReturnTypeDescr->nSize )
                                                : pUnoReturn); // direct way
            *pPT++ = 'H';
            pCppStack += sizeof(void *);
        }
    }

    // Push "this" pointer
    void * pAdjustedThisPtr = reinterpret_cast< void ** >( pThis->getCppI() ) + aVtableSlot.offset;
    *(void **)pCppStack = pAdjustedThisPtr;
    *pPT++ = 'H';
    pCppStack += sizeof(void *);

    // stack space
    // Args
    void ** pCppArgs  = (void **)alloca( 3 * sizeof(void *) * nParams );
    // Indizes of values this have to be converted (interface conversion cpp<=>uno)
    sal_Int32 * pTempIndizes = (sal_Int32 *)(pCppArgs + nParams);
    // Type descriptions for reconversions
    typelib_TypeDescription ** ppTempParamTypeDescr = (typelib_TypeDescription **)(pCppArgs + (2 * nParams));

    sal_Int32 nTempIndizes   = 0;

    for ( sal_Int32 nPos = 0; nPos < nParams; ++nPos )
    {
        const typelib_MethodParameter & rParam = pParams[nPos];
        typelib_TypeDescription * pParamTypeDescr = 0;
        TYPELIB_DANGER_GET( &pParamTypeDescr, rParam.pTypeRef );

        if (!rParam.bOut && bridges::cpp_uno::shared::isSimpleType( pParamTypeDescr ))
        {
            uno_copyAndConvertData( pCppArgs[nPos] = pCppStack, pUnoArgs[nPos], pParamTypeDescr,
                                    pThis->getBridge()->getUno2Cpp() );

            switch (pParamTypeDescr->eTypeClass)
            {

                // we need to know type of each param so that we know whether to use
                // gpr or fpr to pass in parameters:
                // Key: I - 32-bit value passed in gpr
                //      B - byte value passed in gpr
                //      S - short value passed in gpr
                //      F - float value pass in fpr
                //      D - double value pass in fpr
                //      H - long value passed in gpr
                //      X - indicates end of parameter description string

            case typelib_TypeClass_LONG:
            case typelib_TypeClass_UNSIGNED_LONG:
            case typelib_TypeClass_ENUM:
                *pPT++ = 'I';
                break;
            case typelib_TypeClass_SHORT:
            case typelib_TypeClass_CHAR:
            case typelib_TypeClass_UNSIGNED_SHORT:
                *pPT++ = 'S';
                break;
            case typelib_TypeClass_BOOLEAN:
            case typelib_TypeClass_BYTE:
                *pPT++ = 'B';
                break;
            case typelib_TypeClass_FLOAT:
                *pPT++ = 'F';
                break;
            case typelib_TypeClass_DOUBLE:
                *pPT++ = 'D';
                break;
            case typelib_TypeClass_HYPER:
            case typelib_TypeClass_UNSIGNED_HYPER:
                *pPT++ = 'H';
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
                    *(void **)pCppStack = pCppArgs[nPos] = alloca( pParamTypeDescr->nSize ),
                    pParamTypeDescr );
                pTempIndizes[nTempIndizes] = nPos; // default constructed for cpp call
                // will be released at reconversion
                ppTempParamTypeDescr[nTempIndizes++] = pParamTypeDescr;
            }
            // is in/inout
            else if (bridges::cpp_uno::shared::relatesToInterfaceType( pParamTypeDescr ))
            {
                uno_copyAndConvertData(
                    *(void **)pCppStack = pCppArgs[nPos] = alloca( pParamTypeDescr->nSize ),
                    pUnoArgs[nPos], pParamTypeDescr, pThis->getBridge()->getUno2Cpp() );

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
            // FIXME: is this the right way to pass these?
            *pPT++='H';
        }
        pCppStack += sizeof(sal_uInt64); // standard parameter length
    }

    // terminate the signature string
    *pPT++ = 'X';
    *pPT = 0;

    try
    {
        OSL_ENSURE( !( (pCppStack - pCppStackStart ) & 7), "UNALIGNED STACK !!! (Please DO panic)" );
        callVirtualMethod(
            pAdjustedThisPtr, aVtableSlot.index,
            pCppReturn, pReturnTypeDescr->eTypeClass, pParamType,
            (sal_uInt64 *)pCppStackStart, (pCppStack - pCppStackStart) / sizeof(sal_uInt64) );
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
        fillUnoException( CPPU_CURRENT_NAMESPACE::__cxa_get_globals()->caughtExceptions, *ppUnoExc, pThis->getBridge()->getCpp2Uno() );

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
namespace bridges { namespace cpp_uno { namespace shared {

void unoInterfaceProxyDispatch(
    uno_Interface * pUnoI, const typelib_TypeDescription * pMemberDescr,
    void * pReturn, void * pArgs[], uno_Any ** ppException )
{
    // is my surrogate
    bridges::cpp_uno::shared::UnoInterfaceProxy * pThis
        = static_cast< bridges::cpp_uno::shared::UnoInterfaceProxy * >(pUnoI);
    typelib_InterfaceTypeDescription * pTypeDescr = pThis->pTypeDescr;

    switch (pMemberDescr->eTypeClass)
    {
    case typelib_TypeClass_INTERFACE_ATTRIBUTE:
    {
        // determine vtable call index
        sal_Int32 nMemberPos = ((typelib_InterfaceMemberTypeDescription *)pMemberDescr)->nPosition;
        OSL_ENSURE( nMemberPos < pTypeDescr->nAllMembers, "### member pos out of range!" );

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
            OUString aVoidName( RTL_CONSTASCII_USTRINGPARAM("void") );
            typelib_typedescriptionreference_new(
                &pReturnTypeRef, typelib_TypeClass_VOID, aVoidName.pData );

            // dependent dispatch
            aVtableSlot.index += 1; // get, then set method
            cpp_call(
                pThis, aVtableSlot, // get, then set method
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
                (*pThis->getBridge()->getUnoEnv()->getRegisteredInterface)(
                    pThis->getBridge()->getUnoEnv(),
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
            OUString( RTL_CONSTASCII_USTRINGPARAM("illegal member type description!") ),
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >() );

        Type const & rExcType = ::getCppuType( &aExc );
        // binary identical null reference
        ::uno_type_any_construct( *ppException, &aExc, rExcType.getTypeLibType(), 0 );
    }
    }
}

} } }
