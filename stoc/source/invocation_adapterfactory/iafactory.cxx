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


#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include <osl/diagnose.h>
#include <osl/interlck.h>
#include <osl/mutex.hxx>

#include <uno/dispatcher.h>
#include <uno/data.h>
#include <uno/any2.h>
#include <uno/mapping.hxx>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/implementationentry.hxx>

#include <com/sun/star/uno/XAggregation.hpp>
#include <com/sun/star/script/XTypeConverter.hpp>
#include <com/sun/star/script/XInvocationAdapterFactory.hpp>
#include <com/sun/star/script/XInvocationAdapterFactory2.hpp>
#include <com/sun/star/script/XInvocation.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/reflection/InvocationTargetException.hpp>
#include "com/sun/star/uno/RuntimeException.hpp"

#define SERVICENAME "com.sun.star.script.InvocationAdapterFactory"
#define IMPLNAME    "com.sun.star.comp.stoc.InvocationAdapterFactory"


using namespace ::std;
using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace stoc_invadp
{

static Sequence< OUString > invadp_getSupportedServiceNames()
{
    Sequence< OUString > seqNames(1);
    seqNames.getArray()[0] =
        OUString(SERVICENAME);
    return seqNames;
}

static OUString invadp_getImplementationName()
{
    return OUString(IMPLNAME);
}

struct hash_ptr
{
    inline size_t operator() ( void * p ) const
        { return (size_t)p; }
};
typedef boost::unordered_set< void *, hash_ptr, equal_to< void * > > t_ptr_set;
typedef boost::unordered_map< void *, t_ptr_set, hash_ptr, equal_to< void * > > t_ptr_map;

//==============================================================================
class FactoryImpl
    : public ::cppu::WeakImplHelper3< lang::XServiceInfo,
                                      script::XInvocationAdapterFactory,
                                      script::XInvocationAdapterFactory2 >
{
public:
    Mapping m_aUno2Cpp;
    Mapping m_aCpp2Uno;
    uno_Interface * m_pConverter;

    typelib_TypeDescription * m_pInvokMethodTD;
    typelib_TypeDescription * m_pSetValueTD;
    typelib_TypeDescription * m_pGetValueTD;
    typelib_TypeDescription * m_pAnySeqTD;
    typelib_TypeDescription * m_pShortSeqTD;
    typelib_TypeDescription * m_pConvertToTD;

    Mutex m_mutex;
    t_ptr_map m_receiver2adapters;

    FactoryImpl( Reference< XComponentContext > const & xContext )
        SAL_THROW( (RuntimeException) );
    virtual ~FactoryImpl() SAL_THROW(());

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw (RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString & rServiceName )
        throw (RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw (RuntimeException);

    // XInvocationAdapterFactory
    virtual Reference< XInterface > SAL_CALL createAdapter(
        const Reference< script::XInvocation > & xReceiver, const Type & rType )
        throw (RuntimeException);
    // XInvocationAdapterFactory2
    virtual Reference< XInterface > SAL_CALL createAdapter(
        const Reference< script::XInvocation > & xReceiver,
        const Sequence< Type > & rTypes )
        throw (RuntimeException);
};
struct AdapterImpl;
//==============================================================================
struct InterfaceAdapterImpl : public uno_Interface
{
    AdapterImpl *                           m_pAdapter;
    typelib_InterfaceTypeDescription *      m_pTypeDescr;
};
//==============================================================================
struct AdapterImpl
{
    oslInterlockedCount         m_nRef;
    FactoryImpl *               m_pFactory;
    void *                      m_key; // map key
    uno_Interface *             m_pReceiver; // XInvocation receiver

    sal_Int32                   m_nInterfaces;
    InterfaceAdapterImpl *      m_pInterfaces;

    // XInvocation calls
    void getValue(
        const typelib_TypeDescription * pMemberType,
        void * pReturn, uno_Any ** ppException );
    void setValue(
        const typelib_TypeDescription * pMemberType,
        void * pArgs[], uno_Any ** ppException );
    void invoke(
        const typelib_TypeDescription * pMemberType,
        void * pReturn, void * pArgs[], uno_Any ** ppException );

    bool coerce_assign(
        void * pDest, typelib_TypeDescriptionReference * pType,
        uno_Any * pSource, uno_Any * pExc );
    inline bool coerce_construct(
        void * pDest, typelib_TypeDescriptionReference * pType,
        uno_Any * pSource, uno_Any * pExc );

    inline void acquire()
        SAL_THROW(());
    inline void release()
        SAL_THROW(());
    inline ~AdapterImpl()
        SAL_THROW(());
    inline AdapterImpl(
        void * key, Reference< script::XInvocation > const & xReceiver,
        const Sequence< Type > & rTypes,
        FactoryImpl * pFactory )
        SAL_THROW( (RuntimeException) );
};
//______________________________________________________________________________
inline AdapterImpl::~AdapterImpl()
    SAL_THROW(())
{
    for ( sal_Int32 nPos = m_nInterfaces; nPos--; )
    {
        ::typelib_typedescription_release(
            (typelib_TypeDescription *)m_pInterfaces[ nPos ].m_pTypeDescr );
    }
    delete [] m_pInterfaces;
    //
    (*m_pReceiver->release)( m_pReceiver );
    m_pFactory->release();
}
//______________________________________________________________________________
inline void AdapterImpl::acquire()
    SAL_THROW(())
{
    osl_atomic_increment( &m_nRef );
}
//______________________________________________________________________________
inline void AdapterImpl::release()
    SAL_THROW(())
{
    bool delete_this = false;
    {
    MutexGuard guard( m_pFactory->m_mutex );
    if (! osl_atomic_decrement( &m_nRef ))
    {
        t_ptr_map::iterator iFind(
            m_pFactory->m_receiver2adapters.find( m_key ) );
        OSL_ASSERT( m_pFactory->m_receiver2adapters.end() != iFind );
        t_ptr_set & adapter_set = iFind->second;
        if (adapter_set.erase( this ) != 1) {
            OSL_ASSERT( false );
        }
        if (adapter_set.empty())
        {
            m_pFactory->m_receiver2adapters.erase( iFind );
        }
        delete_this = true;
    }
    }
    if (delete_this)
        delete this;
}

//------------------------------------------------------------------------------
static inline void constructRuntimeException(
    uno_Any * pExc, const OUString & rMsg )
{
    RuntimeException exc( rMsg, Reference< XInterface >() );
    // no conversion needed due to binary compatibility + no convertible type
    ::uno_type_any_construct(
        pExc, &exc, ::getCppuType( &exc ).getTypeLibType(), 0 );
}

//------------------------------------------------------------------------------
static inline bool type_equals(
    typelib_TypeDescriptionReference * pType1,
    typelib_TypeDescriptionReference * pType2 )
    SAL_THROW(())
{
    return (pType1 == pType2 ||
            (pType1->pTypeName->length == pType2->pTypeName->length &&
             0 == ::rtl_ustr_compare(
                 pType1->pTypeName->buffer, pType2->pTypeName->buffer )));
}

//______________________________________________________________________________
bool AdapterImpl::coerce_assign(
    void * pDest, typelib_TypeDescriptionReference * pType, uno_Any * pSource,
    uno_Any * pOutExc )
{
    if (typelib_TypeClass_ANY == pType->eTypeClass)
    {
        ::uno_type_any_assign(
            (uno_Any *)pDest, pSource->pData, pSource->pType, 0, 0 );
        return true;
    }
    if (::uno_type_assignData(
            pDest, pType, pSource->pData, pSource->pType, 0, 0, 0 ))
    {
        return true;
    }
    else // try type converter
    {
        uno_Any ret;
        void * args[ 2 ];
        args[ 0 ] = pSource;
        args[ 1 ] = &pType;
        uno_Any exc;
        uno_Any * p_exc = &exc;

        // converTo()
        (*m_pFactory->m_pConverter->pDispatcher)(
            m_pFactory->m_pConverter,
            m_pFactory->m_pConvertToTD, &ret, args, &p_exc );

        if (p_exc) // exception occurred
        {
            OSL_ASSERT(
                p_exc->pType->eTypeClass == typelib_TypeClass_EXCEPTION );
            if (typelib_typedescriptionreference_isAssignableFrom(
                    ::getCppuType(
                        (RuntimeException const *) 0 ).getTypeLibType(),
                    p_exc->pType ))
            {
                // is RuntimeException or derived: rethrow
                uno_type_any_construct(
                    pOutExc, p_exc->pData, p_exc->pType, 0 );
            }
            else
            {
                // set runtime exception
                constructRuntimeException(
                    pOutExc, "type coercion failed: " +
                    reinterpret_cast< Exception const * >(
                        p_exc->pData )->Message );
            }
            ::uno_any_destruct( p_exc, 0 );
            // pOutExc constructed
            return false;
        }
        else
        {
            bool succ = (sal_False != ::uno_type_assignData(
                             pDest, pType, ret.pData, ret.pType, 0, 0, 0 ));
            ::uno_any_destruct( &ret, 0 );
            OSL_ENSURE(
                succ, "### conversion succeeded, but assignment failed!?" );
            if (! succ)
            {
                // set runtime exception
                constructRuntimeException(
                    pOutExc,
                    "type coercion failed: "
                    "conversion succeeded, but assignment failed?!" );
            }
            return succ;
        }
    }
}
//______________________________________________________________________________
inline bool AdapterImpl::coerce_construct(
    void * pDest, typelib_TypeDescriptionReference * pType, uno_Any * pSource,
    uno_Any * pExc )
{
    if (typelib_TypeClass_ANY == pType->eTypeClass)
    {
        ::uno_type_copyData( pDest, pSource, pType, 0 );
        return true;
    }
    if (type_equals( pType, pSource->pType))
    {
        ::uno_type_copyData( pDest, pSource->pData, pType, 0 );
        return true;
    }
    ::uno_type_constructData( pDest, pType );
    return coerce_assign( pDest, pType, pSource, pExc );
}

//------------------------------------------------------------------------------
static void handleInvokExc( uno_Any * pDest, uno_Any * pSource )
{
    OUString const & name =
        *reinterpret_cast< OUString const * >( &pSource->pType->pTypeName );

    if ( name == "com.sun.star.reflection.InvocationTargetException" )
    {
        // unwrap invocation target exception
        uno_Any * target_exc =
            &reinterpret_cast< reflection::InvocationTargetException * >(
                pSource->pData )->TargetException;
        ::uno_type_any_construct(
            pDest, target_exc->pData, target_exc->pType, 0 );
    }
    else // all other exceptions are wrapped to RuntimeException
    {
        if (typelib_TypeClass_EXCEPTION == pSource->pType->eTypeClass)
        {
            constructRuntimeException(
                pDest, ((Exception const *)pSource->pData)->Message );
        }
        else
        {
            constructRuntimeException(
                pDest, "no exception has been thrown via invocation?!" );
        }
    }
}
//______________________________________________________________________________
void AdapterImpl::getValue(
    const typelib_TypeDescription * pMemberType,
    void * pReturn, uno_Any ** ppException )
{
    uno_Any aInvokRet;
    void * pInvokArgs[1];
    pInvokArgs[0] =
        &((typelib_InterfaceMemberTypeDescription *)pMemberType)->pMemberName;
    uno_Any aInvokExc;
    uno_Any * pInvokExc = &aInvokExc;

    // getValue()
    (*m_pReceiver->pDispatcher)(
        m_pReceiver, m_pFactory->m_pGetValueTD,
        &aInvokRet, pInvokArgs, &pInvokExc );

    if (pInvokExc) // getValue() call exception
    {
        handleInvokExc( *ppException, pInvokExc );
        ::uno_any_destruct( pInvokExc, 0 ); // cleanup
    }
    else // invocation call succeeded
    {
        if (coerce_construct(
                pReturn,
                ((typelib_InterfaceAttributeTypeDescription *)
                 pMemberType)->pAttributeTypeRef,
                &aInvokRet, *ppException ))
        {
            *ppException = 0; // no exceptions be thrown
        }
        ::uno_any_destruct( &aInvokRet, 0 );
    }
}
//______________________________________________________________________________
void AdapterImpl::setValue(
    const typelib_TypeDescription * pMemberType,
    void * pArgs[], uno_Any ** ppException )
{
    uno_Any aInvokVal;
    ::uno_type_any_construct(
        &aInvokVal, pArgs[0],
        ((typelib_InterfaceAttributeTypeDescription *)
         pMemberType)->pAttributeTypeRef, 0 );

    void * pInvokArgs[2];
    pInvokArgs[0] =
        &((typelib_InterfaceMemberTypeDescription *)pMemberType)->pMemberName;
    pInvokArgs[1] = &aInvokVal;
    uno_Any aInvokExc;
    uno_Any * pInvokExc = &aInvokExc;

    // setValue()
    (*m_pReceiver->pDispatcher)(
        m_pReceiver, m_pFactory->m_pSetValueTD, 0, pInvokArgs, &pInvokExc );

    if (pInvokExc) // setValue() call exception
    {
        handleInvokExc( *ppException, pInvokExc );
        ::uno_any_destruct( pInvokExc, 0 ); // cleanup
    }
    else // invocation call succeeded
    {
        *ppException = 0; // no exceptions be thrown
    }

    ::uno_any_destruct( &aInvokVal, 0 ); // cleanup
}
//______________________________________________________________________________
void AdapterImpl::invoke(
    const typelib_TypeDescription * pMemberType,
    void * pReturn, void * pArgs[], uno_Any ** ppException )
{
    sal_Int32 nParams =
        ((typelib_InterfaceMethodTypeDescription *)pMemberType)->nParams;
    typelib_MethodParameter * pFormalParams =
        ((typelib_InterfaceMethodTypeDescription *)pMemberType)->pParams;

    // in params
    uno_Sequence * pInParamsSeq = 0;
    ::uno_sequence_construct(
        &pInParamsSeq, m_pFactory->m_pAnySeqTD, 0, nParams, 0 );
    uno_Any * pInAnys = (uno_Any *)pInParamsSeq->elements;
    sal_Int32 nOutParams = 0;
    sal_Int32 nPos;
    for ( nPos = nParams; nPos--; )
    {
        typelib_MethodParameter const & rParam = pFormalParams[nPos];
        if (rParam.bIn) // is in/inout param
        {
            ::uno_type_any_assign(
                &pInAnys[nPos], pArgs[nPos], rParam.pTypeRef, 0, 0 );
        }
        // else: pure out is empty any

        if (rParam.bOut)
            ++nOutParams;
    }

    // out params, out indices
    uno_Sequence * pOutIndices;
    uno_Sequence * pOutParams;
    // return value
    uno_Any aInvokRet;
    // perform call
    void * pInvokArgs[4];
    pInvokArgs[0] =
        &((typelib_InterfaceMemberTypeDescription *)pMemberType)->pMemberName;
    pInvokArgs[1] = &pInParamsSeq;
    pInvokArgs[2] = &pOutIndices;
    pInvokArgs[3] = &pOutParams;
    uno_Any aInvokExc;
    uno_Any * pInvokExc = &aInvokExc;

    // invoke() call
    (*m_pReceiver->pDispatcher)(
        m_pReceiver, m_pFactory->m_pInvokMethodTD,
        &aInvokRet, pInvokArgs, &pInvokExc );

    if (pInvokExc)
    {
        handleInvokExc( *ppException, pInvokExc );
        ::uno_any_destruct( pInvokExc, 0 ); // cleanup
    }
    else // no invocation exception
    {
        // write changed out params
        OSL_ENSURE(
            pOutParams->nElements == nOutParams &&
            pOutIndices->nElements == nOutParams,
            "### out params lens differ!" );
        if (pOutParams->nElements == nOutParams &&
            pOutIndices->nElements == nOutParams)
        {
            sal_Int16 * pIndices = (sal_Int16 *)pOutIndices->elements;
            uno_Any * pOut       = (uno_Any *)pOutParams->elements;
            for ( nPos = 0; nPos < nOutParams; ++nPos )
            {
                sal_Int32 nIndex = pIndices[nPos];
                OSL_ENSURE( nIndex < nParams, "### illegal index!" );
                typelib_MethodParameter const & rParam = pFormalParams[nIndex];
                bool succ;
                if (rParam.bIn) // is in/inout param
                {
                    succ = coerce_assign(
                        pArgs[nIndex], rParam.pTypeRef, &pOut[nPos],
                        *ppException );
                }
                else // pure out
                {
                    succ = coerce_construct(
                        pArgs[nIndex], rParam.pTypeRef, &pOut[nPos],
                        *ppException );
                }
                if (! succ) // cleanup of out params
                {
                    for ( sal_Int32 n = 0; n <= nPos; ++n )
                    {
                        sal_Int32 nIndex2 = pIndices[n];
                        OSL_ENSURE( nIndex2 < nParams, "### illegal index!" );
                        typelib_MethodParameter const & rParam2 =
                            pFormalParams[nIndex2];
                        if (! rParam2.bIn) // is pure out param
                        {
                            ::uno_type_destructData(
                                pArgs[nIndex2], rParam2.pTypeRef, 0 );
                        }
                    }
                }
            }
            if (nPos == pOutIndices->nElements)
            {
                // out param copy ok; write return value
                if (coerce_construct(
                        pReturn,
                        ((typelib_InterfaceMethodTypeDescription *)
                         pMemberType)->pReturnTypeRef,
                        &aInvokRet, *ppException ))
                {
                    *ppException = 0; // no exception
                }
            }
        }
        else
        {
            // set runtime exception
            constructRuntimeException(
                *ppException,
                "out params lengths differ after invocation call!" );
        }
        // cleanup invok out params
        ::uno_destructData( &pOutIndices, m_pFactory->m_pShortSeqTD, 0 );
        ::uno_destructData( &pOutParams, m_pFactory->m_pAnySeqTD, 0 );
        // cleanup invok return value
        ::uno_any_destruct( &aInvokRet, 0 );
    }
    // cleanup constructed in params
    ::uno_destructData( &pInParamsSeq, m_pFactory->m_pAnySeqTD, 0 );
}

extern "C"
{
//______________________________________________________________________________
static void SAL_CALL adapter_acquire( uno_Interface * pUnoI )
{
    static_cast< InterfaceAdapterImpl * >( pUnoI )->m_pAdapter->acquire();
}
//______________________________________________________________________________
static void SAL_CALL adapter_release( uno_Interface * pUnoI )
{
    static_cast< InterfaceAdapterImpl * >( pUnoI )->m_pAdapter->release();
}
//______________________________________________________________________________
static void SAL_CALL adapter_dispatch(
    uno_Interface * pUnoI, const typelib_TypeDescription * pMemberType,
    void * pReturn, void * pArgs[], uno_Any ** ppException )
{
    // query to emulated interface
    switch (((typelib_InterfaceMemberTypeDescription *)pMemberType)->nPosition)
    {
    case 0: // queryInterface()
    {
        AdapterImpl * that =
            static_cast< InterfaceAdapterImpl * >( pUnoI )->m_pAdapter;
        *ppException = 0; // no exc
        typelib_TypeDescriptionReference * pDemanded =
            *(typelib_TypeDescriptionReference **)pArgs[0];
        // pInterfaces[0] is XInterface
        for ( sal_Int32 nPos = 0; nPos < that->m_nInterfaces; ++nPos )
        {
            typelib_InterfaceTypeDescription * pTD =
                that->m_pInterfaces[nPos].m_pTypeDescr;
            while (pTD)
            {
                if (type_equals(
                        ((typelib_TypeDescription *)pTD)->pWeakRef, pDemanded ))
                {
                    uno_Interface * pUnoI2 = &that->m_pInterfaces[nPos];
                    ::uno_any_construct(
                        (uno_Any *)pReturn, &pUnoI2,
                        (typelib_TypeDescription *)pTD, 0 );
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
        AdapterImpl * that =
            static_cast< InterfaceAdapterImpl * >( pUnoI )->m_pAdapter;
        if (pMemberType->eTypeClass == typelib_TypeClass_INTERFACE_METHOD)
        {
            that->invoke( pMemberType, pReturn, pArgs, ppException );
        }
        else // attribute
        {
            if (pReturn)
                that->getValue( pMemberType, pReturn, ppException );
            else
                that->setValue( pMemberType, pArgs, ppException );
        }
    }
    }
}
}
//______________________________________________________________________________
AdapterImpl::AdapterImpl(
    void * key, Reference< script::XInvocation > const & xReceiver,
    const Sequence< Type > & rTypes,
    FactoryImpl * pFactory )
    SAL_THROW( (RuntimeException) )
        : m_nRef( 1 ),
          m_pFactory( pFactory ),
          m_key( key )
{
    // init adapters
    m_nInterfaces = rTypes.getLength();
    m_pInterfaces = new InterfaceAdapterImpl[ rTypes.getLength() ];
    const Type * pTypes = rTypes.getConstArray();
    for ( sal_Int32 nPos = rTypes.getLength(); nPos--; )
    {
        InterfaceAdapterImpl * pInterface = &m_pInterfaces[nPos];
        pInterface->acquire = adapter_acquire;
        pInterface->release = adapter_release;
        pInterface->pDispatcher = adapter_dispatch;
        pInterface->m_pAdapter = this;
        pInterface->m_pTypeDescr = 0;
        pTypes[nPos].getDescription(
            (typelib_TypeDescription **)&pInterface->m_pTypeDescr );
        OSL_ASSERT( pInterface->m_pTypeDescr );
        if (! pInterface->m_pTypeDescr)
        {
            for ( sal_Int32 n = 0; n < nPos; ++n )
            {
                ::typelib_typedescription_release(
                    (typelib_TypeDescription *)
                    m_pInterfaces[ n ].m_pTypeDescr );
            }
            delete [] m_pInterfaces;
            throw RuntimeException(
                "cannot retrieve all interface type infos!",
                Reference< XInterface >() );
        }
    }

    // map receiver
    m_pReceiver = (uno_Interface *)m_pFactory->m_aCpp2Uno.mapInterface(
        xReceiver.get(), ::getCppuType( &xReceiver ) );
    OSL_ASSERT( 0 != m_pReceiver );
    if (! m_pReceiver)
    {
        throw RuntimeException(
            "cannot map receiver!", Reference< XInterface >() );
    }

    m_pFactory->acquire();
}

//______________________________________________________________________________
FactoryImpl::FactoryImpl( Reference< XComponentContext > const & xContext )
    SAL_THROW( (RuntimeException) )
    : m_pInvokMethodTD( 0 ),
      m_pSetValueTD( 0 ),
      m_pGetValueTD( 0 ),
      m_pAnySeqTD( 0 ),
      m_pShortSeqTD( 0 ),
      m_pConvertToTD( 0 )
{
    // C++/UNO bridge
    OUString aCppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
    OUString aUnoEnvTypeName = UNO_LB_UNO;
    m_aUno2Cpp = Mapping( aUnoEnvTypeName, aCppEnvTypeName );
    m_aCpp2Uno = Mapping( aCppEnvTypeName, aUnoEnvTypeName );
    OSL_ENSURE(
        m_aUno2Cpp.is() && m_aCpp2Uno.is(), "### no uno / C++ mappings!" );

    // type converter
    Reference< script::XTypeConverter > xConverter(
        xContext->getServiceManager()->createInstanceWithContext(
            OUString("com.sun.star.script.Converter"),
            xContext ),
        UNO_QUERY_THROW );
    m_pConverter = (uno_Interface *)m_aCpp2Uno.mapInterface(
        xConverter.get(), ::getCppuType( &xConverter ) );
    OSL_ASSERT( 0 != m_pConverter );

    // some type info:
    // sequence< any >
    Type const & rAnySeqType = ::getCppuType( (const Sequence< Any > *)0 );
    rAnySeqType.getDescription( &m_pAnySeqTD );
    // sequence< short >
    const Type & rShortSeqType =
        ::getCppuType( (const Sequence< sal_Int16 > *)0 );
    rShortSeqType.getDescription( &m_pShortSeqTD );
    // script.XInvocation
    typelib_TypeDescription * pTD = 0;
    const Type & rInvType = ::getCppuType(
        (const Reference< script::XInvocation > *)0 );
    TYPELIB_DANGER_GET( &pTD, rInvType.getTypeLibType() );
    typelib_InterfaceTypeDescription * pITD;
    pITD = reinterpret_cast<typelib_InterfaceTypeDescription*>(pTD);
    if( ! pITD->aBase.bComplete )
        typelib_typedescription_complete( &pTD );
    ::typelib_typedescriptionreference_getDescription(
        &m_pInvokMethodTD, pITD->ppMembers[ 1 ] ); // invoke()
    ::typelib_typedescriptionreference_getDescription(
        &m_pSetValueTD, pITD->ppMembers[ 2 ] ); // setValue()
    ::typelib_typedescriptionreference_getDescription(
        &m_pGetValueTD, pITD->ppMembers[ 3 ] ); // getValue()
    // script.XTypeConverter
    const Type & rTCType =
        ::getCppuType( (const Reference< script::XTypeConverter > *)0 );
    TYPELIB_DANGER_GET( &pTD, rTCType.getTypeLibType() );
    pITD = reinterpret_cast<typelib_InterfaceTypeDescription*>(pTD);
    ::typelib_typedescriptionreference_getDescription(
        &m_pConvertToTD, pITD->ppMembers[ 0 ] ); // convertTo()
    TYPELIB_DANGER_RELEASE( pTD );

    if (!m_pInvokMethodTD || !m_pSetValueTD || !m_pGetValueTD ||
        !m_pConvertToTD ||
        !m_pAnySeqTD || !m_pShortSeqTD)
    {
        throw RuntimeException(
            "missing type descriptions!", Reference< XInterface >() );
    }
}
//______________________________________________________________________________
FactoryImpl::~FactoryImpl() SAL_THROW(())
{
    ::typelib_typedescription_release( m_pInvokMethodTD );
    ::typelib_typedescription_release( m_pSetValueTD );
    ::typelib_typedescription_release( m_pGetValueTD );
    ::typelib_typedescription_release( m_pAnySeqTD );
    ::typelib_typedescription_release( m_pShortSeqTD );
    ::typelib_typedescription_release( m_pConvertToTD );

    (*m_pConverter->release)( m_pConverter );

#if OSL_DEBUG_LEVEL > 1
    OSL_ENSURE( m_receiver2adapters.empty(), "### still adapters out there!?" );
#endif
}

//------------------------------------------------------------------------------
static inline AdapterImpl * lookup_adapter(
    t_ptr_set ** pp_adapter_set,
    t_ptr_map & map, void * key, Sequence< Type > const & rTypes )
    SAL_THROW(())
{
    t_ptr_set & adapters_set = map[ key ];
    *pp_adapter_set = &adapters_set;
    if (adapters_set.empty())
        return 0; // shortcut
    // find matching adapter
    Type const * pTypes = rTypes.getConstArray();
    sal_Int32 nTypes = rTypes.getLength();
    t_ptr_set::const_iterator iPos( adapters_set.begin() );
    t_ptr_set::const_iterator const iEnd( adapters_set.end() );
    while (iEnd != iPos)
    {
        AdapterImpl * that = reinterpret_cast< AdapterImpl * >( *iPos );
        // iterate thru all types if that is a matching adapter
        sal_Int32 nPosTypes;
        for ( nPosTypes = nTypes; nPosTypes--; )
        {
            Type const & rType = pTypes[ nPosTypes ];
            // find in adapter's type list
            sal_Int32 nPos;
            for ( nPos = that->m_nInterfaces; nPos--; )
            {
                if (::typelib_typedescriptionreference_isAssignableFrom(
                        rType.getTypeLibType(),
                        ((typelib_TypeDescription *)that->
                         m_pInterfaces[ nPos ].m_pTypeDescr)->pWeakRef ))
                {
                    // found
                    break;
                }
            }
            if (nPos < 0) // type not found => next adapter
                break;
        }
        if (nPosTypes < 0) // all types found
            return that;
        ++iPos;
    }
    return 0;
}

// XInvocationAdapterFactory2 impl
//______________________________________________________________________________
Reference< XInterface > FactoryImpl::createAdapter(
    const Reference< script::XInvocation > & xReceiver,
    const Sequence< Type > & rTypes )
    throw (RuntimeException)
{
    Reference< XInterface > xRet;
    if (xReceiver.is() && rTypes.getLength())
    {
        t_ptr_set * adapter_set;
        AdapterImpl * that;
        Reference< XInterface > xKey( xReceiver, UNO_QUERY );
        {
        ClearableMutexGuard guard( m_mutex );
        that = lookup_adapter(
            &adapter_set, m_receiver2adapters, xKey.get(), rTypes );
        if (0 == that) // no entry
        {
            guard.clear();
            // create adapter; already acquired: m_nRef == 1
            AdapterImpl * pNew =
                new AdapterImpl( xKey.get(), xReceiver, rTypes, this );
            // lookup again
            ClearableMutexGuard guard2( m_mutex );
            that = lookup_adapter(
                &adapter_set, m_receiver2adapters, xKey.get(), rTypes );
            if (0 == that) // again no entry
            {
                pair< t_ptr_set::iterator, bool > i(adapter_set->insert(pNew));
                SAL_WARN_IF(
                    !i.second, "stoc",
                    "set already contains " << *(i.first) << " != " << pNew);
                that = pNew;
            }
            else
            {
                that->acquire();
                guard2.clear();
                delete pNew; // has never been inserted
            }
        }
        else // found adapter
        {
            that->acquire();
        }
        }
        // map one interface to C++
        uno_Interface * pUnoI = &that->m_pInterfaces[ 0 ];
        m_aUno2Cpp.mapInterface(
            (void **)&xRet, pUnoI, ::getCppuType( &xRet ) );
        that->release();
        OSL_ASSERT( xRet.is() );
        if (! xRet.is())
        {
            throw RuntimeException(
                "mapping UNO to C++ failed!",
                Reference< XInterface >() );
        }
    }
    return xRet;
}
// XInvocationAdapterFactory impl
//______________________________________________________________________________
Reference< XInterface > FactoryImpl::createAdapter(
    const Reference< script::XInvocation > & xReceiver, const Type & rType )
    throw (RuntimeException)
{
    return createAdapter( xReceiver, Sequence< Type >( &rType, 1 ) );
}

// XServiceInfo
//______________________________________________________________________________
OUString FactoryImpl::getImplementationName()
    throw (RuntimeException)
{
    return invadp_getImplementationName();
}
//______________________________________________________________________________
sal_Bool FactoryImpl::supportsService( const OUString & rServiceName )
    throw (RuntimeException)
{
    const Sequence< OUString > & rSNL = getSupportedServiceNames();
    const OUString * pArray = rSNL.getConstArray();
    for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
    {
        if (pArray[nPos].equals( rServiceName ))
            return sal_True;
    }
    return sal_False;
}
//______________________________________________________________________________
Sequence< OUString > FactoryImpl::getSupportedServiceNames()
    throw (RuntimeException)
{
    return invadp_getSupportedServiceNames();
}

//==============================================================================
static Reference< XInterface > SAL_CALL FactoryImpl_create(
    const Reference< XComponentContext > & xContext )
    throw (Exception)
{
    return (::cppu::OWeakObject *)new FactoryImpl( xContext );
}

}


//##############################################################################
//##############################################################################
//##############################################################################

static struct ::cppu::ImplementationEntry g_entries[] =
{
    {
        ::stoc_invadp::FactoryImpl_create,
        ::stoc_invadp::invadp_getImplementationName,
        ::stoc_invadp::invadp_getSupportedServiceNames,
        ::cppu::createOneInstanceComponentFactory,
        0, 0
    },
    { 0, 0, 0, 0, 0, 0 }
};

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL invocadapt_component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return ::cppu::component_getFactoryHelper(
        pImplName, pServiceManager, pRegistryKey , g_entries );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
