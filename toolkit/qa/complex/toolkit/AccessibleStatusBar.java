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

package complex.toolkit;

import complex.toolkit.accessibility._XAccessibleEventBroadcaster;
import complex.toolkit.accessibility._XAccessibleExtendedComponent;
import complex.toolkit.accessibility._XAccessibleComponent;
import complex.toolkit.accessibility._XAccessibleContext;
import java.util.logging.Logger;
import java.util.logging.Level;
import util.SOfficeFactory;
import util.AccessibilityTools;
import com.sun.star.awt.XWindow;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XCloseable;
import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.awt.XExtendedToolkit;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;

/**
 *
 */
public class AccessibleStatusBar {

    XInterface testObject = null;
    XMultiServiceFactory xMSF = null;
    XWindow xWindow = null;

    private XMultiServiceFactory getMSF()
    {
        return UnoRuntime.queryInterface(XMultiServiceFactory.class, connection.getComponentContext().getServiceManager());
    }

    /**
     * Check document types
     */
    @Test
    public void checkDocs() {
        checkWriterDoc();
        checkMathDoc();
        checkDrawDoc();
        checkImpressDoc();
        checkCalcDoc();
    }

    /**
     * Test the interfaces on a writer document
     */
    public void checkWriterDoc() {
        xMSF = getMSF();
        SOfficeFactory xSOF = SOfficeFactory.getFactory(xMSF);
        XTextDocument xTextDoc = null;
        try {
            System.out.println("****** Open a new writer document");
            xTextDoc = xSOF.createTextDoc("_blank");
            getTestObject();
        }
        catch(com.sun.star.uno.Exception e) {
            Logger.getLogger( this.getClass().getName() ).log( Level.SEVERE, "caught an exception", e );
        }
        runAllInterfaceTests();

        if (xTextDoc != null) {
            XCloseable xClose = UnoRuntime.queryInterface(XCloseable.class, xTextDoc);
            try {
                xClose.close(false);
            }
            catch(com.sun.star.util.CloseVetoException e) {
                Logger.getLogger( this.getClass().getName() ).log( Level.SEVERE, "caught an exception", e );
            }
        }
    }

    /**
     * Test the interfaces on a math document
     */
    public void checkMathDoc() {
        xMSF = getMSF();
        SOfficeFactory xSOF = SOfficeFactory.getFactory(xMSF);
        XComponent xMathDoc = null;
        try {
            System.out.println("****** Open a new math document");
            xMathDoc = xSOF.createMathDoc("_blank");
            getTestObject();
        }
        catch(com.sun.star.uno.Exception e) {
            Logger.getLogger( this.getClass().getName() ).log( Level.SEVERE, "caught an exception", e );
        }
        runAllInterfaceTests();

        if (xMathDoc != null) {
            XCloseable xClose = UnoRuntime.queryInterface(XCloseable.class, xMathDoc);
            try {
                xClose.close(false);
            }
            catch(com.sun.star.util.CloseVetoException e) {
                Logger.getLogger( this.getClass().getName() ).log( Level.SEVERE, "caught an exception", e );
            }
        }
    }

    /**
     * Test the interfaces on a draw document
     */
    public void checkDrawDoc() {
        xMSF = getMSF();
        SOfficeFactory xSOF = SOfficeFactory.getFactory(xMSF);
        XComponent xDrawDoc = null;
        try {
            System.out.println("****** Open a new draw document");
            xDrawDoc = xSOF.createDrawDoc("_blank");
            getTestObject();
        }
        catch(com.sun.star.uno.Exception e) {
            Logger.getLogger( this.getClass().getName() ).log( Level.SEVERE, "caught an exception", e );
        }
        runAllInterfaceTests();

        if (xDrawDoc != null) {
            XCloseable xClose = UnoRuntime.queryInterface(XCloseable.class, xDrawDoc);
            try {
                xClose.close(false);
            }
            catch(com.sun.star.util.CloseVetoException e) {
                Logger.getLogger( this.getClass().getName() ).log( Level.SEVERE, "caught an exception", e );
            }
        }
    }

