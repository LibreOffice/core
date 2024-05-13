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


#include <osl/diagnose.h>
#include <osl/interlck.h>
#include <osl/mutex.hxx>
#include <o3tl/sorted_vector.hxx>
#include <sal/log.hxx>

#include <uno/dispatcher.h>
#include <uno/data.h>
#include <uno/any2.h>
#include <uno/lbnames.h>
#include <uno/mapping.hxx>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/script/XTypeConverter.hpp>
#include <com/sun/star/script/XInvocationAdapterFactory.hpp>
#include <com/sun/star/script/XInvocationAdapterFactory2.hpp>
#include <com/sun/star/script/XInvocation.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/reflection/InvocationTargetException.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <unordered_map>
#include <vector>

using namespace ::osl;
using namespace ::com::sun::star;
using namespace css::uno;

namespace stoc_invadp
{


namespace {

struct hash_ptr
{
    size_t operator() ( void * p ) const
        { return reinterpret_cast<size_t>(p); }
};

}

typedef o3tl::sorted_vector< void * > t_ptr_set;
typedef std::unordered_map< void *, t_ptr_set, hash_ptr > t_ptr_map;

namespace {

class FactoryImpl
    : public ::cppu::WeakImplHelper< lang::XServiceInfo,
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

    explicit FactoryImpl( Reference< XComponentContext > const & xContext );
    virtual ~FactoryImpl() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString & rServiceName ) override;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XInvocationAdapterFactory
    virtual Reference< XInterface > SAL_CALL createAdapter(
        const Reference< script::XInvocation > & xReceiver, const Type & rType ) override;
    // XInvocationAdapterFactory2
    virtual Reference< XInterface > SAL_CALL createAdapter(
        const Reference< script::XInvocation > & xReceiver,
        const Sequence< Type > & rTypes ) override;
};
struct AdapterImpl;

struct InterfaceAdapterImpl : public uno_Interface
{
    AdapterImpl *                           m_pAdapter;
    typelib_InterfaceTypeDescription *      m_pTypeDescr;
};

struct AdapterImpl
{
    oslInterlockedCount         m_nRef;
    FactoryImpl *               m_pFactory;
    void *                      m_key; // map key
    uno_Interface *             m_pReceiver; // XInvocation receiver

    std::vector<InterfaceAdapterImpl>  m_vInterfaces;

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

    inline void acquire();
    inline void release();
    inline ~AdapterImpl();
    inline AdapterImpl(
        void * key, Reference< script::XInvocation > const & xReceiver,
        const Sequence< Type > & rTypes,
        FactoryImpl * pFactory );

    // Copy assignment is forbidden and not implemented.
    AdapterImpl (const AdapterImpl &) = delete;
    AdapterImpl & operator= (const AdapterImpl &) = delete;
};

}

inline AdapterImpl::~AdapterImpl()
{
    for ( size_t nPos = m_vInterfaces.size(); nPos--; )
    {
        ::typelib_typedescription_release(
            &m_vInterfaces[ nPos ].m_pTypeDescr->aBase );
    }

    (*m_pReceiver->release)( m_pReceiver );
    m_pFactory->release();
}

inline void AdapterImpl::acquire()
{
    osl_atomic_increment( &m_nRef );
}

