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

#include "signatureverifierimpl.hxx"
#include <com/sun/star/xml/crypto/XXMLSignatureTemplate.hpp>
#include <com/sun/star/xml/wrapper/XXMLElementWrapper.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

namespace cssu = com::sun::star::uno;
namespace cssl = com::sun::star::lang;
namespace cssxc = com::sun::star::xml::crypto;
namespace cssxw = com::sun::star::xml::wrapper;

#define SERVICE_NAME "com.sun.star.xml.crypto.sax.SignatureVerifier"
#define IMPLEMENTATION_NAME "com.sun.star.xml.security.framework.SignatureVerifierImpl"

#define DECLARE_ASCII( SASCIIVALUE )                                                                            \
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SASCIIVALUE ) )

SignatureVerifierImpl::SignatureVerifierImpl( const cssu::Reference< cssl::XMultiServiceFactory >& rxMSF)
{
    mxMSF = rxMSF;
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
    throw (cssu::Exception, cssu::RuntimeException)
{
    m_xResultListener = listener;
    tryToPerform();
}

void SAL_CALL SignatureVerifierImpl::removeSignatureVerifyResultListener(
    const cssu::Reference< cssxc::sax::XSignatureVerifyResultListener >&)
    throw (cssu::RuntimeException)
{
}

/* XInitialization */
void SAL_CALL SignatureVerifierImpl::initialize(
    const cssu::Sequence< cssu::Any >& aArguments )
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
    aArguments[4] >>= m_xXMLSignature;
}


rtl::OUString SignatureVerifierImpl_getImplementationName ()
    throw (cssu::RuntimeException)
{
    return rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM ( IMPLEMENTATION_NAME ) );
}

sal_Bool SAL_CALL SignatureVerifierImpl_supportsService( const rtl::OUString& ServiceName )
    throw (cssu::RuntimeException)
{
    return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME ));
}

cssu::Sequence< rtl::OUString > SAL_CALL SignatureVerifierImpl_getSupportedServiceNames(  )
    throw (cssu::RuntimeException)
{
    cssu::Sequence < rtl::OUString > aRet(1);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] =  rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );
    return aRet;
}
#undef SERVICE_NAME

cssu::Reference< cssu::XInterface > SAL_CALL SignatureVerifierImpl_createInstance(
    const cssu::Reference< cssl::XMultiServiceFactory >& rSMgr)
    throw( cssu::Exception )
{
    return (cppu::OWeakObject*) new SignatureVerifierImpl(rSMgr);
}

/* XServiceInfo */
rtl::OUString SAL_CALL SignatureVerifierImpl::getImplementationName(  )
    throw (cssu::RuntimeException)
{
    return SignatureVerifierImpl_getImplementationName();
}
sal_Bool SAL_CALL SignatureVerifierImpl::supportsService( const rtl::OUString& rServiceName )
    throw (cssu::RuntimeException)
{
    return SignatureVerifierImpl_supportsService( rServiceName );
}
cssu::Sequence< rtl::OUString > SAL_CALL SignatureVerifierImpl::getSupportedServiceNames(  )
    throw (cssu::RuntimeException)
{
    return SignatureVerifierImpl_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
