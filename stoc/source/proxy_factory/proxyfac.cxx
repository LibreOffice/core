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
#include <osl/doublecheckedlocking.h>
#include <osl/mutex.hxx>
#include <rtl/ref.hxx>
#include <uno/dispatcher.hxx>
#include <uno/data.h>
#include <uno/lbnames.h>
#include <uno/mapping.hxx>
#include <uno/environment.hxx>
#include <typelib/typedescription.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weakagg.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/reflection/XProxyFactory.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>

#define SERVICE_NAME "com.sun.star.reflection.ProxyFactory"
#define IMPL_NAME "com.sun.star.comp.reflection.ProxyFactory"


using namespace ::com::sun::star;
using namespace css::uno;


namespace
{

OUString proxyfac_getImplementationName()
{
    return OUString(IMPL_NAME);
}

Sequence< OUString > proxyfac_getSupportedServiceNames()
{
    OUString str_name = SERVICE_NAME;
    return Sequence< OUString >( &str_name, 1 );
}


struct FactoryImpl : public ::cppu::WeakImplHelper< lang::XServiceInfo,
                                                     reflection::XProxyFactory >
{
    Environment m_uno_env;
    Environment m_cpp_env;
    Mapping m_uno2cpp;
    Mapping m_cpp2uno;

    UnoInterfaceReference binuno_queryInterface(
        UnoInterfaceReference const & unoI,
        typelib_InterfaceTypeDescription * pTypeDescr );

    FactoryImpl();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString & rServiceName ) override;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XProxyFactory
    virtual Reference< XAggregation > SAL_CALL createProxy(
        Reference< XInterface > const & xTarget ) override;
};


UnoInterfaceReference FactoryImpl::binuno_queryInterface(
    UnoInterfaceReference const & unoI,
    typelib_InterfaceTypeDescription * pTypeDescr )
{
    // init queryInterface() td
    static typelib_TypeDescription* s_pQITD = []() {
        typelib_TypeDescription* pTXInterfaceDescr = nullptr;
        TYPELIB_DANGER_GET(&pTXInterfaceDescr, cppu::UnoType<XInterface>::get().getTypeLibType());
        typelib_TypeDescription* pQITD = nullptr;
        typelib_typedescriptionreference_getDescription(
            &pQITD, reinterpret_cast<typelib_InterfaceTypeDescription*>(pTXInterfaceDescr)
                        ->ppAllMembers[0]);
        TYPELIB_DANGER_RELEASE(pTXInterfaceDescr);
        return pQITD;
    }();

    void * args[ 1 ];
    args[ 0 ] = &reinterpret_cast< typelib_TypeDescription * >(
        pTypeDescr )->pWeakRef;
    uno_Any ret_val, exc_space;
    uno_Any * exc = &exc_space;

    unoI.dispatch( s_pQITD, &ret_val, args, &exc );

    if (exc == nullptr)
    {
        UnoInterfaceReference ret;
        if (ret_val.pType->eTypeClass == typelib_TypeClass_INTERFACE)
        {
            ret.set( *static_cast< uno_Interface ** >(ret_val.pData),
                     SAL_NO_ACQUIRE );
            typelib_typedescriptionreference_release( ret_val.pType );
        }
        else
        {
            uno_any_destruct( &ret_val, nullptr );
        }
        return ret;
    }
    else
    {
        // exception occurred:
        OSL_ENSURE(
            typelib_typedescriptionreference_isAssignableFrom( cppu::UnoType<RuntimeException>::get().getTypeLibType(),
                exc->pType ),
            "### RuntimeException expected!" );
        Any cpp_exc;
        uno_type_copyAndConvertData(
            &cpp_exc, exc, cppu::UnoType<decltype(cpp_exc)>::get().getTypeLibType(),
            m_uno2cpp.get() );
        uno_any_destruct( exc, nullptr );
        ::cppu::throwException( cpp_exc );
        OSL_ASSERT( false ); // way of no return
        return UnoInterfaceReference(); // for dummy
    }
}


struct ProxyRoot : public ::cppu::OWeakAggObject
{
    // XAggregation
    virtual Any SAL_CALL queryAggregation( Type const & rType ) override;

    ProxyRoot( ::rtl::Reference< FactoryImpl > const & factory,
                      Reference< XInterface > const & xTarget );

    ::rtl::Reference< FactoryImpl > m_factory;

private:
    UnoInterfaceReference m_target;
};


struct binuno_Proxy : public uno_Interface
{
    oslInterlockedCount m_nRefCount;
    ::rtl::Reference< ProxyRoot > m_root;
    UnoInterfaceReference const m_target;
    OUString const m_oid;
    TypeDescription m_typeDescr;

    binuno_Proxy(
        ::rtl::Reference< ProxyRoot > const & root,
        UnoInterfaceReference const & target,
        OUString const & oid, TypeDescription const & typeDescr );
};

