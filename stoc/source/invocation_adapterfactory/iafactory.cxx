/*************************************************************************
 *
 *  $RCSfile: iafactory.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:29:34 $
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

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _OSL_INTERLOCK_H_
#include <osl/interlck.h>
#endif

#ifndef _UNO_DISPATCHER_H_
#include <uno/dispatcher.h>
#endif
#ifndef _UNO_DATA_H_
#include <uno/data.h>
#endif
#ifndef _UNO_ANY2_H_
#include <uno/any2.h>
#endif
#ifndef _UNO_MAPPING_HXX_
#include <uno/mapping.hxx>
#endif

#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif

#include <com/sun/star/uno/XAggregation.hpp>
#include <com/sun/star/script/XInvocationAdapterFactory.hpp>
#include <com/sun/star/script/XInvocationAdapterFactory2.hpp>
#include <com/sun/star/script/XInvocation.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/reflection/InvocationTargetException.hpp>

using namespace cppu;
using namespace rtl;
using namespace osl;
using namespace com::sun::star::uno;
using namespace com::sun::star::script;
using namespace com::sun::star::reflection;
using namespace com::sun::star::registry;
using namespace com::sun::star::lang;

#define SERVICENAME "com.sun.star.script.InvocationAdapterFactory"
#define IMPLNAME    "com.sun.star.comp.stoc.InvocationAdapterFactory"

namespace stoc_invadp
{

//--------------------------------------------------------------------------------------------------
static inline Sequence< OUString > getSupportedServiceNames()
{
    OUString aName( RTL_CONSTASCII_USTRINGPARAM( SERVICENAME ) );
    return Sequence< OUString >( &aName, 1 );
}

//==================================================================================================
class FactoryImpl
    : public WeakImplHelper3< XServiceInfo, XInvocationAdapterFactory, XInvocationAdapterFactory2 >
{
    Mapping _aUno2Cpp;
    Mapping _aCpp2Uno;

public:
    FactoryImpl();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString & rServiceName ) throw (RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (RuntimeException);

    // XInvocationAdapterFactory
    virtual Reference< XInterface > SAL_CALL createAdapter(
        const Reference< XInvocation > & xReceiver,
        const Type & rType ) throw (RuntimeException);
    // XInvocationAdapterFactory2
    virtual Reference< XInterface > SAL_CALL createAdapter(
        const Reference< XInvocation > & xReceiver,
        const Sequence< Type > & rTypes ) throw (RuntimeException);
};

struct AdapterImpl;
//==================================================================================================
struct InterfaceAdapterImpl : public uno_Interface
{
    AdapterImpl * pAdapter;
    typelib_InterfaceTypeDescription * pTypeDescr;
};
//==================================================================================================
struct AdapterImpl
{
    oslInterlockedCount         nRef;
    uno_Interface *             pReceiver; // XInvocation receiver

    sal_Int32                   nInterfaces;
    InterfaceAdapterImpl *      pInterfaces;

    // XInvocation calls
    void getValue( const typelib_TypeDescription * pMemberType,
                   void * pReturn, void * pArgs[], uno_Any ** ppException );
    void setValue( const typelib_TypeDescription * pMemberType,
                   void * pReturn, void * pArgs[], uno_Any ** ppException );
    void invoke( const typelib_TypeDescription * pMemberType,
                 void * pReturn, void * pArgs[], uno_Any ** ppException );

    inline ~AdapterImpl();

    static inline uno_Interface * createAdapter(
        uno_Interface * pReceiver_, const Sequence< Type > & rTypes );
};

//--------------------------------------------------------------------------------------------------
inline static sal_Bool coerce_assign(
    void * pDest, typelib_TypeDescription * pTD, uno_Any * pSource )
{
    if (pSource->pType->eTypeClass != typelib_TypeClass_VOID)
    {
        if (pTD->eTypeClass == typelib_TypeClass_ANY)
            return uno_assignData( pDest, pTD, pSource, pTD, 0, 0, 0 );
        else
            return uno_type_assignData( pDest, pTD->pWeakRef, pSource->pData, pSource->pType, 0, 0, 0 );
    }
    else
    {
        uno_constructData( pDest, pTD );
        return sal_True;
    }
}

//--------------------------------------------------------------------------------------------------
static inline void copyUnoAny( uno_Any * pDest, uno_Any * pSource )
{
    ::uno_type_any_construct( pDest, pSource->pData, pSource->pType, 0 );
}
//--------------------------------------------------------------------------------------------------
static inline void constructRuntimeException( uno_Any * pExc, const OUString & rMsg )
{
    RuntimeException aExc;
    aExc.Message = rMsg;
    typelib_TypeDescription * pTD = 0;
    const Type & rType = ::getCppuType( (const RuntimeException *)0 );
    // no conversion neeeded due to binary compatibility + no convertable type
    ::uno_type_any_construct( pExc, &aExc, rType.getTypeLibType(), 0 );
}

//__________________________________________________________________________________________________
void AdapterImpl::getValue(
    const typelib_TypeDescription * pMemberType,
    void * pReturn, void * pArgs[], uno_Any ** ppException )
{
    // XInvocation type description
    typelib_InterfaceTypeDescription * pInvocationTD = 0;
    const Type & rIType = ::getCppuType( (const Reference< XInvocation > *)0 );
    TYPELIB_DANGER_GET( (typelib_TypeDescription **)&pInvocationTD, rIType.getTypeLibType() );
    // getValue()
    typelib_InterfaceMethodTypeDescription * pInvokMethodTD = 0;
    TYPELIB_DANGER_GET( (typelib_TypeDescription **)&pInvokMethodTD, pInvocationTD->ppMembers[3] );
    // attribute type
    typelib_TypeDescription * pAttributeTD = 0;
    TYPELIB_DANGER_GET( &pAttributeTD, ((typelib_InterfaceAttributeTypeDescription *)pMemberType)->pAttributeTypeRef );

    uno_Any aInvokRet;
    void * pInvokArgs[1];
    pInvokArgs[0] = &((typelib_InterfaceMemberTypeDescription *)pMemberType)->pMemberName;
    uno_Any aInvokExc;
    uno_Any * pInvokExc = &aInvokExc;

    // getValue()
    (*pReceiver->pDispatcher)(
        pReceiver, (typelib_TypeDescription *)pInvokMethodTD,
        &aInvokRet, pInvokArgs, &pInvokExc );

    if (pInvokExc) // getValue() call exception
    {
        copyUnoAny( *ppException, pInvokExc );
        uno_any_destruct( pInvokExc, 0 ); // cleanup
    }
    else // invocation call succeeded
    {
        uno_constructData( pReturn, pAttributeTD );
        if (coerce_assign( pReturn, pAttributeTD, &aInvokRet ))
        {
            *ppException = 0; // no exceptions be thrown
        }
        else // no assignment possible => throw runtime exception
        {
            uno_destructData( pReturn, pAttributeTD, 0 );
            constructRuntimeException(
                *ppException,
                OUString( RTL_CONSTASCII_USTRINGPARAM("cannot coerce return type of attribute get call!") ) );
        }
        uno_any_destruct( &aInvokRet, 0 );
    }

    TYPELIB_DANGER_RELEASE( pAttributeTD );
    TYPELIB_DANGER_RELEASE( (typelib_TypeDescription *)pInvokMethodTD );
    TYPELIB_DANGER_RELEASE( (typelib_TypeDescription *)pInvocationTD );
}
//__________________________________________________________________________________________________
void AdapterImpl::setValue(
    const typelib_TypeDescription * pMemberType,
    void * pReturn, void * pArgs[], uno_Any ** ppException )
{
    // XInvocation type description
    typelib_InterfaceTypeDescription * pInvocationTD = 0;
    const Type & rIType = ::getCppuType( (const Reference< XInvocation > *)0 );
    TYPELIB_DANGER_GET( (typelib_TypeDescription **)&pInvocationTD, rIType.getTypeLibType() );
    // setValue()
    typelib_InterfaceMethodTypeDescription * pInvokMethodTD = 0;
    TYPELIB_DANGER_GET( (typelib_TypeDescription **)&pInvokMethodTD, pInvocationTD->ppMembers[2] );
    // attribute type
    typelib_TypeDescription * pAttributeTD = 0;
    TYPELIB_DANGER_GET( &pAttributeTD, ((typelib_InterfaceAttributeTypeDescription *)pMemberType)->pAttributeTypeRef );

    uno_Any aInvokVal;
    uno_any_construct( &aInvokVal, pArgs[0], pAttributeTD, 0 );

    void * pInvokArgs[2];
    pInvokArgs[0] = &((typelib_InterfaceMemberTypeDescription *)pMemberType)->pMemberName;
    pInvokArgs[1] = &aInvokVal;
    uno_Any aInvokExc;
    uno_Any * pInvokExc = &aInvokExc;

    // setValue()
    (*pReceiver->pDispatcher)(
        pReceiver, (typelib_TypeDescription *)pInvokMethodTD,
        0, pInvokArgs, &pInvokExc );

    if (pInvokExc) // setValue() call exception
    {
        copyUnoAny( *ppException, pInvokExc );
        uno_any_destruct( pInvokExc, 0 ); // cleanup
    }
    else // invocation call succeeded
    {
        *ppException = 0; // no exceptions be thrown
    }

    uno_any_destruct( &aInvokVal, 0 ); // cleanup

    TYPELIB_DANGER_RELEASE( pAttributeTD );
    TYPELIB_DANGER_RELEASE( (typelib_TypeDescription *)pInvokMethodTD );
    TYPELIB_DANGER_RELEASE( (typelib_TypeDescription *)pInvocationTD );
}
//__________________________________________________________________________________________________
void AdapterImpl::invoke(
    const typelib_TypeDescription * pMemberType,
    void * pReturn, void * pArgs[], uno_Any ** ppException )
{
    // XInvocation type description
    typelib_InterfaceTypeDescription * pInvocationTD = 0;
    const Type & rIType = ::getCppuType( (const Reference< XInvocation > *)0 );
    TYPELIB_DANGER_GET( (typelib_TypeDescription **)&pInvocationTD, rIType.getTypeLibType() );
    // invoke()
    typelib_InterfaceMethodTypeDescription * pInvokMethodTD = 0;
    TYPELIB_DANGER_GET( (typelib_TypeDescription **)&pInvokMethodTD, pInvocationTD->ppMembers[1] );

    sal_Int32 nParams = ((typelib_InterfaceMethodTypeDescription *)pMemberType)->nParams;
    typelib_MethodParameter * pFormalParams = ((typelib_InterfaceMethodTypeDescription *)pMemberType)->pParams;

    // count in params
    sal_Int32 nInParams = 0;
    sal_Int32 nPos;
    for ( nPos = nParams; nPos--; )
    {
        if (pFormalParams[nPos].bIn)
            ++nInParams;
    }

    // in params
    typelib_TypeDescription * pAnySeqTD = 0;
    const Type & rAnyType = ::getCppuType( (const Sequence< Any > *)0 );
    TYPELIB_DANGER_GET( &pAnySeqTD, rAnyType.getTypeLibType() );
    uno_Sequence * pInParamsSeq = 0;
    uno_sequence_construct( &pInParamsSeq, pAnySeqTD, 0, nInParams, 0 );

    uno_Any * pInAnys = (uno_Any *)pInParamsSeq->elements;
    typelib_TypeDescription * pAnyTD = 0;
    TYPELIB_DANGER_GET( &pAnyTD, ((typelib_IndirectTypeDescription *)pAnySeqTD)->pType );
    sal_Int32 nInParamsPos = nInParams;
    for ( nPos = nParams; nPos--; )
    {
        typelib_MethodParameter & rParam = pFormalParams[nPos];
        if (rParam.bIn)
        {
            typelib_TypeDescription * pTD = 0;
            TYPELIB_DANGER_GET( &pTD, rParam.pTypeRef );
            // assignment to any never fails...
            uno_assignData( &pInAnys[--nInParamsPos], pAnyTD, pArgs[nPos], pTD, 0, 0, 0 );
            TYPELIB_DANGER_RELEASE( pTD );
        }
    }

    // out params, out indices
    uno_Sequence * pOutIndices;
    uno_Sequence * pOutParams;
    // return
    uno_Any aInvokRet;
    // perform call
    void * pInvokArgs[4];
    pInvokArgs[0] = &((typelib_InterfaceMemberTypeDescription *)pMemberType)->pMemberName;
    pInvokArgs[1] = &pInParamsSeq;
    pInvokArgs[2] = &pOutIndices;
    pInvokArgs[3] = &pOutParams;
    uno_Any aInvokExc;
    uno_Any * pInvokExc = &aInvokExc;

    // invoke() call
    (*pReceiver->pDispatcher)(
        pReceiver, (typelib_TypeDescription *)pInvokMethodTD,
        &aInvokRet, pInvokArgs, &pInvokExc );

    if (pInvokExc)
    {
        OUString aInvokExcName( pInvokExc->pType->pTypeName );
        if (aInvokExcName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.reflection.InvocationTargetException") ))
        {
            // unwrap invocation target exception
            copyUnoAny( *ppException,
                        &((InvocationTargetException *)pInvokExc->pData)->TargetException );
        }
        else
        {
            // defer original exception to caller
            copyUnoAny( *ppException, pInvokExc );
        }
        uno_any_destruct( pInvokExc, 0 ); // cleanup
    }
    else // no invocation exception
    {
        typelib_TypeDescription * pShortSeqTD = 0;
        const Type & rSeqShortType = ::getCppuType( (const Sequence< sal_Int16 > *)0 );
        TYPELIB_DANGER_GET( &pShortSeqTD, rSeqShortType.getTypeLibType() );

        // write changed out params
        OSL_ENSHURE( pOutParams->nElements == pOutIndices->nElements, "### out params lens differ!" );
        if (pOutParams->nElements == pOutIndices->nElements)
        {
            sal_Int16 * pIndices = (sal_Int16 *)pOutIndices->elements;
            uno_Any * pOut       = (uno_Any *)pOutParams->elements;
            for ( nPos = 0; nPos < pOutIndices->nElements; ++nPos )
            {
                sal_Int32 nIndex = pIndices[nPos];
                typelib_TypeDescription * pTD = 0;
                TYPELIB_DANGER_GET( &pTD, pFormalParams[nIndex].pTypeRef );
                OSL_ENSHURE( nIndex < nParams, "### illegal index!" );
                if (! pFormalParams[nIndex].bIn) // is pure out param
                    uno_constructData( pArgs[nIndex], pTD );
                if (! coerce_assign( pArgs[nIndex], pTD, &pOut[nPos] )) // if fail
                {
                    // cleanup of out params
                    if (! pFormalParams[nIndex].bIn) // is pure out param
                        uno_destructData( pArgs[nIndex], pTD, 0 );
                    TYPELIB_DANGER_RELEASE( pTD );
                    for ( sal_Int32 n = 0; n < nPos; ++n )
                    {
                        sal_Int32 nIndex = pIndices[n];
                        pTD = 0;
                        TYPELIB_DANGER_GET( &pTD, pFormalParams[nIndex].pTypeRef );
                        OSL_ENSHURE( nIndex < nParams, "### illegal index!" );
                        uno_destructData( pArgs[nIndex], pTD, 0 );
                        TYPELIB_DANGER_RELEASE( pTD );
                    }
                    break;
                }
                TYPELIB_DANGER_RELEASE( pTD );
            }
            if (nPos == pOutIndices->nElements) // out param copy ok; write return value
            {
                // return value
                typelib_TypeDescription * pReturnTD = 0;
                TYPELIB_DANGER_GET( &pReturnTD, ((typelib_InterfaceMethodTypeDescription *)pMemberType)->pReturnTypeRef );
                uno_constructData( pReturn, pReturnTD );
                if (coerce_assign( pReturn, pReturnTD, &aInvokRet ))
                {
                    *ppException = 0; // no exception
                }
                else
                {
                    uno_destructData( pReturn, pReturnTD, 0 );
                    // set runtime exception
                    constructRuntimeException(
                        *ppException,
                        OUString( RTL_CONSTASCII_USTRINGPARAM("failed to coerce return type during invocation call!") ) );
                }
                TYPELIB_DANGER_RELEASE( pReturnTD );
            }
            else
            {
                // set runtime exception
                constructRuntimeException(
                    *ppException,
                    OUString( RTL_CONSTASCII_USTRINGPARAM("failed to coerce parameter type during invocation call!") ) );
            }
        }
        else
        {
            // set runtime exception
            constructRuntimeException(
                *ppException,
                OUString( RTL_CONSTASCII_USTRINGPARAM("out params lengths differ after invocation call!") ) );
        }
        // cleanup invok out params
        uno_destructData( &pOutIndices, pShortSeqTD, 0 );
        uno_destructData( &pOutParams, pAnySeqTD, 0 );
        // cleanup invok return value
        uno_any_destruct( &aInvokRet, 0 );

        TYPELIB_DANGER_RELEASE( pShortSeqTD );
    }
    // cleanup constructed in params
    uno_destructData( &pInParamsSeq, pAnySeqTD, 0 );
    TYPELIB_DANGER_RELEASE( pAnyTD );
    TYPELIB_DANGER_RELEASE( pAnySeqTD );

    TYPELIB_DANGER_RELEASE( (typelib_TypeDescription *)pInvokMethodTD );
    TYPELIB_DANGER_RELEASE( (typelib_TypeDescription *)pInvocationTD );
}

//--------------------------------------------------------------------------------------------------
static inline td_equals(
    typelib_TypeDescription * pTD, typelib_TypeDescriptionReference * pType )
{
    return (pTD->pWeakRef == pType ||
            (pTD->pTypeName->length == pType->pTypeName->length &&
             rtl_ustr_compare( pTD->pTypeName->buffer, pType->pTypeName->buffer ) == 0));
}

extern "C"
{
//__________________________________________________________________________________________________
static void SAL_CALL adapter_acquire( uno_Interface * pUnoI )
{
    ::osl_incrementInterlockedCount(
        & static_cast< InterfaceAdapterImpl * >( pUnoI )->pAdapter->nRef );
}
//__________________________________________________________________________________________________
static void SAL_CALL adapter_release( uno_Interface * pUnoI )
{
    AdapterImpl * pThis = static_cast< InterfaceAdapterImpl * >( pUnoI )->pAdapter;
    if (! osl_decrementInterlockedCount( &pThis->nRef ))
    {
        delete pThis;
    }
}
//__________________________________________________________________________________________________
static void SAL_CALL adapter_dispatch(
    uno_Interface * pUnoI, const typelib_TypeDescription * pMemberType,
    void * pReturn, void * pArgs[], uno_Any ** ppException )
{
    AdapterImpl * pThis =
        static_cast< InterfaceAdapterImpl * >( pUnoI )->pAdapter;

    // query to emulated interface
    switch (((typelib_InterfaceMemberTypeDescription *)pMemberType)->nPosition)
    {
    case 0: // queryInterface()
    {
        *ppException = 0; // no exc
        typelib_TypeDescriptionReference * pDemanded =
            *(typelib_TypeDescriptionReference **)pArgs[0];
        // pInterfaces[0] is XInterface
        for ( sal_Int32 nPos = 0; nPos < pThis->nInterfaces; ++nPos )
        {
            typelib_InterfaceTypeDescription * pTD = pThis->pInterfaces[nPos].pTypeDescr;
            while (pTD)
            {
                if (td_equals( (typelib_TypeDescription *)pTD, pDemanded ))
                {
                    uno_Interface * pUnoI = &pThis->pInterfaces[nPos];
                    ::uno_any_construct(
                        (uno_Any *)pReturn, &pUnoI, (typelib_TypeDescription *)pTD, 0 );
                    return;
                }
                pTD = pTD->pBaseTypeDescription;
            }
        }
        ::uno_any_construct( (uno_Any *)pReturn, 0, 0, 0 ); // clear()
        break;
    }
    case 1: // acquire()
        *ppException = 0; // no exc
        adapter_acquire( pUnoI );
        break;
    case 2: // release()
        *ppException = 0; // no exc
        adapter_release( pUnoI );
        break;

    default:
    {
        if (pMemberType->eTypeClass == typelib_TypeClass_INTERFACE_METHOD) // method
        {
            pThis->invoke( pMemberType, pReturn, pArgs, ppException );
        }
        else // attribute
        {
            if (pReturn)
                pThis->getValue( pMemberType, pReturn, pArgs, ppException );
            else
                pThis->setValue( pMemberType, pReturn, pArgs, ppException );
        }
    }
    }
}
}

//__________________________________________________________________________________________________
inline uno_Interface * AdapterImpl::createAdapter(
    uno_Interface * pReceiver_, const Sequence< Type > & rTypes )
{
    AdapterImpl * pThis = new AdapterImpl();

    pThis->nRef = 1;

    (*pReceiver_->acquire)( pReceiver_ );
    pThis->pReceiver = pReceiver_;

    pThis->nInterfaces = rTypes.getLength();
    pThis->pInterfaces = new InterfaceAdapterImpl[ rTypes.getLength() ];

    const Type * pTypes = rTypes.getConstArray();
    for ( sal_Int32 nPos = rTypes.getLength(); nPos--; )
    {
        InterfaceAdapterImpl * pInterface = &pThis->pInterfaces[nPos];

        pInterface->pAdapter = pThis;
        pInterface->pTypeDescr = 0;
        pTypes[nPos].getDescription( (typelib_TypeDescription **)&pInterface->pTypeDescr );
        OSL_ASSERT( pInterface->pTypeDescr );
        //
        pInterface->acquire = adapter_acquire;
        pInterface->release = adapter_release;
        pInterface->pDispatcher = adapter_dispatch;
    }

    // returns XInterface
    return &pThis->pInterfaces[0];
}
//__________________________________________________________________________________________________
inline AdapterImpl::~AdapterImpl()
{
    for ( sal_Int32 nPos = nInterfaces; nPos--; )
    {
        ::typelib_typedescription_release( (typelib_TypeDescription *)pInterfaces[nPos].pTypeDescr );
    }
    delete [] pInterfaces;
    //
    (*pReceiver->release)( pReceiver );
}

//__________________________________________________________________________________________________
FactoryImpl::FactoryImpl()
{
    OUString aCppEnvTypeName( RTL_CONSTASCII_USTRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME) );
    OUString aUnoEnvTypeName( RTL_CONSTASCII_USTRINGPARAM(UNO_LB_UNO) );

    _aUno2Cpp = Mapping( aUnoEnvTypeName, aCppEnvTypeName );
    _aCpp2Uno = Mapping( aCppEnvTypeName, aUnoEnvTypeName );
    OSL_ENSHURE( _aUno2Cpp.is() && _aCpp2Uno.is(), "### no uno / c++ mappings!" );
}

// XInvocationAdapterFactory
//__________________________________________________________________________________________________
Reference< XInterface > FactoryImpl::createAdapter(
    const Reference< XInvocation > & xReceiver, const Type & rType )
    throw (RuntimeException)
{
    return createAdapter( xReceiver, Sequence< Type >( &rType, 1 ) );
}
// XInvocationAdapterFactory2
//__________________________________________________________________________________________________
Reference< XInterface > FactoryImpl::createAdapter(
    const Reference< XInvocation > & xReceiver, const Sequence< Type > & rTypes )
    throw (RuntimeException)
{
    Reference< XInterface > xRet;
    if (xReceiver.is() && rTypes.getLength())
    {
        uno_Interface * pReceiver = (uno_Interface *)_aCpp2Uno.mapInterface(
            xReceiver.get(), ::getCppuType( &xReceiver ) );
        if (pReceiver)
        {
            uno_Interface * pRet = AdapterImpl::createAdapter( pReceiver, rTypes );
            _aUno2Cpp.mapInterface( (void **)&xRet, pRet, ::getCppuType( &xRet ) );
            OSL_ASSERT( xRet.is() );
            (*pRet->release)( pRet );
            (*pReceiver->release)( pReceiver );
        }
    }
    return xRet;
}

// XServiceInfo
//__________________________________________________________________________________________________
OUString FactoryImpl::getImplementationName()
    throw (RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME) );
}
//__________________________________________________________________________________________________
sal_Bool FactoryImpl::supportsService( const OUString & rServiceName )
    throw (RuntimeException)
{
    const Sequence< OUString > & rSNL = getSupportedServiceNames();
    const OUString * pArray = rSNL.getConstArray();
    for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
    {
        if (pArray[nPos] == rServiceName)
            return sal_True;
    }
    return sal_False;
}
//__________________________________________________________________________________________________
Sequence< OUString > FactoryImpl::getSupportedServiceNames()
    throw (RuntimeException)
{
    return stoc_invadp::getSupportedServiceNames();
}

//==================================================================================================
static Reference< XInterface > SAL_CALL FactoryImpl_create(
    const Reference< XMultiServiceFactory > & xMgr )
    throw (Exception)
{
    return Reference< XInterface >( *new FactoryImpl() );
}

}


//##################################################################################################
//##################################################################################################
//##################################################################################################


extern "C"
{
//==================================================================================================
void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
//==================================================================================================
sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            Reference< XRegistryKey > xNewKey(
                reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("/" IMPLNAME "/UNO/SERVICES") ) ) );

            const Sequence< OUString > & rSNL = stoc_invadp::getSupportedServiceNames();
            const OUString * pArray = rSNL.getConstArray();
            for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );

            return sal_True;
        }
        catch (InvalidRegistryException &)
        {
            OSL_ENSHURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}
//==================================================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pRet = 0;

    if (pServiceManager && rtl_str_compare( pImplName, IMPLNAME ) == 0)
    {
        Reference< XSingleServiceFactory > xFactory( createOneInstanceFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME) ),
            stoc_invadp::FactoryImpl_create,
            stoc_invadp::getSupportedServiceNames() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}
}


