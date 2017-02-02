/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/processfactory.hxx>
#include <sal/types.h>
#include <rtl/bootstrap.hxx>
#include <rtl/string.hxx>
#include <rtl/strbuf.hxx>
#include <osl/file.hxx>
#include <osl/thread.h>
#include <com/sun/star/xml/crypto/SecurityEnvironment.hpp>
#include <com/sun/star/xml/crypto/XMLSecurityContext.hpp>
#include <cppuhelper/supportsservice.hxx>

#include "SEInitializer.hxx"
#include "SecurityEnvironment.hxx"

using namespace css;
using namespace css::xml::crypto;


SEInitializer::SEInitializer( const css::uno::Reference< css::uno::XComponentContext > &rxContext )
{
    m_xContext = rxContext;
}

SEInitializer::~SEInitializer()
{
}

/* XSEInitializer */
uno::Reference< XXMLSecurityContext > SAL_CALL SEInitializer::createSecurityContext( const OUString& )
    throw (uno::RuntimeException, std::exception)
{
}

void SAL_CALL SEInitializer::freeSecurityContext( const uno::Reference< XXMLSecurityContext >& )
    throw (uno::RuntimeException, std::exception)
{
}

OUString SEInitializer_getImplementationName ()
    throw (uno::RuntimeException)
{
    return OUString ("com.sun.star.xml.security.SEInitializer_Gpg" );
}

uno::Sequence< OUString > SAL_CALL SEInitializer_getSupportedServiceNames(  )
    throw (uno::RuntimeException)
{
    return { "com.sun.star.xml.crypto.SecurityEnvironment" };
}

uno::Reference< uno::XInterface > SAL_CALL SEInitializer_createInstance( const uno::Reference< lang::XMultiServiceFactory > & rxMSF)
    throw( uno::Exception )
{
    return static_cast<cppu::OWeakObject*>(new SEInitializer(comphelper::getComponentContext(rxMSF)));
}

/* XServiceInfo */
OUString SAL_CALL SEInitializer::getImplementationName(  )
    throw (uno::RuntimeException, std::exception)
{
    return SEInitializer_getImplementationName();
}
sal_Bool SAL_CALL SEInitializer::supportsService( const OUString& rServiceName )
    throw (uno::RuntimeException, std::exception)
{
    return cppu::supportsService( this, rServiceName );
}
uno::Sequence< OUString > SAL_CALL SEInitializer::getSupportedServiceNames(  )
    throw (uno::RuntimeException, std::exception)
{
    return SEInitializer_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
