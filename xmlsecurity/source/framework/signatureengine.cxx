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


#include "signatureengine.hxx"
#include <com/sun/star/xml/crypto/XXMLSignatureTemplate.hpp>
#include <com/sun/star/xml/wrapper/XXMLElementWrapper.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

namespace cssu = com::sun::star::uno;
namespace cssl = com::sun::star::lang;
namespace cssxc = com::sun::star::xml::crypto;
namespace cssxw = com::sun::star::xml::wrapper;

#define SIGNATURE_TEMPLATE "com.sun.star.xml.crypto.XMLSignatureTemplate"

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
 *   AUTHOR
 *  Michael Mi
 *  Email: michael.mi@sun.com
 ******************************************************************************/
{
    if (checkReady())
    {
        const OUString ouSignatureTemplate (
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
    const OUString& uri,
    const cssu::Reference< com::sun::star::io::XInputStream >& aInputStream )
    throw (cssu::Exception, cssu::RuntimeException)
{
    m_vUris.push_back(uri);
    m_vXInputStreams.push_back(aInputStream);
}

cssu::Reference< com::sun::star::io::XInputStream > SAL_CALL SignatureEngine::getUriBinding( const OUString& uri )
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
