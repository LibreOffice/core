/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_desktop.hxx"

#include "dp_manager.h"
#include "dp_resource.h"
#include "cppuhelper/compbase1.hxx"
#include "comphelper/servicedecl.hxx"
#include "com/sun/star/deployment/thePackageManagerFactory.hpp"


using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;

namespace dp_manager {
namespace factory {

typedef ::cppu::WeakComponentImplHelper1<
    deployment::XPackageManagerFactory > t_pmfac_helper;

//==============================================================================
class PackageManagerFactoryImpl : private MutexHolder, public t_pmfac_helper
{
    Reference<XComponentContext> m_xComponentContext;

    Reference<deployment::XPackageManager> m_xUserMgr;
    Reference<deployment::XPackageManager> m_xSharedMgr;
    Reference<deployment::XPackageManager> m_xBundledMgr;
    typedef ::boost::unordered_map<
        OUString, WeakReference<deployment::XPackageManager>,
        ::rtl::OUStringHash > t_string2weakref;
    t_string2weakref m_managers;

protected:
    inline void check();
    virtual void SAL_CALL disposing();

public:
    virtual ~PackageManagerFactoryImpl();
    PackageManagerFactoryImpl(
        Reference<XComponentContext> const & xComponentContext );

    // XPackageManagerFactory
    virtual Reference<deployment::XPackageManager> SAL_CALL getPackageManager(
        OUString const & context ) throw (RuntimeException);
};

//==============================================================================
namespace sdecl = comphelper::service_decl;
sdecl::class_<PackageManagerFactoryImpl> servicePMFI;
extern sdecl::ServiceDecl const serviceDecl(
    servicePMFI,
    // a private one:
    "com.sun.star.comp.deployment.PackageManagerFactory",
    "com.sun.star.comp.deployment.PackageManagerFactory" );

//==============================================================================
bool singleton_entries(
    Reference<registry::XRegistryKey> const & xRegistryKey )
{
    try {
        Reference<registry::XRegistryKey> xKey(
            xRegistryKey->createKey(
                serviceDecl.getImplementationName() +
                // xxx todo: use future generated function to get singleton name
                OUSTR("/UNO/SINGLETONS/"
                      "com.sun.star.deployment.thePackageManagerFactory") ) );
        xKey->setStringValue( serviceDecl.getSupportedServiceNames()[0] );
        return true;
    }
    catch (registry::InvalidRegistryException & exc) {
        (void) exc; // avoid warnings
        OSL_ENSURE( 0, ::rtl::OUStringToOString(
                        exc.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
        return false;
    }
}

//______________________________________________________________________________
PackageManagerFactoryImpl::PackageManagerFactoryImpl(
    Reference<XComponentContext> const & xComponentContext )
    : t_pmfac_helper( getMutex() ),
      m_xComponentContext( xComponentContext )
{
}

//______________________________________________________________________________
PackageManagerFactoryImpl::~PackageManagerFactoryImpl()
{
}

//______________________________________________________________________________
inline void PackageManagerFactoryImpl::check()
{
    ::osl::MutexGuard guard( getMutex() );
    if (rBHelper.bInDispose || rBHelper.bDisposed)
    {
        throw lang::DisposedException(
            OUSTR("PackageManagerFactory instance has already been disposed!"),
            static_cast<OWeakObject *>(this) );
    }
}

//______________________________________________________________________________
void PackageManagerFactoryImpl::disposing()
{
    // dispose all managers:
    ::osl::MutexGuard guard( getMutex() );
    t_string2weakref::const_iterator iPos( m_managers.begin() );
    t_string2weakref::const_iterator const iEnd( m_managers.end() );
    for ( ; iPos != iEnd; ++iPos )
        try_dispose( iPos->second );
    m_managers = t_string2weakref();
    // the below are already disposed:
    m_xUserMgr.clear();
    m_xSharedMgr.clear();
    m_xBundledMgr.clear();
}

// XPackageManagerFactory
//______________________________________________________________________________
Reference<deployment::XPackageManager>
PackageManagerFactoryImpl::getPackageManager( OUString const & context )
    throw (RuntimeException)
{
    Reference< deployment::XPackageManager > xRet;
    ::osl::ResettableMutexGuard guard( getMutex() );
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
    ::std::pair< t_string2weakref::iterator, bool > insertion(
        m_managers.insert( t_string2weakref::value_type( context, xRet ) ) );
    if (insertion.second)
    {
        OSL_ASSERT( insertion.first->second.get() == xRet );
        // hold user, shared mgrs for whole process: live deployment
        if (context.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("user") ))
            m_xUserMgr = xRet;
        else if (context.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("shared") ))
            m_xSharedMgr = xRet;
        else if (context.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("bundled") ))
            m_xBundledMgr = xRet;
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

} // namespace factory
} // namespace dp_manager

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
