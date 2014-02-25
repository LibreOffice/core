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


#include "encryptionengine.hxx"
#include <com/sun/star/xml/crypto/XMLEncryptionTemplate.hpp>
#include <com/sun/star/xml/wrapper/XXMLElementWrapper.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

using namespace com::sun::star::uno;
namespace cssxc = com::sun::star::xml::crypto;
namespace cssxw = com::sun::star::xml::wrapper;

EncryptionEngine::EncryptionEngine( const Reference<XComponentContext> & xContext)
        :m_xContext(xContext), m_nIdOfBlocker(-1)
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
        throw (Exception, RuntimeException)
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
        Reference < cssxc::XXMLEncryptionTemplate > xEncryptionTemplate =
            cssxc::XMLEncryptionTemplate::create( m_xContext );

        Reference< cssxw::XXMLElementWrapper > xXMLElement
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
    Reference < cssxc::sax::XReferenceResolvedBroadcaster >
        xReferenceResolvedBroadcaster( m_xSAXEventKeeper, UNO_QUERY );

    xReferenceResolvedBroadcaster->removeReferenceResolvedListener(
        m_nIdOfTemplateEC,
        (const Reference < cssxc::sax::XReferenceResolvedListener >)((SecurityEngine *)this));

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
    throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException, std::exception)
{
    m_nIdOfBlocker = id;
    tryToPerform();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