extern "C"
{


static void binuno_proxy_free(
    uno_ExtEnvironment * pEnv, void * pProxy )
{
    binuno_Proxy * proxy = static_cast< binuno_Proxy * >(
        static_cast< uno_Interface * >( pProxy ) );
    OSL_ASSERT( proxy->m_root->m_factory->m_uno_env.get()->pExtEnv == pEnv );
    delete proxy;
}


static void binuno_proxy_acquire( uno_Interface * pUnoI )
{
    binuno_Proxy * that = static_cast< binuno_Proxy * >( pUnoI );
    if (osl_atomic_increment( &that->m_nRefCount ) != 1)
        return;

    // rebirth of zombie
    uno_ExtEnvironment * uno_env =
        that->m_root->m_factory->m_uno_env.get()->pExtEnv;
    OSL_ASSERT( uno_env != nullptr );
    (*uno_env->registerProxyInterface)(
        uno_env, reinterpret_cast< void ** >( &pUnoI ), binuno_proxy_free,
        that->m_oid.pData,
        reinterpret_cast< typelib_InterfaceTypeDescription * >(
            that->m_typeDescr.get() ) );
    OSL_ASSERT( that == static_cast< binuno_Proxy * >( pUnoI ) );
}


static void binuno_proxy_release( uno_Interface * pUnoI )
{
    binuno_Proxy * that = static_cast< binuno_Proxy * >( pUnoI );
    if (osl_atomic_decrement( &that->m_nRefCount ) == 0)
    {
        uno_ExtEnvironment * uno_env =
            that->m_root->m_factory->m_uno_env.get()->pExtEnv;
        OSL_ASSERT( uno_env != nullptr );
        (*uno_env->revokeInterface)( uno_env, pUnoI );
    }
}


static void binuno_proxy_dispatch(
    uno_Interface * pUnoI, const typelib_TypeDescription * pMemberType,
    void * pReturn, void * pArgs [], uno_Any ** ppException )
{
    binuno_Proxy * that = static_cast< binuno_Proxy * >( pUnoI );
    switch (reinterpret_cast< typelib_InterfaceMemberTypeDescription const * >(
                pMemberType )->nPosition)
    {
    case 0: // queryInterface()
    {
        try
        {
            Type const & rType =
                *static_cast< Type const * >( pArgs[ 0 ] );
            Any ret( that->m_root->queryInterface( rType ) );
            uno_type_copyAndConvertData(
                pReturn, &ret, cppu::UnoType<decltype(ret)>::get().getTypeLibType(),
                that->m_root->m_factory->m_cpp2uno.get() );
            *ppException = nullptr; // no exc
        }
        catch (RuntimeException &)
        {
            Any exc( ::cppu::getCaughtException() );
            uno_type_any_constructAndConvert(
                *ppException, const_cast< void * >(exc.getValue()),
                exc.getValueTypeRef(),
                that->m_root->m_factory->m_cpp2uno.get() );
        }
        break;
    }
    case 1: // acquire()
        binuno_proxy_acquire( pUnoI );
        *ppException = nullptr; // no exc
        break;
    case 2: // release()
        binuno_proxy_release( pUnoI );
        *ppException = nullptr; // no exc
        break;
    default:
        that->m_target.dispatch( pMemberType, pReturn, pArgs, ppException );
        break;
    }
}

}


binuno_Proxy::binuno_Proxy(
    ::rtl::Reference< ProxyRoot > const & root,
    UnoInterfaceReference const & target,
    OUString const & oid, TypeDescription const & typeDescr )
    : m_nRefCount( 1 ),
      m_root( root ),
      m_target( target ),
      m_oid( oid ),
      m_typeDescr( typeDescr )
{
    uno_Interface::acquire = binuno_proxy_acquire;
    uno_Interface::release = binuno_proxy_release;
    uno_Interface::pDispatcher = binuno_proxy_dispatch;
}

ProxyRoot::ProxyRoot(
    ::rtl::Reference< FactoryImpl > const & factory,
    Reference< XInterface > const & xTarget )
    : m_factory( factory )
{
    m_factory->m_cpp2uno.mapInterface(
        reinterpret_cast< void ** >( &m_target.m_pUnoI ), xTarget.get(),
        cppu::UnoType<decltype(xTarget)>::get() );
    OSL_ENSURE( m_target.is(), "### mapping interface failed!" );
}


