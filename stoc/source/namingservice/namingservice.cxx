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


#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <com/sun/star/uno/XNamingService.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <mutex>
#include <unordered_map>

using namespace cppu;

using namespace css::uno;
using namespace css::lang;
using namespace css::registry;


namespace stoc_namingservice
{

typedef std::unordered_map< OUString, Reference<XInterface > > HashMap_OWString_Interface;

namespace {

class NamingService_Impl
    : public WeakImplHelper < XServiceInfo, XNamingService >
{
    std::mutex                          aMutex;
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



NamingService_Impl::NamingService_Impl() {}

// XServiceInfo
OUString NamingService_Impl::getImplementationName()
{
    return u"com.sun.star.comp.stoc.NamingService"_ustr;
}

// XServiceInfo
sal_Bool NamingService_Impl::supportsService( const OUString & rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

// XServiceInfo
Sequence< OUString > NamingService_Impl::getSupportedServiceNames()
{
    return { u"com.sun.star.uno.NamingService"_ustr };
}

// XServiceInfo
Reference< XInterface > NamingService_Impl::getRegisteredObject( const OUString& Name )
{
    std::scoped_lock aGuard( aMutex );
    Reference< XInterface > xRet;
    HashMap_OWString_Interface::iterator aIt = aMap.find( Name );
    if( aIt != aMap.end() )
        xRet = (*aIt).second;
    return xRet;
}

// XServiceInfo
void NamingService_Impl::registerObject( const OUString& Name, const Reference< XInterface >& Object )
{
    std::scoped_lock aGuard( aMutex );
    aMap[ Name ] = Object;
}

// XServiceInfo
void NamingService_Impl::revokeObject( const OUString& Name )
{
    std::scoped_lock aGuard( aMutex );
    aMap.erase( Name );
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
stoc_NamingService_Impl_get_implementation(
    css::uno::XComponentContext* , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new stoc_namingservice::NamingService_Impl());
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
