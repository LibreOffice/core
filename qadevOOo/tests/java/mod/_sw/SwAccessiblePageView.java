/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
package mod._sw;

import java.io.PrintWriter;

import lib.Status;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.AccessibilityTools;
import util.WriterTools;
import util.utils;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleValue;
import com.sun.star.awt.XWindow;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XEnumeration;
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.ControlCharacter;
import com.sun.star.text.XText;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.URL;
import com.sun.star.util.XURLTransformer;

public class SwAccessiblePageView extends TestCase {

    XTextDocument xTextDoc = null;

    /**
    * Called to create an instance of <code>TestEnvironment</code>
    * with an object to test and related objects.
    * Switchs the document to Print Preview mode.
    * Obtains accissible object for the page view.
    *
    * @param tParam test parameters
    * @param log writer to log information while testing
    *
    * @see TestEnvironment
    * @see #getTestEnvironment()
    */
    protected TestEnvironment createTestEnvironment(
        TestParameters Param, PrintWriter log) {

        XInterface oObj = null;
        XInterface port = null;
        XInterface para = null;
        XPropertySet paraP = null;
        XPropertySet portP = null;

        XText oText = xTextDoc.getText();
        XTextCursor oCursor = oText.createTextCursor();

        log.println( "inserting some lines" );
        try {
            for (int i=0; i<2; i++){
                oText.insertString( oCursor,"Paragraph Number: " + i, false);
                oText.insertString( oCursor,
                " The quick brown fox jumps over the lazy Dog: SwXParagraph",
                false);
                oText.insertControlCharacter(
                oCursor, ControlCharacter.PARAGRAPH_BREAK, false );
                oText.insertString( oCursor,
                "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG: SwXParagraph",
                false);
                oText.insertControlCharacter(oCursor,
                ControlCharacter.PARAGRAPH_BREAK, false );
                oText.insertControlCharacter(
                oCursor, ControlCharacter.LINE_BREAK, false );
            }
        } catch ( com.sun.star.lang.IllegalArgumentException e ){
            e.printStackTrace(log);
            throw new StatusException( "Couldn't insert lines", e );
        }

        // Enumeration
        XEnumerationAccess oEnumA = (XEnumerationAccess)
        UnoRuntime.queryInterface(XEnumerationAccess.class, oText );
        XEnumeration oEnum = oEnumA.createEnumeration();

        try {
            para = (XInterface) AnyConverter.toObject(
            new Type(XInterface.class),oEnum.nextElement());
            XEnumerationAccess oEnumB = (XEnumerationAccess)
            UnoRuntime.queryInterface( XEnumerationAccess.class, para );
            XEnumeration oEnum2 = oEnumB.createEnumeration();
            port = (XInterface) AnyConverter.toObject(
            new Type(XInterface.class),oEnum2.nextElement());
        } catch ( com.sun.star.lang.WrappedTargetException e ) {
            e.printStackTrace(log);
            log.println("Error: exception occurred...");
        } catch ( com.sun.star.container.NoSuchElementException e ) {
            e.printStackTrace(log);
            log.println("Error: exception occurred...");
        } catch ( com.sun.star.lang.IllegalArgumentException e ) {
            e.printStackTrace(log);
            log.println("Error: exception occurred...");
        }

        try {
            portP = (XPropertySet)
            UnoRuntime.queryInterface(XPropertySet.class, port);
            paraP = (XPropertySet)
            UnoRuntime.queryInterface(XPropertySet.class, para);
            paraP.setPropertyValue("BreakType",com.sun.star.style.BreakType.PAGE_AFTER);
        } catch ( com.sun.star.lang.WrappedTargetException e ) {
            log.println("Error, exception occurred...");
            e.printStackTrace(log);
            throw new StatusException( "Couldn't get Paragraph", e );
        } catch ( com.sun.star.lang.IllegalArgumentException e ) {
            log.println("Error, exception occurred...");
            e.printStackTrace(log);
            throw new StatusException( "Couldn't get Paragraph", e );
        } catch ( com.sun.star.beans.UnknownPropertyException e ) {
            log.println("Error, exception occurred...");
            e.printStackTrace(log);
            throw new StatusException( "Couldn't get Paragraph", e );
        } catch ( com.sun.star.beans.PropertyVetoException e ) {
            log.println("Error, exception occurred...");
            e.printStackTrace(log);
            throw new StatusException( "Couldn't get Paragraph", e );
        }

        shortWait();

        XController xController = xTextDoc.getCurrentController();

        XModel aModel = (XModel)
            UnoRuntime.queryInterface(XModel.class, xTextDoc);

        //switch to 'Print Preview' mode
        try {
            XDispatchProvider xDispProv = (XDispatchProvider)
                UnoRuntime.queryInterface(XDispatchProvider.class, xController);
            XURLTransformer xParser = (com.sun.star.util.XURLTransformer)
                UnoRuntime.queryInterface(XURLTransformer.class,
            ((XMultiServiceFactory)Param.getMSF()).createInstance("com.sun.star.util.URLTransformer"));
            // Because it's an in/out parameter we must use an array of URL objects.
            URL[] aParseURL = new URL[1];
            aParseURL[0] = new URL();
            aParseURL[0].Complete = ".uno:PrintPreview";
            xParser.parseStrict(aParseURL);
            URL aURL = aParseURL[0];
            XDispatch xDispatcher = xDispProv.queryDispatch(aURL, "", 0);
            if(xDispatcher != null)
                xDispatcher.dispatch( aURL, null );
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't change mode");
            throw new StatusException(Status.failed("Couldn't change mode"));
        }

        shortWait();

        AccessibilityTools at = new AccessibilityTools();

        XWindow xWindow = at.getCurrentWindow((XMultiServiceFactory)Param.getMSF(), aModel);
        XAccessible xRoot = at.getAccessibleObject(xWindow);

        at.printAccessibleTree(log, xRoot, Param.getBool(util.PropertyName.DEBUG_IS_ACTIVE));

        System.out.println("Panel: "+AccessibleRole.PANEL);
        System.out.println("ScrollPane: "+AccessibleRole.SCROLL_PANE);
        oObj = at.getAccessibleObjectForRole(xRoot, AccessibleRole.PANEL,"Page");

        log.println("ImplementationName " + utils.getImplName(oObj));


        TestEnvironment tEnv = new TestEnvironment(oObj);

        getAccessibleObjectForRole(xRoot, AccessibleRole.SCROLL_BAR);
        final XAccessibleValue xAccVal = (XAccessibleValue) UnoRuntime.queryInterface
                                (XAccessibleValue.class, SearchedContext) ;

        tEnv.addObjRelation("EventProducer",
            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer() {
                public void fireEvent() {
                    Integer old = (Integer) xAccVal.getCurrentValue();
                    Integer newValue = new Integer(old.intValue()+10);
                    xAccVal.setCurrentValue(newValue);
                    xAccVal.setCurrentValue(old);
                }
            });

        return tEnv;

    }

