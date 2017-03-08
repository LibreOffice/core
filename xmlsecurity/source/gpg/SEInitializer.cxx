/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "SEInitializer.hxx"
#include "SecurityEnvironment.hxx"

#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <com/sun/star/xml/crypto/gpg/GpgSecurityEnvironment.hpp>
#include <com/sun/star/xml/crypto/gpg/GpgXMLSecurityContext.hpp>

using namespace css;
using namespace css::lang;
using namespace css::uno;
using namespace css::xml::crypto;


SEInitializerGpg::SEInitializerGpg( const css::uno::Reference< css::uno::XComponentContext > &rxContext )
{
    m_xContext = rxContext;
}

SEInitializerGpg::~SEInitializerGpg()
{
}

/* XSEInitializer */
Reference< XXMLSecurityContext > SAL_CALL SEInitializerGpg::createSecurityContext( const OUString& )
{
    try
    {
        /* Build XML Security Context */
        Reference< XXMLSecurityContext > xSecCtx = xml::crypto::gpg::GpgXMLSecurityContext::create(m_xContext);

        Reference< XSecurityEnvironment > xSecEnv = xml::crypto::gpg::GpgSecurityEnvironment::create(m_xContext);
        Reference< XUnoTunnel > xSecEnvTunnel(xSecEnv, uno::UNO_QUERY_THROW);

        sal_Int32 n = xSecCtx->addSecurityEnvironment(xSecEnv);
        //originally the SecurityEnvironment with the internal slot was set as default
        xSecCtx->setDefaultSecurityEnvironmentIndex( n );
        return xSecCtx;
    }
    catch( const uno::Exception& )
    {
        return nullptr;
    }
}

void SAL_CALL SEInitializerGpg::freeSecurityContext( const uno::Reference< XXMLSecurityContext >& )
{
}

OUString SEInitializer_getImplementationName()
{
    return OUString("com.sun.star.xml.security.SEInitializer_Gpg");
}

uno::Sequence< OUString > SAL_CALL SEInitializer_getSupportedServiceNames()
{
    return {"com.sun.star.xml.crypto.gpg.GpgSEInitializer"};
}

uno::Reference< uno::XInterface > SAL_CALL SEInitializer_createInstance( const uno::Reference< lang::XMultiServiceFactory > & rxMSF)
{
    return static_cast<cppu::OWeakObject*>(new SEInitializerGpg(comphelper::getComponentContext(rxMSF)));
}

/* XServiceInfo */
OUString SAL_CALL SEInitializerGpg::getImplementationName()
{
    return SEInitializer_getImplementationName();
}

sal_Bool SAL_CALL SEInitializerGpg::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL SEInitializerGpg::getSupportedServiceNames()
{
    return SEInitializer_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