inline void AdapterImpl::release()
{
    bool delete_this = false;
    {
    MutexGuard guard( m_pFactory->m_mutex );
    if (! osl_atomic_decrement( &m_nRef ))
    {
        t_ptr_map::iterator iFind(
            m_pFactory->m_receiver2adapters.find( m_key ) );
        assert( m_pFactory->m_receiver2adapters.end() != iFind );
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


static void constructRuntimeException(
    uno_Any * pExc, const OUString & rMsg )
{
    RuntimeException exc( rMsg );
    // no conversion needed due to binary compatibility + no convertible type
    ::uno_type_any_construct(
        pExc, &exc, cppu::UnoType<decltype(exc)>::get().getTypeLibType(), nullptr );
}


static bool type_equals(
    typelib_TypeDescriptionReference * pType1,
    typelib_TypeDescriptionReference * pType2 )
{
    return (pType1 == pType2 ||
            (pType1->pTypeName->length == pType2->pTypeName->length &&
             0 == ::rtl_ustr_compare(
                 pType1->pTypeName->buffer, pType2->pTypeName->buffer )));
}


bool AdapterImpl::coerce_assign(
    void * pDest, typelib_TypeDescriptionReference * pType, uno_Any * pSource,
    uno_Any * pOutExc )
{
    if (typelib_TypeClass_ANY == pType->eTypeClass)
    {
        ::uno_type_any_assign(
            static_cast<uno_Any *>(pDest), pSource->pData, pSource->pType, nullptr, nullptr );
        return true;
    }
    if (::uno_type_assignData(
            pDest, pType, pSource->pData, pSource->pType, nullptr, nullptr, nullptr ))
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
            if (typelib_typedescriptionreference_isAssignableFrom( cppu::UnoType<RuntimeException>::get().getTypeLibType(),
                    p_exc->pType ))
            {
                // is RuntimeException or derived: rethrow
                uno_type_any_construct(
                    pOutExc, p_exc->pData, p_exc->pType, nullptr );
            }
            else
            {
                // set runtime exception
                constructRuntimeException(
                    pOutExc, "type coercion failed: " +
                    static_cast< Exception const * >(
                        p_exc->pData )->Message );
            }
            ::uno_any_destruct( p_exc, nullptr );
            // pOutExc constructed
            return false;
        }
        else
        {
            bool succ = ::uno_type_assignData(
                             pDest, pType, ret.pData, ret.pType, nullptr, nullptr, nullptr );
            ::uno_any_destruct( &ret, nullptr );
            OSL_ENSURE(
                succ, "### conversion succeeded, but assignment failed!?" );
            if (! succ)
            {
                // set runtime exception
                constructRuntimeException(
                    pOutExc,
                    u"type coercion failed: "
                    "conversion succeeded, but assignment failed?!"_ustr );
            }
            return succ;
        }
    }
}

inline bool AdapterImpl::coerce_construct(
    void * pDest, typelib_TypeDescriptionReference * pType, uno_Any * pSource,
    uno_Any * pExc )
{
    if (typelib_TypeClass_ANY == pType->eTypeClass)
    {
        ::uno_type_copyData( pDest, pSource, pType, nullptr );
        return true;
    }
    if (type_equals( pType, pSource->pType))
    {
        ::uno_type_copyData( pDest, pSource->pData, pType, nullptr );
        return true;
    }
    ::uno_type_constructData( pDest, pType );
    return coerce_assign( pDest, pType, pSource, pExc );
}


static void handleInvokExc( uno_Any * pDest, uno_Any * pSource )
{
    OUString const & name =
        OUString::unacquired( &pSource->pType->pTypeName );

    if ( name == "com.sun.star.reflection.InvocationTargetException" )
    {
        // unwrap invocation target exception
        uno_Any * target_exc =
            &static_cast< reflection::InvocationTargetException * >(
                pSource->pData )->TargetException;
        ::uno_type_any_construct(
            pDest, target_exc->pData, target_exc->pType, nullptr );
    }
    else // all other exceptions are wrapped to RuntimeException
    {
        if (typelib_TypeClass_EXCEPTION == pSource->pType->eTypeClass)
        {
            constructRuntimeException(
                pDest, static_cast<Exception const *>(pSource->pData)->Message );
        }
        else
        {
            constructRuntimeException(
                pDest, u"no exception has been thrown via invocation?!"_ustr );
        }
    }
}

void AdapterImpl::getValue(
    const typelib_TypeDescription * pMemberType,
    void * pReturn, uno_Any ** ppException )
{
    uno_Any aInvokRet;
    void * pInvokArgs[1];
    pInvokArgs[0] = const_cast<rtl_uString **>(
        &reinterpret_cast<typelib_InterfaceMemberTypeDescription const *>(pMemberType)->pMemberName);
    uno_Any aInvokExc;
    uno_Any * pInvokExc = &aInvokExc;

    // getValue()
    (*m_pReceiver->pDispatcher)(
        m_pReceiver, m_pFactory->m_pGetValueTD,
        &aInvokRet, pInvokArgs, &pInvokExc );

    if (pInvokExc) // getValue() call exception
    {
        handleInvokExc( *ppException, pInvokExc );
        ::uno_any_destruct( pInvokExc, nullptr ); // cleanup
    }
    else // invocation call succeeded
    {
        if (coerce_construct(
                pReturn,
                reinterpret_cast<typelib_InterfaceAttributeTypeDescription const *>(
                 pMemberType)->pAttributeTypeRef,
                &aInvokRet, *ppException ))
        {
            *ppException = nullptr; // no exceptions be thrown
        }
        ::uno_any_destruct( &aInvokRet, nullptr );
    }
}

void AdapterImpl::setValue(
    const typelib_TypeDescription * pMemberType,
    void * pArgs[], uno_Any ** ppException )
{
    uno_Any aInvokVal;
    ::uno_type_any_construct(
        &aInvokVal, pArgs[0],
        reinterpret_cast<typelib_InterfaceAttributeTypeDescription const *>(
         pMemberType)->pAttributeTypeRef, nullptr );

    void * pInvokArgs[2];
    pInvokArgs[0] = const_cast<rtl_uString **>(
        &reinterpret_cast<typelib_InterfaceMemberTypeDescription const *>(pMemberType)->pMemberName);
    pInvokArgs[1] = &aInvokVal;
    uno_Any aInvokExc;
    uno_Any * pInvokExc = &aInvokExc;

    // setValue()
    (*m_pReceiver->pDispatcher)(
        m_pReceiver, m_pFactory->m_pSetValueTD, nullptr, pInvokArgs, &pInvokExc );

    if (pInvokExc) // setValue() call exception
    {
        handleInvokExc( *ppException, pInvokExc );
        ::uno_any_destruct( pInvokExc, nullptr ); // cleanup
    }
    else // invocation call succeeded
    {
        *ppException = nullptr; // no exceptions be thrown
    }

    ::uno_any_destruct( &aInvokVal, nullptr ); // cleanup
}

void AdapterImpl::invoke(
    const typelib_TypeDescription * pMemberType,
    void * pReturn, void * pArgs[], uno_Any ** ppException )
{
    sal_Int32 nParams =
        reinterpret_cast<typelib_InterfaceMethodTypeDescription const *>(pMemberType)->nParams;
    typelib_MethodParameter * pFormalParams =
        reinterpret_cast<typelib_InterfaceMethodTypeDescription const *>(pMemberType)->pParams;

    // in params
    uno_Sequence * pInParamsSeq = nullptr;
    ::uno_sequence_construct(
        &pInParamsSeq, m_pFactory->m_pAnySeqTD, nullptr, nParams, nullptr );
    uno_Any * pInAnys = reinterpret_cast<uno_Any *>(pInParamsSeq->elements);
    sal_Int32 nOutParams = 0;
    sal_Int32 nPos;
    for ( nPos = nParams; nPos--; )
    {
        typelib_MethodParameter const & rParam = pFormalParams[nPos];
        if (rParam.bIn) // is in/inout param
        {
            ::uno_type_any_assign(
                &pInAnys[nPos], pArgs[nPos], rParam.pTypeRef, nullptr, nullptr );
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
    pInvokArgs[0] = const_cast<rtl_uString **>(
        &reinterpret_cast<typelib_InterfaceMemberTypeDescription const *>(pMemberType)->pMemberName);
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
        ::uno_any_destruct( pInvokExc, nullptr ); // cleanup
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
            sal_Int16 * pIndices = reinterpret_cast<sal_Int16 *>(pOutIndices->elements);
            uno_Any * pOut       = reinterpret_cast<uno_Any *>(pOutParams->elements);
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
                                pArgs[nIndex2], rParam2.pTypeRef, nullptr );
                        }
                    }
                }
            }
            if (nPos == pOutIndices->nElements)
            {
                // out param copy ok; write return value
                if (coerce_construct(
                        pReturn,
                        reinterpret_cast<typelib_InterfaceMethodTypeDescription const *>(
                         pMemberType)->pReturnTypeRef,
                        &aInvokRet, *ppException ))
                {
                    *ppException = nullptr; // no exception
                }
            }
        }
        else
        {
            // set runtime exception
            constructRuntimeException(
                *ppException,
                u"out params lengths differ after invocation call!"_ustr );
        }
        // cleanup invok out params
        ::uno_destructData( &pOutIndices, m_pFactory->m_pShortSeqTD, nullptr );
        ::uno_destructData( &pOutParams, m_pFactory->m_pAnySeqTD, nullptr );
        // cleanup invok return value
        ::uno_any_destruct( &aInvokRet, nullptr );
    }
    // cleanup constructed in params
    ::uno_destructData( &pInParamsSeq, m_pFactory->m_pAnySeqTD, nullptr );
}

