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

package com.sun.star.xml.security.uno;

/* uno classes */
import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.XComponentContext;

import com.sun.star.xml.crypto.*;
import com.sun.star.xml.crypto.sax.*;

/*
 * this class maintains the data for a security operation.
 */
class SecurityEntity
{
    /*
     * the security id, which identifies this security entity
     * uniquely.
     */
    private static int m_nNextSecurityId = 1;
    protected int m_nSecurityId;

    /*
     * xml security related components
     */
    protected XXMLSecurityContext        m_xXMLSecurityContext;
    protected XXMLSignature              m_xXMLSignature;
    protected XXMLEncryption             m_xXMLEncryption;
    protected XMultiComponentFactory     m_xRemoteServiceManager;
    protected XComponentContext          m_xRemoteContext;
    protected XReferenceResolvedListener m_xReferenceResolvedListener;
    protected XSecuritySAXEventKeeper    m_xSAXEventKeeper;

    /*
     * the uri of the key material of this security entity
     */
    private String m_keyURI;

    SecurityEntity(
        XSecuritySAXEventKeeper xSAXEventKeeper,
        XXMLSecurityContext xXMLSecurityContext,
        XXMLSignature xXMLSignature,
        XXMLEncryption xXMLEncryption,
        XMultiComponentFactory xRemoteServiceManager,
        XComponentContext xRemoteContext)
    {
        m_xSAXEventKeeper = xSAXEventKeeper;
        m_xXMLSecurityContext = xXMLSecurityContext;
        m_xXMLSignature = xXMLSignature;
        m_xXMLEncryption = xXMLEncryption;
        m_xRemoteServiceManager = xRemoteServiceManager;
        m_xRemoteContext = xRemoteContext;

        m_nSecurityId = getNextSecurityId();
        m_keyURI = null;
    }

/**************************************************************************************
 * private methods
 **************************************************************************************/

    /*
     * generates a new security id.
     */
    private static int getNextSecurityId()
    {
        int id = m_nNextSecurityId++;
        return id;
    }

/**************************************************************************************
 * protected methods
 **************************************************************************************/

    /*
     * notifies the key collector about the key id, this key id
     * is used to ask the SAXEventKeeper to release the bufferred
     * key element.
     * when the id is 0, that means there is no independant key
     * element needed.
     */
    protected void setKeyId(int id)
    {
        try
        {
            XKeyCollector xKeyCollector =
                (XKeyCollector)UnoRuntime.queryInterface(
                    XKeyCollector.class, m_xReferenceResolvedListener);
            xKeyCollector.setKeyId(id);
        }
        catch( com.sun.star.uno.Exception e)
        {
            e.printStackTrace();
        }
    }

    /*
     * set the key uri, which will be the value of the id attribute
     * of the key element
     */
    protected void setKeyURI(String uri)
    {
        m_keyURI = new String(uri);
    }

    protected XReferenceResolvedListener getReferenceListener()
    {
        return m_xReferenceResolvedListener;
    }

    protected int getSecurityId()
    {
        return m_nSecurityId;
    }

    /*
     * configures the key material to the security entity.
     *
     * if the uri is the key, then:
     * 1. askes the SAXEventKeeper to add a ElementCollector to the key
     * element;
     * 2. notifies the key collector;
     * 3. configures this ElementCollector's security id;
     * 4. tells the SAXEventKeeper which listener will receive the reference
     * resolved notification.
     */
    protected boolean setKey(String uri, boolean isExporting)
    {
        boolean rc = false;

        if (m_keyURI != null &&
            m_keyURI.equals(uri))
        {
            int referenceId = m_xSAXEventKeeper.addSecurityElementCollector(
                isExporting?
                (ElementMarkPriority.BEFOREMODIFY):(ElementMarkPriority.AFTERMODIFY),
                false );

            setKeyId(referenceId);
            m_xSAXEventKeeper.setSecurityId(referenceId, m_nSecurityId);

            XReferenceResolvedBroadcaster xReferenceResolvedBroadcaster =
                (XReferenceResolvedBroadcaster)UnoRuntime.queryInterface(
                    XReferenceResolvedBroadcaster.class, m_xSAXEventKeeper);

            xReferenceResolvedBroadcaster.addReferenceResolvedListener(referenceId, m_xReferenceResolvedListener);

            rc = true;
        }

        return rc;
    }

    /*
     * ends this misstion, asks the security engine to clear up all
     * resources.
     */
    protected boolean endMission()
    {
        XMissionTaker xMissionTaker =
            (XMissionTaker)UnoRuntime.queryInterface(
                XMissionTaker.class, m_xReferenceResolvedListener);

        boolean rc = xMissionTaker.endMission();

        m_xXMLSecurityContext = null;
        m_xXMLSignature = null;
        m_xXMLEncryption = null;
        m_xReferenceResolvedListener = null;
        m_xSAXEventKeeper = null;

        return rc;
    }
}

