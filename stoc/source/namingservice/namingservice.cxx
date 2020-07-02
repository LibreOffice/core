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


#include <osl/mutex.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <com/sun/star/uno/XNamingService.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <unordered_map>

using namespace cppu;
using namespace osl;
using namespace std;

using namespace css::uno;
using namespace css::lang;
using namespace css::registry;


#define SERVICENAME "com.sun.star.uno.NamingService"
#define IMPLNAME    "com.sun.star.comp.stoc.NamingService"

namespace stoc_namingservice
{

static Sequence< OUString > ns_getSupportedServiceNames()
{
    Sequence< OUString > seqNames { SERVICENAME };
    return seqNames;
}

static OUString ns_getImplementationName()
{
    return IMPLNAME;
}

typedef std::unordered_map< OUString, Reference<XInterface > > HashMap_OWString_Interface;

namespace {

class NamingService_Impl
    : public WeakImplHelper < XServiceInfo, XNamingService >
{
    Mutex                               aMutex;
    HashMap_OWString_Interface          aMap;
public:
    NamingService_Impl();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getRegisteredObject( const OUString& Name ) override;
    virtual void SAL_CALL registerObject( const OUString& Name, const css::uno::Reference< css::uno::XInterface >& Object ) override;
    virtual void SAL_CALL revokeObject( const OUString& Name ) override;
};

}

static Reference<XInterface> NamingService_Impl_create(
    SAL_UNUSED_PARAMETER const Reference<XComponentContext> & )
{
    return *new NamingService_Impl();
}


NamingService_Impl::NamingService_Impl() {}

// XServiceInfo
OUString NamingService_Impl::getImplementationName()
{
    return ns_getImplementationName();
}

// XServiceInfo
sal_Bool NamingService_Impl::supportsService( const OUString & rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

// XServiceInfo
Sequence< OUString > NamingService_Impl::getSupportedServiceNames()
{
    return ns_getSupportedServiceNames();
}

// XServiceInfo
Reference< XInterface > NamingService_Impl::getRegisteredObject( const OUString& Name )
{
    Guard< Mutex > aGuard( aMutex );
    Reference< XInterface > xRet;
    HashMap_OWString_Interface::iterator aIt = aMap.find( Name );
    if( aIt != aMap.end() )
        xRet = (*aIt).second;
    return xRet;
}

// XServiceInfo
void NamingService_Impl::registerObject( const OUString& Name, const Reference< XInterface >& Object )
{
    Guard< Mutex > aGuard( aMutex );
    aMap[ Name ] = Object;
}

// XServiceInfo
void NamingService_Impl::revokeObject( const OUString& Name )
{
    Guard< Mutex > aGuard( aMutex );
    aMap.erase( Name );
}

}

using namespace stoc_namingservice;
const struct ImplementationEntry g_entries[] =
{
    {
        NamingService_Impl_create, ns_getImplementationName,
        ns_getSupportedServiceNames, createSingleComponentFactory,
        nullptr, 0
    },
    { nullptr, nullptr, nullptr, nullptr, nullptr, 0 }
};

extern "C" SAL_DLLPUBLIC_EXPORT void * namingservice_component_getFactory(
    const char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey , g_entries );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
