/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "XMLSecurityContext.hxx"
#include "SecurityEnvironment.hxx"

#include <cppuhelper/supportsservice.hxx>



using namespace css::uno;
using namespace css::lang;
using namespace css::xml::crypto;

XMLSecurityContext::XMLSecurityContext()
    : m_nDefaultEnvIndex(-1)
{
}

XMLSecurityContext::~XMLSecurityContext()
{
}

sal_Int32 SAL_CALL XMLSecurityContext::addSecurityEnvironment(
    const Reference< XSecurityEnvironment >& aSecurityEnvironment)
    throw (css::security::SecurityInfrastructureException, RuntimeException, std::exception)
{
}


sal_Int32 SAL_CALL XMLSecurityContext::getSecurityEnvironmentNumber()
    throw (RuntimeException, std::exception)
{
}

Reference< XSecurityEnvironment > SAL_CALL XMLSecurityContext::getSecurityEnvironmentByIndex(sal_Int32 /*index*/)
    throw (RuntimeException, std::exception)
{
}

Reference< XSecurityEnvironment > SAL_CALL XMLSecurityContext::getSecurityEnvironment()
    throw (RuntimeException, std::exception)
{
}

sal_Int32 SAL_CALL XMLSecurityContext::getDefaultSecurityEnvironmentIndex()
    throw (RuntimeException, std::exception)
{
    return m_nDefaultEnvIndex ;
}

void SAL_CALL XMLSecurityContext::setDefaultSecurityEnvironmentIndex(sal_Int32 nDefaultEnvIndex)
    throw (RuntimeException, std::exception)
{
    m_nDefaultEnvIndex = nDefaultEnvIndex;
}

/* XServiceInfo */
OUString SAL_CALL XMLSecurityContext::getImplementationName()
    throw( RuntimeException, std::exception )
{
    return impl_getImplementationName() ;
}

/* XServiceInfo */
sal_Bool SAL_CALL XMLSecurityContext::supportsService( const OUString& serviceName)
    throw( RuntimeException, std::exception )
{
    return cppu::supportsService(this, serviceName);
}

/* XServiceInfo */
Sequence< OUString > SAL_CALL XMLSecurityContext::getSupportedServiceNames()
    throw( RuntimeException, std::exception )
{
    return impl_getSupportedServiceNames() ;
}

//Helper for XServiceInfo
Sequence< OUString > XMLSecurityContext::impl_getSupportedServiceNames()
{
    return {"com.sun.star.xml.crypto.XMLSecurityContext"};
}

OUString XMLSecurityContext::impl_getImplementationName()
    throw( RuntimeException )
{
    return "com.sun.star.xml.security.XMLSecurityContext_Gpg";
}

//Helper for registry
Reference< XInterface > SAL_CALL XMLSecurityContext::impl_createInstance( const Reference< XMultiServiceFactory >& )
    throw( RuntimeException )
{
    return Reference< XInterface >( *new XMLSecurityContext ) ;
}

Reference< XSingleServiceFactory > XMLSecurityContext::impl_createFactory( const Reference< XMultiServiceFactory >& aServiceManager )
{
    return ::cppu::createSingleFactory( aServiceManager , impl_getImplementationName() , impl_createInstance , impl_getSupportedServiceNames() ) ;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