    /**
     * Test the interfaces on an impress document
     */
    public void checkImpressDoc() {
        xMSF = getMSF();
        SOfficeFactory xSOF = SOfficeFactory.getFactory(xMSF);
        XComponent xImpressDoc = null;
        try {
            System.out.println("****** Open a new impress document");
            xImpressDoc = xSOF.createImpressDoc("_blank");
            getTestObject();
        }
        catch(com.sun.star.uno.Exception e) {
            Logger.getLogger( this.getClass().getName() ).log( Level.SEVERE, "caught an exception", e );
        }
        runAllInterfaceTests();

        if (xImpressDoc != null) {
            XCloseable xClose = UnoRuntime.queryInterface(XCloseable.class, xImpressDoc);
            try {
                xClose.close(false);
            }
            catch(com.sun.star.util.CloseVetoException e) {
                Logger.getLogger( this.getClass().getName() ).log( Level.SEVERE, "caught an exception", e );
            }
        }
    }
    /**
     * Test the interfaces on a calc document
     */
    public void checkCalcDoc() {
        xMSF = getMSF();
        SOfficeFactory xSOF = SOfficeFactory.getFactory(xMSF);
        XSpreadsheetDocument xSpreadsheetDoc = null;
        try {
            System.out.println("****** Open a new calc document");
            xSpreadsheetDoc = xSOF.createCalcDoc("_blank");
            util.utils.waitForEventIdle(xMSF);
            getTestObject();
        }
        catch(com.sun.star.uno.Exception e) {
            Logger.getLogger( this.getClass().getName() ).log( Level.SEVERE, "caught an exception", e );
        }
        runAllInterfaceTests();

        if (xSpreadsheetDoc != null) {
            XCloseable xClose = UnoRuntime.queryInterface(XCloseable.class, xSpreadsheetDoc);
            try {
                xClose.close(false);
            }
            catch(com.sun.star.util.CloseVetoException e) {
                Logger.getLogger( this.getClass().getName() ).log( Level.SEVERE, "caught an exception", e );
            }
        }
    }

    public void getTestObject() {
        try {
            XInterface xIfc = (XInterface) xMSF.createInstance(
                                            "com.sun.star.awt.Toolkit") ;
            XExtendedToolkit tk = UnoRuntime.queryInterface(XExtendedToolkit.class, xIfc);

            util.utils.waitForEventIdle(xMSF);
            xWindow = UnoRuntime.queryInterface(XWindow.class, tk.getActiveTopWindow());

            util.utils.waitForEventIdle(xMSF);
            XAccessible xRoot = AccessibilityTools.getAccessibleObject(xWindow);
            XAccessibleContext parentContext = null;

            System.out.println("Get the accessible status bar.");
            parentContext = AccessibilityTools.getAccessibleObjectForRole(
                                        xRoot, AccessibleRole.STATUS_BAR, "");
            util.utils.waitForEventIdle(xMSF);
            System.out.println("...OK.");

            XServiceInfo xSI = UnoRuntime.queryInterface(XServiceInfo.class, parentContext);
            String[] services = xSI.getSupportedServiceNames();
            System.out.println("*****");
            System.out.println("* Implementation Name: " + xSI.getImplementationName());
            for (int i=0; i<services.length; i++)
            {
                System.out.println("* ServiceName " + i + ": " + services[i]);
            }
            System.out.println("*****");
            testObject=parentContext;
        }
        catch(com.sun.star.uno.Exception e) {
            Logger.getLogger( this.getClass().getName() ).log( Level.SEVERE, "caught an exception", e );
        }
        catch(Throwable t) {
            System.out.println("Got throwable:");
            Logger.getLogger( this.getClass().getName() ).log( Level.SEVERE, "caught an exception", t );
        }
    }

