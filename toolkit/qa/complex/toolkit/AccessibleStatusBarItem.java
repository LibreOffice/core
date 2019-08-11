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

import java.util.logging.Logger;
import java.util.logging.Level;
import complex.toolkit.accessibility._XAccessibleEventBroadcaster;
import complex.toolkit.accessibility._XAccessibleExtendedComponent;
import complex.toolkit.accessibility._XAccessibleText;
import complex.toolkit.accessibility._XAccessibleComponent;
import complex.toolkit.accessibility._XAccessibleContext;
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
public class AccessibleStatusBarItem {

    XMultiServiceFactory xMSF = null;
    XAccessibleContext testObject = null;
    XWindow xWindow = null;

    /**
     * Check document types
     */
    @Test
    public void checkDocs()
    {
        checkWriterDoc();
        checkMathDoc();
        checkDrawDoc();
        checkImpressDoc();
        checkCalcDoc();
    }

    private XMultiServiceFactory getMSF()
    {
        return UnoRuntime.queryInterface(XMultiServiceFactory.class, connection.getComponentContext().getServiceManager());
    }

    /**
     * Test the interfaces on a writer document
     */
    private void checkWriterDoc() {
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
            XExtendedToolkit tk =
                        UnoRuntime.queryInterface(XExtendedToolkit.class,xIfc);

            util.utils.waitForEventIdle(xMSF);
            xWindow = UnoRuntime.queryInterface(
                                    XWindow.class,tk.getActiveTopWindow());

            util.utils.waitForEventIdle(xMSF);
            XAccessible xRoot = AccessibilityTools.getAccessibleObject(xWindow);
            XAccessibleContext parentContext = null;

            System.out.println("Get the accessible status bar.");
            parentContext = AccessibilityTools.getAccessibleObjectForRole(
                                        xRoot, AccessibleRole.STATUS_BAR, "");
            util.utils.waitForEventIdle(xMSF);
            if ( parentContext == null ) {
                fail("Could not create a test object.");
            }
            System.out.println("...OK.");

            testObject=parentContext;
        }
        catch(com.sun.star.uno.Exception e) {
            Logger.getLogger( this.getClass().getName() ).log( Level.SEVERE, "caught an exception", e );
        }
        catch(Throwable t) {
            Logger.getLogger( this.getClass().getName() ).log( Level.SEVERE, "caught an exception", t );
        }
    }

    public void runAllInterfaceTests() {
        int count = testObject.getAccessibleChildCount();
        System.out.println("*****");
        System.out.println("**** Found items to test: " + count);
        for (int i=0;i<count;i++){
            System.out.println("**** Now testing StatusBarItem " + i + ".");
            XAccessible object = null;
            try {
                object = testObject.getAccessibleChild(i);
            }
            catch(com.sun.star.lang.IndexOutOfBoundsException e) {
                System.out.println("* Cannot get item Nr: " + i);
                continue;
            }
            XServiceInfo xSI = UnoRuntime.queryInterface(
                                        XServiceInfo.class,object);
            String[] services = xSI.getSupportedServiceNames();
            System.out.println("* Implementation Name: " + xSI.getImplementationName());
            String accName = object.getAccessibleContext().getAccessibleName();
            System.out.println("* Accessible Name: " + accName);
            for (int j=0; j<services.length; j++)
            {
                System.out.println("* ServiceName "+j+": "+ services[j]);
            }
            System.out.println("*****");

            System.out.println("*** Now testing XAccessibleComponent ***");
            _XAccessibleComponent _xAccCompTest =
                                    new _XAccessibleComponent(object);
            assertTrue("failed: "+accName+" - XAccessibleComponent::getBounds", _xAccCompTest._getBounds());
            assertTrue("failed: "+accName+" - XAccessibleComponent::contains", _xAccCompTest._containsPoint());
            assertTrue("failed: "+accName+" - XAccessibleComponent::getAccessibleAt", _xAccCompTest._getAccessibleAtPoint());
            assertTrue("failed: "+accName+" - XAccessibleComponent::getBackground", _xAccCompTest._getBackground());
            assertTrue("failed: "+accName+" - XAccessibleComponent::getForeground", _xAccCompTest._getForeground());
            assertTrue("failed: "+accName+" - XAccessibleComponent::getLocation", _xAccCompTest._getLocation());
            assertTrue("failed: "+accName+" - XAccessibleComponent::getLocationOnScreen", _xAccCompTest._getLocationOnScreen());
            assertTrue("failed: "+accName+" - XAccessibleComponent::getSize", _xAccCompTest._getSize());
            assertTrue("failed: "+accName+" - XAccessibleComponent::grabFocus", _xAccCompTest._grabFocus());

            System.out.println("*** Now testing XAccessibleContext ***");
            _XAccessibleContext _xAccContext =
                                    new _XAccessibleContext(object);
            assertTrue("failed: "+accName+" - XAccessibleContext::getAccessibleChildCount", _xAccContext._getAccessibleChildCount());
            assertTrue("failed: "+accName+" - XAccessibleContext::getAccessibleChild", _xAccContext._getAccessibleChild());
            assertTrue("failed: "+accName+" - XAccessibleContext::getAccessibleDescription", _xAccContext._getAccessibleDescription());
            assertTrue("failed: "+accName+" - XAccessibleContext::getAccessibleName", _xAccContext._getAccessibleName());
            assertTrue("failed: "+accName+" - XAccessibleContext::getAccessibleParent", _xAccContext._getAccessibleParent());
            assertTrue("failed: "+accName+" - XAccessibleContext::getAccessibleIndexInParent", _xAccContext._getAccessibleIndexInParent());
            assertTrue("failed: "+accName+" - XAccessibleContext::getAccessibleRelationSet", _xAccContext._getAccessibleRelationSet());
            assertTrue("failed: "+accName+" - XAccessibleContext::getAccessibleRole", _xAccContext._getAccessibleRole());
            assertTrue("failed: "+accName+" - XAccessibleContext::getAccessibleStateSet", _xAccContext._getAccessibleStateSet());
            assertTrue("failed: "+accName+" - XAccessibleContext::getLocale", _xAccContext._getLocale());

            System.out.println("*** Now testing XAccessibleExtendedComponent ***");
            _XAccessibleExtendedComponent _xAccExtComp =
                                    new _XAccessibleExtendedComponent(object);
            assertTrue("failed: "+accName+" - XAccessibleExtendedComponent::getFont", _xAccExtComp._getFont());
            assertTrue("failed: "+accName+" - XAccessibleExtendedComponent::getTitledBorderText", _xAccExtComp._getTitledBorderText());
            assertTrue("failed: "+accName+" - XAccessibleExtendedComponent::getToolTipText", _xAccExtComp._getToolTipText());

            System.out.println("*** Now testing XAccessibleEventBroadcaster ***");
            _XAccessibleEventBroadcaster _xAccEvBcast =
                                    new _XAccessibleEventBroadcaster(object, xWindow);
            assertTrue("failed: "+accName+" - XAccessibleEventBroadcaster::addEventListener", _xAccEvBcast._addEventListener(xMSF));
            assertTrue("failed: "+accName+" - XAccessibleEventBroadcaster::removeEventListener", _xAccEvBcast._removeEventListener(xMSF));

            System.out.println("*** Now testing XAccessibleText ***");
            _XAccessibleText _xAccText =
                                    new _XAccessibleText(object, xMSF, "true");
            assertTrue("failed: "+accName+" - XAccessibleText::getText", _xAccText._getText());
            assertTrue("failed: "+accName+" - XAccessibleText::getCharacterCount", _xAccText._getCharacterCount());
            assertTrue("failed: "+accName+" - XAccessibleText::getCharacterBounds", _xAccText._getCharacterBounds());
            assertTrue("failed: "+accName+" - XAccessibleText::setSelection", _xAccText._setSelection());
            assertTrue("failed: "+accName+" - XAccessibleText::copyText", _xAccText._copyText());
            assertTrue("failed: "+accName+" - XAccessibleText::getCharacter", _xAccText._getCharacter());
            assertTrue("failed: "+accName+" - XAccessibleText::getCharacterAttributes", _xAccText._getCharacterAttributes());
            assertTrue("failed: "+accName+" - XAccessibleText::getIndexAtPoint", _xAccText._getIndexAtPoint());
            assertTrue("failed: "+accName+" - XAccessibleText::getSelectedText", _xAccText._getSelectedText());
            assertTrue("failed: "+accName+" - XAccessibleText::getSelectionEnd", _xAccText._getSelectionEnd());
            assertTrue("failed: "+accName+" - XAccessibleText::getSelectionStart", _xAccText._getSelectionStart());
            assertTrue("failed: "+accName+" - XAccessibleText::getTextAtIndex", _xAccText._getTextAtIndex());
            assertTrue("failed: "+accName+" - XAccessibleText::getTextBeforeIndex", _xAccText._getTextBeforeIndex());
            assertTrue("failed: "+accName+" - XAccessibleText::getBehindIndex", _xAccText._getTextBehindIndex());
            assertTrue("failed: "+accName+" - XAccessibleText::getTextRange", _xAccText._getTextRange());
            assertTrue("failed: "+accName+" - XAccessibleText::setCaretPosition", _xAccText._setCaretPosition());
            assertTrue("failed: "+accName+" - XAccessibleText::getCaretPosition", _xAccText._getCaretPosition());
        }
    }




    @BeforeClass public static void setUpConnection() throws Exception {
        System.out.println("setUpConnection()");
        connection.setUp();
    }

    @AfterClass public static void tearDownConnection()
        throws InterruptedException, com.sun.star.uno.Exception
    {
        System.out.println("tearDownConnection()");
        connection.tearDown();
    }

    private static final OfficeConnection connection = new OfficeConnection();


}
