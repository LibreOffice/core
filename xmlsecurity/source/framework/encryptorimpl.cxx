/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmlsecurity.hxx"

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

#define DECLARE_ASCII( SASCIIVALUE )                                                                            \
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SASCIIVALUE ) )

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

    rtl::OUString ouTempString;

    aArguments[0] >>= ouTempString;
    m_nSecurityId = ouTempString.toInt32();
    aArguments[1] >>= m_xSAXEventKeeper;
    aArguments[2] >>= ouTempString;
    m_nIdOfTemplateEC = ouTempString.toInt32();
    aArguments[3] >>= m_xSecurityEnvironment;
    aArguments[4] >>= m_xXMLEncryption;
}


rtl::OUString EncryptorImpl_getImplementationName ()
    throw (cssu::RuntimeException)
{
    return rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( IMPLEMENTATION_NAME ) );
}

sal_Bool SAL_CALL EncryptorImpl_supportsService( const rtl::OUString& ServiceName )
    throw (cssu::RuntimeException)
{
    return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME ));
}

cssu::Sequence< rtl::OUString > SAL_CALL EncryptorImpl_getSupportedServiceNames(  )
    throw (cssu::RuntimeException)
{
    cssu::Sequence < rtl::OUString > aRet(1);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] =  rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );
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
rtl::OUString SAL_CALL EncryptorImpl::getImplementationName(  )
    throw (cssu::RuntimeException)
{
    return EncryptorImpl_getImplementationName();
}
sal_Bool SAL_CALL EncryptorImpl::supportsService( const rtl::OUString& rServiceName )
    throw (cssu::RuntimeException)
{
    return EncryptorImpl_supportsService( rServiceName );
}
cssu::Sequence< rtl::OUString > SAL_CALL EncryptorImpl::getSupportedServiceNames(  )
    throw (cssu::RuntimeException)
{
    return EncryptorImpl_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
