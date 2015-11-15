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


#include "encryptorimpl.hxx"
#include <com/sun/star/xml/crypto/XXMLEncryptionTemplate.hpp>
#include <com/sun/star/xml/wrapper/XXMLElementWrapper.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <osl/diagnose.h>

using namespace com::sun::star::uno;
namespace cssl = com::sun::star::lang;
namespace cssxc = com::sun::star::xml::crypto;
namespace cssxw = com::sun::star::xml::wrapper;

#define IMPLEMENTATION_NAME "com.sun.star.xml.security.framework.EncryptorImpl"

EncryptorImpl::EncryptorImpl(const Reference<XComponentContext> & xContext) : EncryptorImpl_Base(xContext)
{
    m_nReferenceId = -1;
}

EncryptorImpl::~EncryptorImpl()
{
}

bool EncryptorImpl::checkReady() const
/****** EncryptorImpl/checkReady *********************************************
 *
 *   NAME
 *  checkReady -- checks the conditions for the encryption.
 *
 *   SYNOPSIS
 *  bReady = checkReady( );
 *
 *   FUNCTION
 *  checks whether all following conditions are satisfied:
 *  1. the result listener is ready;
 *  2. the EncryptionEngine is ready.
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
    sal_Int32 nKeyInc = 0;
    if (m_nIdOfKeyEC != 0)
    {
        nKeyInc = 1;
    }

    return (m_xResultListener.is() &&
        (m_nReferenceId != -1) &&
        (2+nKeyInc == m_nNumOfResolvedReferences) &&
        EncryptionEngine::checkReady());
}

void EncryptorImpl::notifyResultListener() const
    throw (Exception, RuntimeException)
/****** DecryptorImpl/notifyResultListener ***********************************
 *
 *   NAME
 *  notifyResultListener -- notifies the listener about the encryption
 *  result.
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
    Reference< cssxc::sax::XEncryptionResultListener >
        xEncryptionResultListener ( m_xResultListener , UNO_QUERY ) ;

    xEncryptionResultListener->encrypted( m_nSecurityId, m_nStatus );
}

void EncryptorImpl::startEngine( const Reference<
    cssxc::XXMLEncryptionTemplate >&
    xEncryptionTemplate)
        throw (Exception, RuntimeException)
/****** EncryptorImpl/startEngine ********************************************
 *
 *   NAME
 *  startEngine -- generates the encryption.
 *
 *   SYNOPSIS
 *  startEngine( xEncryptionTemplate );
 *
 *   FUNCTION
 *  generates the encryption element, then if succeeds, updates the link
 *  of old template element to the new encryption element in
 *  SAXEventKeeper.
 *
 *   INPUTS
 *  xEncryptionTemplate - the encryption template to be encrypted.
 *
 *   RESULT
 *  empty
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    Reference < cssxc::XXMLEncryptionTemplate > xResultTemplate;

    Reference< cssxw::XXMLElementWrapper >
        xXMLElement = m_xSAXEventKeeper->getElement( m_nReferenceId );
    xEncryptionTemplate->setTarget(xXMLElement);

    try
    {
        xResultTemplate = m_xXMLEncryption->encrypt(
            xEncryptionTemplate, m_xSecurityEnvironment);
        m_nStatus = xResultTemplate->getStatus();
    }
    catch( Exception& )
    {
        m_nStatus = cssxc::SecurityOperationStatus_RUNTIMEERROR_FAILED;
    }

    if (m_nStatus == cssxc::SecurityOperationStatus_OPERATION_SUCCEEDED)
    {
        Reference < cssxw::XXMLElementWrapper > xResultEncryption
            = xResultTemplate->getTemplate();
        m_xSAXEventKeeper->setElement(m_nIdOfTemplateEC, xResultEncryption);
        m_xSAXEventKeeper->setElement(m_nReferenceId, nullptr);
    }
}

/* XReferenceCollector */
void SAL_CALL EncryptorImpl::setReferenceCount(sal_Int32)
    throw (Exception, RuntimeException, std::exception)
{
    /*
     * dummy method, because there is only one reference in
     * encryption, different from signature.
     * so the referenceNumber is always 1
     */
}

void SAL_CALL EncryptorImpl::setReferenceId( sal_Int32 id )
    throw (Exception, RuntimeException, std::exception)
{
    m_nReferenceId = id;
}

/* XEncryptionResultBroadcaster */
void SAL_CALL EncryptorImpl::addEncryptionResultListener( const Reference< cssxc::sax::XEncryptionResultListener >& listener )
        throw (Exception, RuntimeException, std::exception)
{
    m_xResultListener = listener;
    tryToPerform();
}

void SAL_CALL EncryptorImpl::removeEncryptionResultListener( const Reference< cssxc::sax::XEncryptionResultListener >&)
        throw (RuntimeException, std::exception)
{
}

/* XInitialization */
void SAL_CALL EncryptorImpl::initialize( const Sequence< Any >& aArguments )
    throw (Exception, RuntimeException, std::exception)
{
    OSL_ASSERT(aArguments.getLength() == 5);

    OUString ouTempString;

    aArguments[0] >>= ouTempString;
    m_nSecurityId = ouTempString.toInt32();
    aArguments[1] >>= m_xSAXEventKeeper;
    aArguments[2] >>= ouTempString;
    m_nIdOfTemplateEC = ouTempString.toInt32();
    aArguments[3] >>= m_xSecurityEnvironment;
    aArguments[4] >>= m_xXMLEncryption;
}


OUString EncryptorImpl_getImplementationName ()
    throw (RuntimeException)
{
    return OUString ( IMPLEMENTATION_NAME );
}

Sequence< OUString > SAL_CALL EncryptorImpl_getSupportedServiceNames(  )
    throw (RuntimeException)
{
    Sequence<OUString> aRet { "com.sun.star.xml.crypto.sax.Encryptor" };
    return aRet;
}

Reference< XInterface > SAL_CALL EncryptorImpl_createInstance(
    const Reference< cssl::XMultiServiceFactory >& xMSF)
    throw( Exception )
{
    return static_cast<cppu::OWeakObject*>(new EncryptorImpl( comphelper::getComponentContext( xMSF ) ));
}

/* XServiceInfo */
OUString SAL_CALL EncryptorImpl::getImplementationName(  )
    throw (RuntimeException, std::exception)
{
    return EncryptorImpl_getImplementationName();
}

sal_Bool SAL_CALL EncryptorImpl::supportsService( const OUString& rServiceName )
    throw (RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SAL_CALL EncryptorImpl::getSupportedServiceNames(  )
    throw (RuntimeException, std::exception)
{
    return EncryptorImpl_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
