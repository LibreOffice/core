/*************************************************************************
 *
 *  $RCSfile: signaturecreatorimpl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: mt $ $Date: 2004-07-12 13:15:23 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "signaturecreatorimpl.hxx"

#ifndef _COM_SUN_STAR_XML_CRYPTO_XXMLSIGNATURETEMPLATE_HPP_
#include <com/sun/star/xml/crypto/XXMLSignatureTemplate.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_WRAPPER_XXMLELEMENTWRAPPER_HPP_
#include <com/sun/star/xml/wrapper/XXMLElementWrapper.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_CRYPTO_SAX_SIGNATURECREATIONRESULT_HPP_
#include <com/sun/star/xml/crypto/sax/SignatureCreationResult.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

namespace cssu = com::sun::star::uno;
namespace cssl = com::sun::star::lang;
namespace cssxc = com::sun::star::xml::crypto;
namespace cssxw = com::sun::star::xml::wrapper;

#define SERVICE_NAME "com.sun.star.xml.crypto.sax.SignatureCreator"
#define IMPLEMENTATION_NAME "com.sun.star.xml.security.framework.SignatureCreatorImpl"

#define DECLARE_ASCII( SASCIIVALUE )                                                                            \
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SASCIIVALUE ) )

SignatureCreatorImpl::SignatureCreatorImpl( const cssu::Reference< cssl::XMultiServiceFactory >& rxMSF )
    :m_nIdOfBlocker(-1)
{
    mxMSF = rxMSF;
}

SignatureCreatorImpl::~SignatureCreatorImpl( )
{
}

bool SignatureCreatorImpl::checkReady() const
/****** SignatureCreatorImpl/checkReady **************************************
 *
 *   NAME
 *  checkReady -- checks the conditions for the signature generation.
 *
 *   SYNOPSIS
 *  bReady = checkReady( );
 *
 *   FUNCTION
 *  checks whether all following conditions are satisfied:
 *  1. the result listener is ready;
 *  2. the id of the template blocker is known;
 *  3. the SignatureEngine is ready.
 *
 *   INPUTS
 *  empty
 *
 *   RESULT
 *  bReady - true if all conditions are satisfied, false otherwise
 *
 *   HISTORY
 *  05.01.2004 -    implemented
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    return (m_xResultListener.is() &&
            (m_nIdOfBlocker != -1) &&
            SignatureEngine::checkReady());
}

void SignatureCreatorImpl::notifyResultListener() const
    throw (cssu::Exception, cssu::RuntimeException)
/****** SignatureCreatorImpl/notifyResultListener *****************************
 *
 *   NAME
 *  notifyResultListener -- notifies the listener about the signature
 *  creation result.
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
 *   HISTORY
 *  05.01.2004 -    implemented
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    cssu::Reference< cssxc::sax::XSignatureCreationResultListener >
        xSignatureCreationResultListener ( m_xResultListener , cssu::UNO_QUERY ) ;

    xSignatureCreationResultListener->signatureCreated(
        m_nSecurityId,
        m_bOperationSucceed?(cssxc::sax::SignatureCreationResult_CREATIONSUCCEED):
              (cssxc::sax::SignatureCreationResult_CREATIONFAIL));
}

void SignatureCreatorImpl::startEngine( const cssu::Reference<
    cssxc::XXMLSignatureTemplate >&
    xSignatureTemplate)
        throw (cssu::Exception, cssu::RuntimeException)
/****** SignatureCreatorImpl/startEngine *************************************
 *
 *   NAME
 *  startEngine -- generates the signature.
 *
 *   SYNOPSIS
 *  startEngine( xSignatureTemplate );
 *
 *   FUNCTION
 *  generates the signature element, then if succeeds, updates the link
 *  of old template element to the new signature element in
 *  SAXEventKeeper.
 *
 *   INPUTS
 *  xSignatureTemplate - the signature template (along with all referenced
 *  elements) to be signed.
 *
 *   RESULT
 *  empty
 *
 *   HISTORY
 *  05.01.2004 -    implemented
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    cssu::Reference< cssxc::XXMLSignatureTemplate > xResultTemplate;
    try
    {
        xResultTemplate = m_xXMLSignature->generate(xSignatureTemplate, m_xXMLSecurityContext);
    }
    catch( cssu::Exception& )
    {
        xResultTemplate = NULL;
    }

    if (xResultTemplate.is())
    {
        cssu::Reference < cssxw::XXMLElementWrapper > xResultSignature = xResultTemplate->getTemplate();
        m_xSAXEventKeeper->setElement(m_nIdOfTemplateEC, xResultSignature);

        m_bOperationSucceed = true;
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
 *
 *   INPUTS
 *  empty
 *
 *   RESULT
 *  empty
 *
 *   HISTORY
 *  05.01.2004 -    implemented
 *
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
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
        throw (cssu::Exception, cssu::RuntimeException)
{
    m_nIdOfBlocker = id;
    tryToPerform();
}

/* XSignatureCreationResultBroadcaster */
void SAL_CALL SignatureCreatorImpl::addSignatureCreationResultListener(
    const cssu::Reference< cssxc::sax::XSignatureCreationResultListener >& listener )
    throw (cssu::Exception, cssu::RuntimeException)
{
    m_xResultListener = listener;
    tryToPerform();
}

