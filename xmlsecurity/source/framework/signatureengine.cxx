/*************************************************************************
 *
 *  $RCSfile: signatureengine.cxx,v $
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

#include "signatureengine.hxx"

#ifndef _COM_SUN_STAR_XML_CRYPTO_XXMLSIGNATURETEMPLATE_HPP_
#include <com/sun/star/xml/crypto/XXMLSignatureTemplate.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_WRAPPER_XXMLELEMENTWRAPPER_HPP_
#include <com/sun/star/xml/wrapper/XXMLElementWrapper.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

namespace cssu = com::sun::star::uno;
namespace cssl = com::sun::star::lang;
namespace cssxc = com::sun::star::xml::crypto;
namespace cssxw = com::sun::star::xml::wrapper;

#define SIGNATURE_TEMPLATE "com.sun.star.xml.crypto.XMLSignatureTemplate"

#define DECLARE_ASCII( SASCIIVALUE )                                                                            \
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SASCIIVALUE ) )

SignatureEngine::SignatureEngine( )
    :m_nTotalReferenceNumber(-1)
{
}

bool SignatureEngine::checkReady() const
/****** SignatureEngine/checkReady *******************************************
 *
 *   NAME
 *  checkReady -- checks the conditions for the main operation.
 *
 *   SYNOPSIS
 *  bReady = checkReady( );
 *
 *   FUNCTION
 *  checks whether all following conditions are satisfied:
 *  1. the main operation has't begun yet;
 *  2. the key material is known;
 *  3. the amount of reference is known;
 *  4. all of referenced elements, the key element and the signature
 *     template are bufferred.
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
    bool rc = true;

    sal_Int32 nKeyInc = 0;
    if (m_nIdOfKeyEC != 0)
    {
        nKeyInc = 1;
    }

    if (m_bMissionDone ||
        m_nIdOfKeyEC == -1 ||
        m_nTotalReferenceNumber == -1 ||
        m_nTotalReferenceNumber+1+nKeyInc > m_nNumOfResolvedReferences)
    {
        rc = false;
    }

    return rc;
}

void SignatureEngine::tryToPerform( )
        throw (cssu::Exception, cssu::RuntimeException)
/****** SignatureEngine/tryToPerform *****************************************
 *
 *   NAME
 *  tryToPerform -- tries to perform the signature operation.
 *
 *   SYNOPSIS
 *  tryToPerform( );
 *
 *   FUNCTION
 *  if the situation is ready, perform following operations.
 *  1. prepares a signature template;
 *  2. calls the signature bridge component;
 *  3. clears up all used resources;
 *  4. notifies the result listener;
 *  5. sets the "accomplishment" flag.
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
    if (checkReady())
    {
        const rtl::OUString ouSignatureTemplate (
            RTL_CONSTASCII_USTRINGPARAM( SIGNATURE_TEMPLATE ) );
        cssu::Reference < cssxc::XXMLSignatureTemplate >
            xSignatureTemplate( mxMSF->createInstance( ouSignatureTemplate ), cssu::UNO_QUERY );

        OSL_ASSERT( xSignatureTemplate.is() );

        cssu::Reference< cssxw::XXMLElementWrapper >
            xXMLElement = m_xSAXEventKeeper->getElement( m_nIdOfTemplateEC );

        xSignatureTemplate->setTemplate(xXMLElement);

        std::vector< sal_Int32 >::const_iterator ii = m_vReferenceIds.begin();

        for( ; ii != m_vReferenceIds.end() ; ++ii )
        {
            xXMLElement = m_xSAXEventKeeper->getElement( *ii );
            xSignatureTemplate->setTarget(xXMLElement);
        }

        /*
         * set the Uri binding
         */
        xSignatureTemplate->setBinding( this );

        startEngine( xSignatureTemplate );

        /*
         * done
         */
        clearUp( );

        notifyResultListener();

        m_bMissionDone = true;
    }
}

void SignatureEngine::clearUp( ) const
/****** SignatureEngine/clearUp **********************************************
 *
 *   NAME
 *  clearUp -- clear up all resources used by this operation.
 *
 *   SYNOPSIS
 *  clearUp( );
 *
 *   FUNCTION
 *  cleaning resources up includes:
 *  1. releases the ElementCollector for the signature template element;
 *  2. releases ElementCollectors for referenced elements;
 *  3. releases the ElementCollector for the key element, if there is one.
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
    cssu::Reference < cssxc::sax::XReferenceResolvedBroadcaster >
        xReferenceResolvedBroadcaster( m_xSAXEventKeeper, cssu::UNO_QUERY );
    xReferenceResolvedBroadcaster->removeReferenceResolvedListener(
        m_nIdOfTemplateEC,
        (const cssu::Reference < cssxc::sax::XReferenceResolvedListener >)((SecurityEngine *)this));

    m_xSAXEventKeeper->removeElementCollector(m_nIdOfTemplateEC);

    std::vector< sal_Int32 >::const_iterator ii = m_vReferenceIds.begin();

    for( ; ii != m_vReferenceIds.end() ; ++ii )
    {
        xReferenceResolvedBroadcaster->removeReferenceResolvedListener(
            *ii,
            (const cssu::Reference < cssxc::sax::XReferenceResolvedListener >)((SecurityEngine *)this));
        m_xSAXEventKeeper->removeElementCollector(*ii);
    }

    if (m_nIdOfKeyEC != 0 && m_nIdOfKeyEC != -1)
    {
        m_xSAXEventKeeper->removeElementCollector(m_nIdOfKeyEC);
    }
}

/* XReferenceCollector */
void SAL_CALL SignatureEngine::setReferenceCount( sal_Int32 count )
    throw (cssu::Exception, cssu::RuntimeException)
{
    m_nTotalReferenceNumber = count;
    tryToPerform();
}

void SAL_CALL SignatureEngine::setReferenceId( sal_Int32 id )
    throw (cssu::Exception, cssu::RuntimeException)
{
    m_vReferenceIds.push_back( id );
}

/* XUriBinding */
void SAL_CALL SignatureEngine::setUriBinding(
    const rtl::OUString& uri,
    const cssu::Reference< com::sun::star::io::XInputStream >& aInputStream )
    throw (cssu::Exception, cssu::RuntimeException)
{
    m_vUris.push_back(uri);
    m_vXInputStreams.push_back(aInputStream);
}

cssu::Reference< com::sun::star::io::XInputStream > SAL_CALL SignatureEngine::getUriBinding( const rtl::OUString& uri )
    throw (cssu::Exception, cssu::RuntimeException)
{
    cssu::Reference< com::sun::star::io::XInputStream > xInputStream;

    int size = m_vUris.size();

    for( int i=0; i<size; ++i)
    {
        if (m_vUris[i] == uri)
        {
            xInputStream = m_vXInputStreams[i];
            break;
        }
    }

    return xInputStream;
}