Any ProxyRoot::queryAggregation( Type const & rType )
{
    Any ret( OWeakAggObject::queryAggregation( rType ) );
    if (! ret.hasValue())
    {
        typelib_TypeDescription * pTypeDescr = nullptr;
        TYPELIB_DANGER_GET( &pTypeDescr, rType.getTypeLibType() );
        try
        {
            Reference< XInterface > xProxy;
            uno_ExtEnvironment * cpp_env = m_factory->m_cpp_env.get()->pExtEnv;
            OSL_ASSERT( cpp_env != nullptr );

            // mind a new delegator, calculate current root:
            Reference< XInterface > xRoot(
                static_cast< OWeakObject * >(this), UNO_QUERY_THROW );
            OUString oid;
            (*cpp_env->getObjectIdentifier)( cpp_env, &oid.pData, xRoot.get() );
            OSL_ASSERT( !oid.isEmpty() );

            (*cpp_env->getRegisteredInterface)(
                cpp_env, reinterpret_cast< void ** >( &xProxy ),
                oid.pData, reinterpret_cast<
                typelib_InterfaceTypeDescription * >(pTypeDescr) );
            if (! xProxy.is())
            {
                // perform query on target:
                UnoInterfaceReference proxy_target(
                    m_factory->binuno_queryInterface(
                        m_target, reinterpret_cast<
                        typelib_InterfaceTypeDescription * >(pTypeDescr) ) );
                if (proxy_target.is())
                {
                    // ensure root's object entries:
                    UnoInterfaceReference root;
                    m_factory->m_cpp2uno.mapInterface(
                        reinterpret_cast< void ** >( &root.m_pUnoI ),
                        xRoot.get(), cppu::UnoType<decltype(xRoot)>::get() );

                    UnoInterfaceReference proxy(
                        // ref count initially 1:
                        new binuno_Proxy( this, proxy_target, oid, pTypeDescr ),
                        SAL_NO_ACQUIRE );
                    uno_ExtEnvironment * uno_env =
                        m_factory->m_uno_env.get()->pExtEnv;
                    OSL_ASSERT( uno_env != nullptr );
                    (*uno_env->registerProxyInterface)(
                        uno_env, reinterpret_cast< void ** >( &proxy.m_pUnoI ),
                        binuno_proxy_free, oid.pData,
                        reinterpret_cast< typelib_InterfaceTypeDescription * >(
                            pTypeDescr ) );

                    m_factory->m_uno2cpp.mapInterface(
                        reinterpret_cast< void ** >( &xProxy ),
                        proxy.get(), pTypeDescr );
                }
            }
            if (xProxy.is())
                ret.setValue( &xProxy, pTypeDescr );
        }
        catch (...) // finally
        {
            TYPELIB_DANGER_RELEASE( pTypeDescr );
            throw;
        }
        TYPELIB_DANGER_RELEASE( pTypeDescr );
    }
    return ret;
}


FactoryImpl::FactoryImpl()
{
    OUString uno = UNO_LB_UNO;
    OUString cpp = CPPU_CURRENT_LANGUAGE_BINDING_NAME;

    uno_getEnvironment(
        reinterpret_cast< uno_Environment ** >( &m_uno_env ), uno.pData, nullptr );
    OSL_ENSURE( m_uno_env.is(), "### cannot get binary uno env!" );

    uno_getEnvironment(
        reinterpret_cast< uno_Environment ** >( &m_cpp_env ), cpp.pData, nullptr );
    OSL_ENSURE( m_cpp_env.is(), "### cannot get C++ uno env!" );

    uno_getMapping(
        reinterpret_cast< uno_Mapping ** >( &m_uno2cpp ),
        m_uno_env.get(), m_cpp_env.get(), nullptr );
    OSL_ENSURE( m_uno2cpp.is(), "### cannot get bridge uno <-> C++!" );

    uno_getMapping(
        reinterpret_cast< uno_Mapping ** >( &m_cpp2uno ),
        m_cpp_env.get(), m_uno_env.get(), nullptr );
    OSL_ENSURE( m_cpp2uno.is(), "### cannot get bridge C++ <-> uno!" );
}

// XProxyFactory

Reference< XAggregation > FactoryImpl::createProxy(
    Reference< XInterface > const & xTarget )
{
    return new ProxyRoot( this, xTarget );
}

// XServiceInfo

OUString FactoryImpl::getImplementationName()
{
    return proxyfac_getImplementationName();
}

sal_Bool FactoryImpl::supportsService( const OUString & rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > FactoryImpl::getSupportedServiceNames()
{
    return proxyfac_getSupportedServiceNames();
}

/// @throws Exception
Reference< XInterface > proxyfac_create(
    SAL_UNUSED_PARAMETER Reference< XComponentContext > const & )
{
    Reference< XInterface > xRet;
    static osl::Mutex s_mutex;
    // note: don't use ::osl::Mutex::getGlobalMutex() here, it deadlocks
    //       with getImplHelperInitMutex()
    ::osl::MutexGuard guard(s_mutex);
    static WeakReference < XInterface > rwInstance;
    xRet = rwInstance;

    if (! xRet.is())
    {
        xRet = static_cast< ::cppu::OWeakObject * >(new FactoryImpl);
        rwInstance = xRet;
    }
    return xRet;
}

static const ::cppu::ImplementationEntry g_entries [] =
{
    {
        proxyfac_create, proxyfac_getImplementationName,
        proxyfac_getSupportedServiceNames, ::cppu::createSingleComponentFactory,
        nullptr, 0
    },
    { nullptr, nullptr, nullptr, nullptr, nullptr, 0 }
};

}

extern "C" SAL_DLLPUBLIC_EXPORT void * proxyfac_component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return ::cppu::component_getFactoryHelper(
        pImplName, pServiceManager, pRegistryKey, g_entries );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
