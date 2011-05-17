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

#include "encryptionengine.hxx"
#include <com/sun/star/xml/crypto/XXMLEncryptionTemplate.hpp>
#include <com/sun/star/xml/wrapper/XXMLElementWrapper.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

namespace cssu = com::sun::star::uno;
namespace cssl = com::sun::star::lang;
namespace cssxc = com::sun::star::xml::crypto;
namespace cssxw = com::sun::star::xml::wrapper;

#define ENCRYPTION_TEMPLATE "com.sun.star.xml.crypto.XMLEncryptionTemplate"

#define DECLARE_ASCII( SASCIIVALUE )                                                                            \
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SASCIIVALUE ) )

EncryptionEngine::EncryptionEngine( )
        :m_nIdOfBlocker(-1)
{
}

bool EncryptionEngine::checkReady() const
/****** EncryptionEngine/checkReady ******************************************
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
 *  3. the id of the template blocker is known;
 *  4. both the key element and the encryption template
 *     are bufferred.
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
        m_nIdOfBlocker == -1 ||
        1+nKeyInc > m_nNumOfResolvedReferences )
    {
        rc = false;
    }

    return rc;
}

void EncryptionEngine::tryToPerform( )
        throw (cssu::Exception, cssu::RuntimeException)
/****** EncryptionEngine/tryToPerform ****************************************
 *
 *   NAME
 *  tryToPerform -- tries to perform the encryption/decryption operation.
 *
 *   SYNOPSIS
 *  tryToPerform( );
 *
 *   FUNCTION
 *  if the situation is ready, perform following operations.
 *  1. prepares a encryption template;
 *  2. calls the encryption bridge component;
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
        const rtl::OUString sEncryptionTemplate (
            RTL_CONSTASCII_USTRINGPARAM( ENCRYPTION_TEMPLATE ) );
        cssu::Reference < cssxc::XXMLEncryptionTemplate > xEncryptionTemplate(
            mxMSF->createInstance( sEncryptionTemplate ), cssu::UNO_QUERY );

        OSL_ASSERT( xEncryptionTemplate.is() );

        cssu::Reference< cssxw::XXMLElementWrapper > xXMLElement
            = m_xSAXEventKeeper->getElement( m_nIdOfTemplateEC );

        xEncryptionTemplate->setTemplate(xXMLElement);

        startEngine( xEncryptionTemplate );

        /*
         * done
         */
        clearUp( );

        notifyResultListener();

        m_bMissionDone = true;
    }
}

void EncryptionEngine::clearUp( ) const
/****** EncryptionEngine/clearup *********************************************
 *
 *   NAME
 *  clearUp -- clear up all resources used by this operation.
 *
 *   SYNOPSIS
 *  clearUp( );
 *
 *   FUNCTION
 *  cleaning resources up includes:
 *  1. releases the ElementCollector for the encryption template element;
 *  2. releases the Blocker for the encryption template element;
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

    if (m_nIdOfBlocker != -1)
    {
        m_xSAXEventKeeper->removeBlocker(m_nIdOfBlocker);
    }

    if (m_nIdOfKeyEC != 0 && m_nIdOfKeyEC != -1)
    {
        m_xSAXEventKeeper->removeElementCollector(m_nIdOfKeyEC);
    }
}

/* XBlockerMonitor */
void SAL_CALL EncryptionEngine::setBlockerId( sal_Int32 id )
    throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException)
{
    m_nIdOfBlocker = id;
    tryToPerform();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
