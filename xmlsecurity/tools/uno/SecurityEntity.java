/*************************************************************************
 *
 *  $RCSfile: SecurityEntity.java,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: mt $ $Date: 2004-07-12 13:15:24 $
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
                (ElementMarkPriority.PRI_BEFOREMODIFY):(ElementMarkPriority.PRI_AFTERMODIFY),
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

