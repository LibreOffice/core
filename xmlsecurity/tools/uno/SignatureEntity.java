/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SignatureEntity.java,v $
 * $Revision: 1.4 $
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

package com.sun.star.xml.security.uno;

import java.util.Vector;

/* uno classes */
import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XInitialization;
import com.sun.star.uno.XComponentContext;

import com.sun.star.xml.crypto.*;
import com.sun.star.xml.crypto.sax.*;

/*
 * this class maintains the data for a signature operation.
 */
class SignatureEntity extends SecurityEntity
{
    private Vector m_vReferenceIds;
    private int    m_nSignatureElementCollectorId;

    SignatureEntity(
        XSecuritySAXEventKeeper xSAXEventKeeper,
        boolean isExporting,
        Object resultListener,
        XXMLSecurityContext xXMLSecurityContext,
        XXMLSignature xXMLSignature,
        XXMLEncryption xXMLEncryption,
        XMultiComponentFactory xRemoteServiceManager,
        XComponentContext xRemoteContext)
    {
        super(xSAXEventKeeper, xXMLSecurityContext, xXMLSignature,
            xXMLEncryption, xRemoteServiceManager, xRemoteContext);

        m_vReferenceIds = new Vector();

        if (isExporting)
        {
            m_nSignatureElementCollectorId = m_xSAXEventKeeper.addSecurityElementCollector(
                ElementMarkPriority.AFTERMODIFY,
                true);

            m_xSAXEventKeeper.setSecurityId(m_nSignatureElementCollectorId, m_nSecurityId);

            try
            {
                /*
                 * creates a SignatureCreator.
                 */
                Object signatureCreator = m_xRemoteServiceManager.createInstanceWithContext(
                    TestTool.SIGNATURECREATOR_COMPONENT, m_xRemoteContext);

                m_xReferenceResolvedListener =
                    (XReferenceResolvedListener)UnoRuntime.queryInterface(
                        XReferenceResolvedListener.class, signatureCreator);

                            /*
                             * initializes the SignatureCreator.
                             */
                XInitialization xInitialization =
                    (XInitialization)UnoRuntime.queryInterface(
                        XInitialization.class, m_xReferenceResolvedListener);

                Object args[]=new Object[5];
                args[0] = new Integer(m_nSecurityId).toString();
                args[1] = m_xSAXEventKeeper;
                args[2] = new Integer(m_nSignatureElementCollectorId).toString();
                args[3] = m_xXMLSecurityContext.getSecurityEnvironment();
                args[4] = m_xXMLSignature;
                xInitialization.initialize(args);

                /*
                 * creates a Blocker.
                 */
                int blockerId = m_xSAXEventKeeper.addBlocker();
                m_xSAXEventKeeper.setSecurityId(blockerId, m_nSecurityId);

                XBlockerMonitor xBlockerMonitor = (XBlockerMonitor)UnoRuntime.queryInterface(
                    XBlockerMonitor.class, m_xReferenceResolvedListener);
                xBlockerMonitor.setBlockerId(blockerId);

                /*
                 * sets signature creation result listener.
                 */
                XSignatureCreationResultBroadcaster xSignatureCreationResultBroadcaster =
                    (XSignatureCreationResultBroadcaster)UnoRuntime.queryInterface(
                        XSignatureCreationResultBroadcaster.class, m_xReferenceResolvedListener);
                xSignatureCreationResultBroadcaster.addSignatureCreationResultListener(
                    (XSignatureCreationResultListener)UnoRuntime.queryInterface(
                        XSignatureCreationResultListener.class, resultListener));
            }
            catch( com.sun.star.uno.Exception e)
            {
                e.printStackTrace();
            }

        }
        else
        {
            m_nSignatureElementCollectorId = m_xSAXEventKeeper.addSecurityElementCollector(
                ElementMarkPriority.BEFOREMODIFY, false);

            m_xSAXEventKeeper.setSecurityId(m_nSignatureElementCollectorId, m_nSecurityId);

            try
            {
                /*
                 * creates a SignatureVerifier.
                 */
                Object signatureVerifier = m_xRemoteServiceManager.createInstanceWithContext(
                    TestTool.SIGNATUREVERIFIER_COMPONENT, m_xRemoteContext);

                m_xReferenceResolvedListener =
                    (XReferenceResolvedListener)UnoRuntime.queryInterface(
                        XReferenceResolvedListener.class, signatureVerifier);

                            /*
                             * initializes the SignatureVerifier.
                             */
                XInitialization xInitialization =
                    (XInitialization)UnoRuntime.queryInterface(
                        XInitialization.class, m_xReferenceResolvedListener);
                Object args[]=new Object[5];
                args[0] = new Integer(m_nSecurityId).toString();
                args[1] = m_xSAXEventKeeper;
                args[2] = new Integer(m_nSignatureElementCollectorId).toString();
                args[3] = m_xXMLSecurityContext;
                args[4] = m_xXMLSignature;
                xInitialization.initialize(args);

                /*
                 * sets signature verify result listener.
                 */
                XSignatureVerifyResultBroadcaster xSignatureVerifyResultBroadcaster =
                    (XSignatureVerifyResultBroadcaster)UnoRuntime.queryInterface(
                        XSignatureVerifyResultBroadcaster.class, m_xReferenceResolvedListener);
                xSignatureVerifyResultBroadcaster.addSignatureVerifyResultListener(
                    (XSignatureVerifyResultListener)UnoRuntime.queryInterface(
                        XSignatureVerifyResultListener.class, resultListener));
            }
            catch( com.sun.star.uno.Exception e)
            {
                e.printStackTrace();
            }
        }

        /*
         * configures the resolve listener for the signature template.
         */
        XReferenceResolvedBroadcaster xReferenceResolvedBroadcaster =
            (XReferenceResolvedBroadcaster)UnoRuntime.queryInterface(
                XReferenceResolvedBroadcaster.class, m_xSAXEventKeeper);
        xReferenceResolvedBroadcaster.addReferenceResolvedListener(
            m_nSignatureElementCollectorId, m_xReferenceResolvedListener);
    }

/**************************************************************************************
 * private methods
 **************************************************************************************/

