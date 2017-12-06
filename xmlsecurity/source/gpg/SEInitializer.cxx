/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_gpgme.h>

#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <gpg/SEInitializer.hxx>
#include "SecurityEnvironment.hxx"
#include "XMLSecurityContext.hxx"

#include <gpgme.h>
#include <context.h>

using namespace css;
using namespace css::lang;
using namespace css::uno;
using namespace css::xml::crypto;


SEInitializerGpg::SEInitializerGpg()
{
    // Also init GpgME while we're at it
    GpgME::initializeLibrary();
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
        Reference< XXMLSecurityContext > xSecCtx(new XMLSecurityContextGpg());

        Reference< XSecurityEnvironment > xSecEnv(new SecurityEnvironmentGpg());
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



uno::Reference< uno::XInterface > SAL_CALL SEInitializerGpg::impl_createInstance( const uno::Reference< lang::XMultiServiceFactory > & /*rxMSF*/)
{
    return static_cast<cppu::OWeakObject*>(new SEInitializerGpg());
}

uno::Reference< XSingleServiceFactory > SEInitializerGpg::impl_createFactory( const Reference< XMultiServiceFactory >& aServiceManager ) {
    return cppu::createSingleFactory( aServiceManager, impl_getImplementationName(), impl_createInstance, impl_getSupportedServiceNames() ) ;
}

/* XServiceInfo */
OUString SEInitializerGpg::impl_getImplementationName()
{
    return OUString("com.sun.star.xml.security.SEInitializer_Gpg");
}

uno::Sequence< OUString > SEInitializerGpg::impl_getSupportedServiceNames()
{
    return {"com.sun.star.xml.crypto.GPGSEInitializer"};
}

sal_Bool SAL_CALL SEInitializerGpg::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL SEInitializerGpg::getSupportedServiceNames()
{
    return impl_getSupportedServiceNames();
}

OUString SAL_CALL SEInitializerGpg::getImplementationName()
{
    return impl_getImplementationName();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
