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


#include <framework/signatureverifierimpl.hxx>
#include <framework/xmlsignaturetemplateimpl.hxx>
#include <com/sun/star/xml/crypto/XXMLSignatureTemplate.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <osl/diagnose.h>
#include <rtl/ref.hxx>


SignatureVerifierImpl::SignatureVerifierImpl()
{
}

SignatureVerifierImpl::~SignatureVerifierImpl()
{
}

void SignatureVerifierImpl::notifyResultListener() const
/****** SignatureVerifierImpl/notifyResultListener ***************************
 *
 *   NAME
 *  notifyResultListener -- notifies the listener about the verify result.
 ******************************************************************************/
{
    css::uno::Reference< css::xml::crypto::sax::XSignatureVerifyResultListener >
        xSignatureVerifyResultListener ( m_xResultListener , css::uno::UNO_QUERY ) ;

    xSignatureVerifyResultListener->signatureVerified( m_nSecurityId, m_nStatus );
}

void SignatureVerifierImpl::startEngine( const rtl::Reference<XMLSignatureTemplateImpl>& xSignatureTemplate)
/****** SignatureVerifierImpl/startEngine ************************************
 *
 *   NAME
 *  startEngine -- verifies the signature.
 *
 *   INPUTS
 *  xSignatureTemplate - the signature template (along with all referenced
 *  elements) to be verified.
 ******************************************************************************/
{
    css::uno::Reference< css::xml::crypto::XXMLSignatureTemplate > xResultTemplate;
    try
    {
        xResultTemplate = m_xXMLSignature->validate(css::uno::Reference<css::xml::crypto::XXMLSignatureTemplate>(xSignatureTemplate), m_xXMLSecurityContext);
        m_nStatus = xResultTemplate->getStatus();
    }
    catch( css::uno::Exception& )
    {
        m_nStatus = css::xml::crypto::SecurityOperationStatus_RUNTIMEERROR_FAILED;
    }
}

/* XSignatureVerifyResultBroadcaster */
void SAL_CALL SignatureVerifierImpl::addSignatureVerifyResultListener(
    const css::uno::Reference< css::xml::crypto::sax::XSignatureVerifyResultListener >& listener )
{
    m_xResultListener = listener;
    tryToPerform();
}

void SAL_CALL SignatureVerifierImpl::removeSignatureVerifyResultListener(
    const css::uno::Reference< css::xml::crypto::sax::XSignatureVerifyResultListener >&)
{
}

/* XInitialization */
void SAL_CALL SignatureVerifierImpl::initialize(
    const css::uno::Sequence< css::uno::Any >& aArguments )
{
    OSL_ASSERT(aArguments.getLength() == 5);

    OUString ouTempString;

    aArguments[0] >>= ouTempString;
    m_nSecurityId = ouTempString.toInt32();
    aArguments[1] >>= m_xSAXEventKeeper;
    aArguments[2] >>= ouTempString;
    m_nIdOfTemplateEC = ouTempString.toInt32();
    aArguments[3] >>= m_xXMLSecurityContext;
    aArguments[4] >>= m_xXMLSignature;
}


OUString SignatureVerifierImpl_getImplementationName ()
{
    return u"com.sun.star.xml.security.framework.SignatureVerifierImpl"_ustr;
}

css::uno::Sequence< OUString > SignatureVerifierImpl_getSupportedServiceNames(  )
{
    return { u"com.sun.star.xml.crypto.sax.SignatureVerifier"_ustr };
}

/* XServiceInfo */
OUString SAL_CALL SignatureVerifierImpl::getImplementationName(  )
{
    return SignatureVerifierImpl_getImplementationName();
}

sal_Bool SAL_CALL SignatureVerifierImpl::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL SignatureVerifierImpl::getSupportedServiceNames(  )
{
    return SignatureVerifierImpl_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
