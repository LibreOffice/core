/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "seinitializer_appleimpl.hxx"
#include "securityenvironment_appleimpl.hxx"

#include <com/sun/star/xml/crypto/XMLSecurityContext.hpp>
#include <rtl/ref.hxx>
#include <sal/log.hxx>

#include <cert.h>

using namespace css;

SEInitializer_AppleImpl::SEInitializer_AppleImpl(
    const uno::Reference<cpo::uno::XComponentContext>& rxContext)
    : SEInitializer_NssImpl(rxContext)
{
}

SEInitializer_AppleImpl::~SEInitializer_AppleImpl() {}

uno::Reference<xml::crypto::XXMLSecurityContext>
SEInitializer_AppleImpl::createSecurityContext(const OUString&)
{
    SAL_INFO("xmlsecurity.xmlsec", "SEInitializer_AppleImpl: creating security context");

    // NSS still provides the XML-DSig engine and certificate parsing.
    if (!initNSS(m_xContext))
        return nullptr;

    CERTCertDBHandle* pCertHandle = CERT_GetDefaultCertDB();

    try
    {
        uno::Reference<xml::crypto::XXMLSecurityContext> xSecCtx
            = xml::crypto::XMLSecurityContext::create(m_xContext);

        rtl::Reference<SecurityEnvironment_AppleImpl> xSecEnv(new SecurityEnvironment_AppleImpl);
        xSecEnv->setCertDb(pCertHandle);

        sal_Int32 n = xSecCtx->addSecurityEnvironment(xSecEnv);
        xSecCtx->setDefaultSecurityEnvironmentIndex(n);
        return xSecCtx;
    }
    catch (const cpo::uno::Exception&)
    {
        return nullptr;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
