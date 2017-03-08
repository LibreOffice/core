/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "XMLSignature.hxx"
#include "SecurityEnvironment.hxx"
#include "XMLSecurityContext.hxx"

#include <cppuhelper/supportsservice.hxx>

using namespace css::uno;
using namespace css::lang;
using namespace css::xml::wrapper;
using namespace css::xml::crypto;

XMLSignatureGpg::XMLSignatureGpg()
{
}

XMLSignatureGpg::~XMLSignatureGpg()
{
}

/* XXMLSignature */
Reference< XXMLSignatureTemplate > SAL_CALL XMLSignatureGpg::generate(const Reference< XXMLSignatureTemplate >& /*aTemplate*/,
                                                                   const Reference< XSecurityEnvironment >& /*aEnvironment*/)
{
    return nullptr;
}

/* XXMLSignature */
Reference< XXMLSignatureTemplate > SAL_CALL XMLSignatureGpg::validate(const Reference< XXMLSignatureTemplate >& /*aTemplate*/,
                                                                   const Reference< XXMLSecurityContext >& /*aSecurityCtx*/)
{
    return nullptr;
}

/* XServiceInfo */
OUString SAL_CALL XMLSignatureGpg::getImplementationName()
{
    return impl_getImplementationName() ;
}

/* XServiceInfo */
sal_Bool SAL_CALL XMLSignatureGpg::supportsService( const OUString& serviceName)
{
    return cppu::supportsService(this, serviceName);
}

/* XServiceInfo */
Sequence< OUString > SAL_CALL XMLSignatureGpg::getSupportedServiceNames()
{
    return impl_getSupportedServiceNames() ;
}

//Helper for XServiceInfo
Sequence< OUString > XMLSignatureGpg::impl_getSupportedServiceNames()
{
    return {"com.sun.star.xml.crypto.gpg.GpgXMLSignature"};
}

OUString XMLSignatureGpg::impl_getImplementationName()
{
    return OUString("com.sun.star.xml.security.XMLSignature_Gpg");
}

//Helper for registry
Reference< XInterface > SAL_CALL XMLSignatureGpg::impl_createInstance( const Reference< XMultiServiceFactory >& )
{
    return Reference< XInterface >( *new XMLSignatureGpg ) ;
}

Reference< XSingleServiceFactory > XMLSignatureGpg::impl_createFactory( const Reference< XMultiServiceFactory >& aServiceManager )
{
    return cppu::createSingleFactory( aServiceManager , impl_getImplementationName() , impl_createInstance , impl_getSupportedServiceNames() ) ;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