    /*
     * checks whether this signature has a reference with
     * the particular id.
     */
    private boolean hasReference(String id)
    {
        boolean rc = false;

        int length = m_vReferenceIds.size();
        for (int i=0; i<length; ++i)
        {
            if (id.equals((String)m_vReferenceIds.elementAt(i)))
            {
                rc = true;
                break;
            }
        }

        return rc;
    }


/**************************************************************************************
 * protected methods
 **************************************************************************************/

    /*
     * adds a new reference id.
     */
    protected void addReferenceId(String referenceId)
    {
        m_vReferenceIds.add(referenceId);
    }

    /*
     * notifies how many reference in this signature.
     */
    protected void setReferenceNumber()
    {
        try
        {
            XReferenceCollector xReferenceCollector =
                (XReferenceCollector)UnoRuntime.queryInterface(
                    XReferenceCollector.class, m_xReferenceResolvedListener);
            xReferenceCollector.setReferenceCount(m_vReferenceIds.size());
        }
        catch( com.sun.star.uno.Exception e)
        {
            e.printStackTrace();
        }
    }

    /*
     * tries to add a reference to this signature.
     *
     * If the specific id belongs to this signature's references, then:
     * 1. askes the SAXEventKeeper to add a ElementCollector to for the new
     * referenced element;
     * 2. configures this ElementCollector's security id;
     * 3. tells the SAXEventKeeper which listener will receive the reference
     * resolved notification.
     * 4. notifies the SignatureCollector about the reference id.
     */
    protected boolean setReference(String id, boolean isExporting)
    {
        boolean rc = false;

        if (hasReference(id))
        {
            int referenceId = m_xSAXEventKeeper.addSecurityElementCollector(
                isExporting?
                (ElementMarkPriority.AFTERMODIFY):(ElementMarkPriority.BEFOREMODIFY),
                false );

            m_xSAXEventKeeper.setSecurityId(referenceId, m_nSecurityId);

            XReferenceResolvedBroadcaster xReferenceResolvedBroadcaster =
                (XReferenceResolvedBroadcaster)UnoRuntime.queryInterface(
                    XReferenceResolvedBroadcaster.class, m_xSAXEventKeeper);
            xReferenceResolvedBroadcaster.addReferenceResolvedListener(
                referenceId, m_xReferenceResolvedListener);

            try
            {
                XReferenceCollector xReferenceCollector =
                    (XReferenceCollector)UnoRuntime.queryInterface(
                        XReferenceCollector.class, m_xReferenceResolvedListener);
                xReferenceCollector.setReferenceId(referenceId);
            }
            catch( com.sun.star.uno.Exception e)
            {
                e.printStackTrace();
            }

            rc = true;
        }

        return rc;
    }
}

