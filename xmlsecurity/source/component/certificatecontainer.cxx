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

#include <map>

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/security/XCertificateContainer.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/supportsservice.hxx>
#include <rtl/ref.hxx>

#include <sal/config.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

class CertificateContainer
    : public ::cppu::WeakImplHelper<css::lang::XServiceInfo, css::security::XCertificateContainer>
{
private:
    typedef std::map<OUString, OUString> Map;
    Map certMap;
    Map certTrustMap;

    static bool searchMap(const OUString& url, const OUString& certificate_name, Map& _certMap);
    /// @throws css::uno::RuntimeException
    bool isTemporaryCertificate(const OUString& url, const OUString& certificate_name);
    /// @throws css::uno::RuntimeException
    bool isCertificateTrust(const OUString& url, const OUString& certificate_name);

public:
    explicit CertificateContainer(const uno::Reference<uno::XComponentContext>&) {}
    virtual sal_Bool SAL_CALL addCertificate(const OUString& url, const OUString& certificate_name,
                                             sal_Bool trust) override;
    virtual css::security::CertificateContainerStatus SAL_CALL
    hasCertificate(const OUString& url, const OUString& certificate_name) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};

bool
CertificateContainer::searchMap( const OUString & url, const OUString & certificate_name, Map &_certMap )
{
    Map::iterator p = _certMap.find(url);

    bool ret = false;

    while( p != _certMap.end() )
    {
        ret = (*p).second == certificate_name;
        if( ret )
                    break;
        ++p;
    }

    return ret;
}

bool
CertificateContainer::isTemporaryCertificate ( const OUString & url, const OUString & certificate_name )
{
    return searchMap( url, certificate_name, certMap);
}

bool
CertificateContainer::isCertificateTrust ( const OUString & url, const OUString & certificate_name )
{
    return searchMap( url, certificate_name, certTrustMap);
}

sal_Bool
CertificateContainer::addCertificate( const OUString & url, const OUString & certificate_name, sal_Bool trust )
{
    certMap.emplace( url, certificate_name );

    //remember that the cert is trusted
    if (trust)
        certTrustMap.emplace( url, certificate_name );

    return true;
}

::security::CertificateContainerStatus
CertificateContainer::hasCertificate( const OUString & url, const OUString & certificate_name )
{
    if ( isTemporaryCertificate( url, certificate_name ) )
    {
        if ( isCertificateTrust( url, certificate_name ) )
            return security::CertificateContainerStatus_TRUSTED;
        else
            return security::CertificateContainerStatus_UNTRUSTED;
    } else
    {
        return security::CertificateContainerStatus_NOCERT;
    }
}

OUString SAL_CALL
CertificateContainer::getImplementationName( )
{
    return OUString("com.sun.star.security.CertificateContainer");
}

sal_Bool SAL_CALL
CertificateContainer::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

Sequence< OUString > SAL_CALL
CertificateContainer::getSupportedServiceNames(  )
{
    Sequence< OUString > aRet { "com.sun.star.security.CertificateContainer" };
    return aRet;
}

namespace
{
struct Instance
{
    explicit Instance(css::uno::Reference<css::uno::XComponentContext> const& context)
        : instance(new CertificateContainer(context))
    {
    }

    rtl::Reference<CertificateContainer> instance;
};

struct Singleton
    : public rtl::StaticWithArg<Instance, css::uno::Reference<css::uno::XComponentContext>,
                                Singleton>
{
};
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_security_CertificateContainer_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(Singleton::get(context).instance.get());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
