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

namespace cssu = com::sun::star::uno;
namespace cssl = com::sun::star::lang;
namespace cssxc = com::sun::star::xml::crypto;
namespace cssxw = com::sun::star::xml::wrapper;

#define SERVICE_NAME "com.sun.star.xml.crypto.sax.Encryptor"
#define IMPLEMENTATION_NAME "com.sun.star.xml.security.framework.EncryptorImpl"

EncryptorImpl::EncryptorImpl( const cssu::Reference< cssl::XMultiServiceFactory >& rxMSF)
{
    m_nReferenceId = -1;
    mxMSF = rxMSF;
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
    throw (cssu::Exception, cssu::RuntimeException)
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
    cssu::Reference< cssxc::sax::XEncryptionResultListener >
        xEncryptionResultListener ( m_xResultListener , cssu::UNO_QUERY ) ;

    xEncryptionResultListener->encrypted( m_nSecurityId, m_nStatus );
}

void EncryptorImpl::startEngine( const cssu::Reference<
    cssxc::XXMLEncryptionTemplate >&
    xEncryptionTemplate)
        throw (cssu::Exception, cssu::RuntimeException)
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
    cssu::Reference < cssxc::XXMLEncryptionTemplate > xResultTemplate;

    cssu::Reference< cssxw::XXMLElementWrapper >
        xXMLElement = m_xSAXEventKeeper->getElement( m_nReferenceId );
    xEncryptionTemplate->setTarget(xXMLElement);

    try
    {
        xResultTemplate = m_xXMLEncryption->encrypt(
            xEncryptionTemplate, m_xSecurityEnvironment);
        m_nStatus = xResultTemplate->getStatus();
    }
    catch( cssu::Exception& )
    {
        m_nStatus = cssxc::SecurityOperationStatus_RUNTIMEERROR_FAILED;
    }

    if (m_nStatus == cssxc::SecurityOperationStatus_OPERATION_SUCCEEDED)
    {
        cssu::Reference < cssxw::XXMLElementWrapper > xResultEncryption
            = xResultTemplate->getTemplate();
        m_xSAXEventKeeper->setElement(m_nIdOfTemplateEC, xResultEncryption);
        m_xSAXEventKeeper->setElement(m_nReferenceId, NULL);
    }
}

/* XReferenceCollector */
void SAL_CALL EncryptorImpl::setReferenceCount(sal_Int32)
    throw (cssu::Exception, cssu::RuntimeException)
{
    /*
     * dummp method, because there is only one reference in
     * encryption, different from signature.
     * so the referenceNumber is always 1
     */
}

void SAL_CALL EncryptorImpl::setReferenceId( sal_Int32 id )
    throw (cssu::Exception, cssu::RuntimeException)
{
    m_nReferenceId = id;
}

/* XEncryptionResultBroadcaster */
void SAL_CALL EncryptorImpl::addEncryptionResultListener( const cssu::Reference< cssxc::sax::XEncryptionResultListener >& listener )
        throw (cssu::Exception, cssu::RuntimeException)
{
    m_xResultListener = listener;
    tryToPerform();
}

void SAL_CALL EncryptorImpl::removeEncryptionResultListener( const cssu::Reference< cssxc::sax::XEncryptionResultListener >&)
        throw (cssu::RuntimeException)
{
}

/* XInitialization */
void SAL_CALL EncryptorImpl::initialize( const cssu::Sequence< cssu::Any >& aArguments )
    throw (cssu::Exception, cssu::RuntimeException)
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
    throw (cssu::RuntimeException)
{
    return OUString ( RTL_CONSTASCII_USTRINGPARAM ( IMPLEMENTATION_NAME ) );
}

sal_Bool SAL_CALL EncryptorImpl_supportsService( const OUString& ServiceName )
    throw (cssu::RuntimeException)
{
    return ServiceName == SERVICE_NAME;
}

cssu::Sequence< OUString > SAL_CALL EncryptorImpl_getSupportedServiceNames(  )
    throw (cssu::RuntimeException)
{
    cssu::Sequence < OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] =  OUString ( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );
    return aRet;
}
#undef SERVICE_NAME

cssu::Reference< cssu::XInterface > SAL_CALL EncryptorImpl_createInstance(
    const cssu::Reference< cssl::XMultiServiceFactory >& rSMgr)
    throw( cssu::Exception )
{
    return (cppu::OWeakObject*) new EncryptorImpl(rSMgr);
}

/* XServiceInfo */
OUString SAL_CALL EncryptorImpl::getImplementationName(  )
    throw (cssu::RuntimeException)
{
    return EncryptorImpl_getImplementationName();
}
sal_Bool SAL_CALL EncryptorImpl::supportsService( const OUString& rServiceName )
    throw (cssu::RuntimeException)
{
    return EncryptorImpl_supportsService( rServiceName );
}
cssu::Sequence< OUString > SAL_CALL EncryptorImpl::getSupportedServiceNames(  )
    throw (cssu::RuntimeException)
{
    return EncryptorImpl_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
