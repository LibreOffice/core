/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SecurityEntity.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 17:44:21 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

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

