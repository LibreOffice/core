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
import com.sun.star.awt.XWindow;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XModel;
import com.sun.star.text.ControlCharacter;
import com.sun.star.text.XText;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.URL;
import com.sun.star.util.XURLTransformer;

public class SwAccessibleDocumentPageView extends TestCase {

    XTextDocument xTextDoc = null;

    /**
    * Called to create an instance of <code>TestEnvironment</code>
    * with an object to test and related objects.
    * Switchs the document to Print Preview mode.
    * Obtains accissible object for the document page view.
    *
    * @param Param test parameters
    * @param log writer to log information while testing
    *
    * @see TestEnvironment
    * @see #getTestEnvironment
    */
    @Override
    protected TestEnvironment createTestEnvironment(
        final TestParameters Param, PrintWriter log) throws Exception{

        XInterface oObj = null;

        XText oText = xTextDoc.getText();
        XTextCursor oCursor = oText.createTextCursor();

        log.println( "inserting some lines" );
        for (int i=0; i<25; i++){
            oText.insertString( oCursor,"Paragraph Number: " + i, false);
            oText.insertString( oCursor,
                " The quick brown fox jumps over the lazy Dog: SwAccessibleDocumentPageView",
                false);
            oText.insertControlCharacter(
                oCursor, ControlCharacter.PARAGRAPH_BREAK, false );
            oText.insertString( oCursor,
                "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG: SwAccessibleDocumentPageView",
                false);
            oText.insertControlCharacter(oCursor,
                ControlCharacter.PARAGRAPH_BREAK, false );
            oText.insertControlCharacter(
                oCursor, ControlCharacter.LINE_BREAK, false );
        }

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

        oObj = AccessibilityTools.getAccessibleObjectForRole(xRoot, AccessibleRole.DOCUMENT );

        log.println("ImplementationName " + utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment(oObj);

        final XText the_text = oText;

        tEnv.addObjRelation("EventProducer",
            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer() {
                public void fireEvent() {
                    String oldText = the_text.getString();
                    the_text.setString("EVENT FIRED");
                    util.utils.waitForEventIdle(Param.getMSF());
                    the_text.setString(oldText);
                }
            });

        return tEnv;

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
        xTextDoc = WriterTools.createTextDoc( Param.getMSF());
    }
}