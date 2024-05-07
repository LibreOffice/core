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


#include "dp_manager.h"
#include <dp_misc.h>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/deployment/XPackageManagerFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <unordered_map>

using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace dp_manager::factory {

typedef ::cppu::WeakComponentImplHelper<
    deployment::XPackageManagerFactory, lang::XServiceInfo > t_pmfac_helper;

namespace {

class PackageManagerFactoryImpl : private cppu::BaseMutex, public t_pmfac_helper
{
    Reference<XComponentContext> m_xComponentContext;

    Reference<deployment::XPackageManager> m_xUserMgr;
    Reference<deployment::XPackageManager> m_xSharedMgr;
    Reference<deployment::XPackageManager> m_xBundledMgr;
    Reference<deployment::XPackageManager> m_xTmpMgr;
    Reference<deployment::XPackageManager> m_xBakMgr;
    typedef std::unordered_map<
        OUString, WeakReference<deployment::XPackageManager> > t_string2weakref;
    t_string2weakref m_managers;

protected:
    inline void check();
    virtual void SAL_CALL disposing() override;

public:
    explicit PackageManagerFactoryImpl(
        Reference<XComponentContext> const & xComponentContext );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XPackageManagerFactory
    virtual Reference<deployment::XPackageManager> SAL_CALL getPackageManager(
        OUString const & context ) override;
};

}

PackageManagerFactoryImpl::PackageManagerFactoryImpl(
    Reference<XComponentContext> const & xComponentContext )
    : t_pmfac_helper( m_aMutex ),
      m_xComponentContext( xComponentContext )
{
}

// XServiceInfo
OUString PackageManagerFactoryImpl::getImplementationName()
{
    return u"com.sun.star.comp.deployment.PackageManagerFactory"_ustr;
}

sal_Bool PackageManagerFactoryImpl::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence< OUString > PackageManagerFactoryImpl::getSupportedServiceNames()
{
    // a private one:
    return { u"com.sun.star.comp.deployment.PackageManagerFactory"_ustr };
}

inline void PackageManagerFactoryImpl::check()
{
    ::osl::MutexGuard guard( m_aMutex );
    if (rBHelper.bInDispose || rBHelper.bDisposed)
    {
        throw lang::DisposedException(
            u"PackageManagerFactory instance has already been disposed!"_ustr,
            static_cast<OWeakObject *>(this) );
    }
}


void PackageManagerFactoryImpl::disposing()
{
    // dispose all managers:
    ::osl::MutexGuard guard( m_aMutex );
    for (auto const& elem : m_managers)
        try_dispose( elem.second );
    m_managers = t_string2weakref();
    // the below are already disposed:
    m_xUserMgr.clear();
    m_xSharedMgr.clear();
    m_xBundledMgr.clear();
    m_xTmpMgr.clear();
    m_xBakMgr.clear();
}

// XPackageManagerFactory

Reference<deployment::XPackageManager>
PackageManagerFactoryImpl::getPackageManager( OUString const & context )
{
    Reference< deployment::XPackageManager > xRet;
    ::osl::ResettableMutexGuard guard( m_aMutex );
    check();
    t_string2weakref::const_iterator const iFind( m_managers.find( context ) );
    if (iFind != m_managers.end()) {
        xRet = iFind->second;
        if (xRet.is())
            return xRet;
    }

    guard.clear();
    xRet.set( PackageManagerImpl::create( m_xComponentContext, context ) );
    guard.reset();
    std::pair< t_string2weakref::iterator, bool > insertion(
        m_managers.emplace( context, xRet ) );
    if (insertion.second)
    {
        OSL_ASSERT( insertion.first->second.get() == xRet );
        // hold user, shared mgrs for whole process: live deployment
        if ( context == "user" )
            m_xUserMgr = xRet;
        else if ( context == "shared" )
            m_xSharedMgr = xRet;
        else if ( context == "bundled" )
            m_xBundledMgr = xRet;
        else if ( context == "tmp" )
            m_xTmpMgr = xRet;
        else if ( context == "bak" )
            m_xBakMgr = xRet;
    }
    else
    {
        Reference< deployment::XPackageManager > xAlreadyIn(
            insertion.first->second );
        if (xAlreadyIn.is())
        {
            guard.clear();
            try_dispose( xRet );
            xRet = xAlreadyIn;
        }
        else
        {
            insertion.first->second = xRet;
        }
    }
    return xRet;
}

} // namespace dp_manager::factory

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_deployment_PackageManagerFactory_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& )
{
    return cppu::acquire(new dp_manager::factory::PackageManagerFactoryImpl(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
