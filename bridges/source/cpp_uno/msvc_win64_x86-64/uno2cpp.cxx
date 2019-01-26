/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_bridges.hxx"

#include <malloc.h>

#include <com/sun/star/uno/genfunc.hxx>
#include <uno/data.h>

#include "bridges/cpp_uno/shared/bridge.hxx"
#include "bridges/cpp_uno/shared/types.hxx"
#include "bridges/cpp_uno/shared/unointerfaceproxy.hxx"
#include "bridges/cpp_uno/shared/vtables.hxx"

#include "abi.hxx"
#include "mscx.hxx"

using namespace ::rtl;
using namespace ::com::sun::star::uno;

namespace
{

//==================================================================================================
extern "C" void callVirtualMethod(
    void * pAdjustedThisPtr, sal_Int32 nVtableIndex,
    void * pRegisterReturn, typelib_TypeClass eReturnTypeClass,
    sal_uInt64 * pStackLongs, sal_uInt32 nStackLongs );

//==================================================================================================
static void cpp_call(
    bridges::cpp_uno::shared::UnoInterfaceProxy * pThis,
    bridges::cpp_uno::shared::VtableSlot aVtableSlot,
    typelib_TypeDescriptionReference * pReturnTypeRef,
    sal_Int32 nParams, typelib_MethodParameter * pParams,
    void * pUnoReturn, void * pUnoArgs[], uno_Any ** ppUnoExc ) throw ()
{
    // max space for: [complex ret ptr], values|ptr ...
    sal_uInt64 *pStack = (sal_uInt64 *)alloca( (nParams + 3) * sizeof(sal_uInt64) );
    sal_uInt64 *pStackStart = pStack;

    // Push "this" pointer
    void * pAdjustedThisPtr = reinterpret_cast< void ** >( pThis->getCppI() ) + aVtableSlot.offset;
    *pStack++ = *(sal_uInt64*)&pAdjustedThisPtr;

    // return
    typelib_TypeDescription * pReturnTypeDescr = 0;
    TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );
    OSL_ENSURE( pReturnTypeDescr, "### expected return type description!" );

    void * pCppReturn = 0; // if != 0 && != pUnoReturn, needs reconversion

    if ( pReturnTypeDescr )
    {
        if ( x86_64::return_in_hidden_param( pReturnTypeRef ) )
        {
            // complex return via ptr
            pCppReturn = bridges::cpp_uno::shared::relatesToInterfaceType( pReturnTypeDescr )?
                         alloca( pReturnTypeDescr->nSize ) : pUnoReturn; // direct way
            *pStack++ = *(sal_uInt64*)&pCppReturn;
        }
        else
            pCppReturn = pUnoReturn; // direct way for simple types
    }

    // Args
    void ** pCppArgs = (void **)alloca( 3 * sizeof(void *) * nParams );
    // Indices of values this have to be converted (interface conversion cpp<=>uno)
    sal_Int32 * pTempIndizes = (sal_Int32 *)(pCppArgs + nParams);
    // Type descriptions for reconversions
    typelib_TypeDescription ** ppTempParamTypeDescr = (typelib_TypeDescription **)(pCppArgs + (2 * nParams));

    sal_Int32 nTempIndizes = 0;

    for ( sal_Int32 nPos = 0; nPos < nParams; ++nPos )
    {
        const typelib_MethodParameter & rParam = pParams[nPos];
        typelib_TypeDescription * pParamTypeDescr = 0;
        TYPELIB_DANGER_GET( &pParamTypeDescr, rParam.pTypeRef );

        if (!rParam.bOut && bridges::cpp_uno::shared::isSimpleType( pParamTypeDescr ))
        {
            uno_copyAndConvertData( pCppArgs[nPos] = alloca( 8 ), pUnoArgs[nPos], pParamTypeDescr,
                                    pThis->getBridge()->getUno2Cpp() );

            switch (pParamTypeDescr->eTypeClass)
            {
            case typelib_TypeClass_HYPER:
            case typelib_TypeClass_UNSIGNED_HYPER:
                *pStack++ = *(sal_uInt64*)&pCppArgs[nPos];
                break;
            case typelib_TypeClass_LONG:
            case typelib_TypeClass_UNSIGNED_LONG:
            case typelib_TypeClass_ENUM:
                *pStack++ = *(sal_uInt32*)&pCppArgs[nPos];
                break;
            case typelib_TypeClass_SHORT:
            case typelib_TypeClass_UNSIGNED_SHORT:
            case typelib_TypeClass_CHAR:
                *pStack++ = *(sal_uInt16*)&pCppArgs[nPos];
                break;
            case typelib_TypeClass_BOOLEAN:
            case typelib_TypeClass_BYTE:
                *pStack++ = *(sal_uInt8*)&pCppArgs[nPos];
                break;
            case typelib_TypeClass_FLOAT:
            case typelib_TypeClass_DOUBLE:
                *pStack++ = *(sal_uInt64*)&pCppArgs[nPos]; // verbatim!
                break;
            default:
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
                    pCppArgs[nPos] = alloca( pParamTypeDescr->nSize ),
                    pParamTypeDescr );
                pTempIndizes[nTempIndizes] = nPos; // default constructed for cpp call
                // will be released at reconversion
                ppTempParamTypeDescr[nTempIndizes++] = pParamTypeDescr;
            }
            // is in/inout
            else if (bridges::cpp_uno::shared::relatesToInterfaceType( pParamTypeDescr ))
            {
                uno_copyAndConvertData(
                    pCppArgs[nPos] = alloca( pParamTypeDescr->nSize ),
                    pUnoArgs[nPos], pParamTypeDescr, pThis->getBridge()->getUno2Cpp() );

                pTempIndizes[nTempIndizes] = nPos; // has to be reconverted
                // will be released at reconversion
                ppTempParamTypeDescr[nTempIndizes++] = pParamTypeDescr;
            }
            else // direct way
            {
                pCppArgs[nPos] = pUnoArgs[nPos];
                // no longer needed
                TYPELIB_DANGER_RELEASE( pParamTypeDescr );
            }
            *pStack++ = *(sal_uInt64*)&pCppArgs[nPos];
        }
    }

    __try
    {
        // pCppI is mscx this pointer
        callVirtualMethod(
            pAdjustedThisPtr,
            aVtableSlot.index,
            pCppReturn, pReturnTypeDescr->eTypeClass,
            pStackStart, (pStack - pStackStart) );
    }
    __except (CPPU_CURRENT_NAMESPACE::mscx_filterCppException(
                  GetExceptionInformation(),
                  *ppUnoExc, pThis->getBridge()->getCpp2Uno() ))
   {
        // *ppUnoExc was constructed by filter function
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
        // end here
        return;
    }

    // NO exception occurred
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
    // return type
    if (pReturnTypeDescr)
        TYPELIB_DANGER_RELEASE( pReturnTypeDescr );
}

}

namespace bridges { namespace cpp_uno { namespace shared {

void unoInterfaceProxyDispatch(
    uno_Interface * pUnoI, const typelib_TypeDescription * pMemberDescr,
    void * pReturn, void * pArgs[], uno_Any ** ppException )
{
    // is my surrogate
    bridges::cpp_uno::shared::UnoInterfaceProxy * pThis
        = static_cast< bridges::cpp_uno::shared::UnoInterfaceProxy * >(pUnoI);

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
