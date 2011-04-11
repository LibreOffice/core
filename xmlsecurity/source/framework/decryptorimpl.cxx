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

#include "decryptorimpl.hxx"
#include <com/sun/star/xml/crypto/XXMLEncryptionTemplate.hpp>
#include <com/sun/star/xml/wrapper/XXMLElementWrapper.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

namespace cssu = com::sun::star::uno;
namespace cssl = com::sun::star::lang;
namespace cssxc = com::sun::star::xml::crypto;
namespace cssxw = com::sun::star::xml::wrapper;

#define SERVICE_NAME "com.sun.star.xml.crypto.sax.Decryptor"
#define IMPLEMENTATION_NAME "com.sun.star.xml.security.framework.DecryptorImpl"

#define DECLARE_ASCII( SASCIIVALUE )                                                                            \
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SASCIIVALUE ) )

DecryptorImpl::DecryptorImpl( const cssu::Reference< cssl::XMultiServiceFactory >& rxMSF)
{
    mxMSF = rxMSF;
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
        throw (cssu::Exception, cssu::RuntimeException)
{
    m_xResultListener = listener;
    tryToPerform();
}

void SAL_CALL DecryptorImpl::removeDecryptionResultListener( const cssu::Reference< cssxc::sax::XDecryptionResultListener >&)
        throw (cssu::RuntimeException)
{
}

/* XInitialization */
void SAL_CALL DecryptorImpl::initialize( const cssu::Sequence< cssu::Any >& aArguments )
    throw (cssu::Exception, cssu::RuntimeException)
{
    OSL_ASSERT(aArguments.getLength() == 5);

    rtl::OUString ouTempString;

    aArguments[0] >>= ouTempString;
    m_nSecurityId = ouTempString.toInt32();
    aArguments[1] >>= m_xSAXEventKeeper;
    aArguments[2] >>= ouTempString;
    m_nIdOfTemplateEC = ouTempString.toInt32();
    aArguments[3] >>= m_xXMLSecurityContext;
    aArguments[4] >>= m_xXMLEncryption;
}

rtl::OUString DecryptorImpl_getImplementationName ()
    throw (cssu::RuntimeException)
{
    return rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( IMPLEMENTATION_NAME ) );
}

sal_Bool SAL_CALL DecryptorImpl_supportsService( const rtl::OUString& ServiceName )
    throw (cssu::RuntimeException)
{
    return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME ));
}

cssu::Sequence< rtl::OUString > SAL_CALL DecryptorImpl_getSupportedServiceNames(  )
    throw (cssu::RuntimeException)
{
    cssu::Sequence < rtl::OUString > aRet(1);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] =  rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );
    return aRet;
}
#undef SERVICE_NAME

cssu::Reference< cssu::XInterface > SAL_CALL DecryptorImpl_createInstance( const cssu::Reference< cssl::XMultiServiceFactory >& rSMgr)
    throw( cssu::Exception )
{
    return (cppu::OWeakObject*) new DecryptorImpl(rSMgr);
}

/* XServiceInfo */
rtl::OUString SAL_CALL DecryptorImpl::getImplementationName(  )
    throw (cssu::RuntimeException)
{
    return DecryptorImpl_getImplementationName();
}
sal_Bool SAL_CALL DecryptorImpl::supportsService( const rtl::OUString& rServiceName )
    throw (cssu::RuntimeException)
{
    return DecryptorImpl_supportsService( rServiceName );
}
cssu::Sequence< rtl::OUString > SAL_CALL DecryptorImpl::getSupportedServiceNames(  )
    throw (cssu::RuntimeException)
{
    return DecryptorImpl_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
