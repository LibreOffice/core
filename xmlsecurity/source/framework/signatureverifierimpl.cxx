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


#include "signatureverifierimpl.hxx"
#include <com/sun/star/xml/crypto/XXMLSignatureTemplate.hpp>
#include <com/sun/star/xml/wrapper/XXMLElementWrapper.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <osl/diagnose.h>

namespace cssu = com::sun::star::uno;
namespace cssl = com::sun::star::lang;
namespace cssxc = com::sun::star::xml::crypto;

#define IMPLEMENTATION_NAME "com.sun.star.xml.security.framework.SignatureVerifierImpl"

SignatureVerifierImpl::SignatureVerifierImpl( const cssu::Reference< css::uno::XComponentContext >& xContext)
  : SignatureVerifierImpl_Base(xContext)
{
}

SignatureVerifierImpl::~SignatureVerifierImpl()
{
}

bool SignatureVerifierImpl::checkReady() const
/****** SignatureVerifierImpl/checkReady *************************************
 *
 *   NAME
 *  checkReady -- checks the conditions for the signature verification.
 *
 *   SYNOPSIS
 *  bReady = checkReady( );
 *
 *   FUNCTION
 *  checks whether all following conditions are satisfied:
 *  1. the result listener is ready;
 *  2. the SignatureEngine is ready.
 *
 *   INPUTS
 *  empty
 *
 *   RESULT
 *  bReady - true if all conditions are satisfied, false otherwise
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    return (m_xResultListener.is() && SignatureEngine::checkReady());
}

void SignatureVerifierImpl::notifyResultListener() const
    throw (cssu::Exception, cssu::RuntimeException)
/****** SignatureVerifierImpl/notifyResultListener ***************************
 *
 *   NAME
 *  notifyResultListener -- notifies the listener about the verify result.
 *
 *   SYNOPSIS
 *  notifyResultListener( );
 *
 *   FUNCTION
 *  see NAME.
 *
 *   INPUTS
 *  empty
 *
 *   RESULT
 *  empty
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    cssu::Reference< cssxc::sax::XSignatureVerifyResultListener >
        xSignatureVerifyResultListener ( m_xResultListener , cssu::UNO_QUERY ) ;

    xSignatureVerifyResultListener->signatureVerified( m_nSecurityId, m_nStatus );
}

void SignatureVerifierImpl::startEngine( const cssu::Reference<
    cssxc::XXMLSignatureTemplate >&
    xSignatureTemplate)
    throw (cssu::Exception, cssu::RuntimeException)
/****** SignatureVerifierImpl/startEngine ************************************
 *
 *   NAME
 *  startEngine -- verifies the signature.
 *
 *   SYNOPSIS
 *  startEngine( xSignatureTemplate );
 *
 *   FUNCTION
 *  see NAME.
 *
 *   INPUTS
 *  xSignatureTemplate - the signature template (along with all referenced
 *  elements) to be verified.
 *
 *   RESULT
 *  empty
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    cssu::Reference< cssxc::XXMLSignatureTemplate > xResultTemplate;
    try
    {
        xResultTemplate = m_xXMLSignature->validate(xSignatureTemplate, m_xXMLSecurityContext);
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
    throw (cssu::Exception, cssu::RuntimeException, std::exception)
{
    m_xResultListener = listener;
    tryToPerform();
}

void SAL_CALL SignatureVerifierImpl::removeSignatureVerifyResultListener(
    const cssu::Reference< cssxc::sax::XSignatureVerifyResultListener >&)
    throw (cssu::RuntimeException, std::exception)
{
}

/* XInitialization */
void SAL_CALL SignatureVerifierImpl::initialize(
    const cssu::Sequence< cssu::Any >& aArguments )
    throw (cssu::Exception, cssu::RuntimeException, std::exception)
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
    throw (cssu::RuntimeException)
{
    return OUString( IMPLEMENTATION_NAME );
}

cssu::Sequence< OUString > SAL_CALL SignatureVerifierImpl_getSupportedServiceNames(  )
    throw (cssu::RuntimeException)
{
    cssu::Sequence<OUString> aRet { "com.sun.star.xml.crypto.sax.SignatureVerifier" };
    return aRet;
}

cssu::Reference< cssu::XInterface > SAL_CALL SignatureVerifierImpl_createInstance(
    const cssu::Reference< cssl::XMultiServiceFactory >& rSMgr)
    throw( cssu::Exception )
{
    return static_cast<cppu::OWeakObject*>(new SignatureVerifierImpl( comphelper::getComponentContext(rSMgr) ));
}

/* XServiceInfo */
OUString SAL_CALL SignatureVerifierImpl::getImplementationName(  )
    throw (cssu::RuntimeException, std::exception)
{
    return SignatureVerifierImpl_getImplementationName();
}

sal_Bool SAL_CALL SignatureVerifierImpl::supportsService( const OUString& rServiceName )
    throw (cssu::RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

cssu::Sequence< OUString > SAL_CALL SignatureVerifierImpl::getSupportedServiceNames(  )
    throw (cssu::RuntimeException, std::exception)
{
    return SignatureVerifierImpl_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
