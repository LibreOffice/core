/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "XMLEncryption.hxx"

#include <cppuhelper/supportsservice.hxx>

using namespace css::uno;
using namespace css::lang;
using namespace css::xml::wrapper;
using namespace css::xml::crypto;

XMLEncryptionGpg::XMLEncryptionGpg() {
}

XMLEncryptionGpg::~XMLEncryptionGpg() {
}

/* XXMLEncryption */
Reference< XXMLEncryptionTemplate > SAL_CALL XMLEncryptionGpg::encrypt(const Reference< XXMLEncryptionTemplate >& /*aTemplate*/,
                                                                    const Reference< XSecurityEnvironment >& /*aEnvironment*/)
    throw (XMLEncryptionException, SecurityException, RuntimeException, std::exception)
{
    return nullptr;
}

/* XXMLEncryption */
Reference< XXMLEncryptionTemplate > SAL_CALL XMLEncryptionGpg::decrypt(const Reference< XXMLEncryptionTemplate >& /*aTemplate*/,
                                                                    const Reference< XXMLSecurityContext >& /*aSecurityCtx*/)
    throw (XMLEncryptionException, SecurityException, RuntimeException, std::exception)
{
    return nullptr;
}

/* XServiceInfo */
OUString SAL_CALL XMLEncryptionGpg::getImplementationName()
    throw( RuntimeException, std::exception )
{
    return impl_getImplementationName() ;
}

/* XServiceInfo */
sal_Bool SAL_CALL XMLEncryptionGpg::supportsService( const OUString& serviceName)
    throw( RuntimeException, std::exception )
{
    return cppu::supportsService(this, serviceName);
}

/* XServiceInfo */
Sequence< OUString > SAL_CALL XMLEncryptionGpg::getSupportedServiceNames()
    throw( RuntimeException, std::exception )
{
    return impl_getSupportedServiceNames() ;
}

//Helper for XServiceInfo
Sequence< OUString > XMLEncryptionGpg::impl_getSupportedServiceNames()
{
    return {"com.sun.star.xml.crypto.gpg.GpgXMLEncryption"};
}

OUString XMLEncryptionGpg::impl_getImplementationName()
    throw( RuntimeException )
{
    return OUString("com.sun.star.xml.security.XMLEncryption_Gpg");
}

//Helper for registry
Reference< XInterface > SAL_CALL XMLEncryptionGpg::impl_createInstance( const Reference< XMultiServiceFactory >&  )
    throw( RuntimeException )
{
    return Reference< XInterface >(*new XMLEncryptionGpg);
}

Reference< XSingleServiceFactory > XMLEncryptionGpg::impl_createFactory( const Reference< XMultiServiceFactory >& aServiceManager )
{
    return ::cppu::createSingleFactory(aServiceManager, impl_getImplementationName(), impl_createInstance, impl_getSupportedServiceNames());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
