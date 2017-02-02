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

XMLSecurityContextGpg::XMLSecurityContextGpg()
    : m_nDefaultEnvIndex(-1)
{
}

XMLSecurityContextGpg::~XMLSecurityContextGpg()
{
}

sal_Int32 SAL_CALL XMLSecurityContextGpg::addSecurityEnvironment(
    const Reference< XSecurityEnvironment >& aSecurityEnvironment)
    throw (css::security::SecurityInfrastructureException, RuntimeException, std::exception)
{
    if(!aSecurityEnvironment.is())
        throw RuntimeException("Invalid SecurityEnvironment given!");

    m_vSecurityEnvironments.push_back(aSecurityEnvironment);
    return m_vSecurityEnvironments.size() - 1 ;
}


sal_Int32 SAL_CALL XMLSecurityContextGpg::getSecurityEnvironmentNumber()
    throw (RuntimeException, std::exception)
{
    return m_vSecurityEnvironments.size();
}

Reference< XSecurityEnvironment > SAL_CALL XMLSecurityContextGpg::getSecurityEnvironmentByIndex(sal_Int32 index)
    throw (RuntimeException, std::exception)
{
    if (index < 0 || index >= ( sal_Int32 )m_vSecurityEnvironments.size())
        throw RuntimeException("Invalid index");

    return m_vSecurityEnvironments[index];
}

Reference< XSecurityEnvironment > SAL_CALL XMLSecurityContextGpg::getSecurityEnvironment()
    throw (RuntimeException, std::exception)
{
    if (m_nDefaultEnvIndex < 0 || m_nDefaultEnvIndex >= (sal_Int32) m_vSecurityEnvironments.size())
        throw RuntimeException("Invalid index");

    return getSecurityEnvironmentByIndex(m_nDefaultEnvIndex);
}

sal_Int32 SAL_CALL XMLSecurityContextGpg::getDefaultSecurityEnvironmentIndex()
    throw (RuntimeException, std::exception)
{
    return m_nDefaultEnvIndex ;
}

void SAL_CALL XMLSecurityContextGpg::setDefaultSecurityEnvironmentIndex(sal_Int32 nDefaultEnvIndex)
    throw (RuntimeException, std::exception)
{
    m_nDefaultEnvIndex = nDefaultEnvIndex;
}

/* XServiceInfo */
OUString SAL_CALL XMLSecurityContextGpg::getImplementationName()
    throw( RuntimeException, std::exception )
{
    return impl_getImplementationName() ;
}

/* XServiceInfo */
sal_Bool SAL_CALL XMLSecurityContextGpg::supportsService( const OUString& serviceName)
    throw( RuntimeException, std::exception )
{
    return cppu::supportsService(this, serviceName);
}

/* XServiceInfo */
Sequence< OUString > SAL_CALL XMLSecurityContextGpg::getSupportedServiceNames()
    throw( RuntimeException, std::exception )
{
    return impl_getSupportedServiceNames() ;
}

//Helper for XServiceInfo
Sequence< OUString > XMLSecurityContextGpg::impl_getSupportedServiceNames()
{
    return {"com.sun.star.xml.crypto.gpg.GpgXMLSecurityContext"};
}

OUString XMLSecurityContextGpg::impl_getImplementationName()
    throw( RuntimeException )
{
    return OUString("com.sun.star.xml.security.XMLSecurityContext_Gpg");
}

//Helper for registry
Reference< XInterface > SAL_CALL XMLSecurityContextGpg::impl_createInstance( const Reference< XMultiServiceFactory >& )
    throw( RuntimeException )
{
    return Reference< XInterface >( *new XMLSecurityContextGpg ) ;
}

Reference< XSingleServiceFactory > XMLSecurityContextGpg::impl_createFactory( const Reference< XMultiServiceFactory >& aServiceManager )
{
    return ::cppu::createSingleFactory( aServiceManager , impl_getImplementationName() , impl_createInstance , impl_getSupportedServiceNames() ) ;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
