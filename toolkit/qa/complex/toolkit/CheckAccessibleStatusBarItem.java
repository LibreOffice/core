/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: CheckAccessibleStatusBarItem.java,v $
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

package complex.toolkit;

import complexlib.ComplexTestCase;
import util.SOfficeFactory;
import complexlib.ComplexTestCase;
import util.SOfficeFactory;
import util.AccessibilityTools;
import complex.toolkit.interface_tests._XAccessibleComponent;
import complex.toolkit.interface_tests._XAccessibleContext;
import complex.toolkit.interface_tests._XAccessibleEventBroadcaster;
import complex.toolkit.interface_tests._XAccessibleExtendedComponent;
import complex.toolkit.interface_tests._XAccessibleText;
import com.sun.star.awt.XWindow;
import com.sun.star.chart.XChartDocument;
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
import java.io.PrintWriter;
/**
 *
 */
public class CheckAccessibleStatusBarItem extends ComplexTestCase {

    XMultiServiceFactory xMSF = null;
    XAccessibleContext testObject = null;
    XWindow xWindow = null;


    public String[] getTestMethodNames() {
        return new String[]{"checkDocs"};//{"checkWriterDoc", "checkDrawDoc",
//                    "checkMathDoc", "checkImpressDoc", "checkCalcDoc"};
    }

    /**
    * Sleeps for 0.5 sec. to allow StarOffice to react on <code>
    * reset</code> call.
    */
    private void shortWait() {
        shortWait(500) ;
    }

    /**
    * Sleeps for a certain time.
    * @param Thread is sleeping for this time in milliseconds.
    */
    private void shortWait(int time) {
        try {
            Thread.sleep(500);
        } catch (InterruptedException e) {
            log.println("While waiting :" + e) ;
        }
    }

    /**
     * Check document types
     */
    public void checkDocs() {
        Object doc = param.get("DocType");
        String testDocType;
        if (doc == null)
            testDocType = "all";
        else
            testDocType = (String)doc;

        System.out.println("Param was " + doc);
        System.out.println("DocType " + testDocType);
        if (testDocType.equalsIgnoreCase("writer") || testDocType.equalsIgnoreCase("all"))
            checkWriterDoc();
        if (testDocType.equalsIgnoreCase("math") || testDocType.equalsIgnoreCase("all"))
            checkMathDoc();
        if (testDocType.equalsIgnoreCase("draw") || testDocType.equalsIgnoreCase("all"))
            checkDrawDoc();
        if (testDocType.equalsIgnoreCase("impress") || testDocType.equalsIgnoreCase("all"))
            checkImpressDoc();
        if (testDocType.equalsIgnoreCase("calc") || testDocType.equalsIgnoreCase("all"))
            checkCalcDoc();

    }

    /**
     * Test the interfaces on a writer document
     */
    public void checkWriterDoc() {
        xMSF = (XMultiServiceFactory)param.getMSF();
        SOfficeFactory xSOF = SOfficeFactory.getFactory(xMSF);
        XTextDocument xTextDoc = null;
        try {
            log.println("****** Open a new writer document");
            xTextDoc = xSOF.createTextDoc("_blank");
            getTestObject();
        }
        catch(com.sun.star.uno.Exception e) {
            e.printStackTrace();
        }
        runAllInterfaceTests();

        if (xTextDoc != null) {
            XCloseable xClose = (XCloseable)UnoRuntime.queryInterface(XCloseable.class, xTextDoc);
            try {
                xClose.close(false);
            }
            catch(com.sun.star.util.CloseVetoException e) {
                e.printStackTrace();
            }
        }
    }

