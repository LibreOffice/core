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

package mod._sd;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.AccessibilityTools;
import util.SOfficeFactory;
import util.utils;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.awt.XWindow;
import com.sun.star.drawing.XDrawPages;
import com.sun.star.drawing.XDrawPagesSupplier;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.URL;
import com.sun.star.util.XURLTransformer;

public class AccessibleOutlineView extends TestCase {

    XModel aModel = null;
    XComponent xImpressDoc = null;

    /**
     * Called to create an instance of <code>TestEnvironment</code> with an
     * object to test and related objects. Subclasses should implement this
     * method to provide the implementation and related objects. The method is
     * called from <code>getTestEnvironment()</code>.
     *
     * @param Param test parameters
     * @param log writer to log information while testing
     *
     * @see TestEnvironment
     * @see #getTestEnvironment()
     */
    protected TestEnvironment createTestEnvironment
            (TestParameters Param, PrintWriter log) {
        XInterface oObj = null;

        AccessibilityTools at = new AccessibilityTools();

        XWindow xWindow = at.getCurrentWindow (
                        (XMultiServiceFactory)Param.getMSF(),aModel);
        XAccessible xRoot = at.getAccessibleObject(xWindow);

        at.getAccessibleObjectForRole(xRoot, AccessibleRole.DOCUMENT);

        oObj = AccessibilityTools.SearchedContext;

        log.println("ImplementationName "+utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment(oObj);

        XDrawPagesSupplier oDPS = UnoRuntime.queryInterface(XDrawPagesSupplier.class, aModel);
        final XDrawPages oDPn = oDPS.getDrawPages();

        tEnv.addObjRelation("EventMsg","Inserting a drawpage via API has no "+
                                        "effect to the outline view #101050# \r\n"+
                                        "Therefore the listener isn't called");

        tEnv.addObjRelation("EventProducer",
            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer() {
                public void fireEvent() {
                    oDPn.insertNewByIndex(1);
                }
            });

        return tEnv;

    }

    /**
    * Called while disposing a <code>TestEnvironment</code>.
    * Disposes Impress documents.
    * @param Param test parameters
    * @param log writer to log information while testing
    */
    protected void cleanup( TestParameters Param, PrintWriter log ) {
        log.println("disposing Impress document");
        util.DesktopTools.closeDoc(xImpressDoc);;
    }

    /**
     * Called while the <code>TestCase</code> initialization. In the
     * implementation does nothing. Subclasses can override to initialize
     * objects shared among all <code>TestEnvironment</code>s.
     *
     * @param Param test parameters
     * @param log writer to log information while testing
     *
     * @see #initializeTestCase()
     */
    protected void initialize(TestParameters Param, PrintWriter log) {
        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory(
                                (XMultiServiceFactory)Param.getMSF());

        try {
            log.println( "creating a impress document" );
            xImpressDoc = SOF.createImpressDoc(null);
            shortWait();
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace( log );
            throw new StatusException("Couldn't create document", e);
        }

        aModel = UnoRuntime.queryInterface(XModel.class, xImpressDoc);

        XInterface oObj = aModel.getCurrentController();

        //Change to Outline view
        try {
            String aSlotID = "slot:27010";
            XDispatchProvider xDispProv = UnoRuntime.queryInterface( XDispatchProvider.class, oObj );
            XURLTransformer xParser = UnoRuntime.queryInterface(XURLTransformer.class,
            ((XMultiServiceFactory)Param.getMSF()).
            createInstance("com.sun.star.util.URLTransformer"));
            // Because it's an in/out parameter we must use an array of URL objects.
            URL[] aParseURL = new URL[1];
            aParseURL[0] = new URL();
            aParseURL[0].Complete = aSlotID;
            xParser.parseStrict(aParseURL);
            URL aURL = aParseURL[0];
            XDispatch xDispatcher = xDispProv.queryDispatch( aURL,"",0);
            if( xDispatcher != null )
                    xDispatcher.dispatch( aURL, null );
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't change mode");
        }
        shortWait();
    }

    private void shortWait() {
        try {
            Thread.sleep(2000) ;
        } catch (InterruptedException e) {
            System.out.println("While waiting :" + e) ;
        }
    }
}

