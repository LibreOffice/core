/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: EncryptionEntity.java,v $
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

/* uno classes */
import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XInitialization;
import com.sun.star.uno.XComponentContext;

import com.sun.star.xml.crypto.*;
import com.sun.star.xml.crypto.sax.*;

/*
 * this class maintains the data for an encryption operation.
 */
class EncryptionEntity extends SecurityEntity
{
    private int m_nEncryptionElementCollectorId;

    EncryptionEntity(
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

        m_nEncryptionElementCollectorId = m_xSAXEventKeeper.addSecurityElementCollector(
            ElementMarkPriority.AFTERMODIFY,
            true);

        m_xSAXEventKeeper.setSecurityId(m_nEncryptionElementCollectorId, m_nSecurityId);

        if (isExporting)
        {
            try
            {
                /*
                 * creates a Encryptor.
                 */
                Object encryptor = m_xRemoteServiceManager.createInstanceWithContext(
                    TestTool.ENCRYPTOR_COMPONENT, m_xRemoteContext);

                m_xReferenceResolvedListener =
                    (XReferenceResolvedListener)UnoRuntime.queryInterface(
                        XReferenceResolvedListener.class, encryptor);

                /*
                 * initializes the Encryptor.
                 */
                XInitialization xInitialization =
                    (XInitialization)UnoRuntime.queryInterface(
                        XInitialization.class, m_xReferenceResolvedListener);
                Object args[]=new Object[5];
                args[0] = new Integer(m_nSecurityId).toString();
                args[1] = m_xSAXEventKeeper;
                args[2] = new Integer(m_nEncryptionElementCollectorId).toString();
                args[3] = m_xXMLSecurityContext.getSecurityEnvironment();
                args[4] = m_xXMLEncryption;
                xInitialization.initialize(args);

                /*
                 * sets encryption result listener.
                 */
                XEncryptionResultBroadcaster m_xEncryptionResultBroadcaster =
                    (XEncryptionResultBroadcaster)UnoRuntime.queryInterface(
                        XEncryptionResultBroadcaster.class, m_xReferenceResolvedListener);
                m_xEncryptionResultBroadcaster.addEncryptionResultListener(
                    (XEncryptionResultListener)UnoRuntime.queryInterface(
                        XEncryptionResultListener.class, resultListener));
            }
            catch( com.sun.star.uno.Exception e)
            {
                e.printStackTrace();
            }
        }
        else
        {
            try{
                /*
                 * creates a Decryptor.
                 */
                Object decryptor = m_xRemoteServiceManager.createInstanceWithContext(
                    TestTool.DECRYPTOR_COMPONENT, m_xRemoteContext);

                m_xReferenceResolvedListener =
                    (XReferenceResolvedListener)UnoRuntime.queryInterface(
                        XReferenceResolvedListener.class, decryptor);

                /*
                 * initializes the Decryptor.
                 */
                XInitialization xInitialization = (XInitialization)UnoRuntime.queryInterface(XInitialization.class, m_xReferenceResolvedListener);
                Object args[]=new Object[5];
                args[0] = new Integer(m_nSecurityId).toString();
                args[1] = m_xSAXEventKeeper;
                args[2] = new Integer(m_nEncryptionElementCollectorId).toString();
                args[3] = m_xXMLSecurityContext;
                args[4] = m_xXMLEncryption;
                xInitialization.initialize(args);

                /*
                 * sets decryption result listener.
                 */
                XDecryptionResultBroadcaster m_xDecryptionResultBroadcaster =
                    (XDecryptionResultBroadcaster)UnoRuntime.queryInterface(
                        XDecryptionResultBroadcaster.class, m_xReferenceResolvedListener);
                m_xDecryptionResultBroadcaster.addDecryptionResultListener(
                    (XDecryptionResultListener)UnoRuntime.queryInterface(
                        XDecryptionResultListener.class, resultListener));
            }
            catch( com.sun.star.uno.Exception e)
            {
                e.printStackTrace();
            }
        }

        /*
         * creates a Blocker.
         */
        int blockerId = m_xSAXEventKeeper.addBlocker();
        m_xSAXEventKeeper.setSecurityId(blockerId, m_nSecurityId);

        try
        {
            XBlockerMonitor xBlockerMonitor = (XBlockerMonitor)UnoRuntime.queryInterface(
                XBlockerMonitor.class, m_xReferenceResolvedListener);
            xBlockerMonitor.setBlockerId(blockerId);
        }
        catch( com.sun.star.uno.Exception e)
        {
            e.printStackTrace();
        }

        /*
         * configures the resolve listener for the encryption template.
         */
        XReferenceResolvedBroadcaster xReferenceResolvedBroadcaster =
            (XReferenceResolvedBroadcaster)UnoRuntime.queryInterface(
                XReferenceResolvedBroadcaster.class, m_xSAXEventKeeper);
        xReferenceResolvedBroadcaster.addReferenceResolvedListener(m_nEncryptionElementCollectorId, m_xReferenceResolvedListener);
    }

    /*
     * add the reference to this encryption.
     *
     * 1. askes the SAXEventKeeper to add a ElementCollector to for the new
     * referenced element;
     * 2. configures this ElementCollector's security id;
     * 3. tells the SAXEventKeeper which listener will receive the reference
     * resolved notification.
     * 4. notifies the SignatureCollector about the reference id.
     */
    protected boolean setReference(boolean isExporting)
    {
        boolean rc = false;

        int referenceId = m_xSAXEventKeeper.addSecurityElementCollector(
            isExporting?
            (ElementMarkPriority.AFTERMODIFY):(ElementMarkPriority.BEFOREMODIFY),
            true);

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
            rc = false;
        }

        return rc;
    }
}