    /**
     * Test the interfaces on a math document
     */
    public void checkMathDoc() {
        xMSF = (XMultiServiceFactory)param.getMSF();
        SOfficeFactory xSOF = SOfficeFactory.getFactory(xMSF);
        XComponent xMathDoc = null;
        try {
            log.println("****** Open a new math document");
            xMathDoc = xSOF.createMathDoc("_blank");
            getTestObject();
        }
        catch(com.sun.star.uno.Exception e) {
            e.printStackTrace();
        }
        runAllInterfaceTests();

        if (xMathDoc != null) {
            XCloseable xClose = (XCloseable)UnoRuntime.queryInterface(XCloseable.class, xMathDoc);
            try {
                xClose.close(false);
            }
            catch(com.sun.star.util.CloseVetoException e) {
                e.printStackTrace();
            }
        }
    }

    /**
     * Test the interfaces on a draw document
     */
    public void checkDrawDoc() {
        xMSF = (XMultiServiceFactory)param.getMSF();
        SOfficeFactory xSOF = SOfficeFactory.getFactory(xMSF);
        XComponent xDrawDoc = null;
        try {
            log.println("****** Open a new draw document");
            xDrawDoc = xSOF.createDrawDoc("_blank");
            getTestObject();
        }
        catch(com.sun.star.uno.Exception e) {
            e.printStackTrace();
        }
        runAllInterfaceTests();

        if (xDrawDoc != null) {
            XCloseable xClose = (XCloseable)UnoRuntime.queryInterface(XCloseable.class, xDrawDoc);
            try {
                xClose.close(false);
            }
            catch(com.sun.star.util.CloseVetoException e) {
                e.printStackTrace();
            }
        }
    }

    /**
     * Test the interfaces on an impress document
     */
    public void checkImpressDoc() {
        xMSF = (XMultiServiceFactory)param.getMSF();
        SOfficeFactory xSOF = SOfficeFactory.getFactory(xMSF);
        XComponent xImpressDoc = null;
        try {
            log.println("****** Open a new impress document");
            xImpressDoc = xSOF.createImpressDoc("_blank");
            getTestObject();
        }
        catch(com.sun.star.uno.Exception e) {
            e.printStackTrace();
        }
        runAllInterfaceTests();

        if (xImpressDoc != null) {
            XCloseable xClose = (XCloseable)UnoRuntime.queryInterface(XCloseable.class, xImpressDoc);
            try {
                xClose.close(false);
            }
            catch(com.sun.star.util.CloseVetoException e) {
                e.printStackTrace();
            }
        }
    }
    /**
     * Test the interfaces on an calc document
     */
    public void checkCalcDoc() {
        xMSF = (XMultiServiceFactory)param.getMSF();
        SOfficeFactory xSOF = SOfficeFactory.getFactory(xMSF);
        XSpreadsheetDocument xSpreadsheetDoc = null;
        try {
            log.println("****** Open a new calc document");
            xSpreadsheetDoc = xSOF.createCalcDoc("_blank");
            shortWait();
            getTestObject();
        }
        catch(com.sun.star.uno.Exception e) {
            e.printStackTrace();
        }
        runAllInterfaceTests();

        if (xSpreadsheetDoc != null) {
            XCloseable xClose = (XCloseable)UnoRuntime.queryInterface(XCloseable.class, xSpreadsheetDoc);
            try {
                xClose.close(false);
            }
            catch(com.sun.star.util.CloseVetoException e) {
                e.printStackTrace();
            }
        }
    }

    public void getTestObject() {
        try {
            XInterface xIfc = (XInterface) xMSF.createInstance(
                                            "com.sun.star.awt.Toolkit") ;
            XExtendedToolkit tk = (XExtendedToolkit)
                        UnoRuntime.queryInterface(XExtendedToolkit.class,xIfc);

            shortWait();
            xWindow = (XWindow)UnoRuntime.queryInterface(
                                    XWindow.class,tk.getActiveTopWindow());

            shortWait();
            AccessibilityTools at = new AccessibilityTools();
            XAccessible xRoot = at.getAccessibleObject(xWindow);
            XAccessibleContext parentContext = null;

            log.println("Get the accessible status bar.");
            parentContext = at.getAccessibleObjectForRole(
                                        xRoot, AccessibleRole.STATUS_BAR, "");
            shortWait();
            if ( parentContext == null ) {
                log.println("Could not get the test object: set the correct focus in the next 30 seconds.");
                shortWait(30000);
                parentContext = at.getAccessibleObjectForRole(
                                        xRoot, AccessibleRole.STATUS_BAR, "");

                if ( parentContext == null )
                    failed("Could not create a test object.");
            }
            log.println("...OK.");

            testObject=parentContext;
        }
        catch(com.sun.star.uno.Exception e) {
            e.printStackTrace();
        }
        catch(Throwable t) {
            System.out.println("Got throwable:");
            t.printStackTrace();
        }
    }