void SAL_CALL SignatureCreatorImpl::removeSignatureCreationResultListener(
    const cssu::Reference< cssxc::sax::XSignatureCreationResultListener >& listener )
    throw (cssu::RuntimeException)
{
}

/* XInitialization */
void SAL_CALL SignatureCreatorImpl::initialize( const cssu::Sequence< cssu::Any >& aArguments )
    throw (cssu::Exception, cssu::RuntimeException)
{
    sal_Int32 nLength = aArguments.getLength();
    OSL_ASSERT(nLength == 5);

    rtl::OUString ouTempString;

    aArguments[0] >>= ouTempString;
    m_nSecurityId = ouTempString.toInt32();
    aArguments[1] >>= m_xSAXEventKeeper;
    aArguments[2] >>= ouTempString;
    m_nIdOfTemplateEC = ouTempString.toInt32();
    aArguments[3] >>= m_xXMLSecurityContext;
    aArguments[4] >>= m_xXMLSignature;
}


rtl::OUString SignatureCreatorImpl_getImplementationName ()
    throw (cssu::RuntimeException)
{
    return rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( IMPLEMENTATION_NAME ) );
}

sal_Bool SAL_CALL SignatureCreatorImpl_supportsService( const rtl::OUString& ServiceName )
    throw (cssu::RuntimeException)
{
    return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME ));
}

cssu::Sequence< rtl::OUString > SAL_CALL SignatureCreatorImpl_getSupportedServiceNames(  )
    throw (cssu::RuntimeException)
{
    cssu::Sequence < rtl::OUString > aRet(1);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] =  rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );
    return aRet;
}
#undef SERVICE_NAME

cssu::Reference< cssu::XInterface > SAL_CALL SignatureCreatorImpl_createInstance(
    const cssu::Reference< cssl::XMultiServiceFactory >& rSMgr)
    throw( cssu::Exception )
{
    return (cppu::OWeakObject*) new SignatureCreatorImpl( rSMgr );
}

/* XServiceInfo */
rtl::OUString SAL_CALL SignatureCreatorImpl::getImplementationName(  )
    throw (cssu::RuntimeException)
{
    return SignatureCreatorImpl_getImplementationName();
}
sal_Bool SAL_CALL SignatureCreatorImpl::supportsService( const rtl::OUString& rServiceName )
    throw (cssu::RuntimeException)
{
    return SignatureCreatorImpl_supportsService( rServiceName );
}
cssu::Sequence< rtl::OUString > SAL_CALL SignatureCreatorImpl::getSupportedServiceNames(  )
    throw (cssu::RuntimeException)
{
    return SignatureCreatorImpl_getSupportedServiceNames();
}

