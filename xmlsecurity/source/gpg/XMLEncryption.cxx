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

XMLEncryption::XMLEncryption() {
}

XMLEncryption::~XMLEncryption() {
}

/* XXMLEncryption */
Reference< XXMLEncryptionTemplate > SAL_CALL XMLEncryption::encrypt(const Reference< XXMLEncryptionTemplate >& aTemplate,
                                                                    const Reference< XSecurityEnvironment >& aEnvironment)
    throw (XMLEncryptionException, SecurityException, RuntimeException, std::exception)
{
}

/* XXMLEncryption */
Reference< XXMLEncryptionTemplate > SAL_CALL XMLEncryption::decrypt(const Reference< XXMLEncryptionTemplate >& aTemplate,
                                                                    const Reference< XXMLSecurityContext >& aSecurityCtx)
    throw (XMLEncryptionException, SecurityException, RuntimeException, std::exception)
{
}

/* XServiceInfo */
OUString SAL_CALL XMLEncryption::getImplementationName() throw( RuntimeException, std::exception )
{
    return impl_getImplementationName() ;
}

/* XServiceInfo */
sal_Bool SAL_CALL XMLEncryption::supportsService( const OUString& serviceName) throw( RuntimeException, std::exception )
{
    return cppu::supportsService(this, serviceName);
}

/* XServiceInfo */
Sequence< OUString > SAL_CALL XMLEncryption::getSupportedServiceNames() throw( RuntimeException, std::exception )
{
    return impl_getSupportedServiceNames() ;
}

//Helper for XServiceInfo
Sequence< OUString > XMLEncryption::impl_getSupportedServiceNames()
{
    ::osl::Guard< ::osl::Mutex > aGuard(osl::Mutex::getGlobalMutex());
    Sequence<OUString> seqServiceNames { "com.sun.star.xml.crypto.XMLEncryption" };
    return seqServiceNames;
}

OUString XMLEncryption::impl_getImplementationName() throw( RuntimeException )
{
    return OUString("com.sun.star.xml.security.XMLEncryption_Gpg");
}

//Helper for registry
Reference< XInterface > SAL_CALL XMLEncryption::impl_createInstance( const Reference< XMultiServiceFactory >&  ) throw( RuntimeException )
{
    return Reference< XInterface >(*new XMLEncryption);
}

Reference< XSingleServiceFactory > XMLEncryption::impl_createFactory( const Reference< XMultiServiceFactory >& aServiceManager )
{
    return ::cppu::createSingleFactory(aServiceManager , impl_getImplementationName() , impl_createInstance , impl_getSupportedServiceNames());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