    public void runAllInterfaceTests() {
        int count = testObject.getAccessibleChildCount();
        log.println("*****");
        log.println("**** Found items to test: " + count);
        for (int i=0;i<count;i++){
            log.println("**** Now testing StatusBarItem " + i + ".");
            XAccessible object = null;
            try {
                object = testObject.getAccessibleChild(i);
            }
            catch(com.sun.star.lang.IndexOutOfBoundsException e) {
                System.out.println("* Cannot get item Nr: " + i);
                continue;
            }
            XServiceInfo xSI = (XServiceInfo)UnoRuntime.queryInterface(
                                        XServiceInfo.class,object);
            String[] services = xSI.getSupportedServiceNames();
            log.println("* Implementation Name: " + xSI.getImplementationName());
            String accName = object.getAccessibleContext().getAccessibleName();
            log.println("* Accessible Name: " + accName);
            for (int j=0; i<services.length; i++)
                log.println("* ServiceName "+i+": "+ services[j]);
            log.println("*****");

            log.println("*** Now testing XAccessibleComponent ***");
            _XAccessibleComponent _xAccCompTest =
                                    new _XAccessibleComponent(object, log);
            assure("failed: "+accName+" - XAccessibleComponent::getBounds", _xAccCompTest._getBounds(), true);
            assure("failed: "+accName+" - XAccessibleComponent::contains", _xAccCompTest._containsPoint(), true);
            assure("failed: "+accName+" - XAccessibleComponent::getAccessibleAt", _xAccCompTest._getAccessibleAtPoint(), true);
            assure("failed: "+accName+" - XAccessibleComponent::getBackground", _xAccCompTest._getBackground(), true);
            assure("failed: "+accName+" - XAccessibleComponent::getForeground", _xAccCompTest._getForeground(), true);
            assure("failed: "+accName+" - XAccessibleComponent::getLocation", _xAccCompTest._getLocation(), true);
            assure("failed: "+accName+" - XAccessibleComponent::getLocationOnScreen", _xAccCompTest._getLocationOnScreen(), true);
            assure("failed: "+accName+" - XAccessibleComponent::getSize", _xAccCompTest._getSize(), true);
            assure("failed: "+accName+" - XAccessibleComponent::grabFocus", _xAccCompTest._grabFocus(), true);

            log.println("*** Now testing XAccessibleContext ***");
            _XAccessibleContext _xAccContext =
                                    new _XAccessibleContext(object, log);
            assure("failed: "+accName+" - XAccessibleContext::getAccessibleChildCount", _xAccContext._getAccessibleChildCount(), true);
            assure("failed: "+accName+" - XAccessibleContext::getAccessibleChild", _xAccContext._getAccessibleChild(), true);
            assure("failed: "+accName+" - XAccessibleContext::getAccessibleDescription", _xAccContext._getAccessibleDescription(), true);
            assure("failed: "+accName+" - XAccessibleContext::getAccessibleName", _xAccContext._getAccessibleName(), true);
            assure("failed: "+accName+" - XAccessibleContext::getAccessibleParent", _xAccContext._getAccessibleParent(), true);
            assure("failed: "+accName+" - XAccessibleContext::getAccessibleIndexInParent", _xAccContext._getAccessibleIndexInParent(), true);
            assure("failed: "+accName+" - XAccessibleContext::getAccessibleRelationSet", _xAccContext._getAccessibleRelationSet(), true);
            assure("failed: "+accName+" - XAccessibleContext::getAccessibleRole", _xAccContext._getAccessibleRole(), true);
            assure("failed: "+accName+" - XAccessibleContext::getAccessibleStateSet", _xAccContext._getAccessibleStateSet(), true);
            assure("failed: "+accName+" - XAccessibleContext::getLocale", _xAccContext._getLocale(), true);

            log.println("*** Now testing XAccessibleExtendedComponent ***");
            _XAccessibleExtendedComponent _xAccExtComp =
                                    new _XAccessibleExtendedComponent(object, log);
            assure("failed: "+accName+" - XAccessibleExtendedComponent::getFont", _xAccExtComp._getFont(), true);
            assure("failed: "+accName+" - XAccessibleExtendedComponent::getTitledBorderText", _xAccExtComp._getTitledBorderText(), true);
            assure("failed: "+accName+" - XAccessibleExtendedComponent::getToolTipText", _xAccExtComp._getToolTipText(), true);

            log.println("*** Now testing XAccessibleEventBroadcaster ***");
            _XAccessibleEventBroadcaster _xAccEvBcast =
                                    new _XAccessibleEventBroadcaster(object, log, "Pfff", xWindow);
            assure("failed: "+accName+" - XAccessibleEventBroadcaster::addEventListener", _xAccEvBcast._addEventListener(), true);
            assure("failed: "+accName+" - XAccessibleEventBroadcaster::removeEventListener", _xAccEvBcast._removeEventListener(), true);

            log.println("*** Now testing XAccessibleText ***");
            _XAccessibleText _xAccText =
                                    new _XAccessibleText(object, log, xMSF, "true");
            assure("failed: "+accName+" - XAccessibleText::getText", _xAccText._getText(), true);
            assure("failed: "+accName+" - XAccessibleText::getCharacterCount", _xAccText._getCharacterCount(), true);
            assure("failed: "+accName+" - XAccessibleText::getCharacterBounds", _xAccText._getCharacterBounds(), true);
            assure("failed: "+accName+" - XAccessibleText::setSelection", _xAccText._setSelection(), true);
            assure("failed: "+accName+" - XAccessibleText::copyText", _xAccText._copyText(), true);
            assure("failed: "+accName+" - XAccessibleText::getCharacter", _xAccText._getCharacter(), true);
            assure("failed: "+accName+" - XAccessibleText::getCharacterAttributes", _xAccText._getCharacterAttributes(), true);
            assure("failed: "+accName+" - XAccessibleText::getIndexAtPoint", _xAccText._getIndexAtPoint(), true);
            assure("failed: "+accName+" - XAccessibleText::getSelectedText", _xAccText._getSelectedText(), true);
            assure("failed: "+accName+" - XAccessibleText::getSelectionEnd", _xAccText._getSelectionEnd(), true);
            assure("failed: "+accName+" - XAccessibleText::getSelectionStart", _xAccText._getSelectionStart(), true);
            assure("failed: "+accName+" - XAccessibleText::getTextAtIndex", _xAccText._getTextAtIndex(), true);
            assure("failed: "+accName+" - XAccessibleText::getTextBeforeIndex", _xAccText._getTextBeforeIndex(), true);
            assure("failed: "+accName+" - XAccessibleText::getBehindIndex", _xAccText._getTextBehindIndex(), true);
            assure("failed: "+accName+" - XAccessibleText::getTextRange", _xAccText._getTextRange(), true);
            assure("failed: "+accName+" - XAccessibleText::setCaretPosition", _xAccText._setCaretPosition(), true);
            assure("failed: "+accName+" - XAccessibleText::getCaretPosition", _xAccText._getCaretPosition(), true);
        }
    }

}