    public static boolean first = false;
    public static XAccessibleContext SearchedContext = null;

    public static void getAccessibleObjectForRole(XAccessible xacc,short role) {
        XAccessibleContext ac = xacc.getAccessibleContext();
        if (ac.getAccessibleRole()==role) {
            if (first) SearchedContext = ac;
                else first=true;
        } else {
            int k = ac.getAccessibleChildCount();
            for (int i=0;i<k;i++) {
                try {
                    getAccessibleObjectForRole(ac.getAccessibleChild(i),role);
                    if (SearchedContext != null) return ;
                } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                    System.out.println("Couldn't get Child");
                }
            }
        }
    }


    /**
    * Called while disposing a <code>TestEnvironment</code>.
    * Disposes text document.
    * @param tParam test parameters
    * @param tEnv the environment to cleanup
    * @param log writer to log information while testing
    */
    protected void cleanup( TestParameters Param, PrintWriter log) {
        log.println("dispose text document");
        util.DesktopTools.closeDoc(xTextDoc);
    }

    /**
     * Called while the <code>TestCase</code> initialization. In the
     * implementation does nothing. Subclasses can override to initialize
     * objects shared among all <code>TestEnvironment</code>s.
     *
     * @param tParam test parameters
     * @param log writer to log information while testing
     *
     * @see #initializeTestCase()
     */
    protected void initialize(TestParameters Param, PrintWriter log) {
        log.println( "creating a text document" );
        xTextDoc = WriterTools.createTextDoc((XMultiServiceFactory)Param.getMSF());
    }

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
}