    public void runAllInterfaceTests() {
        System.out.println("*** Now testing XAccessibleComponent ***");
        _XAccessibleComponent _xAccCompTest =
                                new _XAccessibleComponent(testObject);
        assertTrue("failed: XAccessibleComponent::getBounds", _xAccCompTest._getBounds());
        assertTrue("failed: XAccessibleComponent::contains", _xAccCompTest._containsPoint());
        assertTrue("failed: XAccessibleComponent::getAccessibleAt", _xAccCompTest._getAccessibleAtPoint());
        assertTrue("failed: XAccessibleComponent::getBackground", _xAccCompTest._getBackground());
        assertTrue("failed: XAccessibleComponent::getForeground", _xAccCompTest._getForeground());
        assertTrue("failed: XAccessibleComponent::getLocation", _xAccCompTest._getLocation());
        assertTrue("failed: XAccessibleComponent::getLocationOnScreen", _xAccCompTest._getLocationOnScreen());
        assertTrue("failed: XAccessibleComponent::getSize", _xAccCompTest._getSize());
        assertTrue("failed: XAccessibleComponent::grabFocus", _xAccCompTest._grabFocus());

        System.out.println("*** Now testing XAccessibleContext ***");
        _XAccessibleContext _xAccContext =
                                new _XAccessibleContext(testObject);
        assertTrue("failed: XAccessibleContext::getAccessibleChildCount", _xAccContext._getAccessibleChildCount());
        assertTrue("failed: XAccessibleContext::getAccessibleChild", _xAccContext._getAccessibleChild());
        assertTrue("failed: XAccessibleContext::getAccessibleDescription", _xAccContext._getAccessibleDescription());
        assertTrue("failed: XAccessibleContext::getAccessibleName", _xAccContext._getAccessibleName());
        assertTrue("failed: XAccessibleContext::getAccessibleParent", _xAccContext._getAccessibleParent());
        assertTrue("failed: XAccessibleContext::getAccessibleIndexInParent", _xAccContext._getAccessibleIndexInParent());
        assertTrue("failed: XAccessibleContext::getAccessibleRelationSet", _xAccContext._getAccessibleRelationSet());
        assertTrue("failed: XAccessibleContext::getAccessibleRole", _xAccContext._getAccessibleRole());
        assertTrue("failed: XAccessibleContext::getAccessibleStateSet", _xAccContext._getAccessibleStateSet());
        assertTrue("failed: XAccessibleContext::getLocale", _xAccContext._getLocale());

        System.out.println("*** Now testing XAccessibleExtendedComponent ***");
        _XAccessibleExtendedComponent _xAccExtComp =
                                new _XAccessibleExtendedComponent(testObject);
        assertTrue("failed: XAccessibleExtendedComponent::getFont", _xAccExtComp._getFont());
        assertTrue("failed: XAccessibleExtendedComponent::getTitledBorderText", _xAccExtComp._getTitledBorderText());
        assertTrue("failed: XAccessibleExtendedComponent::getToolTipText", _xAccExtComp._getToolTipText());

        System.out.println("*** Now testing XAccessibleEventBroadcaster ***");
        _XAccessibleEventBroadcaster _xAccEvBcast =
                                new _XAccessibleEventBroadcaster(testObject, xWindow);
        assertTrue("failed: XAccessibleEventBroadcaster::addEventListener", _xAccEvBcast._addEventListener(xMSF));
        assertTrue("failed: XAccessibleEventBroadcaster::removeEventListener", _xAccEvBcast._removeEventListener(xMSF));
    }




    @BeforeClass
    public static void setUpConnection() throws Exception {
        System.out.println("setUpConnection()");
        connection.setUp();
    }

    @AfterClass
    public static void tearDownConnection()
        throws InterruptedException, com.sun.star.uno.Exception
    {
        System.out.println("tearDownConnection()");
        connection.tearDown();
    }

    private static final OfficeConnection connection = new OfficeConnection();

}
