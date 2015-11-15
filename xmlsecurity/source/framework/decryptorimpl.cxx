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


#include "decryptorimpl.hxx"
#include <com/sun/star/xml/crypto/XXMLEncryptionTemplate.hpp>
#include <com/sun/star/xml/wrapper/XXMLElementWrapper.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <osl/diagnose.h>

using namespace com::sun::star::uno;
namespace cssu = com::sun::star::uno;
namespace cssl = com::sun::star::lang;
namespace cssxc = com::sun::star::xml::crypto;
namespace cssxw = com::sun::star::xml::wrapper;

#define IMPLEMENTATION_NAME "com.sun.star.xml.security.framework.DecryptorImpl"

DecryptorImpl::DecryptorImpl(const Reference< XComponentContext > & xContext) : DecryptorImpl_Base(xContext)
{
}

DecryptorImpl::~DecryptorImpl()
{
}

bool DecryptorImpl::checkReady() const
/****** DecryptorImpl/checkReady *********************************************
 *
 *   NAME
 *  checkReady -- checks the conditions for the decryption.
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
    return (m_xResultListener.is() && EncryptionEngine::checkReady());
}

void DecryptorImpl::notifyResultListener() const
    throw (cssu::Exception, cssu::RuntimeException)
/****** DecryptorImpl/notifyResultListener ***********************************
 *
 *   NAME
 *  notifyResultListener -- notifies the listener about the decryption
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
    cssu::Reference< cssxc::sax::XDecryptionResultListener >
        xDecryptionResultListener ( m_xResultListener , cssu::UNO_QUERY ) ;

    xDecryptionResultListener->decrypted(m_nSecurityId,m_nStatus);
}

void DecryptorImpl::startEngine( const cssu::Reference<
    cssxc::XXMLEncryptionTemplate >&
    xEncryptionTemplate)
    throw (cssu::Exception, cssu::RuntimeException)
/****** DecryptorImpl/startEngine ********************************************
 *
 *   NAME
 *  startEngine -- decrypts the encryption.
 *
 *   SYNOPSIS
 *  startEngine( xEncryptionTemplate );
 *
 *   FUNCTION
 *  decrypts the encryption element, then if succeeds, updates the link
 *  of old template element to the new encryption element in
 *  SAXEventKeeper.
 *
 *   INPUTS
 *  xEncryptionTemplate - the encryption template to be decrypted.
 *
 *   RESULT
 *  empty
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    cssu::Reference< cssxc::XXMLEncryptionTemplate > xResultTemplate;
    try
    {
        xResultTemplate = m_xXMLEncryption->decrypt(xEncryptionTemplate, m_xXMLSecurityContext);
        m_nStatus = xResultTemplate->getStatus();
    }
    catch( cssu::Exception& )
    {
        m_nStatus = cssxc::SecurityOperationStatus_RUNTIMEERROR_FAILED;
    }

    if (m_nStatus == cssxc::SecurityOperationStatus_OPERATION_SUCCEEDED)
    {
        cssu::Reference< cssxw::XXMLElementWrapper > xDecryptedElement
            = xResultTemplate->getTemplate();
        m_xSAXEventKeeper->setElement(m_nIdOfTemplateEC, xDecryptedElement);
    }
}

/* XDecryptionResultBroadcaster */
void SAL_CALL DecryptorImpl::addDecryptionResultListener( const cssu::Reference< cssxc::sax::XDecryptionResultListener >& listener )
        throw (cssu::Exception, cssu::RuntimeException, std::exception)
{
    m_xResultListener = listener;
    tryToPerform();
}

void SAL_CALL DecryptorImpl::removeDecryptionResultListener( const cssu::Reference< cssxc::sax::XDecryptionResultListener >&)
        throw (cssu::RuntimeException, std::exception)
{
}

/* XInitialization */
void SAL_CALL DecryptorImpl::initialize( const cssu::Sequence< cssu::Any >& aArguments )
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
    aArguments[4] >>= m_xXMLEncryption;
}

OUString DecryptorImpl_getImplementationName ()
    throw (cssu::RuntimeException)
{
    return OUString ( IMPLEMENTATION_NAME );
}

cssu::Sequence< OUString > SAL_CALL DecryptorImpl_getSupportedServiceNames(  )
    throw (cssu::RuntimeException)
{
    cssu::Sequence<OUString> aRet { "com.sun.star.xml.crypto.sax.Decryptor" };
    return aRet;
}

cssu::Reference< cssu::XInterface > SAL_CALL DecryptorImpl_createInstance( const cssu::Reference< cssl::XMultiServiceFactory >& xMSF)
    throw( cssu::Exception )
{
    return static_cast<cppu::OWeakObject*>(new DecryptorImpl( comphelper::getComponentContext( xMSF ) ));
}

/* XServiceInfo */
OUString SAL_CALL DecryptorImpl::getImplementationName(  )
    throw (cssu::RuntimeException, std::exception)
{
    return DecryptorImpl_getImplementationName();
}

sal_Bool SAL_CALL DecryptorImpl::supportsService( const OUString& rServiceName )
    throw (cssu::RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

cssu::Sequence< OUString > SAL_CALL DecryptorImpl::getSupportedServiceNames(  )
    throw (cssu::RuntimeException, std::exception)
{
    return DecryptorImpl_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