extern "C"
{

static void adapter_acquire( uno_Interface * pUnoI )
{
    static_cast< InterfaceAdapterImpl * >( pUnoI )->m_pAdapter->acquire();
}

static void adapter_release( uno_Interface * pUnoI )
{
    static_cast< InterfaceAdapterImpl * >( pUnoI )->m_pAdapter->release();
}

static void adapter_dispatch(
    uno_Interface * pUnoI, const typelib_TypeDescription * pMemberType,
    void * pReturn, void * pArgs[], uno_Any ** ppException )
{
    // query to emulated interface
    switch (reinterpret_cast<typelib_InterfaceMemberTypeDescription const *>(pMemberType)->nPosition)
    {
    case 0: // queryInterface()
    {
        AdapterImpl * that =
            static_cast< InterfaceAdapterImpl * >( pUnoI )->m_pAdapter;
        *ppException = nullptr; // no exc
        typelib_TypeDescriptionReference * pDemanded =
            *static_cast<typelib_TypeDescriptionReference **>(pArgs[0]);
        // pInterfaces[0] is XInterface
        for ( size_t nPos = 0; nPos < that->m_vInterfaces.size(); ++nPos )
        {
            typelib_InterfaceTypeDescription * pTD =
                that->m_vInterfaces[nPos].m_pTypeDescr;
            while (pTD)
            {
                if (type_equals( pTD->aBase.pWeakRef, pDemanded ))
                {
                    uno_Interface * pUnoI2 = &that->m_vInterfaces[nPos];
                    ::uno_any_construct(
                        static_cast<uno_Any *>(pReturn), &pUnoI2,
                        &pTD->aBase, nullptr );
                    return;
                }
                pTD = pTD->pBaseTypeDescription;
            }
        }
        ::uno_any_construct( static_cast<uno_Any *>(pReturn), nullptr, nullptr, nullptr ); // clear()
        break;
    }
    case 1: // acquire()
        *ppException = nullptr; // no exc
        adapter_acquire( pUnoI );
        break;
    case 2: // release()
        *ppException = nullptr; // no exc
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

AdapterImpl::AdapterImpl(
    void * key, Reference< script::XInvocation > const & xReceiver,
    const Sequence< Type > & rTypes,
    FactoryImpl * pFactory )
        : m_nRef( 1 ),
          m_pFactory( pFactory ),
          m_key( key ),
          m_vInterfaces( rTypes.getLength() )
{
    // init adapters
    const Type * pTypes = rTypes.getConstArray();
    for ( sal_Int32 nPos = rTypes.getLength(); nPos--; )
    {
        InterfaceAdapterImpl * pInterface = &m_vInterfaces[nPos];
        pInterface->acquire = adapter_acquire;
        pInterface->release = adapter_release;
        pInterface->pDispatcher = adapter_dispatch;
        pInterface->m_pAdapter = this;
        pInterface->m_pTypeDescr = nullptr;
        pTypes[nPos].getDescription(
            reinterpret_cast<typelib_TypeDescription **>(&pInterface->m_pTypeDescr) );
        OSL_ASSERT( pInterface->m_pTypeDescr );
        if (! pInterface->m_pTypeDescr)
        {
            for ( sal_Int32 n = 0; n < nPos; ++n )
            {
                ::typelib_typedescription_release(
                    &m_vInterfaces[ n ].m_pTypeDescr->aBase );
            }
            throw RuntimeException(
                u"cannot retrieve all interface type infos!"_ustr );
        }
    }

    // map receiver
    m_pReceiver = static_cast<uno_Interface *>(m_pFactory->m_aCpp2Uno.mapInterface(
        xReceiver.get(), cppu::UnoType<decltype(xReceiver)>::get() ));
    OSL_ASSERT( nullptr != m_pReceiver );
    if (! m_pReceiver)
    {
        throw RuntimeException( u"cannot map receiver!"_ustr );
    }

    m_pFactory->acquire();
}


FactoryImpl::FactoryImpl( Reference< XComponentContext > const & xContext )
    : m_aUno2Cpp(Mapping( u"" UNO_LB_UNO ""_ustr, CPPU_CURRENT_LANGUAGE_BINDING_NAME )),
      m_aCpp2Uno(Mapping( CPPU_CURRENT_LANGUAGE_BINDING_NAME, u"" UNO_LB_UNO ""_ustr)),
      m_pInvokMethodTD( nullptr ),
      m_pSetValueTD( nullptr ),
      m_pGetValueTD( nullptr ),
      m_pAnySeqTD( nullptr ),
      m_pShortSeqTD( nullptr ),
      m_pConvertToTD( nullptr )
{
    // C++/UNO bridge
    OSL_ENSURE(
        m_aUno2Cpp.is() && m_aCpp2Uno.is(), "### no uno / C++ mappings!" );

    // type converter
    Reference< script::XTypeConverter > xConverter(
        xContext->getServiceManager()->createInstanceWithContext(
            u"com.sun.star.script.Converter"_ustr,
            xContext ),
        UNO_QUERY_THROW );
    m_pConverter = static_cast<uno_Interface *>(m_aCpp2Uno.mapInterface(
        xConverter.get(), cppu::UnoType<decltype(xConverter)>::get() ));
    OSL_ASSERT( nullptr != m_pConverter );

    // some type info:
    // sequence< any >
    Type const & rAnySeqType = cppu::UnoType<Sequence< Any >>::get();
    rAnySeqType.getDescription( &m_pAnySeqTD );
    // sequence< short >
    const Type & rShortSeqType =
        cppu::UnoType<Sequence< sal_Int16 >>::get();
    rShortSeqType.getDescription( &m_pShortSeqTD );
    // script.XInvocation
    typelib_TypeDescription * pTD = nullptr;
    const Type & rInvType = cppu::UnoType<script::XInvocation>::get();
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
        cppu::UnoType<script::XTypeConverter>::get();
    TYPELIB_DANGER_GET( &pTD, rTCType.getTypeLibType() );
    pITD = reinterpret_cast<typelib_InterfaceTypeDescription*>(pTD);
    ::typelib_typedescriptionreference_getDescription(
        &m_pConvertToTD, pITD->ppMembers[ 0 ] ); // convertTo()
    TYPELIB_DANGER_RELEASE( pTD );

    if (!m_pInvokMethodTD || !m_pSetValueTD || !m_pGetValueTD ||
        !m_pConvertToTD ||
        !m_pAnySeqTD || !m_pShortSeqTD)
    {
        throw RuntimeException( u"missing type descriptions!"_ustr );
    }
}

FactoryImpl::~FactoryImpl()
{
    ::typelib_typedescription_release( m_pInvokMethodTD );
    ::typelib_typedescription_release( m_pSetValueTD );
    ::typelib_typedescription_release( m_pGetValueTD );
    ::typelib_typedescription_release( m_pAnySeqTD );
    ::typelib_typedescription_release( m_pShortSeqTD );
    ::typelib_typedescription_release( m_pConvertToTD );

    (*m_pConverter->release)( m_pConverter );

#if OSL_DEBUG_LEVEL > 0
    assert(m_receiver2adapters.empty() && "still adapters out there!?");
#endif
}


static AdapterImpl * lookup_adapter(
    t_ptr_set ** pp_adapter_set,
    t_ptr_map & map, void * key, Sequence< Type > const & rTypes )
{
    t_ptr_set & adapters_set = map[ key ];
    *pp_adapter_set = &adapters_set;
    if (adapters_set.empty())
        return nullptr; // shortcut
    // find matching adapter
    Type const * pTypes = rTypes.getConstArray();
    sal_Int32 nTypes = rTypes.getLength();
    for (const auto& rpAdapter : adapters_set)
    {
        AdapterImpl * that = static_cast< AdapterImpl * >( rpAdapter );
        // iterate through all types if that is a matching adapter
        sal_Int32 nPosTypes;
        for ( nPosTypes = nTypes; nPosTypes--; )
        {
            Type const & rType = pTypes[ nPosTypes ];
            // find in adapter's type list
            sal_Int32 nPos;
            for ( nPos = that->m_vInterfaces.size(); nPos--; )
            {
                if (::typelib_typedescriptionreference_isAssignableFrom(
                        rType.getTypeLibType(),
                        that->m_vInterfaces[ nPos ].m_pTypeDescr->aBase.pWeakRef ))
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
    }
    return nullptr;
}

// XInvocationAdapterFactory2 impl

Reference< XInterface > FactoryImpl::createAdapter(
    const Reference< script::XInvocation > & xReceiver,
    const Sequence< Type > & rTypes )
{
    Reference< XInterface > xRet;
    if (xReceiver.is() && rTypes.hasElements())
    {
        t_ptr_set * adapter_set;
        AdapterImpl * that;
        Reference< XInterface > xKey( xReceiver, UNO_QUERY );
        {
        ClearableMutexGuard guard( m_mutex );
        that = lookup_adapter(
            &adapter_set, m_receiver2adapters, xKey.get(), rTypes );
        if (nullptr == that) // no entry
        {
            guard.clear();
            // create adapter; already acquired: m_nRef == 1
            AdapterImpl * pNew =
                new AdapterImpl( xKey.get(), xReceiver, rTypes, this );
            // lookup again
            ClearableMutexGuard guard2( m_mutex );
            that = lookup_adapter(
                &adapter_set, m_receiver2adapters, xKey.get(), rTypes );
            if (nullptr == that) // again no entry
            {
                std::pair< t_ptr_set::const_iterator, bool > i(adapter_set->insert(pNew));
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
        uno_Interface * pUnoI = that->m_vInterfaces.data();
        m_aUno2Cpp.mapInterface(
            reinterpret_cast<void **>(&xRet), pUnoI, cppu::UnoType<decltype(xRet)>::get() );
        that->release();
        OSL_ASSERT( xRet.is() );
        if (! xRet.is())
        {
            throw RuntimeException( u"mapping UNO to C++ failed!"_ustr );
        }
    }
    return xRet;
}
// XInvocationAdapterFactory impl

Reference< XInterface > FactoryImpl::createAdapter(
    const Reference< script::XInvocation > & xReceiver, const Type & rType )
{
    return createAdapter( xReceiver, Sequence< Type >( &rType, 1 ) );
}

// XServiceInfo

OUString FactoryImpl::getImplementationName()
{
    return u"com.sun.star.comp.stoc.InvocationAdapterFactory"_ustr;
}

sal_Bool FactoryImpl::supportsService( const OUString & rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > FactoryImpl::getSupportedServiceNames()
{
    return { u"com.sun.star.script.InvocationAdapterFactory"_ustr };
}

}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
stoc_invocation_adapter_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new stoc_invadp::FactoryImpl(context));
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
