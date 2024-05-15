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

#include <sal/types.h>
#include <com/sun/star/xml/crypto/SecurityEnvironment.hpp>
#include <com/sun/star/xml/crypto/XMLSecurityContext.hpp>
#include <cppuhelper/supportsservice.hxx>

#include "seinitializer_nssimpl.hxx"
#include "securityenvironment_nssimpl.hxx"

#include <cert.h>


using namespace com::sun::star;


SEInitializer_NssImpl::SEInitializer_NssImpl( const css::uno::Reference< css::uno::XComponentContext > &rxContext )
{
    m_xContext = rxContext;
}

SEInitializer_NssImpl::~SEInitializer_NssImpl()
{
}

/* XSEInitializer */
uno::Reference< css::xml::crypto::XXMLSecurityContext > SAL_CALL
    SEInitializer_NssImpl::createSecurityContext( const OUString& )
{
    CERTCertDBHandle    *pCertHandle = nullptr ;

    if( !initNSS( m_xContext ) )
        return nullptr;

    pCertHandle = CERT_GetDefaultCertDB() ;

    try
    {
        /* Build XML Security Context */
        uno::Reference< css::xml::crypto::XXMLSecurityContext > xSecCtx = css::xml::crypto::XMLSecurityContext::create( m_xContext );

        uno::Reference< css::xml::crypto::XSecurityEnvironment > xSecEnv = css::xml::crypto::SecurityEnvironment::create( m_xContext );
        SecurityEnvironment_NssImpl* pSecEnv = dynamic_cast<SecurityEnvironment_NssImpl*>(xSecEnv.get());
        assert(pSecEnv && "can only succeed");
        pSecEnv->setCertDb(pCertHandle);

        sal_Int32 n = xSecCtx->addSecurityEnvironment(xSecEnv);
        //originally the SecurityEnvironment with the internal slot was set as default
        xSecCtx->setDefaultSecurityEnvironmentIndex( n );
        return xSecCtx;
    }
    catch( const uno::Exception& )
    {
        //PK11_LogoutAll();
        //NSS_Shutdown();
        return nullptr;
    }
}

void SAL_CALL SEInitializer_NssImpl::freeSecurityContext( const uno::Reference< css::xml::crypto::XXMLSecurityContext >& )
{
    /*
     * because the security context will free all its content when it
     * is destructed, so here no free process for the security context
     * is needed.
     */
    //PK11_LogoutAll();
    //NSS_Shutdown();
}

/* XServiceInfo */
OUString SAL_CALL SEInitializer_NssImpl::getImplementationName(  )
{
    return u"com.sun.star.xml.crypto.SEInitializer"_ustr;
}
sal_Bool SAL_CALL SEInitializer_NssImpl::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService( this, rServiceName );
}
uno::Sequence< OUString > SAL_CALL SEInitializer_NssImpl::getSupportedServiceNames(  )
{
    return { u"com.sun.star.xml.crypto.SEInitializer"_ustr };
}

namespace {

class NSSInitializer_NssImpl : public SEInitializer_NssImpl
{
public:
    explicit NSSInitializer_NssImpl(const uno::Reference<uno::XComponentContext>& xContext);
    OUString SAL_CALL getImplementationName() override;
    uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};

}

NSSInitializer_NssImpl::NSSInitializer_NssImpl(const uno::Reference<uno::XComponentContext>& xContext)
    : SEInitializer_NssImpl(xContext)
{
}

OUString NSSInitializer_NssImpl::getImplementationName()
{
    return u"com.sun.star.xml.crypto.NSSInitializer"_ustr;
}

uno::Sequence<OUString> SAL_CALL NSSInitializer_NssImpl::getSupportedServiceNames()
{
    return { u"com.sun.star.xml.crypto.NSSInitializer"_ustr };
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_xml_crypto_NSSInitializer_get_implementation(
    uno::XComponentContext* pCtx, uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new NSSInitializer_NssImpl(pCtx));
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_xml_crypto_SEInitializer_get_implementation(
    uno::XComponentContext* pCtx, uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new SEInitializer_NssImpl(pCtx));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
