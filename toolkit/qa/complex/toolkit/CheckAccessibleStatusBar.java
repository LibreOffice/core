/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: CheckAccessibleStatusBar.java,v $
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
import util.AccessibilityTools;
import complex.toolkit.interface_tests._XAccessibleComponent;
import complex.toolkit.interface_tests._XAccessibleContext;
import complex.toolkit.interface_tests._XAccessibleExtendedComponent;
import complex.toolkit.interface_tests._XAccessibleEventBroadcaster;
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
public class CheckAccessibleStatusBar extends ComplexTestCase {

    XInterface testObject = null;
    XMultiServiceFactory xMSF = null;
    XWindow xWindow = null;

    public String[] getTestMethodNames() {
        return new String[]{"checkDocs"};//WriterDoc", "checkDrawDoc",
//                    "checkMathDoc", "checkImpressDoc", "checkCalcDoc"};
    }

/*    public String getTestObjectName() {
        return "com.sun.star.awt.AccessibleStatusBar";
    }
*/
    /**
    * Sleeps for 0.5 sec. to allow StarOffice to react on <code>
    * reset</code> call.
    */
    private void shortWait() {
        try {
            Thread.sleep(500) ;
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
            log.println("...OK.");

            XServiceInfo xSI = (XServiceInfo)UnoRuntime.queryInterface(
                                        XServiceInfo.class,parentContext);
            String[] services = xSI.getSupportedServiceNames();
            log.println("*****");
            log.println("* Implementation Name: " + xSI.getImplementationName());
            for (int i=0; i<services.length; i++)
                log.println("* ServiceName "+i+": "+ services[i]);
            log.println("*****");
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
        log.println("*** Now testing XAccessibleComponent ***");
        _XAccessibleComponent _xAccCompTest =
                                new _XAccessibleComponent(testObject, log);
        assure("failed: XAccessibleComponent::getBounds", _xAccCompTest._getBounds());
        assure("failed: XAccessibleComponent::contains", _xAccCompTest._containsPoint());
        assure("failed: XAccessibleComponent::getAccessibleAt", _xAccCompTest._getAccessibleAtPoint());
        assure("failed: XAccessibleComponent::getBackground", _xAccCompTest._getBackground());
        assure("failed: XAccessibleComponent::getForeground", _xAccCompTest._getForeground());
        assure("failed: XAccessibleComponent::getLocation", _xAccCompTest._getLocation());
        assure("failed: XAccessibleComponent::getLocationOnScreen", _xAccCompTest._getLocationOnScreen());
        assure("failed: XAccessibleComponent::getSize", _xAccCompTest._getSize());
        assure("failed: XAccessibleComponent::grabFocus", _xAccCompTest._grabFocus());

        log.println("*** Now testing XAccessibleContext ***");
        _XAccessibleContext _xAccContext =
                                new _XAccessibleContext(testObject, log);
        assure("failed: XAccessibleContext::getAccessibleChildCount", _xAccContext._getAccessibleChildCount());
        assure("failed: XAccessibleContext::getAccessibleChild", _xAccContext._getAccessibleChild());
        assure("failed: XAccessibleContext::getAccessibleDescription", _xAccContext._getAccessibleDescription());
        assure("failed: XAccessibleContext::getAccessibleName", _xAccContext._getAccessibleName());
        assure("failed: XAccessibleContext::getAccessibleParent", _xAccContext._getAccessibleParent());
        assure("failed: XAccessibleContext::getAccessibleIndexInParent", _xAccContext._getAccessibleIndexInParent());
        assure("failed: XAccessibleContext::getAccessibleRelationSet", _xAccContext._getAccessibleRelationSet());
        assure("failed: XAccessibleContext::getAccessibleRole", _xAccContext._getAccessibleRole());
        assure("failed: XAccessibleContext::getAccessibleStateSet", _xAccContext._getAccessibleStateSet());
        assure("failed: XAccessibleContext::getLocale", _xAccContext._getLocale());

        log.println("*** Now testing XAccessibleExtendedComponent ***");
        _XAccessibleExtendedComponent _xAccExtComp =
                                new _XAccessibleExtendedComponent(testObject, log);
        assure("failed: XAccessibleExtendedComponent::getFont", _xAccExtComp._getFont());
        assure("failed: XAccessibleExtendedComponent::getTitledBorderText", _xAccExtComp._getTitledBorderText());
        assure("failed: XAccessibleExtendedComponent::getToolTipText", _xAccExtComp._getToolTipText());

        log.println("*** Now testing XAccessibleEventBroadcaster ***");
        _XAccessibleEventBroadcaster _xAccEvBcast =
                                new _XAccessibleEventBroadcaster(testObject, log, "Pfff", xWindow);
        assure("failed: XAccessibleEventBroadcaster::addEventListener", _xAccEvBcast._addEventListener());
        assure("failed: XAccessibleEventBroadcaster::removeEventListener", _xAccEvBcast._removeEventListener());
    }
}
