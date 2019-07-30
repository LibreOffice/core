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

namespace cssu = com::sun::star::uno;
namespace cssl = com::sun::star::lang;
namespace cssxc = com::sun::star::xml::crypto;

#define IMPLEMENTATION_NAME "com.sun.star.xml.security.framework.SignatureVerifierImpl"

SignatureVerifierImpl::SignatureVerifierImpl()
  : SignatureVerifierImpl_Base()
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
    cssu::Reference< cssxc::sax::XSignatureVerifyResultListener >
        xSignatureVerifyResultListener ( m_xResultListener , cssu::UNO_QUERY ) ;

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
    cssu::Reference< cssxc::XXMLSignatureTemplate > xResultTemplate;
    try
    {
        xResultTemplate = m_xXMLSignature->validate(css::uno::Reference<css::xml::crypto::XXMLSignatureTemplate>(xSignatureTemplate.get()), m_xXMLSecurityContext);
        m_nStatus = xResultTemplate->getStatus();
    }
    catch( cssu::Exception& )
    {
        m_nStatus = cssxc::SecurityOperationStatus_RUNTIMEERROR_FAILED;
    }
}

/* XSignatureVerifyResultBroadcaster */
void SAL_CALL SignatureVerifierImpl::addSignatureVerifyResultListener(
    const cssu::Reference< cssxc::sax::XSignatureVerifyResultListener >& listener )
{
    m_xResultListener = listener;
    tryToPerform();
}

void SAL_CALL SignatureVerifierImpl::removeSignatureVerifyResultListener(
    const cssu::Reference< cssxc::sax::XSignatureVerifyResultListener >&)
{
}

/* XInitialization */
void SAL_CALL SignatureVerifierImpl::initialize(
    const cssu::Sequence< cssu::Any >& aArguments )
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
    return IMPLEMENTATION_NAME;
}

cssu::Sequence< OUString > SignatureVerifierImpl_getSupportedServiceNames(  )
{
    cssu::Sequence<OUString> aRet { "com.sun.star.xml.crypto.sax.SignatureVerifier" };
    return aRet;
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

cssu::Sequence< OUString > SAL_CALL SignatureVerifierImpl::getSupportedServiceNames(  )
{
    return SignatureVerifierImpl_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
