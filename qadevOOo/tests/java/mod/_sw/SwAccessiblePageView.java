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
package mod._sw;

import java.io.PrintWriter;

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
    * Switches the document to Print Preview mode.
    * Obtains accessible object for the page view.
    *
    * @param Param test parameters
    * @param log writer to log information while testing
    *
    * @see TestEnvironment
    * @see #getTestEnvironment
    */
    @Override
    protected TestEnvironment createTestEnvironment(
        TestParameters Param, PrintWriter log) throws Exception {

        XInterface oObj = null;
        XInterface port = null;
        XInterface para = null;
        XPropertySet paraP = null;
        XText oText = xTextDoc.getText();
        XTextCursor oCursor = oText.createTextCursor();

        log.println( "inserting some lines" );
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

        // Enumeration
        XEnumerationAccess oEnumA = UnoRuntime.queryInterface(XEnumerationAccess.class, oText );
        XEnumeration oEnum = oEnumA.createEnumeration();

        para = (XInterface) AnyConverter.toObject(
        new Type(XInterface.class),oEnum.nextElement());
        XEnumerationAccess oEnumB = UnoRuntime.queryInterface( XEnumerationAccess.class, para );
        XEnumeration oEnum2 = oEnumB.createEnumeration();
        port = (XInterface) AnyConverter.toObject(
        new Type(XInterface.class),oEnum2.nextElement());

        UnoRuntime.queryInterface(XPropertySet.class, port);
        paraP = UnoRuntime.queryInterface(XPropertySet.class, para);
        paraP.setPropertyValue("BreakType",com.sun.star.style.BreakType.PAGE_AFTER);

        util.utils.waitForEventIdle(Param.getMSF());

        XController xController = xTextDoc.getCurrentController();

        XModel aModel = UnoRuntime.queryInterface(XModel.class, xTextDoc);

        //switch to 'Print Preview' mode
        XDispatchProvider xDispProv = UnoRuntime.queryInterface(XDispatchProvider.class, xController);
        XURLTransformer xParser = UnoRuntime.queryInterface(XURLTransformer.class,
               Param.getMSF().createInstance("com.sun.star.util.URLTransformer"));
        // Because it's an in/out parameter we must use an array of URL objects.
        URL[] aParseURL = new URL[1];
        aParseURL[0] = new URL();
        aParseURL[0].Complete = ".uno:PrintPreview";
        xParser.parseStrict(aParseURL);
        URL aURL = aParseURL[0];
        XDispatch xDispatcher = xDispProv.queryDispatch(aURL, "", 0);
        if(xDispatcher != null)
            xDispatcher.dispatch( aURL, null );

        util.utils.waitForEventIdle(Param.getMSF());

        XWindow xWindow = AccessibilityTools.getCurrentWindow(aModel);
        XAccessible xRoot = AccessibilityTools.getAccessibleObject(xWindow);

        AccessibilityTools.printAccessibleTree(log, xRoot, Param.getBool(util.PropertyName.DEBUG_IS_ACTIVE));

        System.out.println("Panel: "+AccessibleRole.PANEL);
        System.out.println("ScrollPane: "+AccessibleRole.SCROLL_PANE);
        oObj = AccessibilityTools.getAccessibleObjectForRole(xRoot, AccessibleRole.PANEL,"Page");

        log.println("ImplementationName " + utils.getImplName(oObj));


        TestEnvironment tEnv = new TestEnvironment(oObj);

        getAccessibleObjectForRole(xRoot, AccessibleRole.SCROLL_BAR);
        final XAccessibleValue xAccVal = UnoRuntime.queryInterface
                                (XAccessibleValue.class, SearchedContext) ;

        tEnv.addObjRelation("EventProducer",
            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer() {
                public void fireEvent() {
                    Integer old = (Integer) xAccVal.getCurrentValue();
                    Integer newValue = Integer.valueOf(old.intValue()+10);
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
    * @param Param test parameters
    * @param log writer to log information while testing
    */
    @Override
    protected void cleanup( TestParameters Param, PrintWriter log) {
        log.println("dispose text document");
        util.DesktopTools.closeDoc(xTextDoc);
    }

    /**
     * Called while the <code>TestCase</code> initialization. In the
     * implementation does nothing. Subclasses can override to initialize
     * objects shared among all <code>TestEnvironment</code>s.
     *
     * @param Param test parameters
     * @param log writer to log information while testing
     *
     * @see #initializeTestCase
     */
    @Override
    protected void initialize(TestParameters Param, PrintWriter log) throws Exception {
        log.println( "creating a text document" );
        xTextDoc = WriterTools.createTextDoc(Param.getMSF());
    }
}
