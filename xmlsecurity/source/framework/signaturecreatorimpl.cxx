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


#include <framework/signaturecreatorimpl.hxx>
#include <framework/xmlsignaturetemplateimpl.hxx>
#include <com/sun/star/xml/crypto/XXMLSignatureTemplate.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <osl/diagnose.h>
#include <rtl/ref.hxx>

namespace com::sun::star::xml::wrapper { class XXMLElementWrapper; }

using namespace com::sun::star::uno;

SignatureCreatorImpl::SignatureCreatorImpl()
    : SignatureCreatorImpl_Base(), m_nIdOfBlocker(-1)
{
}

SignatureCreatorImpl::~SignatureCreatorImpl( )
{
}

void SignatureCreatorImpl::notifyResultListener() const
/****** SignatureCreatorImpl/notifyResultListener *****************************
 *
 *   NAME
 *  notifyResultListener -- notifies the listener about the signature
 *  creation result.
 ******************************************************************************/
{
    css::uno::Reference< css::xml::crypto::sax::XSignatureCreationResultListener >
        xSignatureCreationResultListener ( m_xResultListener , css::uno::UNO_QUERY ) ;

    xSignatureCreationResultListener->signatureCreated( m_nSecurityId, m_nStatus );
}

void SignatureCreatorImpl::startEngine(const rtl::Reference<XMLSignatureTemplateImpl>& xSignatureTemplate)
/****** SignatureCreatorImpl/startEngine *************************************
 *
 *   NAME
 *  startEngine -- generates the signature.
 *
 *   FUNCTION
 *  generates the signature element, then if succeeds, updates the link
 *  of old template element to the new signature element in
 *  SAXEventKeeper.
 *
 *   INPUTS
 *  xSignatureTemplate - the signature template (along with all referenced
 *  elements) to be signed.
 ******************************************************************************/
{
    css::uno::Reference< css::xml::crypto::XXMLSignatureTemplate > xResultTemplate;
    try
    {
        xResultTemplate = m_xXMLSignature->generate(css::uno::Reference<css::xml::crypto::XXMLSignatureTemplate>(xSignatureTemplate), m_xSecurityEnvironment);
        m_nStatus = xResultTemplate->getStatus();
    }
    catch( css::uno::Exception& )
    {
        m_nStatus = css::xml::crypto::SecurityOperationStatus_RUNTIMEERROR_FAILED;
    }

    if (m_nStatus == css::xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED)
    {
        css::uno::Reference < css::xml::wrapper::XXMLElementWrapper > xResultSignature = xResultTemplate->getTemplate();
        m_xSAXEventKeeper->setElement(m_nIdOfTemplateEC, xResultSignature);
    }
}

void SignatureCreatorImpl::clearUp() const
/****** SignatureCreatorImpl/clearUp *****************************************
 *
 *   NAME
 *  clearUp -- clear up all resources used by the signature generation.
 *
 *   SYNOPSIS
 *  clearUp( );
 *
 *   FUNCTION
 *  cleaning resources up includes:
 *  1. SignatureEngine's clearing up;
 *  2. releases the Blocker for the signature template element.
 ******************************************************************************/
{
    SignatureEngine::clearUp();

    if (m_nIdOfBlocker != -1)
    {
        m_xSAXEventKeeper->removeBlocker(m_nIdOfBlocker);
    }
}

/* XBlockerMonitor */
void SAL_CALL SignatureCreatorImpl::setBlockerId( sal_Int32 id )
{
    m_nIdOfBlocker = id;
    tryToPerform();
}

/* XSignatureCreationResultBroadcaster */
void SAL_CALL SignatureCreatorImpl::addSignatureCreationResultListener(
    const css::uno::Reference< css::xml::crypto::sax::XSignatureCreationResultListener >& listener )
{
    m_xResultListener = listener;
    tryToPerform();
}

void SAL_CALL SignatureCreatorImpl::removeSignatureCreationResultListener(
    const css::uno::Reference< css::xml::crypto::sax::XSignatureCreationResultListener >&)
{
}

/* XInitialization */
void SAL_CALL SignatureCreatorImpl::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
{
    OSL_ASSERT(aArguments.getLength() == 5);

    OUString ouTempString;

    aArguments[0] >>= ouTempString;
    m_nSecurityId = ouTempString.toInt32();
    aArguments[1] >>= m_xSAXEventKeeper;
    aArguments[2] >>= ouTempString;
    m_nIdOfTemplateEC = ouTempString.toInt32();
    aArguments[3] >>= m_xSecurityEnvironment;
    aArguments[4] >>= m_xXMLSignature;
}


OUString SignatureCreatorImpl_getImplementationName ()
{
    return "com.sun.star.xml.security.framework.SignatureCreatorImpl";
}

css::uno::Sequence< OUString > SignatureCreatorImpl_getSupportedServiceNames(  )
{
    css::uno::Sequence<OUString> aRet { "com.sun.star.xml.crypto.sax.SignatureCreator" };
    return aRet;
}

/* XServiceInfo */
OUString SAL_CALL SignatureCreatorImpl::getImplementationName(  )
{
    return SignatureCreatorImpl_getImplementationName();
}

sal_Bool SAL_CALL SignatureCreatorImpl::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL SignatureCreatorImpl::getSupportedServiceNames(  )
{
    return SignatureCreatorImpl_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
